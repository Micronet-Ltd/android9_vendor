/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "PmcSetConfigHandler.h"
#include "PmcService.h"
#include "HostManagerDefinitions.h"
#include "PmcSequence.h"
#include "PmcSetConfigHandlerSequences.h"

void PmcSetConfigHandler::HandleRequest(const PmcSetConfigRequest& jsonRequest, PmcSetConfigResponse& jsonResponse)
{
    const std::string& deviceName = jsonRequest.GetDeviceName();
    LOG_DEBUG << "PMC Set Configuration Handler." << std::endl;
    BasebandType type;
    auto nameTypeValidRes = PmcSequence::ValidateDeviceNameAndType(deviceName, type);
    if (!nameTypeValidRes.IsSuccess())
    {
        jsonResponse.Fail(nameTypeValidRes.GetStatusMessage());
        return;
    }

    JsonValueBoxed<bool> setCollectIdleSm = jsonRequest.GetCollectIdleSmEvents();
    JsonValueBoxed<bool> setCollectRxPpdu = jsonRequest.GetCollectRxPpduEvents();
    JsonValueBoxed<bool> setCollectTxPpdu = jsonRequest.GetCollectTxPpduEvents();
    JsonValueBoxed<bool> setCollectUCode = jsonRequest.GetCollectUCodeEvents();
    LOG_DEBUG << "Setting Configurations." << " Device: " << deviceName << " IDLE_SM: " << setCollectIdleSm << " RX PPDU: " << setCollectRxPpdu << " TX PPDU: " << setCollectTxPpdu << " uCode: "
            << setCollectUCode << std::endl;
    if (setCollectIdleSm.GetState() == JSON_VALUE_PROVIDED)
    {
        LOG_DEBUG << "Setting PMC IDLE_SM event collection: " << setCollectIdleSm << std::endl;
        auto collectSmRes = PmcSetConfigHandlerSequences::ConfigCollectIdleSm(deviceName, setCollectIdleSm.GetValue());
        if (!collectSmRes.IsSuccess())
        {
            jsonResponse.Fail(collectSmRes.GetStatusMessage());
            return;
        }
    }
    if (setCollectRxPpdu.GetState() == JSON_VALUE_PROVIDED)
    {
        LOG_DEBUG << "Setting PMC RX PPDU event collection: " << setCollectRxPpdu << std::endl;
        auto rxPpduRes = PmcSetConfigHandlerSequences::ConfigCollectRxPpdu(deviceName, setCollectRxPpdu.GetValue());
        if (!rxPpduRes.IsSuccess())
        {
            jsonResponse.Fail(rxPpduRes.GetStatusMessage());
            return;
        }
    }
    if (setCollectTxPpdu.GetState() == JSON_VALUE_PROVIDED)
    {
        LOG_DEBUG << "Setting PMC TX PPDU event collection: " << setCollectTxPpdu << std::endl;
        auto txPpduRes = PmcSetConfigHandlerSequences::ConfigCollectTxPpdu(deviceName, setCollectTxPpdu.GetValue());
        if (!txPpduRes.IsSuccess())
        {
            jsonResponse.Fail(txPpduRes.GetStatusMessage());
            return;
        }
    }

    if (setCollectUCode.GetState() == JSON_VALUE_PROVIDED)
    {
        LOG_DEBUG << "Setting PMC uCode event collection: " << setCollectUCode << std::endl;
        auto collectUcode = PmcSetConfigHandlerSequences::ConfigCollectUcode(deviceName, setCollectUCode.GetValue());
        if (!collectUcode.IsSuccess())
        {
            jsonResponse.Fail(collectUcode.GetStatusMessage());
            return;
        }
    }

    //TODO: for now, everything else gets default values
    auto st = PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, "MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_0.delimiter", 0xcafe);
    if (!st.IsSuccess())
    {
        jsonResponse.Fail(st.GetStatusMessage());
        return;

    }

    auto st2 = PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, "MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_0.ucpu_udef_en", 1U);
    if (!st2.IsSuccess())
    {
        jsonResponse.Fail(st2.GetStatusMessage());
        return;
    }

    auto st3 = PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, "MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_0.fw_udef_en", 1U);
    if (!st3.IsSuccess())
    {
        jsonResponse.Fail(st3.GetStatusMessage());
        return;
    }

    auto st4 = PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, "MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_1.intf_type", 1);
    if (!st4.IsSuccess())
    {
        jsonResponse.Fail(st4.GetStatusMessage());
        return;
    }
    auto st5 = PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, "MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_1.dma_if_en", 1);
    if (!st5.IsSuccess())
    {
        jsonResponse.Fail(st5.GetStatusMessage());
        return;
    }

    // Configure PMC HW FIFO threshold (in uint32_t). When the FIFO is filled to this threshold, its content is flushed to the current DMA descriptor.
    // For optimal ring utilization this threshold should be equal to the descriptor buffer size, hence threshold = descriptor_bytes/4.

    auto st6 = PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, "MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_1.pkt_treshhold", 256);
    if (!st6.IsSuccess())
    {
        jsonResponse.Fail(st6.GetStatusMessage());
        return;
    }

    auto st7 = PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, "MAC_RGF.PMC.RX_TX.MAC_MAC_RGF_PMC_RX_TX_0", 0xFC);
    if (!st7.IsSuccess())
    {
        jsonResponse.Fail(st7.GetStatusMessage());
        return;
    }
    auto st8 = PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, "MAC_RGF.PMC.RX_TX.RX.MAC_MAC_RGF_PMC_RX_0.qid_mask", 0x1);
    if (!st8.IsSuccess())
    {
        jsonResponse.Fail(st8.GetStatusMessage());
        return;
    }
    auto st9 = PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, "MAC_RGF.PMC.RX_TX.TX.MAC_MAC_RGF_PMC_RX_TX_TX_0.qid_mask", 0x1);
    if (!st9.IsSuccess())
    {
        jsonResponse.Fail(st9.GetStatusMessage());
        return;
    }
    auto st10 = PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, "MAC_RGF.PRS.CTRL.MAC_PRS_CTRL_0.pmc_post_dec_mode", 1U);
    if (!st10.IsSuccess())
    {
        jsonResponse.Fail(st10.GetStatusMessage());
        return;
    }

    if (type == BasebandType::BASEBAND_TYPE_TALYN)
    {
        auto st11 = PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, "MAC_RGF.PRS.CTRL.MAC_PRS_CTRL_0.pmc_post_dec_mode", 1U);
        if (!st11.IsSuccess())
        {
            jsonResponse.Fail(st11.GetStatusMessage());
            return;
        }
    }
    auto rxMpduRes = PmcSetConfigHandlerSequences::ConfigRxMpdu(deviceName);
    if(!rxMpduRes.IsSuccess())
    {
        jsonResponse.Fail(rxMpduRes.GetStatusMessage());
        return;
    }
    auto txMpduRes = PmcSetConfigHandlerSequences::ConfigTxMpdu(deviceName);
    if(!txMpduRes.IsSuccess())
    {
        jsonResponse.Fail(txMpduRes.GetStatusMessage());
        return;
    }
    LOG_DEBUG << "PMC SetConfig Response: " << jsonResponse << std::endl;
}

