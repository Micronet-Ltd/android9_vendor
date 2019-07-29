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

#ifndef __udp_server_h
#define __udp_server_h

class UdpServer
{
public:
    int start(int localUdpPortIn, int remoteUdpPortIn);
    int stop();

    static int SendAll(std::string message);
    static std::string GetHostDetails() { return  UdpServer::cmd_get_host_identity + UdpServer::cmd_delimiter + host_ip + UdpServer::cmd_delimiter + host_alias; }
    static void SetHostAlias(std::string alias) { host_alias = alias; }

    static const std::string host_details_file_name;

private:
    // members
    static const std::string local_host;
    static const std::string cmd_get_host_identity;
    static const std::string cmd_delimiter;
    static std::string host_broadcast_ip;
    static std::string host_ip;
    static std::string host_alias;
    static int sock_in;
    static int sock_out;
    static int port_in;
    static int port_out;

    // methods
    static void* start_udp_server(void* unused);
    static int LoadHostDetails();
    static int  LoadHostIp(); // assumption: each host has only one IP address for ethernet interfaces
    static void LoadHostAlias();
    static int HandleRequest(char* buf);
    static int LoadHostAndBroadcastIps();
};

#endif // udp_server_h
