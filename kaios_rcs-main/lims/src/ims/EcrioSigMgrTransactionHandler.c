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
#include "EcrioSigMgrTimer.h"
#include "EcrioSigMgrCallbacks.h"
#include "EcrioSigMgrUtilities.h"
#include "EcrioSigMgrIMSLibHandler.h"
#include "EcrioSigMgrTransactionHandler.h"
#include "EcrioSigMgrDialogHandler.h"
#include "EcrioSigMgrSubscribe.h"
#include "EcrioSigMgrSubscribeInternal.h"

/**************************************************************************

Function:		_EcrioSigMgrInitalizeTransactionMgr()

Purpose:		To initialize the Transaction Manager.

Description:	Initializes the Transaction manager.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager handle.

OutPut:			None

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrInitalizeTransactionMgr
(
	EcrioSigMgrStruct *pSigMgr
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioTXNInitParamStruct txnInitInfo = {0};

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpSigMgr=%p",
		__FUNCTION__, __LINE__, pSigMgr);

	pal_MemorySet(
		&(txnInitInfo),
		0,
		sizeof(EcrioTXNInitParamStruct));
	if (pSigMgr->timerX == 0)
	{
		txnInitInfo.timerX = 1;
	}
	else
	{
		txnInitInfo.timerX = pSigMgr->timerX;
	}

	txnInitInfo.pal = pSigMgr->pal;
	txnInitInfo.pLogHandle = pSigMgr->pLogHandle;
	txnInitInfo.customSIPTimers.T1 = pSigMgr->customSIPTimers.T1;
	txnInitInfo.customSIPTimers.T2 = pSigMgr->customSIPTimers.T2;
	txnInitInfo.customSIPTimers.T4 = pSigMgr->customSIPTimers.T4;
	txnInitInfo.customSIPTimers.TimerA = pSigMgr->customSIPTimers.TimerA;
	txnInitInfo.customSIPTimers.TimerB = pSigMgr->customSIPTimers.TimerB;
	txnInitInfo.customSIPTimers.TimerD = pSigMgr->customSIPTimers.TimerD;
	txnInitInfo.customSIPTimers.TimerE = pSigMgr->customSIPTimers.TimerE;
	txnInitInfo.customSIPTimers.TimerF = pSigMgr->customSIPTimers.TimerF;
	txnInitInfo.customSIPTimers.TimerG = pSigMgr->customSIPTimers.TimerG;
	txnInitInfo.customSIPTimers.TimerH = pSigMgr->customSIPTimers.TimerH;
	txnInitInfo.customSIPTimers.TimerI = pSigMgr->customSIPTimers.TimerI;
	txnInitInfo.customSIPTimers.TimerJ = pSigMgr->customSIPTimers.TimerJ;
	txnInitInfo.customSIPTimers.TimerK = pSigMgr->customSIPTimers.TimerK;

	txnInitInfo.timerIds.timerAId = ECRIO_SIGMGR_SIP_TIMER_A_TYPE;
	txnInitInfo.timerIds.timerBId = ECRIO_SIGMGR_SIP_TIMER_B_TYPE;

	txnInitInfo.timerIds.timerDId = ECRIO_SIGMGR_SIP_TIMER_D_TYPE;
	txnInitInfo.timerIds.timerEId = ECRIO_SIGMGR_SIP_TIMER_E_TYPE;
	txnInitInfo.timerIds.timerFId = ECRIO_SIGMGR_SIP_TIMER_F_TYPE;
	txnInitInfo.timerIds.timerGId = ECRIO_SIGMGR_SIP_TIMER_G_TYPE;
	txnInitInfo.timerIds.timerHId = ECRIO_SIGMGR_SIP_TIMER_H_TYPE;
	txnInitInfo.timerIds.timerIId = ECRIO_SIGMGR_SIP_TIMER_I_TYPE;
	txnInitInfo.timerIds.timerJId = ECRIO_SIGMGR_SIP_TIMER_J_TYPE;
	txnInitInfo.timerIds.timerKId = ECRIO_SIGMGR_SIP_TIMER_K_TYPE;

	txnInitInfo.timerExpiryNotificationFunctions.pTimerANotificationFn =
		_EcrioSigMgrTimerANotificationFn;
	txnInitInfo.timerExpiryNotificationFunctions.pTimerBNotificationFn =
		_EcrioSigMgrTimerBNotificationFn;

	txnInitInfo.timerExpiryNotificationFunctions.pTimerDNotificationFn =
		_EcrioSigMgrTimerDNotificationFn;
	txnInitInfo.timerExpiryNotificationFunctions.pTimerENotificationFn =
		_EcrioSigMgrTimerENotificationFn;
	txnInitInfo.timerExpiryNotificationFunctions.pTimerFNotificationFn =
		_EcrioSigMgrTimerFNotificationFn;
	txnInitInfo.timerExpiryNotificationFunctions.pTimerGNotificationFn =
		_EcrioSigMgrTimerGNotificationFn;
	txnInitInfo.timerExpiryNotificationFunctions.pTimerHNotificationFn =
		_EcrioSigMgrTimerHNotificationFn;
	txnInitInfo.timerExpiryNotificationFunctions.pTimerINotificationFn =
		_EcrioSigMgrTimerINotificationFn;
	txnInitInfo.timerExpiryNotificationFunctions.pTimerJNotificationFn =
		_EcrioSigMgrTimerJNotificationFn;
	txnInitInfo.timerExpiryNotificationFunctions.pTimerKNotificationFn =
		_EcrioSigMgrTimerKNotificationFn;

	txnInitInfo.pTimerNotificationData = (void *)pSigMgr;

	error = EcrioTXNInit((void **)&(pSigMgr->pTransactionMgrHndl), &txnInitInfo);

	if (error != EcrioTXNNoError)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tEcrioTXNInit() error=%u", __FUNCTION__, __LINE__, error);

		if (EcrioTXNTimerError == error)
		{
			error = ECRIO_SIGMGR_CUSTOM_TIMER_RANGE_ERROR;
		}
		else
		{
			error = ECRIO_SIGMGR_TRANSACTIONMGR_ERROR;
		}
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrDeInitalizeTransactionMgr()

Purpose:		To Deinitialize the transaction Manager.

Description:	DeInitializes the Transaction Manager.

Input:			IN EcrioSigMgrStruct* pSigMgr - Handle to Signaling Manager instance.

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrDeInitalizeTransactionMgr
(
	EcrioSigMgrStruct *pSigMgr
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	if (pSigMgr == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		goto Error_Level_01;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpSigMgr=%p",
		__FUNCTION__, __LINE__, pSigMgr);

	error = EcrioTXNExit((void **)&(pSigMgr->pTransactionMgrHndl));
	if (error != EcrioTXNNoError)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tEcrioTXNExit() error=%u", __FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_BAD_TRANSACTIONMGR_HANDLE;
		goto Error_Level_02;
	}

	error = ECRIO_SIGMGR_NO_ERROR;
	goto Error_Level_02;

Error_Level_02:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

Error_Level_01:
	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrSendMsgToTXNHandler()

Purpose:		decides whether a message is for a server or client txn and sends message to it.

Description:	decides whether a message is for a server or client txn and sends message to it.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager handle.
                EcrioSigMgrCommonInfoStruct* pSigMgrCmnInfo - Information (matching params)
                EcrioSigMgrTXNInfoStruct* pTXNInfo -TXN specific information

OutPut:			None

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrSendMsgToTXNHandler
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrCommonInfoStruct *pSigMgrCmnInfo,
	EcrioSigMgrTXNInfoStruct *pTXNInfo
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioTXNInputParamStruct txnInputParam;
	EcrioTXNTxnMatchingCriteriaEnum txnMatchingCriteria = EcrioTXNTxnMatchingCriteriaDefault;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
		"%s:%u\tpSigMgr=%p, pSigMgrCmnInfo=%p, pTXNInfo=%p",
		__FUNCTION__, __LINE__, pSigMgr, pSigMgrCmnInfo, pTXNInfo);

	if ((pSigMgrCmnInfo->pBranch == NULL) && (pTXNInfo->retransmitContext != ECRIO_SIGMGR_INVALID_CONTEXT))
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;

		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data passed, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	if (NULL == pSigMgr->pSigMgrTransportStruct)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;

		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tInsufficient data passed, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	pal_MemorySet(&txnInputParam, 0, sizeof(EcrioTXNInputParamStruct));

	if (pTXNInfo->retransmitContext != ECRIO_SIGMGR_INVALID_CONTEXT)
	{
		txnInputParam.txnMatchingParam.commonMatchingParam.pBranchParam =
			pSigMgrCmnInfo->pBranch;

		txnInputParam.txnMatchingParam.commonMatchingParam.cSeq = pSigMgrCmnInfo->cSeq;	/* Resolve BUG#7459 */

		if (pSigMgrCmnInfo->pCallId != NULL)
		{
			txnInputParam.txnMatchingParam.commonMatchingParam.pCallId =
				pSigMgrCmnInfo->pCallId;
		}

		if (pSigMgrCmnInfo->pToTag != NULL)
		{
			txnInputParam.txnMatchingParam.commonMatchingParam.pToTag =
				pSigMgrCmnInfo->pToTag;
		}

		if (pSigMgrCmnInfo->pFromTag != NULL)
		{
			txnInputParam.txnMatchingParam.commonMatchingParam.pFromTag =
				pSigMgrCmnInfo->pFromTag;
		}

		if (pSigMgrCmnInfo->responseCode != ECRIO_SIG_MGR_INVALID_RESONSE_CODE)
		{
			txnInputParam.txnMatchingParam.commonMatchingParam.responseCode =
				pSigMgrCmnInfo->responseCode;
		}

		txnInputParam.txnMatchingParam.commonMatchingParam.txnIdentifier.txnType =
			_EcrioSigMgrGetTXNType(pTXNInfo->messageType, pSigMgrCmnInfo->role);
		txnInputParam.txnMatchingParam.commonMatchingParam.txnIdentifier.pMethodName =
			pSigMgrCmnInfo->pMethodName;

		txnInputParam.txnMatchingCriteria = txnMatchingCriteria;
	}
	else
	{
		txnInputParam.txnMatchingParam.commonMatchingParam.txnIdentifier.pMethodName =
			pSigMgrCmnInfo->pMethodName;
		txnInputParam.txnMatchingParam.commonMatchingParam.pBranchParam =
			pSigMgrCmnInfo->pBranch;

		txnInputParam.txnMatchingParam.commonMatchingParam.pCallId =
			pSigMgrCmnInfo->pCallId;
	}

	txnInputParam.sessionTxnMappingParam.pSessionTxnMappingParamString =
		pTXNInfo->pSessionMappingStr;
	txnInputParam.pMsg = pTXNInfo->pMsg;
	txnInputParam.msgLen = pTXNInfo->msglen;
	
	if (pTXNInfo->pSMSRetryData != NULL)
	{
		txnInputParam.pSMSRetryData = pTXNInfo->pSMSRetryData;
	}
	txnInputParam.SmsRetryCount = pTXNInfo->retry_count;
	
	if ((pSigMgr->pIPSecDetails == NULL) || 
		((pSigMgr->pIPSecDetails != NULL) && (pSigMgr->pIPSecDetails->eIPSecState == ECRIO_SIP_IPSEC_STATE_ENUM_None)))
	{
		if (pTXNInfo->transport == EcrioSigMgrTransportUDP)
		{
			txnInputParam.transportDetails.transportType = EcrioTXNTransportTypeUDP;
			txnInputParam.uReceivedChannelIndex = pSigMgr->pSigMgrTransportStruct->sendingChannelIndex;
		}
		else if (pTXNInfo->transport == EcrioSigMgrTransportTCP)
		{
			SOCKETHANDLE pHnd = NULL;
			EcrioTxnMgrSignalingCommunicationChannelStruct pChannel = { 0 };
			u_int32 uChannelIndex = 0;
			EcrioTxnMgrSignalingCommunicationStruct *pComm = pSigMgr->pSigMgrTransportStruct->pCommunicationStruct;
			if (pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE)
			{
#ifdef ENABLE_QCMAPI
				uChannelIndex = 0;
#else
				uChannelIndex = 1;
#endif
			}
			else if (pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableUdp == Enum_FALSE)
			{
				uChannelIndex = 0;
			}
			pChannel = pComm->pChannels[uChannelIndex];
			pHnd = pChannel.hChannelHandle;
			if (pHnd)
			{
				txnInputParam.transportDetails.transportType = EcrioTXNTransportTypeTCP;
				txnInputParam.transportDetails.pRemoteHost = pTXNInfo->pRemoteHost;
				txnInputParam.transportDetails.port = pTXNInfo->remotePort;
				txnInputParam.uReceivedChannelIndex = pSigMgr->pSigMgrTransportStruct->sendingTCPChannelIndex;
				_EcrioSigMgrChangeViaTransportUDPtoTCP(txnInputParam.pMsg);
			}
			else
			{
				pHnd = _EcrioSigMgrCreateChannel(pSigMgr, pComm->pChannels[uChannelIndex].pChannelInfo, &error);
				if (error != ECRIO_SIGMGR_NO_ERROR || !pHnd)
				{
					_EcrioSigMgrChangeViaTransportTCPtoUDP(txnInputParam.pMsg);
					txnInputParam.transportDetails.transportType = EcrioTXNTransportTypeUDP;
					txnInputParam.uReceivedChannelIndex = pSigMgr->pSigMgrTransportStruct->sendingChannelIndex;
				}
				else
				{
					pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[uChannelIndex].hChannelHandle = pHnd;
				}
			}
		}
		else if (pTXNInfo->transport == EcrioSigMgrTransportTLS)
		{
			SOCKETHANDLE pHnd = NULL;
			EcrioTxnMgrSignalingCommunicationChannelStruct pChannel = { 0 };
			u_int32 uChannelIndex = 0;
			EcrioTxnMgrSignalingCommunicationStruct *pComm = pSigMgr->pSigMgrTransportStruct->pCommunicationStruct;
			if (pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE)
			{

				uChannelIndex = 3;    // udp = 0 , tcp-cleint = 1 , tcp-server = 2, tls = 3

			}
			else if (pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableUdp == Enum_FALSE)
			{
				uChannelIndex = 2;
			}
			pChannel = pComm->pChannels[uChannelIndex];
			pHnd = pChannel.hChannelHandle;
			if (pHnd)
			{
				txnInputParam.transportDetails.transportType = EcrioTXNTransportTypeTLS;
				txnInputParam.transportDetails.pRemoteHost = pTXNInfo->pRemoteHost;
				txnInputParam.transportDetails.port = pTXNInfo->remotePort;
				txnInputParam.uReceivedChannelIndex = pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex;
				_EcrioSigMgrChangeViaTransportUDPtoTLS(txnInputParam.pMsg);
				_EcrioSigMgrChangeContactTransportUDPtoTLS(txnInputParam.pMsg);
			}
			else
			{
				pHnd = _EcrioSigMgrCreateChannel(pSigMgr, pComm->pChannels[uChannelIndex].pChannelInfo, &error);
				if (error != ECRIO_SIGMGR_NO_ERROR || !pHnd)
				{
					_EcrioSigMgrChangeViaTransportTLStoUDP(txnInputParam.pMsg);
					_EcrioSigMgrChangeContactTransportTLStoUDP(txnInputParam.pMsg);
					txnInputParam.transportDetails.transportType = EcrioTXNTransportTypeUDP;
					txnInputParam.uReceivedChannelIndex = pSigMgr->pSigMgrTransportStruct->sendingChannelIndex;
				}
				else
				{
					pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[uChannelIndex].hChannelHandle = pHnd;
				}
			}
		}
		else
		{
			error = ECRIO_SIGMGR_INSUFFICIENT_DATA;

			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tInsufficient data passed, error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}
	}
	else
	{
		if (pTXNInfo->transport == EcrioSigMgrTransportUDP)
		{
			txnInputParam.transportDetails.transportType = EcrioTXNTransportTypeUDP;
			txnInputParam.uReceivedChannelIndex = pSigMgr->pSigMgrTransportStruct->sendingIPsecUdpChannelIndex;
		}
		else if (pTXNInfo->transport == EcrioSigMgrTransportTCP)
		{
			SOCKETHANDLE pHnd = NULL;
			EcrioTxnMgrSignalingCommunicationChannelStruct pChannel = { 0 };
			u_int32 uChannelIndex = 0;
			EcrioTxnMgrSignalingCommunicationStruct *pComm = pSigMgr->pSigMgrTransportStruct->pCommunicationStruct;
			if (pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE)
			{
				uChannelIndex = 4;
			}
			else if (pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableUdp == Enum_FALSE)
			{
				uChannelIndex = 3;
			}
			pChannel = pComm->pChannels[uChannelIndex];
			pHnd = pChannel.hChannelHandle;
			if (pHnd)
			{
				txnInputParam.transportDetails.transportType = EcrioTXNTransportTypeTCP;
				txnInputParam.transportDetails.pRemoteHost = pTXNInfo->pRemoteHost;
				txnInputParam.transportDetails.port = pTXNInfo->remotePort;
				txnInputParam.uReceivedChannelIndex = pSigMgr->pSigMgrTransportStruct->sendingIPsecTCPChannelIndex;
				_EcrioSigMgrChangeViaTransportUDPtoTCP(txnInputParam.pMsg);
			}
			else
			{
				pHnd = _EcrioSigMgrCreateChannel(pSigMgr, pComm->pChannels[uChannelIndex].pChannelInfo, &error);
				if (error != ECRIO_SIGMGR_NO_ERROR || !pHnd)
				{
					_EcrioSigMgrChangeViaTransportTCPtoUDP(txnInputParam.pMsg);
					txnInputParam.transportDetails.transportType = EcrioTXNTransportTypeUDP;
					txnInputParam.uReceivedChannelIndex = pSigMgr->pSigMgrTransportStruct->sendingIPsecUdpChannelIndex;
				}
				else
				{
					pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[uChannelIndex].hChannelHandle = pHnd;
				}
			}
		}
		else
		{
			error = ECRIO_SIGMGR_INSUFFICIENT_DATA;

			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tInsufficient data passed, error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}
	}

	if ((pTXNInfo->messageType == EcrioSigMgrMessageInviteRequest) || (pTXNInfo->messageType
		== EcrioSigMgrMessageNonInviteRequest))
	{
		txnInputParam.messageType = EcrioTXNMessageTypeRequest;
	}
	else
	{
		txnInputParam.messageType = EcrioTXNMessageTypeResponse;
	}

	txnInputParam.currentContext = pTXNInfo->currentContext;
	txnInputParam.retransmitContext = pTXNInfo->retransmitContext;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\t messageType=%u, retransmitContext=%u, transport=%u",
		__FUNCTION__, __LINE__, txnInputParam.messageType, txnInputParam.retransmitContext, pTXNInfo->transport);

	if (pTXNInfo->retransmitContext == ECRIO_SIGMGR_INVALID_CONTEXT)
	{
		txnInputParam.uReceivedChannelIndex = pTXNInfo->uReceivedChannelIndex;
		txnInputParam.uRemoteSendingPort = pTXNInfo->uRemoteSendingPort;

		error = EcrioTXNSendMessageDirectToTransport(pSigMgr->pTransactionMgrHndl, &txnInputParam);
		pTXNInfo->bSendDirect = Enum_TRUE;
	}
	else if (pSigMgrCmnInfo->role == EcrioSigMgrRoleUAC)
	{
		if ((pTXNInfo->messageType == EcrioSigMgrMessageInviteRequest) || (pTXNInfo->messageType ==
			EcrioSigMgrMessageNonInviteRequest))
		{
			BoolEnum bIPSecFlag = Enum_FALSE;

			if ((pSigMgr->pIPSecDetails == NULL) ||
				((pSigMgr->pIPSecDetails != NULL) && (pSigMgr->pIPSecDetails->eIPSecState == ECRIO_SIP_IPSEC_STATE_ENUM_None)))
			{
				bIPSecFlag = Enum_FALSE;
			}
			else
			{
				bIPSecFlag = Enum_TRUE;
			}

			error = EcrioTXNSendRequestToClientTxnHandler(pSigMgr->pTransactionMgrHndl,
				&txnInputParam, pTXNInfo->deleteflag, bIPSecFlag);
#if 0 /* Disabled sending over UDP for now */
			/* Start - Sending SIP Request through UDP if previous attempt failed in TCP. */
			if (error == EcrioTXNTransportError &&
				txnInputParam.transportDetails.transportType == EcrioTXNTransportTypeTCP &&
				txnInputParam.msgLen <= MAX_SIP_MAX_LEN_OVER_UDP)
			{
				_EcrioSigMgrChangeViaTransportTCPtoUDP(txnInputParam.pMsg);
			}

			/* End - Sending SIP Request through UDP if previous attempt failed in TCP. */
			/*	Support for Authorization header in all request	end	*/
#endif
		}
		else
		{
			if (pSigMgr->pSigMgrTransportStruct != NULL)
			{
				if ((pSigMgr->pIPSecDetails == NULL) ||
					((pSigMgr->pIPSecDetails != NULL) && (pSigMgr->pIPSecDetails->eIPSecState == ECRIO_SIP_IPSEC_STATE_ENUM_None)))
				{
					txnInputParam.uRemoteSendingPort = pSigMgr->pSigMgrTransportStruct->remoteSendingPort;
					if (pTXNInfo->transport == EcrioSigMgrTransportUDP)
					{
						error = EcrioTXNSetChannelIndexInTXNNode(pSigMgr->pTransactionMgrHndl, &txnInputParam, pSigMgr->pSigMgrTransportStruct->sendingChannelIndex);
					}
					else if (pTXNInfo->transport == EcrioSigMgrTransportTCP)
					{
						error = EcrioTXNSetChannelIndexInTXNNode(pSigMgr->pTransactionMgrHndl, &txnInputParam, pSigMgr->pSigMgrTransportStruct->sendingTCPChannelIndex);
					}
					if (error != EcrioTXNNoError)
					{
						if (EcrioTXNNoMatchingTxnFoundError != error && EcrioSigMgrMessageInviteResponse != pTXNInfo->messageType)
						{
							SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
								"%s:%u\tTransaction failed, error=%u",
								__FUNCTION__, __LINE__, error);
							error = ECRIO_SIGMGR_TRANSACTION_FAILED;
							goto Error_Level_01;
						}
					}
				}
				else
				{
					txnInputParam.uRemoteSendingPort = pSigMgr->pIPSecDetails->pIPsecNegParams->uPort_ps;
					if (pTXNInfo->transport == EcrioSigMgrTransportUDP)
					{
						error = EcrioTXNSetChannelIndexInTXNNode(pSigMgr->pTransactionMgrHndl, &txnInputParam, pSigMgr->pSigMgrTransportStruct->sendingIPsecUdpChannelIndex);
					}
					else if (pTXNInfo->transport == EcrioSigMgrTransportTCP)
					{
						error = EcrioTXNSetChannelIndexInTXNNode(pSigMgr->pTransactionMgrHndl, &txnInputParam, pSigMgr->pSigMgrTransportStruct->sendingIPsecTCPChannelIndex);
					}
					if (error != EcrioTXNNoError)
					{
						if (EcrioTXNNoMatchingTxnFoundError != error && EcrioSigMgrMessageInviteResponse != pTXNInfo->messageType)
						{
							SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
								"%s:%u\tTransaction failed, error=%u",
								__FUNCTION__, __LINE__, error);
							error = ECRIO_SIGMGR_TRANSACTION_FAILED;
							goto Error_Level_01;
						}
					}
				}
			}

			error = EcrioTXNSendResponseToClientTxnHandler(pSigMgr->pTransactionMgrHndl,
				&txnInputParam, pTXNInfo->deleteflag);
		}
	}
	else
	{
		if ((pTXNInfo->messageType == EcrioSigMgrMessageInviteRequest) || (pTXNInfo->messageType ==
			EcrioSigMgrMessageNonInviteRequest))
		{
			if (pTXNInfo->deleteflag == Enum_FALSE)
			{
				if (pSigMgr->pSigMgrTransportStruct != NULL)
				{
					txnInputParam.uReceivedChannelIndex = pSigMgr->pSigMgrTransportStruct->receivedOnChannelIndex;
					txnInputParam.uRemoteSendingPort = pSigMgr->pSigMgrTransportStruct->remoteSendingPort;
				}
			}

			error = EcrioTXNSendRequestToServerTxnHandler(pSigMgr->pTransactionMgrHndl,
				&txnInputParam, pTXNInfo->deleteflag);
		}
		else
		{
			error = EcrioTXNSendResponseToServerTxnHandler(pSigMgr->pTransactionMgrHndl,
				&txnInputParam, pTXNInfo->deleteflag);
		}
	}

	if (error == EcrioTXNRetransmittedMsgReceivedError)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tMessage retransmitted",	__FUNCTION__, __LINE__);

		error = ECRIO_SIGMGR_MSG_RETRANSMITTED;
		goto Error_Level_01;
	}

	if (error == EcrioTXNNoMatchingTxnFoundError)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tTransaction not found", __FUNCTION__, __LINE__);

		error = ECRIO_SIGMGR_TRANSACTION_NOT_FOUND;
		goto Error_Level_01;
	}

	// if (error != EcrioTXNNoError)
	// {
	// SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
	// "%s:%u\tTransaction failed, error=%u", __FUNCTION__, __LINE__, error);

	// error = ECRIO_SIGMGR_TRANSACTION_FAILED;
	// goto Error_Level_01;
	// }

	if (error == EcrioTXNTransportError)
		error = ECRIO_SIG_MGR_SIG_SOCKET_ERROR;
	else
		error = ECRIO_SIGMGR_NO_ERROR;

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return error;
}

void _EcrioSigMgrChangeViaTransportTCPtoUDP
(
	u_char *pMsg
)
{
	u_char *pTemp = NULL;

	pTemp = pal_SubString(pMsg, (u_char *)"SIP/2.0/TCP");
	if (pTemp == NULL)
		return;
	pTemp += pal_StringLength((u_char *)"SIP/2.0/TCP");

	

	while (*pTemp != '/')
	{
		pTemp--;
	}

	pTemp++;
	pal_MemoryCopy((void *)pTemp, pal_StringLength((const u_char*)ECRIO_SIG_MGR_SIP_UDP_PARAM), (void *)ECRIO_SIG_MGR_SIP_UDP_PARAM, pal_StringLength((const u_char*)ECRIO_SIG_MGR_SIP_UDP_PARAM));
}

void _EcrioSigMgrChangeViaTransportUDPtoTCP
(
	u_char *pMsg
)
{
	u_char *pTemp = NULL;

	pTemp = pal_SubString(pMsg, (u_char *)"SIP/2.0/UDP");
	if (pTemp == NULL)
		return;
	pTemp += pal_StringLength((u_char *)"SIP/2.0/UDP");
	

	while (*pTemp != '/')
	{
		pTemp--;
	}

	pTemp++;
	pal_MemoryCopy((void *)pTemp, pal_StringLength((const u_char*)ECRIO_SIG_MGR_SIP_TCP_PARAM), (void *)ECRIO_SIG_MGR_SIP_TCP_PARAM, pal_StringLength((const u_char*)ECRIO_SIG_MGR_SIP_TCP_PARAM));
}

void _EcrioSigMgrChangeViaTransportUDPtoTLS
(
	u_char *pMsg
)
{
	u_char *pTemp = NULL;

	pTemp = pal_SubString(pMsg, (u_char *)"SIP/2.0/UDP");
	if (pTemp == NULL)
		return;
	pTemp += pal_StringLength((u_char *)"SIP/2.0/UDP");


	while (*pTemp != '/')
	{
		pTemp--;
	}

	pTemp++;
	pal_MemoryCopy((void *)pTemp, pal_StringLength((const u_char*)ECRIO_SIG_MGR_SIP_TLS_PARAM), (void *)ECRIO_SIG_MGR_SIP_TLS_PARAM, pal_StringLength((const u_char*)ECRIO_SIG_MGR_SIP_TLS_PARAM));
}

void _EcrioSigMgrChangeViaTransportTLStoUDP
(
	u_char *pMsg
)
{
	u_char *pTemp = NULL;

	pTemp = pal_SubString(pMsg, (u_char *)"SIP/2.0/TLS");
	if (pTemp == NULL)
		return;
	pTemp += pal_StringLength((u_char *)"SIP/2.0/TLS");



	while (*pTemp != '/')
	{
		pTemp--;
	}

	pTemp++;
	pal_MemoryCopy((void *)pTemp, pal_StringLength((const u_char*)ECRIO_SIG_MGR_SIP_UDP_PARAM), (void *)ECRIO_SIG_MGR_SIP_UDP_PARAM, pal_StringLength((const u_char*)ECRIO_SIG_MGR_SIP_UDP_PARAM));
}

void _EcrioSigMgrChangeContactTransportUDPtoTLS
(
	u_char *pMsg
)
{
	u_char *pTemp = NULL;

	pTemp = pal_SubString(pMsg, (u_char *)"transport=udp");
	if (pTemp == NULL)
	{
		pTemp = pal_SubString(pMsg, (u_char *)"transport=UDP");
		if (pTemp == NULL)
		{
			return;
		}
	}
	pTemp += pal_StringLength((u_char *)"transport=udp");


	while (*pTemp != '=')
	{
		pTemp--;
	}

	pTemp++;
	pal_MemoryCopy((void *)pTemp, pal_StringLength((const u_char*)ECRIO_SIG_MGR_SIP_TLS_TRANSPORT_PARAM), (void *)ECRIO_SIG_MGR_SIP_TLS_TRANSPORT_PARAM, pal_StringLength((const u_char*)ECRIO_SIG_MGR_SIP_TLS_TRANSPORT_PARAM));
}

void _EcrioSigMgrChangeContactTransportTLStoUDP
(
	u_char *pMsg
)
{
	u_char *pTemp = NULL;

	pTemp = pal_SubString(pMsg, (u_char *)"transport=tls");
	if (pTemp == NULL)
		return;
	pTemp += pal_StringLength((u_char *)"transport=tls");



	while (*pTemp != '=')
	{
		pTemp--;
	}

	pTemp++;
	pal_MemoryCopy((void *)pTemp, pal_StringLength((const u_char*)ECRIO_SIG_MGR_SIP_UDP_TRANSPORT_PARAM), (void *)ECRIO_SIG_MGR_SIP_UDP_TRANSPORT_PARAM, pal_StringLength((const u_char*)ECRIO_SIG_MGR_SIP_UDP_TRANSPORT_PARAM));
}

/**************************************************************************

Function:		_EcrioSigMgrGetTXNType()

Purpose:		Get transaction type.

Description:	Get transaction type depending upon message type and
                role of the client.

Input:			EcrioSigMgrMessageTypeEnum messageType - Message type (INVITE or NONINVITE).
                EcrioSigMgrRoleEnum role - UAC or UAS.

Output:			EcrioTXNTxnTypeEnum - INVITE/NONINVITE client/server transaction.

Returns:		error code.
**************************************************************************/
EcrioTXNTxnTypeEnum _EcrioSigMgrGetTXNType
(
	EcrioSigMgrMessageTypeEnum messageType,
	EcrioSigMgrRoleEnum role
)
{
	EcrioTXNTxnTypeEnum txnType = EcrioTXNTxnTypeNone;

	switch (messageType)
	{
		case EcrioSigMgrMessageInviteRequest:
		case EcrioSigMgrMessageInviteResponse:
		{
			txnType = (role == EcrioSigMgrRoleUAC) ? EcrioTXNTxnTypeClientInvite :
				EcrioTXNTxnTypeServerInvite;
		}
		break;

		case EcrioSigMgrMessageNonInviteRequest:
		case EcrioSigMgrMessageNonInviteResponse:
		{
			txnType = (role == EcrioSigMgrRoleUAC) ? EcrioTXNTxnTypeClientNonInvite :
				EcrioTXNTxnTypeServerNonInvite;
		}
		break;

		default:
		{
		}
		break;
	}

	return txnType;
}

/**************************************************************************

Function:		_EcrioSigMgrTimerANotificationFn()

Purpose:		Timer A call back function.

Description:	Notify the TU for timer A expiry.

Input:			void* pData - TU data.

Output:			None.

Returns:		void.
**************************************************************************/
void _EcrioSigMgrTimerANotificationFn
(
	void *pData
)
{
	// @temp Waiting for implementation.
	(void)pData;
}

/**************************************************************************

Function:		_EcrioSigMgrTimerBNotificationFn()

Purpose:		Timer B call back function.

Description:	Notify the TU for timer B expiry.

Input:			void* pData - TU data.

Output:			None.

Returns:		void.
**************************************************************************/
void _EcrioSigMgrTimerBNotificationFn
(
	void *pData
)
{
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioTXNTUNotificationDataStruct *pTxnTuNotificationData = NULL;
	EcrioSigMgrSipMessageStruct inviteResp = { .eMethodType = EcrioSipMessageTypeNone };
	EcrioSigMgrCommonInfoStruct cmnInfo;

	EcrioTXNAppCBStruct	*pTxnAppCBData = NULL;

	if (pData == NULL)
	{
		return;
	}

	pTxnAppCBData = (EcrioTXNAppCBStruct *)pData;

	pSigMgr = (EcrioSigMgrStruct *)pTxnAppCBData->pAppCBData;
	if (pSigMgr == NULL)
	{
		return;
	}

	if (pSigMgr->bIsDeInitializing == Enum_TRUE)
	{
		return;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
		"%s:%u\tpData=%p", __FUNCTION__, __LINE__, pData);

	if (pTxnAppCBData->pTxnCBData == NULL)
	{
		goto Error_Level_01;
	}

	pTxnTuNotificationData = (EcrioTXNTUNotificationDataStruct *)pTxnAppCBData->pTxnCBData;
	if (pTxnTuNotificationData != NULL)
	{
		if (pal_StringCompare(pTxnTuNotificationData->pTxnMatchingParam->
			commonMatchingParam.txnIdentifier.pMethodName, (const u_char *)ECRIO_SIG_MGR_METHOD_INVITE) == 0)
		{
			pal_MemorySet((void *)&cmnInfo, 0, sizeof(EcrioSigMgrCommonInfoStruct));
			cmnInfo.cSeq = pTxnTuNotificationData->pTxnMatchingParam->
				commonMatchingParam.cSeq;
			cmnInfo.pBranch = pTxnTuNotificationData->pTxnMatchingParam->
				commonMatchingParam.pBranchParam;
			cmnInfo.pCallId = pTxnTuNotificationData->pTxnMatchingParam->
				commonMatchingParam.pCallId;
			cmnInfo.pFromTag = pTxnTuNotificationData->pTxnMatchingParam->
				commonMatchingParam.pFromTag;
			cmnInfo.pToTag = pTxnTuNotificationData->pTxnMatchingParam->
				commonMatchingParam.pToTag;
			cmnInfo.responseCode = ECRIO_SIGMGR_RESPONSE_CODE_REQUEST_TIMEOUT;
			cmnInfo.role = EcrioSigMgrRoleUAC;
			cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_INVITE;
			inviteResp.eMethodType = EcrioSipMessageTypeInvite;
			inviteResp.eReqRspType = EcrioSigMgrSIPResponse;
			inviteResp.responseCode = ECRIO_SIGMGR_RESPONSE_CODE_REQUEST_TIMEOUT;
			inviteResp.pReasonPhrase = (u_char *)ECRIO_SIGMGR_RESPONSE_REQUEST_TIMEOUT;

			/*Populating Mandatory header in inviteResponse*/
			if (inviteResp.pMandatoryHdrs == NULL)
			{
				pal_MemoryAllocate((u_int32)sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&inviteResp.pMandatoryHdrs);
				if (inviteResp.pMandatoryHdrs == NULL)
				{
					goto Error_Level_01;
				}
				inviteResp.pMandatoryHdrs->pCallId = pal_StringCreate((u_char *)cmnInfo.pCallId, pal_StringLength((u_char *)cmnInfo.pCallId));
				inviteResp.pMandatoryHdrs->CSeq = cmnInfo.cSeq;
			}
			_EcrioSigMgrHandleInviteResponse(pSigMgr, &inviteResp, &cmnInfo, Enum_TRUE);
			inviteResp.pReasonPhrase = NULL;
			_EcrioSigMgrReleaseSipMessage(pSigMgr, &inviteResp);

		}
	}

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}

/**************************************************************************

Function:		_EcrioSigMgrTimerDNotificationFn()

Purpose:		Timer D call back function.

Description:	Release the transaction information on timer D expiry.

Input:			void* pData - TU data.

Output:			None.

Returns:		void.
**************************************************************************/
void _EcrioSigMgrTimerDNotificationFn
(
	void *pData
)
{
	// @temp Waiting for implementation.
	(void)pData;
}

/**************************************************************************

Function:		_EcrioSigMgrTimerENotificationFn()
Purpose:		Timer E expiry notification from TXN Manager

Description:	Timer E expiry notification from TXN Manager

Input:			void* pData - callback data.

OutPut:			None

Returns:		error code.
**************************************************************************/
void _EcrioSigMgrTimerENotificationFn
(
	void *pData
)
{
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioTXNTUNotificationDataStruct *pTxnTuNotificationData = NULL;
	u_int32 contextValue = 0;

	EcrioTXNAppCBStruct	*pTxnAppCBData = NULL;

	if (pData == NULL)
	{
		return;
	}

	pTxnAppCBData = (EcrioTXNAppCBStruct *)pData;

	pSigMgr = (EcrioSigMgrStruct *)pTxnAppCBData->pAppCBData;
	if (pSigMgr == NULL)
	{
		return;
	}

	if (pSigMgr->bIsDeInitializing == Enum_TRUE)
	{
		return;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpData=%p",
		__FUNCTION__, __LINE__, pData);

	if (pTxnAppCBData->pTxnCBData == NULL)
	{
		goto Error_Level_01;
	}

	pTxnTuNotificationData =
		(EcrioTXNTUNotificationDataStruct *)pTxnAppCBData->pTxnCBData;
	if (pTxnTuNotificationData != NULL)
	{
		if (pal_StringCompare(pTxnTuNotificationData->pTxnMatchingParam->
			commonMatchingParam.txnIdentifier.pMethodName, (const u_char *)ECRIO_SIG_MGR_METHOD_REGISTER) == 0)
		{
			contextValue = ECRIO_SIGMGR_TIMER_RE_EXPIRES_REGISTER;
		}
	}

	if (contextValue != 0)
	{
		_EcrioSigMgrSendInfo(pSigMgr, EcrioSigMgrWarning, contextValue, NULL);
	}

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}

/**************************************************************************

Function:		_EcrioSigMgrTimerFNotificationFn()
Purpose:		Timer F expiry notification from TXN Manager

Description:	Timer F expiry notification from TXN Manager

Input:			void* pData - callback data.

OutPut:			None

Returns:		error code.
**************************************************************************/
void _EcrioSigMgrTimerFNotificationFn
(
	void *pData
)
{
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioTXNTUNotificationDataStruct *pTxnTuNotificationData = NULL;

	EcrioTXNAppCBStruct	*pTxnAppCBData = NULL;
	EcrioSigMgrCommonInfoStruct cmnInfo = { 0 };
	EcrioSigMgrSipMessageStruct	sipResponse = { .eMethodType = EcrioSipMessageTypeNone };

	if (pData == NULL)
	{
		return;
	}

	pTxnAppCBData = (EcrioTXNAppCBStruct *)pData;

	pSigMgr = (EcrioSigMgrStruct *)pTxnAppCBData->pAppCBData;
	if (pSigMgr == NULL)
	{
		return;
	}

	if (pSigMgr->bIsDeInitializing == Enum_TRUE)
	{
		return;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpData=%p",
		__FUNCTION__, __LINE__, pData);

	if (pTxnAppCBData->pTxnCBData == NULL)
	{
		goto Error_Level_01;
	}

	pTxnTuNotificationData =
		(EcrioTXNTUNotificationDataStruct *)pTxnAppCBData->pTxnCBData;

	if (pTxnTuNotificationData != NULL)
	{
		cmnInfo.cSeq =
			pTxnTuNotificationData->pTxnMatchingParam->commonMatchingParam.cSeq;
		cmnInfo.pBranch =
			pTxnTuNotificationData->pTxnMatchingParam->commonMatchingParam.pBranchParam;
		cmnInfo.pCallId =
			pTxnTuNotificationData->pTxnMatchingParam->commonMatchingParam.pCallId;
		cmnInfo.pFromTag =
			pTxnTuNotificationData->pTxnMatchingParam->commonMatchingParam.pFromTag;
		cmnInfo.pToTag =
			pTxnTuNotificationData->pTxnMatchingParam->commonMatchingParam.pToTag;
		cmnInfo.responseCode = ECRIO_SIGMGR_RESPONSE_CODE_REQUEST_TIMEOUT;
		cmnInfo.role = EcrioSigMgrRoleUAC;
		cmnInfo.pMethodName =
			pTxnTuNotificationData->pTxnMatchingParam->commonMatchingParam.txnIdentifier.pMethodName;

		sipResponse.eReqRspType = EcrioSigMgrSIPResponse;
		sipResponse.responseCode = ECRIO_SIGMGR_RESPONSE_CODE_REQUEST_TIMEOUT;
		sipResponse.pReasonPhrase = (u_char *)ECRIO_SIGMGR_RESPONSE_REQUEST_TIMEOUT;

		/* REGISTER Time Out */
		if (pal_StringCompare(pTxnTuNotificationData->pTxnMatchingParam->
			commonMatchingParam.txnIdentifier.pMethodName, (const u_char *)ECRIO_SIG_MGR_METHOD_REGISTER) == 0)
		{
			_EcrioSigMgrHandleRegTimeOut(pSigMgr, pTxnTuNotificationData->pTxnMatchingParam->
				commonMatchingParam.pCallId);
		}
		else if (pal_StringCompare(pTxnTuNotificationData->pTxnMatchingParam->
			commonMatchingParam.txnIdentifier.pMethodName, (const u_char *)ECRIO_SIG_MGR_METHOD_MESSAGE) == 0)
		{
			sipResponse.eMethodType = EcrioSipMessageTypeMessage;

			_EcrioSigMgrHandleMessageResponse(pSigMgr, &sipResponse, &cmnInfo);
		}
		else if (pal_StringCompare(pTxnTuNotificationData->pTxnMatchingParam->
			commonMatchingParam.txnIdentifier.pMethodName, (const u_char *)ECRIO_SIG_MGR_METHOD_PUBLISH) == 0)
		{
			sipResponse.eMethodType = EcrioSipMessageTypePublish;
			_EcrioSigMgrHandlePublishResponse(pSigMgr, &sipResponse, &cmnInfo);
		}
		else if (pal_StringCompare(pTxnTuNotificationData->pTxnMatchingParam->
			commonMatchingParam.txnIdentifier.pMethodName, (const u_char *)ECRIO_SIG_MGR_METHOD_BYE) == 0)
		{
			sipResponse.eMethodType = EcrioSipMessageTypeBye;

			_EcrioSigMgrHandleByeResponse(pSigMgr, &sipResponse, &cmnInfo, Enum_TRUE);
		}
		else if (pal_StringCompare(pTxnTuNotificationData->pTxnMatchingParam->
			commonMatchingParam.txnIdentifier.pMethodName, (const u_char *)ECRIO_SIG_MGR_METHOD_PRACK) == 0)
		{
			sipResponse.eMethodType = EcrioSipMessageTypePrack;

			_EcrioSigMgrHandlePrackResponse(pSigMgr, &sipResponse, &cmnInfo, Enum_TRUE);
		}
		else if (pal_StringCompare(pTxnTuNotificationData->pTxnMatchingParam->
			commonMatchingParam.txnIdentifier.pMethodName, (const u_char *)ECRIO_SIG_MGR_METHOD_SUBSCRIBE) == 0)
		{
			sipResponse.eMethodType = EcrioSipMessageTypeSubscribe;

			_EcrioSigMgrHandleSubscribeResponse(pSigMgr, &sipResponse, &cmnInfo, Enum_TRUE);
		}
		else if (pal_StringCompare(pTxnTuNotificationData->pTxnMatchingParam->
			commonMatchingParam.txnIdentifier.pMethodName, (const u_char *)ECRIO_SIG_MGR_METHOD_UPDATE) == 0)
		{
			sipResponse.eMethodType = EcrioSipMessageTypeUpdate;

			// _EcrioSigMgrHandleUpdateResponse(pSigMgr, &sipResponse, &cmnInfo, Enum_TRUE);
		}
	}

Error_Level_01:

	sipResponse.pReasonPhrase = NULL;
	_EcrioSigMgrReleaseSipMessage(pSigMgr, &sipResponse);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, ECRIO_SIGMGR_NO_ERROR);
}

/**************************************************************************

Function:		_EcrioSigMgrTimerHNotificationFn()
Purpose:		Timer H expiry notification from TXN Manager

Description:	Timer H expiry notification from TXN Manager

Input:			void* pData - callback data.

OutPut:			None

Returns:		error code.
**************************************************************************/
void _EcrioSigMgrTimerHNotificationFn
(
	void *pData
)
{
	EcrioSigMgrStruct *pSigMgr = NULL;
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	EcrioTXNTUNotificationDataStruct *pTxnTuNotificationData = NULL;
	EcrioSigMgrCommonInfoStruct	cmnInfo = { 0 };
	_EcrioSigMgrInviteUsageInfoStruct *pInviteInfoStruct = NULL;

	EcrioTXNAppCBStruct	*pTxnAppCBData = NULL;
	BoolEnum bDialogMatched = Enum_FALSE;
	BoolEnum bIsForked = Enum_FALSE;
	u_int32 responseCode = ECRIO_SIGMGR_INVALID_RESPONSE_CODE;
	_EcrioSigMgrDialogNodeStruct *pDialogHandle = NULL;

	if (pData == NULL)
	{
		return;
	}

	pTxnAppCBData = (EcrioTXNAppCBStruct *)pData;

	pSigMgr = (EcrioSigMgrStruct *)pTxnAppCBData->pAppCBData;
	if (pSigMgr == NULL)
	{
		return;
	}

	if (pSigMgr->bIsDeInitializing == Enum_TRUE)
	{
		return;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pTxnAppCBData->pTxnCBData == NULL)
	{
		goto Error_Level_01;
	}

	pTxnTuNotificationData = (EcrioTXNTUNotificationDataStruct *)pTxnAppCBData->pTxnCBData;
	if (pTxnTuNotificationData != NULL)
	{
		if (pal_StringCompare(pTxnTuNotificationData->pTxnMatchingParam->
			commonMatchingParam.txnIdentifier.pMethodName, (u_char *)ECRIO_SIG_MGR_METHOD_INVITE) == 0)
		{
			cmnInfo.cSeq = pTxnTuNotificationData->pTxnMatchingParam->
				commonMatchingParam.cSeq;
			cmnInfo.pBranch = pTxnTuNotificationData->pTxnMatchingParam->
				commonMatchingParam.pBranchParam;
			cmnInfo.pCallId = pTxnTuNotificationData->pTxnMatchingParam->
				commonMatchingParam.pCallId;
			cmnInfo.pFromTag = pTxnTuNotificationData->pTxnMatchingParam->
				commonMatchingParam.pFromTag;
			cmnInfo.pToTag = pTxnTuNotificationData->pTxnMatchingParam->
				commonMatchingParam.pToTag;
			cmnInfo.role = EcrioSigMgrRoleUAS;
			cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_INVITE;

			pDialogHandle = EcrioSigMgrFindMatchedDialog(pSigMgr,
				cmnInfo.pToTag, cmnInfo.pFromTag, cmnInfo.pCallId, cmnInfo.cSeq, EcrioSipMessageTypeAck,
				EcrioSigMgrSIPRequest, &bDialogMatched, &bIsForked, &responseCode);
			if (pDialogHandle == NULL)
			{
				error = ECRIO_SIGMGR_INVITE_FAILED;
				goto Error_Level_01;
			}

			pInviteInfoStruct = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogHandle->pDialogContext;

			if (pInviteInfoStruct->callState == EcrioSigMgrCallStateRejectedAckPending)
			{
				_EcrioSigMgrInternalStructRelease(pSigMgr, EcrioSigMgrStructType_InviteUsageInfo,
					(void **)&pInviteInfoStruct, Enum_TRUE);
			}
		}
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);
	error = error;
}

/**************************************************************************

Function:		_EcrioSigMgrTimerCLNotificationFn()
Purpose:		Timer CL expiry notification from TXN Manager

Description:	Timer CL expiry notification from TXN Manager

Input:			void* pData - callback data.

OutPut:			None

Returns:		error code.
**************************************************************************/
void _EcrioSigMgrTimerCLNotificationFn
(
	void *pData
)
{
	// @temp Waiting for implementation.
	(void)pData;
}

/**************************************************************************

Function:		_EcrioSigMgrTimerGNotificationFn()
Purpose:		Timer G expiry notification from TXN Manager

Description:	Timer G expiry notification from TXN Manager

Input:			void* pData - callback data.

OutPut:			None

Returns:		error code.
**************************************************************************/
void _EcrioSigMgrTimerGNotificationFn
(
	void *pData
)
{
	// @temp Waiting for implementation.
	(void)pData;
}

/**************************************************************************

Function:		_EcrioSigMgrTimerINotificationFn()
Purpose:		Timer I expiry notification from TXN Manager

Description:	Timer I expiry notification from TXN Manager. Delete
                transaction information from SAC.

Input:			void* pData - callback data.

OutPut:			None

Returns:		error code.
**************************************************************************/
void _EcrioSigMgrTimerINotificationFn
(
	void *pData
)
{
	// @temp Waiting for implementation.
	(void)pData;
}

/**************************************************************************

Function:		_EcrioSigMgrTimerJNotificationFn()
Purpose:		Timer J expiry notification from TXN Manager

Description:	Timer J expiry notification from TXN Manager.
                Delete transaction information from SAC on this
                timer expiry.

Input:			void* pData - callback data.

OutPut:			None

Returns:		error code.
**************************************************************************/
void _EcrioSigMgrTimerJNotificationFn
(
	void *pData
)
{
	// @temp Waiting for implementation.
	(void)pData;
}

/**************************************************************************

Function:		_EcrioSigMgrTimerKNotificationFn()
Purpose:		Timer K expiry notification from TXN Manager

Description:	Timer K expiry notification from TXN Manager.
                Delete transaction information from SAC on this
                timer expiry.

Input:			void* pData - callback data.

OutPut:			None

Returns:		error code.
**************************************************************************/
void _EcrioSigMgrTimerKNotificationFn
(
	void *pData
)
{
	// @temp Waiting for implementation.
	(void)pData;
}

/**************************************************************************

Function:		EcrioTUNotifyError()

**************************************************************************/
void EcrioTUNotifyError
(
	EcrioTXNReturnCodeEnum txnRetCode,
	void *pData
)
{
	/* To remove level 4 warning*/
	pData = pData;
	txnRetCode = txnRetCode;
}

u_int32 _EcrioSigMgrGetTxnMappingParameter
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrMandatoryHeaderStruct *pMandatoryHdrs,
	u_char *pMethodName,
	u_char **ppTxnSessionMatchingParam
)
{
	EcrioTXNTxnIdentificationStruct	txnIdentifier = { 0 };
	EcrioTXNOutputParamStruct *pOutputParameter	= NULL;
	EcrioSigMgrViaStruct **ppVia = NULL;
	EcrioTXNReturnCodeEnum retCode = EcrioTXNNoError;
	u_int32	index = 0;
	u_int32	error = ECRIO_SIGMGR_TRANSACTION_NOT_FOUND;

	if ((pSigMgr == NULL) || (pMandatoryHdrs == NULL) || (pMethodName == NULL))
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
// quiet		__FUNCTION__, __LINE__);

	pal_MemorySet(&txnIdentifier, 0, sizeof(EcrioTXNTxnIdentificationStruct));

	pal_MemoryAllocate(sizeof(EcrioTXNOutputParamStruct), (void **)&pOutputParameter);

	if (pOutputParameter == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}	/* if */

	txnIdentifier.pMethodName = pMethodName;
	{
		txnIdentifier.txnType = EcrioTXNTxnTypeClientNonInvite;
	}

	retCode = EcrioTXNGetDetailsOfAllTxnMatchedTxnIdentifier(pSigMgr->pTransactionMgrHndl, &txnIdentifier, pOutputParameter);

	if (retCode != EcrioTXNNoError)
	{
		error = ECRIO_SIGMGR_TRANSACTIONMGR_ERROR;
		goto Error_Level_01;
	}

	ppVia = pMandatoryHdrs->ppVia;

	for (index = 0; index < pOutputParameter->noOfOutputParamDetails; ++index)
	{
		if (pal_StringICompare(pOutputParameter->ppOutputParamaDetails[index]->pTxnMatchingParam->commonMatchingParam.pBranchParam,
			ppVia[0]->pBranch) == 0)
		{
			if (pOutputParameter->ppOutputParamaDetails[index]->pSessionTxnMappingParam->pSessionTxnMappingParamString)
			{
				*ppTxnSessionMatchingParam = pal_StringCreate(pOutputParameter->ppOutputParamaDetails[index]->pSessionTxnMappingParam->pSessionTxnMappingParamString,
					pal_StringLength(pOutputParameter->ppOutputParamaDetails[index]->pSessionTxnMappingParam->pSessionTxnMappingParamString));
			}

			break;
		}
	}

Error_Level_01:

	if (pOutputParameter != NULL)
	{
		EcrioTXNStructRelease(pSigMgr->pTransactionMgrHndl, (void **)&pOutputParameter, EcrioTXNStructType_EcrioTXNOutputParamStruct,
			Enum_TRUE);
	}

// @note Recharacterize this function... Caller doesn't care if it is not found...
// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
// quiet		__FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrInitalizeTransactionMgr()

Purpose:		To initialize the Transaction Manager.

Description:	Initializes the Transaction manager.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager handle.

OutPut:			None

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrChangeTimersInTransactionMgr
(
	EcrioSigMgrStruct *pSigMgr
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioTXNCustomTimersStruct customSIPTimers = {0};

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpSigMgr=%p",
		__FUNCTION__, __LINE__, pSigMgr);

	pal_MemorySet(
		&(customSIPTimers),
		0,
		sizeof(EcrioTXNCustomTimersStruct));

	customSIPTimers.T1 = pSigMgr->customSIPTimers.T1;
	customSIPTimers.T2 = pSigMgr->customSIPTimers.T2;
	customSIPTimers.T4 = pSigMgr->customSIPTimers.T4;
	customSIPTimers.TimerA = pSigMgr->customSIPTimers.TimerA;
	customSIPTimers.TimerB = pSigMgr->customSIPTimers.TimerB;
	customSIPTimers.TimerD = pSigMgr->customSIPTimers.TimerD;
	customSIPTimers.TimerE = pSigMgr->customSIPTimers.TimerE;
	customSIPTimers.TimerF = pSigMgr->customSIPTimers.TimerF;
	customSIPTimers.TimerG = pSigMgr->customSIPTimers.TimerG;
	customSIPTimers.TimerH = pSigMgr->customSIPTimers.TimerH;
	customSIPTimers.TimerI = pSigMgr->customSIPTimers.TimerI;
	customSIPTimers.TimerJ = pSigMgr->customSIPTimers.TimerJ;
	customSIPTimers.TimerK = pSigMgr->customSIPTimers.TimerK;

	error = EcrioTXNChangeCustomTimerValues(pSigMgr->pTransactionMgrHndl, &customSIPTimers);
	if (error != EcrioTXNNoError)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tEcrioTXNChangeCustomTimerValues() error=%u",
			__FUNCTION__, __LINE__, error);

		if (EcrioTXNTimerError == error)
		{
			error = ECRIO_SIGMGR_CUSTOM_TIMER_RANGE_ERROR;
		}
		else
		{
			error = ECRIO_SIGMGR_TRANSACTIONMGR_ERROR;
		}
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/* Start - Delete Transaction API Support */
/************************************************************

Function:		EcrioSigMgrDeleteAllTransaction()

Purpose:		Release all the transaction from transaction manager.

Description:	Release all the transaction from transaction manager.

Input:			SIGMGRHANDLE sigMgrHandle - SigMgr handle.

Output:			None.

Returns:		Error code.
**************************************************************/
u_int32 EcrioSigMgrDeleteAllTransaction
(
	SIGMGRHANDLE sigMgrHandle
)
{
	/* Start - Variable Declaration */
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;

	/* End - Variable Declaration */

	/* Start - check for incoming parameters */
	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	/* End - check for incoming parameters */

	/* Type Casting of Handle to Structure */
	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",	__FUNCTION__, __LINE__);

	/* TXN Layer API Calling */
	error = EcrioTXNDeleteTxnAll(pSigMgr->pTransactionMgrHndl);
	if (error != EcrioTXNNoError)
	{
		error = ECRIO_SIGMGR_TRANSACTIONMGR_ERROR;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u", __FUNCTION__, __LINE__, error);

	return error;
}


void _EcrioSigMgrGetTxnSMSRetryParameter
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrCommonInfoStruct *pSigMgrCmnInfo,
	EcrioSigMgrTXNInfoStruct *pTXNInfo,
	u_char ** 	ppBuffer,
	u_int32*	SmsRetryCount
)


{
	EcrioTXNInputParamStruct txnInputParam;
	EcrioTXNTxnMatchingCriteriaEnum txnMatchingCriteria = EcrioTXNTxnMatchingCriteriaDefault;
	EcrioTXNReturnCodeEnum retCode = EcrioTXNNoError;
	u_char *			pSMSRetry = NULL;
	u_int32 SmsRetryCountTemp = 0;

	if(ppBuffer == NULL)
	{
		return;
	}
	pal_MemorySet(&txnInputParam, 0, sizeof(EcrioTXNInputParamStruct));

	if (pTXNInfo->retransmitContext != ECRIO_SIGMGR_INVALID_CONTEXT)
	{
		txnInputParam.txnMatchingParam.commonMatchingParam.pBranchParam =
			pSigMgrCmnInfo->pBranch;
		txnInputParam.txnMatchingParam.commonMatchingParam.cSeq = pSigMgrCmnInfo->cSeq;	/* Resolve BUG#7459 */

		if (pSigMgrCmnInfo->pCallId != NULL)
		{
			txnInputParam.txnMatchingParam.commonMatchingParam.pCallId =
				pSigMgrCmnInfo->pCallId;
		}

		if (pSigMgrCmnInfo->pToTag != NULL)
		{
			txnInputParam.txnMatchingParam.commonMatchingParam.pToTag =
				pSigMgrCmnInfo->pToTag;
		}

		if (pSigMgrCmnInfo->pFromTag != NULL)
		{
			txnInputParam.txnMatchingParam.commonMatchingParam.pFromTag =
				pSigMgrCmnInfo->pFromTag;
		}

		if (pSigMgrCmnInfo->responseCode != ECRIO_SIG_MGR_INVALID_RESONSE_CODE)
		{
			txnInputParam.txnMatchingParam.commonMatchingParam.responseCode =
				pSigMgrCmnInfo->responseCode;
		}

		txnInputParam.txnMatchingParam.commonMatchingParam.txnIdentifier.txnType =
			_EcrioSigMgrGetTXNType(pTXNInfo->messageType, pSigMgrCmnInfo->role);
		txnInputParam.txnMatchingParam.commonMatchingParam.txnIdentifier.pMethodName =
			pSigMgrCmnInfo->pMethodName;

		txnInputParam.txnMatchingCriteria = txnMatchingCriteria;
	}
	else
	{
		txnInputParam.txnMatchingParam.commonMatchingParam.txnIdentifier.pMethodName =
			pSigMgrCmnInfo->pMethodName;
		txnInputParam.txnMatchingParam.commonMatchingParam.pBranchParam =
			pSigMgrCmnInfo->pBranch;

		txnInputParam.txnMatchingParam.commonMatchingParam.pCallId =
			pSigMgrCmnInfo->pCallId;
	}

	txnInputParam.sessionTxnMappingParam.pSessionTxnMappingParamString =
			pTXNInfo->pSessionMappingStr;
	txnInputParam.pMsg = pTXNInfo->pMsg;
	txnInputParam.msgLen = pTXNInfo->msglen;


	EcrioTXNFindMatchingTxnSMSRetryData(pSigMgr->pTransactionMgrHndl,
		&(txnInputParam.txnMatchingParam),
		txnInputParam.txnMatchingCriteria,
		&retCode, &pSMSRetry, &SmsRetryCountTemp);

	if(retCode != EcrioTXNNoError)
	{
		pSMSRetry = NULL;
		SmsRetryCountTemp = 0;
	}
	*ppBuffer = pSMSRetry;
	*SmsRetryCount = SmsRetryCountTemp;

}


/* End - Delete Transaction API Support */
