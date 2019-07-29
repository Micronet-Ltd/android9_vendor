/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "PmcStartHandler.h"
#include "PmcSequence.h"

void PmcStartHandler::HandleRequest(const PmcStartRequest& jsonRequest, PmcStartResponse& jsonResponse)
{
    const std::string& deviceName = jsonRequest.GetDeviceName();
    LOG_DEBUG << "PMC start request for Device: " << deviceName << std::endl;
    BasebandType type;
    auto nameTypeValidRes = PmcSequence::ValidateDeviceNameAndType(deviceName, type);
    if(!nameTypeValidRes.IsSuccess())
    {
        jsonResponse.Fail(nameTypeValidRes.GetStatusMessage());
        return;
    }
    // Check if PMC is already running
    bool recActive;
    auto res = PmcSequence::IsPmcRecordingActive(deviceName,recActive);
    if(!res.IsSuccess())
    {
        jsonResponse.Fail(res.GetStatusMessage());
        return;
    }
    if(recActive)
    {
        jsonResponse.Fail("Recording already active");
        return;
    }
    auto activateRes = PmcSequence::ActivateRecording(deviceName);
    if(!activateRes.IsSuccess())
    {
        jsonResponse.Fail(activateRes.GetStatusMessage());
        return;
    }
}

