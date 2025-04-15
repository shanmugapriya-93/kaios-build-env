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
* @file EcrioSDPParseHeaderA.c
* @brief Implementation of SDP module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"

%%{
	machine ec_sdp_Parse_Header_A;

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

	action ParseField
	{
		eType = ec_sdp_FindMediaAttributeType(pStream, (u_char*)tag_start);
	}

	action ParseValue
	{
		uError = ec_sdp_HandleMediaAttributeValue(pStrings, pStream, (u_char*)tag_start, (u_char*)p, eType);
	}

	action error
	{
		uError = ECRIO_SDP_PARSING_A_LINE_ERROR;
	}

	field = token>tag %ParseField;
	value = any*>tag %ParseValue;

	A = 'a' SP* "=" SP*<: ((field ":" value) | (field)) :>(CRLF | LF)?;

	main := A @!error;

	write data;
}%%


typedef enum
{
	SDP_ATTRIBUTE_TYPE_NONE = 0,
	SDP_ATTRIBUTE_TYPE_MAXPTIME,
	SDP_ATTRIBUTE_TYPE_PTIME,
	SDP_ATTRIBUTE_TYPE_RTPMAP,
	SDP_ATTRIBUTE_TYPE_FMTP,
	SDP_ATTRIBUTE_TYPE_ACCEPT_TYPES,
	SDP_ATTRIBUTE_TYPE_ACCEPT_WRAPPED_TYPES,
	SDP_ATTRIBUTE_TYPE_PATH,
	SDP_ATTRIBUTE_TYPE_SETUP,
	SDP_ATTRIBUTE_TYPE_FILE_SELECTOR,
	SDP_ATTRIBUTE_TYPE_FILE_DISPOSITION,
	SDP_ATTRIBUTE_TYPE_FILE_TRANSER_ID,
	SDP_ATTRIBUTE_TYPE_FILE_DATE,
	SDP_ATTRIBUTE_TYPE_FILE_ICON,
	SDP_ATTRIBUTE_TYPE_FILE_RANGE,
	SDP_ATTRIBUTE_TYPE_CURRENT_STATUS,
	SDP_ATTRIBUTE_TYPE_DESIRED_STATUS,
	SDP_ATTRIBUTE_TYPE_CONFIRM_STATUS,
	SDP_ATTRIBUTE_TYPE_FINGERPRINT,
} ec_SDP_MediaAttributeTypeEnum;

ec_SDP_MediaAttributeTypeEnum ec_sdp_FindMediaAttributeType
(
	EcrioSDPStreamStruct *pStream,
	u_char *pStr
)
{
	ec_SDP_MediaAttributeTypeEnum eType = SDP_ATTRIBUTE_TYPE_NONE;

	switch (pStream->eMediaType)
	{
		case ECRIO_SDP_MEDIA_TYPE_AUDIO:
		{
			if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_DIRECTION_SENDONLY_STRING,
									pal_StringLength(ECRIO_SDP_MEDIA_DIRECTION_SENDONLY_STRING)) == 0)
			{
				pStream->eDirection = ECRIO_SDP_MEDIA_DIRECTION_SENDONLY;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_DIRECTION_RECVONLY_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_DIRECTION_RECVONLY_STRING)) == 0)
			{
				pStream->eDirection = ECRIO_SDP_MEDIA_DIRECTION_RECVONLY;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_DIRECTION_INACTIVE_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_DIRECTION_INACTIVE_STRING)) == 0)
			{
				pStream->eDirection = ECRIO_SDP_MEDIA_DIRECTION_INACTIVE;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_DIRECTION_SENDRECV_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_DIRECTION_SENDRECV_STRING)) == 0)
			{
				pStream->eDirection = ECRIO_SDP_MEDIA_DIRECTION_SENDRECV;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_MAXPTIME_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_MAXPTIME_STRING)) == 0)
			{
				eType = SDP_ATTRIBUTE_TYPE_MAXPTIME;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_PTIME_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_PTIME_STRING)) == 0)
			{
				eType = SDP_ATTRIBUTE_TYPE_PTIME;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_RTPMAP_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_RTPMAP_STRING)) == 0)
			{
				eType = SDP_ATTRIBUTE_TYPE_RTPMAP;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FMTP_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FMTP_STRING)) == 0)
			{
				eType = SDP_ATTRIBUTE_TYPE_FMTP;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_CURRENT_STATUS_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_CURRENT_STATUS_STRING)) == 0)
			{
				eType = SDP_ATTRIBUTE_TYPE_CURRENT_STATUS;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_DESIRED_STATUS_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_DESIRED_STATUS_STRING)) == 0)
			{
				eType = SDP_ATTRIBUTE_TYPE_DESIRED_STATUS;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_CONFIRM_STATUS_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_CONFIRM_STATUS_STRING)) == 0)
			{
				eType = SDP_ATTRIBUTE_TYPE_CONFIRM_STATUS;
			}
		}
		break;

		case ECRIO_SDP_MEDIA_TYPE_MSRP:
		{
			if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_DIRECTION_SENDONLY_STRING,
									pal_StringLength(ECRIO_SDP_MEDIA_DIRECTION_SENDONLY_STRING)) == 0)
			{
				pStream->eDirection = ECRIO_SDP_MEDIA_DIRECTION_SENDONLY;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_DIRECTION_RECVONLY_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_DIRECTION_RECVONLY_STRING)) == 0)
			{
				pStream->eDirection = ECRIO_SDP_MEDIA_DIRECTION_RECVONLY;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_DIRECTION_INACTIVE_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_DIRECTION_INACTIVE_STRING)) == 0)
			{
				pStream->eDirection = ECRIO_SDP_MEDIA_DIRECTION_INACTIVE;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_DIRECTION_SENDRECV_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_DIRECTION_SENDRECV_STRING)) == 0)
			{
				pStream->eDirection = ECRIO_SDP_MEDIA_DIRECTION_SENDRECV;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_MSRP_CEMA_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_MSRP_CEMA_STRING)) == 0)
			{
				if (pStream->eMediaType == ECRIO_SDP_MEDIA_TYPE_MSRP)
				{
					pStream->u.msrp.bCEMA = Enum_TRUE;
				}
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_CHATROOM_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_CHATROOM_STRING)) == 0)
			{
				u_char* pTmp = pStr + pal_StringLength(ECRIO_SDP_MEDIA_CHATROOM_STRING) + 1;
				if (pal_StringNCompare(pTmp, ECRIO_SDP_CLOSED_GROUPCHAT_STRING,
										pal_StringLength(ECRIO_SDP_CLOSED_GROUPCHAT_STRING)) == 0)
				{
					if (pStream->eMediaType == ECRIO_SDP_MEDIA_TYPE_MSRP)
					{
						pStream->u.msrp.bIsClosed = Enum_TRUE;
					}
				}
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_ACCEPT_TYPES_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_ACCEPT_TYPES_STRING)) == 0)
			{
				eType = SDP_ATTRIBUTE_TYPE_ACCEPT_TYPES;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_ACCEPT_WRAPPED_TYPES_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_ACCEPT_WRAPPED_TYPES_STRING)) == 0)
			{
				eType = SDP_ATTRIBUTE_TYPE_ACCEPT_WRAPPED_TYPES;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_PATH_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_PATH_STRING)) == 0)
			{
				eType = SDP_ATTRIBUTE_TYPE_PATH;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_SETUP_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_SETUP_STRING)) == 0)
			{
				eType = SDP_ATTRIBUTE_TYPE_SETUP;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FILE_SELECTOR_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FILE_SELECTOR_STRING)) == 0)
			{
				eType = SDP_ATTRIBUTE_TYPE_FILE_SELECTOR;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FILE_DISPOSITION_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FILE_DISPOSITION_STRING)) == 0)
			{
				eType = SDP_ATTRIBUTE_TYPE_FILE_DISPOSITION;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FILE_TRANSFER_ID_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FILE_TRANSFER_ID_STRING)) == 0)
			{
				eType = SDP_ATTRIBUTE_TYPE_FILE_TRANSER_ID;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FILE_DATE_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FILE_DATE_STRING)) == 0)
			{
				eType = SDP_ATTRIBUTE_TYPE_FILE_DATE;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FILE_ICON_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FILE_ICON_STRING)) == 0)
			{
				eType = SDP_ATTRIBUTE_TYPE_FILE_ICON;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FILE_RANGE_STRING,
										pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FILE_RANGE_STRING)) == 0)
			{
				eType = SDP_ATTRIBUTE_TYPE_FILE_RANGE;
			}
			else if (pal_StringNCompare(pStr, ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FINGERPRINT,
										pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FINGERPRINT)) == 0)
			{
				eType = SDP_ATTRIBUTE_TYPE_FINGERPRINT;
			}						
		}
		break;

		default:
			break;
	}

	return eType;
}

u_int32 ec_sdp_HandleMediaAttributeValue
(
	EcrioSDPStringStruct *pStrings,
	EcrioSDPStreamStruct *pStream,
	u_char *pStart,
	u_char *pEnd,
	ec_SDP_MediaAttributeTypeEnum eType
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;

	switch (pStream->eMediaType)
	{
		case ECRIO_SDP_MEDIA_TYPE_AUDIO:
		{
			switch (eType)
			{
				case SDP_ATTRIBUTE_TYPE_MAXPTIME:
				{
					pStream->u.audio.uMaxPtime = (u_int16)pal_StringConvertToUNum(pStart, NULL, 10);
				}
				break;

				case SDP_ATTRIBUTE_TYPE_PTIME:
				{
					pStream->u.audio.uPtime = (u_int16)pal_StringConvertToUNum(pStart, NULL, 10);
				}
				break;

				case SDP_ATTRIBUTE_TYPE_RTPMAP:
				{
					uError = ec_sdp_ParseRtpmap(pStream, pStart, (pEnd - pStart));
				}
				break;

				case SDP_ATTRIBUTE_TYPE_FMTP:
				{
					uError = ec_sdp_ParseFmtp(pStream, pStart, (pEnd - pStart));
				}
				break;

				case SDP_ATTRIBUTE_TYPE_CURRENT_STATUS:
				{
					uError = ec_sdp_ParseCurrentStatus(pStream, Enum_TRUE, pStart, (pEnd - pStart));
				}
				break;

				case SDP_ATTRIBUTE_TYPE_CONFIRM_STATUS:
				{
					uError = ec_sdp_ParseCurrentStatus(pStream, Enum_FALSE, pStart, (pEnd - pStart));
				}
				break;

				case SDP_ATTRIBUTE_TYPE_DESIRED_STATUS:
				{
					uError = ec_sdp_ParseDesiredStatus(pStream, pStart, (pEnd - pStart));
				}
				break;

				default:
					break;
			}
		}
		break;

		case ECRIO_SDP_MEDIA_TYPE_MSRP:
		{
			switch (eType)
			{
				case SDP_ATTRIBUTE_TYPE_ACCEPT_TYPES:
				{
					uError = ec_sdp_ParseAcceptTypes((u_int32*)&pStream->u.msrp.eAcceptTypes, pStart, (pEnd - pStart));
				}
				break;

				case SDP_ATTRIBUTE_TYPE_ACCEPT_WRAPPED_TYPES:
				{
					uError = ec_sdp_ParseAcceptTypes((u_int32*)&pStream->u.msrp.eAcceptWrappedTypes, pStart, (pEnd - pStart));
				}
				break;

				case SDP_ATTRIBUTE_TYPE_PATH:
				{
					uError = ec_sdp_StringCopy(pStrings, (void **)&pStream->u.msrp.pPath, pStart, (pEnd - pStart));
					if (uError != ECRIO_SDP_NO_ERROR)
					{
						goto END;
					}
				}
				break;

				case SDP_ATTRIBUTE_TYPE_SETUP:
				{
					if (pal_StringNCompare(pStart, ECRIO_SDP_MEDIA_ATTRIBUTE_SETUP_ACTIVE_STRING,
											pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_SETUP_ACTIVE_STRING)) == 0)
					{
						pStream->u.msrp.eSetup = ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_ACTIVE;
					}
					else if (pal_StringNCompare(pStart, ECRIO_SDP_MEDIA_ATTRIBUTE_SETUP_PASSIVE_STRING,
												pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_SETUP_PASSIVE_STRING)) == 0)
					{
						pStream->u.msrp.eSetup = ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_PASSIVE;
					}
					else if (pal_StringNCompare(pStart, ECRIO_SDP_MEDIA_ATTRIBUTE_SETUP_ACTPASS_STRING,
												pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_SETUP_ACTPASS_STRING)) == 0)
					{
						pStream->u.msrp.eSetup = ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_ACTPASS;
					}
					else if (pal_StringNCompare(pStart, ECRIO_SDP_MEDIA_ATTRIBUTE_SETUP_HOLDCONN_STRING,
												pal_StringLength(ECRIO_SDP_MEDIA_ATTRIBUTE_SETUP_HOLDCONN_STRING)) == 0)
					{
						pStream->u.msrp.eSetup = ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_HOLDCONN;
					}
					else
					{
						pStream->u.msrp.eSetup = ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_NONE;
					}
				}
				break;

				case SDP_ATTRIBUTE_TYPE_FILE_SELECTOR:
				{
					uError = ec_sdp_ParseFileSelector(pStrings, pStream, pStart, (pEnd - pStart));
				}
				break;

				case SDP_ATTRIBUTE_TYPE_FILE_DISPOSITION:
				{
					uError = ec_sdp_StringCopy(pStrings, (void **)&pStream->u.msrp.pFileDisposition, pStart, (pEnd - pStart));
					if (uError != ECRIO_SDP_NO_ERROR)
					{
						goto END;
					}
				}
				break;

				case SDP_ATTRIBUTE_TYPE_FILE_TRANSER_ID:
				{
					uError = ec_sdp_StringCopy(pStrings, (void **)&pStream->u.msrp.pFileTransferId, pStart, (pEnd - pStart));
					if (uError != ECRIO_SDP_NO_ERROR)
					{
						goto END;
					}
				}
				break;

				case SDP_ATTRIBUTE_TYPE_FILE_DATE:
				{
					uError = ec_sdp_ParseFileDate(pStrings, pStream, pStart, (pEnd - pStart));
				}
				break;

				case SDP_ATTRIBUTE_TYPE_FILE_ICON:
				{
					uError = ec_sdp_StringCopy(pStrings, (void **)&pStream->u.msrp.pFileIcon, pStart, (pEnd - pStart));
					if (uError != ECRIO_SDP_NO_ERROR)
					{
						goto END;
					}
				}
				break;

				case SDP_ATTRIBUTE_TYPE_FILE_RANGE:
				{
					uError = ec_sdp_ParseFileRange(pStream, pStart, (pEnd - pStart));
				}
				break;
				case SDP_ATTRIBUTE_TYPE_FINGERPRINT:
				{
					uError = ec_sdp_StringCopy(pStrings, (void **)&pStream->u.msrp.pFingerPrint, pStart, (pEnd - pStart));
					if (uError != ECRIO_SDP_NO_ERROR)
					{
						goto END;
					}
				}
				break;
				default:
					break;
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
 * This is used to parse SDP Attributes ("a=") line and fill up the SDP structure.
 * 
 * @param[in]	pStrings		Pointer to the string structure. Must be non-NULL.
 * @param[out]	pStream			Pointer to the stream structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the SDP Attributes ("a=") line.
 * 								Must be non-NULL.
 * @param[in]	uSize			The size of SDP buffer.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseHeaderA
(
	EcrioSDPStringStruct *pStrings,
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
	ec_SDP_MediaAttributeTypeEnum	eType = SDP_ATTRIBUTE_TYPE_NONE;

	%% write init;
	%% write exec;

	if (pStream->eMediaType == ECRIO_SDP_MEDIA_TYPE_MSRP)
	{
		if (pStream->u.msrp.fileSelector.pType != NULL)
		{
			pStream->u.msrp.eAcceptTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_GENERAL;
		}
	}

	return uError;
}
