/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "data_system_determination_v01.h"
#include "qmi_client.h"
#include "common_v01.h"
#include "modules/qmi/ModemEndPoint.h"
#include "modules/qmi/ModemEndPointModule.h"
#include "modules/qmi/QmiServiceUpIndMessage.h"
#include "modules/qmi/QmiSetupRequest.h"

#define QCRIL_DATA_QMI_TIMEOUT 10000

class DSDModemEndPointModule : public ModemEndPointModule
{
private:
  qmi_idl_service_object_type getServiceObject() override;
  bool handleQmiBinding(qcril_instance_id_e_type instanceId, int8_t stackId) override;
  void handleQmiDsdIndMessage(std::shared_ptr<Message> msg);
  void indicationHandler(unsigned int msg_id, unsigned char *decoded_payload, uint32_t decoded_payload_len);
  void processUiInfoInd(dsd_ui_info_ind_msg_v01 *ind_data);
  void handleSetApnInfoSync(std::shared_ptr<Message> msg);
  boolean isApnTypeFound(const RIL_ApnTypes &apn_type, const uint8_t &apn_types_mask);
  void sendApnInfoQmi( const std::string &apn_name,
  dsd_apn_type_enum_v01 apn_type);

public:
  DSDModemEndPointModule(string name, ModemEndPoint& owner);
  virtual ~DSDModemEndPointModule();

  void init();
};
