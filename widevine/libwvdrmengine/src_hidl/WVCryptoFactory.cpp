//
// Copyright 2017 Google Inc. All Rights Reserved.
//

//#define LOG_NDEBUG 0
#define LOG_TAG "WVCdm"
#include <utils/Log.h>

#include "WVCryptoFactory.h"
#include "WVCDMSingleton.h"
#include "WVCryptoPlugin.h"
#include "WVUUID.h"

namespace wvdrm {
namespace hardware {
namespace drm {
namespace V1_1 {
namespace widevine {

using ::android::hardware::drm::V1_0::Status;
using ::android::hardware::Void;

Return<bool> WVCryptoFactory::isCryptoSchemeSupported(
    const hidl_array<uint8_t, 16>& uuid) {
  return isWidevineUUID(uuid.data());
}

Return<void> WVCryptoFactory::createPlugin(
    const hidl_array<uint8_t, 16>& uuid,
    const hidl_vec<uint8_t>& initData,
    createPlugin_cb _hidl_cb) {

  WVCryptoPlugin *plugin = NULL;
  if (!isCryptoSchemeSupported(uuid.data())) {
    ALOGE("Widevine Drm HAL: failed to create crypto plugin, " \
        "invalid crypto scheme");
    _hidl_cb(Status::ERROR_DRM_CANNOT_HANDLE, plugin);
    return Void();
  }

  plugin = new WVCryptoPlugin(initData.data(), initData.size(), getCDM());
  _hidl_cb(Status::OK, plugin);
  return Void();
}

}  // namespace widevine
}  // namespace V1_1
}  // namespace drm
}  // namespace hardware
}  // namespace wvdrm
