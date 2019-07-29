/*!
  @file
  qcril_qmi_ims_radio_service.h

  @brief

*/

/*===========================================================================

   Copyright (c) 2017 Qualcomm Technologies, Inc.
   All Rights Reserved.
   Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#ifndef IMS_RADIO_SERVICE_H
#define IMS_RADIO_SERVICE_H

#include "comdef.h"
#include "imsIF.pb.h"
#include "telephony/ril.h"
#include <utils/RWLock.h>

#include <vendor/qti/hardware/radio/ims/1.4/IImsRadio.h>
#include <vendor/qti/hardware/radio/ims/1.4/IImsRadioResponse.h>
#include <vendor/qti/hardware/radio/ims/1.4/IImsRadioIndication.h>
#include <vendor/qti/hardware/radio/ims/1.3/types.h>
#include <vendor/qti/hardware/radio/ims/1.4/types.h>
#include <hidl/HidlSupport.h>

using namespace vendor::qti::hardware::radio::ims::V1_0;
using ::android::hardware::hidl_death_recipient;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::sp;
using ::android::RWLock;

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_0 {
namespace implementation {

void imsRadioProcessResponse(ims_MsgTag tag, void* data);
void imsRadioRegisterService();

/** Class implementing the IImsRadio server implementation for all requests coming in
 * from Ims to RIL. Also registers the callback of type IImsRadioResponse to send the
 * solicited response back and IImsRadioIndication to send unsolicited responses.
 */
class ImsRadioImpl : public V1_4::IImsRadio, public hidl_death_recipient {
    sp<IImsRadioResponse> mImsRadioResponseCb;
    sp<IImsRadioIndication> mImsRadioIndicationCb;
    sp<V1_1::IImsRadioIndication> mImsRadioIndicationCbV1_1;
    sp<V1_2::IImsRadioResponse> mImsRadioResponseCbV1_2;
    sp<V1_2::IImsRadioIndication> mImsRadioIndicationCbV1_2;
    sp<V1_3::IImsRadioResponse> mImsRadioResponseCbV1_3;
    sp<V1_3::IImsRadioIndication> mImsRadioIndicationCbV1_3;
    sp<V1_4::IImsRadioResponse> mImsRadioResponseCbV1_4;
    sp<V1_4::IImsRadioIndication> mImsRadioIndicationCbV1_4;
    RWLock mCallbackLock;
    sp<IImsRadioIndication> getIndicationCallback();
    sp<IImsRadioResponse> getResponseCallback();
    sp<V1_1::IImsRadioIndication> getIndicationCallbackV1_1();
    sp<V1_2::IImsRadioResponse> getResponseCallbackV1_2();
    sp<V1_2::IImsRadioIndication> getIndicationCallbackV1_2();
    sp<V1_3::IImsRadioResponse> getResponseCallbackV1_3();
    sp<V1_3::IImsRadioIndication> getIndicationCallbackV1_3();
    sp<V1_4::IImsRadioResponse> getResponseCallbackV1_4();
    sp<V1_4::IImsRadioIndication> getIndicationCallbackV1_4();

    void serviceDied(uint64_t, const ::android::wp<::android::hidl::base::V1_0::IBase>&);
    void sendEmptyErrorResponse(int32_t token, ims_MsgId id, ims_Error error);
    void callForwardStatusInternal(int32_t token, const CallForwardInfo& callForwardInfo,
            ims_MsgId id);
#ifdef QMI_RIL_UTF
public:
    // make setCallback and clearCallbacks as public for UTF
#endif
    Return<void> setCallback(
      const sp<IImsRadioResponse>& mImsRadioResponseCbParam,
      const sp<IImsRadioIndication>& mImsRadioIndicationCbParam);
    void clearCallbacks();

public:
    void processResponse(ims_MsgTag tag, void* data);
    ImsRadioImpl();
    ~ImsRadioImpl();
    ims_Error processRequest(int32_t token, ims_MsgId id, void* data);
    Return<void> dial(int32_t token, const DialRequest& dialRequest);
    Return<void> dial_1_4(int32_t token, const V1_4::DialRequest& dialRequest);
    Return<void> getImsRegistrationState(int32_t token);
    Return<void> answer(int32_t token, CallType callType, IpPresentation presentation,
                         RttMode rttMode);
    Return<void> hangup_1_3(int32_t token, const V1_3::HangupRequestInfo& hangup);
    Return<void> hangup_1_2(int32_t token, const V1_2::HangupRequestInfo& hangup);
    Return<void> hangup_1_1(int32_t token, const V1_1::HangupRequestInfo& hangup);
    Return<void> hangup(int32_t token, const HangupRequestInfo& hangup);
    Return<void> requestRegistrationChange(int32_t token, RegState state);
    Return<void> queryServiceStatus(int32_t token);
    Return<void> setServiceStatus(int32_t token, const ServiceStatusInfo& srvStatusInfo);
    Return<void> hold(int32_t token, uint32_t callId);
    Return<void> resume(int32_t token, uint32_t callId);
    Return<void> setConfig(int32_t token, const ConfigInfo& config);
    Return<void> getConfig(int32_t token, const ConfigInfo& config);
    Return<void> conference(int32_t token);
    Return<void> getClip(int32_t token);
    Return<void> getClir(int32_t token);
    Return<void> setClir(int32_t token, const ClirInfo& clirInfo);
    Return<void> getColr(int32_t token);
    Return<void> setColr_1_3(int32_t token, const V1_3::ColrInfo& colrInfo);
    Return<void> setColr(int32_t token, const ColrInfo& colrInfo);
    Return<void> exitEmergencyCallbackMode(int32_t token);
    Return<void> sendDtmf(int32_t token, const DtmfInfo& dtmfInfo);
    Return<void> startDtmf(int32_t token, const DtmfInfo& dtmfInfo);
    Return<void> stopDtmf(int32_t token);
    Return<void> setUiTtyMode(int32_t token, const TtyInfo& ttyInfo);
    Return<void> modifyCallInitiate(int32_t token, const CallModifyInfo& callModifyInfo);
    Return<void> cancelModifyCall(int32_t token, uint32_t callId);
    Return<void> modifyCallConfirm(int32_t token, const CallModifyInfo& callModifyInfo);
    Return<void> getCallWaiting(int32_t token, uint32_t serviceClass);
    Return<void> setSuppServiceNotification(int32_t token, ServiceClassStatus status);
    Return<void> queryCallForwardStatus(int32_t token, const CallForwardInfo& callForwardInfo);
    Return<void> setCallForwardStatus(int32_t token, const CallForwardInfo& callForwardInfo);
    Return<void> setCallWaiting(int32_t token, ServiceClassStatus inStatus,
            uint32_t inServiceClass);
    Return<void> explicitCallTransfer(int32_t token, const ExplicitCallTransferInfo& ectInfo);
    Return<void> suppServiceStatus(int32_t token, int32_t operationType, FacilityType facilityType,
            const CbNumListInfo& cbNumListInfo);
    Return<void> deflectCall(int32_t token, const DeflectRequestInfo& deflectRequestInfo);
    template<typename T> void checkReturnStatus(const Return<T>& ret);
    Return<void> getRtpStatistics(int32_t token);
    Return<void> getRtpErrorStatistics(int32_t token);
    Return<void> addParticipant(int32_t token, const DialRequest& dialRequest);
    Return<void> addParticipant_1_4(int32_t token, const V1_4::DialRequest& dialRequest);
    Return<void> sendGeolocationInfo_1_2(int32_t token, double lat, double lon,
            const V1_2::AddressInfo& addressInfo);
    Return<void> sendGeolocationInfo(int32_t token, double lat, double lon,
            const AddressInfo& addressInfo);
    Return<void> getImsSubConfig(int32_t token);
    Return<void> sendImsSms(int32_t token, const V1_2::ImsSmsMessage& imsSms);
    Return<void> acknowledgeSms(int32_t token, uint32_t messageRef, V1_2::ImsSmsDeliverStatusResult smsdeliverResult);
    Return<void> acknowledgeSmsReport(int32_t token, uint32_t messageRef, V1_2::ImsSmsStatusReportResult smsstatusreport);
    Return<void> getSmsFormat(getSmsFormat_cb _hidl_cb);

    boolean hasValidCallbacks();
    boolean sendMessage(RIL_Token token, ims_MsgType type, ims_MsgId message_id,
            ims_Error error, const void* msg, int msg_len);
    Return<void> sendRttMessage(int32_t token, const hidl_string& rttMessage);
    Return<void> registerMultiIdentityLines(int32_t token,
            const hidl_vec<V1_4::MultiIdentityLineInfoHal>& info);
    Return<void> queryVirtualLineInfo(int32_t token, const hidl_string& msisdn);
};
} // namespace implementation
} // namespace V1_0
} // namespace ims
} // namespace radio
} // namespace hardware
} // namespace qti
} // namespace vendor
#endif  // IMS_RADIO_SERVICE_H
