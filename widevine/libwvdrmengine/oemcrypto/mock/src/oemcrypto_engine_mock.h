// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.
//
//  Mock implementation of OEMCrypto APIs
//
#ifndef MOCK_OEMCRYPTO_ENGINE_MOCK_H_
#define MOCK_OEMCRYPTO_ENGINE_MOCK_H_

#include <stdint.h>
#include <time.h>
#include <map>
#include <memory>
#include <vector>

#include <openssl/rsa.h>

#include "OEMCryptoCENC.h"  // Needed for enums only.
#include "file_store.h"
#include "lock.h"
#include "oemcrypto_auth_mock.h"
#include "oemcrypto_key_mock.h"
#include "oemcrypto_rsa_key_shared.h"
#include "oemcrypto_session.h"
#include "oemcrypto_usage_table_mock.h"
#include "wv_cdm_types.h"

namespace wvoec_mock {

typedef std::map<SessionId, SessionContext*> ActiveSessions;

class CryptoEngine {
 public:
  // This is like a factory method, except we choose which version to use at
  // compile time. It is defined in several source files. The build system
  // should choose which one to use by only linking in the correct one.
  // NOTE: The caller must instantiate a FileSystem object - ownership
  // will be transferred to the new CryptoEngine object.
  static CryptoEngine* MakeCryptoEngine(
      std::auto_ptr<wvcdm::FileSystem> file_system);

  virtual ~CryptoEngine();

  virtual bool Initialize() { return true; }

  bool ValidRootOfTrust() { return root_of_trust_.Validate(); }

  bool InstallKeybox(const uint8_t* keybox, size_t keybox_length) {
    return root_of_trust_.InstallKeybox(keybox, keybox_length);
  }

  bool UseTestKeybox(const uint8_t* keybox_data, size_t keybox_length) {
    return root_of_trust_.UseTestKeybox(keybox_data, keybox_length);
  }

  bool LoadTestRsaKey() { return root_of_trust_.LoadTestRsaKey(); }

  KeyboxError ValidateKeybox() { return root_of_trust_.ValidateKeybox(); }

  const std::vector<uint8_t>& DeviceRootKey(bool override_to_real = false) {
    return root_of_trust_.DeviceKey(override_to_real);
  }

  const std::vector<uint8_t>& DeviceRootId() {
    return root_of_trust_.DeviceId();
  }

  size_t DeviceRootTokenLength() { return root_of_trust_.DeviceTokenLength(); }

  const uint8_t* DeviceRootToken() {
    return root_of_trust_.DeviceToken();
  }

  virtual void Terminate() {}

  SessionId CreateSession();

  bool DestroySession(SessionId sid);

  SessionContext* FindSession(SessionId sid);

  size_t GetNumberOfOpenSessions() { return sessions_.size(); }

  size_t GetMaxNumberOfSessions() {
    // An arbitrary limit for mock implementation.
    static const size_t kMaxSupportedOEMCryptoSessions = 64;
    return kMaxSupportedOEMCryptoSessions;
  }

  // Returns the HDCP version currently in use.
  virtual OEMCrypto_HDCP_Capability config_current_hdcp_capability();

  // Returns the max HDCP version supported.
  virtual OEMCrypto_HDCP_Capability config_maximum_hdcp_capability();

  UsageTable& usage_table() { return usage_table_; }
  wvcdm::FileSystem* file_system() { return file_system_.get(); }

  // If config_local_display_only() returns true, we pretend we are using a
  // built-in display, instead of HDMI or WiFi output.
  virtual bool config_local_display_only() { return false; }

  // A closed platform is permitted to use clear buffers.
  virtual bool config_closed_platform() { return false; }

  // Returns true if the client supports persistent storage of
  // offline usage table information.
  virtual bool config_supports_usage_table() { return true; }

  virtual OEMCrypto_ProvisioningMethod config_provisioning_method() {
    return OEMCrypto_Keybox;
  }

  virtual OEMCryptoResult get_oem_certificate(SessionContext* session,
                                              uint8_t* public_cert,
                                              size_t* public_cert_length) {
    return OEMCrypto_ERROR_NOT_IMPLEMENTED;
  }

  // Used for OEMCrypto_IsAntiRollbackHwPresent.
  virtual bool config_is_anti_rollback_hw_present() { return false; }

  // Returns "L3" for a software only library.  L1 is for hardware protected
  // data paths.
  virtual const char* config_security_level() { return "L3"; }

  // This should start at 0, and be incremented only when a security patch has
  // been applied to the device that fixes a security bug.
  virtual uint8_t config_security_patch_level() { return 0; }

  // If 0 no restriction, otherwise it's the max buffer for DecryptCENC.
  virtual size_t max_buffer_size() { return 1024 * 100; }  // 100 KiB.

  virtual bool srm_update_supported() { return false; }

  virtual OEMCryptoResult current_srm_version(uint16_t* version) {
    return OEMCrypto_ERROR_NOT_IMPLEMENTED;
  }

  virtual OEMCryptoResult load_srm(const uint8_t* buffer,
                                   size_t buffer_length) {
    return OEMCrypto_ERROR_NOT_IMPLEMENTED;
  }

  virtual OEMCryptoResult remove_srm() {
    return OEMCrypto_ERROR_NOT_IMPLEMENTED;
  }

  virtual bool srm_blacklisted_device_attached() { return false; }

  // Rate limit for nonce generation.  Default to 20 nonce/second.
  virtual int nonce_flood_count() { return 20; }

  // Set destination pointer based on the output destination description.
  OEMCryptoResult SetDestination(OEMCrypto_DestBufferDesc* out_description,
                                 size_t data_length, uint8_t subsample_flags);

  // The current destination.
  uint8_t* destination() { return destination_; }

  // Subclasses can adjust the destination -- for use in testing.
  virtual void adjust_destination(OEMCrypto_DestBufferDesc* out_description,
                                  size_t data_length, uint8_t subsample_flags) {
  }

  // Push destination buffer to output -- used by subclasses for testing.
  virtual OEMCryptoResult PushDestination(
      OEMCrypto_DestBufferDesc* out_description, uint8_t subsample_flags) {
    return OEMCrypto_SUCCESS;
  }

 protected:
  explicit CryptoEngine(std::auto_ptr<wvcdm::FileSystem> file_system);
  uint8_t* destination_;

 private:
  ActiveSessions sessions_;
  AuthenticationRoot root_of_trust_;
  wvcdm::Lock session_table_lock_;
  std::auto_ptr<wvcdm::FileSystem> file_system_;
  UsageTable usage_table_;

  CORE_DISALLOW_COPY_AND_ASSIGN(CryptoEngine);
};

}  // namespace wvoec_mock

#endif  // MOCK_OEMCRYPTO_ENGINE_MOCK_H_
