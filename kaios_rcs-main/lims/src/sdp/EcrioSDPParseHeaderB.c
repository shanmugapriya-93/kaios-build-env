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
* @file EcrioSDPParseHeaderB.c
* @brief Implementation of SDP module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"


/* #line 56 "EcrioSDPParseHeaderB.c" */
static const int ec_sdp_Parse_Header_B_start = 1;

/* #line 92 "EcrioSDPParseHeaderB.rl" */


EcrioSDPBandwidthModifierEnum ec_sdp_FindBandwidth
(
	u_char *pStr
)
{
	EcrioSDPBandwidthModifierEnum eBwtype = ECRIO_SDP_BANDWIDTH_MODIFIER_NONE;

	if (pal_StringNCompare(pStr, ECRIO_SDP_BANDWIDTH_AS_STRING,
							pal_StringLength(ECRIO_SDP_BANDWIDTH_AS_STRING)) == 0)
	{
		eBwtype = ECRIO_SDP_BANDWIDTH_MODIFIER_AS;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_BANDWIDTH_RS_STRING,
								pal_StringLength(ECRIO_SDP_BANDWIDTH_RS_STRING)) == 0)
	{
		eBwtype = ECRIO_SDP_BANDWIDTH_MODIFIER_RS;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_BANDWIDTH_RR_STRING,
								pal_StringLength(ECRIO_SDP_BANDWIDTH_RR_STRING)) == 0)
	{
		eBwtype = ECRIO_SDP_BANDWIDTH_MODIFIER_RR;
	}
	else
	{
		eBwtype = ECRIO_SDP_BANDWIDTH_MODIFIER_NONE;
	}

	return eBwtype;
}

u_int32 ec_sdp_FillBandwidth
(
	EcrioSDPBandwidthStruct *pBandwidth,
	u_char *pStart,
	EcrioSDPBandwidthModifierEnum eBwtype
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;

	pBandwidth->uModifier |= eBwtype;
	switch (eBwtype)
	{
		case ECRIO_SDP_BANDWIDTH_MODIFIER_AS:
		{
			pBandwidth->uAS = pal_StringConvertToUNum(pStart, NULL, 10);
		}
		break;

		case ECRIO_SDP_BANDWIDTH_MODIFIER_RS:
		{
			pBandwidth->uRS = pal_StringConvertToUNum(pStart, NULL, 10);
		}
		break;

		case ECRIO_SDP_BANDWIDTH_MODIFIER_RR:
		{
			pBandwidth->uRR = pal_StringConvertToUNum(pStart, NULL, 10);
		}
		break;

		default:
			break;
	}

	return uError;
}

/**
 * This is used to parse SDP bandwidth ("b=") line and fill up the SDP structure.
 * 
 * @param[out]	pBandwidth		Pointer to the bandwidth structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the SDP bandwidth ("b=") line. 
 * 								Must be non-NULL.
 * @param[in]	uSize			The size of SDP buffer.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseHeaderB
(
	EcrioSDPBandwidthStruct *pBandwidth,
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
	EcrioSDPBandwidthModifierEnum	eBwtype = ECRIO_SDP_BANDWIDTH_MODIFIER_NONE;

	
/* #line 159 "EcrioSDPParseHeaderB.c" */
	{
	cs = ec_sdp_Parse_Header_B_start;
	}

/* #line 186 "EcrioSDPParseHeaderB.rl" */
	
/* #line 166 "EcrioSDPParseHeaderB.c" */
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	if ( (*p) == 98 )
		goto st2;
	goto tr0;
tr0:
/* #line 80 "EcrioSDPParseHeaderB.rl" */
	{
		uError = ECRIO_SDP_PARSING_B_LINE_ERROR;
	}
	goto st0;
/* #line 182 "EcrioSDPParseHeaderB.c" */
st0:
cs = 0;
	goto _out;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	switch( (*p) ) {
		case 32: goto st2;
		case 61: goto st3;
	}
	goto tr0;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	switch( (*p) ) {
		case 32: goto st3;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr3:
/* #line 65 "EcrioSDPParseHeaderB.rl" */
	{
		tag_start = p;
	}
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
/* #line 231 "EcrioSDPParseHeaderB.c" */
	switch( (*p) ) {
		case 33: goto st4;
		case 37: goto st4;
		case 39: goto st4;
		case 58: goto tr5;
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
tr5:
/* #line 70 "EcrioSDPParseHeaderB.rl" */
	{
		eBwtype = ec_sdp_FindBandwidth((u_char*)tag_start);
	}
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
/* #line 264 "EcrioSDPParseHeaderB.c" */
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr6;
	goto tr0;
tr6:
/* #line 65 "EcrioSDPParseHeaderB.rl" */
	{
		tag_start = p;
	}
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
/* #line 278 "EcrioSDPParseHeaderB.c" */
	switch( (*p) ) {
		case 10: goto tr8;
		case 13: goto tr10;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st7;
	goto st0;
tr8:
/* #line 75 "EcrioSDPParseHeaderB.rl" */
	{
		uError = ec_sdp_FillBandwidth(pBandwidth, (u_char*)tag_start, eBwtype);
	}
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
/* #line 296 "EcrioSDPParseHeaderB.c" */
	goto st0;
tr10:
/* #line 75 "EcrioSDPParseHeaderB.rl" */
	{
		uError = ec_sdp_FillBandwidth(pBandwidth, (u_char*)tag_start, eBwtype);
	}
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
/* #line 308 "EcrioSDPParseHeaderB.c" */
	if ( (*p) == 10 )
		goto st8;
	goto tr0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 7: 
/* #line 75 "EcrioSDPParseHeaderB.rl" */
	{
		uError = ec_sdp_FillBandwidth(pBandwidth, (u_char*)tag_start, eBwtype);
	}
	break;
	case 1: 
	case 2: 
	case 3: 
	case 4: 
	case 5: 
	case 6: 
/* #line 80 "EcrioSDPParseHeaderB.rl" */
	{
		uError = ECRIO_SDP_PARSING_B_LINE_ERROR;
	}
	break;
/* #line 342 "EcrioSDPParseHeaderB.c" */
	}
	}

	_out: {}
	}

/* #line 187 "EcrioSDPParseHeaderB.rl" */

	return uError;
}
