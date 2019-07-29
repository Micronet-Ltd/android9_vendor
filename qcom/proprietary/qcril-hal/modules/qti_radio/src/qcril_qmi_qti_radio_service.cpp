/******************************************************************************
  @ file    qcril_qmi_qti_radio_service.cpp
  @brief   qcril qmi - qtiRadioService_service

  DESCRIPTION
    Implements the server side of the IQtiqtiRadioService interface. Handles RIL
    requests and responses and indications to be received and sent to client
    respectively

  ---------------------------------------------------------------------------

  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

#define TAG "RILQ"

#include "qcril_qmi_qti_radio_service.h"
#include "qcril_log.h"
#include "modules/qti_radio/UimGetAtrMsgCallback.h"
#include "modules/uim/UimGetAtrRequestMsg.h"
#include "qcril_reqlist.h"
#include <modules/android/qcril_android_event_context.h>
#include "modules/android/ril_request_info.h"
#include "modules/sms/RilRequestCdmaSendSmsMessage.h"
#include "modules/qti_radio/RilRequestCdmaSendSmsMessageCallback.h"
#include "modules/qti_radio/RilRequestNrMessageCallback.h"
#include "GetDataNrIconTypeMessage.h"

using namespace vendor::qti::hardware::radio::qtiradio::V1_0::implementation;
using ::android::hardware::Void;
using ::vendor::qti::hardware::radio::qtiradio::V2_0::Status;

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V1_0 {
namespace implementation {

static sp<QtiRadioImpl> qtiRadioService = nullptr;

QtiRadioImpl::QtiRadioImpl(qcril_instance_id_e_type instanceId) {
  mResponseCb = nullptr;
  mIndicationCb = nullptr;
  mResponseCbV2_0 = nullptr;
  mIndicationCbV2_0 = nullptr;
  mResponseCbV2_1 = nullptr;
  mIndicationCbV2_1 = nullptr;
  mResponseCbV2_2 = nullptr;
  mIndicationCbV2_2 = nullptr;
  mInstanceId = instanceId;
}

/*
 * Service Died callback.
 */
void QtiRadioImpl::serviceDied(uint64_t,
                               const ::android::wp<::android::hidl::base::V1_0::IBase> &) {
  QCRIL_LOG_DEBUG("QtiRadioImpl::serviceDied: Client died. Cleaning up callbacks");
  clearCallbacks();
}

/*
 * Resets indication callback upon client's death
 */
void QtiRadioImpl::clearCallbacks() {
  RWLock::AutoWLock lock(mCallbackLock);
  if (mIndicationCb != nullptr) {
    mIndicationCb = nullptr;
  }
  if (mResponseCb != nullptr) {
    mResponseCb = nullptr;
  }
  if (mResponseCbV2_0 != nullptr) {
    mResponseCbV2_0 = nullptr;
  }
  if (mIndicationCbV2_0 != nullptr) {
    mIndicationCbV2_0 = nullptr;
  }
  if (mResponseCbV2_1 != nullptr) {
    mResponseCbV2_1 = nullptr;
  }
  if (mIndicationCbV2_1 != nullptr) {
    mIndicationCbV2_1 = nullptr;
  }
  if (mResponseCbV2_2 != nullptr) {
    mResponseCbV2_2 = nullptr;
  }
  if (mIndicationCbV2_2 != nullptr) {
    mIndicationCbV2_2 = nullptr;
  }
}

//===========================================================================
// QtiRadioImpl::setCallback
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Registers the callbacks for IQtiRadioResponse and IQtiRadioIndication
    passed by the client as a parameter

    @return
    None
*/
/*=========================================================================*/
Return<void> QtiRadioImpl::setCallback(const sp<IQtiRadioResponse> &responseCallback,
                                       const sp<IQtiRadioIndication> &indicationCallback) {
  QCRIL_LOG_DEBUG("QtiRadioImpl::setCallback");
  RWLock::AutoWLock lock(mCallbackLock);
  if (mResponseCb != nullptr) {
    mResponseCb->unlinkToDeath(this);
  }
  mIndicationCb = indicationCallback;
  mResponseCb = responseCallback;
  if (mResponseCb != nullptr) {
    mResponseCb->linkToDeath(this, 0);

    mResponseCbV2_0 = V2_0::IQtiRadioResponse::castFrom(mResponseCb).withDefault (nullptr);
    mIndicationCbV2_0 = V2_0::IQtiRadioIndication::castFrom(mIndicationCb).withDefault (nullptr);

    mResponseCbV2_1 = V2_1::IQtiRadioResponse::castFrom(responseCallback).withDefault (nullptr);
    mIndicationCbV2_1 = V2_1::IQtiRadioIndication::castFrom(indicationCallback).withDefault (nullptr);

    mResponseCbV2_2 = V2_2::IQtiRadioResponse::castFrom(responseCallback).withDefault (nullptr);
    mIndicationCbV2_2 = V2_2::IQtiRadioIndication::castFrom(indicationCallback).withDefault (nullptr);
  }

  return Void();
}

sp<IQtiRadioResponse> QtiRadioImpl::getResponseCallback() {
  RWLock::AutoRLock lock(mCallbackLock);
  return mResponseCb;
}

sp<IQtiRadioIndication> QtiRadioImpl::getIndicationCallback() {
  RWLock::AutoRLock lock(mCallbackLock);
  return mIndicationCb;
}

sp<V2_0::IQtiRadioResponse> QtiRadioImpl::getResponseCallbackV2_0() {
    RWLock::AutoRLock lock(mCallbackLock);
    return mResponseCbV2_0;
}

sp<V2_0::IQtiRadioIndication> QtiRadioImpl::getIndicationCallbackV2_0() {
    RWLock::AutoRLock lock(mCallbackLock);
    return mIndicationCbV2_0;
}

sp<V2_1::IQtiRadioResponse> QtiRadioImpl::getResponseCallbackV2_1() {
    RWLock::AutoRLock lock(mCallbackLock);
    return mResponseCbV2_1;
}

sp<V2_1::IQtiRadioIndication> QtiRadioImpl::getIndicationCallbackV2_1() {
    RWLock::AutoRLock lock(mCallbackLock);
    return mIndicationCbV2_1;
}

sp<V2_2::IQtiRadioResponse> QtiRadioImpl::getResponseCallbackV2_2() {
    RWLock::AutoRLock lock(mCallbackLock);
    return mResponseCbV2_2;
}

sp<V2_2::IQtiRadioIndication> QtiRadioImpl::getIndicationCallbackV2_2() {
    RWLock::AutoRLock lock(mCallbackLock);
    return mIndicationCbV2_2;
}



//===========================================================================
// QtiRadioImpl::getAtr
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the getAtr request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> QtiRadioImpl::getAtr(int32_t serial) {
  QCRIL_LOG_ERROR("QtiRadioImpl::getAtr serial=%d", serial);
  RIL_Token token = qcril_qmi_qti_convert_radio_token_to_ril_token(serial /*request token*/);
  UimGetAtrMsgCallback cb("get-atr");
  std::shared_ptr<UimGetAtrRequestMsg> req = std::make_shared<UimGetAtrRequestMsg>(token, &cb);
  if (req) {
    req->dispatch();
  }
  return Void();
}

Return<void> QtiRadioImpl::sendCdmaSms(int32_t serial,
        const android::hardware::radio::V1_0::CdmaSmsMessage& sms, bool expectMore) {
    QCRIL_LOG_DEBUG("QtiRadioImpl:: sendCdmaSms serial=%d expectMore=%d", serial, expectMore);
    RIL_Token token = qcril_qmi_qti_convert_radio_token_to_ril_token(serial /*request token*/);
    if(token != nullptr) {
        RIL_CDMA_SMS_Message rcsm = {};
#ifndef QMI_RIL_UTF
        constructCdmaSms(rcsm, sms);
#endif
        rcsm.expectMore = 0;
        if(expectMore) {
            rcsm.expectMore = 1;
        }
        qcril_event_context *ctx =
                new qcril_event_context(mInstanceId, RIL_REQUEST_CDMA_SEND_SMS, token);
        struct ril_request_type req  = {};
        req.instance_id = mInstanceId;
        req.req_id_android = RIL_REQUEST_CDMA_SEND_SMS;
        req.payload = &rcsm;
        req.payload_len = sizeof(rcsm);
        req.t = static_cast<RIL_Token>(ctx);
        RilRequestCdmaSendSmsMessageCallback cb("QTI_RIL_REQUEST_CDMA_SEND_SMS");
        std::shared_ptr<RilRequestCdmaSendSmsMessage>msg =
                std::make_shared<RilRequestCdmaSendSmsMessage>(req);
        if (ctx != nullptr && msg != nullptr) {
            ctx->msg = msg;
            msg->setCallback(&cb);
            msg->dispatch();
        } else {
            QtiRadioResponseInfo responseInfo = {};
            android::hardware::radio::V1_0::SendSmsResult result = {};
            populateResponseInfo(responseInfo, serial, QtiRadioResponseType::SOLICITED,
                                 RIL_E_GENERIC_FAILURE);
            result.ackPDU = hidl_string();
            sp<V2_0::IQtiRadioResponse> respCb = getResponseCallbackV2_0();
            if (respCb != nullptr) {
                Return<void> ret = respCb->sendCdmaSmsResponse(responseInfo, result);
                if (!ret.isOk()) {
                    QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
                                    ret.description().c_str());
                }
            }
            if (ctx != nullptr) {
                delete ctx;
            }
        }
    }
    return Void();
}

void QtiRadioImpl::sendCdmaSmsResponse(RIL_Token t, RIL_Errno e, void *response,
        size_t responselen) {
    sp<V2_0::IQtiRadioResponse> respCb = getResponseCallbackV2_0();
    qcril_event_context *ctx = static_cast<qcril_event_context *>(t);
    if (respCb != nullptr && ctx != nullptr) {
        int serial = qcril_qmi_qti_free_and_convert_ril_token_to_radio_token(ctx->t);
        delete ctx;
        QCRIL_LOG_DEBUG("QtiRadioImpl::sendCdmaSmsResponse, serial=%d", serial);
        QtiRadioResponseInfo responseInfo = {};
        android::hardware::radio::V1_0::SendSmsResult result = {};
        populateResponseInfo(responseInfo, serial, QtiRadioResponseType::SOLICITED, e);
        hidl_string retAck;
        if (response == NULL || responselen != sizeof(RIL_SMS_Response)) {
            if (e == RIL_E_SUCCESS) responseInfo.error = QtiRadioError::GENERIC_FAILURE;
            result.ackPDU = hidl_string();
        } else {
            RIL_SMS_Response *resp = (RIL_SMS_Response *) response;
            result.messageRef = resp->messageRef;
            if (resp->ackPDU != nullptr) {
                retAck.setToExternal(resp->ackPDU, strlen(resp->ackPDU));
            }
            result.ackPDU = retAck;
            result.errorCode = resp->errorCode;
        }
        Return<void> ret = respCb->sendCdmaSmsResponse(responseInfo, result);
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
        }
    } else {
        QCRIL_LOG_ERROR("QtiRadioImpl::sendCdmaSmsResponse respCb NULL or ctx NULL");
    }
}

Return<void> QtiRadioImpl::enable5g(int32_t serial) {
    QCRIL_LOG_ERROR("QtiRadioImpl::enable5g, serial=%d", serial);
    RIL_Token token = qcril_qmi_qti_convert_radio_token_to_ril_token(serial);
    if (token != nullptr) {
        onRequest(RIL_REQUEST_ENABLE_5G, NULL, 0, token);
    }
    return Void();
}

void QtiRadioImpl::enable5gResponse(RIL_Token token, RIL_Errno err_num) {
    if (mResponseCbV2_0 != nullptr) {
        int serial = qcril_qmi_qti_free_and_convert_ril_token_to_radio_token(token);
        QCRIL_LOG_ERROR ("QtiRadioImpl::enable5g serial=%d", serial);
        Return<void> ret = mResponseCbV2_0->onEnable5gResponse(serial,
                static_cast<uint32_t>(err_num), Status::SUCCESS);
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
        }
    }
    else {
        QCRIL_LOG_ERROR("QtiRadioImpl::enable5g respCb NULL");
    }
}

Return<void> QtiRadioImpl::disable5g(int32_t serial) {
    QCRIL_LOG_ERROR("QtiRadioImpl::disable5g, serial=%d", serial);
    RIL_Token token = qcril_qmi_qti_convert_radio_token_to_ril_token(serial);
    if (token != nullptr) {
        onRequest(RIL_REQUEST_DISABLE_5G, NULL, 0, token);
    }
    return Void();
}

void QtiRadioImpl::disable5gResponse(RIL_Token token, RIL_Errno err_num) {
    if (mResponseCbV2_0 != nullptr) {
        int serial = qcril_qmi_qti_free_and_convert_ril_token_to_radio_token(token);
        QCRIL_LOG_ERROR ("QtiRadioImpl::disable5g serial=%d", serial);
        Return<void> ret = mResponseCbV2_0->onDisable5gResponse(serial,
                static_cast<uint32_t>(err_num), Status::SUCCESS);
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
        }
    }
    else {
        QCRIL_LOG_ERROR("QtiRadioImpl::disable5g respCb NULL");
    }
}

Return<void> QtiRadioImpl::enable5gOnly (int32_t serial) {
    QCRIL_LOG_ERROR("QtiRadioImpl::enable5gOnly, serial=%d", serial);
    RIL_Token token = qcril_qmi_qti_convert_radio_token_to_ril_token(serial);
    if (token != nullptr) {
        onRequest(RIL_REQUEST_ENABLE_5G_ONLY, NULL, 0, token);
    }
    return Void();
}

void QtiRadioImpl::enable5gOnlyResponse(RIL_Token token, RIL_Errno err_num) {
    if (mResponseCbV2_0 != nullptr) {
        int serial = qcril_qmi_qti_free_and_convert_ril_token_to_radio_token(token);
        QCRIL_LOG_ERROR ("QtiRadioImpl::enable5gOnly serial=%d", serial);
        Return<void> ret = mResponseCbV2_0->onEnable5gOnlyResponse(serial,
                static_cast<uint32_t>(err_num), Status::SUCCESS);
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
        }
    }
    else {
        QCRIL_LOG_ERROR("QtiRadioImpl::enable5gOnly respCb NULL");
    }
}

Return<void> QtiRadioImpl::query5gStatus (int32_t serial) {
    QCRIL_LOG_ERROR ("QtiRadioImpl::query5gStatus, serial=%d", serial);
    RIL_Token token = qcril_qmi_qti_convert_radio_token_to_ril_token(serial);
    if (token != nullptr) {
        onRequest(RIL_REQUEST_QUERY_5G_STATUS, NULL, 0, token);
    }
    return Void();
}

void QtiRadioImpl::query5gStatusResponse(RIL_Token token, RIL_Errno err_num,
        five_g_status status) {
    if (mResponseCbV2_0 != nullptr) {
        int serial = qcril_qmi_qti_free_and_convert_ril_token_to_radio_token(token);
        QCRIL_LOG_ERROR ("QtiRadioImpl::query5gStatus serial=%d", serial);
        Return<void> ret = mResponseCbV2_0->on5gStatusResponse(serial,
                static_cast<uint32_t>(err_num), convert_five_g_status(status));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
        }
    }
    else {
        QCRIL_LOG_ERROR("QtiRadioImpl::query5gStatus respCb NULL");
    }
}

Return<void> QtiRadioImpl::queryNrDcParam (int32_t serial) {
    QCRIL_LOG_ERROR ("QtiRadioImpl::queryNrDcParam, serial=%d", serial);
    RIL_Token token = qcril_qmi_qti_convert_radio_token_to_ril_token(serial);
    if (token != nullptr) {
        onRequest(RIL_REQUEST_QUERY_ENDC_DCNR, NULL, 0, token);
    }
    return Void();
}

void QtiRadioImpl::queryNrDcParamResponse(RIL_Token token, RIL_Errno err_num,
        five_g_endc_dcnr endc_available_restrict_dcnr)
{
    if (mResponseCbV2_0 != nullptr) {
        int serial = qcril_qmi_qti_free_and_convert_ril_token_to_radio_token(token);
        QCRIL_LOG_ERROR ("QtiRadioImpl::queryNrDcParam serial=%d", serial);
        Return<void> ret = mResponseCbV2_0->onNrDcParamResponse(serial,
                static_cast<uint32_t>(err_num), convert_five_g_endc_dcnr(endc_available_restrict_dcnr));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
        }
    }
    else {
        QCRIL_LOG_ERROR("QtiRadioImpl::queryNrDcParam respCb NULL");
    }
}

Return<void> QtiRadioImpl::queryNrBearerAllocation (int32_t serial) {
    QCRIL_LOG_ERROR ("QtiRadioImpl::queryNrBearerAllocation, serial=%d", serial);
    RIL_Token token = qcril_qmi_qti_convert_radio_token_to_ril_token(serial);
    if (token != nullptr) {
        onRequest(RIL_REQUEST_QUERY_NR_BEARER_ALLOCATION, NULL, 0, token);
    }
    return Void();
}

void QtiRadioImpl::queryNrBearerAllocationResponse(RIL_Token token,
        RIL_Errno err_num, five_g_bearer_status bearerStatus) {
    int serial = qcril_qmi_qti_free_and_convert_ril_token_to_radio_token(token);
    QCRIL_LOG_ERROR ("QtiRadioImpl::queryNrBearerAllocation serial=%d", serial);
    if (mResponseCbV2_1 != nullptr) {
        Return<void> ret = mResponseCbV2_1->onNrBearerAllocationResponse_2_1(serial,
                static_cast<uint32_t>(err_num), convert_five_g_bearer_status_2_1(bearerStatus));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
        }

    } else if (mResponseCbV2_0 != nullptr) {
        Return<void> ret = mResponseCbV2_0->onNrBearerAllocationResponse(serial,
                static_cast<uint32_t>(err_num), convert_five_g_bearer_status(bearerStatus));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
        }
    } else {
        QCRIL_LOG_ERROR("QtiRadioImpl::queryNrBearerAllocation respCb NULL");
    }
}

Return<void> QtiRadioImpl::queryNrSignalStrength (int32_t serial) {
    QCRIL_LOG_ERROR ("QtiRadioImpl::queryNrSignalStrength, serial=%d", serial);
    RIL_Token token = qcril_qmi_qti_convert_radio_token_to_ril_token(serial);
    if (token != nullptr) {
        onRequest(RIL_REQUEST_QUERY_NR_SIGNAL_STRENGTH, NULL, 0, token);
    }
    return Void();
}

void QtiRadioImpl::queryNrSignalStrengthResponse(RIL_Token token, RIL_Errno err_num,
        five_g_signal_strength signalStrength) {
    if (mResponseCbV2_0 != nullptr) {
        int serial = qcril_qmi_qti_free_and_convert_ril_token_to_radio_token(token);
        QCRIL_LOG_ERROR ("QtiRadioImpl::queryNrSignalStrength serial=%d", serial);
        Return<void> ret = mResponseCbV2_0->onSignalStrengthResponse(serial,
                static_cast<uint32_t>(err_num), convert_five_g_signal_strength(signalStrength));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
        }
    }
    else {
        QCRIL_LOG_ERROR("QtiRadioImpl::queryNrSignalStrength respCb NULL");
    }
}

Return<void> QtiRadioImpl::queryUpperLayerIndInfo(int32_t serial) {
    QCRIL_LOG_ERROR ("QtiRadioImpl::queryUpperLayerIndInfo, serial=%d", serial);
    RIL_Token token = qcril_qmi_qti_convert_radio_token_to_ril_token(serial);
    if (token != nullptr) {
        onRequest(RIL_REQUEST_QUERY_UPPERLAYER_IND_INFO, NULL, 0, token);
    }
    return Void();
}

void QtiRadioImpl::queryUpperLayerIndInfoResponse(RIL_Token token, RIL_Errno err_num,
        five_g_upper_layer_ind_info upli_info) {
    if (mResponseCbV2_1 != nullptr) {
        int serial = qcril_qmi_qti_free_and_convert_ril_token_to_radio_token(token);
        QCRIL_LOG_ERROR ("QtiRadioImpl::queryNrSignalStrength serial=%d", serial);
        Return<void> ret = mResponseCbV2_1->onUpperLayerIndInfoResponse(serial,
            static_cast<uint32_t>(err_num), convert_five_g_upper_layer_ind_info(upli_info));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
        }
    }
    else {
        QCRIL_LOG_ERROR("QtiRadioImpl::queryUpperLayerIndInfoResponse respCb NULL");
    }
}

Return<void> QtiRadioImpl::query5gConfigInfo(int32_t serial) {
    QCRIL_LOG_ERROR ("QtiRadioImpl::query5gConfigInfo, serial=%d", serial);
    RIL_Token token = qcril_qmi_qti_convert_radio_token_to_ril_token(serial);
    if (token != nullptr) {
        onRequest(RIL_REQUEST_QUERY_5G_CONFIG_INFO, NULL, 0, token);
    }
    return Void();
}

void QtiRadioImpl::query5gConfigInfoResponse(RIL_Token token, RIL_Errno err_num,
	    five_g_config_type config) {
    if (mResponseCbV2_1 != nullptr) {
        int serial = qcril_qmi_qti_free_and_convert_ril_token_to_radio_token(token);
        QCRIL_LOG_ERROR ("QtiRadioImpl::query5gConfigInfoResponse serial=%d", serial);
        Return<void> ret = mResponseCbV2_1->on5gConfigInfoResponse(serial,
                static_cast<uint32_t>(err_num), convert_five_g_config_info(config));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
        }
    }
    else {
        QCRIL_LOG_ERROR("QtiRadioImpl::query5gConfigInfoResponse respCb NULL");
    }
}

Return<void> QtiRadioImpl::queryNrIconType(int32_t serial) {
    QCRIL_LOG_DEBUG ("QtiRadioImpl::queryNrIconType, serial=%d", serial);
    RIL_Token token = qcril_qmi_qti_convert_radio_token_to_ril_token(serial);
    if (token) {
        std::shared_ptr<rildata::GetDataNrIconTypeMessage> msg =
            std::make_shared<rildata::GetDataNrIconTypeMessage>();
        if(msg)
        {
            QtiRadioGetDataNrIconTypeMessageCb cb(token);
            msg->setCallback(&cb);
            msg->dispatch();
        }
    }
    return Void();
}

void QtiRadioImpl::queryNrIconTypeResponse(RIL_Token token, RIL_Errno err_num,
        five_g_icon_type iconType) {
    if (mResponseCbV2_2 != nullptr) {
        int serial = qcril_qmi_qti_free_and_convert_ril_token_to_radio_token(token);
        QCRIL_LOG_DEBUG ("QtiRadioImpl::queryNrIconTypeResponse serial=%d", serial);
        Return<void> ret = mResponseCbV2_2->onNrIconTypeResponse(serial,
                static_cast<uint32_t>(err_num), convert_five_g_icon_type(iconType));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
        }
    }
    else {
        QCRIL_LOG_ERROR("QtiRadioImpl::queryNrIconTypeResponse respCb NULL");
    }
}

void QtiRadioImpl::on5gStatusChange(five_g_status status) {
    if (mIndicationCbV2_0 != nullptr) {
        QCRIL_LOG_ERROR ("QtiRadioImpl:::on5gStatusChange");
        Return<void> ret = mIndicationCbV2_0->on5gStatusChange(convert_five_g_status(status));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    }
    else {
        QCRIL_LOG_ERROR("QtiRadioImpl:::on5gStatusChange indCb NULL");
    }
}

void QtiRadioImpl::onNrDcParamChange(five_g_endc_dcnr dcParam) {
    if (mIndicationCbV2_0 != nullptr) {
        QCRIL_LOG_ERROR ("QtiRadioImpl::onNrDcParamChange");
        Return<void> ret = mIndicationCbV2_0->onNrDcParamChange(convert_five_g_endc_dcnr(dcParam));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    }
    else {
        QCRIL_LOG_ERROR("QtiRadioImpl::onNrDcParamChange indCb NULL");
    }
}

void QtiRadioImpl::onNrBearerAllocationChange(five_g_bearer_status bearerStatus) {
    if (mIndicationCbV2_1 != nullptr) {
        Return<void> ret = mIndicationCbV2_1->onNrBearerAllocationChange_2_1(
            convert_five_g_bearer_status_2_1(bearerStatus));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    } else if (mIndicationCbV2_0 != nullptr) {
        QCRIL_LOG_ERROR ("QtiRadioImpl::onNrBearerAllocationChange");
        Return<void> ret = mIndicationCbV2_0->onNrBearerAllocationChange(convert_five_g_bearer_status(bearerStatus));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    }
    else {
        QCRIL_LOG_ERROR("QtiRadioImpl::onNrBearerAllocationChange indCb NULL");
    }
}

void QtiRadioImpl::onSignalStrengthChange(five_g_signal_strength signalStrength) {
    if (mIndicationCbV2_0 != nullptr) {
        QCRIL_LOG_ERROR ("QtiRadioImpl::onSignalStrengthChange");
        Return<void> ret = mIndicationCbV2_0->onSignalStrengthChange(convert_five_g_signal_strength(signalStrength));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    }
    else {
        QCRIL_LOG_ERROR("QtiRadioImpl::onSignalStrengthChange indCb NULL");
    }
}

void QtiRadioImpl::onUpperLayerIndInfoChange(five_g_upper_layer_ind_info upli_info) {
    if (mIndicationCbV2_1 != nullptr) {
        QCRIL_LOG_ERROR ("QtiRadioImpl::onUpperLayerIndInfoChange");
        Return<void> ret = mIndicationCbV2_1->onUpperLayerIndInfoChange(
            convert_five_g_upper_layer_ind_info(upli_info));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    }
    else {
        QCRIL_LOG_ERROR("QtiRadioImpl::onUpperLayerIndInfoChange indCb NULL");
    }
}

void QtiRadioImpl::on5gConfigInfoChange(five_g_config_type config) {
    if (mIndicationCbV2_1 != nullptr) {
        QCRIL_LOG_ERROR ("QtiRadioImpl::on5gConfigInfoChange");
        Return<void> ret = mIndicationCbV2_1->on5gConfigInfoChange(
            convert_five_g_config_info(config));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    }
    else {
        QCRIL_LOG_ERROR("QtiRadioImpl::on5gConfigInfoChange indCb NULL");
    }
}

void QtiRadioImpl::onNrIconTypeChange(five_g_icon_type iconType) {
    if (mIndicationCbV2_2 != nullptr) {
        QCRIL_LOG_DEBUG ("QtiRadioImpl::onNrIconTypeChange");
        Return<void> ret = mIndicationCbV2_2->onNrIconTypeChange(
            convert_five_g_icon_type(iconType));
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    }
    else {
        QCRIL_LOG_ERROR("QtiRadioImpl::onNrIconTypeChange indCb NULL");
    }
}


void QtiRadioImpl::getAtrResponse(RIL_Token token, RIL_Errno error, char *buf, int bufLen) {
  qcril_reqlist_free(mInstanceId, token);
  sp<V1_0::IQtiRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    QtiRadioResponseInfo responseInfo;
    hidl_string data;
    QCRIL_LOG_ERROR("QtiRadioImpl::getAtrResponse ");
    int serial = qcril_qmi_qti_free_and_convert_ril_token_to_radio_token(token);
    QCRIL_LOG_ERROR("QtiRadioImpl::getAtrResponse serial=%d", serial);
    responseInfo.serial = serial;
    responseInfo.type = QtiRadioResponseType::SOLICITED;
    responseInfo.error = (QtiRadioError)error;
    if (buf != nullptr) {
      data.setToExternal(buf, bufLen);
    }
    Return<void> ret = respCb->getAtrResponse(responseInfo, data);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  } else {
    QCRIL_LOG_ERROR("QtiRadioImpl::getAtrResponse respCb NULL");
  }
}


void QtiRadioImpl::sendQtiIndication() {}

/*
 * Register qtiRadioService service with Service Manager
 */
void qcril_qmi_qti_radio_service_init(int instanceId) {
  android::status_t ret = android::OK;
  QCRIL_LOG_ERROR("qcril_qmi_qti_radio_service_init");
  qtiRadioService = new QtiRadioImpl((qcril_instance_id_e_type)instanceId);
  std::string serviceName = "slot";
  QCRIL_LOG_INFO("qcril_qmi_qti_radio_service_init adding for slot %d", instanceId);
  ret = qtiRadioService->registerAsService(serviceName + std::to_string(instanceId + 1));
  QCRIL_LOG_INFO("qtiRadioServiceRegisterService instanceId=%d status=%d", instanceId, ret);
}

V2_0::EnableStatus convert_five_g_status(five_g_status status) {
    switch (status) {
        case FIVE_G_STATUS_DISABLED:
            return V2_0::EnableStatus::DISABLED;
        case FIVE_G_STATUS_ENABLED:
            return V2_0::EnableStatus::ENABLED;
        default:
            return V2_0::EnableStatus::INVALID;
    }
}

V2_0::DcParam convert_five_g_endc_dcnr(five_g_endc_dcnr &endc_dcnr_info) {
    V2_0::DcParam dc_param;

    switch (endc_dcnr_info.endc_available) {
        case 0:
            dc_param.endc = V2_0::EndcAvailableType::ENDC_UNAVAILABLE;
            break;
        case 1:
            dc_param.endc = V2_0::EndcAvailableType::ENDC_AVAILABLE;
            break;
        default:
            dc_param.endc = V2_0::EndcAvailableType::INVALID;
            break;
    }

    switch (endc_dcnr_info.restrict_dcnr) {
        case 0:
            dc_param.dcnr = V2_0::RestrictDcnrType::DCNR_UNRESTRICTED;
            break;
        case 1:
            dc_param.dcnr = V2_0::RestrictDcnrType::DCNR_RESTRICTED;
            break;
        default:
            dc_param.dcnr = V2_0::RestrictDcnrType::INVALID;
            break;
    }

    return dc_param;
}

V2_0::SignalStrength convert_five_g_signal_strength(
        five_g_signal_strength &signal_strength) {
    V2_0::SignalStrength ss;
    ss.rsrp = signal_strength.rsrp;
    ss.snr = signal_strength.snr;
    return ss;
}

V2_0::BearerStatus convert_five_g_bearer_status(five_g_bearer_status bearer_status) {
    switch (bearer_status) {
        case FIVE_G_BEARER_STATUS_NOT_ALLOCATED:
            return V2_0::BearerStatus::NOT_ALLOCATED;
        case FIVE_G_BEARER_STATUS_ALLOCATED:
        case FIVE_G_BEARER_STATUS_MMW_ALLOCATED:
            return V2_0::BearerStatus::ALLOCATED;
        default:
            return V2_0::BearerStatus::INVALID;
    }
}

V2_1::BearerStatus convert_five_g_bearer_status_2_1(five_g_bearer_status bearer_status) {
    switch (bearer_status) {
        case FIVE_G_BEARER_STATUS_NOT_ALLOCATED:
            return V2_1::BearerStatus::NOT_ALLOCATED;
        case FIVE_G_BEARER_STATUS_ALLOCATED:
            return V2_1::BearerStatus::ALLOCATED;
        case FIVE_G_BEARER_STATUS_MMW_ALLOCATED:
            return V2_1::BearerStatus::MMW_ALLOCATED;
        default:
            return V2_1::BearerStatus::INVALID;
    }
}

V2_1::PlmnInfoListStatus convert_five_g_plmn_list_info_status(five_g_plmn_info_list_status plmn_list_status) {
    switch (plmn_list_status) {
        case FIVE_G_PLMN_LIST_UNAVAILABLE:
            return V2_1::PlmnInfoListStatus::UNAVAILABLE;
        case FIVE_G_PLMN_LIST_AVAILABLE:
            return V2_1::PlmnInfoListStatus::AVAILABLE;
        default:
            return V2_1::PlmnInfoListStatus::INVALID;
    }
}

V2_1::UpperLayerIndStatus convert_five_g_upper_layer_ind_status(five_g_upper_layer_ind_status upli_status) {
    switch (upli_status) {
        case FIVE_G_UPPER_LAYER_IND_UNAVAILABLE:
            return V2_1::UpperLayerIndStatus::UNAVAILABLE;
        case FIVE_G_UPPER_LAYER_IND_AVAILABLE:
            return V2_1::UpperLayerIndStatus::AVAILABLE;
        default:
            return V2_1::UpperLayerIndStatus::INVALID;
    }
}

V2_1::UpperLayerIndInfo convert_five_g_upper_layer_ind_info(five_g_upper_layer_ind_info &five_g_upli_info) {
    V2_1::UpperLayerIndInfo upli_info;

    upli_info.plmnInfoList   = convert_five_g_plmn_list_info_status(five_g_upli_info.plmn_list_status);
    upli_info.upplerLayerInd = convert_five_g_upper_layer_ind_status(five_g_upli_info.upper_layer_ind_info_status);

    return upli_info;
}

V2_1::ConfigType convert_five_g_config_info(five_g_config_type config) {
    switch (config) {
        case FIVE_G_CONFIG_TYPE_NSA:
            return V2_1::ConfigType::NSA_CONFIGURATION;
        case FIVE_G_CONFIG_TYPE_SA:
            return V2_1::ConfigType::SA_CONFIGURATION;
        default:
            return V2_1::ConfigType::INVALID;
    }
}

V2_2::NrIconType convert_five_g_icon_type(five_g_icon_type iconType) {
    switch (iconType) {
        case FIVE_G_ICON_TYPE_NONE:
            return V2_2::NrIconType::TYPE_NONE;
        case FIVE_G_ICON_TYPE_BASIC:
            return V2_2::NrIconType::TYPE_5G_BASIC;
        case FIVE_G_ICON_TYPE_UWB:
            return V2_2::NrIconType::TYPE_5G_UWB;
        default:
            return V2_2::NrIconType::INVALID;
    }
}

void qtiGetAtrResponse(RIL_Token token, RIL_Errno error, char *buf, int bufLen) {
  if (qtiRadioService != nullptr) {
    qtiRadioService->getAtrResponse(token, error, buf, bufLen);
  }
}

void qtiSendCdmaSmsResponse(RIL_Token t, RIL_Errno error, void *response, size_t responselen){
    if (qtiRadioService != nullptr) {
        qtiRadioService->sendCdmaSmsResponse(t, error, response, responselen);
    }
}
void enable5gResponse(RIL_Token token, RIL_Errno err_num) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->enable5gResponse(token, err_num);
    }
}

void enable5gOnlyResponse(RIL_Token token, RIL_Errno err_num) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->enable5gOnlyResponse(token, err_num);
    }
}

void disable5gResponse(RIL_Token token, RIL_Errno err_num) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->disable5gResponse(token, err_num);
    }
}

void query5gStatusResponse(RIL_Token token, RIL_Errno err_num, five_g_status status) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->query5gStatusResponse(token, err_num, status);
    }
}

void queryNrDcParamResponse(RIL_Token token, RIL_Errno err_num,
        five_g_endc_dcnr &endc_dcnr_info) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->queryNrDcParamResponse(token, err_num, endc_dcnr_info);
    }
}

void queryNrSignalStrengthResponse(RIL_Token token, RIL_Errno err_num,
        five_g_signal_strength &signal_strength) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->queryNrSignalStrengthResponse(token, err_num, signal_strength);
    }
}

void queryNrBearerAllocationResponse(RIL_Token token, RIL_Errno err_num,
        five_g_bearer_status bearer_status) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->queryNrBearerAllocationResponse(token, err_num, bearer_status);
    }
}

void on5gStatusChange(five_g_status status) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->on5gStatusChange(status);
    }
}

void onNrDcParamChange(five_g_endc_dcnr dcParam) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->onNrDcParamChange(dcParam);
    }
}

void onNrBearerAllocationChange(five_g_bearer_status bearerStatus) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->onNrBearerAllocationChange(bearerStatus);
    }
}

void onSignalStrengthChange(five_g_signal_strength signalStrength) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->onSignalStrengthChange(signalStrength);
    }
}

void queryUpperLayerIndInfoResponse(RIL_Token token, RIL_Errno err_num, five_g_upper_layer_ind_info upli_info) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->queryUpperLayerIndInfoResponse(token, err_num, upli_info);
    }
}

void query5gConfigInfoResponse(RIL_Token token, RIL_Errno err_num, five_g_config_type config) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->query5gConfigInfoResponse(token, err_num, config);
    }
}

void queryNrIconTypeResponse(RIL_Token token, RIL_Errno err_num, five_g_icon_type iconType) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->queryNrIconTypeResponse(token, err_num, iconType);
    }
}

void onUpperLayerIndInfoChange(five_g_upper_layer_ind_info upli_info) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->onUpperLayerIndInfoChange(upli_info);
    }
}

void on5gConfigInfoChange(five_g_config_type config) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->on5gConfigInfoChange(config);
    }
}

void onNrIconTypeChange(five_g_icon_type iconType) {
    if (qtiRadioService != nullptr) {
        qtiRadioService->onNrIconTypeChange(iconType);
    }
}

RIL_Token qcril_qmi_qti_convert_radio_token_to_ril_token(uint32_t oem_token) {
  RIL_Token ret = qcril_malloc(sizeof(uint32_t));
  if (nullptr != ret) {
    uint32_t *tmp = (uint32_t *)ret;
    *tmp = oem_token ^ 0xc0000000;
  }
  return ret;
} // qcril_qmi_oem_convert_oem_token_to_ril_token

void populateResponseInfo(QtiRadioResponseInfo& responseInfo, int serial,
        QtiRadioResponseType responseType, RIL_Errno e) {
    responseInfo.serial = serial;
    responseInfo.type = responseType;
    responseInfo.error = (QtiRadioError) e;
}

uint32_t qcril_qmi_qti_free_and_convert_ril_token_to_radio_token(RIL_Token ril_token) {
  uint32_t ret = 0xFFFFFFFF;
  if (ril_token) {
    ret = (*((uint32_t *)ril_token)) ^ 0xc0000000;
    QCRIL_LOG_INFO("oem token: %d", ret);
    qcril_free((void *)ril_token);
  } else {
    QCRIL_LOG_ERROR("ril_token is NULL");
  }

  return ret;
} // qcril_qmi_oem_free_and_convert_ril_token_to_oem_token

} // namespace implementation
} // namespace V1_0
} // namespace qtiradio
} // namespace radio
} // namespace hardware
} // namespace qti
} // namespace vendor
