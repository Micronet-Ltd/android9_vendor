/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "LogCollectorSetDeferredModeHandler.h"
#include "LogCollector.h"
#include "Host.h"

void LogCollectorSetDeferredModeHandler::HandleRequest(const LogCollectorSetDeferredModeRequest& jsonRequest, LogCollectorSetDeferredModeResponse& jsonResponse)
{
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

    LOG_INFO << "Deferred recording mode was set to " << setRecordingMode.GetValue() << std::endl;
    Host::GetHost().GetDeviceManager().SetLogRecordingDeferredMode(setRecordingMode.GetValue());

    LOG_INFO << "Deferred publishing mode was set to " << setPublishLogsMode.GetValue() << std::endl;
    Host::GetHost().GetDeviceManager().SetLogPublishingDeferredMode(setPublishLogsMode.GetValue());
}

