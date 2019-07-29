/**
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#include "qcrili.h"
#include "modules/qmi/ModemEndPointFactory.h"
#include "modules/qmi/QmiSetupRequestCallback.h"

#include "AuthManager.h"
#include "AuthModemEndPoint.h"

void AuthManager::init()
{
  Log::getInstance().d("[AuthManager]: init");

  QmiSetupRequestCallback qmiAuthSetupCallback("AuthModule-Token");
  ModemEndPointFactory<AuthModemEndPoint>::getInstance().buildEndPoint()->requestSetup(
    "AuthModule-Token",
    &qmiAuthSetupCallback);
}

bool AuthManager::isCarrierImsiKeyValid()
{
  Log::getInstance().d("[AuthManager]: isCarrierImsiKeyValid = " + std::to_string(carrierImsiKeyCache.valid));
  return carrierImsiKeyCache.valid;
}

void AuthManager::resetCarrierImsiKeyCache()
{
  Log::getInstance().d("[AuthManager]: resetCarrierImsiKeyCache");
  memset(&carrierImsiKeyCache, 0, sizeof(CarrierImsiKeyType));
}

void AuthManager::qmiAuthServiceRegisterIndications(bool flag)
{
  int rc;
  Log::getInstance().d("[AuthManager]: qmiAuthServiceRegisterIndications = " + std::to_string(flag));

  auth_indication_register_req_msg_v01 req;
  auth_indication_register_resp_msg_v01 resp;
  memset(&req, 0, sizeof(req));

  if (flag) {
    req.report_imsi_public_key_request_valid  = true;
    req.report_imsi_public_key_request        = true;
  }

  rc = ModemEndPointFactory<AuthModemEndPoint>::getInstance().buildEndPoint()->sendRawSync(
    QMI_AUTH_INDICATION_REGISTER_REQ_V01,
    &req, sizeof(req),
    &resp, sizeof(resp));

  if (rc!=QMI_NO_ERR) {
    Log::getInstance().d("[AuthManager]: Failed to register Auth service indication rc = " + std::to_string(rc)
     + " resp.result = " + std::to_string(resp.resp.result)
     + " resp.error = " + std::to_string(resp.resp.error));
  }
}

void AuthManager::qmiAuthServiceIndicationHandler(unsigned int msg_id, unsigned char *payload, uint32_t payload_len)
{
  Log::getInstance().d("[AuthManager]: qmiAuthServiceIndicationHandler = " + std::to_string(msg_id) + std::to_string(payload_len));

  auth_retrieve_imsi_public_key_ind_msg_v01 *ind_data = nullptr;

  switch (msg_id)
  {
    case QMI_AUTH_RETRIEVE_IMSI_PUBLIC_KEY_IND_V01:
      ind_data = (auth_retrieve_imsi_public_key_ind_msg_v01 *)payload;
      if(ind_data->action == AUTH_IMSI_PUBLIC_KEY_REFRESH_V01) {
        Log::getInstance().d("[AuthManager]: Refresh is needed");
        resetCarrierImsiKeyCache();

        qcril_unsol_resp_params_type unsol_resp;
        unsol_resp.instance_id = QCRIL_DEFAULT_INSTANCE_ID;
        unsol_resp.response_id = RIL_UNSOL_CARRIER_INFO_IMSI_ENCRYPTION;
        unsol_resp.resp_pkt = NULL;
        unsol_resp.resp_len = 0;
        unsol_resp.logstr = NULL;
        qcril_send_unsol_response( &unsol_resp );
      }
      else {
        Log::getInstance().d("[AuthManager]: Update modem with cached carrier IMSI key");
        updateModemWithCarrierImsiKeyCache();
      }
    break;

    default:
      Log::getInstance().d("[AuthManager]: Unexpected indication = " + std::to_string(msg_id));
    break;
  }
}

void AuthManager::updateModemWithCarrierImsiKeyCache()
{
  Log::getInstance().d("[AuthManager]: updateModemWithCarrierImsiKeyCache");

  int rc;
  auth_send_imsi_public_key_req_msg_v01 req;
  auth_send_imsi_public_key_resp_msg_v01 resp;
  memset(&req, 0, sizeof(req));

  readCarrierImsiKeyCache(&req);

  rc = ModemEndPointFactory<AuthModemEndPoint>::getInstance().buildEndPoint()->sendRawSync(
    QMI_AUTH_SEND_IMSI_PUBLIC_KEY_REQ_V01,
    &req, sizeof(req),
    &resp, sizeof(resp));

  if (rc!=QMI_NO_ERR) {
    Log::getInstance().d("[AuthManager]: Failed to send QMI msg rc = " + std::to_string(rc)
     + " resp.result = " + std::to_string(resp.resp.result)
     + " resp.error = " + std::to_string(resp.resp.error));
  }
}

void AuthManager::updateCarrierImsiKeyCache(RIL_CarrierInfoForImsiEncryption *in_data_ptr)
{
  unsigned size;
  Log::getInstance().d("[AuthManager]: updateCarrierImsiKeyCache");
  resetCarrierImsiKeyCache();

  size = MIN(strlen(in_data_ptr->mcc), QMI_AUTH_MCC_MNC_SIZE_V01)+1;
  memcpy(carrierImsiKeyCache.mcc, in_data_ptr->mcc, size);
  size = MIN(strlen(in_data_ptr->mnc), QMI_AUTH_MCC_MNC_SIZE_V01)+1;
  memcpy(carrierImsiKeyCache.mnc, in_data_ptr->mnc, size);
  carrierImsiKeyCache.carrierKeyLen = MIN(in_data_ptr->carrierKeyLength, QMI_AUTH_IMSI_PUBLIC_KEY_MAX_V01);
  memcpy(carrierImsiKeyCache.carrierKey, in_data_ptr->carrierKey, carrierImsiKeyCache.carrierKeyLen);
  carrierImsiKeyCache.keyIdentifierLen = MIN(strlen(in_data_ptr->keyIdentifier), QMI_AUTH_IMSI_KEY_ID_AVP_MAX_V01);
  memcpy(carrierImsiKeyCache.keyIdentifier, in_data_ptr->keyIdentifier, carrierImsiKeyCache.keyIdentifierLen);

  carrierImsiKeyCache.expirationTime = in_data_ptr->expirationTime;
  carrierImsiKeyCache.valid = true;
}

void AuthManager::readCarrierImsiKeyCache(auth_send_imsi_public_key_req_msg_v01 *req)
{
  unsigned size;
  Log::getInstance().d("[AuthManager]: readCarrierImsiKeyCache");
  if (carrierImsiKeyCache.valid) {
    req->public_key_len = carrierImsiKeyCache.carrierKeyLen;
    memcpy(req->public_key, carrierImsiKeyCache.carrierKey, req->public_key_len);

    req->key_identifier_valid = true;
    req->key_identifier_len = carrierImsiKeyCache.keyIdentifierLen;
    memcpy(req->key_identifier, carrierImsiKeyCache.keyIdentifier, req->key_identifier_len);

    req->mcc_mnc_info_valid = true;
    size = strlen(carrierImsiKeyCache.mcc)+1;
    strlcpy(req->mcc_mnc_info.mcc, carrierImsiKeyCache.mcc, size);

    size = strlen(carrierImsiKeyCache.mnc)+1;
    strlcpy(req->mcc_mnc_info.mnc, carrierImsiKeyCache.mnc, size);

    req->expiry_time_valid = true;
    req->expiry_time = carrierImsiKeyCache.expirationTime;
  }
  else {
    Log::getInstance().d("[AuthManager]: carrierImsiKeyCache is not valid");
    qcril_unsol_resp_params_type unsol_resp;
    unsol_resp.instance_id = QCRIL_DEFAULT_INSTANCE_ID;
    unsol_resp.response_id = RIL_UNSOL_CARRIER_INFO_IMSI_ENCRYPTION;
    unsol_resp.resp_pkt = NULL;
    unsol_resp.resp_len = 0;
    unsol_resp.logstr = NULL;
    qcril_send_unsol_response( &unsol_resp );
  }
}

void AuthManager::setCarrierInfoImsiEncryption(const qcril_request_params_type *const params_ptr)
{
  Log::getInstance().d("[AuthManager]: setCarrierInfoImsiEncryption");

  RIL_CarrierInfoForImsiEncryption *in_data_ptr = NULL;
  in_data_ptr = (RIL_CarrierInfoForImsiEncryption *)params_ptr->data;

  updateCarrierImsiKeyCache(in_data_ptr);
  updateModemWithCarrierImsiKeyCache();
}
