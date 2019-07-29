/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#define LOG_TAG "LocSvc_HIDL_vendor.qti.gnss@1.1-service"

#include <android/hardware/gnss/1.1/IGnss.h>
#include <vendor/qti/gnss/1.1/ILocHidlGnss.h>

#include <hidl/LegacySupport.h>

#ifdef ARCH_ARM_32
#define DEFAULT_HW_BINDER_MEM_SIZE 65536
#endif

using android::hardware::configureRpcThreadpool;
using android::hardware::registerPassthroughServiceImplementation;
using android::hardware::joinRpcThreadpool;

using android::status_t;
using android::OK;

using vendor::qti::gnss::V1_1::ILocHidlGnss;

int main(int /* argc */, char* /* argv */ []) {

    ALOGI("%s", __FUNCTION__);

#ifdef ARCH_ARM_32
    android::hardware::ProcessState::initWithMmapSize((size_t)(DEFAULT_HW_BINDER_MEM_SIZE));
#endif
    configureRpcThreadpool(1, true);
    status_t status;

    status = registerPassthroughServiceImplementation<android::hardware::gnss::V1_1::IGnss>();
    if (status != OK) {
        ALOGW("Failed to register IGnss 1.1 service: %d, try IGnss 1.0", status);
        status = registerPassthroughServiceImplementation<android::hardware::gnss::V1_0::IGnss>();
        if (status != OK) {
            ALOGE("Error while registering IGnss 1.0 service: %d", status);
            return -1;
        }
    }

    status = registerPassthroughServiceImplementation<ILocHidlGnss>("gnss_vendor");
    // LOG_ALWAYS_FATAL_IF(status != OK, "Error while registering gnss vendor hal service: %d", status);
    if (status != OK) {
        ALOGE("Error while registering gnss vendor hal service: %d", status);
        return -1;
    }

    joinRpcThreadpool();
    return 0;
}
