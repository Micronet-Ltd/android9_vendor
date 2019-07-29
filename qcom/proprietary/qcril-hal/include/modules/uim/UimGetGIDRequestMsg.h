/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "telephony/ril.h"
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"
#include "modules/uim/qcril_uim_srvc.h"
#include <vector>


/*===========================================================================*/
/*!
    @brief
    Provides GID of requested aid of a slot.

    mSlot     - In Slot on which GID needed
    mAidPtr   - In Pointer to AID on which GID is needed
    mGidType  - GID1 or GID2

    Retruns unit8_t shared pointer
*/
/*==========================================================================*/
class UimGetGIDRequestMsg : public SolicitedMessage<std::vector<uint8_t>>,
                            public add_message_id<UimGetGIDRequestMsg>
{
  public:
    enum gid_type
    {
      GID1,
      GID2
    };

  private:
    uint8_t            mSlot;
    char             * mAidPtr;
    qmi_uim_app_type   mAppType;
    gid_type           mGidType;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.get_gid_request";

    UimGetGIDRequestMsg() = delete;
    ~UimGetGIDRequestMsg()
    {
      if (mAidPtr != nullptr)
      {
        delete[] mAidPtr;
        mAidPtr = nullptr;
      }
    }

    UimGetGIDRequestMsg(uint8_t slot, char *aid_ptr, qmi_uim_app_type app_type, gid_type gid,
                        GenericCallback<std::vector<uint8_t>> *callback) :
                        SolicitedMessage<std::vector<uint8_t>>(get_class_message_id())
    {
      mSlot = slot;
      mAidPtr = nullptr;
      mAppType = app_type;

      if (aid_ptr != nullptr)
      {
        mAidPtr = new char[strlen(aid_ptr) + 1]{0};

        if (mAidPtr != nullptr)
        {
          strlcpy(mAidPtr, aid_ptr, strlen(aid_ptr) + 1);
        }
      }
      mName = MESSAGE_NAME;
      mGidType = gid;
      setCallback(callback);
    }

    inline uint8_t get_slot(void)
    {
      return mSlot;
    }

    inline const char *get_aid(void)
    {
      return mAidPtr;
    }

    inline gid_type get_gid_type()
    {
      return mGidType;
    }

    inline qmi_uim_app_type get_app_type()
    {
      return mAppType;
    }

    inline string dump()
    {
      return mName + "{ slot :" + std::to_string(mSlot) + "} AidPtr: " + ((mAidPtr != nullptr) ? mAidPtr : "NULL");
    }
};
