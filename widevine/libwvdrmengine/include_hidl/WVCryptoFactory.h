//
// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.
//

#ifndef WV_CRYPTO_FACTORY_H_
#define WV_CRYPTO_FACTORY_H_

#include <android/hardware/drm/1.1/ICryptoFactory.h>

#include "WVTypes.h"

namespace wvdrm {
namespace hardware {
namespace drm {
namespace V1_1 {
namespace widevine {

using ::android::hardware::drm::V1_1::ICryptoFactory;
using ::android::hardware::drm::V1_0::ICryptoPlugin;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;

struct WVCryptoFactory : public ICryptoFactory {
 public:
  WVCryptoFactory() {}
  virtual ~WVCryptoFactory() {}

  Return<bool> isCryptoSchemeSupported(const hidl_array<uint8_t, 16>& uuid)
      override;

  Return<void> createPlugin(
      const hidl_array<uint8_t, 16>& uuid,
      const hidl_vec<uint8_t>& initData,
      createPlugin_cb _hidl_cb) override;

 private:
  WVDRM_DISALLOW_COPY_AND_ASSIGN(WVCryptoFactory);
};

}  // namespace widevine
}  // namespace V1_1
}  // namespace drm
}  // namespace hardware
}  // namespace wvdrm

#endif // WV_CRYPTO_FACTORY_H_
