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

#ifndef _ECRIO_SIG_MGR_CALL_CONTROL_INTERNAL_H_
#define  _ECRIO_SIG_MGR_CALL_CONTROL_INTERNAL_H_

/**
* @def
*Constant Definition - START
*/

typedef	struct
{
	u_int32	uRseq;
	BoolEnum bAcknowledged;
} _EcrioSigMgrRSeqStruct;

typedef	struct
{
	BoolEnum bPrackInitiator;
	u_int32	uRSeqArrayCount;
	_EcrioSigMgrRSeqStruct *pRseqArray;
	u_int32	uLastRseq;
} _EcrioSigMgrPrackDetailsStruct;

typedef struct
{
	/* Call state*/
	EcrioSigMgrCallStateEnum callState;
	BoolEnum bPrackEnabled;
	_EcrioSigMgrPrackDetailsStruct *pPrackDetails;
	/** Barnch parameter of Invite.*/
	u_char *pBranch;
	/** Application data*/
	void *pData;
	/** retransmitted message*/
	u_char *pMsg;
	/** retransmitted message length*/
	u_int32 msgLength;
	/** ACK message*/
	u_char *pAckMsg;
	/** Ack message length*/
	u_int32 msgAckLength;
	/**Bool variable to identify caller or callee */
	BoolEnum isCaller;
	/** Sig Mgr Context */
	void *pSigMgr;
	/** Retransmit Timer interval */
	u_int32 retransmitInterval;
	/** Response retransmit TimerID*/
	TIMERHANDLE retransmitTimerId;
	/** 1xx Response periodic TimerID*/
	TIMERHANDLE periodic1xxTimerId;
	/* Expire timer */
	TIMERHANDLE expireTimerId;
	/* Sessin Expire timer */
	TIMERHANDLE sessionExpireTimerId;
	/** Invite Req*/
	EcrioSigMgrSipMessageStruct *pInviteReq;

	/** Session-Expires value */
	u_int32 sessionExpireVal;
	/** Min-SE value */
	u_int32 minSEVal;
	/** Is session refresher */
	BoolEnum isRefresher;
	/** Is session refresh support */
	BoolEnum isSessionRefresh;
	/** Is update in Allow header */
	BoolEnum isUpdateAllow;
} _EcrioSigMgrInviteUsageInfoStruct;

/**
*Constant Definition - END
*/

/**
* @def
*Function Definition - START
*/
u_int32 _EcrioSigMgrSendInvite
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pInviteReq,
	u_char** ppCallID
);

u_int32 _EcrioSigMgrSendAck
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrDialogNodeStruct *pDialogNode,
	BoolEnum isErrorResponse
);

u_int32 _EcrioSigMgrSendPrack
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pPrackReq
);

u_int32 _EcrioSigMgrSendBye
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrDialogNodeStruct *pDialogNode,
	EcrioSigMgrSipMessageStruct *pByeReqInfo
);

u_int32 _EcrioSigMgrSendCancel
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrDialogNodeStruct *pDialogNode,
	EcrioSigMgrSipMessageStruct *pCancelReqInfo
);

u_int32 _EcrioSigMgrSendInviteResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pInviteResp
);

u_int32 _EcrioSigMgrSendInviteErrorResponse
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData,
	EcrioSigMgrSipMessageStruct *pInviteReq,
	u_char *pReasonPhrase,
	u_int32 reasonCode
);

u_int32 _EcrioSigMgrHandleInviteResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pInviteResp,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	BoolEnum bRequestTimedout
);

u_int32 _EcrioSigMgrHandleByeResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pByeResp,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	BoolEnum bRequestTimedout
);

u_int32 _EcrioSigMgrHandleUpdateResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pUpdateResp,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	BoolEnum bRequestTimedout
);

u_int32 _EcrioSigMgrHandlePrackResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pPrackResp,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	BoolEnum bRequestTimedout
);

u_int32 _EcrioSigMgrHandleCancelResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pCancelResp,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	BoolEnum bRequestTimedout
);

u_int32 _EcrioSigMgrHandleInviteRequest
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pInviteReq,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	u_char *pReasonPhrase,
	u_int32 reasonCode
);

u_int32 _EcrioSigMgrHandleAckRequest
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pAckReq,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	u_int32 responseCode,
	u_char *pReasonPhrase
);

void _EcrioSigMgrCallStateExpireTimerCallbackUAS
(
	void *pData,
	TIMERHANDLE timerID
);

void _EcrioSigMgrCallStateRetransmitTimerCallback
(
	void *pData,
	TIMERHANDLE timerID
);

void _EcrioSigMgrCallState1xxPeriodicTimerCallback
(
	void *pData,
	TIMERHANDLE timerID
);

u_int32 _EcrioSigMgrCallStateRetransmitResponse
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrDialogNodeStruct *pInviteUsage
);

void _EcrioSigMgrCallStateSessionExpireTimerCallback
(
	void *pData,
	TIMERHANDLE timerID
);

void _EcrioSigMgrCallStateSessionRefreshTimerCallback
(
	void *pData,
	TIMERHANDLE timerID
);

/* Session refresh/expire */
u_int32 _EcrioSigMgrStartSessionRefreshExpireTimer
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage,
	_EcrioSigMgrDialogNodeStruct *pDialogNode,
	u_int32 interval,
	BoolEnum isRefresh
);

void _EcrioSigMgrTransactionTimerMCallback
(
	void *pData,
	TIMERHANDLE timerID
);

u_int32 _EcrioSigMgrHandleCancelRequest
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pCancelReq,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	u_int32 responseCode,
	u_char *pReasonPhrase
);

u_int32 _EcrioSigMgrHandleByeRequest
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pByeReq,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	u_int32 responseCode,
	u_char *pReasonPhrase
);

u_int32 _EcrioSigMgrHandleUpdateRequest
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pUpdateReq,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	u_int32 responseCode,
	u_char *pReasonPhrase
);

u_int32 _EcrioSigMgrHandlePrackRequest
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pPrackReq,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	u_int32 responseCode,
	u_char *pReasonPhrase
);

u_int32 _EcrioSigMgrSendUpdateResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pUpdateResp
);

u_int32 _EcrioSigMgrSendByeResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pByeResp,
	BoolEnum isTreminationInProgress
);

u_int32 _EcrioSigMgrSendPrackResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pPrackResp
);

u_int32 _EcrioSigMgrSendCancelResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pCancelResp
);

u_int32 _EcrioSigMgrGetCallState
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrDialogNodeStruct *pDialogNode,
	EcrioSigMgrCallStateEnum *pState
);

u_int32 _EcrioSigMgrTerminateSession
(
	SIGMGRHANDLE pSigMgrHandle,
	_EcrioSigMgrDialogNodeStruct *pDialogNode
);

u_int32 _EcrioSigMgrHandleUACEventInitialized
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pInviteReq,
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage
);

u_int32 _EcrioSigMgrHandleUACEventSendInvite
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage,
	u_char** ppCallID
);

u_int32 _EcrioSigMgrHandleUACEventSendReInvite
(
	EcrioSigMgrStruct* pSigMgr,
	EcrioSigMgrSipMessageStruct *pInviteReq,
	_EcrioSigMgrDialogNodeStruct *pDialogNode
);

u_int32 _EcrioSigMgrSendInDialogSipRequests
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipReqInfo,
	_EcrioSigMgrDialogNodeStruct *pDialogNode
);

u_int32 _EcrioSigMgrSendReferInDialog
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipReqInfo,
	_EcrioSigMgrDialogNodeStruct *pDialogNode,
	u_char** ppReferId
);

u_int32 _EcrioSigMgrCallStateChangeINVPending
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData,
	EcrioSigMgrSipMessageStruct *pInviteResp
);

u_int32 _EcrioSigMgrGeneratePrackResp
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pPrackReq,
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData,
	u_char *pReasonPhrase,
	u_int32 responseCode
);

u_int32 _EcrioSigMgrGenerateByeResp
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pByeReq,
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData,
	u_char *pReasonPhrase,
	u_int32 responseCode
);

u_int32 _EcrioSigMgrGenerateUpdateResp
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pUpdateReq,
	u_char *pReasonPhrase,
	u_int32 responseCode
);

u_int32 _EcrioSigMgrGenerateCancelResp
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pCancelReq,
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData,
	u_char *pReasonPhrase,
	u_int32 responseCode
);

u_int32	_EcrioSigMgrCheckInviteResponseInTransaction
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrInviteUsageInfoStruct *pInviteInfoStruct,
	EcrioSigMgrCommonInfoStruct *pCmnInfo
);

u_int32	_EcrioSigMgrHandle422InviteResponse
(
	EcrioSigMgrStruct* pSigMgr,
	EcrioSigMgrSipMessageStruct* pInviteResp,
	_EcrioSigMgrDialogNodeStruct* pDialogNode
);

u_int32	_EcrioSigMgrCheckTransportInTransaction
(
	EcrioSigMgrSipMessageStruct *pInviteResp,
	_EcrioSigMgrInviteUsageInfoStruct *pInviteInfoStruct
);

u_int32 _EcrioSigMgrSendErrorResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pIncomingReq,
	u_char *pMethodName,
	u_int32 responseCode,
	u_char *pReasonPhrase
);
/************************************************************************
*						Structure Release APIs
************************************************************************/
u_int32 _EcrioSigMgrReleaseInviteUsageInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsageInfoHdr
);

#endif /* _ECRIO_SIG_MGR_CALL_CONTROL_INTERNAL_H_ */
