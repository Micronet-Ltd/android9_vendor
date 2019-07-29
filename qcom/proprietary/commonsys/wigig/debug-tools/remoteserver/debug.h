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

#ifndef __debug_h
#define __debug_h

#include <iostream>

// Severity values are expected to raise from zero
enum LogSeverity
{
    LOG_SEV_ERROR   = 0,   // Unexpected input/events that may cause server misbehavior
    LOG_SEV_WARNING = 1,   // Suspicious events
    LOG_SEV_INFO    = 2,   // Events like command/response
    LOG_SEV_DEBUG   = 3,   // Detailed functionality
    LOG_SEV_VERBOSE = 4    // Excessive debug
};

#define TRACE_WITH_PREFIX(SEV)    \
    g_LogConfig.ShouldPrint(SEV) && std::cout << LogMsgPrefix(SEV, __FILE__, __LINE__)

#define LOG_ERROR   TRACE_WITH_PREFIX(LOG_SEV_ERROR)
#define LOG_WARNING TRACE_WITH_PREFIX(LOG_SEV_WARNING)
#define LOG_INFO    TRACE_WITH_PREFIX(LOG_SEV_INFO)
#define LOG_DEBUG   TRACE_WITH_PREFIX(LOG_SEV_DEBUG)
#define LOG_VERBOSE TRACE_WITH_PREFIX(LOG_SEV_VERBOSE)

// *************************************************************************************************

struct LogConfig
{
public:
    LogConfig(LogSeverity maxSeverity, bool bPrintLocation);
    void SetMaxSeverity(int traceLevel);

    bool ShouldPrint(LogSeverity sev) const { return sev <= m_MaxSeverity; }
    bool ShouldPrintLocation() const { return m_PrintLocation; }

private:

    LogSeverity m_MaxSeverity;
    const bool m_PrintLocation;

};

// *************************************************************************************************

extern LogConfig g_LogConfig;

// *************************************************************************************************

class LogMsgPrefix
{
    friend std::ostream& operator<<(std::ostream& os, const LogMsgPrefix& prefix);

public:

    LogMsgPrefix(LogSeverity severity, const char* pFile, int line)
        : Severity(severity), File(pFile), Line(line) {}

private:

    static const char* SeverityToString(LogSeverity sev);

    const LogSeverity Severity;
    const char* const File;
    const int Line;
};


// *************************************************************************************************
// Stream Formatters
// *************************************************************************************************

// Print a boolean value as a string
struct BoolStr
{
    explicit BoolStr(bool value): Value(value) {}
    const bool Value;
};

inline std::ostream& operator<<(std::ostream& os, const BoolStr& boolStr)
{
    return os << std::boolalpha << boolStr.Value << std::noboolalpha;
}

// *************************************************************************************************

// Print a string while displaying newline characters
struct PlainStr
{
    explicit PlainStr(const std::string& value): Value(value) {}
    const std::string& Value;
};

inline std::ostream& operator<<(std::ostream& os, const PlainStr& plainStr)
{
    for (std::string::const_iterator it = plainStr.Value.begin(); it != plainStr.Value.end(); ++it)
    {
        switch (*it)
        {
        case '\r': os << "\\r"; break;
        case '\n': os << "\\n"; break;
        case '\t': os << "\\t"; break;
        default:   os << *it; break;
        }
    }

    return os;
}


#endif
