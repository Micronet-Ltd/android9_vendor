/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once

#include <utility>
#include <vector>
#include <framework/Log.h>
#include <modules/qmi/ModemEndPoint.h>
#include <modules/qmi/NasModemEndPointModule.h>
#include <network_access_service_v01.h>

std::string toString(nas_radio_if_enum_v01 radio_if);
std::string toString(nas_active_band_enum_v01 active_band);

class QmiNasRfBandInfoMessage: public QmiIndicationMessage<QmiNasRfBandInfoMessage, nas_rf_band_info_ind_msg_v01>,
                               public add_message_id<QmiNasRfBandInfoMessage>
{
    public:
    static const constexpr char *MESSAGE_NAME = "com.qualcomm.qti.qcril.qmi.QmiNasRfBandInfoMessage";
    using payload_t = nas_rf_band_info_ind_msg_v01;

    QmiNasRfBandInfoMessage() = delete;
    ~QmiNasRfBandInfoMessage() {}

    QmiNasRfBandInfoMessage(const nas_rf_band_info_ind_msg_v01 &band_info):
        QmiIndicationMessage(band_info) {
        // For testing only
        //auto &payload = getPayload();
        //payload.nas_rf_bandwidth_info_valid = true;
        //payload.nas_rf_bandwidth_info.bandwidth = NAS_LTE_BW_NRB_100_V01;
    }
    std::string dump() {
        return std::string("QmiNasRfBandInfoMessage[if: ") + toString(getPayload().rf_band_info.radio_if) +
            " active_band: " + toString(getPayload().rf_band_info.active_band) +
            " active_channel: " + std::to_string(getPayload().rf_band_info.active_channel) + "]";

    }
    std::shared_ptr<UnSolicitedMessage> clone() {
        return std::make_shared<QmiNasRfBandInfoMessage>(getPayload());
    }


};

class QmiNasLteCphyCaIndMessage: public QmiIndicationMessage<QmiNasLteCphyCaIndMessage, nas_lte_cphy_ca_ind_msg_v01>,
                                 public add_message_id<QmiNasLteCphyCaIndMessage>
{
    public:
    static const constexpr char *MESSAGE_NAME = "com.qualcomm.qti.qcril.qmi.QmiNasLteCphyCaIndMessage";
    using payload_t = nas_lte_cphy_ca_ind_msg_v01;

    QmiNasLteCphyCaIndMessage() = delete;
    ~QmiNasLteCphyCaIndMessage() {}

    QmiNasLteCphyCaIndMessage(const nas_lte_cphy_ca_ind_msg_v01 &lte_cphy_ca):
        QmiIndicationMessage(lte_cphy_ca) {
            // For testing only
            // auto &payload = getPayload();
            // payload.scell_info_valid = true;
            // payload.scell_info.cphy_ca_dl_bandwidth = NAS_LTE_CPHY_CA_BW_NRB_6_V01;
        }

    std::string dump() {
        return std::string("QmiNasLteCphyCaIndMessage{ ") +
            "pci: " + std::to_string(getPayload().cphy_ca.pci) +
            "freq: " + std::to_string(getPayload().cphy_ca.freq) +
            "scell_state: " + std::to_string(getPayload().cphy_ca.scell_state) +
            (getPayload().cphy_ca_dl_bandwidth_valid ? "[CPHY_CA_DL_BANDWIDTH]" : "" ) +
            (getPayload().scell_info_valid ? "[SCELL_INFO]" : "" ) +
            (getPayload().pcell_info_valid ? "[PCELL_INFO]" : "" ) +
            (getPayload().scell_idx_valid ? "[SCELL_IDX]" : "" ) +
            (getPayload().cphy_ca_aggregated_dl_bandwidth_valid ?
                        "[CPHY_CA_AGGREGATED_DL_BANDWIDTH]":"" ) +
            (getPayload().num_scells_configured_valid ? "[NUM_SCELLS_CONFIGURED]" : "" ) +
            (getPayload().unchanged_scell_info_list_valid ? "[UNCHANGED_SCELL_INFO_LIST]" : "" ) +
            " }";

    }
    std::shared_ptr<UnSolicitedMessage> clone() {
        return std::make_shared<QmiNasLteCphyCaIndMessage>(getPayload());
    }


};

using QmiNasRegisterForIndResponse = QmiResponse<nas_indication_register_resp_msg_v01>;

class QmiNasRegisterForIndRequest: public SolicitedMessage<QmiNasRegisterForIndResponse>,
    public add_message_id<QmiNasRegisterForIndRequest>
{
    public:
        static const constexpr char *MESSAGE_NAME = "com.qualcomm.qti.qcril.qmi.QmiNasRegisterForIndRequest";
        using cb_t = GenericCallback<QmiNasRegisterForIndResponse>::cb_t;
        using IndicationList = std::vector<std::pair<unsigned,bool>>;
        QmiNasRegisterForIndRequest() = delete;
        ~QmiNasRegisterForIndRequest() {}

        QmiNasRegisterForIndRequest(const IndicationList &indication_ids, cb_t callbackfn):
            SolicitedMessage<QmiNasRegisterForIndResponse>(get_class_message_id()),
            mIndicationIds(indication_ids) {
                mName = MESSAGE_NAME;
                GenericCallback<QmiNasRegisterForIndResponse> cb(callbackfn);
                setCallback(&cb);
            }

        QmiNasRegisterForIndRequest(IndicationList &&indication_ids, cb_t callbackfn):
            SolicitedMessage<QmiNasRegisterForIndResponse>(get_class_message_id()),
            mIndicationIds(std::move(indication_ids)) {
                mName = MESSAGE_NAME;
                GenericCallback<QmiNasRegisterForIndResponse> cb(callbackfn);
                setCallback(&cb);
            }

        std::string dump() {
            return mName;
        }

        IndicationList getIndicationIds() { return mIndicationIds; }
    private:
        IndicationList mIndicationIds;
};

class NasModemEndPoint : public ModemEndPoint {
  public:
    static constexpr const char *NAME = "NAS";
    NasModemEndPoint() : ModemEndPoint(NAME) {
      mModule = new NasModemEndPointModule("NasModemEndPointModule", *this);
      mModule->init();
    }
    ~NasModemEndPoint() {
      Log::getInstance().d("[NasModemEndPoint]: destructor");
      delete mModule;
      mModule = nullptr;
    }

    void requestSetup(string clientToken, GenericCallback<string>* cb);
    void registerForRfBandInfoInd(bool enable, std::shared_ptr<MessageContext> ctx, QmiNasRegisterForIndRequest::cb_t cb);
    void registerForIndications(QmiNasRegisterForIndRequest::IndicationList list,
            std::shared_ptr<MessageContext> ctx, QmiNasRegisterForIndRequest::cb_t cb);
};
