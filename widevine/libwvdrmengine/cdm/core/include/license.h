// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.

#ifndef WVCDM_CORE_LICENSE_H_
#define WVCDM_CORE_LICENSE_H_

#include <set>

#include "initialization_data.h"
#include "license_protocol.pb.h"
#include "scoped_ptr.h"
#include "service_certificate.h"
#include "wv_cdm_types.h"

namespace video_widevine {
class SignedMessage;
class LicenseRequest;
}  // namespace video_widevine

namespace wvcdm {

class Clock;
class CryptoSession;
class PolicyEngine;
class CdmSession;
class CryptoKey;

class CdmLicense {
 public:
  CdmLicense(const CdmSessionId& session_id);
  virtual ~CdmLicense();

  virtual bool Init(const std::string& client_token,
                    CdmClientTokenType client_token_type,
                    const std::string& device_id, bool use_privacy_mode,
                    const std::string& signed_service_certificate,
                    CryptoSession* session, PolicyEngine* policy_engine);

  virtual CdmResponseType PrepareKeyRequest(
      const InitializationData& init_data, CdmLicenseType license_type,
      const CdmAppParameterMap& app_parameters, CdmKeyMessage* signed_request,
      std::string* server_url);
  virtual CdmResponseType PrepareKeyUpdateRequest(
      bool is_renewal, const CdmAppParameterMap& app_parameters,
      CdmSession* cdm_session, CdmKeyMessage* signed_request,
      std::string* server_url);
  virtual CdmResponseType HandleKeyResponse(
      const CdmKeyResponse& license_response);
  virtual CdmResponseType HandleKeyUpdateResponse(
      bool is_renewal, const CdmKeyResponse& license_response);
  virtual CdmResponseType HandleSubLicense(const InitializationData& init_data);

  virtual bool RestoreOfflineLicense(
      const CdmKeyMessage& license_request,
      const CdmKeyResponse& license_response,
      const CdmKeyResponse& license_renewal_response,
      int64_t playback_start_time, int64_t last_playback_time,
      int64_t grace_period_end_time, CdmSession* cdm_session);
  virtual bool RestoreLicenseForRelease(const CdmKeyMessage& license_request,
                                        const CdmKeyResponse& license_response);
  virtual bool HasInitData() { return stored_init_data_.get(); }
  virtual bool IsKeyLoaded(const KeyId& key_id);

  virtual std::string provider_session_token() {
    return provider_session_token_;
  }

  virtual bool is_offline() { return is_offline_; }

  static bool ExtractProviderSessionToken(
      const CdmKeyResponse& license_response,
      std::string* provider_session_token);

 private:
  CdmResponseType HandleKeyErrorResponse(
      const video_widevine::SignedMessage& signed_message);

  CdmResponseType PrepareClientId(
      const CdmAppParameterMap& app_parameters,
      video_widevine::LicenseRequest* license_request);

  CdmResponseType PrepareContentId(
      const InitializationData& init_data, CdmLicenseType license_type,
      const std::string& request_id,
      video_widevine::LicenseRequest* license_request);

  CdmResponseType HandleContentKeyResponse(
      const std::string& msg, const std::string& signature,
      const std::string& mac_key_iv, const std::string& mac_key,
      const std::vector<CryptoKey>& key_array,
      const video_widevine::License& license);

  // HandleEntitlementKeyResponse loads the entitlement keys in |key_array| into
  // the crypto session. In addition, it also extracts content keys from
  // |wrapped_keys_| and loads them for use.
  CdmResponseType HandleEntitlementKeyResponse(
      const std::string& msg, const std::string& signature,
      const std::string& mac_key_iv, const std::string& mac_key,
      const std::vector<CryptoKey>& key_array,
      const video_widevine::License& license);

  template <typename T>
  bool SetTypeAndId(CdmLicenseType license_type, const std::string& request_id,
                    T* content_id);

  CryptoSession* crypto_session_;
  PolicyEngine* policy_engine_;
  std::string server_url_;
  std::string client_token_;
  CdmClientTokenType client_token_type_;
  std::string device_id_;
  const CdmSessionId session_id_;
  scoped_ptr<InitializationData> stored_init_data_;
  bool initialized_;
  std::set<KeyId> loaded_keys_;
  std::string provider_session_token_;
  bool renew_with_client_id_;
  bool is_offline_;

  // Associated with ClientIdentification encryption
  bool use_privacy_mode_;
  ServiceCertificate service_certificate_;

  // Used for certificate based licensing
  CdmKeyMessage key_request_;

  scoped_ptr<Clock> clock_;

  // For testing
  // CdmLicense takes ownership of the clock.
  CdmLicense(const CdmSessionId& session_id, Clock* clock);

  // For sublicense key embedding. This key array will be initilized with any
  // sub session keys we may have received in a license response. These keys
  // may be used to support key rotation.
  std::vector<CryptoKey> sub_session_key_array_;

  // For entitlement key licensing. This holds the keys from the init_data.
  // These keys are extracted from the pssh when we generate a license request.
  // It is used to load content keys after we have received a license and
  // entitelement keys. It is also used in updating the key status info.
  std::vector<video_widevine::WrappedKey> wrapped_keys_;
#if defined(UNIT_TEST)
  friend class CdmLicenseTest;
#endif

  CORE_DISALLOW_COPY_AND_ASSIGN(CdmLicense);
};

}  // namespace wvcdm

#endif  // WVCDM_CORE_LICENSE_H_
