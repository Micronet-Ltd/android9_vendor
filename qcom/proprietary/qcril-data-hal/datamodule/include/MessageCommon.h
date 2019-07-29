/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#include <string>
#include <vector>
#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"

#pragma once

using namespace std;

namespace rildata {

enum ResponseError_t {
    NO_ERROR = 0,
    NOT_SUPPORTED,
    CALL_NOT_AVAILABLE,
    INTERNAL_ERROR,
    INVALID_ARGUMENT,
};

enum RatType_t {
    RAT_UNSPECIFIED = 0,
    RAT_4G,
    RAT_5G,
    RAT_SPLITED,
};

enum class NrIconEnum_t : int {
    NONE = 0,
    BASIC,
    UWB
};

struct BearerInfo_t {
    int32_t bearerId;
    RatType_t uplink;
    RatType_t downlink;
};

struct AllocatedBearer_t {
    int32_t cid;
    std::string apn;
    std::string ifaceName;
    std::vector<BearerInfo_t> bearers;
};

struct AllocatedBearerResult_t {
    ResponseError_t error;
    std::vector<AllocatedBearer_t> connections;
};

class NrIconType_t {
public:
    NrIconType_t(NrIconEnum_t setIcon): icon(setIcon) {}
    bool isUwb() { return icon == NrIconEnum_t::UWB; }
    bool isBasic() { return icon == NrIconEnum_t::BASIC; }
    bool isNone() { return icon == NrIconEnum_t::NONE; }

private:
    NrIconEnum_t icon;
};

}
