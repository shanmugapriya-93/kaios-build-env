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
* @file EcrioSDPParseHeaderO.c
* @brief Implementation of SDP module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"

extern EcrioSDPAddressTypeEnum ec_sdp_FindAddressType(u_char *pStr);


/* #line 58 "EcrioSDPParseHeaderO.c" */
static const int ec_sdp_Parse_Header_O_start = 1;

/* #line 128 "EcrioSDPParseHeaderO.rl" */


/**
 * This is used to parse SDP Origin ("o=") line and fill up the SDP structure.
 * 
 * @param[in]	pStrings	Pointer to the string structure. Must be non-NULL.
 * @param[out]	pOrigin		Pointer to the origin structure. Must be non-NULL.
 * @param[in]	pData		Pointer to the SDP Origin ("o=") line. Must be non-NULL.
 * @param[in]	uSize		The size of SDP buffer.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseHeaderO
(
	EcrioSDPStringStruct *pStrings,
	EcrioSDPOriginStruct *pOrigin,
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

	
/* #line 94 "EcrioSDPParseHeaderO.c" */
	{
	cs = ec_sdp_Parse_Header_O_start;
	}

/* #line 155 "EcrioSDPParseHeaderO.rl" */
	
/* #line 101 "EcrioSDPParseHeaderO.c" */
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	if ( (*p) == 111 )
		goto st2;
	goto tr0;
tr0:
/* #line 112 "EcrioSDPParseHeaderO.rl" */
	{
		uError = ECRIO_SDP_PARSING_O_LINE_ERROR;
	}
	goto st0;
/* #line 117 "EcrioSDPParseHeaderO.c" */
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
	if ( (*p) == 32 )
		goto st3;
	goto tr3;
tr3:
/* #line 64 "EcrioSDPParseHeaderO.rl" */
	{
		tag_start = p;
	}
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
/* #line 147 "EcrioSDPParseHeaderO.c" */
	if ( (*p) == 32 )
		goto tr5;
	goto st4;
tr5:
/* #line 69 "EcrioSDPParseHeaderO.rl" */
	{
		uError = ec_sdp_StringCopy(pStrings, (void**)&pOrigin->pUsername, (u_char*)tag_start, (p - tag_start));
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
/* #line 165 "EcrioSDPParseHeaderO.c" */
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr6;
	goto tr0;
tr6:
/* #line 64 "EcrioSDPParseHeaderO.rl" */
	{
		tag_start = p;
	}
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
/* #line 179 "EcrioSDPParseHeaderO.c" */
	if ( (*p) == 32 )
		goto tr7;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st6;
	goto tr0;
tr7:
/* #line 78 "EcrioSDPParseHeaderO.rl" */
	{
		pOrigin->uSessionid = ec_sdp_ConvertToUI64Num((u_char*)tag_start);
	}
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
/* #line 195 "EcrioSDPParseHeaderO.c" */
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr9;
	goto tr0;
tr9:
/* #line 64 "EcrioSDPParseHeaderO.rl" */
	{
		tag_start = p;
	}
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
/* #line 209 "EcrioSDPParseHeaderO.c" */
	if ( (*p) == 32 )
		goto tr10;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st8;
	goto tr0;
tr10:
/* #line 83 "EcrioSDPParseHeaderO.rl" */
	{
		pOrigin->uVersion = ec_sdp_ConvertToUI64Num((u_char*)tag_start);
	}
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
/* #line 225 "EcrioSDPParseHeaderO.c" */
	if ( (*p) == 32 )
		goto tr13;
	goto tr12;
tr12:
/* #line 64 "EcrioSDPParseHeaderO.rl" */
	{
		tag_start = p;
	}
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
/* #line 239 "EcrioSDPParseHeaderO.c" */
	if ( (*p) == 32 )
		goto tr15;
	goto st10;
tr13:
/* #line 64 "EcrioSDPParseHeaderO.rl" */
	{
		tag_start = p;
	}
/* #line 88 "EcrioSDPParseHeaderO.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_ADDR_NETTYPE_INTERNET_STRING,
								pal_StringLength(ECRIO_SDP_ADDR_NETTYPE_INTERNET_STRING)) != 0)
		{
			uError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
			goto END;
		}
	}
	goto st11;
tr15:
/* #line 88 "EcrioSDPParseHeaderO.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_ADDR_NETTYPE_INTERNET_STRING,
								pal_StringLength(ECRIO_SDP_ADDR_NETTYPE_INTERNET_STRING)) != 0)
		{
			uError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
			goto END;
		}
	}
	goto st11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
/* #line 273 "EcrioSDPParseHeaderO.c" */
	if ( (*p) == 32 )
		goto tr17;
	goto tr16;
tr16:
/* #line 64 "EcrioSDPParseHeaderO.rl" */
	{
		tag_start = p;
	}
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
/* #line 287 "EcrioSDPParseHeaderO.c" */
	if ( (*p) == 32 )
		goto tr19;
	goto st12;
tr17:
/* #line 64 "EcrioSDPParseHeaderO.rl" */
	{
		tag_start = p;
	}
/* #line 98 "EcrioSDPParseHeaderO.rl" */
	{
		pOrigin->eAddressType = ec_sdp_FindAddressType((u_char*)tag_start);
	}
	goto st14;
tr19:
/* #line 98 "EcrioSDPParseHeaderO.rl" */
	{
		pOrigin->eAddressType = ec_sdp_FindAddressType((u_char*)tag_start);
	}
	goto st14;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
/* #line 311 "EcrioSDPParseHeaderO.c" */
	switch( (*p) ) {
		case 10: goto tr22;
		case 13: goto tr23;
	}
	goto tr21;
tr21:
/* #line 64 "EcrioSDPParseHeaderO.rl" */
	{
		tag_start = p;
	}
	goto st15;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
/* #line 327 "EcrioSDPParseHeaderO.c" */
	switch( (*p) ) {
		case 10: goto tr25;
		case 13: goto tr26;
	}
	goto st15;
tr22:
/* #line 64 "EcrioSDPParseHeaderO.rl" */
	{
		tag_start = p;
	}
/* #line 103 "EcrioSDPParseHeaderO.rl" */
	{
		uError = ec_sdp_StringCopy(pStrings, (void**)&pOrigin->pAddress, (u_char*)tag_start, (p - tag_start));
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}
	goto st16;
tr25:
/* #line 103 "EcrioSDPParseHeaderO.rl" */
	{
		uError = ec_sdp_StringCopy(pStrings, (void**)&pOrigin->pAddress, (u_char*)tag_start, (p - tag_start));
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}
	goto st16;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
/* #line 361 "EcrioSDPParseHeaderO.c" */
	goto st0;
tr23:
/* #line 64 "EcrioSDPParseHeaderO.rl" */
	{
		tag_start = p;
	}
/* #line 103 "EcrioSDPParseHeaderO.rl" */
	{
		uError = ec_sdp_StringCopy(pStrings, (void**)&pOrigin->pAddress, (u_char*)tag_start, (p - tag_start));
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}
	goto st13;
tr26:
/* #line 103 "EcrioSDPParseHeaderO.rl" */
	{
		uError = ec_sdp_StringCopy(pStrings, (void**)&pOrigin->pAddress, (u_char*)tag_start, (p - tag_start));
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}
	goto st13;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
/* #line 391 "EcrioSDPParseHeaderO.c" */
	if ( (*p) == 10 )
		goto st16;
	goto tr0;
	}
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
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 15: 
/* #line 103 "EcrioSDPParseHeaderO.rl" */
	{
		uError = ec_sdp_StringCopy(pStrings, (void**)&pOrigin->pAddress, (u_char*)tag_start, (p - tag_start));
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
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
	case 9: 
	case 10: 
	case 11: 
	case 12: 
	case 13: 
/* #line 112 "EcrioSDPParseHeaderO.rl" */
	{
		uError = ECRIO_SDP_PARSING_O_LINE_ERROR;
	}
	break;
	case 14: 
/* #line 64 "EcrioSDPParseHeaderO.rl" */
	{
		tag_start = p;
	}
/* #line 103 "EcrioSDPParseHeaderO.rl" */
	{
		uError = ec_sdp_StringCopy(pStrings, (void**)&pOrigin->pAddress, (u_char*)tag_start, (p - tag_start));
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}
	break;
/* #line 458 "EcrioSDPParseHeaderO.c" */
	}
	}

	_out: {}
	}

/* #line 156 "EcrioSDPParseHeaderO.rl" */

END:
	return uError;
}
