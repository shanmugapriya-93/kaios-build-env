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

#include "EcrioSigMgrCommon.h"
#include "EcrioSigMgrInit.h"

#include "EcrioSUEInternalFunctions.h"

/******************************************************************************

                Internal Function _EcrioSUEExit

******************************************************************************/

/**
 * This internal function is used to de-initialize the SUE. This function will deactivate
 * the bearer network, undeploy and disconnect from SAC, de-initialize the lower layer
 * modules like SUEExtn, XMLLibrary, SigMgr, Phone Module etc and clean
 * up all resources and free all memory used by the SUE. This is a synchronous
 * function is called by EcrioSUEDeinit.
 *
 * @param[in,out] ppSUEGlobalData	Pointer to the SUE Handle. If the referenced
 *									pointer is NULL, then the function will return
 *									successfully. If not NULL, an attempt will be
 *									made to clean up resources and memory.
 * @return                          ECRIO_SUE_NO_ERROR if successful otherwise a
 *									non-zero value indicating the reason of failure.
 */
u_int32 _EcrioSUEExit
(
	_EcrioSUEGlobalDataStruct **ppSUEGlobalData
)
{
	u_int32 uError = ECRIO_SUE_NO_ERROR;
	_EcrioSUEGlobalDataStruct *pSUEData = NULL;
	void *pLogHandle = NULL;

	pSUEData = *ppSUEGlobalData;

	pLogHandle = pSUEData->pLogHandle;

	SUELOGI( pLogHandle, KLogTypeFuncExit, "%s:%u\t Entry --> ", __FUNCTION__, __LINE__ );

	if (pSUEData->eEngineState != _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Initialized)
	{
		/* Start - To Support LTE Requirement */
		if (Enum_TRUE == pSUEData->bRegReAttemptTimerStarted)
		{
			uError = pal_TimerStop(pSUEData->uRegReAttemptTimerId);
			pSUEData->uRegReAttemptTimerId = NULL;
			pSUEData->bRegReAttemptTimerStarted = Enum_FALSE;
		}

		if (Enum_TRUE == pSUEData->bSubReAttemptTimerStarted)
		{
			uError = pal_TimerStop(pSUEData->uSubReAttemptTimerId);
			pSUEData->uSubReAttemptTimerId = NULL;
			pSUEData->bSubReAttemptTimerStarted = Enum_FALSE;
		}

		if (pSUEData->pSigMgrHandle)
		{
			/*Memory Leak resolved - START*/
			if (pSUEData->pRegisterIdentifier)
			{
				EcrioSigMgrReleaseRegObject(pSUEData->pSigMgrHandle, &pSUEData->pRegisterIdentifier);
				pSUEData->pRegisterIdentifier = NULL;
			}

			if (pSUEData->pSubscribeSessionId)
			{
				EcrioSigMgrAbortSession(pSUEData->pSigMgrHandle, pSUEData->pSubscribeSessionId);
				//pal_MemoryFree((void**)&pSUEData->pSubscribeSessionId);
				pSUEData->pSubscribeSessionId = NULL;
			}
		}
	}

	if (pSUEData)
	{
		_EcrioSUEStructRelease((void **)&pSUEData, _ECRIO_SUE_STRUCT_ENUM_EcrioSUEGlobalDataStruct, Enum_TRUE);
		pSUEData = NULL;
	}

	*ppSUEGlobalData = pSUEData;/* returns NULL*/

	if (pLogHandle)
	{
		SUELOGI( pLogHandle, KLogTypeFuncExit, "%s:%u\t Exit --> %u", __FUNCTION__, __LINE__, uError);
	}

	return uError;
}
