/*!
  @file
  qcril_qmi_ims_radio_utils.h

  @brief
  Utility file to convert from IImsRadio data types to proto types

*/

/*===========================================================================

   Copyright (c) 2017 Qualcomm Technologies, Inc.
   All Rights Reserved.
   Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#ifndef IMS_RADIO_UTILS_H
#define IMS_RADIO_UTILS_H

#include <hidl/HidlSupport.h>
#include "qcril_qmi_ims_misc.h"
#include <vendor/qti/hardware/radio/ims/1.3/types.h>
#include <vendor/qti/hardware/radio/ims/1.4/types.h>

using namespace vendor::qti::hardware::radio::ims::V1_0;

using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_0 {
namespace utils {

    //Functions to convert hidl to proto.
    //Struct conversions.
    ims_Error convertHidlToProtoDial(const V1_4::DialRequest& inDial, ims_Dial& outDial);
    ims_Error convertHidlToProtoCallDetails(const CallDetails& inCallDetails,
        ims_CallDetails& outCallDetails);
    ims_Error convertHidlToProtoCallDetailsExtras(
        const android::hardware::hidl_vec<hidl_string>& inCallDetailsExtras,
        char** outCallDetailsExtras);
    ims_Error convertHidlToProtoServiceStatusInfoList(const hidl_vec<ServiceStatusInfo>& inList,
        ims_Info** outList);
    ims_Error convertHidlToProtoHangup(const V1_3::HangupRequestInfo& inHangup,
            ims_Hangup& outHangup);
    ims_Error convertHidlToProtoCallFailCauseResponse(
            const V1_3::CallFailCauseResponse& inFailCause,ims_CallFailCauseResponse& outFailCause);
    ims_Error convertHidlToProtoSipErrorInfo(const SipErrorInfo& inErrorInfo,
            ims_SipErrorInfo& outErrorInfo);
    ims_Error convertHidlToProtoServiceStatusInfo(const ServiceStatusInfo& inServiceInfo,
            ims_Info& outServiceInfo);
    ims_Error convertHidlToProtoStatusForAccessTech(const StatusForAccessTech& inAccTechStatus,
            ims_StatusForAccessTech& outAccTechStatus);
    ims_Error convertHidlToProtoRegistrationInfo(const RegistrationInfo& inRegistration,
            ims_Registration& outRegistration);
    ims_Error convertHidlToProtoDtmf(const DtmfInfo& inDtmf, ims_Dtmf& outDtmf);
    ims_Error convertHidlToProtoBinaryData(const hidl_vec<uint8_t>& inBinaryData,
            qcril_binary_data_type& outBinaryData);
    ims_Error convertHidlToProtoCallModifyInfo(const CallModifyInfo &in, ims_CallModify &out);
    ims_Error convertHidlToProtoCallForwardInfo(const CallForwardInfo& inCallForwardInfo,
        ims_CallForwardInfoList_CallForwardInfo& outCallForwardInfo);
    void convertHidlToProtoCallFwdTimerInfo(const CallFwdTimerInfo& inCallFwdTimerInfo,
         ims_CallFwdTimerInfo& outCallFwdTimerInfo);
    ims_Error convertHidlToProtoString(const hidl_string& inStr, char*& outStr);
    ims_Error convertHidlToProtoConfig(const ConfigInfo& inConfig, ims_ConfigMsg& outConfig);
    ims_Error convertHidlToProtoNumListInfo(const CbNumListInfo& inCbNumListInfo,
        ims_CbNumListType& outCbNumListInfo);
    ims_Error convertHidlToProtoEctInfo(const ExplicitCallTransferInfo &inEctRequest,
            ims_ExplicitCallTransfer &outEctRequest);
    ims_Error convertHidlToProtoNumInfo(const CbNumInfo& inCbNumInfo, ims_CbNumList& outCbNumInfo);
    ims_Error convertHidlToProtoDeflectRequestInfo(const DeflectRequestInfo &inDeflectRequest,
            ims_DeflectCall &outDeflectRequest);
    //enum conversions.
    ims_CallDomain convertHidlToProtoCallDomain(const CallDomain& inCallDomain);
    ims_CallType convertHidlToProtoCallType(const CallType& callType);
    ims_CallFailCause convertHidlToProtoCallFailCause(const V1_3::CallFailCause& failCause);
    ims_CallFailCause convertHidlToProtoCallFailCauseV1_2(const V1_2::CallFailCause& failCause);
    ims_CallFailCause convertHidlToProtoCallFailCauseV1_1(const V1_1::CallFailCause& failCause);
    ims_CallFailCause convertHidlToProtoCallFailCauseV1_0(const CallFailCause& failCause);
    ims_Registration_RegState convertHidlToProtoRegState(const RegState& state);
    ims_SrvType convertHidlToProtoServiceType(const ServiceType& type);
    ims_StatusType convertHidlToProtoStatusType(const StatusType& type);
    ims_IpPresentation convertHidlToProtoPresentation(const IpPresentation& inPresentation);
    ims_RadioTechType convertHidlToProtoRadioTech(const RadioTechType& radioTechType);
    ims_Tty_Mode_Type convertHidlToProtoTtyMode(TtyMode ttyMode);
    bool convertHidlToProtoCallModifyFailCause(const CallModifyFailCause &in, ims_Error &out);
    ims_ServiceClassStatus convertHidlToProtoServiceClassStatus(ServiceClassStatus status);
    ims_ServiceClassProvisionStatus convertHidlToProtoServiceClassProvisionStatus(
            V1_3::ServiceClassProvisionStatus status);
    ims_Error convertHidlToProtoConfigItem(const ConfigItem& inConfigItem,
            ims_ConfigItem& outConfigItem);
    ims_Error convertHidlToProtoErrorCause(const ConfigFailureCause& inErrorCause,
            ims_ConfigFailureCause& outErrorCause);
    int convertHidlToProtoFacilityType(FacilityType facilityType);
    bool convertHidlToProtoEctType(const EctType &in, ims_EctType &out);
    ims_Error convertHidlToProtoGeolocation(double lat, double lon,
            const V1_2::AddressInfo& inAddress, ims_GeoLocationInfo& outAddress);
    bool copy(const hidl_string& in, char*& out, const char* log_tag);
    void freeGeolocationInfo(ims_GeoLocationInfo& geoInfo);
    void freeAddressInfo(ims_AddressInfo& addressInfo);
    void migrateAddressInfo(const V1_0::AddressInfo& from,
            V1_2::AddressInfo& to);
    ims_RttMode convertHidlToProtoRttMode(const RttMode& inRttMode);
    RttMode convertProtoToHidlRttMode(const ims_RttMode& inRttMode);

    //Functions to convert proto to hidl
    //Struct conversions.
    void convertProtoToHidlRegistration(const ims_Registration& inReg, RegistrationInfo& outReg);
    ims_Error convertProtoToHidlSipErrorInfo(const ims_SipErrorInfo& inErrorInfo,
            SipErrorInfo& outErrorInfo);
    void convertProtoToHidlCallList(const ims_CallList& inCallList,
            hidl_vec<V1_4::CallInfo>& outCallList);
    void convertProtoToHidlCall(const ims_CallList_Call& inCall, V1_4::CallInfo& outCall);
    void convertProtoToHidlCallDetails(const ims_CallDetails& inCallDetails,
            CallDetails& outCallDetails);
    void convertProtoToHidlServiceInfoList(const ims_Info** const inServiceInfoList,
            hidl_vec<ServiceStatusInfo>& outServiceInfoList);
    void convertProtoToHidlServiceStatusInfo(const ims_Info& inServiceStatusInfo,
            ServiceStatusInfo& outServiceStatusInfo);
    void convertProtoToHidlStatusForAccessTech(const ims_StatusForAccessTech& inAccTechStatus,
            StatusForAccessTech& outAccTechStatus);
    void convertProtoToHidlCallFailCauseResponse(const ims_CallFailCauseResponse& inFailCause,
            V1_3::CallFailCauseResponse& outFailCause);
    void convertProtoToHidlHandoverInfo(const ims_Handover& inHandover, HandoverInfo& outHandover);
    void convertProtoToHidlHandoverExtra(const ims_Extra& inExtra, Extra& outExtra);
    void convertProtoToHidlSuppServiceStatus(const ims_SuppSvcResponse& inSuppSvcResponse,
        V1_3::SuppServiceStatus& outSuppServiceStatus);
    void convertProtoToHidlSuppServiceStatusV1_0(const ims_SuppSvcResponse& inSuppSvcResponse,
        SuppServiceStatus& outSuppServiceStatus);
    void convertProtoToHidlNumListInfo(const ims_CbNumListType& incbNumListType,
        CbNumListInfo& outCbNumListInfo);
    void convertProtoToHidlNumInfo(const ims_CbNumList& inCbNumList, CbNumInfo& outCbNumInfo);
    void convertProtoToHidlClipProvisionStatus(
        const ims_ClipProvisionStatus& inClipProvisionStatus,
        ClipProvisionStatus& outClipProvisionStatus);
    bool convertProtoToHidlClirInfo(const ims_Clir& inClir, ClirInfo& outClir);
    void convertProtoToHidlColrInfo(const ims_Colr& inColr, V1_3::ColrInfo& outColr);
    void convertProtoToHidlColrInfoV1_0(const ims_Colr& inColr, ColrInfo& outColr);
    void convertProtoToHidlBinaryData(const qcril_binary_data_type* inBinaryData,
        hidl_vec<uint8_t>& outBinaryData);
    bool convertProtoToHidlCallModifyInfo(const ims_CallModify &in, CallModifyInfo &out);
    void convertProtoToHidlCallForwardInfoList(
            const ims_CallForwardInfoList_CallForwardInfo** const inCfInfoList,
            hidl_vec<CallForwardInfo>& outCfInfoList);
    void convertProtoToHidlCallForwardInfo(
            const ims_CallForwardInfoList_CallForwardInfo& inCallForwardInfo,
            CallForwardInfo& outCallForwardInfo);
    void convertProtoToHidlCallForwardTimerInfo(const ims_CallFwdTimerInfo& inCallFwdTimerInfo,
            CallFwdTimerInfo& outCallFwdTimerInfo);
    void convertProtoToHidlString(const char * inStr, hidl_string& outStr);
    void convertProtoToHidlSuppServiceNotification(
            const ims_SuppSvcNotification& inSuppServiceNotification,
            SuppServiceNotification& outSuppServiceNotification);
    NotificationType convertProtoToHidlNotificationType(ims_NotificationType inType) ;
    void convertProtoToHidlConfig(const ims_ConfigMsg& inConfig, ConfigInfo& outConfig);
    void setDefaultVerstatInfo(V1_3::VerstatInfo& verstatInfo);
    void convertToHidlVerstatInfo(
            const ims_VerstatInfo& inVerstatInfo,
            V1_3::VerstatInfo& outVerstatInfo);
    //enum conversions.
    RegState convertProtoToHidlRegState(const ims_Registration_RegState& state);
    RadioTechType convertProtoToHidlRadioTech(const ims_RadioTechType& radioTechType);
    CallState convertProtoToHidlCallState(const ims_CallState& inState);
    CallType convertProtoToHidlCallType(const ims_CallType& callType);
    CallDomain convertProtoToHidlCallDomain(const ims_CallDomain& callDomain);
    CallDomain convertProtoToHidlCallDomain(const ims_CallDomain& inCallDomain);
    ServiceType convertProtoToHidlServiceType(const ims_SrvType& type);
    StatusType convertProtoToHidlStatusType(const ims_StatusType& status);
    V1_3::CallFailCause convertProtoToHidlCallFailCause(const ims_CallFailCause& failCause);
    CallFailCause convertProtoToHidlCallFailCauseV1_0(const ims_CallFailCause& failCause);
    ToneOperation convertProtoToHidlTone(const ims_RingBackTone& inTone);
    ToneOperation convertProtoToHidlToneOperation(const ims_RingBackTone_ToneFlag& flag);
    HandoverType convertProtoToHidlHandoverType(const ims_Handover_Msg_Type& inType);
    RadioState convertProtoToHidlRadioState(const ims_RadioStateChanged& inState);
    ServiceClassStatus convertProtoToHidlServiceClassStatus(
            ims_ServiceClassStatus serviceClassStatus);
    V1_3::ServiceClassProvisionStatus convertProtoToHidlServiceClassProvisionStatus(
        ims_ServiceClassProvisionStatus provisionStatus);
    FacilityType convertProtoToHidlFacilityType(int facilityType);
    TtyMode convertProtoToHidlTtyMode(ims_Tty_Mode_Type ttyMode);
    ClipStatus convertProtoToHidlClipStatus(ims_ClipStatus clipStatus);
    IpPresentation convertProtoToHidlPresentation(ims_IpPresentation presentation);
    ConferenceCallState convertProtoToHidlConferenceCallState(
        ims_ConfCallState conferenceCallState);
    MessageType convertProtoToHidlMwiMessageType(ims_MwiMessageType mwiMessageType);
    MessagePriority convertProtoToHidlMwiMessagePriority(ims_MwiPriority mwiPriority);

    bool convertProtoToHidlCallModifyFailCause(const ims_Error &in, CallModifyFailCause &out);
    ConfigFailureCause convertProtoToHidlErrorCause(const ims_ConfigFailureCause& inErrorCause);
    ConfigItem convertProtoToHidlConfigItem(const ims_ConfigItem& outConfigItem);
    void convertProtoToHidlMessageWaitingIndication(const ims_Mwi& mwi,
            MessageWaitingIndication& messageWaitingIndication);
    void convertProtoToHidlMessageSummary(const ims_MwiMessageSummary& mwiMessageSummary,
            MessageSummary& messageSummary);
    void convertProtoToHidlMessageDetails(const ims_MwiMessageDetails& mwiMessageDetails,
            MessageDetails& messageDetails);
    void convertProtoToHidlImsSubConfig(const ims_ImsSubConfig& subConfig,
            ImsSubConfigInfo& subConfigInfo);
    void convertProtoToHidlParticipantStatusInfo(const ims_ParticipantStatusInfo& in,
            ParticipantStatusInfo& out);
    void convertProtoToHidlConfParticipantOperation(const ims_ConfParticipantOperation& in,
            ConfParticipantOperation& out);
    void convertProtoToHidlBlockStatus(const ims_BlockStatus &inStatus, BlockStatus &outStatus);
    void convertProtoToHidlBlockReasonDetails(const ims_BlockReasonDetails &inDetails,
            BlockReasonDetails &outDetails);
    BlockReasonType convertProtoToHidlBlockReasonType(ims_BlockReasonType reason);
    RegFailureReasonType convertProtoToHidlRegFailureReasonType(ims_RegFailureReasonType reason);
    VoWiFiCallQualityInfo convertProtoToHidlVoWiFiCallQuality(const ims_VoWiFiCallQuality&
        voWiFiCallQuality);
    void convertToHidlStkCcUnsolSsResult(ims_ril_ss_ind_type &in, StkCcUnsolSsResult &out);
    void convertToHidlAutoCallRejectionInfo(const ims_AutoCallRejectionInfo& in,
        V1_3::AutoCallRejectionInfo& out);
    V1_3::VoiceInfo convertToHidlVoiceInfo(ims_VoiceInfo voiceInfo);

    V1_2::VerificationStatus convertToHidlVerstatVerificationStatus(
            ims_VerstatVerificationStatus status);
    //Utility functions
    void convertCallListV14ToV13(hidl_vec<V1_4::CallInfo>& callListV14,
                    hidl_vec<V1_3::CallInfo>& callListV13);
    void convertCallV14ToV13(V1_4::CallInfo& callInfoV14,
                    V1_3::CallInfo& callInfoV13);
    void convertCallListV13ToV12(hidl_vec<V1_3::CallInfo>& callListV13,
                    hidl_vec<V1_2::CallInfo>& callListV12);
    void convertCallV13ToV12(V1_3::CallInfo& callInfoV13,
                    V1_2::CallInfo& callInfoV12);
    V1_2::CallFailCause convertCallFailCauseV13ToV12(const V1_3::CallFailCause& failcause);
    void convertCallListV12ToV11(hidl_vec<V1_2::CallInfo>& callListV12,
                    hidl_vec<V1_1::CallInfo>& callListV11);
    void convertCallV12ToV11(V1_2::CallInfo& callInfoV12,
                    V1_1::CallInfo& callInfoV11);
    V1_1::CallFailCause convertCallFailCauseV12ToV11(const V1_2::CallFailCause& failcause);
    void convertCallListV11ToV10(hidl_vec<V1_1::CallInfo>& callListV11,
                    hidl_vec<V1_0::CallInfo>& callListV10);
    void convertCallV11ToV10(V1_1::CallInfo& callInfoV11,
                    V1_0::CallInfo& callInfoV10);
    V1_0::CallFailCause convertCallFailCauseToV10(const V1_1::CallFailCause& failcause);
    void convertSuppServiceStatusV13ToV10(const V1_3::SuppServiceStatus& suppServiceStatusV13,
                    V1_0::SuppServiceStatus& suppServiceStatusV10);
    void convertColrInfoV13ToV10(const V1_3::ColrInfo& colrInfoV13, V1_0::ColrInfo& colrInfoV10);
    void imsRadioGetTag(int32_t token, ims_MsgType type, ims_MsgId msgid,
            ims_Error error, ims_MsgTag& msgTag);
    void imsRadioGetTag(int32_t token, ims_MsgId id, ims_MsgTag& msgTag);
    template <typename T> uint32_t getCount(T** ptr);
    bool isError(ims_Error ret);
    void migrateHangup(const V1_0::HangupRequestInfo& from,
            V1_1::HangupRequestInfo& to);
    void migrateHangup(const V1_1::HangupRequestInfo& from,
            V1_2::HangupRequestInfo& to);
    void migrateHangup(const V1_2::HangupRequestInfo& from,
            V1_3::HangupRequestInfo& to);
    void migrateColrInfo(const V1_0::ColrInfo& from,
            V1_3::ColrInfo& to);
    void migrateDialRequest(const V1_0::DialRequest& from,
            V1_4::DialRequest& to);

    void convertToHidlMultiIdentityRegistrationStatus(
            const std::vector<QcrilImsMultiIdentityInfo>& in_lines,
            hidl_vec<V1_4::MultiIdentityLineInfoHal>& out_lines);
    void convertToRilMultiIdentityInfo(const V1_4::MultiIdentityLineInfoHal& in_line,
            QcrilImsMultiIdentityInfo& out_line);
    int convertToRilMultiIdentityRegistrationStatus(
            V1_4::MultiIdentityRegistrationStatus status);
    int convertToRilMultiIdentityLineType(
            V1_4::MultiIdentityLineType type);
    V1_4::MultiIdentityRegistrationStatus convertToHIdlMultiIdentityRegistrationStatus(
        int status);
    void convertToHidlVirtualLines(hidl_vec<hidl_string>& out,
            const std::vector<std::string>& in);
} // namespace utils
} // namespace V1_0
} // namespace ims
} // namespace radio
} // namespace hardware
} // namespace qti
} // namespace vendor
#endif  // IMS_RADIO_UTILS_H
