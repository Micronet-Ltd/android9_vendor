/*
 * Copyright 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define LOG_TAG "android.hardware.drm@1.0-service.widevine"

#include <WVCryptoFactory.h>
#include <WVDrmFactory.h>

#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>

#include <binder/ProcessState.h>

using ::android::hardware::configureRpcThreadpool;
using ::android::hardware::joinRpcThreadpool;
using ::android::sp;

using android::hardware::drm::V1_1::ICryptoFactory;
using android::hardware::drm::V1_1::IDrmFactory;
using wvdrm::hardware::drm::V1_1::widevine::WVCryptoFactory;
using wvdrm::hardware::drm::V1_1::widevine::WVDrmFactory;

int main(int /* argc */, char** /* argv */) {
    ALOGD("android.hardware.drm@1.1-service.widevine starting...");

    // The DRM HAL may communicate to other vendor components via
    // /dev/vndbinder
    android::ProcessState::initWithDriver("/dev/vndbinder");

    sp<IDrmFactory> drmFactory = new WVDrmFactory;
    sp<ICryptoFactory> cryptoFactory = new WVCryptoFactory;

    configureRpcThreadpool(8, true /* callerWillJoin */);

    // Setup hwbinder service
    CHECK_EQ(drmFactory->registerAsService("widevine"), android::NO_ERROR)
        << "Failed to register Widevine Factory HAL";
    CHECK_EQ(cryptoFactory->registerAsService("widevine"), android::NO_ERROR)
        << "Failed to register Widevine Crypto  HAL";

    joinRpcThreadpool();
}
