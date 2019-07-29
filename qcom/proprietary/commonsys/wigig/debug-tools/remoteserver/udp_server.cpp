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
#include <stdlib.h>
#include <string.h>
#include <cerrno>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fstream>
#include "server.h"
#include "cmdiface.h"
#include "debug.h"
#include "pmc_file.h"
#include "file_reader.h"
#include "udp_server.h"

const std::string UdpServer::local_host = "127.0.0.1";
const std::string UdpServer::host_details_file_name = "/etc/wigig_remoteserver_details";
const std::string UdpServer::cmd_get_host_identity = "GetHostIdentity";
const std::string UdpServer::cmd_delimiter = ";";

std::string UdpServer::host_broadcast_ip = "";
std::string UdpServer::host_ip = "";
std::string UdpServer::host_alias = "";
int UdpServer::sock_in = 0;
int UdpServer::sock_out = 0;
int UdpServer::port_in = 0;
int UdpServer::port_out = 0;

int UdpServer::SendAll(std::string message)
{
    int result = -1;

    struct sockaddr_in dstAddress;
    dstAddress.sin_family = AF_INET;
    inet_pton(AF_INET , host_broadcast_ip.c_str(), &dstAddress.sin_addr.s_addr);
    dstAddress.sin_port = htons(UdpServer::port_out);
    int messageSize = message.length() * sizeof(char);
    result = sendto(UdpServer::sock_out, message.c_str(), messageSize, 0, (sockaddr*)&dstAddress, sizeof(dstAddress));
    LOG_DEBUG << "INFO : sendto with sock_out=" << UdpServer::sock_out << ", message=" << message << " messageSize=" << messageSize << " returned with " << result << std::endl;
    if (result < 0)
    {
        LOG_DEBUG << "ERROR : Cannot send udp broadcast message, error " << errno << ": " << strerror(errno)  << std::endl;
        return -4;
    }

    return 0;
}

int UdpServer::HandleRequest(char* buf)
{
    if(NULL == buf)
    {
        return 0;
    }

    LOG_DEBUG << "INFO : Client Command: " << PlainStr(buf) << std::endl;

    std::string cmd(buf);
    std::string answer;

    if (UdpServer::local_host == host_ip)
    {
        LOG_DEBUG << "WARNING : There is no support in UDP srevices when using local host" << std::endl;
        return 0;
    }

    if (UdpServer::cmd_get_host_identity == cmd)
    {
        answer = GetHostDetails();
    }
    else
    {
        LOG_DEBUG << "ERROR : Unkonwn UDP command" << std::endl;
    }

    // send back response in broadcast
    // For the time being, all requests will have broadcast response
    // All responses will use the same outgoing communication port
    LOG_DEBUG << "INFO : Answer is: " << PlainStr(answer.c_str()) << std::endl;
    SendAll(answer);

    return 0;
}


/*
  Close the server to allow un-bind te socket - allowing future connections without delay
*/
int UdpServer::stop()
{
    LOG_DEBUG << "INFO : Stopping the UDP server" << std::endl;
    shutdown(UdpServer::sock_in, SHUT_RDWR);
    return 0;
}

/*
  Initialize UPD server on the given port. The function returns in case of error,
  otherwise it doesn't return
*/
int UdpServer::start(int localUdpPortIn, int remoteUdpPortIn)
{
    LOG_DEBUG << "INFO : Starting UDP server on port " << localUdpPortIn << std::endl;
    LOG_DEBUG << "INFO : Sending broadcast answers to port " << remoteUdpPortIn << std::endl;

    //  open the UDP server on a new thread
    UdpServer::port_in = localUdpPortIn;
    UdpServer::port_out = remoteUdpPortIn;
    pthread_t thread;
    pthread_create(&thread, 0, start_udp_server, NULL);
    pthread_detach(thread);

    return 0;
}

void* UdpServer::start_udp_server(void* unused)
{
    (void)unused;
    if (LoadHostDetails() < 0)
    {
        return NULL;
    }

    // Create outgoing response socket
    UdpServer::sock_out = socket(AF_INET, SOCK_DGRAM, 0);
    if (UdpServer::sock_out < 0)
    {
        LOG_DEBUG << "ERROR : Cannot open udp broadcast outgoing socket" << std::endl;
        return NULL;
    }

    int enabled = 1;
    int result;
    result = setsockopt(UdpServer::sock_out, SOL_SOCKET, SO_BROADCAST, (char*)&enabled, sizeof(enabled));
    if (result < 0)
    {
        LOG_DEBUG << "ERROR : Cannot set broadcast option for udp socket, error " << errno << ": " << strerror(errno) << std::endl;
        shutdown(UdpServer::sock_out, SHUT_RDWR);
        return NULL;
    }

    // Create incoming request socket
    struct sockaddr_in address;
    UdpServer::sock_in = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(UdpServer::sock_in < 0)
    {
        LOG_DEBUG << "ERROR : Cannot create a socket on port " << UdpServer::port_in << std::endl;
        return NULL;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(UdpServer::port_in);
    // Set the "Re-Use" socket option - allows reconnections after wilserver exit
    int reuse = 1;
    setsockopt(UdpServer::sock_in, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
    if(bind(UdpServer::sock_in, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0)
    {
        LOG_DEBUG << "ERROR : Cannot bind socket to port " << UdpServer::port_in << ", error " << errno << ": " << strerror(errno)  << std::endl;
        return NULL;
    }

    char* buf = new char[MAX_INPUT_BUF];
    if (!buf)
    {
        LOG_ERROR << "Cannot allocate receive buffer for UDP messages" << std::endl;
        return NULL;
    }

    do
    {
        if (recvfrom(UdpServer::sock_in, buf, MAX_INPUT_BUF, 0, NULL, 0) < 0)
        {
            LOG_DEBUG << "ERROR : Cannot listen on port " << UdpServer::port_in << std::endl;
            return NULL;
        }

        HandleRequest(buf);

    } while (true);

    LOG_DEBUG << "INFO : UDP server shutdown" << std::endl;
    return NULL; // Wont get here, just to avoid the warning
}

bool FindEthernetInterface(struct ifreq& ifr, int& fd)
{
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0)
    {
        LOG_DEBUG << "ERROR : Cannot get host and broadcast IPs" << std::endl;
        return -1;
    }

    for (int i = 0; i < 100; i++)
    {
        snprintf(ifr.ifr_name, IFNAMSIZ-1, "eth%d", i);

        if (ioctl(fd, SIOCGIFADDR, &ifr) >= 0)
        {
            return true;
        }
    }

    return false;
}

int UdpServer::LoadHostAndBroadcastIps()
{
    int fd;
    struct ifreq ifr;

    ifr.ifr_addr.sa_family = AF_INET; // IP4V

    // Get IP address according to OS
    if (FindEthernetInterface(ifr, fd))
    {
        LOG_INFO << "Linux OS" << std::endl;
    }
    else
    {
        snprintf(ifr.ifr_name, IFNAMSIZ, "br-lan");
        if (ioctl(fd, SIOCGIFADDR, &ifr) >= 0)
        {
            LOG_INFO << "OpenWRT OS" << std::endl;
        }
        else
        {
            // Probably Android OS
            LOG_INFO << "Android OS (no external IP Adress)" << std::endl;
            host_ip = UdpServer::local_host;
            host_broadcast_ip = UdpServer::local_host;
            return 0;
        }
    }
    host_ip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);

    if (ioctl(fd, SIOCGIFBRDADDR, &ifr) < 0)
    {
        LOG_DEBUG << "ERROR : Cannot get broadcast IP" << std::endl;
        return -3;
    }
    host_broadcast_ip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    LOG_DEBUG << "INFO : " << "host_ip=" << host_ip << std::endl;
    LOG_DEBUG << "INFO : " << "host_broadcast_ip=" << host_broadcast_ip << std::endl;

    close(fd);
    return 0;
}

void UdpServer::LoadHostAlias()
{
    std::ifstream infile;
    infile.open(host_details_file_name.c_str());
    std::getline(infile, host_alias);
    infile.close();
}

int UdpServer::LoadHostDetails()
{
    if (LoadHostAndBroadcastIps() < 0)
    {
        return -1;
    }
    LoadHostAlias();
    return 0;
}
