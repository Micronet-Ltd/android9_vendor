/**
* Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#pragma once
#include "module/IDataModule.h"
#include "DataCommon.h"
#include "AuthManager.h"
#include "LceHandler.h"
#include "ProfileHandler.h"
#include "NetworkServiceHandler.h"

namespace rildata {

class DataModule : public IDataModule {
 public:
  DataModule();
  ~DataModule();
  void init();

 private:
  bool mAuthServiceReady = false;
  std::unique_ptr<AuthManager> auth_manager;
  LceHandler lceHandler;
  std::unique_ptr<ProfileHandler> profile_handler;
  std::unique_ptr<NetworkServiceHandler> network_service_handler;

   void handleQcrilInitMessage(std::shared_ptr<Message> msg);
   void broadcastReady();
   void handleSetupDataCallMessage(std::shared_ptr<Message> msg);
   void handleDataCallListMessage(std::shared_ptr<Message> msg);
   void handleGetDataCallProfileMessage(std::shared_ptr<Message> msg);
   void handleDeactivateDataCallMessage(std::shared_ptr<Message> msg);
   void handleEmbmsActivateDeactivateTmgiMessage(std::shared_ptr<Message> msg);
   void handleEmbmsActivateTmgiMessage(std::shared_ptr<Message> msg);
   void handleEmbmsContentDescUpdateMessage(std::shared_ptr<Message> msg);
   void handleEmbmsDeactivateTmgiMessage(std::shared_ptr<Message> msg);
   void handleEmbmsEnableDataReqMessage(std::shared_ptr<Message> msg);
   void handleEmbmsGetActiveTmgiMessage(std::shared_ptr<Message> msg);
   void handleEmbmsGetAvailTmgiMessage(std::shared_ptr<Message> msg);
   void handleEmbmsSendIntTmgiListMessage(std::shared_ptr<Message> msg);
   void handleGoDormantMessage(std::shared_ptr<Message> msg);
   void handleLastDataCallFailCauseMessage(std::shared_ptr<Message> msg);
   void handleProcessScreenStateChangeMessage(std::shared_ptr<Message> msg);
   void handleProcessStackSwitchMessage(std::shared_ptr<Message> msg);
   void handlePullLceDataMessage(std::shared_ptr<Message> msg);
   void handleSetApnInfoMessage(std::shared_ptr<Message> msg);
   void handleSetDataProfileMessage(std::shared_ptr<Message> msg);
   void handleSetIsDataEnabledMessage(std::shared_ptr<Message> msg);
   void handleSetIsDataRoamingEnabledMessage(std::shared_ptr<Message> msg);
   void handleSetLteAttachProfileMessage(std::shared_ptr<Message> msg);
   void handleSetQualityMeasurementMessage(std::shared_ptr<Message> msg);
   void handleSetRatPrefMessage(std::shared_ptr<Message> msg);
   void handleStartLceMessage(std::shared_ptr<Message> msg);
   void handleStopLceMessage(std::shared_ptr<Message> msg);
   void handleToggleDormancyIndMessage(std::shared_ptr<Message> msg);
   void handleToggleLimitedSysIndMessage(std::shared_ptr<Message> msg);
   void handleUpdateMtuMessage(std::shared_ptr<Message> msg);
   void handleDataEmbmsActiveMessage(std::shared_ptr<Message> msg);
   void handleGetDdsSubIdMessage(std::shared_ptr<Message> msg);
   void handleDataRequestDDSSwitchMessage(std::shared_ptr<Message> msg);
   void handleSetInitialAttachApn(std::shared_ptr<Message> msg);
   void handleSetLteAttachPdnListActionResult(std::shared_ptr<Message> msg);

#if (QCRIL_RIL_VERSION >= 15)
   void handleSetLteAttachProfileMessage_v15(std::shared_ptr<Message> msg);
   void handleSetDataProfileMessage_v15(std::shared_ptr<Message> msg);

   void handleStartKeepaliveMessage(std::shared_ptr<Message> msg);
   void handleStopKeepaliveMessage(std::shared_ptr<Message> msg);

   void handleSetCarrierInfoImsiEncryptionMessage(std::shared_ptr<Message> msg);
   void handleQmiAuthServiceIndMessage(std::shared_ptr<Message> msg);
   void handleQmiAuthEndpointStatusIndMessage(std::shared_ptr<Message> msg);
   void handleQmiDsdEndpointStatusIndMessage(std::shared_ptr<Message> msg);

   void handleSetLinkCapFilterMessage(std::shared_ptr<Message> msg);
   void handleSetLinkCapRptCriteriaMessage(std::shared_ptr<Message> msg);

#endif

   void handleNasSrvDomainPrefInd(std::shared_ptr<Message> msg);
   void handleNasRequestDataShutdown(std::shared_ptr<Message> msg);
   void handleGetDataNrIconType(std::shared_ptr<Message> msg);
   void handleDataAllBearerTypeUpdate(std::shared_ptr<Message> msg);
   void handleDataBearerTypeUpdate(std::shared_ptr<Message> msg);
   void handleToggleBearerAllocationUpdate(std::shared_ptr<Message> msg);
   void handleGetBearerAllocation(std::shared_ptr<Message> msg);
   void handleGetAllBearerAllocations(std::shared_ptr<Message> msg);
   void handleQmiWdsEndpointStatusIndMessage(std::shared_ptr<Message> msg);
   void handleDataConnectionStateChangedMessage(std::shared_ptr<Message> msg);

   TimeKeeper::timer_id setTimeoutForMsg(std::shared_ptr<Message> msg, TimeKeeper::millisec maxTimeout);
   RIL_Errno map_internalerr_from_reqlist_new_to_ril_err(IxErrnoType error);
   void deleteEntryInReqlist( std::shared_ptr<Message> msg );
   void handleUimCardAppStatusIndMsg(std::shared_ptr<Message> msg);

};
void qcrilDataprocessMccMncInfo
(
    const qcril_request_params_type *const params_ptr,
    qcril_request_return_type       *const ret_ptr
);

} //namespace
