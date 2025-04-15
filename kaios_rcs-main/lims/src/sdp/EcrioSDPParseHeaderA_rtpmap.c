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
* @file EcrioSDPParseHeaderA_rtpmap.c
* @brief Implementation of SDP module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"


/* #line 56 "EcrioSDPParseHeaderA_rtpmap.c" */
static const int ec_sdp_Parse_Header_A_rtpmap_start = 1;

/* #line 115 "EcrioSDPParseHeaderA_rtpmap.rl" */



EcrioAudioCodecEnum ec_sdp_FindAudioEncodingName
(
	u_char *pStr
)
{
	EcrioAudioCodecEnum eCodec = EcrioAudioCodec_None;

	if (pal_StringNCompare(pStr, ECRIO_SDP_AUDIO_CODEC_PCMU_STRING,
							pal_StringLength(ECRIO_SDP_AUDIO_CODEC_PCMU_STRING)) == 0)
	{
		eCodec = EcrioAudioCodec_PCMU;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_AUDIO_CODEC_L16_STRING,
								pal_StringLength(ECRIO_SDP_AUDIO_CODEC_L16_STRING)) == 0)
	{
		eCodec = EcrioAudioCodec_L16;
	}
#if !defined(DISABLE_CODEC_AMR_NB) || !defined(DISABLE_CODEC_AMR_WB)
	else if (pal_StringNCompare(pStr, ECRIO_SDP_AUDIO_CODEC_AMR_WB_STRING,
								pal_StringLength(ECRIO_SDP_AUDIO_CODEC_AMR_WB_STRING)) == 0)
	{
		eCodec = EcrioAudioCodec_AMR_WB;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_AUDIO_CODEC_AMR_STRING,
								pal_StringLength(ECRIO_SDP_AUDIO_CODEC_AMR_STRING)) == 0)
	{
		eCodec = EcrioAudioCodec_AMR;
	}
#endif
	else if (pal_StringNCompare(pStr, ECRIO_SDP_AUDIO_CODEC_DTMF_STRING,
								pal_StringLength(ECRIO_SDP_AUDIO_CODEC_DTMF_STRING)) == 0)
	{
		eCodec = EcrioAudioCodec_DTMF;
	}
	else
	{
		eCodec = EcrioAudioCodec_None;
	}

	return eCodec;
}

/**
 * This function is used to parse an RTP payload ("a=rtpmap") media attribute
 * line and store information to the SDP information structure. 
 * 
 * The parsing string may have a non-NULL terminator, and pData should be
 * starting payload type. It should be specified proper size to uSize.
 * For example, in case of parsing the following 
 * "a=rtpmap" line:
 *   a=rtpmap:104 AMR-WB/16000<CR><LF>...
 * Then pData should be indicated pointer of a head of payload type "104"
 * that after "rtpmap:" string. uSize should be set 16 which do not include
 * a CRLF part.
 * 
 * @param[out]	pStream			Pointer to the stream structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the string of payload type. Must be non-NULL.
 * @param[in]	uSize			The size of parsing string.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseRtpmap
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
	EcrioSDPPayloadStruct	*pPayload = NULL;
	u_int8	uType = 0;
	u_int8	uIndex = 0;

	
/* #line 146 "EcrioSDPParseHeaderA_rtpmap.c" */
	{
	cs = ec_sdp_Parse_Header_A_rtpmap_start;
	}

/* #line 196 "EcrioSDPParseHeaderA_rtpmap.rl" */
	
/* #line 153 "EcrioSDPParseHeaderA_rtpmap.c" */
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
/* #line 101 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		uError = ECRIO_SDP_PARSING_A_RTPMAP_LINE_ERROR;
	}
	goto st0;
/* #line 169 "EcrioSDPParseHeaderA_rtpmap.c" */
st0:
cs = 0;
	goto _out;
tr1:
/* #line 62 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		tag_start = p;
	}
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
/* #line 183 "EcrioSDPParseHeaderA_rtpmap.c" */
	switch( (*p) ) {
		case 32: goto tr3;
		case 47: goto tr4;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st2;
	goto tr2;
tr11:
/* #line 62 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		tag_start = p;
	}
	goto st3;
tr2:
/* #line 67 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		uType = (u_int8)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		for (uIndex = 0; uIndex < pStream->uNumOfPayloads; uIndex++)
		{
			if (uType == pStream->payload[uIndex].uType)
			{
				pPayload = &pStream->payload[uIndex];
				break;
			}
		}

		if (uIndex == pStream->uNumOfPayloads)
		{
			/** Media type is not found in payload slot. */
			goto END;
		}
	}
/* #line 62 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		tag_start = p;
	}
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
/* #line 225 "EcrioSDPParseHeaderA_rtpmap.c" */
	if ( (*p) == 47 )
		goto tr7;
	goto st3;
tr4:
/* #line 67 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		uType = (u_int8)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		for (uIndex = 0; uIndex < pStream->uNumOfPayloads; uIndex++)
		{
			if (uType == pStream->payload[uIndex].uType)
			{
				pPayload = &pStream->payload[uIndex];
				break;
			}
		}

		if (uIndex == pStream->uNumOfPayloads)
		{
			/** Media type is not found in payload slot. */
			goto END;
		}
	}
/* #line 62 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		tag_start = p;
	}
/* #line 86 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		pPayload->eCodec = ec_sdp_FindAudioEncodingName((u_char*)tag_start);
	}
	goto st4;
tr7:
/* #line 86 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		pPayload->eCodec = ec_sdp_FindAudioEncodingName((u_char*)tag_start);
	}
	goto st4;
tr13:
/* #line 62 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		tag_start = p;
	}
/* #line 86 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		pPayload->eCodec = ec_sdp_FindAudioEncodingName((u_char*)tag_start);
	}
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
/* #line 277 "EcrioSDPParseHeaderA_rtpmap.c" */
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr8;
	goto tr0;
tr8:
/* #line 62 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		tag_start = p;
	}
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
/* #line 291 "EcrioSDPParseHeaderA_rtpmap.c" */
	switch( (*p) ) {
		case 10: goto tr14;
		case 13: goto tr16;
		case 47: goto tr17;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st8;
	goto st0;
tr14:
/* #line 91 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		pPayload->u.audio.uClockrate = pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
	}
	goto st9;
tr19:
/* #line 96 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		pPayload->u.audio.uChannels = (u_int8)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
	}
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
/* #line 316 "EcrioSDPParseHeaderA_rtpmap.c" */
	goto st0;
tr16:
/* #line 91 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		pPayload->u.audio.uClockrate = pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
	}
	goto st5;
tr20:
/* #line 96 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		pPayload->u.audio.uChannels = (u_int8)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
	}
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
/* #line 334 "EcrioSDPParseHeaderA_rtpmap.c" */
	if ( (*p) == 10 )
		goto st9;
	goto tr0;
tr17:
/* #line 91 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		pPayload->u.audio.uClockrate = pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
	}
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
/* #line 348 "EcrioSDPParseHeaderA_rtpmap.c" */
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr10;
	goto tr0;
tr10:
/* #line 62 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		tag_start = p;
	}
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
/* #line 362 "EcrioSDPParseHeaderA_rtpmap.c" */
	switch( (*p) ) {
		case 10: goto tr19;
		case 13: goto tr20;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st10;
	goto st0;
tr3:
/* #line 67 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		uType = (u_int8)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		for (uIndex = 0; uIndex < pStream->uNumOfPayloads; uIndex++)
		{
			if (uType == pStream->payload[uIndex].uType)
			{
				pPayload = &pStream->payload[uIndex];
				break;
			}
		}

		if (uIndex == pStream->uNumOfPayloads)
		{
			/** Media type is not found in payload slot. */
			goto END;
		}
	}
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
/* #line 394 "EcrioSDPParseHeaderA_rtpmap.c" */
	switch( (*p) ) {
		case 32: goto st7;
		case 47: goto tr13;
	}
	goto tr11;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 8: 
/* #line 91 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		pPayload->u.audio.uClockrate = pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
	}
	break;
	case 10: 
/* #line 96 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		pPayload->u.audio.uChannels = (u_int8)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
	}
	break;
	case 1: 
	case 2: 
	case 3: 
	case 4: 
	case 5: 
	case 6: 
	case 7: 
/* #line 101 "EcrioSDPParseHeaderA_rtpmap.rl" */
	{
		uError = ECRIO_SDP_PARSING_A_RTPMAP_LINE_ERROR;
	}
	break;
/* #line 439 "EcrioSDPParseHeaderA_rtpmap.c" */
	}
	}

	_out: {}
	}

/* #line 197 "EcrioSDPParseHeaderA_rtpmap.rl" */

END:
	return uError;
}
