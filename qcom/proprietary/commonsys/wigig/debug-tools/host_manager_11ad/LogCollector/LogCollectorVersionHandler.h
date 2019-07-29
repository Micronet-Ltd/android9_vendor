/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef _LOG_COLLECTOR_VERSION_HANDLER_H_
#define _LOG_COLLECTOR_VERSION_HANDLER_H_
#pragma once

#include "JsonHandlerSDK.h"

class LogCollectorVersionRequest
{
public:
    LogCollectorVersionRequest(const Json::Value& jsonRequestValue) : m_jsonRequestValue(jsonRequestValue) {}
private:
    const Json::Value& m_jsonRequestValue;
};

class LogCollectorVersionResponse : public JsonResponse
{
public:
    LogCollectorVersionResponse(const Json::Value& jsonRequestValue, Json::Value& jsonResponseValue)
        : JsonResponse("LogCollectorVersionResponse", jsonRequestValue, jsonResponseValue)
    {
        m_jsonResponseValue["Version"] = "2.1";
    }
};

class LogCollectorVersionHandler : public JsonOpCodeHandlerBase<LogCollectorVersionRequest, LogCollectorVersionResponse>
{
private:
    void HandleRequest(const LogCollectorVersionRequest& jsonRequest, LogCollectorVersionResponse& jsonResponse) override
    {
        (void)jsonRequest;
        (void)jsonResponse;
    }
};

#endif// _LOG_COLLECTOR_VERSION_HANDLER_H_