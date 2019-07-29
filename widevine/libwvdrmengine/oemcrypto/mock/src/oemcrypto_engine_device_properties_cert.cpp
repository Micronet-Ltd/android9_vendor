// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.
//
//  Mock implementation of OEMCrypto APIs
//
//  This file contains oemcrypto engine properties that would be for a device
//  that does not have persistant storage or a keybox.
//
//  Note: We also define it to be L2 for illustration only.  Production devices
//  are rarely level 2.
#include "oemcrypto_engine_mock.h"

namespace wvoec_mock {

class CertOnlyCryptoEngine : public CryptoEngine {
 public:
  explicit CertOnlyCryptoEngine(std::auto_ptr<wvcdm::FileSystem> file_system)
      : CryptoEngine(file_system) {}

  bool config_local_display_only() { return true; }

  bool config_supports_usage_table() { return false; }

  OEMCrypto_ProvisioningMethod config_provisioning_method() {
    return OEMCrypto_DrmCertificate;
  }

  const char* config_security_level() { return "L2"; }
};

CryptoEngine* CryptoEngine::MakeCryptoEngine(
    std::auto_ptr<wvcdm::FileSystem> file_system) {
  return new CertOnlyCryptoEngine(file_system);
}

}  // namespace wvoec_mock
