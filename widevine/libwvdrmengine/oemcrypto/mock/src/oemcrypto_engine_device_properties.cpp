// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.
//
//  Mock implementation of OEMCrypto APIs
//

#include "oemcrypto_engine_mock.h"

namespace wvoec_mock {

CryptoEngine* CryptoEngine::MakeCryptoEngine(
    std::auto_ptr<wvcdm::FileSystem> file_system) {
  return new CryptoEngine(file_system);
}

}  // namespace wvoec_mock
