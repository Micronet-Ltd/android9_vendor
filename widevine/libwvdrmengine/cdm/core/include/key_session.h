#ifndef WVCDM_CORE_KEY_SESSSION_H_
#define WVCDM_CORE_KEY_SESSSION_H_

#include "metrics_collections.h"

namespace wvcdm {

class CryptoKey;

class KeySession {
 protected:
  KeySession(metrics::CryptoMetrics* metrics) : metrics_(metrics) {}

 public:
  typedef enum { kDefault, kSubLicense, kEntitlement } KeySessionType;
  virtual ~KeySession() {}
  virtual KeySessionType Type() = 0;
  virtual bool GenerateDerivedKeys(const std::string& message) = 0;
  virtual bool GenerateDerivedKeys(const std::string& message,
                                   const std::string& session_key) = 0;
  virtual OEMCryptoResult LoadKeys(const std::string& message,
                                   const std::string& signature,
                                   const std::string& mac_key_iv,
                                   const std::string& mac_key,
                                   const std::vector<CryptoKey>& keys,
                                   const std::string& provider_session_token,
                                   CdmCipherMode* cipher_mode,
                                   const std::string& srm_requirement) = 0;
  virtual OEMCryptoResult LoadEntitledContentKeys(
      const std::vector<CryptoKey>& keys) = 0;
  virtual OEMCryptoResult SelectKey(const std::string& key_id,
                                    CdmCipherMode cipher_mode) = 0;
  virtual OEMCryptoResult Decrypt(
      const CdmDecryptionParameters& params,
      OEMCrypto_DestBufferDesc& buffer_descriptor,
      OEMCrypto_CENCEncryptPatternDesc& pattern_descriptor) = 0;

 protected:
  metrics::CryptoMetrics* metrics_;
};

typedef std::map<std::string, CryptoSessionId> SubLicenseSessionMap;

}  // namespace wvcdm

#endif  // WVCDM_CORE_KEY_SESSSION_H_