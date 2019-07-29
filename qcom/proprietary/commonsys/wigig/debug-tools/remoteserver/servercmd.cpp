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
#include <sstream>

#include "servercmd.h"
#include "debug.h"

#define EXP_NON        0
#define EXP_CMD        (-1)
#define EXP_IFC        1
#define EXP_NUM        2
#define EXP_HEX        3
#define EXP_SEP        4
#define EXP_END        5

const char* parser_state_to_string(int parserState)
{
    switch (parserState)
    {
    case EXP_NON: return "EXP_NON";
    case EXP_CMD: return "EXP_CMD";
    case EXP_IFC: return "EXP_IFC";
    case EXP_NUM: return "EXP_NUM";
    case EXP_HEX: return "EXP_HEX";
    case EXP_SEP: return "EXP_SEP";
    case EXP_END: return "EXP_END";
    default:      return "<unknown>";
    }
}


static int states_table[][5] =
{
    { EXP_END, 0,       0,       0,       0 },      // 0 - get_interfaces
    { EXP_IFC, EXP_END, 0,       0,       0 },      // 1 - open_interface name
    { EXP_IFC, EXP_END, 0,       0,       0 },      // 2 - close_interface name
    { EXP_IFC, EXP_NUM, EXP_END, 0,       0 },      // 3 - r name address
    { EXP_IFC, EXP_NUM, EXP_NUM, EXP_END, 0 },      // 4 - rb name address count
    { EXP_IFC, EXP_NUM, EXP_NUM, EXP_END, 0 },      // 5 - w name address value
    { EXP_IFC, EXP_NUM, EXP_HEX, EXP_END, 0 },      // 6 - wb name address hexstring
    { EXP_IFC, EXP_END, 0,       0,       0 },      // 7 - interface_reset name
    { EXP_IFC, EXP_END, 0,       0,       0 },      // 8 - sw_reset name
    { EXP_END, 0,       0,       0,       0 },      // 9 - exit
    { EXP_IFC, EXP_NUM, EXP_NUM, EXP_END, 0 },      // 10 - Alloc_PMC name number_of_descriptors size_of_descriptors
    { EXP_IFC, EXP_NUM, EXP_END, 0,       0 },      // 11 - read_pmc interface requestReferenceNumber
    { EXP_NUM, EXP_END, 0,       0,       0 },      // 12 - read_pmc_file requestReferenceNumber
    { EXP_IFC, EXP_END, 0,          0,       0 },         // 13 - set_host_alias
};


// The return value is copied because std::stringstream::str() returns a temporary object
std::string parser_state_machine_to_string(int* pStateMachine)
{
    if (!pStateMachine)
    {
        return std::string("{}");
    }

    std::stringstream ss;

    ss << "{ ";

    size_t numStates = sizeof(states_table[0])/sizeof(int);
    for (size_t i = 0; i < numStates; ++i)
    {
        ss << parser_state_to_string(pStateMachine[i]) << " ";
    }
    ss << "}";

    return ss.str();
}


const char* command_to_string(int cmdCode)
{
    switch (cmdCode)
    {
    case CMD_COMMAND_UNKNOWN: return "CMD_COMMAND_UNKNOWN";
    case CMD_GET_INTERFACES:  return "CMD_GET_INTERFACES";
    case CMD_OPEN_INTERFACE:  return "CMD_OPEN_INTERFACE";
    case CMD_CLOSE_INTERFACE: return "CMD_CLOSE_INTERFACE";
    case CMD_R:               return "CMD_R";
    case CMD_RB:              return "CMD_RB";
    case CMD_W:               return "CMD_W";
    case CMD_WB:              return "CMD_WB";
    case CMD_INTERFACE_RESET: return "CMD_INTERFACE_RESET";
    case CMD_SW_RESET:        return "CMD_SW_RESET";
    case CMD_EXIT:            return "CMD_EXIT";
    case CMD_ALLOC_PMC:       return "CMD_ALLOC_PMC";
    case CMD_READ_PMC:        return "CMD_READ_PMC";
    case CMD_READ_PMC_FILE:   return "CMD_READ_PMC_FILE";
    case CMD_SET_HOST_ALIAS:  return "CMD_SET_HOST_ALIAS";
    default:                  return "<unknown>";
    }
}


const char* parser_error_to_string(int parserError)
{
    switch (parserError)
    {
    case ERR_BAD_TOKEN:           return "ERR_BAD_TOKEN";
    case ERR_BAD_VALUE:           return "VALUE";
    case ERR_UNTERMINATED_STRING: return "ERR_UNTERMINATED_STRING";
    case ERR_BAD_HEX_VALUE:       return "ERR_BAD_HEX_VALUE";
    case ERR_CMD_NOT_EXPECTED:    return "ERR_CMD_NOT_EXPECTED";
    case ERR_IFACE_NOT_EXPECTED:  return "ERR_IFACE_NOT_EXPECTED";
    case ERR_VALUE_NOT_EXPECTED:  return "ERR_VALUE_NOT_EXPECTED";
    case ERR_HEX_NOT_EXPECTED:    return "ERR_HEX_NOT_EXPECTED";
    case ERR_END_NOT_EXPECTED:    return "ERR_END_NOT_EXPECTED";
    default:                      return "<unknown>";
    }
}



/*
  Callback on parser start.
  Returns the handler to be used in all subsequent callbacks
  or NULL in case of error, the parsing will be aborted then
*/
void cb_parser_start(servercmd_t *s)
{
    s->states = NULL;
    s->state = EXP_CMD;    // We start with expecting a command
    s->cmd = CMD_COMMAND_UNKNOWN;
    s->error = 0;
    s->address = (unsigned int)-1;
    s->value = (unsigned int)-1;
    s->hexdata_len = 0;
    s->hexdata = (unsigned int*)malloc (sizeof(int)*MAX_REGS_LEN);
}

/*
  Callback on a command. Gets the command code.
  Returns 0 if ok, 1 if error, the parsing will be aborted then
*/
int  cb_cmd(servercmd_t *s, int cmd)
{
    LOG_VERBOSE <<"cb_cmd(" << cmd << ") state index = " << s->state << std::endl;
    LOG_VERBOSE << "State Machine: " << parser_state_machine_to_string(s->states) << std::endl;

    if(s->state != EXP_CMD)  {
        s->error = ERR_CMD_NOT_EXPECTED;
        LOG_ERROR << "Command not expected, expecting " << s->state << std::endl;
        return -1;
    }
    LOG_VERBOSE << "Parsed Command: " << cmd << "(" << command_to_string(cmd) << ")" << std::endl;
    s->cmd = cmd;
    s->states = states_table[cmd];
    s->state++;

    return 0;
}


/*
  Returns 0 if ok, 1 if error, the parsing will be aborted then
*/
int  cb_id(servercmd_t *s, const char *id)
{
    LOG_VERBOSE << "cb_id(" << id << ") state index = " << s->state << std::endl;
    LOG_VERBOSE << "State Machine: " << parser_state_machine_to_string(s->states) << std::endl;

    if((s->states == NULL) || (s->states[s->state] != EXP_IFC)) {
        s->error = ERR_IFACE_NOT_EXPECTED;
        LOG_ERROR << "Interface not expected, expecting: "
                  << (s->states?s->states[s->state]:-1) << std::endl;
        return -1;
    }

    LOG_VERBOSE << "Interface id: " << id << std::endl;
    snprintf(s->interface, MAX_INTERFACE_NAME, "%s", id);
    s->state++;
    return 0;
}

/*
  Returns 0 if ok, 1 if error, the parsing will be aborted then
*/
int  cb_number(servercmd_t *s, const char *id)
{
    LOG_VERBOSE << "cb_number(" << id << ") state index = " << s->state << std::endl;
    LOG_VERBOSE << "State Machine: " << parser_state_machine_to_string(s->states) << std::endl;

    if((s->states == NULL) || (s->states[s->state] != EXP_NUM))  {
        s->error = ERR_VALUE_NOT_EXPECTED;
        LOG_ERROR << "Number not expected, expecting " << (s->states?s->states[s->state]:-1) << std::endl;
        return -1;
    }

    // A hack now. if address already set, the number is for data
    LOG_VERBOSE << "string number " << id << std::endl;

    if(s->address != (unsigned int)-1){
        sscanf(id, "%u", &(s->value));
        //s->value = strtoul(id, tmp, 10); //10 is the base for conversion
    //    dprint("str to uint Parsed val %u\n",  strtoul(id, tmp, 10));
    //    dprint("str to int Parsed val %u\n", atol(id));//s->value = atol(id);
        LOG_VERBOSE << "scanf Parsed value: " << s->value << std::endl;
    }
    else{
    //    s->address = atol(id);
        sscanf(id, "%u", &(s->address));
        LOG_VERBOSE << "Parsed addr " << s->address << std::endl;
    }
    s->state++;
    return 0;
}

/*
  Callback on hex data
  Returns 0 if ok, 1 if error, the parsing will be aborted then
*/
int  cb_hexbyte(servercmd_t *s, int b)
{
    LOG_VERBOSE << "cb_hexbyte(0x" << std::hex << b << std::dec << ") state index = " << s->state << std::endl;
    LOG_VERBOSE << "State Machine: " << parser_state_machine_to_string(s->states) << std::endl;

    if((s->states == NULL) || (s->states[s->state] != EXP_HEX)) {
        s->error = ERR_HEX_NOT_EXPECTED;
        LOG_ERROR << "Hex not expected, expecting " << (s->states?s->states[s->state]:-1) << std::endl;
        return -1;
    }
    if(s->hexdata_len == MAX_REGS_LEN) {
        s->error = ERR_BAD_HEX_VALUE;
        LOG_ERROR << "Too long hex data" << std::endl;
        return -1;
    }
    LOG_VERBOSE << "Hex byte 0x" << std::hex << b << std::dec << std::endl;
    s->hexdata[s->hexdata_len] = b;
    s->hexdata_len++;
    // Do not change the s->state here, we are still expecting hex data. cb_endhex will change the state
    return 0;
}

/*
  Callback on end of hex data string
  Returns 0 if ok, 1 if error, the parsing will be aborted then
*/

int cb_endhex(servercmd_t *s)
{
    LOG_VERBOSE << "cb_endhex() state index = " << s->state << std::endl;
    LOG_VERBOSE << "State Machine: " << parser_state_machine_to_string(s->states) << std::endl;

    if((s->states == NULL) || (s->states[s->state] != EXP_HEX)) {
        s->error = ERR_HEX_NOT_EXPECTED;
        LOG_ERROR << "Hex not expected, expecting " << (s->states?s->states[s->state]:-1) << std::endl;
        return -1;
    }
    s->state++;
    return 0;
}

/*
  Callback on a separator, not used.
  Returns 0 if ok, 1 if error, the parsing will be aborted then
*/
int  cb_separator(servercmd_t *s)
{
    (void)s;
    return 0;
}

/*
  Callback on the parser end
  Returns the parser result: 0 if ok, error otherwise
*/
void  cb_parser_end(servercmd_t *s)
{
    LOG_VERBOSE << "cb_parser_end() state index = " << s->state << std::endl;
    LOG_VERBOSE << "State Machine: " << parser_state_machine_to_string(s->states) << std::endl;

    if((s->error == 0) && ((s->states == NULL) || (s->states[s->state] != EXP_END)))  {
        LOG_ERROR << "End of parser while still expecting " << (s->states?s->states[s->state]:-1) << std::endl;
        s->error = ERR_END_NOT_EXPECTED;
    }

    free(s->hexdata);
}

/*
  Callback on parser error
*/
void cb_error(servercmd_t *s, int error, const char *str)
{
    (void)str;
    s->error = error;
    LOG_ERROR << "Parser error: " << error << std::endl;
}

/*
  Service function. Converting a hex digit from a character.
*/
int hexdigit(char d)
{
    if((d >= '0') && (d <= '9'))
        return d-'0';
    else if((d >= 'A') && (d <= 'F'))
        return d-'A'+10;
    else if((d >- 'a') && (d <= 'f'))
        return d-'a'+10;
    else    // This shouldn't happen, it's the parser's responsibility to parse valid hex digits
        return 0;
}
