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

/*****************************************************************************************
File Name         : EcrioSigMgrCallControlImpl.c

Description       : This file implements the functions for the SIP Signaling for Call.

Revision History:

VERSION DATE/AUTHOR					COMMENT
--------------------------------------------------------------------------

*****************************************************************************************/

#include "EcrioSigMgrCommon.h"
#include "EcrioSigMgrInit.h"
#include "EcrioSigMgrInternal.h"
#include "EcrioSigMgrUtilities.h"
#include "EcrioDSListInterface.h"
#include "EcrioSigMgrIMSLibHandler.h"
#include "EcrioSigMgrTransactionHandler.h"
#include "EcrioSigMgrCallbacks.h"
#include "EcrioSigMgrDialogHandler.h"

/**
* Process incoming INVITE request. If loop detected then it
* sends 482 response. It also sends 481 transaction not found
* response if dialog does not match. For valid request it post
* receive invite event to state machine handler.
*
*/
u_int32 _EcrioSigMgrHandleInviteRequest
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pInviteReq,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	u_char *pReasonPhrase,
	u_int32 reasonCode
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;
	EcrioSigMgrMessageStruct message = { .msgCmd = EcrioSigMgrMessageNone };
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	u_int32 sessionInterval = 0;
	EcrioSigMgrHeaderStruct *pAllowHeader = NULL;
	u_int32	i = 0;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pCmnInfo->pToTag == NULL)
	{
		pal_MemoryAllocate(sizeof(_EcrioSigMgrInviteUsageInfoStruct), (void **)&pInviteUsage);
		if (pInviteUsage == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pInviteUsage, error=%u",
				__FUNCTION__, __LINE__, error);

			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pDialogNode = _EcrioSigMgrCreateDialog(pSigMgr, pInviteReq, Enum_TRUE, pInviteUsage);
		if (pDialogNode == NULL)
		{
			error = ECRIO_SIGMGR_BAD_DIALOGMGR_HANDLE;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrCreateDialog() error=%u", __FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}

		ec_MapInsertData(pSigMgr->hHashMap, pDialogNode->pCallId, (void*)pDialogNode);
		pInviteUsage->callState = EcrioSigMgrCallStateInitialized;

		message.msgCmd = EcrioSigMgrInviteRequestNotification;
		message.pData = (void *)pInviteReq;

		pal_MemoryAllocate(sizeof(EcrioSigMgrSipMessageStruct), (void **)&pInviteUsage->pInviteReq);
		if (pInviteUsage->pInviteReq == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pInviteUsage->pInviteReq, error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}

		_EcrioSigMgrCopySipMessage(pSigMgr, pInviteReq, pInviteUsage->pInviteReq);
	}
	else
	{
		u_int32 i = 0;
		BoolEnum bIsForked = Enum_FALSE;
		u_int32 respCode = 0;

		pDialogNode = EcrioSigMgrMatchDialogAndUpdateState(pSigMgr, pInviteReq, &bIsForked, &respCode);
		if (pDialogNode == NULL)
		{
			error = ECRIO_SIGMGR_BAD_DIALOGMGR_HANDLE;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSigMgrMatchDialogAndUpdateState() error=%u", __FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}

		/*ec_MapGetKeyData(pSigMgr->hHashMap, pCmnInfo->pCallId, (void**)&pDialogNode);
		if (pDialogNode == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tec_MapGetKeyData() failed to return valid dialog for callID %s",
				__FUNCTION__, __LINE__, pCmnInfo->pCallId);
			goto Error_Level_01;
		}*/
		pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct*)pDialogNode->pDialogContext;
		pInviteUsage->callState = EcrioSigMgrCallStateInitialized;

		message.msgCmd = EcrioSigMgrReInviteRequestNotification;
		message.pData = (void *)pInviteReq;

		if (pInviteUsage->pInviteReq->pMandatoryHdrs->ppVia != NULL)
		{
			for (i = 0; i < pInviteUsage->pInviteReq->pMandatoryHdrs->numVia; ++i)
			{
				_EcrioSigMgrReleaseViaStruct(pSigMgr, pInviteUsage->pInviteReq->pMandatoryHdrs->ppVia[i]);
				pal_MemoryFree((void **)&(pInviteUsage->pInviteReq->pMandatoryHdrs->ppVia[i]));
			}

			pal_MemoryFree((void **)&(pInviteUsage->pInviteReq->pMandatoryHdrs->ppVia));
		}

		if (pInviteReq->pMandatoryHdrs->ppVia != NULL)
		{
			pInviteUsage->pInviteReq->pMandatoryHdrs->numVia = pInviteReq->pMandatoryHdrs->numVia;

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected(pInviteUsage->pInviteReq->pMandatoryHdrs->numVia, 
				sizeof(EcrioSigMgrViaStruct *)) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pal_MemoryAllocate(sizeof(EcrioSigMgrViaStruct *) * (pInviteUsage->pInviteReq->pMandatoryHdrs->numVia), (void **)&pInviteUsage->pInviteReq->pMandatoryHdrs->ppVia);
			if (pInviteUsage->pInviteReq->pMandatoryHdrs->ppVia == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryAllocate() for pMandatoryHdrs->ppVia, error=%u",
					__FUNCTION__, __LINE__, error);

				goto Error_Level_01;
			}

			for (i = 0; i < pInviteUsage->pInviteReq->pMandatoryHdrs->numVia; ++i)
			{
				pal_MemoryAllocate(sizeof(EcrioSigMgrViaStruct), (void **)&pInviteUsage->pInviteReq->pMandatoryHdrs->ppVia[i]);
				if (pInviteUsage->pInviteReq->pMandatoryHdrs->ppVia[i] == NULL)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tpal_MemoryAllocate() for ppVia, error=%u",
						__FUNCTION__, __LINE__, error);

					goto Error_Level_01;
				}

				error = _EcrioSigMgrPopulateViaHdr(pSigMgr, pInviteReq->pMandatoryHdrs->ppVia[i],
					pInviteUsage->pInviteReq->pMandatoryHdrs->ppVia[i]);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrPopulateViaHdr() error=%u",
						__FUNCTION__, __LINE__, error);

					error = ECRIO_SIGMGR_NO_MEMORY;
					goto Error_Level_01;
				}
			}
		}

		pInviteUsage->callState = EcrioSigMgrCallStateReInviteInProgress;
	}

	if (pInviteUsage->pBranch != NULL)
	{
		pal_MemoryFree((void **)&pInviteUsage->pBranch);
		pInviteUsage->pBranch = NULL;
	}

	_EcrioSigMgrStringCreate(pSigMgr, pCmnInfo->pBranch, &pInviteUsage->pBranch);

	pInviteUsage->isCaller = Enum_FALSE;
	pInviteUsage->bPrackEnabled = Enum_FALSE;

	/* Check Supported header whether support timer value */
	pInviteUsage->isSessionRefresh = Enum_FALSE;
	if (EcrioSigMgrCheckTimerSupport((SIGMGRHANDLE)pSigMgr, pInviteReq->pOptionalHeaderList) == Enum_TRUE)
	{
		pInviteUsage->isSessionRefresh = Enum_TRUE;
	}

	if (pSigMgr->pOOMObject)
	{
		if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeInvite, EcrioSipHeaderTypeSessionExpires))
		{
			/* Check Supported header whether support timer value */
			if (pInviteUsage->isSessionRefresh == Enum_TRUE)
			{
				u_int32 i = 0;
				EcrioSigMgrHeaderStruct *pSessionExpireHeader = NULL;

				/* Check whether session refresher is UAC or UAS */
				EcrioSigMgrGetOptionalHeader(pSigMgr, pInviteReq->pOptionalHeaderList, EcrioSipHeaderTypeSessionExpires, &pSessionExpireHeader);
				if (pSessionExpireHeader != NULL)
				{
					for (i = 0; i < pSessionExpireHeader->numHeaderValues; i++)
					{
						if (pSessionExpireHeader->ppHeaderValues && pSessionExpireHeader->ppHeaderValues[i])
						{
							if (pSessionExpireHeader->ppHeaderValues[i]->pHeaderValue)
							{
								sessionInterval = pal_StringConvertToUNum(pSessionExpireHeader->ppHeaderValues[i]->pHeaderValue, NULL, 10);
							}

							if (sessionInterval > 0)
							{
								if (pSessionExpireHeader->ppHeaderValues[i]->ppParams != NULL &&
									pal_SubString(pSessionExpireHeader->ppHeaderValues[i]->ppParams[0]->pParamName, (u_char *)ECRIO_SIG_MGR_SESSION_REFRESHER_PARAM))
								{
									if (pal_SubString(pSessionExpireHeader->ppHeaderValues[i]->ppParams[0]->pParamValue, (u_char *)ECRIO_SIG_MGR_SESSION_REFRESHER_UAC))
									{
										pInviteUsage->isRefresher = Enum_FALSE;
									}
									else if (pal_SubString(pSessionExpireHeader->ppHeaderValues[i]->ppParams[0]->pParamValue, (u_char *)ECRIO_SIG_MGR_SESSION_REFRESHER_UAS))
									{
										pInviteUsage->isRefresher = Enum_TRUE;
									}
								}
								else
								{
									pInviteUsage->isRefresher = Enum_FALSE;
								}

								pInviteUsage->sessionExpireVal = sessionInterval;
								break;
							}
						}
					}
				}
				else
				{
					pInviteUsage->isRefresher = Enum_TRUE;
				}
			}
		}

		/* Checking in Allow headder ; Check UAC supports "UPDATE" method or not? */
		EcrioSigMgrGetOptionalHeader(pSigMgr, pInviteReq->pOptionalHeaderList,
			EcrioSipHeaderTypeAllow, &pAllowHeader);

		pInviteUsage->isUpdateAllow = Enum_FALSE;
		if (pAllowHeader != NULL)
		{
			if (pAllowHeader->ppHeaderValues && pAllowHeader->ppHeaderValues[0] && pAllowHeader->ppHeaderValues[0]->pHeaderValue)
			{
				for (i = 0; i < pAllowHeader->numHeaderValues; i++)
				{
					if ((pAllowHeader->ppHeaderValues[i] != NULL) &&
						(pal_StringICompare((u_char *)pAllowHeader->ppHeaderValues[i]->pHeaderValue,
						(u_char *)ECRIO_SIG_MGR_METHOD_UPDATE) == 0))
					{
						pInviteUsage->isUpdateAllow = Enum_TRUE;
						break;
					}
				}
			}
		}
	}

	error = _EcrioSigMgrGetConversationsIdHeader(pSigMgr, pInviteReq);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateConversationsIdHeader() failed, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	if (reasonCode != ECRIO_SIGMGR_RESPONSE_CODE_LOOP_DETECTED)
	{
		EcrioSigMgrCallbackRegisteringModuleEnums eModuleId = EcrioSigMgrCallbackRegisteringModule_LIMS;

		eModuleId = _EcrioSigMgrFindModuleId(pSigMgr, pInviteReq);
		_EcrioSigMgrStoreModuleRoutingInfo(pSigMgr, pInviteReq->pMandatoryHdrs->pCallId, eModuleId);

		if (eModuleId == EcrioSigMgrCallbackRegisteringModule_CPM)
		{
			_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
		}
		else
		{
			_EcrioSigMgrUpdateStatusMoIP(pSigMgr, &message);
		}
	}

	if ((pInviteReq->isRequestInValid == Enum_TRUE) && (reasonCode != ECRIO_SIGMGR_INVALID_RESPONSE_CODE))
	{
		error = _EcrioSigMgrSendInviteErrorResponse(pSigMgr, pInviteUsage, pInviteReq,
			pReasonPhrase, reasonCode);
	}
	else if (reasonCode == ECRIO_SIGMGR_RESPONSE_CODE_LOOP_DETECTED)
	{
		error = _EcrioSigMgrSendInviteErrorResponse(pSigMgr, pInviteUsage, pInviteReq,
			pReasonPhrase, reasonCode);
	}

	if (pInviteReq->pConvId)
	{
		/*Just free pID struct, struct release API does not contain this release*/
		pal_MemoryFree((void**)&pInviteReq->pConvId->pContributionId);
		pal_MemoryFree((void**)&pInviteReq->pConvId->pConversationId);
		pal_MemoryFree((void**)&pInviteReq->pConvId->pInReplyToContId);
		pal_MemoryFree((void**)&pInviteReq->pConvId);
	}
	

Error_Level_01:

	if (pInviteUsage &&
		pInviteUsage->callState == EcrioSigMgrCallStateUnInitialized)
	{
		/*message.msgCmd = EcrioSigMgrDeleteSessionNotification;
		message.pData = (void*)pSessionHandle;

		#ifdef _ENABLE_DEFAULT_CALL_HANDLER_CPM_
		_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
		#else
		_EcrioSigMgrUpdateStatusMoIP(pSigMgr, &message);
		#endif
		*/
		/*Routing support*/
		//ec_SigMgrHashMap_Delete(pSigMgr->pHashMap, pCmnInfo->pCallId);

		ec_MapGetKeyData(pSigMgr->hHashMap, pCmnInfo->pCallId, (void **)&pDialogNode);

		_EcrioSigMgrDeleteDialog(pSigMgr, pDialogNode);

		_EcrioSigMgrInternalStructRelease(pSigMgr, EcrioSigMgrStructType_InviteUsageInfo,
			(void **)&pInviteUsage, Enum_TRUE);

		SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tDialog deleted", __FUNCTION__, __LINE__);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* It post response send event to state machine handler.
*
*/
u_int32 _EcrioSigMgrSendInviteResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pInviteResp
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrCommonInfoStruct cmnInfo = { 0 };
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	ec_MapGetKeyData(pSigMgr->hHashMap, pInviteResp->pMandatoryHdrs->pCallId, (void **)&pDialogNode);

	pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
	if (pInviteUsage == NULL)
	{
		return ECRIO_SIGMGR_INVALID_OPERATION;
	}

	cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_INVITE;
	cmnInfo.role = EcrioSigMgrRoleUAS;
	cmnInfo.responseCode = pInviteResp->responseCode;

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\tInvite response code=%d", __FUNCTION__, __LINE__, pInviteResp->responseCode);

	if (pInviteUsage->pMsg != NULL)
	{
		pal_MemoryFree((void **)&pInviteUsage->pMsg);
		pInviteUsage->msgLength = 0;
	}

	error = _EcrioSigMgrCallStateChangeINVPending(pSigMgr, &cmnInfo, pInviteUsage, pInviteResp);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrCallStateChangeINVPending() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	switch (pInviteResp->responseCode / 100)
	{
		case ECRIO_SIGMGR_1XX_RESPONSE:
		{
			pInviteUsage->callState = EcrioSigMgrCallStateSetupInProgress;
		}
		break;

		case ECRIO_SIGMGR_2XX_RESPONSE:
		{
			if (pInviteUsage->callState == EcrioSigMgrCallStateReInviteInProgress)
				pInviteUsage->callState = EcrioSigMgrCallStateReInviteAcceptedAckPending;
			else
				pInviteUsage->callState = EcrioSigMgrCallStateAcceptedAckPending;
		}
		break;

		case ECRIO_SIGMGR_3XX_RESPONSE:
		case ECRIO_SIGMGR_4XX_RESPONSE:
		case ECRIO_SIGMGR_5XX_RESPONSE:
		case ECRIO_SIGMGR_6XX_RESPONSE:
		{
			if (pInviteUsage->callState == EcrioSigMgrCallStateReInviteInProgress)
				pInviteUsage->callState = EcrioSigMgrCallStateReInviteRejectedAckPending;
			else
				pInviteUsage->callState = EcrioSigMgrCallStateRejectedAckPending;
		}
		break;
	}

	/* Setup retransmission for invite response
	*/
	if (((EcrioSigMgrCallStateSetupInProgress == pInviteUsage->callState) ||
		(EcrioSigMgrCallStateAcceptedAckPending == pInviteUsage->callState) ||
		(EcrioSigMgrCallStateReInviteAcceptedAckPending == pInviteUsage->callState))&&
		pInviteResp->responseCode != ECRIO_SIGMGR_RESPONSE_CODE_TRYING)
	{
		error = _EcrioSigMgrCallStateRetransmitResponse(pSigMgr, pDialogNode);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrCallStateRetransmitResponse() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

Error_Level_01:
	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* It starts retransmit timer for invite response.
*
*/
u_int32 _EcrioSigMgrCallStateRetransmitResponse
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrDialogNodeStruct *pDialogNode
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 uPFDerror = KPALErrorNone;
	TimerStartConfigStruct timerConfig = { 0 };
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;

	/* keep reference to Sig Mgr
	*/
	pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
	pInviteUsage->pSigMgr = pSigMgr;

	/* Setup timer*/
	timerConfig.uPeriodicInterval = 0;
	timerConfig.pCallbackData = (void *)pDialogNode;
	timerConfig.bEnableGlobalMutex = Enum_TRUE;

	if (EcrioSigMgrCallStateSetupInProgress == pInviteUsage->callState)
	{
		if (pInviteUsage->bPrackEnabled)
		{
			timerConfig.uInitialInterval = ECRIO_SIG_MGR_1XX_REL_PERIODIC_INTERVAL;
			timerConfig.timerCallback = _EcrioSigMgrCallState1xxPeriodicTimerCallback;

			if (pInviteUsage->periodic1xxTimerId != NULL)
			{
				pal_TimerStop(pInviteUsage->periodic1xxTimerId);
				pInviteUsage->periodic1xxTimerId = NULL;
			}

			/* Start retransmit timer
			*/
			SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStarting INVITE 1xx rel response TU periodic timer in state %u for interval %u ",
				__FUNCTION__, __LINE__, pInviteUsage->callState, timerConfig.uInitialInterval);
			uPFDerror = pal_TimerStart(pSigMgr->pal, &timerConfig, &pInviteUsage->periodic1xxTimerId);
			if (KPALErrorNone != uPFDerror)
			{
				error = ECRIO_SIGMGR_TIMER_ERROR;
				goto Error_Level_01;
			}

			timerConfig.uInitialInterval = 64 * pSigMgr->customSIPTimers.T1;
			timerConfig.timerCallback = _EcrioSigMgrCallStateExpireTimerCallbackUAS;

			SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStarting 1xx INVITE response TU timeout timer in state %u for interval %u ",
				__FUNCTION__, __LINE__, pInviteUsage->callState, timerConfig.uInitialInterval);

			uPFDerror = pal_TimerStart(pSigMgr->pal, &timerConfig, &pInviteUsage->expireTimerId);
			if (KPALErrorNone != uPFDerror)
			{
				error = ECRIO_SIGMGR_TIMER_ERROR;
				goto Error_Level_01;
			}

			pInviteUsage->retransmitInterval = pSigMgr->customSIPTimers.T1;
			timerConfig.uInitialInterval = pInviteUsage->retransmitInterval;
			timerConfig.timerCallback = _EcrioSigMgrCallStateRetransmitTimerCallback;

			/* Start retransmit timer
			*/
			SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStarting INVITE response TU retransmission timer in state %u for interval %u ",
				__FUNCTION__, __LINE__, pInviteUsage->callState, timerConfig.uInitialInterval);
			uPFDerror = pal_TimerStart(pSigMgr->pal, &timerConfig, &pInviteUsage->retransmitTimerId);
			if (KPALErrorNone != uPFDerror)
			{
				error = ECRIO_SIGMGR_TIMER_ERROR;
				goto Error_Level_01;
			}
		}
		else
		{
			timerConfig.uInitialInterval = ECRIO_SIG_MGR_1XX_UNREL_PERIODIC_INTERVAL;
			timerConfig.timerCallback = _EcrioSigMgrCallState1xxPeriodicTimerCallback;

			/* Start retransmit timer
			*/
			SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStarting INVITE 1xx unrel response TU periodic timer in state %u for interval %u ",
				__FUNCTION__, __LINE__, pInviteUsage->callState, timerConfig.uInitialInterval);
			uPFDerror = pal_TimerStart(pSigMgr->pal, &timerConfig, &pInviteUsage->periodic1xxTimerId);
			if (KPALErrorNone != uPFDerror)
			{
				error = ECRIO_SIGMGR_TIMER_ERROR;
				goto Error_Level_01;
			}
		}
	}
	else if (EcrioSigMgrCallStateAcceptedAckPending == pInviteUsage->callState ||
		EcrioSigMgrCallStateReInviteAcceptedAckPending == pInviteUsage->callState)
	{
		/** Stop retransmit timer for 1xx. */
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStoping INVITE response TU retransmission timer in state %u ",
			__FUNCTION__, __LINE__, pInviteUsage->callState);

		if (pInviteUsage->bPrackEnabled)
		{
			if (NULL != pInviteUsage->retransmitTimerId)
			{
				pal_TimerStop(pInviteUsage->retransmitTimerId);
				pInviteUsage->retransmitTimerId = NULL;
			}

			if (NULL != pInviteUsage->expireTimerId)
			{
				pal_TimerStop(pInviteUsage->expireTimerId);
				pInviteUsage->expireTimerId = NULL;
			}
		}

		if (NULL != pInviteUsage->periodic1xxTimerId)
		{
			pal_TimerStop(pInviteUsage->periodic1xxTimerId);
			pInviteUsage->periodic1xxTimerId = NULL;
		}

		/* Start wait timer for 2xx retransmit. */
		timerConfig.uInitialInterval = 64 * pSigMgr->customSIPTimers.T1;
		timerConfig.timerCallback = _EcrioSigMgrCallStateExpireTimerCallbackUAS;

		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStarting INVITE response TU timeout timer in state %u for interval %u ",
			__FUNCTION__, __LINE__, pInviteUsage->callState, timerConfig.uInitialInterval);

		uPFDerror = pal_TimerStart(pSigMgr->pal, &timerConfig, &pInviteUsage->expireTimerId);
		if (KPALErrorNone != uPFDerror)
		{
			error = ECRIO_SIGMGR_TIMER_ERROR;
			goto Error_Level_01;
		}

		pInviteUsage->retransmitInterval = pSigMgr->customSIPTimers.T1;
		timerConfig.uInitialInterval = pInviteUsage->retransmitInterval;
		timerConfig.timerCallback = _EcrioSigMgrCallStateRetransmitTimerCallback;

		/* Start retransmit timer
		*/
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStarting INVITE response TU retransmission timer in state %u for interval %u ",
			__FUNCTION__, __LINE__, pInviteUsage->callState, timerConfig.uInitialInterval);
		uPFDerror = pal_TimerStart(pSigMgr->pal, &timerConfig, &pInviteUsage->retransmitTimerId);
		if (KPALErrorNone != uPFDerror)
		{
			error = ECRIO_SIGMGR_TIMER_ERROR;
			goto Error_Level_01;
		}
	}
	else
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tNot a valid state",
			__FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

Error_Level_01:
	return error;
}

/**
* Expiry Timer callback for retransmit 2xx invite response
*
*/
void _EcrioSigMgrCallStateExpireTimerCallbackUAS
(
	void *pData,
	TIMERHANDLE timerID
)
{
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrSipMessageStruct sipMessage = { .eMethodType = EcrioSipMessageTypeNone };
	EcrioSigMgrMessageStruct message = { .msgCmd = EcrioSigMgrMessageNone };
	EcrioSigMgrSipMessageStruct byeReq = { .eMethodType = EcrioSipMessageTypeNone };

	/* Stop the timer before proceeding further. */
	pal_TimerStop(timerID);

	if (NULL == pData)
	{
		return;
	}

	pDialogNode = (_EcrioSigMgrDialogNodeStruct *)pData;

	pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
	if (NULL == pInviteUsage)
	{
		return;
	}

	pSigMgr = (EcrioSigMgrStruct *)pInviteUsage->pSigMgr;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\ttimerID=%u, pData=%p",
		__FUNCTION__, __LINE__, timerID, pInviteUsage);

	/* Reset expire timer handle
	*/
	pInviteUsage->expireTimerId = NULL;

	if (EcrioSigMgrCallStateSetupInProgress == pInviteUsage->callState)
	{
		if (pInviteUsage->bPrackEnabled == Enum_TRUE)
		{
			/** stop retransmit intvite response timer
			*/
			SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStoping INVITE 1xx response TU retransmission timer in state %u ",
				__FUNCTION__, __LINE__, pInviteUsage->callState);
			if (NULL != pInviteUsage->retransmitTimerId)
			{
				pal_TimerStop(pInviteUsage->retransmitTimerId);
				pInviteUsage->retransmitTimerId = NULL;
				pInviteUsage->retransmitInterval = 0;
			}
		}

		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStoping INVITE 1xx response TU periodic timer in state %u ",
			__FUNCTION__, __LINE__, pInviteUsage->callState);
		if (NULL != pInviteUsage->periodic1xxTimerId)
		{
			pal_TimerStop(pInviteUsage->periodic1xxTimerId);
			pInviteUsage->periodic1xxTimerId = NULL;
		}

		sipMessage.eMethodType = EcrioSipMessageTypeInvite;
		sipMessage.eReqRspType = EcrioSigMgrSIPResponse;
		sipMessage.pReasonPhrase = ECRIO_SIGMGR_RESPONSE_SERVER_INTERNAL_ERROR;
		sipMessage.responseCode = ECRIO_SIGMGR_RESPONSE_CODE_SERVER_INTERNAL_ERROR;
		_EcrioSigMgrSendInviteResponse(pSigMgr, &sipMessage);
	}
	else
	{
		/** stop retransmit intvite response timer
		*/
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStoping INVITE response TU retransmission timer in state %u ",
			__FUNCTION__, __LINE__, pInviteUsage->callState);
		if (NULL != pInviteUsage->retransmitTimerId)
		{
			pal_TimerStop(pInviteUsage->retransmitTimerId);
			pInviteUsage->retransmitTimerId = NULL;
			pInviteUsage->retransmitInterval = 0;
		}

		if (EcrioSigMgrCallStateAcceptedAckPending != pInviteUsage->callState &&
			EcrioSigMgrCallStateReInviteAcceptedAckPending != pInviteUsage->callState)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tNot a valid state, ignored expiry",
				__FUNCTION__, __LINE__);

			goto Error_Level_01;
		}

		/* Send bye
		*/
		error = _EcrioSigMgrSendBye(pSigMgr, pDialogNode, &sipMessage);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrSendBye() returns error = %d",
				__FUNCTION__, __LINE__, error);
		}
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}

void _EcrioSigMgrCallStateSessionExpireTimerCallback
(
	void *pData,
	TIMERHANDLE timerID
)
{

	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrSipMessageStruct byeReq = { .eMethodType = EcrioSipMessageTypeNone };
	EcrioSigMgrMessageStruct message = { .msgCmd = EcrioSigMgrMessageNone };
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	/* Stop the timer before proceeding further. */
	pal_TimerStop(timerID);

	if (NULL == pData)
	{
		return;
	}

	pDialogNode = (_EcrioSigMgrDialogNodeStruct *)pData;
	if (NULL == pDialogNode)
	{
		return;
	}

	pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
	if (NULL == pInviteUsage)
	{
		return;
	}

	if (NULL == pInviteUsage->pInviteReq)
	{
		return;
	}

	pSigMgr = (EcrioSigMgrStruct *)pInviteUsage->pSigMgr;
	if (NULL == pSigMgr)
	{
		return;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\ttimerID=%u, pData=%p",
		__FUNCTION__, __LINE__, timerID, pInviteUsage);

	/* Ignore this event for now. This need to be handled properly with session refresh support*/
#if 0
	if (pInviteUsage->callState == EcrioSigMgrCallStateCallEstablished)
	{
		error = _EcrioSigMgrSendBye(pSigMgr, pDialogNode, &byeReq);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t__EcrioSigMgrSendBye() returns error = %d",
				__FUNCTION__, __LINE__, error);
		}
	}
#endif
	if (pInviteUsage != NULL)
		pInviteUsage->sessionExpireTimerId = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	
	return;
}

/**
* Session refresh timer call back. It sends UPDATE request for
* session refresh.
*/
void _EcrioSigMgrCallStateSessionRefreshTimerCallback
(
	void *pData,
	TIMERHANDLE timerID
)
{
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrMessageStruct message = { .msgCmd = EcrioSigMgrMessageNone };

	/* Stop the timer before proceeding further. */
	pal_TimerStop(timerID);

	if (NULL == pData)
	{
		return;
	}

	pDialogNode = (_EcrioSigMgrDialogNodeStruct *)pData;

	pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
	if (NULL == pInviteUsage)
	{
		return;
	}

	if (NULL == pInviteUsage->pInviteReq)
	{
		return;
	}

	pInviteUsage->sessionExpireTimerId = NULL;

	pSigMgr = (EcrioSigMgrStruct *)pInviteUsage->pSigMgr;
	if (NULL == pSigMgr)
	{
		return;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\ttimerID=%u, pData=%p",
		__FUNCTION__, __LINE__, timerID, pInviteUsage);

	if (pInviteUsage->callState == EcrioSigMgrCallStateCallEstablished)
	{
		message.msgCmd = EcrioSigMgrSessionRefreshNotification;
		message.pData = pInviteUsage->pInviteReq;
		if (_EcrioSigMgrGetModuleId(pSigMgr, pDialogNode->pCallId) == EcrioSigMgrCallbackRegisteringModule_CPM)
		{
			_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
		}
		else
		{
			_EcrioSigMgrUpdateStatusMoIP(pSigMgr, &message);
		}
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}

void _EcrioSigMgrCallState1xxPeriodicTimerCallback
(
	void *pData,
	TIMERHANDLE timerID
)
{
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrCommonInfoStruct cmnInfo = { 0 };

	/* Stop the timer before proceeding further. */
	pal_TimerStop(timerID);

	if (NULL == pData)
	{
		return;
	}

	pDialogNode = (_EcrioSigMgrDialogNodeStruct *)pData;
	pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
	if (NULL == pInviteUsage)
	{
		return;
	}

	pSigMgr = (EcrioSigMgrStruct *)pInviteUsage->pSigMgr;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\ttimerID=%u, pData=%p",
		__FUNCTION__, __LINE__, timerID, pInviteUsage);

	if (NULL == pInviteUsage->pMsg)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tpMsg is NULL",
			__FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	/* Reset retransmit timer handle
	*/
	pInviteUsage->periodic1xxTimerId = NULL;

	if (EcrioSigMgrCallStateSetupInProgress == pInviteUsage->callState)
	{
		cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_INVITE;
		cmnInfo.role = EcrioSigMgrRoleUAS;

		if (pInviteUsage->bPrackEnabled == Enum_TRUE && pInviteUsage->pPrackDetails)
		{
			_EcrioSigMgrUpdateRSeqInProvResp(pInviteUsage);
		}

		error = _EcrioSigMgrCallStateChangeINVPending(pSigMgr, &cmnInfo, pInviteUsage, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrCallStateChangeINVPending() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		_EcrioSigMgrCallStateRetransmitResponse(pSigMgr, pDialogNode);
	}

Error_Level_01:

	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}

/**
* Timer callback to retransmit invite response
*
*/
void _EcrioSigMgrCallStateRetransmitTimerCallback
(
	void *pData,
	TIMERHANDLE timerID
)
{
	u_int32 uPFDerror = KPALErrorNone;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrCommonInfoStruct cmnInfo = { 0 };
	TimerStartConfigStruct timerConfig = { 0 };

	/* Stop the timer before proceeding further. */
	pal_TimerStop(timerID);

	if (NULL == pData)
	{
		return;
	}

	pDialogNode = (_EcrioSigMgrDialogNodeStruct *)pData;
	pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
	if (NULL == pInviteUsage)
	{
		return;
	}

	pSigMgr = (EcrioSigMgrStruct *)pInviteUsage->pSigMgr;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\ttimerID=%u, pData=%p",
		__FUNCTION__, __LINE__, timerID, pInviteUsage);

	if (NULL == pInviteUsage->pMsg)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tpMsg is NULL",
			__FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	/* Reset retransmit timer handle
	*/
	pInviteUsage->retransmitTimerId = NULL;

	cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_INVITE;
	cmnInfo.role = EcrioSigMgrRoleUAS;
	error = _EcrioSigMgrCallStateChangeINVPending(pSigMgr, &cmnInfo, pInviteUsage, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrCallStateChangeINVPending() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	timerConfig.uPeriodicInterval = 0;
	timerConfig.pCallbackData = (void *)pDialogNode;
	timerConfig.bEnableGlobalMutex = Enum_TRUE;
	timerConfig.timerCallback = _EcrioSigMgrCallStateRetransmitTimerCallback;

	if (EcrioSigMgrCallStateSetupInProgress == pInviteUsage->callState)
	{
		if (pInviteUsage->bPrackEnabled == Enum_TRUE)
		{
			pInviteUsage->retransmitInterval = 2 * pInviteUsage->retransmitInterval;
			timerConfig.uInitialInterval = pInviteUsage->retransmitInterval;
		}
		else
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tPrack not enabled, hence no reransmission",
				__FUNCTION__, __LINE__);
			goto Error_Level_01;
		}
	}
	else if (EcrioSigMgrCallStateAcceptedAckPending == pInviteUsage->callState ||
		EcrioSigMgrCallStateReInviteAcceptedAckPending == pInviteUsage->callState)
	{
		pInviteUsage->retransmitInterval = (2 * pInviteUsage->retransmitInterval) > pSigMgr->customSIPTimers.T2 ? pSigMgr->customSIPTimers.T2 : (2 * pInviteUsage->retransmitInterval);
		timerConfig.uInitialInterval = pInviteUsage->retransmitInterval;
	}
	else
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tNot a valid state, retransmission timer couldnt start",
			__FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	/* Start retransmit timer
	*/
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tRe-starting INVITE response TU retransmission timer in state %u for interval %u ",
		__FUNCTION__, __LINE__, pInviteUsage->callState, timerConfig.uInitialInterval);
	uPFDerror = pal_TimerStart(pSigMgr->pal, &timerConfig, &pInviteUsage->retransmitTimerId);
	if (KPALErrorNone != uPFDerror)
	{
		goto Error_Level_01;
	}

Error_Level_01:

	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}

/**
* It post ACK request received event to state machine handler.
*
*/
u_int32 _EcrioSigMgrHandleAckRequest
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pAckReq,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	u_int32 responseCode,
	u_char *pReasonPhrase
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrMessageStruct message = { .msgCmd = EcrioSigMgrMessageNone };
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	BoolEnum bIsForked = Enum_FALSE;
	u_int32 respCode = 0;

	// @todo Does pReasonPhrase or responseCode need to be provided to this function?
	(void)pReasonPhrase;
	(void)responseCode;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pCmnInfo->pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_ACK;
	pCmnInfo->role = EcrioSigMgrRoleUAS;

	pDialogNode = EcrioSigMgrMatchDialogAndUpdateState(pSigMgr, pAckReq, &bIsForked, &respCode);
	if (pDialogNode == NULL)
	{
		error = ECRIO_SIGMGR_BAD_DIALOGMGR_HANDLE;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tEcrioSigMgrMatchDialogAndUpdateState() error=%u", __FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;

	if (pInviteUsage->callState == EcrioSigMgrCallStateAcceptedAckPending ||
		pInviteUsage->callState == EcrioSigMgrCallStateRejectedAckPending ||
		pInviteUsage->callState == EcrioSigMgrCallStateReInviteAcceptedAckPending ||
		pInviteUsage->callState == EcrioSigMgrCallStateReInviteRejectedAckPending)
	{
		message.msgCmd = EcrioSigMgrAckRequestNotifiction;
		message.pData = (void *)pAckReq;
	}

	if (pAckReq->isRequestInValid == Enum_FALSE)
	{
		if (pInviteUsage->callState == EcrioSigMgrCallStateRejectedAckPending)
		{
			pInviteUsage->callState = EcrioSigMgrCallStateUnInitialized;
		}
		else if (pInviteUsage->callState == EcrioSigMgrCallStateAcceptedAckPending ||
			pInviteUsage->callState == EcrioSigMgrCallStateReInviteAcceptedAckPending ||
			pInviteUsage->callState == EcrioSigMgrCallStateReInviteRejectedAckPending)
		{
			pInviteUsage->callState = EcrioSigMgrCallStateCallEstablished;

			if (pInviteUsage->pMsg != NULL)
			{
				pal_MemoryFree((void **)&pInviteUsage->pMsg);
				pInviteUsage->msgLength = 0;
			}
		}
	}

#ifdef _ENABLE_DEFAULT_CALL_HANDLER_CPM_
	_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
#else
	_EcrioSigMgrUpdateStatusMoIP(pSigMgr, &message);
#endif

	/* Stop retransmission & expiry timer for invite response
	*/
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStoping INVITE response TU retransmission timer in state %u ",
		__FUNCTION__, __LINE__, pInviteUsage->callState);
	if (NULL != pInviteUsage->retransmitTimerId)
	{
		pal_TimerStop(pInviteUsage->retransmitTimerId);
		pInviteUsage->retransmitTimerId = NULL;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStoping INVITE response TU timeout timer in state %u",
		__FUNCTION__, __LINE__, pInviteUsage->callState);
	if (NULL != pInviteUsage->expireTimerId)
	{
		pal_TimerStop(pInviteUsage->expireTimerId);
		pInviteUsage->expireTimerId = NULL;
	}

	/* Delete dialog resources when callstate became uninitialized
	*/
	if ((pDialogNode != NULL) && (pInviteUsage->callState == EcrioSigMgrCallStateUnInitialized))
	{
		/*message.msgCmd = EcrioSigMgrDeleteSessionNotification;
		message.pData = (void*)pSessionHandle;

		#ifdef _ENABLE_DEFAULT_CALL_HANDLER_CPM_
		_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
		#else
		_EcrioSigMgrUpdateStatusMoIP(pSigMgr, &message);
		#endif
		*/

		_EcrioSigMgrDeleteDialog(pSigMgr, pDialogNode);

		_EcrioSigMgrInternalStructRelease(pSigMgr, EcrioSigMgrStructType_InviteUsageInfo,
			(void **)&pInviteUsage, Enum_TRUE);

		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tDialog deleted", __FUNCTION__, __LINE__);
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrHandlePrackRequest
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pPrackReq,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	u_int32 responseCode,
	u_char *pReasonPhrase
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrMessageStruct message = { .msgCmd = EcrioSigMgrMessageNone };
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	BoolEnum bIsForked = Enum_FALSE;
	u_int32 respCode = 0;
	EcrioSigMgrHeaderStruct *pRAckHeader = NULL;
	u_int32	uRSeq = 0;
	u_int32	i = 0;
	BoolEnum bRseqMatched = Enum_FALSE;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pCmnInfo->pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_PRACK;
	pCmnInfo->role = EcrioSigMgrRoleUAS;

	pDialogNode = EcrioSigMgrMatchDialogAndUpdateState(pSigMgr, pPrackReq, &bIsForked, &respCode);
	if (pDialogNode == NULL)
	{
		error = ECRIO_SIGMGR_BAD_DIALOGMGR_HANDLE;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tEcrioSigMgrMatchDialogAndUpdateState() error=%u", __FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;

	if (pPrackReq->isRequestInValid == Enum_FALSE)
	{
		if (pInviteUsage->bPrackEnabled == Enum_TRUE && pInviteUsage->pPrackDetails)
		{
			error = EcrioSigMgrGetOptionalHeader(pSigMgr, pPrackReq->pOptionalHeaderList,
				EcrioSipHeaderTypeRAck, &pRAckHeader);
			if (pRAckHeader == NULL)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tEcrioSigMgrGetOptionalHeader() failed, error=%u",
					__FUNCTION__, __LINE__, error);

				goto Error_Level_01;
			}

			if (pRAckHeader->ppHeaderValues && pRAckHeader->ppHeaderValues[0] && pRAckHeader->ppHeaderValues[0]->pHeaderValue)
			{
				error = _EcrioSigMgrExtractRSeqFromRAck(pRAckHeader->ppHeaderValues[0]->pHeaderValue, &uRSeq);
			}

			for (i = 0; i < pInviteUsage->pPrackDetails->uRSeqArrayCount; i++)
			{
				if (pInviteUsage->pPrackDetails->pRseqArray[i].uRseq == uRSeq)
				{
					if (pInviteUsage->pPrackDetails->pRseqArray[i].bAcknowledged == Enum_FALSE)
					{
						pInviteUsage->pPrackDetails->pRseqArray[i].bAcknowledged = Enum_TRUE;
					}

					if (pInviteUsage->callState == EcrioSigMgrCallStateSetupInProgress)
					{
						if (pInviteUsage->retransmitTimerId)
						{
							pal_TimerStop(pInviteUsage->retransmitTimerId);
							pInviteUsage->retransmitTimerId = NULL;
						}

						if (pInviteUsage->expireTimerId)
						{
							pal_TimerStop(pInviteUsage->expireTimerId);
							pInviteUsage->expireTimerId = NULL;
						}
					}

					bRseqMatched = Enum_TRUE;
					break;
				}
			}
		}

		if (bRseqMatched == Enum_TRUE)
		{
			responseCode = ECRIO_SIGMGR_RESPONSE_CODE_OK;
			pReasonPhrase = (u_char *)ECRIO_SIGMGR_RESPONSE_OK;
		}
		else
		{
			responseCode = ECRIO_SIGMGR_RESPONSE_CODE_CALL_TXN_NOT_EXIST;
			pReasonPhrase = (u_char *)ECRIO_SIGMGR_RESPONSE_CALL_TXN_NOT_EXIST;
		}
	}

	if ((pInviteUsage->callState == EcrioSigMgrCallStateCallEstablished) ||
		(pInviteUsage->callState == EcrioSigMgrCallStateSetupInProgress) ||
		(pInviteUsage->callState == EcrioSigMgrCallStateRejectedAckPending) ||
		(pInviteUsage->callState == EcrioSigMgrCallStateAcceptedAckPending))
	{
		/*if (pInviteUsage->pMsg != NULL)
		{
		    ECRIO_FREE_MEM(pSigMgr->moduleId, (void **)&pInviteUsage->pMsg);
		    pInviteUsage->msgLength = 0;
		}*/

		if (responseCode != 200)
		{
			error = _EcrioSigMgrGeneratePrackResp(pSigMgr, pPrackReq, pInviteUsage,
				pReasonPhrase, responseCode);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrGenerateByeResp() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}
		else
		{
			message.msgCmd = EcrioSigMgrPrackRequestNotification;
			message.pData = (void *)pPrackReq;

			if (_EcrioSigMgrGetModuleId(pSigMgr, pCmnInfo->pCallId) == EcrioSigMgrCallbackRegisteringModule_CPM)
			{
				_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
			}
			else
			{
				_EcrioSigMgrUpdateStatusMoIP(pSigMgr, &message);
			}
		}
	}
	else if (pInviteUsage->callState == EcrioSigMgrCallStateTerminationInProgress)
	{
		error = _EcrioSigMgrGeneratePrackResp(pSigMgr, pPrackReq, pInviteUsage,
			pReasonPhrase, responseCode);

		goto Error_Level_01;
	}
	else
	{
		goto Error_Level_01;
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* It posts send2xx to state machine handler for valid request. Otherwise, it
* determines error response code.
*
*/
u_int32 _EcrioSigMgrHandleByeRequest
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pByeReq,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	u_int32 responseCode,
	u_char *pReasonPhrase
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrMessageStruct message = { .msgCmd = EcrioSigMgrMessageNone };
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	BoolEnum bIsForked = Enum_FALSE;
	u_int32 respCode = 0;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pCmnInfo->pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_BYE;
	pCmnInfo->role = EcrioSigMgrRoleUAS;

	pDialogNode = EcrioSigMgrMatchDialogAndUpdateState(pSigMgr, pByeReq, &bIsForked, &respCode);
	if (pDialogNode == NULL)
	{
		error = ECRIO_SIGMGR_BAD_DIALOGMGR_HANDLE;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tEcrioSigMgrMatchDialogAndUpdateState() error=%u", __FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;

	if ((pInviteUsage->callState == EcrioSigMgrCallStateCallEstablished) ||
		(pInviteUsage->callState == EcrioSigMgrCallStateSetupInProgress) ||
		(pInviteUsage->callState == EcrioSigMgrCallStateRejectedAckPending) ||
		(pInviteUsage->callState == EcrioSigMgrCallStateAcceptedAckPending) ||
		(pInviteUsage->callState == EcrioSigMgrCallStateReInviteAcceptedAckPending) ||
		(pInviteUsage->callState == EcrioSigMgrCallStateReInviteRejectedAckPending))
	{
		if (pByeReq->isRequestInValid == Enum_FALSE)
		{
			pInviteUsage->callState = EcrioSigMgrCallStateUnInitialized;
			responseCode = ECRIO_SIGMGR_RESPONSE_CODE_OK;
			pReasonPhrase = (u_char *)ECRIO_SIGMGR_RESPONSE_OK;

			if (pInviteUsage->pMsg != NULL)
			{
				pal_MemoryFree((void **)&pInviteUsage->pMsg);
				pInviteUsage->msgLength = 0;
			}
		}

		error = _EcrioSigMgrGenerateByeResp(pSigMgr, pByeReq, pInviteUsage,
			pReasonPhrase, responseCode);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrGenerateByeResp() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		message.msgCmd = EcrioSigMgrByeRequestNotification;
		message.pData = (void *)pByeReq;

		if (_EcrioSigMgrGetModuleId(pSigMgr, pCmnInfo->pCallId) == EcrioSigMgrCallbackRegisteringModule_CPM)
		{
			_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
		}
		else
		{
			_EcrioSigMgrUpdateStatusMoIP(pSigMgr, &message);
		}
	}
	else if (pInviteUsage->callState == EcrioSigMgrCallStateTerminationInProgress)
	{
		error = _EcrioSigMgrGenerateByeResp(pSigMgr, pByeReq, pInviteUsage,
			(u_char *)ECRIO_SIGMGR_RESPONSE_OK, ECRIO_SIGMGR_RESPONSE_CODE_OK);

		goto Error_Level_01;
	}
	else
	{
		goto Error_Level_01;
	}

Error_Level_01:

	if ((pDialogNode != NULL) && (pInviteUsage->callState == EcrioSigMgrCallStateUnInitialized))
	{
		_EcrioSigMgrDeleteDialog(pSigMgr, pDialogNode);

		_EcrioSigMgrInternalStructRelease(pSigMgr, EcrioSigMgrStructType_InviteUsageInfo,
			(void **)&pInviteUsage, Enum_TRUE);

		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tDialog deleted", __FUNCTION__, __LINE__);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* Creates PRACK response and sends it to transaction manager.
*
*/
u_int32 _EcrioSigMgrSendPrackResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pPrackResp
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 respLen = 0;
	EcrioSigMgrCommonInfoStruct cmnInfo = { 0 };
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };
	u_char *pRespData = NULL;
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	error = _EcrioSigMgrPopulateCommonInfo(pSigMgr, pPrackResp->pMandatoryHdrs, &cmnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateCommonInfo() for pCmnInfo, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	cmnInfo.responseCode = pPrackResp->responseCode;

	eTransportType = pPrackResp->pMandatoryHdrs->ppVia[0]->transport;

	error = _EcrioSigMgrSipMessageForm(pSigMgr, EcrioSigMgrSipMessageModeFullName, pPrackResp, &pRespData, &respLen);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSipMessageForm() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_INVITE_FAILED;
		goto Error_Level_01;
	}

	cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_PRACK;
	cmnInfo.role = EcrioSigMgrRoleUAS;
	cmnInfo.responseCode = pPrackResp->responseCode;

	txnInfo.currentContext = ECRIO_SIGMGR_PRACK_RESPONSE_SEND;
	txnInfo.retransmitContext = ECRIO_SIGMGR_PRACK_RESPONSE_RESEND;

	txnInfo.deleteflag = Enum_FALSE;
	txnInfo.messageType = EcrioSigMgrMessageNonInviteResponse;
	txnInfo.transport = eTransportType;
	txnInfo.pMsg = pRespData;
	txnInfo.msglen = respLen;
	txnInfo.pSessionMappingStr = NULL;

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeSIP,
		"%s:%u\tOutgoing SIP message constructed:", __FUNCTION__, __LINE__);
	SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pRespData, respLen);

	error = _EcrioSigMgrSendMsgToTXNHandler(pSigMgr, &cmnInfo, &txnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendMsgToTXNHandler() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_INVITE_FAILED;
		goto Error_Level_01;
	}

	error = ECRIO_SIGMGR_NO_ERROR;
	goto Error_Level_01;

Error_Level_01:

	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);
	if (pRespData != NULL)
	{
		pal_MemoryFree((void **)&pRespData);
		pRespData = NULL;
		respLen = 0;
	}

	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* Creates BYE response and sends it to transaction manager.
*
*/
u_int32 _EcrioSigMgrSendByeResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pByeResp,
	BoolEnum isTreminationInProgress
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 respLen = 0;
	EcrioSigMgrCommonInfoStruct cmnInfo = { 0 };
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };
	u_char *pRespData = NULL;
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	error = _EcrioSigMgrPopulateCommonInfo(pSigMgr, pByeResp->pMandatoryHdrs, &cmnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateCommonInfo() for pCmnInfo, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	cmnInfo.responseCode = pByeResp->responseCode;

	eTransportType = pByeResp->pMandatoryHdrs->ppVia[0]->transport;

	error = _EcrioSigMgrSipMessageForm(pSigMgr, EcrioSigMgrSipMessageModeFullName, pByeResp, &pRespData, &respLen);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSipMessageForm() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_INVITE_FAILED;
		goto Error_Level_01;
	}

	cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_BYE;
	cmnInfo.role = EcrioSigMgrRoleUAS;
	cmnInfo.responseCode = pByeResp->responseCode;

	if (isTreminationInProgress == Enum_TRUE)
	{
		txnInfo.currentContext = ECRIO_SIGMGR_TERMINATION_BYE_RESPONSE_SEND;
		txnInfo.retransmitContext = ECRIO_SIGMGR_TERMINATION_BYE_RESPONSE_RESEND;
	}
	else
	{
		txnInfo.currentContext = ECRIO_SIGMGR_BYE_RESPONSE_SEND;
		txnInfo.retransmitContext = ECRIO_SIGMGR_BYE_RESPONSE_RESEND;
	}

	txnInfo.deleteflag = Enum_FALSE;
	txnInfo.messageType = EcrioSigMgrMessageNonInviteResponse;
	txnInfo.transport = eTransportType;
	txnInfo.pMsg = pRespData;
	txnInfo.msglen = respLen;
	txnInfo.pSessionMappingStr = NULL;

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeSIP,
		"%s:%u\tOutgoing SIP message constructed:", __FUNCTION__, __LINE__);
	SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pRespData, respLen);

	error = _EcrioSigMgrSendMsgToTXNHandler(pSigMgr, &cmnInfo, &txnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendMsgToTXNHandler() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_INVITE_FAILED;
		goto Error_Level_01;
	}

	error = ECRIO_SIGMGR_NO_ERROR;
	goto Error_Level_01;

Error_Level_01:

	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);
	if (pRespData != NULL)
	{
		pal_MemoryFree((void **)&pRespData);
		pRespData = NULL;
		respLen = 0;
	}

	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* Creates CANCEL response and sends it to transaction manager.
*
*/
u_int32 _EcrioSigMgrSendCancelResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pCancelResp
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 respLen = 0;
	EcrioSigMgrCommonInfoStruct cmnInfo = { 0 };
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };
	u_char *pRespData = NULL;
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	error = _EcrioSigMgrPopulateCommonInfo(pSigMgr, pCancelResp->pMandatoryHdrs, &cmnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateCommonInfo() for pCmnInfo, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	cmnInfo.responseCode = pCancelResp->responseCode;

	eTransportType = pCancelResp->pMandatoryHdrs->ppVia[0]->transport;

	error = _EcrioSigMgrSipMessageForm(pSigMgr, EcrioSigMgrSipMessageModeFullName, pCancelResp, &pRespData, &respLen);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSipMessageForm() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_INVITE_FAILED;
		goto Error_Level_01;
	}

	cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_CANCEL;
	cmnInfo.role = EcrioSigMgrRoleUAS;
	cmnInfo.responseCode = pCancelResp->responseCode;

	txnInfo.currentContext = ECRIO_SIGMGR_CANCEL_RESPONSE_SEND;
	txnInfo.retransmitContext = ECRIO_SIGMGR_CANCEL_RESPONSE_RESEND;

	txnInfo.deleteflag = Enum_FALSE;
	txnInfo.messageType = EcrioSigMgrMessageNonInviteResponse;
	txnInfo.transport = eTransportType;
	txnInfo.pMsg = pRespData;
	txnInfo.msglen = respLen;
	txnInfo.pSessionMappingStr = NULL;

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeSIP,
		"%s:%u\tOutgoing SIP message constructed:", __FUNCTION__, __LINE__);
	SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pRespData, respLen);

	error = _EcrioSigMgrSendMsgToTXNHandler(pSigMgr, &cmnInfo, &txnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendMsgToTXNHandler() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_INVITE_FAILED;
		goto Error_Level_01;
	}

	error = ECRIO_SIGMGR_NO_ERROR;
	goto Error_Level_01;

Error_Level_01:

	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);
	if (pRespData != NULL)
	{
		pal_MemoryFree((void **)&pRespData);
		pRespData = NULL;
		respLen = 0;
	}

	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* Create invite response message and sends it to transaction manager.
*
*/
u_int32 _EcrioSigMgrCallStateChangeINVPending
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData,
	EcrioSigMgrSipMessageStruct *pInviteResp
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	u_int32	respLen = 0, i = 0;
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };
	u_char *pReqData = NULL;
	BoolEnum deleteMsg = Enum_TRUE;
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;
	EcrioSigMgrSipMessageStruct	tempInviteResp = { .eMethodType = EcrioSipMessageTypeNone };
	u_char *ppRequire[] = { (u_char *)ECRIO_SIG_MGR_PRACK_EXTENSION };
	u_int16	numRequire = 0;
	u_char *pRseqHeaderVal = NULL;
	u_char RseqStr[11] = { 0 };
	u_int32	provRespCode = 0;
	u_int32 uSessionExpireValue = 0;
	EcrioSipMessageTypeEnum eMsgType = EcrioSipMessageTypeNone;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pCmnInfo->pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_INVITE;

	if (pUsageData->pMsg == NULL)
	{
		error = _EcrioSigMgrCopySipMessage(pSigMgr, pInviteResp, &tempInviteResp);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrCopySipMessage() populating invite response, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		if (tempInviteResp.pMandatoryHdrs == NULL)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&tempInviteResp.pMandatoryHdrs);
			if (tempInviteResp.pMandatoryHdrs == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryAllocate() for tempInviteResp.pMandatoryHdrs, error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}

		tempInviteResp.eMethodType = EcrioSipMessageTypeInvite;
		tempInviteResp.eReqRspType = EcrioSigMgrSIPResponse;

		ec_MapGetKeyData(pSigMgr->hHashMap, tempInviteResp.pMandatoryHdrs->pCallId, (void **)&pDialogNode);

		/* Update display name if available */
		if (pSigMgr->pSignalingInfo->pDisplayName != NULL)
		{
			if ((pDialogNode != NULL) && (pDialogNode->pLocalUri != NULL) && (pDialogNode->pLocalUri->nameAddr.pDisplayName == NULL))
			{
				pDialogNode->pLocalUri->nameAddr.pDisplayName = pal_StringCreate(pSigMgr->pSignalingInfo->pDisplayName, 
					pal_StringLength(pSigMgr->pSignalingInfo->pDisplayName));
			}
		}

		error = _EcrioSigMgePopulateHeaderFromDialogAndUpdateState(pSigMgr, &tempInviteResp, pDialogNode);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrGetHeadersFromDialogMgr() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		if (tempInviteResp.pMandatoryHdrs->ppVia == NULL)
		{
			tempInviteResp.pMandatoryHdrs->numVia = pUsageData->pInviteReq->pMandatoryHdrs->numVia;

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected(pUsageData->pInviteReq->pMandatoryHdrs->numVia, 
				sizeof(EcrioSigMgrViaStruct *)) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pal_MemoryAllocate(sizeof(EcrioSigMgrViaStruct *) * (pUsageData->pInviteReq->pMandatoryHdrs->numVia), (void **)&tempInviteResp.pMandatoryHdrs->ppVia);
			if (tempInviteResp.pMandatoryHdrs->ppVia == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryAllocate() for pMandatoryHdrs->ppVia, error=%u",
					__FUNCTION__, __LINE__, error);

				goto Error_Level_01;
			}

			for (i = 0; i < pUsageData->pInviteReq->pMandatoryHdrs->numVia; ++i)
			{
				pal_MemoryAllocate(sizeof(EcrioSigMgrViaStruct), (void **)&tempInviteResp.pMandatoryHdrs->ppVia[i]);
				if (tempInviteResp.pMandatoryHdrs->ppVia[i] == NULL)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tpal_MemoryAllocate() for ppVia, error=%u",
						__FUNCTION__, __LINE__, error);

					goto Error_Level_01;
				}

				error = _EcrioSigMgrPopulateViaHdr(pSigMgr, pUsageData->pInviteReq->pMandatoryHdrs->ppVia[i],
					tempInviteResp.pMandatoryHdrs->ppVia[i]);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrPopulateViaHdr() error=%u",
						__FUNCTION__, __LINE__, error);

					error = ECRIO_SIGMGR_NO_MEMORY;
					goto Error_Level_01;
				}
			}
		}

		/* Create optional header object */
		error = _EcrioSigMgrUpdateMessageRequestStruct(pSigMgr, &tempInviteResp, Enum_FALSE);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tOptional Header Creation error, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		if (pSigMgr->pOOMObject)
		{

			switch (tempInviteResp.responseCode / 100)
			{
				case 1:
				{
					eMsgType = EcrioSipMessageTypeInvite_1xx;
				}
				break;

				case 2:
				{
					eMsgType = EcrioSipMessageTypeInvite_2xx;
				}
				break;

				case 4:
				{
					eMsgType = EcrioSipMessageTypeInvite_4xx;
				}
				break;

				case 6:
				{
					eMsgType = EcrioSipMessageTypeInvite_6xx;
				}
				break;

				default:
				{
				}
				break;
			}

			if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(eMsgType, EcrioSipHeaderTypeP_AccessNetworkInfo))
			{
				error = _EcrioSigMgrAddP_AccessNWInfoHeader(pSigMgr, tempInviteResp.pOptionalHeaderList);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrAddPAccessNWInfoHeader() error=%u",
						__FUNCTION__, __LINE__, error);

					goto Error_Level_01;
				}
			}

			if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(eMsgType, EcrioSipHeaderTypeP_LastAccessNetworkInfo))
			{
				error = _EcrioSigMgrAddP_LastAccessNWInfoHeader(pSigMgr, tempInviteResp.pOptionalHeaderList);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tEcrioSipHeaderTypeP_LastAccessNetworkInfo() error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}
			}

			if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(eMsgType, EcrioSipHeaderTypeP_PreferredService))
			{
				// error = _EcrioSigMgrAddP_PreferredServiceHeader(pSigMgr, tempInviteResp.pOptionalHeaderList);
				error = _EcrioSigMgrAddP_PreferredServiceHeader(pSigMgr, &tempInviteResp);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrAddP_PreferredServiceHeader() error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}
			}
		}

		if (tempInviteResp.responseCode > 100 && tempInviteResp.responseCode < 200)
		{
			if (pUsageData->bPrackEnabled == Enum_TRUE)
			{
				numRequire = (u_int16)(sizeof(ppRequire) / sizeof(u_char *));
				error = _EcrioSigMgrAddOptionalHeader(pSigMgr, tempInviteResp.pOptionalHeaderList, EcrioSipHeaderTypeRequire, numRequire, ppRequire, 0, NULL, NULL);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
						__FUNCTION__, __LINE__, error);

					goto Error_Level_01;
				}

				if (pUsageData->pPrackDetails)
				{
					(pUsageData->pPrackDetails->uLastRseq)++;
					if (0 >= pal_NumToString(pUsageData->pPrackDetails->uLastRseq, RseqStr, 11))
					{
						SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
							"%s:%u\tString Copy error",
							__FUNCTION__, __LINE__);

						goto Error_Level_01;
					}
					pRseqHeaderVal = RseqStr;

					error = _EcrioSigMgrAddOptionalHeader(pSigMgr, tempInviteResp.pOptionalHeaderList, EcrioSipHeaderTypeRSeq, 1, &pRseqHeaderVal, 0, NULL, NULL);
					if (error != ECRIO_SIGMGR_NO_ERROR)
					{
						SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
							"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
							__FUNCTION__, __LINE__, error);

						goto Error_Level_01;
					}

					/* Check arithmetic overflow */
					if (pal_UtilityArithmeticOverflowDetected(pUsageData->pPrackDetails->uRSeqArrayCount, 1) == Enum_TRUE)
					{
						error = ECRIO_SIGMGR_NO_MEMORY;
						goto Error_Level_01;
					}

					/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
					if (pal_UtilityDataOverflowDetected((pUsageData->pPrackDetails->uRSeqArrayCount + 1), sizeof(_EcrioSigMgrRSeqStruct)) == Enum_TRUE)
					{
						error = ECRIO_SIGMGR_NO_MEMORY;
						goto Error_Level_01;
					}

					pal_MemoryReallocate((pUsageData->pPrackDetails->uRSeqArrayCount + 1) * sizeof(_EcrioSigMgrRSeqStruct), (void **)&pUsageData->pPrackDetails->pRseqArray);
					if (pUsageData->pPrackDetails->pRseqArray == NULL)
					{
						error = ECRIO_SIGMGR_NO_MEMORY;
						goto Error_Level_01;
					}
					pUsageData->pPrackDetails->pRseqArray[pUsageData->pPrackDetails->uRSeqArrayCount].uRseq = pUsageData->pPrackDetails->uLastRseq;
					pUsageData->pPrackDetails->pRseqArray[pUsageData->pPrackDetails->uRSeqArrayCount].bAcknowledged = Enum_FALSE;

					(pUsageData->pPrackDetails->uRSeqArrayCount)++;
				}
			}
		}
		else if (tempInviteResp.responseCode >= 200 && tempInviteResp.responseCode < 300)
		{
			if (pSigMgr->pOOMObject)
			{
				if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeInvite_2xx, EcrioSipHeaderTypeSessionExpires))
				{
					/* Use incoming session expire value if its greater than minSe. Otherwise fallback on
					local session expire value. */
					if (pUsageData->sessionExpireVal > 0 && pUsageData->sessionExpireVal > pSigMgr->pSignalingInfo->minSe)
					{
						uSessionExpireValue = pUsageData->sessionExpireVal;
					}
					else
					{
						uSessionExpireValue = pSigMgr->pSignalingInfo->sessionExpires;
					}

					error = _EcrioSigMgrAddSessionExpiresHeader(pSigMgr, tempInviteResp.pOptionalHeaderList, pUsageData->isSessionRefresh, pUsageData->isRefresher, uSessionExpireValue);
					if (error != ECRIO_SIGMGR_NO_ERROR)
					{
						SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
							"%s:%u\t_EcrioSigMgrAddSessionExpiresHeader() error=%u",
							__FUNCTION__, __LINE__, error);
						goto Error_Level_01;
					}

					if (tempInviteResp.ppPPreferredIdentity == NULL)
					{
						//add P-PreferredIdentity header 
						error = _EcrioSigMgrAddP_PreferrdIdHeader(pSigMgr, &tempInviteResp);
						if (error != ECRIO_SIGMGR_NO_ERROR)
						{
							SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t_EcrioSigMgrAddP_preferredIdHeader() error=%u",
								__FUNCTION__, __LINE__, error);
							goto Error_Level_01;
						}

						if (pSigMgr->pOOMObject->ec_oom_HasUserPhoneSupport(EcrioSipMessageTypeInvite, EcrioSipHeaderTypeP_PreferredIdentity, EcrioSipURIType_SIP) == Enum_TRUE)
						{
							/* All outgoing SIP uri which contain telephone number in user info part must have "user=phone" parameter as SIP uri parameter.	*/
							error = _EcrioSigMgrAddUserPhoneParam(pSigMgr, &tempInviteResp.ppPPreferredIdentity[0]->addrSpec);
							if (error != ECRIO_SIGMGR_NO_ERROR)
							{
								SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t_EcrioSigMgrAddUserPhoneParam, error=%u", 
									__FUNCTION__, __LINE__, error);
								goto Error_Level_01;
							}
						}
					}

					_EcrioSigMgrStartSessionRefreshExpireTimer(pSigMgr, pUsageData,
						pDialogNode, uSessionExpireValue, pUsageData->isRefresher);					
				}
			}
#ifdef ENABLE_RCS
			//adding Conversation and Contribution Ids to Optional header .
			if (tempInviteResp.pConvId)
			{
				error = _EcrioSigMgrAddConversationsIDHeader(pSigMgr, &tempInviteResp);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrAddALlowHeader() error=%u",
						__FUNCTION__, __LINE__, error);

					goto Error_Level_01;
				}
			}
#endif //ENABLE_RCS
			if (pUsageData->isSessionRefresh == Enum_TRUE)
			{
				u_char *pRequire = (u_char *)ECRIO_SIG_MGR_SESSION_TIMER_EXTENSION;
				/*Add timer tag in Require header*/
				error = _EcrioSigMgrAddOptionalHeader(pSigMgr, tempInviteResp.pOptionalHeaderList, EcrioSipHeaderTypeRequire, 1, &pRequire, 0, NULL, NULL);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
						__FUNCTION__, __LINE__, error);

					goto Error_Level_01;
				}
			}
			//pInviteResp->pMessageBody
			//pUsageData->pInviteReq->pMessageBody
			/* Overwrite SDP offer with SDP answer. Book keep and use this SDP for session refresh and sesison update */
			if (pUsageData->pInviteReq->pMessageBody != NULL)
			{
				_EcrioSigMgrReleaseMsgBodyStruct(pSigMgr, pUsageData->pInviteReq->pMessageBody);
			}
			else
			{
				pal_MemoryAllocate(sizeof(EcrioSigMgrMessageBodyStruct),
					(void **)&pUsageData->pInviteReq->pMessageBody);
				if (pUsageData->pInviteReq->pMessageBody == NULL)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tpal_MemoryAllocate() for pUsageData->pInviteReq->pMessageBody, error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}
			}
			_EcrioSigMgrCopySipMessageBody(pSigMgr, pInviteResp->pMessageBody, pUsageData->pInviteReq->pMessageBody);
		}
		else if (tempInviteResp.responseCode == 421)
		{
			if (pInviteResp->pOptionalHeaderList != NULL)
				tempInviteResp.pOptionalHeaderList = pInviteResp->pOptionalHeaderList;

			pInviteResp->pOptionalHeaderList = NULL;
		}

		if (tempInviteResp.pContact == NULL)
		{
			error = _EcrioSigMgrAddLocalContactUri(pSigMgr,
				&tempInviteResp.pContact, tempInviteResp.bPrivacy);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddLocalContactUri() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}

		eTransportType = tempInviteResp.pMandatoryHdrs->ppVia[0]->transport;

		error = _EcrioSigMgrSipMessageForm(pSigMgr, EcrioSigMgrSipMessageModeFullName, &tempInviteResp, &pReqData, &respLen);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrCreateInviteResponse() error=%u",
				__FUNCTION__, __LINE__, error);
			error = ECRIO_SIGMGR_INVITE_FAILED;
			goto Error_Level_01;
		}

		pCmnInfo->role = EcrioSigMgrRoleUAS;
		error = _EcrioSigMgrPopulateCommonInfo(pSigMgr, tempInviteResp.pMandatoryHdrs, pCmnInfo);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrPopulateCommonInfo() error=%u",
				__FUNCTION__, __LINE__, error);
			error = ECRIO_SIGMGR_INVITE_FAILED;
			goto Error_Level_01;
		}
	}
	else
	{
		provRespCode = _EcrioSigMgrExtractResponseCode(pUsageData);
		pReqData = pUsageData->pMsg;
		respLen = pUsageData->msgLength;

		if (pUsageData->callState == EcrioSigMgrCallStateSetupInProgress)
		{
			pCmnInfo->responseCode = provRespCode;
		}
		else if (pUsageData->callState == EcrioSigMgrCallStateAcceptedAckPending ||
			pUsageData->callState == EcrioSigMgrCallStateReInviteAcceptedAckPending)
		{
			pCmnInfo->responseCode = ECRIO_SIGMGR_RESPONSE_CODE_OK;
		}

		pCmnInfo->role = EcrioSigMgrRoleUAS;
		_EcrioSigMgrStringCreate(pSigMgr, pUsageData->pInviteReq->pMandatoryHdrs->ppVia[0]->pBranch, &pCmnInfo->pBranch);
		deleteMsg = Enum_FALSE;

		SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tGet remote port from pUsageData->ppViaHdrs", __FUNCTION__, __LINE__);
	}

	txnInfo.currentContext = ECRIO_SIGMGR_INVITE_RESPONSE_SEND;
	if ((pUsageData->pMsg != NULL) && (pCmnInfo->responseCode == ECRIO_SIGMGR_RESPONSE_CODE_OK))
	{
		txnInfo.retransmitContext = ECRIO_SIGMGR_INVALID_CONTEXT;

		if (eTransportType == EcrioSigMgrTransportUDP)
		{
			txnInfo.uReceivedChannelIndex = pSigMgr->pSigMgrTransportStruct->sendingChannelIndex;
		}
		else if (eTransportType == EcrioSigMgrTransportTCP)
		{
			txnInfo.uReceivedChannelIndex = pSigMgr->pSigMgrTransportStruct->sendingTCPChannelIndex;
		}
		else if (eTransportType == EcrioSigMgrTransportTLS)
		{
			txnInfo.uReceivedChannelIndex = pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex;
		}
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tOn retransmitted 200 OK, INVITE passing in TxnInfo txnInfo.uReceivedChannelIndex:%d", __FUNCTION__, __LINE__, txnInfo.uReceivedChannelIndex);
	}
	else
	{
		txnInfo.retransmitContext = ECRIO_SIGMGR_INVITE_RESPONSE_RESEND;
	}

	txnInfo.deleteflag = Enum_FALSE;
	txnInfo.messageType = EcrioSigMgrMessageInviteResponse;
	txnInfo.transport = eTransportType;
	txnInfo.pMsg = pReqData;
	txnInfo.msglen = respLen;
	txnInfo.pSessionMappingStr = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeSIP,
		"%s:%u\t eTransportType:%d", __FUNCTION__, __LINE__, eTransportType);

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeSIP,
		"%s:%u\tOutgoing SIP message constructed:", __FUNCTION__, __LINE__);
	SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pReqData, respLen);

	error = _EcrioSigMgrSendMsgToTXNHandler(pSigMgr, pCmnInfo, &txnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendMsgToTXNHandler() error=%u",
			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_INVITE_FAILED;
		goto Error_Level_01;
	}

	/* keep reference of response message
	*/
	if (NULL == pUsageData->pMsg)
	{
		pUsageData->pMsg = pReqData;
		pUsageData->msgLength = respLen;
		pReqData = NULL;
	}

	if (txnInfo.bSendDirect)
	{
		deleteMsg = Enum_FALSE;
	}

Error_Level_01:
	if ((deleteMsg == Enum_TRUE) && (pReqData != NULL))
	{
		pal_MemoryFree((void **)&pReqData);
		pReqData = NULL;
	}

	if (tempInviteResp.pReasonPhrase != NULL)
	{
		pal_MemoryFree((void **)&tempInviteResp.pReasonPhrase);
	}

	tempInviteResp.pFetaureTags = NULL;
	_EcrioSigMgrReleaseSipMessage(pSigMgr, &tempInviteResp);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* Generate PRACK response structure. It populates headers from dialog.
*
*/
u_int32 _EcrioSigMgrGeneratePrackResp
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pPrackReq,
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData,
	u_char *pReasonPhrase,
	u_int32 responseCode
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrSipMessageStruct	prackResp = { .eMethodType = EcrioSipMessageTypeNone };
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	(void)pUsageData;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&prackResp.pMandatoryHdrs);
	if (prackResp.pMandatoryHdrs == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;

		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tECRIO_ALLOCATE_MEM() for byeResp.pMandatoryHdrs, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}

	error = _EcrioSigMgrPopulateManHdrs(pSigMgr, pPrackReq->pMandatoryHdrs,
		prackResp.pMandatoryHdrs);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateManHdrs() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}

	prackResp.responseCode = responseCode;
	prackResp.pReasonPhrase = pReasonPhrase;
	prackResp.eMethodType = EcrioSipMessageTypePrack;
	prackResp.eReqRspType = EcrioSigMgrSIPResponse;

	ec_MapGetKeyData(pSigMgr->hHashMap, pPrackReq->pMandatoryHdrs->pCallId, (void **)&pDialogNode);
	error = _EcrioSigMgePopulateHeaderFromDialogAndUpdateState(pSigMgr, &prackResp, pDialogNode);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgePopulateHeaderFromDialogAndUpdateState() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}

	error = _EcrioSigMgrSendPrackResponse(pSigMgr, &prackResp);
	
Error_Level_02:
	prackResp.pReasonPhrase = NULL;
	_EcrioSigMgrReleaseSipMessage(pSigMgr, &prackResp);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* Generate BYE response structure. It populates headers from dialog.
*
*/
u_int32 _EcrioSigMgrGenerateByeResp
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pByeReq,
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData,
	u_char *pReasonPhrase,
	u_int32 responseCode
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrSipMessageStruct	byeResp = { .eMethodType = EcrioSipMessageTypeNone };
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&byeResp.pMandatoryHdrs);
	if (byeResp.pMandatoryHdrs == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;

		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for byeResp.pMandatoryHdrs, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}

	error = _EcrioSigMgrPopulateManHdrs(pSigMgr, pByeReq->pMandatoryHdrs,
		byeResp.pMandatoryHdrs);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateManHdrs() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}

	byeResp.responseCode = responseCode;
	byeResp.pReasonPhrase = pReasonPhrase;
	byeResp.eMethodType = EcrioSipMessageTypeBye;
	byeResp.eReqRspType = EcrioSigMgrSIPResponse;

	ec_MapGetKeyData(pSigMgr->hHashMap, pByeReq->pMandatoryHdrs->pCallId, (void **)&pDialogNode);

	error = _EcrioSigMgePopulateHeaderFromDialogAndUpdateState(pSigMgr, &byeResp, pDialogNode);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgePopulateHeaderFromDialogAndUpdateState() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}

	if (pUsageData && pUsageData->callState == EcrioSigMgrCallStateTerminationInProgress)
	{
		error = _EcrioSigMgrSendByeResponse(pSigMgr, &byeResp, Enum_TRUE);
	}
	else
	{
		error = _EcrioSigMgrSendByeResponse(pSigMgr, &byeResp, Enum_FALSE);
	}

	/* End - Resolving BYE Issue - BUG#7427 */

Error_Level_02:
	byeResp.pReasonPhrase = NULL;
	_EcrioSigMgrReleaseSipMessage(pSigMgr, &byeResp);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* Create 4XX invite response and send it to transaction manager.
*/
u_int32 _EcrioSigMgrSendInviteErrorResponse
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData,
	EcrioSigMgrSipMessageStruct *pInviteReq,
	u_char *pReasonPhrase,
	u_int32 reasonCode
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR, noVia = 0;
	EcrioSigMgrSipMessageStruct inviteResp = { .eMethodType = EcrioSipMessageTypeNone };
	EcrioSigMgrCommonInfoStruct cmnInfo = { 0 };
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };
	EcrioSigMgrViaStruct **ppVia = NULL;
	u_char *pRespData = NULL;
	u_int32 respLen = 0;
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpSigMgr=%p, pInviteReq=%p, reasonCode=%u",
		__FUNCTION__, __LINE__, pSigMgr, pInviteReq, reasonCode);

	inviteResp.responseCode = reasonCode;
	inviteResp.pReasonPhrase = pReasonPhrase;

	if (inviteResp.pContact == NULL)
	{
		error = _EcrioSigMgrAddLocalContactUri(pSigMgr,
			&inviteResp.pContact, inviteResp.bPrivacy);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddLocalContactUri() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&inviteResp.pMandatoryHdrs);
	if (inviteResp.pMandatoryHdrs == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;

		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for inviteResp.pResponseHdrs->pMandatoryHdrs, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	error = _EcrioSigMgrPopulateManHdrs(pSigMgr, pInviteReq->pMandatoryHdrs,
		inviteResp.pMandatoryHdrs);

	if (pUsageData != NULL)
	{
		_EcrioSigMgrSendInviteResponse(pSigMgr, &inviteResp);
	}
	else
	{
		cmnInfo.role = EcrioSigMgrRoleUAS;
		cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_INVITE;
		ppVia = pInviteReq->pMandatoryHdrs->ppVia;
		noVia = pInviteReq->pMandatoryHdrs->numVia;
		_EcrioSigMgrStringCreate(pSigMgr, ppVia[noVia - 1]->pBranch, &cmnInfo.pBranch);
		eTransportType = inviteResp.pMandatoryHdrs->ppVia[0]->transport;
		error = _EcrioSigMgrSipMessageForm(pSigMgr, EcrioSigMgrSipMessageModeFullName, &inviteResp, &pRespData, &respLen);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrCreateInviteResponse() error=%u",
				__FUNCTION__, __LINE__, error);
			error = ECRIO_SIGMGR_INVITE_FAILED;
			goto Error_Level_01;
		}

		txnInfo.currentContext = ECRIO_SIGMGR_INVITE_RESPONSE_SEND;
		txnInfo.retransmitContext = ECRIO_SIGMGR_INVITE_RESPONSE_RESEND;
		txnInfo.deleteflag = Enum_FALSE;
		txnInfo.messageType = EcrioSigMgrMessageInviteResponse;
		txnInfo.transport = eTransportType;
		txnInfo.pMsg = pRespData;
		txnInfo.msglen = respLen;
		txnInfo.pSessionMappingStr = NULL;

		SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeSIP,
			"%s:%u\tOutgoing SIP message constructed:", __FUNCTION__, __LINE__);
		SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pRespData, respLen);

		error = _EcrioSigMgrSendMsgToTXNHandler(pSigMgr, &cmnInfo, &txnInfo);

		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrSendMsgToTXNHandler() error=%u",
				__FUNCTION__, __LINE__, error);
			error = ECRIO_SIGMGR_INVITE_FAILED;
			goto Error_Level_01;
		}
	}

Error_Level_01:
	if (pRespData != NULL)
	{
		pal_MemoryFree((void **)&pRespData);
		pRespData = NULL;
		respLen = 0;
	}

	inviteResp.pReasonPhrase = NULL;
	inviteResp.responseCode = ECRIO_SIGMGR_INVALID_RESPONSE_CODE;
	_EcrioSigMgrReleaseSipMessage(pSigMgr, &inviteResp);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrHandleCancelRequest
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pCancelReq,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	u_int32 responseCode,
	u_char *pReasonPhrase
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrMessageStruct message = { .msgCmd = EcrioSigMgrMessageNone };
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	BoolEnum bIsForked = Enum_FALSE;
	u_int32 respCode = 0;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pCmnInfo->pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_CANCEL;
	pCmnInfo->role = EcrioSigMgrRoleUAS;

	pDialogNode = EcrioSigMgrMatchDialogAndUpdateState(pSigMgr, pCancelReq, &bIsForked, &respCode);
	if (pDialogNode == NULL)
	{
		error = ECRIO_SIGMGR_BAD_DIALOGMGR_HANDLE;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tEcrioSigMgrMatchDialogAndUpdateState() error=%u", __FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;

	if (pInviteUsage->callState == EcrioSigMgrCallStateSetupInProgress)
	{
		if (pCancelReq->isRequestInValid == Enum_FALSE)
		{
			responseCode = ECRIO_SIGMGR_RESPONSE_CODE_OK;
			pReasonPhrase = (u_char *)ECRIO_SIGMGR_RESPONSE_OK;

			if (pInviteUsage->pMsg != NULL)
			{
				pal_MemoryFree((void **)&pInviteUsage->pMsg);
				pInviteUsage->msgLength = 0;
			}
		}

		error = _EcrioSigMgrGenerateCancelResp(pSigMgr, pCancelReq, pInviteUsage,
			pReasonPhrase, responseCode);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrGenerateCancelResp() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		error = _EcrioSigMgrSendInviteErrorResponse(pSigMgr, pInviteUsage, pInviteUsage->pInviteReq,
			ECRIO_SIGMGR_RESPONSE_REQUEST_TERMINATED, ECRIO_SIGMGR_RESPONSE_CODE_REQUEST_TERMINATED);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t_EcrioSigMgrSendInviteErrorResponse() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		message.msgCmd = EcrioSigMgrCancelRequestNotification;
		message.pData = (void *)pCancelReq;
#if 0
		{
			EcrioSigMgrCallbackRegisteringModuleEnums eModuleId = EcrioSigMgrCallbackRegisteringModule_LIMS;

			eModuleId = _EcrioSigMgrFindModuleId(pSigMgr, pCancelReq);
			_EcrioSigMgrStoreModuleRoutingInfo(pSigMgr, pCancelReq->pMandatoryHdrs->pCallId, eModuleId);

			if (eModuleId == EcrioSigMgrCallbackRegisteringModule_CPM)
			{
				_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
			}
			else
			{
				_EcrioSigMgrUpdateStatusMoIP(pSigMgr, &message);
			}
		}
#else
		if (_EcrioSigMgrGetModuleId(pSigMgr, pCmnInfo->pCallId) == EcrioSigMgrCallbackRegisteringModule_CPM)
		{
			_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
		}
		else
		{
			_EcrioSigMgrUpdateStatusMoIP(pSigMgr, &message);
		}
#endif
	}
	else if ((pInviteUsage->callState == EcrioSigMgrCallStateCallEstablished) ||
		(pInviteUsage->callState == EcrioSigMgrCallStateRejectedAckPending) ||
		(pInviteUsage->callState == EcrioSigMgrCallStateAcceptedAckPending))
	{
		error = _EcrioSigMgrGenerateCancelResp(pSigMgr, pCancelReq, pInviteUsage,
			(u_char *)ECRIO_SIGMGR_RESPONSE_CALL_TXN_NOT_EXIST, ECRIO_SIGMGR_RESPONSE_CODE_CALL_TXN_NOT_EXIST);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t_EcrioSigMgrGenerateCancelResp() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}
	else
	{
		goto Error_Level_01;
	}

Error_Level_01:

	if ((pDialogNode != NULL) && (pInviteUsage->callState == EcrioSigMgrCallStateUnInitialized))
	{
		/*message.msgCmd = EcrioSigMgrDeleteSessionNotification;
		message.pData = (void*)pSessionHandle;

		#ifdef _ENABLE_DEFAULT_CALL_HANDLER_CPM_
		_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
		#else
		_EcrioSigMgrUpdateStatusMoIP(pSigMgr, &message);
		#endif
		*/

		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tDialog deleted", __FUNCTION__, __LINE__);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* Generate BYE response structure. It populates headers from dialog.
*
*/
u_int32 _EcrioSigMgrGenerateCancelResp
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pCancelReq,
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData,
	u_char *pReasonPhrase,
	u_int32 responseCode
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrSipMessageStruct	cancelResp = { .eMethodType = EcrioSipMessageTypeNone };
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;

	// @todo Is pUsageData needed by this function?
	(void)pUsageData;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&cancelResp.pMandatoryHdrs);
	if (cancelResp.pMandatoryHdrs == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;

		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for byeResp.pMandatoryHdrs, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}

	error = _EcrioSigMgrPopulateManHdrs(pSigMgr, pCancelReq->pMandatoryHdrs,
		cancelResp.pMandatoryHdrs);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateManHdrs() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}

	cancelResp.responseCode = responseCode;
	cancelResp.pReasonPhrase = pReasonPhrase;
	cancelResp.eMethodType = EcrioSipMessageTypeCancel;
	cancelResp.eReqRspType = EcrioSigMgrSIPResponse;

	ec_MapGetKeyData(pSigMgr->hHashMap, pCancelReq->pMandatoryHdrs->pCallId, (void **)&pDialogNode);
	error = _EcrioSigMgePopulateHeaderFromDialogAndUpdateState(pSigMgr, &cancelResp, pDialogNode);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgePopulateHeaderFromDialogAndUpdateState() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}

	error = _EcrioSigMgrSendCancelResponse(pSigMgr, &cancelResp);

Error_Level_02:
	cancelResp.pReasonPhrase = NULL;
	_EcrioSigMgrReleaseSipMessage(pSigMgr, &cancelResp);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* It posts send2xx to state machine handler for valid request. Otherwise, it
* determines error response code.
*
*/
u_int32 _EcrioSigMgrHandleUpdateRequest
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pUpdateReq,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	u_int32 responseCode,
	u_char *pReasonPhrase
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrMessageStruct message = { .msgCmd = EcrioSigMgrMessageNone };
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	BoolEnum bIsForked = Enum_FALSE;
	u_int32 respCode = 0;
	u_int32 sessionInterval = 0;
	u_int32 i = 0;
	EcrioSigMgrHeaderStruct *pSessionExpireHeader = NULL;
	BoolEnum bSend422 = Enum_FALSE;
 

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pCmnInfo->pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_UPDATE;
	pCmnInfo->role = EcrioSigMgrRoleUAS;

	pDialogNode = EcrioSigMgrMatchDialogAndUpdateState(pSigMgr, pUpdateReq, &bIsForked, &respCode);
	if (pDialogNode == NULL)
	{
		error = ECRIO_SIGMGR_BAD_DIALOGMGR_HANDLE;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tEcrioSigMgrMatchDialogAndUpdateState() error=%u", __FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}
	pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;

	/* check if 422 needs to be sent */	
	if (pSigMgr->pOOMObject)
	{
		if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeUpdate, EcrioSipHeaderTypeSessionExpires))
		{
			EcrioSigMgrGetOptionalHeader(pSigMgr, pUpdateReq->pOptionalHeaderList, EcrioSipHeaderTypeSessionExpires, &pSessionExpireHeader);
			if (pSessionExpireHeader != NULL)
			{
				for (i = 0; i < pSessionExpireHeader->numHeaderValues; i++)
				{
					if (pSessionExpireHeader->ppHeaderValues && pSessionExpireHeader->ppHeaderValues[i])
					{
						if (pSessionExpireHeader->ppHeaderValues[i]->pHeaderValue)
						{
							sessionInterval = pal_StringConvertToUNum(pSessionExpireHeader->ppHeaderValues[i]->pHeaderValue, NULL, 10);
							if (sessionInterval < pInviteUsage->minSEVal)
							{
								bSend422 = Enum_TRUE;
								break;
							}
						}
					}
				}
			}
		}
	}

	/* Create response code */
	if (bSend422 == Enum_TRUE)
	{
		pUpdateReq->responseCode = ECRIO_SIGMGR_RESPONSE_CODE_SESSION_INTERVAL_SMALL;
		pUpdateReq->pReasonPhrase = pal_StringCreate(ECRIO_SIGMGR_RESPONSE_INTERVAL_TOO_BRIEF,
							pal_StringLength(ECRIO_SIGMGR_RESPONSE_INTERVAL_TOO_BRIEF));
	}
	else
	{
		if (responseCode == 0)
		{
			pUpdateReq->responseCode = ECRIO_SIGMGR_RESPONSE_CODE_OK;
			pUpdateReq->pReasonPhrase = pal_StringCreate(ECRIO_SIGMGR_RESPONSE_OK,
				pal_StringLength(ECRIO_SIGMGR_RESPONSE_OK));
		}
		else
		{
			pUpdateReq->responseCode = responseCode;
			pUpdateReq->pReasonPhrase = pal_StringCreate(pReasonPhrase,
				pal_StringLength(pReasonPhrase));
		}
	}

	if (pInviteUsage->callState == EcrioSigMgrCallStateCallEstablished)
	{
		message.msgCmd = EcrioSigMgrUpdateRequestNotification;
		message.pData = (void *)pUpdateReq;

		if (_EcrioSigMgrGetModuleId(pSigMgr, pCmnInfo->pCallId) == EcrioSigMgrCallbackRegisteringModule_CPM)
		{
			_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
		}
		else
		{
			_EcrioSigMgrUpdateStatusMoIP(pSigMgr, &message);
		}
	}
	else if ( pInviteUsage->callState == EcrioSigMgrCallStateSetupInProgress)
	{
		message.msgCmd = EcrioSigMgrUpdateRequestNotification;
		message.pData = (void *)pUpdateReq;
		if (_EcrioSigMgrGetModuleId(pSigMgr, pCmnInfo->pCallId) == EcrioSigMgrCallbackRegisteringModule_CPM)
		{
			_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
		}
		else
		{
			_EcrioSigMgrUpdateStatusMoIP(pSigMgr, &message);
		}
	}
	else
	{
		goto Error_Level_01;
	}
Error_Level_01:

	if (pUpdateReq->pReasonPhrase != NULL)
	{
		pal_MemoryFree((void**)&pUpdateReq->pReasonPhrase);
	}

	if ((pDialogNode != NULL) && (pInviteUsage->callState == EcrioSigMgrCallStateUnInitialized))
	{
		_EcrioSigMgrDeleteDialog(pSigMgr, pDialogNode);

		_EcrioSigMgrInternalStructRelease(pSigMgr, EcrioSigMgrStructType_InviteUsageInfo,
			(void **)&pInviteUsage, Enum_TRUE);

		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tDialog deleted", __FUNCTION__, __LINE__);
	}
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* Generate UPDATE response structure. It populates headers from dialog.
*
*/
u_int32 _EcrioSigMgrGenerateUpdateResp
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pUpdateReq,
	u_char *pReasonPhrase,
	u_int32 responseCode
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 uSessionExpireValue = 0;
	EcrioSigMgrSipMessageStruct	updateResp = { .eMethodType = EcrioSipMessageTypeNone };
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	u_int32 uMinSEValue = 0;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&updateResp.pMandatoryHdrs);
	if (updateResp.pMandatoryHdrs == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;

		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for updateResp.pMandatoryHdrs, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}

	error = _EcrioSigMgrPopulateManHdrs(pSigMgr, pUpdateReq->pMandatoryHdrs,
		updateResp.pMandatoryHdrs);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateManHdrs() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}

	updateResp.responseCode = responseCode;
	updateResp.pReasonPhrase = pReasonPhrase;
	updateResp.eMethodType = EcrioSipMessageTypeUpdate;
	updateResp.eReqRspType = EcrioSigMgrSIPResponse;

	ec_MapGetKeyData(pSigMgr->hHashMap, pUpdateReq->pMandatoryHdrs->pCallId, (void **)&pDialogNode);
	error = _EcrioSigMgePopulateHeaderFromDialogAndUpdateState(pSigMgr, &updateResp, pDialogNode);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgePopulateHeaderFromDialogAndUpdateState() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}

	pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
	if (pInviteUsage == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpInviteUsage is NULL cannot proceed further",
			__FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	/* Use incoming session expire value if its greater than minSe. Otherwise fallback on
	local session expire value. */
	if (pInviteUsage->sessionExpireVal > 0 && pInviteUsage->sessionExpireVal > pSigMgr->pSignalingInfo->minSe)
	{
		uSessionExpireValue = pInviteUsage->sessionExpireVal;
	}
	else
	{
		uSessionExpireValue = pSigMgr->pSignalingInfo->sessionExpires;
	}

	if (pSigMgr->pOOMObject)
	{
		switch (responseCode)
		{
			case ECRIO_SIGMGR_RESPONSE_CODE_OK:
			{
				if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeUpdate_2xx, EcrioSipHeaderTypeSessionExpires))
				{
					/* Session Expires Header*/
					if (updateResp.pOptionalHeaderList == NULL)
					{
						error = EcrioSigMgrCreateOptionalHeaderList(pSigMgr, &updateResp.pOptionalHeaderList);
						if (error != ECRIO_SIGMGR_NO_ERROR)
						{
							SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
								"%s:%u\tEcrioSigMgrCreateOptionalHeaderList() error=%u",
								__FUNCTION__, __LINE__, error);

							error = ECRIO_SIGMGR_NO_MEMORY;
							goto Error_Level_02;
						}
					}
					error = _EcrioSigMgrAddSessionExpiresHeader(pSigMgr, updateResp.pOptionalHeaderList, pInviteUsage->isSessionRefresh, pInviteUsage->isRefresher, uSessionExpireValue);
					if (error != ECRIO_SIGMGR_NO_ERROR)
					{
						SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
							"%s:%u\t__EcrioSigMgrAddSessionExpiresHeader() error=%u",
							__FUNCTION__, __LINE__, error);
						goto Error_Level_02;
					}
				}
				_EcrioSigMgrStartSessionRefreshExpireTimer(pSigMgr, pInviteUsage,
					pDialogNode, uSessionExpireValue, pInviteUsage->isRefresher);
			}
			break;
			case ECRIO_SIGMGR_RESPONSE_CODE_SESSION_INTERVAL_SMALL:
			{
				if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeUpdate_4xx, EcrioSipHeaderTypeSessionExpires))
				{
					/* MIn-SE Header*/
					if (updateResp.pOptionalHeaderList == NULL)
					{
						error = EcrioSigMgrCreateOptionalHeaderList(pSigMgr, &updateResp.pOptionalHeaderList);
						if (error != ECRIO_SIGMGR_NO_ERROR)
						{
							SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
								"%s:%u\tEcrioSigMgrCreateOptionalHeaderList() error=%u",
								__FUNCTION__, __LINE__, error);

							error = ECRIO_SIGMGR_NO_MEMORY;
							goto Error_Level_02;
						}
					}
					error = _EcrioSigMgrAddSessionExpiresHeader(pSigMgr, updateResp.pOptionalHeaderList, pInviteUsage->isSessionRefresh, pInviteUsage->isRefresher, uSessionExpireValue);
					if (error != ECRIO_SIGMGR_NO_ERROR)
					{
						SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
							"%s:%u\t__EcrioSigMgrAddSessionExpiresHeader() error=%u",
							__FUNCTION__, __LINE__, error);
						goto Error_Level_02;
					}
				}
				if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeUpdate_4xx, EcrioSipHeaderTypeMinSE))
				{
					if (pInviteUsage->minSEVal > 0)
					{
						uMinSEValue = pInviteUsage->minSEVal;
					}
					else
					{
						uMinSEValue = pSigMgr->pSignalingInfo->minSe;
					}

					if (uMinSEValue != 0)
					{
						error = _EcrioSigMgrAddMinSEHeader(pSigMgr, updateResp.pOptionalHeaderList, uMinSEValue);
						if (error != ECRIO_SIGMGR_NO_ERROR)
						{
							goto Error_Level_01;
						}
					}
				}
			}
			break;
			default:
			{
				_EcrioSigMgrStartSessionRefreshExpireTimer(pSigMgr, pInviteUsage,
					pDialogNode, uSessionExpireValue, pInviteUsage->isRefresher);
			}
		}
	}

	if (pUpdateReq->pMessageBody != NULL)
		updateResp.pMessageBody = pUpdateReq->pMessageBody;

	/*if(pInviteUsage->callState == EcrioSigMgrCallStateSetupInProgress)
	{
		updateResp.pMessageBody =  NULL;
	}*/

	error = _EcrioSigMgrSendUpdateResponse(pSigMgr, &updateResp);

Error_Level_02:
	updateResp.pMessageBody = NULL;
	updateResp.pReasonPhrase = NULL;
	_EcrioSigMgrReleaseSipMessage(pSigMgr, &updateResp);

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* Creates UPDATE response and sends it to transaction manager.
*
*/
u_int32 _EcrioSigMgrSendUpdateResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pUpdateResp
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 respLen = 0;
	EcrioSigMgrCommonInfoStruct cmnInfo = { 0 };
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };
	u_char *pRespData = NULL;
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	error = _EcrioSigMgrPopulateCommonInfo(pSigMgr, pUpdateResp->pMandatoryHdrs, &cmnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateCommonInfo() for pCmnInfo, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	cmnInfo.responseCode = pUpdateResp->responseCode;

	eTransportType = pUpdateResp->pMandatoryHdrs->ppVia[0]->transport;

	error = _EcrioSigMgrSipMessageForm(pSigMgr, EcrioSigMgrSipMessageModeFullName, pUpdateResp, &pRespData, &respLen);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSipMessageForm() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_UPDATE_FAILURE;
		goto Error_Level_01;
	}

	cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_UPDATE;
	cmnInfo.role = EcrioSigMgrRoleUAS;
	cmnInfo.responseCode = pUpdateResp->responseCode;

	txnInfo.currentContext = ECRIO_SIGMGR_UPDATE_RESPONSE_SEND;
	txnInfo.retransmitContext = ECRIO_SIGMGR_UPDATE_RESPONSE_RESEND;

	txnInfo.deleteflag = Enum_FALSE;
	txnInfo.messageType = EcrioSigMgrMessageNonInviteResponse;
	txnInfo.transport = eTransportType;
	txnInfo.pMsg = pRespData;
	txnInfo.msglen = respLen;
	txnInfo.pSessionMappingStr = NULL;

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeSIP,
		"%s:%u\tOutgoing SIP message constructed:", __FUNCTION__, __LINE__);
	SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pRespData, respLen);

	error = _EcrioSigMgrSendMsgToTXNHandler(pSigMgr, &cmnInfo, &txnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendMsgToTXNHandler() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_UPDATE_FAILURE;
		goto Error_Level_01;
	}

	error = ECRIO_SIGMGR_NO_ERROR;
	goto Error_Level_01;

Error_Level_01:

	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);
	if (pRespData != NULL)
	{
		pal_MemoryFree((void **)&pRespData);
		pRespData = NULL;
		respLen = 0;
	}

	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* Transaction Timer M call back. It will cleanup all stored forked dialogs
*/
void _EcrioSigMgrTransactionTimerMCallback
(
	void *pData,
	TIMERHANDLE timerID
)
{
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	_EcrioSigMgrForkedMessageListNodeStruct *pForkedMsgHead = NULL;
	_EcrioSigMgrForkedMessageListNodeStruct *pTempNode = NULL;
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;
	EcrioSigMgrStruct *pSigMgr = NULL;

	/* Stop the timer before proceeding further. */
	pal_TimerStop(timerID);

	if (NULL == pData)
	{
		return;
	}

	pDialogNode = (_EcrioSigMgrDialogNodeStruct *)pData;
	if (NULL == pDialogNode)
	{
		return;
	}

	pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
	if (NULL == pInviteUsage)
	{
		return;
	}

	pDialogNode->timerMId = NULL;

	pSigMgr = (EcrioSigMgrStruct *)pInviteUsage->pSigMgr;
	if (NULL == pSigMgr)
	{
		return;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pForkedMsgHead = pDialogNode->pForkedMsgHead;
	pTempNode = pDialogNode->pForkedMsgHead;
	while (pForkedMsgHead != NULL)
	{
		if (pForkedMsgHead != pDialogNode->pAcceptedForkedMsgNode)
		{
			if (pForkedMsgHead->pRemoteContact)
			{
				_EcrioSigMgrReleaseContactStruct(pSigMgr, pForkedMsgHead->pRemoteContact);
				pal_MemoryFree((void **)&pForkedMsgHead->pRemoteContact);
			}

			if (pForkedMsgHead->pRemoteTag)
			{
				pal_MemoryFree((void **)&pForkedMsgHead->pRemoteTag);
			}

			pTempNode = pForkedMsgHead;
			pForkedMsgHead = pTempNode->pNext;
			pal_MemoryFree((void **)&pTempNode);
		}
		else
		{
			pForkedMsgHead = pForkedMsgHead->pNext;
		}
	}

	pDialogNode->pForkedMsgHead = pDialogNode->pCurrForkedMsgNode = pDialogNode->pAcceptedForkedMsgNode;
	pDialogNode->pAcceptedForkedMsgNode->pNext = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}


/**
* Generate Error response structure.
*
*/
u_int32 _EcrioSigMgrSendErrorResponse
(
EcrioSigMgrStruct *pSigMgr,
EcrioSigMgrSipMessageStruct *pIncomingReq,
u_char *pMethodName,
u_int32 responseCode,
u_char *pReasonPhrase
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 respLen = 0;
	EcrioSigMgrCommonInfoStruct cmnInfo = { 0 };
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };
	u_char *pRespData = NULL;
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;
	EcrioSigMgrSipMessageStruct	errResp = { .eMethodType = EcrioSipMessageTypeNone };

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&errResp.pMandatoryHdrs);
	if (errResp.pMandatoryHdrs == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;

		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for errResp.pMandatoryHdrs, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}

	error = _EcrioSigMgrPopulateManHdrs(pSigMgr, pIncomingReq->pMandatoryHdrs,
		errResp.pMandatoryHdrs);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateManHdrs() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}

	
	errResp.responseCode = responseCode;
	errResp.pReasonPhrase = pReasonPhrase;
	errResp.eMethodType = pIncomingReq->eMethodType;
	errResp.eReqRspType = EcrioSigMgrSIPResponse;


	error = _EcrioSigMgrPopulateCommonInfo(pSigMgr, errResp.pMandatoryHdrs, &cmnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateCommonInfo() for pCmnInfo, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	cmnInfo.responseCode = errResp.responseCode;

	if (ECRIO_SIGMGR_RESPONSE_CODE_METHOD_NOT_ALLOWED == errResp.responseCode) {

		if (errResp.pOptionalHeaderList == NULL)
		{
			error = EcrioSigMgrCreateOptionalHeaderList(pSigMgr, &errResp.pOptionalHeaderList);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tEcrioSigMgrCreateOptionalHeaderList() error=%u",
					__FUNCTION__, __LINE__, error);

				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
		}

		error = _EcrioSigMgrAddAllowHeader(pSigMgr, errResp.pOptionalHeaderList);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddAllowHeader() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}
	}

	eTransportType = errResp.pMandatoryHdrs->ppVia[0]->transport;

	error = _EcrioSigMgrSipMessageForm(pSigMgr, EcrioSigMgrSipMessageModeFullName, &errResp, &pRespData, &respLen);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSipMessageForm() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_UNKNOWN_ERROR;
		goto Error_Level_01;
	}

	cmnInfo.pMethodName = pMethodName;
	cmnInfo.role = EcrioSigMgrRoleUAS;
	cmnInfo.responseCode = errResp.responseCode;

	txnInfo.currentContext = ECRIO_SIGMGR_ERROR_RESPONSE_SEND;
	txnInfo.retransmitContext = ECRIO_SIGMGR_ERROR_RESPONSE_RESEND;

	txnInfo.deleteflag = Enum_FALSE;
	txnInfo.messageType = EcrioSigMgrMessageNonInviteResponse;
	txnInfo.transport = eTransportType;
	txnInfo.pMsg = pRespData;
	txnInfo.msglen = respLen;
	txnInfo.pSessionMappingStr = NULL;

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeSIP,
		"%s:%u\tOutgoing SIP message constructed:", __FUNCTION__, __LINE__);
	SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pRespData, respLen);

	error = _EcrioSigMgrSendMsgToTXNHandler(pSigMgr, &cmnInfo, &txnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendMsgToTXNHandler() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_UNKNOWN_ERROR;
		goto Error_Level_01;
	}

Error_Level_01:

	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);
	if (pRespData != NULL)
	{
		pal_MemoryFree((void **)&pRespData);
		pRespData = NULL;
		respLen = 0;
	}

Error_Level_02:
	errResp.pReasonPhrase = NULL;
	_EcrioSigMgrReleaseSipMessage(pSigMgr, &errResp);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}
