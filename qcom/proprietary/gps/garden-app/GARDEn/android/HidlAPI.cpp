/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#include "GardenUtil.h"
#include "HidlAPI.h"

#include <android/hardware/gnss/1.0/IGnss.h>
using android::OK;
using android::sp;
using android::wp;
using android::status_t;

using android::hardware::Return;
using android::hardware::Void;
using android::hardware::hidl_vec;
using android::hardware::hidl_death_recipient;
using android::hidl::base::V1_0::IBase;

using android::hardware::gnss::V1_0::IAGnss;
using android::hardware::gnss::V1_0::IAGnssCallback;
using android::hardware::gnss::V1_0::IAGnssCallback;
using android::hardware::gnss::V1_0::IAGnssRil;
using android::hardware::gnss::V1_0::IAGnssRilCallback;
using android::hardware::gnss::V1_0::IGnss;
using android::hardware::gnss::V1_0::IGnssBatching;
using android::hardware::gnss::V1_0::IGnssBatchingCallback;
using android::hardware::gnss::V1_0::IGnssCallback;
using android::hardware::gnss::V1_0::IGnssConfiguration;
using android::hardware::gnss::V1_0::IGnssDebug;
using android::hardware::gnss::V1_0::IGnssGeofenceCallback;
using android::hardware::gnss::V1_0::IGnssGeofencing;
using android::hardware::gnss::V1_0::IGnssMeasurement;
using android::hardware::gnss::V1_0::IGnssMeasurementCallback;
using android::hardware::gnss::V1_0::IGnssNavigationMessage;
using android::hardware::gnss::V1_0::IGnssNavigationMessageCallback;
using android::hardware::gnss::V1_0::IGnssNi;
using android::hardware::gnss::V1_0::IGnssNiCallback;
using android::hardware::gnss::V1_0::IGnssXtra;
using android::hardware::gnss::V1_0::IGnssXtraCallback;

namespace garden {

struct GnssDeathRecipient : virtual public hidl_death_recipient {
    virtual void serviceDied(uint64_t /*cookie*/, const wp<IBase>& /*who*/) override {
        gardenPrint("hidl_death_recipient");
    }
};

sp<GnssDeathRecipient> gnssHalDeathRecipient = nullptr;
sp<IGnss> gnssHal = nullptr;
sp<IGnssXtra> gnssXtraIface = nullptr;
sp<IAGnssRil> agnssRilIface = nullptr;
sp<IGnssGeofencing> gnssGeofencingIface = nullptr;
sp<IAGnss> agnssIface = nullptr;
sp<IGnssBatching> gnssBatchingIface = nullptr;
sp<IGnssDebug> gnssDebugIface = nullptr;
sp<IGnssConfiguration> gnssConfigurationIface = nullptr;
sp<IGnssNi> gnssNiIface = nullptr;
sp<IGnssMeasurement> gnssMeasurementIface = nullptr;
sp<IGnssNavigationMessage> gnssNavigationMessageIface = nullptr;

void HidlAPI::setGnssCbs(GnssCbBase* callbacks) {
    (void)(callbacks);
}

void HidlAPI::setFlpCbs(GnssCbBase* callbacks) {
    (void)(callbacks);
}

void HidlAPI::setGeofenceCbs(GnssCbBase* callbacks) {
    (void)(callbacks);
}

HidlAPI::HidlAPI() {
    gnssHal = IGnss::getService();
    if (gnssHal != nullptr) {
        gardenPrint("get hidl api success.");
    } else {
        gardenPrint("get hidl api failed.");
    }
}

int HidlAPI::gnssStart() {
    return 0;
}

int HidlAPI::gnssStop() {
    return 0;
}

void HidlAPI::gnssCleanup() {
}

int HidlAPI::gnssInjectLocation(double latitude, double longitude, float accuracy) {
    (void)(latitude);
    (void)(longitude);
    (void)(accuracy);
    return 0;
}

void HidlAPI::gnssDeleteAidingData(LocGpsAidingData flags) {
    (void)(flags);
}

int HidlAPI::gnssSetPositionMode(LocGpsPositionMode mode,
        LocGpsPositionRecurrence recurrence, uint32_t min_interval,
        uint32_t preferred_accuracy, uint32_t preferred_time) {
    (void)(mode);
    (void)(recurrence);
    (void)(min_interval);
    (void)(preferred_accuracy);
    (void)(preferred_time);
    return 0;
}

int HidlAPI::flpStartSession(uint32_t /*mode*/, LocationOptions& /*options*/, uint32_t /*id*/) {
    return 0;
}

int HidlAPI::flpStopSession(uint32_t /*id*/) {
    return 0;
}

void HidlAPI::configurationUpdate(const char* config_data, int32_t length) {
    (void)(config_data);
    (void)(length);
}

void HidlAPI::updateNetworkAvailability(int /*available*/, const char* /*apn*/) {
}

void HidlAPI::gnssNiResponse(uint32_t id, GnssNiResponse response) {
    (void)(id);
    (void)(response);
}

} // namespace garden
