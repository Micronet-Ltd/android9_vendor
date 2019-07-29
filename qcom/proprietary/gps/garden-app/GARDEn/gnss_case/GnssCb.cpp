/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#include "GardenUtil.h"
#include "GnssCb.h"

namespace garden {
GnssCb::GnssCb(GnssCase* gCase): mNiCount(0), mNetworkInitiatedResponse({}) {
    mGnssCase = gCase;
}

GnssCb::~GnssCb() {
    if(mGnssCase) {
        mGnssCase = nullptr;
    }
}

void GnssCb::gnssLocationCb(Location& location) {
    GnssCbBase::gnssLocationCb(location);
    if(mGnssCase && mGnssCase->mStartTime != 0 && mGnssCase->mTTFF == 0) {
        uint64_t now = getUpTimeSec();
        if (now > mGnssCase->mStartTime) {
            mGnssCase->mTTFF = now - mGnssCase->mStartTime;
            gardenPrint("TTFF: %" PRIu64, mGnssCase->mTTFF);
        }
    }
    if (mGnssCase && mGnssCase->hasFlag(GNSS_CASE_SESSION) &&
            mGnssCase->mRecurrence == LOC_GPS_POSITION_RECURRENCE_SINGLE) {
        AutoLock lock(&mGnssCase->mLock);
        mGnssCase->unsetFlag(GNSS_CASE_SESSION);
    }
}

void GnssCb::gnssStatusCb(uint32_t status) {
    GnssCbBase::gnssStatusCb(status);
    if(status == LOC_GPS_STATUS_ENGINE_OFF) {
        AutoLock lock(&mGnssCase->mLock);
        mGnssCase->unsetFlag(GNSS_CASE_ENGINEOFF);
    }
}

void GnssCb::gnssSvStatusCb(GnssSvNotification& svNotify) {
    GnssCbBase::gnssSvStatusCb(svNotify);
    if(mGnssCase->mSvDetails) {
        for(size_t i = 0; i < svNotify.count; ++i) {
            gardenPrint("%02d : SVID: %04d, CNR: %09.4f, "
                    "ELE: %09.4f, AZI: %09.4f , FREQ: %9.0f, "
                    "TYPE: %u EPH: %u ALM: %u USED: %u FRE: %u",
                    i+1, svNotify.gnssSvs[i].svId, svNotify.gnssSvs[i].cN0Dbhz,
                    svNotify.gnssSvs[i].elevation, svNotify.gnssSvs[i].azimuth,
                    svNotify.gnssSvs[i].carrierFrequencyHz,
                    svNotify.gnssSvs[i].type,
                    (svNotify.gnssSvs[i].gnssSvOptionsMask & GNSS_SV_OPTIONS_HAS_EPHEMER_BIT) ==
                    GNSS_SV_OPTIONS_HAS_EPHEMER_BIT,
                    (svNotify.gnssSvs[i].gnssSvOptionsMask & GNSS_SV_OPTIONS_HAS_ALMANAC_BIT) ==
                    GNSS_SV_OPTIONS_HAS_ALMANAC_BIT,
                    (svNotify.gnssSvs[i].gnssSvOptionsMask & GNSS_SV_OPTIONS_USED_IN_FIX_BIT) ==
                    GNSS_SV_OPTIONS_USED_IN_FIX_BIT,
                    (svNotify.gnssSvs[i].gnssSvOptionsMask & GNSS_SV_OPTIONS_HAS_CARRIER_FREQUENCY_BIT) ==
                    GNSS_SV_OPTIONS_HAS_CARRIER_FREQUENCY_BIT);
        }
    }

    // if the minimum number of SVs with minimum number of SNR
    // has been satisfied then stop session
    if(mGnssCase->hasFlag(GNSS_CASE_SESSION) && mGnssCase->mMinSvs && mGnssCase->mMinSnr) {
        if(mGnssCase->mMinSvs <= svNotify.count) {
            size_t minCnSvCount = 0;
            for(size_t i = 0; i < svNotify.count; ++i) {
                if(svNotify.gnssSvs[i].cN0Dbhz >= mGnssCase->mMinSnr) {
                    ++minCnSvCount;
                }
            }
            if(minCnSvCount >= mGnssCase->mMinSvs) {
                gardenPrint("Stop test, as %zu SVs are seen with at least a SNR of %f",
                        mGnssCase->mMinSvs, mGnssCase->mMinSnr);
                AutoLock lock(&mGnssCase->mLock);
                mGnssCase->unsetFlag(GNSS_CASE_SESSION);
            }
        }
    }
}

void GnssCb::gnssNmeaCb(uint64_t timestamp, const char* nmea, int length) {
    if(mGnssCase->mPrintNmea) {
        gardenPrint ("## %s]: () ##:: Timestamp: %" PRIu64 " NMEA string Length: %d",
                __FUNCTION__, timestamp, length);
        if(length>0) {
            gardenPrint ("## %s]: () ##:: NMEA String: %s", __FUNCTION__, nmea);
        }
    }
}

void GnssCb::gnssNiNotifyCb(uint32_t id, GnssNiNotification& notification) {
    static int sNiCount = 1;
    GnssCbBase::gnssNiNotifyCb(id, notification);

    if(sNiCount <= mNiCount) {
        static int pc = 0;
        pc = pc % mNetworkInitiatedResponse.size();
        Runnable runnable = [this, id]() {
            if(mNetworkInitiatedResponse[pc] != 3) {
                mGnssCase->getGnssAPI()->gnssNiResponse(id,
                        (GnssNiResponse)mNetworkInitiatedResponse[pc]);
            }
            ++pc;
        };
        mGnssCase->sendMsg(runnable);
    }
    if(mNiCount == sNiCount) {
        AutoLock lock(&mGnssCase->mLock);
        mGnssCase->unsetFlag(GNSS_CASE_NIBACKTOBACK);
    }
    ++sNiCount;
}

void GnssCb::setNiParam(int niCount, int niResPatCount, int* networkInitiatedResponse) {
    mNiCount = niCount;
    for(int i = 0; i < niResPatCount; ++i) {
        mNetworkInitiatedResponse.push_back(networkInitiatedResponse[i]);
    }
}
} // namespace garden
