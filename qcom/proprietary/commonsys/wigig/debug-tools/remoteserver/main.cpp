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

#include <stdio.h>
#include <unistd.h>     // getopt
#include <stdlib.h>     // atoi
#include <signal.h>    // SIGQUIT handling
#include "parser.h"
#include "server.h"
#include "udp_server.h"
#include "debug.h"

#define WILSERVER_VERSION 0x0100
#define DEFAULT_SERVER_PORT 12348
#define DEFAULT_UDP_LOCAL_SERVER_PORT_IN 12349
#define DEFAULT_UDP_REMOTE_SERVER_PORT_IN 12350

UdpServer us;
Server s;

void sig_quit_handler(int signum)
{
    if (signum == SIGQUIT)
    {
        printf("Exiting Wilserver as per user request\n");
        us.stop();
        s.stop();
        exit(signum);
    }
}

int main(int argc, char* argv[])
{
    // Register for SIGQUIT signal
    signal(SIGQUIT, sig_quit_handler);

    int opt;
    int port=DEFAULT_SERVER_PORT;
    int localUdpPortIn = DEFAULT_UDP_LOCAL_SERVER_PORT_IN;
    int remoteUdpPortIn = DEFAULT_UDP_REMOTE_SERVER_PORT_IN;
    int traceLevel = LOG_SEV_WARNING;

    while ((opt = getopt(argc, argv, "d:vp:")) != -1) {
        switch (opt) {
        case 'd':
            traceLevel = atoi(optarg);
            g_LogConfig.SetMaxSeverity(traceLevel);
            break;
        case 'p':
            port = atoi(optarg);
            if(port == 0)  {
                LOG_ERROR << "Invalid port specified" << optarg << std::endl;
                exit(EXIT_FAILURE);
            }
            break;
        default:
            fprintf(stderr, "Usage: %s [-d debug_level] | [-p port]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    LOG_INFO << "Starting WIGIG Remote Server" << std::endl;
    us.start(localUdpPortIn, remoteUdpPortIn);
    s.start(port);
    return 0;
}


