/**
* Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#include <iostream>
#include <cstdlib>
#include <pthread.h>

#include "qmi_client.h"
#include "wireless_data_service_v01.h"

/* Framework includes */
#include "framework/Dispatcher.h"
#include "framework/Looper.h"
#include "framework/ModuleLooper.h"
#include "framework/QcrilInitMessage.h"
#include "modules/android/ril_message_factory.h"
#include "modules/qmi/QmiIndMessage.h"
#include "modules/qmi/EndpointStatusIndMessage.h"
#include "modules/qmi/ModemEndPointFactory.h"

/* Module includes */
#include "local/DataModule.h"
#include "UnSolMessages/DataInitMessage.h"
#include "request/RilRequestSetupDataCallMessage.h"
#include "request/RilRequestDataCallListMessage.h"
#include "request/RilRequestGetDataCallProfileMessage.h"
#include "request/RilRequestDeactivateDataCallMessage.h"
#include "request/RilRequestEmbmsActivateDeactivateTmgiMessage.h"
#include "request/RilRequestEmbmsActivateTmgiMessage.h"
#include "request/RilRequestEmbmsContentDescUpdateMessage.h"
#include "request/RilRequestEmbmsDeactivateTmgiMessage.h"
#include "request/EmbmsEnableDataReqMessage.h"
#include "request/RilRequestEmbmsGetActiveTmgiMessage.h"
#include "request/RilRequestEmbmsGetAvailTmgiMessage.h"
#include "request/RilRequestEmbmsSendIntTmgiListMessage.h"
#include "request/RilRequestGoDormantMessage.h"
#include "request/RilRequestLastDataCallFailCauseMessage.h"
#include "request/ProcessScreenStateChangeMessage.h"
#include "request/ProcessStackSwitchMessage.h"
#include "request/RilRequestPullLceDataMessage.h"
#include "request/SetApnInfoMessage.h"
#include "request/RilRequestSetDataProfileMessage.h"
#include "request/SetIsDataEnabledMessage.h"
#include "request/SetIsDataRoamingEnabledMessage.h"
#include "request/SetLteAttachProfileMessage.h"
#include "request/SetQualityMeasurementMessage.h"
#include "request/SetRatPrefMessage.h"
#include "request/RilRequestStartLceMessage.h"
#include "request/RilRequestStopLceMessage.h"
#include "request/ToggleDormancyIndMessage.h"
#include "request/ToggleLimitedSysIndMessage.h"
#include "request/UpdateMtuMessage.h"
#include "request/GetDdsSubIdMessage.h"
#include "request/RequestDdsSwitchMessage.h"
#include "request/RilRequestStartKeepaliveMessage.h"
#include "request/RilRequestStopKeepaliveMessage.h"
#include "request/RilRequestSetCarrierInfoImsiEncryptionMessage.h"
#include "request/SetLinkCapRptCriteriaMessage.h"
#include "request/SetLinkCapFilterMessage.h"
#include "request/SetInitialAttachApnRequestMessage.h"
#include "request/RegisterBearerAllocationUpdateRequestMessage.h"
#include "request/GetBearerAllocationRequestMessage.h"
#include "request/GetAllBearerAllocationsRequestMessage.h"
#include "request/GetDataNrIconTypeMessage.h"

#include "event/DataGetMccMncCallback.h"
#include "sync/RilDataEmbmsActiveMessage.h"
#include "UnSolMessages/SetLteAttachPdnListActionResultMessage.h"
#include "UnSolMessages/DataAllBearerTypeChangedMessage.h"
#include "UnSolMessages/DataBearerTypeChangedMessage.h"
#include "UnSolMessages/CallStatusMessage.h"

#include "legacy/qcril_data_qmi_wds.h"
#include "legacy/qcril_data.h"

#include "AuthModemEndPoint.h"
#include "WDSModemEndPoint.h"
#include "DSDModemEndPoint.h"

#include "ProfileHandler.h"
#include "NetworkServiceHandler.h"
#include "qcril_reqlist.h"

#include "telephony/ril.h"
#include "modules/nas/NasSrvDomainPrefIndMessage.h"
#include "modules/nas/NasRequestDataShutdownMessage.h"

#ifndef QCRIL_RIL_VERSION
#error "undefined QCRIL_RIL_VERSION!"
#endif

extern pthread_mutex_t ddsSubMutex;

#include "modules/uim/qcril_uim.h"
#include "modules/uim/UimCardAppStatusIndMsg.h"
#include "modules/uim/UimGetMccMncRequestMsg.h"
#include "modules/uim/UimGetCardStatusRequestSyncMsg.h"

#define MCC_LENGTH 4
#define MNC_LENGTH 4

static load_module<rildata::DataModule> dataModule;
ProfileHandler::RilRespParams ProfileHandler::m_resp_params={ QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, 0,0};
extern DDSSubIdInfo currentDDSSUB;

namespace rildata {

bool clearTimeoutForMessage( std::shared_ptr<Message> msg );

DataModule::DataModule() {
  mName = "DataModule";
  mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);
  ModemEndPointFactory<AuthModemEndPoint>::getInstance().buildEndPoint();
  ModemEndPointFactory<WDSModemEndPoint>::getInstance().buildEndPoint();
  ModemEndPointFactory<DSDModemEndPoint>::getInstance().buildEndPoint();

  using std::placeholders::_1;

  mMessageHandler = {
                      {QcrilInitMessage::get_class_message_id(), std::bind(&DataModule::handleQcrilInitMessage, this, _1)},
                      {RilRequestDataCallListMessage::get_class_message_id(), std::bind(&DataModule::handleDataCallListMessage, this, _1)},
                      {RilRequestGetDataCallProfileMessage::get_class_message_id(), std::bind(&DataModule::handleGetDataCallProfileMessage, this, _1)},
                      {RilRequestDeactivateDataCallMessage::get_class_message_id(), std::bind(&DataModule::handleDeactivateDataCallMessage, this, _1)},
                      {RilRequestEmbmsActivateDeactivateTmgiMessage::get_class_message_id(), std::bind(&DataModule::handleEmbmsActivateDeactivateTmgiMessage, this, _1)},
                      {RilRequestEmbmsActivateTmgiMessage::get_class_message_id(), std::bind(&DataModule::handleEmbmsActivateTmgiMessage, this, _1)},
                      {RilRequestEmbmsContentDescUpdateMessage::get_class_message_id(), std::bind(&DataModule::handleEmbmsContentDescUpdateMessage, this, _1)},
                      {RilRequestEmbmsDeactivateTmgiMessage::get_class_message_id(), std::bind(&DataModule::handleEmbmsDeactivateTmgiMessage, this, _1)},
                      {EmbmsEnableDataReqMessage::get_class_message_id(), std::bind(&DataModule::handleEmbmsEnableDataReqMessage, this, _1)},
                      {RilRequestEmbmsGetActiveTmgiMessage::get_class_message_id(), std::bind(&DataModule::handleEmbmsGetActiveTmgiMessage, this, _1)},
                      {RilRequestEmbmsGetAvailTmgiMessage::get_class_message_id(), std::bind(&DataModule::handleEmbmsGetAvailTmgiMessage, this, _1)},
                      {RilRequestEmbmsSendIntTmgiListMessage::get_class_message_id(), std::bind(&DataModule::handleEmbmsSendIntTmgiListMessage, this, _1)},
                      {RilRequestGoDormantMessage::get_class_message_id(), std::bind(&DataModule::handleGoDormantMessage, this, _1)},
                      {RilRequestLastDataCallFailCauseMessage::get_class_message_id(), std::bind(&DataModule::handleLastDataCallFailCauseMessage, this, _1)},
                      {ProcessScreenStateChangeMessage::get_class_message_id(), std::bind(&DataModule::handleProcessScreenStateChangeMessage, this, _1)},
                      {ProcessStackSwitchMessage::get_class_message_id(), std::bind(&DataModule::handleProcessStackSwitchMessage, this, _1)},
                      {RilRequestPullLceDataMessage::get_class_message_id(), std::bind(&DataModule::handlePullLceDataMessage, this, _1)},
                      {SetApnInfoMessage::get_class_message_id(), std::bind(&DataModule::handleSetApnInfoMessage, this, _1)},
                      {SetIsDataEnabledMessage::get_class_message_id(), std::bind(&DataModule::handleSetIsDataEnabledMessage, this, _1)},
                      {SetIsDataRoamingEnabledMessage::get_class_message_id(), std::bind(&DataModule::handleSetIsDataRoamingEnabledMessage, this, _1)},
                      {SetQualityMeasurementMessage::get_class_message_id(), std::bind(&DataModule::handleSetQualityMeasurementMessage, this, _1)},
                      {SetRatPrefMessage::get_class_message_id(), std::bind(&DataModule::handleSetRatPrefMessage, this, _1)},
                      {RilRequestSetupDataCallMessage::get_class_message_id(), std::bind(&DataModule::handleSetupDataCallMessage, this, _1)},
                      {RilRequestStartLceMessage::get_class_message_id(), std::bind(&DataModule::handleStartLceMessage, this, _1)},
                      {RilRequestStopLceMessage::get_class_message_id(), std::bind(&DataModule::handleStopLceMessage, this, _1)},
                      {ToggleDormancyIndMessage::get_class_message_id(), std::bind(&DataModule::handleToggleDormancyIndMessage, this, _1)},

                      {ToggleLimitedSysIndMessage::get_class_message_id(), std::bind(&DataModule::handleToggleLimitedSysIndMessage, this, _1)},
                      {UpdateMtuMessage::get_class_message_id(), std::bind(&DataModule::handleUpdateMtuMessage, this, _1)},
                      {RilDataEmbmsActiveMessage::get_class_message_id(), std::bind(&DataModule::handleDataEmbmsActiveMessage, this, _1)},
                      {GetDdsSubIdMessage::get_class_message_id(), std::bind(&DataModule::handleGetDdsSubIdMessage, this, _1)},
                      {RequestDdsSwitchMessage::get_class_message_id(), std::bind(&DataModule::handleDataRequestDDSSwitchMessage, this, _1)},
#if (QCRIL_RIL_VERSION >= 15)
                      {SetLteAttachProfileMessage_v15::get_class_message_id(), std::bind(&DataModule::handleSetLteAttachProfileMessage_v15, this, _1)},
                      {RilRequestSetDataProfileMessage_v15::get_class_message_id(), std::bind(&DataModule::handleSetDataProfileMessage_v15, this, _1)},
                      {RilRequestStartKeepaliveMessage::get_class_message_id(), std::bind(&DataModule::handleStartKeepaliveMessage, this, _1)},
                      {RilRequestStopKeepaliveMessage::get_class_message_id(), std::bind(&DataModule::handleStopKeepaliveMessage, this, _1)},
                      {RilRequestSetCarrierInfoImsiEncryptionMessage::get_class_message_id(), std::bind(&DataModule::handleSetCarrierInfoImsiEncryptionMessage, this, _1)},
                      {REG_MSG("AUTH_QMI_IND"), std::bind(&DataModule::handleQmiAuthServiceIndMessage, this, _1)},
                      {REG_MSG("AUTH_ENDPOINT_STATUS_IND"), std::bind(&DataModule::handleQmiAuthEndpointStatusIndMessage, this, _1)},
                      {SetLinkCapFilterMessage::get_class_message_id(), std::bind(&DataModule::handleSetLinkCapFilterMessage, this, _1)},
                      {SetLinkCapRptCriteriaMessage::get_class_message_id(), std::bind(&DataModule::handleSetLinkCapRptCriteriaMessage, this, _1)},
#else
                      {SetLteAttachProfileMessage::get_class_message_id(), std::bind(&DataModule::handleSetLteAttachProfileMessage, this, _1)},
                      {RilRequestSetDataProfileMessage::get_class_message_id(), std::bind(&DataModule::handleSetDataProfileMessage, this, _1)},
#endif /* QCRIL_RIL_VERSION >= 15 */
                      {SetInitialAttachApnRequestMessage::get_class_message_id(), std::bind(&DataModule::handleSetInitialAttachApn, this, _1)},
                      {SetLteAttachPdnListActionResultMessage::get_class_message_id(), std::bind(&DataModule::handleSetLteAttachPdnListActionResult, this, _1)},
                      {NasSrvDomainPrefIndMessage::get_class_message_id(), std::bind(&DataModule::handleNasSrvDomainPrefInd, this, _1)},
                      {NasRequestDataShutdownMessage::get_class_message_id(), std::bind(&DataModule::handleNasRequestDataShutdown, this, _1)},
                      {DataBearerTypeChangedMessage::get_class_message_id(), std::bind(&DataModule::handleDataBearerTypeUpdate, this, _1)},
                      {DataAllBearerTypeChangedMessage::get_class_message_id(), std::bind(&DataModule::handleDataAllBearerTypeUpdate, this, _1)},
                      {RegisterBearerAllocationUpdateRequestMessage::get_class_message_id(), std::bind(&DataModule::handleToggleBearerAllocationUpdate, this, _1)},
                      {GetBearerAllocationRequestMessage::get_class_message_id(), std::bind(&DataModule::handleGetBearerAllocation, this, _1)},
                      {GetAllBearerAllocationsRequestMessage::get_class_message_id(), std::bind(&DataModule::handleGetAllBearerAllocations, this, _1)},
                      {REG_MSG("WDSModemEndPoint_ENDPOINT_STATUS_IND"), std::bind(&DataModule::handleQmiWdsEndpointStatusIndMessage, this, _1)},
                      {CallStatusMessage::get_class_message_id(), std::bind(&DataModule::handleDataConnectionStateChangedMessage, this, _1)},
                      {REG_MSG("DSDModemEndPoint_ENDPOINT_STATUS_IND"), std::bind(&DataModule::handleQmiDsdEndpointStatusIndMessage, this, _1)},
                      {GetDataNrIconTypeMessage::get_class_message_id(), std::bind(&DataModule::handleGetDataNrIconType, this, _1)},
                      {UimCardAppStatusIndMsg::get_class_message_id(), std::bind(&DataModule::handleUimCardAppStatusIndMsg, this, _1)},
                    };
}

DataModule::~DataModule() {
  mLooper = nullptr;
  //mDsdEndPoint = nullptr;
}

void DataModule::init() {
  /* Call base init before doing any other stuff.*/
  Module::init();
  auth_manager = std::make_unique<AuthManager>();
  profile_handler = std::make_unique<ProfileHandler>();
  network_service_handler = std::make_unique<NetworkServiceHandler>();
}

void DataModule::broadcastReady()
{
    std::shared_ptr<DataInitMessage> data_init_msg =
                       std::make_shared<DataInitMessage>(global_instance_id);
    Dispatcher::getInstance().broadcast(data_init_msg);
}

void DataModule::handleQcrilInitMessage(std::shared_ptr<Message> msg)
{
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  auto qcril_init_msg = std::static_pointer_cast<QcrilInitMessage>(msg);
  if( qcril_init_msg != NULL )
  {
    global_instance_id = qcril_init_msg->get_instance_id();
    qcril_data_init();
    if(auth_manager)
    {
      auth_manager->init();
    } else{
      Log::getInstance().d("ERROR!! Failed to initialize AuthManager object");
    }
    if(profile_handler)
    {
      profile_handler->init();
    } else{
      Log::getInstance().d("ERROR!! Failed to initialize Profile Handler object");
    }

    broadcastReady();
    Log::getInstance().d("[" + mName + "]: Done msg = " + msg->dump());
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received. =" + msg->dump() + "QCRIL DATA Init not triggered!!!");
  }
}

void DataModule::handleDataCallListMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  qcril_request_return_type ret;
  auto m = std::static_pointer_cast<RilRequestDataCallListMessage>(msg);
  if( m != NULL ) {
    qcril_data_request_data_call_list(&(m->get_params()), &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleGetDataCallProfileMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestGetDataCallProfileMessage> m = std::static_pointer_cast<RilRequestGetDataCallProfileMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_request_omh_profile_info(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleDeactivateDataCallMessage(std::shared_ptr<Message> msg) {

  if( msg == NULL ) {
    Log::getInstance().d("[" + mName + "]: ERROR!!! Msg received is NULL");
    return;
  }
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());

  // Cancel the default timer
  clearTimeoutForMessage(msg);

  //Add a new timeout handler
  setTimeoutForMsg(msg, msg->getMessageExpiryTimer());

  std::shared_ptr<RilRequestDeactivateDataCallMessage> m = std::static_pointer_cast<RilRequestDeactivateDataCallMessage>(msg);

  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_request_deactivate_data_call(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received");
  }
}

void DataModule::handleEmbmsActivateDeactivateTmgiMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestEmbmsActivateDeactivateTmgiMessage> m = std::static_pointer_cast<RilRequestEmbmsActivateDeactivateTmgiMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_embms_activate_deactivate_tmgi(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleEmbmsActivateTmgiMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestEmbmsActivateTmgiMessage> m = std::static_pointer_cast<RilRequestEmbmsActivateTmgiMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_embms_activate_tmgi(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleEmbmsContentDescUpdateMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestEmbmsContentDescUpdateMessage> m = std::static_pointer_cast<RilRequestEmbmsContentDescUpdateMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_embms_content_desc_update(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleEmbmsDeactivateTmgiMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestEmbmsDeactivateTmgiMessage> m = std::static_pointer_cast<RilRequestEmbmsDeactivateTmgiMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_embms_deactivate_tmgi(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleEmbmsEnableDataReqMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<EmbmsEnableDataReqMessage> m = std::static_pointer_cast<EmbmsEnableDataReqMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_embms_enable_data_req(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleEmbmsGetActiveTmgiMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestEmbmsGetActiveTmgiMessage> m = std::static_pointer_cast<RilRequestEmbmsGetActiveTmgiMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_embms_get_active_tmgi(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleEmbmsGetAvailTmgiMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestEmbmsGetAvailTmgiMessage> m = std::static_pointer_cast<RilRequestEmbmsGetAvailTmgiMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_embms_get_available_tmgi(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleEmbmsSendIntTmgiListMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestEmbmsSendIntTmgiListMessage> m = std::static_pointer_cast<RilRequestEmbmsSendIntTmgiListMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_embms_send_interested_list(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleGoDormantMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestGoDormantMessage> m = std::static_pointer_cast<RilRequestGoDormantMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_process_qcrilhook_go_dormant(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleLastDataCallFailCauseMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestLastDataCallFailCauseMessage> m = std::static_pointer_cast<RilRequestLastDataCallFailCauseMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_request_last_data_call_fail_cause(&req,&ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleProcessScreenStateChangeMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<ProcessScreenStateChangeMessage> m = std::static_pointer_cast<ProcessScreenStateChangeMessage>(msg);
  if( m != NULL ) {
    int ret = qcril_data_process_screen_state_change(m->screenState);
    Message::Callback::Status status = (ret == QCRIL_DS_SUCCESS ?
           Message::Callback::Status::SUCCESS : Message::Callback::Status::FAILURE);
    auto resp = std::make_shared<int>(ret);
    m->sendResponse(msg, status, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleProcessStackSwitchMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<ProcessStackSwitchMessage> m = std::static_pointer_cast<ProcessStackSwitchMessage>(msg);
  if( m != NULL ) {
    ProcessStackSwitchMessage::StackSwitchReq info = m->getParams();
    qcril_data_process_stack_switch( info.oldStackId, info.newStackId, info.instanceId);
    auto resp = std::make_shared<int>(QCRIL_DS_SUCCESS);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handlePullLceDataMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestPullLceDataMessage> m = std::static_pointer_cast<RilRequestPullLceDataMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_lqe_get_info(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleSetApnInfoMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<SetApnInfoMessage> m = std::static_pointer_cast<SetApnInfoMessage>(msg);
  if( m != NULL ) {
    RIL_Errno ret = qcril_data_set_apn_info(&m->mParams, m->mApnType, m->mApnName, m->mIsApnValid);
    auto resp = std::make_shared<RIL_Errno>(ret);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleSetIsDataEnabledMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<SetIsDataEnabledMessage> m = std::static_pointer_cast<SetIsDataEnabledMessage>(msg);
  if ( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    RIL_Errno ret = qcril_data_set_is_data_enabled( &req, m->is_data_enabled);
    auto resp = std::make_shared<RIL_Errno>(ret);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleSetIsDataRoamingEnabledMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<SetIsDataRoamingEnabledMessage> m = std::static_pointer_cast<SetIsDataRoamingEnabledMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    RIL_Errno ret = qcril_data_set_is_data_roaming_enabled(&req, m->is_data_roaming_enabled);
    auto resp = std::make_shared<RIL_Errno>(ret);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

#if (QCRIL_RIL_VERSION >= 15)
void DataModule::handleSetLteAttachProfileMessage_v15(std::shared_ptr<Message> msg) {
  std::shared_ptr<SetLteAttachProfileMessage_v15> reqMsg = std::static_pointer_cast<SetLteAttachProfileMessage_v15>(msg);
  if( reqMsg != NULL ) {
    RIL_Errno ret = qcril_data_request_set_lte_attach_profile_v15 ( reqMsg->get_params() );
    auto resp = std::make_shared<RIL_Errno>(ret);
    reqMsg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleSetDataProfileMessage_v15(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestSetDataProfileMessage_v15> m = std::static_pointer_cast<RilRequestSetDataProfileMessage_v15>(msg);
  if ( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_request_set_data_profile(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

#else
void DataModule::handleSetLteAttachProfileMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<SetLteAttachProfileMessage> reqMsg = std::static_pointer_cast<SetLteAttachProfileMessage>(msg);
  if( reqMsg != NULL ) {
    RIL_Errno ret = qcril_data_request_set_lte_attach_profile ( reqMsg->get_params() );
    auto resp = std::make_shared<RIL_Errno>(ret);
    reqMsg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleSetDataProfileMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestSetDataProfileMessage> m = std::static_pointer_cast<RilRequestSetDataProfileMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_request_set_data_profile(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}
#endif /* #if QCRIL_RIL_VERSION >= 15 */

void DataModule::handleSetQualityMeasurementMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<SetQualityMeasurementMessage> reqMsg = std::static_pointer_cast<SetQualityMeasurementMessage>(msg);
  if( reqMsg != NULL ) {
    dsd_set_quality_measurement_info_req_msg_v01 info = reqMsg->getInfo();
    qmi_response_type_v01 ret= qcril_data_set_quality_measurement(&info);
    auto resp = std::make_shared<qmi_response_type_v01>(ret);
    reqMsg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleSetRatPrefMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<SetRatPrefMessage> m = std::static_pointer_cast<SetRatPrefMessage>(msg);
  if( m != NULL ) {
    RIL_Errno ret = qcril_data_set_rat_preference(m->ratPrefType);
    auto resp = std::make_shared<RIL_Errno>(ret);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleSetupDataCallMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<RilRequestSetupDataCallMessage> m = std::static_pointer_cast<RilRequestSetupDataCallMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_request_setup_data_call(&req, &ret);
    // We don't return response here. It will be sent upon
    // success/failure at a later point in time.
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleStartLceMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestStartLceMessage> m = std::static_pointer_cast<RilRequestStartLceMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_lqe_start(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleStopLceMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestStopLceMessage> m = std::static_pointer_cast<RilRequestStopLceMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_lqe_stop(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleToggleDormancyIndMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<ToggleDormancyIndMessage> m = std::static_pointer_cast<ToggleDormancyIndMessage>(msg);
  if( m != NULL ) {
    int ret = qcril_data_toggle_dormancy_indications(m->dormIndSwitch);
    Message::Callback::Status status = (ret == QCRIL_DS_SUCCESS ?
             Message::Callback::Status::SUCCESS : Message::Callback::Status::FAILURE);
    auto resp = std::make_shared<int>(ret);
    m->sendResponse(msg, status, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleToggleLimitedSysIndMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<ToggleLimitedSysIndMessage> m = std::static_pointer_cast<ToggleLimitedSysIndMessage>(msg);
  if( m != NULL ) {
    int ret = qcril_data_toggle_limited_sys_indications(m->sysIndSwitch);
    Message::Callback::Status status = (ret == QCRIL_DS_SUCCESS ?
             Message::Callback::Status::SUCCESS : Message::Callback::Status::FAILURE);
    auto resp = std::make_shared<int>(ret);
    m->sendResponse(msg, status, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleUpdateMtuMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<UpdateMtuMessage> m = std::static_pointer_cast<UpdateMtuMessage>(msg);
  if( m != NULL ) {
    qcril_data_update_mtu(m->Mtu);
    auto resp = std::make_shared<int>(QCRIL_DS_SUCCESS);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleDataEmbmsActiveMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilDataEmbmsActiveMessage> m = std::static_pointer_cast<RilDataEmbmsActiveMessage>(msg);
  if( m != NULL ) {
    int is_Embms_Active = qcril_data_is_embms_active();
    auto resp = std::make_shared<bool>(is_Embms_Active);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleGetDdsSubIdMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<GetDdsSubIdMessage> m = std::static_pointer_cast<GetDdsSubIdMessage>(msg);
  if( m != NULL ) {
    DDSSubIdInfo ddsInfo = qcril_data_get_dds_sub_info();

    LOCK_MUTEX(ddsSubMutex);
    currentDDSSUB.dds_sub_id = ddsInfo.dds_sub_id;
    currentDDSSUB.switch_type = ddsInfo.switch_type;
    UNLOCK_MUTEX(ddsSubMutex);
    Log::getInstance().d("[" + mName + "]:Current DDS is on SUB ="+std::to_string(currentDDSSUB.dds_sub_id)+
                         "switch type = "+std::to_string(currentDDSSUB.switch_type));

    auto resp = std::make_shared<DDSSubIdInfo>(ddsInfo);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleDataRequestDDSSwitchMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RequestDdsSwitchMessage> m = std::static_pointer_cast<RequestDdsSwitchMessage>(msg);
  if( m != NULL ) {
    RIL_Errno ret = qcril_data_request_dds_switch(m->dds_sub_info);
    auto resp = std::make_shared<RIL_Errno>(ret);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

#if (QCRIL_RIL_VERSION >= 15)
void DataModule::handleStartKeepaliveMessage(std::shared_ptr<Message> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    std::shared_ptr<RilRequestStartKeepaliveMessage> m = std::static_pointer_cast<RilRequestStartKeepaliveMessage>(msg);
    if( m != NULL ) {
      qcril_request_params_type req = m->get_params();
      qcril_request_return_type ret;
      qcril_data_start_modem_assist_keepalive(&req, &ret);
    } else {
      Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
    }
}

void DataModule::handleStopKeepaliveMessage(std::shared_ptr<Message> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    std::shared_ptr<RilRequestStopKeepaliveMessage> m = std::static_pointer_cast<RilRequestStopKeepaliveMessage>(msg);
    if( m != NULL ) {
      qcril_request_params_type req = m->get_params();
      qcril_request_return_type ret;
      qcril_data_stop_modem_assist_keepalive(&req, &ret);
    } else {
      Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
    }
}

void DataModule::handleSetCarrierInfoImsiEncryptionMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  std::shared_ptr<RilRequestSetCarrierInfoImsiEncryptionMessage> m = std::static_pointer_cast<RilRequestSetCarrierInfoImsiEncryptionMessage>(msg);

  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    auth_manager->setCarrierInfoImsiEncryption(&req);

    qcril_request_resp_params_type resp;
    resp.instance_id        = QCRIL_DEFAULT_INSTANCE_ID;
    resp.t                  = req.t;
    resp.request_id         = req.event_id;
    resp.request_id_android = RIL_REQUEST_SET_CARRIER_INFO_IMSI_ENCRYPTION;
    resp.ril_err_no         = RIL_E_SUCCESS;
    resp.resp_pkt           = NULL;
    resp.resp_len           = 0;
    resp.logstr             = NULL;
    resp.rild_sock_oem_req  = 0;
    qcril_send_request_response( &resp );
  }else {
    Log::getInstance().d("[" + mName + "]: Improper message received");
  }
}

void DataModule::handleQmiAuthServiceIndMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  auto shared_indMsg(std::static_pointer_cast<QmiIndMessage>(msg));
  QmiIndMsgDataStruct *indData = shared_indMsg->getData();

  if (indData != nullptr) {
    auth_manager->qmiAuthServiceIndicationHandler(indData->msgId, indData->indData,
        indData->indSize);
  } else {
    Log::getInstance().d("[" + mName + "] Unexpected, null data from message");
  }
}

void DataModule::handleQmiAuthEndpointStatusIndMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  auto shared_indMsg(std::static_pointer_cast<EndpointStatusIndMessage>(msg));

  if (shared_indMsg->getState() == ModemEndPoint::State::OPERATIONAL) {
     if (!mAuthServiceReady) {
       mAuthServiceReady = true;
       auth_manager->qmiAuthServiceRegisterIndications(true);
       auth_manager->updateModemWithCarrierImsiKeyCache();
     }
  }
  else {
    mAuthServiceReady = false;
    Log::getInstance().d("[" + mName + "]: ModemEndPoint is not operational");
  }
}

void DataModule::handleQmiDsdEndpointStatusIndMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  auto shared_indMsg(std::static_pointer_cast<EndpointStatusIndMessage>(msg));

  if (shared_indMsg->getState() == ModemEndPoint::State::OPERATIONAL) {
    ModemEndPointFactory<DSDModemEndPoint>::getInstance().buildEndPoint()->registerForUiChangeInd();
  }
  else {
    Log::getInstance().d("[" + mName + "]: ModemEndPoint is not operational");
  }
}

void DataModule::handleGetDataNrIconType(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  auto m = std::static_pointer_cast<GetDataNrIconTypeMessage>(msg);
  if (m != NULL)
  {
    dsd_sys_ui_mask_v01 result;
    Message::Callback::Status status = ModemEndPointFactory<DSDModemEndPoint>::getInstance()
                                            .buildEndPoint()->getUiInfoSync(result);
    std::shared_ptr<NrIconType_t> resp = nullptr;
    if (status == Message::Callback::Status::SUCCESS) {
      if (0 != (QMI_DSD_UI_MASK_3GPP_5G_UWB_V01 & result)) {
        Log::getInstance().d("handleGetDataNrIconType icontype UWB");
        resp = std::make_shared<NrIconType_t>(rildata::NrIconEnum_t::UWB);
      } else if (0 != (QMI_DSD_UI_MASK_3GPP_5G_BASIC_V01 & result)) {
        Log::getInstance().d("handleGetDataNrIconType icontype BASIC");
        resp = std::make_shared<NrIconType_t>(rildata::NrIconEnum_t::BASIC);
      } else {
        Log::getInstance().d("handleGetDataNrIconType icontype NONE");
      }
    }
    if (resp == nullptr) {
      resp = std::make_shared<NrIconType_t>(rildata::NrIconEnum_t::NONE);
    }
    m->sendResponse(msg, status, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleSetLinkCapFilterMessage(std::shared_ptr<Message> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    std::shared_ptr<SetLinkCapFilterMessage> m = std::static_pointer_cast<SetLinkCapFilterMessage>(msg);
    if( m != NULL ) {
      auto rf = m->getParams();
      int result = -1;
      if (lceHandler.toggleReporting(rf)) {
        result = 0;
      }
      auto resp = std::make_shared<int>(result);
      m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
    } else {
      Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
    }
}

void DataModule::handleSetLinkCapRptCriteriaMessage(std::shared_ptr<Message> msg)
{
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    std::shared_ptr<SetLinkCapRptCriteriaMessage> m = std::static_pointer_cast<SetLinkCapRptCriteriaMessage>(msg);
    if( m != NULL ) {
      auto rf = m->getParams();
      Message::Callback::Status status = Message::Callback::Status::SUCCESS;
      LinkCapCriteriaResult_t result = lceHandler.setCriteria(rf);
      if (result != rildata::LinkCapCriteriaResult_t::success) {
        status = Message::Callback::Status::FAILURE;
      }
      auto resp = std::make_shared<rildata::LinkCapCriteriaResult_t>(result);
      m->sendResponse(msg, status, resp);
    } else {
      Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
    }
}
#endif


/*===========================================================================

  FUNCTION:  handleSetInitialAttachApn

===========================================================================*/
/*!
    @brief
    Handler to handle SetInitialAttachApnRequestMessage message request

    @return
*/
/*=========================================================================*/
void DataModule::handleSetInitialAttachApn(std::shared_ptr<Message> msg)
{
  RIL_Errno  res = RIL_E_SUCCESS;
  qcril_request_resp_params_type    resp;
  qcril_reqlist_public_type       qcril_req_info;
  IxErrnoType reqlist_status = E_SUCCESS;
  std::shared_ptr<SetInitialAttachApnRequestMessage> m =
       std::static_pointer_cast<SetInitialAttachApnRequestMessage>(msg);

  memset(&resp,0,sizeof(qcril_request_resp_params_type));
  memset(&qcril_req_info,0,sizeof(qcril_reqlist_public_type));
  if (m!= nullptr)
  {
    Log::getInstance().d("[DataModule]: Handling msg = " + m->dump());
    const qcril_request_params_type *params_ptr = m->get_params();
    if( params_ptr == NULL )
    {
      Log::getInstance().d("ERROR!!params_ptr is NULL . Returning error response to telephony");
      return;
    }
    do
    {
      Log::getInstance().d("[DataModule]: Inserting"
             " QCRIL_EVT_QMI_REQUEST_INIT_ATTACH_APN event in reqlist ");
      qcril_reqlist_default_entry( params_ptr->t,
                                   params_ptr->event_id,
                                   params_ptr->modem_id,
                                   QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                                   QCRIL_EVT_QMI_REQUEST_INIT_ATTACH_APN,
                                   NULL,
                                   &qcril_req_info );
      if (( reqlist_status = qcril_reqlist_new( params_ptr->instance_id,
                                       &qcril_req_info )) != E_SUCCESS)
      {
        Log::getInstance().d("Reqlist entry failure..status: "+ std::to_string(reqlist_status));
        res = map_internalerr_from_reqlist_new_to_ril_err(reqlist_status);
        break;
      }
      ProfileHandler::RilRespParams resp_params{ params_ptr->instance_id,
                                                 params_ptr->modem_id,
                                                 params_ptr->event_id,
                                                 params_ptr->t
                                               };
      Log::getInstance().d("[DataModule]: Handling token = " + std::to_string((long long)params_ptr->t)) ;
      if( profile_handler )
      {
        profile_handler->handleInitialAttachRequest(m->get_attach_params(), resp_params);
      } else
      {
        Log::getInstance().d("[DataModule]: Profile Handler is NULL. Returning ERROR response") ;
        qcril_default_request_resp_params( params_ptr->instance_id,
                                           params_ptr->t,
                                           params_ptr->event_id,
                                           RIL_E_INTERNAL_ERR,
                                           &resp );
        qcril_send_request_response( &resp );
      }
    } while(0);

    if (res != RIL_E_SUCCESS)
    {
      Log::getInstance().d("[DataModule]:handleSetInitialAttachApn:"
                           " Sending Error response");
      qcril_default_request_resp_params( params_ptr->instance_id,
                                         params_ptr->t,
                                         params_ptr->event_id,
                                         res,
                                         &resp );
      qcril_send_request_response( &resp );
    }
  } else
  {
    Log::getInstance().d("[" + mName + "]: Improper message received for handleSetInitialAttachApn");
  }
}

/*===========================================================================

  FUNCTION:  handleSetLteAttachPdnListActionResult

===========================================================================*/
/*!
    @brief
    Handler to handle SetInitialAttachApnRequestMessage message request

    @return
*/
/*=========================================================================*/
void DataModule::handleSetLteAttachPdnListActionResult(std::shared_ptr<Message> msg)
{
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    std::shared_ptr<SetLteAttachPdnListActionResultMessage> m = std::static_pointer_cast<SetLteAttachPdnListActionResultMessage>(msg);
    if( m != NULL && profile_handler != NULL ) {
      Log::getInstance().d("[DataModule]::Invoking handleWdsUnSolInd" );
      profile_handler->handleWdsUnSolInd(&(m->getParams()));
    } else {
      Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
    }
}

/*===========================================================================

  FUNCTION:  handleNasSrvDomainPrefInd

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when NasSrvDomainPrefIndMessage is received

    @return
*/
/*=========================================================================*/
void DataModule::handleNasSrvDomainPrefInd(std::shared_ptr<Message> m)
{
  Log::getInstance().d("[DataModule]: Handling msg = " + m->dump());
  std::shared_ptr<NasSrvDomainPrefIndMessage> msg = std::static_pointer_cast<NasSrvDomainPrefIndMessage>(m);
  if (msg != NULL)
  {
    uint8_t domainPrefValid;
    nas_srv_domain_pref_enum_type_v01 domainPref;
    msg->getDomainPref(domainPrefValid,domainPref);
    if(profile_handler)
    {
      Log::getInstance().d("[DataModule]::Invoking qcril_data_nas_detach_attach_ind_hdlr" );
      profile_handler->qcril_data_nas_detach_attach_ind_hdlr(domainPrefValid, domainPref);
    } else
    {
      Log::getInstance().d("[DataModule]::Invalid nas_helper object. Returning");
    }
    Log::getInstance().d("[DataModule]::handleNasSrvDomainPrefInd EXIT" );
  }
}

/*===========================================================================

  FUNCTION:  handleNasRequestDataShutdown

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when NasRequestDataShutdownMessage is received

    @return
*/
/*=========================================================================*/
void DataModule::handleNasRequestDataShutdown(std::shared_ptr<Message> msg)
{
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());
  std::shared_ptr<NasRequestDataShutdownMessage> m = std::static_pointer_cast<NasRequestDataShutdownMessage>(msg);
  if (m != NULL)
  {
    NasRequestDataShutdownResponse ret = NasRequestDataShutdownResponse::FAILURE;
    if(qcril_data_device_shutdown()) {
      ret = NasRequestDataShutdownResponse::SUCCESS;
    }
    auto resp = std::make_shared<NasRequestDataShutdownResponse>(ret);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

/*===========================================================================

  FUNCTION:  handleDataAllBearerTypeUpdate

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when DataBearerTypeChangedMessage is received

    @return
*/
/*=========================================================================*/
void DataModule::handleDataAllBearerTypeUpdate(std::shared_ptr<Message> msg)
{
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());
  std::shared_ptr<DataAllBearerTypeChangedMessage> m = std::static_pointer_cast<DataAllBearerTypeChangedMessage>(msg);
  if (m != NULL)
  {
    AllocatedBearer_t bearerInfo = m->getBearerInfo();
    network_service_handler->handleDataAllBearerTypeUpdate(bearerInfo);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

/*===========================================================================

  FUNCTION:  handleDataBearerTypeUpdate

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when DataBearerTypeChangedMessage is received

    @return
*/
/*=========================================================================*/
void DataModule::handleDataBearerTypeUpdate(std::shared_ptr<Message> msg)
{
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());
  std::shared_ptr<DataBearerTypeChangedMessage> m = std::static_pointer_cast<DataBearerTypeChangedMessage>(msg);
  if (m != NULL)
  {
    int32_t cid = m->getCid();
    BearerInfo_t bearer = m->getBearerInfo();
    network_service_handler->handleDataBearerTypeUpdate(cid, bearer);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

/*===========================================================================

  FUNCTION:  handleToggleBearerAllocationUpdate

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when RegisterBearerAllocationUpdateRequestMessage is received

    @return
*/
/*=========================================================================*/
void DataModule::handleToggleBearerAllocationUpdate(std::shared_ptr<Message> msg)
{
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());

  std::shared_ptr<RegisterBearerAllocationUpdateRequestMessage> m =
    std::static_pointer_cast<RegisterBearerAllocationUpdateRequestMessage>(msg);
  if (m != NULL)
  {
    bool enable = m->getToggleSwitch();
    ResponseError_t ret = network_service_handler->handleToggleBearerAllocationUpdate(enable);
    auto resp = std::make_shared<ResponseError_t>(ret);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

/*===========================================================================

  FUNCTION:  handleGetBearerAllocation

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when GetBearerAllocationRequestMessage is received
    Invokes the callback with the allocated bearers that were retrieved

    @return
*/
/*=========================================================================*/
void DataModule::handleGetBearerAllocation(std::shared_ptr<Message> msg)
{
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());

  std::shared_ptr<GetBearerAllocationRequestMessage> m =
    std::static_pointer_cast<GetBearerAllocationRequestMessage>(msg);
  if (m != NULL)
  {
    int32_t cid = m->getCallId();
    Message::Callback::Status status = Message::Callback::Status::SUCCESS;
    AllocatedBearerResult_t bearerAllocations = network_service_handler->handleGetBearerAllocation(cid);
    auto resp = std::make_shared<AllocatedBearerResult_t>(bearerAllocations);
    m->sendResponse(msg, status, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

RIL_Errno qcrilDataUimGetCardStatus
(
  uint8_t             slot,
  RIL_CardStatus_v6 * ril_card_status /* out ptr */
)
{
  std::shared_ptr<UimGetCardStatusRequestSyncMsg> card_status_ptr = nullptr;
  std::shared_ptr<RIL_Errno>                      respPtr         = nullptr;

  card_status_ptr = std::make_shared<UimGetCardStatusRequestSyncMsg>(slot,
                                                                     ril_card_status);
  if (card_status_ptr != nullptr)
  {
    if (card_status_ptr->dispatchSync(respPtr) == Message::Callback::Status::SUCCESS)
    {
      if (respPtr != nullptr)
      {
        Log::getInstance().d("qcrilDataUimGetCardStatus:: returning response");
        return *respPtr;
      }
    }
  }
  Log::getInstance().d("qcrilDataUimGetCardStatus:: returning generic failure");
  return RIL_E_GENERIC_FAILURE;
}

void qcrilDataUimFreeAidAndLabelInfo
(
  RIL_CardStatus_v6      * ril_card_status_ptr
)
{
  int app_index = 0;
  int max_apps = 0;

  Log::getInstance().d("qcrilDataUimFreeAidAndLabelInfo:: ENTRY");
  if (ril_card_status_ptr == NULL)
  {
    Log::getInstance().d("Invalid input, cannot proceed");
    return;
  }

  max_apps = (ril_card_status_ptr->num_applications <= RIL_CARD_MAX_APPS)
               ? ril_card_status_ptr->num_applications : RIL_CARD_MAX_APPS;

  /* Loop through all the apps and free buffers allocated */
  for (app_index = 0; app_index < max_apps; app_index++)
  {
    if(ril_card_status_ptr->applications[app_index].aid_ptr != NULL)
    {
      qcril_free(ril_card_status_ptr->applications[app_index].aid_ptr);
      ril_card_status_ptr->applications[app_index].aid_ptr = NULL;
    }
    if(ril_card_status_ptr->applications[app_index].app_label_ptr != NULL)
    {
      qcril_free(ril_card_status_ptr->applications[app_index].app_label_ptr);
      ril_card_status_ptr->applications[app_index].app_label_ptr = NULL;
    }
  }
} /* qcril_uim_free_aid_and_label_info */

/*===========================================================================

    qcrilDataUimEventAppStatusUpdate

============================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_CM_CARD_APP_STATUS_CHANGED

    @return
    None
*/
/*=========================================================================*/
void qcrilDataUimEventAppStatusUpdate
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_card_app_info_type       *card_app_info;
  RIL_CardStatus_v6               ril_card_status;
  qcril_uim_get_mcc_mnc_req_type  get_mcc_mnc_req;
  char                           *aid = NULL;
  RIL_AppType                     request_app_type;
  qcril_modem_id_e_type           modem_id;

  Log::getInstance().d("qcrilDataUimEventAppStatusUpdate:: ENTRY");
  QCRIL_NOTUSED(ret_ptr);

  if (!params_ptr)
  {
    Log::getInstance().d("PARAMS ptr is NULL");
    return;
  }
  memset(&get_mcc_mnc_req, 0, sizeof(get_mcc_mnc_req));
  modem_id      = params_ptr->modem_id;
  card_app_info = (qcril_card_app_info_type *)(params_ptr->data);

  /* Process only this slots SIM card applications */
  if (card_app_info != NULL &&
      card_app_info->slot == qmi_ril_get_sim_slot() &&
      card_app_info->app_state == QMI_UIM_APP_STATE_READY)
  {
    Log::getInstance().d("app type"+std::to_string(card_app_info->app_type)+
                         "app state"+std::to_string(card_app_info->app_state));
    /* retrieve card status info */
    if (qcrilDataUimGetCardStatus(
        qmi_ril_get_process_instance_id(), &ril_card_status)
        != RIL_E_SUCCESS)
    {
      Log::getInstance().d("Get card status failed");
      return;
    }
    /* retrieve aid from card status */
    if (qcril_data_retrieve_aid_from_card_status(&ril_card_status,
                                                 get_mcc_mnc_req.aid_buffer, &request_app_type)
                                                 != E_SUCCESS)
    {
      Log::getInstance().d("Retrieval of AID from card status failed");
      qcrilDataUimFreeAidAndLabelInfo(&ril_card_status);
      return;
    }

    std::string str= get_mcc_mnc_req.aid_buffer;
    Log::getInstance().d("Received SIM aid_buffer="+str);
    qcril_uim_app_type  app_type = QCRIL_UIM_APP_UNKNOWN;

    aid = (char *)qcril_malloc(QMI_UIM_MAX_AID_LEN+1);
    //proceed only when memory is allocated
    if(!aid)
    {
      Log::getInstance().d("AID Memory allocation failed");
      qcrilDataUimFreeAidAndLabelInfo(&ril_card_status);
      return;
    }
    memset(aid, 0x00, QMI_UIM_MAX_AID_LEN + 1);

    if (strlen(get_mcc_mnc_req.aid_buffer))
    {
      strlcpy(aid,get_mcc_mnc_req.aid_buffer, QMI_UIM_MAX_AID_LEN+1);
    }
    switch(request_app_type)
    {
      case RIL_APPTYPE_SIM:
        app_type = QCRIL_UIM_APP_SIM;
        break;
     case RIL_APPTYPE_USIM:
        app_type = QCRIL_UIM_APP_USIM;
        break;
     case RIL_APPTYPE_RUIM:
        app_type = QCRIL_UIM_APP_RUIM;
        break;
     case RIL_APPTYPE_CSIM:
        app_type = QCRIL_UIM_APP_CSIM;
        break;
     default:
        app_type = QCRIL_UIM_APP_UNKNOWN;
        break;
     }

     DataGetMccMncCallback Cb("set-cb-token");
     std::shared_ptr<UimGetMccMncRequestMsg> req =
     std::make_shared<UimGetMccMncRequestMsg>(aid, app_type, &Cb);
     if(req)
     {
       Log::getInstance().d("Dispatching UimGetMccMncRequestMsg Message");
       req->dispatch();
     }
       qcril_free(aid);
       qcrilDataUimFreeAidAndLabelInfo(&ril_card_status);
   } else {
       Log::getInstance().d("Card APP info is NULL or slot id mismatch or Card APP status isn't READY");
   }
}

void DataModule::handleUimCardAppStatusIndMsg(std::shared_ptr<Message> m)
{
  qcril_request_return_type ret_ptr;
  qcril_request_params_type params_ptr;
  Log::getInstance().d("[DataModule]: Handling msg = " + m->dump());

  std::shared_ptr<UimCardAppStatusIndMsg> msg =
       std::static_pointer_cast<UimCardAppStatusIndMsg>(m);
  std::memset(&params_ptr, 0, sizeof(params_ptr));
  std::memset(&ret_ptr, 0, sizeof(ret_ptr));

  if( msg != NULL )
  {
    params_ptr.data = static_cast<void *>(new char[sizeof(qcril_card_app_info_type)+1]);
    if(params_ptr.data)
    {
      std::memcpy(params_ptr.data, msg->get_app_info(), sizeof(qcril_card_app_info_type));
      params_ptr.datalen = sizeof(qcril_card_app_info_type);
      params_ptr.modem_id = QCRIL_DEFAULT_MODEM_ID;

      qcrilDataUimEventAppStatusUpdate (&params_ptr, &ret_ptr);
    } else
    {
      Log::getInstance().d("[DataModule]: Memory allocation failure");
    }
  } else
  {
    Log::getInstance().d("[" + mName + "]: Improper message received");
  }
}

/*===========================================================================

  FUNCTION:  handleGetAllBearerAllocations

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when GetAllBearerAllocationsRequestMessage is received
    Invokes the callback with the allocated bearers that were retrieved

    @return
*/
/*=========================================================================*/
void DataModule::handleGetAllBearerAllocations(std::shared_ptr<Message> msg)
{
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());

  std::shared_ptr<GetBearerAllocationRequestMessage> m =
    std::static_pointer_cast<GetBearerAllocationRequestMessage>(msg);
  if (m != NULL)
  {
    Message::Callback::Status status = Message::Callback::Status::SUCCESS;
    AllocatedBearerResult_t bearerAllocations = network_service_handler->handleGetAllBearerAllocations();
    auto resp = std::make_shared<AllocatedBearerResult_t>(bearerAllocations);
    m->sendResponse(msg, status, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

/*===========================================================================

  FUNCTION:  handleQmiWdsEndpointStatusIndMessage

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when QMI WDS modem endpoint status is changed

    @return
*/
/*=========================================================================*/
void DataModule::handleQmiWdsEndpointStatusIndMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  auto shared_indMsg(std::static_pointer_cast<EndpointStatusIndMessage>(msg));

  if (shared_indMsg->getState() == ModemEndPoint::State::NON_OPERATIONAL) {
    network_service_handler->cleanUpAllBearerAllocation();
    Log::getInstance().d("[" + mName + "]: WDSModemEndPoint is not operational");
  }
}

/*===========================================================================

  FUNCTION:  handleDataConnectionStateChangedMessage

===========================================================================*/
/*!
    @brief
    Handler when data connection state is changed

    @return
*/
/*=========================================================================*/
void DataModule::handleDataConnectionStateChangedMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<CallStatusMessage> m = std::static_pointer_cast<CallStatusMessage>(msg);
  if (m != NULL)
  {
    const CallStatusParams callParams = m->getCallParams();
    if(callParams.evt == QCRIL_DATA_EVT_CALL_RELEASED) {
      network_service_handler->cleanUpBearerAllocation((int32_t)m->getCallId());
    }
  }
}

/*===========================================================================

  FUNCTION:  map_internalerr_from_reqlist_new_to_ril_err

===========================================================================*/
/*!
    @brief
    Helper API to convert data of IxErrnoType type to RIL_Errno type
    input: data of IxErrnoType type

    @return
*/
/*=========================================================================*/
RIL_Errno DataModule::map_internalerr_from_reqlist_new_to_ril_err(IxErrnoType error)
{
  RIL_Errno ret;
  switch(error)
  {
    case E_SUCCESS:
      ret = RIL_E_SUCCESS;
      break;
    case E_NOT_ALLOWED:
      ret = RIL_E_INVALID_STATE; //needs to be changed after internal discussion
      break;
    case E_NO_MEMORY:
      ret = RIL_E_NO_MEMORY;
      break;
    case E_NO_RESOURCES:
      ret = RIL_E_NO_RESOURCES;
      break;
    case E_RADIO_NOT_AVAILABLE:
      ret = RIL_E_RADIO_NOT_AVAILABLE;
      break;
    case E_INVALID_ARG:
      ret = RIL_E_INVALID_ARGUMENTS;
      break;
    default:
      ret = RIL_E_INTERNAL_ERR;
      break;
  }
  return ret;
}

/*===========================================================================

  FUNCTION:  setTimeoutForMsg

===========================================================================*/
/*!
    @brief
    API to set the timeout for message

    @return
    TimeKeeper::timer_id value
*/
/*=========================================================================*/
TimeKeeper::timer_id DataModule::setTimeoutForMsg
(
  std::shared_ptr<Message> msg, TimeKeeper::millisec maxTimeout
)
{
  if( msg == NULL ) {
    Log::getInstance().d("[" + mName + "]: ERROR!!! Msg received is NULL");
    return 0; /*'0' is the init value of timer_id parameter */
  }
  Log::getInstance().d("[DataModule: set timeout for "+msg->dump());

  TimeKeeper::timer_id tid = TimeKeeper::getInstance().set_timer(
      [this, msg](void* user_data) {

        QCRIL_NOTUSED(user_data);

        if (!(msg->isCallbackExecuting() || msg->isCallbackExecuted()))
        {
          Log::getInstance().d("[DataModule:: Timer expired for " +
                                     msg->dump());
          msg->cancelling();
          Log::getInstance().d("[DataModule]: calling dispatcher inform fun");
          Dispatcher::getInstance().informMessageDispatchFailure(
              msg, Message::Callback::Status::TIMEOUT);

          Log::getInstance().d("Finished");
          msg->cancelled();

          deleteEntryInReqlist(msg);
        }
  },
  nullptr,
  maxTimeout);

  Log::getInstance().d("[" + msg->to_string() + "]: timer_id = " + std::to_string(tid));
  msg->setTimerId(tid);
  return tid;
}

/*===========================================================================

  FUNCTION:  deleteEntryInReqlist

===========================================================================*/
/*!
    @brief
    API to delete request from reqlist
===========================================================================*/
/*!
    @brief
    API to delete request from reqlist

    @return
    None
*/
/*=========================================================================*/
void DataModule::deleteEntryInReqlist
    (
    std::shared_ptr<Message> msg
    )
    {
/* Remove entry from reqlist */
std::shared_ptr<RilRequestDeactivateDataCallMessage> m = std::static_pointer_cast<RilRequestDeactivateDataCallMessage>(msg);

if( m != NULL ) {
	qcril_request_params_type req = m->get_params();
	IxErrnoType reqlistErrno = qcril_data_reqlist_free(&req);

	if( reqlistErrno == E_SUCCESS)
	{
		Log::getInstance().d("qcril_data_deactivate_timeout_handle::Reqlist Free SUCCESS");
	}
	else
	{
		Log::getInstance().d("qcril_data_deactivate_timeout_handler::Reqlist Free failed!!! with Error code "+ std::to_string(reqlistErrno));
	}
} else {
	Log::getInstance().d("[" + mName + "]: Message received is not DeactivateDataCall message!!!");
}
}

/*===========================================================================

FUNCTION:  clearTimeoutForMsg

===========================================================================*/
/*!
  @brief
  API to clear the existing timeout for message

    @return
    TimeKeeper::timer_id value
*/
/*=========================================================================*/
bool clearTimeoutForMessage
(
  std::shared_ptr<Message> msg
)
{
  return TimeKeeper::getInstance().clear_timer(msg->getTimerId());
}

/*============================================================================

    qcrilDataprocessMccMncInfo

============================================================================*/
/*!
    @brief
    Process mcc mnc info

    @return
    None
*/
/*=========================================================================*/
void qcrilDataprocessMccMncInfo
(
    const qcril_request_params_type *const params_ptr,
    qcril_request_return_type       *const ret_ptr
)
{
  qcril_mcc_mnc_info_type *uim_mcc_mnc_info = NULL;

  Log::getInstance().d("qcrilDataprocessMccMncInfo: ENTRY");

  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    Log::getInstance().d("ERROR!! Invalid input, cannot process request");
    return;
  }

  uim_mcc_mnc_info = (qcril_mcc_mnc_info_type*)params_ptr->data;
  if (uim_mcc_mnc_info == NULL)
  {
    Log::getInstance().d("NULL uim_mcc_mnc_info");
    return;
  }

  if (uim_mcc_mnc_info->err_code != RIL_E_SUCCESS)
  {
    Log::getInstance().d("uim_get_mcc_mnc_info error:"+ std::to_string(uim_mcc_mnc_info->err_code));
    return;
  }

  //According to the declaration of size in 'UimGetMccMncRequestMsg.h'
  //each of mcc & mnc is 4 bytes, adding the error check based on this size

  if ( (uim_mcc_mnc_info->mcc[MCC_LENGTH - 1] != '\0')
    || (uim_mcc_mnc_info->mnc[MNC_LENGTH - 1] != '\0') )
  {
    Log::getInstance().d("ERROR!! Improper input received. Either of MCC or MNC is not NULL terminated");
    return;
  }
  std::string str = uim_mcc_mnc_info->mcc;
  std::string str1 = uim_mcc_mnc_info->mnc;
  Log::getInstance().d("mcc:"+ str+"mnc="+ str1);

  qdp_compare_and_update(uim_mcc_mnc_info->mcc,
                                 uim_mcc_mnc_info->mnc);
}

}//namespace
