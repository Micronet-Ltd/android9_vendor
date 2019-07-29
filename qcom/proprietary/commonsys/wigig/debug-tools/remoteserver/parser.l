%{
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


#include "servercmd.h"

static int acc_byte = 0;
static int acc_len =  0;

%}
%option noyywrap
%option nounput
/* We need reentratnt scanner for multiple connections */
%option reentrant
/* %option c++ */
%x str
DIGIT		[0-9]
HEXDIGIT	[0-9A-Fa-f]

ID		[A-Za-z_!][A-Za-z_!0-9]*

%%
"get_interfaces"  {
		if(cb_cmd(yyextra, CMD_GET_INTERFACES))
			yyterminate();
		}
"open_interface"	{
		if(cb_cmd(yyextra, CMD_OPEN_INTERFACE))
			yyterminate();
		}
"close_interface" {
		if(cb_cmd(yyextra, CMD_CLOSE_INTERFACE))
			yyterminate();
		}
"r"		{
		if(cb_cmd(yyextra, CMD_R))
			yyterminate();
		}
"rb"		{
		if(cb_cmd(yyextra, CMD_RB))
			yyterminate();
		}
"w"		{
		if(cb_cmd(yyextra, CMD_W))
			yyterminate();
		}
"wb"		{
		if(cb_cmd(yyextra, CMD_WB))
			yyterminate();
		}
"alloc_pmc"	{
		if(cb_cmd(yyextra, CMD_ALLOC_PMC))
			yyterminate();
		}
"read_pmc"	{
		if(cb_cmd(yyextra, CMD_READ_PMC))
			yyterminate();
		}
"read_pmc_file" {
		if(cb_cmd(yyextra, CMD_READ_PMC_FILE))
			yyterminate();
		}
"interface_reset" {
		if(cb_cmd(yyextra, CMD_INTERFACE_RESET))
			yyterminate();
		}

"sw_reset"      {
		if(cb_cmd(yyextra, CMD_SW_RESET))
			yyterminate();
		}
"exit"  {
        if(cb_cmd(yyextra, CMD_EXIT))
            yyterminate();
        }
"set_host_alias"  {
        if(cb_cmd(yyextra, CMD_SET_HOST_ALIAS))
            yyterminate();
        }
{DIGIT}+	{
		/* A decimal number represents the address/value */
		if(cb_number(yyextra, yytext))
			yyterminate();
		}

{ID}		{
		/* The protocol doesn't separate between interface id and command */
		if(cb_id(yyextra, yytext))
			yyterminate();
		}
\"		{
		/* Start of string */
		BEGIN(str);
		acc_byte = 0;
		acc_len = 0;
		}
<str>\"		{
		/* End of string */
		if(acc_len > 0) {
			if(cb_hexbyte(yyextra, acc_byte))
				yyterminate();
		}
		/* Callback for end of hex string */
		if(cb_endhex(yyextra))
			yyterminate();
		BEGIN(INITIAL);
		}
<str>{HEXDIGIT}	{
		/* We get hex bytes in the string */
		if(acc_len == 10)  {
			cb_error(yyextra, ERR_BAD_HEX_VALUE, "");
			yyterminate();
		}else if(acc_len <= 1){
			acc_len++;
		// do nothing on the 0x
		}
		else  {
			acc_len++;
			acc_byte = acc_byte * 16 +  hexdigit(yytext[0]);
			}
		}
<str>[ \t]	{
		/* Hex bytes separated by spaces */
			if(acc_len > 0) {
				if(cb_hexbyte(yyextra, acc_byte))
					yyterminate();
				acc_len = 0;
				acc_byte = 0;
			}
		}
<str><<EOF>>	{
		/* Check the end of string with unterminated quote */
		cb_error(yyextra, ERR_UNTERMINATED_STRING, "");
		yyterminate();
		}
<str>x		{
		/* if we ancounter 0x, remove the x and continue */
			if(acc_len != 1){
				cb_error(yyextra, ERR_BAD_HEX_VALUE, "");
			}
			acc_len++;
		}
<str>.		{
		/* Anything except the hex bytes is not allowed in the string parameter */
		cb_error(yyextra, ERR_BAD_HEX_VALUE, "");
		yyterminate();
		}

[ \t\r\n]+	{
		/* Whitespace */
		if(cb_separator(yyextra))
			yyterminate();
		}
.		{
		cb_error(yyextra, ERR_BAD_TOKEN, yytext);
		yyterminate();
		}
%%

/*
 Parses the input line
*/
int parse_line(const char *line, servercmd_t *s)
{
	yyscan_t scanner;
	YY_BUFFER_STATE bs;
	cb_parser_start(s);
        yylex_init(&scanner);

        if (!scanner)
        {
            return 1;
        }

	yyset_extra(s, scanner);
	bs = yy_scan_string(line, scanner);
	yylex(scanner);
	yy_delete_buffer(bs, scanner);
	yylex_destroy(scanner);
	cb_parser_end(s);
	return s->error;
}



