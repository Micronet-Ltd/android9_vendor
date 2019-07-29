/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "telephony/ril.h"
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"


/*===========================================================================*/
/*!
    @brief
    Provides IMSI of requested aid of a slot.

    mSlot     - In Slot on which IMSI needed
    mAidPtr   - In Pointer to AID on which IMSI is needed

    Retruns RIL_Errno shared pointer
*/
/*==========================================================================*/
class UimGetImsiRequestMsg : public SolicitedMessage<char>,
                             public add_message_id<UimGetImsiRequestMsg>
{
  private:
    uint8_t   mSlot;
    char     *mAidPtr;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.get_imsi_request";
    UimGetImsiRequestMsg() = delete;
    inline ~UimGetImsiRequestMsg()
    {
      if (mAidPtr != nullptr)
      {
        delete[] mAidPtr;
        mAidPtr = nullptr;
      }
    }

    UimGetImsiRequestMsg(uint8_t slot, char *aid_ptr,
                         GenericCallback<char> *callback):
                          SolicitedMessage<char>(get_class_message_id())
    {
      mSlot = slot;
      mAidPtr = nullptr;

      if (aid_ptr != nullptr)
      {
        mAidPtr = new char[strlen(aid_ptr) + 1]{0};

        if (mAidPtr != nullptr)
        {
          strlcpy(mAidPtr, aid_ptr, strlen(aid_ptr) + 1);
        }
      }
      mName = MESSAGE_NAME;
      setCallback(callback);
    }

    inline uint8_t get_slot(void)
    {
      return mSlot;
    }

    inline char *get_aid(void)
    {
      return mAidPtr;
    }

    inline string dump()
    {
      return mName + "{ slot :" + std::to_string(mSlot) + "} AidPtr: " + ((mAidPtr != nullptr) ? mAidPtr : "NULL");
    }
};
