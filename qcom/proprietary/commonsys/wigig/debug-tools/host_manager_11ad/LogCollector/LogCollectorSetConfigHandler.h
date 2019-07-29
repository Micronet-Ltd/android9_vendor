/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef _LOG_COLLECTOR_SET_CONFIG_HANDLER_H_
#define _LOG_COLLECTOR_SET_CONFIG_HANDLER_H_
#pragma once

#include "JsonHandlerSDK.h"
#include "DebugLogger.h"
#include "LogCollectorDefinitions.h"
#include "LogCollectorJsonValueParser.h"

class LogCollectorSetConfigRequest : public JsonDeviceRequest
{
public:
    LogCollectorSetConfigRequest(const Json::Value& jsonRequestValue) :
        JsonDeviceRequest(jsonRequestValue),
        m_outputFileSuffix(JsonValueParser::ParseStringValue(m_jsonRequestValue, log_collector::OUTPUT_FILE_SUFFIX.c_str()))
    {
    }

    const JsonValueBoxed<CpuType> GetCpuType() const
    {
        return LogCollectorJsonValueParser::ParseCpuType(m_jsonRequestValue, "CpuType");
    }

    const JsonValueBoxed<uint32_t> GetPollingIntervalMs() const
    {
        return JsonValueParser::ParseUnsignedValue(m_jsonRequestValue, log_collector::POLLING_INTERVAL_MS.c_str());
    }

    const JsonValueBoxed<uint32_t> GetMaxSingleFileSizeMb() const
    {
        return JsonValueParser::ParseUnsignedValue(m_jsonRequestValue, log_collector::MAX_SINGLE_FILE_SIZE_MB.c_str());
    }

    const JsonValueBoxed<uint32_t> GetMaxNumOfLogFiles() const
    {
        return JsonValueParser::ParseUnsignedValue(m_jsonRequestValue, log_collector::MAX_NUM_OF_LOG_FILES.c_str());
    }

    const JsonValueBoxed<std::string>& GetOutputFileSuffix() const
    {
        return m_outputFileSuffix;
    }
private:
    JsonValueBoxed<std::string> m_outputFileSuffix;
};

class LogCollectorSetConfigResponse : public JsonResponse
{
public:
    LogCollectorSetConfigResponse(const Json::Value& jsonRequestValue, Json::Value& jsonResponseValue) :
        JsonResponse("LogCollectorSetConfigResponse", jsonRequestValue, jsonResponseValue)
    {
    }
};

class LogCollectorSetConfigHandler : public JsonOpCodeHandlerBase<LogCollectorSetConfigRequest, LogCollectorSetConfigResponse>
{
private:
    void HandleRequest(const LogCollectorSetConfigRequest& jsonRequest, LogCollectorSetConfigResponse& jsonResponse) override;
};

#endif  // _LOG_COLLECTOR_SET_CONFIG_HANDLER_H_
