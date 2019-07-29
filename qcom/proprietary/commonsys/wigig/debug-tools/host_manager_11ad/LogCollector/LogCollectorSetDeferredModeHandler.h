/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef _LOG_COLLECTOR_SET_DEFERRED_MODE_HANDLER_H_
#define _LOG_COLLECTOR_SET_DEFERRED_MODE_HANDLER_H_
#pragma once

#include "JsonHandlerSDK.h"
#include "DebugLogger.h"

class LogCollectorSetDeferredModeRequest : public JsonDeviceRequest
{
public:
    LogCollectorSetDeferredModeRequest(const Json::Value& jsonRequestValue) :
        JsonDeviceRequest(jsonRequestValue)
    {
    }

    JsonValueBoxed<bool> GetRecordingMode() const
    {
        return JsonValueParser::ParseBooleanValue(m_jsonRequestValue, "RecordLogs");
    }

    JsonValueBoxed<bool> GetPublishLogsMode() const
    {
        return JsonValueParser::ParseBooleanValue(m_jsonRequestValue, "PublishLogs");
    }
};

class LogCollectorSetDeferredModeResponse : public JsonResponse
{
public:
    LogCollectorSetDeferredModeResponse(const Json::Value& jsonRequestValue, Json::Value& jsonResponseValue) :
        JsonResponse("LogCollectorSetDeferredModeResponse", jsonRequestValue, jsonResponseValue)
    {
    }
};

class LogCollectorSetDeferredModeHandler : public JsonOpCodeHandlerBase<LogCollectorSetDeferredModeRequest, LogCollectorSetDeferredModeResponse>
{
private:
    void HandleRequest(const LogCollectorSetDeferredModeRequest& jsonRequest, LogCollectorSetDeferredModeResponse& jsonResponse) override;
};

#endif  // _LOG_COLLECTOR_SET_DEFERRED_MODE_HANDLER_H_
