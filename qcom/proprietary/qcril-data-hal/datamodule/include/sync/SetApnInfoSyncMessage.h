/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/SolicitedSyncMessage.h"
#include "framework/Util.h"
#include "framework/add_message_id.h"
#include "framework/Dispatcher.h"
#include <list>

class SetApnInfoSyncMessage : public SolicitedSyncMessage<int>,
                                    public add_message_id<SetApnInfoSyncMessage>
{
private:
  std::string m_apn;
  uint8_t m_apn_type_mask;

public:
  static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.SET_APN_INFO_SYNC";
  inline SetApnInfoSyncMessage(GenericCallback<int> *callback) :
    SolicitedSyncMessage<int>(get_class_message_id())
  {
    ignore(callback);
    mName = MESSAGE_NAME;
  }
  ~SetApnInfoSyncMessage() {}

  void setParams(const std::string apn,
    const uint8_t apn_type_mask)
  {
    m_apn = apn;
    m_apn_type_mask = apn_type_mask;
  }

  void getParams(std::string& apn_name,
                 uint8_t &apn_type_mask)
  {
    apn_name = m_apn;
    apn_type_mask = m_apn_type_mask;
  }
  string dump() {
    return mName;
  }
};