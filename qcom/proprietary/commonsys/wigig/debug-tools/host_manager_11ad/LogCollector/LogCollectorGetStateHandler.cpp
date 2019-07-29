/*
* Copyright (c)  2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "LogCollectorGetStateHandler.h"
#include "LogCollector.h"
#include "Host.h"

void LogCollectorGetStateHandler::HandleRequest(const LogCollectorGetStateRequest& jsonRequest, LogCollectorGetStateResponse& jsonResponse)
{
    const std::string& deviceName = jsonRequest.GetDeviceName();

    const JsonValueBoxed<CpuType> cpuType = jsonRequest.GetCpuType();
    if (cpuType.GetState() == JSON_VALUE_MISSING)
    {
        jsonResponse.Fail("CpuType field is missing");
        return;
    }
    if (cpuType.GetState() == JSON_VALUE_MALFORMED)
    {
        jsonResponse.Fail("CpuType is wrong, it should be 'fw' or 'ucode'");
        return;
    }

    LOG_DEBUG << "Log Collector get state request for Device: " << jsonRequest.GetDeviceName() << " with CPU type: " << (CPU_TYPE_TO_STRING.find(cpuType))->second << std::endl;

    bool isRecordig;
    OperationStatus os = Host::GetHost().GetDeviceManager().GetLogRecordingState(deviceName, cpuType, isRecordig);
    if (!os)
    {
        jsonResponse.Fail(os.GetStatusMessage());
        return;
    }

    bool isPublishing;
    os = Host::GetHost().GetDeviceManager().GetLogPublishingState(deviceName, cpuType, isPublishing);
    if (!os)
    {
        jsonResponse.Fail(os.GetStatusMessage());
        return;
    }

    jsonResponse.SetIsRecording(isRecordig);
    jsonResponse.SetIsPublishing(isPublishing);
}

