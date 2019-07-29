/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <string>
#include <vector>
#include <map>
#include "MessageCommon.h"

namespace rildata {
class NetworkServiceHandler {
private:
    std::map<int32_t, rildata::AllocatedBearer_t> bearerCollection;
    bool toggleBearerUpdateToDataConnectionService;
public:
    NetworkServiceHandler(){
        toggleBearerUpdateToDataConnectionService = false;
    };
    ~NetworkServiceHandler(){}
    ResponseError_t handleToggleBearerAllocationUpdate(bool enable);
    void handleDataBearerTypeUpdate(int32_t cid, rildata::BearerInfo_t bearer);
    void handleDataAllBearerTypeUpdate(rildata::AllocatedBearer_t bearer);
    AllocatedBearerResult_t handleGetBearerAllocation(int32_t cid);
    AllocatedBearerResult_t handleGetAllBearerAllocations();
    void updateAllBearerAllocations();
    void cleanUpBearerAllocation(int32_t cid);
    void cleanUpAllBearerAllocation();
};

}