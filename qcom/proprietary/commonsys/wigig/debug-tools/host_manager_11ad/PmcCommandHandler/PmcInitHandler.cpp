/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "PmcInitHandler.h"
#include "PmcSequence.h"

void PmcInitHandler::HandleRequest(const PmcInitRequest& jsonRequest, PmcInitResponse& jsonResponse)
{
    const std::string& deviceName = jsonRequest.GetDeviceName();
    LOG_DEBUG << "PMC init request for Device: " << deviceName << std::endl;
    BasebandType type;
    auto nameTypeValidRes = PmcSequence::ValidateDeviceNameAndType(deviceName, type);
    if(!nameTypeValidRes.IsSuccess())
    {
        jsonResponse.Fail(nameTypeValidRes.GetStatusMessage());
        return;
    }
    // It's better to make sure the PMC is initialized to recover from possible errors.
    auto shutdownst = PmcSequence::Shutdown(deviceName);
    if (!shutdownst.IsSuccess())
    {
        jsonResponse.Fail(shutdownst.GetStatusMessage());
        return;
    }

    LOG_INFO << "Initializing for PMC Recording on " << deviceName << std::endl;

    auto enablest = PmcSequence::EnablePmcClock(deviceName);
    if (!enablest.IsSuccess())
    {
        jsonResponse.Fail(enablest.GetStatusMessage());
        return;
    }

    auto allocatest = PmcSequence::AllocatePmcRing(deviceName);
    if (!allocatest.IsSuccess())
    {
        jsonResponse.Fail(allocatest.GetStatusMessage());
        return;
    }

    auto disablest = PmcSequence::DisablePrpClockGating(deviceName);
    if (!disablest.IsSuccess())
    {
        jsonResponse.Fail(disablest.GetStatusMessage());
        return;
    }
}

