/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "LogCollectorSetModeHandler.h"
#include "LogCollector.h"
#include "Host.h"

void LogCollectorSetModeHandler::HandleRequest(const LogCollectorSetModeRequest& jsonRequest, LogCollectorSetModeResponse& jsonResponse)
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

    LOG_DEBUG << "Log Collector set mode request for Device: " << jsonRequest.GetDeviceName() << " with CPU type: " << (CPU_TYPE_TO_STRING.find(cpuType))->second << std::endl;

    JsonValueBoxed<bool> setRecordingMode = jsonRequest.GetRecordingMode();

    if (setRecordingMode.GetState() != JSON_VALUE_PROVIDED)
    {
        jsonResponse.Fail("Couldn't parse 'RecordLogs' field: " + setRecordingMode.GetState());
        return;
    }

    JsonValueBoxed<bool> setPublishLogsMode = jsonRequest.GetPublishLogsMode();

    if (setPublishLogsMode.GetState() != JSON_VALUE_PROVIDED)
    {
        jsonResponse.Fail("Couldn't parse 'PublishLogs' field: " + setPublishLogsMode.GetState());
        return;
    }

    LOG_INFO << "Recording mode was set to " << setRecordingMode.GetValue() << std::endl;
    OperationStatus os = Host::GetHost().GetDeviceManager().SetLogRecordingMode(setRecordingMode.GetValue(), deviceName, cpuType);
    if (!os)
    {
        jsonResponse.Fail(os.GetStatusMessage());
        return;
    }

    LOG_INFO << "Collection mode was set to " << setPublishLogsMode.GetValue() << std::endl;
    os = Host::GetHost().GetDeviceManager().SetLogPublishingMode(setPublishLogsMode.GetValue(), deviceName, cpuType);
    if (!os)
    {
        jsonResponse.Fail(os.GetStatusMessage());
        return;
    }
}

