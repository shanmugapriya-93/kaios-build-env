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

#include "EcrioSigMgrCommon.h"
#include "EcrioSigMgrInit.h"
#include "EcrioSigMgrInternal.h"
#include "EcrioSigMgrUtilities.h"
#include "EcrioSigMgrSipMessage.h"
#include "EcrioSigMgrCallControl.h"
#include "EcrioSigMgrCallControlInternal.h"

/*****************************************************************************
                Internal API Declaration Section - Begin
*****************************************************************************/

/*****************************************************************************
                Internal API Declaration Section - End
*****************************************************************************/

/*****************************************************************************
                    API Definition Section - Begin
*****************************************************************************/

/*****************************************************************************

Function:		EcrioSigMgrSendInvite()

Purpose:		Initiates the signaling for a call

Description:	Creates and sends a Invite request.

Input:			SIGMGRHANDLE sigMgrHandle - Handle to signaling manager.
                EcrioSigMgrInviteRequestStruct* pInviteReqInfo - Invite Info.

Input / OutPut:	SIGSESSIONHANDLE* pSessionHandle - Handle to session.

Returns:		error code.
*****************************************************************************/
u_int32 EcrioSigMgrSendInvite
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSipMessageStruct *pInviteReq,
	u_char** ppCallID
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	EcrioSigMgrStruct *pSigMgr = NULL;

	/* Add checks for the sigMgrHandle in case socket error,
	initilization fails, then lots of dump..*/
	/* That might be one of the scenario which ecrio is facing..*/
	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	if (pSigMgr->pSignalingInfo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tSignaling Manager not initialized",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_NOT_INITIALIZED;
		goto Error_Level_01;
	}

	if ((pInviteReq == NULL) && (ppCallID == NULL))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient Data Provided",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	/* Check is invalid in case of sending Re-Invite */
	/*{
		BoolEnum bValid = Enum_FALSE;

		if ((pInviteReq->pMandatoryHdrs == NULL) ||
			(pInviteReq->pMandatoryHdrs->pTo == NULL))
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tMissing To header",
				__FUNCTION__, __LINE__);
			error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
			goto Error_Level_01;
		}

		error = _EcrioSigMgrValidateUri(pSigMgr,
			&(pInviteReq->pMandatoryHdrs->pTo->nameAddr.addrSpec),
			&bValid);
		if ((error != ECRIO_SIGMGR_NO_ERROR) || (bValid == Enum_FALSE))
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tInvalid To URI",
				__FUNCTION__, __LINE__);
			error = ECRIO_SIGMGR_INVALID_DATA;
			goto Error_Level_01;
		}
	}*/

	error = _EcrioSigMgrSendInvite(pSigMgr, pInviteReq, ppCallID);

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************

Function:		EcrioSigMgrTerminateSession()

Purpose:		Terminates the call irrespective of the sate.

Description:	This API will internally determine whether bye/cancel should
                be send. It will also invoke EcrioSigMgrUpdateMessageCallback
                and will pass EcrioSigMgrByeRequestHdrStruct or
                EcrioSigMgrCancelRequestHdrStruct depending on the constructed
                message.

Input:			SIGMGRHANDLE sigMgrHandle - Handle to signaling manager.

Input:			SIGSESSIONHANDLE sessionHandle - Handle to session signaling
                manager.

Returns:		error code.

*****************************************************************************/
u_int32 EcrioSigMgrTerminateSession
(
	SIGMGRHANDLE sigMgrHandle,
	u_char *pCallId,
	BoolEnum bIsTerminatedByUser
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrCallStateEnum state = EcrioSigMgrCallStateUnInitialized;
	EcrioSigMgrSipMessageStruct sipReqInfo = { .eMethodType = EcrioSipMessageTypeNone };
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;

	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	if (pCallId == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInvalid data passed, pCallId = NULL",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	ec_MapGetKeyData(pSigMgr->hHashMap, pCallId, (void **)&pDialogNode);

	error = _EcrioSigMgrGetCallState(sigMgrHandle, pDialogNode, &state);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t Error while retrieving Call State",
			__FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	if (state == EcrioSigMgrCallStateCallEstablished ||
		state == EcrioSigMgrCallStateAcceptedAckPending)
	{
		if (bIsTerminatedByUser != Enum_TRUE)
		{
			u_int32 error = ECRIO_SIGMGR_NO_ERROR;
			u_char **ppReasonParam = NULL;
			u_char **ppReasonVal = NULL;
			u_char **ppVal = NULL;
			u_int16 i = 0;

			pal_MemoryAllocate(1 * sizeof(u_char *), (void **)&ppVal);
			if (NULL == ppVal)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeMemory,
					"%s:%u\tpal_MemoryAllocate() for ppVal returned NULL", __FUNCTION__, __LINE__);
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			ppVal[0] = pal_StringCreate((u_char *)ECRIO_SIG_MGR_BYE_REASON_PROTOCOL, pal_StringLength((u_char *)ECRIO_SIG_MGR_BYE_REASON_PROTOCOL));

			pal_MemoryAllocate(2 * sizeof(u_char *), (void **)&ppReasonParam);
			if (ppReasonParam == NULL)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeMemory,
					"%s:%u\tpal_MemoryAllocate() for ppReasonParam returned NULL", __FUNCTION__, __LINE__);
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			ppReasonParam[0] = pal_StringCreate((u_char *)ECRIO_SIG_MGR_BYE_REASON_CAUSE, pal_StringLength((u_char *)ECRIO_SIG_MGR_BYE_REASON_CAUSE));
			ppReasonParam[1] = pal_StringCreate((u_char *)ECRIO_SIG_MGR_BYE_REASON_TEXT, pal_StringLength((u_char *)ECRIO_SIG_MGR_BYE_REASON_TEXT));

			pal_MemoryAllocate(2 * sizeof(u_char *), (void **)&ppReasonVal);
			if (ppReasonVal == NULL)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeMemory,
					"%s:%u\tpal_MemoryAllocate() for ppReasonVal returned NULL", __FUNCTION__, __LINE__);
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			ppReasonVal[0] = pal_StringCreate((u_char *)"503", pal_StringLength((u_char *)"503"));
			ppReasonVal[1] = pal_StringCreate((u_char *)"\"Service Unavailable\"", pal_StringLength((u_char *)"\"Service Unavailable\""));

			EcrioSigMgrCreateOptionalHeaderList(pSigMgr, &sipReqInfo.pOptionalHeaderList);
			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, sipReqInfo.pOptionalHeaderList, EcrioSipHeaderTypeReason, 1, ppVal, 2, ppReasonParam, ppReasonVal);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}

			if (ppVal)
			{
				for (i = 0; i < 1; i++)
				{
					if (ppVal[i])
					{
						pal_MemoryFree((void **)&ppVal[i]);
					}
				}

				pal_MemoryFree((void **)&ppVal);
			}

			if (ppReasonParam)
			{
				for (i = 0; i < 2; i++)
				{
					if (ppReasonParam[i])
					{
						pal_MemoryFree((void **)&ppReasonParam[i]);
					}
				}

				pal_MemoryFree((void **)&ppReasonParam);
			}

			if (ppReasonVal)
			{
				for (i = 0; i < 2; i++)
				{
					if (ppReasonVal[i])
					{
						pal_MemoryFree((void **)&ppReasonVal[i]);
					}
				}

				pal_MemoryFree((void **)&ppReasonVal);
			}
		}
			
		pDialogNode->pCurrForkedMsgNode = pDialogNode->pAcceptedForkedMsgNode;
		error = _EcrioSigMgrSendBye(sigMgrHandle, pDialogNode, &sipReqInfo);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrSendBye() returns error = %d",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}
	else if (state == EcrioSigMgrCallStateSetupInProgress)
	{
		error = _EcrioSigMgrSendCancel(sigMgrHandle, pDialogNode, &sipReqInfo);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSigMgrSendCancel() returns error = %d",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}
	else
	{
		error = ECRIO_SIGMGR_CANCEL_FAILED;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInvalid state, could not send BYE or CANCEL",
			__FUNCTION__, __LINE__);
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************

Function:		EcrioSigMgrSendInviteResponse()

Purpose:		To respond to INVITE Request

Description:	Construct and send INVITE Response

Input:			SIGMGRHANDLE sigMgrHandle - Handle to signaling manager.

Input:			EcrioSigMgrSipMessageStruct* pInviteResp - Invite response
                header values.

Returns:		error code.
*****************************************************************************/
u_int32 EcrioSigMgrSendInviteResponse
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSipMessageStruct *pInviteResp
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;

	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if ((pInviteResp == NULL) ||
		(pInviteResp->responseCode == ECRIO_SIGMGR_INVALID_RESPONSE_CODE) ||
		(pInviteResp->pReasonPhrase == NULL))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient Data Provided pInviteResp = %x",
			__FUNCTION__, __LINE__, pInviteResp);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	if ((pInviteResp->responseCode / 100 < ECRIO_SIGMGR_1XX_RESPONSE) ||
		(pInviteResp->responseCode / 100 > ECRIO_SIGMGR_6XX_RESPONSE))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInvalid response code passed",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INVALID_ARGUMENTS;
		goto Error_Level_01;
	}

	error = _EcrioSigMgrSendInviteResponse(pSigMgr, pInviteResp);

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 EcrioSigMgrSendPrackRequest
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSipMessageStruct *pPrackReq
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;

	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	if (pPrackReq == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient Data Provided pPrackReq = %x",
			__FUNCTION__, __LINE__, pPrackReq);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	error = _EcrioSigMgrSendPrack(pSigMgr, pPrackReq);

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 EcrioSigMgrSendPrackResponse
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSipMessageStruct *pPrackReq,
	u_char *pReasonPhrase,
	u_int32	responseCode
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;

	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	if (pPrackReq == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient Data Provided pPrackReq = %x",
			__FUNCTION__, __LINE__, pPrackReq);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	ec_MapGetKeyData(pSigMgr->hHashMap, pPrackReq->pMandatoryHdrs->pCallId, (void **)&pDialogNode);
	pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;

	error = _EcrioSigMgrGeneratePrackResp(pSigMgr, pPrackReq, pInviteUsage,
		pReasonPhrase, responseCode);

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

	return error;
}


/*****************************************************************************

Function:		EcrioSigMgrSendUpdateResponse()

Purpose:		To respond to UPDATE Request

Description:	Construct and send UPDATE Response

Input:			SIGMGRHANDLE sigMgrHandle - Handle to signaling manager.

Input:			EcrioSigMgrSipMessageStruct* pUpdateResp - Update response
header values.

Returns:		error code.
*****************************************************************************/
u_int32 EcrioSigMgrSendUpdateResponse
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSipMessageStruct *pUpdateResp
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;

	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if ((pUpdateResp == NULL) ||
		(pUpdateResp->responseCode == ECRIO_SIGMGR_INVALID_RESPONSE_CODE) ||
		(pUpdateResp->pReasonPhrase == NULL))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient Data Provided pInviteResp = %x",
			__FUNCTION__, __LINE__, pUpdateResp);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	if ((pUpdateResp->responseCode / 100 < ECRIO_SIGMGR_1XX_RESPONSE) ||
		(pUpdateResp->responseCode / 100 > ECRIO_SIGMGR_6XX_RESPONSE))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInvalid response code passed",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INVALID_ARGUMENTS;
		goto Error_Level_01;
	}

	error = _EcrioSigMgrGenerateUpdateResp(pSigMgr, pUpdateResp, pUpdateResp->pReasonPhrase, pUpdateResp->responseCode);

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

	return error;
}


/*****************************************************************************

Function:		EcrioSigMgrTerminateForkedEarlyDialogs()

Purpose:		Terminates all forked early dialogs.

Description:	This API will iterate all the forked nodes and terminate, all
early dialogs.

Input:			SIGMGRHANDLE sigMgrHandle - Handle to signaling manager.

Input:			u_char* pCallId - callId.
manager.

Returns:		error code.

*****************************************************************************/
u_int32 EcrioSigMgrTerminateForkedEarlyDialogs
(
SIGMGRHANDLE sigMgrHandle,
u_char* pCallId
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrSipMessageStruct sipReqInfo = { .eMethodType = EcrioSipMessageTypeNone };
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	_EcrioSigMgrForkedMessageListNodeStruct *pForkedMsgHeadBackup = NULL;
	_EcrioSigMgrForkedMessageListNodeStruct *pHeadNode = NULL;
	_EcrioSigMgrForkedMessageListNodeStruct *pNextNode = NULL;


	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	if (pCallId == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInvalid data passed, pCallId = NULL",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	ec_MapGetKeyData(pSigMgr->hHashMap, pCallId, (void **)&pDialogNode);

	pForkedMsgHeadBackup = pDialogNode->pForkedMsgHead;
	pHeadNode = pDialogNode->pForkedMsgHead;

	while (pHeadNode != NULL)
	{
		/* Store pointer to next node */
		pNextNode = pHeadNode->pNext;

		if (pHeadNode->eDialogState == _EcrioSigMgrDialogState_Early)
		{
			pDialogNode->pCurrForkedMsgNode = pHeadNode;
			pDialogNode->pForkedMsgHead = pHeadNode;
			pHeadNode->bIsEarlyDialogTermination = Enum_TRUE;

			error = _EcrioSigMgrSendBye(sigMgrHandle, pDialogNode, &sipReqInfo);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrSendBye() returns error = %d",
					__FUNCTION__, __LINE__, error);
			}
		}
		pHeadNode = pNextNode;
	}

	/* Set head node to both pointers*/
	pDialogNode->pCurrForkedMsgNode = pForkedMsgHeadBackup;
	pDialogNode->pForkedMsgHead = pForkedMsgHeadBackup;

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

	return error;

}
/*****************************************************************************
                    API Definition Section - End
*****************************************************************************/
