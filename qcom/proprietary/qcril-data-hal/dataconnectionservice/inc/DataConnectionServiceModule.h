/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "framework/Module.h"
#include "DataConnectionServiceImpl.h"

namespace rildata {

using ::vendor::qti::hardware::data::connection::V1_0::DataConnectionServiceImpl;
using ::vendor::qti::hardware::data::connection::V1_0::IDataConnection;

class DataConnectionServiceModule : public Module {
public:
    DataConnectionServiceModule();
    ~DataConnectionServiceModule() {}
    void init();

private:
    void handleQcrilInit(std::shared_ptr<Message> msg);
    void handleBearerAllocationUpdate(std::shared_ptr<Message> msg);
    DataConnectionServiceImpl *mConnection;
};

}