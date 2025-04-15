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
 * @file EcrioCPMParseMultipartMessage.c
 * @brief Implementation of the CPM module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioCPM.h"
#include "EcrioCPMInternal.h"

%%{
	machine ec_cpm_parse_multipart_message;

	CRLF = "\r\n";
	DQUOTE = "\"";

	alphanum = [a-zA-Z0-9];

	LWS    = ((" "* CRLF)? " "+);
	SWS    = (LWS?);

	word   = (alphanum | "-" | "=" | "." | "!" | "%" | "*" | "_" | "+" | "`" | "'" | "~" | "(" | ")" | ":" | "@" | "\\" | DQUOTE | "/" | "[" | "]" | "?" | "{" | "}")+;

	EQUAL  = (SWS "=" SWS);
	SEMI   = (SWS ";" SWS);
	HCOLON = ((" " | "\t")* ":" SWS);
	RAQUOT = (">" SWS);
	LAQUOT = (SWS "<");


	action tag
	{
		tag_start = p;
	}

	action ParseContentTypeParam
	{
	}

	action ParseContentTypeHeader
	{
		pStruct->pContentType = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}

	action ParseContentIDHeader
	{
		pStruct->pContentId = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}

	action ParseContentLengthHeader
	{
		pal_MemorySet(cTemp, 0, 8);
		if (NULL == pal_StringNCopy(cTemp, 8, (u_char*)tag_start, (p - tag_start)))
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
				__FUNCTION__, __LINE__);
			goto END;
		}
		uCntLen = pal_StringToNum(cTemp, NULL);
	}

	action ParseContentDispositionHeader
	{
		pStruct->pContentDisposition = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}

	action ParseContentTransferEncodingHeader
	{
		pStruct->pContentTransferEncoding = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}

	action ParseMessageBody
	{
		if (uCntLen == 0 || uCntLen > (p - tag_start))
		{
			uCntLen = p - tag_start;
		}

		pal_MemoryAllocate(uCntLen + 1, (void **)&pStruct->pMessageBody);
		if (pStruct->pMessageBody == NULL)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto END;
		}
		pal_MemorySet((void*)pStruct->pMessageBody, 0, uCntLen + 1);
		pal_MemoryCopy(pStruct->pMessageBody, uCntLen + 1, (u_char*)tag_start, uCntLen);
		pStruct->uContentLength = uCntLen;
	}

	action error
	{
		/** This is a general error, but usually no CRLF at the end of line. */
		uCPMError = ECRIO_CPM_MESSAGE_PARSING_ERROR;
	}

	m_parameter = (word EQUAL (SWS word>tag %ParseContentTypeParam));
	media_type = (word>tag %ParseContentTypeHeader (SEMI m_parameter)*);

	Content_Type = ("Content-Type" HCOLON media_type) CRLF;
	Content_ID = ("Content-ID" HCOLON (LAQUOT)? word>tag %ParseContentIDHeader (RAQUOT)?) CRLF;
	Content_Disposition = ("Content-Disposition" HCOLON word>tag ((SEMI word)*)? %ParseContentDispositionHeader) CRLF;
	Content_Length = ("Content-Length" HCOLON digit+>tag %ParseContentLengthHeader) CRLF;
	Content_Transfer_Encoding = (("Content-Transfer-Encoding" | "CONTENT-TRANSFER-ENCODING") HCOLON word>tag %ParseContentTransferEncodingHeader) CRLF;

	message_body = any*>tag %ParseMessageBody;

	content_header =	(Content_Type |
						 Content_ID |
						 Content_Disposition |
						 Content_Length |
						 Content_Transfer_Encoding)>tag >1;

	mime_message = (content_header)* CRLF message_body;
	main := mime_message @!error;

	write data;
}%%



u_int32 ec_CPM_ParseMIMEMessageBody
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMMultipartBodyStruct *pStruct,
	u_char *pData,
	u_int32 uLen
)
{
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
	u_int32 uCntLen = 0;
	s_int32 cs;
	const char *p;
	const char *pe;
	const char *eof;
	const char *tag_start = NULL;
	u_char cTemp[8];

	/** Check parameter validity. */
	if (pContext == NULL || pStruct == NULL || pData == NULL || uLen == 0)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	p = (char*)pData;
	pe = (char*)pData + uLen;
	eof = pe;

	/** Parsing MIME message by Ragel */
	%% write init;
	%% write exec;

END:
	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u\tuCPMError=%u", __FUNCTION__, __LINE__, uCPMError);

	return uCPMError;
}

u_int32 ec_CPM_ParseMultipartMessage
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMMultipartMessageStruct **ppStruct,
	char *pBoundary,
	char *pData,
	u_int32 uLen
)
{
	EcrioCPMMultipartMessageStruct *pStruct = NULL;
	EcrioCPMMultipartBodyStruct *pBody = NULL;
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
	u_int32 i;
	char *pCur = NULL, *pStart = NULL, *pEnd = NULL;
	char *pWork = NULL;

	/** Check parameter validity. */
	if (pContext == NULL || ppStruct == NULL || pBoundary == NULL || pData == NULL || uLen == 0)
	{
		return  ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pal_MemoryAllocate((u_int32)sizeof(EcrioCPMMultipartMessageStruct), (void **)&pStruct);
	if (pStruct == NULL)
	{
		uCPMError = ECRIO_CPM_MEMORY_ERROR;

		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
			__FUNCTION__, __LINE__, uCPMError);
		goto END;
	}
	pal_MemorySet((void*)pStruct, 0, sizeof(EcrioCPMMultipartMessageStruct));

	pCur = pData;

	/** Initialize output buffer structure */
	pStruct->uNumOfBody = 0;

	/** We assumed that message only contains SDP and resource-list. */
	for (i = 0; i < 2; i++)
	{
		/* 1. Find boundary string */
		pStart = (char*)pal_StringFindSubString((u_char*)pCur, (u_char*)pBoundary);
		if (pStart == NULL)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tboundary not found.", __FUNCTION__, __LINE__);
			uCPMError = ECRIO_CPM_MESSAGE_PARSING_ERROR;
			goto Error;
		}

		/** 2. pStart pointer goes to boundary + crlf, there is head of MIME message body. */
		pStart = pStart + pal_StringLength((u_char*)pBoundary) + 2;

		if (uLen <= (pStart - pData))
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tInvalid position of pStart.", __FUNCTION__, __LINE__);
			uCPMError = ECRIO_CPM_MESSAGE_PARSING_ERROR;
			goto Error;
		}

		/** 3. Find next boundary, 4 bytes ("--" string + crlf) before of pEnd is tail of MIME message body. */
		pEnd = (char*)pal_MemorySearch(pStart, uLen - (pStart - pData), (u_char*)pBoundary, pal_StringLength((u_char*)pBoundary));

		if (pEnd == NULL)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tboundary not found.", __FUNCTION__, __LINE__);
			uCPMError = ECRIO_CPM_MESSAGE_PARSING_ERROR;
			goto Error;
		}
		pCur = pEnd;
		pEnd -= 2;

		/** 3.1. Check the last crlf */
		if (*(pEnd - 1) == '\n')
		{
			pEnd--;
		}
		if (*(pEnd - 1) == '\r')
		{
			pEnd--;
		}

		/** 4. Copy MIME message to working buffer. */
		pal_MemoryAllocate((u_int32)(pEnd - pStart + 1), (void **)&pWork);
		if (pWork == NULL)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto Error;
		}
		pal_MemorySet((void*)pWork, 0, (pEnd - pStart + 1));
		pal_MemoryCopy((void*)pWork, (pEnd - pStart + 1), (const void*)pStart, (u_int32)(pEnd - pStart));

		if (pStruct->uNumOfBody == 0)
		{
			pal_MemoryAllocate((u_int32)sizeof(EcrioCPMMultipartBodyStruct*), (void **)&pStruct->ppBody);
		}
		else
		{
			/* Check arithmetic overflow */
			if (pal_UtilityArithmeticOverflowDetected(pStruct->uNumOfBody, 1) == Enum_TRUE)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;
				goto Error;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((pStruct->uNumOfBody + 1), sizeof(EcrioCPMMultipartBodyStruct*)) == Enum_TRUE)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;
				goto Error;
			}
			pal_MemoryReallocate((u_int32)((pStruct->uNumOfBody + 1) * sizeof(EcrioCPMMultipartBodyStruct*)), (void **)&pStruct->ppBody);
		}

		if (pStruct->ppBody == NULL)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto Error;
		}

		pal_MemoryAllocate((u_int32)sizeof(EcrioCPMMultipartBodyStruct), (void **)&pBody);
		if (pBody == NULL)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto Error;
		}
		pal_MemorySet((void*)pBody, 0, sizeof(EcrioCPMMultipartBodyStruct));

		pStruct->ppBody[pStruct->uNumOfBody] = pBody;
		pStruct->uNumOfBody++;

		/** 5. Parse MIME message */
		uCPMError = ec_CPM_ParseMIMEMessageBody(pContext, pBody, (u_char*)pWork, (pEnd - pStart));
		if (uCPMError != ECRIO_CPM_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tParsing MIME message error.", __FUNCTION__, __LINE__);
			uCPMError = ECRIO_CPM_MESSAGE_PARSING_ERROR;
			goto Error;
		}

		pal_MemoryFree((void**)&pWork);
		pWork = NULL;

		/** 6. Check whether the next boundary indicates a term boundary (exists "--" to bottom). */
		if (pal_StringNCompare((u_char*)pCur + pal_StringLength((u_char*)pBoundary), (u_char*)"--", 2) == 0)
		{
			CPMLOGI(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tFound final boundary.", __FUNCTION__, __LINE__);
			uCPMError = ECRIO_CPM_NO_ERROR;
			break;
		}
	}

	*ppStruct = pStruct;
	goto END;

Error:
	ec_CPM_ReleaseMultiPartStruct(&pStruct, Enum_TRUE);

END:
	if (pWork != NULL)
	{
		pal_MemoryFree((void**)&pWork);
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u\tuCPMError=%u", __FUNCTION__, __LINE__, uCPMError);

	return uCPMError;
}

void ec_CPM_ReleaseMultiPartStruct
(
	EcrioCPMMultipartMessageStruct **ppStruct,
	BoolEnum release
)
{
	u_int32 i;
	EcrioCPMMultipartMessageStruct *pStruct;

	if (ppStruct == NULL || *ppStruct == NULL)
	{
		return;
	}

	pStruct = *ppStruct;

	for (i = 0; i < pStruct->uNumOfBody; i++)
	{
		pal_MemoryFree((void**)&pStruct->ppBody[i]->pContentType);
		pal_MemoryFree((void**)&pStruct->ppBody[i]->pContentId);
		pal_MemoryFree((void**)&pStruct->ppBody[i]->pContentDisposition);
		pal_MemoryFree((void**)&pStruct->ppBody[i]->pContentTransferEncoding);
		pal_MemoryFree((void**)&pStruct->ppBody[i]->pMessageBody);
		pal_MemoryFree((void**)&pStruct->ppBody[i]);
	}
	pal_MemoryFree((void**)&pStruct->ppBody);

	if (release == Enum_TRUE)
	{
		pal_MemoryFree((void**)ppStruct);
	}
}
