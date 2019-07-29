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


#include "ArgumentsParser.h"
#include "HostInfo.h"
#include "Host.h"


#include <cstdio>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;
// *************************************************************************************************

bool ArgumentsParser::ParseAndHandleArguments(int argc, char * argv[])
{
    for (int i = 0; i < argc; i++)
    {
        m_arguments.push_back(string(argv[i]));
    }
    if (DoesArgumentExist("-v"))
    { //Argument for the version of host_manager_11ad.
      // No need to run host manager
        printf("Host Manager 11ad version: %s\n", HostInfo::GetVersion().c_str());
        return false;
    }
    if (DoesArgumentExist("-p"))
    { //Argument for setting the port of the commands TCP port

    }
    if (DoesArgumentExist("-statusBar"))
    { //Argument for setting the textual status bar display
        printf("Showing Status Bar Periodically");
        g_LogConfig.SetStatusBarPrinter(true);
    }
    if (DoesArgumentExist("-menu"))
    { //Argument for showing host_manager menu
        Host::GetHost().SetMenuDisplay(true);
    }

    unsigned val;
    if (GetArgumentValue("-d", val))
    {   //Argument for setting verbosity level
        g_LogConfig.SetMaxSeverity(val);
    }

    return true;
}

bool ArgumentsParser::DoesArgumentExist(const string& option)
{
    bool doesArgumentExist = find(m_arguments.begin(), m_arguments.end(), option) != m_arguments.end();
    return doesArgumentExist;
}

bool ArgumentsParser::GetArgumentValue(const string& option, unsigned& val)
{
    auto argumentIter = find(m_arguments.begin(), m_arguments.end(), option);
    if (argumentIter != m_arguments.end())
    {
        ++argumentIter;
        if (argumentIter != m_arguments.end())
        {
            const char* valStr = (*argumentIter).c_str();
            char* end;
            val = strtoul(valStr, &end, 10);
            if (end == valStr)
            {
                LOG_WARNING << "Failed to parse input argument '" << option << "', value is '"<< valStr << "'" << endl;
                return false;
            }

            return true;
        }
    }
    // else given option wasn't passed

    return false;
}
