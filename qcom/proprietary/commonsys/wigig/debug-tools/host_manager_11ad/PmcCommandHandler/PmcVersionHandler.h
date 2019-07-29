/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef _PMC_VERSION_HANDLER_H_
#define _PMC_VERSION_HANDLER_H_

#include "JsonHandlerSDK.h"

class PmcVersionRequest
{
public:
    PmcVersionRequest(const Json::Value& jsonRequestValue) : m_jsonRequestValue(jsonRequestValue) {}
private:
    const Json::Value& m_jsonRequestValue;
};

class PmcVersionResponse: public JsonResponse
{
public:
    PmcVersionResponse(const Json::Value& jsonRequestValue, Json::Value& jsonResponseValue)
        : JsonResponse("PmcVersionResponse", jsonRequestValue, jsonResponseValue)
    {
        m_jsonResponseValue["Version"] = "2.0";
    }
};

class PmcVersionHandler : public JsonOpCodeHandlerBase<PmcVersionRequest, PmcVersionResponse>
{
private:
    void HandleRequest(const PmcVersionRequest& jsonRequest, PmcVersionResponse& jsonResponse) override
    {
        (void)jsonRequest;
        (void)jsonResponse;

    }
};

#endif// _PMC_VERSION_HANDLER_H_
