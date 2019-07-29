/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef _LOG_COLLECTOR_RECORDER_H
#define _LOG_COLLECTOR_RECORDER_H
#pragma once

#include <string>
#include <vector>

class Device;

namespace log_collector
{
    class LogCollectorRecorder
    {
    public:
        LogCollectorRecorder(
            Device* device, CpuType tracerType, std::string debugLogPrefix,
            std::shared_ptr<LogCollectorConfigurator> m_configurator, log_buffer* logHeader);

        // recording methods
        size_t WriteToOutputFile(const std::string& logLine, bool debugPrint);
        OperationStatus HandleRecording();
        OperationStatus PrepareRecording();
        void StopRecording(); // Perform all required operations after collecting logs (e.g. close the log file if applicable)

        void IncreaseCurrentOutputFileSize(size_t outputFileSize);

    private:
        Device* m_device;
        std::string m_debugLogPrefix;
        std::shared_ptr<LogCollectorConfigurator> m_configurator;
        log_buffer* m_logHeader;
        std::string m_logErrorMessagePrefix;

        // output file methods
        std::string GetNextOutputFileFullName();
        bool CreateNewOutputFile();
        void CloseOutputFile();
        void RemoveOldFilesIfNeeded();
        long GetNumOfLogFilesInFolder() const;
        std::vector<std::string> GetLogFilesSorted() const;
        bool RemoveOldestLogFileFromSortestVector(std::vector<std::string>& logFiles);

        // output file
        std::ofstream m_outputFile;
        size_t m_currentOutputFileSize = 0;
        uint32_t m_currentNumOfOutputFiles = 0;
        const std::string m_logFilePrefix;
        const std::string m_logFilesFolder;
        const std::string m_logFileExtension;
    };
}

#endif // !_LOG_COLLECTOR_RECORDER_H