#ifndef WVCDM_CORE_CONTENT_KEY_SESSSION_H_
#define WVCDM_CORE_CONTENT_KEY_SESSSION_H_

#include "key_session.h"
#include "timer_metric.h"

namespace wvcdm {

class ContentKeySession : public KeySession {
 public:
  ContentKeySession(CryptoSessionId oec_session_id,
                    metrics::CryptoMetrics* metrics)
      : KeySession(metrics),
        oec_session_id_(oec_session_id),
        cipher_mode_(kCipherModeCtr) {}
  virtual ~ContentKeySession() {}

  KeySessionType Type() { return kDefault; }

  // Generate Derived Keys for ContentKeySession
  bool GenerateDerivedKeys(const std::string& message);

  // Generate Derived Keys (from session key) for ContentKeySession
  bool GenerateDerivedKeys(const std::string& message,
                           const std::string& session_key);

  // Load Keys for ContentKeySession
  OEMCryptoResult LoadKeys(const std::string& message,
                           const std::string& signature,
                           const std::string& mac_key_iv,
                           const std::string& mac_key,
                           const std::vector<CryptoKey>& keys,
                           const std::string& provider_session_token,
                           CdmCipherMode* cipher_mode,
                           const std::string& srm_requirement);

  OEMCryptoResult LoadEntitledContentKeys(const std::vector<CryptoKey>&) {
    return OEMCrypto_ERROR_INVALID_CONTEXT;
  }

  // Select Key for ContentKeySession
  OEMCryptoResult SelectKey(const std::string& key_id,
                            CdmCipherMode cipher_mode);

  // Decrypt for ContentKeySession
  OEMCryptoResult Decrypt(const CdmDecryptionParameters& params,
                          OEMCrypto_DestBufferDesc& buffer_descriptor,
                          OEMCrypto_CENCEncryptPatternDesc& pattern_descriptor);

 protected:
  OEMCryptoResult LoadKeys(
      const std::string& message, const std::string& signature,
      const std::string& mac_key_iv, const std::string& mac_key,
      const std::vector<CryptoKey>& keys,
      const std::string& provider_session_token, CdmCipherMode* cipher_mode,
      const std::string& srm_requirement, OEMCrypto_LicenseType license_type);
  CryptoSessionId oec_session_id_;

 private:
  KeyId cached_key_id_;
  CdmCipherMode cipher_mode_;
};

}  // namespace wvcdm

#endif  // WVCDM_CORE_CONTENT_KEY_SESSSION_H_
