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

#include "PmcData.h"
#include "DebugLogger.h"
#include "OperationStatus.h"

#include <cstring>
#include <cerrno>

#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>

#include <sys/stat.h>
#include <sys/types.h>

// *************************************************************************************************
#ifdef __ANDROID__
#define PMC_DATA_DIRECTORY "/data/pmc"
#else
#define PMC_DATA_DIRECTORY "/var/pmc"
#endif

// PMC directory and file name pattern should be managed separately as directory is required as a
// separate variable.

const char* const PmcDataFileLocator::s_pDirectory = PMC_DATA_DIRECTORY;
const char* const PmcDataFileLocator::s_pFileNamePrefix = "pmc_data_";

// *************************************************************************************************

PmcDataFileLocator::PmcDataFileLocator(int fileId)
    : m_FileId(fileId)
{
    std::stringstream ss;
    ss << s_pDirectory << '/' << s_pFileNamePrefix << fileId;
    m_FileName = ss.str();

    LOG_DEBUG << "PMC file name #" << fileId << " generated: " << m_FileName << std::endl;
}

bool PmcDataFileLocator::FileExists() const
{
    struct stat st = {};
    return (stat(m_FileName.c_str(), &st) != -1);
}

std::ostream& operator<<(std::ostream& os, const PmcDataFileLocator& pmcDataFileLocator)
{
    return os << "PMC file #" << pmcDataFileLocator.GetFileId()
              << " (" << pmcDataFileLocator.GetFileName() << ')';
}

// *************************************************************************************************

PmcDataFileWriter::PmcDataFileWriter(int fileId, const char* szDebugFsPath)
    : m_PmcDataFileLocator(fileId)
{
    if (!szDebugFsPath)
    {
        LOG_ERROR << "No Debug FS path is provided" << std::endl;
    }

    std::stringstream pathBuilder;
    pathBuilder << szDebugFsPath << '/' << "pmcdata";

    m_SrcPmcDataPath = pathBuilder.str();

    LOG_DEBUG << "PMC Data File Writer Created"
              << "\n    Debug FS Path: " << szDebugFsPath
              << "\n    Src PMC Data: " << m_SrcPmcDataPath
              << "\n    Dst PMC Data: " << m_PmcDataFileLocator.GetFileName()
              << std::endl;
}

// *************************************************************************************************

OperationStatus PmcDataFileWriter::MeetWritePrerequisites() const
{
    // Forbid file overwrite

    if (m_PmcDataFileLocator.FileExists())
    {
        std::stringstream msgBuilder;
        msgBuilder << "Destination PMC data file already exists: " << m_PmcDataFileLocator.GetFileName();
        return OperationStatus(false, msgBuilder.str().c_str());
    }

    // Create a PMC directory if does not exist

    struct stat st = {};
    if (stat(m_PmcDataFileLocator.GetDirectory(), &st) != -1)
    {
        LOG_DEBUG << "Found existing PMC data directory " << m_PmcDataFileLocator.GetDirectory() << std::endl;
        return OperationStatus(true);
    }

    LOG_DEBUG << "Creating a PMC data directory: " << m_PmcDataFileLocator.GetDirectory() << std::endl;

    int status = mkdir(m_PmcDataFileLocator.GetDirectory(), S_IRWXU);
    if (0 != status)
    {
        std::stringstream msgBuilder;
        msgBuilder << "Cannot create a PMC data directory "<< m_PmcDataFileLocator.GetDirectory()
                   << " Error: " << strerror(errno);
        return OperationStatus(false, msgBuilder.str().c_str());
    }

    return OperationStatus(true);
}

// *************************************************************************************************

OperationStatus PmcDataFileWriter::WriteFile() const
{
    OperationStatus st = MeetWritePrerequisites();
    if (!st.IsSuccess())
    {
        return st;
    }

    // Just to shorten expressions
    const char* srcFileName = m_SrcPmcDataPath.c_str();
    const char* dstFileName = m_PmcDataFileLocator.GetFileName();

    std::ifstream srcPmcData(srcFileName, std::ifstream::binary);
    if (!srcPmcData.is_open())
    {
        std::stringstream msgBuilder;
        msgBuilder << "Cannot open source PMC data " << srcFileName << ": " << strerror(errno);
        return OperationStatus(false, msgBuilder.str().c_str());
    }

    std::ofstream dstPmcData(dstFileName, std::ofstream::binary);
    if (!dstPmcData.is_open())
    {
        std::stringstream msgBuilder;
        msgBuilder << "Cannot open destination PMC data " << dstFileName << ": " << strerror(errno);
        return OperationStatus(false, msgBuilder.str().c_str());
    }

    // Buffered copy through user space is required as pmcdata does not support offset functionality
    // and therefore its size cannot be queried. As a result, sendfile() cannot be used.

    static const size_t DATA_COPY_BUFFER_LEN = 1024 * 1024;
    std::unique_ptr<char[]> spDataCopyBuffer(new char[DATA_COPY_BUFFER_LEN]);
    if (!spDataCopyBuffer)
    {
        std::stringstream msgBuilder;
        msgBuilder << "Cannot allocate data copy buffer of " << DATA_COPY_BUFFER_LEN << " B";
        return OperationStatus(false, msgBuilder.str());
    }

    std::streamsize dataSize = 0;
    while (srcPmcData && dstPmcData)
    {
        // Read a data chunk up to the buffer capacity
        srcPmcData.read(spDataCopyBuffer.get(), DATA_COPY_BUFFER_LEN);
        std::streamsize chunkSize = srcPmcData.gcount();
        LOG_VERBOSE << "Read chunk from pmcdata: " << chunkSize << " B" << std::endl;

        if (chunkSize > 0)
        {
            // Write the chunk
            dstPmcData.write(spDataCopyBuffer.get(), chunkSize);
            dataSize += chunkSize;
            LOG_VERBOSE << "Written PMC data chunk: " << chunkSize << " Accumulated: " << dataSize << std::endl;

            if (!dstPmcData)
            {
                std::stringstream msgBuilder;
                msgBuilder << "Cannot write PMC data " << dstFileName << ": " << strerror(errno);
                return OperationStatus(false, msgBuilder.str());
            }
        }

        // Check stop conditions
        if (srcPmcData.fail() && srcPmcData.eof())
        {
            // EoF reached
            LOG_DEBUG << "Source PMC data: EoF reached" << std::endl;
            LOG_DEBUG << "Written PMC data file: " << dstFileName << " Size: " << dataSize << " B" << std::endl;

            std::stringstream msgBuilder;
            msgBuilder << dataSize;
            return OperationStatus(true, msgBuilder.str());
        }

        if (!srcPmcData)
        {
            // Any non-EoF failure or I/O error
            std::stringstream msgBuilder;
            msgBuilder << "Cannot read PMC data from " << srcFileName << ": " << strerror(errno);
            return OperationStatus(false, msgBuilder.str());
        }
    }

    // The function flow should not get here
    return OperationStatus(false, "Unknown Error");
}
