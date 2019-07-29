/*===========================================================================
  Copyright (c) 2017 - 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/
#pragma once

#include "telephony/ril.h"

class UimModuleInterface
{
  private:
    static UimModuleInterface *mUimInterface;
    UimModuleInterface() {}
  public:
    ~UimModuleInterface() {}
    static UimModuleInterface *get_uim_interface()
    {
      if (!mUimInterface)
      {
        static UimModuleInterface mUimModule_instance;
        mUimInterface = &mUimModule_instance;
      }
      return mUimInterface;
    }

    /* Need to  be call cleanup_get_card_status after get_card_status to
       clean up memory allocated in ril_card_status */
    RIL_Errno get_card_status
    (
      uint8_t             slot,
      RIL_CardStatus_v6 * ril_card_status /* out ptr */
    );

    RIL_Errno get_imsi
    (
      uint8_t          slot,
      char           * aid_ptr,
      char          ** imsi_out_ptr /* out ptr, UIM allocates memory */
    );

    RIL_Errno read_iccid
    (
      uint8_t            slot,
      uint8_t          * iccid_data, /* out ptr */
      uint8_t          * iccid_len   /* out ptr */
    );

    /* Need to  be called after get_card_status to clean up memory allocated
       in ril_card_status */
    void cleanup_get_card_status
    (
      RIL_CardStatus_v6 * ril_card_status
    );
};
