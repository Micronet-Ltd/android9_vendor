/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/Module.h"
#include "modules/qmi/NasModemEndPoint.h"
#include "modules/qmi/QmiAsyncResponseMessage.h"
#include "modules/qmi/EndpointStatusIndMessage.h"

#include "modules/nas/NasUimHeaders.h"
#include "modules/nas/NasSetNetworkSelectionManualRequestMessage.h"
#include "modules/nas/NasRequestDataShutdownMessage.h"

#include <modules/android/RilRequestMessage.h>
#include <modules/ims/ImsServiceStatusInd.h>
#include <modules/ims/ImsWfcSettingsStatusInd.h>
#include <modules/ims/ImsPdpStatusInd.h>
#include "modules/nas/NasSetVoiceDomainPreferenceRequest.h"

// Messages from Data
#include "modules/nas/NasEmbmsEnableDataConMessage.h"
#include "modules/nas/NasEmbmsDisableDataMessage.h"
// NAS exported API
#include "modules/nas/NasSetPsAttachDetachMessage.h"
#include "modules/nas/NasGetSysInfoMessage.h"

//Data Unsol Messages
#include "modules/nas/NasDataCache.h"
#include "CallStatusMessage.h"
#include "DataSysStatusMessage.h"
#include "SetPrefDataTechMessage.h"
#include "DsdSysStatusMessage.h"
#ifndef QMI_RIL_UTF
#include "DataRegistrationStateMessage.h"
#endif
#include "NewDDSInfoMessage.h"
#include "DDSStatusFollowupMessage.h"

#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"

enum class NasSettingResultCode {
    SUCCESS = RIL_E_SUCCESS,
    FAILURE = RIL_E_INTERNAL_ERR,
};

/** NasSetSignalStrengthCriteriaSupported.
 * This message informs the Nas module that the
 * SetSignalStrengthReportingCriteria request is supported.
 * If the parameter "supported" is true, it will adjust its
 * call to QMI_NAS_CONFIG_SIG_INFO2 to prevent overriding
 * user-provided values.
 */
class NasSetSignalStrengthCriteriaSupported : public SolicitedMessage<NasSettingResultCode>,
                              public add_message_id<NasSetSignalStrengthCriteriaSupported>
{
 private:
  bool mSupported;

 public:
 static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.nas.SetSignalStrengthCriteriaSupported";
 NasSetSignalStrengthCriteriaSupported() = delete;
  ~NasSetSignalStrengthCriteriaSupported() {}

  inline NasSetSignalStrengthCriteriaSupported(bool supported):
      SolicitedMessage<NasSettingResultCode>(get_class_message_id()),
      mSupported(supported)
  {
    mName = MESSAGE_NAME;
  }

  inline bool isSupported() {
    return mSupported;
  }

  string dump() {
      return mName + "[ mSupported = " + (mSupported ? "true" : "false") + "]";
  }
};
/** NasEnablePhysChanConfigReporting.
 * This message informs the Nas module whether it should
 * report the physical channel config information.
 * If the parameter "enabled" is true, the indication will be
 * enabled.
 */
class NasEnablePhysChanConfigReporting : public SolicitedMessage<NasSettingResultCode>,
                              public add_message_id<NasEnablePhysChanConfigReporting>
{
 private:
  bool mEnable;

 public:
 using cb_t = GenericCallback<NasSettingResultCode>::cb_t;
 static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.nas.EnablePhysChanConfigReporting";
 NasEnablePhysChanConfigReporting() = delete;
  ~NasEnablePhysChanConfigReporting() {}

  inline NasEnablePhysChanConfigReporting(bool enabled, std::weak_ptr<MessageContext> ctx, cb_t callbackfn):
      SolicitedMessage<NasSettingResultCode>(MESSAGE_NAME, ctx, get_class_message_id()),
      mEnable(enabled)
  {
      GenericCallback<NasSettingResultCode> cb(callbackfn);
      setCallback(&cb);
  }

  inline bool isEnable() {
    return mEnable;
  }

  string dump() {
      return mName + "[ mEnable = " + (mEnable ? "true" : "false") + "]";
  }
};


class NasPhysChanInfo
{
    public:
        enum class Status {
            NONE,
            PRIMARY,
            SECONDARY,
        };
        NasPhysChanInfo(Status status, int32_t bw):
            mStatus(status), mBandwidth(bw) {}

        Status getStatus() const { return mStatus; }
        int32_t getBandwidth() const { return mBandwidth; }
    private:
        Status mStatus;
        int32_t mBandwidth;

};

int32_t toKhz(nas_bandwidth_enum_type_v01 bw);
int32_t toKhz(nas_lte_cphy_ca_bandwidth_enum_v01 bw);

/** NasPhysChanConfigMessage.
 * This message is sent by the Nas module to notify of changes in the
 * physical channel configuration
 */
class NasPhysChanConfigMessage : public UnSolicitedMessage,
                              public add_message_id<NasPhysChanConfigMessage>
{
 private:
     std::vector<NasPhysChanInfo> info;

 public:
 static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.nas.PhysChanConfig";
  ~NasPhysChanConfigMessage() {}

  NasPhysChanConfigMessage():
        UnSolicitedMessage(get_class_message_id()) {
      mName = MESSAGE_NAME;
  }

  inline const std::vector<NasPhysChanInfo> &getInfo() {
    return info;
  }
  void addInfo(NasPhysChanInfo &&entry) {
      info.push_back(std::move(entry));
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
      return nullptr;
  }
  string dump() {
      return mName;
  }
};

class NasModule : public Module {
  public:
    NasModule();
    ~NasModule();
    void init();
#ifdef QMI_RIL_UTF
    void qcrilHalNasModuleCleanup();
#endif

    struct PhysChanInfo
    {
        bool bw_info_valid;
        nas_rf_bandwidth_info_type_v01 bw_info;
        bool pcell_info_valid;
        nas_lte_cphy_pcell_info_type_v01 pcell_info;
        bool scell_info_valid;
        nas_lte_cphy_scell_info_type_v01 scell_info;
        bool unchanged_scell_info_list_valid;
        uint32_t unchanged_scell_info_list_len;
        nas_cphy_scell_info_v01 unchanged_scell_info_list[NAS_MAX_SCELL_LIST_LEN_V01];
    };

  private:
    bool mReady = false;
    std::recursive_mutex mMutex;
    bool mSetSignalStrengthCriteriaSupported = false;
    PhysChanInfo chaninfo;


  public:
    bool getSetSignalStrengthCriteriaSupported() {
        return mSetSignalStrengthCriteriaSupported;
    }
    int32_t getLteBw();
    void resetPhysChanConfig();
  private:

    void handleQcrilInit(std::shared_ptr<Message> msg);
    void handleNasQmiIndMessage(std::shared_ptr<Message> msg);
    void handleQmiAsyncRespMessage(std::shared_ptr<QmiAsyncResponseMessage> msg);
    void handleNasEndpointStatusIndMessage(std::shared_ptr<Message> msg);

    void handleGetPreferredNetworkType(std::shared_ptr<RilRequestMessage> msg);
    void handleSetPreferredNetworkType(std::shared_ptr<RilRequestMessage> msg);
    void handleEnable5g(std::shared_ptr<RilRequestMessage> shared_msg);
    void handleEnable5gOnly(std::shared_ptr<RilRequestMessage> shared_msg);
    void handleDisable5g(std::shared_ptr<RilRequestMessage> shared_msg);
    void handleQuery5gStatus(std::shared_ptr<RilRequestMessage> shared_msg);
    void handleQueryEndcDcnr(std::shared_ptr<RilRequestMessage> shared_msg);
    void handleQueryNrBearerAllocation(std::shared_ptr<RilRequestMessage> shared_msg);
    void handleQueryNrSignalStrength(std::shared_ptr<RilRequestMessage> shared_msg);
    void handleQueryUpperLayerIndInfo(std::shared_ptr<RilRequestMessage> shared_msg);
    void handleQuery5gConfigInfo(std::shared_ptr<RilRequestMessage> shared_msg);
    void handleSetCdmaSubscription(std::shared_ptr<RilRequestMessage> msg);
    void handleSetCdmaSubscriptionSource(std::shared_ptr<RilRequestMessage> msg);
    void handleExitEmergencyCallbackMode(std::shared_ptr<RilRequestMessage> msg);
    void handleGetVoiceRadioTech(std::shared_ptr<RilRequestMessage> msg);
    void handleSetUiccSubscription(std::shared_ptr<RilRequestMessage> msg);
    void handleGetDcRtInfo(std::shared_ptr<RilRequestMessage> msg);
    void handleShutdown(std::shared_ptr<RilRequestMessage> msg);
    void handleGetRadioCapability(std::shared_ptr<RilRequestMessage> msg);
    void handleSetRadioCapability(std::shared_ptr<RilRequestMessage> msg);
    void handleQueryNetworkSelectionMode(std::shared_ptr<RilRequestMessage> msg);
    void handleSetBandMode(std::shared_ptr<RilRequestMessage> msg);
    void handleGetNeighboringCellIds(std::shared_ptr<RilRequestMessage> msg);
    void handleSetNetworkSelectionAutomatic(std::shared_ptr<RilRequestMessage> msg);
    void handleSetNetworkSelectionManual(
        std::shared_ptr<NasSetNetworkSelectionManualRequestMessage> msg);
    void handleVoiceRegistrationState(std::shared_ptr<RilRequestMessage> msg);
    void handleDataRegistrationState(std::shared_ptr<RilRequestMessage> msg);
    void handleOperator(std::shared_ptr<RilRequestMessage> msg);
    void handleAllowData(std::shared_ptr<RilRequestMessage> msg);
    void handleCdmaSubscription(std::shared_ptr<RilRequestMessage> msg);
    void handleCdmaSetRoamingPreference(std::shared_ptr<RilRequestMessage> msg);
    void handleSetLocationUpdates(std::shared_ptr<RilRequestMessage> msg);
    void handleQueryAvailableNetworks(std::shared_ptr<RilRequestMessage> msg);
    void handleCancelQueryAvailableNetworks(std::shared_ptr<RilRequestMessage> msg);
    void handleQueryAvailableBandMode(std::shared_ptr<RilRequestMessage> msg);
    void handleSignalStrength(std::shared_ptr<RilRequestMessage> msg);
    void handleSetScreenState(std::shared_ptr<RilRequestMessage> msg);
    void handleSetDeviceState(std::shared_ptr<RilRequestMessage> msg);
    void handleSetUnsolRespFilter(std::shared_ptr<RilRequestMessage> msg);
    void handleStartNetworkScan(std::shared_ptr<RilRequestMessage> msg);
    void handleStopNetworkScan(std::shared_ptr<RilRequestMessage> msg);

    // UIM handling
    void handleUimProvisioningStatusIndMsg(std::shared_ptr<UimProvisionStatusIndMsg> msg);
    void handleUimCardStatusIndMsg(std::shared_ptr<UimCardStatusIndMsg> msg);
    void handleUimCardStatusChangeIndMsg(std::shared_ptr<UimCardStatusChangeIndMsg> msg);
    void handleUimCardAppStatusIndMsg(std::shared_ptr<UimCardAppStatusIndMsg> msg);

    // IMS handling
    void handleImsServiceStatusInd(std::shared_ptr<ImsServiceStatusInd> msg);
    void handleImsWfcStatusInd(std::shared_ptr<ImsWfcSettingsStatusInd> msg);
    void handleImsPdpStatusInd(std::shared_ptr<ImsPdpStatusInd> msg);
    void handleSetVoiceDomainPreferenceRequest(std::shared_ptr<NasSetVoiceDomainPreferenceRequest> msg);

    //Handlers for Messages from Data
    void handleEmbmsEnableDataConMessage(std::shared_ptr<NasEmbmsEnableDataConMessage> msg);
    void handleEmbmsDisableDataMessage(std::shared_ptr<NasEmbmsDisableDataMessage> msg);

    //Data Indication Handlers
    void handleDataCallStatusMessage(std::shared_ptr<rildata::CallStatusMessage> msg);
    void handleDataSetPrefDataTechMessage(std::shared_ptr<rildata::SetPrefDataTechMessage> msg);
    void handleDataSysStatusMessage(std::shared_ptr<rildata::DataSysStatusMessage> msg);
    void handleDataDsdSysStatusMessage(std::shared_ptr<rildata::DsdSysStatusMessage> msg);
#ifndef QMI_RIL_UTF
    void handleDataRegistrationStateMessage(std::shared_ptr<rildata::DataRegistrationStateMessage> msg);
#endif
    void handleDataNewDDSInfoMessage(std::shared_ptr<rildata::NewDDSInfoMessage> msg);
    void handleDataDDSStatusFollowupMessage(std::shared_ptr<rildata::DDSStatusFollowupMessage> msg);

    // Nas API
    void handlePsAttachDetachMessage(std::shared_ptr<NasSetPsAttachDetachMessage> msg);
    void handleNasGetSysInfoMessage(std::shared_ptr<NasGetSysInfoMessage> msg);

    bool isDmsEndpointReady();
    bool shouldReportScell(const nas_lte_cphy_scell_info_type_v01 &scell_info);

    // Handler for DMS Endpoint Status Indications
    void handleDmsEndpointStatusIndMsg(std::shared_ptr<Message> msg);

    // Settings
    void handleSetSignalStrengthCriteriaSupported(
            std::shared_ptr<NasSetSignalStrengthCriteriaSupported> msg);
    void handleEnablePhysChanConfigReporting(
            std::shared_ptr<NasEnablePhysChanConfigReporting> msg);
    void reportCurrentPhysChanConfig();
    void handleQmiNasRfBandInfoMessage(
            std::shared_ptr<QmiNasRfBandInfoMessage> bandInfoMsg);
    void handleQmiNasLteCphyCaIndMessage(
            std::shared_ptr<QmiNasLteCphyCaIndMessage> lteCphyMsg);
};

NasModule &getNasModule();

