/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include <algorithm>

#include "Host.h"
#include "LogCollector/LogCollectorRecorder.h"

namespace log_collector
{
    using namespace std;

    LogCollectorRecorder::LogCollectorRecorder(
        Device* device, CpuType tracerType, std::string debugLogPrefix,
        std::shared_ptr<LogCollectorConfigurator> configurator, log_buffer* logHeader) :
        m_device(device),
        m_debugLogPrefix(debugLogPrefix),
        m_configurator(configurator),
        m_logHeader(logHeader),
        m_currentOutputFileSize(0),
        m_currentNumOfOutputFiles(0),
        m_logFilePrefix(device->GetDeviceName() + "_" + CPU_TYPE_TO_STRING[tracerType] + "_"),
        m_logFilesFolder(FileSystemOsAbstraction::GetOutputFilesLocation() + "Logs" + FileSystemOsAbstraction::GetDirectoriesDilimeter()),
        m_logFileExtension(".log")
    {
        stringstream ss;
        ss << endl << "host_manager_11ad error: " << CPU_TYPE_TO_STRING[tracerType] << " log tracer error in device " << device->GetDeviceName() << " - ";
        m_logErrorMessagePrefix = ss.str();

        if (!FileSystemOsAbstraction::DoesFolderExist(m_logFilesFolder) && !FileSystemOsAbstraction::CreateFolder(m_logFilesFolder))
        {
            LOG_ERROR << m_debugLogPrefix << "Failed to create logs folder " << m_logFilesFolder << endl;
        }
    }

    void LogCollectorRecorder::StopRecording()
    {
        CloseOutputFile();
    }

    OperationStatus LogCollectorRecorder::PrepareRecording()
    {
        m_currentOutputFileSize = 0;
        m_currentNumOfOutputFiles = GetNumOfLogFilesInFolder();
        if (!CreateNewOutputFile())
        {
            LOG_ERROR << m_debugLogPrefix << "failed to create the first output file" << endl;
            return OperationStatus(false, m_debugLogPrefix + "failed to create the first output file");
        }

        LOG_DEBUG << m_debugLogPrefix << "Log collector preparation was successfully finished" << endl;
        return OperationStatus(true);
    }

    OperationStatus LogCollectorRecorder::HandleRecording()
    {
        size_t maxSingleFileSizeInByte = m_configurator->m_configuration->GetMaxSingleFileSizeMb() * 1024 * 1024;
        // split output file if required
        if (maxSingleFileSizeInByte > 0 && m_currentOutputFileSize > maxSingleFileSizeInByte)
        {
            CloseOutputFile();
            RemoveOldFilesIfNeeded();
            if (!CreateNewOutputFile())
            {
                LOG_ERROR << m_debugLogPrefix << "failed to create a new output file" << endl;
                return OperationStatus(false, m_debugLogPrefix + "failed to create a new output file");
            }
        }
        return OperationStatus(true);
    }

    void LogCollectorRecorder::IncreaseCurrentOutputFileSize(size_t outputFileSize)
    {
        m_currentOutputFileSize += outputFileSize;
        LOG_DEBUG << m_debugLogPrefix << "Current File Size = " << m_currentOutputFileSize << endl;
    }

    string LogCollectorRecorder::GetNextOutputFileFullName()
    {
        std::stringstream ss;

        ss << m_logFilesFolder << m_logFilePrefix << Utils::GetCurrentLocalTimeForFileName()
            << "_" << m_configurator->m_configuration->GetOutputFileSuffix() << m_logFileExtension;
        return ss.str();
    }

    bool LogCollectorRecorder::CreateNewOutputFile()
    {
        string fullFileName = GetNextOutputFileFullName();
        LOG_INFO << m_debugLogPrefix << "Creating output file: " << fullFileName << endl;

        m_outputFile.open(fullFileName.c_str());

        if (m_outputFile.fail())
        {
            LOG_ERROR << m_debugLogPrefix << "Error opening output file: " << fullFileName << " Error: " << strerror(errno) << endl;
            return false;
        }

        m_currentNumOfOutputFiles++;

        std::ostringstream headerBuilder;

        const FwVersion fwVersion = m_device->GetFwVersion();
        const FwTimestamp fwTs = m_device->GetFwTimestamp();

        headerBuilder << "<LogFile>"
            << "<FW_Ver>"
            << "<Major>" << fwVersion.m_major << "</Major>"
            << "<Minor>" << fwVersion.m_minor << "</Minor>"
            << "<Sub>" << fwVersion.m_subMinor << "</Sub>"
            << "<Build>" << fwVersion.m_build << "</Build>"
            << "</FW_Ver>"
            << "<Compilation_Time>"
            << "<Hour>" << fwTs.m_hour << "</Hour>"
            << "<Minute>" << fwTs.m_min << "</Minute>"
            << "<Second>" << fwTs.m_sec << "</Second>"
            << "<Day>" << fwTs.m_day << "</Day>"
            << "<Month>" << fwTs.m_month << "</Month>"
            << "<Year>" << fwTs.m_year << "</Year>"
            << "</Compilation_Time>"
            << "<Third_Party_Flags>";
        // make a list of the Third_party_flag for each module.
        for (int i = 0; i < NUM_MODULES; i++)
        {
            headerBuilder << "<Flag><value>" << static_cast<int>(m_logHeader->module_level_info_array[i].third_party_mode) << "</value></Flag>";
        }
        headerBuilder << "</Third_Party_Flags>"
            << "<Logs>";


        m_outputFile << headerBuilder.str();
        return true;
    }

    void LogCollectorRecorder::CloseOutputFile()
    {
        if (m_outputFile.is_open())
        {
            m_outputFile << "</Logs></LogFile>";
            m_outputFile.close();
            LOG_INFO << m_debugLogPrefix << "Output file closed" << endl;
        }
        m_currentOutputFileSize = 0;
    }

    void LogCollectorRecorder::RemoveOldFilesIfNeeded()
    {
        if (0 == m_configurator->m_configuration->GetMaxNumOfLogFiles())
        {
            // no need to remove old files
            return;
        }

        int numOfRemovedLogFiles = 0;
        vector<string> logFiles = GetLogFilesSorted();

        while (logFiles.size() > 0 &&
            (m_currentNumOfOutputFiles >= m_configurator->m_configuration->GetMaxNumOfLogFiles()))
        {
            if (!RemoveOldestLogFileFromSortestVector(logFiles))
            {
                LOG_WARNING << m_debugLogPrefix << "Skipping old log files removal for tracer" << endl;
                break;
            }
            numOfRemovedLogFiles++;
            m_currentNumOfOutputFiles = GetNumOfLogFilesInFolder();
        }
        LOG_DEBUG << m_debugLogPrefix << "Removed " << numOfRemovedLogFiles << " oldest log files from the folder. Current tracer's log file size in folder: " << m_currentNumOfOutputFiles << endl;
    }

    bool LogCollectorRecorder::RemoveOldestLogFileFromSortestVector(vector<string>& logFiles)
    {
        if (logFiles.size() <= 0)
        {
            return false;
        }
        LOG_VERBOSE << m_debugLogPrefix << "Remove old log file: " << (*logFiles.begin()).c_str() << endl;
        if (std::remove((m_logFilesFolder + *logFiles.begin()).c_str()))
        {
            LOG_WARNING << m_debugLogPrefix << "Failed to remove old log file " << m_logFilesFolder << (*logFiles.begin()).c_str() << " error: " << std::strerror(errno) << endl;
            return false;
        }
        logFiles.erase(logFiles.begin());
        return true;
    }

    vector<string> LogCollectorRecorder::GetLogFilesSorted() const
    {
        vector<string> files = FileSystemOsAbstraction::GetFilesByPattern(m_logFilesFolder, m_logFilePrefix, m_logFileExtension);
        std::sort(files.begin(), files.end());
        return files;
    }

    long LogCollectorRecorder::GetNumOfLogFilesInFolder() const
    {
        return FileSystemOsAbstraction::GetFilesByPattern(m_logFilesFolder, m_logFilePrefix, m_logFileExtension).size();
    }

    size_t LogCollectorRecorder::WriteToOutputFile(const string& logLine, bool debugPrint)
    {
        size_t writtenSize = 0;
        if (m_outputFile.is_open())
        {
            if (debugPrint)
            {
                m_outputFile << m_logErrorMessagePrefix;
                writtenSize += m_logErrorMessagePrefix.size();
            }
            m_outputFile << logLine;
            writtenSize += logLine.size();
            if (debugPrint)
            {
                m_outputFile << endl;
            }
        }
        return writtenSize;
    }
}
