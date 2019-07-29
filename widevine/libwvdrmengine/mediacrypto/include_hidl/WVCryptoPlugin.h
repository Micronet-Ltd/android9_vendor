//
// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.
//

#ifndef WV_CRYPTO_PLUGIN_H_
#define WV_CRYPTO_PLUGIN_H_

#include <android/hardware/drm/1.0/ICryptoPlugin.h>
#include <android/hidl/memory/1.0/IMemory.h>

#include "wv_content_decryption_module.h"
#include "WVTypes.h"

namespace wvdrm {
namespace hardware {
namespace drm {
namespace V1_1 {
namespace widevine {

using ::android::hardware::drm::V1_0::DestinationBuffer;
using ::android::hardware::drm::V1_0::ICryptoPlugin;
using ::android::hardware::drm::V1_0::Mode;
using ::android::hardware::drm::V1_0::Pattern;
using ::android::hardware::drm::V1_0::SharedBuffer;
using ::android::hardware::drm::V1_0::Status;
using ::android::hardware::drm::V1_0::SubSample;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::hidl_memory;
using ::android::hidl::memory::V1_0::IMemory;
using ::android::sp;

struct WVCryptoPlugin : public ICryptoPlugin {
  WVCryptoPlugin(const void* data, size_t size,
                 const sp<wvcdm::WvContentDecryptionModule>& cdm);
  virtual ~WVCryptoPlugin() {}

  Return<bool> requiresSecureDecoderComponent(const hidl_string& mime)
      override;

  Return<void> notifyResolution(uint32_t width, uint32_t height) override;

  Return<Status> setMediaDrmSession(const hidl_vec<uint8_t>& sessionId)
      override;

  Return<void> setSharedBufferBase(const hidl_memory& base,
                                   uint32_t bufferId) override;

  Return<void> decrypt(
      bool secure,
      const hidl_array<uint8_t, 16>& keyId,
      const hidl_array<uint8_t, 16>& iv,
      Mode mode,
      const Pattern& pattern,
      const hidl_vec<SubSample>& subSamples,
      const SharedBuffer& source,
      uint64_t offset,
      const DestinationBuffer& destination,
      decrypt_cb _hidl_cb) override;

 private:
  WVDRM_DISALLOW_COPY_AND_ASSIGN_AND_NEW(WVCryptoPlugin);

  wvcdm::CdmSessionId mSessionId;
  std::map<uint32_t, sp<IMemory> > mSharedBufferMap;

  sp<wvcdm::WvContentDecryptionModule> const mCDM;

  Status attemptDecrypt(
      const wvcdm::CdmDecryptionParameters& params,
      bool haveEncryptedSubsamples, std::string* errorDetailMsg);
  static wvcdm::CdmResponseType countEncryptedBlocksInPatternedRange(
      size_t range, const Pattern& pattern, uint64_t* result);
  static void incrementIV(uint64_t increaseBy, std::vector<uint8_t>* ivPtr);
};

}  // namespace widevine
}  // namespace V1_1
}  // namespace drm
}  // namespace hardware
}  // namespace wvdrm

#endif // WV_CRYPTO_PLUGIN_H_
