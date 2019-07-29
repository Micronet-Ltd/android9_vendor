/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef _PMC_SET_CONFIG_HANDLER_H_
#define _PMC_SET_CONFIG_HANDLER_H_

#include "JsonHandlerSDK.h"
#include "DebugLogger.h"
#include "OperationStatus.h"

class PmcSetConfigRequest: public JsonDeviceRequest
{
    public:
        PmcSetConfigRequest(const Json::Value& jsonRequestValue) :
                JsonDeviceRequest(jsonRequestValue)
        {
        }

        JsonValueBoxed<bool> GetCollectIdleSmEvents() const
        {
            return JsonValueParser::ParseBooleanValue(m_jsonRequestValue, "CollectIdleSmEvents");
        }

        JsonValueBoxed<bool> GetCollectRxPpduEvents() const
        {
            return JsonValueParser::ParseBooleanValue(m_jsonRequestValue, "CollectRxPpduEvents");
        }

        JsonValueBoxed<bool> GetCollectTxPpduEvents() const
        {
            return JsonValueParser::ParseBooleanValue(m_jsonRequestValue, "CollectTxPpduEvents");
        }

        JsonValueBoxed<bool> GetCollectUCodeEvents() const
        {
            return JsonValueParser::ParseBooleanValue(m_jsonRequestValue, "CollectUCodeEvents");
        }
};

class PmcSetConfigResponse: public JsonResponse
{
    public:
        PmcSetConfigResponse(const Json::Value& jsonRequestValue, Json::Value& jsonResponseValue) :
                JsonResponse("PmcSetConfigResponse", jsonRequestValue, jsonResponseValue)
        {
        }
};

class PmcSetConfigHandler: public JsonOpCodeHandlerBase<PmcSetConfigRequest, PmcSetConfigResponse>
{
    private:
        void HandleRequest(const PmcSetConfigRequest& jsonRequest, PmcSetConfigResponse& jsonResponse) override;
        OperationStatus ConfigureUCodeEvents(const std::string& deviceName, bool ucodeCoreWriteEnabled);
};

#endif// _PMC_SET_CONFIG_HANDLER_H_
