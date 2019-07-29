/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "UnSolMessages/DataNrIconTypeIndMessage.h"

using namespace rildata;

DataNrIconTypeIndMessage::DataNrIconTypeIndMessage(NrIconType_t setIcon):
    UnSolicitedMessage(get_class_message_id()), mIcon(setIcon)
{
  mName = MESSAGE_NAME;
}

DataNrIconTypeIndMessage::~DataNrIconTypeIndMessage()
{
}

bool DataNrIconTypeIndMessage::isUwb()
{
  return mIcon.isUwb();
}

bool DataNrIconTypeIndMessage::isBasic()
{
  return mIcon.isBasic();
}

bool DataNrIconTypeIndMessage::isNone()
{
  return mIcon.isNone();
}

std::shared_ptr<UnSolicitedMessage> DataNrIconTypeIndMessage::clone()
{
  return std::make_shared<DataNrIconTypeIndMessage>(mIcon);
}

string DataNrIconTypeIndMessage::dump()
{
  return MESSAGE_NAME;
}
