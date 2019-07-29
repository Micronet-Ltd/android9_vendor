/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "wireless_data_service_v01.h"
#include "qmi_client.h"
#include "common_v01.h"
#include <list>
#include "modules/qmi/ModemEndPoint.h"
#include "modules/qmi/ModemEndPointModule.h"
#include "modules/qmi/QmiServiceUpIndMessage.h"
#include "modules/qmi/QmiSetupRequest.h"
#include "sync/SetAttachListSyncMessage.h"
#include "sync/GetAttachListCapabilitySyncMessage.h"

#define QCRIL_DATA_QMI_TIMEOUT 10000

class WDSModemEndPointModule : public ModemEndPointModule
{
private:
	enum AttachAction
	{
    ATTACH_PDN_ACTION_NOT_SUPPORTED =0,
    ATTACH_PDN_ACTION_SUPPORTED
	};
  qmi_idl_service_object_type getServiceObject() override;
  bool handleQmiBinding(qcril_instance_id_e_type instanceId,
                        int8_t stackId) override;
  void handleGetAttachList(std::shared_ptr<Message> msg);
  void handleSetAttachList(std::shared_ptr<Message> msg);
  void handleGetAttachListCap(std::shared_ptr<Message> msg);
  void handleWdsQmiIndMessage(std::shared_ptr<Message> msg);
  void WdsUnsolicitedIndHdlr(unsigned int   msg_id,
  unsigned char *decoded_payload,
  uint32_t       decoded_payload_len);

public:
  WDSModemEndPointModule(string name, ModemEndPoint& owner);
  virtual ~WDSModemEndPointModule();
  void init();
};
