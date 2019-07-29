//
// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.
//

#ifndef WV_DRM_FACTORY_H_
#define WV_DRM_FACTORY_H_

#include <android/hardware/drm/1.1/IDrmFactory.h>
#include <android/hardware/drm/1.1/IDrmPlugin.h>

#include "WVGenericCryptoInterface.h"
#include "WVTypes.h"

namespace wvdrm {
namespace hardware {
namespace drm {
namespace V1_1 {
namespace widevine {

using ::android::hardware::drm::V1_1::IDrmFactory;
using ::android::hardware::drm::V1_1::IDrmPlugin;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_string;
using ::android::hardware::Return;

struct WVDrmFactory : public IDrmFactory {
  WVDrmFactory() {}
  virtual ~WVDrmFactory() {}

  Return<bool> isCryptoSchemeSupported(const hidl_array<uint8_t, 16>& uuid)
      override;

  Return<bool> isContentTypeSupported(const hidl_string &mimeType)
      override;

  Return<void> createPlugin(
      const hidl_array<uint8_t, 16>& uuid,
      const hidl_string& appPackageName,
      createPlugin_cb _hidl_cb) override;

 private:
  WVDRM_DISALLOW_COPY_AND_ASSIGN(WVDrmFactory);

  static WVGenericCryptoInterface sOemCryptoInterface;

  static bool areSpoidsEnabled();

  friend class WVDrmFactoryTest_CalculatesSpoidUseCorrectly_Test;
};

extern "C" IDrmFactory* HIDL_FETCH_IDrmFactory(const char* name);

}  // namespace widevine
}  // namespace V1_1
}  // namespace drm
}  // namespace hardware
}  // namespace wvdrm

#endif // WV_DRM_FACTORY_H_
