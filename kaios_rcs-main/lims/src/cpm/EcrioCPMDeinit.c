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

#include "EcrioCPM.h"
#include "EcrioCPMInternal.h"

u_int32 EcrioCPMDeinit
(
	CPMHANDLE *hCPMHandle
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	u_int32	uSDPError = ECRIO_SDP_NO_ERROR;
	EcrioCPMContextStruct *pContext = NULL;
	EcrioCPMSessionStruct *pCPMSession = NULL;
	LOGHANDLE hLogHandle = NULL;

	if ((NULL == hCPMHandle) || (NULL == *hCPMHandle))
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	/** Get the CPM structure from the handle */
	pContext = (EcrioCPMContextStruct *)*hCPMHandle;

	hLogHandle = pContext->hLogHandle;

	CPMLOGI(hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** De-initialize SDP module */
#if 0
	/** Delete session if the SDP session handle exists. */
	for (i = 0; i < ECRIO_CPM_MAX_SESSIONS; i++)
	{
		if (pContext->CPMActiveSessions[i].hSDPSessionHandle != NULL)
		{
			uSDPError = EcrioSDPSessionDelete(pContext->CPMActiveSessions[i].hSDPSessionHandle);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				uError = ECRIO_CPM_INTERNAL_ERROR;
				CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSDPSessionDelete() uSDPError=%u",
					__FUNCTION__, __LINE__, uSDPError);
				goto Error_Level_01;
			}

			pal_MemoryFree((void **)&(pContext->CPMActiveSessions[i].pSDPSessionInstance));
			pContext->CPMActiveSessions[i].pSDPSessionInstance = NULL;
			pal_MemoryFree((void **)&(pContext->CPMActiveSessions[i].pSDPInformationBuffer));
			pContext->CPMActiveSessions[i].pSDPInformationBuffer = NULL;
			pal_MemoryFree((void **)&(pContext->CPMActiveSessions[i].pSDPWorkingBuffer));
			pContext->CPMActiveSessions[i].pSDPWorkingBuffer = NULL;
			pal_MemoryFree((void **)&(pContext->CPMActiveSessions[i].pSDPStringBuffer));
			pContext->CPMActiveSessions[i].pSDPStringBuffer = NULL;

			pContext->CPMActiveSessions[i].hSDPSessionHandle = NULL;
		}
	}
#endif

	/* Check if any active session and release them */
#if 0
	EcrioSigMgrGetNextAppData(pContext->hEcrioSigMgrHandle, (void **)&pCPMSession);
	while (pCPMSession != NULL)
	{
		ec_CPM_ReleaseCPMSession(pContext, pCPMSession);
		pCPMSession = NULL;
		EcrioSigMgrGetNextAppData(pContext->hEcrioSigMgrHandle, (void **)&pCPMSession);
	}
#endif

	if (pContext->hSDPHandle != NULL)
	{
		uSDPError = EcrioSDPDeinit(pContext->hSDPHandle);
		if (uSDPError != ECRIO_SDP_NO_ERROR)
		{
			uError = ECRIO_CPM_INTERNAL_ERROR;
			CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSDPDeinit() uSDPError=%u",
				__FUNCTION__, __LINE__, uSDPError);
			goto Error_Level_01;
		}

		pContext->hSDPHandle = NULL;
	}

	if (pContext->hCPIMHandle)
	{
		EcrioCPIMDeinit(pContext->hCPIMHandle);
		pContext->hCPIMHandle = NULL;
	}

	if (pContext->hIMDNHandle)
	{
		EcrioIMDNDeinit(pContext->hIMDNHandle);
		pContext->hIMDNHandle = NULL;
	}


	if (pContext->hHashMap)
	{
		ec_CPM_MapDeInit(pContext->hHashMap);
		pContext->hHashMap = NULL;
	}

	pal_MemoryFree((void **)&(pContext->pSDPInstance));
	pal_MemoryFree((void **)&(pContext->pCPIMInstance));
	pal_MemoryFree((void **)&(pContext->pIMDNInstance));
	pal_MemoryFree((void **)&(pContext->pMSRPInstance));
	pal_MemoryFree((void **)&(pContext->pString));

	/** release CPM context */
	pal_MemoryFree((void **)hCPMHandle);
	*hCPMHandle = NULL;

Error_Level_01:
	CPMLOGI(hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return uError;
}
