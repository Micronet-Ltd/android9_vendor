#ifndef WVCDM_CORE_ENTITLEMENT_KEY_SESSSION_H_
#define WVCDM_CORE_ENTITLEMENT_KEY_SESSSION_H_

#include "content_key_session.h"
#include "key_session.h"

namespace wvcdm {

class EntitlementKeySession : public ContentKeySession {
 public:
  EntitlementKeySession(CryptoSessionId oec_session_id,
                        metrics::CryptoMetrics* metrics);
  virtual ~EntitlementKeySession() {}

  KeySessionType Type() { return kEntitlement; }

  // Load Keys for ContentKeySession
  OEMCryptoResult LoadKeys(const std::string& message,
                           const std::string& signature,
                           const std::string& mac_key_iv,
                           const std::string& mac_key,
                           const std::vector<CryptoKey>& keys,
                           const std::string& provider_session_token,
                           CdmCipherMode* cipher_mode,
                           const std::string& srm_requirement);
  OEMCryptoResult LoadEntitledContentKeys(const std::vector<CryptoKey>& keys);

 private:
  std::vector<CryptoKey> keys_;
};

}  // namespace wvcdm

#endif  // WVCDM_CORE_ENTITLEMENT_KEY_SESSSION_H_