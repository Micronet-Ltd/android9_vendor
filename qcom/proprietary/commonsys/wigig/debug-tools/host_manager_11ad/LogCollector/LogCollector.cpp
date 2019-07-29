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

#include <locale> // std::tolower
#include <sstream>
#include <iostream>
#include <map>
#include <algorithm>

#include "LogCollector.h"
#include "Device.h"
#include "Host.h"

namespace log_collector
{

    LogCollector::LogCollector(Device* device, CpuType tracerType, bool publishLogs, bool recordLogs) :
        m_configurator(new LogCollectorConfigurator(device->GetDeviceName(), tracerType)),
        m_device(device),
        m_deviceName(device->GetDeviceName()),
        m_tracerType(tracerType),
        m_isCollecting(false),
        m_isRecording(false),
        m_isPublishing(false),
        m_debugPrint(false),
        m_logHeader(new log_buffer()),
        m_logAddress(0),
        m_rptr(0),
        m_lastWptr(0)
    {
        if (CPU_TYPE_FW == m_tracerType)
        {
            m_debugLogPrefix = m_deviceName + " FW tracer: ";
        }
        else
        {
            m_debugLogPrefix = m_deviceName + " uCode tracer: ";
        }

        m_logCollectorRecorder.reset(new LogCollectorRecorder(device, tracerType, m_debugLogPrefix, m_configurator, m_logHeader.get()));

        ComputeLogBufferStartAddress();
        m_logBufferContentSizeInDwords = GetLogBufferContentSizeInDwords();
        m_logBufferContentSizeInBytes = GetLogBufferContentSizeInBytes();
        m_logTotalBufferSizeInBytes = GetTotalLogBufferSizeInBytes();
        m_log_buf.resize(m_logTotalBufferSizeInBytes);
        GetModuleInfoFromDevice();

        OperationStatus os = SetPublishingMode(publishLogs);
        if (!os)
        {
            LOG_ERROR << os.GetStatusMessage() << endl;
        }
        os = SetRecordingMode(recordLogs);
        if (!os)
        {
            LOG_ERROR << os.GetStatusMessage() << endl;
        }

        m_pollerTaskId = Host::GetHost().GetTaskScheduler().RegisterTask(
            std::bind(&LogCollector::Poll, this), m_configurator->m_configuration->GetPollingIntervalMs(), true);
    }

    LogCollector::~LogCollector()
    {
        Host::GetHost().GetTaskScheduler().UnregisterTask(m_pollerTaskId);
    }

    void LogCollector::Poll()
    {
        lock_guard<mutex> lock(m_device->m_mutex);

        if (m_device->GetSilenceMode() || (!m_isRecording && !m_isPublishing))
        {
            // In this case collection is not needed (device is silent or nor recording nor publishing)
            return;
        }

        vector<log_collector::RawLogLine> rawLogLines;
        vector<string> errorMessages;

        GetNextLogs(rawLogLines, errorMessages);

        PublishLogsIfNeeded(rawLogLines);

        if (errorMessages.size() > 0)
        {
            //events.emplace_back(new...) // TODO - create error Message events
        }
    }

    void LogCollector::PublishLogsIfNeeded(const vector<log_collector::RawLogLine>& rawLogLines)
    {
        if (m_isPublishing && rawLogLines.size() > 0)
        {
            Host::GetHost().PushEvent(NewLogsEvent(m_device->GetDeviceName(), m_tracerType, rawLogLines));
        }
    }

    size_t LogCollector::WriteToFileIfNeeded(const string& logLine, bool debugPrint)
    {
        if (m_isRecording)
        {
            return m_logCollectorRecorder->WriteToOutputFile(logLine, debugPrint);
        }
        return 0; // nothing was written, therefore size 0
    }

    bool LogCollector::GetNextLogs(vector<RawLogLine>& rawLogLines, vector<string>& errorMessages)
    {

        string errorMsg;

        if (m_isRecording)
        {
            OperationStatus os = m_logCollectorRecorder->HandleRecording();
            if (!os)
            {
                errorMessages.push_back(os.GetStatusMessage());
            }
        }

        // Read log buffer
        if (ReadLogWritePointer(m_log_buf.data()))
        {
            m_logCollectorRecorder->IncreaseCurrentOutputFileSize(ParseLog(rawLogLines));
        }
        // else flow is still valid and happens when the interface isn't up yet

        return true;
    }

    OperationStatus LogCollector::SubmitConfiguration(const LogCollectorConfiguration& newConfiguration)
    {
        bool isPollingChanged = m_configurator->m_configuration->GetPollingIntervalMs() != newConfiguration.GetPollingIntervalMs();
        *(m_configurator->m_configuration) = newConfiguration;
        if (isPollingChanged)
        {
            Host::GetHost().GetTaskScheduler().ChangePollingInterval(m_pollerTaskId, m_configurator->m_configuration->GetPollingIntervalMs());
        }
        m_configurator->Save();
        return OperationStatus(true);
    }

    bool LogCollector::SetParamerter(const std::string& assignment)
    {
        bool valueWasChanged = false;
        bool res = m_configurator->SetParameterAssignment(assignment, valueWasChanged);
        ApplyModuleVerbosityIfNeeded();

        // TODO: this is a quick and dirty fix for applying polling interval changes. Need to add notification mechanism.
        if (valueWasChanged && std::string::npos != assignment.find(POLLING_INTERVAL_MS))
        {
            Host::GetHost().GetTaskScheduler().ChangePollingInterval(m_pollerTaskId, m_configurator->m_configuration->GetPollingIntervalMs());
        }
        return res;
    }

    OperationStatus LogCollector::SetModuleVerbosity(const std::string& module, const std::string& level)
    {
        OperationStatus os = m_configurator->SetModuleVerbosity(module, level);
        ApplyModuleVerbosityIfNeeded();
        return os;
    }

    size_t LogCollector::ParseLog(std::vector<RawLogLine>& rawLogLines)
    {
        size_t sizeAdded = 0;

        // Re-Read the entire buffer + update m_logaddress and m_log_buf
        ReadLogBuffer();
        // Prepare a header pointing to log buffer top
        struct log_buffer *h = (struct log_buffer*)m_log_buf.data(); // h contains all the log buffer (including the log buffer header)

        unsigned wptr = h->write_ptr;

        if (wptr == m_rptr)
        {
            // Nothing to read.
            return 0;
        }

        sizeAdded += WriteToFileIfNeeded(Utils::GetCurrentLocalTimeXml() + "<Content>", false);
        if (wptr < m_lastWptr) // device was restarted
        {
            // previously was just retun 0
            LOG_WARNING << m_debugLogPrefix << "device was restarted (wptr < m_rptr: wptr == " << wptr << " , rptr == " << m_rptr << ")" << endl;
            sizeAdded += WriteToFileIfNeeded("device was restarted", true);
            rawLogLines.push_back(RawLogLine(RawLogLine::DEVICE_WAS_RESTARTED, wptr));
            m_rptr = 0;
            m_lastWptr = 0;
        }

        if (wptr < m_rptr) // read more lines than were created
        {
            // previously was just retun 0
            LOG_WARNING << m_debugLogPrefix << "read more lines than were created (wptr < m_rptr: wptr == " << wptr << " , rptr == " << m_rptr << ")" << endl;
            sizeAdded += WriteToFileIfNeeded("read more lines than were created", true);
            sizeAdded += WriteToFileIfNeeded("</Content></Log_Content>", false);
            rawLogLines.push_back(RawLogLine(RawLogLine::RPTR_LARGER_THAN_WPTR, wptr));
            return 0;
        }

        if (wptr > m_logBufferContentSizeInDwords + m_rptr)
        {
            // overflow; try to parse last wrap
            stringstream missedDwords;
            missedDwords << wptr - m_rptr - m_logBufferContentSizeInDwords;
            sizeAdded += WriteToFileIfNeeded("found buffer overrun - missed DWORDS: " + missedDwords.str(), true); // DWORD == uint32
            rawLogLines.push_back(RawLogLine(RawLogLine::BUFFER_OVERRUN, (unsigned)(wptr - m_rptr - m_logBufferContentSizeInDwords))); // condition assures that the value is non-negative integer
            m_rptr = wptr - (unsigned)m_logBufferContentSizeInDwords;
        }

        LOG_DEBUG << m_debugLogPrefix << "  wptr = " << wptr << ", rptr = " << m_rptr << endl;

        for (; wptr > m_rptr && (wptr != m_lastWptr); m_rptr++)
        {
            LOG_DEBUG << m_debugLogPrefix << "wptr = " << wptr << ", rptr = " << m_rptr << endl;

            union log_event *evt = &h->evt[m_rptr % m_logBufferContentSizeInDwords]; // h->evt is the log line payload.

            if (evt->hdr.signature != 5)
            {
                sizeAdded += WriteToFileIfNeeded("got corrupted entry", true);
                rawLogLines.push_back(RawLogLine(RawLogLine::INVALID_SIGNATURE, 1));
                continue;
            }
            unsigned dword_num = 4 * evt->hdr.dword_num_msb + evt->hdr.dword_num_lsb;

            //TODO-Omri: change to constant!!
            if (dword_num > 14)
            {
                LOG_DEBUG << m_debugLogPrefix << "Parameter Num = " << dword_num << endl;
                continue;
            }

            vector<int> params(dword_num); // parameters array (each log line can have at most three parameters). size need to be set to 3 due to log collector's old output format (each line has three params where dummy params has the value of 0)
            for (unsigned i = 0; i < dword_num; i++)
            {
                params[i] = (h->evt[(m_rptr + i + 1) % m_logBufferContentSizeInDwords].param);
            }

            stringstream parsedLogLine;
            int string_ofst = evt->hdr.strring_offset << 2;
            parsedLogLine << evt->hdr.module << "," << levels[evt->hdr.level] << "," << string_ofst << ":";
            unsigned i;
            for (i = 0; i < dword_num; i++)
            {
                parsedLogLine << params[i];
                if (i < dword_num - 1)
                {
                    parsedLogLine << ",";
                }
            }

            // for backward compatibility - to enable parsing logs collected by new host_manager by old DmTools.
            // put '0' to pad up to 3 '0's (dwords)
            for (; i < 3; i++)
            {
                if (i > 0)
                {
                    parsedLogLine << ",";
                }
                parsedLogLine << 0;
            }
            parsedLogLine << endl;
            sizeAdded += WriteToFileIfNeeded(parsedLogLine.str(), false);

            RawLogLine line = RawLogLine(evt->hdr.module, evt->hdr.level, evt->hdr.signature, string_ofst, evt->hdr.is_string, params);

            // Gather new line for event
            rawLogLines.push_back(line);

            m_rptr += dword_num;
        }

        m_lastWptr = wptr;

        sizeAdded += WriteToFileIfNeeded("</Content></Log_Content>", false);

        fflush(stdout);
        return sizeAdded;
    }

    //void LogCollector::RearrangeBuffer(unsigned pivot)
    //{
    //    unsigned pivotInBuffer = pivot % fw_log_buf_entries_in_dwords;
    //    unsigned char tempLogBuffer[sizeof(m_log_buf)];
    //    memcpy(m_log_buf + pivotInBuffer, tempLogBuffer, fw_log_buf_entries_in_dwords - pivotInBuffer);
    //    memcpy(m_log_buf, tempLogBuffer + (fw_log_buf_entries_in_dwords - pivotInBuffer), pivotInBuffer);
    //    memcpy(tempLogBuffer, m_log_buf, log_buffer_total_size);
    //}

    OperationStatus LogCollector::SetRecordingMode(bool doRecording)
    {
        if (m_isRecording == doRecording)
        {
            //return OperationStatus(false, "LogCollector is already " + string(doRecording ? "" : "not ") + "recording");
            return OperationStatus(true);
        }

        if (doRecording)
        {
            OperationStatus os = m_logCollectorRecorder->PrepareRecording();
            if (!os)
            {
                return os;
            }

            os = StartCollectIfNeeded();
            if (!os)
            {
                return os;
            }
        }
        else
        {
            m_logCollectorRecorder->StopRecording();
        }

        // This update for the state should be last in the function
        m_isRecording = doRecording;
        return OperationStatus(true);
    }

    OperationStatus LogCollector::SetPublishingMode(bool publishMode)
    {
        if (m_isPublishing == publishMode)
        {
            //return OperationStatus(false, "LogCollector is already " + string(publishMode ? "" : "not ") + "publishing");
            return OperationStatus(true);
        }

        if (publishMode)
        {
            OperationStatus os = StartCollectIfNeeded();
            if (!os)
            {
                return os;
            }
        }

        // This update for the state should be last in the function
        m_isPublishing = publishMode;
        return OperationStatus(true);
    }

    OperationStatus LogCollector::StartCollectIfNeeded()
    {
        if (m_isRecording || m_isPublishing)
        {
            return OperationStatus(true);
        }

        // setting configuration
        OperationStatus os = m_configurator->Load();
        if (!os)
        {
            return os;
        }

        // Set the module verbosity
        os = ApplyModuleVerbosity();
        if (!os)
        {
            return os;
        }
        return OperationStatus(true);
    }

    // TODO : after finishing refactoring, it has to be removed
    void LogCollector::SetCollectionMode(bool collect)
    {
        if (!m_isCollecting && collect)
        {
            // setting configuration
            m_configurator->Load(); // TODO - do we need to inform the user about a failure? how?

            //// Set FW/uCode which logs to collect
            ApplyModuleVerbosity(); // TODO - do we need to inform the user about a failure? how?

            m_isCollecting = true;
        }
        else if (m_isCollecting && !collect)
        {
            if (m_isRecording)
            {
                m_logCollectorRecorder->StopRecording();
            }
            m_isCollecting = false;
        }
    }

    bool LogCollector::IsRecording() const
    {
        return m_isRecording;
    }

    bool LogCollector::IsPublishing() const
    {
        return m_isPublishing;
    }

    // TODO : after finishing refactoring, it has to be removed
    bool LogCollector::SetRecordingFlag(bool isRecordedNeeded)
    {

        if (m_isRecording && !isRecordedNeeded) // Asked to stop log collection
        {
            m_logCollectorRecorder->StopRecording();
            m_isRecording = false;
        }
        else if (!m_isRecording && isRecordedNeeded) // Asked to start log recording
        {
            if (!m_logCollectorRecorder->PrepareRecording())
            {
                return false;
            }
            SetCollectionMode(true); // start recording implies start collecting
            m_isRecording = true;
        }
        return true;
    }

    bool LogCollector::ApplyModuleVerbosityIfNeeded()
    {
        bool neededUpdate = false;
        for (int i = 0; i < NUM_MODULES; ++i)
        {
            module_level_info m;
            if (!m_configurator->GetModuleVerbosity(i, m)) // shouldn't happen
            {
                LOG_ERROR << m_debugLogPrefix << "Failed to get verbosity for module number " << i << endl;
                continue;
            }
            if (m != m_logHeader->module_level_info_array[i])
            {
                neededUpdate = true;
                break;
            }
        }

        if (neededUpdate)
        {
            return ApplyModuleVerbosity();
        }
        return true;
    }


    bool LogCollector::GetModuleInfoFromDevice()
    {
        if (!m_device->GetDriver()->ReadBlock(m_logAddress + sizeof(m_logHeader->write_ptr), sizeof(m_logHeader->module_level_info_array), (char*)m_logHeader->module_level_info_array))
        {
            LOG_ERROR << __FUNCTION__ << "," << __LINE__ << m_debugLogPrefix << "Failed to read module verbosity structure for "
                << CPU_TYPE_TO_STRING[m_tracerType] << "(address 0x" << hex << m_logAddress + sizeof(m_logHeader->write_ptr) << ", data "
                << ((char*)m_logHeader->module_level_info_array) << ", size " << sizeof(m_logHeader->module_level_info_array) << ")" << endl;
            return false;
        }
        for (int i = 0; i < NUM_MODULES; i++) {
            LOG_DEBUG << "i is:" << i << "module_level_info_array[i] is :" << m_logHeader->module_level_info_array[i].third_party_mode << endl;
        };
        return true;
    }



    OperationStatus LogCollector::ApplyModuleVerbosity()
    {
        LOG_DEBUG << m_debugLogPrefix << "Apply module verbosity" << endl;
        // Update FW & uCode log addresses
        //if (!m_logAddress) // TODO: if this line isn't a comment, we have an issue with run_sta since the BL updates the offset with its value
        {
            ComputeLogBufferStartAddress();
        }

        //LOG_INFO << __FUNCTION__ << "," << __LINE__ << m_debugLogPrefix << "Log buffer start address is 0x" << hex << m_logAddress << endl; // TODO - remove

        //set the m_logHeader->module_level_info_array[i] from the device
        GetModuleInfoFromDevice();

        // Write verbosity to the device
        for (int i = 0; i < NUM_MODULES; ++i)
        {
            if (!m_configurator->GetModuleVerbosity(i, m_logHeader->module_level_info_array[i]))
            {
                LOG_ERROR << __FUNCTION__ << "," << __LINE__ << m_debugLogPrefix << "Should not happen: got index out of boundary (i == " << i << ")" << endl;
            }
        }

        if (!m_device->GetDriver()->WriteBlock(m_logAddress + sizeof(m_logHeader->write_ptr), sizeof(m_logHeader->module_level_info_array), (char*)m_logHeader->module_level_info_array))
        {
            stringstream st;
            st << "," << __LINE__ << m_debugLogPrefix << "Failed to write module verbosity structure for "
                << CPU_TYPE_TO_STRING[m_tracerType] << "(address 0x" << hex << m_logAddress + sizeof(m_logHeader->write_ptr) << ", data "
                << ((char*)m_logHeader->module_level_info_array) << ", size " << sizeof(m_logHeader->module_level_info_array) << ")";
            LOG_ERROR << __FUNCTION__ << st.str() << endl;
            return OperationStatus(false, st.str());
        }
        else
        {
            LOG_DEBUG << __FUNCTION__ << "," << __LINE__ << m_debugLogPrefix << "Module verbosity for " << CPU_TYPE_TO_STRING[m_tracerType]
                << "was set to " << m_configurator->GetAllModulesVerbosityString() << endl;
        }
        return OperationStatus(true);
    }

    bool LogCollector::ReadLogWritePointer(void* logBuffer)
    {
        // Update FW & uCode log addresses
        //if (!m_logAddress)  // TODO: if this line isn't a comment, we have an issue with run_sta since the BL updates the offset with its value
        {
            ComputeLogBufferStartAddress();
        }

        // Read the write pointer
        DWORD writePointer = 0;
        if (m_logAddress && !m_device->GetDriver()->Read(m_logAddress, writePointer))
        {
            return false;
        }

        // Set the write pointer to the buffer
        memcpy(logBuffer, &writePointer, sizeof(writePointer)); // TODO - is this necessary? does it do anything? what is the meaning of life?
        return true;
    }

    bool LogCollector::ReadLogBuffer()
    {
        // Update FW & uCode log addresses add put it in m_logAddress
        // if (!m_logAddress)  // TODO: if this line isn't a comment, we have an issue with run_sta since the BL updates the offset with its value
        {
            ComputeLogBufferStartAddress();
        }

        // Read the actual log
        if (m_logAddress && !m_device->GetDriver()->ReadBlock(m_logAddress, (DWORD)m_logTotalBufferSizeInBytes, (char*)m_log_buf.data()))
        {
            return false;
        }

        return true;
    }

    unsigned LogCollector::GetAhbStartAddress(BasebandType bb)
    {
        if (CPU_TYPE_FW == m_tracerType)
        {
            auto it = baseband_to_peripheral_memory_start_address_ahb.find(bb);
            if (baseband_to_peripheral_memory_start_address_ahb.end() == it)
            {
                LOG_ERROR << m_debugLogPrefix << "Log collector doesn't support baseband type (type code is " << bb << ")" << endl;
                return false;
            }
            return it->second;
        }
        else
        {
            auto it = baseband_to_ucode_dccm_start_address_ahb.find(bb);
            if (baseband_to_ucode_dccm_start_address_ahb.end() == it)
            {
                LOG_ERROR << m_debugLogPrefix << "Log collector doesn't support baseband type (type code is " << bb << ")" << endl;
                return false;
            }
            return it->second;
        }
    }

    unsigned LogCollector::GetLinkerStartAddress(BasebandType bb)
    {
        if (CPU_TYPE_FW == m_tracerType)
        {
            auto it = baseband_to_peripheral_memory_start_address_linker.find(bb);
            if (baseband_to_peripheral_memory_start_address_linker.end() == it)
            {
                LOG_ERROR << m_debugLogPrefix << "Log collector doesn't support baseband type (type code is " << bb << ")" << endl;
                return false;
            }
            return it->second;
        }
        else
        {
            auto it = baseband_to_ucode_dccm_start_address_linker.find(bb);
            if (baseband_to_ucode_dccm_start_address_linker.end() == it)
            {
                LOG_ERROR << m_debugLogPrefix << "Log collector doesn't support baseband type (type code is " << bb << ")" << endl;
                return false;
            }
            return it->second;
        }
    }

    //TOD-Omri: check if can be called once and be kept as member.
    bool LogCollector::ComputeLogBufferStartAddress()
    {
        // retrieve BB type (for getting ahb start address)
        BasebandType bb = BASEBAND_TYPE_NONE;
        bb = m_device->GetBasebandType();

        // get ahb start address
        unsigned ahb_start_address = GetAhbStartAddress(bb);

        // get linker start address
        unsigned linker_start_address = GetLinkerStartAddress(bb);

        // calculate the difference between the buffer start address as driver exposes it to the buffer start address as fw/ucode exposes it
        unsigned ahbToLinkerDelta = ahb_start_address - linker_start_address;

        // retrieve fw/ucode offset from peripheral_memory_start_linker_address
        DWORD addressToReadLogOffset = logTracerTypeToLogOffsetAddress.at(m_tracerType);
        DWORD logBufferInfoDword = 0;

        if (!m_device->GetDriver()->Read(addressToReadLogOffset, logBufferInfoDword))
        {
            LOG_ERROR << m_debugLogPrefix << "Log collector failed to read log offset address" << endl;
            return false;
        }

        /* calculate first address of fw/ucode log buffer
                              +--------------------------+----------------+
         logBufferInfoDword = |3bits of buffer size index|29bits of offset|
                              +--------------------------+----------------+
        */
        int logOffset = logBufferInfoDword & 0x1FFFFFFF; // we take only the lower 29 bit
        m_logAddress = ahbToLinkerDelta + logOffset; // calculate the lob buffer start address.

        //we want to determine the buffer size:
        int bufferSizeId = (logBufferInfoDword & 0xE0000000) >> 29; /* The three upper bits are the index in the size table */
                                                                    /* explained in GetBufferSizeById */
        int bufferSizeDW = GetBufferSizeById(bufferSizeId) / sizeof(DWORD);
        m_bufferSize = bufferSizeDW;
        return true;
    }

    int LogCollector::GetBufferSizeById(int bufferSizeId)
    {
        /*
        * The 3 MSBs of the value in REG_FW_USAGE_1 (for fw) and REG_FW_USAGE_2 (for uCode) are used to determine the size
        * of the their log buffer (respectively). The number created with these 3 MSBs is an index in the following sizes table:
        * 0 - default (4KB for FW, 1KB for uCode) - for backward compatibility
        * 1 - 1K
        * 2 - 2K
        * 3 - 4K
        * 4 - 8K
        * 5 - 16K
        * 6 - 32K
        * 7 - 64K
        */
        const int bufferSizeMultiplier[] = { 0, 1, 2, 4, 8, 16, 32, 64 };
        int result;
        const int KB = 1024;

        if (bufferSizeId < 0 || bufferSizeId > 7)
        {
            LOG_ERROR << "Could not get buffer size from logs header roll back to default size (CPU_TYPE_FW = 1024, CPU_TYPE_UCODE = 256)" << endl;
            bufferSizeId = 0;
        }

        if (bufferSizeId == 0)
        { // default values, for backward compatibility
            result = (m_tracerType == CPU_TYPE_FW) ? 4 * KB : 1 * KB;
        }
        else
        {
            result = bufferSizeMultiplier[bufferSizeId] * KB; // buffer size id is calculated by 3 bits only, so its range is 0-7. Therefore can't be out of range.
        }

        return result;
    }


    size_t LogCollector::GetLogBufferContentSizeInDwords()
    {
        // m_bufferSize should be set in ComputeLogBufferStartAddress - that happens in the constructor.
        return m_bufferSize;
    }

    size_t LogCollector::GetLogBufferContentSizeInBytes()
    {
        // m_bufferSize should be set in ComputeLogBufferStartAddress - that happens in the constructor.
        return m_bufferSize * 4;
    }

    size_t LogCollector::GetTotalLogBufferSizeInBytes()
    {
        return sizeof(struct log_buffer) + GetLogBufferContentSizeInBytes();
    }
}