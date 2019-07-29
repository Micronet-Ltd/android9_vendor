/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include <vector>
#include "telephony/ril.h"
#include "qcril_qmi_ims_sms.h"

using namespace vendor::qti::hardware::radio::ims;

RIL_IMS_SMS_Message* getRILImsSmsFormatPayload(const V1_2::ImsSmsMessage& imsSms);
void freeRILImsSmsFormatPayload(RIL_IMS_SMS_Message* ril_ims_msg);
bool convertCdmaFormatToPseudoPdu(RIL_CDMA_SMS_Message& cdmaMsg, std::vector<uint8_t>& payload);
void dumpRILCdmaFormat(RIL_CDMA_SMS_Message& cdmaMsg);
