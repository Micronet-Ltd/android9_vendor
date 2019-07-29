// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.
//
//  Mock implementation of OEMCrypto APIs
//
#ifndef OEMCRYPTO_AUTH_MOCK_H_
#define OEMCRYPTO_AUTH_MOCK_H_

#include <stdint.h>
#include <vector>

#include <openssl/rsa.h>

#include "OEMCryptoCENC.h"  // Needed for enums only.
#include "oemcrypto_key_mock.h"
#include "oemcrypto_keybox_mock.h"
#include "oemcrypto_rsa_key_shared.h"
#include "wv_cdm_types.h"

namespace wvoec_mock {

class AuthenticationRoot {
 public:
  explicit AuthenticationRoot(OEMCrypto_ProvisioningMethod method);
  ~AuthenticationRoot() {}

  bool Validate();

  KeyboxError ValidateKeybox();

  bool InstallKeybox(const uint8_t* keybox_data, size_t keybox_length) {
    return keybox().InstallKeybox(keybox_data, keybox_length);
  }

  const std::vector<uint8_t>& DeviceKey(bool use_real_keybox = false) {
    return use_real_keybox ? real_keybox().device_key() :
                             keybox().device_key();
  }

  const std::vector<uint8_t>& DeviceId() {
    return keybox().device_id();
  }

  size_t DeviceTokenLength() {
    return keybox().key_data_length();
  }

  const uint8_t* DeviceToken() {
    return keybox().key_data();
  }

  WvKeybox& keybox() { return use_test_keybox_ ? test_keybox_ : keybox_; }
  bool UseTestKeybox(const uint8_t* keybox_data, size_t keybox_length) {
    use_test_keybox_ = true;
    return test_keybox_.InstallKeybox(keybox_data, keybox_length);
  }

  RSA_shared_ptr& SharedRsaKey() { return rsa_key_; }
  RSA* rsa_key() { return rsa_key_.get(); }
  bool LoadTestRsaKey();

 private:
  OEMCrypto_ProvisioningMethod provisioning_method_;
  WvKeybox& real_keybox() { return keybox_; }

  WvKeybox keybox_;
  WvTestKeybox test_keybox_;
  bool use_test_keybox_;

  RSA_shared_ptr rsa_key_;  // If no keybox, this is baked in certificate.

  CORE_DISALLOW_COPY_AND_ASSIGN(AuthenticationRoot);
};

}  // namespace wvoec_mock

#endif  // OEMCRYPTO_AUTH_MOCK_H_
