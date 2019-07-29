#pragma once

/*===========================================================================

  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#include "framework/Message.h"
#include <framework/legacy.h>
#include "DataCommon.h"
#include "framework/message_translator.h"
#include "framework/SolicitedMessage.h"
#include "framework/GenericCallback.h"
#include "framework/add_message_id.h"
#include "legacy/qmi_embms_v01.h"
#include <modules/android/RilRequestMessage.h>

namespace rildata {

class RilRequestEmbmsActivateTmgiMessage : public SolicitedMessage<generic_callback_payload>,
                                           public add_message_id<RilRequestEmbmsActivateTmgiMessage> {
    private:

    legacy_request_payload params;

    inline void deepCopy(qcril_request_params_type &p) {
        embms_activate_tmgi_req_msg_v01 *payload = new embms_activate_tmgi_req_msg_v01;
        memcpy(payload, (char *)&p.data, sizeof(embms_activate_tmgi_req_msg_v01));
        params.get_params().data = static_cast<void*>(payload);
    }

     public:

     static constexpr const char* MESSAGE_NAME = "HOOK_EMBMS_ACTIVATE_TMGI";
     RilRequestEmbmsActivateTmgiMessage() = delete;

     RilRequestEmbmsActivateTmgiMessage(const qcril_request_params_type &p)
     :SolicitedMessage<generic_callback_payload>(get_class_message_id()), params(p) {
        mName = MESSAGE_NAME;
        deepCopy(params.get_params());
    }
    ~RilRequestEmbmsActivateTmgiMessage();

    qcril_request_params_type &get_params() {
          return params.get_params();
    }
    string dump();
};

}//namespace
