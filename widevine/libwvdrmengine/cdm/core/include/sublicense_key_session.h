#ifndef WVCDM_CORE_SUBLICENSE_KEY_SESSSION_H_
#define WVCDM_CORE_SUBLICENSE_KEY_SESSSION_H_

#include "crypto_key.h"
#include "key_session.h"

namespace wvcdm {

class SubLicenseKeySession : public KeySession {
  typedef enum {
    kInitializing,
    kInitialLicenseLoaded,
    kInitialLicenseFailed,
  } SubLicenseState;

 public:
  SubLicenseKeySession(SubLicenseSessionMap& sub_license_oec_sessions,
                       metrics::CryptoMetrics* metrics,
                       const std::string& wrapped_private_device_key,
                       SecurityLevel requested_security_level,
                       const std::string& group_id);

  virtual ~SubLicenseKeySession();

  KeySessionType Type() { return kSubLicense; }

  // This version of GenerateDerivedKeys is for devices using keyboxes. It is
  // not supported using sub licenses.
  bool GenerateDerivedKeys(const std::string&) { return false; }

  // GenerateDerivedKeys is called for each open oemcrypto session and is only
  // called once.
  bool GenerateDerivedKeys(const std::string& message,
                           const std::string& session_key);

  // Load the keys in |keys|. The initial keys are saved for key rotation.
  OEMCryptoResult LoadKeys(const std::string& message,
                           const std::string& signature,
                           const std::string& mac_key_iv,
                           const std::string& mac_key,
                           const std::vector<CryptoKey>& keys,
                           const std::string& provider_session_token,
                           CdmCipherMode* cipher_mode,
                           const std::string& srm_requirement);

  OEMCryptoResult LoadEntitledContentKeys(const std::vector<CryptoKey>& /*keys*/) {
    return OEMCrypto_ERROR_INVALID_CONTEXT;
  }

  // Each oemcrypto session contains a single key. Find the right sub session
  // and save it's id as the selected oemcrypto session.
  OEMCryptoResult SelectKey(const std::string& key_id,
                            CdmCipherMode cipher_mode);

  // Decrypt performs the decryption using the selected oemcrypto session.
  // TODO(jfore): Support DecryptInChunks.
  OEMCryptoResult Decrypt(const CdmDecryptionParameters& params,
                          OEMCrypto_DestBufferDesc& buffer_descriptor,
                          OEMCrypto_CENCEncryptPatternDesc& pattern_descriptor);

 private:
  // Destroy each open oemcrypto session and relace them with new ones.
  OEMCryptoResult ResetCryptoSessions();

  // DoLoadKeys loads a single key into each oemcrypto session.
  OEMCryptoResult DoLoadKeys(const std::string& message,
                             const std::string& signature,
                             const std::string& mac_key_iv,
                             const std::string& mac_key,
                             const std::vector<CryptoKey>& keys,
                             const std::string& provider_session_token,
                             CdmCipherMode* cipher_mode,
                             const std::string& srm_requirement);

  // DoLoadKeys loads a single key into each oemcrypto session.
  OEMCryptoResult DoSubLicenseLoadKeys(
      const std::string& message, const std::string& signature,
      const std::string& mac_key_iv, const std::string& mac_key,
      const CryptoKey& key, const std::string& provider_session_token,
      CdmCipherMode*, const std::string& srm_requirement);

  SubLicenseState state_;
  std::string cached_sub_session_key_id_;
  std::string wrapped_private_device_key_;
  std::string message_;
  std::string session_key_;
  std::vector<CryptoKey> keys_;
  SubLicenseSessionMap& sub_license_oec_sessions_;
  SecurityLevel requested_security_level_;
  KeyId group_id_;
};

}  // namespace wvcdm

#endif  // WVCDM_CORE_SUBLICENSE_KEY_SESSSION_H_
