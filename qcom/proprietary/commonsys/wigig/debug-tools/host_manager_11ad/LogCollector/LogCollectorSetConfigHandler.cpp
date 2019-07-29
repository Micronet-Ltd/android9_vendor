/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include <sstream>

#include "LogCollectorSetConfigHandler.h"
#include "Host.h"
#include "Utils.h"

void LogCollectorSetConfigHandler::HandleRequest(const LogCollectorSetConfigRequest& jsonRequest, LogCollectorSetConfigResponse& jsonResponse)
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

    LOG_DEBUG << "Log Collector set configuration request for Device: " << jsonRequest.GetDeviceName() << " with CPU type: " << (CPU_TYPE_TO_STRING.find(cpuType))->second << std::endl;

    log_collector::LogCollectorConfiguration logCollectorConfiguration;
    OperationStatus os = Host::GetHost().GetDeviceManager().GetLogCollectorConfiguration(deviceName, cpuType, logCollectorConfiguration);
    if (!os)
    {
        jsonResponse.Fail(os.GetStatusMessage());
        return;
    }

    const JsonValueBoxed<uint32_t> getPollingIntervalMs = jsonRequest.GetPollingIntervalMs();
    if (getPollingIntervalMs.GetState() == JSON_VALUE_PROVIDED)
    {
        logCollectorConfiguration.SetPollingIntervalMs(getPollingIntervalMs);
    }
    else if (getPollingIntervalMs.GetState() == JSON_VALUE_MALFORMED)
    {
        jsonResponse.Fail("'polling_interval_ms' field is malformed");
        return;
    }

    const JsonValueBoxed<uint32_t> getMaxSingleFileSizeMB = jsonRequest.GetMaxSingleFileSizeMb();
    if (getMaxSingleFileSizeMB.GetState() == JSON_VALUE_PROVIDED)
    {
        logCollectorConfiguration.SetMaxSingleFileSizeMb(getMaxSingleFileSizeMB);
    }
    else if (getMaxSingleFileSizeMB.GetState() == JSON_VALUE_MALFORMED)
    {
        jsonResponse.Fail("'max_single_file_size_mb' field is malformed");
        return;
    }

    const JsonValueBoxed<uint32_t> getMaxNumOfLogFiles = jsonRequest.GetMaxNumOfLogFiles();
    if (getMaxNumOfLogFiles.GetState() == JSON_VALUE_PROVIDED)
    {
        logCollectorConfiguration.SetMaxNumOfLogFiles(getMaxNumOfLogFiles);
    }
    else if (getMaxNumOfLogFiles.GetState() == JSON_VALUE_MALFORMED)
    {
        jsonResponse.Fail("'max_number_of_log_files' field is malformed");
        return;
    }

    const JsonValueBoxed<std::string>& getOutputFileSuffix = jsonRequest.GetOutputFileSuffix();
    if (getOutputFileSuffix.GetState() == JSON_VALUE_PROVIDED)
    {
        logCollectorConfiguration.SetOutputFileSuffix(getOutputFileSuffix);
    }

    // Get configuration (by value) and then submit it, allows us to keep the code thread safe (because the device can be destroyed
    // while this function holds the configuration object) and to keep atomicity (all parameters are set only when we know that all
    // of them are valid)
    os = Host::GetHost().GetDeviceManager().SubmitLogCollectorConfiguration(deviceName, cpuType, logCollectorConfiguration);
    if (!os)
    {
        jsonResponse.Fail(os.GetStatusMessage());
    }
}
