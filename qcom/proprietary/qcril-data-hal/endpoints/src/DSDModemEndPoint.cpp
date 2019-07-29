/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "DSDModemEndPoint.h"
#include "sync/SetApnInfoSyncMessage.h"
#include <sstream>

using std::to_string;
constexpr const char *DSDModemEndPoint::NAME;

void DSDModemEndPoint::requestSetup(string clientToken,
                                      GenericCallback<string>* callback)
{
  auto shared_setupMsg = std::make_shared<QmiSetupRequest>
        (clientToken, 0, nullptr, callback);
  mModule->dispatch(shared_setupMsg);
}

void DSDModemEndPoint::registerForUiChangeInd()
{
  Log::getInstance().d("[DSDModemEndPoint]: registerForUiChangeInd ENTRY");
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    dsd_register_ui_info_change_req_msg_v01 qmiReq;
    dsd_register_ui_info_change_resp_msg_v01 qmiResp;
    memset(&qmiReq, 0, sizeof(qmiReq));
    memset(&qmiResp, 0, sizeof(qmiResp));

    qmiReq.report_ui_changes_valid = true;
    qmiReq.report_ui_changes = 1;
    qmiReq.suppress_so_change_valid = true;
    qmiReq.suppress_so_change = 1;
    qmiReq.suppress_null_bearer_reason_valid = true;
    qmiReq.suppress_null_bearer_reason = 1;

    auto retVal = sendRawSync(QMI_DSD_REGISTER_UI_INFO_CHANGE_REQ_V01,
                              (void *)&qmiReq, sizeof(qmiReq),
                              (void *)&qmiResp, sizeof(qmiResp),
                              QCRIL_DATA_QMI_TIMEOUT);
    if (retVal != QMI_NO_ERR) {
      Log::getInstance().d("[DSDModemEndPoint::registerForUiChangeInd]"
                           "Failed to send QMI_DSD_REGISTER_UI_CHANGE_REQ_V01");
    }
  } else {
    Log::getInstance().d("[DSDModemEndPoint::registerForUiChangeInd]"
                         "Failed to register for UI indication");
  }
}

Message::Callback::Status DSDModemEndPoint::setApnInfoSync
( const std::string apn_name,
  const uint8_t apn_type_mask)
{
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[DSDModemEndPoint::handleSetApnInfo]"
                         "dispatching message SetApnInfoSyncMessage");
    // Note that callback is not required for sync calls.
    auto msg = std::make_shared<SetApnInfoSyncMessage>(nullptr);
    msg->setParams(apn_name,apn_type_mask);
    Message::Callback::Status apiStatus;
    auto r = std::make_shared<int>();
    apiStatus = msg->dispatchSync(r);
    std::ostringstream ss;
    ss << "[DSDModemEndPoint::handleSetApnInfo] status = " << (int) apiStatus;
    Log::getInstance().d(ss.str());
    return apiStatus;
  } else {
    Log::getInstance().d("[DSDModemEndPoint::handleSetApnInfo]"
                         "Failed to send message SetApnInfoSyncMessage");
    return Message::Callback::Status::FAILURE;
  }
}

Message::Callback::Status DSDModemEndPoint::getUiInfoSync(dsd_sys_ui_mask_v01& uiMask)
{
  Log::getInstance().d("[DSDModemEndPoint]: getUiInfoSync ENTRY");
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    dsd_get_ui_info_req_msg_v01 qmiReq;
    dsd_get_ui_info_resp_msg_v01 qmiResp;
    memset(&qmiReq, 0, sizeof(qmiReq));
    memset(&qmiResp, 0, sizeof(qmiResp));

    auto retVal = sendRawSync(QMI_DSD_GET_UI_INFO_REQ_V01,
                              (void *)&qmiReq, sizeof(qmiReq),
                              (void *)&qmiResp, sizeof(qmiResp),
                              QCRIL_DATA_QMI_TIMEOUT);
    if (retVal != QMI_NO_ERR) {
      Log::getInstance().d("[DSDModemEndPoint::getUiInfoSync]"
                           "Failed to send QMI_DSD_GET_UI_INFO_REQ_V01");
      return Message::Callback::Status::FAILURE;
    }
    if (qmiResp.global_ui_info_valid) {
      uiMask = qmiResp.global_ui_info.ui_mask;
      return Message::Callback::Status::SUCCESS;
    }
    Log::getInstance().d("[DSDModemEndPoint::getUiInfoSync] bad QMI response");
    return Message::Callback::Status::FAILURE;
  } else {
    Log::getInstance().d("[DSDModemEndPoint::getUiInfoSync] Failed to get UI info");
    uiMask = QMI_DSD_UI_MASK_UNSPECIFIED_V01;
    return Message::Callback::Status::SUCCESS;
  }
}
