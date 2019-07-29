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

#ifndef __servercmd_h
#define __servercmd_h

// This header is included by the parser, which is flex-generated C code
#ifdef __cplusplus
extern "C" {
#endif

// Commands list
#define CMD_COMMAND_UNKNOWN     (-1)
#define CMD_GET_INTERFACES      0
#define CMD_OPEN_INTERFACE      1
#define CMD_CLOSE_INTERFACE     2
#define CMD_R                   3
#define CMD_RB                  4
#define CMD_W                   5
#define CMD_WB                  6
#define CMD_INTERFACE_RESET     7
#define CMD_SW_RESET            8
#define CMD_EXIT                9  // debug command
#define CMD_ALLOC_PMC           10
#define CMD_READ_PMC            11
#define CMD_READ_PMC_FILE       12
#define CMD_SET_HOST_ALIAS        13


const char* command_to_string(int cmdCode);

#define MAX_INTERFACE_NAME 32          // TODO: better to include WlctPciAcss.h that defines MAX_IF_NAME_LENGTH==32
#define MAX_REGS_LEN    (256* 1024)    // Max registers to read/write at once

// Parser errors
#define ERR_BAD_TOKEN (-1)
#define ERR_BAD_VALUE (-2)
#define ERR_UNTERMINATED_STRING (-3)
#define ERR_BAD_HEX_VALUE (-4)
#define ERR_CMD_NOT_EXPECTED (-5)
#define ERR_IFACE_NOT_EXPECTED (-6)
#define ERR_VALUE_NOT_EXPECTED (-7)
#define ERR_HEX_NOT_EXPECTED (-8)
#define ERR_END_NOT_EXPECTED (-9)

const char* parser_error_to_string(int parserError);


/*
 Parser result/state structure
*/
typedef struct {
    int state;
    int cmd;
    int *states;
    char interface[MAX_INTERFACE_NAME+1];
    unsigned int address;
    unsigned int value;
    unsigned int* hexdata;
    int hexdata_len;
    int error;
} servercmd_t;



/*
 Parser callbacks
*/
void cb_parser_start(servercmd_t *s);
int  cb_cmd(servercmd_t *s, int cmd);
int  cb_id(servercmd_t *s, const char *id);
int  cb_number(servercmd_t *s, const char *id);
int  cb_hexbyte(servercmd_t *s, int b);
int  cb_endhex(servercmd_t *s);
int  cb_separator(servercmd_t *s);
void cb_parser_end(servercmd_t *s);
void cb_error(servercmd_t *s, int error, const char *str);

/* helper functions */
int hexdigit(char d);

#ifdef __cplusplus
}
#endif

#endif // servercmd_h
