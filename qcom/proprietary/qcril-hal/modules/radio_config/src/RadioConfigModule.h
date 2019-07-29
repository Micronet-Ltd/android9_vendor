/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/Module.h"
#include <framework/QcrilInitMessage.h>
#include "modules/uim/UimQmiUimIndicationMsg.h"

class RadioConfigModule : public Module
{
  public:
    bool mReady;
    bool mServiceUp;
    bool mQmiUimUp;

    RadioConfigModule();
    ~RadioConfigModule();
    void init();

  private:
    void handleQcrilInitMessage(std::shared_ptr<QcrilInitMessage> msg);
    void handleQmiUimIndiaction(std::shared_ptr<UimQmiUimIndicationMsg> msg);
};
