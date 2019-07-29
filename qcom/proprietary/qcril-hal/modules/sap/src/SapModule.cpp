/******************************************************************************
#  Copyright (c) 2018 - 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#ifndef QMI_RIL_UTF
#define LOG_TAG "RIL_SAP"

#include <android/hardware/radio/1.2/ISap.h>

#include <hwbinder/IPCThreadState.h>
#include <hwbinder/ProcessState.h>

#include <framework/ModuleLooper.h>
#include "SapModule.h"

using namespace android::hardware::radio::V1_0;
using ::android::hardware::Return;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_array;
using ::android::hardware::Void;
using ::android::hardware::hidl_death_recipient;
using android::sp;
using android::RWLock;

struct SapImpl;

static load_module<SapModule>    sap_module;
static android::sp<SapImpl>      sapService;
static qmi_uim_sap_status_type   qcril_sap_state;


struct SapImpl : public android::hardware::radio::V1_2::ISap, public hidl_death_recipient
{
  sp<ISapCallback> sapCallback;

  RWLock mCallbackLock;

  sp<ISapCallback> getCallback();

  void clearCallbacks();

  void serviceDied( uint64_t, const ::android::wp<::android::hidl::base::V1_0::IBase> &);

  Return<void> setCallback(const ::android::sp<ISapCallback>& sapCallbackParam);

  Return<void> connectReq(int32_t token, int32_t maxMsgSize);

  Return<void> disconnectReq(int32_t token);

  Return<void> apduReq(int32_t token, SapApduType type, const hidl_vec<uint8_t>& command);

  Return<void> transferAtrReq(int32_t token);

  Return<void> powerReq(int32_t token, bool state);

  Return<void> resetSimReq(int32_t token);

  Return<void> transferCardReaderStatusReq(int32_t token);

  Return<void> setTransferProtocolReq(int32_t token, SapTransferProtocol transferProtocol);

  void sendFailedResponse(UimSapRequestType msgId, int32_t token, int err);

  SapResultCode sap_convert_err_to_sap_err(UimSapRequestType msg_id, int err);

  void checkReturnStatus(Return<void>& ret);
};


/*=========================================================================

FUNCTION:  SapImpl::sap_convert_err_to_sap_err

===========================================================================*/
SapResultCode SapImpl::sap_convert_err_to_sap_err
(
  UimSapRequestType msg_id,
  int               err_code
)
{
  SapResultCode err = SapResultCode::GENERIC_FAILURE;

  switch (msg_id)
  {
    case UIM_SAP_APDU_REQ:
      switch(err_code)
      {
        case QCRIL_UIM_SAP_SUCCESS:
          err = SapResultCode::SUCCESS;
          break;
        case QCRIL_UIM_SAP_CARD_NOT_ACCESSSIBLE:
          err = SapResultCode::CARD_NOT_ACCESSSIBLE;
          break;
        case QCRIL_UIM_SAP_CARD_ALREADY_POWERED_OFF:
          err = SapResultCode::CARD_ALREADY_POWERED_OFF;
          break;
        case QCRIL_UIM_SAP_CARD_REMOVED:
          err = SapResultCode::CARD_REMOVED;
          break;
        default:
          err = SapResultCode::GENERIC_FAILURE;
      }
      break;

    case UIM_SAP_TRANSFER_ATR_REQ:
      switch(err_code)
      {
        case QCRIL_UIM_SAP_SUCCESS:
          err = SapResultCode::SUCCESS;
          break;
        case QCRIL_UIM_SAP_CARD_NOT_ACCESSSIBLE:
          err = SapResultCode::DATA_NOT_AVAILABLE;
          break;
        case QCRIL_UIM_SAP_CARD_ALREADY_POWERED_OFF:
          err = SapResultCode::CARD_ALREADY_POWERED_OFF;
          break;
        case QCRIL_UIM_SAP_CARD_REMOVED:
          err = SapResultCode::CARD_REMOVED;
          break;
        default:
          err = SapResultCode::GENERIC_FAILURE;
      }
      break;

    case UIM_SAP_POWER_REQ:
      switch(err_code)
      {
        case QCRIL_UIM_SAP_SUCCESS:
          err = SapResultCode::SUCCESS;
          break;
        case QCRIL_UIM_SAP_CARD_NOT_ACCESSSIBLE:
          err = SapResultCode::CARD_NOT_ACCESSSIBLE;
          break;
        case QCRIL_UIM_SAP_CARD_ALREADY_POWERED_OFF:
          err = SapResultCode::CARD_ALREADY_POWERED_OFF;
          break;
        case QCRIL_UIM_SAP_CARD_ALREADY_POWERED_ON:
          err = SapResultCode::CARD_ALREADY_POWERED_ON;
          break;
        case QCRIL_UIM_SAP_CARD_REMOVED:
          err = SapResultCode::CARD_REMOVED;
          break;
        default:
          err = SapResultCode::GENERIC_FAILURE;
      }
      break;

    case UIM_SAP_RESET_SIM_REQ:
      switch(err_code)
      {
        case QCRIL_UIM_SAP_SUCCESS:
          err = SapResultCode::SUCCESS;
          break;
        case QCRIL_UIM_SAP_CARD_NOT_ACCESSSIBLE:
          err = SapResultCode::CARD_NOT_ACCESSSIBLE;
          break;
        case QCRIL_UIM_SAP_CARD_ALREADY_POWERED_OFF:
          err = SapResultCode::CARD_ALREADY_POWERED_OFF;
          break;
        case QCRIL_UIM_SAP_CARD_REMOVED:
          err = SapResultCode::CARD_REMOVED;
          break;
        default:
          err = SapResultCode::GENERIC_FAILURE;
      }
      break;

    case UIM_SAP_TRANSFER_CARD_READER_STATUS_REQ:
      switch(err_code)
      {
        case QCRIL_UIM_SAP_SUCCESS:
          err = SapResultCode::SUCCESS;
          break;
        case QCRIL_UIM_SAP_CARD_NOT_ACCESSSIBLE:
        case QCRIL_UIM_SAP_CARD_REMOVED:
          err = SapResultCode::DATA_NOT_AVAILABLE;
          break;
        default:
          err = SapResultCode::GENERIC_FAILURE;
      }
      break;
    default:
      break;
  }

  return err;
} /* SapImpl::sap_convert_err_to_sap_err */


/*===========================================================================

FUNCTION:  SapImpl::clearCallbacks

===========================================================================*/
void SapImpl::clearCallbacks()
{
  RLOGD("SecureElementImpl::clearCallbacks");

  RWLock::AutoWLock lock(mCallbackLock);
  if (sapCallback != nullptr)
  {
    sapCallback->unlinkToDeath(this);
    sapCallback = nullptr;
  }
} /* SapImpl::clearCallbacks */


/*===========================================================================

FUNCTION:  SapImpl::serviceDied

===========================================================================*/
void SapImpl::serviceDied
(
  uint64_t,
  const ::android::wp<::android::hidl::base::V1_0::IBase> &
)
{
  RLOGD("SapImpl::serviceDied: Client died. Cleaning up callbacks");
  clearCallbacks();
} /* SapImpl::serviceDied */


/*===========================================================================

FUNCTION:  SapImpl::checkReturnStatus

===========================================================================*/
void SapImpl::checkReturnStatus
(
  Return<void>& ret
)
{
  if (ret.isOk() == false)
  {
    RLOGE("checkReturnStatus: unable to call response/indication callback: %s",
            ret.description().c_str());
    // Remote process (SapRilReceiver.java) hosting the callback must be dead. Reset the
    // callback object; there's no other recovery to be done here. When the client process is
    // back up, it will call setCallback()
    sapCallback = nullptr;
  }
} /* SapImpl::checkReturnStatus */


/*===========================================================================

FUNCTION:  SapImpl::setCallback

===========================================================================*/
Return<void> SapImpl::setCallback
(
  const ::android::sp<ISapCallback>& sapCallbackParam
)
{
  RLOGD("SapImpl::setCallback");

  {
    RWLock::AutoWLock lock(mCallbackLock);
    if (sapCallback != nullptr)
    {
      sapCallback->unlinkToDeath(this);
    }
    sapCallback = sapCallbackParam;
    sapCallback->linkToDeath(this, 0);
  }

  return Void();
} /* SapImpl::setCallback */

sp<ISapCallback> SapImpl::getCallback() {
  RWLock::AutoRLock lock(mCallbackLock);
  return sapCallback;
}


/*===========================================================================

FUNCTION:  SapImpl::sendFailedResponse

===========================================================================*/
void SapImpl::sendFailedResponse
(
  UimSapRequestType msgId,
  int32_t           token,
  int               err_code
)
{
  Return<void>      retStatus  = Void();
  sp<ISapCallback>  callback   = getCallback();

  if (callback == nullptr)
  {
    return;
  }

  switch(msgId)
  {
    case UIM_SAP_CONNECT_REQ:
      retStatus = callback->connectResponse(token, SapConnectRsp::CONNECT_FAILURE, 0);
      break;

    case UIM_SAP_DISCONNECT_REQ:
      retStatus = callback->disconnectResponse(token);
      break;

    case UIM_SAP_APDU_REQ:
      {
        hidl_vec<uint8_t> apduRsp = {};
        retStatus = callback->apduResponse(token,
                                           sap_convert_err_to_sap_err(msgId, err_code),
                                           apduRsp);
        break;
      }

    case UIM_SAP_TRANSFER_ATR_REQ:
      {
        hidl_vec<uint8_t> atr = {};
        retStatus = callback->transferAtrResponse(token,
                                                  sap_convert_err_to_sap_err(msgId, err_code),
                                                  atr);
        break;
      }

    case UIM_SAP_POWER_REQ:
      retStatus = callback->powerResponse(token, sap_convert_err_to_sap_err(msgId, err_code));
      break;

    case UIM_SAP_RESET_SIM_REQ:
      retStatus = callback->resetSimResponse(token, sap_convert_err_to_sap_err(msgId, err_code));
      break;

    case UIM_SAP_TRANSFER_CARD_READER_STATUS_REQ:
      retStatus = callback->transferCardReaderStatusResponse(token,
              sap_convert_err_to_sap_err(msgId, err_code), 0);
      break;

    case UIM_SAP_SET_TRANSFER_PROTOCOL_REQ:
      retStatus = callback->transferProtocolResponse(token, SapResultCode::NOT_SUPPORTED);
      break;

    default:
      return;
  }

  checkReturnStatus(retStatus);
} /* SapImpl::sendFailedResponse */


/*===========================================================================

FUNCTION:  SapImpl::connectReq

===========================================================================*/
Return<void> SapImpl::connectReq
(
  int32_t token,
  int32_t maxMsgSize
)
{
  RLOGD("SapImpl::connectReq");

  (void) maxMsgSize;
  if (!sap_module.get_module().handleSapConnection(token, UIM_SAP_CONNECT_REQ))
  {
    sendFailedResponse(UIM_SAP_CONNECT_REQ, token, QCRIL_UIM_SAP_CONNECT_FAILURE);
  }

  return Void();
} /* SapImpl::connectReq */


/*===========================================================================

FUNCTION:  SapImpl::disconnectReq

===========================================================================*/
Return<void> SapImpl::disconnectReq
(
  int32_t token
)
{
  RLOGD("SapImpl::disconnectReq");

  if (!sap_module.get_module().handleSapConnection(token, UIM_SAP_DISCONNECT_REQ))
  {
    sendFailedResponse(UIM_SAP_DISCONNECT_REQ, token, QCRIL_UIM_SAP_CONNECT_FAILURE);
  }

  return Void();
} /* SapImpl::disconnectReq */


/*===========================================================================

FUNCTION:  SapImpl::apduReq

===========================================================================*/
Return<void> SapImpl::apduReq
(
  int32_t                  token,
  SapApduType              type,
  const hidl_vec<uint8_t>& command)
{
  int err_code = QCRIL_UIM_SAP_GENERIC_FAILURE;

  RLOGD("SapImpl::apduReq");

  (void) type;
  if (!sap_module.get_module().handleSapRequest(token,
                                   UIM_SAP_APDU_REQ,
                                   command.data(),
                                   command.size(),
                                   &err_code))
  {
    sendFailedResponse(UIM_SAP_APDU_REQ, token, err_code);
  }

  return Void();
} /* SapImpl::apduReq */


/*===========================================================================

FUNCTION:  SapImpl::transferAtrReq

===========================================================================*/
Return<void> SapImpl::transferAtrReq
(
  int32_t token
)
{
  int err_code = QCRIL_UIM_SAP_GENERIC_FAILURE;

  RLOGD("SapImpl::transferAtrReq");

  if (!sap_module.get_module().handleSapRequest(token,
                                   UIM_SAP_TRANSFER_ATR_REQ,
                                   NULL,
                                   0,
                                   &err_code))
  {
    sendFailedResponse(UIM_SAP_TRANSFER_ATR_REQ, token, err_code);
  }

  return Void();
} /* SapImpl::transferAtrReq */


/*===========================================================================

FUNCTION:  SapImpl::powerReq

===========================================================================*/
Return<void> SapImpl::powerReq
(
  int32_t token,
  bool state
)
{
  int err_code = QCRIL_UIM_SAP_GENERIC_FAILURE;

  RLOGD("SapImpl::powerReq");

  if (!sap_module.get_module().handleSapRequest(token,
                                   UIM_SAP_POWER_REQ,
                                   (const uint8_t *)&state,
                                   sizeof(state),
                                   &err_code))
  {
    sendFailedResponse(UIM_SAP_POWER_REQ, token, err_code);
  }

  return Void();
} /* SapImpl::powerReq */


/*===========================================================================

FUNCTION:  SapImpl::resetSimReq

===========================================================================*/
Return<void> SapImpl::resetSimReq
(
  int32_t token
)
{
  int err_code = QCRIL_UIM_SAP_GENERIC_FAILURE;

  RLOGD("SapImpl::resetSimReq");

  if (!sap_module.get_module().handleSapRequest(token,
                                   UIM_SAP_RESET_SIM_REQ,
                                   NULL,
                                   0,
                                   &err_code))
  {
    sendFailedResponse(UIM_SAP_RESET_SIM_REQ, token, err_code);
  }

  return Void();
} /* SapImpl::resetSimReq */


/*===========================================================================

FUNCTION:  SapImpl::transferCardReaderStatusReq

===========================================================================*/
Return<void> SapImpl::transferCardReaderStatusReq
(
  int32_t token
)
{
  int err_code = QCRIL_UIM_SAP_GENERIC_FAILURE;

  RLOGD("SapImpl::transferCardReaderStatusReq");

  if (!sap_module.get_module().handleSapRequest(token,
                                   UIM_SAP_TRANSFER_CARD_READER_STATUS_REQ,
                                   NULL,
                                   0,
                                   &err_code))
  {
    sendFailedResponse(UIM_SAP_TRANSFER_CARD_READER_STATUS_REQ, token, err_code);
  }

  return Void();
} /* SapImpl::transferCardReaderStatusReq */


/*===========================================================================

FUNCTION:  SapImpl::setTransferProtocolReq

===========================================================================*/
Return<void> SapImpl::setTransferProtocolReq
(
  int32_t token,
  SapTransferProtocol transferProtocol
)
{
  RLOGD("SapImpl::setTransferProtocolReq");

  (void)transferProtocol;
  sendFailedResponse(UIM_SAP_SET_TRANSFER_PROTOCOL_REQ, token, QCRIL_UIM_SAP_NOT_SUPPORTED);

  return Void();
} /* SapImpl::setTransferProtocolReq */


/*===========================================================================

FUNCTION:  SapModule::SapModule

===========================================================================*/
SapModule::SapModule()
{
  mName  = "SapModule";
  mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);

  using std::placeholders::_1;
  mMessageHandler = {
      HANDLER(QcrilInitMessage, SapModule::handleQcrilInitMessage)
  };

  qcril_sap_state = QMI_UIM_SAP_STATE_NOT_ENABLED;
} /* SapModule::SapModule */


/*===========================================================================

FUNCTION:  SapModule::~SapModule

===========================================================================*/
SapModule::~SapModule()
{
  sapService->sapCallback = nullptr;

  sapService = nullptr;
} /* SapModule::~SapModule */


/*===========================================================================

FUNCTION:  SapModule::init

===========================================================================*/
void SapModule::init()
{
  Module::init();
} /* SapModule::init */


/*===========================================================================

FUNCTION:  SapModule::handleQcrilInitMessage

===========================================================================*/
void SapModule::handleQcrilInitMessage
(
  std::shared_ptr<QcrilInitMessage> msg
)
{
  android::status_t          ret = android::OK;

  (void)msg;

  sapService = new SapImpl{};

  if (sapService != NULL)
  {
    qcril_uim_sap_register_client(QCRIL_UIM_SAP_CLIENT_ID_ATEL,
                                  SapModule::handleAtelSapInd,
                                  SapModule::handleAtelSapResp,
                                  SapModule::handleAtelCardStatus);

    switch(qmi_ril_get_process_instance_id())
    {
      case 0:
        ret = sapService->registerAsService("slot1");
        break;
      case 1:
        ret = sapService->registerAsService("slot2");
        break;
      default:
        return;
    }
  }
  RLOGD("sap_register_service status=%d", ret);
} /* SapModule::handleQcrilInitMessage */


/*=========================================================================

FUNCTION:  SapModule::handleAtelSapInd

===========================================================================*/
void SapModule::handleAtelSapInd
(
  qmi_uim_sap_status_type  sap_state
)
{
  sp<ISapCallback>  callback   =
    (sapService != nullptr) ? sapService->getCallback() : nullptr;
  Return<void>      retStatus  = Void();

  RLOGD("handleAtelSapInd, sap_state: 0x%x ", sap_state);

  /* Only 2 unsol responses are send from the QMI IND & it depends on the sap_status:
  1. RIL_SIM_SAP_STATUS_IND, upon status for connection establishment/disconnection
  2. RIL_SIM_SAP_DISCONNECT_IND, upon disconnection request only */
  qcril_sap_state = sap_state;

  if (sapService == nullptr || callback == nullptr)
  {
    RLOGE("NULL sap_service or Null callback");
    return;
  }

  switch (qcril_sap_state)
  {
    case QMI_UIM_SAP_STATE_DISCONNECTED_SUCCESSFULLY:
      retStatus = callback->disconnectIndication(0, SapDisconnectType::GRACEFUL);
      break;

    case QMI_UIM_SAP_STATE_CONNECTION_ERROR:
    case QMI_UIM_SAP_STATE_NOT_ENABLED:
      retStatus = callback->statusIndication(0, SapStatus::CARD_NOT_ACCESSIBLE);
      break;

    case QMI_UIM_SAP_STATE_CONNECTED_SUCCESSFULLY:
    case QMI_UIM_SAP_STATE_CONNECTING:
    case QMI_UIM_SAP_STATE_DISCONNECTING:
    default:
      /* Note that for a succesfully connected case, if the connection response
      isnt sent yet, the STATUS_IND is sent after response callback comes */
      RLOGD("Skipping SAP UNSOL response for sap_state: 0x%x", qcril_sap_state);
      return;
  }

  sapService->checkReturnStatus(retStatus);
} /* SapModule::handleAtelSapInd */


/*=========================================================================

FUNCTION:  SapModule::handleAtelCardStatus

===========================================================================*/
void SapModule::handleAtelCardStatus
(
  qcril_card_status_e_type  card_state
)
{
  boolean      send_unsol       = FALSE;
  SapStatus    status           = SapStatus::UNKNOWN_ERROR;

  RLOGD("handleAtelCardStatus, card_state: 0x%x ", card_state);

  switch (card_state)
  {
    case QCRIL_CARD_STATUS_ABSENT:
      send_unsol = TRUE;
      status = SapStatus::CARD_REMOVED;
      break;

    case QCRIL_CARD_STATUS_UP:
      /* Send card insert event only if SAP is already connected */
      if (qcril_sap_state == QMI_UIM_SAP_STATE_CONNECTED_SUCCESSFULLY)
      {
        send_unsol = TRUE;
        status = SapStatus::CARD_INSERTED;
      }
      break;

    default:
      break;
  }

  if (send_unsol)
  {
    sp<ISapCallback>  callback =
      (sapService != nullptr) ? sapService->getCallback() : nullptr;

    if (sapService != nullptr && callback != nullptr)
    {
      Return<void> retStatus = callback->statusIndication(0, status);

      sapService->checkReturnStatus(retStatus);
    }
  }
} /* SapModule::handleAtelCardStatus */


/*=========================================================================

FUNCTION:  SapModule::handleAtelSapResp

===========================================================================*/
void SapModule::handleAtelSapResp
(
  int             request_id,
  int             err_code,
  int             qmi_err_code,
  void          * tokenPtr,
  uint8_t       * data_ptr,
  uint32_t        data_len
)
{
  UimSapRequestType   msg_id     = (UimSapRequestType)request_id;
  sp<ISapCallback>    callback   =
    (sapService != nullptr) ? sapService->getCallback() : nullptr;
  Return<void>        retStatus  = Void();
  int32_t             token      = 0;

  /* Sanity check */
  if (sapService == nullptr || callback == nullptr || tokenPtr == nullptr)
  {
    RLOGE("NULL sap_service or Null callback");
    return;
  }

  token = *((int32_t *)tokenPtr);
  delete (int32_t *)tokenPtr;
  tokenPtr = NULL;

  RLOGD("handle_sap_request_resp, token: %d, MsgId: 0x%x, qmi_err_code: 0x%x, err_code: %d",
                  token,
                  msg_id,
                  qmi_err_code,
                  err_code);

  /* Update the parameters for respective responses */
  switch (msg_id)
  {
    case UIM_SAP_CONNECT_REQ:
      retStatus = callback->connectResponse(token,
        (qmi_err_code == 0) ? SapConnectRsp::SUCCESS : SapConnectRsp::CONNECT_FAILURE, 0);

      /* connect request failed, reset to max clients */
      if (qmi_err_code != 0 &&
          qcril_uim_sap_get_active_client() == QCRIL_UIM_SAP_CLIENT_ID_ATEL)
      {
        qcril_uim_sap_set_active_client(QCRIL_UIM_SAP_MAX_CLIENTS);
      }

      /* Now send UNSOL response if needed */
      if ((qmi_err_code == 0) &&
          (qcril_sap_state == QMI_UIM_SAP_STATE_CONNECTED_SUCCESSFULLY))
      {
        retStatus = callback->statusIndication(0, SapStatus::CARD_RESET);
      }
      break;

    case UIM_SAP_DISCONNECT_REQ:
      callback->disconnectResponse(token);
      /* Also reset the active client ID upon a successful disconnection */
      qcril_uim_sap_set_active_client(QCRIL_UIM_SAP_MAX_CLIENTS);
      break;

    case UIM_SAP_APDU_REQ:
      {
        hidl_vec<uint8_t> apduRsp = {};

        if (data_ptr != NULL && data_len > 0)
        {
          apduRsp.setToExternal(data_ptr, data_len);
        }
        retStatus = callback->apduResponse(token,
                                    sapService->sap_convert_err_to_sap_err(msg_id, err_code),
                                    apduRsp);
      }
      break;

    case UIM_SAP_TRANSFER_ATR_REQ:
      {
        hidl_vec<uint8_t>   atr = {};

        if (data_ptr != NULL && data_len > 0)
        {
          atr.setToExternal(data_ptr, data_len);
        }
        retStatus = callback->transferAtrResponse(token,
                                       sapService->sap_convert_err_to_sap_err(msg_id, err_code),
                                       atr);
      }
      break;

    case UIM_SAP_POWER_REQ:
      retStatus =
        callback->powerResponse(token,
                                sapService->sap_convert_err_to_sap_err(msg_id, err_code));
      break;

    case UIM_SAP_RESET_SIM_REQ:
      retStatus =
        callback->resetSimResponse(token,
                                   sapService->sap_convert_err_to_sap_err(msg_id, err_code));
      break;

    case UIM_SAP_TRANSFER_CARD_READER_STATUS_REQ:
      {
        int32_t card_status = 0;

        if ((data_ptr != NULL) &&
            (data_len > 0) &&
            (data_len <= sizeof(int32_t)))
        {
          card_status = *data_ptr;
        }

        retStatus = callback->transferCardReaderStatusResponse(token,
              sapService->sap_convert_err_to_sap_err(msg_id, err_code), card_status);
      }
      break;

    case UIM_SAP_SET_TRANSFER_PROTOCOL_REQ:
      retStatus =
        callback->transferProtocolResponse(token, SapResultCode::NOT_SUPPORTED);
      break;

    default:
      RLOGE("Unsupported MsgId: 0x%x", msg_id);
      return;
  }

  sapService->checkReturnStatus(retStatus);
} /* SapModule::handleAtelSapResp */


/*=========================================================================

FUNCTION:  SapModule::handleSapRequest

===========================================================================*/
boolean SapModule::handleSapRequest
(
  int32_t                      token,
  UimSapRequestType            message_id,
  const uint8_t              * payload_ptr,
  size_t                       payload_len,
  int                        * error_code
)
{
  int                                 qmi_result         = 0;
  qmi_uim_slot_type                   qmi_slot           = QMI_UIM_SLOT_1;
  qmi_uim_sap_request_params_type     sap_request_params = {};
  int32_t                           * token_ptr          = NULL;

  if (error_code == NULL ||
      !qcril_uim_sap_convert_slot_id_to_slot_type(qmi_ril_get_sim_slot(), &qmi_slot))
  {
    return FALSE;
  }

  token_ptr = new int32_t(token);

  if (token_ptr == NULL)
  {
    return FALSE;
  }

  /* Update QMI parameters from protobuf request & dispatch it to modem */
  sap_request_params.slot = qmi_slot;

  switch (qcril_uim_sap_get_sim_state())
  {
    case QCRIL_CARD_STATUS_UNKNOWN:
    case QCRIL_CARD_STATUS_NOT_APPLICABLE:
    case QCRIL_CARD_STATUS_NOT_ACCESSIBLE:
      *error_code = QCRIL_UIM_SAP_CARD_NOT_ACCESSSIBLE;
      delete token_ptr;
      return FALSE;
    case QCRIL_CARD_STATUS_ABSENT:
      *error_code = QCRIL_UIM_SAP_CARD_REMOVED;
      delete token_ptr;
      return FALSE;
    case QCRIL_CARD_STATUS_DOWN:
      /* Only POWER UP request works in power down state */
      if (message_id == UIM_SAP_POWER_REQ &&
          payload_ptr != NULL &&
          *payload_ptr)
      {
        break;
      }
      *error_code = QCRIL_UIM_SAP_CARD_ALREADY_POWERED_OFF;
      delete token_ptr;
      return FALSE;
    case QCRIL_CARD_STATUS_UP:
      /* Only POWER UP request fails in power up state */
      if (message_id == UIM_SAP_POWER_REQ &&
          payload_ptr != NULL &&
          *payload_ptr)
      {
        *error_code = QCRIL_UIM_SAP_CARD_ALREADY_POWERED_ON;
        delete token_ptr;
        return FALSE;
      }
      break;
    default:
      break;
  }

  switch (message_id)
  {
    case UIM_SAP_APDU_REQ:
      if (payload_ptr == NULL)
      {
        RLOGE( "%s", "Null apdu ptr, cannot proceed");
        delete token_ptr;
        return FALSE;
      }

      if (payload_len <= 0 ||
          payload_len > QMI_UIM_MAX_APDU_DATA)
      {
        RLOGE("invalid command->size: 0x%x, cannot proceed",
                        (uint32_t)payload_len);
        delete token_ptr;
        return FALSE;
      }

      sap_request_params.request_type = QMI_UIM_SAP_REQUEST_OP_SEND_APDU;
      sap_request_params.apdu.data_len = payload_len;
      sap_request_params.apdu.data_ptr = (unsigned char *)payload_ptr;
      break;

    case UIM_SAP_POWER_REQ:
      if (payload_ptr != NULL &&
          *payload_ptr)
      {
        sap_request_params.request_type = QMI_UIM_SAP_REQUEST_OP_POWER_SIM_ON;
        qcril_uim_sap_set_sim_state(QCRIL_CARD_STATUS_UP);
      }
      else
      {
        sap_request_params.request_type = QMI_UIM_SAP_REQUEST_OP_POWER_SIM_OFF;
        qcril_uim_sap_set_sim_state(QCRIL_CARD_STATUS_DOWN);
      }
      break;

    case UIM_SAP_RESET_SIM_REQ:
      sap_request_params.request_type = QMI_UIM_SAP_REQUEST_OP_RESET_SIM;
      break;

    case UIM_SAP_TRANSFER_ATR_REQ:
      sap_request_params.request_type = QMI_UIM_SAP_REQUEST_OP_GET_ATR;
      break;

    case UIM_SAP_TRANSFER_CARD_READER_STATUS_REQ:
      sap_request_params.request_type = QMI_UIM_SAP_REQUEST_OP_READER_STATUS;
      break;

    default:
      RLOGE( "Unspported message_id: 0x%x\n", message_id);
      delete token_ptr;
      return FALSE;
  }

  qmi_result = qcril_uim_sap_send_request(QCRIL_UIM_SAP_CLIENT_ID_ATEL,
                                          QCRIL_UIM_SAP_REQUEST_REQ,
                                          (int)message_id,
                                          (void *)token_ptr,
                                          &sap_request_params);

  if (qmi_result >= 0)
  {
    return TRUE;
  }

  RLOGE("SAP request failed, qmi_result: 0x%x", qmi_result);

  delete token_ptr;
  return FALSE;
} /* SapModule::handleSapRequest */


/*=========================================================================

FUNCTION:  SapModule::handleSapConnection

===========================================================================*/
boolean SapModule::handleSapConnection
(
  int32_t                      token,
  UimSapRequestType            message_id
)
{
  int                                   qmi_result         = 0;
  qmi_uim_slot_type                     qmi_slot           = QMI_UIM_SLOT_1;
  qmi_uim_sap_connection_params_type    sap_request_params = {};
  uint8                                 active_client_id   = qcril_uim_sap_get_active_client();
  int32_t                             * token_ptr          = NULL;

  /* Reject SAP connect request if there is already an active client */
  if ((message_id       == UIM_SAP_CONNECT_REQ) &&
      (active_client_id != QCRIL_UIM_SAP_MAX_CLIENTS))
  {
    RLOGE("Rejecting SAP connect req, active_client_id: 0x%x", active_client_id);
    return FALSE;
  }

  if (!qcril_uim_sap_convert_slot_id_to_slot_type(qmi_ril_get_sim_slot(), &qmi_slot))
  {
    return FALSE;
  }

  token_ptr = new int32_t(token);

  if (token_ptr == NULL)
  {
    return FALSE;
  }

  /* Update QMI parameters from protobuf request & dispatch it to modem */
  sap_request_params.slot           = qmi_slot;
  sap_request_params.conn_condition = QMI_UIM_SAP_CONN_COND_BLOCK_VOICE;

  if (message_id == UIM_SAP_CONNECT_REQ)
  {
    sap_request_params.operation_type = QMI_UIM_SAP_CONNECTION_CONNECT;
  }
  else if (message_id == UIM_SAP_DISCONNECT_REQ)
  {
    /* Note - for disconnect req, there is no mode passed from client */
    sap_request_params.operation_type  = QMI_UIM_SAP_CONNECTION_DISCONNECT;
    sap_request_params.disconnect_mode = QMI_UIM_SAP_DISCONNECT_MODE_GRACEFUL;
  }
  else
  {
    RLOGE( "Unspported message_id: 0x%x\n", message_id);
    delete token_ptr;
    return FALSE;
  }

  qmi_result = qcril_uim_sap_send_request(QCRIL_UIM_SAP_CLIENT_ID_ATEL,
                                          QCRIL_UIM_SAP_CONNECTION_REQ,
                                          (int)message_id,
                                          (void *)token_ptr,
                                          &sap_request_params);

  if (qmi_result < 0)
  {
    RLOGE("SAP connection request failed, qmi_result: 0x%x", qmi_result);
    delete token_ptr;
    return FALSE;
  }
  qcril_uim_sap_set_active_client(QCRIL_UIM_SAP_CLIENT_ID_ATEL);

  return TRUE;
} /* SapModule::handleSapConnection */
#endif
