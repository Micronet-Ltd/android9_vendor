//
// Copyright 2017 Google Inc. All Rights Reserved.
//

//#define LOG_NDEBUG 0
#define LOG_TAG "WVCdm"
#include <utils/Log.h>

#include "WVDrmFactory.h"

#include "cutils/properties.h"
#include "wv_cdm_constants.h"
#include "WVCDMSingleton.h"
#include "wv_content_decryption_module.h"
#include "WVDrmPlugin.h"
#include "WVUUID.h"

namespace wvdrm {
namespace hardware {
namespace drm {
namespace V1_1 {
namespace widevine {

using ::android::hardware::drm::V1_0::Status;
using ::android::hardware::Void;

WVGenericCryptoInterface WVDrmFactory::sOemCryptoInterface;

Return<bool> WVDrmFactory::isCryptoSchemeSupported(
    const hidl_array<uint8_t, 16>& uuid) {
  return isWidevineUUID(uuid.data());
}

Return<bool> WVDrmFactory::isContentTypeSupported(
    const hidl_string& initDataType) {
  return wvcdm::WvContentDecryptionModule::IsSupported(initDataType.c_str());
}

Return<void> WVDrmFactory::createPlugin(
    const hidl_array<uint8_t, 16>& uuid,
    const hidl_string& appPackageName,
    createPlugin_cb _hidl_cb) {

  WVDrmPlugin *plugin = NULL;
  if (!isCryptoSchemeSupported(uuid.data())) {
    ALOGE("Widevine Drm HAL: failed to create drm plugin, " \
        "invalid crypto scheme");
    _hidl_cb(Status::ERROR_DRM_CANNOT_HANDLE, plugin);
    return Void();
  }

  plugin = new WVDrmPlugin(getCDM(), appPackageName.c_str(),
                           &sOemCryptoInterface, areSpoidsEnabled());
  _hidl_cb(Status::OK, plugin);
  return Void();
}

bool WVDrmFactory::areSpoidsEnabled() {
  // Check what this device's first API level was.
  int32_t firstApiLevel = property_get_int32("ro.product.first_api_level", 0);
  if (firstApiLevel == 0) {
    // First API Level is 0 on factory ROMs, but we can assume the current SDK
    // version is the first if it's a factory ROM.
    firstApiLevel = property_get_int32("ro.build.version.sdk", 0);
  }
  return firstApiLevel >= 26;  // Android O
}


}  // namespace widevine
}  // namespace V1_1
}  // namespace drm
}  // namespace hardware
}  // namespace wvdrm
