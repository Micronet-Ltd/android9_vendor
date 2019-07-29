/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <stdarg.h>
#include <ctime>

#ifndef _WINDOWS    // Linux
#include <unistd.h>
#include <getopt.h>
#include <err.h>
#else                // Windows
#include <windows.h>
#endif  //#ifdef _WINDOWS



#include "WlctPciAcss.h"

using namespace std;

////////// TYPE DEFINITIONS //////////
enum TRACER_TYPE
{
    TRACER_TYPE_FW,
    TRACER_TYPE_UCODE
};

enum MODULES
{
    SYSTEM,
    DRIVERS,
    MAC_MON,
    HOST_CMD,
    PHY_MON,
    INFRA,
    CALIBS,
    TXRX,
    RAD_MGR,
    SCAN,
    MLME,
    L2_MGR,
    DISC,
    MGMT_SRV,
    SECURITY,
    PSM,
    WBE_MNGR,
    NUM_MODULES,
};

string module_names[NUM_MODULES] = {"SYSTEM", "DRIVERS", "MAC_MON", "HOST_CMD", "PHY_MON", "INFRA", "CALIBS", "TXRX", "RAD_MGR", "SCAN", "MLME", "L2_MGR", "DISC", "MGMT_SRV", "SECURITY", "PSM", "WBE_MNGR"};

typedef uint32_t u32;
typedef int32_t s32;
typedef unsigned int uint;

#ifdef _WINDOWS
struct module_level_enable { /* Little Endian */
    uint error_level_enable : 1;
    uint warn_level_enable : 1;
    uint info_level_enable : 1;
    uint verbose_level_enable : 1;
    uint reserved0 : 4;
};
struct log_trace_header { /* Little Endian */
    uint strring_offset : 20;
    uint module : 4; /* module that outputs the trace */
    uint level : 2;
    uint parameters_num : 2; /* [0..3] */
    uint is_string : 1; /* indicate if the printf uses %s */
    uint signature : 3; /* should be 5 (2'101) in valid header */
};
#else
struct module_level_enable { /* Little Endian */
    uint error_level_enable : 1;
    uint warn_level_enable : 1;
    uint info_level_enable : 1;
    uint verbose_level_enable : 1;
    uint reserved0 : 4;
} __attribute__((packed));

struct log_trace_header { /* Little Endian */
    /* the offset of the trace string in the strings sections */
    uint strring_offset : 20;
    uint module : 4; /* module that outputs the trace */
    /*    0 - Error
        1- WARN
        2 - INFO
        3 - VERBOSE */
    uint level : 2;
    uint parameters_num : 2; /* [0..3] */
    uint is_string : 1; /* indicate if the printf uses %s */
    uint signature : 3; /* should be 5 (2'101) in valid header */
} __attribute__((packed));
#endif

union log_event {
    struct log_trace_header hdr;
    u32 param;
};

struct log_table_header {
    u32 write_ptr; /* incremented by trace producer every write */
    struct module_level_enable module_level_enable[NUM_MODULES];
    union log_event evt[0];
};

enum {
    str_mask = 0xFFFFF,
};

////////// END - TYPE DEFINITIONS ///////

/////////// CONSTANTS ///////////////////

const int SECOND_IN_MILLISECONDS = 1000;

// 50 MB file size
const int MAX_FILE_FRAGMENT_SIZE = 1024 * 1024 * 50;

// RGFs containing log buffer addresses
// FW log address
const int REG_FW_USAGE_1 = 0x880004;
// uCode log address
const int REG_FW_USAGE_2 = 0x880008;

// Firmware version RGFs
const int FW_VERSION_MAJOR     = 0x880a2c;
const int FW_VERSION_MINOR     = 0x880a30;
const int FW_VERSION_SUB     = 0x880a34;
const int FW_VERSION_BUILD     = 0x880a38;

// Firmware Compilation Time RGFs
const int FW_COMP_TIME_HOUR     = 0x880a14;
const int FW_COMP_TIME_MINUTE     = 0x880a18;
const int FW_COMP_TIME_SECOND     = 0x880a1c;
const int FW_COMP_TIME_DAY         = 0x880a20;
const int FW_COMP_TIME_MONTH     = 0x880a24;
const int FW_COMP_TIME_YEAR     = 0x880a28;

// Log buffer offsets
// FW log address offset
const int FW_LOG_ADDRESS_OFFSET = 0xc8000;
// uCode log address offset
//const int UCODE_LOG_ADDRESS_OFFSET = 0;

// Entries in the fw log buf
const size_t fw_log_buf_entries = 0x1000/4; // 1024 dec
const size_t ucode_log_buf_entries = 256;

// Config file filename
#ifdef _WINDOWS
const char* const DEFAULT_CONFIG_FILE_NAME = "wigig_logcollector.ini";
#else
const char* const DEFAULT_CONFIG_FILE_NAME = "/etc/wigig_logcollector.ini";
#endif

//////////// END - CONSTANTS ////////////

//////////// Global Variables ///////////

// Log addresses
int fw_log_address = 0;
int ucode_log_address = 0;

/////// Config file parameters /////////
int pollingInterval = 100;
string resultPath = "";
string deviceName = "";
string deviceType = "";
TRACER_TYPE m_tracerType  = TRACER_TYPE_FW;
int fileFragmentSize = MAX_FILE_FRAGMENT_SIZE;
bool debugPrint = false;
DType_t devType = MST_NONE;

log_table_header logHeader;

//// END - Config file parameters /////

string log_file_prefix = "logFile_fw_";

void* handler = NULL;

DWORD fwVersionMajor;
DWORD fwVersionMinor;
DWORD fwVersionSub;
DWORD fwVersionBuild;

DWORD fwCompTimeHour;
DWORD fwCompTimeMinute;
DWORD fwCompTimeSecond;
DWORD fwCompTimeDay;
DWORD fwCompTimeMonth;
DWORD fwCompTimeYear;

std::string configFilePath;
const char *mod;
int i;
unsigned long x;
char *endptr;
int help; /* = 0; */

void *log_bu; /* memory allocated for the log buf */

 void *str_buf;
 size_t str_sz;
 u32 rptr; /* = 0; */
 u32 last_wptr; /* = 0; */
 const char *const levels[] = {
    "E",
    "W",
    "I",
    "V",
};

const char *modules[16];

//////////// END - Global Variables //////

static inline size_t log_size(size_t entry_num)
{
    return sizeof(struct log_table_header) + entry_num * 4;
}

// OS Agnostic system time
struct tm OSGetSystemTime()
{
    time_t current_time;
    time(&current_time);

    struct tm *pTimeStruct = localtime(&current_time);
    struct tm timeStruct = {};
    if (pTimeStruct)
    {
        timeStruct = *pTimeStruct;
    }

    return timeStruct;
}

// OS agnostic debug print function
void DebugPrint(const char* error_message, ...)
{
    if (debugPrint == false) return;

    va_list argptr;
    va_start(argptr, error_message);

    vfprintf(stderr, error_message, argptr);

    va_end(argptr);
}

void DisplayHelp()
{
        static char *help_str = (char*)("Usage: LogCollector [OPTION]...\n"
                                "Extract trace log from firmware\n"
                                "\n"
                                "Mandatory arguments to long options are mandatory for short options too.\n"
                                " The following switches are mandatory:\n"
                                "  -m, --memdump=FILE        File to read memory dump from\n"
                                "  -l, --logsize=NUMBER        Log buffer size, entries\n");
        printf("%s", help_str);
        exit(1);
}

// OS agnostic sleep function
void OSSleep(int sleep_period)
{
#ifdef _WINDOWS
    Sleep(sleep_period);
#else
    usleep(sleep_period * SECOND_IN_MILLISECONDS);
#endif
}

// OS agnostic error print function
void OSError(const char* error_message, ...)
{
    va_list argptr;
    va_start(argptr, error_message);

#ifdef _WINDOWS
    vfprintf(stderr, error_message, argptr);

    exit(0);
#else
    vfprintf(stderr, error_message, argptr);

    exit(0);
#endif

    va_end(argptr);
}

void ParseModuelLevel(string moduleString)
{
    for (int i = 0; i < NUM_MODULES; i++)
    {
        if (moduleString.find(module_names[i]) == 0)
        {
            // + 1 for '='
            string levels = moduleString.substr(module_names[i].size() + 1);

            if (levels.find("V") != string::npos)
            {
                logHeader.module_level_enable[i].verbose_level_enable = 1;
            }
            if (levels.find("I") != string::npos)
            {
                logHeader.module_level_enable[i].info_level_enable = 1;
            }
            if (levels.find("E") != string::npos)
            {
                logHeader.module_level_enable[i].error_level_enable = 1;
            }
            if (levels.find("W") != string::npos)
            {
                logHeader.module_level_enable[i].warn_level_enable = 1;
            }
        }
    }
}

// OS agnostic get arguments function
void ParseConfigLine(string line)
{
    const char* device_name = "device_name=";
    const char* device_type = "device_type=";
    const char* polling_interval = "polling_interval=";
    const char* result_path = "result_path=";
    const char* module_level_prefix = "MODULE_LEVEL_";
    const char* debug_print = "debug_print=";
    const char* log_fragment_size = "log_fragment_size=";
    const char* tracer_type_string = "tracer_type=";

    DebugPrint("configuration line = %s\n", line.c_str());

    // Skip comments
    if (line.find("//") == 0)
    {
        return;
    }

    try
    {
        if (line.find(debug_print) == 0)
        {
            debugPrint = (line.substr(strlen(debug_print)).find("TRUE") == 0);
            DebugPrint("Debug Prints Enabled\n");
        }
        else if (line.find(log_fragment_size) == 0)
        {
            fileFragmentSize = atoi(line.substr(strlen(log_fragment_size)).c_str()) * 1024 * 1024;
            DebugPrint("File fragment size is %d MB\n", atoi(line.substr(strlen(log_fragment_size)).c_str()));
        }
        else if (line.find(device_name) == 0)
        {
            deviceName = line.substr(strlen(device_name));
            DebugPrint("Device Name Requested: %s\n", deviceName.c_str());
        }
        else if (line.find(polling_interval) == 0)
        {
            pollingInterval = atoi(line.substr(strlen(polling_interval)).c_str());
            DebugPrint("Polling Interval Is:%d\n", pollingInterval);
        }
        else if (line.find(result_path) == 0)
        {
            resultPath = line.substr(strlen(result_path));
            DebugPrint("Result path = %s\n", resultPath.c_str());
        }
        else if (line.find(module_level_prefix) == 0)
        {
            ParseModuelLevel(line.substr(strlen(module_level_prefix)));
        }
        else if (line.find(device_type) == 0)
        {
            deviceType = line.substr(strlen(device_type));

            DebugPrint("Device Type = %s\n", deviceType.c_str());

            if (deviceType.find("MARLON") == 0)
            {
                DebugPrint("Device selected MARLON=%d\n", MST_MARLON);
                devType = MST_MARLON;
            }
            else if (deviceType.find("SPARROW") == 0)
            {
                DebugPrint("Device selected SPARROW=%d\n", MST_SPARROW);
                devType = MST_SPARROW;
            }
            else if (deviceType.find("TALYN") == 0)
            {
                DebugPrint("Device selected TALYN=%d\n", MST_LAST);
                devType = MST_LAST;
            }
            else
            {
                DebugPrint("Device selected NONE=%d\n", MST_NONE);
                devType = MST_NONE;
            }

        }
        else if (line.find(tracer_type_string) == 0)
        {
            string tracerType = line.substr(strlen(tracer_type_string));
            DebugPrint("Tracer Type = %s\n", tracerType.c_str());

            if (tracerType.find("FW") == 0)
            {
                DebugPrint("Tracer selected FW=%d\n", TRACER_TYPE_FW);
                m_tracerType = TRACER_TYPE_FW;
            }
            else if (tracerType.find("uCode") == 0)
            {
                DebugPrint("Tracer selected uCode=%d\n", TRACER_TYPE_UCODE);
                m_tracerType = TRACER_TYPE_UCODE;
            }
            else
            {
                DebugPrint("Unknown tracer type (only FW/uCode are supported). Set tracer to FW=%d\n", TRACER_TYPE_FW);
                m_tracerType = TRACER_TYPE_FW;
            }
        }
        else
        {
        }
    }
    catch (int)
    {
        OSError("Error: Failed to parse Config File");
    }
}

// Open device interface
static void OpenDevice()
{
    INTERFACE_LIST interfaces;
    int num_items;
    int res;
    res = GetInterfaces(&interfaces, &num_items);

    if (res != 0 || num_items == 0)
    {
        OSError("Error: retrieving interfaces");
    }

    string currentDeviceName;

    // No specific device was requested or only one device is available
    // Select first device in device list
    bool deviceFound = false;

    if (deviceName.compare("") == 0 || num_items == 1)
    {
        cout << "Selecting device: " << interfaces.list[0].ifName << "\n";
        currentDeviceName = interfaces.list[0].ifName;
    }
    else
    {
        // Iterate through all found devices to find the one in config file
        for (int i = 0 ; i < MAX_INTERFACES; i ++)
        {
            currentDeviceName = interfaces.list[i].ifName;

            if (deviceName.compare(currentDeviceName) == 0)
            {
                // We have found the device we are looking for
                cout << "Found device: " << currentDeviceName.c_str() << "\n";
                deviceFound = true;
                break;
            }
        }

        if (!deviceFound)
        {
            OSError("A device named: %s, was not found", deviceName.c_str());
        }
    }

    DebugPrint ("Opening Device: %s\n", currentDeviceName.c_str());

    // Create a handler to the device
    DWORD err = CreateDeviceAccessHandler(currentDeviceName.c_str(), devType, &handler);
    if (!(err == 0 && isInit(handler)))
    {
        OSError("Failed to open device: %s, Error code: %d\n", currentDeviceName.c_str(), err);
    }
}

// Get log address
static int GetLogAddress(TRACER_TYPE type)
{
    static const unsigned peripheral_memory_start_address_ahb_sparrow = 0x908000;
    static const unsigned peripheral_memory_start_address_linker_sparrow = 0x840000;
    static const unsigned ucode_dccm_start_address_linker_sparrow = 0x800000;
    static const unsigned ucode_dccm_start_address_ahb_sparrow = 0x940000;
    static const unsigned REG_FW_USAGE_1 = 0x880004; // fw log address
    static const unsigned REG_FW_USAGE_2 = 0x880008; // uCode log address
    unsigned ahbToLinkerDelta = 0;

    DWORD val;
    DWORD addr;

    if (type == TRACER_TYPE_UCODE)
    {
        ahbToLinkerDelta = ucode_dccm_start_address_ahb_sparrow - ucode_dccm_start_address_linker_sparrow;

        addr = REG_FW_USAGE_2;
    }
    else if (type == TRACER_TYPE_FW)
    {
        ahbToLinkerDelta = peripheral_memory_start_address_ahb_sparrow - peripheral_memory_start_address_linker_sparrow;

        addr = REG_FW_USAGE_1;
    }
    else
    {
        OSError("Invalid log tracer type - using FW as default");
    }


    // Read log offset
    WlctAccssRead(handler, addr, val);

    if (val == 0)
    {
        OSError("Invalid log buffer pointer address");
    }

    return val + ahbToLinkerDelta;
}

// OS agnostic read log function
static bool OSReadLog(void *buf, size_t size)
{
    // Check if the device is open
    if ((NULL == handler) || (!isInit(handler)))
    {
        OSError("Device not opened when trying the read log");
    }

    // Update FW & uCode log addresses
    int log_address = 0;
    if (TRACER_TYPE_FW == m_tracerType)
    {
        if (!fw_log_address) fw_log_address = GetLogAddress(TRACER_TYPE_FW);
        log_address = fw_log_address;
    }
    else
    {
        if (!ucode_log_address) ucode_log_address = GetLogAddress(TRACER_TYPE_UCODE);
        log_address = ucode_log_address;
    }

    DebugPrint("OSReadLog: tracer: %d, log address = 0x%08X\n", m_tracerType, log_address);

    // Read the actual log
    if(0 != readBlock(handler, log_address, size, (char*)buf))
    {
        return false;
        // OSError("Failed to read log buffer");
    }
    return true;
}

// OS agnostic read configuration file function
static void OSReadConfigFile()
{
    try
    {
            ifstream file(configFilePath.c_str());

        if(!file || !file.good())
        {
                    OSError("Error: failed to read config file: %s. %s \n", configFilePath.c_str(), strerror(errno));
        }

        string line;
        while(std::getline(file, line))
        {
            ParseConfigLine(line);
        }
    }
    catch (int i)
    {
            OSError("Exception while trying to open config file: %s\n", configFilePath.c_str());
    }
}

 void AddTimestamp(ofstream& outputFile)
{
    struct tm now = OSGetSystemTime();

    std::ostringstream timeStampBuilder;

    timeStampBuilder << "<Log_Content>"
        << "<Sample_Time>"
        << "<Hour>" << now.tm_hour << "</Hour>"
        << "<Minute>" << now.tm_min << "</Minute>"
        << "<Second>" << now.tm_sec << "</Second>"
        << "<Day>" << now.tm_mday << "</Day>"
        << "<Month>" << now.tm_mon + 1 << "</Month>"
        << "<Year>" << now.tm_year + 1900 << "</Year>"
        << "</Sample_Time>";

    outputFile << timeStampBuilder.str();
}

static int ParseLog(void* log_buf, size_t log_buf_entries, ofstream& outputFile)
{
//    DebugPrint("Parsing log\n");

    int sizeAdded = 0;

    // Prepare a header pointing to log buffer top
    struct log_table_header *h = (struct log_table_header*)log_buf;

    u32 wptr = h->write_ptr;

    if ((wptr - rptr) <= 0)
    {
        // Nothing to read.
        return 0;
    }

    if ((wptr - rptr) >= log_buf_entries)
    {
        // overflow; try to parse last wrap
        rptr = wptr - log_buf_entries;
    }
    DebugPrint("  wptr = %d rptr = %d\n", wptr, rptr);

    AddTimestamp(outputFile);
    outputFile << "<Content>";

    for (; ((s32)(wptr - rptr) > 0) && (wptr != last_wptr); rptr++)
    {
        DebugPrint("wptr = %d, rptr = %d\n", wptr, rptr);

        int i;
        u32 p[3] = {0};
        union log_event *evt = &h->evt[rptr % log_buf_entries];

        if (evt->hdr.signature != 5)
        {
            continue;
        }
        if (evt->hdr.strring_offset > str_sz)
        {
//            continue;
        }
        if (evt->hdr.parameters_num > 3)
        {
            DebugPrint("Parameter Num = %d", evt->hdr.parameters_num);
            continue;
        }
        for (i = 0; i < evt->hdr.parameters_num; i++)
            p[i] = h->evt[(rptr + i + 1) % log_buf_entries].param;
        /*DebugPrint("%d,%s,%d:", evt->hdr.module,
                    levels[evt->hdr.level],
                    evt->hdr.strring_offset);

        DebugPrint("%d,%d,%d\n",
            (p[0]),
            (p[1]),
            (p[2]));*/

        outputFile << evt->hdr.module << "," << levels[evt->hdr.level] << "," << evt->hdr.strring_offset << ":" << p[0] << "," << p[1] << "," << p[2] << "\n";

        // (paramters) (verbosity type) (delimiters)
        sizeAdded += (5 * sizeof(int)) + (1 * sizeof(char)) + (4 * sizeof(char));

        rptr += evt->hdr.parameters_num;
    }

    last_wptr = wptr;

    outputFile << "</Content></Log_Content>";

    fflush(stdout);
    return sizeAdded;
}

void SetModuleVerbosity()
{
    // Check if the device is open
    if ((NULL == handler) || (!isInit(handler)))
    {
        OSError("Device not opened when trying to set module verbosity");
    }

    // Update FW & uCode log addresses
    int log_address = 0;
    if (TRACER_TYPE_FW == m_tracerType)
    {
        if (!fw_log_address) fw_log_address = GetLogAddress(TRACER_TYPE_FW);
        log_address = fw_log_address;
    }
    else
    {
        if (!ucode_log_address) ucode_log_address = GetLogAddress(TRACER_TYPE_UCODE);
        log_address = ucode_log_address;
    }

    DebugPrint("SetModuleVerbosity: tracer: %d, log address = 0x%08X, write address: 0x%08X, size: %d\n", m_tracerType, log_address, log_address + sizeof(logHeader.write_ptr), sizeof(logHeader.module_level_enable));

    // Write verbosity to the device
    if(-1 == writeBlock(handler, log_address + sizeof(logHeader.write_ptr), sizeof(logHeader.module_level_enable), (char*)logHeader.module_level_enable))
    {
        OSError("Failed to write module verbosity structure");
    }
}

void ReadDeviceInfo()
{
    // Check if the device is open
    if ((NULL == handler) || (!isInit(handler)))
    {
        OSError("Device not opened when trying to read device info");
    }

    // Read FW Version
    WlctAccssRead(handler, FW_VERSION_MAJOR, fwVersionMajor);
    WlctAccssRead(handler, FW_VERSION_MINOR, fwVersionMinor);
    WlctAccssRead(handler, FW_VERSION_SUB, fwVersionSub);
    WlctAccssRead(handler, FW_VERSION_BUILD, fwVersionBuild);

    // Read compilation Time
    WlctAccssRead(handler, FW_COMP_TIME_HOUR, fwCompTimeHour);
    WlctAccssRead(handler, FW_COMP_TIME_MINUTE, fwCompTimeMinute);
    WlctAccssRead(handler, FW_COMP_TIME_SECOND, fwCompTimeSecond);
    WlctAccssRead(handler, FW_COMP_TIME_DAY, fwCompTimeDay);
    WlctAccssRead(handler, FW_COMP_TIME_MONTH, fwCompTimeMonth);
    WlctAccssRead(handler, FW_COMP_TIME_YEAR, fwCompTimeYear);
}

void* AllocateBuffer()
{
    void* log_buf = malloc(log_size(fw_log_buf_entries));
    if (!log_buf)
    {
        OSError("Error: Unable to allocate log buffer %zd bytes", log_size(fw_log_buf_entries));
    }

    return log_buf;
}

bool CreateNewOutputFile(string path, ofstream& outputFile)
{
    std::ostringstream fileNameBuilder;
    fileNameBuilder << path << log_file_prefix << time(0);

    DebugPrint("Path: %s\n", path.c_str());
    DebugPrint("Creating output file: %s\n", fileNameBuilder.str().c_str());

    outputFile.open(fileNameBuilder.str().c_str());

    if (outputFile.fail())
    {
        printf("Error opening output file: %s\n", fileNameBuilder.str().c_str());
        return false;
    }

    std::ostringstream headerBuilder;

    headerBuilder << "<LogFile>"
        << "<FW_Ver>"
        << "<Major>" << fwVersionMajor << "</Major>"
        << "<Minor>" << fwVersionMinor <<"</Minor>"
        << "<Sub>" << fwVersionSub << "</Sub>"
        << "<Build>" << fwVersionBuild << "</Build>"
        << "</FW_Ver>"
        << "<Compilation_Time>"
        << "<Hour>" << fwCompTimeHour << "</Hour>"
        << "<Minute>" << fwCompTimeMinute << "</Minute>"
        << "<Second>" << fwCompTimeSecond << "</Second>"
        << "<Day>" << fwCompTimeDay << "</Day>"
        << "<Month>" << fwCompTimeMonth << "</Month>"
        << "<Year>" << fwCompTimeYear << "</Year>"
        << "</Compilation_Time>"
        << "<Logs>";

    outputFile << headerBuilder.str();
    return true;
}

void CloseOutputFile(ofstream& outputFile)
{
    outputFile << "</Logs></LogFile>";
    outputFile.close();
    DebugPrint("Output file closed\n");
}

int main(int argc, char *argv[])
{
    configFilePath = DEFAULT_CONFIG_FILE_NAME;
    if (argc == 2)
    {
            configFilePath = argv[1];
    }

    // Read configuration file
    OSReadConfigFile();

    size_t buffer_size_in_dwords = 0;
    if (TRACER_TYPE_FW == m_tracerType)
    {
        buffer_size_in_dwords = fw_log_buf_entries;
        log_file_prefix = "logFile_fw_";
    }
    else
    {
        //ucode
        buffer_size_in_dwords = ucode_log_buf_entries;
        log_file_prefix = "logFile_ucode_";
    }

    if (false) // TODO decide when to show help
    {
        help = 1;
    }

    if (help)
    {
        DisplayHelp();
    }

    OpenDevice();

    ReadDeviceInfo();

    SetModuleVerbosity();

    // Allocate log buffer
    void* log_buf = AllocateBuffer();

    long currentFileSize = 0;

    ofstream outputFile;

    bool status = CreateNewOutputFile(resultPath, outputFile);
    if (!status) return 1;

    while (true)
    {
        if (currentFileSize > fileFragmentSize)
        {
            currentFileSize = 0;
            CloseOutputFile(outputFile);
            status = CreateNewOutputFile(resultPath, outputFile);
            if (!status) return 1;
        }

        // Read the log
        if (!OSReadLog(log_buf, log_size(buffer_size_in_dwords)))
        {
            OSSleep(pollingInterval);
            continue;
        }

        currentFileSize += ParseLog(log_buf, buffer_size_in_dwords, outputFile);
        DebugPrint("Current File Size = %d\n", currentFileSize);

        OSSleep(pollingInterval);
    }

    return 0;
}
