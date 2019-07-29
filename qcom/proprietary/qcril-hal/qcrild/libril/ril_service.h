/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*
* Not a Contribution.
* Apache license notifications and license are retained
* for attribution purposes only.
*/
/*
 * Copyright (c) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef QMI_RIL_UTF
#ifndef RIL_SERVICE_H
#define RIL_SERVICE_H

#include <QcrildServiceFactory.h>
#include <android/hardware/radio/1.1/IRadio.h>
#include <android/hardware/radio/1.1/IRadioResponse.h>
#include <android/hardware/radio/1.1/IRadioIndication.h>
#include <android/hardware/radio/1.1/types.h>

#include <android/hardware/radio/deprecated/1.0/IOemHook.h>

#include <hwbinder/IPCThreadState.h>
#include <hwbinder/ProcessState.h>

#include <telephony/ril.h>
#include <ril_internal.h>
#include <framework/GenericCallback.h>
#include <modules/android/RilRequestMessage.h>
#include "SetLinkCapRptCriteriaMessage.h"

#include <hidl/HidlTransportSupport.h>
#include <utils/SystemClock.h>
#include <inttypes.h>

using namespace android::hardware::radio;
using namespace android::hardware::radio::V1_0;
using namespace android::hardware::radio::deprecated::V1_0;
using ::android::hardware::configureRpcThreadpool;
using ::android::hardware::joinRpcThreadpool;
using ::android::hardware::Return;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_array;
using ::android::hardware::Void;
using android::CommandInfo;
using android::RequestInfo;
using android::requestToString;
using android::sp;


#define BOOL_TO_INT(x) (x ? 1 : 0)
#define ATOI_NULL_HANDLED(x) (x ? atoi(x) : -1)
#define ATOI_NULL_HANDLED_DEF(x, defaultVal) (x ? atoi(x) : defaultVal)

#if defined(ANDROID_MULTI_SIM)
#define CALL_ONREQUEST(a, b, c, d, e) \
        s_vendorFunctions->onRequest((a), (b), (c), (d), ((RIL_SOCKET_ID)(e)))
#define CALL_ONSTATEREQUEST(a) s_vendorFunctions->onStateRequest((RIL_SOCKET_ID)(a))
#else
#define CALL_ONREQUEST(a, b, c, d, e) s_vendorFunctions->onRequest((a), (b), (c), (d))
#define CALL_ONSTATEREQUEST(a) s_vendorFunctions->onStateRequest()
#endif
extern RIL_RadioFunctions *s_vendorFunctions;

namespace radio {
void registerService(RIL_RadioFunctions *callbacks, android::CommandInfo *commands);

int getIccCardStatusResponse(int slotId, int responseType,
                            int token, RIL_Errno e, void *response, size_t responselen);

int supplyIccPinForAppResponse(int slotId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responselen);

int supplyIccPukForAppResponse(int slotId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responselen);

int supplyIccPin2ForAppResponse(int slotId,
                               int responseType, int serial, RIL_Errno e, void *response,
                               size_t responselen);

int supplyIccPuk2ForAppResponse(int slotId,
                               int responseType, int serial, RIL_Errno e, void *response,
                               size_t responselen);

int changeIccPinForAppResponse(int slotId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responselen);

int changeIccPin2ForAppResponse(int slotId,
                               int responseType, int serial, RIL_Errno e, void *response,
                               size_t responselen);

int supplyNetworkDepersonalizationResponse(int slotId,
                                          int responseType, int serial, RIL_Errno e,
                                          void *response, size_t responselen);

int getCurrentCallsResponse(int slotId,
                           int responseType, int serial, RIL_Errno e, void *response,
                           size_t responselen);

int dialResponse(int slotId,
                int responseType, int serial, RIL_Errno e, void *response, size_t responselen);

int getIMSIForAppResponse(int slotId, int responseType,
                         int serial, RIL_Errno e, void *response, size_t responselen);

int hangupConnectionResponse(int slotId, int responseType,
                            int serial, RIL_Errno e, void *response, size_t responselen);

int hangupWaitingOrBackgroundResponse(int slotId,
                                     int responseType, int serial, RIL_Errno e, void *response,
                                     size_t responselen);

int hangupForegroundResumeBackgroundResponse(int slotId,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responselen);

int switchWaitingOrHoldingAndActiveResponse(int slotId,
                                           int responseType, int serial, RIL_Errno e,
                                           void *response, size_t responselen);

int conferenceResponse(int slotId, int responseType,
                      int serial, RIL_Errno e, void *response, size_t responselen);

int rejectCallResponse(int slotId, int responseType,
                      int serial, RIL_Errno e, void *response, size_t responselen);

int getLastCallFailCauseResponse(int slotId,
                                int responseType, int serial, RIL_Errno e, void *response,
                                size_t responselen);

int getSignalStrengthResponse(int slotId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responseLen);

int getVoiceRegistrationStateResponse(int slotId,
                                     int responseType, int serial, RIL_Errno e, void *response,
                                     size_t responselen);

int getDataRegistrationStateResponse(int slotId,
                                    int responseType, int serial, RIL_Errno e, void *response,
                                    size_t responselen);

int getOperatorResponse(int slotId,
                       int responseType, int serial, RIL_Errno e, void *response,
                       size_t responselen);

int setRadioPowerResponse(int slotId,
                         int responseType, int serial, RIL_Errno e, void *response,
                         size_t responselen);

int sendDtmfResponse(int slotId,
                    int responseType, int serial, RIL_Errno e, void *response,
                    size_t responselen);

int sendSmsResponse(int slotId,
                   int responseType, int serial, RIL_Errno e, void *response,
                   size_t responselen);

int sendSMSExpectMoreResponse(int slotId,
                             int responseType, int serial, RIL_Errno e, void *response,
                             size_t responselen);

int setupDataCallResponse(int slotId,
                          int responseType, int serial, RIL_Errno e, void *response,
                          size_t responseLen);

int iccIOForAppResponse(int slotId,
                       int responseType, int serial, RIL_Errno e, void *response,
                       size_t responselen);

int sendUssdResponse(int slotId,
                    int responseType, int serial, RIL_Errno e, void *response,
                    size_t responselen);

int cancelPendingUssdResponse(int slotId,
                             int responseType, int serial, RIL_Errno e, void *response,
                             size_t responselen);

int getClirResponse(int slotId,
                   int responseType, int serial, RIL_Errno e, void *response, size_t responselen);

int setClirResponse(int slotId,
                   int responseType, int serial, RIL_Errno e, void *response, size_t responselen);

int getCallForwardStatusResponse(int slotId,
                                int responseType, int serial, RIL_Errno e, void *response,
                                size_t responselen);

int setCallForwardResponse(int slotId,
                          int responseType, int serial, RIL_Errno e, void *response,
                          size_t responselen);

int getCallWaitingResponse(int slotId,
                          int responseType, int serial, RIL_Errno e, void *response,
                          size_t responselen);

int setCallWaitingResponse(int slotId,
                          int responseType, int serial, RIL_Errno e, void *response,
                          size_t responselen);

int acknowledgeLastIncomingGsmSmsResponse(int slotId,
                                         int responseType, int serial, RIL_Errno e, void *response,
                                         size_t responselen);

int acceptCallResponse(int slotId,
                      int responseType, int serial, RIL_Errno e, void *response,
                      size_t responselen);

int deactivateDataCallResponse(int slotId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responselen);

int getFacilityLockForAppResponse(int slotId,
                                 int responseType, int serial, RIL_Errno e, void *response,
                                 size_t responselen);

int setFacilityLockForAppResponse(int slotId,
                                 int responseType, int serial, RIL_Errno e, void *response,
                                 size_t responselen);

int setBarringPasswordResponse(int slotId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responselen);

int getNetworkSelectionModeResponse(int slotId,
                                   int responseType, int serial, RIL_Errno e, void *response,
                                   size_t responselen);

int setNetworkSelectionModeAutomaticResponse(int slotId,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responselen);

int setNetworkSelectionModeManualResponse(int slotId,
                                         int responseType, int serial, RIL_Errno e, void *response,
                                         size_t responselen);

int getAvailableNetworksResponse(int slotId,
                                int responseType, int serial, RIL_Errno e, void *response,
                                size_t responselen);

int startNetworkScanResponse(int slotId,
                             int responseType, int serial, RIL_Errno e, void *response,
                             size_t responselen);

int stopNetworkScanResponse(int slotId,
                            int responseType, int serial, RIL_Errno e, void *response,
                            size_t responselen);

int startDtmfResponse(int slotId,
                     int responseType, int serial, RIL_Errno e, void *response,
                     size_t responselen);

int stopDtmfResponse(int slotId,
                    int responseType, int serial, RIL_Errno e, void *response,
                    size_t responselen);

int getBasebandVersionResponse(int slotId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responselen);

int separateConnectionResponse(int slotId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responselen);

int setMuteResponse(int slotId,
                   int responseType, int serial, RIL_Errno e, void *response,
                   size_t responselen);

int getMuteResponse(int slotId,
                   int responseType, int serial, RIL_Errno e, void *response,
                   size_t responselen);

int getClipResponse(int slotId,
                   int responseType, int serial, RIL_Errno e, void *response,
                   size_t responselen);

int getDataCallListResponse(int slotId,
                            int responseType, int serial, RIL_Errno e,
                            void *response, size_t responseLen);

int setSuppServiceNotificationsResponse(int slotId,
                                       int responseType, int serial, RIL_Errno e, void *response,
                                       size_t responselen);

int writeSmsToSimResponse(int slotId,
                         int responseType, int serial, RIL_Errno e, void *response,
                         size_t responselen);

int deleteSmsOnSimResponse(int slotId,
                          int responseType, int serial, RIL_Errno e, void *response,
                          size_t responselen);

int setBandModeResponse(int slotId,
                       int responseType, int serial, RIL_Errno e, void *response,
                       size_t responselen);

int getAvailableBandModesResponse(int slotId,
                                 int responseType, int serial, RIL_Errno e, void *response,
                                 size_t responselen);

int sendEnvelopeResponse(int slotId,
                        int responseType, int serial, RIL_Errno e, void *response,
                        size_t responselen);

int sendTerminalResponseToSimResponse(int slotId,
                                     int responseType, int serial, RIL_Errno e, void *response,
                                     size_t responselen);

int handleStkCallSetupRequestFromSimResponse(int slotId,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responselen);

int explicitCallTransferResponse(int slotId,
                                int responseType, int serial, RIL_Errno e, void *response,
                                size_t responselen);

int setPreferredNetworkTypeResponse(int slotId,
                                   int responseType, int serial, RIL_Errno e, void *response,
                                   size_t responselen);

int getPreferredNetworkTypeResponse(int slotId,
                                   int responseType, int serial, RIL_Errno e, void *response,
                                   size_t responselen);

int getNeighboringCidsResponse(int slotId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responselen);

int setLocationUpdatesResponse(int slotId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responselen);

int setCdmaSubscriptionSourceResponse(int slotId,
                                     int responseType, int serial, RIL_Errno e, void *response,
                                     size_t responselen);

int setCdmaRoamingPreferenceResponse(int slotId,
                                    int responseType, int serial, RIL_Errno e, void *response,
                                    size_t responselen);

int getCdmaRoamingPreferenceResponse(int slotId,
                                    int responseType, int serial, RIL_Errno e, void *response,
                                    size_t responselen);

int setTTYModeResponse(int slotId,
                      int responseType, int serial, RIL_Errno e, void *response,
                      size_t responselen);

int getTTYModeResponse(int slotId,
                      int responseType, int serial, RIL_Errno e, void *response,
                      size_t responselen);

int setPreferredVoicePrivacyResponse(int slotId,
                                    int responseType, int serial, RIL_Errno e, void *response,
                                    size_t responselen);

int getPreferredVoicePrivacyResponse(int slotId,
                                    int responseType, int serial, RIL_Errno e, void *response,
                                    size_t responselen);

int sendCDMAFeatureCodeResponse(int slotId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responselen);

int sendBurstDtmfResponse(int slotId,
                         int responseType, int serial, RIL_Errno e, void *response,
                         size_t responselen);

int sendCdmaSmsResponse(int slotId,
                       int responseType, int serial, RIL_Errno e, void *response,
                       size_t responselen);

int acknowledgeLastIncomingCdmaSmsResponse(int slotId,
                                          int responseType, int serial, RIL_Errno e, void *response,
                                          size_t responselen);

int getGsmBroadcastConfigResponse(int slotId,
                                 int responseType, int serial, RIL_Errno e, void *response,
                                 size_t responselen);

int setGsmBroadcastConfigResponse(int slotId,
                                 int responseType, int serial, RIL_Errno e, void *response,
                                 size_t responselen);

int setGsmBroadcastActivationResponse(int slotId,
                                     int responseType, int serial, RIL_Errno e, void *response,
                                     size_t responselen);

int getCdmaBroadcastConfigResponse(int slotId,
                                  int responseType, int serial, RIL_Errno e, void *response,
                                  size_t responselen);

int setCdmaBroadcastConfigResponse(int slotId,
                                  int responseType, int serial, RIL_Errno e, void *response,
                                  size_t responselen);

int setCdmaBroadcastActivationResponse(int slotId,
                                      int responseType, int serial, RIL_Errno e,
                                      void *response, size_t responselen);

int getCDMASubscriptionResponse(int slotId,
                               int responseType, int serial, RIL_Errno e, void *response,
                               size_t responselen);

int writeSmsToRuimResponse(int slotId,
                          int responseType, int serial, RIL_Errno e, void *response,
                          size_t responselen);

int deleteSmsOnRuimResponse(int slotId,
                           int responseType, int serial, RIL_Errno e, void *response,
                           size_t responselen);

int getDeviceIdentityResponse(int slotId,
                             int responseType, int serial, RIL_Errno e, void *response,
                             size_t responselen);

int exitEmergencyCallbackModeResponse(int slotId,
                                     int responseType, int serial, RIL_Errno e, void *response,
                                     size_t responselen);

int getSmscAddressResponse(int slotId,
                          int responseType, int serial, RIL_Errno e, void *response,
                          size_t responselen);

int setCdmaBroadcastActivationResponse(int slotId,
                                      int responseType, int serial, RIL_Errno e,
                                      void *response, size_t responselen);

int setSmscAddressResponse(int slotId,
                          int responseType, int serial, RIL_Errno e,
                          void *response, size_t responselen);

int reportSmsMemoryStatusResponse(int slotId,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responselen);

int reportStkServiceIsRunningResponse(int slotId,
                                      int responseType, int serial, RIL_Errno e,
                                      void *response, size_t responseLen);

int getCdmaSubscriptionSourceResponse(int slotId,
                                     int responseType, int serial, RIL_Errno e, void *response,
                                     size_t responselen);

int requestIsimAuthenticationResponse(int slotId,
                                     int responseType, int serial, RIL_Errno e, void *response,
                                     size_t responselen);

int acknowledgeIncomingGsmSmsWithPduResponse(int slotId,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responselen);

int sendEnvelopeWithStatusResponse(int slotId,
                                  int responseType, int serial, RIL_Errno e, void *response,
                                  size_t responselen);

int getVoiceRadioTechnologyResponse(int slotId,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responselen);

int getCellInfoListResponse(int slotId,
                            int responseType,
                            int serial, RIL_Errno e, void *response,
                            size_t responseLen);

int setCellInfoListRateResponse(int slotId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responselen);

int setInitialAttachApnResponse(int slotId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responselen);

int getImsRegistrationStateResponse(int slotId,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responselen);

int sendImsSmsResponse(int slotId, int responseType,
                      int serial, RIL_Errno e, void *response, size_t responselen);

int iccTransmitApduBasicChannelResponse(int slotId,
                                       int responseType, int serial, RIL_Errno e,
                                       void *response, size_t responselen);

int iccOpenLogicalChannelResponse(int slotId,
                                  int responseType, int serial, RIL_Errno e, void *response,
                                  size_t responselen);


int iccCloseLogicalChannelResponse(int slotId,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responselen);

int iccTransmitApduLogicalChannelResponse(int slotId,
                                         int responseType, int serial, RIL_Errno e,
                                         void *response, size_t responselen);

int nvReadItemResponse(int slotId,
                      int responseType, int serial, RIL_Errno e,
                      void *response, size_t responselen);


int nvWriteItemResponse(int slotId,
                       int responseType, int serial, RIL_Errno e,
                       void *response, size_t responselen);

int nvWriteCdmaPrlResponse(int slotId,
                          int responseType, int serial, RIL_Errno e,
                          void *response, size_t responselen);

int nvResetConfigResponse(int slotId,
                         int responseType, int serial, RIL_Errno e,
                         void *response, size_t responselen);

int setUiccSubscriptionResponse(int slotId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responselen);

int setDataAllowedResponse(int slotId,
                          int responseType, int serial, RIL_Errno e,
                          void *response, size_t responselen);

int getHardwareConfigResponse(int slotId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responseLen);

int requestIccSimAuthenticationResponse(int slotId,
                                       int responseType, int serial, RIL_Errno e,
                                       void *response, size_t responselen);

int setDataProfileResponse(int slotId,
                          int responseType, int serial, RIL_Errno e,
                          void *response, size_t responselen);

int requestShutdownResponse(int slotId,
                           int responseType, int serial, RIL_Errno e,
                           void *response, size_t responselen);

int getRadioCapabilityResponse(int slotId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen);

int setRadioCapabilityResponse(int slotId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen);

int startLceServiceResponse(int slotId,
                           int responseType, int serial, RIL_Errno e,
                           void *response, size_t responselen);

int stopLceServiceResponse(int slotId,
                          int responseType, int serial, RIL_Errno e,
                          void *response, size_t responselen);

int pullLceDataResponse(int slotId,
                        int responseType, int serial, RIL_Errno e,
                        void *response, size_t responseLen);

int getModemActivityInfoResponse(int slotId,
                                int responseType, int serial, RIL_Errno e,
                                void *response, size_t responselen);

int setAllowedCarriersResponse(int slotId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responselen);

int getAllowedCarriersResponse(int slotId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responselen);

int sendDeviceStateResponse(int slotId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responselen);

int setIndicationFilterResponse(int slotId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responselen);

int setSimCardPowerResponse(int slotId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responselen);

int startKeepaliveResponse(int slotId,
                           int responseType, int serial, RIL_Errno e,
                           void *response, size_t responselen);

int stopKeepaliveResponse(int slotId,
                          int responseType, int serial, RIL_Errno e,
                          void *response, size_t responselen);

// int setSignalStrengthReportingCriteriaResponse(int slotId,
//                           int responseType, int serial, RIL_Errno e,
//                           void *response, size_t responselen);

int setLinkCapacityReportingCriteriaResponse(int slotId,
                          int responseType, int serial, RIL_Errno e,
                          void *response, size_t responselen);

void acknowledgeRequest(int slotId, int serial);

int radioStateChangedInd(int slotId,
                          int indicationType, int token, RIL_Errno e, void *response,
                          size_t responseLen);

int callStateChangedInd(int slotId, int indType, int token,
                        RIL_Errno e, void *response, size_t responselen);

int networkStateChangedInd(int slotId, int indType,
                                int token, RIL_Errno e, void *response, size_t responselen);

int newSmsInd(int slotId, int indicationType,
              int token, RIL_Errno e, void *response, size_t responselen);

int newSmsStatusReportInd(int slotId, int indicationType,
                          int token, RIL_Errno e, void *response, size_t responselen);

int newSmsOnSimInd(int slotId, int indicationType,
                   int token, RIL_Errno e, void *response, size_t responselen);

int onUssdInd(int slotId, int indicationType,
              int token, RIL_Errno e, void *response, size_t responselen);

int nitzTimeReceivedInd(int slotId, int indicationType,
                        int token, RIL_Errno e, void *response, size_t responselen);

int currentSignalStrengthInd(int slotId,
                             int indicationType, int token, RIL_Errno e,
                             void *response, size_t responselen);

int dataCallListChangedInd(int slotId, int indicationType,
                           int token, RIL_Errno e, void *response, size_t responselen);

int suppSvcNotifyInd(int slotId, int indicationType,
                     int token, RIL_Errno e, void *response, size_t responselen);

int stkSessionEndInd(int slotId, int indicationType,
                     int token, RIL_Errno e, void *response, size_t responselen);

int stkProactiveCommandInd(int slotId, int indicationType,
                           int token, RIL_Errno e, void *response, size_t responselen);

int stkEventNotifyInd(int slotId, int indicationType,
                      int token, RIL_Errno e, void *response, size_t responselen);

int stkCallSetupInd(int slotId, int indicationType,
                    int token, RIL_Errno e, void *response, size_t responselen);

int simSmsStorageFullInd(int slotId, int indicationType,
                         int token, RIL_Errno e, void *response, size_t responselen);

int simRefreshInd(int slotId, int indicationType,
                  int token, RIL_Errno e, void *response, size_t responselen);

int callRingInd(int slotId, int indicationType,
                int token, RIL_Errno e, void *response, size_t responselen);

int simStatusChangedInd(int slotId, int indicationType,
                        int token, RIL_Errno e, void *response, size_t responselen);

int cdmaNewSmsInd(int slotId, int indicationType,
                  int token, RIL_Errno e, void *response, size_t responselen);

int newBroadcastSmsInd(int slotId,
                       int indicationType, int token, RIL_Errno e, void *response,
                       size_t responselen);

int cdmaRuimSmsStorageFullInd(int slotId,
                              int indicationType, int token, RIL_Errno e, void *response,
                              size_t responselen);

int restrictedStateChangedInd(int slotId,
                              int indicationType, int token, RIL_Errno e, void *response,
                              size_t responselen);

int enterEmergencyCallbackModeInd(int slotId,
                                  int indicationType, int token, RIL_Errno e, void *response,
                                  size_t responselen);

int cdmaCallWaitingInd(int slotId,
                       int indicationType, int token, RIL_Errno e, void *response,
                       size_t responselen);

int cdmaOtaProvisionStatusInd(int slotId,
                              int indicationType, int token, RIL_Errno e, void *response,
                              size_t responselen);

int cdmaInfoRecInd(int slotId,
                   int indicationType, int token, RIL_Errno e, void *response,
                   size_t responselen);

int oemHookRawInd(int slotId,
                  int indicationType, int token, RIL_Errno e, void *response,
                  size_t responselen);

int indicateRingbackToneInd(int slotId,
                            int indicationType, int token, RIL_Errno e, void *response,
                            size_t responselen);

int resendIncallMuteInd(int slotId,
                        int indicationType, int token, RIL_Errno e, void *response,
                        size_t responselen);

int cdmaSubscriptionSourceChangedInd(int slotId,
                                     int indicationType, int token, RIL_Errno e,
                                     void *response, size_t responselen);

int cdmaPrlChangedInd(int slotId,
                      int indicationType, int token, RIL_Errno e, void *response,
                      size_t responselen);

int exitEmergencyCallbackModeInd(int slotId,
                                 int indicationType, int token, RIL_Errno e, void *response,
                                 size_t responselen);

int rilConnectedInd(int slotId,
                    int indicationType, int token, RIL_Errno e, void *response,
                    size_t responselen);

int voiceRadioTechChangedInd(int slotId,
                             int indicationType, int token, RIL_Errno e, void *response,
                             size_t responselen);

int cellInfoListInd(int slotId,
                           int indicationType, int token, RIL_Errno e, void *response,
                           size_t responseLen);
int cellInfoListInd_1_1(int slotId,
                    int indicationType, int token, RIL_Errno e, void *response,
                    size_t responselen);

int imsNetworkStateChangedInd(int slotId,
                              int indicationType, int token, RIL_Errno e, void *response,
                              size_t responselen);

int subscriptionStatusChangedInd(int slotId,
                                 int indicationType, int token, RIL_Errno e, void *response,
                                 size_t responselen);

int srvccStateNotifyInd(int slotId,
                        int indicationType, int token, RIL_Errno e, void *response,
                        size_t responselen);

int hardwareConfigChangedInd(int slotId,
                             int indicationType, int token, RIL_Errno e, void *response,
                             size_t responselen);

int radioCapabilityIndicationInd(int slotId,
                                 int indicationType, int token, RIL_Errno e, void *response,
                                 size_t responselen);

int onSupplementaryServiceIndicationInd(int slotId,
                                        int indicationType, int token, RIL_Errno e,
                                        void *response, size_t responselen);

int stkCallControlAlphaNotifyInd(int slotId,
                                 int indicationType, int token, RIL_Errno e, void *response,
                                 size_t responselen);

int lceDataInd(int slotId,
               int indicationType, int token, RIL_Errno e, void *response,
               size_t responselen);

int pcoDataInd(int slotId,
               int indicationType, int token, RIL_Errno e, void *response,
               size_t responselen);

int modemResetInd(int slotId,
                  int indicationType, int token, RIL_Errno e, void *response,
                  size_t responselen);

int networkScanResultInd(int slotId,
                         int indicationType, int token, RIL_Errno e, void *response,
                         size_t responselen);

int keepaliveStatusInd(int slotId,
                       int indicationType, int token, RIL_Errno e, void *response,
                       size_t responselen);

int sendRequestRawResponse(int slotId,
                           int responseType, int serial, RIL_Errno e,
                           void *response, size_t responseLen);

int sendRequestStringsResponse(int slotId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen);

int setCarrierInfoForImsiEncryptionResponse(int slotId,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responseLen);

int carrierInfoForImsiEncryption(int slotId,
                        int responseType, int serial, RIL_Errno e,
                        void *response, size_t responseLen);

pthread_rwlock_t * getRadioServiceRwlock(int slotId);

class RilSvcDataSetLinkCapCriteriaCallback : public GenericCallback<rildata::LinkCapCriteriaResult_t> {
private:
  int32_t mSerial;
  int mSlot;
public:
  inline RilSvcDataSetLinkCapCriteriaCallback(string clientToken,
      int32_t serial, int32_t slot) : GenericCallback(clientToken), mSerial(serial), mSlot(slot) {};

  inline ~RilSvcDataSetLinkCapCriteriaCallback() {};

  Message::Callback *clone() override;

  void onResponse(std::shared_ptr<Message> solicitedMsg, Status status,
      std::shared_ptr<rildata::LinkCapCriteriaResult_t> responseDataPtr) override;

};
}   // namespace radio
  void populateResponseInfo(RadioResponseInfo& responseInfo, int serial, int responseType,
            RIL_Errno e);

RadioIndicationType convertIntToRadioIndicationType(int indicationType);
hidl_string convertCharPtrToHidlString(const char *ptr);
void sendErrorResponse(RequestInfo *pRI, RIL_Errno err);
int fillNetworkScanRequest_1_1(const V1_1::NetworkScanRequest& request,
        RIL_NetworkScanRequest &scanRequest,
        RequestInfo *pRI);
int convertToHal(GsmSignalStrength &out, const RIL_GW_SignalStrength &in);
int convertToHal(GsmSignalStrength &out, const RIL_WCDMA_SignalStrength &in);
int convertToHal(WcdmaSignalStrength &out, const RIL_GW_SignalStrength &in);
int convertToHal(CdmaSignalStrength &out, const RIL_CDMA_SignalStrength &in);
int convertToHal(EvdoSignalStrength &out, const RIL_EVDO_SignalStrength &in);
int convertToHal(LteSignalStrength &out, RIL_LTE_SignalStrength_v8 in);
int convertToHal(TdScdmaSignalStrength &out, const RIL_TD_SCDMA_SignalStrength &in);
int convertRilSignalStrengthToHal(void *response, size_t responseLen,
        SignalStrength& signalStrength);
sp<RadioImpl> getRadioService(qcril_instance_id_e_type instance);

template <class T>
void fillCellIdentityGsm(T &out, const RIL_CellIdentityGsm_v12 &in);
template <class T>
void fillCellIdentityWcdma(T &out, const RIL_CellIdentityWcdma_v12 &in);
template <class T>
void fillCellIdentityCdma(T &out, const RIL_CellIdentityCdma &in);
template <class T>
void fillCellIdentityLte(T &out, const RIL_CellIdentityLte_v12 &in);
template <class T>
void fillCellIdentityTdscdma(T &out, const RIL_CellIdentityTdscdma &in);

template <class T>
void fillCellIdentityResponse(T &cellIdentity, const RIL_CellIdentity_v16 &rilCellIdentity) {

    cellIdentity.cellIdentityGsm.resize(0);
    cellIdentity.cellIdentityWcdma.resize(0);
    cellIdentity.cellIdentityCdma.resize(0);
    cellIdentity.cellIdentityTdscdma.resize(0);
    cellIdentity.cellIdentityLte.resize(0);
    cellIdentity.cellInfoType = (CellInfoType)rilCellIdentity.cellInfoType;
    switch(rilCellIdentity.cellInfoType) {

        case RIL_CELL_INFO_TYPE_GSM: {
            cellIdentity.cellIdentityGsm.resize(1);
            fillCellIdentityGsm(cellIdentity.cellIdentityGsm[0], rilCellIdentity.cellIdentityGsm);
            break;
        }

        case RIL_CELL_INFO_TYPE_WCDMA: {
            cellIdentity.cellIdentityWcdma.resize(1);
            fillCellIdentityWcdma(cellIdentity.cellIdentityWcdma[0], rilCellIdentity.cellIdentityWcdma);
            break;
        }

        case RIL_CELL_INFO_TYPE_CDMA: {
            cellIdentity.cellIdentityCdma.resize(1);
            fillCellIdentityCdma(cellIdentity.cellIdentityCdma[0], rilCellIdentity.cellIdentityCdma);
            break;
        }

        case RIL_CELL_INFO_TYPE_LTE: {
            cellIdentity.cellIdentityLte.resize(1);
            fillCellIdentityLte(cellIdentity.cellIdentityLte[0], rilCellIdentity.cellIdentityLte);
            break;
        }

        case RIL_CELL_INFO_TYPE_TD_SCDMA: {
            cellIdentity.cellIdentityTdscdma.resize(1);
            fillCellIdentityTdscdma(cellIdentity.cellIdentityTdscdma[0], rilCellIdentity.cellIdentityTdscdma);
            break;
        }

        default: {
            break;
        }
    }
}


struct RadioImpl : public V1_1::IRadio {
    int32_t mSlotId;
    qcril_instance_id_e_type mInstanceId;
    sp<IRadioResponse> mRadioResponse;
    sp<IRadioIndication> mRadioIndication;
    sp<V1_1::IRadioResponse> mRadioResponseV1_1;
    sp<V1_1::IRadioIndication> mRadioIndicationV1_1;

    RadioImpl(qcril_instance_id_e_type instance):
        mInstanceId(instance) {}

    virtual void clearCallbacks();

    Return<void> setResponseFunctions(
            const ::android::sp<IRadioResponse>& radioResponse,
            const ::android::sp<IRadioIndication>& radioIndication);

    Return<void> getIccCardStatus(int32_t serial);

    Return<void> supplyIccPinForApp(int32_t serial, const hidl_string& pin,
            const hidl_string& aid);

    Return<void> supplyIccPukForApp(int32_t serial, const hidl_string& puk,
            const hidl_string& pin, const hidl_string& aid);

    Return<void> supplyIccPin2ForApp(int32_t serial,
            const hidl_string& pin2,
            const hidl_string& aid);

    Return<void> supplyIccPuk2ForApp(int32_t serial, const hidl_string& puk2,
            const hidl_string& pin2, const hidl_string& aid);

    Return<void> changeIccPinForApp(int32_t serial, const hidl_string& oldPin,
            const hidl_string& newPin, const hidl_string& aid);

    Return<void> changeIccPin2ForApp(int32_t serial, const hidl_string& oldPin2,
            const hidl_string& newPin2, const hidl_string& aid);

    Return<void> supplyNetworkDepersonalization(int32_t serial, const hidl_string& netPin);

    Return<void> getCurrentCalls(int32_t serial);

    Return<void> dial(int32_t serial, const Dial& dialInfo);

    Return<void> getImsiForApp(int32_t serial,
            const ::android::hardware::hidl_string& aid);

    Return<void> hangup(int32_t serial, int32_t gsmIndex);

    Return<void> hangupWaitingOrBackground(int32_t serial);

    Return<void> hangupForegroundResumeBackground(int32_t serial);

    Return<void> switchWaitingOrHoldingAndActive(int32_t serial);

    Return<void> conference(int32_t serial);

    Return<void> rejectCall(int32_t serial);

    Return<void> getLastCallFailCause(int32_t serial);

    Return<void> getSignalStrength(int32_t serial);

    Return<void> getVoiceRegistrationState(int32_t serial);

    Return<void> getDataRegistrationState(int32_t serial);

    Return<void> getOperator(int32_t serial);

    Return<void> setRadioPower(int32_t serial, bool on);

    Return<void> sendDtmf(int32_t serial,
            const ::android::hardware::hidl_string& s);

    Return<void> sendSms(int32_t serial, const GsmSmsMessage& message);

    Return<void> sendSMSExpectMore(int32_t serial, const GsmSmsMessage& message);

    Return<void> setupDataCall(int32_t serial,
            RadioTechnology radioTechnology,
            const DataProfileInfo& profileInfo,
            bool modemCognitive,
            bool roamingAllowed,
            bool isRoaming);

    Return<void> iccIOForApp(int32_t serial,
            const IccIo& iccIo);

    Return<void> sendUssd(int32_t serial,
            const ::android::hardware::hidl_string& ussd);

    Return<void> cancelPendingUssd(int32_t serial);

    Return<void> getClir(int32_t serial);

    Return<void> setClir(int32_t serial, int32_t status);

    Return<void> getCallForwardStatus(int32_t serial,
            const CallForwardInfo& callInfo);

    Return<void> setCallForward(int32_t serial,
            const CallForwardInfo& callInfo);

    Return<void> getCallWaiting(int32_t serial, int32_t serviceClass);

    Return<void> setCallWaiting(int32_t serial, bool enable, int32_t serviceClass);

    Return<void> acknowledgeLastIncomingGsmSms(int32_t serial,
            bool success, SmsAcknowledgeFailCause cause);

    Return<void> acceptCall(int32_t serial);

    Return<void> deactivateDataCall(int32_t serial,
            int32_t cid, bool reasonRadioShutDown);

    Return<void> getFacilityLockForApp(int32_t serial,
            const ::android::hardware::hidl_string& facility,
            const ::android::hardware::hidl_string& password,
            int32_t serviceClass,
            const ::android::hardware::hidl_string& appId);

    Return<void> setFacilityLockForApp(int32_t serial,
            const ::android::hardware::hidl_string& facility,
            bool lockState,
            const ::android::hardware::hidl_string& password,
            int32_t serviceClass,
            const ::android::hardware::hidl_string& appId);

    Return<void> setBarringPassword(int32_t serial,
            const ::android::hardware::hidl_string& facility,
            const ::android::hardware::hidl_string& oldPassword,
            const ::android::hardware::hidl_string& newPassword);

    Return<void> getNetworkSelectionMode(int32_t serial);

    Return<void> setNetworkSelectionModeAutomatic(int32_t serial);

    Return<void> setNetworkSelectionModeManual(int32_t serial,
            const ::android::hardware::hidl_string& operatorNumeric);

    Return<void> getAvailableNetworks(int32_t serial);

    Return<void> startNetworkScan(int32_t serial, const V1_1::NetworkScanRequest& request);

    Return<void> stopNetworkScan(int32_t serial);

    Return<void> startDtmf(int32_t serial,
            const ::android::hardware::hidl_string& s);

    Return<void> stopDtmf(int32_t serial);

    Return<void> getBasebandVersion(int32_t serial);

    Return<void> separateConnection(int32_t serial, int32_t gsmIndex);

    Return<void> setMute(int32_t serial, bool enable);

    Return<void> getMute(int32_t serial);

    Return<void> getClip(int32_t serial);

    Return<void> getDataCallList(int32_t serial);

    Return<void> setSuppServiceNotifications(int32_t serial, bool enable);

    Return<void> writeSmsToSim(int32_t serial,
            const SmsWriteArgs& smsWriteArgs);

    Return<void> deleteSmsOnSim(int32_t serial, int32_t index);

    Return<void> setBandMode(int32_t serial, RadioBandMode mode);

    Return<void> getAvailableBandModes(int32_t serial);

    Return<void> sendEnvelope(int32_t serial,
            const ::android::hardware::hidl_string& command);

    Return<void> sendTerminalResponseToSim(int32_t serial,
            const ::android::hardware::hidl_string& commandResponse);

    Return<void> handleStkCallSetupRequestFromSim(int32_t serial, bool accept);

    Return<void> explicitCallTransfer(int32_t serial);

    Return<void> setPreferredNetworkType(int32_t serial, PreferredNetworkType nwType);

    Return<void> getPreferredNetworkType(int32_t serial);

    Return<void> getNeighboringCids(int32_t serial);

    Return<void> setLocationUpdates(int32_t serial, bool enable);

    Return<void> setCdmaSubscriptionSource(int32_t serial,
            CdmaSubscriptionSource cdmaSub);

    Return<void> setCdmaRoamingPreference(int32_t serial, CdmaRoamingType type);

    Return<void> getCdmaRoamingPreference(int32_t serial);

    Return<void> setTTYMode(int32_t serial, TtyMode mode);

    Return<void> getTTYMode(int32_t serial);

    Return<void> setPreferredVoicePrivacy(int32_t serial, bool enable);

    Return<void> getPreferredVoicePrivacy(int32_t serial);

    Return<void> sendCDMAFeatureCode(int32_t serial,
            const ::android::hardware::hidl_string& featureCode);

    Return<void> sendBurstDtmf(int32_t serial,
            const ::android::hardware::hidl_string& dtmf,
            int32_t on,
            int32_t off);

    Return<void> sendCdmaSms(int32_t serial, const CdmaSmsMessage& sms);

    Return<void> acknowledgeLastIncomingCdmaSms(int32_t serial,
            const CdmaSmsAck& smsAck);

    Return<void> getGsmBroadcastConfig(int32_t serial);

    Return<void> setGsmBroadcastConfig(int32_t serial,
            const hidl_vec<GsmBroadcastSmsConfigInfo>& configInfo);

    Return<void> setGsmBroadcastActivation(int32_t serial, bool activate);

    Return<void> getCdmaBroadcastConfig(int32_t serial);

    Return<void> setCdmaBroadcastConfig(int32_t serial,
            const hidl_vec<CdmaBroadcastSmsConfigInfo>& configInfo);

    Return<void> setCdmaBroadcastActivation(int32_t serial, bool activate);

    Return<void> getCDMASubscription(int32_t serial);

    Return<void> writeSmsToRuim(int32_t serial, const CdmaSmsWriteArgs& cdmaSms);

    Return<void> deleteSmsOnRuim(int32_t serial, int32_t index);

    Return<void> getDeviceIdentity(int32_t serial);

    Return<void> exitEmergencyCallbackMode(int32_t serial);

    Return<void> getSmscAddress(int32_t serial);

    Return<void> setSmscAddress(int32_t serial,
            const ::android::hardware::hidl_string& smsc);

    Return<void> reportSmsMemoryStatus(int32_t serial, bool available);

    Return<void> reportStkServiceIsRunning(int32_t serial);

    Return<void> getCdmaSubscriptionSource(int32_t serial);

    Return<void> requestIsimAuthentication(int32_t serial,
            const ::android::hardware::hidl_string& challenge);

    Return<void> acknowledgeIncomingGsmSmsWithPdu(int32_t serial,
            bool success,
            const ::android::hardware::hidl_string& ackPdu);

    Return<void> sendEnvelopeWithStatus(int32_t serial,
            const ::android::hardware::hidl_string& contents);

    Return<void> getVoiceRadioTechnology(int32_t serial);

    Return<void> getCellInfoList(int32_t serial);

    Return<void> setCellInfoListRate(int32_t serial, int32_t rate);

    Return<void> setInitialAttachApn(int32_t serial, const DataProfileInfo& dataProfileInfo,
            bool modemCognitive, bool isRoaming);

    Return<void> getImsRegistrationState(int32_t serial);

    Return<void> sendImsSms(int32_t serial, const ImsSmsMessage& message);

    Return<void> iccTransmitApduBasicChannel(int32_t serial, const SimApdu& message);

    Return<void> iccOpenLogicalChannel(int32_t serial,
            const ::android::hardware::hidl_string& aid, int32_t p2);

    Return<void> iccCloseLogicalChannel(int32_t serial, int32_t channelId);

    Return<void> iccTransmitApduLogicalChannel(int32_t serial, const SimApdu& message);

    Return<void> nvReadItem(int32_t serial, NvItem itemId);

    Return<void> nvWriteItem(int32_t serial, const NvWriteItem& item);

    Return<void> nvWriteCdmaPrl(int32_t serial,
            const ::android::hardware::hidl_vec<uint8_t>& prl);

    Return<void> nvResetConfig(int32_t serial, ResetNvType resetType);

    Return<void> setUiccSubscription(int32_t serial, const SelectUiccSub& uiccSub);

    Return<void> setDataAllowed(int32_t serial, bool allow);

    Return<void> getHardwareConfig(int32_t serial);

    Return<void> requestIccSimAuthentication(int32_t serial,
            int32_t authContext,
            const ::android::hardware::hidl_string& authData,
            const ::android::hardware::hidl_string& aid);

    Return<void> setDataProfile(int32_t serial,
            const ::android::hardware::hidl_vec<DataProfileInfo>& profiles, bool isRoaming);

    Return<void> requestShutdown(int32_t serial);

    Return<void> getRadioCapability(int32_t serial);

    Return<void> setRadioCapability(int32_t serial, const RadioCapability& rc);

    Return<void> startLceService(int32_t serial, int32_t reportInterval, bool pullMode);

    Return<void> stopLceService(int32_t serial);

    Return<void> pullLceData(int32_t serial);

    Return<void> getModemActivityInfo(int32_t serial);

    Return<void> setAllowedCarriers(int32_t serial,
            bool allAllowed,
            const CarrierRestrictions& carriers);

    Return<void> getAllowedCarriers(int32_t serial);

    Return<void> sendDeviceState(int32_t serial, DeviceStateType deviceStateType, bool state);

    Return<void> setIndicationFilter(int32_t serial, int32_t indicationFilter);

    Return<void> startKeepalive(int32_t serial, const V1_1::KeepaliveRequest& keepalive);

    Return<void> stopKeepalive(int32_t serial, int32_t sessionHandle);

    Return<void> setSimCardPower(int32_t serial, bool powerUp);
    Return<void> setSimCardPower_1_1(int32_t serial,
            const V1_1::CardPowerState state);

    Return<void> responseAcknowledgement();

    Return<void> setCarrierInfoForImsiEncryption(int32_t serial,
            const V1_1::ImsiEncryptionInfo& message);

    void checkReturnStatus(Return<void>& ret);

    Return<void> setSignalStrengthReportingCriteria(int32_t serial, int32_t hysteresisMs, int32_t hysteresisDb, const hidl_vec<int32_t>& thresholdsDbm, V1_1::RadioAccessNetworks ran);

    Return<void> setLinkCapacityReportingCriteria(int32_t serial, int32_t hysteresisMs, int32_t hysteresisDlKbps, int32_t hysteresisUlKbps, const hidl_vec<int32_t>& thresholdsDownlinkKbps, const hidl_vec<int32_t>& thresholdsUplinkKbps, V1_1::RadioAccessNetworks ran);

    virtual  int networkScanResultInd(int slotId,
                          int indicationType, int token, RIL_Errno e, void *response,
                          size_t responselen);

    static const QcrildServiceVersion &getVersion();
    virtual const char *getDescriptor() {
        return descriptor;
    }

    qcril_instance_id_e_type getInstanceId() {
        return mInstanceId;
    }
    virtual ::android::status_t registerAsService( const std::string &serviceName);
    virtual int sendCellInfoListInd(int slotId,
            int indicationType,
            int token,
            RIL_Errno e,
            void *response,
            size_t responseLen);

    virtual int sendGetCurrentCallsResponse(int slotId,
                                int responseType, int serial, RIL_Errno e,
                                void *response, size_t responseLen);
    virtual int sendGetIccCardStatusResponse(int slotId,
                                int responseType, int serial, RIL_Errno e,
                                void *response, size_t responseLen);
    virtual int sendGetCellInfoListResponse(int slotId,
                                int responseType,
                                int serial, RIL_Errno e, void *response,
                                size_t responseLen);
    virtual int sendCurrentSignalStrengthInd(int slotId,
                                int indicationType, int token, RIL_Errno e,
                                void *response, size_t responseLen);
    virtual int sendGetSignalStrengthResponse(int slotId,
                                int responseType, int serial, RIL_Errno e,
                                void *response, size_t responseLen);
    virtual int sendGetVoiceRegistrationStateResponse(int slotId,
                                int responseType, int serial, RIL_Errno e,
                                void *response, size_t responseLen);
    virtual int sendGetDataRegistrationStateResponse(int slotId,
                                int responseType, int serial, RIL_Errno e,
                                void *response, size_t responseLen);
    protected:
    Return<void> setResponseFunctions_nolock(
            const ::android::sp<IRadioResponse>& radioResponseParam,
            const ::android::sp<IRadioIndication>& radioIndicationParam);
    int convertGetCurrentCallsResponse(RadioResponseInfo &responseInfo,
            hidl_vec<Call> &calls,
            int responseType, int serial, RIL_Errno e,
            void *response, size_t responseLen);
    int convertToHidl(Call &hidlCall, RIL_Call &rilCall);
    int convertGetIccCardStatusResponse(RadioResponseInfo &responseInfo,
            V1_0::CardStatus &cardStatus, int responseType,
            int serial, RIL_Errno e, void *response, size_t responseLen);
    template <class T>
    int fillVoiceRegistrationStateResponse(T &voiceRegResponse,
            const RIL_VoiceRegistrationStateResponse &voiceRegState) {
        voiceRegResponse.regState = (RegState) voiceRegState.regState;
        voiceRegResponse.rat = voiceRegState.rat;;
        voiceRegResponse.cssSupported = voiceRegState.cssSupported;
        voiceRegResponse.roamingIndicator = voiceRegState.roamingIndicator;
        voiceRegResponse.systemIsInPrl = voiceRegState.systemIsInPrl;
        voiceRegResponse.defaultRoamingIndicator = voiceRegState.defaultRoamingIndicator;
        voiceRegResponse.reasonForDenial = voiceRegState.reasonForDenial;
        fillCellIdentityResponse(voiceRegResponse.cellIdentity,
                voiceRegState.cellIdentity);
        return 0;
    }
    template <class T>
    void fillDataRegistrationStateResponse(T &out, const RIL_DataRegistrationStateResponse &in) {
                out.regState = (RegState) in.regState;
                     out.rat = in.rat;
        out.reasonDataDenied = in.reasonDataDenied;
            out.maxDataCalls = in.maxDataCalls;
        fillCellIdentityResponse(out.cellIdentity, in.cellIdentity);
    }


};

#endif  // RIL_SERVICE_H
#endif
