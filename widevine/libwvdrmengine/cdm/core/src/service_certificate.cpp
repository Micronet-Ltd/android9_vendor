// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.

#include "service_certificate.h"

#include "crypto_key.h"
#include "crypto_session.h"
#include "license_protocol.pb.h"
#include "log.h"
#include "privacy_crypto.h"
#include "properties.h"
#include "wv_cdm_constants.h"

namespace {
// Root certificate for all Google/Widevine certificates.  I.e. all service
// certificates and DRM certificates are signed by this cert, or have this cert
// as the root of a signing chain.
static const unsigned char kRootCertForProd[] = {
  0x0a, 0x9c, 0x03, 0x08, 0x00, 0x12, 0x01, 0x00,
  0x18, 0xdd, 0x94, 0x88, 0x8b, 0x05, 0x22, 0x8e,
  0x03, 0x30, 0x82, 0x01, 0x8a, 0x02, 0x82, 0x01,
  0x81, 0x00, 0xb4, 0xfe, 0x39, 0xc3, 0x65, 0x90,
  0x03, 0xdb, 0x3c, 0x11, 0x97, 0x09, 0xe8, 0x68,
  0xcd, 0xf2, 0xc3, 0x5e, 0x9b, 0xf2, 0xe7, 0x4d,
  0x23, 0xb1, 0x10, 0xdb, 0x87, 0x65, 0xdf, 0xdc,
  0xfb, 0x9f, 0x35, 0xa0, 0x57, 0x03, 0x53, 0x4c,
  0xf6, 0x6d, 0x35, 0x7d, 0xa6, 0x78, 0xdb, 0xb3,
  0x36, 0xd2, 0x3f, 0x9c, 0x40, 0xa9, 0x95, 0x26,
  0x72, 0x7f, 0xb8, 0xbe, 0x66, 0xdf, 0xc5, 0x21,
  0x98, 0x78, 0x15, 0x16, 0x68, 0x5d, 0x2f, 0x46,
  0x0e, 0x43, 0xcb, 0x8a, 0x84, 0x39, 0xab, 0xfb,
  0xb0, 0x35, 0x80, 0x22, 0xbe, 0x34, 0x23, 0x8b,
  0xab, 0x53, 0x5b, 0x72, 0xec, 0x4b, 0xb5, 0x48,
  0x69, 0x53, 0x3e, 0x47, 0x5f, 0xfd, 0x09, 0xfd,
  0xa7, 0x76, 0x13, 0x8f, 0x0f, 0x92, 0xd6, 0x4c,
  0xdf, 0xae, 0x76, 0xa9, 0xba, 0xd9, 0x22, 0x10,
  0xa9, 0x9d, 0x71, 0x45, 0xd6, 0xd7, 0xe1, 0x19,
  0x25, 0x85, 0x9c, 0x53, 0x9a, 0x97, 0xeb, 0x84,
  0xd7, 0xcc, 0xa8, 0x88, 0x82, 0x20, 0x70, 0x26,
  0x20, 0xfd, 0x7e, 0x40, 0x50, 0x27, 0xe2, 0x25,
  0x93, 0x6f, 0xbc, 0x3e, 0x72, 0xa0, 0xfa, 0xc1,
  0xbd, 0x29, 0xb4, 0x4d, 0x82, 0x5c, 0xc1, 0xb4,
  0xcb, 0x9c, 0x72, 0x7e, 0xb0, 0xe9, 0x8a, 0x17,
  0x3e, 0x19, 0x63, 0xfc, 0xfd, 0x82, 0x48, 0x2b,
  0xb7, 0xb2, 0x33, 0xb9, 0x7d, 0xec, 0x4b, 0xba,
  0x89, 0x1f, 0x27, 0xb8, 0x9b, 0x88, 0x48, 0x84,
  0xaa, 0x18, 0x92, 0x0e, 0x65, 0xf5, 0xc8, 0x6c,
  0x11, 0xff, 0x6b, 0x36, 0xe4, 0x74, 0x34, 0xca,
  0x8c, 0x33, 0xb1, 0xf9, 0xb8, 0x8e, 0xb4, 0xe6,
  0x12, 0xe0, 0x02, 0x98, 0x79, 0x52, 0x5e, 0x45,
  0x33, 0xff, 0x11, 0xdc, 0xeb, 0xc3, 0x53, 0xba,
  0x7c, 0x60, 0x1a, 0x11, 0x3d, 0x00, 0xfb, 0xd2,
  0xb7, 0xaa, 0x30, 0xfa, 0x4f, 0x5e, 0x48, 0x77,
  0x5b, 0x17, 0xdc, 0x75, 0xef, 0x6f, 0xd2, 0x19,
  0x6d, 0xdc, 0xbe, 0x7f, 0xb0, 0x78, 0x8f, 0xdc,
  0x82, 0x60, 0x4c, 0xbf, 0xe4, 0x29, 0x06, 0x5e,
  0x69, 0x8c, 0x39, 0x13, 0xad, 0x14, 0x25, 0xed,
  0x19, 0xb2, 0xf2, 0x9f, 0x01, 0x82, 0x0d, 0x56,
  0x44, 0x88, 0xc8, 0x35, 0xec, 0x1f, 0x11, 0xb3,
  0x24, 0xe0, 0x59, 0x0d, 0x37, 0xe4, 0x47, 0x3c,
  0xea, 0x4b, 0x7f, 0x97, 0x31, 0x1c, 0x81, 0x7c,
  0x94, 0x8a, 0x4c, 0x7d, 0x68, 0x15, 0x84, 0xff,
  0xa5, 0x08, 0xfd, 0x18, 0xe7, 0xe7, 0x2b, 0xe4,
  0x47, 0x27, 0x12, 0x11, 0xb8, 0x23, 0xec, 0x58,
  0x93, 0x3c, 0xac, 0x12, 0xd2, 0x88, 0x6d, 0x41,
  0x3d, 0xc5, 0xfe, 0x1c, 0xdc, 0xb9, 0xf8, 0xd4,
  0x51, 0x3e, 0x07, 0xe5, 0x03, 0x6f, 0xa7, 0x12,
  0xe8, 0x12, 0xf7, 0xb5, 0xce, 0xa6, 0x96, 0x55,
  0x3f, 0x78, 0xb4, 0x64, 0x82, 0x50, 0xd2, 0x33,
  0x5f, 0x91, 0x02, 0x03, 0x01, 0x00, 0x01, 0x12,
  0x80, 0x03, 0x58, 0xf1, 0xd6, 0x4d, 0x04, 0x09,
  0x7b, 0xdf, 0xd7, 0xef, 0x5d, 0x3b, 0x02, 0x39,
  0x17, 0xfa, 0x14, 0x36, 0x75, 0x4a, 0x38, 0x67,
  0x85, 0x57, 0x12, 0xa7, 0x14, 0xee, 0x35, 0x16,
  0xd5, 0x3d, 0xbf, 0x42, 0x86, 0xf6, 0x69, 0x00,
  0x76, 0xcd, 0x93, 0xf4, 0x7c, 0xb2, 0xdf, 0x9e,
  0x44, 0xcd, 0x4c, 0xd4, 0xae, 0x09, 0x18, 0x53,
  0x44, 0x32, 0xec, 0xe0, 0x61, 0x1b, 0xe5, 0xda,
  0x13, 0xd3, 0x55, 0xc5, 0xdd, 0x1a, 0xcb, 0x90,
  0x1e, 0x7e, 0x5b, 0xc6, 0xe9, 0x0f, 0x22, 0x9f,
  0xbe, 0x85, 0x02, 0xfe, 0x90, 0x31, 0xcc, 0x6b,
  0x03, 0x84, 0xbd, 0x22, 0xc4, 0x55, 0xfa, 0xf5,
  0xf2, 0x08, 0xcd, 0x65, 0x41, 0x58, 0xe8, 0x7d,
  0x29, 0xda, 0x04, 0x58, 0x82, 0xf5, 0x37, 0x69,
  0xbc, 0xf3, 0x5a, 0x57, 0x84, 0x17, 0x7b, 0x32,
  0x87, 0x70, 0xb2, 0xb0, 0x76, 0x9c, 0xb2, 0xc3,
  0x15, 0xd1, 0x11, 0x26, 0x2a, 0x23, 0x75, 0x99,
  0x3e, 0xb9, 0x77, 0x22, 0x32, 0x0d, 0xbc, 0x1a,
  0x19, 0xc1, 0xd5, 0x65, 0x90, 0x76, 0x55, 0x74,
  0x0f, 0x0e, 0x69, 0x4d, 0x5f, 0x4d, 0x8f, 0x19,
  0xaf, 0xdf, 0xd6, 0x16, 0x31, 0x94, 0xa8, 0x92,
  0x5f, 0x4f, 0xbc, 0x7a, 0x31, 0xf8, 0xae, 0x8e,
  0xad, 0x33, 0xb7, 0xe9, 0x30, 0xd0, 0x8c, 0x0a,
  0x8a, 0x6c, 0x83, 0x35, 0xf8, 0x8a, 0x81, 0xb2,
  0xfe, 0x1c, 0x88, 0xac, 0x2a, 0x66, 0xc5, 0xff,
  0xbd, 0xe6, 0x17, 0xd0, 0x62, 0x0b, 0xdc, 0x8a,
  0x45, 0xf7, 0xb0, 0x3e, 0x5a, 0xc8, 0x1e, 0x4a,
  0x24, 0x2f, 0x6c, 0xa5, 0xe3, 0x1c, 0x88, 0x14,
  0x83, 0xd5, 0xc5, 0xef, 0x5e, 0x9f, 0x3d, 0x85,
  0x45, 0x73, 0xe2, 0x6b, 0x50, 0x52, 0x57, 0x4c,
  0xfb, 0x92, 0x6c, 0x66, 0x75, 0x8a, 0xd6, 0x0d,
  0x1b, 0xae, 0xf3, 0xec, 0xaf, 0x51, 0x22, 0x03,
  0x5d, 0x0a, 0x2e, 0x63, 0x93, 0x9c, 0x0b, 0x01,
  0x20, 0xa8, 0xa9, 0x84, 0x2e, 0x17, 0xca, 0xae,
  0x73, 0xec, 0x22, 0x1b, 0x79, 0xae, 0xf6, 0xa0,
  0x72, 0x2c, 0xdf, 0x07, 0x47, 0xdb, 0x88, 0x86,
  0x30, 0x14, 0x78, 0x21, 0x11, 0x22, 0x88, 0xac,
  0xd7, 0x54, 0x74, 0xf9, 0xf3, 0x26, 0xc2, 0xa5,
  0x56, 0xc8, 0x56, 0x4f, 0x00, 0x29, 0x1d, 0x08,
  0x7b, 0x7a, 0xfb, 0x95, 0x89, 0xc3, 0xee, 0x98,
  0x54, 0x9e, 0x3c, 0x6b, 0x94, 0x05, 0x13, 0x12,
  0xf6, 0x71, 0xb9, 0xab, 0x13, 0xc3, 0x0c, 0x9b,
  0x46, 0x08, 0x7b, 0x3d, 0x32, 0x6a, 0x68, 0xca,
  0x1e, 0x9c, 0x90, 0x62, 0xc5, 0xed, 0x10, 0xb9,
  0x1f, 0x17, 0x25, 0xce, 0x90, 0xb9, 0x6d, 0xcd,
  0xc4, 0x46, 0xf5, 0xa3, 0x62, 0x13, 0x74, 0x02,
  0xa7, 0x62, 0xa4, 0xfa, 0x55, 0xd9, 0xde, 0xcf,
  0xa2, 0xe6, 0x80, 0x74, 0x55, 0x06, 0x49, 0xd5,
  0x02, 0x0c
};

}  // namespace

namespace wvcdm {

// Protobuf generated classes.
using video_widevine::ClientIdentification;
using video_widevine::DrmDeviceCertificate;
using video_widevine::EncryptedClientIdentification;
using video_widevine::LicenseError;
using video_widevine::SignedDrmDeviceCertificate;
using video_widevine::SignedMessage;

CdmResponseType ServiceCertificate::Init(const std::string& certificate) {
  std::string root_cert_str(reinterpret_cast<const char*>(&kRootCertForProd[0]),
                            sizeof(kRootCertForProd));

  // Load root cert public key. Don't bother verifying it.
  SignedDrmDeviceCertificate signed_root_cert;
  if (!signed_root_cert.ParseFromString(root_cert_str)) {
    LOGE("Failed to deserialize signed root certificate.");
    return DEVICE_CERTIFICATE_ERROR_1;
  }
  DrmDeviceCertificate root_cert;
  if (!root_cert.ParseFromString(signed_root_cert.drm_certificate())) {
    LOGE("Failed to deserialize signed root certificate.");
    return DEVICE_CERTIFICATE_ERROR_1;
  }
  RsaPublicKey root_key;
  if (!root_key.Init(root_cert.public_key())) {
    LOGE("Failed to load root certificate public key.");
    return DEVICE_CERTIFICATE_ERROR_1;
  }

  // Load the provided service certificate.
  // First, parse it and verify its signature.
  SignedDrmDeviceCertificate signed_service_cert;
  if (!signed_service_cert.ParseFromString(certificate)) {
    LOGE("Failed to parse signed service certificate.");
    return DEVICE_CERTIFICATE_ERROR_2;
  }
  if (!root_key.VerifySignature(signed_service_cert.drm_certificate(),
                                signed_service_cert.signature())) {
    LOGE("Service certificate signature verification failed.");
    return DEVICE_CERTIFICATE_ERROR_3;
  }
  DrmDeviceCertificate service_cert;
  if (!service_cert.ParseFromString(signed_service_cert.drm_certificate())) {
    LOGE("Failed to parse service certificate.");
    return DEVICE_CERTIFICATE_ERROR_2;
  }
  if (service_cert.type() !=
      video_widevine::DrmDeviceCertificate_CertificateType_SERVICE) {
    LOGE("Not a service certificate.");
    return DEVICE_CERTIFICATE_ERROR_3;
  }

  // Service certificate passes all checks - set up its RSA public key.
  public_key_.reset(new RsaPublicKey);
  if (!public_key_->Init(service_cert.public_key())) {
    public_key_.reset();
    LOGE("Failed to load service certificate public key.");
    return DEVICE_CERTIFICATE_ERROR_2;
  }

  // Have service certificate and its public key - keep relevant fields.
  certificate_ = certificate;
  serial_number_ = service_cert.serial_number();
  provider_id_ = service_cert.provider_id();
  has_certificate_ = true;

  return NO_ERROR;
}

CdmResponseType ServiceCertificate::VerifySignedMessage(
    const std::string& message, const std::string& signature) {
  if (public_key_.get() == NULL) {
    LOGE("Service certificate not set.");
    return DEVICE_CERTIFICATE_ERROR_4;
  }

  if (!public_key_->VerifySignature(message, signature))
    return CLIENT_ID_RSA_ENCRYPT_ERROR;  // TODO(tinskip): Need new error code.

  return NO_ERROR;
}

CdmResponseType ServiceCertificate::EncryptRsaOaep(const std::string& plaintext,
                                                   std::string* ciphertext) {
  if (public_key_.get() == NULL) {
    LOGE("Service certificate not set.");
    return DEVICE_CERTIFICATE_ERROR_4;
  }

  if (!public_key_->Encrypt(plaintext, ciphertext))
    return CLIENT_ID_RSA_ENCRYPT_ERROR;

  return NO_ERROR;
}

CdmResponseType ServiceCertificate::EncryptClientId(
    CryptoSession* crypto_session, const ClientIdentification* clear_client_id,
    EncryptedClientIdentification* encrypted_client_id) {
  encrypted_client_id->set_provider_id(provider_id_);
  encrypted_client_id->set_service_certificate_serial_number(serial_number_);

  std::string iv(KEY_IV_SIZE, 0);
  std::string key(KEY_SIZE, 0);

  if (!crypto_session->GetRandom(key.size(),
                                reinterpret_cast<uint8_t*>(&key[0])))
    return CLIENT_ID_GENERATE_RANDOM_ERROR;
  if (!crypto_session->GetRandom(iv.size(),
                                reinterpret_cast<uint8_t*>(&iv[0])))
    return CLIENT_ID_GENERATE_RANDOM_ERROR;
  std::string id, enc_id, enc_key;
  clear_client_id->SerializeToString(&id);

  AesCbcKey aes;
  if (!aes.Init(key)) return CLIENT_ID_AES_INIT_ERROR;
  if (!aes.Encrypt(id, &enc_id, &iv)) return CLIENT_ID_AES_ENCRYPT_ERROR;

  CdmResponseType encrypt_result = EncryptRsaOaep(key, &enc_key);
  if (encrypt_result != NO_ERROR)
    return encrypt_result;

  encrypted_client_id->set_encrypted_client_id_iv(iv);
  encrypted_client_id->set_encrypted_privacy_key(enc_key);
  encrypted_client_id->set_encrypted_client_id(enc_id);
  return NO_ERROR;
}

bool ServiceCertificate::GetRequest(CdmKeyMessage* request) {
  if (!request) {
    LOGE("ServiceCertificate::PrepareRequest: no request parameter provided");
    return false;
  }
  SignedMessage message;
  message.set_type(SignedMessage::SERVICE_CERTIFICATE_REQUEST);
  message.SerializeToString(request);
  return true;
}

CdmResponseType ServiceCertificate::ParseResponse(
    const std::string& response, std::string* certificate) {
  if (response.empty()) {
    LOGE("ServiceCertificate::ParseResponse: empty response");
    return EMPTY_RESPONSE_ERROR_1;
  }
  if (!certificate) {
    LOGE("ServiceCertificate::ParseResponse: null return parameter");
    return INVALID_PARAMETERS_ENG_24;
  }

  SignedMessage signed_response;
  if (!signed_response.ParseFromString(response)) {
    LOGE("ServiceCertificate::ParseResponse: cannot parse response");
    return PARSE_RESPONSE_ERROR_1;
  }

  if (signed_response.type() == SignedMessage::ERROR_RESPONSE) {
    LicenseError license_error;
    if (!license_error.ParseFromString(signed_response.msg())) {
      LOGE("ServiceCertificate::ParseResponse: cannot parse license error");
      return PARSE_RESPONSE_ERROR_2;
    }
    LOGE("ServiceCertificate::ParseResponse: server returned error = %d",
         license_error.error_code());
    return PARSE_RESPONSE_ERROR_3;
  }

  if (signed_response.type() != SignedMessage::SERVICE_CERTIFICATE) {
    LOGE("ServiceCertificate::ParseResponse: response (%d) is wrong type",
         signed_response.type());
    return PARSE_RESPONSE_ERROR_4;
  }

  certificate->assign(signed_response.msg());
  return NO_ERROR;
}



}  // namespace wvcdm
