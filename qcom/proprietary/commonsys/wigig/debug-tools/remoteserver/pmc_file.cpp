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

#include "pmc_file.h"
#include "debug.h"

#include <cstring>
#include <cerrno>
#include <sstream>
#include <stdlib.h>
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

const char* const PmcFile::s_pDirectory = PMC_DATA_DIRECTORY;
const char* const PmcFile::s_pFileNamePrefix = "pmc_data_";

// *************************************************************************************************

PmcFile::PmcFile(int fileId)
    : m_FileId(fileId)
{
    std::stringstream ss;
    ss << s_pDirectory << '/' << s_pFileNamePrefix << fileId;
    m_FileName = ss.str();

    LOG_DEBUG << "PMC file name #" << fileId << " generated: " << m_FileName << std::endl;
}

std::ostream& operator<<(std::ostream& os, const PmcFile& pmcFile)
{
    return os << "PMC file #" << pmcFile.GetFileId() << " (" << pmcFile.GetFileName() << ')';
}

// *************************************************************************************************

PmcFileWriter::PmcFileWriter(const PmcFile& pmcFile)
    : m_PmcFile(pmcFile)
{
}

// *************************************************************************************************

bool PmcFileWriter::CreateDirectoryIfNeeded() const
{
    // Create a PMC directory if does not exist
    struct stat st = {};
    if (stat(m_PmcFile.GetDirectory(), &st) != -1)
    {
        LOG_DEBUG << "PMC directory " << m_PmcFile.GetDirectory()
                  << " exists for " << m_PmcFile.GetFileName() << std::endl;
        return true;
    }

    LOG_DEBUG << "Creating a PMC directory: " << m_PmcFile.GetDirectory() << std::endl;

    int status = mkdir(m_PmcFile.GetDirectory(), 0700);
    if (0 != status)
    {
        int lastErrno = errno;
        LOG_ERROR << "Cannot create PMC directory."
                  << " Path: "    << m_PmcFile.GetDirectory()
                  << " Error:"    <<  lastErrno
                  << " Message: " << strerror(lastErrno)
                  << std::endl;

        return false;
    }

    return true;
}

// *************************************************************************************************

bool PmcFileWriter::WriteFile() const
{
    if (false == CreateDirectoryIfNeeded())
    {
        LOG_ERROR << "Cannot create a PMC directory for " << m_PmcFile << std::endl;
        return false;
    }

    // Create a PMC file
    const char* pCmdPrefix =
        "D=$(find /sys/kernel/debug/ieee80211/ -name wil6210); cat $D/pmcdata >> ";
    std::stringstream ss;
    ss << pCmdPrefix << m_PmcFile.GetFileName();

    system(ss.str().c_str());
    return true;

}

// *************************************************************************************************

size_t PmcFileWriter::GetFileSize() const
{
    FILE *pFile = fopen(m_PmcFile.GetFileName(), "r");

    if (NULL == pFile)
    {
        int lastErrno = errno;
        LOG_ERROR << "Cannot open " << m_PmcFile << " for writing."
                  << " Error: "   << lastErrno
                  << " Message: " << strerror(lastErrno)
                  << std::endl;
        return 0;
    }

    fseek (pFile, 0, SEEK_END);
    size_t fileSize = ftell(pFile);
    fclose(pFile);

    LOG_DEBUG << "Get PMC file size for " << m_PmcFile
              << ": " << fileSize << "B" << std::endl;

    return fileSize;
}
