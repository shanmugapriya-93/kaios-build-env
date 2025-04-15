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
#include "EcrioSigMgrCallbacks.h"
#include "EcrioSigMgrUtilities.h"
#include "EcrioSigMgrIMSLibHandler.h"
#include "EcrioSigMgrTransactionHandler.h"
#include "EcrioSigMgrDialogHandler.h"
#include "EcrioSigMgrSubscribe.h"
#include "EcrioSigMgrSubscribeInternal.h"
#include "EcrioSigMgrRefer.h"
#include "EcrioSigMgrReferInternal.h"

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

Function:		_EcrioSigMgrSendInvite()

Purpose:		Initiates the call.

Description:	Creates and sends a Invite request.

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
Input:			EcrioSigMgrSipMessageStruct* pInviteReq  - Invite Request Info.
Input:			BoolEnum reqReSending - Indicates wheather request is re seinding or not.
                (In case of 401 / 407).

Input / OutPut:	SIGSESSIONHANDLE* pSessionHandle - Session Id  of the initiated call.

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrSendInvite
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pInviteReq,
	u_char** ppCallID
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (*ppCallID == NULL)
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

		error = _EcrioSigMgrHandleUACEventInitialized(pSigMgr, pInviteReq, pInviteUsage);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrHandleUACEventInitialized() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		error = _EcrioSigMgrHandleUACEventSendInvite(pSigMgr, pInviteUsage, ppCallID);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrHandleUACEventSendInvite() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		pInviteUsage->callState = EcrioSigMgrCallStateInitialized;
	}
	else
	{
		_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
		_EcrioSigMgrInviteUsageInfoStruct* pInviteUsage = NULL;
		/* Retrive Dialog Node using ppCallID*/
		ec_MapGetKeyData(pSigMgr->hHashMap, *ppCallID, (void **)&pDialogNode);
		if (pDialogNode == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tNo matching dialog found for %s call-ID",
				__FUNCTION__, __LINE__, *ppCallID);
			error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
			goto Error_Level_01;
		}
		error = _EcrioSigMgrHandleUACEventSendReInvite(pSigMgr, pInviteReq, pDialogNode);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrHandleUACEventSendInvite() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
		pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct*)pDialogNode->pDialogContext;
		pInviteUsage->callState = EcrioSigMgrCallStateReInviteInProgress;		

		if (pInviteUsage->pAckMsg != NULL)
		{
			pal_MemoryFree((void**)&pInviteUsage->pAckMsg);
			pInviteUsage->msgAckLength = 0;
		}
	}

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************

Function:		_EcrioSigMgrHandleInviteResponse()

Purpose:		Handles the Invite responses.

Description:	Changes the state machine and updates it to the upper layer.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance
Input:			EcrioSigMgrSipMessageStruct* pInviteResp - received Invite response
Input:			EcrioSigMgrCommonInfoStruct* pCmnInfo - common Info struct
Input:			EcrioSigMgrDialogInfoStruct* pDlgInfo - dialog Info struct

Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrHandleInviteResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pInviteResp,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	BoolEnum bRequestTimedout
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	BoolEnum bIsForked = Enum_FALSE;
	u_int32 responseCode = ECRIO_SIGMGR_INVALID_RESPONSE_CODE;
	EcrioSigMgrMessageStruct message = { .msgCmd = EcrioSigMgrMessageNone };
	EcrioSigMgrSipMessageStruct	byeReq = { .eMethodType = EcrioSipMessageTypeNone };
	EcrioTXNTxnIdentificationStruct txnIdentStruct = { 0 };
	EcrioSigMgrHeaderStruct *pRseqHeader = NULL;

	EcrioSigMgrHeaderStruct *pSessionExpireHeader = NULL;
	EcrioSigMgrHeaderStruct *pAllowHeader = NULL;

	u_int32	uRseq = 0;
	u_int32	i = 0;
	BoolEnum bRseqExists = Enum_FALSE;
	EcrioSigMgrHeaderStruct *pMinSEHeader = NULL;
	u_int32	minSEInterval = 0;

	// u_char*									pParamVal = NULL;
	u_int32	sessionInterval = 0;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pCmnInfo->pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_INVITE;

	if (bRequestTimedout != Enum_TRUE)
	{
		pDialogNode = EcrioSigMgrMatchDialogAndUpdateState(pSigMgr,
			pInviteResp, &bIsForked, &responseCode);
		if (pDialogNode == NULL)
		{
			error = ECRIO_SIGMGR_INVITE_FAILED;
			goto Error_Level_01;
		}
	}
	else
	{
		BoolEnum bDialogMatched = Enum_FALSE;
		pDialogNode = EcrioSigMgrFindMatchedDialog(pSigMgr,
			pCmnInfo->pFromTag, pCmnInfo->pToTag, pCmnInfo->pCallId, pCmnInfo->cSeq, pInviteResp->eMethodType,
			pInviteResp->eReqRspType, &bDialogMatched, &bIsForked, &responseCode);
		if (pDialogNode == NULL)
		{
			error = ECRIO_SIGMGR_INVITE_FAILED;
			goto Error_Level_01;
		}
	}

	pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
	if (pInviteUsage == NULL)
	{
		error = ECRIO_SIGMGR_INVITE_FAILED;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInvite usage not found () INVITE response dropped, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}
	pInviteUsage->pSigMgr = pSigMgr;

	error = _EcrioSigMgrCheckInviteResponseInTransaction(pSigMgr, pInviteUsage, pCmnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrCheckInviteResponseInTransaction() INVITE response dropped since the branchId is not matched with the sent INVITE request, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tInvite Response Code=%d",
		__FUNCTION__, __LINE__, pInviteResp->responseCode);

	switch (pInviteResp->responseCode / 100)
	{
		case ECRIO_SIGMGR_1XX_RESPONSE:
		{
			pInviteResp->statusCode = ECRIO_SIGMGR_INVITE_IN_PROGRESS;
			if ((pInviteUsage->callState == EcrioSigMgrCallStateInitialized) ||
				(pInviteUsage->callState == EcrioSigMgrCallStateSetupInProgress))
			{
				BoolEnum bRequire = Enum_FALSE;
				message.msgCmd = EcrioSigMgrInviteResponse;

				if (pInviteResp->responseCode != ECRIO_SIGMGR_RESPONSE_CODE_TRYING)
				{
					pInviteUsage->callState = EcrioSigMgrCallStateSetupInProgress;
					pInviteUsage->bPrackEnabled = Enum_FALSE;
				}

				message.pData = (void *)pInviteResp;
				if (bRseqExists != Enum_TRUE)
				{
					if (_EcrioSigMgrGetModuleId(pSigMgr, pCmnInfo->pCallId) == EcrioSigMgrCallbackRegisteringModule_CPM)
					{
						_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
					}
					else
					{
						_EcrioSigMgrUpdateStatusMoIP(pSigMgr, &message);
					}
				}
				if (bRequire == Enum_TRUE)
				{
					EcrioSigMgrSipMessageStruct	sipRequest = { .eMethodType = EcrioSipMessageTypeNone };
					EcrioSigMgrMandatoryHeaderStruct mandatoryHdrs = { 0 };
					sipRequest.eMethodType = EcrioSipMessageTypePrack;
					sipRequest.eReqRspType = EcrioSigMgrSIPRequest;

					mandatoryHdrs.pCallId = pCmnInfo->pCallId;
					sipRequest.pMandatoryHdrs = &mandatoryHdrs;
					error = _EcrioSigMgrSendPrack(pSigMgr, &sipRequest);
					if (ECRIO_SIGMGR_NO_ERROR != error)
					{
						SIGMGRLOGE(pSigMgr, KLogTypeGeneral, "%s:%u\tEcrioSigMgrSendPrackRequest() error=%u",
							__FUNCTION__, __LINE__, error);
						goto Error_Level_01;
					}
				}
			}
		}
		break;

		case ECRIO_SIGMGR_2XX_RESPONSE:
		{
			pInviteResp->statusCode = ECRIO_SIGMGR_RESPONSE_SUCCESS;
			if ((pInviteUsage->callState == EcrioSigMgrCallStateInitialized) ||
				(pInviteUsage->callState == EcrioSigMgrCallStateSetupInProgress) ||
				(pInviteUsage->callState == EcrioSigMgrCallStateReInviteInProgress))
			{
				if (pInviteUsage->callState == EcrioSigMgrCallStateReInviteInProgress)
					message.msgCmd = EcrioSigMgrReInviteResponse;
				else
					message.msgCmd = EcrioSigMgrInviteResponse;

				pInviteUsage->callState = EcrioSigMgrCallStateCallEstablished;

				/* session expire support start */
				if (pSigMgr->pOOMObject)
				{
					if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeInvite_2xx, EcrioSipHeaderTypeSessionExpires))
					{
						EcrioSigMgrGetOptionalHeader(pSigMgr, pInviteResp->pOptionalHeaderList,
							EcrioSipHeaderTypeSessionExpires, &pSessionExpireHeader);

						if (pSessionExpireHeader != NULL)
						{
							if (pSessionExpireHeader && pSessionExpireHeader->ppHeaderValues && pSessionExpireHeader->ppHeaderValues[0] && pSessionExpireHeader->ppHeaderValues[0]->pHeaderValue)
							{
								sessionInterval = pal_StringConvertToUNum(pSessionExpireHeader->ppHeaderValues[0]->pHeaderValue, NULL, 10);
							}

							if (sessionInterval > 0)
							{
								for (i = 0; i < pSessionExpireHeader->numHeaderValues; i++)
								{
									if (pSessionExpireHeader->ppHeaderValues && pSessionExpireHeader->ppHeaderValues[i])
									{
										if (pSessionExpireHeader->ppHeaderValues[i]->ppParams != NULL &&
											pal_SubString(pSessionExpireHeader->ppHeaderValues[i]->ppParams[0]->pParamName, (u_char *)ECRIO_SIG_MGR_SESSION_REFRESHER_PARAM))
										{
											if (pal_SubString(pSessionExpireHeader->ppHeaderValues[i]->ppParams[0]->pParamValue, (u_char *)ECRIO_SIG_MGR_SESSION_REFRESHER_UAC))
											{
												pInviteUsage->isRefresher = Enum_TRUE;
											}
											else if (pal_SubString(pSessionExpireHeader->ppHeaderValues[i]->ppParams[0]->pParamValue, (u_char *)ECRIO_SIG_MGR_SESSION_REFRESHER_UAS))
											{
												pInviteUsage->isRefresher = Enum_FALSE;
											}
										}
										else
										{
											pInviteUsage->isRefresher = Enum_TRUE;
										}

										break;
									}
								}

								pInviteUsage->sessionExpireVal = sessionInterval;

								_EcrioSigMgrStartSessionRefreshExpireTimer(pSigMgr, pInviteUsage,
									pDialogNode, sessionInterval, pInviteUsage->isRefresher);
							}	// session interval
						}	// pSessionExpireHeader
						else// get it from Request
						{
							// As per RFC 4028 section 7.2, if the 2XX response does not contain session expires header then UAC can assume the role 
							// of a refresher
							EcrioSigMgrGetOptionalHeader(pSigMgr, pInviteUsage->pInviteReq->pOptionalHeaderList,
								EcrioSipHeaderTypeSessionExpires, &pSessionExpireHeader);

							if (pSessionExpireHeader != NULL)
							{
								if (pSessionExpireHeader && pSessionExpireHeader->ppHeaderValues && pSessionExpireHeader->ppHeaderValues[0] && pSessionExpireHeader->ppHeaderValues[0]->pHeaderValue)
								{
									sessionInterval = pal_StringConvertToUNum(pSessionExpireHeader->ppHeaderValues[0]->pHeaderValue, NULL, 10);
								}

								if (sessionInterval > 0)
								{
									for (i = 0; i < pSessionExpireHeader->numHeaderValues; i++)
									{
										if (pSessionExpireHeader->ppHeaderValues && pSessionExpireHeader->ppHeaderValues[i])
										{
											if (pSessionExpireHeader->ppHeaderValues[i]->ppParams != NULL &&
												pal_SubString(pSessionExpireHeader->ppHeaderValues[i]->ppParams[0]->pParamName, (u_char *)ECRIO_SIG_MGR_SESSION_REFRESHER_PARAM))
											{
												if (pal_SubString(pSessionExpireHeader->ppHeaderValues[i]->ppParams[0]->pParamValue, (u_char *)ECRIO_SIG_MGR_SESSION_REFRESHER_UAS) ||
													pal_SubString(pSessionExpireHeader->ppHeaderValues[i]->ppParams[0]->pParamValue, (u_char *)ECRIO_SIG_MGR_SESSION_REFRESHER_UAC))
												{
													pInviteUsage->isRefresher = Enum_TRUE;
													pInviteUsage->sessionExpireVal = sessionInterval;
													_EcrioSigMgrStartSessionRefreshExpireTimer(pSigMgr, pInviteUsage,
														pDialogNode, pInviteUsage->sessionExpireVal, pInviteUsage->isRefresher);
												}
											}

											break;
										}
									}
								}
							}
						}
					}

					/* TODO */
					/* MinSE and Checking in Allow headder All*/
					{
						/* Check UAS supports "UPDATE" method or not? */
						error = EcrioSigMgrGetOptionalHeader(pSigMgr, pInviteResp->pOptionalHeaderList,
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
				}
				/* session expire support end*/

				error = _EcrioSigMgrGetConversationsIdHeader(pSigMgr, pInviteResp);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrPopulateConversationsIdHeader() failed, error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}
				
				message.pData = (void *)pInviteResp;

				_EcrioSigMgrSendAck(pSigMgr, pDialogNode, Enum_FALSE);

				pDialogNode->pAcceptedForkedMsgNode = pDialogNode->pCurrForkedMsgNode;

				if (_EcrioSigMgrGetModuleId(pSigMgr, pCmnInfo->pCallId) == EcrioSigMgrCallbackRegisteringModule_CPM)
				{
					_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
				}
				else
				{
					_EcrioSigMgrUpdateStatusMoIP(pSigMgr, &message);
				}

				if (pInviteResp->pConvId)
				{
					/*Just free pID struct, struct release API does not contain this release*/
					pal_MemoryFree((void**)&pInviteResp->pConvId->pContributionId);
					pal_MemoryFree((void**)&pInviteResp->pConvId->pConversationId);
					pal_MemoryFree((void**)&pInviteResp->pConvId->pInReplyToContId);
					pal_MemoryFree((void**)&pInviteResp->pConvId);
				}

				if (pDialogNode->bIsFirst200OKReceived != Enum_TRUE)
				{
					TimerStartConfigStruct timerConfig = { 0 };
					pDialogNode->bIsFirst200OKReceived = Enum_TRUE;
					/* Start Timer M*/
					timerConfig.uPeriodicInterval = 0;
					timerConfig.pCallbackData = (void *)pDialogNode;
					timerConfig.bEnableGlobalMutex = Enum_TRUE;
					timerConfig.uInitialInterval = pSigMgr->customSIPTimers.TimerM;
					timerConfig.timerCallback = _EcrioSigMgrTransactionTimerMCallback;
					if (KPALErrorNone != pal_TimerStart(pSigMgr->pal, &timerConfig, &pDialogNode->timerMId))
					{
						SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
							"%s:%u\tpal_TimerStart() failed to start timer M",
							__FUNCTION__, __LINE__);
					}

				}
			}
			else if (pInviteUsage->callState == EcrioSigMgrCallStateCallEstablished)
			{
				if (pDialogNode->bIsFirst200OKReceived == Enum_TRUE &&
					pDialogNode->pAcceptedForkedMsgNode == pDialogNode->pCurrForkedMsgNode)
					_EcrioSigMgrSendAck(pSigMgr, pDialogNode, Enum_FALSE);
				else
				{
					_EcrioSigMgrSendAck(pSigMgr, pDialogNode, Enum_FALSE);
					_EcrioSigMgrSendBye(pSigMgr, pDialogNode, &byeReq);
				}
			}
			else if (pInviteUsage->callState == EcrioSigMgrCallStateCancelInProgress)
			{
				txnIdentStruct.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_CANCEL;
				txnIdentStruct.txnType = EcrioTXNTxnTypeClientNonInvite;
				EcrioTXNDeleteTxnMatchedSessionMatchedTxnIdentifier(pSigMgr->pTransactionMgrHndl,
					pInviteUsage->pInviteReq->pMandatoryHdrs->pCallId,
					pInviteUsage->pInviteReq->pMandatoryHdrs->ppVia[0]->pBranch,
					&txnIdentStruct);

				_EcrioSigMgrSendAck(pSigMgr, pDialogNode, Enum_FALSE);

				_EcrioSigMgrSendBye(pSigMgr, pDialogNode, &byeReq);
			}
			else
			{
				error = ECRIO_SIGMGR_INVALID_OPERATION;
				goto Error_Level_01;
			}
		}
		break;	
		case ECRIO_SIGMGR_3XX_RESPONSE:
		case ECRIO_SIGMGR_4XX_RESPONSE:
		case ECRIO_SIGMGR_5XX_RESPONSE:
		case ECRIO_SIGMGR_6XX_RESPONSE:
		default:
		{
			BoolEnum bSkipNotify = Enum_FALSE;			
			if ((pInviteUsage->callState == EcrioSigMgrCallStateInitialized) ||
				(pInviteUsage->callState == EcrioSigMgrCallStateSetupInProgress) ||
				(pInviteUsage->callState == EcrioSigMgrCallStateReInviteInProgress))
			{
				if (bRequestTimedout != Enum_TRUE)
				{
					_EcrioSigMgrSendAck(pSigMgr, pDialogNode, Enum_TRUE);
				}

				/* Check if error response need to be notified or not. If we have multiple dialogs due to forking,
				only success or last error response is notified. */
				if (pDialogNode->pForkedMsgHead != NULL)
				{
					u_int32 count = 0;
					_EcrioSigMgrForkedMessageListNodeStruct* pTemp = pDialogNode->pForkedMsgHead;
					while (pTemp != NULL)
					{
						/* Count only non terminated dialogs */
						if (pTemp->bIsEarlyDialogTermination != Enum_TRUE)
							count++;

						/* Check and mark the dialog as terminated*/
						if (pal_StringCompare(pTemp->pRemoteTag, pDialogNode->pCurrForkedMsgNode->pRemoteTag) == 0)
							pTemp->bIsEarlyDialogTermination = Enum_TRUE;
						
						pTemp = pTemp->pNext;
					}

					if (count > 1)
						bSkipNotify = Enum_TRUE;
				}

				if (pInviteResp->responseCode == ECRIO_SIGMGR_RESPONSE_CODE_SESSION_INTERVAL_SMALL)
				{
					error = _EcrioSigMgrHandle422InviteResponse(pSigMgr, pInviteResp, pDialogNode);
					if (error != ECRIO_SIGMGR_NO_ERROR)
					{
						SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
							"%s:%u\t_EcrioSigMgrHandle422InviteResponse() failed, error=%u", __FUNCTION__, __LINE__, error);
						goto Error_Level_01;
					}

					bSkipNotify = Enum_TRUE;
				}

				if (bSkipNotify != Enum_TRUE)
				{
					if (pInviteUsage->callState == EcrioSigMgrCallStateReInviteInProgress)
						message.msgCmd = EcrioSigMgrReInviteResponse;
					else
						message.msgCmd = EcrioSigMgrInviteResponse;

					message.pData = (void*)pInviteResp;

					pInviteResp->statusCode = ECRIO_SIGMGR_REQUEST_FAILED;
					if (_EcrioSigMgrGetModuleId(pSigMgr, pCmnInfo->pCallId) == EcrioSigMgrCallbackRegisteringModule_CPM)
					{
						_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
					}
					else
					{
						_EcrioSigMgrUpdateStatusMoIP(pSigMgr, &message);
					}

					pInviteUsage->callState = pInviteUsage->callState == EcrioSigMgrCallStateReInviteInProgress ? EcrioSigMgrCallStateCallEstablished : EcrioSigMgrCallStateUnInitialized;
				}
			}
			else if (pInviteUsage->callState == EcrioSigMgrCallStateCancelInProgress)
			{
				txnIdentStruct.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_CANCEL;
				txnIdentStruct.txnType = EcrioTXNTxnTypeClientNonInvite;
				EcrioTXNDeleteTxnMatchedSessionMatchedTxnIdentifier(pSigMgr->pTransactionMgrHndl,
					pInviteUsage->pInviteReq->pMandatoryHdrs->pCallId,
					pInviteUsage->pInviteReq->pMandatoryHdrs->ppVia[0]->pBranch,
					&txnIdentStruct);

				_EcrioSigMgrSendAck(pSigMgr, pDialogNode, Enum_TRUE);

				pInviteUsage->callState = EcrioSigMgrCallStateUnInitialized;
			}
			else
			{
				error = ECRIO_SIGMGR_INVALID_OPERATION;
				goto Error_Level_01;
			}
		}
		break;
	}

Error_Level_01:
	if ((NULL != pInviteUsage) && (pInviteUsage->callState == EcrioSigMgrCallStateUnInitialized))
	{
		_EcrioSigMgrInternalStructRelease(pSigMgr, EcrioSigMgrStructType_InviteUsageInfo,
			(void **)&pInviteUsage, Enum_TRUE);

		_EcrioSigMgrDeleteDialog(pSigMgr, pDialogNode);

		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tDialog deleted", __FUNCTION__, __LINE__);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32	_EcrioSigMgrHandle422InviteResponse
(
	EcrioSigMgrStruct* pSigMgr, 
	EcrioSigMgrSipMessageStruct* pInviteResp, 
	_EcrioSigMgrDialogNodeStruct* pDialogNode
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrSipMessageStruct	byeReq = { .eMethodType = EcrioSipMessageTypeNone };
	EcrioSigMgrHeaderStruct* pMinSEHeader = NULL;
	u_int32	minSEInterval = 0;
	_EcrioSigMgrInviteUsageInfoStruct* pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct*)pDialogNode->pDialogContext;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/* Min-SE support start */
	if (pSigMgr->pOOMObject)
	{
		/* Handle MinSE */
		if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeUpdate_4xx, EcrioSipHeaderTypeMinSE))
		{
			error = EcrioSigMgrGetOptionalHeader(pSigMgr, pInviteResp->pOptionalHeaderList,
				EcrioSipHeaderTypeMinSE, &pMinSEHeader);
			if (pMinSEHeader != NULL)
			{
				if (pMinSEHeader->ppHeaderValues && pMinSEHeader->ppHeaderValues[0] && pMinSEHeader->ppHeaderValues[0]->pHeaderValue)
				{
					minSEInterval = pal_StringConvertToUNum(pMinSEHeader->ppHeaderValues[0]->pHeaderValue, NULL, 10);
				}
				if (minSEInterval > 0)
				{
					pInviteUsage->sessionExpireVal = minSEInterval;
					pInviteUsage->minSEVal = minSEInterval;
					/* send fresh UPDATE */
					_EcrioSigMgrStartSessionRefreshExpireTimer(pSigMgr, pInviteUsage,
						pDialogNode, 1, pInviteUsage->isRefresher);
				}
			}
			else
			{
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
			}
		}
	}
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u", __FUNCTION__, __LINE__, error);
	return error;
}

/**
* Check the invite response in transaction or not. It
* compare the branch parameter only.
*/
u_int32	_EcrioSigMgrCheckInviteResponseInTransaction
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrInviteUsageInfoStruct *pInviteInfoStruct,
	EcrioSigMgrCommonInfoStruct *pCmnInfo
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;

	// @todo Will this function ever need pSigMgr?
	(void)pSigMgr;

	if (pal_StringICompare(pInviteInfoStruct->pBranch, pCmnInfo->pBranch) != 0)
	{
		error = ECRIO_SIGMGR_TRANSACTION_NOT_FOUND;
	}

	return error;
}

/*****************************************************************************

Function:		_EcrioSigMgrSendAck()

Purpose:		Create and Send ACK Request.

Description:	Construct and send ACK Request on receiving INVITE Response.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
Input:			SIGSESSIONHANDLE sessionHandle - handle to session
Input:			BoolEnum updateFlag - Override flag
Input:			BoolEnum isErrorResponse - is 2xx or non-2xx INVITE response

Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrSendAck
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrDialogNodeStruct *pDialogNode,
	BoolEnum isErrorResponse
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	/* EcrioSigMgrURISchemesEnum reqUriScheme = EcrioSigMgrURISchemeNone; */
	EcrioSigMgrCommonInfoStruct	cmnInfo = { 0 };
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };
	EcrioSigMgrMandatoryHeaderStruct *pMandatoryHdrs = NULL;
	EcrioSigMgrSipMessageStruct *pAckReqInfo = NULL;
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData = NULL;
	u_int32	reqLen = 0;
	u_char *pReqData = NULL;
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;

	if (pSigMgr == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_ACK;
	cmnInfo.role = EcrioSigMgrRoleUAC;

	pUsageData = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;

	/* Check and free stored ACK */
	if (pDialogNode->pAcceptedForkedMsgNode != pDialogNode->pCurrForkedMsgNode &&
		pUsageData->pAckMsg != NULL)
		pal_MemoryFree((void**)&pUsageData->pAckMsg);

	if (pUsageData->pAckMsg == NULL)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrSipMessageStruct), (void **)&pAckReqInfo);
		if (pAckReqInfo == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pAckReqInfo, error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}

		pal_MemoryAllocate((u_int32)sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&pMandatoryHdrs);

		if (pMandatoryHdrs == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pMandatoryHdrs, error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}

		pAckReqInfo->pMandatoryHdrs = pMandatoryHdrs;

		pAckReqInfo->eMethodType = EcrioSipMessageTypeAck;
		pAckReqInfo->eReqRspType = EcrioSigMgrSIPRequest;

		error = _EcrioSigMgePopulateHeaderFromDialogAndUpdateState(pSigMgr,
			pAckReqInfo, pDialogNode);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddLocalContactUri() for pOptionalHdrs->pContact, error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}

		error = _EcrioSigMgrPopulateMandatoryHeaders(pSigMgr, pAckReqInfo,
			EcrioSigMgrSIPRequest, Enum_FALSE, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrPopulateMandatoryHeaders() for pIMSManHdrs, error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}

		if (pAckReqInfo->pContact == NULL)
		{
			error = _EcrioSigMgrAddLocalContactUri(pSigMgr, &pAckReqInfo->pContact, pAckReqInfo->bPrivacy);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddLocalContactUri() for pOptionalHdrs->pContact, error=%u", __FUNCTION__, __LINE__, error);

				goto Error_Level_01;
			}

			if (pSigMgr->pOOMObject->ec_oom_HasUserPhoneSupport(EcrioSipMessageTypeAck, EcrioSipHeaderTypeContact, EcrioSipURIType_SIP) == Enum_TRUE)
			{
				/* All outgoing SIP uri which contain telephone number in user info part must have "user=phone" parameter as SIP uri parameter.	*/
				error = _EcrioSigMgrAddUserPhoneParam(pSigMgr, &pAckReqInfo->pContact->ppContactDetails[0]->nameAddr.addrSpec);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrAddUserPhoneParam, error=%u", __FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}
			}
		}

		if (isErrorResponse == Enum_TRUE)
		{
			if (pUsageData->pInviteReq->pRouteSet != NULL)
			{
				if (pAckReqInfo->pRouteSet != NULL)
					_EcrioSigMgrReleaseRouteStruct(pSigMgr, pAckReqInfo->pRouteSet);
				else
					pal_MemoryAllocate(sizeof(EcrioSigMgrRouteStruct), (void **)&pAckReqInfo->pRouteSet);

				if (pAckReqInfo->pRouteSet == NULL)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tpal_MemoryAllocate() for pAckReqInfo->pRouteSet, error=%u",
						__FUNCTION__, __LINE__, error);

					goto Error_Level_01;
				}

				error = _EcrioSigMgrCopyRouteSet(pSigMgr, (u_int16)pUsageData->pInviteReq->pRouteSet->numRoutes,
					pUsageData->pInviteReq->pRouteSet->ppRouteDetails,
					&pAckReqInfo->pRouteSet->ppRouteDetails);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrCopyRouteSet() for pOptionalHdrs->pRouteSet->ppRouteDetails, error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}

				pAckReqInfo->pRouteSet->numRoutes = pUsageData->pInviteReq->pRouteSet->numRoutes;
			}

			if (pUsageData->pInviteReq->pMandatoryHdrs != NULL &&
				pUsageData->pInviteReq->pMandatoryHdrs->ppVia != NULL)
			{
				u_int32 i = 0;

				/* Free Via (if non-null) & Overwrite from dialog information
				*/
				if (pMandatoryHdrs->ppVia != NULL)
				{
					u_int32 i = 0;

					for (i = 0; i < pMandatoryHdrs->numVia; ++i)
					{
						_EcrioSigMgrReleaseViaStruct(pSigMgr, pMandatoryHdrs->ppVia[i]);
						pal_MemoryFree((void **)&(pMandatoryHdrs->ppVia[i]));
					}

					pal_MemoryFree((void **)&(pMandatoryHdrs->ppVia));
				}

				/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
				if (pal_UtilityDataOverflowDetected(pUsageData->pInviteReq->pMandatoryHdrs->numVia, 
					sizeof(EcrioSigMgrViaStruct *)) == Enum_TRUE)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					goto Error_Level_01;
				}

				/* Overwrite Via from dialog information
				*/
				pal_MemoryAllocate(sizeof(EcrioSigMgrViaStruct *) * (pUsageData->pInviteReq->pMandatoryHdrs->numVia), (void **)&pMandatoryHdrs->ppVia);
				if (pMandatoryHdrs->ppVia == NULL)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					goto Error_Level_01;
				}

				pMandatoryHdrs->numVia = pUsageData->pInviteReq->pMandatoryHdrs->numVia;

				for (i = 0; i < pMandatoryHdrs->numVia; ++i)
				{
					pal_MemoryAllocate(sizeof(EcrioSigMgrViaStruct), (void **)&pMandatoryHdrs->ppVia[i]);
					if (pMandatoryHdrs->ppVia[i] == NULL)
					{
						error = ECRIO_SIGMGR_NO_MEMORY;
						goto Error_Level_01;
					}

					error = _EcrioSigMgrPopulateViaHdr(pSigMgr, \
						pUsageData->pInviteReq->pMandatoryHdrs->ppVia[i], \
						pMandatoryHdrs->ppVia[i]);
					if (error != ECRIO_SIGMGR_NO_ERROR)
					{
						goto Error_Level_01;
					}
				}
			}
		}

		pAckReqInfo->pMandatoryHdrs = pMandatoryHdrs;

		if (isErrorResponse)
		{
			/* MemoryLeakFIx*/
			_EcrioSigMgrStringCreate(pSigMgr, pUsageData->pBranch, &cmnInfo.pBranch);
		}
		else
		{
		}

		/*	Support for Authorization header in all request	start	*/

		/*if(pUsageData && pUsageData->pInviteReq)
		{
		    _EcrioSigMgrCopyACKCredentials(pSigMgr,
		        &(pAckReqInfo->numProxyAuth),
		        &(pAckReqInfo->ppProxyAuth),
		        &(pAckReqInfo->pAuthorization),
		        pUsageData->pInviteReq->numProxyAuth,
		        pUsageData->pInviteReq->ppProxyAuth,
		        pUsageData->pInviteReq->pAuthorization);
		}*/

		/* Looking for transport type. it TCP, don't need to call _EcrioSigMgrCreateTCPBasedBuffer()*/
		eTransportType = pAckReqInfo->pMandatoryHdrs->ppVia[0]->transport;

		error = _EcrioSigMgrSipMessageForm(pSigMgr, EcrioSigMgrSipMessageModeFullName, pAckReqInfo, &pReqData, &reqLen);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrCreateAckReq() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		if (pSigMgr->pSignalingInfo->bEnableTls == Enum_TRUE && pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex != 0)
		{
			eTransportType = EcrioSigMgrTransportTLS;
		}
		else if (pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE)
		{
			if ((eTransportType != EcrioSigMgrTransportTCP) && (reqLen > pSigMgr->pSignalingInfo->uUdpMtu))
			{
				error = _EcrioSigMgrCreateTCPBasedBuffer(pSigMgr, &pReqData, EcrioSigMgrSIPRequest);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrCreateTCPBasedBuffer() error=%u",
						__FUNCTION__, __LINE__, error);

					error = ECRIO_SIGMGR_IMS_LIB_ERROR;
					goto Error_Level_01;
				}

				eTransportType = EcrioSigMgrTransportTCP;
			}
		}
	}
	else
	{
		pReqData = pUsageData->pAckMsg;
		reqLen = pUsageData->msgAckLength;
	}

	txnInfo.currentContext = ECRIO_SIGMGR_ACK_SEND;

	if (isErrorResponse)
	{
		txnInfo.retransmitContext = ECRIO_SIGMGR_ACK_RESEND;
	}
	else
	{
		txnInfo.retransmitContext = ECRIO_SIGMGR_INVALID_CONTEXT;
	}

	txnInfo.deleteflag = Enum_FALSE;
	txnInfo.messageType = EcrioSigMgrMessageInviteRequest;
	txnInfo.transport = eTransportType;
	txnInfo.pMsg = pReqData;
	txnInfo.msglen = reqLen;
#if 0	// disabled this code to fix the compiler warning...
	if (pMandatoryHdrs && pMandatoryHdrs->pRequestUri)
	{
		reqUriScheme = pMandatoryHdrs->pRequestUri->uriScheme;
	}
#endif

	/*TODO: Write log for outgoing request at the correc position in the code*/
	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeSIP,
		"%s:%u\tOutgoing SIP message constructed:", __FUNCTION__, __LINE__);
	SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pReqData, reqLen);

	error = _EcrioSigMgrPopulateCommonInfo(pSigMgr, pMandatoryHdrs, &cmnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		error = ECRIO_SIGMGR_REQUEST_FAILED;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateCommonInfo() error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	error = _EcrioSigMgrSendMsgToTXNHandler(pSigMgr, &cmnInfo, &txnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		error = ECRIO_SIGMGR_REQUEST_FAILED;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendMsgToTXNHandler() error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	/*EcrioDLGMgrGetAuthParams(pSigMgr->pDialogMgrHndl, &pSessionHandle->pDialogNode, (void**)&pTempAuth);
	if((pTempAuth != NULL) && ((pTempAuth->numProxyAuth > 0) || (pTempAuth->pAuth != NULL)))
	{
	    ++pTempAuth->authCount;

	    SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral,
	        "%s:%u\tauthCount=%d", __FUNCTION__, __LINE__, pTempAuth->authCount);
	}*/

	if (pUsageData->pAckMsg == NULL)
	{
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpAckMsg is NULL", __FUNCTION__, __LINE__);

		if (isErrorResponse == Enum_FALSE)
		{
			pUsageData->pAckMsg = pReqData;
			pReqData = NULL;
			pUsageData->msgAckLength = reqLen;
		}
	}

	if (txnInfo.bSendDirect)
	{
		SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tACK Send Direct", __FUNCTION__, __LINE__);

		pUsageData->pAckMsg = txnInfo.pMsg;
		txnInfo.pMsg = NULL;
		pReqData = NULL;
		pUsageData->msgAckLength = txnInfo.msglen;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\tcallState=%d", __FUNCTION__, __LINE__, pUsageData->callState);

	pUsageData->isCaller = Enum_FALSE;
#if 0
	if ((isErrorResponse == Enum_FALSE) && (terminateCall == Enum_TRUE))
	{
		_EcrioSigMgrSendBye(pSigMgr, pDialogNode, &byeReqInfo);
	}
#endif
Error_Level_01:

	if (pReqData != NULL)
	{
		pal_MemoryFree((void **)&pReqData);
		pReqData = NULL;
		reqLen = 0;
		pUsageData->pAckMsg = NULL;
		pUsageData->msgAckLength = 0;
	}

	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);
	if (pAckReqInfo)
	{
		pAckReqInfo->pReasonPhrase = NULL;
		_EcrioSigMgrReleaseSipMessage(pSigMgr, pAckReqInfo);
		pal_MemoryFree((void **)&pAckReqInfo);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrSendPrack
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pPrackReq
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	ec_MapGetKeyData(pSigMgr->hHashMap, pPrackReq->pMandatoryHdrs->pCallId, (void **)&pDialogNode);

	pUsageData = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;

	if (pUsageData->callState == EcrioSigMgrCallStateSetupInProgress)
	{
		if (((pSigMgr->bIPSecEnabled == Enum_TRUE) && (pSigMgr->pIPSecDetails->eIPSecState == ECRIO_SIP_IPSEC_STATE_ENUM_Established)) ||
			pSigMgr->pSignalingInfo->pSecurityVerify != NULL)
		{
			u_char *pRequire = (u_char *)ECRIO_SIG_MGR_HEADER_VALUE_SEC_AGREE;
			error = _EcrioSigMgrCreateSecurityVerifyHdr(pSigMgr, &pPrackReq->pOptionalHeaderList);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error_Level_01;
			}
			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pPrackReq->pOptionalHeaderList, EcrioSipHeaderTypeRequire, 1, &pRequire, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);

				goto Error_Level_01;
			}
			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pPrackReq->pOptionalHeaderList, EcrioSipHeaderTypeProxyRequire, 1, &pRequire, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);

				goto Error_Level_01;
			}
		}
		error = _EcrioSigMgrSendInDialogSipRequests(pSigMgr, pPrackReq, pDialogNode);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrSendInDialogSipRequests() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************

Function:		_EcrioSigMgrSendCancel()

Purpose:		Canceling any pending call.

Description:	Creates and sends a Cancel request.

Input:			SIGMGRHANDLE sigMgrHandle - Handle to signaling manager.

Input:			EcrioSigMgrCancelReqHdrStruct* pCancelReq - Cancel request
Header values.

Input:			SIGSESSIONHANDLE sessionHandle - Handle to session.

Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrSendCancel
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrDialogNodeStruct *pDialogNode,
	EcrioSigMgrSipMessageStruct *pCancelReq
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pUsageData = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
	if (pUsageData->callState == EcrioSigMgrCallStateInitialized)
	{
		EcrioTXNTxnIdentificationStruct txnIdentStruct = { 0 };

		if ((pUsageData->callState == EcrioSigMgrCallStateInitialized) &&
			(pUsageData->pInviteReq != NULL))
		{
			txnIdentStruct.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_INVITE;
			txnIdentStruct.txnType = EcrioTXNTxnTypeClientInvite;
			EcrioTXNDeleteTxnMatchedSessionMatchedTxnIdentifier(pSigMgr->pTransactionMgrHndl,
				pUsageData->pInviteReq->pMandatoryHdrs->pCallId,
				pUsageData->pInviteReq->pMandatoryHdrs->ppVia[0]->pBranch,
				&txnIdentStruct);
		}

		pUsageData->callState = EcrioSigMgrCallStateUnInitialized;
	}
	else
	{
		if (((pUsageData->isCaller == Enum_TRUE) &&
			((pUsageData->callState == EcrioSigMgrCallStateSetupInProgress))))
		{
			if (((pSigMgr->bIPSecEnabled == Enum_TRUE) && (pSigMgr->pIPSecDetails->eIPSecState == ECRIO_SIP_IPSEC_STATE_ENUM_Established)) ||
				pSigMgr->pSignalingInfo->pSecurityVerify != NULL)
			{
				u_char *pRequire = (u_char *)ECRIO_SIG_MGR_HEADER_VALUE_SEC_AGREE;

				error = _EcrioSigMgrCreateSecurityVerifyHdr(pSigMgr, &pCancelReq->pOptionalHeaderList);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					goto Error_Level_01;
				}
				error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pCancelReq->pOptionalHeaderList, EcrioSipHeaderTypeRequire, 1, &pRequire, 0, NULL, NULL);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
						__FUNCTION__, __LINE__, error);

					goto Error_Level_01;
				}
				error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pCancelReq->pOptionalHeaderList, EcrioSipHeaderTypeProxyRequire, 1, &pRequire, 0, NULL, NULL);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
						__FUNCTION__, __LINE__, error);

					goto Error_Level_01;
				}
			}
			pCancelReq->eMethodType = EcrioSipMessageTypeCancel;
			error = _EcrioSigMgrSendInDialogSipRequests(pSigMgr, pCancelReq, pDialogNode);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrSendInDialogSipRequests() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}

			pUsageData->callState = EcrioSigMgrCallStateCancelInProgress;
		}
		else
		{
			error = ECRIO_SIGMGR_INVALID_OPERATION;
			goto Error_Level_01;
		}
	}

Error_Level_01:

	if ((pDialogNode != NULL) && (pUsageData->callState == EcrioSigMgrCallStateUnInitialized))
	{
		/*EcrioSigMgrMessageStruct				message = { 0 };
		message.msgCmd = EcrioSigMgrDeleteSessionNotification;
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

/*****************************************************************************

Function:		_EcrioSigMgrSendBye()

Purpose:		Create and Send BYE Request.

Description:	Construct and send BYE Request if call is not in
                initial state. Otherwise terminate the call.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
Input:			SIGSESSIONHANDLE sessionHandle - handle to session
Input:			EcrioSigMgrByeRequestStruct* pByeReqInfo - BYE request structure.
Input:			BoolEnum updateFlag - Override flag.

Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrSendBye
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrDialogNodeStruct *pDialogNode,
	EcrioSigMgrSipMessageStruct *pByeReqInfo
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pUsageData = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;

	if (((pSigMgr->bIPSecEnabled == Enum_TRUE) && (pSigMgr->pIPSecDetails->eIPSecState == ECRIO_SIP_IPSEC_STATE_ENUM_Established)) ||
		pSigMgr->pSignalingInfo->pSecurityVerify != NULL)
	{
		u_char *pRequire = (u_char *)ECRIO_SIG_MGR_HEADER_VALUE_SEC_AGREE;
		error = _EcrioSigMgrCreateSecurityVerifyHdr(pSigMgr, &pByeReqInfo->pOptionalHeaderList);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pByeReqInfo->pOptionalHeaderList, EcrioSipHeaderTypeRequire, 1, &pRequire, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}
		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pByeReqInfo->pOptionalHeaderList, EcrioSipHeaderTypeProxyRequire, 1, &pRequire, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}
	}

	{
		if ((pUsageData->callState == EcrioSigMgrCallStateCallEstablished) ||
			(pUsageData->callState == EcrioSigMgrCallStateAcceptedAckPending) ||
			(pUsageData->callState == EcrioSigMgrCallStateReInviteAcceptedAckPending) ||
			(pUsageData->callState == EcrioSigMgrCallStateCancelInProgress))
		{
			pByeReqInfo->eMethodType = EcrioSipMessageTypeBye;
			error = _EcrioSigMgrSendInDialogSipRequests(pSigMgr, pByeReqInfo, pDialogNode);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrSendInDialogSipRequests() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}

			if (pDialogNode->pAcceptedForkedMsgNode != pDialogNode->pCurrForkedMsgNode)
			{
				SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tBYE request sent within forked dialog, hence ignore the state change",
					__FUNCTION__, __LINE__);

				return error;
			}

			pUsageData->callState = EcrioSigMgrCallStateTerminationInProgress;
		}
		else
		{
			error = ECRIO_SIGMGR_INVALID_OPERATION;
			goto Error_Level_01;
		}
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************

Function:		EcrioSigMgrSendUpdate()

Purpose:		Create and Send UPDATE Request.

Description:	Construct and send UPDATE Request if call is not in
initial state. Otherwise terminate the call.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
Input:			SIGSESSIONHANDLE sessionHandle - handle to session
Input:			EcrioSigMgrByeRequestStruct* pUpdateReqInfo - UPDATE request structure.

Returns:		error code.
*****************************************************************************/
u_int32 EcrioSigMgrSendUpdate
(
	SIGMGRHANDLE sigMgrHandle,
	u_char *pCallId,
	EcrioSigMgrSipMessageStruct *pUpdateReqInfo
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;

	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	ec_MapGetKeyData(pSigMgr->hHashMap, pCallId, (void **)&pDialogNode);

	pUsageData = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
	if (NULL == pUsageData)
	{
		error = ECRIO_SIGMGR_INVALID_DATA;
		goto Error_Level_01;
	}

	if (((pSigMgr->bIPSecEnabled == Enum_TRUE) && (pSigMgr->pIPSecDetails->eIPSecState == ECRIO_SIP_IPSEC_STATE_ENUM_Established)) ||
		pSigMgr->pSignalingInfo->pSecurityVerify != NULL)
	{
		error = _EcrioSigMgrCreateSecurityVerifyHdr(pSigMgr, &pUpdateReqInfo->pOptionalHeaderList);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	/* Operation starts here */
	pUpdateReqInfo->eMethodType = EcrioSipMessageTypeUpdate;
	error = _EcrioSigMgrSendInDialogSipRequests(pSigMgr, pUpdateReqInfo, pDialogNode);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendInDialogSipRequests() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}


Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrHandlePrackResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pPrackResp,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	BoolEnum bRequestTimedout
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrMessageStruct messageInfo = { .msgCmd = EcrioSigMgrMessageNone };
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	BoolEnum bIsForked = Enum_FALSE;
	u_int32 responseCode = ECRIO_SIGMGR_INVALID_RESPONSE_CODE;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pCmnInfo->pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_BYE;

	if (bRequestTimedout != Enum_TRUE)
	{
		pDialogNode = EcrioSigMgrMatchDialogAndUpdateState(pSigMgr, pPrackResp, &bIsForked, &responseCode);
		if (pDialogNode == NULL)
		{
			error = ECRIO_SIGMGR_DIALOG_NOT_EXISTS;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSigMgrMatchDialogAndUpdateState() Dialog not exist, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}
	else
	{
		BoolEnum bDialogMatched = Enum_FALSE;
		pDialogNode = EcrioSigMgrFindMatchedDialog(pSigMgr,
			pCmnInfo->pFromTag, pCmnInfo->pToTag, pCmnInfo->pCallId, pCmnInfo->cSeq, pPrackResp->eMethodType,
			pPrackResp->eReqRspType, &bDialogMatched, &bIsForked, &responseCode);
		if (pDialogNode == NULL)
		{
			error = ECRIO_SIGMGR_INVITE_FAILED;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSigMgrFindMatchedDialog() Dialog not exist, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	pUsageData = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
	if (pUsageData == NULL)
	{
		error = ECRIO_SIGMGR_BYE_FAILED;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInvite usage not found () INVITE response dropped, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	messageInfo.msgCmd = EcrioSigMgrPrackResponseNotification;
	messageInfo.pData = (void *)pPrackResp;

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\t PRACK response Code=%d",
		__FUNCTION__, __LINE__, pPrackResp->responseCode);

	if ((pPrackResp->responseCode / 100) == ECRIO_SIGMGR_1XX_RESPONSE)
	{
		pPrackResp->statusCode = ECRIO_SIGMGR_REQUEST_IN_PROGRESS;
	}
	else
	{
		if ((pPrackResp->responseCode / 100) == ECRIO_SIGMGR_2XX_RESPONSE)
		{
			pPrackResp->statusCode = ECRIO_SIGMGR_RESPONSE_SUCCESS;
		}
	}

	if (_EcrioSigMgrGetModuleId(pSigMgr, pCmnInfo->pCallId) == EcrioSigMgrCallbackRegisteringModule_CPM)
	{
		_EcrioSigMgrUpdateStatusCPM(pSigMgr, &messageInfo);
	}
	else
	{
		_EcrioSigMgrUpdateStatusMoIP(pSigMgr, &messageInfo);
	}


Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* Handle BYE response. This function will resend request for authentication also. It also
* post the event to state machine handler.
*
*/
u_int32 _EcrioSigMgrHandleByeResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pByeResp,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	BoolEnum bRequestTimedout
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrMessageStruct messageInfo = { .msgCmd = EcrioSigMgrMessageNone };
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	BoolEnum bIsForked = Enum_FALSE;
	u_int32 responseCode = ECRIO_SIGMGR_INVALID_RESPONSE_CODE;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pCmnInfo->pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_BYE;

	if (bRequestTimedout != Enum_TRUE)
	{
		pDialogNode = EcrioSigMgrMatchDialogAndUpdateState(pSigMgr, pByeResp, &bIsForked, &responseCode);
		if (pDialogNode == NULL)
		{
			error = ECRIO_SIGMGR_DIALOG_NOT_EXISTS;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSigMgrMatchDialogAndUpdateState() Dialog not exist, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}
	else
	{
		BoolEnum bDialogMatched = Enum_FALSE;
		pDialogNode = EcrioSigMgrFindMatchedDialog(pSigMgr,
			pCmnInfo->pFromTag, pCmnInfo->pToTag, pCmnInfo->pCallId, pCmnInfo->cSeq, pByeResp->eMethodType,
			pByeResp->eReqRspType, &bDialogMatched, &bIsForked, &responseCode);
		if (pDialogNode == NULL)
		{
			error = ECRIO_SIGMGR_INVITE_FAILED;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSigMgrFindMatchedDialog() Dialog not exist, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
		/* Populate call-id in bye response structure as 408 was generated internally. */
		if (pByeResp->pMandatoryHdrs == NULL)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void**)&pByeResp->pMandatoryHdrs);
			if (pByeResp->pMandatoryHdrs == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			pal_MemorySet((void*)pByeResp->pMandatoryHdrs, 0, sizeof(EcrioSigMgrMandatoryHeaderStruct));
			pByeResp->pMandatoryHdrs->pCallId = pal_StringCreate((const u_char*)pCmnInfo->pCallId, pal_StringLength((const u_char*)pCmnInfo->pCallId));
		}
	}

	if (pDialogNode->pAcceptedForkedMsgNode != pDialogNode->pCurrForkedMsgNode)
	{
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tBYE response received for forked dialog, hence ignore the message",
			__FUNCTION__, __LINE__);

		return error;
	}

	pUsageData = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
	if (pUsageData == NULL)
	{
		error = ECRIO_SIGMGR_BYE_FAILED;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInvite usage not found () INVITE response dropped, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	messageInfo.msgCmd = EcrioSigMgrByeResponseNotification;
	messageInfo.pData = (void *)pByeResp;

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\t_BYE response Code=%d",
		__FUNCTION__, __LINE__, pByeResp->responseCode);

	if ((pByeResp->responseCode / 100) == ECRIO_SIGMGR_1XX_RESPONSE)
	{
		pByeResp->statusCode = ECRIO_SIGMGR_REQUEST_IN_PROGRESS;
	}
	else
	{
		if ((pByeResp->responseCode / 100) == ECRIO_SIGMGR_2XX_RESPONSE)
		{
			pByeResp->statusCode = ECRIO_SIGMGR_RESPONSE_SUCCESS;
		}

		if (pUsageData->callState == EcrioSigMgrCallStateTerminationInProgress)
		{
			pUsageData->callState = EcrioSigMgrCallStateUnInitialized;
			messageInfo.msgCmd = EcrioSigMgrByeResponseNotification;
		}
		else
		{
			error = ECRIO_SIGMGR_INVALID_OPERATION;
			goto Error_Level_01;
		}
	}

	if (_EcrioSigMgrGetModuleId(pSigMgr, pCmnInfo->pCallId) == EcrioSigMgrCallbackRegisteringModule_CPM)
	{
		_EcrioSigMgrUpdateStatusCPM(pSigMgr, &messageInfo);
	}
	else
	{
		_EcrioSigMgrUpdateStatusMoIP(pSigMgr, &messageInfo);
	}

Error_Level_01:

	if ((pDialogNode != NULL) && (pUsageData != NULL && pUsageData->callState == EcrioSigMgrCallStateUnInitialized))
	{
		/*EcrioSigMgrMessageStruct				message = { 0 };
		message.msgCmd = EcrioSigMgrDeleteSessionNotification;
		message.pData = (void*)pSessionHandleInfo;

		#ifdef _ENABLE_DEFAULT_CALL_HANDLER_CPM_
		_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
		#else
		_EcrioSigMgrUpdateStatusMoIP(pSigMgr, &message);
		#endif
		                */
		{
			/*Routing support*/
			//ec_SigMgrHashMap_Delete(pSigMgr->hHashMap, pCmnInfo->pCallId);
		}
		_EcrioSigMgrDeleteDialog(pSigMgr, pDialogNode);

		_EcrioSigMgrInternalStructRelease(pSigMgr, EcrioSigMgrStructType_InviteUsageInfo,
			(void **)&pUsageData, Enum_TRUE);

		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tDialog deleted", __FUNCTION__, __LINE__);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* Handle CANCEL response. This function will resend request for authentication also. It also
* post the event to state machine handler.
*
*/
u_int32 _EcrioSigMgrHandleCancelResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pCancelResp,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	BoolEnum bRequestTimedout
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrMessageStruct message = { .msgCmd = EcrioSigMgrMessageNone };
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	BoolEnum bIsForked = Enum_FALSE;
	u_int32 responseCode = ECRIO_SIGMGR_INVALID_RESPONSE_CODE;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pCmnInfo->pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_CANCEL;

	if (bRequestTimedout != Enum_TRUE)
	{
		pDialogNode = EcrioSigMgrMatchDialogAndUpdateState(pSigMgr, pCancelResp, &bIsForked, &responseCode);
		if (pDialogNode == NULL)
		{
			error = ECRIO_SIGMGR_DIALOG_NOT_EXISTS;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSigMgrMatchDialogAndUpdateState() Dialog not exist, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}
	else
	{
		BoolEnum bDialogMatched = Enum_FALSE;
		pDialogNode = EcrioSigMgrFindMatchedDialog(pSigMgr,
			pCmnInfo->pFromTag, pCmnInfo->pToTag, pCmnInfo->pCallId, pCmnInfo->cSeq, pCancelResp->eMethodType,
			pCancelResp->eReqRspType, &bDialogMatched, &bIsForked, &responseCode);
		if (pDialogNode == NULL)
		{
			error = ECRIO_SIGMGR_INVITE_FAILED;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSigMgrFindMatchedDialog() Dialog not exist, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	if (bIsForked == Enum_FALSE) 
	{
		message.msgCmd = EcrioSigMgrCancelResponseNotification;
		message.pData = (void *)pCancelResp;

		if (_EcrioSigMgrGetModuleId(pSigMgr, pCmnInfo->pCallId) == EcrioSigMgrCallbackRegisteringModule_CPM)
		{
			_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
		}
		else
		{
			_EcrioSigMgrUpdateStatusMoIP(pSigMgr, &message);
		}
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrHandleUACEventInitialized()

Purpose:		Responsible for doing actions which are required fro State transition from
                UnInittialized to idle.

Description:	Creates invite request and stores in Invite usage info object

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
Input:			EcrioSigMgrSipMessageStruct* pInviteReq  - Invite Request Info.
Input:			_EcrioSigMgrInviteUsageInfoStruct* pInviteUsage  - Invite usage info object.

Input / OutPut:	None.

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrHandleUACEventInitialized
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pInviteReq,
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrSipMessageStruct *pTempInviteReq = NULL;
	EcrioSigMgrMandatoryHeaderStruct *pTempMandatoryHdrs = NULL;
	EcrioSigMgrUriStruct *pReqUri = NULL;
	u_char *ppAcceptList[] = { (u_char *)"application/sdp", (u_char *)"application/3gpp-ims+xml" };
	u_char *ppPrivacy[] = {0};

	BoolEnum bIsTimerSupported = Enum_TRUE;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	/* create temporary invite request object so that it can be used in case of 401 / 407 response.*/
	pal_MemoryAllocate(sizeof(EcrioSigMgrSipMessageStruct), (void **)&pTempInviteReq);

	if (pTempInviteReq == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for pTempInviteReq, error=%u",
			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	error = _EcrioSigMgrCopySipMessage(pSigMgr, pInviteReq, pTempInviteReq);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrCopyInviteRequest() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	pTempInviteReq->pRouteSet = _EcrioSigMgrAddRouteHeader(pSigMgr);

	error = _EcrioSigMgrAppendServiceRouteToRouteSet(pSigMgr,
		&pTempInviteReq->pRouteSet);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAppendServiceRouteToRouteSet() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	if (pTempInviteReq->ppPPreferredIdentity == NULL)
	{
		//add P-PreferredIdentity header 
		error = _EcrioSigMgrAddP_PreferrdIdHeader(pSigMgr, pTempInviteReq);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddP_preferredIdHeader() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		if (pSigMgr->pOOMObject->ec_oom_HasUserPhoneSupport(EcrioSipMessageTypeInvite, EcrioSipHeaderTypeP_PreferredIdentity, EcrioSipURIType_SIP) == Enum_TRUE)
		{
			/* All outgoing SIP uri which contain telephone number in user info part must have "user=phone" parameter as SIP uri parameter.	*/
			error = _EcrioSigMgrAddUserPhoneParam(pSigMgr, &pTempInviteReq->ppPPreferredIdentity[0]->addrSpec);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddUserPhoneParam, error=%u", __FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}
	}
		
	if (pTempInviteReq->pContact == NULL)
	{
		error = _EcrioSigMgrAddLocalContactUri(pSigMgr, &pTempInviteReq->pContact, pTempInviteReq->bPrivacy);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddLocalContactUri() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		if (pSigMgr->pOOMObject->ec_oom_HasUserPhoneSupport(EcrioSipMessageTypeInvite, EcrioSipHeaderTypeContact, EcrioSipURIType_SIP) == Enum_TRUE)
		{
			/* All outgoing SIP uri which contain telephone number in user info part must have "user=phone" parameter as SIP uri parameter.	*/
			error = _EcrioSigMgrAddUserPhoneParam(pSigMgr, &pTempInviteReq->pContact->ppContactDetails[0]->nameAddr.addrSpec);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddUserPhoneParam, error=%u", __FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}
	}

	if (pTempInviteReq->pOptionalHeaderList == NULL)
	{
		if (pInviteReq->pOptionalHeaderList != NULL)
		{
			pTempInviteReq->pOptionalHeaderList = pInviteReq->pOptionalHeaderList;
			pInviteReq->pOptionalHeaderList = NULL;
		}
		else
		{
			error = EcrioSigMgrCreateOptionalHeaderList(pSigMgr, &pTempInviteReq->pOptionalHeaderList);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tEcrioSigMgrCreateOptionalHeaderList() error=%u",
					__FUNCTION__, __LINE__, error);

				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
		}
	}

	/* User-Agent support */
	if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeInvite, EcrioSipHeaderTypeUserAgent))
	{
		error = _EcrioSigMgrAddUserAgentHeader(pSigMgr, pTempInviteReq->pOptionalHeaderList);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddUserAgentHeader() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	error = _EcrioSigMgrAddAllowHeader(pSigMgr, pTempInviteReq->pOptionalHeaderList);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddAllowHeader() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	//add Privacy header if provide privacy to initial INVITE.
	if (pTempInviteReq->bPrivacy == Enum_TRUE)
	{
		ppPrivacy[0] =  (u_char *)"id" ;
		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempInviteReq->pOptionalHeaderList, EcrioSipHeaderTypePrivacy, \
			(sizeof(ppPrivacy) / sizeof(u_char *)), ppPrivacy, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}
	error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempInviteReq->pOptionalHeaderList, EcrioSipHeaderTypeAccept,\
		(sizeof(ppAcceptList) / sizeof(u_char *)), ppAcceptList, 0, NULL, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	//adding Converstion id and contribution id to optional headers
	// error checking not required as it is an optional header.
#ifdef ENABLE_RCS
	 _EcrioSigMgrAddConversationsIDHeader(pSigMgr, pTempInviteReq);
#endif //ENABLE_RCS
	if (pSigMgr->pOOMObject)
	{
		if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeInvite, EcrioSipHeaderTypeP_AccessNetworkInfo))
		{
			error = _EcrioSigMgrAddP_AccessNWInfoHeader(pSigMgr, pTempInviteReq->pOptionalHeaderList);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddP_AccessNWInfoHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}

		if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeInvite, EcrioSipHeaderTypeP_LastAccessNetworkInfo))
		{
			error = _EcrioSigMgrAddP_LastAccessNWInfoHeader(pSigMgr, pTempInviteReq->pOptionalHeaderList);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tEcrioSipHeaderTypeP_LastAccessNetworkInfo() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}

		if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeInvite, EcrioSipHeaderTypeAcceptContact))
		{
			error = _EcrioSigMgrAddAccptContactHeader(pSigMgr, pTempInviteReq);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddAccptContactHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}

		if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeInvite, EcrioSipHeaderTypeP_PreferredService))
		{
			error = _EcrioSigMgrAddP_PreferredServiceHeader(pSigMgr, pTempInviteReq);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddP_PreferredServiceHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}

		if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeInvite, EcrioSipHeaderTypeP_EarlyMedia))
		{
			/* P Early Media Header*/
			error = _EcrioSigMgrAddP_EarlyMediaHeader(pSigMgr, pTempInviteReq);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t__EcrioSigMgrAddP_EarlyMediaHeader error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}
				
	}

	if (bIsTimerSupported == Enum_TRUE)
	{
		pInviteUsage->isSessionRefresh = Enum_TRUE;
		pInviteUsage->isRefresher = Enum_TRUE;
	}
	else
	{
		pInviteUsage->isSessionRefresh = Enum_FALSE;
		pInviteUsage->isRefresher = Enum_FALSE;
	}

	/* Copy supported and required optional header*/
	if (pInviteReq->pOptionalHeaderList != NULL)
	{
		EcrioSigMgrHeaderStruct *pHeader = NULL;
		EcrioSigMgrGetOptionalHeader(pSigMgr, pInviteReq->pOptionalHeaderList, EcrioSipHeaderTypeSupported, &pHeader);
		if (pHeader != NULL)
			_EcrioSigMgrInsertOptionalHeader(pSigMgr, pTempInviteReq->pOptionalHeaderList, pHeader);

		EcrioSigMgrGetOptionalHeader(pSigMgr, pInviteReq->pOptionalHeaderList, EcrioSipHeaderTypeRequire, &pHeader);
		if (pHeader != NULL)
			_EcrioSigMgrInsertOptionalHeader(pSigMgr, pTempInviteReq->pOptionalHeaderList, pHeader);
	}

	if (pTempInviteReq->pMandatoryHdrs->pRequestUri == NULL)
	{
		pTempMandatoryHdrs = pTempInviteReq->pMandatoryHdrs;
		pReqUri = &pTempMandatoryHdrs->pTo->nameAddr.addrSpec;

		/* TODO: Support strict routing */
		pal_MemoryAllocate(sizeof(EcrioSigMgrUriStruct), (void **)&pTempMandatoryHdrs->pRequestUri);
		if (pTempMandatoryHdrs->pRequestUri == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pTempMandatoryHdrs->pRequestUri, error=%u",
				__FUNCTION__, __LINE__, error);
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		error = _EcrioSigMgrPopulateUri(pSigMgr, pReqUri,
			pTempMandatoryHdrs->pRequestUri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrPopulateUri() populate request uri, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
#if 0 /* Disable Adding user=phone as per ATT IODT */
		/* All outgoing SIP uri which contain telephone number in user info part must have "user=phone" parameter as SIP uri parameter.	*/
		error = _EcrioSigMgrAddUserPhoneParam(pSigMgr, pTempMandatoryHdrs->pRequestUri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddUserPhoneParam, error=%u", __FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
#endif
	}

	pInviteUsage->pInviteReq = pTempInviteReq;
	pInviteUsage->isCaller = Enum_TRUE;
	error = ECRIO_SIGMGR_NO_ERROR;

Error_Level_01:

	if ((error != ECRIO_SIGMGR_NO_ERROR) && (pTempInviteReq != NULL))
	{
		EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_SipMessage, (void **)&pTempInviteReq,
			Enum_TRUE);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrHandleUACEventSendInvite()

Purpose:		Responsible for doing actions which are required fro State transition from
                idle to init.

Description:	Sends invite request to the socket.

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
Input:			BoolEnum override - if Enum_TRUE send override message callback to upper layer.
Input:			_EcrioSigMgrInviteUsageInfoStruct* pInviteUsage  - Invite usage info object.

Input / OutPut:	EcrioSigMgrSessionHandleStruct** pSessionHandle - session handle.

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrHandleUACEventSendInvite
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage,
	u_char** ppCallID
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrSipMessageStruct *pTempInviteReq = NULL;
	EcrioSigMgrMandatoryHeaderStruct *pMandatoryHdrs = NULL;
	u_int32 reqLen = 0;
	u_char *pReqData = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	EcrioSigMgrCommonInfoStruct cmnInfo = { 0 };
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };
	// EcrioSigMgrUriStruct *pReqUri = NULL;
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pTempInviteReq = pInviteUsage->pInviteReq;
	pMandatoryHdrs = pTempInviteReq->pMandatoryHdrs;
	// pReqUri = pTempInviteReq->pMandatoryHdrs->pRequestUri;

	if (pInviteUsage->callState != EcrioSigMgrCallStateCallEstablished)
	{
		++(pTempInviteReq->pMandatoryHdrs->CSeq);
	}

	error = _EcrioSigMgrCopyCredentials(pSigMgr,
		(u_char *)ECRIO_SIG_MGR_METHOD_INVITE,
		NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrCopyCredentials() for INVITE request, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	if (pMandatoryHdrs->ppVia && pMandatoryHdrs->ppVia[0]
		&& pMandatoryHdrs->ppVia[0]->pBranch == NULL)
	{
		error = _EcrioSigMgrGenerateBranchParam(pSigMgr, &(pMandatoryHdrs->ppVia[0]->pBranch));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrGenerateBranchParam() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	pTempInviteReq->eMethodType = EcrioSipMessageTypeInvite;
	pTempInviteReq->eReqRspType = EcrioSigMgrSIPRequest;

	error = _EcrioSigMgrPopulateMandatoryHeaders(pSigMgr, pTempInviteReq,
		EcrioSigMgrSIPRequest, Enum_FALSE, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateMandatoryHeaders() for pIMSManHdrs, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	error = _EcrioSigMgrPopulateCommonInfo(pSigMgr, pMandatoryHdrs, &cmnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateCommonInfo() error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	if (pSigMgr->pOOMObject)
	{
		if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeInvite, EcrioSipHeaderTypeSessionExpires))
		{
			/* Session Expires Header*/
			error = _EcrioSigMgrAddSessionExpiresHeader(pSigMgr, pTempInviteReq->pOptionalHeaderList, 
				pInviteUsage->isSessionRefresh, pInviteUsage->isRefresher, pSigMgr->pSignalingInfo->sessionExpires);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t__EcrioSigMgrAddSessionExpiresHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}

		if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeInvite, EcrioSipHeaderTypeMinSE))
		{
			if (pSigMgr->pSignalingInfo->minSe != 0)
			{
				error = _EcrioSigMgrAddMinSEHeader(pSigMgr, pTempInviteReq->pOptionalHeaderList, pSigMgr->pSignalingInfo->minSe);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					goto Error_Level_01;
				}
			}
		}
	}		

	if (((pSigMgr->bIPSecEnabled == Enum_TRUE) && (pSigMgr->pIPSecDetails->eIPSecState == ECRIO_SIP_IPSEC_STATE_ENUM_Established)) ||
		pSigMgr->pSignalingInfo->pSecurityVerify != NULL)
	{
		u_char *pRequire = (u_char *)ECRIO_SIG_MGR_HEADER_VALUE_SEC_AGREE;
		error = _EcrioSigMgrCreateSecurityVerifyHdr(pSigMgr, &pTempInviteReq->pOptionalHeaderList);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}

		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempInviteReq->pOptionalHeaderList, EcrioSipHeaderTypeRequire, 1, &pRequire, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}
		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempInviteReq->pOptionalHeaderList, EcrioSipHeaderTypeProxyRequire, 1, &pRequire, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}
	}

	/* Construct Message request */
	error = _EcrioSigMgrSipMessageForm(pSigMgr, EcrioSigMgrSipMessageModeFullName, pTempInviteReq, &pReqData, &reqLen);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSipMessageForm() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_IMS_LIB_ERROR;
		goto Error_Level_01;
	}
	if (pSigMgr->pSignalingInfo->bEnableTls == Enum_TRUE && pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex != 0)
	{
		eTransportType = EcrioSigMgrTransportTLS;
	}
	else if ((pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE) && (reqLen > pSigMgr->pSignalingInfo->uUdpMtu))
	{
		error = _EcrioSigMgrCreateTCPBasedBuffer(pSigMgr, &pReqData, EcrioSigMgrSIPRequest);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrCreateTCPBasedBuffer() error=%u",
				__FUNCTION__, __LINE__, error);

			error = ECRIO_SIGMGR_IMS_LIB_ERROR;
			goto Error_Level_01;
		}

		eTransportType = EcrioSigMgrTransportTCP;
	}
	else if (pSigMgr->pSignalingInfo->bEnableUdp == Enum_FALSE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE)
	{
		eTransportType = EcrioSigMgrTransportTCP;
	}


	cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_INVITE;
	cmnInfo.role = EcrioSigMgrRoleUAC;

	txnInfo.currentContext = ECRIO_SIGMGR_INVITE_REQUEST;
	txnInfo.retransmitContext = ECRIO_SIGMGR_INVITE_REQUEST_RESEND;
	txnInfo.deleteflag = Enum_FALSE;
	txnInfo.messageType = EcrioSigMgrMessageInviteRequest;
	txnInfo.transport = eTransportType;
	txnInfo.pMsg = pReqData;
	txnInfo.msglen = reqLen;

	txnInfo.pSessionMappingStr = pTempInviteReq->pMandatoryHdrs->pCallId;

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeSIP,
		"%s:%u\tOutgoing SIP message constructed:", __FUNCTION__, __LINE__);
	SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pReqData, reqLen);

	if (NULL != pInviteUsage->pBranch)
	{
		pal_MemoryFree((void **)&pInviteUsage->pBranch);
	}

	pInviteUsage->pBranch = cmnInfo.pBranch;

	pDialogNode = _EcrioSigMgrCreateDialog(pSigMgr, pTempInviteReq, Enum_FALSE, pInviteUsage);
	if (pDialogNode == NULL)
	{
		error = ECRIO_SIGMGR_INVITE_FAILED;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrCreateDialog() error=%u",
			__FUNCTION__, __LINE__, error);
		pInviteUsage->pBranch = NULL;

		goto Error_Level_01;
	}

	ec_MapInsertData(pSigMgr->hHashMap, pDialogNode->pCallId, (void*)pDialogNode);
	*ppCallID = pDialogNode->pCallId;

	error = _EcrioSigMgrSendMsgToTXNHandler(pSigMgr, &cmnInfo, &txnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendMsgToTXNHandler() error=%u",
			__FUNCTION__, __LINE__, error);
		/* Resolving crash issue in the error condition */
		pInviteUsage->pBranch = NULL;

		error = ECRIO_SIGMGR_INVITE_FAILED;
		goto Error_Level_01;
	}

	cmnInfo.pBranch = NULL;

	/* Add session expire timer */
	// {
	// if ( NULL == pSigMgr->pSignalingInfo)
	// {
	// error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
	// SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
	// "%s:%u\tpSigMgr->pSignalingInfo is NULL , error=%u",
	// __FUNCTION__, __LINE__, error);
	// goto Error_Level_01;

	// }

	// if (pSigMgr->pSignalingInfo->sessionExipire)
	// {
	/// * keep reference to Sig Mgr
	// */
	// pInviteUsage->pSigMgr = pSigMgr;

	/// * Setup timer*/
	// timerConfig.uPeriodicInterval = 0;
	// timerConfig.pCallbackData = (void *)pTempSessionHandle;
	// timerConfig.bEnableGlobalMutex = Enum_TRUE;
	// timerConfig.uInitialInterval = (pSigMgr->pSignalingInfo->sessionExipire)*1000; /* seconds  to milliseconds*/
	// timerConfig.timerCallback = _EcrioSigMgrCallStateSessionExpireTimerCallback;

	/// * Start session expire timer
	// */
	// SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStarting Session Expire Timer ... in state %u for interval %u ",
	// __FUNCTION__, __LINE__, pInviteUsage->callState, timerConfig.uInitialInterval);
	// uPFDerror = pal_TimerStart(pSigMgr->pal, &timerConfig, &pInviteUsage->sessionExpireTimerId);
	// if (KPALErrorNone != uPFDerror)
	// {
	// error = ECRIO_SIGMGR_TIMER_ERROR;
	// goto Error_Level_01;
	// }
	// }

	// }

	if (pInviteUsage->pAckMsg != NULL)
	{
		pal_MemoryFree((void **)&pInviteUsage->pAckMsg);
	}

	/*Module routing support*/
	_EcrioSigMgrStoreModuleRoutingInfo(pSigMgr, pTempInviteReq->pMandatoryHdrs->pCallId, pTempInviteReq->eModuleId);

Error_Level_01:

	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);
	if (pReqData != NULL)
	{
		pal_MemoryFree((void **)&pReqData );
		pReqData = NULL;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrHandleUACEventSendReInvite
(
	EcrioSigMgrStruct* pSigMgr, 
	EcrioSigMgrSipMessageStruct *pInviteReq, 
	_EcrioSigMgrDialogNodeStruct *pDialogNode
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 reqLen = 0;
	u_int32 uSessionExpireValue = 0;
	u_char *pReqData = NULL;
	EcrioSigMgrCommonInfoStruct cmnInfo = { 0 };
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };
	EcrioSigMgrSipMessageStruct *pTempSipRequest = NULL;
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;
	_EcrioSigMgrInviteUsageInfoStruct* pInviteUsage = NULL;
	
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct*)pDialogNode->pDialogContext;

	cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_INVITE;
	cmnInfo.role = EcrioSigMgrRoleUAC;

	pal_MemoryAllocate(sizeof(EcrioSigMgrSipMessageStruct), (void **)&pTempSipRequest);
	if (pTempSipRequest == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for pTempSipRequest, error=%u",
			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_NO_ERROR;
		goto Error_Level_01;
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&pTempSipRequest->pMandatoryHdrs);
	if (pTempSipRequest->pMandatoryHdrs == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for pTempSipRequest->pMandatoryHdrs, error=%u",
			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_NO_ERROR;
		goto Error_Level_01;
	}

	pTempSipRequest->eMethodType = EcrioSipMessageTypeInvite;
	pTempSipRequest->eReqRspType = EcrioSigMgrSIPRequest;

	error = _EcrioSigMgePopulateHeaderFromDialogAndUpdateState(pSigMgr,
		pTempSipRequest, pDialogNode);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgePopulateHeaderFromDialogAndUpdateState() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}

	error = _EcrioSigMgrPopulateMandatoryHeaders(pSigMgr, pTempSipRequest,
		EcrioSigMgrSIPRequest, Enum_FALSE, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateMandatoryHeaders() for pIMSManHdrs, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	error = _EcrioSigMgrUpdateMessageRequestStruct(pSigMgr, pTempSipRequest, Enum_FALSE);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tOptional header creation, error=%u",
			__FUNCTION__, __LINE__, error);

		/*error  = ECRIO_SIGMGR_MESSAGE_FAILED;*/   /*return proper error code*/
		goto Error_Level_02;
	}		


	/* Use message body from upper layer - This is a case of session updates. */
	if (pInviteReq != NULL)
		pTempSipRequest->pMessageBody = pInviteReq->pMessageBody;

	if (pTempSipRequest->pMessageBody == NULL) /* If no message body received, use stored message body from INVITE/200OK - This is a case of session refresh. */
		pTempSipRequest->pMessageBody = pInviteUsage->pInviteReq->pMessageBody;

	if (pTempSipRequest->pRouteSet == NULL && pInviteUsage->pInviteReq->pRouteSet != NULL)
	{
		/*if (pTempSipRequest->pRouteSet != NULL)
			_EcrioSigMgrReleaseRouteStruct(pSigMgr, pTempSipRequest->pRouteSet);
		else*/
			pal_MemoryAllocate(sizeof(EcrioSigMgrRouteStruct), (void **)&pTempSipRequest->pRouteSet);

		if (pTempSipRequest->pRouteSet == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pOptionalHdrs->pRouteSet, error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}

		error = _EcrioSigMgrCopyRouteSet(pSigMgr, (u_int16)pInviteUsage->pInviteReq->pRouteSet->numRoutes,
			pInviteUsage->pInviteReq->pRouteSet->ppRouteDetails,
			&pTempSipRequest->pRouteSet->ppRouteDetails);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrCopyRouteSet() for pOptionalHdrs->pRouteSet->ppRouteDetails, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		pTempSipRequest->pRouteSet->numRoutes = pInviteUsage->pInviteReq->pRouteSet->numRoutes;
	}

#if 0
	if (pInviteUsage->pInviteReq->pMandatoryHdrs != NULL &&
		pInviteUsage->pInviteReq->pMandatoryHdrs->ppVia != NULL)
	{
		u_int32 i = 0;

		/* Free Via (if non-null) & Overwrite from dialog information
		*/
		if (pTempSipRequest->pMandatoryHdrs->ppVia != NULL)
		{
			u_int32 i = 0;

			for (i = 0; i < pTempSipRequest->pMandatoryHdrs->numVia; ++i)
			{
				_EcrioSigMgrReleaseViaStruct(pSigMgr, pTempSipRequest->pMandatoryHdrs->ppVia[i]);
				pal_MemoryFree((void **)&(pTempSipRequest->pMandatoryHdrs->ppVia[i]));
			}

			pal_MemoryFree((void **)&(pTempSipRequest->pMandatoryHdrs->ppVia));
		}

		/* Overwrite Via from dialog information
		*/
		pal_MemoryAllocate(sizeof(EcrioSigMgrViaStruct *) * (pInviteUsage->pInviteReq->pMandatoryHdrs->numVia), (void **)&pTempSipRequest->pMandatoryHdrs->ppVia);
		if (pTempSipRequest->pMandatoryHdrs->ppVia == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pTempSipRequest->pMandatoryHdrs->numVia = pInviteUsage->pInviteReq->pMandatoryHdrs->numVia;

		for (i = 0; i < pTempSipRequest->pMandatoryHdrs->numVia; ++i)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrViaStruct), (void **)&pTempSipRequest->pMandatoryHdrs->ppVia[i]);
			if (pTempSipRequest->pMandatoryHdrs->ppVia[i] == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrPopulateViaHdr(pSigMgr, \
				pInviteUsage->pInviteReq->pMandatoryHdrs->ppVia[i], \
				pTempSipRequest->pMandatoryHdrs->ppVia[i]);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error_Level_01;
			}
		}
	}
#endif

	/* As single boolean variable used for adding header and starting session timer, 
	we are seeing UAC and UAS alternating to refresh the session. */
	if (pSigMgr->pOOMObject)
	{
		if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeInvite, EcrioSipHeaderTypeSessionExpires))
		{
			/* Use incoming session expire value if its greater than minSe. Otherwise fallback on
			local session expire value. */
			if (pInviteUsage->sessionExpireVal > 0 &&
				pInviteUsage->sessionExpireVal > pSigMgr->pSignalingInfo->minSe)
				uSessionExpireValue = pInviteUsage->sessionExpireVal;
			else
				uSessionExpireValue = pSigMgr->pSignalingInfo->sessionExpires;
			/* Session Expires Header*/
			error = _EcrioSigMgrAddSessionExpiresHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList, pInviteUsage->isSessionRefresh, pInviteUsage->isRefresher, uSessionExpireValue);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t__EcrioSigMgrAddSessionExpiresHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}
	}

	if (((pSigMgr->bIPSecEnabled == Enum_TRUE) && (pSigMgr->pIPSecDetails->eIPSecState == ECRIO_SIP_IPSEC_STATE_ENUM_Established)) ||
		pSigMgr->pSignalingInfo->pSecurityVerify != NULL)
	{
		u_char *pRequire = (u_char *)ECRIO_SIG_MGR_HEADER_VALUE_SEC_AGREE;

		error = _EcrioSigMgrCreateSecurityVerifyHdr(pSigMgr, &pTempSipRequest->pOptionalHeaderList);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_02;
		}
		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList, EcrioSipHeaderTypeRequire, 1, &pRequire, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_02;
		}
		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList, EcrioSipHeaderTypeProxyRequire, 1, &pRequire, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_02;
		}
	}

	if (pSigMgr->pOOMObject)
	{
		if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeInvite, EcrioSipHeaderTypeMinSE))
		{
			if (pSigMgr->pSignalingInfo->minSe != 0)
			{
				error = _EcrioSigMgrAddMinSEHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList, pSigMgr->pSignalingInfo->minSe);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					goto Error_Level_01;
				}
			}
		}
	}

	error = _EcrioSigMgrSipMessageForm(pSigMgr, EcrioSigMgrSipMessageModeFullName, pTempSipRequest, &pReqData, &reqLen);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrCreateByeReq() error=%u",
			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_BYE_FAILED;
		goto Error_Level_02;
	}

	if (pSigMgr->pSignalingInfo->bEnableTls == Enum_TRUE && pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex != 0)
	{
		eTransportType = EcrioSigMgrTransportTLS;
	}
	else if ((pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE) && (reqLen > pSigMgr->pSignalingInfo->uUdpMtu))
	{
		error = _EcrioSigMgrCreateTCPBasedBuffer(pSigMgr, &pReqData, EcrioSigMgrSIPRequest);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrCreateTCPBasedBuffer() error=%u",
				__FUNCTION__, __LINE__, error);

			error = ECRIO_SIGMGR_IMS_LIB_ERROR;
			goto Error_Level_01;
		}
		eTransportType = EcrioSigMgrTransportTCP;
	}
	else if (pSigMgr->pSignalingInfo->bEnableUdp == Enum_FALSE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE)
	{
		eTransportType = EcrioSigMgrTransportTCP;
	}

	cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_INVITE;
	cmnInfo.role = EcrioSigMgrRoleUAC;

	txnInfo.currentContext = ECRIO_SIGMGR_INVITE_REQUEST;
	txnInfo.retransmitContext = ECRIO_SIGMGR_INVITE_REQUEST_RESEND;
	txnInfo.deleteflag = Enum_FALSE;
	txnInfo.messageType = EcrioSigMgrMessageInviteRequest;
	txnInfo.transport = eTransportType;
	txnInfo.pMsg = pReqData;
	txnInfo.msglen = reqLen;

	txnInfo.pSessionMappingStr = pDialogNode->pCallId;

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeSIP,
		"%s:%u\tOutgoing SIP message constructed:", __FUNCTION__, __LINE__);
	SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pReqData, reqLen);

	if (NULL != pInviteUsage->pBranch)
	{
		pal_MemoryFree((void **)&pInviteUsage->pBranch);
	}

	pInviteUsage->pBranch = pal_StringCreate(pTempSipRequest->pMandatoryHdrs->ppVia[0]->pBranch, pal_StringLength(pTempSipRequest->pMandatoryHdrs->ppVia[0]->pBranch));
	cmnInfo.pBranch = pInviteUsage->pBranch;
	cmnInfo.pCallId = pDialogNode->pCallId;

	error = _EcrioSigMgrSendMsgToTXNHandler(pSigMgr, &cmnInfo, &txnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendMsgToTXNHandler() error=%u",
			__FUNCTION__, __LINE__, error);
		/* Resolving crash issue in the error condition */
		pInviteUsage->pBranch = NULL;

		error = ECRIO_SIGMGR_INVITE_FAILED;
		goto Error_Level_01;
	}

	cmnInfo.pBranch = NULL;
	cmnInfo.pCallId = NULL;

	if (pReqData != NULL)
	{
		pal_MemoryFree((void **)&pReqData);
		pReqData = NULL;
	}

Error_Level_02:
Error_Level_01 :
	cmnInfo.pMethodName = NULL;
	cmnInfo.pBranch = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);
	pTempSipRequest->pMessageBody = NULL;
	if (pTempSipRequest)
	{
		_EcrioSigMgrReleaseSipMessage(pSigMgr, pTempSipRequest);
		pal_MemoryFree((void **)&pTempSipRequest);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* Create request and send it to network.
*/
u_int32 _EcrioSigMgrSendInDialogSipRequests
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipReqInfo,
	_EcrioSigMgrDialogNodeStruct *pDialogNode
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 reqLen = 0;
	u_char *pReqData = NULL;
	/* EcrioSigMgrURISchemesEnum reqUriScheme = EcrioSigMgrURISchemeNone; */
	EcrioSigMgrUriStruct reqUri = { .uriScheme = EcrioSigMgrURISchemeNone };
	EcrioSigMgrCommonInfoStruct cmnInfo = { 0 };
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };
	EcrioSigMgrSipMessageStruct *pTempSipRequest = NULL;
	u_int32 i = 0;
	u_int32 uSize = 0;
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;
	u_char *pRackHeader = NULL;
	u_int32	uRackHdrLen = 0;
	u_char RseqStr[11] = { 0 }, CseqStr[11] = { 0 };
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;
	_EcrioSigMgrSubscribeUsageInfoStruct *pSubscribeUsage = NULL;
	u_int32	uEventHdrCount = 0, uAcceptHdrCount = 0;
	u_int32 uSessionExpireValue = 0;
	u_int32 uMinSEValue = 0;
	u_char **ppEventHdrStr = NULL;
	u_char **ppAcceptHdrStr = NULL;
	u_char **ppPPreferredServiceHdrStr = NULL;
	u_char ** ppAcceptContactHdrStr = NULL;
	EcrioSigMgrHeaderStruct *pHeader = NULL;
	u_int32 uLength = 0;
	u_int16 uRequireHdrCount = 0;
	u_char *pReferToHdr = NULL;
	u_char *pReferredByHdr = NULL;
	u_char *pReferSubHdr = NULL;
	u_char **ppRequireHdr = NULL;
	u_char *pContentIdHdr = NULL;
	u_char *pContentDispositionHdr = NULL;
	u_char *pEvent = NULL;
	u_char *pSupported = NULL;
	u_char *pSubject = NULL;
	u_char *pFromUri = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pSipReqInfo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tpSipReqInfo is NULL",
				   __FUNCTION__, __LINE__);
		return ECRIO_SIGMGR_INVALID_DATA;
	}

	if (pSipReqInfo->eMethodType == EcrioSipMessageTypeCancel)
	{
		cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_CANCEL;
	}
	else if (pSipReqInfo->eMethodType == EcrioSipMessageTypeBye)
	{
		cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_BYE;
	}
	else if (pSipReqInfo->eMethodType == EcrioSipMessageTypePrack)
	{
		cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_PRACK;
	}
	else if (pSipReqInfo->eMethodType == EcrioSipMessageTypeSubscribe)
	{
		cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_SUBSCRIBE;
	}
	else if (pSipReqInfo->eMethodType == EcrioSipMessageTypeUpdate)
	{
		cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_UPDATE;
	}
	else if (pSipReqInfo->eMethodType == EcrioSipMessageTypeRefer)
	{
		cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_REFER;
	}

	cmnInfo.role = EcrioSigMgrRoleUAC;

	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrSipMessageStruct), (void **)&pTempSipRequest);
		if (pTempSipRequest == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pTempSipRequest, error=%u",
				__FUNCTION__, __LINE__, error);
			error = ECRIO_SIGMGR_NO_ERROR;
			goto Error_Level_01;
		}

		pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&pTempSipRequest->pMandatoryHdrs);
		if (pTempSipRequest->pMandatoryHdrs == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pTempSipRequest->pMandatoryHdrs, error=%u",
				__FUNCTION__, __LINE__, error);
			error = ECRIO_SIGMGR_NO_ERROR;
			goto Error_Level_01;
		}

		pTempSipRequest->eMethodType = pSipReqInfo->eMethodType;
		pTempSipRequest->eReqRspType = EcrioSigMgrSIPRequest;

		if (pSipReqInfo && pSipReqInfo->pMandatoryHdrs)
		{
			error = _EcrioSigMgrPopulateManHdrs(pSigMgr, pSipReqInfo->pMandatoryHdrs,
				pTempSipRequest->pMandatoryHdrs);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrPopulateManHdrs() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}
		}

		error = _EcrioSigMgePopulateHeaderFromDialogAndUpdateState(pSigMgr,
			pTempSipRequest, pDialogNode);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgePopulateHeaderFromDialogAndUpdateState() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_02;
		}

		error = _EcrioSigMgrPopulateMandatoryHeaders(pSigMgr, pTempSipRequest,
			EcrioSigMgrSIPRequest, Enum_FALSE, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrPopulateMandatoryHeaders() for pIMSManHdrs, error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}

		if ((pSipReqInfo != NULL) && (pSipReqInfo->pOptionalHeaderList != NULL))
		{
			pTempSipRequest->pOptionalHeaderList = pSipReqInfo->pOptionalHeaderList;
		}
		else
		{
			error = _EcrioSigMgrUpdateMessageRequestStruct(pSigMgr, pTempSipRequest, Enum_FALSE);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tOptional header creation, error=%u",
					__FUNCTION__, __LINE__, error);

				/*error  = ECRIO_SIGMGR_MESSAGE_FAILED;*/   /*return proper error code*/
				goto Error_Level_02;
			}
		}

		if (pSipReqInfo->eMethodType == EcrioSipMessageTypeBye)
		{
			if (pSigMgr->pOOMObject)
			{
				if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeBye, EcrioSipHeaderTypeP_AccessNetworkInfo))
				{
					error = _EcrioSigMgrAddP_AccessNWInfoHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList);
					if (error != ECRIO_SIGMGR_NO_ERROR)
					{
						SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
							"%s:%u\t_EcrioSigMgrAddP_AccessNWInfoHeader() error=%u",
							__FUNCTION__, __LINE__, error);
						goto Error_Level_02;
					}
				}

				if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeBye, EcrioSipHeaderTypeP_LastAccessNetworkInfo))		
				{
					error = _EcrioSigMgrAddP_LastAccessNWInfoHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList);
					if (error != ECRIO_SIGMGR_NO_ERROR)
					{
						SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
							"%s:%u\tEcrioSipHeaderTypeP_LastAccessNetworkInfo() error=%u",
							__FUNCTION__, __LINE__, error);
						goto Error_Level_01;
					}
				}

				/* Add reason header */
				if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeBye, EcrioSipHeaderTypeReason))
				{
					EcrioSigMgrHeaderStruct *pReasonHeader = NULL;
					EcrioSigMgrGetOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList, EcrioSipHeaderTypeReason, &pReasonHeader);
					/* Add this header only if not already added */
					if (pReasonHeader == NULL)
					{
						error = _EcrioSigMgrAddReasonHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList, pTempSipRequest->pMandatoryHdrs->pCallId);
						if (error != ECRIO_SIGMGR_NO_ERROR)
						{
							SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
								"%s:%u\t_EcrioSigMgrAddReasonHeader() error=%u",
								__FUNCTION__, __LINE__, error);
							goto Error_Level_02;
						}
					}
				}
			}
		}
	}

	if (pSipReqInfo->eMethodType == EcrioSipMessageTypeCancel)
	{
		pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
		if (pInviteUsage->pInviteReq->pRouteSet != NULL)
		{
			if (pTempSipRequest->pRouteSet != NULL)
				_EcrioSigMgrReleaseRouteStruct(pSigMgr, pTempSipRequest->pRouteSet);
			else
				pal_MemoryAllocate(sizeof(EcrioSigMgrRouteStruct), (void **)&pTempSipRequest->pRouteSet);
			
			if (pTempSipRequest->pRouteSet == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryAllocate() for pOptionalHdrs->pRouteSet, error=%u",
					__FUNCTION__, __LINE__, error);

				goto Error_Level_01;
			}

			error = _EcrioSigMgrCopyRouteSet(pSigMgr, (u_int16)pInviteUsage->pInviteReq->pRouteSet->numRoutes,
				pInviteUsage->pInviteReq->pRouteSet->ppRouteDetails,
				&pTempSipRequest->pRouteSet->ppRouteDetails);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrCopyRouteSet() for pOptionalHdrs->pRouteSet->ppRouteDetails, error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}

			pTempSipRequest->pRouteSet->numRoutes = pInviteUsage->pInviteReq->pRouteSet->numRoutes;
		}

		if (pInviteUsage->pInviteReq->pMandatoryHdrs != NULL &&
			pInviteUsage->pInviteReq->pMandatoryHdrs->ppVia != NULL)
		{
			u_int32 i = 0;

			/* Free Via (if non-null) & Overwrite from dialog information
			*/
			if (pTempSipRequest->pMandatoryHdrs->ppVia != NULL)
			{
				u_int32 i = 0;

				for (i = 0; i < pTempSipRequest->pMandatoryHdrs->numVia; ++i)
				{
					_EcrioSigMgrReleaseViaStruct(pSigMgr, pTempSipRequest->pMandatoryHdrs->ppVia[i]);
					pal_MemoryFree((void **)&(pTempSipRequest->pMandatoryHdrs->ppVia[i]));
				}

				pal_MemoryFree((void **)&(pTempSipRequest->pMandatoryHdrs->ppVia));
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected(pInviteUsage->pInviteReq->pMandatoryHdrs->numVia, 
				sizeof(EcrioSigMgrViaStruct *)) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			/* Overwrite Via from dialog information
			*/
			pal_MemoryAllocate(sizeof(EcrioSigMgrViaStruct *) * (pInviteUsage->pInviteReq->pMandatoryHdrs->numVia), (void **)&pTempSipRequest->pMandatoryHdrs->ppVia);
			if (pTempSipRequest->pMandatoryHdrs->ppVia == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pTempSipRequest->pMandatoryHdrs->numVia = pInviteUsage->pInviteReq->pMandatoryHdrs->numVia;

			for (i = 0; i < pTempSipRequest->pMandatoryHdrs->numVia; ++i)
			{
				pal_MemoryAllocate(sizeof(EcrioSigMgrViaStruct), (void **)&pTempSipRequest->pMandatoryHdrs->ppVia[i]);
				if (pTempSipRequest->pMandatoryHdrs->ppVia[i] == NULL)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					goto Error_Level_01;
				}

				error = _EcrioSigMgrPopulateViaHdr(pSigMgr, \
					pInviteUsage->pInviteReq->pMandatoryHdrs->ppVia[i], \
					pTempSipRequest->pMandatoryHdrs->ppVia[i]);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					goto Error_Level_01;
				}
			}
		}

		if (pSigMgr->pOOMObject)
		{
			/* Add reason header */
			if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeCancel, EcrioSipHeaderTypeReason))
			{
				error = _EcrioSigMgrAddReasonHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList, pTempSipRequest->pMandatoryHdrs->pCallId);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrAddReasonHeader() error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_02;
				}
			}
		}
	}
	else if (pSipReqInfo->eMethodType == EcrioSipMessageTypePrack)
	{
		pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
		if (pInviteUsage->bPrackEnabled == Enum_TRUE && pInviteUsage->pPrackDetails)
		{
			if (0 >= pal_NumToString(pInviteUsage->pPrackDetails->uLastRseq, RseqStr, 11))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				goto Error_Level_01;
			}
			if (0 >= pal_NumToString(pInviteUsage->pInviteReq->pMandatoryHdrs->CSeq, CseqStr, 11))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				goto Error_Level_01;
			}
			uRackHdrLen += pal_StringLength(RseqStr);
			uRackHdrLen += 1;
			uRackHdrLen += pal_StringLength(CseqStr);
			uRackHdrLen += 1;
			uRackHdrLen += pal_StringLength((u_char *)"INVITE");

			pal_MemoryAllocate(uRackHdrLen + 1, (void **)&pRackHeader);
			if (pRackHeader == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			uSize = uRackHdrLen + 1;
			pal_MemorySet(pRackHeader, 0, uSize);

			if (NULL == pal_StringNConcatenate(pRackHeader, uSize - pal_StringLength(pRackHeader), RseqStr, pal_StringLength(RseqStr)))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pRackHeader, uSize - pal_StringLength(pRackHeader), (u_char *)" ", pal_StringLength((const u_char*)" ")))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pRackHeader, uSize - pal_StringLength(pRackHeader), CseqStr, pal_StringLength(CseqStr)))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pRackHeader, uSize - pal_StringLength(pRackHeader), (u_char *)" ", pal_StringLength((const u_char*)" ")))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pRackHeader, uSize - pal_StringLength(pRackHeader), (u_char *)"INVITE", pal_StringLength((const u_char *)"INVITE")))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList, EcrioSipHeaderTypeRAck, 1, &pRackHeader, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}
		}
	}
	else if (pSipReqInfo->eMethodType == EcrioSipMessageTypeSubscribe)
	{
		pSubscribeUsage = (_EcrioSigMgrSubscribeUsageInfoStruct *)pDialogNode->pDialogContext;

		EcrioSigMgrGetOptionalHeader((SIGMGRHANDLE)pSigMgr, pSubscribeUsage->pSubscribeReq->pOptionalHeaderList, EcrioSipHeaderTypeEvent, &pHeader);
		if (pHeader)
		{
			if (pHeader->numHeaderValues && pHeader->ppHeaderValues)
			{
				for (i = 0; i < pHeader->numHeaderValues; i++)
				{
					if (pHeader->ppHeaderValues[i] && pHeader->ppHeaderValues[i]->pHeaderValue)
					{
						if (ppEventHdrStr == NULL)
						{
							pal_MemoryAllocate(sizeof(u_char *), (void **)&ppEventHdrStr);
						}
						else
						{
							/* Check arithmetic overflow */
							if (pal_UtilityArithmeticOverflowDetected(uEventHdrCount, 1) == Enum_TRUE)
							{
								error = ECRIO_SIGMGR_NO_MEMORY;
								goto Error_Level_02;
							}

							/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
							if (pal_UtilityDataOverflowDetected((uEventHdrCount + 1), sizeof(u_char *)) == Enum_TRUE)
							{
								error = ECRIO_SIGMGR_NO_MEMORY;
								goto Error_Level_02;
							}
							pal_MemoryReallocate((uEventHdrCount + 1) * sizeof(u_char *), (void **)&ppEventHdrStr);
						}
						if (ppEventHdrStr == NULL)
						{
							error = ECRIO_SIGMGR_NO_MEMORY;
							SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
								"%s:%u\tpal_MemoryReallocate failed() error=%u",
								__FUNCTION__, __LINE__, error);
							goto Error_Level_02;
						}

						ppEventHdrStr[uEventHdrCount] = pal_StringCreate(pHeader->ppHeaderValues[i]->pHeaderValue,
							pal_StringLength(pHeader->ppHeaderValues[i]->pHeaderValue));
						uEventHdrCount++;
					}
				}

				error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList, EcrioSipHeaderTypeEvent, (u_int16)uEventHdrCount, ppEventHdrStr, 0, NULL, NULL);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_02;
				}

				pHeader = NULL;
			}

			if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeSubscribe, EcrioSipHeaderTypeP_AccessNetworkInfo) == Enum_TRUE)
			{
				error = _EcrioSigMgrAddP_AccessNWInfoHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrAddP_AccessNWInfoHeader() error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}
			}

			if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeSubscribe, EcrioSipHeaderTypeP_LastAccessNetworkInfo))
			{
				error = _EcrioSigMgrAddP_LastAccessNWInfoHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tEcrioSipHeaderTypeP_LastAccessNetworkInfo() error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}
			}
		}

		EcrioSigMgrGetOptionalHeader((SIGMGRHANDLE)pSigMgr, pSubscribeUsage->pSubscribeReq->pOptionalHeaderList, EcrioSipHeaderTypeAccept, &pHeader);
		if (pHeader)
		{
			if (pHeader->numHeaderValues && pHeader->ppHeaderValues)
			{
				for (i = 0; i < pHeader->numHeaderValues; i++)
				{
					if (pHeader->ppHeaderValues[i] && pHeader->ppHeaderValues[i]->pHeaderValue)
					{
						if (ppAcceptHdrStr == NULL)
						{
							pal_MemoryAllocate(sizeof(u_char *), (void **)&ppAcceptHdrStr);
						}
						else
						{
							/* Check arithmetic overflow */
							if (pal_UtilityArithmeticOverflowDetected(uAcceptHdrCount, 1) == Enum_TRUE)
							{
								error = ECRIO_SIGMGR_NO_MEMORY;
								goto Error_Level_02;
							}

							/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
							if (pal_UtilityDataOverflowDetected((uAcceptHdrCount + 1), sizeof(u_char *)) == Enum_TRUE)
							{
								error = ECRIO_SIGMGR_NO_MEMORY;
								goto Error_Level_02;
							}
							pal_MemoryReallocate((uAcceptHdrCount + 1) * sizeof(u_char *), (void **)&ppAcceptHdrStr);
						}
						if (ppAcceptHdrStr == NULL)
						{
							error = ECRIO_SIGMGR_NO_MEMORY;
							SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
								"%s:%u\tpal_MemoryReallocate failed() error=%u",
								__FUNCTION__, __LINE__, error);
							goto Error_Level_02;
						}

						ppAcceptHdrStr[uAcceptHdrCount] = pal_StringCreate(pHeader->ppHeaderValues[i]->pHeaderValue,
							pal_StringLength(pHeader->ppHeaderValues[i]->pHeaderValue));
						uAcceptHdrCount++;
					}
				}

				error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList, EcrioSipHeaderTypeAccept, (u_int16)uAcceptHdrCount, ppAcceptHdrStr, 0, NULL, NULL);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_02;
				}

				pHeader = NULL;
			}
		}

		/* Copy P-Preferred-Service */
		EcrioSigMgrGetOptionalHeader((SIGMGRHANDLE)pSigMgr, pSubscribeUsage->pSubscribeReq->pOptionalHeaderList, EcrioSipHeaderTypeP_PreferredService, &pHeader);
		if (pHeader)
		{
			if (pHeader->numHeaderValues > 0 && pHeader->ppHeaderValues != NULL)
			{
				if (pHeader->ppHeaderValues[0] != NULL && pHeader->ppHeaderValues[0]->pHeaderValue != NULL)
				{
					if (ppPPreferredServiceHdrStr == NULL)
					{
						pal_MemoryAllocate(sizeof(u_char *), (void **)&ppPPreferredServiceHdrStr);
					}
						
					if (ppPPreferredServiceHdrStr == NULL)
					{
						error = ECRIO_SIGMGR_NO_MEMORY;
						SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
							"%s:%u\tpal_MemoryReallocate failed() error=%u",
							__FUNCTION__, __LINE__, error);
						goto Error_Level_02;
					}

					ppPPreferredServiceHdrStr[0] = pal_StringCreate(pHeader->ppHeaderValues[0]->pHeaderValue,
						pal_StringLength(pHeader->ppHeaderValues[0]->pHeaderValue));
				}
				

				error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList, EcrioSipHeaderTypeP_PreferredService, (u_int16)1, ppPPreferredServiceHdrStr, 0, NULL, NULL);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_02;
				}

				pHeader = NULL;
			}			
		}


		/* Copy Accept-Contact */
		EcrioSigMgrGetOptionalHeader((SIGMGRHANDLE)pSigMgr, pSubscribeUsage->pSubscribeReq->pOptionalHeaderList, EcrioSipHeaderTypeAcceptContact, &pHeader);
		if (pHeader)
		{
			if (pHeader->numHeaderValues > 0 && pHeader->ppHeaderValues != NULL)
			{
				if (pHeader->ppHeaderValues[0] != NULL && pHeader->ppHeaderValues[0]->pHeaderValue != NULL)
				{
					if (ppAcceptContactHdrStr == NULL)
					{
						pal_MemoryAllocate(sizeof(u_char *), (void **)&ppAcceptContactHdrStr);
					}

					if (ppAcceptContactHdrStr == NULL)
					{
						error = ECRIO_SIGMGR_NO_MEMORY;
						SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
							"%s:%u\tpal_MemoryReallocate failed() error=%u",
							__FUNCTION__, __LINE__, error);
						goto Error_Level_02;
					}

					ppAcceptContactHdrStr[0] = pal_StringCreate(pHeader->ppHeaderValues[0]->pHeaderValue,
						pal_StringLength(pHeader->ppHeaderValues[0]->pHeaderValue));
				}

				error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList, EcrioSipHeaderTypeAcceptContact, (u_int16)1, ppAcceptContactHdrStr, 0, NULL, NULL);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_02;
				}

				pHeader = NULL;
			}
		}

		/** Add P-PreferredIdentity header */
		error = _EcrioSigMgrAddP_PreferrdIdHeader(pSigMgr, pTempSipRequest);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddP_preferredIdHeader() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		if (pSigMgr->pOOMObject->ec_oom_HasUserPhoneSupport(EcrioSipMessageTypeSubscribe, EcrioSipHeaderTypeP_PreferredIdentity, EcrioSipURIType_SIP) == Enum_TRUE)
		{
			/* All outgoing SIP uri which contain telephone number in user info part must have "user=phone" parameter as SIP uri parameter.	*/
			error = _EcrioSigMgrAddUserPhoneParam(pSigMgr, &pTempSipRequest->ppPPreferredIdentity[0]->addrSpec);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddUserPhoneParam, error=%u", __FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}

		if (pSubscribeUsage->pSubscribeReq->pExpires)
		{
			if (pTempSipRequest->pExpires == NULL)
			{
				pal_MemoryAllocate(sizeof(u_int32), (void **)&pTempSipRequest->pExpires);
				if (pTempSipRequest->pExpires == NULL)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tpal_MemoryAllocate failed() error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_02;
				}
			}

			*(pTempSipRequest->pExpires) = *(pSubscribeUsage->pSubscribeReq->pExpires);
		}

		/* Copy the list of feature tags */
		pTempSipRequest->pFetaureTags = pSubscribeUsage->pSubscribeReq->pFetaureTags;
	}
	else if (pSipReqInfo->eMethodType == EcrioSipMessageTypeUpdate)
	{
		_EcrioSigMgrInviteUsageInfoStruct *pUsageData = NULL;
		pUsageData = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
		if (NULL == pUsageData)
		{
			error = ECRIO_SIGMGR_INVALID_DATA;
			goto Error_Level_02;
		}

		if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeUpdate, EcrioSipHeaderTypeSessionExpires))
		{
			/* Use incoming session expire value if its greater than minSe. Otherwise fallback on
			local session expire value. */
			if (pUsageData->sessionExpireVal > 0 &&
				pUsageData->sessionExpireVal > pSigMgr->pSignalingInfo->minSe)
				uSessionExpireValue = pUsageData->sessionExpireVal;
			else
				uSessionExpireValue = pSigMgr->pSignalingInfo->sessionExpires;

			/* Session Expires Header*/
			error = _EcrioSigMgrAddSessionExpiresHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList, pUsageData->isSessionRefresh, 
				!pUsageData->isRefresher, uSessionExpireValue);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t__EcrioSigMgrAddSessionExpiresHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}

		/* Min-SE support*/
		if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeUpdate, EcrioSipHeaderTypeMinSE))
		{
			if (pUsageData->minSEVal > 0)
			{
				uMinSEValue = pUsageData->minSEVal;
			}
			else
			{
				uMinSEValue = pSigMgr->pSignalingInfo->minSe;
			}
	
			if (uMinSEValue != 0)
			{
				error = _EcrioSigMgrAddMinSEHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList, uMinSEValue);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					goto Error_Level_01;
				}
			}
		}

		if (pSipReqInfo->pMessageBody != NULL)
			pTempSipRequest->pMessageBody = pSipReqInfo->pMessageBody;
	}
	else if (pSipReqInfo->eMethodType == EcrioSipMessageTypeRefer)
	{
		_EcrioSigMgrInviteUsageInfoStruct *pUsageData = NULL;
		EcrioSigMgrReferRecipientInfoStruct *pStruct = NULL;

		pUsageData = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
		if (NULL == pUsageData)
		{
			error = ECRIO_SIGMGR_INVALID_DATA;
			goto Error_Level_02;
		}

		pStruct = (EcrioSigMgrReferRecipientInfoStruct*)pUsageData->pData;

		error = _EcrioSigMgrFormUri(pSigMgr, pSigMgr->pSignalingInfo->pFromSipURI, &pFromUri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrFormUri() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_02;
		}

		if (pStruct->bIsMultiRecipient == Enum_TRUE)
		{
			/** More than one principal */

			/** Refer-To header */
			uLength = 0;
			uLength += 5;
			uLength += pal_StringLength(pStruct->pContentId);
			uLength += 1;

			/* Check arithmetic overflow */
			if (pal_UtilityArithmeticOverflowDetected(uLength, 1) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((uLength+1), sizeof(u_char)) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pal_MemoryAllocate((uLength + 1)*sizeof(u_char), (void **)&pReferToHdr);
			if (pReferToHdr == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			uSize = uLength + 1;
			pal_MemorySet(pReferToHdr, 0, uSize);

			if (NULL == pal_StringNConcatenate(pReferToHdr, uSize - pal_StringLength(pReferToHdr), (u_char*)"<cid:", pal_StringLength((const u_char *)"<cid:")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pReferToHdr, uSize - pal_StringLength(pReferToHdr), (u_char*)pStruct->pContentId, pal_StringLength((const u_char *)pStruct->pContentId)))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pReferToHdr, uSize - pal_StringLength(pReferToHdr), (u_char*)">", pal_StringLength((const u_char*)">")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeReferTo, 1, &pReferToHdr, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			/** Referred-By header */
			uLength = 0;

			uLength += 1;
			uLength += pal_StringLength(pFromUri);
			uLength += 1;

			uLength += 6;
			uLength += pal_StringLength(pStruct->pContentId);
			uLength += 1;

			/* Check arithmetic overflow */
			if (pal_UtilityArithmeticOverflowDetected(uLength, 1) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((uLength + 1), sizeof(u_char)) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pal_MemoryAllocate((uLength + 1)*sizeof(u_char), (void **)&pReferredByHdr);
			if (pReferredByHdr == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			uSize = uLength + 1;
			pal_MemorySet(pReferredByHdr, 0, uSize);

			if (NULL == pal_StringNConcatenate(pReferredByHdr, uSize - pal_StringLength(pReferredByHdr), (u_char*)"<", pal_StringLength((const u_char*)"<")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pReferredByHdr, uSize - pal_StringLength(pReferredByHdr), (u_char*)pFromUri, pal_StringLength((const u_char *)pFromUri)))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pReferredByHdr, uSize - pal_StringLength(pReferredByHdr), (u_char*)">", pal_StringLength((const u_char*)">")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			if (NULL == pal_StringNConcatenate(pReferredByHdr, uSize - pal_StringLength(pReferredByHdr), (u_char*)";cid=\"", pal_StringLength((const u_char *)";cid=\"")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pReferredByHdr, uSize - pal_StringLength(pReferredByHdr), (u_char*)pStruct->pContentId, pal_StringLength((const u_char *)pStruct->pContentId)))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pReferredByHdr, uSize - pal_StringLength(pReferredByHdr), (u_char*)"\"", pal_StringLength((const u_char*)"\"")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeReferredBy, 1, &pReferredByHdr, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeSubscribe, EcrioSipHeaderTypeP_AccessNetworkInfo) == Enum_TRUE)
			{
				error = _EcrioSigMgrAddP_AccessNWInfoHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrAddP_AccessNWInfoHeader() error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}
			}

			if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeSubscribe, EcrioSipHeaderTypeP_LastAccessNetworkInfo))
			{
				error = _EcrioSigMgrAddP_LastAccessNWInfoHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tEcrioSipHeaderTypeP_LastAccessNetworkInfo() error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}
			}

			/** Refer-Sub header: "false" */
			uLength = 0;
			uLength += 5;

			pal_MemoryAllocate(uLength + 1, (void **)&pReferSubHdr);
			if (pReferSubHdr == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			uSize = uLength + 1;
			pal_MemorySet(pReferSubHdr, 0, uSize);

			if (NULL == pal_StringNConcatenate(pReferSubHdr, uSize - pal_StringLength(pReferSubHdr), (u_char*)"false", pal_StringLength((const u_char *)"false")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeReferSub, 1, &pReferSubHdr, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			/** Require header */
			pal_MemoryAllocate(sizeof(u_char *) * 2, (void **)&ppRequireHdr);
			if (ppRequireHdr == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			uRequireHdrCount = 2;
			ppRequireHdr[0] = pal_StringCreate((u_char*)"multiple-refer", pal_StringLength((u_char*)"multiple-refer"));
			ppRequireHdr[1] = pal_StringCreate((u_char*)"norefersub", pal_StringLength((u_char*)"norefersub"));

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeRequire, uRequireHdrCount, ppRequireHdr, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			/** Content-ID header */
			uLength = 0;
			uLength += 1;
			uLength += pal_StringLength(pStruct->pContentId);
			uLength += 1;

			/* Check arithmetic overflow */
			if (pal_UtilityArithmeticOverflowDetected(uLength, 1) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((uLength + 1), sizeof(u_char)) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pal_MemoryAllocate((uLength + 1)*sizeof(u_char), (void **)&pContentIdHdr);
			if (pContentIdHdr == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			uSize = uLength + 1;
			pal_MemorySet(pContentIdHdr, 0, uSize);

			if (NULL == pal_StringNConcatenate(pContentIdHdr, uSize - pal_StringLength(pContentIdHdr), (u_char*)"<", pal_StringLength((const u_char*)"<")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pContentIdHdr, uSize - pal_StringLength(pContentIdHdr), (u_char*)pStruct->pContentId, pal_StringLength((const u_char *)pStruct->pContentId)))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pContentIdHdr, uSize - pal_StringLength(pContentIdHdr), (u_char*)">", pal_StringLength((const u_char*)">")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeContentID, 1, &pContentIdHdr, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			/** Content-Disposition header */
			uLength = 0;
			uLength += pal_StringLength((u_char*)"recipient-list");

			pal_MemoryAllocate(uLength + 1, (void **)&pContentDispositionHdr);
			if (pContentDispositionHdr == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			uSize = uLength + 1;
			pal_MemorySet(pContentDispositionHdr, 0, uSize);

			if (NULL == pal_StringNConcatenate(pContentDispositionHdr, uSize - pal_StringLength(pContentDispositionHdr), (u_char*)"recipient-list", pal_StringLength((const u_char *)"recipient-list")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeContentDisposition, 1, &pContentDispositionHdr, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			/** Event header */
			uLength = 0;
			uLength += pal_StringLength((u_char*)"refer");

			pal_MemoryAllocate(uLength + 1, (void **)&pEvent);
			if (pEvent == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			uSize = uLength + 1;
			pal_MemorySet(pEvent, 0, uSize);

			if (NULL == pal_StringNConcatenate(pEvent, uSize - pal_StringLength(pEvent), (u_char*)"refer", pal_StringLength((const u_char *)"refer")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeEvent, 1, &pEvent, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			/** Supported header */
			uLength = 0;
			uLength += pal_StringLength((u_char*)"norefersub");

			pal_MemoryAllocate(uLength + 1, (void **)&pSupported);
			if (pSupported == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			uSize = uLength + 1;
			pal_MemorySet(pSupported, 0, uSize);

			if (NULL == pal_StringNConcatenate(pSupported, uSize - pal_StringLength(pSupported), (u_char*)"norefersub", pal_StringLength((const u_char *)"norefersub")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeSupported, 1, &pSupported, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			/** Subject header */
			EcrioSigMgrGetOptionalHeader((SIGMGRHANDLE)pSigMgr, pUsageData->pInviteReq->pOptionalHeaderList, EcrioSipHeaderTypeSubject, &pHeader);
			if (pHeader)
			{
				if (pHeader)
				{
					if (pHeader->numHeaderValues && pHeader->ppHeaderValues)
					{
						for (i = 0; i < pHeader->numHeaderValues; i++)
						{
							if (pHeader->ppHeaderValues[i] && pHeader->ppHeaderValues[i]->pHeaderValue)
							{
								pal_MemoryAllocate(pal_StringLength(pHeader->ppHeaderValues[i]->pHeaderValue) + 1, (void **)&pSubject);
								if (pSubject == NULL)
								{
									error = ECRIO_SIGMGR_NO_MEMORY;
									goto Error_Level_01;
								}
								pal_MemorySet((void*)pSubject, 0, pal_StringLength(pHeader->ppHeaderValues[i]->pHeaderValue) + 1);
								if (NULL == pal_StringNCopy(pSubject, pal_StringLength(pHeader->ppHeaderValues[i]->pHeaderValue) + 1, (u_char *)pHeader->ppHeaderValues[i]->pHeaderValue, pal_StringLength(pHeader->ppHeaderValues[i]->pHeaderValue)))
								{
									error = ECRIO_SIGMGR_STRING_ERROR;
									goto Error_Level_01;
								}

								error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
									EcrioSipHeaderTypeSubject, 1, &pSubject, 0, NULL, NULL);
								if (error != ECRIO_SIGMGR_NO_ERROR)
								{
									SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
										"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
										__FUNCTION__, __LINE__, error);
									goto Error_Level_02;
								}
							}
						}
					}
				}
			}
		}
		else
		{
			/** Only one principal */

			/** Refer-To header */
			uLength = 0;
			uLength += 1;
			uLength += pal_StringLength(pStruct->pRecipient);
			uLength += 8;
			if (pStruct->eMethod == EcrioSigMgrReferMethodINVITE)
			{
				uLength += 6;
			}
			else if (pStruct->eMethod == EcrioSigMgrReferMethodBYE)
			{
				uLength += 3;
			}
			uLength += 1;

			/* Check arithmetic overflow */
			if (pal_UtilityArithmeticOverflowDetected(uLength, 1) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((uLength + 1), sizeof(u_char)) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pal_MemoryAllocate((uLength + 1)*sizeof(u_char), (void **)&pReferToHdr);
			if (pReferToHdr == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			uSize = uLength + 1;
			pal_MemorySet(pReferToHdr, 0, uSize);

			if (NULL == pal_StringNConcatenate(pReferToHdr, uSize - pal_StringLength(pReferToHdr), (u_char*)"<", pal_StringLength((const u_char*)"<")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pReferToHdr, uSize - pal_StringLength(pReferToHdr), (u_char*)pStruct->pRecipient, pal_StringLength((const u_char *)pStruct->pRecipient)))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pReferToHdr, uSize - pal_StringLength(pReferToHdr), (u_char*)";method=", pal_StringLength((const u_char *)";method=")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (pStruct->eMethod == EcrioSigMgrReferMethodINVITE)
			{
				if (NULL == pal_StringNConcatenate(pReferToHdr, uSize - pal_StringLength(pReferToHdr), (u_char*)"INVITE", pal_StringLength((const u_char *)"INVITE")))
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					goto Error_Level_01;
				}
			}
			else if (pStruct->eMethod == EcrioSigMgrReferMethodBYE)
			{
				if (NULL == pal_StringNConcatenate(pReferToHdr, uSize - pal_StringLength(pReferToHdr), (u_char*)"BYE", pal_StringLength((const u_char *)"BYE")))
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					goto Error_Level_01;
				}
			}
			if (NULL == pal_StringNConcatenate(pReferToHdr, uSize - pal_StringLength(pReferToHdr), (u_char*)">", pal_StringLength((const u_char*)">")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeReferTo, 1, &pReferToHdr, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			/** Referred-By header */
			uLength = 0;

			uLength += 1;
			uLength += pal_StringLength(pFromUri);
			uLength += 1;

			/* Check arithmetic overflow */
			if (pal_UtilityArithmeticOverflowDetected(uLength, 1) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((uLength + 1), sizeof(u_char)) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pal_MemoryAllocate((uLength + 1)*sizeof(u_char), (void **)&pReferredByHdr);
			if (pReferredByHdr == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			uSize = uLength + 1;
			pal_MemorySet(pReferredByHdr, 0, uSize);

			if (NULL == pal_StringNConcatenate(pReferredByHdr, uSize - pal_StringLength(pReferredByHdr), (u_char*)"<", pal_StringLength((const u_char*)"<")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pReferredByHdr, uSize - pal_StringLength(pReferredByHdr), (u_char*)pFromUri, pal_StringLength((const u_char *)pFromUri)))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pReferredByHdr, uSize - pal_StringLength(pReferredByHdr), (u_char*)">", pal_StringLength((const u_char*)">")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeReferredBy, 1, &pReferredByHdr, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			/** Refer-Sub header: "false" */
			uLength = 0;
			uLength += 5;

			pal_MemoryAllocate(uLength + 1, (void **)&pReferSubHdr);
			if (pReferSubHdr == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			uSize = uLength + 1;
			pal_MemorySet(pReferSubHdr, 0, uSize);

			if (NULL == pal_StringNConcatenate(pReferSubHdr, uSize - pal_StringLength(pReferSubHdr), (u_char*)"false", pal_StringLength((const u_char *)"false")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeReferSub, 1, &pReferSubHdr, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			/** Require header */
			pal_MemoryAllocate(sizeof(u_char *), (void **)&ppRequireHdr);
			if (ppRequireHdr == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			uRequireHdrCount = 1;
			ppRequireHdr[0] = pal_StringCreate((u_char*)"norefersub", pal_StringLength((u_char*)"norefersub"));

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeRequire, uRequireHdrCount, ppRequireHdr, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}
		}

		/** Conversations ID */
		if (pSipReqInfo->pConvId != NULL)
			pTempSipRequest->pConvId = pSipReqInfo->pConvId;

#ifdef ENABLE_RCS
		 _EcrioSigMgrAddConversationsIDHeader(pSigMgr, pTempSipRequest);
#endif //ENABLE_RCS

		if (pSipReqInfo->pMessageBody != NULL)
			pTempSipRequest->pMessageBody = pSipReqInfo->pMessageBody;
	}

	/*error = _EcrioSigMgrCopyCredentials(pSigMgr,
	    (u_char *)ECRIO_SIG_MGR_METHOD_BYE,
	    NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
	    SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
	        "%s:%u\t_EcrioSigMgrCopyCredentials() for BYE request, error=%u",
	        __FUNCTION__, __LINE__, error);
	    goto Error_Level_01;
	}*/

	error = _EcrioSigMgrSipMessageForm(pSigMgr, EcrioSigMgrSipMessageModeFullName, pTempSipRequest, &pReqData, &reqLen);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrCreateByeReq() error=%u",
			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_BYE_FAILED;
		goto Error_Level_02;
	}

	if (pSigMgr->pSignalingInfo->bEnableTls == Enum_TRUE && pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex != 0)
	{
		eTransportType = EcrioSigMgrTransportTLS;
	}
	else if ((pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE) && (reqLen > pSigMgr->pSignalingInfo->uUdpMtu))
	{
		error = _EcrioSigMgrCreateTCPBasedBuffer(pSigMgr, &pReqData, EcrioSigMgrSIPRequest);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrCreateTCPBasedBuffer() error=%u",
				__FUNCTION__, __LINE__, error);

			error = ECRIO_SIGMGR_IMS_LIB_ERROR;
			goto Error_Level_01;
		}
		eTransportType = EcrioSigMgrTransportTCP;
	}
	else if (pSigMgr->pSignalingInfo->bEnableUdp == Enum_FALSE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE)
	{
		eTransportType = EcrioSigMgrTransportTCP;
	}

	if (pSipReqInfo->eMethodType == EcrioSipMessageTypeCancel)
	{
		txnInfo.currentContext = ECRIO_SIGMGR_CANCEL_SEND;
		txnInfo.retransmitContext = ECRIO_SIGMGR_CANCEL_RESEND;
	}
	else if (pSipReqInfo->eMethodType == EcrioSipMessageTypeBye)
	{
		txnInfo.currentContext = ECRIO_SIGMGR_BYE_SEND;
		txnInfo.retransmitContext = ECRIO_SIGMGR_BYE_RESEND;
	}
	else if (pSipReqInfo->eMethodType == EcrioSipMessageTypePrack)
	{
		txnInfo.currentContext = ECRIO_SIGMGR_PRACK_SEND;
		txnInfo.retransmitContext = ECRIO_SIGMGR_PRACK_RESEND;
	}
	else if (pSipReqInfo->eMethodType == EcrioSipMessageTypeSubscribe)
	{
		txnInfo.currentContext = ECRIO_SIGMGR_SUBSCRIBE_SEND;
		txnInfo.retransmitContext = ECRIO_SIGMGR_SUBSCRIBE_RESEND;
	}
	else if (pSipReqInfo->eMethodType == EcrioSipMessageTypeUpdate)
	{
		txnInfo.currentContext = ECRIO_SIGMGR_UPDATE_SEND;
		txnInfo.retransmitContext = ECRIO_SIGMGR_UPDATE_RESEND;
	}

	txnInfo.deleteflag = Enum_FALSE;
	txnInfo.messageType = EcrioSigMgrMessageNonInviteRequest;
	txnInfo.transport = eTransportType;
	txnInfo.pMsg = pReqData;
	txnInfo.msglen = reqLen;
	txnInfo.pSessionMappingStr = pTempSipRequest->pMandatoryHdrs->pCallId;

#if 0
	reqUriScheme = pTempSipRequest->pMandatoryHdrs->pRequestUri->uriScheme;
#endif

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeSIP,
		"%s:%u\tOutgoing SIP message constructed:", __FUNCTION__, __LINE__);
	SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pReqData, reqLen);

	error = _EcrioSigMgrPopulateCommonInfo(pSigMgr, pTempSipRequest->pMandatoryHdrs, &cmnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		error = ECRIO_SIGMGR_BYE_FAILED;
		goto Error_Level_03;
	}

	error = _EcrioSigMgrSendMsgToTXNHandler(pSigMgr, &cmnInfo, &txnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		if (error != ECRIO_SIG_MGR_SIG_SOCKET_ERROR)
		{
			if (pSipReqInfo->eMethodType == EcrioSipMessageTypeCancel)
				error = ECRIO_SIGMGR_CANCEL_FAILED;
			else if (pSipReqInfo->eMethodType == EcrioSipMessageTypeBye)
				error = ECRIO_SIGMGR_BYE_FAILED;
			else
				error = ECRIO_SIGMGR_UNKNOWN_ERROR;
		}			

		goto Error_Level_03;
	}

	if (pTempSipRequest->pMandatoryHdrs->numVia > 0)
	{
		for (i = 0; i < pTempSipRequest->pMandatoryHdrs->numVia; ++i)
		{
			EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_Via,
				(void **)&pTempSipRequest->pMandatoryHdrs->ppVia[i], Enum_TRUE);
		}

		pal_MemoryFree((void **)&pTempSipRequest->pMandatoryHdrs->ppVia);
	}

	pTempSipRequest->pMandatoryHdrs->numVia = 0;

Error_Level_03:
	if (pReqData != NULL)
	{
		pal_MemoryFree((void **)&pReqData );
		pReqData = NULL;
	}

Error_Level_02:
Error_Level_01:
	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);
	_EcrioSigMgrReleaseUriStruct(pSigMgr, &reqUri);
	if (pTempSipRequest)
	{
		pTempSipRequest->pMessageBody = NULL;

		if (pTempSipRequest->pFetaureTags != NULL)
			pTempSipRequest->pFetaureTags = NULL;

		_EcrioSigMgrReleaseSipMessage(pSigMgr, pTempSipRequest);
		pal_MemoryFree((void **)&pTempSipRequest);
	}

	if (pRackHeader)
	{
		pal_MemoryFree((void **)&pRackHeader);
	}

	if (uEventHdrCount && ppEventHdrStr)
	{
		for (i = 0; i < uEventHdrCount; i++)
		{
			if (ppEventHdrStr[i])
			{
				pal_MemoryFree((void **)&ppEventHdrStr[i]);
			}
		}

		pal_MemoryFree((void **)&ppEventHdrStr);
	}

	if (uAcceptHdrCount && ppAcceptHdrStr)
	{
		for (i = 0; i < uAcceptHdrCount; i++)
		{
			if (ppAcceptHdrStr[i])
			{
				pal_MemoryFree((void **)&ppAcceptHdrStr[i]);
			}
		}

		pal_MemoryFree((void **)&ppAcceptHdrStr);
	}

	if (ppPPreferredServiceHdrStr != NULL)
	{
		pal_MemoryFree((void **)&ppPPreferredServiceHdrStr[0]);
		pal_MemoryFree((void **)&ppPPreferredServiceHdrStr);
	}

	if (ppAcceptContactHdrStr != NULL)
	{
		pal_MemoryFree((void **)&ppAcceptContactHdrStr[0]);
		pal_MemoryFree((void **)&ppAcceptContactHdrStr);
	}

	if (pReferToHdr)
	{
		pal_MemoryFree((void **)&pReferToHdr);
	}
	if (pReferredByHdr)
	{
		pal_MemoryFree((void **)&pReferredByHdr);
	}
	if (pReferSubHdr)
	{
		pal_MemoryFree((void **)&pReferSubHdr);
	}
	if (pContentIdHdr)
	{
		pal_MemoryFree((void **)&pContentIdHdr);
	}
	if (pContentDispositionHdr)
	{
		pal_MemoryFree((void **)&pContentDispositionHdr);
	}
	if (pEvent)
	{
		pal_MemoryFree((void **)&pEvent);
	}
	if (pSupported)
	{
		pal_MemoryFree((void **)&pSupported);
	}
	if (pSubject)
	{
		pal_MemoryFree((void **)&pSubject);
	}
	if (pFromUri)
	{
		pal_MemoryFree((void **)&pFromUri);
	}

	if (uRequireHdrCount > 0 && ppRequireHdr)
	{
		for (i = 0; i < uRequireHdrCount; i++)
		{
			if (ppRequireHdr[i])
			{
				pal_MemoryFree((void **)&ppRequireHdr[i]);
			}
		}
		pal_MemoryFree((void **)&ppRequireHdr);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* Create REFER request and send it to network.
*/
u_int32 _EcrioSigMgrSendReferInDialog
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipReqInfo,
	_EcrioSigMgrDialogNodeStruct *pDialogNode,
	u_char** ppReferId
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 reqLen = 0;
	u_char *pReqData = NULL;
	/* EcrioSigMgrURISchemesEnum reqUriScheme = EcrioSigMgrURISchemeNone; */
	EcrioSigMgrUriStruct reqUri = { .uriScheme = EcrioSigMgrURISchemeNone };
	EcrioSigMgrCommonInfoStruct cmnInfo = { 0 };
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };
	EcrioSigMgrSipMessageStruct *pTempSipRequest = NULL;
	u_int32 i = 0;
	u_int32 uSize = 0;
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;
	u_char *pRackHeader = NULL;
	u_int32	uRackHdrLen = 0;
	u_char RseqStr[11] = { 0 }, CseqStr[11] = { 0 };
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;
	_EcrioSigMgrSubscribeUsageInfoStruct *pSubscribeUsage = NULL;
	u_int32	uEventHdrCount = 0, uAcceptHdrCount = 0;
	u_char **ppEventHdrStr = NULL;
	u_char **ppAcceptHdrStr = NULL;
	u_char **ppPPreferredServiceHdrStr = NULL;
	u_char ** ppAcceptContactHdrStr = NULL;
	EcrioSigMgrHeaderStruct *pHeader = NULL;
	u_int32 uLength = 0;
	u_int16 uRequireHdrCount = 0;
	u_char *pReferToHdr = NULL;
	u_char *pReferredByHdr = NULL;
	u_char *pReferSubHdr = NULL;
	u_char **ppRequireHdr = NULL;
	u_char *pContentIdHdr = NULL;
	u_char *pContentDispositionHdr = NULL;
	u_char *pEvent = NULL;
	u_char *pSupported = NULL;
	u_char *pSubject = NULL;
	u_char *pFromUri = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pSipReqInfo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tpSipReqInfo is NULL",
				   __FUNCTION__, __LINE__);
		return ECRIO_SIGMGR_INVALID_DATA;
	}

	if (pSipReqInfo->eMethodType == EcrioSipMessageTypeRefer)
	{
		cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_REFER;
	}
	else
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tWrong method passed",
			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_NO_ERROR;
		goto Error_Level_01;
	}

	cmnInfo.role = EcrioSigMgrRoleUAC;

	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrSipMessageStruct), (void **)&pTempSipRequest);
		if (pTempSipRequest == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pTempSipRequest, error=%u",
				__FUNCTION__, __LINE__, error);
			error = ECRIO_SIGMGR_NO_ERROR;
			goto Error_Level_01;
		}

		pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&pTempSipRequest->pMandatoryHdrs);
		if (pTempSipRequest->pMandatoryHdrs == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pTempSipRequest->pMandatoryHdrs, error=%u",
				__FUNCTION__, __LINE__, error);
			error = ECRIO_SIGMGR_NO_ERROR;
			goto Error_Level_01;
		}

		pTempSipRequest->eMethodType = pSipReqInfo->eMethodType;
		pTempSipRequest->eReqRspType = EcrioSigMgrSIPRequest;

		if (pSipReqInfo && pSipReqInfo->pMandatoryHdrs)
		{
			error = _EcrioSigMgrPopulateManHdrs(pSigMgr, pSipReqInfo->pMandatoryHdrs,
				pTempSipRequest->pMandatoryHdrs);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrPopulateManHdrs() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}
		}

		error = _EcrioSigMgePopulateHeaderFromDialogAndUpdateState(pSigMgr,
			pTempSipRequest, pDialogNode);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgePopulateHeaderFromDialogAndUpdateState() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_02;
		}

		error = _EcrioSigMgrPopulateMandatoryHeaders(pSigMgr, pTempSipRequest,
			EcrioSigMgrSIPRequest, Enum_FALSE, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrPopulateMandatoryHeaders() for pIMSManHdrs, error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}

		if ((pSipReqInfo != NULL) && (pSipReqInfo->pOptionalHeaderList != NULL))
		{
			pTempSipRequest->pOptionalHeaderList = pSipReqInfo->pOptionalHeaderList;
		}
		else
		{
			error = _EcrioSigMgrUpdateMessageRequestStruct(pSigMgr, pTempSipRequest, Enum_FALSE);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tOptional header creation, error=%u",
					__FUNCTION__, __LINE__, error);

				/*error  = ECRIO_SIGMGR_MESSAGE_FAILED;*/   /*return proper error code*/
				goto Error_Level_02;
			}
		}		
	}

	if (pSipReqInfo->eMethodType == EcrioSipMessageTypeRefer)
	{
		_EcrioSigMgrInviteUsageInfoStruct *pUsageData = NULL;
		EcrioSigMgrReferRecipientInfoStruct *pStruct = NULL;

		pUsageData = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
		if (NULL == pUsageData)
		{
			error = ECRIO_SIGMGR_INVALID_DATA;
			goto Error_Level_02;
		}

		pStruct = (EcrioSigMgrReferRecipientInfoStruct*)pUsageData->pData;

		error = _EcrioSigMgrFormUri(pSigMgr, pSigMgr->pSignalingInfo->pFromSipURI, &pFromUri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrFormUri() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_02;
		}

		if (pStruct->bIsMultiRecipient == Enum_TRUE)
		{
			/** More than one principal */

			/** Refer-To header */
			uLength = 0;
			uLength += 5;
			uLength += pal_StringLength(pStruct->pContentId);
			uLength += 1;

			/* Check arithmetic overflow */
			if (pal_UtilityArithmeticOverflowDetected(uLength, 1) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((uLength + 1), sizeof(u_char)) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pal_MemoryAllocate((uLength + 1)*sizeof(u_char), (void **)&pReferToHdr);
			if (pReferToHdr == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			uSize = uLength + 1;
			pal_MemorySet(pReferToHdr, 0, uSize);

			if (NULL == pal_StringNConcatenate(pReferToHdr, uSize - pal_StringLength(pReferToHdr), (u_char*)"<cid:", pal_StringLength((const u_char *)"<cid:")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pReferToHdr, uSize - pal_StringLength(pReferToHdr), (u_char*)pStruct->pContentId, pal_StringLength((const u_char *)pStruct->pContentId)))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pReferToHdr, uSize - pal_StringLength(pReferToHdr), (u_char*)">", pal_StringLength((const u_char*)">")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeReferTo, 1, &pReferToHdr, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			/** Referred-By header */
			uLength = 0;

			uLength += 1;
			uLength += pal_StringLength(pFromUri);
			uLength += 1;

			uLength += 6;
			uLength += pal_StringLength(pStruct->pContentId);
			uLength += 1;

			/* Check arithmetic overflow */
			if (pal_UtilityArithmeticOverflowDetected(uLength, 1) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((uLength + 1), sizeof(u_char)) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pal_MemoryAllocate((uLength + 1)*sizeof(u_char), (void **)&pReferredByHdr);
			if (pReferredByHdr == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			uSize = uLength + 1;
			pal_MemorySet(pReferredByHdr, 0, uSize);

			if (NULL == pal_StringNConcatenate(pReferredByHdr, uSize - pal_StringLength(pReferredByHdr), (u_char*)"<", pal_StringLength((const u_char*)"<")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pReferredByHdr, uSize - pal_StringLength(pReferredByHdr), (u_char*)pFromUri, pal_StringLength((const u_char *)pFromUri)))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pReferredByHdr, uSize - pal_StringLength(pReferredByHdr), (u_char*)">", pal_StringLength((const u_char*)">")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			if (NULL == pal_StringNConcatenate(pReferredByHdr, uSize - pal_StringLength(pReferredByHdr), (u_char*)";cid=\"", pal_StringLength((const u_char *)";cid=\"")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pReferredByHdr, uSize - pal_StringLength(pReferredByHdr), (u_char*)pStruct->pContentId, pal_StringLength((const u_char *)pStruct->pContentId)))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pReferredByHdr, uSize - pal_StringLength(pReferredByHdr), (u_char*)"\"", pal_StringLength((const u_char*)"\"")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeReferredBy, 1, &pReferredByHdr, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeSubscribe, EcrioSipHeaderTypeP_AccessNetworkInfo) == Enum_TRUE)
			{
				error = _EcrioSigMgrAddP_AccessNWInfoHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrAddP_AccessNWInfoHeader() error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}
			}

			if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeSubscribe, EcrioSipHeaderTypeP_LastAccessNetworkInfo))
			{
				error = _EcrioSigMgrAddP_LastAccessNWInfoHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tEcrioSipHeaderTypeP_LastAccessNetworkInfo() error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}
			}

			/** Refer-Sub header: "false" */
			uLength = 0;
			uLength += 5;

			pal_MemoryAllocate(uLength + 1, (void **)&pReferSubHdr);
			if (pReferSubHdr == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			uSize = uLength + 1;
			pal_MemorySet(pReferSubHdr, 0, uSize);

			if (NULL == pal_StringNConcatenate(pReferSubHdr, uSize - pal_StringLength(pReferSubHdr), (u_char*)"false", pal_StringLength((const u_char *)"false")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeReferSub, 1, &pReferSubHdr, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			/** Require header */
			pal_MemoryAllocate(sizeof(u_char *) * 2, (void **)&ppRequireHdr);
			if (ppRequireHdr == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			uRequireHdrCount = 2;
			ppRequireHdr[0] = pal_StringCreate((u_char*)"multiple-refer", pal_StringLength((u_char*)"multiple-refer"));
			ppRequireHdr[1] = pal_StringCreate((u_char*)"norefersub", pal_StringLength((u_char*)"norefersub"));

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeRequire, uRequireHdrCount, ppRequireHdr, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			/** Content-ID header */
			uLength = 0;
			uLength += 1;
			uLength += pal_StringLength(pStruct->pContentId);
			uLength += 1;

			/* Check arithmetic overflow */
			if (pal_UtilityArithmeticOverflowDetected(uLength, 1) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((uLength + 1), sizeof(u_char)) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pal_MemoryAllocate((uLength + 1)*sizeof(u_char), (void **)&pContentIdHdr);
			if (pContentIdHdr == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			uSize = uLength + 1;
			pal_MemorySet(pContentIdHdr, 0, uSize);

			if (NULL == pal_StringNConcatenate(pContentIdHdr, uSize - pal_StringLength(pContentIdHdr), (u_char*)"<", pal_StringLength((const u_char*)"<")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pContentIdHdr, uSize - pal_StringLength(pContentIdHdr), (u_char*)pStruct->pContentId, pal_StringLength((const u_char *)pStruct->pContentId)))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pContentIdHdr, uSize - pal_StringLength(pContentIdHdr), (u_char*)">", pal_StringLength((const u_char*)">")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeContentID, 1, &pContentIdHdr, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			/** Content-Disposition header */
			uLength = 0;
			uLength += pal_StringLength((u_char*)"recipient-list");

			pal_MemoryAllocate(uLength + 1, (void **)&pContentDispositionHdr);
			if (pContentDispositionHdr == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			uSize = uLength + 1;
			pal_MemorySet(pContentDispositionHdr, 0, uSize);

			if (NULL == pal_StringNConcatenate(pContentDispositionHdr, uSize - pal_StringLength(pContentDispositionHdr), (u_char*)"recipient-list", pal_StringLength((const u_char *)"recipient-list")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeContentDisposition, 1, &pContentDispositionHdr, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			/** Event header */
			uLength = 0;
			uLength += pal_StringLength((u_char*)"refer");

			pal_MemoryAllocate(uLength + 1, (void **)&pEvent);
			if (pEvent == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			uSize = uLength + 1;
			pal_MemorySet(pEvent, 0, uSize);

			if (NULL == pal_StringNConcatenate(pEvent, uSize - pal_StringLength(pEvent), (u_char*)"refer", pal_StringLength((const u_char *)"refer")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeEvent, 1, &pEvent, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			/** Supported header */
			uLength = 0;
			uLength += pal_StringLength((u_char*)"norefersub");

			pal_MemoryAllocate(uLength + 1, (void **)&pSupported);
			if (pSupported == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			uSize = uLength + 1;
			pal_MemorySet(pSupported, 0, uSize);

			if (NULL == pal_StringNConcatenate(pSupported, uSize - pal_StringLength(pSupported), (u_char*)"norefersub", pal_StringLength((const u_char *)"norefersub")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeSupported, 1, &pSupported, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			/** Subject header */
			EcrioSigMgrGetOptionalHeader((SIGMGRHANDLE)pSigMgr, pUsageData->pInviteReq->pOptionalHeaderList, EcrioSipHeaderTypeSubject, &pHeader);
			if (pHeader)
			{
				if (pHeader)
				{
					if (pHeader->numHeaderValues && pHeader->ppHeaderValues)
					{
						for (i = 0; i < pHeader->numHeaderValues; i++)
						{
							if (pHeader->ppHeaderValues[i] && pHeader->ppHeaderValues[i]->pHeaderValue)
							{
								pal_MemoryAllocate(pal_StringLength(pHeader->ppHeaderValues[i]->pHeaderValue) + 1, (void **)&pSubject);
								if (pSubject == NULL)
								{
									error = ECRIO_SIGMGR_NO_MEMORY;
									goto Error_Level_01;
								}
								pal_MemorySet((void*)pSubject, 0, pal_StringLength(pHeader->ppHeaderValues[i]->pHeaderValue) + 1);
								if (NULL == pal_StringNCopy(pSubject, pal_StringLength(pHeader->ppHeaderValues[i]->pHeaderValue) + 1, (u_char *)pHeader->ppHeaderValues[i]->pHeaderValue, pal_StringLength(pHeader->ppHeaderValues[i]->pHeaderValue)))
								{
									error = ECRIO_SIGMGR_STRING_ERROR;
									goto Error_Level_01;
								}

								error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
									EcrioSipHeaderTypeSubject, 1, &pSubject, 0, NULL, NULL);
								if (error != ECRIO_SIGMGR_NO_ERROR)
								{
									SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
										"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
										__FUNCTION__, __LINE__, error);
									goto Error_Level_02;
								}
							}
						}
					}
				}
			}
		}
		else
		{
			/** Only one principal */

			/** Refer-To header */
			uLength = 0;
			uLength += 1;
			uLength += pal_StringLength(pStruct->pRecipient);
			uLength += 8;
			if (pStruct->eMethod == EcrioSigMgrReferMethodINVITE)
			{
				uLength += 6;
			}
			else if (pStruct->eMethod == EcrioSigMgrReferMethodBYE)
			{
				uLength += 3;
			}
			uLength += 1;

			/* Check arithmetic overflow */
			if (pal_UtilityArithmeticOverflowDetected(uLength, 1) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((uLength + 1), sizeof(u_char)) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pal_MemoryAllocate((uLength + 1)*sizeof(u_char), (void **)&pReferToHdr);
			if (pReferToHdr == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			uSize = uLength + 1;
			pal_MemorySet(pReferToHdr, 0, uSize);

			if (NULL == pal_StringNConcatenate(pReferToHdr, uSize - pal_StringLength(pReferToHdr), (u_char*)"<", pal_StringLength((const u_char*)"<")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pReferToHdr, uSize - pal_StringLength(pReferToHdr), (u_char*)pStruct->pRecipient, pal_StringLength((const u_char *)pStruct->pRecipient)))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pReferToHdr, uSize - pal_StringLength(pReferToHdr), (u_char*)";method=", pal_StringLength((const u_char *)";method=")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (pStruct->eMethod == EcrioSigMgrReferMethodINVITE)
			{
				if (NULL == pal_StringNConcatenate(pReferToHdr, uSize - pal_StringLength(pReferToHdr), (u_char*)"INVITE", pal_StringLength((const u_char *)"INVITE")))
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					goto Error_Level_01;
				}
			}
			else if (pStruct->eMethod == EcrioSigMgrReferMethodBYE)
			{
				if (NULL == pal_StringNConcatenate(pReferToHdr, uSize - pal_StringLength(pReferToHdr), (u_char*)"BYE", pal_StringLength((const u_char *)"BYE")))
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					goto Error_Level_01;
				}
			}
			if (NULL == pal_StringNConcatenate(pReferToHdr, uSize - pal_StringLength(pReferToHdr), (u_char*)">", pal_StringLength((const u_char*)">")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeReferTo, 1, &pReferToHdr, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			/** Referred-By header */
			uLength = 0;

			uLength += 1;
			uLength += pal_StringLength(pFromUri);
			uLength += 1;

			/* Check arithmetic overflow */
			if (pal_UtilityArithmeticOverflowDetected(uLength, 1) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((uLength + 1), sizeof(u_char)) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pal_MemoryAllocate((uLength + 1)*sizeof(u_char), (void **)&pReferredByHdr);
			if (pReferredByHdr == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			uSize = uLength + 1;
			pal_MemorySet(pReferredByHdr, 0, uSize);

			if (NULL == pal_StringNConcatenate(pReferredByHdr, uSize - pal_StringLength(pReferredByHdr), (u_char*)"<", pal_StringLength((const u_char*)"<")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pReferredByHdr, uSize - pal_StringLength(pReferredByHdr), (u_char*)pFromUri, pal_StringLength((const u_char *)pFromUri)))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			if (NULL == pal_StringNConcatenate(pReferredByHdr, uSize - pal_StringLength(pReferredByHdr), (u_char*)">", pal_StringLength((const u_char*)">")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeReferredBy, 1, &pReferredByHdr, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			/** Refer-Sub header: "false" */
			uLength = 0;
			uLength += 5;

			pal_MemoryAllocate(uLength + 1, (void **)&pReferSubHdr);
			if (pReferSubHdr == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			uSize = uLength + 1;
			pal_MemorySet(pReferSubHdr, 0, uSize);

			if (NULL == pal_StringNConcatenate(pReferSubHdr, uSize - pal_StringLength(pReferSubHdr), (u_char*)"false", pal_StringLength((const u_char *)"false")))
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeReferSub, 1, &pReferSubHdr, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}

			/** Require header */
			pal_MemoryAllocate(sizeof(u_char *), (void **)&ppRequireHdr);
			if (ppRequireHdr == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			uRequireHdrCount = 1;
			ppRequireHdr[0] = pal_StringCreate((u_char*)"norefersub", pal_StringLength((u_char*)"norefersub"));

			error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSipRequest->pOptionalHeaderList,
				EcrioSipHeaderTypeRequire, uRequireHdrCount, ppRequireHdr, 0, NULL, NULL);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_02;
			}
		}

		/** Conversations ID */
		if (pSipReqInfo->pConvId != NULL)
			pTempSipRequest->pConvId = pSipReqInfo->pConvId;

#ifdef ENABLE_RCS
		_EcrioSigMgrAddConversationsIDHeader(pSigMgr, pTempSipRequest);
#endif //ENABLE_RCS

		if (pSipReqInfo->pMessageBody != NULL)
			pTempSipRequest->pMessageBody = pSipReqInfo->pMessageBody;
	}

	error = _EcrioSigMgrSipMessageForm(pSigMgr, EcrioSigMgrSipMessageModeFullName, pTempSipRequest, &pReqData, &reqLen);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrCreateByeReq() error=%u",
			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_BYE_FAILED;
		goto Error_Level_02;
	}

	if ((pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE) && (reqLen > pSigMgr->pSignalingInfo->uUdpMtu))
	{
		error = _EcrioSigMgrCreateTCPBasedBuffer(pSigMgr, &pReqData, EcrioSigMgrSIPRequest);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrCreateTCPBasedBuffer() error=%u",
				__FUNCTION__, __LINE__, error);

			error = ECRIO_SIGMGR_IMS_LIB_ERROR;
			goto Error_Level_01;
		}
		eTransportType = EcrioSigMgrTransportTCP;
	}
	else if (pSigMgr->pSignalingInfo->bEnableUdp == Enum_FALSE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE)
	{
		eTransportType = EcrioSigMgrTransportTCP;
	}	

	txnInfo.deleteflag = Enum_FALSE;
	txnInfo.messageType = EcrioSigMgrMessageNonInviteRequest;
	txnInfo.transport = eTransportType;
	txnInfo.pMsg = pReqData;
	txnInfo.msglen = reqLen;
	txnInfo.pSessionMappingStr = pTempSipRequest->pMandatoryHdrs->pCallId;

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeSIP,
		"%s:%u\tOutgoing SIP message constructed:", __FUNCTION__, __LINE__);
	SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pReqData, reqLen);

	error = _EcrioSigMgrPopulateCommonInfo(pSigMgr, pTempSipRequest->pMandatoryHdrs, &cmnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		error = ECRIO_SIGMGR_BYE_FAILED;
		goto Error_Level_03;
	}

	error = _EcrioSigMgrSendMsgToTXNHandler(pSigMgr, &cmnInfo, &txnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		if (error != ECRIO_SIG_MGR_SIG_SOCKET_ERROR)
		{
			if (pSipReqInfo->eMethodType == EcrioSipMessageTypeCancel)
				error = ECRIO_SIGMGR_CANCEL_FAILED;
			else if (pSipReqInfo->eMethodType == EcrioSipMessageTypeBye)
				error = ECRIO_SIGMGR_BYE_FAILED;
			else
				error = ECRIO_SIGMGR_UNKNOWN_ERROR;
		}

		goto Error_Level_03;
	}

	if (pTempSipRequest->pMandatoryHdrs->numVia > 0)
	{
		for (i = 0; i < pTempSipRequest->pMandatoryHdrs->numVia; ++i)
		{
			EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_Via,
				(void **)&pTempSipRequest->pMandatoryHdrs->ppVia[i], Enum_TRUE);
		}

		pal_MemoryFree((void **)&pTempSipRequest->pMandatoryHdrs->ppVia);
	}

	pTempSipRequest->pMandatoryHdrs->numVia = 0;

	/* Pass the branch value as refer id. It will be freed in upper layer. */
	*ppReferId = cmnInfo.pBranch;
	cmnInfo.pBranch = NULL;

Error_Level_03:
	if (pReqData != NULL)
	{
		pal_MemoryFree((void **)&pReqData);
		pReqData = NULL;
	}

Error_Level_02:
Error_Level_01 :
	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);
	_EcrioSigMgrReleaseUriStruct(pSigMgr, &reqUri);
	if (pTempSipRequest)
	{
		pTempSipRequest->pMessageBody = NULL;

		if (pTempSipRequest->pFetaureTags != NULL)
			pTempSipRequest->pFetaureTags = NULL;

		_EcrioSigMgrReleaseSipMessage(pSigMgr, pTempSipRequest);
		pal_MemoryFree((void **)&pTempSipRequest);
	}

	if (pRackHeader)
	{
		pal_MemoryFree((void **)&pRackHeader);
	}

	if (uEventHdrCount && ppEventHdrStr)
	{
		for (i = 0; i < uEventHdrCount; i++)
		{
			if (ppEventHdrStr[i])
			{
				pal_MemoryFree((void **)&ppEventHdrStr[i]);
			}
		}

		pal_MemoryFree((void **)&ppEventHdrStr);
	}

	if (uAcceptHdrCount && ppAcceptHdrStr)
	{
		for (i = 0; i < uAcceptHdrCount; i++)
		{
			if (ppAcceptHdrStr[i])
			{
				pal_MemoryFree((void **)&ppAcceptHdrStr[i]);
			}
		}

		pal_MemoryFree((void **)&ppAcceptHdrStr);
	}

	if (ppPPreferredServiceHdrStr != NULL)
	{
		pal_MemoryFree((void **)&ppPPreferredServiceHdrStr[0]);
		pal_MemoryFree((void **)&ppPPreferredServiceHdrStr);
	}

	if (ppAcceptContactHdrStr != NULL)
	{
		pal_MemoryFree((void **)&ppAcceptContactHdrStr[0]);
		pal_MemoryFree((void **)&ppAcceptContactHdrStr);
	}

	if (pReferToHdr)
	{
		pal_MemoryFree((void **)&pReferToHdr);
	}
	if (pReferredByHdr)
	{
		pal_MemoryFree((void **)&pReferredByHdr);
	}
	if (pReferSubHdr)
	{
		pal_MemoryFree((void **)&pReferSubHdr);
	}
	if (pContentIdHdr)
	{
		pal_MemoryFree((void **)&pContentIdHdr);
	}
	if (pContentDispositionHdr)
	{
		pal_MemoryFree((void **)&pContentDispositionHdr);
	}
	if (pEvent)
	{
		pal_MemoryFree((void **)&pEvent);
	}
	if (pSupported)
	{
		pal_MemoryFree((void **)&pSupported);
	}
	if (pSubject)
	{
		pal_MemoryFree((void **)&pSubject);
	}
	if (pFromUri)
	{
		pal_MemoryFree((void **)&pFromUri);
	}

	if (uRequireHdrCount > 0 && ppRequireHdr)
	{
		for (i = 0; i < uRequireHdrCount; i++)
		{
			if (ppRequireHdr[i])
			{
				pal_MemoryFree((void **)&ppRequireHdr[i]);
			}
		}
		pal_MemoryFree((void **)&ppRequireHdr);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* Get call state for a session.
*
*/
u_int32 _EcrioSigMgrGetCallState
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrDialogNodeStruct *pDialogNode,
	EcrioSigMgrCallStateEnum *pState
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData = NULL;

	(void)pSigMgr;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pUsageData = pDialogNode->pDialogContext;

	if (pUsageData == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpUsageData not found, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	*pState = pUsageData->callState;

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* Handle UPDATE response. This function will resend request for authentication also. It also
* post the event to state machine handler.
*
*/
u_int32 _EcrioSigMgrHandleUpdateResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pUpdateResp,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	BoolEnum bRequestTimedout
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrMessageStruct messageInfo = { .msgCmd = EcrioSigMgrMessageNone };
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData = NULL;
	EcrioSigMgrSipMessageStruct byeReq = { .eMethodType = EcrioSipMessageTypeNone };
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	BoolEnum bIsForked = Enum_FALSE;
	u_int32 responseCode = ECRIO_SIGMGR_INVALID_RESPONSE_CODE;

	EcrioSigMgrHeaderStruct *pSessionExpireHeader = NULL;
	EcrioSigMgrHeaderStruct *pMinSEHeader = NULL;

	u_int32	i = 0;
	u_int32	sessionInterval = 0;
	u_int32	minSEInterval = 0;
	EcrioSigMgrSipMessageStruct updateReq = { .eMethodType = EcrioSipMessageTypeNone };

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pCmnInfo->pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_UPDATE;

	if (bRequestTimedout != Enum_TRUE)
	{
		pDialogNode = EcrioSigMgrMatchDialogAndUpdateState(pSigMgr, pUpdateResp, &bIsForked, &responseCode);
		if (pDialogNode == NULL)
		{
			error = ECRIO_SIGMGR_DIALOG_NOT_EXISTS;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSigMgrMatchDialogAndUpdateState() Dialog not exist, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}
	else
	{
		BoolEnum bDialogMatched = Enum_FALSE;
		pDialogNode = EcrioSigMgrFindMatchedDialog(pSigMgr,
			pCmnInfo->pFromTag, pCmnInfo->pToTag, pCmnInfo->pCallId, pCmnInfo->cSeq, pUpdateResp->eMethodType,
			pUpdateResp->eReqRspType, &bDialogMatched, &bIsForked, &responseCode);
		if (pDialogNode == NULL)
		{
			error = ECRIO_SIGMGR_INVALID_DATA;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSigMgrFindMatchedDialog() Dialog not exist, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	pUsageData = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
	if (pUsageData == NULL)
	{
		error = ECRIO_SIGMGR_BYE_FAILED;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t Invite usage not found () INVITE response dropped, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	messageInfo.msgCmd = EcrioSigMgrUpdateResponseNotification;
	messageInfo.pData = (void *)pUpdateResp;

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\t_UPDATE response Code=%d",
		__FUNCTION__, __LINE__, pUpdateResp->responseCode);
	
	switch (pUpdateResp->responseCode / 100)
	{
		case ECRIO_SIGMGR_2XX_RESPONSE:
		{
			if (pUpdateResp->responseCode == ECRIO_SIGMGR_RESPONSE_CODE_OK)
			{
				pUpdateResp->statusCode = ECRIO_SIGMGR_RESPONSE_SUCCESS;
				/* session expire support start */
				if (pSigMgr->pOOMObject)
				{
					if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeUpdate_2xx, EcrioSipHeaderTypeSessionExpires))
					{
						error = EcrioSigMgrGetOptionalHeader(pSigMgr, pUpdateResp->pOptionalHeaderList,
							EcrioSipHeaderTypeSessionExpires, &pSessionExpireHeader);
						if (pSessionExpireHeader == NULL)
						{	/* get it from request */
							error = EcrioSigMgrGetOptionalHeader(pSigMgr, pUsageData->pInviteReq->pOptionalHeaderList,
								EcrioSipHeaderTypeSessionExpires, &pSessionExpireHeader);
						}
						if (pSessionExpireHeader != NULL)
						{
							if (pSessionExpireHeader->ppHeaderValues && pSessionExpireHeader->ppHeaderValues[0] && pSessionExpireHeader->ppHeaderValues[0]->pHeaderValue)
							{
								sessionInterval = pal_StringConvertToUNum(pSessionExpireHeader->ppHeaderValues[0]->pHeaderValue, NULL, 10);
							}
							if (sessionInterval > 0)
							{
								for (i = 0; i < pSessionExpireHeader->numHeaderValues; i++)
								{
									if (pSessionExpireHeader->ppHeaderValues && pSessionExpireHeader->ppHeaderValues[i])
									{
										if (pSessionExpireHeader->ppHeaderValues[i]->ppParams != NULL &&
											pal_SubString(pSessionExpireHeader->ppHeaderValues[i]->ppParams[0]->pParamName, (u_char *)ECRIO_SIG_MGR_SESSION_REFRESHER_PARAM))
										{
											if (pal_SubString(pSessionExpireHeader->ppHeaderValues[i]->ppParams[0]->pParamValue, (u_char *)ECRIO_SIG_MGR_SESSION_REFRESHER_UAC))
											{
												pUsageData->isRefresher = Enum_TRUE;
											}
											else if (pal_SubString(pSessionExpireHeader->ppHeaderValues[i]->ppParams[0]->pParamValue, (u_char *)ECRIO_SIG_MGR_SESSION_REFRESHER_UAS))
											{
												pUsageData->isRefresher = Enum_FALSE;
											}
										}
										else
										{
											pUsageData->isRefresher = Enum_TRUE;
										}
										break;
									}
								}
								pUsageData->sessionExpireVal = sessionInterval;
								_EcrioSigMgrStartSessionRefreshExpireTimer(pSigMgr, pUsageData,
									pDialogNode, sessionInterval, pUsageData->isRefresher);
							}	// session interval
						}	// pSessionExpireHeader
						else
						{
							if (pUsageData->callState == EcrioSigMgrCallStateCallEstablished)
							{
								error = _EcrioSigMgrSendBye(pSigMgr, pDialogNode, &byeReq);
								if (error != ECRIO_SIGMGR_NO_ERROR)
								{
									SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
										"%s:%u\t__EcrioSigMgrSendBye() returns error = %d",
										__FUNCTION__, __LINE__, error);
									goto Error_Level_01;
								}
							}
							else
							{
								error = ECRIO_SIGMGR_INVALID_OPERATION;
								SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
									"%s:%u\t INVALID operation returns error = %d",
									__FUNCTION__, __LINE__, error);
								goto Error_Level_01;
							}
						}
					}
					else
					{
						; // UPDATE not supported - ignore
					}
				}
				else
				{
					; // OOM not available - ignore
				}
			}
			else
			{
				; /* Other 2xx responses*/
			}
		}
		break;
		case ECRIO_SIGMGR_4XX_RESPONSE:
		{
			if (pUpdateResp->responseCode == ECRIO_SIGMGR_RESPONSE_CODE_SESSION_INTERVAL_SMALL)
			{
				/* session expire support start */
				if (pSigMgr->pOOMObject)
				{
					/* Handle MinSE */
					if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeUpdate_4xx, EcrioSipHeaderTypeMinSE))
					{
						error = EcrioSigMgrGetOptionalHeader(pSigMgr, pUpdateResp->pOptionalHeaderList,
							EcrioSipHeaderTypeMinSE, &pMinSEHeader);
						if (pMinSEHeader != NULL)
						{
							if (pMinSEHeader->ppHeaderValues && pMinSEHeader->ppHeaderValues[0] && pMinSEHeader->ppHeaderValues[0]->pHeaderValue)
							{
								minSEInterval = pal_StringConvertToUNum(pMinSEHeader->ppHeaderValues[0]->pHeaderValue, NULL, 10);
							}
							if (minSEInterval > 0)
							{								
								pUsageData->sessionExpireVal = minSEInterval;
								pUsageData->minSEVal = minSEInterval;
								/* send fresh UPDATE */
								_EcrioSigMgrStartSessionRefreshExpireTimer(pSigMgr, pUsageData,
									pDialogNode, 1, pUsageData->isRefresher);
							}
						}
						else
						{
							if (pUsageData->callState == EcrioSigMgrCallStateCallEstablished)
							{
								error = _EcrioSigMgrSendBye(pSigMgr, pDialogNode, &byeReq);
								if (error != ECRIO_SIGMGR_NO_ERROR)
								{
									SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
										"%s:%u\t__EcrioSigMgrSendBye() returns error = %d",
										__FUNCTION__, __LINE__, error);
								}
							}
						}
					}
				}
			}
			else  
			{
				; /* other 4xx responses */
			}
		}
		break;
		case ECRIO_SIGMGR_3XX_RESPONSE:
		case ECRIO_SIGMGR_5XX_RESPONSE:
		case ECRIO_SIGMGR_6XX_RESPONSE:
		default:
		{
			if (pUsageData->callState == EcrioSigMgrCallStateCallEstablished)
			{
				error = _EcrioSigMgrSendBye(pSigMgr, pDialogNode, &byeReq);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t__EcrioSigMgrSendBye() returns error = %d",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}
			}
			else
			{
				error = ECRIO_SIGMGR_INVALID_OPERATION;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t INVALID operation returns error = %d",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}			
		}
		break;
	} /* response switch */

	if (_EcrioSigMgrGetModuleId(pSigMgr, pCmnInfo->pCallId) == EcrioSigMgrCallbackRegisteringModule_CPM)
	{
		_EcrioSigMgrUpdateStatusCPM(pSigMgr, &messageInfo);
	}
	else
	{
		_EcrioSigMgrUpdateStatusMoIP(pSigMgr, &messageInfo);
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* Start the session refresh timer.
* Session Expire and Refresh Timer
*
*/
u_int32 _EcrioSigMgrStartSessionRefreshExpireTimer
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage,
	_EcrioSigMgrDialogNodeStruct *pDialogNode,
	u_int32 interval,
	BoolEnum isRefresh
)
{
	u_int32 uPFDerror = KPALErrorNone;
	TimerStartConfigStruct timerConfig = { 0 };
	// EcrioSigMgrSessionHandleStruct *pTempSessionHandle = NULL;

	u_int32 error = 0;
	u_int32	newSessionTimerInterval = 0;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (NULL == pInviteUsage)
	{
		goto Error_level1;
	}

	if (pInviteUsage->sessionExpireTimerId != NULL)
	{
		pal_TimerStop(pInviteUsage->sessionExpireTimerId);
		pInviteUsage->sessionExpireTimerId = NULL;
	}


	/* keep reference to Sig Mgr
	*/
	pInviteUsage->pSigMgr = pSigMgr;

	/* Setup timer*/
	timerConfig.uPeriodicInterval = 0;
	timerConfig.pCallbackData = (void *)pDialogNode;
	timerConfig.bEnableGlobalMutex = Enum_TRUE;

	if (isRefresh == Enum_TRUE)
	{
		/* Setup timer*/
		newSessionTimerInterval = (interval / 2) * 1000;/* seconds  to milliseconds*/
		timerConfig.uInitialInterval = newSessionTimerInterval;
		timerConfig.timerCallback = _EcrioSigMgrCallStateSessionRefreshTimerCallback;
	}
	else if (isRefresh == Enum_FALSE)
	{
		if (pSigMgr->pSignalingInfo->sessionExpires > 5)
		{
			/* Setup timer as per RFC 4028 section 10 */
			if (interval < 32)
			{
				newSessionTimerInterval = 32 * 1000;
			}
			else
			{
				newSessionTimerInterval = ((interval * 2 / 3) + 32) * 1000;
			}
			timerConfig.uInitialInterval = newSessionTimerInterval;
			timerConfig.timerCallback = _EcrioSigMgrCallStateSessionExpireTimerCallback;
		}
	}

	/* Start session expire timer
	*/
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStarting Session Expire Timer ... in state %u for interval %u ",
		__FUNCTION__, __LINE__, pInviteUsage->callState, timerConfig.uInitialInterval);

	uPFDerror = pal_TimerStart(pSigMgr->pal, &timerConfig, &pInviteUsage->sessionExpireTimerId);
	if (KPALErrorNone != uPFDerror)
	{
		error = ECRIO_SIGMGR_TIMER_ERROR;
		goto Error_level1;
	}

Error_level1:

	// pTempSessionHandle = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}
