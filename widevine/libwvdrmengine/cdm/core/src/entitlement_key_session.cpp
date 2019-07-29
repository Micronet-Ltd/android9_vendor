#include "entitlement_key_session.h"

#include "crypto_key.h"

namespace wvcdm {
EntitlementKeySession::EntitlementKeySession(CryptoSessionId oec_session_id,
                                             metrics::CryptoMetrics* metrics)
    : ContentKeySession(oec_session_id, metrics) {}

OEMCryptoResult EntitlementKeySession::LoadKeys(
    const std::string& message, const std::string& signature,
    const std::string& mac_key_iv, const std::string& mac_key,
    const std::vector<CryptoKey>& keys,
    const std::string& provider_session_token, CdmCipherMode* cipher_mode,
    const std::string& srm_requirement) {
  keys_.resize(keys.size());
  return ContentKeySession::LoadKeys(
      message, signature, mac_key_iv, mac_key, keys, provider_session_token,
      cipher_mode, srm_requirement, OEMCrypto_EntitlementLicense);
}

OEMCryptoResult EntitlementKeySession::LoadEntitledContentKeys(
    const std::vector<CryptoKey>& keys) {
  // The array |keys| contains new content keys, plus entitlement key ids for
  // those content keys.
  std::vector<OEMCrypto_EntitledContentKeyObject> entitlements;
  entitlements.resize(keys.size());

  for (size_t i = 0; i < keys.size(); ++i) {
    entitlements[i].entitlement_key_id =
        reinterpret_cast<const uint8_t*>(keys[i].entitlement_key_id().data());
    entitlements[i].entitlement_key_id_length =
        keys[i].entitlement_key_id().size();

    entitlements[i].content_key_id =
        reinterpret_cast<const uint8_t*>(keys[i].key_id().data());
    entitlements[i].content_key_id_length = keys[i].key_id().size();

    entitlements[i].content_key_data_iv =
        reinterpret_cast<const uint8_t*>(keys[i].key_data_iv().data());

    entitlements[i].content_key_data =
        reinterpret_cast<const uint8_t*>(keys[i].key_data().data());
    entitlements[i].content_key_data_length = keys[i].key_data().size();
  }

  OEMCryptoResult result = OEMCrypto_SUCCESS;
  M_TIME(result = OEMCrypto_LoadEntitledContentKeys(
             oec_session_id_, entitlements.size(), &entitlements[0]),
         metrics_, oemcrypto_load_entitled_keys_, result);
  return result;
}

}  // namespace wvcdm