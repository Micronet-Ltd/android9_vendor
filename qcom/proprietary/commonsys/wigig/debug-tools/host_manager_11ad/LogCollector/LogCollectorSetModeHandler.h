/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef _LOG_COLLECTOR_SET_MODE_HANDLER_H_
#define _LOG_COLLECTOR_SET_MODE_HANDLER_H_
#pragma once

#include "JsonHandlerSDK.h"
#include "DebugLogger.h"
#include "LogCollectorJsonValueParser.h"

class LogCollectorSetModeRequest : public JsonDeviceRequest
{
public:
    LogCollectorSetModeRequest(const Json::Value& jsonRequestValue) :
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

    const JsonValueBoxed<CpuType> GetCpuType() const
    {
        return LogCollectorJsonValueParser::ParseCpuType(m_jsonRequestValue, "CpuType");
    }
};

class LogCollectorSetModeResponse : public JsonResponse
{
public:
    LogCollectorSetModeResponse(const Json::Value& jsonRequestValue, Json::Value& jsonResponseValue) :
        JsonResponse("LogCollectorSetModeResponse", jsonRequestValue, jsonResponseValue)
    {
    }
};

class LogCollectorSetModeHandler : public JsonOpCodeHandlerBase<LogCollectorSetModeRequest, LogCollectorSetModeResponse>
{
private:
    void HandleRequest(const LogCollectorSetModeRequest& jsonRequest, LogCollectorSetModeResponse& jsonResponse) override;
};

#endif  // _LOG_COLLECTOR_SET_MODE_HANDLER_H_
