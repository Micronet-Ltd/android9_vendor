/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef _LOG_COLLECTOR_JSON_VALUE_PARSER_H_
#define _LOG_COLLECTOR_JSON_VALUE_PARSER_H_
#pragma once

#include "JsonHandlerSDK.h"
#include "LogCollectorDefinitions.h"

class LogCollectorJsonValueParser
{
public:
    static JsonValueBoxed<CpuType> ParseCpuType(const Json::Value& jsonValue, const char* szKey)
    {
        const Json::Value& value = jsonValue[szKey];
        if (Json::Value::nullSingleton() == value)
        {
            return JsonValueBoxed<CpuType>(JSON_VALUE_MISSING);
        }

        auto cpuTypeIter = STRING_TO_CPU_TYPE.find(value.asString());
        if (cpuTypeIter == STRING_TO_CPU_TYPE.end())
        {
            return JsonValueBoxed<CpuType>(JSON_VALUE_MALFORMED);
        }

        return JsonValueBoxed<CpuType>(cpuTypeIter->second);
    }

};

#endif  // _LOG_COLLECTOR_JSON_VALUE_PARSER_H_
