/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "PmcPauseHandler.h"
#include "PmcSequence.h"

void PmcPauseHandler::HandleRequest(const PmcPauseRequest& jsonRequest, PmcPauseResponse& jsonResponse)
{
    const std::string& deviceName = jsonRequest.GetDeviceName();
    LOG_DEBUG << "PMC pause request for Device: " << deviceName << std::endl;
    BasebandType type;
    auto nameTypeValidRes = PmcSequence::ValidateDeviceNameAndType(deviceName, type);
    if (!nameTypeValidRes.IsSuccess())
    {
        jsonResponse.Fail(nameTypeValidRes.GetStatusMessage());
        return;
    }
    auto res = PmcSequence::PausePmcRecording(deviceName);
    if(!res.IsSuccess())
    {
        jsonResponse.Fail(res.GetStatusMessage());
        return;
    }
}
