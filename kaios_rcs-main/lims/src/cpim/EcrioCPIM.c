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
 * @file EcrioCPIM.c
 * @brief Implementation of the Ecrio CPIM Interface implementation instance
 * functions.
*/

#include "EcrioPAL.h"
#include "EcrioCPIM.h"
#include "EcrioCPIMCommon.h"


u_int32 ec_cpim_MaintenanceBuffer
(
	EcrioCPIMBufferStruct *pBuff,
	u_int32 uTotal
)
{
	if (pBuff == NULL)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_1;
	}

	if (pBuff->uSize + uTotal > pBuff->uContainerSize)
	{
		/** Extra 1 byte for null-termination of character string. */
		pal_MemoryReallocate(pBuff->uSize + uTotal + 1, (void **)&pBuff->pData);
		if (pBuff->pData == NULL)
		{
			return ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
		}
		pBuff->uContainerSize = pBuff->uSize + uTotal + 1;
	}

	return ECRIO_CPIM_NO_ERROR;
}

u_int32 ec_cpim_StringCopy
(
	EcrioCPIMBufferStruct *pStrings,
	void **ppDest,
	const void *pSource,
	u_int32 strLength
)
{
	if (pStrings == NULL)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_1;
	}

	if (ppDest == NULL)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_2;
	}

	if (pSource == NULL)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_3;
	}

	if (strLength == 0)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_4;
	}

	if ((pStrings->uSize + strLength + 1) > pStrings->uContainerSize)
	{
		return ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
	}

	*ppDest = &pStrings->pData[pStrings->uSize];
	pal_MemoryCopy(&pStrings->pData[pStrings->uSize], pStrings->uContainerSize, pSource, strLength);
	pStrings->pData[pStrings->uSize + strLength] = 0;
	pStrings->uSize += strLength + 1;

	return ECRIO_CPIM_NO_ERROR;
}

void ec_cpim_StringConcatenate
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

CPIM_HANDLE EcrioCPIMInit
(
	CPIMConfigStruct *pConfig,
	u_int32 *pError
)
{
	EcrioCPIMStruct *c = NULL;
	u_int32 uCPIMError = ECRIO_CPIM_NO_ERROR;
	u_int32 uPALError = KPALErrorNone;
	u_int32 i = 0;

	if (pError == NULL)
	{
		return NULL;
	}

	if (pConfig == NULL)
	{
		*pError = ECRIO_CPIM_INVALID_PARAMETER_1;
		return NULL;
	}

	CPIMLOGI(pConfig->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	*pError = ECRIO_CPIM_NO_ERROR;

	/** Assign memory for the primary instance structure. */
	if (pConfig->uAllocationSize >= sizeof(EcrioCPIMStruct))
	{
		CPIMLOGI(pConfig->logHandle, KLogTypeGeneral, "%s:%u\tMemory allocation: uAllocationSize %d bytes, required: %d bytes.",
			__FUNCTION__, __LINE__, pConfig->uAllocationSize, sizeof(EcrioCPIMStruct));
		c = (EcrioCPIMStruct *)pConfig->pAllocated;
	}
	else
	{
		CPIMLOGE(pConfig->logHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error, uAllocationSize %d bytes, %d bytes or more is required.",
			__FUNCTION__, __LINE__, pConfig->uAllocationSize, sizeof(EcrioCPIMStruct));
		uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
		goto ERR_AllocFail;
	}

	if (c == NULL)
	{
		uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
		goto ERR_AllocFail;
	}

	pal_MemorySet(c, 0, sizeof(EcrioCPIMStruct));

	c->logHandle = pConfig->logHandle;

	pal_MemoryAllocate(CPIM_STRING_BUFFER_LENGTH, (void **)&c->strings.pData);
	if (c->strings.pData == NULL)
	{
		CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error in the strings buffer.", __FUNCTION__, __LINE__);
		*pError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
		goto ERR_AllocFail;
	}
	c->strings.uContainerSize = CPIM_STRING_BUFFER_LENGTH;
	c->strings.uSize = 0;
	c->strings.uOpaque = 0;


	pal_MemoryAllocate(CPIM_MESSAGE_BUFFER_LENGTH, (void **)&c->msgBuff.pData);
	if (c->msgBuff.pData == NULL)
	{
		CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error in sending message buffer.", __FUNCTION__, __LINE__);
		*pError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
		goto ERR_AllocFail;
	}
	c->msgBuff.uContainerSize = CPIM_MESSAGE_BUFFER_LENGTH;
	c->msgBuff.uSize = 0;
	c->msgBuff.uOpaque = 0;

	pal_MemoryAllocate(CPIM_BUFFER_LENGTH, (void **)&c->cmnBuff.pData);
	if (c->cmnBuff.pData == NULL)
	{
		CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error in common buffer.", __FUNCTION__, __LINE__);
		*pError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
		goto ERR_AllocFail;
	}
	c->cmnBuff.uContainerSize = CPIM_BUFFER_LENGTH;
	c->cmnBuff.uSize = 0;
	c->cmnBuff.uOpaque = 0;

	pal_MemoryAllocate(CPIM_WORKING_BUFFER_LENGTH, (void **)&c->work.pData);
	if (c->work.pData == NULL)
	{
		CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error in working buffer.", __FUNCTION__, __LINE__);
		*pError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
		goto ERR_AllocFail;
	}
	c->work.uContainerSize = CPIM_WORKING_BUFFER_LENGTH;
	c->work.uSize = 0;
	c->work.uOpaque = 0;

	for (i = 0; i < CPIM_MSG_NUM; i++)
	{
		pal_MemoryAllocate(CPIM_MESSAGE_BUFFER_LENGTH, (void **)&c->buff[i].pData);
		if (c->buff[i].pData == NULL)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error in internal message buffer storage.", __FUNCTION__, __LINE__);
			*pError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto ERR_AllocFail;
		}
		c->buff[i].uContainerSize = CPIM_MESSAGE_BUFFER_LENGTH;
		c->buff[i].uSize = 0;
		c->buff[i].uOpaque = 0;
	}

	c->mutexAPI = NULL;
	CPIM_MUTEX_CREATE(c->mutexAPI, uCPIMError, uPALError, ERR_MutexFail, pConfig->logHandle);

	goto ERR_None;

ERR_MutexFail:
ERR_AllocFail:

	*pError = uCPIMError;
	c = NULL;

ERR_None:

	CPIMLOGI(pConfig->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return (CPIM_HANDLE)c;
}

u_int32 EcrioCPIMDeinit
(
	CPIM_HANDLE handle
)
{
	EcrioCPIMStruct *c = NULL;
	LOGHANDLE logHandle = NULL;
	u_int32 i;

	if (handle == NULL)
	{
		return ECRIO_CPIM_NO_ERROR;
	}

	c = (EcrioCPIMStruct *)handle;

	logHandle = c->logHandle;

	CPIMLOGI(logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Cleanup the CPIM instance handle, c. */
	if (c->strings.pData != NULL)
	{
		pal_MemoryFree((void**)&c->strings.pData);
	}

	if (c->msgBuff.pData != NULL)
	{
		pal_MemoryFree((void**)&c->msgBuff.pData);
	}

	if (c->cmnBuff.pData != NULL)
	{
		pal_MemoryFree((void**)&c->cmnBuff.pData);
	}

	if (c->work.pData != NULL)
	{
		pal_MemoryFree((void**)&c->work.pData);
	}

	for (i = 0; i < CPIM_MSG_NUM; i++)
	{
		if (c->buff[i].pData != NULL)
		{
			pal_MemoryFree((void**)&c->buff[i].pData);
		}
	}

	/** Cleanup the API-level mutex. */
	CPIM_MUTEX_DELETE(c->mutexAPI, logHandle);

	CPIMLOGI(logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return ECRIO_CPIM_NO_ERROR;
}
