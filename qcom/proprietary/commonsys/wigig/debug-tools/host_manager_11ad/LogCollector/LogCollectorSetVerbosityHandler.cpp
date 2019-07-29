/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include <sstream>

#include "LogCollectorSetVerbosityHandler.h"
#include "Host.h"
#include "Utils.h"

void LogCollectorSetVerbosityHandler::HandleRequest(const LogCollectorSetVerbosityRequest& jsonRequest, LogCollectorSetVerbosityResponse& jsonResponse)
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

    LOG_DEBUG << "Log Collector set verbosity request for Device: " << jsonRequest.GetDeviceName() << " with CPU type: " << (CPU_TYPE_TO_STRING.find(cpuType))->second << std::endl;

    JsonValueBoxed<std::string> getDefaultVerbosity = jsonRequest.GetDefaultVerbosity();
    OperationStatus os;

    for (int i = 0; i < log_collector::NUM_MODULES; i++)
    {
        const JsonValueBoxed<std::string> getModuleVerbosity = jsonRequest.GetModuleVerbosity(log_collector::module_names[i]);

        if (getModuleVerbosity.GetState() == JSON_VALUE_PROVIDED)
        {
            os = Host::GetHost().GetDeviceManager().SetLogCollectionModuleVerbosity(deviceName, cpuType, log_collector::module_names[i], getModuleVerbosity);
            if (!os)
            {
                jsonResponse.Fail(os.GetStatusMessage());
                return;
            }
        }
        else if (getDefaultVerbosity.GetState() == JSON_VALUE_PROVIDED)
        {
            os = Host::GetHost().GetDeviceManager().SetLogCollectionModuleVerbosity(deviceName, cpuType, log_collector::module_names[i], getDefaultVerbosity);
            if (!os)
            {
                jsonResponse.Fail(os.GetStatusMessage());
                return;
            }
        }
    }


}
