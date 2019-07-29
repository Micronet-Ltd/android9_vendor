/**
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/
#pragma once

#include <string>
#include "DataCommon.h"
#include "authentication_service_v01.h"

typedef struct {
  bool valid;
  char mcc[QMI_AUTH_MCC_MNC_SIZE_V01+1];
  char mnc[QMI_AUTH_MCC_MNC_SIZE_V01+1];

  uint32_t carrierKeyLen;
  uint8_t carrierKey[QMI_AUTH_IMSI_PUBLIC_KEY_MAX_V01];

  uint32_t keyIdentifierLen;
  char keyIdentifier[QMI_AUTH_IMSI_KEY_ID_AVP_MAX_V01];

  int64_t expirationTime;
} CarrierImsiKeyType;

class AuthManager {
 public:
  AuthManager() = default;
  ~AuthManager(){}

  void init();
  bool isCarrierImsiKeyValid();
  void updateModemWithCarrierImsiKeyCache();
  void qmiAuthServiceRegisterIndications(bool flag);
  void qmiAuthServiceIndicationHandler(unsigned int msg_id, unsigned char *payload, uint32_t payload_len);
  void setCarrierInfoImsiEncryption(const qcril_request_params_type *const params_ptr);

 private:
  CarrierImsiKeyType carrierImsiKeyCache;

  void resetCarrierImsiKeyCache();
  void updateCarrierImsiKeyCache(RIL_CarrierInfoForImsiEncryption *in_data_ptr);
  void readCarrierImsiKeyCache(auth_send_imsi_public_key_req_msg_v01 *req);
};
