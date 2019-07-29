// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.
//
// Description:
//   Dummy version of privacy crypto classes for systems which
//   can't tolerate OpenSSL as a dependency.
//

#include "privacy_crypto.h"

namespace wvcdm {

AesCbcKey::AesCbcKey() {}

AesCbcKey::~AesCbcKey() {}

bool AesCbcKey::Init(const std::string& key) { return false; }

bool AesCbcKey::Encrypt(const std::string& in, std::string* out,
                        std::string* iv) {
  return false;
}

RsaPublicKey::RsaPublicKey() {}

RsaPublicKey::~RsaPublicKey() {}

bool RsaPublicKey::Init(const std::string& serialized_key) { return false; }

bool RsaPublicKey::Encrypt(const std::string& clear_message,
                           std::string* encrypted_message) {
  return false;
}

bool RsaPublicKey::VerifySignature(const std::string& message,
                                   const std::string& signature) {
  return false;
}

}  // namespace wvcdm
