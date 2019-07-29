/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <cstring>
#include "DSDModemEndPointModule.h"
#include <modules/qmi/QmiIndMessage.h>
#include "sync/SetApnInfoSyncMessage.h"
#include "UnSolMessages/DataNrIconTypeIndMessage.h"

DSDModemEndPointModule::DSDModemEndPointModule(string name,
    ModemEndPoint &owner) : ModemEndPointModule(name, owner)
{
  mServiceObject = nullptr;
  mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);

  using std::placeholders::_1;
  mMessageHandler = {
      HANDLER(SetApnInfoSyncMessage, DSDModemEndPointModule::handleSetApnInfoSync),
      {REG_MSG("DSDModemEndPoint_QMI_IND"),
        std::bind(&DSDModemEndPointModule::handleQmiDsdIndMessage, this, _1)},
  };
}

DSDModemEndPointModule::~DSDModemEndPointModule()
{
  mLooper = nullptr;
}

void DSDModemEndPointModule::init()
{
    ModemEndPointModule::init();
    mStackId[0] = 0;
}

qmi_idl_service_object_type DSDModemEndPointModule::getServiceObject()
{
  return dsd_get_service_object_v01();
}

bool DSDModemEndPointModule::handleQmiBinding
(
  qcril_instance_id_e_type instanceId, int8_t stackId)
{
  Log::getInstance().d(
        "[DSDModemEndPointModule] handleQmiBinding");

  (void)instanceId;
  if (stackId < 0)
    return false;

  dsd_bind_subscription_req_msg_v01 bind_request;
  dsd_bind_subscription_resp_msg_v01 bind_resp;
  memset(&bind_request, 0, sizeof(bind_request));
  memset(&bind_resp, 0, sizeof(bind_resp));

  if (stackId == 0) {
    bind_request.bind_subs = DSD_PRIMARY_SUBS_V01;
  } else if (stackId == 1) {
    bind_request.bind_subs = DSD_SECONDARY_SUBS_V01;
  } else if (stackId == 2) {
    bind_request.bind_subs = DSD_TERTIARY_SUBS_V01;
  } else
    return false;

  int ntries = 0;
  do {
    qmi_client_error_type res = qmi_client_send_msg_sync(
         mQmiSvcClient, QMI_DSD_BIND_SUBSCRIPTION_REQ_V01,
         (void*)&bind_request, sizeof(bind_request),
         (void*)&bind_resp, sizeof(bind_resp),
         ModemEndPointModule::DEFAULT_SYNC_TIMEOUT);
    if (QMI_NO_ERR == res && bind_resp.resp.result ==
            QMI_RESULT_SUCCESS_V01) {
      Log::getInstance().d(
        "[DSDModemEndPointModule]: QMI binding succeeds. instanceId: "
        + std::to_string((int)instanceId) + " stackId: " + std::to_string(stackId));
      return true;
    }
    usleep(500*1000);
  } while (++ntries < 10);
  return false;
}

void DSDModemEndPointModule::handleQmiDsdIndMessage
(
  std::shared_ptr<Message> msg
)
{
  auto shared_indMsg(std::static_pointer_cast<QmiIndMessage>(msg));
  Log::getInstance().d("[DSDModemEndPointModule]: Handling msg = " + msg->dump());

  QmiIndMsgDataStruct *indData = shared_indMsg->getData();
  if (indData != nullptr) {
    indicationHandler(indData->msgId, indData->indData, indData->indSize);
  }
  else {
    Log::getInstance().d("Unexpected, null data from message");
  }
}

void DSDModemEndPointModule::indicationHandler
(
  unsigned int   msg_id,
  unsigned char *decoded_payload,
  uint32_t       decoded_payload_len
)
{
  (void)decoded_payload_len;
  Log::getInstance().d("[DSDModemEndPointModule]: indicationHandler ENTRY");

  if(decoded_payload == nullptr) {
    Log::getInstance().d("decoded_payload is nullptr");
  }
  else {
    Log::getInstance().d("msg_id = "+ std::to_string(msg_id));
    switch(msg_id)
    {
      case QMI_DSD_UI_INFO_IND_V01:
        processUiInfoInd((dsd_ui_info_ind_msg_v01 *)decoded_payload);
        break;

      default:
        Log::getInstance().d("Unexpected dsd indication, ignoring = "+ std::to_string(msg_id));
        break;
    }
  }
}

void DSDModemEndPointModule::processUiInfoInd(dsd_ui_info_ind_msg_v01 *ind_data)
{
  Log::getInstance().d("[DSDModemEndPointModule]: processUiInfoInd ENTRY");
  if (ind_data->global_ui_info_valid) {
    if (0 != (QMI_DSD_UI_MASK_3GPP_5G_UWB_V01 & ind_data->global_ui_info.ui_mask)) {
      auto msg = std::make_shared<rildata::DataNrIconTypeIndMessage>(rildata::NrIconEnum_t::UWB);
      Log::getInstance().d("[DSDModemEndPointModule]: DataNrIconType UWB");
      msg->broadcast();
    } else if (0 != (QMI_DSD_UI_MASK_3GPP_5G_BASIC_V01 & ind_data->global_ui_info.ui_mask)) {
      auto msg = std::make_shared<rildata::DataNrIconTypeIndMessage>(rildata::NrIconEnum_t::BASIC);
      Log::getInstance().d("[DSDModemEndPointModule]: DataNrIconType BASIC");
      msg->broadcast();
    } else {
      auto msg = std::make_shared<rildata::DataNrIconTypeIndMessage>(rildata::NrIconEnum_t::NONE);
      Log::getInstance().d("[DSDModemEndPointModule]: DataNrIconType NONE");
      msg->broadcast();
    }
  } else {
    Log::getInstance().d("[DSDModemEndPointModule]: DataNrIconType TLV not valid");
  }
}

boolean DSDModemEndPointModule::isApnTypeFound
(
  const RIL_ApnTypes &apn_type, const uint8_t &apn_types_mask
)
{
  if ((apn_type & apn_types_mask) == apn_type)
  {
    Log::getInstance().d("apn type %x was enabled" + std::to_string(apn_type));
    return TRUE;
  }
  return FALSE;
}

void DSDModemEndPointModule::handleSetApnInfoSync(std::shared_ptr<Message> m)
{
  Log::getInstance().d("[DSDModemEndPointModule]: handleSetApnInfoSync : Handling msg = " + m->dump());
  auto msg= std::static_pointer_cast<SetApnInfoSyncMessage>(m);
  if (msg != NULL)
  {
    uint8_t ril_apn_types_mask = 0;
    std::string apn_name("");
    msg->getParams(apn_name, ril_apn_types_mask);
    Log::getInstance().d("[DSDModemEndPointModule]: apn_name= " +apn_name+"apn_types= %d" +std::to_string(ril_apn_types_mask));

    do
    {
      if (ril_apn_types_mask <= 0)
      {
        Log::getInstance().d("Invalid inputs");
        msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
      }

      if (isApnTypeFound(RIL_APN_TYPE_ALL, ril_apn_types_mask))
      {
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_DEFAULT_V01);
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_IMS_V01);
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_MMS_V01);
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_DUN_V01);
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_SUPL_V01);
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_HIPRI_V01);
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_FOTA_V01);
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_CBS_V01);
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_IA_V01);
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_EMERGENCY_V01);
        break;
      }
      if (isApnTypeFound(RIL_APN_TYPE_DEFAULT, ril_apn_types_mask))
      {
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_DEFAULT_V01);
      }
      if (isApnTypeFound(RIL_APN_TYPE_IMS, ril_apn_types_mask))
      {
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_IMS_V01);
      }
      if (isApnTypeFound(RIL_APN_TYPE_MMS, ril_apn_types_mask))
      {
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_MMS_V01);
      }
      if (isApnTypeFound(RIL_APN_TYPE_DUN, ril_apn_types_mask))
      {
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_DUN_V01);
      }
      if (isApnTypeFound(RIL_APN_TYPE_SUPL, ril_apn_types_mask))
      {
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_SUPL_V01);
      }
      if (isApnTypeFound(RIL_APN_TYPE_HIPRI, ril_apn_types_mask))
      {
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_HIPRI_V01);
      }
      if (isApnTypeFound(RIL_APN_TYPE_FOTA, ril_apn_types_mask))
      {
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_FOTA_V01);
      }
      if (isApnTypeFound(RIL_APN_TYPE_CBS, ril_apn_types_mask))
      {
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_CBS_V01);
      }
      if (isApnTypeFound(RIL_APN_TYPE_IA, ril_apn_types_mask))
      {
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_IA_V01);
      }
      if (isApnTypeFound(RIL_APN_TYPE_EMERGENCY, ril_apn_types_mask))
      {
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_EMERGENCY_V01);
      }
    } while(0);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS,nullptr);
  } else
  {
    Log::getInstance().d("Msg received is NULL");
    msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
  }
}

void DSDModemEndPointModule::sendApnInfoQmi
(
  const std::string &apn_name,
  dsd_apn_type_enum_v01 apn_type
)
{
  Log::getInstance().d("DSDModemEndPointModule::sendApnInfoQmi ENTRY");
  if (mQmiSvcClient != nullptr)
  {
    dsd_set_apn_info_req_msg_v01 data_setting_set_apn_param;
    dsd_set_apn_info_resp_msg_v01 data_setting_set_apn_param_resp;

    memset(&data_setting_set_apn_param, 0, sizeof(data_setting_set_apn_param));
    memset(&data_setting_set_apn_param_resp, 0,
      sizeof(dsd_set_apn_info_resp_msg_v01));
    strlcpy( &data_setting_set_apn_param.apn_info.apn_name[0],
             apn_name.c_str(), QMI_DSD_MAX_APN_LEN_V01);

    // indicate that the apn_invalid flag is set
    data_setting_set_apn_param.apn_invalid_valid = FALSE;
    data_setting_set_apn_param.apn_info.apn_type = apn_type;

    int rc = qmi_client_send_msg_sync(mQmiSvcClient,
                                   QMI_DSD_SET_APN_INFO_REQ_V01,
                                   (void *)&data_setting_set_apn_param,
                                   sizeof(data_setting_set_apn_param),
                                   (void *)&data_setting_set_apn_param_resp,
                                   sizeof(dsd_set_apn_info_resp_msg_v01),
                                   QCRIL_DATA_QMI_TIMEOUT);
    if (rc != QMI_NO_ERR) {
      Log::getInstance().d("[DSDModemEndPointModule] Failed to send"
        "QMI_DSD_SET_APN_INFO_REQ_V01 for apn_name:"+apn_name+
        ", type:"+std::to_string(apn_type)+"with rc ="+std::to_string(rc));
    } else
    {
      Log::getInstance().d("[DSDModemEndPointModule] sendApnInfoQmi::Successfully sent"
        "QMI_DSD_SET_APN_INFO_REQ_V01 for type ="+ std::to_string(apn_type));
    }
  }
}
