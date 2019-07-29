// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.
// These tests are for the cdm engine, and code below it in the stack.  In
// particular, we assume that the OEMCrypo layer works, and has a valid keybox.
// This is because we need a valid RSA certificate, and will attempt to connect
// to the provisioning server to request one if we don't.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "cdm_engine.h"
#include "config_test_env.h"
#include "initialization_data.h"
#include "file_store.h"
#include "license_request.h"
#include "log.h"
#include "metrics.pb.h"
#include "OEMCryptoCENC.h"
#include "properties.h"
#include "scoped_ptr.h"
#include "string_conversions.h"
#include "test_printers.h"
#include "url_request.h"
#include "wv_cdm_constants.h"
#include "wv_cdm_types.h"

namespace wvcdm {

using drm_metrics::WvCdmMetrics;
using drm_metrics::DistributionMetric;

namespace {

// Http OK response code.
const int kHttpOk = 200;

// Default license server, can be configured using --server command line option
// Default key id (pssh), can be configured using --keyid command line option
std::string g_client_auth;
KeyId g_key_id_pssh;
KeyId g_key_id_unwrapped;
CdmKeySystem g_key_system;
std::string g_license_server;
std::string g_provisioning_server;
std::string g_provisioning_service_certificate;
std::string g_license_service_certificate;
KeyId g_wrong_key_id;

const std::string kCencMimeType = "video/mp4";
const std::string kWebmMimeType = "video/webm";

static void CommonSetup(ServerConfigurationId which,
                        bool bin_prov = false) {

  Properties::set_provisioning_messages_are_binary(bin_prov);
  Properties::Init();

  // NOTE: Select configuration
  ConfigTestEnv config(which);

  g_client_auth.assign(config.client_auth());
  g_key_system.assign(config.key_system());
  g_wrong_key_id.assign(config.wrong_key_id());
  g_license_server.assign(config.license_server());
  g_key_id_pssh.assign(a2bs_hex(config.key_id()));
  g_provisioning_service_certificate.assign(
      config.provisioning_service_certificate());
  g_license_service_certificate.assign(config.license_service_certificate());
  g_provisioning_server.assign(config.provisioning_server());

  // Extract the key ID from the PSSH box.
  InitializationData extractor(CENC_INIT_DATA_FORMAT, g_key_id_pssh);
  g_key_id_unwrapped = extractor.data();
}

/*
 * Locate the portion of the server's response message that is between
 * the strings jason_start_substr and json_end_substr. Returns the string
 * through *result. If the start substring match fails, assume the entire
 * string represents a serialized protobuf mesaage and return true with
 * the entire string. If the end_substring match fails, return false with
 * an empty *result.
 */
bool ExtractSignedMessage(const std::string& response,
                          const std::string& json_start_substr,
                          const std::string& json_end_substr,
                          std::string* result) {
  std::string response_string;
  size_t start = response.find(json_start_substr);

  if (start == response.npos) {
    // Assume serialized protobuf message.
    result->assign(response);
  } else {
    // Assume JSON-wrapped protobuf.
    size_t end = response.find(json_end_substr,
                               start + json_start_substr.length());
    if (end == response.npos) {
      LOGE("ExtractSignedMessage cannot locate end substring");
      result->clear();
      return false;
    }
    size_t result_string_size = end - start - json_start_substr.length();
    result->assign(response, start + json_start_substr.length(),
                   result_string_size);
  }

  if (result->empty()) {
    LOGE("ExtractSignedMessage: Response message is empty");
    return false;
  }
  return true;
}

}  // namespace

class WvCdmEnginePreProvTest : public testing::Test {
 public:
  WvCdmEnginePreProvTest() : cdm_engine_(&file_system_),
                             session_opened_(false) {}

  virtual ~WvCdmEnginePreProvTest() {}

  virtual void SetUp() {
    session_opened_ = false;
  }

  virtual void OpenSession() {
    CdmResponseType status =
        cdm_engine_.OpenSession(g_key_system, NULL, NULL, &session_id_);
    if (status == NEED_PROVISIONING) {
      Provision();
      status = cdm_engine_.OpenSession(g_key_system, NULL, NULL, &session_id_);
    }
    ASSERT_EQ(status, NO_ERROR);
    ASSERT_NE("", session_id_) << "Could not open CDM session.";
    ASSERT_TRUE(cdm_engine_.IsOpenSession(session_id_));
    session_opened_ = true;
  }

  virtual void TearDown() {
    if (cdm_engine_.IsProvisioned(kSecurityLevelL1)) {
      cdm_engine_.Unprovision(kSecurityLevelL1);
    }
    if (cdm_engine_.IsProvisioned(kSecurityLevelL3)) {
      cdm_engine_.Unprovision(kSecurityLevelL3);
    }
    if (session_opened_) {
      cdm_engine_.CloseSession(session_id_);
      session_opened_ = false;
    }
  }

 protected:

  // Trade request for response via the license server.
  virtual bool LicenseServerRequestResponse(const std::string& request,
                                            std::string* response) {
    LOGV("LicenseServerRequestResponse: server url: %s",
         g_license_server.c_str());
    UrlRequest url_request(g_license_server + g_client_auth);
    url_request.PostRequest(request);

    std::string http_response;
    if (!url_request.GetResponse(&http_response)) {
      return false;
    }

    LOGV("http_response:\n%s\n", http_response.c_str());

    // Separate message from HTTP headers.
    LicenseRequest license_request;
    license_request.GetDrmMessage(http_response, *response);

    LOGV("response: size=%u, string:\n%s\n", response->size(),
         Base64SafeEncode(std::vector<uint8_t>(response->begin(),
                                               response->end())).c_str());
    return true;
  }

  virtual void Provision() {
    LOGV("WvCdmEnginePreProvTest::Provision: url=%s",
         g_provisioning_server.c_str());
    CdmProvisioningRequest prov_request;
    std::string provisioning_server_url;
    CdmCertificateType cert_type = kCertificateWidevine;
    std::string cert_authority;
    std::string cert, wrapped_key;
    // Keep a crypto session alive so that OEMCrypto won't terminate while we
    // try to provision. This is needed for testing nonce floods.
    CryptoSession keep_alive(cdm_engine_.GetMetrics()->GetCryptoMetrics());

    CdmResponseType result = NO_ERROR;
    for(int i = 0; i < 2; ++i) {  // Retry once if there is a nonce problem.
      result = cdm_engine_.GetProvisioningRequest(
          cert_type, cert_authority, g_provisioning_service_certificate,
          &prov_request, &provisioning_server_url);
      if (result == CERT_PROVISIONING_NONCE_GENERATION_ERROR) {
        LOGW("Woops.  Nonce problem.  Try again?");
        sleep(1);
      } else {
        break;
      }
    }
    ASSERT_EQ(NO_ERROR, result);

    LOGV("WvCdmEnginePreProvTest::Provision: req=%s", prov_request.c_str());

    // Ignore URL provided by CdmEngine.  Use ours, as configured
    // for test vs. production server.
    provisioning_server_url.assign(g_provisioning_server);
    UrlRequest url_request(provisioning_server_url);
    EXPECT_TRUE(url_request.is_connected());
    url_request.PostCertRequestInQueryString(prov_request);
    std::string http_message;
    bool ok = url_request.GetResponse(&http_message);
    EXPECT_TRUE(ok) << http_message;

    LOGV("WvCdmEnginePreProvTest::Provision: http_message: \n%s\n",
         http_message.c_str());

    ASSERT_EQ(NO_ERROR,
              cdm_engine_.HandleProvisioningResponse(http_message,
                                                     &cert, &wrapped_key))
        << "message = " << http_message;
  }

  FileSystem file_system_;
  CdmEngine cdm_engine_;
  bool session_opened_;
  std::string key_msg_;
  std::string session_id_;
};

class WvCdmEnginePreProvTestStaging : public WvCdmEnginePreProvTest {
 public:
  WvCdmEnginePreProvTestStaging() {}

  virtual ~WvCdmEnginePreProvTestStaging() {}

  static void SetUpTestCase() {
    // NOTE: Select server configuration
    CommonSetup(kContentProtectionStagingServer);
  }
};

class WvCdmEnginePreProvTestProd : public WvCdmEnginePreProvTest {
 public:
  WvCdmEnginePreProvTestProd() {}

  virtual ~WvCdmEnginePreProvTestProd() {}

  static void SetUpTestCase() {
    // NOTE: Select server configuration
    CommonSetup(kContentProtectionProductionServer);
  }
};

class WvCdmEnginePreProvTestUat : public WvCdmEnginePreProvTest {
 public:
  WvCdmEnginePreProvTestUat() {}

  virtual ~WvCdmEnginePreProvTestUat() {}

  static void SetUpTestCase() {
    // NOTE: Select server configuration
    CommonSetup(kContentProtectionUatServer);
  }
};

class WvCdmEnginePreProvTestUatBinary : public WvCdmEnginePreProvTest {
 public:
  WvCdmEnginePreProvTestUatBinary() {}

  virtual ~WvCdmEnginePreProvTestUatBinary() {}

  static void SetUpTestCase() {
    // NOTE: Select server configuration
    // Override default setting of provisioning_messages_are_binary property
    CommonSetup(kContentProtectionUatServer, true);
  }

 protected:

  virtual void Provision() {
    LOGV("WvCdmEnginePreProvTestProv30Binary::Provision: url=%s",
         g_provisioning_server.c_str());
    CdmProvisioningRequest binary_prov_request;
    std::string provisioning_server_url;
    CdmCertificateType cert_type = kCertificateWidevine;
    std::string cert_authority;
    std::string cert, wrapped_key;
    ASSERT_EQ(NO_ERROR, cdm_engine_.GetProvisioningRequest(
        cert_type, cert_authority, g_provisioning_service_certificate,
        &binary_prov_request, &provisioning_server_url));

    // prov_request is binary - base64 encode it
    std::string prov_request(Base64SafeEncodeNoPad(
        std::vector<uint8_t>(binary_prov_request.begin(),
                             binary_prov_request.end())));

    LOGV("WvCdmEnginePreProvTest::Provision: req=%s", prov_request.c_str());

    // Ignore URL provided by CdmEngine.  Use ours, as configured
    // for test vs. production server.
    provisioning_server_url.assign(g_provisioning_server);
    UrlRequest url_request(provisioning_server_url);
    EXPECT_TRUE(url_request.is_connected());
    url_request.PostCertRequestInQueryString(prov_request);
    std::string http_message;
    bool ok = url_request.GetResponse(&http_message);
    EXPECT_TRUE(ok);

    LOGV("WvCdmEnginePreProvTest::Provision: http_message: \n%s\n",
         http_message.c_str());

    // extract provisioning response from received message
    // Extracts signed response from JSON string, result is serialized protobuf.
    const std::string kMessageStart = "\"signedResponse\": \"";
    const std::string kMessageEnd = "\"";
    std::string protobuf_response;
    EXPECT_TRUE (ExtractSignedMessage(http_message, kMessageStart, kMessageEnd,
                                      &protobuf_response)) <<
        "Failed to extract signed serialized response from JSON response";

    LOGV("WvCdmEnginePreProvTest::Provision: extracted response "
         "message: \n%s\n", protobuf_response.c_str());

    // base64 decode response to yield binary protobuf
    std::vector<uint8_t> response_vec(Base64SafeDecode(
        std::string(protobuf_response.begin(), protobuf_response.end())));
    std::string binary_protobuf_response(response_vec.begin(),
                                         response_vec.end());
    ASSERT_EQ(NO_ERROR,
              cdm_engine_.HandleProvisioningResponse(binary_protobuf_response,
                                                     &cert, &wrapped_key));
  }

};

class WvCdmEngineTest : public WvCdmEnginePreProvTest {
 public:
  WvCdmEngineTest() {}

  static void SetUpTestCase() {
    // NOTE: Select server configuration
    CommonSetup(kContentProtectionUatServer);
  }

  virtual void SetUp() {
    CdmResponseType status =
        cdm_engine_.OpenSession(g_key_system, NULL, NULL, &session_id_);
    if (status == NEED_PROVISIONING) {
      Provision();
      status = cdm_engine_.OpenSession(g_key_system, NULL, NULL, &session_id_);
    }
    ASSERT_EQ(NO_ERROR, status);
    ASSERT_NE("", session_id_) << "Could not open CDM session.";
    ASSERT_TRUE(cdm_engine_.IsOpenSession(session_id_));
  }

 protected:

  void GenerateKeyRequest(const std::string& key_id,
                          const std::string& init_data_type_string) {
    CdmAppParameterMap app_parameters;
    CdmKeySetId key_set_id;

    InitializationData init_data(init_data_type_string, key_id);

    CdmKeyRequest key_request;

    CdmResponseType result = NO_ERROR;
    for(int i=0; i < 2; i++) {  // Retry once if there is a nonce problem.
      result = cdm_engine_.GenerateKeyRequest(
          session_id_, key_set_id, init_data,
          kLicenseTypeStreaming, app_parameters,
          &key_request);
      if (result == LICENSE_REQUEST_NONCE_GENERATION_ERROR) {
        LOGW("Woops.  Nonce problem.  Try again?");
        sleep(1);
      } else {
        break;
      }
    }
    EXPECT_EQ(KEY_MESSAGE, result);

    key_msg_ = key_request.message;
    EXPECT_EQ(kKeyRequestTypeInitial, key_request.type);
  }

  void GenerateRenewalRequest() {
    CdmKeyRequest request;
    CdmResponseType result = NO_ERROR;
    for (int i = 0; i < 2; i++) {  // Retry once if there is a nonce problem.
      result = cdm_engine_.GenerateRenewalRequest(session_id_, &request);
      if (result == LICENSE_RENEWAL_NONCE_GENERATION_ERROR) {
        LOGW("Woops.  Nonce problem.  Try again?");
        sleep(1);
      } else {
        break;
      }
    }
    EXPECT_EQ(KEY_MESSAGE, result);

    key_msg_ = request.message;
    server_url_ = request.url;
  }

  std::string GetKeyRequestResponse(const std::string& server_url,
                                    const std::string& client_auth) {
    return GetKeyRequestResponse(server_url, client_auth, true);
  }

  std::string FailToGetKeyRequestResponse(const std::string& server_url,
                                          const std::string& client_auth) {
    return GetKeyRequestResponse(server_url, client_auth, false);
  }

  // posts a request and extracts the drm message from the response
  std::string GetKeyRequestResponse(const std::string& server_url,
                                    const std::string& client_auth,
                                    bool expect_success) {
    // Use secure connection and chunk transfer coding.

    LOGV("GetKeyRequestResponse: server_url: %s", server_url.c_str());

    UrlRequest url_request(server_url + client_auth);
    if (!url_request.is_connected()) {
      return "";
    }

    url_request.PostRequest(key_msg_);
    std::string response;
    bool ok = url_request.GetResponse(&response);
    LOGV("response: %s\n", response.c_str());
    EXPECT_TRUE(ok);

    int status_code = url_request.GetStatusCode(response);
    if (expect_success) EXPECT_EQ(kHttpOk, status_code)
                            << "Error response: " << response;

    if (status_code != kHttpOk) {
      return "";
    } else {
      std::string drm_msg;
      LicenseRequest lic_request;
      lic_request.GetDrmMessage(response, drm_msg);
      LOGV("drm msg: %u bytes\r\n%s", drm_msg.size(),
           HexEncode(reinterpret_cast<const uint8_t*>(drm_msg.data()),
                     drm_msg.size()).c_str());
      return drm_msg;
    }
  }

  void VerifyNewKeyResponse(const std::string& server_url,
                            const std::string& client_auth) {
    std::string resp = GetKeyRequestResponse(server_url, client_auth);
    CdmKeySetId key_set_id;
    EXPECT_EQ(KEY_ADDED,
              cdm_engine_.AddKey(session_id_, resp, &key_set_id));
    VerifyLicenseRequestLatency(kKeyRequestTypeInitial,
                                *cdm_engine_.GetMetrics());
  }

  void VerifyRenewalKeyResponse(const std::string& server_url,
                                const std::string& client_auth) {
    std::string resp = GetKeyRequestResponse(server_url, client_auth);
    EXPECT_EQ(KEY_ADDED, cdm_engine_.RenewKey(session_id_, resp));
    VerifyLicenseRequestLatency(kKeyRequestTypeRenewal,
                                *cdm_engine_.GetMetrics());
  }

  void VerifyLicenseRequestLatency(
      CdmKeyRequestType key_request_type,
      const metrics::EngineMetrics& engine_metrics) {
    WvCdmMetrics metrics_proto;
    engine_metrics.Serialize(&metrics_proto);
    bool has_request_type = false;
    for (int i = 0; i < metrics_proto.session_metrics_size(); i++) {
      WvCdmMetrics::SessionMetrics session_metrics =
          metrics_proto.session_metrics(i);
      for (int j = 0;
           j < session_metrics.cdm_session_license_request_latency_ms_size();
           j++) {
        DistributionMetric latency_distribution =
            session_metrics.cdm_session_license_request_latency_ms(j);
        if (latency_distribution.attributes().key_request_type()
            == key_request_type && latency_distribution.operation_count() > 0) {
          has_request_type = true;
        }
      }
    }
    std::string serialized_metrics;
    ASSERT_TRUE(metrics_proto.SerializeToString(&serialized_metrics));
    EXPECT_TRUE(has_request_type)
        << "Expected request type " << key_request_type << " was not found. "
        << "metrics: " << wvcdm::b2a_hex(serialized_metrics);
  }

  std::string server_url_;
};

// Tests to validate service certificate
TEST_F(WvCdmEnginePreProvTestUat, ProvisioningServiceCertificateValidTest) {
  ASSERT_EQ(
      cdm_engine_.ValidateServiceCertificate(
          g_provisioning_service_certificate),
      NO_ERROR);
};

TEST_F(WvCdmEnginePreProvTestUat, ProvisioningServiceCertificateInvalidTest) {
  std::string certificate = g_provisioning_service_certificate;
  // Add four nulls to the beginning of the cert to invalidate it
  certificate.insert(0, 4, 0);

  ASSERT_NE(cdm_engine_.ValidateServiceCertificate(certificate), NO_ERROR);
};

// Test that provisioning works, even if device is already provisioned.
TEST_F(WvCdmEnginePreProvTestStaging, DISABLED_ProvisioningTest) {
  uint32_t nonce = 0;
  uint8_t buffer[1];
  size_t size = 0;

  int result = OEMCrypto_RewrapDeviceRSAKey(
      0, buffer, 0, buffer, 0, &nonce, buffer, 0, buffer, buffer, &size);
  int result30 = OEMCrypto_RewrapDeviceRSAKey30(
      0, &nonce, buffer, 0, buffer, 0, buffer, buffer, &size);
  int method = OEMCrypto_GetProvisioningMethod(kLevelDefault);

  if (result == OEMCrypto_ERROR_NOT_IMPLEMENTED &&
      result30 == OEMCrypto_ERROR_NOT_IMPLEMENTED) {
    LOGW("WARNING: Skipping ProvisioningTest because the device does not "
         "support provisioning. If you are using a baked-in certificate, "
         "this is expected. Otherwise, something is wrong.");
    ASSERT_EQ(method, OEMCrypto_DrmCertificate);
  } else {
    if (result == OEMCrypto_ERROR_NOT_IMPLEMENTED) {
      ASSERT_EQ(method, OEMCrypto_OEMCertificate);
    } else {
      ASSERT_EQ(method, OEMCrypto_Keybox);
    }
  }

  Provision();
}

TEST_F(WvCdmEnginePreProvTestUatBinary, DISABLED_ProvisioningTest) {
  Provision();
}

// Test that provisioning works, even if device is already provisioned.
TEST_F(WvCdmEngineTest, DISABLED_ProvisioningTest) {

  Provision();
}

TEST_F(WvCdmEngineTest, BaseIsoBmffMessageTest) {
  GenerateKeyRequest(g_key_id_pssh, kCencMimeType);
  GetKeyRequestResponse(g_license_server, g_client_auth);
}

// TODO(juce): Set up with correct test data.
TEST_F(WvCdmEngineTest, DISABLED_BaseWebmMessageTest) {
  GenerateKeyRequest(g_key_id_unwrapped, kWebmMimeType);
  GetKeyRequestResponse(g_license_server, g_client_auth);
}

TEST_F(WvCdmEngineTest, WrongMessageTest) {
  std::string wrong_message = a2bs_hex(g_wrong_key_id);
  GenerateKeyRequest(wrong_message, kCencMimeType);

  // We should receive a response with no license, i.e. the extracted license
  // response message should be empty.
  ASSERT_EQ("", FailToGetKeyRequestResponse(g_license_server, g_client_auth));
}

TEST_F(WvCdmEngineTest, NormalDecryptionIsoBmff) {
  GenerateKeyRequest(g_key_id_pssh, kCencMimeType);
  VerifyNewKeyResponse(g_license_server, g_client_auth);
}

// TODO(juce): Set up with correct test data.
TEST_F(WvCdmEngineTest, DISABLED_NormalDecryptionWebm) {
  GenerateKeyRequest(g_key_id_unwrapped, kWebmMimeType);
  VerifyNewKeyResponse(g_license_server, g_client_auth);
}

TEST_F(WvCdmEngineTest, LicenseRenewal) {
  GenerateKeyRequest(g_key_id_pssh, kCencMimeType);
  VerifyNewKeyResponse(g_license_server, g_client_auth);

  GenerateRenewalRequest();
  VerifyRenewalKeyResponse(server_url_.empty() ? g_license_server : server_url_,
                           g_client_auth);
}

}  // namespace wvcdm
