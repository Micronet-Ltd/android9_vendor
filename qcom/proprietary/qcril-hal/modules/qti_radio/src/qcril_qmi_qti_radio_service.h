/******************************************************************************
  @file    qcril_qmi_qti_radio_service.h
  @brief   qcril qmi - qtiRadioService_service
---------------------------------------------------------------------------

  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
  ******************************************************************************/

#ifndef VENDOR_QTI_HARDWARE_QTIRADIO_V2_2_QTIRADIO_H
#define VENDOR_QTI_HARDWARE_QTIRADIO_V2_2_QTIRADIO_H

#include <vendor/qti/hardware/radio/qtiradio/2.2/IQtiRadio.h>
#include <vendor/qti/hardware/radio/qtiradio/2.2/IQtiRadioResponse.h>
#include <vendor/qti/hardware/radio/qtiradio/2.2/IQtiRadioIndication.h>

#include <android/hardware/radio/1.0/types.h>
#include <framework/legacy.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/RWLock.h>
#include <telephony/ril.h>

using namespace vendor::qti::hardware::radio::qtiradio::V1_0;
using ::android::RWLock;
using ::android::hardware::Return;
using ::android::hardware::hidl_death_recipient;
using ::android::hardware::hidl_string;
using ::android::sp;
using ::android::wp;

extern void onRequest(int request, void *data, size_t datalen, RIL_Token t);
extern void constructCdmaSms(RIL_CDMA_SMS_Message &rcsm, const android::hardware::radio::V1_0::CdmaSmsMessage& sms);
namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V1_0 {
namespace implementation {

RIL_Token qcril_qmi_qti_convert_radio_token_to_ril_token(uint32_t oem_token);
uint32_t qcril_qmi_qti_free_and_convert_ril_token_to_radio_token(RIL_Token ril_token);
void qcril_qmi_qti_radio_service_init(int instanceId);
void qtiGetAtrResponse(RIL_Token token, RIL_Errno error, char *buf, int bufLen);
void qtiSendCdmaSmsResponse(RIL_Token t, RIL_Errno error, void *response, size_t responselen);
V2_0::EnableStatus convert_five_g_status(five_g_status status);
V2_0::DcParam convert_five_g_endc_dcnr(five_g_endc_dcnr &endc_dcnr_info);
V2_0::SignalStrength convert_five_g_signal_strength(five_g_signal_strength &signal_strength);
V2_0::BearerStatus convert_five_g_bearer_status(five_g_bearer_status bearer_status);
V2_1::BearerStatus convert_five_g_bearer_status_2_1(five_g_bearer_status bearer_status);
V2_1::PlmnInfoListStatus convert_five_g_plmn_list_info_status(five_g_plmn_info_list_status plmn_list_status);
V2_1::UpperLayerIndStatus convert_five_g_upper_layer_ind_status(five_g_upper_layer_ind_status upli_status);
V2_1::UpperLayerIndInfo convert_five_g_upper_layer_ind_info(five_g_upper_layer_ind_info &five_g_upli_info);
V2_1::ConfigType convert_five_g_config_info(five_g_config_type config);
V2_2::NrIconType convert_five_g_icon_type(five_g_icon_type config);

void enable5gResponse(RIL_Token token, RIL_Errno err_num);
void enable5gOnlyResponse(RIL_Token token, RIL_Errno err_num);
void disable5gResponse(RIL_Token token, RIL_Errno err_num);
void query5gStatusResponse(RIL_Token token, RIL_Errno err_num, five_g_status status);
void queryNrDcParamResponse(RIL_Token token, RIL_Errno err_num, five_g_endc_dcnr &endc_dcnr_info);
void queryNrSignalStrengthResponse(RIL_Token token, RIL_Errno err_num, five_g_signal_strength &signal_strength);
void queryNrBearerAllocationResponse(RIL_Token token, RIL_Errno err_num, five_g_bearer_status bearer_status);
void on5gStatusChange(five_g_status status);
void onNrDcParamChange(five_g_endc_dcnr dcParam);
void onNrBearerAllocationChange(five_g_bearer_status bearerStatus);
void onSignalStrengthChange(five_g_signal_strength signalStrength);
void queryUpperLayerIndInfoResponse(RIL_Token token, RIL_Errno err_num, five_g_upper_layer_ind_info upli_info);
void query5gConfigInfoResponse(RIL_Token token, RIL_Errno err_num, five_g_config_type config);
void onUpperLayerIndInfoChange(five_g_upper_layer_ind_info upli_info);
void on5gConfigInfoChange(five_g_config_type config);
void queryNrIconTypeResponse(RIL_Token token, RIL_Errno err_num, five_g_icon_type config);
void onNrIconTypeChange(five_g_icon_type config);
void populateResponseInfo(QtiRadioResponseInfo& responseInfo, int serial,
                          QtiRadioResponseType responseType, RIL_Errno e);
class QtiRadioImpl : public V2_2::IQtiRadio, public hidl_death_recipient {
  sp<V1_0::IQtiRadioResponse> mResponseCb;
  sp<V1_0::IQtiRadioIndication> mIndicationCb;

  sp<V2_0::IQtiRadioResponse> mResponseCbV2_0;
  sp<V2_0::IQtiRadioIndication> mIndicationCbV2_0;

  sp<V2_1::IQtiRadioResponse> mResponseCbV2_1;
  sp<V2_1::IQtiRadioIndication> mIndicationCbV2_1;

  sp<V2_2::IQtiRadioResponse> mResponseCbV2_2;
  sp<V2_2::IQtiRadioIndication> mIndicationCbV2_2;

  RWLock mCallbackLock;
  qcril_instance_id_e_type mInstanceId;

  sp<V1_0::IQtiRadioResponse> getResponseCallback();
  sp<V1_0::IQtiRadioIndication> getIndicationCallback();
  sp<V2_0::IQtiRadioResponse> getResponseCallbackV2_0();
  sp<V2_0::IQtiRadioIndication> getIndicationCallbackV2_0();
  sp<V2_1::IQtiRadioResponse> getResponseCallbackV2_1();
  sp<V2_1::IQtiRadioIndication> getIndicationCallbackV2_1();
  sp<V2_2::IQtiRadioResponse> getResponseCallbackV2_2();
  sp<V2_2::IQtiRadioIndication> getIndicationCallbackV2_2();

  // Function from hidl_death_recipient
  void serviceDied(uint64_t, const ::android::wp<::android::hidl::base::V1_0::IBase> &);


  // Functions from IQtiRadio
  Return<void> getAtr(int32_t serial);
  Return<void> enable5g(int32_t serial);
  Return<void> disable5g(int32_t serial);
  Return<void> enable5gOnly(int32_t serial);
  Return<void> query5gStatus(int32_t serial);
  Return<void> queryNrDcParam(int32_t serial);
  Return<void> queryNrBearerAllocation(int32_t serial);
  Return<void> queryNrSignalStrength(int32_t serial);
  Return<void> queryUpperLayerIndInfo(int32_t serial);
  Return<void> query5gConfigInfo(int32_t serial);
  Return<void> queryNrIconType(int32_t serial);
  Return<void> sendCdmaSms(int32_t serial,
                           const android::hardware::radio::V1_0::CdmaSmsMessage& sms,
                           bool expectMore);
#ifdef QMI_RIL_UTF
public:
    // make setCallback and clearCallbacks as public for UTF
#endif

  // Functions from IQtiRadio
  Return<void> setCallback(const sp<IQtiRadioResponse> &responseCallback,
                           const sp<IQtiRadioIndication> &indicationCallback);
  void clearCallbacks();
public:
  QtiRadioImpl(qcril_instance_id_e_type instaceId);

  void getAtrResponse(RIL_Token token, RIL_Errno error, char *buf, int bufLen);
  void sendCdmaSmsResponse(RIL_Token t, RIL_Errno error, void *response, size_t responselen);
  void sendQtiIndication();
  void enable5gResponse(RIL_Token token, RIL_Errno err_num);
  void enable5gOnlyResponse(RIL_Token token, RIL_Errno err_num);
  void disable5gResponse(RIL_Token token, RIL_Errno err_num);
  void query5gStatusResponse(RIL_Token token, RIL_Errno err_num, five_g_status status);
  void queryNrDcParamResponse(RIL_Token token, RIL_Errno err_num, five_g_endc_dcnr endc_available_restrict_dcnr);
  void queryNrSignalStrengthResponse(RIL_Token token, RIL_Errno err_num, five_g_signal_strength signalStrength);
  void queryNrBearerAllocationResponse(RIL_Token token, RIL_Errno err_num, five_g_bearer_status bearerStatus);
  void on5gStatusChange(five_g_status status);
  void onNrDcParamChange(five_g_endc_dcnr dcParam);
  void onNrBearerAllocationChange(five_g_bearer_status bearerStatus);
  void onSignalStrengthChange(five_g_signal_strength signalStrength);
  void queryUpperLayerIndInfoResponse(RIL_Token token, RIL_Errno err_num, five_g_upper_layer_ind_info upli_info);
  void query5gConfigInfoResponse(RIL_Token token, RIL_Errno err_num, five_g_config_type config);
  void onUpperLayerIndInfoChange(five_g_upper_layer_ind_info upli_info);
  void on5gConfigInfoChange(five_g_config_type config);
  void queryNrIconTypeResponse(RIL_Token token, RIL_Errno err_num, five_g_icon_type config);
  void onNrIconTypeChange(five_g_icon_type config);
};

} // namespace implementation
} // namespace V2_0
} // namespace qtiradio
} // namespace radio
} // namespace hardware
} // namespace qti
} // namespace vendor
#endif // VENDOR_QTI_HARDWARE_QTIRADIO_V2_2_QTIRADIO_H
