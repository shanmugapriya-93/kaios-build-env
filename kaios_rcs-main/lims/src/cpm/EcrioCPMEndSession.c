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

/** API for Ending an ongoing session in the CPM layer.
*
*@ Param IN CPMHANDLE hCPMHandle - CPM Handle. This must be used by the
*				application while invoking any API provided by CPM Layer.
*
*@ Param IN u_int32 sessionId - Session identifier. This is the identifier of the
*				session / session that is being requested to be ended.
*@ Param IN BoolEnum termByUser - Boolean that decides reason code in BYE.
*@ return	u_int32 - Indicating the status of the termination action
ECRIO_CPM_NO_ERROR or ECRIO_CPM_INSUFFICIENT_DATA_ERROR or ECRIO_CPM_INTERNAL_ERROR.
**/
u_int32 EcrioCPMEndSession
(
	CPMHANDLE hCPMHandle,
	char *pSessionId,
	BoolEnum termByUser
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	EcrioCPMContextStruct *pContext = NULL;
	EcrioCPMSessionStruct *pSession = NULL;
	LOGHANDLE hLogHandle = NULL;
	char *pCallId = NULL;

	if (hCPMHandle == NULL)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	/** Get the CPM structure from the handle */
	pContext = (EcrioCPMContextStruct *)hCPMHandle;

	hLogHandle = pContext->hLogHandle;

	CPMLOGI(hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pContext->bIsRelayEnabled == Enum_TRUE)
		ec_CPM_MapGetData(pContext, (u_char *)pSessionId, (void**)&pCallId);
	else
		pCallId = pSessionId;

	/** Get the session structure corresponding to the session id */
	EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, (u_char *)pCallId, (void**)&pSession);

	if (pSession == NULL)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tpSession not found for pSessionId=%s",
			__FUNCTION__, __LINE__, pSessionId);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto ErrTag;

	}

	if (pSession->state != EcrioCPMSessionState_Starting &&
		pSession->state != EcrioCPMSessionState_Active)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tCurrent session is not in Active State...",
			__FUNCTION__, __LINE__);

		uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
		goto ErrTag;
	}

	uError = ec_CPM_EndSession(pContext, pSession, termByUser);
	if (ECRIO_CPM_NO_ERROR != uError)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_EndSession() uError=%u ",
			__FUNCTION__, __LINE__, uError);
	}

ErrTag:

	pSession = NULL;

	CPMLOGI(hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return uError;
}

u_int32 EcrioCPMAbortFileTransfer
(
	CPMHANDLE hCPMHandle,
	char *pSessionId
)
{
	u_int32 uError = ECRIO_CPM_NO_ERROR;
	EcrioCPMContextStruct *pContext = NULL;
	EcrioCPMSessionStruct *pSession = NULL;
	LOGHANDLE hLogHandle = NULL;
	char *pCallId = NULL;

	if (hCPMHandle == NULL)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	/** Get the CPM structure from the handle */
	pContext = (EcrioCPMContextStruct *)hCPMHandle;

	hLogHandle = pContext->hLogHandle;

	CPMLOGI(hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pContext->bIsRelayEnabled == Enum_TRUE)
		ec_CPM_MapGetData(pContext, (u_char *)pSessionId, (void**)&pCallId);
	else
		pCallId = pSessionId;

	/** Get the session structure corresponding to the session id */
	EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, (u_char *)pCallId, (void**)&pSession);

	if (pSession == NULL)
	{
		CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tpSession not found for pSessionId=%s",
			__FUNCTION__, __LINE__, pSessionId);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto ErrTag;

	}

	if (pSession->hMSRPSessionHandle == NULL)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMSRP session handle is NULL...",
			__FUNCTION__, __LINE__);

		uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
		goto ErrTag;
	}

	if (pSession->state != EcrioCPMSessionState_Starting &&
		pSession->state != EcrioCPMSessionState_Active)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tCurrent session is not in Active State...",
			__FUNCTION__, __LINE__);

		uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
		goto ErrTag;
	}

	uError = EcrioMSRPAbortFileTransfer(pSession->hMSRPSessionHandle);
	if (ECRIO_MSRP_NO_ERROR != uError)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_EndSession() uError=%u ",
			__FUNCTION__, __LINE__, uError);
	}

ErrTag:

	pSession = NULL;

	CPMLOGI(hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return uError;
}

u_int32 ec_CPM_EndSession
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMSessionStruct *pSession,
	BoolEnum isTerminatedByUser
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	u_int32 uSigMgrError = ECRIO_SIGMGR_NO_ERROR;

	/** Call the Ecrio IMS layer API to end the session */
	uSigMgrError = EcrioSigMgrTerminateSession(pContext->hEcrioSigMgrHandle, pSession->pCallId, isTerminatedByUser);

	if (ECRIO_SIGMGR_NO_ERROR != uSigMgrError)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrTerminateSession() uSigMgrError=%u ",
			__FUNCTION__, __LINE__, uSigMgrError);

		if (uSigMgrError == ECRIO_SIG_MGR_SIG_SOCKET_ERROR)
			uError = ECRIO_CPM_SOCKET_ERROR;
		else
			uError = ECRIO_CPM_INTERNAL_ERROR;
	}
	
	/* Release the SDP session handle while cancelling session. */
	if (pSession->state != EcrioCPMSessionState_Active && 
		uSigMgrError != ECRIO_SIGMGR_CANCEL_FAILED)
	{
		uError = ec_CPM_ReleaseCPMSession(pContext, pSession);
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_ReleaseCPMSession() uSDPError=%u",
				__FUNCTION__, __LINE__, uError);
		}
	}

	return uError;
}


u_int32 ec_CPM_ReleaseCPMSession
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMSessionStruct *pSession
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	u_int32 i = 0;
	/*
	if (pSession->pCallId)
	{
		pal_MemoryFree((void**)&pSession->pCallId);
		pSession->pCallId = NULL;
	}*/

	if (pSession == NULL)
	{
		return uError;
	}

	/** Release the SDP session handle */
	uError = EcrioSDPSessionDelete(pSession->hSDPSessionHandle);
	if (uError != ECRIO_SDP_NO_ERROR)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSDPSessionDelete() uSDPError=%u",
			__FUNCTION__, __LINE__, uError);
	}

	pSession->hSDPSessionHandle = NULL;

	pal_MemoryFree((void **)&(pSession->pSDPSessionInstance));
	pSession->pSDPSessionInstance = NULL;
	pal_MemoryFree((void **)&(pSession->pSDPInformationBuffer));
	pSession->pSDPInformationBuffer = NULL;
	pal_MemoryFree((void **)&(pSession->pSDPWorkingBuffer));
	pSession->pSDPWorkingBuffer = NULL;
	pal_MemoryFree((void **)&(pSession->pSDPStringBuffer));
	pSession->pSDPStringBuffer = NULL;

	if (pSession->hMSRPSessionHandle)
	{
		uError = EcrioMSRPSessionStop(pSession->hMSRPSessionHandle);
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			uError = ECRIO_CPM_INTERNAL_ERROR;
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioMSRPSessionStop() uMSRPError=%u",
				__FUNCTION__, __LINE__, uError);
		}

		/** Release the MSRP session handle */
		uError = EcrioMSRPSessionDelete(pSession->hMSRPSessionHandle);
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			uError = ECRIO_CPM_INTERNAL_ERROR;
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioMSRPSessionDelete() uMSRPError=%u",
				__FUNCTION__, __LINE__, uError);
		}
	}

	pal_MemoryFree((void **)&(pSession->pMSRPSessionInstance));
	pal_MemoryFree((void **)&(pSession->pMSRPRecvBuffer));

	if (pSession->pCallId != NULL)
		pal_MemoryFree((void **)&(pSession->pCallId));

	pal_MemoryFree((void **)&(pSession->relayData.pAppCallId));

	pal_MemoryFree((void **)&(pSession->relayData.pDest));

	if (pSession->pConvId != NULL)
	{
		pal_MemoryFree((void **)&(pSession->pConvId->pContributionId));
		pal_MemoryFree((void **)&(pSession->pConvId->pConversationId));
		pal_MemoryFree((void **)&(pSession->pConvId->pInReplyToContId));
		pal_MemoryFree((void **)&(pSession->pConvId));
	}

	if (pSession->pFile != NULL)
	{
		if (pSession->pFile->pFilePath != NULL)
		{
			pal_MemoryFree((void **)&(pSession->pFile->pFilePath));
		}

		if (pSession->pFile->pFileName != NULL)
		{
			pal_MemoryFree((void **)&(pSession->pFile->pFileName));
		}

		if (pSession->pFile->pFileType != NULL)
		{
			pal_MemoryFree((void **)&(pSession->pFile->pFileType));
		}

		if (pSession->pFile->pFileTransferId != NULL)
		{
			pal_MemoryFree((void **)&(pSession->pFile->pFileTransferId));
		}

		if (pSession->pFile->pIMDNMsgId != NULL)
		{
			pal_MemoryFree((void **)&(pSession->pFile->pIMDNMsgId));
		}

		pal_MemoryFree((void **)&(pSession->pFile));
	}

	if (pSession->pSubject != NULL)
	{
		pal_MemoryFree((void **)&(pSession->pSubject));
	}

	if (pSession->pGroupSessionId != NULL)
	{
		pal_MemoryFree((void **)&(pSession->pGroupSessionId));
	}
	if (pSession->pList != NULL)
	{
		for (i = 0; i < pSession->pList->uNumOfUsers; i++)
		{
			pal_MemoryFree((void **)&(pSession->pList->ppUri[i]));
		}
		if (pSession->pList->ppUri != NULL)
		{
			pal_MemoryFree((void **)&(pSession->pList->ppUri));
		}
		pal_MemoryFree((void **)&(pSession->pList));
	}

	if (pSession->pGrpChatSubSessId)
	{
		EcrioSigMgrAbortSession(pContext->hEcrioSigMgrHandle, pSession->pGrpChatSubSessId);
		pSession->pGrpChatSubSessId = NULL;
	}

	pal_MemoryFree((void **)&(pSession->relayData.localConnInfo.pLocalIp));

	pal_MemoryFree((void **)&(pSession->relayData.pLocalPath));

	pal_MemoryFree((void **)&(pSession->relayData.pUsePath));

	/** Reinitialize the CurrentCPMSessionStruct member */

	pal_MemoryFree((void **)&(pSession));
	pSession = NULL;

	return uError;
}
