/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#ifndef QMI_RIL_UTF
#include <framework/ModuleLooper.h>
#include <framework/Log.h>
#include <utils/RWLock.h>

#include <android/hardware/radio/config/1.0/IRadioConfig.h>
#include <android/hardware/radio/config/1.0/types.h>
#include <hidl/HidlTransportSupport.h>
#include <inttypes.h>

#include "modules/qmi/ModemEndPointFactory.h"
#include "modules/qmi/UimModemEndPoint.h"
#include "modules/qmi/QmiUimSetupRequestCallback.h"
#include "modules/uim/UimModuleInterface.h"
#include "modules/uim/qcril_uim_srvc.h"
#include "modules/uim/qcril_uim.h"
#include "modules/uim/UimQmiUimRequestSyncMsg.h"

#include "RadioConfigModule.h"

#define TAG "RadioConfig_Module"

using namespace android::hardware::radio;
using namespace android::hardware::radio::config;
using namespace android::hardware::radio::config::V1_0;
using namespace android::hardware::radio::V1_0;
using ::android::hardware::configureRpcThreadpool;
using ::android::hardware::joinRpcThreadpool;
using ::android::hardware::Return;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_array;
using ::android::hardware::Void;
using ::android::hardware::hidl_death_recipient;
using ::android::RWLock;
using android::sp;
using android::hardware::Status;

struct RadioConfigImpl : public IRadioConfig, public hidl_death_recipient {
  sp<IRadioConfigIndication> mIndicationCb;
  sp<IRadioConfigResponse> mResponseCb;

  RWLock mCallbackLock;

  sp<IRadioConfigResponse> getResponseCallback();
  sp<IRadioConfigIndication> getIndicationCallback();

  void clearCallbacks();

  void serviceDied( uint64_t, const ::android::wp<::android::hidl::base::V1_0::IBase> &);

  Return<void> setResponseFunctions(const sp<IRadioConfigResponse>& radioConfigResponse,
                                    const sp<IRadioConfigIndication>& radioConfigIndication);
  Return<void> getSimSlotsStatus(int32_t serial);
  Return<void> setSimSlotsMapping(int32_t serial, const hidl_vec<uint32_t>& slotMap);
};

static load_module<RadioConfigModule> radio_config_module;
static sp<RadioConfigImpl> radioConfigService = NULL;


/*===========================================================================

FUNCTION:  radio_config_iccid_to_string

===========================================================================*/
static char * radio_config_iccid_to_string
(
  const uint8_t * iccid_ptr,
  uint16_t        iccid_len
)
{
  char * iccid_str = nullptr;

  if (iccid_ptr == NULL || iccid_len == 0)
  {
    return iccid_str;
  }

  iccid_str = new char[2 * iccid_len + 1]{};

  if (iccid_str == nullptr)
  {
    return iccid_str;
  }

  for (uint32_t i = 0, j = 0; i < iccid_len && j < (2 * iccid_len); i++, j++)
  {
    uint8_t ch = iccid_ptr[i] & 0x0F;

    if (ch > 9)
    {
      iccid_str[j] = ch + 'A' - 0x0A;
    }
    else
    {
      iccid_str[j] = ch + '0';
    }

    j++;
    if (j >= (2 * iccid_len))
    {
      break;
    }

    ch = iccid_ptr[i] >> 4;
    if (ch > 9)
    {
      iccid_str[j] = ch + 'A' - 0x0A;
    }
    else
    {
      iccid_str[j] = ch + '0';
    }
  }

  return iccid_str;
} /* radio_config_iccid_to_string */


/*===========================================================================

FUNCTION:  radio_config_bin_to_hexstring

===========================================================================*/
static char * radio_config_bin_to_hexstring
(
  const uint8_t * data_ptr,
  uint16_t        data_len
)
{
  char*     out_ptr    = NULL;
  uint32_t  string_len = 0;

  if (data_ptr == NULL || data_len == 0)
  {
    return NULL;
  }

  string_len = (2 * data_len) + sizeof(char);

  out_ptr = new char[string_len];

  if (out_ptr != NULL)
  {
    uint16_t i = 0;

    for (i = 0; i < data_len; i++)
    {
      if (((data_ptr[i] >> 4) & 0x0F) < 0x0a)
      {
        out_ptr[i * 2] = ((data_ptr[i] >> 4) & 0x0F) + '0';
      }
      else
      {
        out_ptr[i * 2] = ((data_ptr[i] >> 4) & 0x0F) + 'a' - 10;
      }

      if ((data_ptr[i] & 0x0F) < 0x0a)
      {
        out_ptr[i * 2 + 1] = (data_ptr[i] & 0x0F) + '0';
      }
      else
      {
        out_ptr[i * 2 + 1] = (data_ptr[i] & 0x0F) + 'a' - 10;
      }
    }
    out_ptr[data_len * 2] = 0x0;
  }

  return out_ptr;
} /* radio_config_bin_to_hexstring */


/*===========================================================================

FUNCTION:  convertUimQmiSlotStatusToHal

===========================================================================*/
static int convertUimQmiSlotStatusToHal
(
  const qmi_uim_slots_status_type *status_rsp_ptr,
  SimSlotStatus ** slot_status_out_ptr
)
{
  uint32_t         i               = 0;
  SimSlotStatus  * slot_status_ptr = NULL;

  if (status_rsp_ptr == NULL || slot_status_out_ptr == NULL || status_rsp_ptr->no_of_slots == 0)
  {
    return 0;
  }

  slot_status_ptr = new SimSlotStatus[status_rsp_ptr->no_of_slots]{};

  if (slot_status_ptr == NULL)
  {
    return 0;
  }

  *slot_status_out_ptr = slot_status_ptr;

  for (; i < status_rsp_ptr->no_of_slots; i++)
  {
    switch(status_rsp_ptr->slot_status[i].card_state)
    {
      case QMI_UIM_SLOT_CARD_STATE_ABSENT:
        slot_status_ptr[i].cardState = CardState::ABSENT;
        break;

      case QMI_UIM_SLOT_CARD_STATE_PRESENT:
        slot_status_ptr[i].cardState = CardState::PRESENT;
        break;

      default:
        slot_status_ptr[i].cardState = CardState::ERROR;
        break;
    }

    switch(status_rsp_ptr->slot_status[i].slot_state)
    {
      case QMI_UIM_SLOT_STATE_ACTIVE:
        slot_status_ptr[i].slotState = SlotState::ACTIVE;
        break;

      default:
        slot_status_ptr[i].slotState = SlotState::INACTIVE;
        break;
    }

    if(slot_status_ptr[i].slotState== SlotState::ACTIVE)
    {
      switch(status_rsp_ptr->slot_status[i].logical_slot)
      {
        case QMI_UIM_SLOT_1:
          slot_status_ptr[i].logicalSlotId = 0;
          break;
        case QMI_UIM_SLOT_2:
          slot_status_ptr[i].logicalSlotId = 1;
          break;
        case QMI_UIM_SLOT_3:
          slot_status_ptr[i].logicalSlotId = 2;
          break;
        default:
          QCRIL_LOG_ERROR("%s", "Invalid input, incorrect logical slot id ");
          break;
      }
    }

    if (slot_status_ptr[i].cardState == CardState::PRESENT)
    {
      char * iccid = nullptr;
      char * atr   = nullptr;

      iccid = radio_config_iccid_to_string(status_rsp_ptr->slot_status[i].iccid,
                                           status_rsp_ptr->slot_status[i].iccid_len);

      if (status_rsp_ptr->slot_status[i].atr_len == 0)
      {
        /* Attempt to get the ATR only when slot is active */
        if (slot_status_ptr[i].slotState == SlotState::ACTIVE)
        {
          qmi_uim_get_atr_params_type       get_atr_params = {};
          qmi_uim_rsp_data_type             rsp_data       = {};
          int                               qmi_err_code   = 0;

          get_atr_params.slot = status_rsp_ptr->slot_status[i].logical_slot;

          /* get atr */
          qmi_err_code = qcril_uim_send_qmi_sync_msg(QCRIL_UIM_REQUEST_GET_ATR,
                                                     &get_atr_params,
                                                     &rsp_data);
          if (qmi_err_code == 0 && rsp_data.qmi_err_code == 0)
          {
            atr = radio_config_bin_to_hexstring(rsp_data.rsp_data.get_atr_rsp.atr_response.data_ptr,
                                                rsp_data.rsp_data.get_atr_rsp.atr_response.data_len);

            if (rsp_data.rsp_data.get_atr_rsp.atr_response.data_ptr != NULL)
            {
              qcril_free(rsp_data.rsp_data.get_atr_rsp.atr_response.data_ptr);
              rsp_data.rsp_data.get_atr_rsp.atr_response.data_ptr = NULL;
            }
          }
        }
      }
      else
      {
        atr = radio_config_bin_to_hexstring(status_rsp_ptr->slot_status[i].atr,
                                            status_rsp_ptr->slot_status[i].atr_len);
      }

      if (atr != nullptr)
      {
        hidl_string hidl_atr = {};

        hidl_atr.setToExternal(atr, strlen(atr));
        slot_status_ptr[i].atr = hidl_atr;

        delete[] atr;
      }

      if (iccid != nullptr)
      {
        hidl_string hidl_iccid = {};

        hidl_iccid.setToExternal(iccid, strlen(iccid));
        slot_status_ptr[i].iccid = hidl_iccid;

        delete[] iccid;
      }
    }
  }
  return i;
} /* convertUimQmiSlotStatusToHal */


/*===========================================================================

FUNCTION:  processSlotStatusInd

===========================================================================*/
static void processSlotStatusInd
(
  const qmi_uim_slots_status_type *status_rsp_ptr
)
{
  SimSlotStatus           * slot_status_ptr = NULL;
  uint32_t                  slot_count      = 0;
  hidl_vec<SimSlotStatus>   slotStatus;
  sp<IRadioConfigIndication>   ind_cb          =
    (radioConfigService == nullptr) ? nullptr : radioConfigService->getIndicationCallback();

  QCRIL_LOG_INFO("processSlotStatusInd");

  if (status_rsp_ptr == nullptr ||
      !radio_config_module.get_module().mReady ||
      ind_cb == nullptr)
  {
    return;
  }

  slot_count = convertUimQmiSlotStatusToHal(status_rsp_ptr, &slot_status_ptr);

  slotStatus.setToExternal(slot_status_ptr, slot_count);

  Return<void> ret = ind_cb->simSlotsStatusChanged(RadioIndicationType::UNSOLICITED, slotStatus);

  if (!ret.isOk())
  {
    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
  }

  if (slot_status_ptr != NULL)
  {
    delete[] slot_status_ptr;
    slot_status_ptr = NULL;
  }
} /* processSlotStatusInd */


/*===========================================================================

FUNCTION:  handleRadioConfigModuleReady

===========================================================================*/
static void handleRadioConfigModuleReady
(
  void
)
{
  qmi_uim_rsp_data_type                    rsp_data         = {};
  int                                      qmi_err_code     = 0;

  qmi_err_code = qcril_uim_send_qmi_sync_msg(QCRIL_UIM_REQUEST_GET_SLOTS_STATUS,
                                                          nullptr,
                                             &rsp_data);
  if (qmi_err_code == 0 && rsp_data.qmi_err_code == 0 &&
      rsp_data.rsp_data.slots_status_rsp.no_of_slots > 0)
  {
    processSlotStatusInd(&rsp_data.rsp_data.slots_status_rsp);
  }
} /* handleRadioConfigModuleReady */


/*===========================================================================

FUNCTION:  RadioConfigImpl::setResponseFunctions

===========================================================================*/
Return<void> RadioConfigImpl::setResponseFunctions
(
  const sp<IRadioConfigResponse>& radioConfigResponse,
  const sp<IRadioConfigIndication>& radioConfigIndication
)
{
  QCRIL_LOG_INFO("RadioConfigImpl::setResponseFunctions");

  {
    RWLock::AutoWLock lock(mCallbackLock);

    if (mResponseCb != nullptr)
    {
      mResponseCb->unlinkToDeath(this);
    }

    mIndicationCb = radioConfigIndication;
    mResponseCb = radioConfigResponse;

    if (mResponseCb != nullptr)
    {
      mResponseCb->linkToDeath(this, 0);
    }

    radio_config_module.get_module().mServiceUp = true;

    if (radio_config_module.get_module().mQmiUimUp)
    {
      radio_config_module.get_module().mReady = true;
    }
  }

  if (radio_config_module.get_module().mReady)
  {
    handleRadioConfigModuleReady();
  }
  return Status::ok();
} /* RadioConfigImpl::setResponseFunctions */

sp<IRadioConfigResponse> RadioConfigImpl::getResponseCallback() {
  RWLock::AutoRLock lock(mCallbackLock);
  return mResponseCb;
}

sp<IRadioConfigIndication> RadioConfigImpl::getIndicationCallback() {
  RWLock::AutoRLock lock(mCallbackLock);
  return mIndicationCb;
}


/*===========================================================================

FUNCTION:  RadioConfigImpl::serviceDied

===========================================================================*/
void RadioConfigImpl::serviceDied
(
 uint64_t,
 const ::android::wp<::android::hidl::base::V1_0::IBase> &
)
{
  QCRIL_LOG_INFO("RadioConfigImpl::serviceDied: Client died. Cleaning up callbacks");
  clearCallbacks();
} /* RadioConfigImpl::serviceDied */


/*===========================================================================

FUNCTION:  RadioConfigImpl::clearCallbacks

===========================================================================*/
void RadioConfigImpl::clearCallbacks()
{
  QCRIL_LOG_INFO("RadioConfigImpl::clearCallbacks");

  RWLock::AutoWLock lock(mCallbackLock);

  if (mResponseCb != nullptr)
  {
    mResponseCb = nullptr;
  }

  if (mIndicationCb != nullptr)
  {
    mIndicationCb = nullptr;
  }

  radio_config_module.get_module().mServiceUp = false;
  radio_config_module.get_module().mReady = false;
} /* RadioConfigImpl::clearCallbacks() */


/*===========================================================================

FUNCTION:  RadioConfigImpl::getSimSlotsStatus

===========================================================================*/
Return<void> RadioConfigImpl::getSimSlotsStatus
(
  int32_t serial
)
{
  RadioResponseInfo                        responseInfo    = {};
  qmi_uim_rsp_data_type                    rsp_data        = {};
  int                                      qmi_err_code    = 0;
  SimSlotStatus                          * slot_status_ptr = nullptr;
  hidl_vec<SimSlotStatus>                  slotStatus;
  uint32_t                                 slot_count      = 0;
  sp<IRadioConfigResponse>                 res_cb          = getResponseCallback();

  QCRIL_LOG_INFO("Serial: %d", serial);

  if (res_cb == nullptr)
  {
    QCRIL_LOG_INFO("mResponseCb == NULL");
    return Void();
  }

  responseInfo.serial = serial;
  responseInfo.type = RadioResponseType::SOLICITED;
  responseInfo.error = RadioError::NONE;

  do
  {
    if (!radio_config_module.get_module().mReady)
    {
      QCRIL_LOG_INFO("module not ready == NULL");
      responseInfo.error = RadioError::INTERNAL_ERR;
      break;
    }

    qmi_err_code = qcril_uim_send_qmi_sync_msg(QCRIL_UIM_REQUEST_GET_SLOTS_STATUS,
                                               nullptr,
                                               &rsp_data);

    if (qmi_err_code != QMI_UIM_SERVICE_ERR_NONE ||
        rsp_data.qmi_err_code != QMI_UIM_SERVICE_ERR_NONE ||
        rsp_data.rsp_data.slots_status_rsp.no_of_slots == 0)
    {
      uint32_t                slot_id         = 0;

      if (qmi_ril_is_feature_supported(QMI_RIL_FEATURE_DSDA)  ||
          qmi_ril_is_feature_supported(QMI_RIL_FEATURE_DSDA2) ||
          qmi_ril_is_feature_supported(QMI_RIL_FEATURE_DSDS))
      {
        slot_count = 2;
      }
      else if (qmi_ril_is_feature_supported(QMI_RIL_FEATURE_TSTS))
      {
        slot_count = 3;
      }
      else
      {
        slot_count = 1;
      }

      slot_status_ptr = new SimSlotStatus[slot_count]{};

      if (slot_status_ptr == nullptr)
      {
        responseInfo.error = RadioError::INTERNAL_ERR;
        break;
      }

      for (; slot_id < slot_count && slot_id < QMI_UIM_MAX_CARD_COUNT; slot_id++)
      {
        RIL_CardStatus_v6        card_status = {};

        slot_status_ptr[slot_id].slotState = SlotState::ACTIVE;
        slot_status_ptr[slot_id].logicalSlotId = slot_id;

        if (UimModuleInterface::get_uim_interface()->get_card_status(slot_id, &card_status)
                                                                            == RIL_E_SUCCESS)
        {
          switch(card_status.card_state)
          {
            case RIL_CARDSTATE_ABSENT:
              slot_status_ptr[slot_id].cardState = CardState::ABSENT;
              break;

            case RIL_CARDSTATE_PRESENT:
              slot_status_ptr[slot_id].cardState = CardState::PRESENT;
              break;

            default:
              slot_status_ptr[slot_id].cardState = CardState::ERROR;
              break;
          }

          if (slot_status_ptr[slot_id].cardState == CardState::PRESENT)
          {
            hidl_string atr = {};
            hidl_string iccid = {};

            if (card_status.atr != nullptr)
            {
              atr.setToExternal(card_status.atr, strlen(card_status.atr));
            }

            if (card_status.iccid != nullptr)
            {
              iccid.setToExternal(card_status.iccid, strlen(card_status.iccid));
            }

            slot_status_ptr[slot_id].atr = atr;
            slot_status_ptr[slot_id].iccid = iccid;
          }
        }
        UimModuleInterface::get_uim_interface()->cleanup_get_card_status(&card_status);
      }
    }
    else
    {
      slot_count = convertUimQmiSlotStatusToHal(&rsp_data.rsp_data.slots_status_rsp,
                                                &slot_status_ptr);
    }
  } while(0);

  slotStatus.setToExternal(slot_status_ptr, slot_count);
  Return<void> ret = res_cb->getSimSlotsStatusResponse(responseInfo, slotStatus);

  if (!ret.isOk())
  {
    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
  }

  if (slot_status_ptr != NULL)
  {
    delete[] slot_status_ptr;
    slot_status_ptr = NULL;
  }

  return Void();
} /* RadioConfigImpl::getSimSlotsStatus */


/*===========================================================================

FUNCTION:  RadioConfigImpl::setSimSlotsMapping

===========================================================================*/
Return<void> RadioConfigImpl::setSimSlotsMapping
(
  int32_t                   serial,
  const hidl_vec<uint32_t>& slotMap
)
{
  qmi_uim_physical_slot_mapping_type       params         = {};
  uint32_t                                 slot_id        = 0;
  RadioResponseInfo                        responseInfo   = {};
  qmi_uim_rsp_data_type                    rsp_data       = {};
  int                                      qmi_err_code   = 0;
  sp<IRadioConfigResponse>                 res_cb          = getResponseCallback();


  QCRIL_LOG_INFO("%d", serial);

  if (res_cb == nullptr)
  {
    QCRIL_LOG_INFO("mResponseCb == NULL");
    return Void();
  }

  responseInfo.serial = serial;
  responseInfo.type = RadioResponseType::SOLICITED;
  responseInfo.error = RadioError::NONE;

  do
  {
    if (!radio_config_module.get_module().mReady)
    {
      QCRIL_LOG_INFO("module not ready");
      responseInfo.error = RadioError::INTERNAL_ERR;
      break;
    }

    params.num_slots = slotMap.size();
    if (params.num_slots > QMI_UIM_MAX_CARD_COUNT)
    {
      responseInfo.error = RadioError::INVALID_ARGUMENTS;
      break;
    }

    for(slot_id = 0; slot_id < params.num_slots; slot_id++)
    {
      /* Find slot info */
      switch(slotMap.data()[slot_id])
      {
        case 0:
          params.logical_slot[slot_id] = QMI_UIM_PHY_SLOT_1;
          break;
        case 1:
          params.logical_slot[slot_id] = QMI_UIM_PHY_SLOT_2;
          break;
        case 2:
          params.logical_slot[slot_id] = QMI_UIM_PHY_SLOT_3;
          break;
        default:
          /* Send the response, actual response will be in payload */
          responseInfo.error = RadioError::INVALID_ARGUMENTS;
          break;
      }

      if (responseInfo.error != RadioError::NONE)
      {
        break;
      }
    }

    qmi_err_code = qcril_uim_send_qmi_sync_msg(QCRIL_UIM_REQUEST_SWITCH_SLOT_EXT,
                                               &params,
                                               &rsp_data);

    if (qmi_err_code != QMI_UIM_SERVICE_ERR_NONE ||
        rsp_data.qmi_err_code != QMI_UIM_SERVICE_ERR_NONE)
    {
      responseInfo.error = RadioError::INTERNAL_ERR;
      break;
    }
  } while(0);

  Return<void> ret = res_cb->setSimSlotsMappingResponse(responseInfo);

  if (!ret.isOk())
  {
    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
  }

  return Void();
} /* RadioConfigImpl::setSimSlotsMapping */


/*===========================================================================

FUNCTION:  RadioConfigModule::RadioConfigModule()

===========================================================================*/
RadioConfigModule::RadioConfigModule()
{
  mName  = "RadioConfigModule";
  mQmiUimUp = false;
  mServiceUp = false;
  mReady = false;
  mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);

  using std::placeholders::_1;
  mMessageHandler = {
      HANDLER(QcrilInitMessage, RadioConfigModule::handleQcrilInitMessage),
      HANDLER(UimQmiUimIndicationMsg, RadioConfigModule::handleQmiUimIndiaction),
  };
} /* RadioConfigModule::RadioConfigModule */


/*===========================================================================

FUNCTION:  RadioConfigModule::~RadioConfigModule()

===========================================================================*/
RadioConfigModule::~RadioConfigModule()
{
  mQmiUimUp = false;
  mServiceUp = false;
  mReady = false;
} /* RadioConfigModule::~RadioConfigModule */


/*===========================================================================

FUNCTION:  RadioConfigModule::init()

===========================================================================*/
void RadioConfigModule::init()
{
  Module::init();

  // Initializations complete.
  mReady = false;
  mQmiUimUp = false;
  mServiceUp = false;

  ModemEndPointFactory<UimModemEndPoint>::getInstance().buildEndPoint();
} /* RadioConfigModule::init */


/*===========================================================================

FUNCTION:  RadioConfigModule::handleQcrilInitMessage

===========================================================================*/
void RadioConfigModule::handleQcrilInitMessage
(
  std::shared_ptr<QcrilInitMessage> msg
)
{
  QmiUimSetupRequestCallback qmiUimSetupCallback("Radio-Config-Module-Token");
  android::status_t          ret = android::OK;

  (void)msg;

  if (qmi_ril_get_process_instance_id() != QCRIL_DEFAULT_INSTANCE_ID)
  {
    return;
  }

  ModemEndPointFactory<UimModemEndPoint>::getInstance().buildEndPoint()->requestSetup(
                                      "Client-Radio-Config-Serv-Token", &qmiUimSetupCallback);

  radioConfigService = new RadioConfigImpl{};

  if (radioConfigService != NULL)
  {
    ret = radioConfigService->registerAsService("default");
  }
  QCRIL_LOG_INFO("radio_config_register_service status=%d", ret);
} /* RadioConfigModule::handleQcrilInitMessage */


/*===========================================================================

FUNCTION:  RadioConfigModule::handleQmiUimIndiaction

===========================================================================*/
void RadioConfigModule::handleQmiUimIndiaction
(
  std::shared_ptr<UimQmiUimIndicationMsg> uim_qmi_ind_msg_ptr
)
{
  if (uim_qmi_ind_msg_ptr != NULL)
  {
    switch(uim_qmi_ind_msg_ptr->get_ind_id())
    {
      case QMI_UIM_SRVC_UP_IND_MSG:
        mQmiUimUp = true;

        if (mServiceUp)
        {
          mReady = true;
          handleRadioConfigModuleReady();
        }
        break;

      case QMI_UIM_SRVC_DOWN_IND_MSG:
        {
          mReady = false;
          mQmiUimUp = false;
        }
        break;

      case QMI_UIM_SRVC_SLOT_STATUS_CHANGE_IND_MSG:
        {
          uint32_t                           param_len = 0;
          qcril_uim_indication_params_type * param_ptr =
            (qcril_uim_indication_params_type *)uim_qmi_ind_msg_ptr->get_message(&param_len);

          if (param_ptr != nullptr)
          {
            processSlotStatusInd(&param_ptr->ind_data.slot_status_change_ind);
          }
        }
        break;
    }
  }
} /* RadioConfigModule::handleQmiUimIndiaction */
#endif
