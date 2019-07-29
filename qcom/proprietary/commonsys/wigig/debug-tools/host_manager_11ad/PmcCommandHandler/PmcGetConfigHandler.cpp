/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "PmcGetConfigHandler.h"
#include "Host.h"
#include "PmcService.h"
#include "PmcSequence.h"

void PmcGetConfigHandler::HandleRequest(const PmcGetConfigRequest& jsonRequest, PmcGetConfigResponse& jsonResponse)
{
    LOG_DEBUG << "PMC configuration request for Device: " << jsonRequest.GetDeviceName() << std::endl;
    const std::string& deviceName = jsonRequest.GetDeviceName();
    BasebandType type;
    auto nameTypeValidRes = PmcSequence::ValidateDeviceNameAndType(deviceName, type);
    if (!nameTypeValidRes.IsSuccess())
    {
        jsonResponse.Fail(nameTypeValidRes.GetStatusMessage());
        return;
    }
    auto configuration = PmcService::GetInstance().GetPmcDeviceContext(deviceName).GetDeviceConfigurations();
    LOG_DEBUG << "Getting configurations. " << configuration << std::endl;
    jsonResponse.SetCollectIdleSmEvents(configuration.IsCollectIdleSmEvents());
    jsonResponse.SetCollectRxPpduEvents(configuration.IsCollectRxPpduEvents());
    jsonResponse.SetCollectTxPpduEvents(configuration.IsCollectTxPpduEvents());
    jsonResponse.SetCollectUCodeEvents(configuration.IsCollectUcodeEvents());
}

