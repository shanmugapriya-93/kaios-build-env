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
* @file EcrioSDPParseHeaderM.c
* @brief Implementation of SDP module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"


/* #line 56 "EcrioSDPParseHeaderM.c" */
static const int ec_sdp_Parse_Header_M_start = 1;

/* #line 104 "EcrioSDPParseHeaderM.rl" */


EcrioSDPMediaTypeEnum ec_sdp_FindMediaType
(
	u_char *pStr
)
{
	EcrioSDPMediaTypeEnum eMediaType = ECRIO_SDP_MEDIA_TYPE_NONE;

	if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_TYPE_AUDIO_STRING,
							pal_StringLength(ECRIO_SDP_MEDIA_TYPE_AUDIO_STRING)) == 0)
	{
		eMediaType = ECRIO_SDP_MEDIA_TYPE_AUDIO;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_TYPE_MSRP_STRING,
								pal_StringLength(ECRIO_SDP_MEDIA_TYPE_MSRP_STRING)) == 0)
	{
		eMediaType = ECRIO_SDP_MEDIA_TYPE_MSRP;
	}
	else
	{
		eMediaType = ECRIO_SDP_MEDIA_TYPE_NONE;
	}

	return eMediaType;
}

EcrioSDPMediaProtocolEnum ec_sdp_FindMediaProtocol
(
	u_char *pStr
)
{
	EcrioSDPMediaProtocolEnum eProtocol = ECRIO_SDP_MEDIA_PROTOCOL_NONE;

	if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_PROTOCOL_RTP_AVP_STRING,
							pal_StringLength(ECRIO_SDP_MEDIA_PROTOCOL_RTP_AVP_STRING)) == 0)
	{
		eProtocol = ECRIO_SDP_MEDIA_PROTOCOL_RTP_AVP;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_PROTOCOL_TCP_MSRP_STRING,
								pal_StringLength(ECRIO_SDP_MEDIA_PROTOCOL_TCP_MSRP_STRING)) == 0)
	{
		eProtocol = ECRIO_SDP_MEDIA_PROTOCOL_TCP_MSRP;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_PROTOCOL_TLS_MSRP_STRING,
								pal_StringLength(ECRIO_SDP_MEDIA_PROTOCOL_TLS_MSRP_STRING)) == 0)
	{
		eProtocol = ECRIO_SDP_MEDIA_PROTOCOL_TLS_MSRP;
	}
	else
	{
		eProtocol = ECRIO_SDP_MEDIA_PROTOCOL_NONE;
	}

	return eProtocol;
}

u_int32 ec_sdp_FindMediaFormat
(
	EcrioSDPStreamStruct *pStream,
	u_char *pStart
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;

	if (pStream->eMediaType == ECRIO_SDP_MEDIA_TYPE_AUDIO)
	{
		/** Note that if number of media type which include in media line exceed
		  * a MAX_PAYLOAD, then later media type will ignore. In order to avoid this,
		  * we need to increase a MAX_PAYLOAD value -- however, it will also increase
		  * usage of memory. */
		if (pStream->uNumOfPayloads < MAX_PAYLOAD)
		{
			pStream->payload[pStream->uNumOfPayloads].uType = (u_int8)pal_StringConvertToUNum(pStart, NULL, 10);
			pStream->uNumOfPayloads++;
		}
	}
	else if (pStream->eMediaType == ECRIO_SDP_MEDIA_TYPE_MSRP)
	{
		/** The format list field MUST be set to "*" in MSRP media. */
		if (pal_StringNCompare(pStart, (u_char*)"*", 1) != 0)
		{
			uError = ECRIO_SDP_PARSING_M_LINE_ERROR;
		}
	}

	return uError;
}

/**
 * This is used to parse SDP Media Descriptions ("m=") line and fill up the SDP structure.
 * 
 * @param[out]	pStream			Pointer to the stream structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the SDP Media Descriptions ("m=") line.
 * 								Must be non-NULL.
 * @param[in]	uSize			The size of SDP buffer.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseHeaderM
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

	pStream->uNumOfPayloads = 0;

	
/* #line 180 "EcrioSDPParseHeaderM.c" */
	{
	cs = ec_sdp_Parse_Header_M_start;
	}

/* #line 219 "EcrioSDPParseHeaderM.rl" */
	
/* #line 187 "EcrioSDPParseHeaderM.c" */
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	if ( (*p) == 109 )
		goto st2;
	goto tr0;
tr0:
/* #line 90 "EcrioSDPParseHeaderM.rl" */
	{
		uError = ECRIO_SDP_PARSING_M_LINE_ERROR;
	}
	goto st0;
/* #line 203 "EcrioSDPParseHeaderM.c" */
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
/* #line 65 "EcrioSDPParseHeaderM.rl" */
	{
		tag_start = p;
	}
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
/* #line 252 "EcrioSDPParseHeaderM.c" */
	switch( (*p) ) {
		case 32: goto tr4;
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
tr4:
/* #line 70 "EcrioSDPParseHeaderM.rl" */
	{
		pStream->eMediaType = ec_sdp_FindMediaType((u_char*)tag_start);
	}
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
/* #line 285 "EcrioSDPParseHeaderM.c" */
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr6;
	goto tr0;
tr6:
/* #line 65 "EcrioSDPParseHeaderM.rl" */
	{
		tag_start = p;
	}
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
/* #line 299 "EcrioSDPParseHeaderM.c" */
	if ( (*p) == 32 )
		goto tr7;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st6;
	goto tr0;
tr7:
/* #line 75 "EcrioSDPParseHeaderM.rl" */
	{
		pStream->uMediaPort = (u_int16)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
	}
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
/* #line 315 "EcrioSDPParseHeaderM.c" */
	switch( (*p) ) {
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
/* #line 65 "EcrioSDPParseHeaderM.rl" */
	{
		tag_start = p;
	}
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
/* #line 347 "EcrioSDPParseHeaderM.c" */
	switch( (*p) ) {
		case 10: goto tr12;
		case 13: goto tr14;
		case 32: goto tr15;
		case 33: goto st10;
		case 37: goto st10;
		case 39: goto st10;
		case 47: goto st9;
		case 126: goto st10;
	}
	if ( (*p) < 45 ) {
		if ( 42 <= (*p) && (*p) <= 43 )
			goto st10;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st10;
		} else if ( (*p) >= 65 )
			goto st10;
	} else
		goto st10;
	goto st0;
tr12:
/* #line 80 "EcrioSDPParseHeaderM.rl" */
	{
		pStream->eProtocol = ec_sdp_FindMediaProtocol((u_char*)tag_start);
	}
	goto st11;
tr19:
/* #line 85 "EcrioSDPParseHeaderM.rl" */
	{
		uError = ec_sdp_FindMediaFormat(pStream, (u_char*)tag_start);
	}
	goto st11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
/* #line 386 "EcrioSDPParseHeaderM.c" */
	goto st0;
tr14:
/* #line 80 "EcrioSDPParseHeaderM.rl" */
	{
		pStream->eProtocol = ec_sdp_FindMediaProtocol((u_char*)tag_start);
	}
	goto st8;
tr20:
/* #line 85 "EcrioSDPParseHeaderM.rl" */
	{
		uError = ec_sdp_FindMediaFormat(pStream, (u_char*)tag_start);
	}
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
/* #line 404 "EcrioSDPParseHeaderM.c" */
	if ( (*p) == 10 )
		goto st11;
	goto tr0;
tr15:
/* #line 80 "EcrioSDPParseHeaderM.rl" */
	{
		pStream->eProtocol = ec_sdp_FindMediaProtocol((u_char*)tag_start);
	}
	goto st12;
tr21:
/* #line 85 "EcrioSDPParseHeaderM.rl" */
	{
		uError = ec_sdp_FindMediaFormat(pStream, (u_char*)tag_start);
	}
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
/* #line 424 "EcrioSDPParseHeaderM.c" */
	switch( (*p) ) {
		case 10: goto st11;
		case 13: goto st8;
		case 33: goto tr18;
		case 37: goto tr18;
		case 39: goto tr18;
		case 126: goto tr18;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr18;
		} else if ( (*p) >= 42 )
			goto tr18;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr18;
		} else if ( (*p) >= 65 )
			goto tr18;
	} else
		goto tr18;
	goto st0;
tr18:
/* #line 65 "EcrioSDPParseHeaderM.rl" */
	{
		tag_start = p;
	}
	goto st13;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
/* #line 458 "EcrioSDPParseHeaderM.c" */
	switch( (*p) ) {
		case 10: goto tr19;
		case 13: goto tr20;
		case 32: goto tr21;
		case 33: goto st13;
		case 37: goto st13;
		case 39: goto st13;
		case 126: goto st13;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st13;
		} else if ( (*p) >= 42 )
			goto st13;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st13;
		} else if ( (*p) >= 65 )
			goto st13;
	} else
		goto st13;
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	switch( (*p) ) {
		case 33: goto st10;
		case 37: goto st10;
		case 39: goto st10;
		case 126: goto st10;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st10;
		} else if ( (*p) >= 42 )
			goto st10;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st10;
		} else if ( (*p) >= 65 )
			goto st10;
	} else
		goto st10;
	goto tr0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 10: 
/* #line 80 "EcrioSDPParseHeaderM.rl" */
	{
		pStream->eProtocol = ec_sdp_FindMediaProtocol((u_char*)tag_start);
	}
	break;
	case 13: 
/* #line 85 "EcrioSDPParseHeaderM.rl" */
	{
		uError = ec_sdp_FindMediaFormat(pStream, (u_char*)tag_start);
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
/* #line 90 "EcrioSDPParseHeaderM.rl" */
	{
		uError = ECRIO_SDP_PARSING_M_LINE_ERROR;
	}
	break;
/* #line 552 "EcrioSDPParseHeaderM.c" */
	}
	}

	_out: {}
	}

/* #line 220 "EcrioSDPParseHeaderM.rl" */

	return uError;
}
