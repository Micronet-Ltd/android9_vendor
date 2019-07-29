/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef _JSON_GROUP_HANDLER_H_
#define _JSON_GROUP_HANDLER_H_

#include <string>
#include <map>
#include <memory>

#include "json/json.h"

// *************************************************************************************************

// TODO: Remove after re-factory, use JsonGroupHandler directly
class IJsonGroupHandler
{
public:
    virtual std::unique_ptr<Json::Value> HandleJsonCommand(const Json::Value& jsonCmd) = 0;
};

// *************************************************************************************************

class IJsonOpcodeHandler
{
public:
    virtual std::unique_ptr<Json::Value> HandleJsonCommand(const Json::Value& jsonCmd) = 0;
};

// *************************************************************************************************

class JsonGroupHandler: public IJsonGroupHandler
{
public:
    std::unique_ptr<Json::Value> HandleJsonCommand(const Json::Value& jsonCmd) { (void)jsonCmd; return std::unique_ptr<Json::Value>(); }

protected:

    JsonGroupHandler() {}
    void RegisterOpcodeHandler(const std::string& opCode, std::unique_ptr<IJsonOpcodeHandler> spJsonOpcodeHandler)
    {
        m_groupHandlerMap.insert(std::make_pair(opCode, std::move(spJsonOpcodeHandler)));
    }

private:

    std::map<std::string, std::unique_ptr<IJsonOpcodeHandler>> m_groupHandlerMap;
};

#endif      // _JSON_GROUP_HANDLER_H_
