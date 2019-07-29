#include "content_key_session.h"
#include "crypto_key.h"
#include "crypto_session.h"
#include "log.h"
#include "wv_cdm_constants.h"

namespace wvcdm {

// Generate Derived Keys for ContentKeySession
bool ContentKeySession::GenerateDerivedKeys(const std::string& message) {
  std::string mac_deriv_message;
  std::string enc_deriv_message;
  GenerateMacContext(message, &mac_deriv_message);
  GenerateEncryptContext(message, &enc_deriv_message);

  LOGV("GenerateDerivedKeys: id=%ld", (uint32_t)oec_session_id_);
  OEMCryptoResult sts;
  M_TIME(sts = OEMCrypto_GenerateDerivedKeys(
             oec_session_id_,
             reinterpret_cast<const uint8_t*>(mac_deriv_message.data()),
             mac_deriv_message.size(),
             reinterpret_cast<const uint8_t*>(enc_deriv_message.data()),
             enc_deriv_message.size()),
         metrics_, oemcrypto_generate_derived_keys_, sts);
  if (OEMCrypto_SUCCESS != sts) {
    LOGE("GenerateDerivedKeys: OEMCrypto_GenerateDerivedKeys error=%d", sts);
    return false;
  }

  return true;
}

// Generate Derived Keys (from session key) for ContentKeySession
bool ContentKeySession::GenerateDerivedKeys(const std::string& message,
                                            const std::string& session_key) {
  std::string mac_deriv_message;
  std::string enc_deriv_message;
  GenerateMacContext(message, &mac_deriv_message);
  GenerateEncryptContext(message, &enc_deriv_message);

  LOGV("GenerateDerivedKeys: id=%ld", (uint32_t)oec_session_id_);
  OEMCryptoResult sts;
  M_TIME(
      sts = OEMCrypto_DeriveKeysFromSessionKey(
          oec_session_id_, reinterpret_cast<const uint8_t*>(session_key.data()),
          session_key.size(),
          reinterpret_cast<const uint8_t*>(mac_deriv_message.data()),
          mac_deriv_message.size(),
          reinterpret_cast<const uint8_t*>(enc_deriv_message.data()),
          enc_deriv_message.size()),
      metrics_, oemcrypto_derive_keys_from_session_key_, sts);

  if (OEMCrypto_SUCCESS != sts) {
    LOGE("GenerateDerivedKeys: OEMCrypto_DeriveKeysFromSessionKey err=%d", sts);
    return false;
  }

  return true;
}

// Load Keys for ContentKeySession
OEMCryptoResult ContentKeySession::LoadKeys(
    const std::string& message, const std::string& signature,
    const std::string& mac_key_iv, const std::string& mac_key,
    const std::vector<CryptoKey>& keys,
    const std::string& provider_session_token, CdmCipherMode* cipher_mode,
    const std::string& srm_requirement) {
  return LoadKeys(message, signature, mac_key_iv, mac_key, keys,
                  provider_session_token, cipher_mode, srm_requirement,
                  OEMCrypto_ContentLicense);
}

// Select Key for ContentKeySession
OEMCryptoResult ContentKeySession::SelectKey(const std::string& key_id,
                                             CdmCipherMode cipher_mode) {
  // Crypto session lock already locked.
  if (!cached_key_id_.empty() && cached_key_id_ == key_id &&
      cipher_mode_ == cipher_mode) {
    // Already using the desired key and cipher mode.
    return OEMCrypto_SUCCESS;
  }

  cached_key_id_ = key_id;
  cipher_mode_ = cipher_mode;

  const uint8_t* key_id_string =
      reinterpret_cast<const uint8_t*>(cached_key_id_.data());

  OEMCryptoResult sts;
  M_TIME(sts = OEMCrypto_SelectKey(
             oec_session_id_, key_id_string, cached_key_id_.size(),
             ToOEMCryptoCipherMode(cipher_mode)),
         metrics_, oemcrypto_select_key_, sts);

  if (OEMCrypto_SUCCESS != sts) {
    cached_key_id_.clear();
  }
  return sts;
}

// Decrypt for ContentKeySession
OEMCryptoResult ContentKeySession::Decrypt(
    const CdmDecryptionParameters& params,
    OEMCrypto_DestBufferDesc& buffer_descriptor,
    OEMCrypto_CENCEncryptPatternDesc& pattern_descriptor) {
  OEMCryptoResult sts;
  M_TIME(sts = OEMCrypto_DecryptCENC(
             oec_session_id_, params.encrypt_buffer, params.encrypt_length,
             params.is_encrypted, &(*params.iv).front(), params.block_offset,
             &buffer_descriptor, &pattern_descriptor, params.subsample_flags),
         metrics_, oemcrypto_decrypt_cenc_, sts,
         metrics::Pow2Bucket(params.encrypt_length));
  return sts;
}

OEMCryptoResult ContentKeySession::LoadKeys(
    const std::string& message, const std::string& signature,
    const std::string& mac_key_iv, const std::string& mac_key,
    const std::vector<CryptoKey>& keys,
    const std::string& provider_session_token, CdmCipherMode* cipher_mode,
    const std::string& srm_requirement, OEMCrypto_LicenseType license_type) {
  const uint8_t* msg = reinterpret_cast<const uint8_t*>(message.data());
  const uint8_t* enc_mac_key = NULL;
  const uint8_t* enc_mac_key_iv = NULL;
  cached_key_id_.clear();
  if (mac_key.size() >= MAC_KEY_SIZE && mac_key_iv.size() >= KEY_IV_SIZE) {
    enc_mac_key = msg + GetOffset(message, mac_key);
    enc_mac_key_iv = msg + GetOffset(message, mac_key_iv);
  } else {
    LOGV("ContentKeySession::LoadKeys: enc_mac_key not set");
  }
  std::vector<OEMCrypto_KeyObject_V13> load_keys(keys.size());
  for (size_t i = 0; i < keys.size(); ++i) {
    const CryptoKey* ki = &keys[i];
    OEMCrypto_KeyObject_V13* ko = &load_keys[i];
    ko->key_id = msg + GetOffset(message, ki->key_id());
    ko->key_id_length = ki->key_id().length();
    ko->key_data_iv = msg + GetOffset(message, ki->key_data_iv());
    ko->key_data = msg + GetOffset(message, ki->key_data());
    ko->key_data_length = ki->key_data().length();
    if (ki->HasKeyControl()) {
      ko->key_control_iv = msg + GetOffset(message, ki->key_control_iv());
      ko->key_control = msg + GetOffset(message, ki->key_control());
    } else {
      LOGE("For key %d: XXX key has no control block. size=%d", i,
           ki->key_control().size());
      ko->key_control_iv = NULL;
      ko->key_control = NULL;
    }
    ko->cipher_mode = ToOEMCryptoCipherMode(ki->cipher_mode());

    // TODO(jfore): Is returning the cipher needed. If not drop this.
    *cipher_mode = ki->cipher_mode();
  }

  uint8_t* pst = NULL;
  if (!provider_session_token.empty()) {
    pst =
        const_cast<uint8_t*>(msg) + GetOffset(message, provider_session_token);
  }

  uint8_t* srm_req = NULL;
  if (!srm_requirement.empty()) {
    srm_req = const_cast<uint8_t*>(msg) + GetOffset(message, srm_requirement);
  }

  LOGV("LoadKeys: id=%ld", (uint32_t)oec_session_id_);
  OEMCryptoResult sts;
  M_TIME(
      sts = ::OEMCrypto_LoadKeys_Back_Compat(
          oec_session_id_, msg, message.size(),
          reinterpret_cast<const uint8_t*>(signature.data()), signature.size(),
          enc_mac_key_iv, enc_mac_key, keys.size(), &load_keys[0], pst,
          provider_session_token.length(), srm_req, license_type),
      metrics_, oemcrypto_load_keys_, sts);
  return sts;
}

}  // namespace wvcdm
