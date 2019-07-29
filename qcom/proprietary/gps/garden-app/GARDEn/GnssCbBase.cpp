/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#include <ContextBase.h>
#include "GardenUtil.h"
#include "GnssCbBase.h"

namespace garden {

void GnssCbBase::gnssLocationCb(Location& location) {
    gardenPrint("## %s]: () ##, LAT: %f, LON: %f, ALT: %f ACC: %f, TIME: %llu",
             __FUNCTION__, location.latitude, location.longitude, location.altitude,
            location.accuracy, (long long)location.timestamp);
}

void GnssCbBase::gnssStatusCb(uint32_t status) {
    gardenPrint("## %s]: () GPS Status: %d ##", __FUNCTION__, status);
}

void GnssCbBase::gnssSvStatusCb(GnssSvNotification& svNotify) {
    gardenPrint ("## %s]: () Number of SVs: %d ##", __FUNCTION__, svNotify.count);
}

void GnssCbBase::gnssNmeaCb(uint64_t /*timestamp*/, const char* /*nmea*/, int /*length*/) {}

void GnssCbBase::gnssSetCapabilitiesCb(uint32_t capabilities) {
    gardenPrint("## %s]: () ##, Capabilities: 0x%x", __FUNCTION__, capabilities);
}

void GnssCbBase::gnssRequestUtcTimeCb() {
    gardenPrint("## %s]: () ##", __FUNCTION__);
}

void GnssCbBase::gnssNiNotifyCb(uint32_t id, GnssNiNotification& notification) {
    gardenPrint ("## %s]: () ##, ACTION_NI_NOTIFY: notification_id %d, ni_type %d, "
            "notify_flags %d, timeout %d, default_response %d, "
            "requestor_id %s, text %s, requestor_id_encoding %d, "
            "text_encoding %d, extras %s, esEnabled %d\n",
            __FUNCTION__, id, (int) notification.type, (int) notification.options,
            notification.timeout, notification.timeoutResponse, notification.requestor,
            notification.message, notification.requestorEncoding,
            notification.messageEncoding, notification.extras,
            loc_core::ContextBase::mGps_conf.SUPL_ES);
}
} // namespace garden
