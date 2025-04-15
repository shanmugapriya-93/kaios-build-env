/******************************************************************************

Copyright (c) 2015-2020 Ecrio, Inc. All Rights Reserved.

Provided as supplementary materials for Licensed Software.

This file contains Confidential Information of Ecrio, Inc. and its suppliers.
Certain inventions disclosed in this file may be claimed within patents owned
or patent applications filed by Ecrio or third parties. No part of this
software may be reproduced or transmitted in any form or by any means or used
to make any derivative work (such as translation, transformation or
adaptation) without express prior written consent from Ecrio. You may not mark
or brand this file with any trade name, trademarks, service marks, or product
names other than the original brand (if any) provided by Ecrio. Any use of
Ecrio's or its suppliers work, confidential information, patented inventions,
or patent-pending inventions is subject to the terms and conditions of your
written license agreement with Ecrio. All other use and disclosure is strictly
prohibited.

Ecrio reserves the right to revise this software and to make changes in
content from time to time without obligation on the part of Ecrio to provide
notification of such revision or changes.

ECRIO MAKES NO REPRESENTATIONS OR WARRANTIES THAT THE SOFTWARE IS FREE OF
ERRORS OR THAT THE SOFTWARE IS SUITABLE FOR YOUR USE. THE SOFTWARE IS PROVIDED
ON AN "AS IS" BASIS FOR USE AT YOUR OWN RISK. ECRIO MAKES NO WARRANTIES,
TERMS OR CONDITIONS, EXPRESS OR IMPLIED,EITHER IN FACT OR BY OPERATION OF LAW,
STATUTORY OR OTHERWISE, INCLUDING WARRANTIES, TERMS, OR CONDITIONS OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND SATISFACTORY QUALITY.

TO THE FULL EXTENT ALLOWED BY LAW, ECRIO ALSO EXCLUDES FOR ITSELF AND ITS
SUPPLIERS ANY LIABILITY, WHETHER BASED IN CONTRACT OR TORT (INCLUDING
NEGLIGENCE), FOR DIRECT, INCIDENTAL, CONSEQUENTIAL, INDIRECT, SPECIAL, OR
PUNITIVE DAMAGES OF ANY KIND, OR FOR LOSS OF REVENUE OR PROFITS, LOSS OF
BUSINESS, LOSS OF INFORMATION OR DATA, OR OTHER FINANCIAL LOSS ARISING OUT
OF OR IN CONNECTION WITH THIS SOFTWARE, EVEN IF ECRIO HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES. THIS SOFTWARE MAY BE PROVIDED ON A DOWNLOAD SITE OR
ON COMPACT DISK AND THE OTHER SOFTWARE AND DOCUMENTATION ON THE DOWNLOAD SITE OR
COMPACT DISK ARE SUBJECT TO THE LICENSE AGREEMENT ACCOMPANYING THE COMPACT DISK.

******************************************************************************/

/**
* @file EcrioSDPParseHeaderA_DTMF_parameters.c
* @brief Implementation of SDP module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"


/* #line 56 "EcrioSDPParseHeaderA_DTMF_parameters.c" */
static const int ec_sdp_Parse_Header_A_DTMF_parameters_start = 1;

/* #line 96 "EcrioSDPParseHeaderA_DTMF_parameters.rl" */


/**
 * This function is used to parse DTMF specific parameters in a particular
 * format ("a=fmtp") media attribute line and store DTMF parameters to the
 * SDP information structure.
 * 
 * The parsing string may have a non-NULL terminator, and pData should be starting
 * AMR specific paramter, coming after format string. It should be specified
 * proper size to uSize. For example, in case of parsing the following part of
 * "a=fmtp" line:
 *   a=fmtp:108 0-15,66,77<CR><LF>...
 * Then pData should be indicated pointer of a head of DTMF parameter that
 * after the format (fmtp:110) string.
 * string and uSize should be set 10 which do not include a CRLF part.
 * 
 * @param[out]	pPayload		Pointer to the RTP payload structure. Must be non-NULL.
 * @param[in]	pData			Pointer to an DTMF Parameter in "a=fmtp" line. Must
 *                              be non-NULL.
 * @param[in]	uSize			The size of parsing string.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseDTMFParameters
(
	EcrioSDPPayloadStruct *pPayload,
	u_char *pData,
	u_int32 uSize
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;
	s_int32	cs;
	const char	*p = (char*)pData;
	const char	*pe = (char*)pData + uSize;
	const char	*eof = pe;
	const char	*tag_start = NULL;
	u_int32 uStart = 0;
	u_int32 uStop = 0;

	pPayload->u.audio.uSettings = 0;

	
/* #line 106 "EcrioSDPParseHeaderA_DTMF_parameters.c" */
	{
	cs = ec_sdp_Parse_Header_A_DTMF_parameters_start;
	}

/* #line 137 "EcrioSDPParseHeaderA_DTMF_parameters.rl" */
	
/* #line 113 "EcrioSDPParseHeaderA_DTMF_parameters.c" */
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr1;
	goto tr0;
tr0:
/* #line 79 "EcrioSDPParseHeaderA_DTMF_parameters.rl" */
	{
		uError = ECRIO_SDP_PARSING_A_FMTP_LINE_ERROR;
	}
	goto st0;
/* #line 129 "EcrioSDPParseHeaderA_DTMF_parameters.c" */
st0:
cs = 0;
	goto _out;
tr1:
/* #line 62 "EcrioSDPParseHeaderA_DTMF_parameters.rl" */
	{
		tag_start = p;
	}
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
/* #line 143 "EcrioSDPParseHeaderA_DTMF_parameters.c" */
	switch( (*p) ) {
		case 10: goto tr6;
		case 13: goto tr8;
		case 32: goto tr9;
		case 44: goto tr10;
		case 45: goto tr11;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st5;
	goto st0;
tr6:
/* #line 67 "EcrioSDPParseHeaderA_DTMF_parameters.rl" */
	{
		uStart = (u_int32)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		pPayload->u.audio.uSettings |= uStart << 24;
	}
	goto st6;
tr16:
/* #line 73 "EcrioSDPParseHeaderA_DTMF_parameters.rl" */
	{
		uStop = (u_int32)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		pPayload->u.audio.uSettings |= uStop << 16;
	}
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
/* #line 172 "EcrioSDPParseHeaderA_DTMF_parameters.c" */
	goto st0;
tr8:
/* #line 67 "EcrioSDPParseHeaderA_DTMF_parameters.rl" */
	{
		uStart = (u_int32)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		pPayload->u.audio.uSettings |= uStart << 24;
	}
	goto st2;
tr17:
/* #line 73 "EcrioSDPParseHeaderA_DTMF_parameters.rl" */
	{
		uStop = (u_int32)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		pPayload->u.audio.uSettings |= uStop << 16;
	}
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
/* #line 192 "EcrioSDPParseHeaderA_DTMF_parameters.c" */
	if ( (*p) == 10 )
		goto st6;
	goto tr0;
tr9:
/* #line 67 "EcrioSDPParseHeaderA_DTMF_parameters.rl" */
	{
		uStart = (u_int32)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		pPayload->u.audio.uSettings |= uStart << 24;
	}
	goto st7;
tr18:
/* #line 73 "EcrioSDPParseHeaderA_DTMF_parameters.rl" */
	{
		uStop = (u_int32)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		pPayload->u.audio.uSettings |= uStop << 16;
	}
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
/* #line 214 "EcrioSDPParseHeaderA_DTMF_parameters.c" */
	switch( (*p) ) {
		case 10: goto st6;
		case 13: goto st2;
		case 32: goto st7;
		case 44: goto st8;
		case 45: goto st4;
	}
	goto st0;
tr10:
/* #line 67 "EcrioSDPParseHeaderA_DTMF_parameters.rl" */
	{
		uStart = (u_int32)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		pPayload->u.audio.uSettings |= uStart << 24;
	}
	goto st8;
tr19:
/* #line 73 "EcrioSDPParseHeaderA_DTMF_parameters.rl" */
	{
		uStop = (u_int32)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		pPayload->u.audio.uSettings |= uStop << 16;
	}
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
/* #line 241 "EcrioSDPParseHeaderA_DTMF_parameters.c" */
	switch( (*p) ) {
		case 10: goto st6;
		case 13: goto st2;
		case 32: goto st3;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr1;
	goto st0;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	if ( (*p) == 32 )
		goto st3;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr1;
	goto tr0;
tr11:
/* #line 67 "EcrioSDPParseHeaderA_DTMF_parameters.rl" */
	{
		uStart = (u_int32)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		pPayload->u.audio.uSettings |= uStart << 24;
	}
	goto st4;
tr20:
/* #line 73 "EcrioSDPParseHeaderA_DTMF_parameters.rl" */
	{
		uStop = (u_int32)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		pPayload->u.audio.uSettings |= uStop << 16;
	}
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
/* #line 277 "EcrioSDPParseHeaderA_DTMF_parameters.c" */
	if ( (*p) == 32 )
		goto st4;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr5;
	goto tr0;
tr5:
/* #line 62 "EcrioSDPParseHeaderA_DTMF_parameters.rl" */
	{
		tag_start = p;
	}
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
/* #line 293 "EcrioSDPParseHeaderA_DTMF_parameters.c" */
	switch( (*p) ) {
		case 10: goto tr16;
		case 13: goto tr17;
		case 32: goto tr18;
		case 44: goto tr19;
		case 45: goto tr20;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st9;
	goto st0;
	}
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 5: 
/* #line 67 "EcrioSDPParseHeaderA_DTMF_parameters.rl" */
	{
		uStart = (u_int32)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		pPayload->u.audio.uSettings |= uStart << 24;
	}
	break;
	case 9: 
/* #line 73 "EcrioSDPParseHeaderA_DTMF_parameters.rl" */
	{
		uStop = (u_int32)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		pPayload->u.audio.uSettings |= uStop << 16;
	}
	break;
	case 1: 
	case 2: 
	case 3: 
	case 4: 
/* #line 79 "EcrioSDPParseHeaderA_DTMF_parameters.rl" */
	{
		uError = ECRIO_SDP_PARSING_A_FMTP_LINE_ERROR;
	}
	break;
/* #line 341 "EcrioSDPParseHeaderA_DTMF_parameters.c" */
	}
	}

	_out: {}
	}

/* #line 138 "EcrioSDPParseHeaderA_DTMF_parameters.rl" */

	return uError;
}
