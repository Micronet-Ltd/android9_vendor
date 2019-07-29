/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "LogCollectorGetVerbosityHandler.h"
#include "LogCollector.h"
#include "Host.h"

void LogCollectorGetVerbosityHandler::HandleRequest(const LogCollectorGetVerbosityRequest& jsonRequest, LogCollectorGetVerbosityResponse& jsonResponse)
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

    LOG_DEBUG << "Log Collector get verbosity request for Device: " << jsonRequest.GetDeviceName() << " with CPU type: " << (CPU_TYPE_TO_STRING.find(cpuType))->second << std::endl;

    std::string value;
    for (int i = 0; i < log_collector::NUM_MODULES; i++)
    {
        OperationStatus os = Host::GetHost().GetDeviceManager().GetLogCollectionModuleVerbosity(deviceName, cpuType, log_collector::module_names[i], value);
        if (!os)
        {
            jsonResponse.Fail(os.GetStatusMessage());
            return;
        }
        jsonResponse.SetModuleVerbosity(log_collector::module_names[i], value);
    }
}

