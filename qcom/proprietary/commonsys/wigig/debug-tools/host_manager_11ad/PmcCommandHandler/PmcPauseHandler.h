/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef _PMC_PAUSE_HANDLER_H_
#define _PMC_PAUSE_HANDLER_H_

#include "JsonHandlerSDK.h"
#include "DebugLogger.h"

class PmcPauseRequest: public JsonDeviceRequest
{
    public:
        PmcPauseRequest(const Json::Value& jsonRequestValue) :
                JsonDeviceRequest(jsonRequestValue)
        {
        }
};

class PmcPauseResponse: public JsonResponse
{
    public:
        PmcPauseResponse(const Json::Value& jsonRequestValue, Json::Value& jsonResponseValue) :
                JsonResponse("PmcPauseResponse", jsonRequestValue, jsonResponseValue)
        {
        }
};

class PmcPauseHandler: public JsonOpCodeHandlerBase<PmcPauseRequest, PmcPauseResponse>
{
    private:
        void HandleRequest(const PmcPauseRequest& jsonRequest, PmcPauseResponse& jsonResponse) override;
};

#endif  // _PMC_PAUSE_HANDLER_H_
