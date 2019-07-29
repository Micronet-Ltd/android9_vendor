/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <vector>
#include "modules/qmi/ModemEndPoint.h"
#include "DSDModemEndPointModule.h"
#include "framework/Log.h"

class DSDModemEndPoint : public ModemEndPoint
{
public:
  static constexpr const char *NAME = "DSDModemEndPoint";
  DSDModemEndPoint() : ModemEndPoint(NAME)
  {
    mModule = new DSDModemEndPointModule("DSDModemEndPointModule", *this);
    mModule->init();
    Log::getInstance().d("[DSDModemEndPoint]: xtor");
  }
  ~DSDModemEndPoint()
  {
      Log::getInstance().d("[DSDModemEndPoint]: destructor");
    //mModule->killLooper();
    delete mModule;
    mModule = nullptr;
  }

  void requestSetup(string clientToken, GenericCallback<string>* cb);
  void registerForUiChangeInd();

  /**
   * @brief      Sets the apn information synchronously
   *
   * @param[in]  apn_name       The apn name
   * @param[in]  apn_type_mask  The apn type mask
   *
   * @return     { description_of_the_return_value }
   */
  Message::Callback::Status setApnInfoSync(const std::string apn_name,
   const uint8_t apn_type_mask);

  Message::Callback::Status getUiInfoSync(dsd_sys_ui_mask_v01 &uiMask);
};
