/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "LogCollectorCommandDispatcher.h"
#include "LogCollectorVersionHandler.h"
#include "LogCollectorGetStateHandler.h"
#include "LogCollectorSetModeHandler.h"
#include "LogCollectorSetDeferredModeHandler.h"
#include "LogCollectorGetConfigHandler.h"
#include "LogCollectorSetConfigHandler.h"
#include "LogCollectorSetVerbosityHandler.h"
#include "LogCollectorGetVerbosityHandler.h"

LogCollectorCommandDispatcher::LogCollectorCommandDispatcher()
{
    RegisterOpCodeHandler("get_version", std::unique_ptr<IJsonHandler>(new LogCollectorVersionHandler()));
    RegisterOpCodeHandler("get_state", std::unique_ptr<IJsonHandler>(new LogCollectorGetStateHandler()));
    RegisterOpCodeHandler("set_mode", std::unique_ptr<IJsonHandler>(new LogCollectorSetModeHandler()));
    RegisterOpCodeHandler("set_deferred_mode", std::unique_ptr<IJsonHandler>(new LogCollectorSetDeferredModeHandler()));
    RegisterOpCodeHandler("get_config", std::unique_ptr<IJsonHandler>(new LogCollectorGetConfigHandler()));
    RegisterOpCodeHandler("set_config", std::unique_ptr<IJsonHandler>(new LogCollectorSetConfigHandler()));
    RegisterOpCodeHandler("set_verbosity", std::unique_ptr<IJsonHandler>(new LogCollectorSetVerbosityHandler()));
    RegisterOpCodeHandler("get_verbosity", std::unique_ptr<IJsonHandler>(new LogCollectorGetVerbosityHandler()));
}
