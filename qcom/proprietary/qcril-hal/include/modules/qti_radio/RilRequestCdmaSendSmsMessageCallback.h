/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/GenericCallback.h"
#include "framework/message_translator.h"
#include "framework/Message.h"

class RilRequestCdmaSendSmsMessageCallback : public GenericCallback<generic_callback_payload> {

 public:
     inline RilRequestCdmaSendSmsMessageCallback(string clientToken)
     : GenericCallback(clientToken){}

     inline ~RilRequestCdmaSendSmsMessageCallback(){};

     Message::Callback *clone() override;

     void onResponse(std::shared_ptr<Message> solicitedMsg, Status status,
                     std::shared_ptr<generic_callback_payload> responseDataPtr) override;
};
