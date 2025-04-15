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

%%{
	machine ec_sdp_Parse_Header_A_rtpmap;

	CR = "\r";
	LF = "\n";
	SP = " ";
	CRLF = CR LF;
	DIGIT = 0x30..0x39;

	action tag
	{
		tag_start = p;
	}

	action ParsePayloadType
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

	action ParseEncodingName
	{
		pPayload->eCodec = ec_sdp_FindAudioEncodingName((u_char*)tag_start);
	}

	action ParseClockRate
	{
		pPayload->u.audio.uClockrate = pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
	}

	action ParseEncodingParameters
	{
		pPayload->u.audio.uChannels = (u_int8)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
	}

	action error
	{
		uError = ECRIO_SDP_PARSING_A_RTPMAP_LINE_ERROR;
	}

	ptype = DIGIT+>tag %ParsePayloadType;
	encode = any*>tag %ParseEncodingName;
	clockrate = DIGIT+>tag %ParseClockRate;
	encparam = DIGIT+>tag %ParseEncodingParameters;

	rtpmap = ptype SP* <: encode :> "/" clockrate ("/" <: encparam)? :>(CRLF | LF)?;

	main := rtpmap @!error;

	write data;
}%%


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

	%% write init;
	%% write exec;

END:
	return uError;
}
