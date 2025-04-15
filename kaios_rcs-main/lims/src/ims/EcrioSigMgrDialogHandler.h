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

#ifndef __ECRIO_SIG_MGR_DIALOG_HANDLER_H__
#define __ECRIO_SIG_MGR_DIALOG_HANDLER_H__

typedef	enum
{
	_EcrioSigMgrDialogType_INVITE,
	_EcrioSigMgrDialogType_SUBSCRIBE
} _EcrioSigMgrDialogTypeEnum;

typedef enum
{
	_EcrioSigMgrDialogState_Initialized,
	_EcrioSigMgrDialogState_Early,
	_EcrioSigMgrDialogState_Confirmed
} _EcrioSigMgrDialogStateEnum;

typedef	struct _tag_EcrioSigMgrForkedMessageListNodeStruct
{
	_EcrioSigMgrDialogStateEnum	eDialogState;
	u_char *pRemoteTag;
	u_int32	uRemoteCSeq;
	EcrioSigMgrContactStruct *pRemoteContact;
	BoolEnum bIsEarlyDialogTermination;
	struct _tag_EcrioSigMgrForkedMessageListNodeStruct	*pNext;
} _EcrioSigMgrForkedMessageListNodeStruct;

typedef	struct _tag_EcrioSigMgrDialogNodeStruct
{
	EcrioSigMgrNameAddrWithParamsStruct	*pLocalUri;
	EcrioSigMgrNameAddrWithParamsStruct	*pRemoteUri;
	u_char *pLocalTag;
	u_char *pCallId;
	BoolEnum bIsFirst200OKReceived;
	u_int32	uLocalCSeq;
	EcrioSigMgrContactStruct *pLocalContact;
	EcrioSigMgrRouteStruct *pRouteSet;
	u_int32	uLastReceivedInviteCSeq;
	u_int32	uLastSentInviteCSeq;
	_EcrioSigMgrDialogTypeEnum eDialogType;
	EcrioSigMgrCallbackRegisteringModuleEnums eModule;
	void *pDialogContext;
	void *pAppData;
	TIMERHANDLE timerMId;
	_EcrioSigMgrForkedMessageListNodeStruct *pForkedMsgHead;
	_EcrioSigMgrForkedMessageListNodeStruct *pCurrForkedMsgNode;
	_EcrioSigMgrForkedMessageListNodeStruct *pAcceptedForkedMsgNode;
} _EcrioSigMgrDialogNodeStruct;

_EcrioSigMgrDialogNodeStruct *_EcrioSigMgrCreateDialog
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct	*pSipMessage,
	BoolEnum bIncomingRequest,
	void *pDialogContext
);

u_int32	_EcrioSigMgePopulateHeaderFromDialogAndUpdateState
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct	*pSipMessage,
	_EcrioSigMgrDialogNodeStruct *pDialogNode
);

_EcrioSigMgrDialogNodeStruct *EcrioSigMgrFindMatchedDialog
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pLocalTag,
	u_char *pRemoteTag,
	u_char *pCallId,
	u_int32 CSeq,
	EcrioSipMessageTypeEnum eMethodType,
	EcrioSigMgrSIPMessageTypeEnum eReqRspType,
	BoolEnum *pDialogMatched,
	BoolEnum *pForkedRequest,
	u_int32	*pResponseCode
);

_EcrioSigMgrDialogNodeStruct *EcrioSigMgrMatchDialogAndUpdateState
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct	*pSipMessage,
	BoolEnum *pForkedRequest,
	u_int32	*pResponseCode
);

u_int32	_EcrioSigMgrDeleteDialog
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrDialogNodeStruct *pDialogNode
);

u_int32	_EcrioSigMgrReleaseDialogNodeStruct
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrDialogNodeStruct *pDialogNode
);
#endif	// __ECRIO_SIG_MGR_DIALOG_HANDLER_H__
