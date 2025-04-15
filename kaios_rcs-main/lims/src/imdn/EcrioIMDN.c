/******************************************************************************

Copyright (c) 2018-2020 Ecrio, Inc. All Rights Reserved.

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
 * @file EcrioIMDN.c
 * @brief Implementation of the Ecrio IMDN Interface implementation instance
 * functions.
*/

#include "EcrioPAL.h"
#include "EcrioIMDN.h"
#include "EcrioIMDNCommon.h"

u_int32 ec_imdn_MaintenanceBuffer
(
	EcrioIMDNBufferStruct *pBuff,
	u_int32 uTotal
)
{
	if (pBuff == NULL)
	{
		return ECRIO_IMDN_INVALID_PARAMETER_1;
	}

	if (pBuff->uSize + uTotal > pBuff->uContainerSize)
	{
		/** Extra 1 byte for null-termination of character string. */
		pal_MemoryReallocate(pBuff->uSize + uTotal + 1, (void **)&pBuff->pData);
		if (pBuff->pData == NULL)
		{
			return ECRIO_IMDN_MEMORY_ALLOCATION_ERROR;
		}
		pBuff->uContainerSize = pBuff->uSize + uTotal + 1;
	}

	return ECRIO_IMDN_NO_ERROR;
}

u_int32 ec_imdn_StringCopy
(
	EcrioIMDNBufferStruct *pStrings,
	void **ppDest,
	u_int32 uDestlen,
	const void *pSource,
	u_int32 strLength
)
{
	if (pStrings == NULL)
	{
		return ECRIO_IMDN_INVALID_PARAMETER_1;
	}

	if (ppDest == NULL)
	{
		return ECRIO_IMDN_INVALID_PARAMETER_2;
	}

	if (pSource == NULL)
	{
		return ECRIO_IMDN_INVALID_PARAMETER_3;
	}

	if (strLength == 0)
	{
		return ECRIO_IMDN_INVALID_PARAMETER_4;
	}

	if ((pStrings->uSize + strLength + 1) > pStrings->uContainerSize)
	{
		return ECRIO_IMDN_MEMORY_ALLOCATION_ERROR;
	}

	*ppDest = &pStrings->pData[pStrings->uSize];
	pal_MemoryCopy(&pStrings->pData[pStrings->uSize], uDestlen, pSource, strLength);
	pStrings->pData[pStrings->uSize + strLength] = 0;
	pStrings->uSize += strLength + 1;

	return ECRIO_IMDN_NO_ERROR;
}

void ec_imdn_StringConcatenate
(
	u_char *pStart,
	u_int32 uStartlen,
	u_char *pSubString,
	u_char **ppEnd
)
{
	if (NULL == pal_StringNCopy(pStart, uStartlen, pSubString, pal_StringLength(pSubString)))
	{
		return;
	}

	if (ppEnd != NULL)
	{
		*ppEnd = pStart + pal_StringLength(pSubString);
	}
	return;
}

IMDN_HANDLE EcrioIMDNInit
(
	IMDNConfigStruct *pConfig,
	u_int32 *pError
)
{
	EcrioIMDNStruct *s = NULL;
	u_int32 uIMDNError = ECRIO_IMDN_NO_ERROR;
	u_int32 uPALError = KPALErrorNone;

	if (pError == NULL)
	{
		return NULL;
	}

	if (pConfig == NULL)
	{
		*pError = ECRIO_IMDN_INVALID_PARAMETER_1;
		return NULL;
	}

	IMDNLOGI(pConfig->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	*pError = ECRIO_IMDN_NO_ERROR;

	/** Assign memory for the primary instance structure. */
	if (pConfig->uAllocationSize >= sizeof(EcrioIMDNStruct))
	{
		IMDNLOGI(pConfig->logHandle, KLogTypeGeneral, "%s:%u\tMemory allocation: uAllocationSize %d bytes, required: %d bytes.",
			__FUNCTION__, __LINE__, pConfig->uAllocationSize, sizeof(EcrioIMDNStruct));
		s = (EcrioIMDNStruct *)pConfig->pAllocated;
	}
	else
	{
		IMDNLOGE(pConfig->logHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error, uAllocationSize %d bytes, %d bytes or more is required.",
			__FUNCTION__, __LINE__, pConfig->uAllocationSize, sizeof(EcrioIMDNStruct));
		uIMDNError = ECRIO_IMDN_MEMORY_ALLOCATION_ERROR;
		goto ERR_AllocFail;
	}

	if (s == NULL)
	{
		uIMDNError = ECRIO_IMDN_MEMORY_ALLOCATION_ERROR;
		goto ERR_AllocFail;
	}

	pal_MemorySet(s, 0, sizeof(EcrioIMDNStruct));

	s->logHandle = pConfig->logHandle;

	pal_MemoryAllocate(IMDN_XML_LENGTH, (void **)&s->work);
	if (s->work.pData == NULL)
	{
		IMDNLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error in working buffer.", __FUNCTION__, __LINE__);
		uIMDNError = ECRIO_IMDN_MEMORY_ALLOCATION_ERROR;
		goto ERR_AllocFail;
	}
	s->work.uContainerSize = IMDN_XML_LENGTH;
	s->work.uSize = 0;

	pal_MemoryAllocate(IMDN_STRING_LENGTH, (void **)&s->strings);
	if (s->strings.pData == NULL)
	{
		IMDNLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error in strings buffer.", __FUNCTION__, __LINE__);
		uIMDNError = ECRIO_IMDN_MEMORY_ALLOCATION_ERROR;
		goto ERR_AllocFail;
	}
	s->strings.uContainerSize = IMDN_STRING_LENGTH;
	s->strings.uSize = 0;

	s->mutexAPI = NULL;
	IMDN_MUTEX_CREATE(s->mutexAPI, uIMDNError, uPALError, ERR_MutexFail, pConfig->logHandle);

	goto ERR_None;

ERR_MutexFail:
ERR_AllocFail:

	*pError = uIMDNError;
	s = NULL;

ERR_None:

	IMDNLOGI(pConfig->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return (IMDN_HANDLE)s;
}

u_int32 EcrioIMDNDeinit
(
	IMDN_HANDLE handle
)
{
	EcrioIMDNStruct *s = NULL;
	LOGHANDLE logHandle = NULL;

	if (handle == NULL)
	{
		return ECRIO_IMDN_NO_ERROR;
	}

	s = (EcrioIMDNStruct *)handle;

	logHandle = s->logHandle;

	IMDNLOGI(logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Cleanup the IMDN instance handle, s. */
	if (s->work.pData != NULL)
	{
		pal_MemoryFree((void**)&s->work.pData);
	}

	if (s->strings.pData != NULL)
	{
		pal_MemoryFree((void**)&s->strings.pData);
	}

	/** Cleanup the API-level mutex. */
	IMDN_MUTEX_DELETE(s->mutexAPI, logHandle);

	IMDNLOGI(logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return ECRIO_IMDN_NO_ERROR;
}
