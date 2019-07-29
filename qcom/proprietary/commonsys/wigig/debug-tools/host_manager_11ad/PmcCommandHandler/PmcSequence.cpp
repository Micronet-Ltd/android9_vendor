/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include <sstream>
#include "PmcSequence.h"
#include "Host.h"
#include "DebugLogger.h"
#include "PmcService.h"

//for now we don't have an API to get a proper revision so using this until one is created
BasebandRevisionEnum PmcSequence::GetDeviceRevision(const std::string& deviceName)
{
    BasebandType type;
    Host::GetHost().GetDeviceManager().GetDeviceBasebandType(deviceName, type);
    if (type == BasebandType::BASEBAND_TYPE_SPARROW)
    {
        return BasebandRevisionEnum::SPR_D0;
    }
    if (type == BasebandType::BASEBAND_TYPE_TALYN)
    {
        return BasebandRevisionEnum::TLN_M_B0;
    }
    return BasebandRevisionEnum::UNKNOWN;
}

OperationStatus PmcSequence::IsPmcRecordingActive(const std::string& deviceName, bool& isRecordingActive)
{
    // Check if PMC is recording
    const std::string CHECK_PMC_STATE_RGF = "MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_0.rec_active";
    uint32_t value;
    OperationStatus status = PmcService::GetInstance().GetPmcRegistersAccessor().ReadPmcRegister(deviceName, CHECK_PMC_STATE_RGF, value);
    if (!status.IsSuccess())
    {
        return status;
    }
    isRecordingActive = (1 == value);
    return OperationStatus(true);
}

OperationStatus PmcSequence::DisablePmcClock(const std::string& deviceName)
{
    LOG_INFO << "Disable PMC clock" << std::endl;
    std::string pmcClockRegister = GetPmcClockRegister(deviceName);
    OperationStatus sta = PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, pmcClockRegister, 0);
    if (!sta.IsSuccess())
    {
        return sta;
    }
    return OperationStatus(true);
}

std::string PmcSequence::GetPmcClockRegister(const std::string& deviceName)
{
    BasebandType type;
    Host::GetHost().GetDeviceManager().GetDeviceBasebandType(deviceName, type);
    if (type == BasebandType::BASEBAND_TYPE_SPARROW)
    {
        return "USER.CLK_CTL_EXTENTION.USER_USER_CLKS_CTL_EXT_SW_BYPASS_HW_CG_0.mac_pmc_clk_sw_bypass_hw";
    }
    return "USER.EXTENTION.USER_USER_EXTENTION_3.mac_pmc_clk_sw_bypass_hw";
}

OperationStatus PmcSequence::FreePmcRing(const std::string& deviceName, bool bSuppressError)
{
    std::string outMessage;
    auto res = Host::GetHost().GetDeviceManager().DeallocPmc(deviceName, bSuppressError, outMessage);
    if (!res == DeviceManagerOperationStatus::dmosSuccess)
    {
        return OperationStatus(false, outMessage);
    }
    return OperationStatus(true);
}

OperationStatus PmcSequence::FindPmcRingIndex(const std::string& deviceName, uint32_t& ringIndex)
{
    const uint32_t DESCQ_SIZE = 12;
    LOG_INFO << "Looking for PMC Descriptor Queue by querying DMA_RGF.DESCQ.<i>.IS.IS.is_pmc_dma for i=[0..{DESCQ_SIZE - 1}]" << std::endl;
    for (uint32_t i = 0; i < DESCQ_SIZE; ++i)
    {
        std::stringstream ss;
        ss << "DMA_RGF.DESCQ.<" << i << ">.IS.IS.is_pmc_dma";
        std::string pmcFlagMnemonic = ss.str();
        uint32_t pmcFlag;
        OperationStatus st = PmcService::GetInstance().GetPmcRegistersAccessor().ReadPmcRegister(deviceName, pmcFlagMnemonic, pmcFlag);
        if (!st.IsSuccess())
        {
            return st;
        }
        if(pmcFlag == Utils::REGISTER_DEFAULT_VALUE)
        {
            return OperationStatus(false, "Cannot read register " + pmcFlagMnemonic + ". Check RGF tree configuration");
        }
        LOG_INFO << "Read PMC flag from " << pmcFlagMnemonic << ": " << pmcFlag << std::endl;
        if (pmcFlag == 1)
        {
            LOG_INFO << "Found PMC ring: DMA_RGF.DESCQ.<" << i << ">" << std::endl;
            ringIndex = i;
            return OperationStatus(true);
        }
    }
    return OperationStatus(false, "Cannot find PMC ring. No is_pmc_dma flag is set in DMA_RGF.DESCQ.<i>.IS.IS");
}

OperationStatus PmcSequence::ValidateDeviceNameAndType(const std::string& deviceName, BasebandType& deviceType)
{
    LOG_DEBUG << "Checking if device name " << deviceName << " is valid " << std::endl;
    if (!Host::GetHost().GetDeviceManager().GetDeviceByName(deviceName))
    {
        return OperationStatus(false, "Device Name does not exist");
    }
    if (!Host::GetHost().GetDeviceManager().GetDeviceBasebandType(deviceName, deviceType))
    {
        return OperationStatus(false, "Failed reading device type");
    }
    return OperationStatus(true);
}

OperationStatus PmcSequence::ReadPmcHead(const std::string& deviceName, uint32_t& pmcHead)
{
    uint32_t pmcRingIndex;
    auto res = PmcSequence::FindPmcRingIndex(deviceName, pmcRingIndex);
    if (!res.IsSuccess())
    {
        return res;
    }
    // Read the SW Head
    std::stringstream ss;
    ss << "DMA_RGF.DESCQ.<" << pmcRingIndex << ">.SW_HEAD.CRNT";
    std::string pmcSwHeadMnemonic = ss.str();
    auto st = PmcService::GetInstance().GetPmcRegistersAccessor().ReadPmcRegister(deviceName, pmcSwHeadMnemonic, pmcHead);
    if (!st.IsSuccess())
    {
        return st;
    }
    LOG_INFO << "Read PMC SW Head from " << pmcSwHeadMnemonic << ": " << pmcHead;
    return OperationStatus(true);
}

// Disable events asap so that multiple stations can stop more or less simultaneously.
// No delays are allowed in this method.
OperationStatus PmcSequence::DisableEventCollection(const std::string& deviceName)
{
    LOG_DEBUG << "Disabling event collection" << std::endl;
    // We should not check for recording status here. The flow should assume the PMC has been stopped
    // by an external trigger.
    PmcService::GetInstance().GetPmcDeviceContext(deviceName).GetDeviceRecordingState().SetState(PMC_RECORDING_STATE::PMC_RECORDING_STATE_STOPPED);
    std::string PMC_GENERAL_RGF_0 = "MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_0";
    uint32_t generalRGF;
    auto st = PmcService::GetInstance().GetPmcRegistersAccessor().ReadPmcRegister(deviceName, PMC_GENERAL_RGF_0, generalRGF);
    if (!st.IsSuccess())
    {
        return st;
    }
    const uint32_t PMC_SAMPLING_NONE_MASK = 0xFFFFFF;
    auto writest = PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, PMC_GENERAL_RGF_0, generalRGF & PMC_SAMPLING_NONE_MASK);
    if (!writest.IsSuccess())
    {
        return writest;
    }
    uint32_t currentTsf;
    auto readst = PmcService::GetInstance().GetPmcRegistersAccessor().ReadPmcRegister(deviceName,
            "MAC_RGF.MAC_SXD.TIMING_INDIRECT.TIMING_INDIRECT_REG_5.msrb_capture_ts_low", currentTsf);
    if (!readst.IsSuccess())
    {
        return readst;
    }
    uint32_t currentTsf24Bit = currentTsf & 0x00FFFFFF;
    LOG_INFO << "Disabling event collection. Current TSF = 0x" << Hex<8>(currentTsf) << "  24-bit TSF: " << Hex<8>(currentTsf24Bit) << std::endl;
    return OperationStatus(true);
}

// Disable PMC recording with some delay after disabling events. This delay is required due to a HW bug.
OperationStatus PmcSequence::DeactivateRecording(const std::string& deviceName)
{
    LOG_INFO << "Deactivating PMC Recording" << std::endl;
    // Disable PMC
    auto res = PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, "MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_0.rec_en_clr", 1);
    if (!res.IsSuccess())
    {
        return res;
    }
    uint32_t currentTsf;
    OperationStatus st = PmcService::GetInstance().GetPmcRegistersAccessor().ReadPmcRegister(deviceName,
            "MAC_RGF.MAC_SXD.TIMING_INDIRECT.TIMING_INDIRECT_REG_5.msrb_capture_ts_low", currentTsf);
    if (!st.IsSuccess())
    {
        return st;
    }
    uint32_t currentTsf24Bit = currentTsf & 0x00FFFFFF;
    LOG_INFO << "Deactivated. Current TSF = 0x" << Hex<8>(currentTsf) << "  24-bit TSF: " << Hex<8>(currentTsf24Bit) << std::endl;
    // Wait for pmc to finish flush internal buffer to dma
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    auto stdisable = PmcSequence::DisablePmcClock(deviceName);
    if (!stdisable.IsSuccess())
    {
        return stdisable;
    }
    return OperationStatus(true);
}

// Make sure PMC is inactive.
OperationStatus PmcSequence::Shutdown(const std::string& deviceName)
{
    LOG_DEBUG << "PMC Shutdown" << std::endl;
    auto disableRes = DisableEventCollection(deviceName);
    if(!disableRes.IsSuccess())
    {
        return disableRes;
    }
    auto deactivateRes = DeactivateRecording(deviceName);
    if(!deactivateRes.IsSuccess())
    {
        return deactivateRes;
    }
    auto freePmcRinRes = PmcSequence::FreePmcRing(deviceName, true);
    if(!freePmcRinRes.IsSuccess())
    {
        return freePmcRinRes;
    }
    BasebandRevisionEnum revision = PmcSequence::GetDeviceRevision(deviceName);
    // Talyn MB adds SW reset feature
    if (revision == BasebandRevisionEnum::TLN_M_B0)
    {
        LOG_DEBUG << "Performing PMC SW Reset" << std::endl;
        std::string PMC_SW_RESET_MNEMONIC = "USER.CLKS_CTL.SW.RST.USER_USER_CLKS_CTL_SW_RST_VEC_0";
        uint32_t pmcSwResetRegValue;
        auto st = PmcService::GetInstance().GetPmcRegistersAccessor().ReadPmcRegister(deviceName, PMC_SW_RESET_MNEMONIC, pmcSwResetRegValue);
        if (!st.IsSuccess())
        {
            return st;
        }
        if (pmcSwResetRegValue == Utils::REGISTER_DEFAULT_VALUE)
        {
            return OperationStatus(false, "No PMC SW reset supported as expected on device");
        }
        LOG_DEBUG << "Got SW Reset register value: 0x" << Hex<8>(pmcSwResetRegValue) << std::endl;
        auto pmcSwResetRegValue15is1 = PmcService::GetInstance().GetPmcRegistersAccessor().WriteToBitMask(pmcSwResetRegValue, 15, 1, 1);
        LOG_DEBUG << "Raising SW Reset flag: 0x" << Hex<8>(pmcSwResetRegValue15is1) << std::endl;
        auto st1 = PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, PMC_SW_RESET_MNEMONIC, pmcSwResetRegValue15is1);
        if (!st1.IsSuccess())
        {
            return st1;
        }
        auto pmcSwResetRegValue15is0 = PmcService::GetInstance().GetPmcRegistersAccessor().WriteToBitMask(pmcSwResetRegValue, 15, 1, 0);
        LOG_DEBUG << "Clearing SW Reset flag: 0x" << Hex<8>(pmcSwResetRegValue15is0) << std::endl;
        auto st2 = PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, PMC_SW_RESET_MNEMONIC, pmcSwResetRegValue15is0);
        if (!st2.IsSuccess())
        {
            return st2;
        }
    }
    return OperationStatus(true);
}

OperationStatus PmcSequence::EnablePmcClock(const std::string& deviceName)
{
    LOG_INFO << "Enable PMC clock" << std::endl;
    std::string pmcClockRegister = PmcSequence::GetPmcClockRegister(deviceName);
    return PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, pmcClockRegister, 1);
}

OperationStatus PmcSequence::AllocatePmcRing(const std::string& deviceName)
{
    std::string outMessage;
    auto descriptorSizeInBytes = DESCRIPTOR_SIZE_KB * 1024;

    // Allocate DMA through driver DebugFS
    auto res = Host::GetHost().GetDeviceManager().AllocPmc(deviceName, descriptorSizeInBytes, NUM_OF_DESCRIPTORS, outMessage);
    if (!res == DeviceManagerOperationStatus::dmosSuccess)
    {
        return OperationStatus(false, "Failed allocating Pmc ring");
    }
    BasebandRevisionEnum revision = PmcSequence::GetDeviceRevision(deviceName);
    if (revision == BasebandRevisionEnum::TLN_M_B0)
    {
        uint32_t pmcRingIndex;
        auto st2 = PmcSequence::FindPmcRingIndex(deviceName, pmcRingIndex);
        if (!st2.IsSuccess())
        {
            return st2;
        }
        std::stringstream ss;
        ss << "DMA_RGF.DESCQ.<"  << pmcRingIndex << ">.RX_MAX_SIZE.MAX_RX_PL_PER_DESCRIPTOR.val";
        std::string pmcDescLengthMnemonic = ss.str();
        LOG_INFO << "Configure descriptor size: " << pmcDescLengthMnemonic << " = 0x " << Hex<8>(descriptorSizeInBytes) << std::endl;
        auto res = PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, pmcDescLengthMnemonic, descriptorSizeInBytes);
        if(!res.IsSuccess())
        {
            return OperationStatus(false, "Cannot configure descriptor size for the PMC ring: error writing to register " + pmcDescLengthMnemonic);
        }
    }
    return OperationStatus(true);
}

/// <summary>
/// Workaround for HW bug Bug 6979 - Disable PRP clock gating when using PMC.
/// </summary>
OperationStatus PmcSequence::DisablePrpClockGating(const std::string& deviceName)
{
    BasebandRevisionEnum revision = PmcSequence::GetDeviceRevision(deviceName);
    if (revision != BasebandRevisionEnum::TLN_M_B0)
    {
        return OperationStatus(true);
    }

    LOG_INFO << "Disabling PRP clock gating" << std::endl;
    std::string CLK_GATE_MNEMONIC = "USER.EXTENTION.USER_USER_EXTENTION_3.mac_prp_ahb_rgs_hclk_sw_bypass_hw";
    uint32_t clkGateRegValue;
    auto st = PmcService::GetInstance().GetPmcRegistersAccessor().ReadPmcRegister(deviceName, CLK_GATE_MNEMONIC, clkGateRegValue);
    if (!st.IsSuccess())
    {
        return st;
    }
    if (clkGateRegValue == Utils::REGISTER_DEFAULT_VALUE)
    {
        return OperationStatus(false, "Clock gating register " + CLK_GATE_MNEMONIC +" has invalid value");
    }
    LOG_INFO << "Got clock gating register value: 0x" << Hex<8>(clkGateRegValue) << std::endl;
    auto dataToWrite = PmcService::GetInstance().GetPmcRegistersAccessor().WriteToBitMask(clkGateRegValue, 16, 1, 1);
    auto st2 = PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, CLK_GATE_MNEMONIC, dataToWrite);
    if (!st2.IsSuccess())
    {
        return st2;
    }
    LOG_INFO << "Disabling clock gating: " << CLK_GATE_MNEMONIC << " = 0x" << Hex<8>(dataToWrite) << std::endl;
    auto res = PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, CLK_GATE_MNEMONIC, clkGateRegValue);
    if(!res.IsSuccess())
    {
        LOG_INFO << "Cannot disable PRP clock gating: error writing to register " + CLK_GATE_MNEMONIC;
    }
    return OperationStatus(true);
}

OperationStatus PmcSequence::PausePmcRecording(const std::string& deviceName)
{
    // Disable PMC
    auto res = PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, "MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_0.rec_en_clr", 1);
    if (!res.IsSuccess())
    {
        return OperationStatus(false, res.GetStatusMessage());
    }

    uint32_t currentTsf;
    auto st = PmcService::GetInstance().GetPmcRegistersAccessor().ReadPmcRegister(deviceName, "MAC_RGF.MAC_SXD.TIMING_INDIRECT.TIMING_INDIRECT_REG_5.msrb_capture_ts_low", currentTsf);
    if (!st.IsSuccess())
    {
        return OperationStatus(false, res.GetStatusMessage());
    }
    uint32_t currentTsf24Bit = currentTsf & 0x00FFFFFF;
    LOG_INFO << "Deactivated. Current TSF = 0x" << Hex<8>(currentTsf) << "  24-bit TSF: " << Hex<8>(currentTsf24Bit);

    // Wait for pmc to finish flush internal buffer to dma
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    auto st2 = PmcSequence::DisablePmcClock(deviceName);
    if (!st2.IsSuccess())
    {
        return OperationStatus(false, res.GetStatusMessage());
    }
    return OperationStatus(true);
}

OperationStatus PmcSequence::ActivateRecording(const std::string& deviceName)
{
    uint32_t currentTsf;
    auto st = PmcService::GetInstance().GetPmcRegistersAccessor().ReadPmcRegister(deviceName, "MAC_RGF.MAC_SXD.TIMING_INDIRECT.TIMING_INDIRECT_REG_5.msrb_capture_ts_low", currentTsf);
    if (!st.IsSuccess())
    {
        return st;
    }
    uint32_t currentTsf24Bit = currentTsf & 0x00FFFFFF;
    LOG_INFO << "Activating PMC Recording. Current TSF = 0x" << Hex<8>(currentTsf) << "  24-bit TSF: " << Hex<8>(currentTsf24Bit);
    auto enableWriteRes = PmcService::GetInstance().GetPmcRegistersAccessor().WritePmcRegister(deviceName, "MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_0.rec_en_set", 1);
    if (!enableWriteRes.IsSuccess())
    {
        return enableWriteRes;
    }
    PmcService::GetInstance().GetPmcDeviceContext(deviceName).GetDeviceRecordingState().SetState(PMC_RECORDING_STATE::PMC_RECORDING_STATE_ACTIVE);
    return OperationStatus(true);
}
