/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "qcril_qmi_qti_radio_service.h"
#include "modules/qti_radio/RilRequestCdmaSendSmsMessageCallback.h"
#include <modules/android/RilRequestMessage.h>
#include "modules/sms/RilRequestCdmaSendSmsMessage.h"
#include "framework/Log.h"
#include "framework/legacy.h"
#include "qcril_log.h"
#define TAG "RILQ"

void RilRequestCdmaSendSmsMessageCallback::onResponse(
  std::shared_ptr<Message> solicitedMsg, Status status,
  std::shared_ptr<generic_callback_payload> responseDataPtr) {

    std::shared_ptr<RilRequestCdmaSendSmsMessage> msg =
            std::static_pointer_cast<RilRequestCdmaSendSmsMessage>(solicitedMsg);
    std::shared_ptr<legacy_response_payload> rsp =
            std::static_pointer_cast<legacy_response_payload>(responseDataPtr);
    Log::getInstance().d("[RilRequestCdmaSendSmsMessageCallback]: Callback[msg = " +
            solicitedMsg->dump() + "] executed. client data = " +  mClientToken );
    if (status == Status::SUCCESS && rsp) {
        qcril_request_resp_params_type &params = rsp->get_params();
        implementation::qtiSendCdmaSmsResponse(params.t, params.ril_err_no,
                                               params.resp_pkt, params.resp_len);
    } else {
        RIL_Errno ril_err = RIL_E_INTERNAL_ERR;
        if (status == Status::CANCELLED) {
            ril_err = RIL_E_CANCELLED;
        }
        qcril_request_params_type params = msg->get_request_params();
        implementation::qtiSendCdmaSmsResponse(params.t, ril_err, NULL,  0);
    }
}

Message::Callback *RilRequestCdmaSendSmsMessageCallback::clone() {
    RilRequestCdmaSendSmsMessageCallback *clone =
            new RilRequestCdmaSendSmsMessageCallback(mClientToken);
  return clone;
}
