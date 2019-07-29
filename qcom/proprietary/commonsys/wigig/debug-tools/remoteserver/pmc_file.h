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

#ifndef _PMC_FILE_H_
#define _PMC_FILE_H_

#include <stdio.h>
#include <string>

// *************************************************************************************************

// Generates a PMC file name from its ID.
class PmcFile
{
public:

    explicit PmcFile(int fileId);

    int GetFileId() const { return m_FileId; }
    const char* GetFileName() const { return m_FileName.c_str(); }

    static const char* GetDirectory() { return s_pDirectory; }

private:

    static const char* const s_pDirectory;
    static const char* const s_pFileNamePrefix;

    const int m_FileId;        // File ID (expected to be unique)
    std::string m_FileName;    // File Name Buffer

};

std::ostream& operator<<(std::ostream& os, const PmcFile& pmcFile);

// *************************************************************************************************

// Creates a PMC data file according to a provided ID.
class PmcFileWriter
{
public:

    explicit PmcFileWriter(const PmcFile& pmcFile);

    bool WriteFile() const;
    size_t GetFileSize() const;

private:

    bool CreateDirectoryIfNeeded() const;

    const PmcFile& m_PmcFile;

};


#endif    // PMC_FILE_H_
