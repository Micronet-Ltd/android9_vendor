/*******************************************************************************

 Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.

 ---------------------------------------------------------------------------

 @file    qti-activity_trigger.cpp
 @brief   Android activity trigger library
 DESCRIPTION

 ---------------------------------------------------------------------------
*******************************************************************************/

#define LOG_TAG "ANDR-PERF-ACT-TRIG"
#include <cutils/log.h>
#include <cutils/properties.h>
#include <string.h>

#include "mp-ctl.h"
#include "client.h"

#define DEBUG_LOGS_PROPERTY "persist.vendor.qti.debug.atlogs_enable"
static bool sEnableGTLogs = false;

///////////////////////////////////////////////////////////////////////////////////
///activity trigger apis
///////////////////////////////////////////////////////////////////////////////////
extern "C" void activity_trigger_init(void) {
    char buf[PROPERTY_VALUE_MAX] = {0};

    sEnableGTLogs = false;
    if((property_get(DEBUG_LOGS_PROPERTY, buf, NULL) > 0) &&
       (!strncmp(buf, "1", PROPERTY_VALUE_MAX ) ||
        !strncasecmp(buf,"true", PROPERTY_VALUE_MAX))) {
        sEnableGTLogs = true;
    }
    ALOGD_IF(sEnableGTLogs, "Activity trigger library initialized successfully");
}

extern "C" void activity_trigger_start(const char *name, int *reserved) {
    ALOGD_IF(sEnableGTLogs, "App trigger starting '%s'", (name) ? name : "<null>");

    if (!name)
        return;

    ALOGD_IF(sEnableGTLogs,"perf:calling NotifyFgAppChange");
    perf_hint(VENDOR_HINT_ACTIVITY_START, name, -1, -1);
}

extern "C" void activity_trigger_resume(const char *name) {

    /* Do the same as activity start */
    if (!name)
        return;

    perf_hint(VENDOR_HINT_ACTIVITY_RESUME, name, -1, -1);
}

extern "C" void activity_trigger_pause(const char *name) {
    /* Do the same as activity start */
    if (!name)
        return;

    perf_hint(VENDOR_HINT_ACTIVITY_PAUSE, name, -1, -1);
}

extern "C" void activity_trigger_stop(const char *name) {
    /* Do the same as activity start */
    if (!name)
        return;

    perf_hint(VENDOR_HINT_ACTIVITY_STOP, name, -1, -1);
}

extern "C" void activity_trigger_misc(int function, const char *name, int type, int flag, float *scale) {
    //do nothing
}

extern "C" void activity_trigger_deinit(void) {
}
