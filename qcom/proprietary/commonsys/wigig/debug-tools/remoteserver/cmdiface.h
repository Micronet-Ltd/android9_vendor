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

#ifndef __interface_h
#define __interface_h

#include <pthread.h>
#include <string>

extern "C"
{
#include "parser.h"
}

struct open_interface_s {
    char interface[MAX_INTERFACE_NAME];
    void *handler;
    struct open_interface_s *next;
};

#define KEEPALIVE_OK    0    // keep the connection
#define KEEPALIVE_CLOSE 1    // close the connection
#define KEEPALIVE_SHUTDOWN 2 // close the connection and shutdown the server. For debugging only, remove from the production


// Max buffer size for a command and a reply. We should consider the longest command/data sequence to fit in a buffer.
// rb reading 1024 (MAX_REGS_LEN) registers: rb HHHHHHHH HHHH 1024*(0xHHHHHHHH) = 17 + 1024*10 = 10257 bytes
// wb writing 1024 (MAX_REGS_LEN) hex values: wb HHHHHHHH "1024*HH" = 14+1024*2 = 2062 bytes
#define MAX_INPUT_BUF (11*MAX_REGS_LEN)

// The reply data may be generated in several ways, the data is expected to be obtained according to this type
enum REPLY_TYPE
{
    REPLY_TYPE_NONE,
    REPLY_TYPE_BUFFER,
    REPLY_TYPE_FILE
};


class CmdIface
{
public:
    CmdIface();

    int do_command(const char *buf, int len);
    const char *get_reply() const { return m_Reply.c_str(); };
    int   get_reply_len();
    REPLY_TYPE get_reply_type() const { return replyType; }

private:

    static struct open_interface_s *open_interfaces;
    static pthread_mutex_t open_interfaces_mutex;
    static int interface_id;                        // Interface id counter returned in open_interface

    void *get_interface(const char *name);
    void *add_interface(const char *name, void *handler);
    void del_interface(void *handler);

    int cmd_get_interfaces();
    int cmd_open_interface(char *interface);
    int cmd_close_interface(char *interface);
    int cmd_r(char *interface, unsigned int address);
    int cmd_rb(char *interface, unsigned int address, unsigned int num_regs);
    int cmd_w(char *interface, unsigned int address, unsigned int data);
    int cmd_wb(char *interface, unsigned int address, unsigned int len, const char *block);
    int cmd_interface_reset(char *interface);
    int cmd_sw_reset(char *interface);
    int cmd_alloc_pmc(char *interface, unsigned int desc_size, unsigned int desc_num);
    int cmd_read_pmc(char *interface, unsigned int ref_number);
    int cmd_read_pmc_file(unsigned int ref_number);
    void dump_parser(servercmd_t *s);
    int cmd_set_host_alias(char *alias);

    void to_lower(char* string);

    REPLY_TYPE replyType;
    std::string m_Reply;

};

#endif // interface_h

