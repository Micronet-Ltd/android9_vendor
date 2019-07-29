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

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <fstream>

#include "cmdiface.h"
#include "parser.h"
#include "debug.h"
#include "WlctPciAcss.h"
#include "pmc_file.h"
#include "udp_server.h"


struct open_interface_s *CmdIface::open_interfaces = NULL;
pthread_mutex_t CmdIface::open_interfaces_mutex = PTHREAD_MUTEX_INITIALIZER;
int CmdIface::interface_id = 0;

/*
*/
void *CmdIface::get_interface(const char *name)
{
    struct open_interface_s *s = open_interfaces;
    LOG_DEBUG << "Looking for interface: " << name << std::endl;

    while(s != NULL)
    {
        LOG_VERBOSE << "Checking interface for match: " << s->interface << std::endl;
        if(0 == strncasecmp(name, s->interface, MAX_INTERFACE_NAME))
        {
            LOG_VERBOSE << "Match found" << std::endl;
            return s->handler;
        }
        s = s->next;
    }

    return NULL;
}

/*
*/
void *CmdIface::add_interface(const char *name, void *handler)
{
    LOG_DEBUG << "Adding interface: " << name << std::endl;

    void *h = get_interface(name);
    if(h != NULL)
    {
        LOG_DEBUG << "The interface is already open" << std::endl;
        return h;            // Interface already opened
    }

    // Add interface to the list
    struct open_interface_s *s = new struct open_interface_s;
    if(s == NULL)
    {
        return NULL;
    }

    snprintf(s->interface, MAX_INTERFACE_NAME, "%s", name);
    s->handler = handler;
    pthread_mutex_lock(&open_interfaces_mutex);
    s->next = open_interfaces;
    open_interfaces = s;
    pthread_mutex_unlock(&open_interfaces_mutex);

    return handler;
}

/*
*/
void CmdIface::del_interface(void *handler)
{
    LOG_DEBUG << "Deleting interfaces by handler" << std::endl;

    struct open_interface_s *s = open_interfaces;
    struct open_interface_s *prev = NULL;

    while(s != NULL) {
        if(handler == s->handler)  {
            // Remove the interface from the list
            pthread_mutex_lock(&open_interfaces_mutex);
            if(prev != NULL)
                prev->next = s->next;
            else
                open_interfaces = s->next;
            pthread_mutex_unlock(&open_interfaces_mutex);
            delete s;
        }
        prev = s;
        s = s->next;
    }
}

/*
*/
int CmdIface::cmd_get_interfaces()
{
    INTERFACE_LIST interfaces;
    int num_interfaces;
    int rc;

    LOG_DEBUG << "Getting active WIGIG card interfaces" << std::endl;

    rc = GetInterfaces(&interfaces, &num_interfaces);
    LOG_DEBUG << "Found " << num_interfaces << " interfaces" << std::endl;

    std::ostringstream replyBuilder;

    if(rc == 0)
    {
        for(int i=0; i < num_interfaces; i++)
        {
            replyBuilder << interfaces.list[i].ifName << ' ';
        }
    }

    replyBuilder << "\r\n";
    m_Reply = replyBuilder.str();

    return 0;
}

/*
*/
int CmdIface::cmd_open_interface(char *interface)
{
    void *handler = NULL;
    int rc=0;

    LOG_DEBUG << "Opening an interface: " << interface << std::endl;

    if( strstr(interface, "SPARROW")|| strstr(interface, "sparrow")){
    rc = CreateDeviceAccessHandler(interface, MST_SPARROW, &handler);
    }
    else if(strstr(interface, "MARLON") || strstr(interface, "marlon")){
    rc = CreateDeviceAccessHandler(interface, MST_MARLON, &handler);
    }
    else{
        m_Reply = "0xDEAD\r\n";
    }
    if(rc != 0)
        m_Reply = "0xDEADDEAD\r\n";
    else {

        std::ostringstream replyBuilder;

        replyBuilder << interface << '_' << interface_id;
        add_interface(replyBuilder.str().c_str(), handler);
        replyBuilder << "\r\n";
        m_Reply = replyBuilder.str();
        interface_id++;                     // TODO: Should be protected by a mutex? I didn't see it's being used in the application
    }
    return 0;
}

/*
*/
int CmdIface::cmd_close_interface(char *interface)
{
    LOG_DEBUG << "Closing interface: " << interface << std::endl;

    void *handler = get_interface(interface);

    if(handler != NULL)
    {
        del_interface(handler);
        interface_id--;
    }
    else
    {
        LOG_WARNING << "Interface " << interface << " wasn't opened" << std::endl;
    }

    m_Reply = "0\r\n";
    return 0;
}

/*
*/
int CmdIface::cmd_r(char *interface, unsigned int address)
{
    unsigned int val = 0xDEADDEAD;
    void *handler = get_interface(interface);

    std::ostringstream replyBuilder;
    if(handler == NULL)
    {
        m_Reply = "0xDEADDEAD";
    }
    else
    {
        int rc = WlctAccssRead(handler, address, val);
        (void)rc;
        replyBuilder << val;
    }

    replyBuilder << "\r\n";
    m_Reply = replyBuilder.str();

    return 0;
}

/*
*/
int CmdIface::cmd_w(char *interface, unsigned int address, unsigned int data)
{
    int rc;
    void *handler = get_interface(interface);

    std::ostringstream replyBuilder;

    if(handler == NULL)
    {
        replyBuilder << "0xDEADDEAD";
    }
    else
    {
        rc = WlctAccssWrite(handler, address, data);
        if(rc == 0)
            replyBuilder << "0";
        else
            replyBuilder << "ERROR";
    }

    replyBuilder << "\r\n";
    m_Reply = replyBuilder.str();

    return 0;
}

/*
*/
int CmdIface::cmd_alloc_pmc(char *interface, unsigned int desc_size, unsigned int desc_num)
{
    int rc;
    LOG_DEBUG << "Allocating PMC descriptors:"
              << " interface = " << interface
              << " size = " << desc_size
              << " number = " << desc_num
              << std::endl;

    std::ostringstream replyBuilder;

    void *handler = get_interface(interface);
    if(handler == NULL)
    {
        LOG_DEBUG << "Cannot get handler for interface " << interface << std::endl;
        replyBuilder << "0xDEADDEAD";
    }
    else
    {
    rc = WlctAccssAllocPmc(handler, desc_size, desc_num);

    if(rc == 0)
        {
            LOG_DEBUG << "Successfully allocated PMC descriptors" << std::endl;
            replyBuilder << "0";
        }
        else
        {
            LOG_ERROR << "Error allocating PMC descriptors" << std::endl;
            replyBuilder << "ERROR";
        }
    }

    replyBuilder << "\r\n";
    m_Reply = replyBuilder.str();

    return 0;
}

/*
*/
int CmdIface::cmd_read_pmc(char *interface, unsigned int ref_number)
{
    LOG_DEBUG << "Reading PMC data:"
              << " interface = " << interface
              << " reference number = " << ref_number
              << std::endl;

    void *handler = get_interface(interface);

    if(handler == NULL)
    {
        LOG_ERROR << "No interface found: " << interface << std::endl;
        m_Reply = "0xDEADDEAD\r\n";
        return 0;
    }

    PmcFile pmcFile(ref_number);
    PmcFileWriter pmcFileWriter(pmcFile);
    bool status = pmcFileWriter.WriteFile();
    if (false == status)
    {
        LOG_ERROR << "Error creating PMC data file" << std::endl;
        m_Reply = "0xDEADDEAD\r\n";
        return 0;
    }

    // Reply with file size
    size_t pmcFileSize = pmcFileWriter.GetFileSize();

    std::ostringstream replyBuilder;
    replyBuilder << pmcFileSize << "\r\n";
    m_Reply = replyBuilder.str();

    return 0;
}

/*
*/
int CmdIface::cmd_read_pmc_file(unsigned int ref_number)
{
    LOG_DEBUG << "Reading PMC File #" << ref_number << std::endl;

    PmcFile pmcFile(ref_number);

    if (NULL == pmcFile.GetFileName())
    {
        LOG_ERROR << "Error getting PMC data file #" << ref_number << std::endl;
        m_Reply = "0xDEADDEAD\r\n";
        return 0;
    }

    // Note: No \r\n is needed here, the file name won't be sent to a clientls
    m_Reply = pmcFile.GetFileName();
    replyType = REPLY_TYPE_FILE;
    return 0;
}

/*
*/
int CmdIface::cmd_rb(char *interface, unsigned int address, unsigned int num_regs)
{
    void *handler = get_interface(interface);

    if((handler == NULL) || (num_regs > MAX_REGS_LEN))
    {
        m_Reply = "0xDEADDEAD\r\n";
        return 0;
    }

    unsigned int *val = new unsigned int[num_regs];
    if (!val)
    {
        m_Reply = "0xDEADDEAD\r\n";
        return 0;
    }

    int rc = readBlock(handler, address, num_regs*sizeof(unsigned int), (char*)val);

    if (rc == 0)
    {
        std::ostringstream replyBuilder;
        replyBuilder << std::hex;

        for(unsigned int i=0; i < num_regs; i++)
        {
            replyBuilder << "0x" << val[i];
            if(i < num_regs -1 )
            {
                replyBuilder << ' ';
            }
        }

        replyBuilder << "\r\n";
        m_Reply = replyBuilder.str();
    }
    else
    {
        m_Reply = "0xDEADDEAD\r\n";
    }

    delete[] val;
    return 0;
}

/*
*/
int CmdIface::cmd_wb(char *interface, unsigned int address, unsigned int len, const char *block)
{
    void *handler = get_interface(interface);

    if((handler == NULL) || (len > MAX_REGS_LEN))
    {
        m_Reply = "0xDEADDEAD\r\n";
        return 0;
    }

    LOG_VERBOSE << "current WB is " << block << " length is " << len << std::endl;
    int rc = writeBlock(handler, address, len, block);
    if(rc == 0)
    {
        m_Reply = "0\r\n";
    }
    else
    {
        m_Reply = "ERROR\r\n";
    }

    return 0;
}


int CmdIface::cmd_interface_reset(char *interface)
{
    void *handler = get_interface(interface);
    if(handler == NULL)
    {
        m_Reply = "0xDEADDEAD\r\n";
    }
    else
    {
        int rc = InterfaceReset(handler);
        if(rc == 0)
        {
            m_Reply = "OK\r\n";
        }
        else
        {
            m_Reply = "0xDEADDEAD\r\n";
        }
    }

    return 0;
}


int CmdIface::cmd_sw_reset(char *interface)
{
    void *handler = get_interface(interface);
    if (handler == NULL)
    {
        m_Reply = "0xDEADDEAD\r\n";
    }
    else
    {
        int rc = SwReset(handler);
        if (rc == 0)
        {
            m_Reply = "OK\r\n";
        }
        else
        {
            m_Reply = "0xDEADDEAD\r\n";
        }
    }

    return 0;
}

int CmdIface::cmd_set_host_alias(char* alias)
{
    std::ofstream fd(UdpServer::host_details_file_name.c_str());
    if (!fd.is_open())
    {
        m_Reply = "FAIL : Coudn't set the new alias: failed to open the configuration file";
        LOG_VERBOSE << m_Reply << std::endl;
        return -1;
    }
    fd << alias;
    if (fd.bad())
    {
        m_Reply = "FAIL : Coudn't set the new alias: failed to write the new alias to the configuration file";
        LOG_VERBOSE << m_Reply << std::endl;
        fd.close();
        return -2;
    }
    fd.close();

    UdpServer::SetHostAlias(alias);
    UdpServer::SendAll(UdpServer::GetHostDetails());
    return 0;
}


/*
 Execute command received from a remote client
 The command of length 'len' is in 'buf'. 'outlen' has
 the max output buffer size.
 On return, 'outbuf' may have a reply to be sent to the client,
 'outlen' should have the reply length or 0 if no reply required.
 Returns KEEPALIVE_OK if ok, KEEPALIVE_CLOSE to close the connection, KEEPALIVE_SHUTDOWN to shutdown the server (debug mode)
*/
int CmdIface::do_command(const char *buf, int len)
{
    servercmd_t s;
    int result = KEEPALIVE_OK;
    replyType = REPLY_TYPE_BUFFER;

    ((char*)buf)[len] = '\0';    // Make a zero-terminated string

    // Parse the command
    parse_line(buf, &s);
    dump_parser(&s);

    // Check for the parser error. Note, not all the commands in the original protocol report about errors. so we check the error
    // for selected commands only TODO: verify the commands list reporting the error
    if(s.cmd == CMD_OPEN_INTERFACE || s.cmd == CMD_R || s.cmd == CMD_RB || s.cmd == CMD_W || s.cmd == CMD_WB)
    {
        if(s.error != 0)
        {
            LOG_ERROR << "Command line parsing error" << std::endl;
            m_Reply = "0xDEADDEAD\r\n";    // TODO: or should it be "dmtools_error"??
            return result;
        }
    }

    switch(s.cmd)
    {
    case CMD_GET_INTERFACES:
        cmd_get_interfaces();
        break;
    case CMD_OPEN_INTERFACE:
        cmd_open_interface(s.interface);
        break;
    case CMD_CLOSE_INTERFACE:
        cmd_close_interface(s.interface);
        break;
    case CMD_R:
        cmd_r(s.interface, s.address);
        break;
    case CMD_RB:
        cmd_rb(s.interface, s.address, s.value);
        break;
    case CMD_W:
        cmd_w(s.interface, s.address, s.value);
        break;
    case CMD_WB:
        // hexdata_len is in dwords, cmd_wb works in bytes. (hence * 4)
        cmd_wb(s.interface, s.address, s.hexdata_len * 4, (const char*)s.hexdata);
        break;
    case CMD_INTERFACE_RESET:
        cmd_interface_reset(s.interface);
        break;
    case CMD_SW_RESET:
    cmd_sw_reset(s.interface);
    break;
    case CMD_EXIT:
        result = KEEPALIVE_CLOSE;   // Terminate the connection
        break;
    case CMD_ALLOC_PMC:
        cmd_alloc_pmc(s.interface, s.address, s.value);
        break;
    case CMD_READ_PMC:
        cmd_read_pmc(s.interface, s.address);
        break;
    case CMD_READ_PMC_FILE:
        cmd_read_pmc_file(s.address);
        break;
    case CMD_SET_HOST_ALIAS:
        cmd_set_host_alias(s.interface);
        break;
    case CMD_COMMAND_UNKNOWN:
    default:
        m_Reply = "dmtools_error\r\n";

    }
    return result;
}

/*
 Dump the parser structure, for debugging only
*/
void CmdIface::dump_parser(servercmd_t *s)
{
    if(s->error)
    {
        LOG_ERROR << "Parser error in comand parsing."
            << " Error: " << s->error
            << " Message: " << parser_error_to_string(s->error)
            << std::endl;

        return;
    }

    LOG_VERBOSE << "Parsed Command: " << command_to_string(s->cmd)
                << " Interface: " <<  s->interface  << std::endl;

    if(s->address != (unsigned int)-1)
    {
        LOG_VERBOSE << "Address: " << s->address << std::endl;
    }
    if(s->value != (unsigned int)-1)
    {
        LOG_VERBOSE << "Value: " << s->value << std::endl;
    }
    if(s->hexdata_len)
    {
        for(int i=0; i < s->hexdata_len && i < MAX_REGS_LEN; i++)
            LOG_VERBOSE << std::hex << "0x" << s->hexdata[i] << std::dec << std::endl;
    }
}

/*
*/
int CmdIface::get_reply_len()
{
    return m_Reply.size();
}

/*
*/
void CmdIface::to_lower(char* string)
{
    for (int i = 0; string[i]; i++)
    {
    string[i] = tolower(string[i]);
    }

    return;
}

/*
*/
CmdIface::CmdIface()
    : replyType(REPLY_TYPE_BUFFER)
{
}
