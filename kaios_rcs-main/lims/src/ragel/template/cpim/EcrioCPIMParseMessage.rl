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
 * @file EcrioParseMessage.c
 * @brief Implementation of the CPIM module parsing MIME message functionality.
*/

#include "EcrioPAL.h"
#include "EcrioCPIM.h"
#include "EcrioCPIMCommon.h"

%%{
	machine ec_cpim_MIMEMessageParser;

	CRLF = "\r\n";
	DQUOTE = "\"";

	alphanum = [a-zA-Z0-9];

	LWS    = ((" "* CRLF)? " "+);
	SWS    = (LWS?);

	word   = (alphanum | "-" | "." | "!" | "%" | "*" | "_" | "+" | "`" | "'" | "~" | "(" | ")" | "<" | ">" | ":" | "@" | "\\" | DQUOTE | "/" | "[" | "]" | "?" | "{" | "}")+;

	EQUAL  = (SWS "=" SWS);
	SEMI   = (SWS ";" SWS);
	HCOLON = ((" " | "\t")* ":" SWS);


	action tag
	{
		tag_start = p;
	}

	action ParseContentTypeParam
	{
	}

	action ParseContentTypeHeader
	{
		if (pal_StringNCompare((u_char*)tag_start, (u_char*)"text/plain",
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

	action ParseMessageBody
	{
		if (uCntLen == 0 || uCntLen > (p - tag_start))
		{
			uCntLen = p - tag_start;
		}

		c->buff[uIndex].uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->buff[uIndex], uCntLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tThe working buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->buff[uIndex].pData, 0, c->buff[uIndex].uContainerSize);
		pal_MemoryCopy(c->buff[uIndex].pData, c->buff[uIndex].uContainerSize, (u_char*)tag_start, uCntLen);
		c->buff[uIndex].uSize = uCntLen;

		pStruct->uNumOfBuffers++;
		pStruct->buff[uIndex].eContentType = eConType;
		if (eConType == CPIMContentType_Specified)
		{
			pStruct->buff[uIndex].pContentType = pContentType;
		}
		else
		{
			pStruct->buff[uIndex].pContentType = NULL;
		}
		pStruct->buff[uIndex].pMsgBody = c->buff[uIndex].pData;
		pStruct->buff[uIndex].uMsgLen = c->buff[uIndex].uSize;
		if (pContentId != NULL)
		{
			pStruct->buff[uIndex].pContentId = pContentId;
		}
		else
		{
			pStruct->buff[uIndex].pContentId = NULL;
		}
		if (pContentDisposition != NULL)
		{
			pStruct->buff[uIndex].pContentDisposition = pContentDisposition;
		}
		else
		{
			pStruct->buff[uIndex].pContentDisposition = NULL;
		}
		if (pContentTransferEncoding != NULL)
		{
			pStruct->buff[uIndex].pContentTransferEncoding = pContentTransferEncoding;
		}
		else
		{
			pStruct->buff[uIndex].pContentTransferEncoding = NULL;
		}
	}

	action error
	{
		/** This is a general error, but usually no CRLF at the end of line. */
		uCPIMError = ECRIO_CPIM_PARSING_INSUFFICIENT_ERROR;
	}

	m_parameter = (word EQUAL (SWS word>tag %ParseContentTypeParam));
	media_type = (word>tag %ParseContentTypeHeader (SEMI m_parameter)*);

	Content_Type = ("Content-Type" HCOLON media_type) CRLF;
	Content_Disposition = ("Content-Disposition" HCOLON word>tag %ParseContentDispositionHeader) CRLF;
	Content_Length = ("Content-Length" HCOLON digit+>tag %ParseContentLengthHeader) CRLF;
	Content_ID = ("Content-ID" HCOLON word>tag %ParseContentIDHeader) CRLF;
	Content_Transfer_Encoding = (("Content-Transfer-Encoding" | "CONTENT-TRANSFER-ENCODING") HCOLON word>tag %ParseContentTransferEncodingHeader) CRLF;

	message_body = any*>tag %ParseMessageBody;

	content_header =	(Content_Type |
						 Content_Disposition |
						 Content_Length |
						 Content_ID |
						 Content_Transfer_Encoding)>tag >1;

	mime_message = (content_header)* CRLF message_body;
	main := mime_message @!error;

	write data;
}%%



u_int32 ec_cpim_ParseMIMEMessage
(
	EcrioCPIMStruct *c,
	CPIMMessageStruct *pStruct,
	u_char *pData,
	u_int32 uLen,
	u_int32 uIndex
)
{
	u_int32 uCPIMError = ECRIO_CPIM_NO_ERROR;
	u_int32 uCntLen = 0;
	s_int32 cs;
	const char *p;
	const char *pe;
	const char *eof;
	const char *tag_start = NULL;
	u_char cTemp[8];
	u_char *pContentType = NULL;
	u_char *pContentDisposition = NULL;
	u_char *pContentId = NULL;
	u_char *pContentTransferEncoding = NULL;
	CPIMContentTypeEnum eConType = CPIMContentType_None;

	/** Check parameter validity. */
	if (c == NULL)
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

	CPIMLOGI(c->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	p = (char*)pData;
	pe = (char*)pData + uLen;
	eof = pe;

	/** Parsing MIME message by Ragel */
	%% write init;
	%% write exec;

END:

	CPIMLOGI(c->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return uCPIMError;
}
