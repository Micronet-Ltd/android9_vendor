//
// Copyright 2017 Google Inc. All Rights Reserved.
//

#include "WVCreatePluginFactories.h"

#include "WVCryptoFactory.h"
#include "WVDrmFactory.h"

namespace wvdrm {
namespace hardware {
namespace drm {
namespace V1_1 {
namespace widevine {

extern "C" {

IDrmFactory* createDrmFactory() {
  return new WVDrmFactory();
}

ICryptoFactory* createCryptoFactory() {
  return new WVCryptoFactory();
}

}  // extern "C"
}  // namespace widevine
}  // namespace V1_1
}  // namespace drm
}  // namespace hardware
}  // namespace wvdrm
