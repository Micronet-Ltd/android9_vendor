/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

#include <hidl/LegacySupport.h>
#include <android/hardware/gnss/1.1/IGnss.h>

#ifdef ARCH_ARM_32
#define DEFAULT_HW_BINDER_MEM_SIZE 65536
#endif

using android::hardware::configureRpcThreadpool;
using android::hardware::registerPassthroughServiceImplementation;
using android::hardware::joinRpcThreadpool;

using android::status_t;
using android::OK;

using vendor::qti::gnss::LOC_HIDL_VERSION::ILocHidlGnss;

int main(int /* argc */, char* /* argv */ []) {

    ALOGI("%s", __FUNCTION__);

#ifdef ARCH_ARM_32
    android::hardware::ProcessState::initWithMmapSize((size_t)(DEFAULT_HW_BINDER_MEM_SIZE));
#endif
    configureRpcThreadpool(1, true);
    status_t status;

    status = registerPassthroughServiceImplementation<android::hardware::gnss::V1_1::IGnss>();
    if (status != OK) {
        ALOGE("Error while registering IGnss 1.1 service: %d, try IGnss 1.0", status);
        status = registerPassthroughServiceImplementation<android::hardware::gnss::V1_0::IGnss>();
        if (status != OK) {
            ALOGE("Error while registering IGnss 1.0 service: %d", status);
            return -1;
        }
    }

    status = registerPassthroughServiceImplementation<ILocHidlGnss>("gnss_vendor");
    if (status != OK) {
        ALOGE("Error while registering gnss vendor hal service: %d", status);
        return -1;
    }

    joinRpcThreadpool();
    return 0;
}
