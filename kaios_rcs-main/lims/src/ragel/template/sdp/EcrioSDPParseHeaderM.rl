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

%%{
	machine ec_sdp_Parse_Header_M;

	CR = "\r";
	LF = "\n";
	SP = " ";
	CRLF = CR LF;
	ALPHA = 0x41..0x5a | 0x61..0x7a;
	DIGIT = 0x30..0x39;
	alphanum = ALPHA | DIGIT;
	token = ( alphanum | "-" | "." | "!" | "%" | "*" | "_" | "+" | "`" | "'" | "~" )+;

	action tag
	{
		tag_start = p;
	}

	action ParseMedia
	{
		pStream->eMediaType = ec_sdp_FindMediaType((u_char*)tag_start);
	}

	action ParsePort
	{
		pStream->uMediaPort = (u_int16)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
	}

	action ParseProtocol
	{
		pStream->eProtocol = ec_sdp_FindMediaProtocol((u_char*)tag_start);
	}

	action ParseMediaFormat
	{
		uError = ec_sdp_FindMediaFormat(pStream, (u_char*)tag_start);
	}

	action error
	{
		uError = ECRIO_SDP_PARSING_M_LINE_ERROR;
	}

	media = token>tag %ParseMedia;
	port = DIGIT+>tag %ParsePort;
	proto = (token ("/" token)*)>tag %ParseProtocol;
	fmt = token>tag %ParseMediaFormat;

	M = 'm' SP* "=" SP*<: media SP port SP proto (SP fmt)* (SP{1})? :>(CRLF | LF)?;

	main := M @!error;

	write data;
}%%

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

	%% write init;
	%% write exec;

	return uError;
}
