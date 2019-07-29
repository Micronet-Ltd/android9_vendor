/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "LogCollectorGetConfigHandler.h"
#include "Host.h"
#include "Utils.h"

void LogCollectorGetConfigHandler::HandleRequest(const LogCollectorGetConfigRequest& jsonRequest, LogCollectorGetConfigResponse& jsonResponse)
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

    LOG_DEBUG << "Log Collector get configuration request for Device: " << jsonRequest.GetDeviceName() << " with CPU type: " << (CPU_TYPE_TO_STRING.find(cpuType))->second << std::endl;

    log_collector::LogCollectorConfiguration logCollectorConfiguration;
    OperationStatus os = Host::GetHost().GetDeviceManager().GetLogCollectorConfiguration(deviceName, cpuType, logCollectorConfiguration);
    if (!os)
    {
        jsonResponse.Fail(os.GetStatusMessage());
        return;
    }
    jsonResponse.SetPollingIntervalMS((uint32_t)logCollectorConfiguration.GetPollingIntervalMs().count());
    jsonResponse.SetMaxSingleFileSizeMB(logCollectorConfiguration.GetMaxSingleFileSizeMb());
    jsonResponse.SetMaxNumOfLogFiles(logCollectorConfiguration.GetMaxNumOfLogFiles());
    jsonResponse.SetOutputFileSuffix(logCollectorConfiguration.GetOutputFileSuffix());
}
