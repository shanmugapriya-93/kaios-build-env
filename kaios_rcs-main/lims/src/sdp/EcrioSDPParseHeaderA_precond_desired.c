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
* @file EcrioSDPParseHeaderA_precond_desired.c
* @brief Implementation of SDP module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"


/* #line 56 "EcrioSDPParseHeaderA_precond_desired.c" */
static const int ec_sdp_Parse_Header_A_precond_desired_start = 1;
static const int ec_sdp_Parse_Header_A_precond_desired_first_final = 9;
static const int ec_sdp_Parse_Header_A_precond_desired_error = 0;

static const int ec_sdp_Parse_Header_A_precond_desired_en_main = 1;


/* #line 180 "EcrioSDPParseHeaderA_precond_desired.rl" */



/**
 * This function is used to parse the desire status attribute ("a=des")in media
 * attribute line and store information to the SDP information structure.
 * 
 * The parsing string may have a non-NULL terminator, but should be specified
 * proper size to uSize.
 * 
 * @param[out]	pStream			Pointer to the stream structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the string of format. Must be non-NULL.
 * @param[in]	uSize			The size of parsing string.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseDesiredStatus
(
	EcrioSDPStreamStruct *pStream,
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
	EcrioSDPPreconditionStatusStruct *pPrecondStatus = NULL;
	EcrioSDPPreconditionStrengthEnum strength = ECRIO_SDP_PRECONDITION_STRENGTH_INVALID;

	
/* #line 97 "EcrioSDPParseHeaderA_precond_desired.c" */
	{
	cs = ec_sdp_Parse_Header_A_precond_desired_start;
	}

/* #line 212 "EcrioSDPParseHeaderA_precond_desired.rl" */
	
/* #line 104 "EcrioSDPParseHeaderA_precond_desired.c" */
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	switch( (*p) ) {
		case 33: goto tr1;
		case 37: goto tr1;
		case 39: goto tr1;
		case 126: goto tr1;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr1;
		} else if ( (*p) >= 42 )
			goto tr1;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr1;
		} else if ( (*p) >= 65 )
			goto tr1;
	} else
		goto tr1;
	goto tr0;
tr0:
/* #line 166 "EcrioSDPParseHeaderA_precond_desired.rl" */
	{
		uError = ECRIO_SDP_PARSING_A_PRECOND_DESIRED_LINE_ERROR;
	}
	goto st0;
/* #line 138 "EcrioSDPParseHeaderA_precond_desired.c" */
st0:
cs = 0;
	goto _out;
tr1:
/* #line 65 "EcrioSDPParseHeaderA_precond_desired.rl" */
	{
		tag_start = p;
	}
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
/* #line 152 "EcrioSDPParseHeaderA_precond_desired.c" */
	switch( (*p) ) {
		case 32: goto tr2;
		case 33: goto st2;
		case 37: goto st2;
		case 39: goto st2;
		case 126: goto st2;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st2;
		} else if ( (*p) >= 42 )
			goto st2;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st2;
		} else if ( (*p) >= 65 )
			goto st2;
	} else
		goto st2;
	goto tr0;
tr2:
/* #line 70 "EcrioSDPParseHeaderA_precond_desired.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING)) != 0)
		{
			uError = ECRIO_SDP_PARSING_A_PRECOND_DESIRED_LINE_ERROR;
			goto END;
		}
	}
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
/* #line 190 "EcrioSDPParseHeaderA_precond_desired.c" */
	switch( (*p) ) {
		case 32: goto st3;
		case 33: goto tr5;
		case 37: goto tr5;
		case 39: goto tr5;
		case 126: goto tr5;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr5;
		} else if ( (*p) >= 42 )
			goto tr5;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr5;
		} else if ( (*p) >= 65 )
			goto tr5;
	} else
		goto tr5;
	goto tr0;
tr5:
/* #line 65 "EcrioSDPParseHeaderA_precond_desired.rl" */
	{
		tag_start = p;
	}
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
/* #line 223 "EcrioSDPParseHeaderA_precond_desired.c" */
	switch( (*p) ) {
		case 32: goto tr6;
		case 33: goto st4;
		case 37: goto st4;
		case 39: goto st4;
		case 126: goto st4;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st4;
		} else if ( (*p) >= 42 )
			goto st4;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st4;
		} else if ( (*p) >= 65 )
			goto st4;
	} else
		goto st4;
	goto tr0;
tr6:
/* #line 80 "EcrioSDPParseHeaderA_precond_desired.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY_STRING)) == 0)
		{
			strength = ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_STRENGTH_OPTIONAL_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_OPTIONAL_STRING)) == 0)
		{
			strength = ECRIO_SDP_PRECONDITION_STRENGTH_OPTIONAL;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_STRENGTH_NONE_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_NONE_STRING)) == 0)
		{
			strength = ECRIO_SDP_PRECONDITION_STRENGTH_NONE;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_STRENGTH_FAILURE_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_FAILURE_STRING)) == 0)
		{
			strength = ECRIO_SDP_PRECONDITION_STRENGTH_FAILURE;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_STRENGTH_UNKNOWN_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_UNKNOWN_STRING)) == 0)
		{
			strength = ECRIO_SDP_PRECONDITION_STRENGTH_UNKNOWN;
		}
		else
		{
			strength = ECRIO_SDP_PRECONDITION_STRENGTH_INVALID;
		}
	}
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
/* #line 284 "EcrioSDPParseHeaderA_precond_desired.c" */
	switch( (*p) ) {
		case 32: goto st5;
		case 33: goto tr9;
		case 37: goto tr9;
		case 39: goto tr9;
		case 126: goto tr9;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr9;
		} else if ( (*p) >= 42 )
			goto tr9;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr9;
		} else if ( (*p) >= 65 )
			goto tr9;
	} else
		goto tr9;
	goto tr0;
tr9:
/* #line 65 "EcrioSDPParseHeaderA_precond_desired.rl" */
	{
		tag_start = p;
	}
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
/* #line 317 "EcrioSDPParseHeaderA_precond_desired.c" */
	switch( (*p) ) {
		case 32: goto tr10;
		case 33: goto st6;
		case 37: goto st6;
		case 39: goto st6;
		case 126: goto st6;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st6;
		} else if ( (*p) >= 42 )
			goto st6;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st6;
		} else if ( (*p) >= 65 )
			goto st6;
	} else
		goto st6;
	goto tr0;
tr10:
/* #line 113 "EcrioSDPParseHeaderA_precond_desired.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_STATUS_E2E_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_STATUS_E2E_STRING)) == 0)
		{
			pPrecondStatus = &pStream->precond.e2e;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_STATUS_LOCAL_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_STATUS_LOCAL_STRING)) == 0)
		{
			pPrecondStatus = &pStream->precond.local;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_STATUS_REMOTE_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_STATUS_REMOTE_STRING)) == 0)
		{
			pPrecondStatus = &pStream->precond.remote;
		}
		else
		{
			uError = ECRIO_SDP_PARSING_A_PRECOND_DESIRED_LINE_ERROR;
			goto END;
		}
	}
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
/* #line 369 "EcrioSDPParseHeaderA_precond_desired.c" */
	switch( (*p) ) {
		case 32: goto st7;
		case 33: goto tr13;
		case 37: goto tr13;
		case 39: goto tr13;
		case 126: goto tr13;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr13;
		} else if ( (*p) >= 42 )
			goto tr13;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr13;
		} else if ( (*p) >= 65 )
			goto tr13;
	} else
		goto tr13;
	goto tr0;
tr13:
/* #line 65 "EcrioSDPParseHeaderA_precond_desired.rl" */
	{
		tag_start = p;
	}
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
/* #line 402 "EcrioSDPParseHeaderA_precond_desired.c" */
	switch( (*p) ) {
		case 10: goto tr15;
		case 13: goto tr17;
		case 33: goto st9;
		case 37: goto st9;
		case 39: goto st9;
		case 126: goto st9;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st9;
		} else if ( (*p) >= 42 )
			goto st9;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st9;
		} else if ( (*p) >= 65 )
			goto st9;
	} else
		goto st9;
	goto st0;
tr15:
/* #line 137 "EcrioSDPParseHeaderA_precond_desired.rl" */
	{
		pPrecondStatus->des.strength = strength;
		if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING)) == 0)
		{
			pPrecondStatus->des.direction = ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING)) == 0)
		{
			pPrecondStatus->des.direction = ECRIO_SDP_PRECONDITION_DIRECTION_NONE;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING)) == 0)
		{
			pPrecondStatus->des.direction = ECRIO_SDP_PRECONDITION_DIRECTION_SEND;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING)) == 0)
		{
			pPrecondStatus->des.direction = ECRIO_SDP_PRECONDITION_DIRECTION_RECV;
		}
		else
		{
			pPrecondStatus->des.direction = ECRIO_SDP_PRECONDITION_DIRECTION_INVALID;
		}
	}
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
/* #line 460 "EcrioSDPParseHeaderA_precond_desired.c" */
	goto st0;
tr17:
/* #line 137 "EcrioSDPParseHeaderA_precond_desired.rl" */
	{
		pPrecondStatus->des.strength = strength;
		if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING)) == 0)
		{
			pPrecondStatus->des.direction = ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING)) == 0)
		{
			pPrecondStatus->des.direction = ECRIO_SDP_PRECONDITION_DIRECTION_NONE;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING)) == 0)
		{
			pPrecondStatus->des.direction = ECRIO_SDP_PRECONDITION_DIRECTION_SEND;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING)) == 0)
		{
			pPrecondStatus->des.direction = ECRIO_SDP_PRECONDITION_DIRECTION_RECV;
		}
		else
		{
			pPrecondStatus->des.direction = ECRIO_SDP_PRECONDITION_DIRECTION_INVALID;
		}
	}
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
/* #line 496 "EcrioSDPParseHeaderA_precond_desired.c" */
	if ( (*p) == 10 )
		goto st10;
	goto tr0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 9: 
/* #line 137 "EcrioSDPParseHeaderA_precond_desired.rl" */
	{
		pPrecondStatus->des.strength = strength;
		if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING)) == 0)
		{
			pPrecondStatus->des.direction = ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING)) == 0)
		{
			pPrecondStatus->des.direction = ECRIO_SDP_PRECONDITION_DIRECTION_NONE;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING)) == 0)
		{
			pPrecondStatus->des.direction = ECRIO_SDP_PRECONDITION_DIRECTION_SEND;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING)) == 0)
		{
			pPrecondStatus->des.direction = ECRIO_SDP_PRECONDITION_DIRECTION_RECV;
		}
		else
		{
			pPrecondStatus->des.direction = ECRIO_SDP_PRECONDITION_DIRECTION_INVALID;
		}
	}
	break;
	case 1: 
	case 2: 
	case 3: 
	case 4: 
	case 5: 
	case 6: 
	case 7: 
	case 8: 
/* #line 166 "EcrioSDPParseHeaderA_precond_desired.rl" */
	{
		uError = ECRIO_SDP_PARSING_A_PRECOND_DESIRED_LINE_ERROR;
	}
	break;
/* #line 558 "EcrioSDPParseHeaderA_precond_desired.c" */
	}
	}

	_out: {}
	}

/* #line 213 "EcrioSDPParseHeaderA_precond_desired.rl" */

END:
	return uError;
}
