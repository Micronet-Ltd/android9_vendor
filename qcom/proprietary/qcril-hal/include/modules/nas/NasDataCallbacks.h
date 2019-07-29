/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include <framework/GenericCallback.h>
#include <modules/android/RilRequestMessage.h>
#include "SetLinkCapRptCriteriaMessage.h"
#ifdef TARGET_USES_QCOM_BSP_ATEL
#include <DataCommon.h>
#endif

#ifndef TARGET_USES_QCOM_BSP_ATEL
class NasDataGetDdsSubIdCallback : public GenericCallback<int> {
#else
class NasDataGetDdsSubIdCallback : public GenericCallback<DDSSubIdInfo> {
#endif
private:
        legacy_request_payload params;
        boolean from_allow_data;
  public:
    inline NasDataGetDdsSubIdCallback(string clientToken,
        const qcril_request_params_type &p, boolean from_allow_data) :
          GenericCallback(clientToken), params(p), from_allow_data(from_allow_data) {};

    inline ~NasDataGetDdsSubIdCallback() {};

    Message::Callback *clone() override;

#ifndef TARGET_USES_QCOM_BSP_ATEL
    void onResponse(std::shared_ptr<Message> solicitedMsg, Status status,
        std::shared_ptr<int> responseDataPtr) override;
#else
    void onResponse(std::shared_ptr<Message> solicitedMsg, Status status,
        std::shared_ptr<DDSSubIdInfo> responseDataPtr) override;
#endif

    qcril_request_params_type &get_params() {
        return params.get_params();
    }
};

class NasDataRequestDdsSwitchCallback : public GenericCallback<RIL_Errno> {
private:
        legacy_request_payload params;
  public:
    inline NasDataRequestDdsSwitchCallback(string clientToken,
        const qcril_request_params_type &p) :
          GenericCallback(clientToken), params(p){};

    inline ~NasDataRequestDdsSwitchCallback() {};

    Message::Callback *clone() override;

    void onResponse(std::shared_ptr<Message> solicitedMsg, Status status,
        std::shared_ptr<RIL_Errno> responseDataPtr) override;

    qcril_request_params_type &get_params() {
        return params.get_params();
    }
};

// TODO move to a separate file later
class NasDataSetLinkCapRptFilterCallback : public GenericCallback<int> {
private:
        legacy_request_payload params;
  public:
    inline NasDataSetLinkCapRptFilterCallback(string clientToken,
        const qcril_request_params_type &p) :
          GenericCallback(clientToken), params(p) {};

    inline ~NasDataSetLinkCapRptFilterCallback() {};

    Message::Callback *clone() override;

    void onResponse(std::shared_ptr<Message> solicitedMsg, Status status,
        std::shared_ptr<int> responseDataPtr) override;

    qcril_request_params_type &get_params() {
        return params.get_params();
    }
};
