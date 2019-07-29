/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <cstring>
#include "WDSModemEndPointModule.h"
#include <modules/qmi/QmiIndMessage.h>
#include "ProfileHandler.h"
#include "sync/GetAttachListMessage.h"
#include "UnSolMessages/SetLteAttachPdnListActionResultMessage.h"

WDSModemEndPointModule::WDSModemEndPointModule(string name,
    ModemEndPoint &owner) : ModemEndPointModule(name, owner)
{
  mServiceObject = nullptr;
  mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);

  using std::placeholders::_1;
  mMessageHandler = {
      HANDLER(GetAttachListMessage, WDSModemEndPointModule::handleGetAttachList),
      HANDLER(SetAttachListSyncMessage, WDSModemEndPointModule::handleSetAttachList),
      HANDLER(GetAttachListCapabilitySyncMessage,
              WDSModemEndPointModule::handleGetAttachListCap),
      {REG_MSG("WDSModemEndPoint_QMI_IND"),
             std::bind(&WDSModemEndPointModule::handleWdsQmiIndMessage, this, _1)},
  };
}

WDSModemEndPointModule::~WDSModemEndPointModule()
{
  mLooper = nullptr;
}

void WDSModemEndPointModule::init()
{
  ModemEndPointModule::init();
  mStackId[0] = 0;
}

qmi_idl_service_object_type WDSModemEndPointModule::getServiceObject()
{
  return wds_get_service_object_v01();
}

bool WDSModemEndPointModule::handleQmiBinding
(
  qcril_instance_id_e_type instanceId, int8_t stackId
)
{
  Log::getInstance().d("[WDSModemEndPointModule] handleQmiBinding");

  (void)instanceId;
  if (stackId < 0)
    return false;

  wds_bind_subscription_req_msg_v01 bind_request;
  wds_bind_subscription_resp_msg_v01 bind_resp;
  memset(&bind_request, 0, sizeof(bind_request));
  memset(&bind_resp, 0, sizeof(bind_resp));

  if (stackId == 0)
  {
    bind_request.subscription = WDS_PRIMARY_SUBS_V01;
  } else if (stackId == 1)
  {
    bind_request.subscription = WDS_SECONDARY_SUBS_V01;
  } else if (stackId == 2)
  {
    bind_request.subscription = WDS_TERTIARY_SUBS_V01;
  } else
    return false;

  int ntries = 0;
  do
  {
    qmi_client_error_type res = qmi_client_send_msg_sync(
         mQmiSvcClient, QMI_WDS_BIND_SUBSCRIPTION_REQ_V01,
         (void*)&bind_request, sizeof(bind_request),
         (void*)&bind_resp, sizeof(bind_resp),
         ModemEndPointModule::DEFAULT_SYNC_TIMEOUT);

    if (QMI_NO_ERR == res && bind_resp.resp.result ==
            QMI_RESULT_SUCCESS_V01)
    {
      Log::getInstance().d(
        "[WDSModemEndPointModule]: QMI binding succeeds. instanceId: "
        + std::to_string((int)instanceId) + " stackId: " + std::to_string(stackId));
      return true;
    }
    usleep(500*1000);
  } while (++ntries < 10);

  return false;
}

void WDSModemEndPointModule::handleWdsQmiIndMessage
(
  std::shared_ptr<Message> msg
)
{
  auto shared_indMsg(
        std::static_pointer_cast<QmiIndMessage>(msg));
  Log::getInstance().d("[WDSModemEndPointModule]: Handling msg = " + msg->dump());

  QmiIndMsgDataStruct *indData = shared_indMsg->getData();
  if (indData != nullptr)
  {
    WdsUnsolicitedIndHdlr(indData->msgId, indData->indData, indData->indSize);
  } else
  {
    Log::getInstance().d("Unexpected, null data from message");
  }
}

void WDSModemEndPointModule::WdsUnsolicitedIndHdlr
(
  unsigned int   msg_id,
  unsigned char *decoded_payload,
  uint32_t       decoded_payload_len
)
{
  (void)decoded_payload_len;
  Log::getInstance().d("[WDSModemEndPointModule]: WdsUnsolicitedIndHdlr ENTRY");
  switch(msg_id)
  {
    case QMI_WDS_SET_LTE_ATTACH_PDN_LIST_ACTION_RESULT_IND_V01:
      if (decoded_payload != NULL)
      {
        wds_set_lte_attach_pdn_list_action_result_ind_msg_v01 *res
        = (wds_set_lte_attach_pdn_list_action_result_ind_msg_v01*) (decoded_payload);

        if(res != NULL)
        {
          Log::getInstance().d("[WDSModemEndPointModule]:"
            "WdsUnsolicitedIndHdlr: result = " + std::to_string(res->result));
          auto msg = std::make_shared<rildata::SetLteAttachPdnListActionResultMessage>(*res);
          msg->broadcast();
        }
      }
      break;
    default:
      Log::getInstance().d("Ignoring wds ind event %d"+ std::to_string(msg_id));
      break;
  }
}

void WDSModemEndPointModule::handleGetAttachList
(
  std::shared_ptr<Message> m
)
{
  Log::getInstance().d("[WDSModemEndPointModule]: Handling msg = " + m->dump());

  auto msg = std::static_pointer_cast<GetAttachListMessage>(m);
  if (msg!=NULL)
  {
    if (mQmiSvcClient != nullptr)
    {
      qmi_client_error_type rc;
      wds_get_lte_attach_pdn_list_req_msg_v01 req;
      wds_get_lte_attach_pdn_list_resp_msg_v01 resp;
      memset(&req, 0, sizeof(req));
      memset(&resp, 0, sizeof(resp));

      rc = qmi_client_send_msg_sync(
          mQmiSvcClient, QMI_WDS_GET_LTE_ATTACH_PDN_LIST_REQ_V01,
          (void *)&req, sizeof(req),
          (void *)&resp, sizeof(resp), QCRIL_DATA_QMI_TIMEOUT);
      if (QMI_NO_ERR != rc)
      {
        Log::getInstance().d("[WDSModemEndPointModule]:"
                             "failed to send get attach list request");
        msg->sendResponse(
            msg, Message::Callback::Status::FAILURE, nullptr);
      } else
      {
        if (QMI_RESULT_SUCCESS_V01 != resp.resp.result)
        {
          Log::getInstance().d("[WDSModemEndPointModule]:"
                               " get attach list response returned failed, err = " +
              std::to_string(resp.resp.result));
          msg->sendResponse(
              msg, Message::Callback::Status::FAILURE, nullptr);
        } else
        {
          Log::getInstance().d("[WDSModemEndPointModule]:"
                               " Get attach list response succeeeded");
          std::shared_ptr<std::list<uint16_t>> temp_list = std::make_shared<std::list<uint16_t>>();
          if (resp.attach_pdn_list_valid)
          {
            if( resp.pending_attach_pdn_list_valid )
            {
              for (uint32_t i = 0; i < resp.pending_attach_pdn_list_len; i++)
              {
                Log::getInstance().d("[WDSModemEndPointModule]:Attach pending pdn list profile = "
                                       +std::to_string(resp.pending_attach_pdn_list[i]));
                temp_list->push_back(resp.pending_attach_pdn_list[i]);
              }
            }
            else
            {
              for (uint32_t i = 0; i < resp.attach_pdn_list_len; i++)
              {
                Log::getInstance().d("[WDSModemEndPointModule]:Attach pdn list profile = "
                                      +std::to_string(resp.attach_pdn_list[i]));
                temp_list->push_back(resp.attach_pdn_list[i]);
              }
            }
          }
          else
          {
            Log::getInstance().d("[WDSModemEndPointModule]::"
                      "Optional parameter, Attach Pdn list in empty ");
          }
          msg->sendResponse(msg, Message::Callback::Status::SUCCESS,
                               temp_list);
        }
      }
    }
  }
}

void WDSModemEndPointModule::handleSetAttachList
(
  std::shared_ptr<Message> m
)
{
  Log::getInstance().d("[WDSModemEndPointModule]: Handling msg = " + m->dump());
  auto msg = std::static_pointer_cast<SetAttachListSyncMessage>(m);
  if (msg!=NULL)
  {
    Log::getInstance().d("[WDSModemEndPointModule]: handleSetAttachList Entry");
    if (mQmiSvcClient != nullptr)
    {
      qmi_client_error_type rc;
      wds_set_lte_attach_pdn_list_req_msg_v01 req;
      wds_set_lte_attach_pdn_list_resp_msg_v01 resp;

      memset(&req, 0, sizeof(req));
      memset(&resp, 0, sizeof(resp));
      auto params = std::make_shared<std::list<uint16_t>>();

      SetAttachListSyncMessage::AttachListAction action = SetAttachListSyncMessage::AttachListAction::NONE;
      msg->getParams(*params, action);

      req.attach_pdn_list_len = params->size();
      std::list<uint16_t>::const_iterator it;
      uint8_t i = 0;
      for(i=0, it = params->begin(); (( it != params->end()) && (i < req.attach_pdn_list_len));++it,++i)
      {
        req.attach_pdn_list[i] = *it;
      }
      // Print all elements in the list
      for(uint8_t i =0;i < params->size(); i++)
      {
        Log::getInstance().d("[WDSModemEndPointModule]:"
          " elements set in attach list ="+ std::to_string(req.attach_pdn_list[i]));
      }
      if( action == SetAttachListSyncMessage::AttachListAction::DISCONNECT_ATTACH_APN_ONLY )
      {
        req.action_valid = TRUE;
        req.action = wds_attach_pdn_list_action_enum_type_v01::WDS_ATTACH_PDN_LIST_ACTION_DETACH_OR_PDN_DISCONNECT_V01;
      } else if (action == SetAttachListSyncMessage::AttachListAction::NONE )
      {
        req.action_valid = FALSE;
        req.action = wds_attach_pdn_list_action_enum_type_v01::WDS_ATTACH_PDN_LIST_ACTION_NONE_V01;
      }
      rc = qmi_client_send_msg_sync(
          mQmiSvcClient, QMI_WDS_SET_LTE_ATTACH_PDN_LIST_REQ_V01,
          (void *)&req, sizeof(req),
          (void *)&resp, sizeof(resp), QCRIL_DATA_QMI_TIMEOUT);

      if (QMI_NO_ERR != rc)
      {
        Log::getInstance().d("[WDSModemEndPointModule]:"
                          " failed to send set attach list request");
        msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
      } else
      {
        if (QMI_RESULT_SUCCESS_V01 != resp.resp.result)
        {
          Log::getInstance().d("[WDSModemEndPointModule]:"
                               " Set attach list response returned failed, err = " +
                                std::to_string(resp.resp.result));
          msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
        } else
        {
          Log::getInstance().d("[WDSModemEndPointModule]: Set attach list response succeeeded.");
          msg->sendResponse(msg, Message::Callback::Status::SUCCESS,
                            nullptr);
        }
      }
    }
  }
}

void WDSModemEndPointModule::handleGetAttachListCap
(
  std::shared_ptr<Message> m
)
{
  Log::getInstance().d("[WDSModemEndPointModule]: Handling msg = " + m->dump());
  auto msg = std::static_pointer_cast<GetAttachListCapabilitySyncMessage>(m);
  if (msg!=NULL)
  {
    if (mQmiSvcClient != nullptr)
    {
      qmi_client_error_type rc;
      wds_get_capabilities_req_msg_v01 req;
      wds_get_capabilities_resp_msg_v01 resp;
      memset(&req, 0, sizeof(req));
      memset(&resp, 0, sizeof(resp));

      req.attach_pdn_list_action_capability_valid = TRUE;
      req.attach_pdn_list_action_capability = AttachAction::ATTACH_PDN_ACTION_SUPPORTED;

      rc = qmi_client_send_msg_sync(
           mQmiSvcClient, QMI_WDS_GET_CAPABILITIES_REQ_V01,
           (void *)&req, sizeof(req),
           (void *)&resp, sizeof(resp), QCRIL_DATA_QMI_TIMEOUT);

      if (QMI_NO_ERR != rc)
      {
        Log::getInstance().d("[WDSModemEndPointModule]:"
                             " failed to send 'get capabilities' request");
        msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
      } else
      {
        if (QMI_RESULT_SUCCESS_V01 != resp.resp.result)
        {
          Log::getInstance().d("[WDSModemEndPointModule]:get attach list response"
                               "returned failed, err = "+ std::to_string(resp.resp.result));
          msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
        } else
        {
          auto cap = std::make_shared<AttachListCap>();
          Log::getInstance().d("[WDSModemEndPointModule]:"
                               " Get Capabilities response succeeeded");
          if (resp.attach_pdn_list_action_capability_valid)
          {
            if ( resp.attach_pdn_list_action_capability ==
                      AttachAction::ATTACH_PDN_ACTION_SUPPORTED )
            {
              Log::getInstance().d("[WDSModemEndPointModule]:Get Capabilities"
                                   "response returned Action_Supported");
              *cap = ACTION_SUPPORTED;
            } else
            {
              Log::getInstance().d("[WDSModemEndPointModule]:Get Capabilities"
                                   " response returned Action_Not_Supported");
              *cap = ACTION_NOT_SUPPORTED;
            }
          } else
          {
            Log::getInstance().d("[WDSModemEndPointModule]:Get Capabilities"
                            "action invalid returning Action_Not_Supported");
            *cap = ACTION_NOT_SUPPORTED;
          }
          msg->sendResponse(msg, Message::Callback::Status::SUCCESS,cap);
        }
      }
    }
  }
}
