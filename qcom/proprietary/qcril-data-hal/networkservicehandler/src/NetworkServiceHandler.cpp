/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <framework/Log.h>
#include "NetworkServiceHandler.h"
#include "BearerAllocationUpdateMessage.h"

namespace rildata {

/**
 * Toggle whether to send bearer allocation updates via HAL
 */
ResponseError_t NetworkServiceHandler::handleToggleBearerAllocationUpdate(bool enable)
{
    Log::getInstance().d("[NetworkServiceHandler]: handleToggleBearerAllocationUpdate = "+std::to_string((int)enable));
    toggleBearerUpdateToDataConnectionService = enable;
    updateAllBearerAllocations();

    return ResponseError_t::NO_ERROR;
}

/**
 * handle modem bearer type update indication
 */
void NetworkServiceHandler::handleDataBearerTypeUpdate(int32_t cid, rildata::BearerInfo_t bearer)
{
    Log::getInstance().d("[NetworkServiceHandler]: handleDataBearerTypeChanged for cid = "
        +std::to_string((int)cid)
        +", bid = "
        +std::to_string((int)bearer.bearerId));

    std::map<int32_t, rildata::AllocatedBearer_t>::iterator it;
    it = bearerCollection.find(cid);
    if (it != bearerCollection.end()) {
        std::vector<rildata::BearerInfo_t>::iterator i;
        for (i=(it->second).bearers.begin() ; i!=(it->second).bearers.end() ; ++i)
        {
            if(i->bearerId == bearer.bearerId) {
                if ((i->uplink != bearer.uplink) || (i->downlink != bearer.downlink)) {
                    i->uplink = bearer.uplink;
                    i->downlink = bearer.downlink;
                }
                break;
            }
        }
        if (i==(it->second).bearers.end()) {
            // new bearer
            (it->second).bearers.push_back(bearer);
        }
        updateAllBearerAllocations();
    }
    else {
        Log::getInstance().d("[NetworkServiceHandler]: no call for cid = "+std::to_string((int)cid));
    }
}

/**
 * handle modem bearer type update indication
 */
void NetworkServiceHandler::handleDataAllBearerTypeUpdate(rildata::AllocatedBearer_t bearer)
{
    // update all bearer with AllocatedBearer_t bearer
    std::map<int32_t, rildata::AllocatedBearer_t>::iterator it;
    it = bearerCollection.find(bearer.cid);
    if (it != bearerCollection.end()) {
        // existing call id, update bearers
        (it->second).bearers.clear();
        (it->second).bearers.insert((it->second).bearers.end(), bearer.bearers.begin(), bearer.bearers.end());
    }
    else {
        // new call id
        bearerCollection.insert(std::pair<int32_t, AllocatedBearer_t>(bearer.cid, bearer));
    }
    updateAllBearerAllocations();
}

/**
 * handle get bearers for one data connection with cid
 */
AllocatedBearerResult_t NetworkServiceHandler::handleGetBearerAllocation(int32_t cid)
{
    Log::getInstance().d("[NetworkServiceHandler]: handleGetBearerAllocation = "+std::to_string((int)cid));

    AllocatedBearerResult_t result;
    std::map<int32_t, rildata::AllocatedBearer_t>::iterator it;
    it = bearerCollection.find(cid);
    if (it != bearerCollection.end()) {
        result.error = ResponseError_t::NO_ERROR;
        result.connections.push_back(it->second);
    }
    else {
        result.error = ResponseError_t::CALL_NOT_AVAILABLE;
    }

    return result;
}

/**
 * handle get all bearer information request
 */
AllocatedBearerResult_t NetworkServiceHandler::handleGetAllBearerAllocations()
{
    Log::getInstance().d("[NetworkServiceHandler]: handleGetAllBearerAllocations");

    AllocatedBearerResult_t result;
    for (std::map<int32_t, rildata::AllocatedBearer_t>::iterator it=bearerCollection.begin(); it!=bearerCollection.end(); ++it) {
        result.connections.push_back(it->second);
    }
    result.error = ResponseError_t::NO_ERROR;

    return result;
}

/**
 * Updates all bearer type info.
 *
 * If there is at least one client registered for bearer allocation updates
 * through IDataConnection HAL, the bearer list will be sent to the HAL client.
 */
void NetworkServiceHandler::updateAllBearerAllocations()
{
    Log::getInstance().d("[NetworkServiceHandler]: updateAllBearerAllocations");
    if(toggleBearerUpdateToDataConnectionService) {
        // Update to IDataConnnection HAL
        AllocatedBearerResult_t localBearers;
        localBearers.error = ResponseError_t::NO_ERROR;
        for (auto it = bearerCollection.begin(); it != bearerCollection.end(); ++it) {
            localBearers.connections.push_back(it->second);
        }
        std::shared_ptr<AllocatedBearerResult_t> bearers = std::make_shared<AllocatedBearerResult_t>(localBearers);
        auto msg = std::make_shared<BearerAllocationUpdateMessage>(bearers);
        msg->broadcast();
    }
}

/**
 * clean bearer type in the call
 */
void NetworkServiceHandler::cleanUpBearerAllocation(int32_t cid)
{
    Log::getInstance().d("[NetworkServiceHandler]: cleanUpBearerAllocation for cid = "+std::to_string((int)cid));
    std::map<int32_t, rildata::AllocatedBearer_t>::iterator it;
    it = bearerCollection.find(cid);
    if (it != bearerCollection.end()) {
        bearerCollection.erase(it);
    }
    updateAllBearerAllocations();
}

/**
 * clean all bearer type info and update
 */
void NetworkServiceHandler::cleanUpAllBearerAllocation()
{
    Log::getInstance().d("[NetworkServiceHandler]: cleanUpAllBearerAllocation");
    bearerCollection.clear();
    updateAllBearerAllocations();
}

}