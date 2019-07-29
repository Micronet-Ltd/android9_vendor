/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/Module.h"
#include <framework/QcrilInitMessage.h>
#include "modules/uim/qcril_uim_sap.h"


typedef enum {
  UIM_SAP_CONNECT_REQ,
  UIM_SAP_DISCONNECT_REQ,
  UIM_SAP_APDU_REQ,
  UIM_SAP_TRANSFER_ATR_REQ,
  UIM_SAP_POWER_REQ,
  UIM_SAP_RESET_SIM_REQ,
  UIM_SAP_TRANSFER_CARD_READER_STATUS_REQ,
  UIM_SAP_SET_TRANSFER_PROTOCOL_REQ
} UimSapRequestType;


class SapModule : public Module
{
  public:
    SapModule();
    ~SapModule();
    void init();
    void handleQcrilInitMessage(std::shared_ptr<QcrilInitMessage> msg);
    boolean handleSapConnection(int32_t token, UimSapRequestType message_id);
    boolean handleSapRequest(int32_t token, UimSapRequestType msg_id, const uint8_t * payload_ptr,
                             size_t payload_len, int * error_code);
    static void handleAtelSapInd(qmi_uim_sap_status_type sap_state);
    static void handleAtelCardStatus(qcril_card_status_e_type card_state);
    static void handleAtelSapResp(int req_id, int err_code, int qmi_err, void * token,
                                  uint8_t * data_ptr, uint32_t data_len);
};
