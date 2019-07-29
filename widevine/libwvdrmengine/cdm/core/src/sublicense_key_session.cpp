#include "sublicense_key_session.h"

#include "crypto_session.h"
#include "log.h"
#include "wv_cdm_constants.h"

namespace wvcdm {

SubLicenseKeySession::SubLicenseKeySession(
    SubLicenseSessionMap& sub_license_oec_sessions,
    metrics::CryptoMetrics* metrics,
    const std::string& wrapped_private_device_key,
    SecurityLevel requested_security_level,
    const std::string& group_id)
    : KeySession(metrics),
      state_(kInitializing),
      wrapped_private_device_key_(wrapped_private_device_key),
      sub_license_oec_sessions_(sub_license_oec_sessions),
      requested_security_level_(requested_security_level),
      group_id_(group_id) {}

SubLicenseKeySession::~SubLicenseKeySession() {
  for (SubLicenseSessionMap::iterator oec_session =
           sub_license_oec_sessions_.begin();
       oec_session != sub_license_oec_sessions_.end(); oec_session++) {
    metrics_->oemcrypto_close_session_.Increment(
        OEMCrypto_CloseSession(oec_session->second));
  }
  sub_license_oec_sessions_.clear();
}

// GenerateDerivedKeys is called for each open oemcrypto session and is only
// called once.
bool SubLicenseKeySession::GenerateDerivedKeys(const std::string& message,
                                               const std::string& session_key) {
  std::string mac_deriv_message;
  std::string enc_deriv_message;
  GenerateMacContext(message, &mac_deriv_message);
  GenerateEncryptContext(message, &enc_deriv_message);

  for (SubLicenseSessionMap::iterator it = sub_license_oec_sessions_.begin();
       it != sub_license_oec_sessions_.end(); it++) {
    LOGV("GenerateDerivedKeys: id=%ld", (uint32_t)it->second);
    OEMCryptoResult sts;
    M_TIME(sts = OEMCrypto_DeriveKeysFromSessionKey(
               it->second, reinterpret_cast<const uint8_t*>(session_key.data()),
               session_key.size(),
               reinterpret_cast<const uint8_t*>(mac_deriv_message.data()),
               mac_deriv_message.size(),
               reinterpret_cast<const uint8_t*>(enc_deriv_message.data()),
               enc_deriv_message.size()),
           metrics_, oemcrypto_derive_keys_from_session_key_, sts);

    if (OEMCrypto_SUCCESS != sts) {
      LOGE("GenerateDerivedKeys: OEMCrypto_DeriveKeysFromSessionKey err=%d",
           sts);
      return false;
    }
  }

  return true;
}

OEMCryptoResult SubLicenseKeySession::LoadKeys(
    const std::string& message, const std::string& signature,
    const std::string& mac_key_iv, const std::string& mac_key,
    const std::vector<CryptoKey>& keys,
    const std::string& provider_session_token, CdmCipherMode* cipher_mode,
    const std::string& srm_requirement) {
  if (state_ == kInitializing) {
    state_ = kInitialLicenseLoaded;
    keys_ = keys;
    OEMCryptoResult sts =
        DoLoadKeys(message, signature, mac_key_iv, mac_key, keys,
                   provider_session_token, cipher_mode, srm_requirement);
    if (OEMCrypto_SUCCESS != sts) {
      state_ = kInitialLicenseFailed;
    }
    return sts;
  }
  return DoSubLicenseLoadKeys(message, signature, mac_key_iv, mac_key, keys[0],
                              provider_session_token, cipher_mode,
                              srm_requirement);
}

OEMCryptoResult SubLicenseKeySession::SelectKey(const std::string& key_id,
                                                CdmCipherMode cipher_mode) {
  for (size_t i = 0; i < keys_.size(); ++i) {
    if (keys_[i].key_id() == key_id) {
      cached_sub_session_key_id_ = keys_[i].sub_session_key_id();
      if (keys_[i].cipher_mode() != cipher_mode) {
        SubLicenseSessionMap::iterator it =
            sub_license_oec_sessions_.find(cached_sub_session_key_id_);
        if (it == sub_license_oec_sessions_.end()) {
          return OEMCrypto_ERROR_INVALID_SESSION;
        }

        OEMCryptoResult status = OEMCrypto_SUCCESS;
        M_TIME(status = OEMCrypto_SelectKey(
                   it->second,
                   reinterpret_cast<const uint8_t*>(keys_[i].key_id().data()),
                   keys_[i].key_id().size(),
                   ToOEMCryptoCipherMode(cipher_mode)),
               metrics_, oemcrypto_select_key_, status);
        if (OEMCrypto_SUCCESS != status) {
          return status;
        }
        keys_[i].set_cipher_mode(cipher_mode);
      }
    }
  }
  return OEMCrypto_SUCCESS;
}

OEMCryptoResult SubLicenseKeySession::Decrypt(
    const CdmDecryptionParameters& params,
    OEMCrypto_DestBufferDesc& buffer_descriptor,
    OEMCrypto_CENCEncryptPatternDesc& pattern_descriptor) {
  SubLicenseSessionMap::iterator it =
      sub_license_oec_sessions_.find(cached_sub_session_key_id_);
  if (it == sub_license_oec_sessions_.end()) {
    return OEMCrypto_ERROR_INVALID_SESSION;
  }
  OEMCryptoResult sts;
  M_TIME(sts = OEMCrypto_DecryptCENC(
             it->second, params.encrypt_buffer, params.encrypt_length,
             params.is_encrypted, &(*params.iv).front(), params.block_offset,
             &buffer_descriptor, &pattern_descriptor, params.subsample_flags),
         metrics_, oemcrypto_decrypt_cenc_, sts,
         metrics::Pow2Bucket(params.encrypt_length));
  return sts;
}

OEMCryptoResult SubLicenseKeySession::ResetCryptoSessions() {
  for (SubLicenseSessionMap::iterator it = sub_license_oec_sessions_.begin();
       it != sub_license_oec_sessions_.end(); it++) {
    OEMCryptoResult sts = OEMCrypto_CloseSession(it->second);
    metrics_->oemcrypto_close_session_.Increment(sts);
    if (OEMCrypto_SUCCESS != sts) {
      return sts;
    }
    sts = OEMCrypto_OpenSession(&it->second, requested_security_level_);
    if (OEMCrypto_SUCCESS != sts) {
      return sts;
    }
    M_TIME(sts = OEMCrypto_LoadDeviceRSAKey(
               it->second,
               reinterpret_cast<const uint8_t*>(
                   wrapped_private_device_key_.data()),
               wrapped_private_device_key_.size()),
           metrics_, oemcrypto_load_device_rsa_key_, sts);
    if (OEMCrypto_SUCCESS != sts) {
      return sts;
    }
  }
  return OEMCrypto_SUCCESS;
}

OEMCryptoResult SubLicenseKeySession::DoLoadKeys(
    const std::string& message, const std::string& signature,
    const std::string& mac_key_iv, const std::string& mac_key,
    const std::vector<CryptoKey>& keys,
    const std::string& provider_session_token, CdmCipherMode* cipher_mode,
    const std::string& srm_requirement) {
  const uint8_t* msg = reinterpret_cast<const uint8_t*>(message.data());
  const uint8_t* enc_mac_key = NULL;
  const uint8_t* enc_mac_key_iv = NULL;
  if (mac_key.size() >= MAC_KEY_SIZE && mac_key_iv.size() >= KEY_IV_SIZE) {
    enc_mac_key = msg + GetOffset(message, mac_key);
    enc_mac_key_iv = msg + GetOffset(message, mac_key_iv);
  } else {
    LOGV("CryptoSession::LoadKeys: enc_mac_key not set");
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

  for (size_t i = 0; i < keys.size(); i++) {
    OEMCrypto_KeyObject_V13 key_object;
    const CryptoKey& key_data = keys[i];
    key_object.key_id = msg + GetOffset(message, key_data.key_id());
    key_object.key_id_length = key_data.key_id().length();
    key_object.key_data_iv = msg + GetOffset(message, key_data.key_data_iv());
    key_object.key_data = msg + GetOffset(message, key_data.key_data());
    key_object.key_data_length = key_data.key_data().length();
    if (key_data.HasKeyControl()) {
      key_object.key_control_iv =
          msg + GetOffset(message, key_data.key_control_iv());
      key_object.key_control = msg + GetOffset(message, key_data.key_control());
    } else {
      LOGE("For key %s: XXX key has no control block. size=%d",
           key_data.key_id().c_str(), key_data.key_control().size());
      key_object.key_control_iv = NULL;
      key_object.key_control = NULL;
    }
    key_object.cipher_mode = ToOEMCryptoCipherMode(key_data.cipher_mode());

    // TODO(jfore): Does returning the cipher mode serve any purpose?
    // If not drop.
    *cipher_mode = key_data.cipher_mode();

    SubLicenseSessionMap::iterator oec_session_id =
        sub_license_oec_sessions_.find(key_data.sub_session_key_id());
    if (oec_session_id == sub_license_oec_sessions_.end()) {
      LOGE("CryptoSession::LoadKeys: Unrecognized sub session %s",
           key_data.sub_session_key_id().c_str());
      return OEMCrypto_ERROR_INVALID_SESSION;
    }

    OEMCryptoResult sts;
    M_TIME(
        sts = OEMCrypto_LoadKeys_Back_Compat(
            oec_session_id->second, msg, message.size(),
            reinterpret_cast<const uint8_t*>(signature.data()),
            signature.size(), enc_mac_key_iv, enc_mac_key, 1, &key_object, pst,
            provider_session_token.length(), srm_req, OEMCrypto_ContentLicense),
        metrics_, oemcrypto_load_keys_, sts);

    if (sts != OEMCrypto_SUCCESS) {
      return sts;
    }

    M_TIME(sts = OEMCrypto_SelectKey(
               oec_session_id->second,
               reinterpret_cast<const uint8_t*>(key_data.key_id().data()),
               key_data.key_id().size(),
               ToOEMCryptoCipherMode(key_data.cipher_mode())),
           metrics_, oemcrypto_select_key_, sts);

    if (sts != OEMCrypto_SUCCESS) {
      return sts;
    }
  }
  keys_ = keys;
  return OEMCrypto_SUCCESS;
}

OEMCryptoResult SubLicenseKeySession::DoSubLicenseLoadKeys(
    const std::string& message, const std::string& signature,
    const std::string& mac_key_iv, const std::string& mac_key,
    const CryptoKey& key, const std::string& provider_session_token,
    CdmCipherMode*, const std::string& srm_requirement) {
  SubLicenseSessionMap::iterator it = sub_license_oec_sessions_.end();
  size_t key_index = 0;
  for (; key_index < keys_.size(); key_index++) {
    if (keys_[key_index].track_label() == key.track_label()) {
      it =
          sub_license_oec_sessions_.find(keys_[key_index].sub_session_key_id());
      CryptoKey tmp = key;
      tmp.set_sub_session_key_id(keys_[key_index].sub_session_key_id());
      tmp.set_sub_session_key(keys_[key_index].sub_session_key());
      keys_[key_index] = tmp;
      break;
    }
  }
  if (it == sub_license_oec_sessions_.end()) {
    return OEMCrypto_SUCCESS;
  }

  LOGV("GenerateDerivedKeys: id=%ld", (uint32_t)it->second);

  std::string mac_deriv_message;
  std::string enc_deriv_message;
  GenerateMacContext(group_id_ + message.c_str(),
                     &mac_deriv_message);
  GenerateEncryptContext(group_id_ + message.c_str(),
                         &enc_deriv_message);

  const uint8_t* msg = reinterpret_cast<const uint8_t*>(message.data());
  const uint8_t* enc_mac_key = NULL;
  const uint8_t* enc_mac_key_iv = NULL;
  if (mac_key.size() >= MAC_KEY_SIZE && mac_key_iv.size() >= KEY_IV_SIZE) {
    enc_mac_key = msg + GetOffset(message, mac_key);
    enc_mac_key_iv = msg + GetOffset(message, mac_key_iv);
  } else {
    LOGV("CryptoSession::LoadKeys: enc_mac_key not set");
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

  OEMCryptoResult sts;
  const std::string& sub_session_key = keys_[key_index].sub_session_key();
  LOGV("ssksize = %d", sub_session_key.size());

  M_TIME(
      sts = OEMCrypto_DeriveKeysFromSessionKey(
          it->second, reinterpret_cast<const uint8_t*>(sub_session_key.data()),
          sub_session_key.size(),
          reinterpret_cast<const uint8_t*>(mac_deriv_message.data()),
          mac_deriv_message.size(),
          reinterpret_cast<const uint8_t*>(enc_deriv_message.data()),
          enc_deriv_message.size()),
      metrics_, oemcrypto_derive_keys_from_session_key_, sts);

  if (OEMCrypto_SUCCESS != sts) {
    LOGE("GenerateDerivedKeys: OEMCrypto_DeriveKeysFromSessionKey err=%d", sts);
    return sts;
  }

  OEMCrypto_KeyObject_V13 key_object;
  key_object.key_id = msg + GetOffset(message, keys_[key_index].key_id());
  key_object.key_id_length = keys_[key_index].key_id().length();
  key_object.key_data_iv =
      msg + GetOffset(message, keys_[key_index].key_data_iv());
  key_object.key_data = msg + GetOffset(message, keys_[key_index].key_data());
  key_object.key_data_length = keys_[key_index].key_data().length();
  if (key.HasKeyControl()) {
    key_object.key_control_iv =
        msg + GetOffset(message, keys_[key_index].key_control_iv());
    key_object.key_control =
        msg + GetOffset(message, keys_[key_index].key_control());
  }
  key_object.cipher_mode =
      ToOEMCryptoCipherMode(keys_[key_index].cipher_mode());

  M_TIME(
      sts = OEMCrypto_LoadKeys_Back_Compat(
          it->second, msg, message.size(),
          reinterpret_cast<const uint8_t*>(signature.data()), signature.size(),
          enc_mac_key_iv, enc_mac_key, 1, &key_object, pst,
          provider_session_token.length(), srm_req, OEMCrypto_ContentLicense),
      metrics_, oemcrypto_load_keys_, sts);

  if (sts != OEMCrypto_SUCCESS) {
    return sts;
  }

  M_TIME(sts = OEMCrypto_SelectKey(
             it->second,
             reinterpret_cast<const uint8_t*>(keys_[key_index].key_id().data()),
             keys_[key_index].key_id().size(),
             ToOEMCryptoCipherMode(keys_[key_index].cipher_mode())),
         metrics_, oemcrypto_select_key_, sts);

  return sts;
}

}  // namespace wvcdm
