/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "LogCollectorDefinitions.h"

namespace log_collector
{
    // consts
    const std::map<CpuType, size_t> logTracerTypeToLogBufferSizeInDwords = { { CPU_TYPE_FW, 1024 },{ CPU_TYPE_UCODE, 256 } }; // value in DWORDs!
    /*const std::string module_names[log_collector::MODULES::NUM_MODULES] = {
    "system", "drivers", "mac_mon", "host_cmd",
    "phy_mon", "infra", "calibs", "txrx",
    "rad_mgr", "scan", "mlme", "l2_mgr",
    "disc", "mgmt_srv", "sec_psm", "wbe_mngr"
    };*/
    // Module names can be changed by the FW, therefore it cannot be hard-coded, so we use module number instead. The current names are commented out above.
    const std::string module_names[log_collector::MODULES::NUM_MODULES] = {
        "Module0", "Module1", "Module2", "Module3",
        "Module4", "Module5", "Module6", "Module7",
        "Module8", "Module9", "Module10", "Module11",
        "Module12", "Module13", "Module14", "Module15"
    };
    const char *const levels[] = { "E", "W", "I", "V" };
    const std::map<BasebandType, unsigned> baseband_to_peripheral_memory_start_address_linker = { { BASEBAND_TYPE_SPARROW, 0x840000 },{ BASEBAND_TYPE_TALYN, 0x840000 } };
    const std::map<BasebandType, unsigned> baseband_to_peripheral_memory_start_address_ahb = { { BASEBAND_TYPE_SPARROW, 0x908000 },{ BASEBAND_TYPE_TALYN, 0xA20000 } };
    const std::map<BasebandType, unsigned> baseband_to_ucode_dccm_start_address_linker = { { BASEBAND_TYPE_SPARROW, 0x800000 },{ BASEBAND_TYPE_TALYN, 0x800000 } };
    const std::map<BasebandType, unsigned> baseband_to_ucode_dccm_start_address_ahb = { { BASEBAND_TYPE_SPARROW, 0x940000 },{ BASEBAND_TYPE_TALYN, 0xA78000 } };
    const std::map<CpuType, int> logTracerTypeToLogOffsetAddress = { { CPU_TYPE_FW, 0x880004 /*REG_FW_USAGE_1*/ },{ CPU_TYPE_UCODE, 0x880008 /*REG_FW_USAGE_2*/ } };

    // configuration parameters
    const std::string POLLING_INTERVAL_MS = "PollingIntervalMs";
    const std::string OUTPUT_FILE_SUFFIX = "OutputFileSuffix";
    const std::string MAX_SINGLE_FILE_SIZE_MB = "MaxSingleFileSizeMb";
    const std::string MAX_NUM_OF_LOG_FILES = "MaxNumberOfLogFiles";
    const std::string MODULES_VERBOSITY_LEVELS = "ModulesVerbosityLevels";
}
