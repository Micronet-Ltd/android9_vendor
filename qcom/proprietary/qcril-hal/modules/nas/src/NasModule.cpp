/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#define TAG "[NasModule]"
#include <cstring>
#include <framework/ModuleLooper.h>
#include <framework/QcrilInitMessage.h>
#include <framework/PolicyManager.h>
#include <framework/SingleDispatchRestriction.h>
#include <framework/SingleDispatchRestrictionHandleDuplicate.h>
#include <framework/RejectSimultaneousDispatchRestriction.h>
#include <framework/ThreadPoolManager.h>

#include <modules/android/ril_message_factory.h>

#include "modules/nas/qcril_qmi_prov.h"
#include "qcril_qmi_client.h"
#include "qcril_am.h"

#include "modules/nas/qcril_qmi_nas.h"
#include "modules/nas/qcril_qmi_nas2.h"
#include "modules/nas/qcril_arb.h"
#include "modules/nas/qcril_nas_legacy.h"
#include "modules/nas/NasUimHeaders.h"

#include "modules/nas/NasModule.h"
#include "modules/android/LegacyHandlerSubModule.h"
#include "modules/android/LegacyHandlerMessage.h"
#include "modules/qmi/QmiIndMessage.h"
#include "modules/qmi/EndpointStatusIndMessage.h"
#include "modules/qmi/ModemEndPointFactory.h"
#include "modules/qmi/NasModemEndPoint.h"
#include "modules/nas/NasImsHelper.h"
#include "modules/qmi/QmiNasSetupRequestCallback.h"

//Data related
#include "modules/nas/NasDataCache.h"
#ifndef QMI_RIL_UTF
#include "SetInitialAttachApnRequestMessage.h"
#endif
#include "DataCommon.h"

using rildata::CallStatusMessage;
using rildata::DataSysStatusMessage;
using rildata::SetPrefDataTechMessage;
using rildata::DsdSysStatusMessage;
#ifndef QMI_RIL_UTF
using rildata::DataRegistrationStateMessage;
#endif
using rildata::NewDDSInfoMessage;
using rildata::DDSStatusFollowupMessage;

#include "telephony/ril.h"

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_ALLOW_DATA, int *, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_ALLOW_DATA);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_CDMA_SUBSCRIPTION, void, char**);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_CDMA_SUBSCRIPTION);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_DATA_REGISTRATION_STATE, void, char **);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_DATA_REGISTRATION_STATE);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE, void, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_GET_DC_RT_INFO, void, RIL_DcRtInfo*);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_GET_DC_RT_INFO);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_GET_NEIGHBORING_CELL_IDS, void, RIL_NeighboringCell**);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_GET_NEIGHBORING_CELL_IDS);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE, void, int*);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_GET_RADIO_CAPABILITY, void, RIL_RadioCapability*);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_GET_RADIO_CAPABILITY);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_VOICE_RADIO_TECH, void, RIL_RadioTechnology*);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_VOICE_RADIO_TECH);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_OPERATOR, void, char **);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_OPERATOR);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE, void, int *);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_QUERY_AVAILABLE_NETWORKS, void,  char **);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_QUERY_AVAILABLE_NETWORKS);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_CANCEL_QUERY_AVAILABLE_NETWORKS, void, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_CANCEL_QUERY_AVAILABLE_NETWORKS);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE, void, int*);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_SCREEN_STATE, int*, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_SCREEN_STATE);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_SEND_DEVICE_STATE, int*, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_SEND_DEVICE_STATE);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER, int*, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_SET_BAND_MODE, RIL_RadioBandMode*, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_SET_BAND_MODE);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE, int *, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE, RIL_CdmaSubscriptionSource*, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_SET_LOCATION_UPDATES, int *, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_SET_LOCATION_UPDATES);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC, void, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC);


QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE, RIL_PreferredNetworkType*, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_ENABLE_5G, RIL_5G_MODE*, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_ENABLE_5G);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_ENABLE_5G_ONLY, RIL_5G_MODE*, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_ENABLE_5G_ONLY);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_DISABLE_5G, RIL_5G_MODE*, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_DISABLE_5G);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_QUERY_5G_STATUS, void, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_QUERY_5G_STATUS);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_QUERY_ENDC_DCNR, void, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_QUERY_ENDC_DCNR);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_QUERY_NR_BEARER_ALLOCATION, void, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_QUERY_NR_BEARER_ALLOCATION);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_QUERY_NR_SIGNAL_STRENGTH, void, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_QUERY_NR_SIGNAL_STRENGTH);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_QUERY_UPPERLAYER_IND_INFO, void, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_QUERY_UPPERLAYER_IND_INFO);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_QUERY_5G_CONFIG_INFO, void, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_QUERY_5G_CONFIG_INFO);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_SET_RADIO_CAPABILITY, RIL_RadioCapability*, RIL_RadioCapability*);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_SET_RADIO_CAPABILITY);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_SET_UICC_SUBSCRIPTION, RIL_SelectUiccSub*, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_SET_UICC_SUBSCRIPTION);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_SHUTDOWN, void, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_SHUTDOWN);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_SIGNAL_STRENGTH, void, RIL_SignalStrength*);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_SIGNAL_STRENGTH);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_VOICE_REGISTRATION_STATE, void, char **);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_VOICE_REGISTRATION_STATE);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_START_NETWORK_SCAN, RIL_NetworkScanRequest *, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_START_NETWORK_SCAN);

QCRIL_ANDROID_DECLARE_REQUEST(RIL_REQUEST_STOP_NETWORK_SCAN, void, void);
QCRIL_ANDROID_DEFINE_REQUEST(RIL_REQUEST_STOP_NETWORK_SCAN);

static load_module<NasModule> nas_module;

NasModule &getNasModule() {
    return (nas_module.get_module());
}

//TODO: Remove this while cleaning up
static qcril_arb_pref_data_tech_e_type map_data_pref_data_tech_to_arb_pref_data_tech(qcril_data_pref_data_tech_e_type pref_data_tech);
static qcril_qmi_nas_data_call_event_type map_data_call_event_to_internal(qcril_data_net_evt_t evt);
static dds_status_info map_dds_status_info_from_data(DDSStatusInfo status_info);
/*
 * 1. Indicate your preference for looper.
 * 2. Subscribe to the list of messages via mMessageHandler.
 * 3. Follow RAII practice.
 */
NasModule::NasModule() {
  mName = "NasModule";
  ModemEndPointFactory<NasModemEndPoint>::getInstance().buildEndPoint();

  using std::placeholders::_1;
  mMessageHandler = {
    HANDLER(QcrilInitMessage, NasModule::handleQcrilInit),

    LEGACY_HANDLER(RIL_REQUEST_VOICE_REGISTRATION_STATE, NasModule::handleVoiceRegistrationState),
    LEGACY_HANDLER(RIL_REQUEST_DATA_REGISTRATION_STATE, NasModule::handleDataRegistrationState),
    LEGACY_HANDLER(RIL_REQUEST_OPERATOR, NasModule::handleOperator),
    LEGACY_HANDLER(RIL_REQUEST_ALLOW_DATA, NasModule::handleAllowData),
    LEGACY_HANDLER(RIL_REQUEST_CDMA_SUBSCRIPTION, NasModule::handleCdmaSubscription),
    LEGACY_HANDLER(RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE, NasModule::handleCdmaSetRoamingPreference),
    LEGACY_HANDLER(RIL_REQUEST_SET_LOCATION_UPDATES, NasModule::handleSetLocationUpdates),
    LEGACY_HANDLER(RIL_REQUEST_QUERY_AVAILABLE_NETWORKS, NasModule::handleQueryAvailableNetworks),
    LEGACY_HANDLER(RIL_REQUEST_CANCEL_QUERY_AVAILABLE_NETWORKS, NasModule::handleCancelQueryAvailableNetworks),
    LEGACY_HANDLER(RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE, NasModule::handleQueryAvailableBandMode),
    LEGACY_HANDLER(RIL_REQUEST_SIGNAL_STRENGTH, NasModule::handleSignalStrength),
    LEGACY_HANDLER(RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE,  NasModule::handleGetPreferredNetworkType),
    LEGACY_HANDLER(RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE,NasModule::handleSetPreferredNetworkType),
    LEGACY_HANDLER(RIL_REQUEST_ENABLE_5G, NasModule::handleEnable5g),
    LEGACY_HANDLER(RIL_REQUEST_ENABLE_5G_ONLY, NasModule::handleEnable5gOnly),
    LEGACY_HANDLER(RIL_REQUEST_DISABLE_5G, NasModule::handleDisable5g),
    LEGACY_HANDLER(RIL_REQUEST_QUERY_5G_STATUS, NasModule::handleQuery5gStatus),
    LEGACY_HANDLER(RIL_REQUEST_QUERY_ENDC_DCNR, NasModule::handleQueryEndcDcnr),
    LEGACY_HANDLER(RIL_REQUEST_QUERY_NR_BEARER_ALLOCATION, NasModule::handleQueryNrBearerAllocation),
    LEGACY_HANDLER(RIL_REQUEST_QUERY_NR_SIGNAL_STRENGTH, NasModule::handleQueryNrSignalStrength),
    LEGACY_HANDLER(RIL_REQUEST_QUERY_UPPERLAYER_IND_INFO, NasModule::handleQueryUpperLayerIndInfo),
    LEGACY_HANDLER(RIL_REQUEST_QUERY_5G_CONFIG_INFO, NasModule::handleQuery5gConfigInfo),
    LEGACY_HANDLER(RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE, NasModule::handleSetCdmaSubscriptionSource),
    LEGACY_HANDLER(RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE, NasModule::handleExitEmergencyCallbackMode),
    LEGACY_HANDLER(RIL_REQUEST_VOICE_RADIO_TECH, NasModule::handleGetVoiceRadioTech),
    LEGACY_HANDLER(RIL_REQUEST_SET_UICC_SUBSCRIPTION, NasModule::handleSetUiccSubscription),
    LEGACY_HANDLER(RIL_REQUEST_GET_DC_RT_INFO, NasModule::handleGetDcRtInfo),
    LEGACY_HANDLER(RIL_REQUEST_SHUTDOWN, NasModule::handleShutdown),
    LEGACY_HANDLER(RIL_REQUEST_GET_RADIO_CAPABILITY, NasModule::handleGetRadioCapability),
    LEGACY_HANDLER(RIL_REQUEST_SET_RADIO_CAPABILITY, NasModule::handleSetRadioCapability),
    LEGACY_HANDLER(RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE, NasModule::handleQueryNetworkSelectionMode),
    LEGACY_HANDLER(RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC, NasModule::handleSetNetworkSelectionAutomatic),
    LEGACY_HANDLER(RIL_REQUEST_SET_BAND_MODE, NasModule::handleSetBandMode),
    LEGACY_HANDLER(RIL_REQUEST_GET_NEIGHBORING_CELL_IDS, NasModule::handleGetNeighboringCellIds),
    LEGACY_HANDLER(RIL_REQUEST_SCREEN_STATE, NasModule::handleSetScreenState),
    LEGACY_HANDLER(RIL_REQUEST_SEND_DEVICE_STATE, NasModule::handleSetDeviceState),
    LEGACY_HANDLER(RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER, NasModule::handleSetUnsolRespFilter),
    HANDLER(NasSetNetworkSelectionManualRequestMessage, NasModule::handleSetNetworkSelectionManual),
    LEGACY_HANDLER(RIL_REQUEST_START_NETWORK_SCAN, NasModule::handleStartNetworkScan),
    LEGACY_HANDLER(RIL_REQUEST_STOP_NETWORK_SCAN, NasModule::handleStopNetworkScan),
    HANDLER(NasSetSignalStrengthCriteriaSupported, NasModule::handleSetSignalStrengthCriteriaSupported),
    HANDLER(NasEnablePhysChanConfigReporting, NasModule::handleEnablePhysChanConfigReporting),
    HANDLER(QmiNasRfBandInfoMessage, NasModule::handleQmiNasRfBandInfoMessage),
    HANDLER(QmiNasLteCphyCaIndMessage, NasModule::handleQmiNasLteCphyCaIndMessage),


    {REG_MSG("NAS_QMI_IND"),
             std::bind(&NasModule::handleNasQmiIndMessage, this, _1)},
    // End Point Status Indication
    {REG_MSG("NAS_ENDPOINT_STATUS_IND"),
             std::bind(&NasModule::handleNasEndpointStatusIndMessage, this, _1)},
    // Qmi Async Response
    HANDLER(QmiAsyncResponseMessage, NasModule::handleQmiAsyncRespMessage),

    {REG_MSG("DMS_ENDPOINT_STATUS_IND"),
             std::bind(&NasModule::handleDmsEndpointStatusIndMsg, this, _1)},

    // UIM indications
    HANDLER(UimProvisionStatusIndMsg, NasModule::handleUimProvisioningStatusIndMsg),
    HANDLER(UimCardStatusIndMsg, NasModule::handleUimCardStatusIndMsg),
    HANDLER(UimCardStatusChangeIndMsg, NasModule::handleUimCardStatusChangeIndMsg),
    HANDLER(UimCardAppStatusIndMsg, NasModule::handleUimCardAppStatusIndMsg),

    // IMS handling
    HANDLER(ImsServiceStatusInd, NasModule::handleImsServiceStatusInd),
    HANDLER(ImsWfcSettingsStatusInd, NasModule::handleImsWfcStatusInd),
    HANDLER(ImsPdpStatusInd, NasModule::handleImsPdpStatusInd),
    HANDLER(NasSetVoiceDomainPreferenceRequest, NasModule::handleSetVoiceDomainPreferenceRequest),

    //Messages from Data
    HANDLER(NasEmbmsEnableDataConMessage, NasModule::handleEmbmsEnableDataConMessage),
    HANDLER(NasEmbmsDisableDataMessage, NasModule::handleEmbmsDisableDataMessage),

#ifndef QMI_RIL_UTF
    //Data indications
    HANDLER(CallStatusMessage, NasModule::handleDataCallStatusMessage),
    HANDLER(SetPrefDataTechMessage, NasModule::handleDataSetPrefDataTechMessage),
    HANDLER(DataSysStatusMessage, NasModule::handleDataSysStatusMessage),
    HANDLER(DsdSysStatusMessage, NasModule::handleDataDsdSysStatusMessage),
    HANDLER(DataRegistrationStateMessage, NasModule::handleDataRegistrationStateMessage),
    HANDLER(NewDDSInfoMessage, NasModule::handleDataNewDDSInfoMessage),
    HANDLER(DDSStatusFollowupMessage, NasModule::handleDataDDSStatusFollowupMessage),
#endif

    HANDLER(NasSetPsAttachDetachMessage, NasModule::handlePsAttachDetachMessage),
    HANDLER(NasGetSysInfoMessage, NasModule::handleNasGetSysInfoMessage),
  };
}

/* Follow RAII.
*/
NasModule::~NasModule() {}

/*
 * Module specific initialization that does not belong to RAII .
 */
void NasModule::init()
{
  Module::init();
  qcril_qmi_nas_pre_init();
  QCRIL_ANDROID_REGISTER_REQUEST(RIL_REQUEST_ALLOW_DATA, callback);
  QCRIL_ANDROID_REGISTER_REQUEST(RIL_REQUEST_SEND_DEVICE_STATE, callback);
  QCRIL_ANDROID_REGISTER_REQUEST(RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER, callback);
  QCRIL_ANDROID_REGISTER_REQUEST(RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC, callback);
  QCRIL_ANDROID_REGISTER_REQUEST(RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE, callback);
  QCRIL_ANDROID_REGISTER_REQUEST(RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE, callback);
  QCRIL_ANDROID_REGISTER_REQUEST(RIL_REQUEST_ENABLE_5G, callback);
  QCRIL_ANDROID_REGISTER_REQUEST(RIL_REQUEST_ENABLE_5G_ONLY, callback);
  QCRIL_ANDROID_REGISTER_REQUEST(RIL_REQUEST_DISABLE_5G, callback);
  QCRIL_ANDROID_REGISTER_REQUEST(RIL_REQUEST_QUERY_5G_STATUS, callback);
  QCRIL_ANDROID_REGISTER_REQUEST(RIL_REQUEST_QUERY_ENDC_DCNR, callback);
  QCRIL_ANDROID_REGISTER_REQUEST(RIL_REQUEST_QUERY_NR_BEARER_ALLOCATION, callback);
  QCRIL_ANDROID_REGISTER_REQUEST(RIL_REQUEST_QUERY_NR_SIGNAL_STRENGTH, callback);
  QCRIL_ANDROID_REGISTER_REQUEST(RIL_REQUEST_QUERY_UPPERLAYER_IND_INFO, callback);
  QCRIL_ANDROID_REGISTER_REQUEST(RIL_REQUEST_QUERY_5G_CONFIG_INFO, callback);
  QCRIL_ANDROID_REGISTER_REQUEST(RIL_REQUEST_START_NETWORK_SCAN, callback);
  QCRIL_ANDROID_REGISTER_REQUEST(RIL_REQUEST_STOP_NETWORK_SCAN, callback);
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_CDMA_SUBSCRIPTION, callback,
    std::make_shared<SingleDispatchRestriction>());
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_DATA_REGISTRATION_STATE, callback,
    std::make_shared<SingleDispatchRestrictionHandleDuplicate>());
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE, callback,
    std::make_shared<SingleDispatchRestriction>());
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_GET_DC_RT_INFO, callback,
    std::make_shared<SingleDispatchRestriction>());
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_GET_NEIGHBORING_CELL_IDS, callback,
    std::make_shared<SingleDispatchRestriction>());
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_GET_RADIO_CAPABILITY, callback,
    std::make_shared<SingleDispatchRestriction>());
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_VOICE_RADIO_TECH, callback,
    std::make_shared<SingleDispatchRestriction>());
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_OPERATOR, callback,
    std::make_shared<SingleDispatchRestrictionHandleDuplicate>());
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE, callback,
    std::make_shared<SingleDispatchRestriction>());
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE, callback,
    std::make_shared<SingleDispatchRestrictionHandleDuplicate>());
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_SCREEN_STATE, callback,
    std::make_shared<SingleDispatchRestriction>());
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_SET_BAND_MODE, callback,
    std::make_shared<SingleDispatchRestriction>());
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE, callback,
    std::make_shared<SingleDispatchRestriction>());
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE, callback,
    std::make_shared<SingleDispatchRestriction>());
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_SET_LOCATION_UPDATES, callback,
    std::make_shared<SingleDispatchRestriction>());
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_SET_UICC_SUBSCRIPTION, callback,
    std::make_shared<SingleDispatchRestriction>());
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_SHUTDOWN, callback,
    std::make_shared<SingleDispatchRestriction>());
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_SIGNAL_STRENGTH, callback,
    std::make_shared<SingleDispatchRestriction>());
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_VOICE_REGISTRATION_STATE, callback,
    std::make_shared<SingleDispatchRestrictionHandleDuplicate>());
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_QUERY_AVAILABLE_NETWORKS, callback,
    std::make_shared<SingleDispatchRestriction>());
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_CANCEL_QUERY_AVAILABLE_NETWORKS, callback,
    std::make_shared<SingleDispatchRestriction>());
  QCRIL_ANDROID_REGISTER_REQUEST_WITH_RESTRICTION(RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC, callback,
    std::make_shared<SingleDispatchRestriction>());

  std::vector<std::string> restMsg {NasSetNetworkSelectionManualRequestMessage::MESSAGE_NAME};
  PolicyManager::getInstance().setMessageRestriction(restMsg,
        std::make_shared<SingleDispatchRestriction>());

  std::vector<std::string> ring1{
    QCRIL_MAKE_ANDROID_REQ_MSG_NAME_EX(RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE),
    QCRIL_MAKE_ANDROID_REQ_MSG_NAME_EX(RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE),
    QCRIL_MAKE_ANDROID_REQ_MSG_NAME_EX(RIL_REQUEST_ENABLE_5G),
    QCRIL_MAKE_ANDROID_REQ_MSG_NAME_EX(RIL_REQUEST_ENABLE_5G_ONLY),
    QCRIL_MAKE_ANDROID_REQ_MSG_NAME_EX(RIL_REQUEST_DISABLE_5G),
    QCRIL_MAKE_ANDROID_REQ_MSG_NAME_EX(RIL_REQUEST_QUERY_5G_STATUS)
  };
  PolicyManager::getInstance().setMessageRestriction(ring1,
         std::make_shared<SingleDispatchRestriction>());

#ifndef QMI_RIL_UTF
  std::vector<std::string> ring2{SetInitialAttachApnRequestMessage::MESSAGE_NAME,
    QCRIL_MAKE_ANDROID_REQ_MSG_NAME_EX(RIL_REQUEST_ALLOW_DATA)};
  PolicyManager::getInstance().setMessageRestriction(ring2,
         std::make_shared<SingleDispatchRestriction>());
#endif

  std::vector<std::string> rejsimul{ QCRIL_MAKE_ANDROID_REQ_MSG_NAME_EX(RIL_REQUEST_QUERY_AVAILABLE_NETWORKS),
    QCRIL_MAKE_ANDROID_REQ_MSG_NAME_EX(RIL_REQUEST_SET_RADIO_CAPABILITY)};
  PolicyManager::getInstance().setMessageRestriction(rejsimul,
         std::make_shared<RejectSimultaneousDispatchRestriction>());
}

bool NasModule::isDmsEndpointReady() {
  return qcril_qmi_client_is_client_initialized(QCRIL_QMI_CLIENT_DMS);
}

/*
 * List of individual private handlers for the subscribed messages.
 */

void NasModule::handleUimCardStatusChangeIndMsg(std::shared_ptr<UimCardStatusChangeIndMsg> msg) {
  Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());

  if (isDmsEndpointReady()) {
    qcril_qmi_nas_trigger_bootup_optimization();
  }
}

void NasModule::handleDmsEndpointStatusIndMsg(std::shared_ptr<Message> msg) {
  auto dms_endpoint_status_ind_msg(std::static_pointer_cast<EndpointStatusIndMessage>(msg));
  Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());
  if (dms_endpoint_status_ind_msg->getState() == ModemEndPoint::State::OPERATIONAL) {
    qcril_qmi_nas_trigger_bootup_optimization();
  }
}

void NasModule::handleUimCardAppStatusIndMsg(std::shared_ptr<UimCardAppStatusIndMsg> msg) {
  qcril_request_return_type ret_ptr;

  Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());
  qcril_request_params_type params_ptr;
  std::memset(&params_ptr, 0, sizeof(params_ptr));
  params_ptr.data = static_cast<void *>(new char[sizeof(qcril_card_app_info_type)]);
  if(params_ptr.data)
  {
    std::memcpy(params_ptr.data, msg->get_app_info(), sizeof(qcril_card_app_info_type));
    params_ptr.datalen = sizeof(qcril_card_app_info_type);
    params_ptr.modem_id = QCRIL_DEFAULT_MODEM_ID;

    qcril_qmi_nas_event_app_status_update (&params_ptr, &ret_ptr);
  }
  else
  {
    Log::getInstance().d("[NasModule]: Memory allocation failure");
  }
}


void NasModule::handleUimCardStatusIndMsg(std::shared_ptr<UimCardStatusIndMsg> msg) {
  qcril_request_return_type ret_ptr;

  Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());
  qcril_request_params_type params_ptr;
  std::memset(&params_ptr, 0, sizeof(params_ptr));
  params_ptr.data = static_cast<void *>(new char[sizeof(qcril_card_info_type)]);
  if(params_ptr.data)
  {
    std::memcpy(params_ptr.data, msg->get_card_status(), sizeof(qcril_card_info_type));
    params_ptr.datalen = sizeof(qcril_card_info_type);
    params_ptr.modem_id = QCRIL_DEFAULT_MODEM_ID;

    qcril_qmi_nas_event_card_status_update (&params_ptr, &ret_ptr);
  }
  else
  {
    Log::getInstance().d("[NasModule]: Memory allocation failure");
  }
}


void NasModule::handleUimProvisioningStatusIndMsg(std::shared_ptr<UimProvisionStatusIndMsg> msg) {
  qcril_request_return_type ret_ptr;

  Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());
  qcril_request_params_type params_ptr;
  std::memset(&params_ptr, 0, sizeof(params_ptr));
  params_ptr.data = static_cast<void *>(new char[sizeof(qcril_provision_info_type)]);
  if(params_ptr.data)
  {
    std::memcpy(params_ptr.data, msg->get_provision_status_info(), sizeof(qcril_provision_info_type));
    params_ptr.datalen = sizeof(qcril_provision_info_type);
    params_ptr.modem_id = QCRIL_DEFAULT_MODEM_ID;

    if(msg->get_provision_type() == UIM_ACTIVATE_PROVISION_STATUS) {
        qcril_qmi_prov_subs_activate_followup (&params_ptr, &ret_ptr);
    } else {
        qcril_qmi_prov_subs_deactivate_followup (&params_ptr, &ret_ptr);
    }
  }
  else
  {
    Log::getInstance().d("[NasModule]: Memory allocation failure");
  }
}


void NasModule::handleNasQmiIndMessage(std::shared_ptr<Message> msg) {
  auto shared_indMsg(
        std::static_pointer_cast<QmiIndMessage>(msg));

  Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());
  QmiIndMsgDataStruct *indData = shared_indMsg->getData();
  if (indData != nullptr) {
    qcril_qmi_nas_unsolicited_indication_cb_helper(indData->msgId, indData->indData,
        indData->indSize);
  } else {
    Log::getInstance().d("Unexpected, null data from message");
  }
}

void NasModule::handleQmiAsyncRespMessage(std::shared_ptr<QmiAsyncResponseMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  QmiAsyncRespData *asyncResp = msg->getData();
  if (asyncResp != nullptr && asyncResp->cb != nullptr) {
    asyncResp->cb(asyncResp->msgId, asyncResp->respData, asyncResp->respDataSize,
        asyncResp->cbData, asyncResp->traspErr);
  } else {
    Log::getInstance().d("Unexpected, null data from message");
  }
}


void NasModule::handleVoiceRegistrationState(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    ThreadPoolManager::getInstance().scheduleExecution(
        qcril_qmi_nas_request_registration_state_wrapper, &shared_msg->get_params());
}


void NasModule::handleDataRegistrationState(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    ThreadPoolManager::getInstance().scheduleExecution(
        qcril_qmi_nas_request_data_registration_state_wrapper, &shared_msg->get_params());
}


void NasModule::handleOperator(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    ThreadPoolManager::getInstance().scheduleExecution(
        qcril_qmi_nas_request_operator_wrapper, &shared_msg->get_params());
}


void NasModule::handleAllowData(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;

    qcril_qmi_nas_request_allow_data(
        &shared_msg->get_params(), &ret_ptr);
}


void NasModule::handleCdmaSubscription(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;

    qcril_qmi_nas_request_cdma_subscription(
        &shared_msg->get_params(), &ret_ptr);
}


void NasModule::handleCdmaSetRoamingPreference(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;

    qcril_qmi_nas_request_set_roaming_preference(
        &shared_msg->get_params(), &ret_ptr);
}


void NasModule::handleSetLocationUpdates(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;

    qcril_qmi_nas_set_location_updates(
        &shared_msg->get_params(), &ret_ptr);
}


void NasModule::handleQueryAvailableNetworks(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;
    static constexpr int MESSAGE_EXPIRY_TIMEOUT{ 1000 * 190 }; //For perform_network_scan timeout should be greater than 3min, so setting as 190sec

    Dispatcher::getInstance().clearTimeoutForMessage(shared_msg);
    Dispatcher::getInstance().setTimeoutForMessage(shared_msg, MESSAGE_EXPIRY_TIMEOUT);
    qcril_qmi_nas_query_available_networks(
        &shared_msg->get_params(), &ret_ptr);
}

void NasModule::handleCancelQueryAvailableNetworks(std::shared_ptr<RilRequestMessage> msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());

    qcril_request_return_type ret_ptr;
    qcril_qmi_nas_cancel_query_available_networks(&msg->get_params(), &ret_ptr);
}

void NasModule::handleQueryAvailableBandMode(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;

    qcril_qmi_nas_query_available_band_mode(
        &shared_msg->get_params(), &ret_ptr);
}


void NasModule::handleSignalStrength(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;

    qcril_qmi_nas_request_signal_strength(
        &shared_msg->get_params(), &ret_ptr);
}


void NasModule::handleSetPreferredNetworkType(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;

    qcril_qmi_nas_request_set_preferred_network_type(
      &shared_msg->get_params(), &ret_ptr);
}


void NasModule::handleGetPreferredNetworkType(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;

    qcril_qmi_nas_request_get_preferred_network_type(
      &shared_msg->get_params(), &ret_ptr);
}

void NasModule::handleEnable5g(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;
    qcril_qmi_nas_request_enable_5g(&shared_msg->get_params(), &ret_ptr);
    Dispatcher::getInstance().informMessageCallbackFinished(shared_msg);
}

void NasModule::handleEnable5gOnly(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;
    qcril_qmi_nas_request_enable_5g_only(&shared_msg->get_params(), &ret_ptr);
    Dispatcher::getInstance().informMessageCallbackFinished(shared_msg);
}

void NasModule::handleDisable5g(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;
    qcril_qmi_nas_request_disable_5g(&shared_msg->get_params(), &ret_ptr);
    Dispatcher::getInstance().informMessageCallbackFinished(shared_msg);
}

void NasModule::handleQuery5gStatus(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;
    qcril_qmi_nas_request_query_5g_status(&shared_msg->get_params(), &ret_ptr);
    Dispatcher::getInstance().informMessageCallbackFinished(shared_msg);
}

void NasModule::handleQueryEndcDcnr(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;
    qcril_qmi_nas_request_query_endc_dcnr(&shared_msg->get_params(), &ret_ptr);
    Dispatcher::getInstance().informMessageCallbackFinished(shared_msg);
}

void NasModule::handleQueryNrBearerAllocation(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;
    qcril_qmi_nas_request_query_nr_bearer_allocation(&shared_msg->get_params(), &ret_ptr);
    Dispatcher::getInstance().informMessageCallbackFinished(shared_msg);
}

void NasModule::handleQueryNrSignalStrength(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;
    qcril_qmi_nas_request_nr_signal_strength(&shared_msg->get_params(), &ret_ptr);
    Dispatcher::getInstance().informMessageCallbackFinished(shared_msg);
}

void NasModule::handleQueryUpperLayerIndInfo(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;
    qcril_qmi_nas_request_query_upperlayer_ind_info(&shared_msg->get_params(), &ret_ptr);
    Dispatcher::getInstance().informMessageCallbackFinished(shared_msg);
}

void NasModule::handleQuery5gConfigInfo(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;
    qcril_qmi_nas_request_query_5g_config_info(&shared_msg->get_params(), &ret_ptr);
    Dispatcher::getInstance().informMessageCallbackFinished(shared_msg);
}

void NasModule::handleSetCdmaSubscription(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;

    qcril_qmi_nas_request_cdma_subscription(
      &shared_msg->get_params(), &ret_ptr);
}

void NasModule::handleSetCdmaSubscriptionSource(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;

    qcril_qmi_nas_set_subscription_source(
      &shared_msg->get_params(), &ret_ptr);
}


void NasModule::handleExitEmergencyCallbackMode(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;

    qcril_qmi_nas_exit_emergency_callback_mode(
      &shared_msg->get_params(), &ret_ptr);
}


void NasModule::handleGetVoiceRadioTech(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;

    qcril_qmi_nas_request_radio_tech(
      &shared_msg->get_params(), &ret_ptr);
}


void NasModule::handleSetUiccSubscription(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;

    qcril_qmi_nas_set_uicc_subscription(
      &shared_msg->get_params(), &ret_ptr);
}


void NasModule::handleGetDcRtInfo(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;

    qcril_qmi_nas_request_get_dc_rt_info(
      &shared_msg->get_params(), &ret_ptr);
}


void NasModule::handleShutdown(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;

    qcril_qmi_nas_request_shutdown(
      &shared_msg->get_params(), &ret_ptr);
}


void NasModule::handleGetRadioCapability(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;

    qcril_qmi_nas_get_radio_capability(
      &shared_msg->get_params(), &ret_ptr);
}


void NasModule::handleSetRadioCapability(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
    qcril_request_return_type ret_ptr;

    qcril_qmi_nas_set_radio_capability(
      &shared_msg->get_params(), &ret_ptr);
}


void NasModule::handleQueryNetworkSelectionMode(std::shared_ptr<RilRequestMessage> shared_msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());

    ThreadPoolManager::getInstance().scheduleExecution(
        qcril_qmi_nas_query_network_selection_mode_wrapper, &shared_msg->get_params());
}


void NasModule::handleSetNetworkSelectionAutomatic(std::shared_ptr<RilRequestMessage> shared_msg) {
  Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
  qcril_request_return_type ret_ptr;

  qcril_qmi_nas_set_network_selection_automatic(
      &shared_msg->get_params(), &ret_ptr);
}


void NasModule::handleSetNetworkSelectionManual(
    std::shared_ptr<NasSetNetworkSelectionManualRequestMessage> shared_msg) {
  Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
  qcril_request_return_type ret_ptr;

  qcril_qmi_nas_set_network_selection_manual(
      shared_msg->get_params(), &ret_ptr);
}


void NasModule::handleSetBandMode(std::shared_ptr<RilRequestMessage> shared_msg) {
  Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
  qcril_request_return_type ret_ptr;

  qcril_qmi_nas_set_band_mode(
      &shared_msg->get_params(), &ret_ptr);
}


void NasModule::handleGetNeighboringCellIds(std::shared_ptr<RilRequestMessage> shared_msg) {
  Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
  qcril_request_return_type ret_ptr;

  qcril_qmi_nas_get_neighboring_cell_ids(
      &shared_msg->get_params(), &ret_ptr);
}


void NasModule::handleSetScreenState(std::shared_ptr<RilRequestMessage> shared_msg) {
  Log::getInstance().d("[NasModule]: Handling msg = " + shared_msg->dump());
  qcril_request_return_type ret_ptr;

  qcril_qmi_nas_request_screen_state(
      &shared_msg->get_params(), &ret_ptr);
}

void NasModule::handleSetDeviceState(std::shared_ptr<RilRequestMessage> msg) {
  Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());

  qcril_request_return_type ret_ptr;
  qcril_qmi_nas_request_send_device_state(
      &msg->get_params(), &ret_ptr);
}

void NasModule::handleSetUnsolRespFilter(std::shared_ptr<RilRequestMessage> msg) {
  Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());

  qcril_request_return_type ret_ptr;
  qcril_qmi_nas_request_set_unsol_response_filter(
      &msg->get_params(), &ret_ptr);
}

void NasModule::handleStartNetworkScan(std::shared_ptr<RilRequestMessage> msg) {
  Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());
  qcril_request_return_type ret_ptr;

  qcril_qmi_nas_start_advanced_scan(
      &msg->get_params(), &ret_ptr);
}

void NasModule::handleStopNetworkScan(std::shared_ptr<RilRequestMessage> msg) {
    Log::getInstance().d("[NasModule]: Handling msg = " + msg->dump());
    qcril_request_return_type ret_ptr;

    qcril_qmi_nas_cancel_query_available_networks(
      &msg->get_params(), &ret_ptr);
}

void NasModule::handleQcrilInit(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  /* Init QMI services.*/
  QmiNasSetupRequestCallback qmiNasSetupCallback("NasModule-Token");
  ModemEndPointFactory<NasModemEndPoint>::getInstance().buildEndPoint()->requestSetup("NasModule-Token", &qmiNasSetupCallback);
}


void NasModule::handleNasEndpointStatusIndMessage(std::shared_ptr<Message> msg) {
  auto shared_indMsg(std::static_pointer_cast<EndpointStatusIndMessage>(msg));

  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  if (shared_indMsg->getState() == ModemEndPoint::State::OPERATIONAL) {
     if (!mReady) {
       mReady = true;
       qcril_qmi_nas_init();
     }
  } else {
    // state is not operational
    mReady = false;
    qcril_qmi_nas_cleanup();
    chaninfo.bw_info_valid = false;
    chaninfo.pcell_info_valid = false;
    chaninfo.scell_info_valid = false;
    chaninfo.unchanged_scell_info_list_valid = false;

    //reset am states
    #ifndef QMI_RIL_UTF
      qcril_am_state_reset();
    #endif
  }
}

// IMS handling
void NasModule::handleImsServiceStatusInd(std::shared_ptr<ImsServiceStatusInd> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  qcril_qmi_nas_handle_ims_service_status_change(msg->getServiceStatusInfo());
}

void NasModule::handleImsWfcStatusInd(std::shared_ptr<ImsWfcSettingsStatusInd> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  qcril_qmi_nas_handle_ims_wfc_status_change(msg->getStatusInfo());
}

void NasModule::handleImsPdpStatusInd(std::shared_ptr<ImsPdpStatusInd> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  qcril_qmi_nas_handle_ims_pdp_status_change(msg->isPdpConnected());
}

void NasModule::handleSetVoiceDomainPreferenceRequest(std::shared_ptr<NasSetVoiceDomainPreferenceRequest> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  qcril_qmi_nas_set_voice_domain_preference(msg);
}

/* Handlers for Messages from Data */
void NasModule::handleEmbmsEnableDataConMessage(std::shared_ptr<NasEmbmsEnableDataConMessage> msg)
{
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  qcril_qmi_nas_embms_enable_data_con(&(msg->get_payload()));
  msg->sendResponse(msg, Message::Callback::Status::SUCCESS, nullptr);
}
void NasModule::handleEmbmsDisableDataMessage(std::shared_ptr<NasEmbmsDisableDataMessage> msg)
{
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  qcril_qmi_nas_embms_disable_data_ind(&(msg->get_payload()));
  msg->sendResponse(msg, Message::Callback::Status::SUCCESS, nullptr);
}

#ifndef QMI_RIL_UTF
/* Data Indication Handlers */
void NasModule::handleDataCallStatusMessage(std::shared_ptr<CallStatusMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  const rildata::CallStatusParams data = msg->getCallParams();
  NasDataCache::getInstance().setNumDataCalls(data.num_calls);

  //CALL FUNCTION IN QCRIL to notify this indication.
  qcril_qmi_nas_handle_data_call_status_notification(map_data_call_event_to_internal(data.evt));
}
void NasModule::handleDataSetPrefDataTechMessage(std::shared_ptr<SetPrefDataTechMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  //NasDataCache::getInstance().setsetPreferredDataTech(indMsg->get());

  //Call Function in QCRIL to notify this indication
  qcril_arb_set_pref_data_tech(map_data_pref_data_tech_to_arb_pref_data_tech(msg->get()));
}
void NasModule::handleDataSysStatusMessage(std::shared_ptr<DataSysStatusMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  //TODO: Revisit this
  qcril_arb_set_data_sys_status(&(msg->getParams()));
}
void NasModule::handleDataDsdSysStatusMessage(std::shared_ptr<DsdSysStatusMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  //TODO: Revisit this
  qcril_arb_set_dsd_sys_status(&(msg->getParams()));
}

void NasModule::handleDataRegistrationStateMessage(std::shared_ptr<DataRegistrationStateMessage> msg) {
  // qcril-data will not be sending DSD indication if 5G rat available and when 5G is OOS.
  // This logic will make sure that in same file, we process 5G and legacy rats.
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (msg->getRatValue() != rildata::Rat::UNKNOWN) {
    dsd_system_status_ind_msg_v01 dsd_ind_msg;
    memset(&dsd_ind_msg, 0, sizeof(dsd_ind_msg));
    dsd_ind_msg.avail_sys_valid = 1;
    dsd_ind_msg.avail_sys_len = 2;
    dsd_ind_msg.avail_sys[0].technology = DSD_SYS_NETWORK_3GPP_V01;
    dsd_ind_msg.avail_sys[0].rat_value = DSD_SYS_RAT_EX_3GPP_5G_V01;
    if (msg->getRatValue() == rildata::Rat::NR_NSA) {
       dsd_ind_msg.avail_sys[0].so_mask = QMI_DSD_3GPP_SO_MASK_5G_NSA_V01;
    } else if (msg->getRatValue() == rildata::Rat::NR_SA) {
       dsd_ind_msg.avail_sys[0].so_mask = QMI_DSD_3GPP_SO_MASK_5G_SA_V01;
    } else if (msg->getRatValue() == rildata::Rat::NR_NSA_MMWV) {
       dsd_ind_msg.avail_sys[0].so_mask = QMI_DSD_3GPP_SO_MASK_5G_NSA_V01 |
                                          QMI_DSD_3GPP_SO_MASK_5G_MMWAVE_V01;
    } else if (msg->getRatValue() == rildata::Rat::NR_SA_MMWV) {
       dsd_ind_msg.avail_sys[0].so_mask = QMI_DSD_3GPP_SO_MASK_5G_SA_V01 |
                                          QMI_DSD_3GPP_SO_MASK_5G_MMWAVE_V01;
    }

    dsd_ind_msg.avail_sys[1].technology = DSD_SYS_NETWORK_3GPP_V01;
    dsd_ind_msg.avail_sys[1].rat_value = DSD_SYS_RAT_EX_3GPP_LTE_V01;
    qcril_arb_set_dsd_sys_status(&dsd_ind_msg);
  }
}

void NasModule::handleDataNewDDSInfoMessage(std::shared_ptr<rildata::NewDDSInfoMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  qcril_qmi_nas_data_event_new_dds_info(msg->getDDS());
}

void NasModule::handleDataDDSStatusFollowupMessage(std::shared_ptr<rildata::DDSStatusFollowupMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  DDSStatusInfo status_info = msg->getDDSStatusInfo();
  qcril_qmi_nas_data_event_dds_status_follow_up(map_dds_status_info_from_data(status_info));
}

dds_status_info map_dds_status_info_from_data(DDSStatusInfo status_info)
{
  dds_status_info ret;
  memset(&ret, 0, sizeof(ret));

  ret.result = status_info.result;
  ret.error_cause = status_info.error_cause;

  return ret;
}

qcril_arb_pref_data_tech_e_type map_data_pref_data_tech_to_arb_pref_data_tech(qcril_data_pref_data_tech_e_type pref_data_tech) {
  qcril_arb_pref_data_tech_e_type ret = QCRIL_ARB_PREF_DATA_TECH_INVALID;
  switch (pref_data_tech) {
  case QCRIL_DATA_PREF_DATA_TECH_UNKNOWN:
      ret = QCRIL_ARB_PREF_DATA_TECH_UNKNOWN;
      break;
  case QCRIL_DATA_PREF_DATA_TECH_CDMA:
      ret = QCRIL_ARB_PREF_DATA_TECH_CDMA;
      break;
  case QCRIL_DATA_PREF_DATA_TECH_EVDO:
      ret = QCRIL_ARB_PREF_DATA_TECH_EVDO;
      break;
  case QCRIL_DATA_PREF_DATA_TECH_GSM:
      ret = QCRIL_ARB_PREF_DATA_TECH_GSM;
      break;
  case QCRIL_DATA_PREF_DATA_TECH_UMTS:
      ret = QCRIL_ARB_PREF_DATA_TECH_UMTS;
      break;
  case QCRIL_DATA_PREF_DATA_TECH_EHRPD:
      ret = QCRIL_ARB_PREF_DATA_TECH_EHRPD;
      break;
  case QCRIL_DATA_PREF_DATA_TECH_LTE:
      ret = QCRIL_ARB_PREF_DATA_TECH_LTE;
      break;
  case QCRIL_DATA_PREF_DATA_TECH_TDSCDMA:
      ret = QCRIL_ARB_PREF_DATA_TECH_TDSCDMA;
      break;
  default:
      ret = QCRIL_ARB_PREF_DATA_TECH_INVALID;
      break;
  }
  return ret;
}
static qcril_qmi_nas_data_call_event_type map_data_call_event_to_internal(qcril_data_net_evt_t evt) {
  qcril_qmi_nas_data_call_event_type ret = QCRIL_QMI_NAS_DATA_EVT_UNKNOWN;
  switch(evt) {
  case QCRIL_DATA_EVT_CALL_CONNECTED:
     ret = QCRIL_QMI_NAS_DATA_EVT_CALL_CONNECTED;
     break;
  case QCRIL_DATA_EVT_CALL_PHYSLINK_UP:
     ret = QCRIL_QMI_NAS_DATA_EVT_CALL_PHYSLINK_UP;
     break;
  case QCRIL_DATA_EVT_CALL_PHYSLINK_DOWN:
     ret = QCRIL_QMI_NAS_DATA_EVT_CALL_PHYSLINK_DOWN;
     break;
  case QCRIL_DATA_EVT_CALL_RELEASED:
     ret = QCRIL_QMI_NAS_DATA_EVT_CALL_RELEASED;
     break;
  default:
     ret = QCRIL_QMI_NAS_DATA_EVT_UNKNOWN;
     break;
  }
  return ret;
}
#endif

void NasModule::handlePsAttachDetachMessage(std::shared_ptr<NasSetPsAttachDetachMessage> msg) {
  nas_srv_domain_pref_enum_type_v01 pref = NAS_SRV_DOMAIN_PREF_ENUM_TYPE_MIN_ENUM_VAL_V01;
  RIL_Errno res = RIL_E_INVALID_ARGUMENTS;

  switch (msg->getAction()) {
  case NasSetPsAttachDetachMessage::PsAttachAction::PS_ATTACH:
    pref = QMI_SRV_DOMAIN_PREF_PS_ATTACH_V01;
    break;
  case NasSetPsAttachDetachMessage::PsAttachAction::PS_DETACH:
    pref = QMI_SRV_DOMAIN_PREF_PS_DETACH_V01;
    break;
  case NasSetPsAttachDetachMessage::PsAttachAction::PS_ATTACH_ON_DEMAND:
    pref = QMI_SRV_DOMAIN_PREF_ON_DEMAND_PS_ATTACH_V01;
    break;
  case NasSetPsAttachDetachMessage::PsAttachAction::PS_ATTACH_FORCED:
    pref = QMI_SRV_DOMAIN_PREF_FORCE_PS_DETACH_V01;
    break;
  default:
    break;
  }
  if (pref != NAS_SRV_DOMAIN_PREF_ENUM_TYPE_MIN_ENUM_VAL_V01) {
    res = qcril_qmi_nas_send_attach_detach_request(pref);
  }

  msg->sendResponse(msg, Message::Callback::Status::SUCCESS,
        std::make_shared<RIL_Errno>(res));
}

void NasModule::handleNasGetSysInfoMessage(std::shared_ptr<NasGetSysInfoMessage> msg) {

  QCRIL_LOG_FUNC_ENTRY();

  nas_get_sys_info_resp_msg_v01     sys_info_list;
  memset(&sys_info_list, 0, sizeof(nas_get_sys_info_resp_msg_v01));
  qcril_qmi_nas_get_sys_info(&sys_info_list);

  QCRIL_LOG_DEBUG("Received NasGetSysInfoMessage Msg. Executing handleNasGetSysInfoMessage  ");

  QCRIL_LOG_DEBUG("sys info list - service valid=%d & service domain=%d", sys_info_list.lte_sys_info.common_sys_info.srv_domain_valid,sys_info_list.lte_sys_info.common_sys_info.srv_domain);
  msg->sendResponse(msg, Message::Callback::Status::SUCCESS,
                               std::make_shared<nas_get_sys_info_resp_msg_v01>(sys_info_list));
  QCRIL_LOG_FUNC_RETURN();
}

#ifdef QMI_RIL_UTF
void NasModule::qcrilHalNasModuleCleanup()
{
    std::shared_ptr<NasModemEndPoint> mNasModemEndPoint =
                ModemEndPointFactory<NasModemEndPoint>::getInstance().buildEndPoint();
    NasModemEndPointModule* mNasModemEndPointModule =
               (NasModemEndPointModule*)mNasModemEndPoint->mModule;
    mNasModemEndPointModule->cleanUpQmiSvcClient();
    mReady = false;
}
#endif

int32_t toKhz(nas_bandwidth_enum_type_v01 bw) {
    int32_t ret = INT_MAX;
    switch(bw) {
        case NAS_LTE_BW_NRB_6_V01:
            ret = 1400;
            break;
        case NAS_LTE_BW_NRB_15_V01:
            ret = 3000;
            break;
        case NAS_LTE_BW_NRB_25_V01:
            ret = 5000;
            break;
        case NAS_LTE_BW_NRB_50_V01:
            ret = 10000;
            break;
        case NAS_LTE_BW_NRB_75_V01:
            ret = 15000;
            break;
        case NAS_LTE_BW_NRB_100_V01:
            ret = 20000;
            break;
        default:
            QCRIL_LOG_ERROR("Unknown bw value");
    }
    return ret;
}

int32_t toKhz(nas_lte_cphy_ca_bandwidth_enum_v01 bw) {

    switch(bw) {
        case NAS_LTE_CPHY_CA_BW_NRB_6_V01  : return 1400;  // 1.4 MHz bandwidth
        case NAS_LTE_CPHY_CA_BW_NRB_15_V01 : return 3000;  // 3 MHz bandwidth
        case NAS_LTE_CPHY_CA_BW_NRB_25_V01 : return 5000;  // 5 MHz bandwidth
        case NAS_LTE_CPHY_CA_BW_NRB_50_V01 : return 10000; // 10 MHz bandwidth
        case NAS_LTE_CPHY_CA_BW_NRB_75_V01 : return 15000; // 15 MHz bandwidth
        case NAS_LTE_CPHY_CA_BW_NRB_100_V01: return 20000; // 20 MHz bandwidth
        default:
            QCRIL_LOG_ERROR("Unknown bw value");
            return INT_MAX;
    }
}

void NasModule::handleSetSignalStrengthCriteriaSupported(
        std::shared_ptr<NasSetSignalStrengthCriteriaSupported> msg)
{
    mSetSignalStrengthCriteriaSupported = msg->isSupported();
}

bool NasModule::shouldReportScell(const nas_lte_cphy_scell_info_type_v01 &scell_info) {
    bool ret = false;
    QCRIL_LOG_FUNC_ENTRY();

    if (scell_info.scell_state == NAS_LTE_CPHY_SCELL_STATE_CONFIGURED_ACTIVATED_V01 ||
        scell_info.scell_state == NAS_LTE_CPHY_SCELL_STATE_CONFIGURED_DEACTIVATED_V01) {
        ret = true;
    }

    return ret;
    QCRIL_LOG_FUNC_RETURN();
}

void NasModule::reportCurrentPhysChanConfig() {
    QCRIL_LOG_FUNC_ENTRY();
    auto msg = std::make_shared<NasPhysChanConfigMessage>();

    if (!msg) {
        QCRIL_LOG_FUNC_RETURN("Unable to allocate msg");
        return;
    }

    bool valid = false;

    int32_t bandwidth = getLteBw();
    if ( bandwidth != INT_MAX ) {
        msg->addInfo(
                NasPhysChanInfo{
                    NasPhysChanInfo::Status::PRIMARY,
                    bandwidth});
        valid = true;
    }

    if ( chaninfo.scell_info_valid &&
            shouldReportScell(chaninfo.scell_info)) {
        msg->addInfo(
                NasPhysChanInfo{
                    NasPhysChanInfo::Status::SECONDARY,
                    toKhz(chaninfo.scell_info.cphy_ca_dl_bandwidth)});
        valid = true;
    }

    if ( chaninfo.unchanged_scell_info_list_valid ) {
        for (size_t i = 0; i < chaninfo.unchanged_scell_info_list_len; i++ ) {
            if (shouldReportScell(chaninfo.unchanged_scell_info_list[i].scell_info)) {
                valid = true;
                msg->addInfo(
                        NasPhysChanInfo{
                            NasPhysChanInfo::Status::SECONDARY,
                            toKhz(chaninfo.unchanged_scell_info_list[i].scell_info.cphy_ca_dl_bandwidth)});
            }
        }
    }

    if ( valid) {
        msg->broadcast();
    }
    QCRIL_LOG_FUNC_RETURN();
}

int32_t NasModule::getLteBw() {
    int32_t ret = INT_MAX;
    bool p_found = false;
    if ( chaninfo.pcell_info_valid) {
        ret = toKhz(chaninfo.pcell_info.cphy_ca_dl_bandwidth);
    }
    if( !p_found && chaninfo.bw_info_valid ) {
        ret = toKhz(chaninfo.bw_info.bandwidth);
    }
    QCRIL_LOG_ESSENTIAL("returning bandwidth: %d", ret);
    return ret;
}

void NasModule::resetPhysChanConfig()
{
    QCRIL_LOG_FUNC_ENTRY();

    bool changed = false;

    if (chaninfo.pcell_info_valid) {
        changed = true;
    }

    if (chaninfo.bw_info_valid) {
        changed = true;
    }

    if (chaninfo.scell_info_valid) {
        changed = true;
    }

    if (chaninfo.unchanged_scell_info_list_valid) {
        changed = true;
    }
    chaninfo = {};

    if (changed) {
        auto msg = std::make_shared<NasPhysChanConfigMessage>();
        if (msg) {
            msg->broadcast();
        }
    }
    QCRIL_LOG_FUNC_RETURN();
}

void NasModule::handleQmiNasRfBandInfoMessage(
        std::shared_ptr<QmiNasRfBandInfoMessage> bandInfoMsg)
{
    QCRIL_LOG_FUNC_ENTRY();
    if (!bandInfoMsg) {
        QCRIL_LOG_FUNC_RETURN("No message");
        return;
    }

    const auto &unsol = std::static_pointer_cast<const QmiNasRfBandInfoMessage>(bandInfoMsg)->getPayload();
    QCRIL_LOG_DEBUG("Bandwidth Info %svalid", unsol.nas_rf_bandwidth_info_valid ? "" : "NOT ");
    if (unsol.nas_rf_bandwidth_info_valid) {
        bool changed = memcmp(&chaninfo.bw_info, &unsol.nas_rf_bandwidth_info, sizeof(unsol.nas_rf_bandwidth_info)) != 0;
        chaninfo.bw_info = unsol.nas_rf_bandwidth_info;
        chaninfo.bw_info_valid = unsol.nas_rf_bandwidth_info_valid;
        if (changed) {
            reportCurrentPhysChanConfig();
        }
    }


    QCRIL_LOG_FUNC_RETURN();
}

void NasModule::handleQmiNasLteCphyCaIndMessage(
        std::shared_ptr<QmiNasLteCphyCaIndMessage> lteCphyMsg)
{
    QCRIL_LOG_FUNC_ENTRY();
    bool changed = false;

    if (!lteCphyMsg) {
        QCRIL_LOG_FUNC_RETURN("No message");
        return;
    }

    auto &unsol = std::static_pointer_cast<const QmiNasLteCphyCaIndMessage>(lteCphyMsg)->getPayload();

    if (unsol.pcell_info_valid) {
        changed = changed || memcmp(&unsol.pcell_info, &chaninfo.pcell_info, sizeof(chaninfo.pcell_info)) != 0;
        chaninfo.pcell_info_valid = unsol.pcell_info_valid;
        chaninfo.pcell_info = unsol.pcell_info;
    }

    if (unsol.scell_info_valid) {
        changed = changed || memcmp(&unsol.scell_info, &chaninfo.scell_info, sizeof(chaninfo.scell_info)) != 0;
        chaninfo.scell_info_valid = unsol.scell_info_valid;
        chaninfo.scell_info = unsol.scell_info;
    }

    if (unsol.unchanged_scell_info_list_valid) {
        changed = changed || unsol.unchanged_scell_info_list_len != chaninfo.unchanged_scell_info_list_len;
        changed = changed || memcmp(&unsol.unchanged_scell_info_list,
                &chaninfo.unchanged_scell_info_list,
                sizeof(unsol.unchanged_scell_info_list)) != 0;

        chaninfo.unchanged_scell_info_list_valid = unsol.unchanged_scell_info_list_valid;
        chaninfo.unchanged_scell_info_list_len = unsol.unchanged_scell_info_list_len;
        int i = 0;
        for (auto e: unsol.unchanged_scell_info_list) {
            chaninfo.unchanged_scell_info_list[i++] = e;
        }
    }

    if (changed) {
        reportCurrentPhysChanConfig();
    }
    QCRIL_LOG_FUNC_RETURN();
}

void NasModule::handleEnablePhysChanConfigReporting(
        std::shared_ptr<NasEnablePhysChanConfigReporting> enableCfgMsg)
{
    if (!enableCfgMsg) {
        QCRIL_LOG_ERROR("No message");
        return;
    }
    auto ctx = enableCfgMsg->getContext();
    bool enable = enableCfgMsg->isEnable();
    auto nasModemEndPoint = ModemEndPointFactory<NasModemEndPoint>::getInstance().buildEndPoint();

    if (nasModemEndPoint) {
        nasModemEndPoint->registerForIndications(
                { { QMI_NAS_RF_BAND_INFO_IND_V01, enable },
                  { QMI_NAS_LTE_CPHY_CA_IND_V01,  enable },
                },
                ctx,
                [enableCfgMsg, ctx]
                (std::shared_ptr<Message> msg,
                    Message::Callback::Status status,
                    std::shared_ptr<QmiNasRegisterForIndResponse> rsp)
                {
                        QCRIL_NOTUSED(rsp);
                        auto lmsg = std::static_pointer_cast<QmiNasRegisterForIndRequest>(msg);
                        if (lmsg) {
                            QCRIL_LOG_INFO("Status: %d", static_cast<int>(status));
                                        NasSettingResultCode rc = NasSettingResultCode::FAILURE;
                                        RIL_Errno e = RIL_E_INTERNAL_ERR;
                                        if (status == Message::Callback::Status::SUCCESS &&
                                                rsp) {
                                            e =
                                                qcril_qmi_util_convert_qmi_response_codes_to_ril_result(
                                                        rsp->err,
                                                        &rsp->payload.resp
                                                        );
                                            if (e == RIL_E_SUCCESS) {
                                                rc = NasSettingResultCode::SUCCESS;
                                            }
                                        }
                                        QCRIL_LOG_INFO("Status: %d RIL_Errno: %d",
                                                static_cast<int>(status),
                                                e);
                                        enableCfgMsg->sendResponse(enableCfgMsg,
                                                status,
                                                std::make_shared<NasSettingResultCode>(rc));
                        }
                });
    }
}
