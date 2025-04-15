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

#include "EcrioPAL.h"
#include "EcrioSUEEngine.h"
#include "EcrioSUEInternalFunctions.h"
#include "EcrioIOTA.h"

/******************************************************************************

                API EcrioSUEInit

******************************************************************************/

/**
 * This function is used to initialize the SUE Engine. This is a synchronous
 * function which should be called first before any other SUE API
 * function is called. This function will perform basic checks of input
 * parameters and perform basic component initialization. This function
 * returns the SUE Handle as an output parameter which points to the
 * complete internal state of the SUE.
 *
 * @param[in] pSUEParam				Pointer to a structure that holds the initial
 *									configuration for the SUE. This structure
 *									should be populated by the calling
 *									function. If the calling function allocates this pointer can free the
 *									memory associated with this pointer after
 *									this function returns.
 * @param[in] pCallbackStruct		Pointer to a structure holding the callback
 *									information used to communicate notification information
 *									to the calling layer. This structure should be
 *									populated by the calling function. If the calling function
 *									allocates this pointer can free the memory associated
 *									with this pointer after this function returns.
 * @param[out] ppSUEHandle			The SUE Handle an output parameter. This is
 *									a pointer to which the SUE will allocate
 *									memory. If the referenced pointer is not NULL,
 *									an error will be returned. The calling layer should
 *									never free the memory associated with the handle
 *									since it is needed when invoking any other SUE
 *									API. The EcrioSUEDeinit() should be called to free
 *									the memory.
 * @return                          ECRIO_SUE_NO_ERROR if successful otherwise a
 *									non-zero value indicating the reason for failure.
 */
u_int32 EcrioSUEInit
(
	EcrioSUEParamStruct *pSUEParam,
	EcrioSUECallbackStruct *pCallbackStruct,
	void *hSigMgrHandle,
	SUEENGINEHANDLE *ppSUEHandle
)
{
	u_int32	uError = ECRIO_SUE_NO_ERROR;
	_EcrioSUEGlobalDataStruct *pSUEGlobalData = NULL;

	if (*ppSUEHandle)
	{
		return ECRIO_SUE_INVALID_OPERATION_ERROR;
	}

	if (!pSUEParam || !pCallbackStruct)
	{
		uError = ECRIO_SUE_INSUFFICIENT_DATA_ERROR;
		return uError;
	}

	/* Logging SMS ALC version*/
	SUELOGI(pSUEParam->pLogHandle, KLogTypeGeneral, "%s:%u\t - SUE Version %s", __FUNCTION__, __LINE__, ECRIO_IOTA_VERSION_STRING);

	SUELOGI(pSUEParam->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pal_MemoryAllocate(sizeof(_EcrioSUEGlobalDataStruct), (void **)&pSUEGlobalData);
	if (!pSUEGlobalData)
	{
		uError = ECRIO_SUE_INSUFFICIENT_MEMORY_ERROR;
		goto EndTag;
	}

	pSUEGlobalData->bNotifyApp = Enum_FALSE;
	pSUEGlobalData->eEngineState = _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Initialized;
	pSUEGlobalData->pal = pSUEParam->pal;
	pSUEGlobalData->pLogHandle = pSUEParam->pLogHandle;
	pSUEGlobalData->pSigMgrHandle = hSigMgrHandle;
	pSUEGlobalData->callbackStruct = *pCallbackStruct;
	pSUEGlobalData->bSubscribeRegEvent = pSUEParam->bSubscribeRegEvent;
	pSUEGlobalData->bUnSubscribeRegEvent = pSUEParam->bUnSubscribeRegEvent;
	pSUEGlobalData->pOOMObject = pSUEParam->pOOMObject;

	uError = _EcrioSUEStart(pSUEGlobalData);
	if (uError != ECRIO_SUE_NO_ERROR)
	{
		SUELOGE(pSUEParam->pLogHandle, KLogTypeGeneral, "%s:%u\t Internal error --> ", __FUNCTION__, __LINE__);
		goto EndTag;
	}

	*ppSUEHandle = pSUEGlobalData;
	pSUEGlobalData = NULL;

EndTag:
	SUELOGI(pSUEParam->pLogHandle, KLogTypeFuncExit, "%s:%u\t error = %u ", __FUNCTION__, __LINE__, uError);
	if (uError != ECRIO_SUE_NO_ERROR)
	{
		if (pSUEGlobalData)
		{
			_EcrioSUEExit(&pSUEGlobalData);
		}
	}

	return uError;
}
