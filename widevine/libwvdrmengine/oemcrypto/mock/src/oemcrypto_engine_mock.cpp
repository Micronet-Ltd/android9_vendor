// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.
//
//  Mock implementation of OEMCrypto APIs
//
#include "oemcrypto_engine_mock.h"

#include <arpa/inet.h>
#include <assert.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <vector>

#include <openssl/err.h>

#include "keys.h"
#include "log.h"
#include "oemcrypto_key_mock.h"
#include "oemcrypto_rsa_key_shared.h"
#include "string_conversions.h"
#include "wv_cdm_constants.h"

namespace wvoec_mock {

// Note: The class CryptoEngine is configured at compile time by compiling in
// different device property files.  The methods in this file are generic to
// all configurations.  See the files oemcrypto_engine_device_properties*.cpp
// for methods that are configured for specific configurations.

CryptoEngine::CryptoEngine(std::auto_ptr<wvcdm::FileSystem> file_system)
    : root_of_trust_(config_provisioning_method()),
      file_system_(file_system),
      usage_table_(this) {
  ERR_load_crypto_strings();
}

CryptoEngine::~CryptoEngine() {
  wvcdm::AutoLock lock(session_table_lock_);
  ActiveSessions::iterator it;
  for (it = sessions_.begin(); it != sessions_.end(); ++it) {
    delete it->second;
  }
  sessions_.clear();
  ERR_free_strings();
}

SessionId CryptoEngine::CreateSession() {
  wvcdm::AutoLock lock(session_table_lock_);
  static int unique_id = 1;
  SessionId sid = (SessionId)++unique_id;
  SessionContext* sctx =
      new SessionContext(this, sid, root_of_trust_.SharedRsaKey());
  sessions_[sid] = sctx;
  return sid;
}

bool CryptoEngine::DestroySession(SessionId sid) {
  SessionContext* sctx = FindSession(sid);
  wvcdm::AutoLock lock(session_table_lock_);
  if (sctx) {
    sessions_.erase(sid);
    delete sctx;
    return true;
  } else {
    return false;
  }
}

SessionContext* CryptoEngine::FindSession(SessionId sid) {
  wvcdm::AutoLock lock(session_table_lock_);
  ActiveSessions::iterator it = sessions_.find(sid);
  if (it != sessions_.end()) {
    return it->second;
  }
  return NULL;
}

OEMCrypto_HDCP_Capability CryptoEngine::config_current_hdcp_capability() {
  return config_local_display_only() ? HDCP_NO_DIGITAL_OUTPUT : HDCP_V1;
}

OEMCrypto_HDCP_Capability CryptoEngine::config_maximum_hdcp_capability() {
  return HDCP_NO_DIGITAL_OUTPUT;
}

OEMCryptoResult CryptoEngine::SetDestination(
    OEMCrypto_DestBufferDesc* out_description, size_t data_length,
    uint8_t subsample_flags) {
  size_t max_length = 0;
  switch (out_description->type) {
    case OEMCrypto_BufferType_Clear:
      destination_ = out_description->buffer.clear.address;
      max_length = out_description->buffer.clear.max_length;
      break;
    case OEMCrypto_BufferType_Secure:
      destination_ =
          reinterpret_cast<uint8_t*>(out_description->buffer.secure.handle) +
          out_description->buffer.secure.offset;
      max_length = out_description->buffer.secure.max_length -
                   out_description->buffer.secure.offset;
      break;
    case OEMCrypto_BufferType_Direct:
      // Direct buffer type is only used on some specialized devices where
      // oemcrypto has a direct connection to the screen buffer.  It is not,
      // for example, supported on Android.
      destination_ = NULL;
      break;
    default:
      return OEMCrypto_ERROR_INVALID_CONTEXT;
  }
  if (out_description->type != OEMCrypto_BufferType_Direct &&
      max_length < data_length) {
    LOGE("[SetDestination(): OEMCrypto_ERROR_SHORT_BUFFER]");
    return OEMCrypto_ERROR_SHORT_BUFFER;
  }
  adjust_destination(out_description, data_length, subsample_flags);
  if ((out_description->type != OEMCrypto_BufferType_Direct) &&
      (destination_ == NULL)) {
    return OEMCrypto_ERROR_INVALID_CONTEXT;
  }
  return OEMCrypto_SUCCESS;
}

}  // namespace wvoec_mock
