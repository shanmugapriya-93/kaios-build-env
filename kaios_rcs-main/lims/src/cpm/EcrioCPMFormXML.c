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
 * @file EcrioCPMFormXML.c
 * @brief Implementation of the CPM module forming functionality.
*/

#include "EcrioPAL.h"
#include "EcrioCPM.h"
#include "EcrioCPMInternal.h"

u_int32 ec_CPM_FormMultipartMessage
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMMultipartMessageStruct *pStruct,
	char **ppData,
	u_int32 *pLen,
	char **ppBoundary
)
{
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
	u_char *pData = NULL;
	u_char *pBoundary = NULL;
	u_int32 uLen = 0;
	u_int32 uBufLen = 0;
	u_int32 i = 0;
	u_char ucString[9] = { 0 };

	/** Check parameter validity. */
	if (pContext == NULL || pStruct == NULL || ppData == NULL || pLen == NULL)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pStruct->uNumOfBody == 0 || pStruct->ppBody == NULL)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tInvalid message body list",
			__FUNCTION__, __LINE__);

		uCPMError = ECRIO_CPM_INTERNAL_ERROR;
		goto END;
	}
	else if (pStruct->uNumOfBody == 1)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral,
			"%s:%u\tJust one message body, could not build multipart message body",
			__FUNCTION__, __LINE__);

		uCPMError = ECRIO_CPM_INTERNAL_ERROR;
		goto END;
	}

	/** Generate boundary string */
	pal_MemoryAllocate(12, (void **)&pBoundary);
	if (NULL == pBoundary)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		uCPMError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}
	pal_MemorySet(pBoundary, 0, 12);
	ec_CPM_getRandomString(pBoundary, 10);

	/** Calcluate buffer size */
	for (i = 0; i < pStruct->uNumOfBody; i++)
	{
		uBufLen += pStruct->ppBody[i]->uContentLength;
		uBufLen += 128;
	}

	pal_MemoryAllocate(uBufLen, (void **)&pData);
	if (NULL == pData)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		uCPMError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}
	pal_MemorySet(pData, 0, uBufLen);

	/** Start boundary */
	if (NULL == pal_StringNCopy(pData, uBufLen, ECRIO_CPM_BOUNDARY, pal_StringLength(ECRIO_CPM_BOUNDARY)))
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
			__FUNCTION__, __LINE__);
		uCPMError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}
	CPM_STRING_CONCATENATE(pData, uBufLen - pal_StringLength(pData), pBoundary, pal_StringLength(pBoundary), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uBufLen - pal_StringLength(pData), ECRIO_CPM_CRLF_SYMBOL, pal_StringLength(ECRIO_CPM_CRLF_SYMBOL), uCPMError, Error);

	for (i = 0; i < pStruct->uNumOfBody; i++)
	{
		/** Content-Type header */
		if (pStruct->ppBody[i]->pContentType == NULL)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tNo content type",
				__FUNCTION__, __LINE__);

			uCPMError = ECRIO_CPM_INTERNAL_ERROR;
			goto Error;
		}
		CPM_STRING_CONCATENATE(pData, uBufLen - pal_StringLength(pData), ECRIO_CPM_CONTENT_TYPE_HEADER, pal_StringLength(ECRIO_CPM_CONTENT_TYPE_HEADER), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uBufLen - pal_StringLength(pData), ECRIO_CPM_COLON_SYMBOL, pal_StringLength(ECRIO_CPM_COLON_SYMBOL), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uBufLen - pal_StringLength(pData), ECRIO_CPM_SPACE_SYMBOL, pal_StringLength(ECRIO_CPM_SPACE_SYMBOL), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uBufLen - pal_StringLength(pData), pStruct->ppBody[i]->pContentType, pal_StringLength(pStruct->ppBody[i]->pContentType), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uBufLen - pal_StringLength(pData), ECRIO_CPM_CRLF_SYMBOL, pal_StringLength(ECRIO_CPM_CRLF_SYMBOL), uCPMError, Error);

		/** Content-Length header */
		if (pStruct->ppBody[i]->uContentLength != 0)
		{
			CPM_STRING_CONCATENATE(pData, uBufLen - pal_StringLength(pData), ECRIO_CPM_CONTENT_LENGTH_HEADER, pal_StringLength(ECRIO_CPM_CONTENT_LENGTH_HEADER), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uBufLen - pal_StringLength(pData), ECRIO_CPM_COLON_SYMBOL, pal_StringLength(ECRIO_CPM_COLON_SYMBOL), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uBufLen - pal_StringLength(pData), ECRIO_CPM_SPACE_SYMBOL, pal_StringLength(ECRIO_CPM_SPACE_SYMBOL), uCPMError, Error);
			pal_MemorySet(ucString, 0, 9);
			if (0 >= pal_NumToString(pStruct->ppBody[i]->uContentLength, ucString, 9))
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tString Copy error.",
					__FUNCTION__, __LINE__);

				uCPMError = ECRIO_CPM_INTERNAL_ERROR;
				goto Error;
			}
			CPM_STRING_CONCATENATE(pData, uBufLen - pal_StringLength(pData), ucString, pal_StringLength(ucString), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uBufLen - pal_StringLength(pData), ECRIO_CPM_CRLF_SYMBOL, pal_StringLength(ECRIO_CPM_CRLF_SYMBOL), uCPMError, Error);
		}
		CPM_STRING_CONCATENATE(pData, uBufLen - pal_StringLength(pData), ECRIO_CPM_CRLF_SYMBOL, pal_StringLength(ECRIO_CPM_CRLF_SYMBOL), uCPMError, Error);

		/** Message body */
		if (pStruct->ppBody[i]->pMessageBody == NULL)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tNo message body",
				__FUNCTION__, __LINE__);

			uCPMError = ECRIO_CPM_INTERNAL_ERROR;
			goto Error;
		}
		CPM_STRING_CONCATENATE(pData, uBufLen - pal_StringLength(pData), pStruct->ppBody[i]->pMessageBody, pal_StringLength(pStruct->ppBody[i]->pMessageBody), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uBufLen - pal_StringLength(pData), ECRIO_CPM_CRLF_SYMBOL, pal_StringLength(ECRIO_CPM_CRLF_SYMBOL), uCPMError, Error);

		/** Boundary */
		CPM_STRING_CONCATENATE(pData, uBufLen - pal_StringLength(pData), ECRIO_CPM_BOUNDARY, pal_StringLength(ECRIO_CPM_BOUNDARY), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uBufLen - pal_StringLength(pData), pBoundary, pal_StringLength(pBoundary), uCPMError, Error);
		if ((i + 1) == pStruct->uNumOfBody)
		{
			CPM_STRING_CONCATENATE(pData, uBufLen - pal_StringLength(pData), ECRIO_CPM_BOUNDARY, pal_StringLength(ECRIO_CPM_BOUNDARY), uCPMError, Error);
		}
		else
		{
			CPM_STRING_CONCATENATE(pData, uBufLen - pal_StringLength(pData), ECRIO_CPM_CRLF_SYMBOL, pal_StringLength(ECRIO_CPM_CRLF_SYMBOL), uCPMError, Error);
		}
	}

	uLen = pal_StringLength(pData);

	*ppData = (char*)pData;
	*pLen = uLen;
	*ppBoundary = (char*)pBoundary;

	goto END;

Error:
	if (pBoundary != NULL)
	{
		pal_MemoryFree((void**)&pBoundary);
	}
	if (pData != NULL)
	{
		pal_MemoryFree((void**)&pData);
	}
	*ppData = NULL;
	*pLen = 0;
	*ppBoundary = NULL;

END:
	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u\tuCPMError=%u", __FUNCTION__, __LINE__, uCPMError);

	return uCPMError;
}

u_int32 ec_CPM_FormGroupSessionDataManagementXML
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMGroupDataRequestStruct *pStruct,
	char **ppData,
	u_int32 *pLen
)
{
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
	u_char *pData = NULL;
	u_int32 uLen = 0;
	u_int32 id = 0;
	u_int32 uSize = 0;
	u_char randStr[15] = { 0 };

	/** Check parameter validity. */
	if (pContext == NULL || pStruct == NULL || ppData == NULL || pLen == NULL)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pStruct->type == EcrioCPMGroupDataMngType_None)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tInvalid data management type",
			__FUNCTION__, __LINE__);

		uCPMError = ECRIO_CPM_INTERNAL_ERROR;
		goto END;
	}

	uSize = ECRIO_CPM_XML_BUFFER_SIZE;

	pal_MemoryAllocate(uSize, (void **)&pData);
	if (NULL == pData)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		uCPMError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}
	pal_MemorySet(pData, 0, uSize);

	/** <?xml version="1.0" encoding="UTF-8"?> */
	if (NULL == pal_StringNCopy(pData, uSize, ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE)))
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
			__FUNCTION__, __LINE__);
		uCPMError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_QUESTIONMARK, pal_StringLength(ECRIO_CPM_XML_QUESTIONMARK), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_VERSION1, pal_StringLength(ECRIO_CPM_XML_VERSION1), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_EQUALTO, pal_StringLength(ECRIO_CPM_XML_EQUALTO), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_VERSION, pal_StringLength(ECRIO_CPM_XML_VERSION), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SPACE, pal_StringLength(ECRIO_CPM_XML_SPACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ENCODING1, pal_StringLength(ECRIO_CPM_XML_ENCODING1), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_EQUALTO, pal_StringLength(ECRIO_CPM_XML_EQUALTO), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ENCODING, pal_StringLength(ECRIO_CPM_XML_ENCODING), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_QUESTIONMARK, pal_StringLength(ECRIO_CPM_XML_QUESTIONMARK), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	/** <cpm-group-management xmlns="urn:oma:xml:cpm:groupdata:1.0"
			id="randam-number"> */
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CPM_GROUP_MANAGEMENT, pal_StringLength(ECRIO_CPM_XML_CPM_GROUP_MANAGEMENT), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SPACE, pal_StringLength(ECRIO_CPM_XML_SPACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ATTRIBUTE_DEFAULTNS, pal_StringLength(ECRIO_CPM_XML_ATTRIBUTE_DEFAULTNS), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_EQUALTO, pal_StringLength(ECRIO_CPM_XML_EQUALTO), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_VALUE_GROUP_DATANS, pal_StringLength(ECRIO_CPM_XML_VALUE_GROUP_DATANS), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SPACE, pal_StringLength(ECRIO_CPM_XML_SPACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ID, pal_StringLength(ECRIO_CPM_XML_ID), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_EQUALTO, pal_StringLength(ECRIO_CPM_XML_EQUALTO), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	id = pal_UtilityRandomNumber();
	if (0 >= pal_NumToString(id, randStr, 15))
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tString Copy error",
			__FUNCTION__, __LINE__);

		uCPMError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error;
	}
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), randStr, pal_StringLength(randStr), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	/** <group-data> */
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_GROUP_DATA, pal_StringLength(ECRIO_CPM_XML_GROUP_DATA), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	/** <request target="subject"> */
	if (pStruct->type == EcrioCPMGroupDataMngType_Subject)
	{
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_REQUEST, pal_StringLength(ECRIO_CPM_XML_REQUEST), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SPACE, pal_StringLength(ECRIO_CPM_XML_SPACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_TARGET, pal_StringLength(ECRIO_CPM_XML_TARGET), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_EQUALTO, pal_StringLength(ECRIO_CPM_XML_EQUALTO), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SUBJECT, pal_StringLength(ECRIO_CPM_XML_SUBJECT), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

		/** <action> */
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ACTION, pal_StringLength(ECRIO_CPM_XML_ACTION), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);

		if (pStruct->subject.type == EcrioCPMGroupDataActionType_Set)
		{
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SET, pal_StringLength(ECRIO_CPM_XML_SET), uCPMError, Error);
		}
		else if (pStruct->subject.type == EcrioCPMGroupDataActionType_Delete)
		{
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DELETE, pal_StringLength(ECRIO_CPM_XML_DELETE), uCPMError, Error);
		}
		else
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tInvalid data action type",
				__FUNCTION__, __LINE__);

			uCPMError = ECRIO_CPM_INTERNAL_ERROR;
			goto Error;
		}

		/** </action> */
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ACTION, pal_StringLength(ECRIO_CPM_XML_ACTION), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

		if (pStruct->subject.type == EcrioCPMGroupDataActionType_Set)
		{
			/** <data> */
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DATA, pal_StringLength(ECRIO_CPM_XML_DATA), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

			/** <subject> */
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SUBJECT, pal_StringLength(ECRIO_CPM_XML_SUBJECT), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);

			/** Subject string */
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), pStruct->subject.pText, pal_StringLength(pStruct->subject.pText), uCPMError, Error);

			/** </subject> */
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SUBJECT, pal_StringLength(ECRIO_CPM_XML_SUBJECT), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

			/** </data> */
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DATA, pal_StringLength(ECRIO_CPM_XML_DATA), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);
		}

		/** </request> */
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_REQUEST, pal_StringLength(ECRIO_CPM_XML_REQUEST), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);
	}

	/** <request target="icon"> */
	if (pStruct->type == EcrioCPMGroupDataMngType_Icon)
	{
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_REQUEST, pal_StringLength(ECRIO_CPM_XML_REQUEST), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SPACE, pal_StringLength(ECRIO_CPM_XML_SPACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_TARGET, pal_StringLength(ECRIO_CPM_XML_TARGET), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_EQUALTO, pal_StringLength(ECRIO_CPM_XML_EQUALTO), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ICON, pal_StringLength(ECRIO_CPM_XML_ICON), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

		/** <action> */
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ACTION, pal_StringLength(ECRIO_CPM_XML_ACTION), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);

		if (pStruct->icon.type == EcrioCPMGroupDataActionType_Set)
		{
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SET, pal_StringLength(ECRIO_CPM_XML_SET), uCPMError, Error);
		}
		else if (pStruct->icon.type == EcrioCPMGroupDataActionType_Delete)
		{
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DELETE, pal_StringLength(ECRIO_CPM_XML_DELETE), uCPMError, Error);
		}
		else
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tInvalid data action type",
				__FUNCTION__, __LINE__);

			uCPMError = ECRIO_CPM_INTERNAL_ERROR;
			goto Error;
		}

		/** </action> */
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ACTION, pal_StringLength(ECRIO_CPM_XML_ACTION), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

		if (pStruct->icon.pUri != NULL || pStruct->icon.pFileInfo != NULL)
		{
			/** <data> */
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DATA, pal_StringLength(ECRIO_CPM_XML_DATA), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

			/** <icon> */
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ICON, pal_StringLength(ECRIO_CPM_XML_ICON), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);

			if (pStruct->icon.pUri != NULL)
			{
				/** <icon-uri> */
				CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
				CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ICON_URI, pal_StringLength(ECRIO_CPM_XML_ICON_URI), uCPMError, Error);
				CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);

				/** Icon URI */
				CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), pStruct->icon.pUri, pal_StringLength(pStruct->icon.pUri), uCPMError, Error);

				/** </icon-uri> */
				CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
				CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
				CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ICON_URI, pal_StringLength(ECRIO_CPM_XML_ICON_URI), uCPMError, Error);
				CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
				CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);
			}

			if (pStruct->icon.pFileInfo != NULL)
			{
				/** <file-info> */
				CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
				CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_FILE_INFO, pal_StringLength(ECRIO_CPM_XML_FILE_INFO), uCPMError, Error);
				CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);

				/** Subject string */
				CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), pStruct->icon.pFileInfo, pal_StringLength(pStruct->icon.pFileInfo), uCPMError, Error);

				/** </file-info> */
				CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
				CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
				CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_FILE_INFO, pal_StringLength(ECRIO_CPM_XML_FILE_INFO), uCPMError, Error);
				CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
				CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);
			}

			/** </icon> */
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ICON, pal_StringLength(ECRIO_CPM_XML_ICON), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

			/** </data> */
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DATA, pal_StringLength(ECRIO_CPM_XML_DATA), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);
		}

		/** </request> */
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_REQUEST, pal_StringLength(ECRIO_CPM_XML_REQUEST), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);
	}

	/** <request target="role"> */
	if (pStruct->type == EcrioCPMGroupDataMngType_Role)
	{
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_REQUEST, pal_StringLength(ECRIO_CPM_XML_REQUEST), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SPACE, pal_StringLength(ECRIO_CPM_XML_SPACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_TARGET, pal_StringLength(ECRIO_CPM_XML_TARGET), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_EQUALTO, pal_StringLength(ECRIO_CPM_XML_EQUALTO), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ROLE, pal_StringLength(ECRIO_CPM_XML_ROLE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

		/** <action> */
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ACTION, pal_StringLength(ECRIO_CPM_XML_ACTION), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);

		if (pStruct->role.type == EcrioCPMGroupDataActionType_Move)
		{
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_MOVE, pal_StringLength(ECRIO_CPM_XML_MOVE), uCPMError, Error);
		}
		else
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tInvalid data action type",
				__FUNCTION__, __LINE__);

			uCPMError = ECRIO_CPM_INTERNAL_ERROR;
			goto Error;
		}

		/** </action> */
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ACTION, pal_StringLength(ECRIO_CPM_XML_ACTION), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

		/** <data> */
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DATA, pal_StringLength(ECRIO_CPM_XML_DATA), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

		/** <user-role> */
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_USER_ROLE, pal_StringLength(ECRIO_CPM_XML_USER_ROLE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);

		/** user-role string */
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), pStruct->role.pUserRole, pal_StringLength(pStruct->role.pUserRole), uCPMError, Error);

		/** </user-role> */
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_USER_ROLE, pal_StringLength(ECRIO_CPM_XML_USER_ROLE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

		/** <participant> */
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_PARTICIPANT, pal_StringLength(ECRIO_CPM_XML_PARTICIPANT), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);

		/** participant string */
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), pStruct->role.pHostUri, pal_StringLength(pStruct->role.pHostUri), uCPMError, Error);

		/** </participant> */
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_PARTICIPANT, pal_StringLength(ECRIO_CPM_XML_PARTICIPANT), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

		/** </data> */
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DATA, pal_StringLength(ECRIO_CPM_XML_DATA), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

		/** </request> */
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_REQUEST, pal_StringLength(ECRIO_CPM_XML_REQUEST), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);
	}

	/** </group-data> */
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_GROUP_DATA, pal_StringLength(ECRIO_CPM_XML_GROUP_DATA), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	/** </cpm-group-management> */
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CPM_GROUP_MANAGEMENT, pal_StringLength(ECRIO_CPM_XML_CPM_GROUP_MANAGEMENT), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	uLen = pal_StringLength(pData);

	*ppData = (char*)pData;
	*pLen = uLen;

	goto END;

Error:
	pal_MemoryFree((void**)&pData);

	*ppData = NULL;
	*pLen = 0;

END:
	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u\tuCPMError=%u", __FUNCTION__, __LINE__, uCPMError);

	return uCPMError;
}

u_int32 ec_CPM_FormResourceListXML
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMParticipantListStruct *pStruct,
	EcrioCPMParticipantTypeEnum eType,
	char **ppData,
	u_int32 *pLen
)
{
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
	u_char *pData = NULL;
	u_int32 uLen = 0;
	u_int32 i = 0;
	u_int32 uSize = 0;
	BoolEnum isSipURI = Enum_TRUE;

	/** Check parameter validity. */
	if (pContext == NULL || pStruct == NULL || ppData == NULL || pLen == NULL)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pStruct->uNumOfUsers == 0 || pStruct->ppUri == NULL)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tInvalid URI-list",
			__FUNCTION__, __LINE__);

		uCPMError = ECRIO_CPM_INTERNAL_ERROR;
		goto END;
	}

	uSize = ECRIO_CPM_XML_BUFFER_SIZE + (ECRIO_CPM_XML_ENTRY_URI_BUFFER_SIZE * pStruct->uNumOfUsers);

	pal_MemoryAllocate(uSize, (void **)&pData);
	if (NULL == pData)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		uCPMError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}
	pal_MemorySet(pData, 0, uSize);

	/** <?xml version="1.0" encoding="UTF-8"?> */
	if (NULL == pal_StringNCopy(pData, uSize, ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE)))
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
			__FUNCTION__, __LINE__);
		uCPMError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_QUESTIONMARK, pal_StringLength(ECRIO_CPM_XML_QUESTIONMARK), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_VERSION1, pal_StringLength(ECRIO_CPM_XML_VERSION1), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_EQUALTO, pal_StringLength(ECRIO_CPM_XML_EQUALTO), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_VERSION, pal_StringLength(ECRIO_CPM_XML_VERSION), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SPACE, pal_StringLength(ECRIO_CPM_XML_SPACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ENCODING1, pal_StringLength(ECRIO_CPM_XML_ENCODING1), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_EQUALTO, pal_StringLength(ECRIO_CPM_XML_EQUALTO), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ENCODING, pal_StringLength(ECRIO_CPM_XML_ENCODING), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_QUESTIONMARK, pal_StringLength(ECRIO_CPM_XML_QUESTIONMARK), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	/** <resource-lists xmlns="urn:ietf:params:xml:ns:resource-lists"
          xmlns:cp="urn:ietf:params:xml:ns:copyControl"> */
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_RESOURCE_LISTS, pal_StringLength(ECRIO_CPM_XML_RESOURCE_LISTS), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SPACE, pal_StringLength(ECRIO_CPM_XML_SPACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ATTRIBUTE_DEFAULTNS, pal_StringLength(ECRIO_CPM_XML_ATTRIBUTE_DEFAULTNS), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_EQUALTO, pal_StringLength(ECRIO_CPM_XML_EQUALTO), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_VALUE_RESOURCE_LISTNS, pal_StringLength(ECRIO_CPM_XML_VALUE_RESOURCE_LISTNS), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SPACE, pal_StringLength(ECRIO_CPM_XML_SPACE), uCPMError, Error);

	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ATTRIBUTE_CPNS, pal_StringLength(ECRIO_CPM_XML_ATTRIBUTE_CPNS), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_EQUALTO, pal_StringLength(ECRIO_CPM_XML_EQUALTO), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_VALUE_CPNS, pal_StringLength(ECRIO_CPM_XML_VALUE_CPNS), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SPACE, pal_StringLength(ECRIO_CPM_XML_SPACE), uCPMError, Error);

	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"xmlns:xsi=", pal_StringLength((u_char*)"xmlns:xsi="), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"http://www.w3.org/2001/XMLSchema-instance", 
		pal_StringLength((u_char*)"http://www.w3.org/2001/XMLSchema-instance"), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	/** <list> */
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_LIST, pal_StringLength(ECRIO_CPM_XML_LIST), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	/** entry uri */
	for (i = 0; i < pStruct->uNumOfUsers; i++)
	{
		CPMLOGD(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tAdding %s to URI-list",
				__FUNCTION__, __LINE__, (u_char *)pStruct->ppUri[i]);
		/** <entry uri="Participant-URI" cp:copyControl="to"/> */
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ENTRY_URI, pal_StringLength(ECRIO_CPM_XML_ENTRY_URI), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_EQUALTO, pal_StringLength(ECRIO_CPM_XML_EQUALTO), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
#if 0
		/** Add default "sip:" if ppUri[i] does not present URI scheme. */
		if ((pal_StringNCompare((u_char *)pStruct->ppUri[i], (u_char*)"sip:", 4) != 0) &&
			(pal_StringNCompare((u_char *)pStruct->ppUri[i], (u_char*)"tel:", 4) != 0))
		{
			pal_StringNConcatenate(pData, uSize - pal_StringLength(pData), (u_char*)"sip:");
			pal_StringNConcatenate(pData, uSize - pal_StringLength(pData), (u_char*)pStruct->ppUri[i]);
			pal_StringNConcatenate(pData, uSize - pal_StringLength(pData), (u_char*)"@");
			pal_StringNConcatenate(pData, uSize - pal_StringLength(pData), pContext->pLocalDomain);
		}
#else
		/** Add default "tel:" if ppUri[i] does not present URI scheme. */
		if ((pal_StringNCompare((u_char *)pStruct->ppUri[i], (u_char*)"sip:", 4) != 0) &&
			(pal_StringNCompare((u_char *)pStruct->ppUri[i], (u_char*)"tel:", 4) != 0))
		{
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"tel:", pal_StringLength((u_char*)"tel:"), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)pStruct->ppUri[i], pal_StringLength((u_char*)pStruct->ppUri[i]), uCPMError, Error);
			isSipURI = Enum_FALSE;
		}
#endif
		else
		{
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)pStruct->ppUri[i], pal_StringLength((u_char*)pStruct->ppUri[i]), uCPMError, Error);

			if (pal_StringNCompare((u_char *)pStruct->ppUri[i], (u_char*)"tel:", 4) == 0)
				isSipURI = Enum_FALSE;
		}

		if (isSipURI == Enum_TRUE)
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)";user=phone", pal_StringLength((u_char*)";user=phone"), uCPMError, Error);

		if (eType != EcrioCPMParticipantType_None)
		{
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_QUESTIONMARK, pal_StringLength(ECRIO_CPM_XML_QUESTIONMARK), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_METHOD, pal_StringLength(ECRIO_CPM_XML_METHOD), uCPMError, Error);
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_EQUALTO, pal_StringLength(ECRIO_CPM_XML_EQUALTO), uCPMError, Error);
			if (eType == EcrioCPMParticipantType_Add)
			{
				CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_INVITE, pal_StringLength(ECRIO_CPM_XML_INVITE), uCPMError, Error);
			}
			else
			{
				CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_BYE, pal_StringLength(ECRIO_CPM_XML_BYE), uCPMError, Error);
			}
		}
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SPACE, pal_StringLength(ECRIO_CPM_XML_SPACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CP_COPYCONTROL, pal_StringLength(ECRIO_CPM_XML_CP_COPYCONTROL), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_EQUALTO, pal_StringLength(ECRIO_CPM_XML_EQUALTO), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SMALL_TO, pal_StringLength(ECRIO_CPM_XML_SMALL_TO), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);
	}

	/** </list> */
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_LIST, pal_StringLength(ECRIO_CPM_XML_LIST), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	/** </resource-lists> */
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_RESOURCE_LISTS, pal_StringLength(ECRIO_CPM_XML_RESOURCE_LISTS), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	uLen = pal_StringLength(pData);

	*ppData = (char*)pData;
	*pLen = uLen;

	goto END;

Error:
	pal_MemoryFree((void**)&pData);

	*ppData = NULL;
	*pLen = 0;

END:
	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u\tuCPMError=%u", __FUNCTION__, __LINE__, uCPMError);

	return uCPMError;
}

u_int32 ec_CPM_FormIsComposingXML
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMComposingStruct *pComposing,
	char **ppData,
	u_int32 *pLen
)
{
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
	u_char *pData = NULL;
	u_int32 uLen = 0;
	u_int32 uSize = 0;
	u_char refreshStr[6] = { 0 };

	/** Check parameter validity. */
	if (pContext == NULL || pComposing == NULL || ppData == NULL || pLen == NULL)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	uSize = ECRIO_CPM_XML_BUFFER_SIZE;

	pal_MemoryAllocate(uSize, (void **)&pData);
	if (NULL == pData)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		uCPMError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}
	pal_MemorySet(pData, 0, uSize);

	/** <?xml version="1.0" encoding="UTF-8"?> */
	if (NULL == pal_StringNCopy(pData, uSize, ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE)))
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
			__FUNCTION__, __LINE__);
		uCPMError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_QUESTIONMARK, pal_StringLength(ECRIO_CPM_XML_QUESTIONMARK), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_VERSION1, pal_StringLength(ECRIO_CPM_XML_VERSION1), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_EQUALTO, pal_StringLength(ECRIO_CPM_XML_EQUALTO), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_VERSION, pal_StringLength(ECRIO_CPM_XML_VERSION), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SPACE, pal_StringLength(ECRIO_CPM_XML_SPACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ENCODING1, pal_StringLength(ECRIO_CPM_XML_ENCODING1), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_EQUALTO, pal_StringLength(ECRIO_CPM_XML_EQUALTO), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ENCODING, pal_StringLength(ECRIO_CPM_XML_ENCODING), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_QUESTIONMARK, pal_StringLength(ECRIO_CPM_XML_QUESTIONMARK), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	/** <isComposing xmlns="urn:ietf:params:xml:ns:im-iscomposing"
		xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
		xsi:schemaLocation="urn:ietf:params:xml:ns:im-composing
		iscomposing.xsd"> */
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"<isComposing xmlns=", pal_StringLength((u_char*)"<isComposing xmlns="), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"urn:ietf:params:xml:ns:im-iscomposing", 
		pal_StringLength((u_char*)"urn:ietf:params:xml:ns:im-iscomposing"), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SPACE, pal_StringLength(ECRIO_CPM_XML_SPACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"xmlns:xsi=", pal_StringLength((u_char*)"xmlns:xsi="), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"http://www.w3.org/2001/XMLSchema-instance", 
		pal_StringLength((u_char*)"http://www.w3.org/2001/XMLSchema-instance"), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SPACE, pal_StringLength(ECRIO_CPM_XML_SPACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"xsi:schemaLocation=", pal_StringLength((u_char*)"xsi:schemaLocation="), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"urn:ietf:params:xml:ns:im-composing",
		pal_StringLength((u_char*)"urn:ietf:params:xml:ns:im-composing"), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SPACE, pal_StringLength(ECRIO_CPM_XML_SPACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"iscomposing.xsd", pal_StringLength((u_char*)"iscomposing.xsd"), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	/** <state> */
	if (pComposing->eComposingState != EcrioCPMComposingType_None)
	{
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"state", pal_StringLength((u_char*)"state"), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		if (pComposing->eComposingState == EcrioCPMComposingType_Idle)
		{
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"idle", pal_StringLength((u_char*)"idle"), uCPMError, Error);
		}
		else
		{
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"active", pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		}
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"state", pal_StringLength((u_char*)"state"), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);
	}

	/** <refresh> */
	if (pComposing->uRefresh != 0)
	{
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"refresh", pal_StringLength((u_char*)"refresh"), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		if (0 >= pal_NumToString(pComposing->uRefresh, refreshStr, 6))
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tString Copy error",
				__FUNCTION__, __LINE__);

			uCPMError = ECRIO_CPM_INTERNAL_ERROR;
			goto Error;
		}
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), refreshStr, pal_StringLength(refreshStr), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"refresh", pal_StringLength((u_char*)"refresh"), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);
	}

	/** <contenttype> */
	if (pComposing->pContent != NULL)
	{
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"contenttype", pal_StringLength((u_char*)"contenttype"), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), pComposing->pContent, pal_StringLength(pComposing->pContent), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"contenttype", pal_StringLength((u_char*)"contenttype"), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);
	}

	/** <lastactive> */
	if (pComposing->pLastActive != NULL)
	{
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"lastactive", pal_StringLength((u_char*)"lastactive"), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), pComposing->pLastActive, pal_StringLength(pComposing->pLastActive), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"lastactive", pal_StringLength((u_char*)"lastactive"), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);
	}

	/** </isComposing> */
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"isComposing", pal_StringLength((u_char*)"isComposing"), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	uLen = pal_StringLength(pData);

	*ppData = (char*)pData;
	*pLen = uLen;

	goto END;

Error:
	pal_MemoryFree((void**)&pData);

	*ppData = NULL;
	*pLen = 0;

END:
	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u\tuCPMError=%u", __FUNCTION__, __LINE__, uCPMError);

	return uCPMError;
}

u_int32 ec_CPM_FormPrivacyManagementCommandXML
(
	EcrioCPMContextStruct *pContext,
	char *pCommandID,
	EcrioCPMPrivMngCmdActionEnum eAction,
	char **ppData,
	u_int32 *pLen
)
{
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
	u_char *pData = NULL;
	u_int32 uLen = 0;
	u_int32 uSize = 0;

	/** Check parameter validity. */
	if (pContext == NULL || pCommandID == NULL || ppData == NULL || pLen == NULL)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	uSize = ECRIO_CPM_XML_BUFFER_SIZE;

	pal_MemoryAllocate(uSize, (void **)&pData);
	if (NULL == pData)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		uCPMError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}
	pal_MemorySet(pData, 0, uSize);

	/** <?xml version="1.0" encoding="UTF-8"?> */
	if (NULL == pal_StringNCopy(pData, uSize, ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE)))
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
			__FUNCTION__, __LINE__);
		uCPMError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_QUESTIONMARK, pal_StringLength(ECRIO_CPM_XML_QUESTIONMARK), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_VERSION1, pal_StringLength(ECRIO_CPM_XML_VERSION1), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_EQUALTO, pal_StringLength(ECRIO_CPM_XML_EQUALTO), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_VERSION, pal_StringLength(ECRIO_CPM_XML_VERSION), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SPACE, pal_StringLength(ECRIO_CPM_XML_SPACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ENCODING1, pal_StringLength(ECRIO_CPM_XML_ENCODING1), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_EQUALTO, pal_StringLength(ECRIO_CPM_XML_EQUALTO), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ENCODING, pal_StringLength(ECRIO_CPM_XML_ENCODING), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_QUESTIONMARK, pal_StringLength(ECRIO_CPM_XML_QUESTIONMARK), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	/** <AM xmlns="urn:gsma:params:xml:ns:rcs:rcs:aliasmgmt"> */
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"<AM xmlns=", pal_StringLength((u_char*)"<AM xmlns="), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"urn:gsma:params:xml:ns:rcs:rcs:aliasmgmt", 
		pal_StringLength((u_char*)"urn:gsma:params:xml:ns:rcs:rcs:aliasmgmt"), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	/** <Command-ID> */
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"Command-ID", pal_StringLength((u_char*)"Command-ID"), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)pCommandID, pal_StringLength((u_char*)pCommandID), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"Command-ID", pal_StringLength((u_char*)"Command-ID"), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	/** <action> */
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"action", pal_StringLength((u_char*)"action"), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);

	switch (eAction)
	{
		case EcrioCPMPrivMngCmdAction_Fetch:
		{
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"fetchsettings", pal_StringLength((u_char*)"fetchsettings"), uCPMError, Error);
		}
		break;

		case EcrioCPMPrivMngCmdAction_On:
		{
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"aliason", pal_StringLength((u_char*)"aliason"), uCPMError, Error);
		}
		break;

		case EcrioCPMPrivMngCmdAction_Off:
		{
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"aliasoff", pal_StringLength((u_char*)"aliasoff"), uCPMError, Error);
		}
		break;

		case EcrioCPMPrivMngCmdAction_Link:
		{
			CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"aliaslink", pal_StringLength((u_char*)"aliaslink"), uCPMError, Error);
		}
		break;

		case EcrioCPMPrivMngCmdAction_None:
		default:
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tInvalid action",
				__FUNCTION__, __LINE__);

			uCPMError = ECRIO_CPM_INTERNAL_ERROR;
			goto Error;
		}
		break;
	}

	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"action", pal_StringLength((u_char*)"action"), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	/** </AM> */
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"AM", pal_StringLength((u_char*)"AM"), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	uLen = pal_StringLength(pData);

	*ppData = (char*)pData;
	*pLen = uLen;

	goto END;

Error:
	if (pData != NULL)
	{
		pal_MemoryFree((void**)&pData);
	}
	*ppData = NULL;
	*pLen = 0;

END:
	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u\tuCPMError=%u", __FUNCTION__, __LINE__, uCPMError);

	return uCPMError;
}

u_int32 ec_CPM_FormSpamReportMessageXML
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMSpamReportMessageStruct *pMsg,
	char **ppData,
	u_int32 *pLen
)
{
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
	u_char *pData = NULL;
	u_int32 uLen = 0;
	u_int32 i = 0;
	u_int32 uSize = 0;

	/** Check parameter validity. */
	if (pContext == NULL || pMsg == NULL || ppData == NULL || pLen == NULL)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	uSize = ECRIO_CPM_XML_BUFFER_SIZE;

	pal_MemoryAllocate(uSize, (void **)&pData);
	if (NULL == pData)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		uCPMError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}
	pal_MemorySet(pData, 0, uSize);

	/** <?xml version="1.0" encoding="UTF-8"?> */
	if (NULL == pal_StringNCopy(pData, uSize, ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE)))
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
			__FUNCTION__, __LINE__);
		uCPMError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_QUESTIONMARK, pal_StringLength(ECRIO_CPM_XML_QUESTIONMARK), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_VERSION1, pal_StringLength(ECRIO_CPM_XML_VERSION1), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_EQUALTO, pal_StringLength(ECRIO_CPM_XML_EQUALTO), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_VERSION, pal_StringLength(ECRIO_CPM_XML_VERSION), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SPACE, pal_StringLength(ECRIO_CPM_XML_SPACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ENCODING1, pal_StringLength(ECRIO_CPM_XML_ENCODING1), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_EQUALTO, pal_StringLength(ECRIO_CPM_XML_EQUALTO), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_ENCODING, pal_StringLength(ECRIO_CPM_XML_ENCODING), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_QUESTIONMARK, pal_StringLength(ECRIO_CPM_XML_QUESTIONMARK), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	/** <SR xmlns="urn:gsma:params:xml:ns:rcs:rcs:spamreport"> */
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"<SR xmlns=", pal_StringLength((u_char*)"<SR xmlns="), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"urn:gsma:params:xml:ns:rcs:rcs:spamreport", 
		pal_StringLength((u_char*)"urn:gsma:params:xml:ns:rcs:rcs:spamreport"), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_DOUBLEQUOTES, pal_StringLength(ECRIO_CPM_XML_DOUBLEQUOTES), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	/** <Chatbot> */
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"Chatbot", pal_StringLength((u_char*)"Chatbot"), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)pMsg->pChatbot, pal_StringLength((u_char*)pMsg->pChatbot), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"Chatbot", pal_StringLength((u_char*)"Chatbot"), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	/** <Message-ID> */
	for (i = 0; i < pMsg->uNumOfMessageID; i++)
	{
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"Message-ID", pal_StringLength((u_char*)"Message-ID"), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)pMsg->ppMessageIDs[i], pal_StringLength((u_char*)pMsg->ppMessageIDs[i]), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"Message-ID", pal_StringLength((u_char*)"Message-ID"), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
		CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);
	}

	/** </SR> */
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_OPENING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_OPENING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_SLASH, pal_StringLength(ECRIO_CPM_XML_SLASH), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), (u_char*)"SR", pal_StringLength((u_char*)"SR"), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_CLOSING_ANGLE_BRACE, pal_StringLength(ECRIO_CPM_XML_CLOSING_ANGLE_BRACE), uCPMError, Error);
	CPM_STRING_CONCATENATE(pData, uSize - pal_StringLength(pData), ECRIO_CPM_XML_NEWLINE, pal_StringLength(ECRIO_CPM_XML_NEWLINE), uCPMError, Error);

	uLen = pal_StringLength(pData);

	*ppData = (char*)pData;
	*pLen = uLen;

	goto END;

Error:
	pal_MemoryFree((void**)&pData);

	*ppData = NULL;
	*pLen = 0;

END:
	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u\tuCPMError=%u", __FUNCTION__, __LINE__, uCPMError);

	return uCPMError;
}
