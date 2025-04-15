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
 * @file EcrioParse.c
 * @brief Implementation of the CPIM module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioCPIM.h"
#include "EcrioCPIMCommon.h"

%%{
	machine ec_cpim_Parser;

	CRLF = "\r\n";
	DQUOTE = "\"";
	SP = " ";

	alphanum = [a-zA-Z0-9];

	LWS    = ((" "* CRLF)? " "+);
	SWS    = (LWS?);

	token  = (alphanum | "-" | "=" | "." | "!" | "%" | "*" | "_" | "+" | "`" | "'" | "~" )+;
	word   = (alphanum | "-" | "=" | "." | "!" | "%" | "*" | "_" | "+" | "`" | "'" | "~" | "(" | ")" | "<" | ">" | ":" | "@" | "\\" | DQUOTE | "/" | "[" | "]" | "?" | "{" | "}")+;
	uri_token   = (alphanum | "-" | "=" | "." | "!" | "%" | "*" | "_" | "+" | "'" | "~" | "(" | ")" | ":" | ";" | "@" | DQUOTE | "/" | "[" | "]" | "?")+;

	qdtext = (LWS | 0x21 | 0x23..0x5B | 0x5D..0x7E);
	quoted_pair = ("\\" (0x00..0x09 | 0x0B..0x0C | 0x0E..0x7F));
	quoted_string = (SWS DQUOTE (qdtext | quoted_pair)* DQUOTE);

	sub_colon = ((" " | "\t")* ":");

	COMMA  = (SWS "," SWS);
	RAQUOT = (">" SWS);
	LAQUOT = (SWS "<");
	EQUAL  = (SWS "=" SWS);
	SEMI   = (SWS ";" SWS);
	HCOLON = ((" " | "\t")* ":" SWS);

	action tag
	{
		tag_start = p;
	}

	action ParseToHeader
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pToAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}

	action ParseFromHeader
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pFromAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}

	action ParseDisplayName
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}

	action ParseDispoNotifParam
	{
		if (pal_StringNCompare((u_char*)tag_start, CPIM_NEGATIVE_DELIVERY_STRING,
				pal_StringLength(CPIM_NEGATIVE_DELIVERY_STRING)) == 0)
		{
			pStruct->dnType |= CPIMDispoNotifReq_Negative;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_POSITIVE_DELIVERY_STRING,
					pal_StringLength(CPIM_POSITIVE_DELIVERY_STRING)) == 0)
		{
			pStruct->dnType |= CPIMDispoNotifReq_Positive;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_DISPLAY_STRING,
					pal_StringLength(CPIM_DISPLAY_STRING)) == 0)
		{
			pStruct->dnType |= CPIMDispoNotifReq_Display;
		}
	}

	action ParseContentTypeParam
	{
		if (bIsMultiPart == Enum_TRUE)
		{
			pal_MemorySet(cBoundary, 0, CPIM_BOUNDARY_LENGTH);
			if (NULL == pal_StringNCopy(cBoundary, CPIM_BOUNDARY_LENGTH, (u_char*)tag_start + 1, (p - tag_start - 2)))
			{
				CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.", __FUNCTION__, __LINE__);
				uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
				goto END;
			}
		}
	}

	action ParseContentTypeHeader
	{
		if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_MULTIPART_MIXED_STRING,
				pal_StringLength(CPIM_CONTENT_TYPE_MULTIPART_MIXED_STRING)) == 0)
		{
			bIsMultiPart = Enum_TRUE;
		}
		else if (pal_StringNCompare((u_char*)tag_start, (u_char*)"text/plain",
					pal_StringLength((u_char*)"text/plain")) == 0)
		{
			eConType = CPIMContentType_Text;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_IMDN_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_IMDN_STRING)) == 0)
		{
			eConType = CPIMContentType_IMDN;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_FILE_TRANSFER_HTTP_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_FILE_TRANSFER_HTTP_STRING)) == 0)
		{
			eConType = CPIMContentType_FileTransferOverHTTP;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_PUSH_LOCATION_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_PUSH_LOCATION_STRING)) == 0)
		{
			eConType = CPIMContentType_PushLocation;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_MESSAGE_REVOKE_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_MESSAGE_REVOKE_STRING)) == 0)
		{
			eConType = CPIMContentType_MessageRevoke;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_PRIVACY_MANAGEMENT_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_PRIVACY_MANAGEMENT_STRING)) == 0)
		{
			eConType = CPIMContentType_PrivacyManagement;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_LINK_REPORT_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_LINK_REPORT_STRING)) == 0)
		{
			eConType = CPIMContentType_LinkReport;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SPAM_REPORT_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SPAM_REPORT_STRING)) == 0)
		{
			eConType = CPIMContentType_SpamReport;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_RICHCARD_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_RICHCARD_STRING)) == 0)
		{
			eConType = CPIMContentType_RichCard;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SUGGESTED_CHIPLIST_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SUGGESTED_CHIPLIST_STRING)) == 0)
		{
			eConType = CPIMContentType_SuggestedChipList;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SUGGESTED_RESPONSE_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SUGGESTED_RESPONSE_STRING)) == 0)
		{
			eConType = CPIMContentType_SuggestionResponse;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SHARED_CLIENT_DATA_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SHARED_CLIENT_DATA_STRING)) == 0)
		{
			eConType = CPIMContentType_SharedClientData;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_CPM_GROUPDATA_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_CPM_GROUPDATA_STRING)) == 0)
		{
			eConType = CPIMContentType_CpmGroupData;
		}
		else
		{
			eConType = CPIMContentType_Specified;
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pContentType,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}

	action ParseDateTimeHeader
	{
		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pDateTime,
			(u_char*)tag_start,
			(p - tag_start));
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}

	action ParseNameSpaceImdn
	{
		bIsImdnNS = Enum_TRUE;
	}

	action ParseNameSpaceMaap
	{
		bIsMaapNS = Enum_TRUE;
	}

	action ParseNameSpaceMyFeatures
	{
		bIsMyFeaturesNS = Enum_TRUE;
	}

	action ParseImdnMessageID
	{
		if (bIsImdnNS == Enum_TRUE)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pMsgId,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
			pStruct->uMsgIdLen = pal_StringLength(pStruct->pMsgId);
		}
	}

	action ParseMaapTrafficType
	{
		if (bIsMaapNS == Enum_TRUE)
		{
			if (pal_StringNCompare((u_char*)tag_start, CPIM_ADVERTISEMENT_STRING,
						pal_StringLength(CPIM_ADVERTISEMENT_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Advertisement;
			}
			else if (pal_StringNCompare((u_char*)tag_start, CPIM_PAYMENT_STRING,
						pal_StringLength(CPIM_PAYMENT_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Payment;
			}
			else if (pal_StringNCompare((u_char*)tag_start, CPIM_PREMIUM_STRING,
						pal_StringLength(CPIM_PREMIUM_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Premium;
			}
			else if (pal_StringNCompare((u_char*)tag_start, CPIM_SUBSCRIPTION_STRING,
						pal_StringLength(CPIM_SUBSCRIPTION_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Subscription;
			}
			else if (pal_StringNCompare((u_char*)tag_start, CPIM_PLUGIN_STRING,
						pal_StringLength(CPIM_PLUGIN_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Plugin;
			}
			else
			{
				pStruct->trfType = CPIMTrafficType_None;
			}
		}
	}

	action ParseMyFeaturesPANI
	{
		if (bIsMyFeaturesNS == Enum_TRUE)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pPANI,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}

	action ParseContentDispositionHeader
	{
		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pContentDisposition,
			(u_char*)tag_start,
			(p - tag_start));
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}

	action ParseContentIDHeader
	{
		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pContentId,
			(u_char*)tag_start,
			(p - tag_start));
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}

	action ParseContentTransferEncodingHeader
	{
		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pContentTransferEncoding,
			(u_char*)tag_start,
			(p - tag_start));
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}

	action ParseIMDNRecordRouteHeader
	{
		CPIMLOGI(c->logHandle, KLogTypeGeneral, "%s:%u\tIMDN-Record-Route.", __FUNCTION__, __LINE__);
	}

	action ParseIMDNRouteHeader
	{
		CPIMLOGI(c->logHandle, KLogTypeGeneral, "%s:%u\tIMDN-Route.", __FUNCTION__, __LINE__);
	}

	action ParseSubjectHeader
	{
		CPIMLOGI(c->logHandle, KLogTypeGeneral, "%s:%u\tSubject.", __FUNCTION__, __LINE__);
	}

	action ParseImdnOriginalTo
	{
		CPIMLOGI(c->logHandle, KLogTypeGeneral, "%s:%u\timdn.Original-To.", __FUNCTION__, __LINE__);
	}

	action ParseContentLengthHeader
	{
		pal_MemorySet(cTemp, 0, 8);
		if (NULL == pal_StringNCopy(cTemp, 8, (u_char*)tag_start, (p - tag_start)))
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		uCntLen = pal_StringToNum(cTemp, NULL);
	}

	action ParseMIMEMessage
	{
		if (bIsMultiPart == Enum_FALSE)
		{
			if (uCntLen == 0 || uCntLen > (p - tag_start))
			{
				uCntLen = p - tag_start;
			}

			c->buff[0].uSize = 0;
			uCPIMError = ec_cpim_MaintenanceBuffer(&c->buff[0], uCntLen);
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tThe working buffer maintenance error.", __FUNCTION__, __LINE__);
				uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
				goto END;
			}
			pal_MemorySet(c->buff[0].pData, 0, c->buff[0].uContainerSize);
			pal_MemoryCopy(c->buff[0].pData, c->buff[0].uContainerSize, (u_char*)tag_start, uCntLen);
			c->buff[0].uSize = uCntLen;

			pStruct->uNumOfBuffers = 1;
			pStruct->buff[0].eContentType = eConType;
			if (eConType == CPIMContentType_Specified)
			{
				pStruct->buff[0].pContentType = pContentType;
			}
			else
			{
				pStruct->buff[0].pContentType = NULL;
			}
			pStruct->buff[0].pMsgBody = c->buff[0].pData;
			pStruct->buff[0].uMsgLen = c->buff[0].uSize;
			if (pContentId != NULL)
			{
				pStruct->buff[0].pContentId = pContentId;
			}
			else
			{
				pStruct->buff[0].pContentId = NULL;
			}
			if (pContentDisposition != NULL)
			{
				pStruct->buff[0].pContentDisposition = pContentDisposition;
			}
			else
			{
				pStruct->buff[0].pContentDisposition = NULL;
			}
			if (pContentTransferEncoding != NULL)
			{
				pStruct->buff[0].pContentTransferEncoding = pContentTransferEncoding;
			}
			else
			{
				pStruct->buff[0].pContentTransferEncoding = NULL;
			}
		}
		else
		{
			uCPIMError = ec_cpim_ParseMultiPartMessage(c, pStruct, cBoundary, (u_char*)tag_start, uCntLen);
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				uCPIMError = ECRIO_CPIM_PARSING_INSUFFICIENT_ERROR;
				goto END;
			}
		}
	}

	action ParseHeaderOther
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}

	action error
	{
		/** This is a general error, but usually no CRLF at the end of line. */
		uCPIMError = ECRIO_CPIM_PARSING_INSUFFICIENT_ERROR;
	}

	action ignore
	{
		/** Ignore when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}

	display_name = ((token LWS)* | quoted_string);
	to_name_addr = ((display_name)? (LAQUOT)? uri_token>tag %ParseToHeader (RAQUOT)?);
	from_name_addr = ((display_name)?>tag %ParseDisplayName (LAQUOT)? uri_token>tag %ParseFromHeader (RAQUOT)?);
	disp_param = (word>tag %ParseDispoNotifParam);
	m_parameter = (word EQUAL (SWS word>tag %ParseContentTypeParam));
	media_type = (word>tag %ParseContentTypeHeader (SEMI m_parameter)*);

	prefix_imdn = ("imdn" SWS word>tag %ParseNameSpaceImdn);
	prefix_maap = ("maap" SWS word>tag %ParseNameSpaceMaap);
	prefix_MyFeatures = ("MyFeatures" SWS word>tag %ParseNameSpaceMyFeatures);
	ns_prefix = (prefix_imdn | prefix_maap | prefix_MyFeatures) >tag >1 | (word | SP)* >tag >0;

	imdn_header_Message_ID = ("Message-ID" HCOLON word>tag %ParseImdnMessageID);
	imdn_header_Disposition_Notification = ("Disposition-Notification" HCOLON disp_param (COMMA disp_param)*);
	imdn_header_Original_To = ("Original-To" HCOLON word>tag %ParseImdnOriginalTo);
	imdn_header_Record_Route = ("IMDN-Record-Route" HCOLON word>tag %ParseIMDNRecordRouteHeader);
	imdn_header_name = 	(imdn_header_Message_ID |
						 imdn_header_Disposition_Notification |
						 imdn_header_Original_To |
						 imdn_header_Record_Route
						) >tag >1 |
						(word | SP)* >tag >0;

	To = ("To" HCOLON to_name_addr) SP* CRLF;
	From = ("From" HCOLON from_name_addr) SP* CRLF;
	DateTime = ("DateTime" HCOLON word>tag %ParseDateTimeHeader) SP* CRLF;
	Subject = ("Subject" sub_colon ((word | SP)*)?>tag %ParseSubjectHeader) CRLF;
	NameSpace = ("NS" HCOLON ns_prefix) SP* CRLF;
	NS_imdn = ("imdn." imdn_header_name) SP* CRLF;
	IMDN_Route = ("IMDN-Route" HCOLON word>tag %ParseIMDNRouteHeader) SP* CRLF;
	IMDN_Record_Route = ("IMDN-Record-Route" HCOLON word>tag %ParseIMDNRecordRouteHeader) SP* CRLF;
	maap_Traffic_Type = ("maap.Traffic-Type" HCOLON word>tag %ParseMaapTrafficType) SP* CRLF;
	MyFeatures_PANI = ("MyFeatures.PANI" HCOLON word>tag ((SEMI word)*)? %ParseMyFeaturesPANI) SP* CRLF;
	Other = (token* sub_colon ((word | SP)*)? %ParseHeaderOther) CRLF;

	Content_Type = (("Content-Type" | "Content-type") HCOLON media_type) SP* CRLF;
	Content_Disposition = ("Content-Disposition" HCOLON word>tag %ParseContentDispositionHeader (SEMI token)*) SP* CRLF;
	Content_Length = (("Content-Length" | "Content-length") HCOLON digit+>tag %ParseContentLengthHeader) SP* CRLF;
	Content_ID = ("Content-ID" HCOLON word>tag %ParseContentIDHeader) SP* CRLF;
	Content_Transfer_Encoding = (("Content-Transfer-Encoding" | "CONTENT-TRANSFER-ENCODING") HCOLON word>tag %ParseContentTransferEncodingHeader) SP* CRLF;

	message_body = any*>tag %ParseMIMEMessage;

	message_header =	(To |
						 From |
						 DateTime |
						 Subject |
						 NameSpace |
						 NS_imdn |
						 IMDN_Record_Route |
						 IMDN_Route |
						 maap_Traffic_Type |
						 MyFeatures_PANI |
						 Other
						)>tag >1;

	content_header =	(Content_Type |
						 Content_Disposition |
						 Content_Length |
						 Content_ID |
						 Content_Transfer_Encoding)>tag >1;

	cpim_message = (message_header)* CRLF (content_header)* (CRLF message_body)?;
	main := cpim_message @!error;

	write data;
}%%


u_int32 ec_cpim_ParseMultiPartMessage
(
	EcrioCPIMStruct *c,
	CPIMMessageStruct *pStruct,
	u_char *pBoundary,
	u_char *pData,
	u_int32 uLen
)
{
	u_int32 uCPIMError = ECRIO_CPIM_NO_ERROR;
	u_int32 i;
	u_char *pCur = NULL, *pStart = NULL, *pEnd = NULL;

	/** Check parameter validity. */
	if (c == NULL)
	{
		return  ECRIO_CPIM_INVALID_HANDLE;
	}

	if (pStruct == NULL)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_2;
	}

	if (pBoundary == NULL)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_3;
	}

	if (pData == NULL)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_4;
	}

	if (uLen == 0)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_5;
	}

	CPIMLOGI(c->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pBoundary[0] == '\0')
	{
		CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tboundary string not specified.", __FUNCTION__, __LINE__);
		uCPIMError = ECRIO_CPIM_PARSING_INSUFFICIENT_ERROR;
		goto END;
	}

	pCur = pData;

	/** Initialize output buffer structure */
	pStruct->uNumOfBuffers = 0;
	pal_MemorySet(&pStruct->buff[0], 0, sizeof(CPIMMessageBufferStruct) * CPIM_MSG_NUM);

	for (i = 0; i < CPIM_MSG_NUM; i++)
	{
		/* 1. Find boundary string */
		pStart = pal_StringFindSubString(pCur, pBoundary);
		if (pStart == NULL)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tboundary not found.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_PARSING_INSUFFICIENT_ERROR;
			goto END;
		}

		/** 2. pStart pointer goes to boundary + crlf, there is head of MIME message body. */
		pStart = pStart + pal_StringLength(pBoundary) + 2;

		if (uLen <= (pStart - pData))
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tInvalid position of pStart.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_PARSING_INSUFFICIENT_ERROR;
			goto END;
		}

		/** 3. Find next boundary, 4 bytes ("--" string + crlf) before of pEnd is tail of MIME message body. */
		pEnd = pal_MemorySearch(pStart, uLen - (pStart - pData), pBoundary, pal_StringLength(pBoundary));

		if (pEnd == NULL)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tboundary not found.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_PARSING_INSUFFICIENT_ERROR;
			goto END;
		}
		pCur = pEnd;
		pEnd -= 4;

		/** 4. Copy MIME message to working buffer. */
		c->work.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->work, (u_int32)(pEnd - pStart));
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tThe working buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->work.pData, 0, c->work.uContainerSize);
		pal_MemoryCopy((void *)c->work.pData, c->work.uContainerSize, (const void *)pStart, (u_int32)(pEnd - pStart));
		c->work.uSize = (u_int32)(pEnd - pStart);

		/** 5. Parse MIME message */
		uCPIMError = ec_cpim_ParseMIMEMessage(c, pStruct, c->work.pData, (pEnd - pStart), i);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tParsing MIME message error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_PARSING_INSUFFICIENT_ERROR;
			goto END;
		}

		/** 6. Check whether the next boundary indicates a term boundary (exists "--" to bottom). */
		if (pal_StringNCompare(pCur + pal_StringLength(pBoundary), (u_char*)"--", 2) == 0)
		{
			CPIMLOGI(c->logHandle, KLogTypeGeneral, "%s:%u\tFound final boundary.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_NO_ERROR;
			break;
		}
	}

END:
	CPIMLOGI(c->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return uCPIMError;
}

u_int32 EcrioCPIMHandler
(
	CPIM_HANDLE handle,
	CPIMMessageStruct *pStruct,
	u_char *pData,
	u_int32 uLen
)
{
	EcrioCPIMStruct *c = NULL;
	u_int32 uCPIMError = ECRIO_CPIM_NO_ERROR;
	u_int32 uCntLen = 0;
	u_int32 uStrLen = 0;
	s_int32 cs = 1;
	const char *p;
	const char *pe;
	const char *eof;
	const char *tag_start = NULL;
	u_char cTemp[8];
	u_char cBoundary[CPIM_BOUNDARY_LENGTH] = { 0 };
	u_char *pBuff = NULL;
	u_char *pContentType = NULL;
	u_char *pContentDisposition = NULL;
	u_char *pContentId = NULL;
	u_char *pContentTransferEncoding = NULL;
	u_char *pAtSign = NULL;
	u_char *pSemiColon = NULL;
	BoolEnum bIsImdnNS = Enum_FALSE;
	BoolEnum bIsMaapNS = Enum_FALSE;
	BoolEnum bIsMyFeaturesNS = Enum_FALSE;
	BoolEnum bIsMultiPart = Enum_FALSE;
	CPIMContentTypeEnum eConType = CPIMContentType_None;

	/** Check parameter validity. */
	if (handle == NULL)
	{
		return  ECRIO_CPIM_INVALID_HANDLE;
	}

	if (pStruct == NULL)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_2;
	}

	if (pData == NULL)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_3;
	}

	if (uLen == 0)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_4;
	}

	/** Set the CPIM handle, c. */
	c = (EcrioCPIMStruct *)handle;

	CPIMLOGI(c->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	CPIM_MUTEX_LOCK(c->mutexAPI, c->logHandle);

	c->strings.uSize = 0;

	p = (char*)pData;
	pe = (char*)pData + uLen;
	eof = pe;

	/** Initialize output structure */
	pal_MemorySet(pStruct, 0, sizeof(CPIMMessageStruct));

	/** Parsing CPIM message by Ragel */
	%% write init;
	%% write exec;

END:

	CPIM_MUTEX_UNLOCK(c->mutexAPI, c->logHandle);

	CPIMLOGI(c->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return uCPIMError;
}
