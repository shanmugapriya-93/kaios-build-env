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
 * @file EcrioUCEParseMultipartMessage.c
 * @brief Implementation of the UCE module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioUCE.h"
#include "EcrioUCEInternal.h"

%%{
	machine ec_uce_parse_multipart_message;

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
		pStruct->pContentType = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}

	action ParseContentLengthHeader
	{
		pal_MemorySet(cTemp, 0, 8);
		if (NULL == pal_StringNCopy(cTemp, 8, (u_char*)tag_start, (p - tag_start)))
		{
			uUCEError = ECRIO_UCE_MEMORY_ERROR;
			UCELOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
				__FUNCTION__, __LINE__);
			goto END;
		}
		uCntLen = pal_StringToNum(cTemp, NULL);
	}

	action ParseContentDispositionHeader
	{
		
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
			uUCEError = ECRIO_UCE_MEMORY_ERROR;
			UCELOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uUCEError);
			goto END;
		}
		pal_MemorySet((void*)pStruct->pMessageBody, 0, uCntLen + 1);
		pal_MemoryCopy(pStruct->pMessageBody, uCntLen + 1, (u_char*)tag_start, uCntLen);
		pStruct->uContentLength = uCntLen;
	}

	action error
	{
		/** This is a general error, but usually no CRLF at the end of line. */
		uUCEError = ECRIO_UCE_MESSAGE_PARSING_ERROR;
	}

	m_parameter = (word EQUAL (SWS word>tag %ParseContentTypeParam));
	media_type = (word>tag %ParseContentTypeHeader (SEMI m_parameter)*);

	Content_Type = ("Content-Type" HCOLON media_type) CRLF;
	Content_Disposition = ("Content-Disposition" HCOLON word>tag %ParseContentDispositionHeader) CRLF;
	Content_Length = ("Content-Length" HCOLON digit+>tag %ParseContentLengthHeader) CRLF;

	message_body = any*>tag %ParseMessageBody;

	content_header =	(Content_Type |
						 Content_Disposition |
						 Content_Length)>tag >1;

	mime_message = (content_header)* CRLF message_body;
	main := mime_message @!error;

	write data;
}%%



u_int32 ec_UCE_ParseMIMEMessageBody
(
	EcrioUCEContextStruct *pContext,
	EcrioUCEMultipartBodyStruct *pStruct,
	u_char *pData,
	u_int32 uLen
)
{
	u_int32 uUCEError = ECRIO_UCE_NO_ERROR;
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
		return ECRIO_UCE_INSUFFICIENT_DATA_ERROR;
	}

	UCELOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	p = (char*)pData;
	pe = (char*)pData + uLen;
	eof = pe;

	/** Parsing MIME message by Ragel */
	%% write init;
	%% write exec;

END:
	UCELOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u\tuUCEError=%u", __FUNCTION__, __LINE__, uUCEError);

	return uUCEError;
}

u_int32 ec_UCE_ParseMultipartMessage
(
	EcrioUCEContextStruct *pContext,
	EcrioUCEMultipartMessageStruct **ppStruct,
	char *pBoundary,
	char *pData,
	u_int32 uLen
)
{
	EcrioUCEMultipartMessageStruct *pStruct = NULL;
	EcrioUCEMultipartBodyStruct *pBody = NULL;
	u_int32 uUCEError = ECRIO_UCE_NO_ERROR;
	u_int32 i;
	u_int32 MAX_PRESENTITIES = 10;
	char *pCur = NULL, *pStart = NULL, *pEnd = NULL;
	char *pWork = NULL;

	/** Check parameter validity. */
	if (pContext == NULL || ppStruct == NULL || pBoundary == NULL || pData == NULL || uLen == 0)
	{
		return  ECRIO_UCE_INSUFFICIENT_DATA_ERROR;
	}

	UCELOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pal_MemoryAllocate((u_int32)sizeof(EcrioUCEMultipartMessageStruct), (void **)&pStruct);
	if (pStruct == NULL)
	{
		uUCEError = ECRIO_UCE_MEMORY_ERROR;

		UCELOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
			__FUNCTION__, __LINE__, uUCEError);
		goto END;
	}
	pal_MemorySet((void*)pStruct, 0, sizeof(EcrioUCEMultipartMessageStruct));

	pCur = pData;

	/** Initialize output buffer structure */
	pStruct->uNumOfBody = 0;

	/** If we handle more than MAX_PRESENTITIES, need to increase maximum value. */
	for (i = 0; i < MAX_PRESENTITIES; i++)
	{
		/* 1. Find boundary string */
		pStart = (char*)pal_StringFindSubString((u_char*)pCur, (u_char*)pBoundary);
		if (pStart == NULL)
		{
			UCELOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tboundary not found.", __FUNCTION__, __LINE__);
			uUCEError = ECRIO_UCE_MESSAGE_PARSING_ERROR;
			goto Error;
		}

		/** 2. pStart pointer goes to boundary + crlf, there is head of MIME message body. */
		pStart = pStart + pal_StringLength((u_char*)pBoundary) + 2;

		if (uLen <= (pStart - pData))
		{
			UCELOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tInvalid position of pStart.", __FUNCTION__, __LINE__);
			uUCEError = ECRIO_UCE_MESSAGE_PARSING_ERROR;
			goto Error;
		}

		/** 3. Find next boundary, 4 bytes ("--" string + crlf) before of pEnd is tail of MIME message body. */
		pEnd = (char*)pal_MemorySearch(pStart, uLen - (pStart - pData), (u_char*)pBoundary, pal_StringLength((u_char*)pBoundary));

		if (pEnd == NULL)
		{
			UCELOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tboundary not found.", __FUNCTION__, __LINE__);
			uUCEError = ECRIO_UCE_MESSAGE_PARSING_ERROR;
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
			uUCEError = ECRIO_UCE_MEMORY_ERROR;
			UCELOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uUCEError);
			goto Error;
		}
		pal_MemorySet((void*)pWork, 0, (pEnd - pStart + 1));
		pal_MemoryCopy((void*)pWork, (pEnd - pStart + 1), (const void*)pStart, (u_int32)(pEnd - pStart));

		if (pStruct->uNumOfBody == 0)
		{
			pal_MemoryAllocate((u_int32)sizeof(EcrioUCEMultipartBodyStruct*), (void **)&pStruct->ppBody);
		}
		else
		{
			pal_MemoryReallocate((u_int32)((pStruct->uNumOfBody + 1) * sizeof(EcrioUCEMultipartBodyStruct*)), (void **)&pStruct->ppBody);
		}

		if (pStruct->ppBody == NULL)
		{
			uUCEError = ECRIO_UCE_MEMORY_ERROR;
			UCELOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uUCEError);
			goto Error;
		}

		pal_MemoryAllocate((u_int32)sizeof(EcrioUCEMultipartBodyStruct), (void **)&pBody);
		if (pBody == NULL)
		{
			uUCEError = ECRIO_UCE_MEMORY_ERROR;
			UCELOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uUCEError);
			goto Error;
		}
		pal_MemorySet((void*)pBody, 0, sizeof(EcrioUCEMultipartBodyStruct));

		pStruct->ppBody[pStruct->uNumOfBody] = pBody;
		pStruct->uNumOfBody++;

		/** 5. Parse MIME message */
		uUCEError = ec_UCE_ParseMIMEMessageBody(pContext, pBody, (u_char*)pWork, (pEnd - pStart));
		if (uUCEError != ECRIO_UCE_NO_ERROR)
		{
			UCELOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tParsing MIME message error.", __FUNCTION__, __LINE__);
			uUCEError = ECRIO_UCE_MESSAGE_PARSING_ERROR;
			goto Error;
		}

		pal_MemoryFree((void**)&pWork);
		pWork = NULL;

		/** 6. Check whether the next boundary indicates a term boundary (exists "--" to bottom). */
		if (pal_StringNCompare((u_char*)pCur + pal_StringLength((u_char*)pBoundary), (u_char*)"--", 2) == 0)
		{
			UCELOGI(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tFound final boundary.", __FUNCTION__, __LINE__);
			uUCEError = ECRIO_UCE_NO_ERROR;
			break;
		}
	}

	*ppStruct = pStruct;
	goto END;

Error:
	ec_UCE_ReleaseMultiPartStruct(&pStruct, Enum_TRUE);

END:
	if (pWork != NULL)
	{
		pal_MemoryFree((void**)&pWork);
	}

	UCELOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u\tuUCEError=%u", __FUNCTION__, __LINE__, uUCEError);

	return uUCEError;
}

void ec_UCE_ReleaseMultiPartStruct
(
	EcrioUCEMultipartMessageStruct **ppStruct,
	BoolEnum release
)
{
	u_int32 i;
	EcrioUCEMultipartMessageStruct *pStruct;

	if (ppStruct == NULL || *ppStruct == NULL)
	{
		return;
	}

	pStruct = *ppStruct;

	for (i = 0; i < pStruct->uNumOfBody; i++)
	{
		pal_MemoryFree((void**)&pStruct->ppBody[i]->pContentType);
		pal_MemoryFree((void**)&pStruct->ppBody[i]->pMessageBody);
		pal_MemoryFree((void**)&pStruct->ppBody[i]);
	}
	pal_MemoryFree((void**)&pStruct->ppBody);

	if (release == Enum_TRUE)
	{
		pal_MemoryFree((void**)ppStruct);
	}
}
