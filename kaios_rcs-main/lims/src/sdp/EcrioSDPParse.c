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
* @file EcrioSDPParse.c
* @brief Implementation of SDP module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"


/* #line 56 "EcrioSDPParse.c" */
static const int ec_sdp_Parser_start = 20;

/* #line 205 "EcrioSDPParse.rl" */


/**
 * This is used to parse a SDP message and fill up the SDP structure.
 * 
 * @param[out]	pSdpStruct		Pointer to the SDP structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the SDP message. Must be non-NULL.
 * @param[in]	uSize			The size of SDP buffer.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 EcrioSDPParse
(
	EcrioSDPSessionStruct *pSdpStruct,
	u_char *pData,
	u_int32 uSize
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;
	s_int32	cs;
	const char	*p;
	const char	*pe;
	const char	*eof;
	const char	*tag_start = NULL;
	EcrioSDPBandwidthStruct	*pBandwidth = NULL;
	EcrioSDPConnectionInfomationStruct	*pConn = NULL;
	BoolEnum	bIsMaxMedia = Enum_FALSE;

	/** Parameter check */
	if (pSdpStruct == NULL || pData == NULL || uSize == 0)
	{
		return ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
	}

	pal_MemorySet(&pSdpStruct->origin, 0, sizeof(EcrioSDPOriginStruct));
	pal_MemorySet(&pSdpStruct->conn, 0, sizeof(EcrioSDPConnectionInfomationStruct));
	pal_MemorySet(&pSdpStruct->bandwidth, 0, sizeof(EcrioSDPBandwidthStruct));
	pSdpStruct->uNumOfMedia = 0;
	pal_MemorySet(&pSdpStruct->stream, 0, sizeof(EcrioSDPStreamStruct) * MAX_MEDIA);

	p = (char*)pData;
	pe = (char*)pData + uSize;
	eof = pe;

	/** Parsing [<type>=<value>] to using Ragel*/
	
/* #line 110 "EcrioSDPParse.c" */
	{
	cs = ec_sdp_Parser_start;
	}

/* #line 250 "EcrioSDPParse.rl" */
	
/* #line 117 "EcrioSDPParse.c" */
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
tr3:
/* #line 66 "EcrioSDPParse.rl" */
	{
		if (bIsMaxMedia != Enum_TRUE)
		{
			/** Check for underflow of uNumOfMedia. Here, we assume that uNumOfMedia is
			 *  greater than or equal to 1 in order to handle the media-level attributes.
			 */
			if (Enum_FALSE == pal_UtilityArithmeticUnderflowDetected(pSdpStruct->uNumOfMedia, 1))
			{
				uError = ec_sdp_ParseHeaderA(&pSdpStruct->strings,
											 &pSdpStruct->stream[pSdpStruct->uNumOfMedia - 1],
											 (u_char*)tag_start,
											 (p - tag_start));
				if (uError != ECRIO_SDP_NO_ERROR)
				{
					goto END;
				}
			}
		}
	}
	goto st20;
tr8:
/* #line 81 "EcrioSDPParse.rl" */
	{
		if (pSdpStruct->uNumOfMedia == 0)
		{
			pBandwidth = &pSdpStruct->bandwidth;
		}
		else if (bIsMaxMedia != Enum_TRUE)
		{
			pBandwidth = &pSdpStruct->stream[pSdpStruct->uNumOfMedia - 1].bandwidth;
		}
		else
		{
			goto END;
		}

		uError = ec_sdp_ParseHeaderB(pBandwidth,
									 (u_char*)tag_start,
									 (p - tag_start));
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}
	goto st20;
tr12:
/* #line 105 "EcrioSDPParse.rl" */
	{
		if (pSdpStruct->uNumOfMedia == 0)
		{
			pConn = &pSdpStruct->conn;
		}
		else if (bIsMaxMedia != Enum_TRUE)
		{
			pConn = &pSdpStruct->stream[pSdpStruct->uNumOfMedia - 1].conn;
		}
		else
		{
			goto END;
		}

		uError = ec_sdp_ParseHeaderC(&pSdpStruct->strings,
									 pConn,
									 (u_char*)tag_start,
									 (p - tag_start));
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}
	goto st20;
tr16:
/* #line 179 "EcrioSDPParse.rl" */
	{
		/** No action when unsupported header was given */
		uError = ECRIO_SDP_NO_ERROR;
	}
	goto st20;
tr20:
/* #line 130 "EcrioSDPParse.rl" */
	{
		if (pSdpStruct->uNumOfMedia == MAX_MEDIA)
		{
			/** media slot is full, then later media line do not parse. */
			bIsMaxMedia = Enum_TRUE;
			goto END;
		}

		uError = ec_sdp_ParseHeaderM(&pSdpStruct->stream[pSdpStruct->uNumOfMedia],
									 (u_char*)tag_start,
									 (p - tag_start));
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
		pSdpStruct->uNumOfMedia++;
	}
	goto st20;
tr24:
/* #line 149 "EcrioSDPParse.rl" */
	{
		uError = ec_sdp_ParseHeaderO(&pSdpStruct->strings,
									 &pSdpStruct->origin,
									 (u_char*)tag_start,
									 (p - tag_start));
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}
	goto st20;
tr28:
/* #line 161 "EcrioSDPParse.rl" */
	{
		/** No check a session name */
		uError = ECRIO_SDP_NO_ERROR;
	}
	goto st20;
tr32:
/* #line 167 "EcrioSDPParse.rl" */
	{
		/** No check a start/end time */
		uError = ECRIO_SDP_NO_ERROR;
	}
	goto st20;
tr36:
/* #line 173 "EcrioSDPParse.rl" */
	{
		/** No check a version number */
		uError = ECRIO_SDP_NO_ERROR;
	}
	goto st20;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
/* #line 255 "EcrioSDPParse.c" */
	switch( (*p) ) {
		case 65: goto tr38;
		case 66: goto tr40;
		case 67: goto tr41;
		case 77: goto tr43;
		case 79: goto tr44;
		case 83: goto tr45;
		case 84: goto tr46;
		case 86: goto tr47;
		case 97: goto tr38;
		case 98: goto tr40;
		case 99: goto tr41;
		case 109: goto tr43;
		case 111: goto tr44;
		case 115: goto tr45;
		case 116: goto tr46;
		case 118: goto tr47;
	}
	if ( (*p) > 90 ) {
		if ( 100 <= (*p) && (*p) <= 122 )
			goto tr42;
	} else if ( (*p) >= 68 )
		goto tr42;
	goto st0;
tr0:
/* #line 185 "EcrioSDPParse.rl" */
	{
		/** This is a general error, but usually no CRLF at the end of line. */
		uError = ECRIO_SDP_PARSING_INSUFFICIENT_ERROR;
	}
	goto st0;
/* #line 287 "EcrioSDPParse.c" */
st0:
cs = 0;
	goto _out;
tr38:
/* #line 61 "EcrioSDPParse.rl" */
	{
		tag_start = p;
	}
	goto st1;
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
/* #line 301 "EcrioSDPParse.c" */
	switch( (*p) ) {
		case 32: goto st1;
		case 61: goto st2;
	}
	goto tr0;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	switch( (*p) ) {
		case 10: goto tr3;
		case 13: goto tr4;
	}
	goto st2;
tr4:
/* #line 66 "EcrioSDPParse.rl" */
	{
		if (bIsMaxMedia != Enum_TRUE)
		{
			/** Check for underflow of uNumOfMedia. Here, we assume that uNumOfMedia is
			 *  greater than or equal to 1 in order to handle the media-level attributes.
			 */
			if (Enum_FALSE == pal_UtilityArithmeticUnderflowDetected(pSdpStruct->uNumOfMedia, 1))
			{
				uError = ec_sdp_ParseHeaderA(&pSdpStruct->strings,
											 &pSdpStruct->stream[pSdpStruct->uNumOfMedia - 1],
											 (u_char*)tag_start,
											 (p - tag_start));
				if (uError != ECRIO_SDP_NO_ERROR)
				{
					goto END;
				}
			}
		}
	}
	goto st3;
tr9:
/* #line 81 "EcrioSDPParse.rl" */
	{
		if (pSdpStruct->uNumOfMedia == 0)
		{
			pBandwidth = &pSdpStruct->bandwidth;
		}
		else if (bIsMaxMedia != Enum_TRUE)
		{
			pBandwidth = &pSdpStruct->stream[pSdpStruct->uNumOfMedia - 1].bandwidth;
		}
		else
		{
			goto END;
		}

		uError = ec_sdp_ParseHeaderB(pBandwidth,
									 (u_char*)tag_start,
									 (p - tag_start));
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}
	goto st3;
tr13:
/* #line 105 "EcrioSDPParse.rl" */
	{
		if (pSdpStruct->uNumOfMedia == 0)
		{
			pConn = &pSdpStruct->conn;
		}
		else if (bIsMaxMedia != Enum_TRUE)
		{
			pConn = &pSdpStruct->stream[pSdpStruct->uNumOfMedia - 1].conn;
		}
		else
		{
			goto END;
		}

		uError = ec_sdp_ParseHeaderC(&pSdpStruct->strings,
									 pConn,
									 (u_char*)tag_start,
									 (p - tag_start));
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}
	goto st3;
tr17:
/* #line 179 "EcrioSDPParse.rl" */
	{
		/** No action when unsupported header was given */
		uError = ECRIO_SDP_NO_ERROR;
	}
	goto st3;
tr21:
/* #line 130 "EcrioSDPParse.rl" */
	{
		if (pSdpStruct->uNumOfMedia == MAX_MEDIA)
		{
			/** media slot is full, then later media line do not parse. */
			bIsMaxMedia = Enum_TRUE;
			goto END;
		}

		uError = ec_sdp_ParseHeaderM(&pSdpStruct->stream[pSdpStruct->uNumOfMedia],
									 (u_char*)tag_start,
									 (p - tag_start));
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
		pSdpStruct->uNumOfMedia++;
	}
	goto st3;
tr25:
/* #line 149 "EcrioSDPParse.rl" */
	{
		uError = ec_sdp_ParseHeaderO(&pSdpStruct->strings,
									 &pSdpStruct->origin,
									 (u_char*)tag_start,
									 (p - tag_start));
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}
	goto st3;
tr29:
/* #line 161 "EcrioSDPParse.rl" */
	{
		/** No check a session name */
		uError = ECRIO_SDP_NO_ERROR;
	}
	goto st3;
tr33:
/* #line 167 "EcrioSDPParse.rl" */
	{
		/** No check a start/end time */
		uError = ECRIO_SDP_NO_ERROR;
	}
	goto st3;
tr37:
/* #line 173 "EcrioSDPParse.rl" */
	{
		/** No check a version number */
		uError = ECRIO_SDP_NO_ERROR;
	}
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
/* #line 448 "EcrioSDPParse.c" */
	if ( (*p) == 10 )
		goto st20;
	goto tr0;
tr40:
/* #line 61 "EcrioSDPParse.rl" */
	{
		tag_start = p;
	}
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
/* #line 462 "EcrioSDPParse.c" */
	switch( (*p) ) {
		case 32: goto st4;
		case 61: goto st5;
	}
	goto tr0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	switch( (*p) ) {
		case 10: goto tr8;
		case 13: goto tr9;
	}
	goto st5;
tr41:
/* #line 61 "EcrioSDPParse.rl" */
	{
		tag_start = p;
	}
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
/* #line 487 "EcrioSDPParse.c" */
	switch( (*p) ) {
		case 32: goto st6;
		case 61: goto st7;
	}
	goto tr0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	switch( (*p) ) {
		case 10: goto tr12;
		case 13: goto tr13;
	}
	goto st7;
tr42:
/* #line 61 "EcrioSDPParse.rl" */
	{
		tag_start = p;
	}
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
/* #line 512 "EcrioSDPParse.c" */
	switch( (*p) ) {
		case 32: goto st8;
		case 61: goto st9;
	}
	goto tr0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	switch( (*p) ) {
		case 10: goto tr16;
		case 13: goto tr17;
	}
	goto st9;
tr43:
/* #line 61 "EcrioSDPParse.rl" */
	{
		tag_start = p;
	}
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
/* #line 537 "EcrioSDPParse.c" */
	switch( (*p) ) {
		case 32: goto st10;
		case 61: goto st11;
	}
	goto tr0;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
	switch( (*p) ) {
		case 10: goto tr20;
		case 13: goto tr21;
	}
	goto st11;
tr44:
/* #line 61 "EcrioSDPParse.rl" */
	{
		tag_start = p;
	}
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
/* #line 562 "EcrioSDPParse.c" */
	switch( (*p) ) {
		case 32: goto st12;
		case 61: goto st13;
	}
	goto tr0;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	switch( (*p) ) {
		case 10: goto tr24;
		case 13: goto tr25;
	}
	goto st13;
tr45:
/* #line 61 "EcrioSDPParse.rl" */
	{
		tag_start = p;
	}
	goto st14;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
/* #line 587 "EcrioSDPParse.c" */
	switch( (*p) ) {
		case 32: goto st14;
		case 61: goto st15;
	}
	goto tr0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	switch( (*p) ) {
		case 10: goto tr28;
		case 13: goto tr29;
	}
	goto st15;
tr46:
/* #line 61 "EcrioSDPParse.rl" */
	{
		tag_start = p;
	}
	goto st16;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
/* #line 612 "EcrioSDPParse.c" */
	switch( (*p) ) {
		case 32: goto st16;
		case 61: goto st17;
	}
	goto tr0;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	switch( (*p) ) {
		case 10: goto tr32;
		case 13: goto tr33;
	}
	goto st17;
tr47:
/* #line 61 "EcrioSDPParse.rl" */
	{
		tag_start = p;
	}
	goto st18;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
/* #line 637 "EcrioSDPParse.c" */
	switch( (*p) ) {
		case 32: goto st18;
		case 61: goto st19;
	}
	goto tr0;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
	switch( (*p) ) {
		case 10: goto tr36;
		case 13: goto tr37;
	}
	goto st19;
	}
	_test_eof20: cs = 20; goto _test_eof; 
	_test_eof1: cs = 1; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 
	_test_eof17: cs = 17; goto _test_eof; 
	_test_eof18: cs = 18; goto _test_eof; 
	_test_eof19: cs = 19; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 1: 
	case 2: 
	case 3: 
	case 4: 
	case 5: 
	case 6: 
	case 7: 
	case 8: 
	case 9: 
	case 10: 
	case 11: 
	case 12: 
	case 13: 
	case 14: 
	case 15: 
	case 16: 
	case 17: 
	case 18: 
	case 19: 
/* #line 185 "EcrioSDPParse.rl" */
	{
		/** This is a general error, but usually no CRLF at the end of line. */
		uError = ECRIO_SDP_PARSING_INSUFFICIENT_ERROR;
	}
	break;
/* #line 703 "EcrioSDPParse.c" */
	}
	}

	_out: {}
	}

/* #line 251 "EcrioSDPParse.rl" */

END:
	return uError;
}
