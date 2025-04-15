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
* @file EcrioSDPParseHeaderA_AMR_parameters.c
* @brief Implementation of SDP module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"

%%{
	machine ec_sdp_Parse_Header_A_AMR_parameters;

	CR = "\r";
	LF = "\n";
	SP = " ";
	CRLF = CR LF;
	DIGIT = 0x30..0x39;
	alnumhyphen = alnum | "-";

	action tag
	{
		tag_start = p;
	}

	action ParseLabel
	{
		eAMRparam = ec_sdp_FindAucioCodecAMRParam((u_char*)tag_start);
	}

	action ParseValue
	{
		uError = ec_sdp_FillAudioCodecAMRParam(pPayload, (u_char*)tag_start, eAMRparam);
	}

	action error
	{
		uError = ECRIO_SDP_PARSING_AMR_PARAMETER_ERROR;
	}

	label = alnumhyphen*>tag %ParseLabel;
	value = DIGIT+>tag %ParseValue;

	parameters = label SP* "=" SP* value (SP* "," SP* value)* SP*;
	format = parameters (SP* ";" SP* parameters)* (";"{1})? :>(CRLF | LF)?; # if the last ";" (separator) remains... it may be a bug.

	main := format @!error;

	write data;
}%%

#if !defined(DISABLE_CODEC_AMR_NB) || !defined(DISABLE_CODEC_AMR_WB)

typedef enum
{
	SDP_AudioCodecAMR_empty = 0,
	SDP_AudioCodecAMR_mode_set,
	SDP_AudioCodecAMR_octet_align,
	SDP_AudioCodecAMR_mode_change_period,
	SDP_AudioCodecAMR_mode_change_capability,
	SDP_AudioCodecAMR_mode_change_neighbor,
	SDP_AudioCodecAMR_crc,
	SDP_AudioCodecAMR_robust_sorting,
	SDP_AudioCodecAMR_max_red,
} ec_SDP_AudioCodecAMREnum;

ec_SDP_AudioCodecAMREnum ec_sdp_FindAucioCodecAMRParam
(
	u_char *pStr
)
{
	ec_SDP_AudioCodecAMREnum eAMRparam = SDP_AudioCodecAMR_empty;

	if (pal_StringNCompare(pStr, ECRIO_SDP_CODEC_AMR_MODE_SET_STRING,
							pal_StringLength(ECRIO_SDP_CODEC_AMR_MODE_SET_STRING)) == 0)
	{
		eAMRparam = SDP_AudioCodecAMR_mode_set;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_CODEC_AMR_OCTET_ALIGN_STRING,
								pal_StringLength(ECRIO_SDP_CODEC_AMR_OCTET_ALIGN_STRING)) == 0)
	{
		eAMRparam = SDP_AudioCodecAMR_octet_align;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_CODEC_AMR_MODE_CHANGE_PERIOD_STRING,
								pal_StringLength(ECRIO_SDP_CODEC_AMR_MODE_CHANGE_PERIOD_STRING)) == 0)
	{
		eAMRparam = SDP_AudioCodecAMR_mode_change_period;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_CODEC_AMR_MODE_CHANGE_CAPABILITY_STRING,
								pal_StringLength(ECRIO_SDP_CODEC_AMR_MODE_CHANGE_CAPABILITY_STRING)) == 0)
	{
		eAMRparam = SDP_AudioCodecAMR_mode_change_capability;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_CODEC_AMR_MODE_CHANGE_NEIGHBOR_STRING,
								pal_StringLength(ECRIO_SDP_CODEC_AMR_MODE_CHANGE_NEIGHBOR_STRING)) == 0)
	{
		eAMRparam = SDP_AudioCodecAMR_mode_change_neighbor;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_CODEC_AMR_CRC_STRING,
								pal_StringLength(ECRIO_SDP_CODEC_AMR_CRC_STRING)) == 0)
	{
		eAMRparam = SDP_AudioCodecAMR_crc;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_CODEC_AMR_ROBUST_SORTING_STRING,
								pal_StringLength(ECRIO_SDP_CODEC_AMR_ROBUST_SORTING_STRING)) == 0)
	{
		eAMRparam = SDP_AudioCodecAMR_robust_sorting;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_CODEC_AMR_MAX_RED_STRING,
								pal_StringLength(ECRIO_SDP_CODEC_AMR_MAX_RED_STRING)) == 0)
	{
		eAMRparam = SDP_AudioCodecAMR_max_red;
	}

	return eAMRparam;
}

u_int32 ec_sdp_FillAudioCodecAMRParam
(
	EcrioSDPPayloadStruct *pPayload,
	u_char *pStr,
	ec_SDP_AudioCodecAMREnum eAMRparam
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;
	u_int16	uVal = 0;
	EcrioAudioCodecAMRStruct param = { 0 };

	switch (eAMRparam)
	{
		case SDP_AudioCodecAMR_mode_set:
		{
			pPayload->u.audio.uOpaque |= ECRIO_SDP_SHOWN_AMR_PARAMETER_MODESET;
			uVal = (u_int16)pal_StringConvertToUNum(pStr, NULL, 10);
			if (KPALErrorNone != pal_MemoryCopy((void *)&param, sizeof(EcrioAudioCodecAMRStruct), (void *)&pPayload->u.audio.uSettings, sizeof(EcrioAudioCodecAMRStruct)))
			{
				uError = ECRIO_SDP_MEMORY_ERROR;
				goto END;
			}
			param.uModeSet |= 1 << uVal;
			if (KPALErrorNone != pal_MemoryCopy((void *)&pPayload->u.audio.uSettings, sizeof(u_int32), (void *)&param, sizeof(EcrioAudioCodecAMRStruct)))
			{
				uError = ECRIO_SDP_MEMORY_ERROR;
				goto END;
			}
		}
		break;

		case SDP_AudioCodecAMR_octet_align:
		{
			pPayload->u.audio.uOpaque |= ECRIO_SDP_SHOWN_AMR_PARAMETER_OCTET_ALIGN;
			uVal = (u_int16)pal_StringConvertToUNum(pStr, NULL, 10);
			if (uVal > 0)
			{
				if (KPALErrorNone != pal_MemoryCopy((void *)&param, sizeof(EcrioAudioCodecAMRStruct), (void *)&pPayload->u.audio.uSettings, sizeof(EcrioAudioCodecAMRStruct)))
				{
					uError = ECRIO_SDP_MEMORY_ERROR;
					goto END;
				}
				param.uOctetAlign = 1;
				if (KPALErrorNone != pal_MemoryCopy((void *)&pPayload->u.audio.uSettings, sizeof(u_int32), (void *)&param, sizeof(EcrioAudioCodecAMRStruct)))
				{
					uError = ECRIO_SDP_MEMORY_ERROR;
					goto END;
				}
			}
		}
		break;

		case SDP_AudioCodecAMR_mode_change_period:
		{
			pPayload->u.audio.uOpaque |= ECRIO_SDP_SHOWN_AMR_PARAMETER_MODE_CHANGE_PERIOD;
			uVal = (u_int16)pal_StringConvertToUNum(pStr, NULL, 10);
			if (uVal == 2)
			{
				if (KPALErrorNone != pal_MemoryCopy((void *)&param, sizeof(EcrioAudioCodecAMRStruct), (void *)&pPayload->u.audio.uSettings, sizeof(EcrioAudioCodecAMRStruct)))
				{
					uError = ECRIO_SDP_MEMORY_ERROR;
					goto END;
				}
				param.uModeChangePeriod = 1;
				if (KPALErrorNone != pal_MemoryCopy((void *)&pPayload->u.audio.uSettings, sizeof(u_int32), (void *)&param, sizeof(EcrioAudioCodecAMRStruct)))
				{
					uError = ECRIO_SDP_MEMORY_ERROR;
					goto END;
				}
			}
			else if (uVal == 1)
			{
				/** This means that the client is not capable of restricting, so we set 0 to
				  * uSettings for mode-change-period field, there is equal to be no operation. */
			}
		}
		break;

		case SDP_AudioCodecAMR_mode_change_capability:
		{
			pPayload->u.audio.uOpaque |= ECRIO_SDP_SHOWN_AMR_PARAMETER_MODE_CHANGE_CAPABILITY;
			uVal = (u_int16)pal_StringConvertToUNum(pStr, NULL, 10);
			if (uVal == 2)
			{
				if (KPALErrorNone != pal_MemoryCopy((void *)&param, sizeof(EcrioAudioCodecAMRStruct), (void *)&pPayload->u.audio.uSettings, sizeof(EcrioAudioCodecAMRStruct)))
				{
					uError = ECRIO_SDP_MEMORY_ERROR;
					goto END;
				}
				param.uModeChangeCapability = 1;
				if (KPALErrorNone != pal_MemoryCopy((void *)&pPayload->u.audio.uSettings, sizeof(u_int32), (void *)&param, sizeof(EcrioAudioCodecAMRStruct)))
				{
					uError = ECRIO_SDP_MEMORY_ERROR;
					goto END;
				}
			}
			else if (uVal == 1)
			{
				/** This means that the client is not capable of restricting, so we set 0 to
				  * uSettings for mode-change-capability field, there is equal to be no operation. */
			}
		}
		break;

		case SDP_AudioCodecAMR_mode_change_neighbor:
		{
			pPayload->u.audio.uOpaque |= ECRIO_SDP_SHOWN_AMR_PARAMETER_MODE_CHANGE_NEIGHBOR;
			uVal = (u_int16)pal_StringConvertToUNum(pStr, NULL, 10);
			if (uVal > 0)
			{
				if (KPALErrorNone != pal_MemoryCopy((void *)&param, sizeof(EcrioAudioCodecAMRStruct), (void *)&pPayload->u.audio.uSettings, sizeof(EcrioAudioCodecAMRStruct)))
				{
					uError = ECRIO_SDP_MEMORY_ERROR;
					goto END;
				}
				param.uModeChangeNeighbor = 1;
				if (KPALErrorNone != pal_MemoryCopy((void *)&pPayload->u.audio.uSettings, sizeof(u_int32), (void *)&param, sizeof(EcrioAudioCodecAMRStruct)))
				{
					uError = ECRIO_SDP_MEMORY_ERROR;
					goto END;
				}
			}
		}
		break;

		case SDP_AudioCodecAMR_crc:
		{
			pPayload->u.audio.uOpaque |= ECRIO_SDP_SHOWN_AMR_PARAMETER_CRC;
			uVal = (u_int16)pal_StringConvertToUNum(pStr, NULL, 10);
			if (uVal > 0)
			{
				if (KPALErrorNone != pal_MemoryCopy((void *)&param, sizeof(EcrioAudioCodecAMRStruct), (void *)&pPayload->u.audio.uSettings, sizeof(EcrioAudioCodecAMRStruct)))
				{
					uError = ECRIO_SDP_MEMORY_ERROR;
					goto END;
				}
				param.uCRC = 1;
				if (KPALErrorNone != pal_MemoryCopy((void *)&pPayload->u.audio.uSettings, sizeof(u_int32), (void *)&param, sizeof(EcrioAudioCodecAMRStruct)))
				{
					uError = ECRIO_SDP_MEMORY_ERROR;
					goto END;
				}
			}
		}
		break;

		case SDP_AudioCodecAMR_robust_sorting:
		{
			pPayload->u.audio.uOpaque |= ECRIO_SDP_SHOWN_AMR_PARAMETER_ROBUST_SORTING;
			uVal = (u_int16)pal_StringConvertToUNum(pStr, NULL, 10);
			if (uVal > 0)
			{
				if (KPALErrorNone != pal_MemoryCopy((void *)&param, sizeof(EcrioAudioCodecAMRStruct), (void *)&pPayload->u.audio.uSettings, sizeof(EcrioAudioCodecAMRStruct)))
				{
					uError = ECRIO_SDP_MEMORY_ERROR;
					goto END;
				}
				param.uRobustSorting = 1;
				if (KPALErrorNone != pal_MemoryCopy((void *)&pPayload->u.audio.uSettings, sizeof(u_int32), (void *)&param, sizeof(EcrioAudioCodecAMRStruct)))
				{
					uError = ECRIO_SDP_MEMORY_ERROR;
					goto END;
				}
			}
		}
		break;

		case SDP_AudioCodecAMR_max_red:
		{
			pPayload->u.audio.uOpaque |= ECRIO_SDP_SHOWN_AMR_PARAMETER_MAX_RED;
			uVal = (u_int16)pal_StringConvertToUNum(pStr, NULL, 10);
			if (uVal > 0)
			{
				if (KPALErrorNone != pal_MemoryCopy((void *)&param, sizeof(EcrioAudioCodecAMRStruct), (void *)&pPayload->u.audio.uSettings, sizeof(EcrioAudioCodecAMRStruct)))
				{
					uError = ECRIO_SDP_MEMORY_ERROR;
					goto END;
				}
				param.uMaxRed = uVal;
				if (KPALErrorNone != pal_MemoryCopy((void *)&pPayload->u.audio.uSettings, sizeof(u_int32), (void *)&param, sizeof(EcrioAudioCodecAMRStruct)))
				{
					uError = ECRIO_SDP_MEMORY_ERROR;
					goto END;
				}
			}
		}
		break;

		default:
			break;
	}

END:
	return uError;
}

/**
 * This function is used to parse AMR specific parameters in a particular
 * format ("a=fmtp") media attribute line and store AMR parameters to the
 * SDP information structure.
 * 
 * The parsing string may have a non-NULL terminator, and pData should be starting
 * AMR specific paramter, coming after format string. It should be specified
 * proper size to uSize. For example, in case of parsing the following part of
 * "a=fmtp" line:
 *   a=fmtp:104 mode-set=2; octet-align=0<CR><LF>...
 * Then pData should be indicated pointer of a head of "mode-set" parameter 
 * that after the format (fmtp:104) string.
 * string and uSize should be set 25 which do not include a CRLF part.
 * 
 * @param[out]	pPayload		Pointer to the RTP payload structure. Must be non-NULL.
 * @param[in]	pData			Pointer to an AMR Parameter in "a=fmtp" line. Must
 *                              be non-NULL.
 * @param[in]	uSize			The size of parsing string.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseAMRParameters
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
	ec_SDP_AudioCodecAMREnum eAMRparam = SDP_AudioCodecAMR_empty;

	pPayload->u.audio.uSettings = 0;

	%% write init;
	%% write exec;

	return uError;
}

#endif
