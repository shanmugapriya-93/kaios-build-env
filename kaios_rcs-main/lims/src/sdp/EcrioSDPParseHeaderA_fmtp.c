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
* @file EcrioSDPParseHeaderA_fmtp.c
* @brief Implementation of SDP module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"


/* #line 56 "EcrioSDPParseHeaderA_fmtp.c" */
static const int ec_sdp_Parse_Header_A_fmtp_start = 1;

/* #line 113 "EcrioSDPParseHeaderA_fmtp.rl" */



/**
 * This function is used to parse a particular format ("a=fmtp") media attribute
 * line and store information to the SDP information structure. 
 * 
 * The parsing string may have a non-NULL terminator, but should be specified
 * proper size to uSize. For example, in case of parsing the following 
 * "a=fmtp" line:
 *   a=fmtp:104 mode-set=2; octet-align=0<CR><LF>...
 * Then pData should be indicated pointer of a head of format "104"
 * that after "fmtp:" string. uSize should be set 29 which do not include
 * a CRLF part.
 * 
 * @param[out]	pStream			Pointer to the stream structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the string of format. Must be non-NULL.
 * @param[in]	uSize			The size of parsing string.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseFmtp
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
	u_int8	uFormat = 0;
	u_int8	uIndex = 0;

	
/* #line 103 "EcrioSDPParseHeaderA_fmtp.c" */
	{
	cs = ec_sdp_Parse_Header_A_fmtp_start;
	}

/* #line 151 "EcrioSDPParseHeaderA_fmtp.rl" */
	
/* #line 110 "EcrioSDPParseHeaderA_fmtp.c" */
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
/* #line 101 "EcrioSDPParseHeaderA_fmtp.rl" */
	{
		uError = ECRIO_SDP_PARSING_A_FMTP_LINE_ERROR;
	}
	goto st0;
/* #line 126 "EcrioSDPParseHeaderA_fmtp.c" */
st0:
cs = 0;
	goto _out;
tr1:
/* #line 62 "EcrioSDPParseHeaderA_fmtp.rl" */
	{
		tag_start = p;
	}
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
/* #line 140 "EcrioSDPParseHeaderA_fmtp.c" */
	if ( (*p) == 32 )
		goto tr2;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st2;
	goto tr0;
tr2:
/* #line 67 "EcrioSDPParseHeaderA_fmtp.rl" */
	{
		uFormat = (u_int8)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		for (uIndex = 0; uIndex < pStream->uNumOfPayloads; uIndex++)
		{
			if (uFormat == pStream->payload[uIndex].uType)
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
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
/* #line 170 "EcrioSDPParseHeaderA_fmtp.c" */
	switch( (*p) ) {
		case 10: goto tr6;
		case 13: goto tr7;
	}
	goto tr5;
tr5:
/* #line 62 "EcrioSDPParseHeaderA_fmtp.rl" */
	{
		tag_start = p;
	}
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
/* #line 186 "EcrioSDPParseHeaderA_fmtp.c" */
	switch( (*p) ) {
		case 10: goto tr9;
		case 13: goto tr10;
	}
	goto st5;
tr6:
/* #line 62 "EcrioSDPParseHeaderA_fmtp.rl" */
	{
		tag_start = p;
	}
/* #line 86 "EcrioSDPParseHeaderA_fmtp.rl" */
	{
#if !defined(DISABLE_CODEC_AMR_NB) || !defined(DISABLE_CODEC_AMR_WB)
		if (pPayload->eCodec == EcrioAudioCodec_AMR_WB ||
			pPayload->eCodec == EcrioAudioCodec_AMR)
		{
			uError = ec_sdp_ParseAMRParameters(pPayload, (u_char*)tag_start, (p - tag_start));
		}
#endif
		if (pPayload->eCodec == EcrioAudioCodec_DTMF)
		{
			uError = ec_sdp_ParseDTMFParameters(pPayload, (u_char*)tag_start, (p - tag_start));
		}
	}
	goto st6;
tr9:
/* #line 86 "EcrioSDPParseHeaderA_fmtp.rl" */
	{
#if !defined(DISABLE_CODEC_AMR_NB) || !defined(DISABLE_CODEC_AMR_WB)
		if (pPayload->eCodec == EcrioAudioCodec_AMR_WB ||
			pPayload->eCodec == EcrioAudioCodec_AMR)
		{
			uError = ec_sdp_ParseAMRParameters(pPayload, (u_char*)tag_start, (p - tag_start));
		}
#endif
		if (pPayload->eCodec == EcrioAudioCodec_DTMF)
		{
			uError = ec_sdp_ParseDTMFParameters(pPayload, (u_char*)tag_start, (p - tag_start));
		}
	}
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
/* #line 232 "EcrioSDPParseHeaderA_fmtp.c" */
	goto st0;
tr7:
/* #line 62 "EcrioSDPParseHeaderA_fmtp.rl" */
	{
		tag_start = p;
	}
/* #line 86 "EcrioSDPParseHeaderA_fmtp.rl" */
	{
#if !defined(DISABLE_CODEC_AMR_NB) || !defined(DISABLE_CODEC_AMR_WB)
		if (pPayload->eCodec == EcrioAudioCodec_AMR_WB ||
			pPayload->eCodec == EcrioAudioCodec_AMR)
		{
			uError = ec_sdp_ParseAMRParameters(pPayload, (u_char*)tag_start, (p - tag_start));
		}
#endif
		if (pPayload->eCodec == EcrioAudioCodec_DTMF)
		{
			uError = ec_sdp_ParseDTMFParameters(pPayload, (u_char*)tag_start, (p - tag_start));
		}
	}
	goto st3;
tr10:
/* #line 86 "EcrioSDPParseHeaderA_fmtp.rl" */
	{
#if !defined(DISABLE_CODEC_AMR_NB) || !defined(DISABLE_CODEC_AMR_WB)
		if (pPayload->eCodec == EcrioAudioCodec_AMR_WB ||
			pPayload->eCodec == EcrioAudioCodec_AMR)
		{
			uError = ec_sdp_ParseAMRParameters(pPayload, (u_char*)tag_start, (p - tag_start));
		}
#endif
		if (pPayload->eCodec == EcrioAudioCodec_DTMF)
		{
			uError = ec_sdp_ParseDTMFParameters(pPayload, (u_char*)tag_start, (p - tag_start));
		}
	}
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
/* #line 274 "EcrioSDPParseHeaderA_fmtp.c" */
	if ( (*p) == 10 )
		goto st6;
	goto tr0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 5: 
/* #line 86 "EcrioSDPParseHeaderA_fmtp.rl" */
	{
#if !defined(DISABLE_CODEC_AMR_NB) || !defined(DISABLE_CODEC_AMR_WB)
		if (pPayload->eCodec == EcrioAudioCodec_AMR_WB ||
			pPayload->eCodec == EcrioAudioCodec_AMR)
		{
			uError = ec_sdp_ParseAMRParameters(pPayload, (u_char*)tag_start, (p - tag_start));
		}
#endif
		if (pPayload->eCodec == EcrioAudioCodec_DTMF)
		{
			uError = ec_sdp_ParseDTMFParameters(pPayload, (u_char*)tag_start, (p - tag_start));
		}
	}
	break;
	case 1: 
	case 2: 
	case 3: 
/* #line 101 "EcrioSDPParseHeaderA_fmtp.rl" */
	{
		uError = ECRIO_SDP_PARSING_A_FMTP_LINE_ERROR;
	}
	break;
	case 4: 
/* #line 62 "EcrioSDPParseHeaderA_fmtp.rl" */
	{
		tag_start = p;
	}
/* #line 86 "EcrioSDPParseHeaderA_fmtp.rl" */
	{
#if !defined(DISABLE_CODEC_AMR_NB) || !defined(DISABLE_CODEC_AMR_WB)
		if (pPayload->eCodec == EcrioAudioCodec_AMR_WB ||
			pPayload->eCodec == EcrioAudioCodec_AMR)
		{
			uError = ec_sdp_ParseAMRParameters(pPayload, (u_char*)tag_start, (p - tag_start));
		}
#endif
		if (pPayload->eCodec == EcrioAudioCodec_DTMF)
		{
			uError = ec_sdp_ParseDTMFParameters(pPayload, (u_char*)tag_start, (p - tag_start));
		}
	}
	break;
/* #line 333 "EcrioSDPParseHeaderA_fmtp.c" */
	}
	}

	_out: {}
	}

/* #line 152 "EcrioSDPParseHeaderA_fmtp.rl" */

END:
	return uError;
}
