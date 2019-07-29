/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "modules/qmi/ModemEndPoint.h"
#include "WDSModemEndPointModule.h"
#include "framework/Log.h"

class WDSModemEndPoint : public ModemEndPoint
{
public:
  static constexpr const char *NAME = "WDSModemEndPoint";
  WDSModemEndPoint() : ModemEndPoint(NAME) {
    mModule = new WDSModemEndPointModule("WDSModemEndPointModule", *this);
    mModule->init();
    Log::getInstance().d("[WDSModemEndPoint]: xtor");
  }
  ~WDSModemEndPoint() {
      Log::getInstance().d("[WDSModemEndPoint]: destructor");
    //mModule->killLooper();
    delete mModule;
    mModule = nullptr;
  }

  void requestSetup(string clientToken, GenericCallback<string>* cb);

  /**
   * @brief      Gets the attach list.
   *
   * @param      attach_list  The attach list
   *
   * @return     Status
   */
  Message::Callback::Status getAttachListSync(
    std::shared_ptr<std::list<uint16_t>>& attach_list
  );

  /**
   * @brief      Set Attach list with desired action
   *
   * @param[in]  attach_list  The attach list
   * @param[in]  action       The action
   */
  Message::Callback::Status setAttachListSync(
    const std::shared_ptr<std::list<uint16_t>>& attach_list,
   const SetAttachListSyncMessage::AttachListAction action);

  Message::Callback::Status getAttachListCapabilitySync(
    std::shared_ptr<AttachListCap>& cap);
};
