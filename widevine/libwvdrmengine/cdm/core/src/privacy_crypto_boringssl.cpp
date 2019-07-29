// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.
//
// Description:
//   Definition of classes representing RSA public keys used
//   for signature verification and encryption and decryption.
//

#include "privacy_crypto.h"

#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>

#include "log.h"

namespace {
const int kPssSaltLength = 20;
const int kRsaPkcs1OaepPaddingLength = 41;

RSA* GetKey(const std::string& serialized_key) {
  BIO* bio = BIO_new_mem_buf(const_cast<char*>(serialized_key.data()),
                             serialized_key.size());
  if (bio == NULL) {
    LOGE("GetKey: BIO_new_mem_buf returned NULL");
    return NULL;
  }
  RSA* key = d2i_RSAPublicKey_bio(bio, NULL);

  if (key == NULL) {
    LOGE("GetKey: RSA key deserialization failure: %s",
         ERR_error_string(ERR_get_error(), NULL));
    BIO_free(bio);
    return NULL;
  }

  BIO_free(bio);
  return key;
}

void FreeKey(RSA* key) {
  if (key != NULL) {
    RSA_free(key);
  }
}

}  // namespace

namespace wvcdm {

AesCbcKey::AesCbcKey() {}

AesCbcKey::~AesCbcKey() {}

bool AesCbcKey::Init(const std::string& key) {
  if (key.size() != AES_BLOCK_SIZE) {
    LOGE("AesCbcKey::Init: unexpected key size: %d", key.size());
    return false;
  }

  key_ = key;
  return true;
}

bool AesCbcKey::Encrypt(const std::string& in, std::string* out,
                        std::string* iv) {
  if (in.empty()) {
    LOGE("AesCbcKey::Encrypt: no cleartext provided");
    return false;
  }
  if (iv == NULL) {
    LOGE("AesCbcKey::Encrypt: initialization vector destination not provided");
    return false;
  }
  if (iv->size() != AES_BLOCK_SIZE) {
    LOGE("AesCbcKey::Encrypt: invalid iv size: %d", iv->size());
    return false;
  }
  if (out == NULL) {
    LOGE("AesCbcKey::Encrypt: crypttext destination not provided");
    return false;
  }
  if (key_.empty()) {
    LOGE("AesCbcKey::Encrypt: AES key not initialized");
    return false;
  }

#if (OPENSSL_VERSION_NUMBER < 0x10100000L)
  EVP_CIPHER_CTX ctx_struct;
  EVP_CIPHER_CTX* evp_cipher_ctx = &ctx_struct;
#else
  EVP_CIPHER_CTX* evp_cipher_ctx = EVP_CIPHER_CTX_new();
#endif
  if (EVP_EncryptInit(evp_cipher_ctx, EVP_aes_128_cbc(),
                      reinterpret_cast<uint8_t*>(&key_[0]),
                      reinterpret_cast<uint8_t*>(&(*iv)[0])) == 0) {
    LOGE("AesCbcKey::Encrypt: AES CBC setup failure: %s",
         ERR_error_string(ERR_get_error(), NULL));
#if (OPENSSL_VERSION_NUMBER < 0x10100000L)
    EVP_CIPHER_CTX_cleanup(evp_cipher_ctx);
#else
    EVP_CIPHER_CTX_free(evp_cipher_ctx);
#endif
    return false;
  }

  out->resize(in.size() + AES_BLOCK_SIZE);
  int out_length = out->size();
  if (EVP_EncryptUpdate(
          evp_cipher_ctx, reinterpret_cast<uint8_t*>(&(*out)[0]), &out_length,
          reinterpret_cast<uint8_t*>(const_cast<char*>(in.data())),
          in.size()) == 0) {
    LOGE("AesCbcKey::Encrypt: encryption failure: %s",
         ERR_error_string(ERR_get_error(), NULL));
#if (OPENSSL_VERSION_NUMBER < 0x10100000L)
    EVP_CIPHER_CTX_cleanup(evp_cipher_ctx);
#else
    EVP_CIPHER_CTX_free(evp_cipher_ctx);
#endif
    return false;
  }

  int padding = 0;
  if (EVP_EncryptFinal_ex(evp_cipher_ctx,
                          reinterpret_cast<uint8_t*>(&(*out)[out_length]),
                          &padding) == 0) {
    LOGE("AesCbcKey::Encrypt: PKCS7 padding failure: %s",
         ERR_error_string(ERR_get_error(), NULL));
#if (OPENSSL_VERSION_NUMBER < 0x10100000L)
    EVP_CIPHER_CTX_cleanup(evp_cipher_ctx);
#else
    EVP_CIPHER_CTX_free(evp_cipher_ctx);
#endif
    return false;
  }

#if (OPENSSL_VERSION_NUMBER < 0x10100000L)
  EVP_CIPHER_CTX_cleanup(evp_cipher_ctx);
#else
  EVP_CIPHER_CTX_free(evp_cipher_ctx);
#endif
  out->resize(out_length + padding);
  return true;
}

RsaPublicKey::RsaPublicKey() {}

RsaPublicKey::~RsaPublicKey() {}

bool RsaPublicKey::Init(const std::string& serialized_key) {
  if (serialized_key.empty()) {
    LOGE("RsaPublicKey::Init: no serialized key provided");
    return false;
  }

  serialized_key_ = serialized_key;
  return true;
}

bool RsaPublicKey::Encrypt(const std::string& clear_message,
                           std::string* encrypted_message) {
  if (clear_message.empty()) {
    LOGE("RsaPublicKey::Encrypt: message to be encrypted is empty");
    return false;
  }
  if (encrypted_message == NULL) {
    LOGE("RsaPublicKey::Encrypt: no encrypt message buffer provided");
    return false;
  }
  if (serialized_key_.empty()) {
    LOGE("RsaPublicKey::Encrypt: RSA key not initialized");
    return false;
  }

  RSA* key = GetKey(serialized_key_);
  if (key == NULL) {
    // Error already logged by GetKey.
    return false;
  }

  int rsa_size = RSA_size(key);
  if (static_cast<int>(clear_message.size()) >
      rsa_size - kRsaPkcs1OaepPaddingLength) {
    LOGE("RsaPublicKey::Encrypt: message too large to be encrypted (actual %d",
         " max allowed %d)", clear_message.size(),
         rsa_size - kRsaPkcs1OaepPaddingLength);
    FreeKey(key);
    return false;
  }

  encrypted_message->assign(rsa_size, 0);
  if (RSA_public_encrypt(
          clear_message.size(),
          const_cast<unsigned char*>(
              reinterpret_cast<const unsigned char*>(clear_message.data())),
          reinterpret_cast<unsigned char*>(&(*encrypted_message)[0]), key,
          RSA_PKCS1_OAEP_PADDING) != rsa_size) {
    LOGE("RsaPublicKey::Encrypt: encrypt failure: %s",
         ERR_error_string(ERR_get_error(), NULL));
    FreeKey(key);
    return false;
  }

  FreeKey(key);
  return true;
}

// LogOpenSSLError is a callback from OpenSSL which is called with each error
// in the thread's error queue.
static int LogOpenSSLError(const char* msg, size_t /* len */, void* /* ctx */) {
  LOGE("  %s", msg);
  return 1;
}

static bool VerifyPSSSignature(EVP_PKEY *pkey, const std::string &message,
                               const std::string &signature) {
#if (OPENSSL_VERSION_NUMBER < 0x10100000L)
  EVP_MD_CTX ctx_struct;
  EVP_MD_CTX* evp_md_ctx = &ctx_struct;
  EVP_MD_CTX_init(evp_md_ctx);
#else
  EVP_MD_CTX* evp_md_ctx = EVP_MD_CTX_new();
#endif
  EVP_PKEY_CTX *pctx = NULL;

  if (EVP_DigestVerifyInit(evp_md_ctx, &pctx, EVP_sha1(), NULL /* no ENGINE */,
                           pkey) != 1) {
    LOGE("EVP_DigestVerifyInit failed in VerifyPSSSignature");
    goto err;
  }

  if (EVP_PKEY_CTX_set_signature_md(pctx,
                                    const_cast<EVP_MD *>(EVP_sha1())) != 1) {
    LOGE("EVP_PKEY_CTX_set_signature_md failed in VerifyPSSSignature");
    goto err;
  }

  if (EVP_PKEY_CTX_set_rsa_padding(pctx, RSA_PKCS1_PSS_PADDING) != 1) {
    LOGE("EVP_PKEY_CTX_set_rsa_padding failed in VerifyPSSSignature");
    goto err;
  }

  if (EVP_PKEY_CTX_set_rsa_pss_saltlen(pctx, kPssSaltLength) != 1) {
    LOGE("EVP_PKEY_CTX_set_rsa_pss_saltlen failed in VerifyPSSSignature");
    goto err;
  }

  if (EVP_DigestVerifyUpdate(evp_md_ctx, message.data(), message.size()) != 1) {
    LOGE("EVP_DigestVerifyUpdate failed in VerifyPSSSignature");
    goto err;
  }

  if (EVP_DigestVerifyFinal(
          evp_md_ctx, const_cast<uint8_t *>(
                    reinterpret_cast<const uint8_t *>(signature.data())),
          signature.size()) != 1) {
    LOGE(
        "EVP_DigestVerifyFinal failed in VerifyPSSSignature. (Probably a bad "
        "signature.)");
    goto err;
  }

#if (OPENSSL_VERSION_NUMBER < 0x10100000L)
    EVP_MD_CTX_cleanup(evp_md_ctx);
#else
    EVP_MD_CTX_free(evp_md_ctx);
#endif
  return true;

err:
  ERR_print_errors_cb(LogOpenSSLError, NULL);
#if (OPENSSL_VERSION_NUMBER < 0x10100000L)
  EVP_MD_CTX_cleanup(evp_md_ctx);
#else
  EVP_MD_CTX_free(evp_md_ctx);
#endif
  return false;
}

bool RsaPublicKey::VerifySignature(const std::string& message,
                                   const std::string& signature) {
  if (serialized_key_.empty()) {
    LOGE("RsaPublicKey::VerifySignature: RSA key not initialized");
    return false;
  }
  if (message.empty()) {
    LOGE("RsaPublicKey::VerifySignature: signed message is empty");
    return false;
  }
  RSA* rsa_key = GetKey(serialized_key_);
  if (rsa_key == NULL) {
    // Error already logged by GetKey.
    return false;
  }
  EVP_PKEY *pkey = EVP_PKEY_new();
  if (pkey == NULL) {
    LOGE("RsaPublicKey::VerifySignature: EVP_PKEY allocation failed");
    FreeKey(rsa_key);
    return false;
  }
  if (EVP_PKEY_set1_RSA(pkey, rsa_key) != 1) {
    LOGE("RsaPublicKey::VerifySignature: failed to wrap key in an EVP_PKEY");
    FreeKey(rsa_key);
    EVP_PKEY_free(pkey);
    return false;
  }
  FreeKey(rsa_key);

  const bool ok = VerifyPSSSignature(pkey, message, signature);
  EVP_PKEY_free(pkey);

  if (!ok) {
    LOGE("RsaPublicKey::VerifySignature: RSA verify failure");
    return false;
  }

  return true;
}

}  // namespace wvcdm
