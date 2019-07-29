/*
 * Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include <vendor/qti/hardware/alarm/1.0/IAlarm.h>
#include <hidl/LegacySupport.h>
#ifdef ARCH_ARM_32
#include <hwbinder/ProcessState.h>
#endif

#include "Alarm.h"

using vendor::qti::hardware::alarm::V1_0::IAlarm;
using android::hardware::defaultPassthroughServiceImplementation;

int main() {
    #ifdef ARCH_ARM_32
        android::hardware::ProcessState::initWithMmapSize((size_t)8192);
    #endif
    return defaultPassthroughServiceImplementation<IAlarm>();
}
