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

#ifndef		_ECRIO_TXN_INTERNAL_H_
#define		_ECRIO_TXN_INTERNAL_H_

#include "EcrioTXN.h"

/* The build project should define ENABLE_LOG to enable logging using the Ecrio
   PAL logging mechanism. */
#ifdef ENABLE_LOG
#define TXNMGRLOGV(a, b, c, ...)		pal_LogMessage((a), KLogLevelVerbose, KLogComponentSignaling | KLogComponent_Engine, (b), (c),##__VA_ARGS__);
#define TXNMGRLOGI(a, b, c, ...)		pal_LogMessage((a), KLogLevelInfo, KLogComponentSignaling | KLogComponent_Engine, (b), (c),##__VA_ARGS__);
#define TXNMGRLOGD(a, b, c, ...)		pal_LogMessage((a), KLogLevelDebug, KLogComponentSignaling | KLogComponent_Engine, (b), (c),##__VA_ARGS__);
#define TXNMGRLOGW(a, b, c, ...)		pal_LogMessage((a), KLogLevelWarning, KLogComponentSignaling | KLogComponent_Engine, (b), (c),##__VA_ARGS__);
#define TXNMGRLOGE(a, b, c, ...)		pal_LogMessage((a), KLogLevelError, KLogComponentSignaling | KLogComponent_Engine, (b), (c),##__VA_ARGS__);

#define TXNMGRLOGDUMP(a, b, c, d)		pal_LogMessageDump((a), KLogLevelVerbose, KLogComponentSignaling | KLogComponent_Engine, (b), (c), (d));
#else
#define TXNMGRLOGV(a, b, c, ...)		if (a != NULL) {;}
#define TXNMGRLOGI(a, b, c, ...)		if (a != NULL) {;}
#define TXNMGRLOGD(a, b, c, ...)		if (a != NULL) {;}
#define TXNMGRLOGW(a, b, c, ...)		if (a != NULL) {;}
#define TXNMGRLOGE(a, b, c, ...)		if (a != NULL) {;}

#define TXNMGRLOGDUMP(a, b, c, d)
#endif

#define     ECRIO_SIGCOMP_MESSAGE						0xF8
#define		ECRIO_TXN_TIMER_RETRANSMIT_MAX				0xFF
#define		TXN_NODE_TIMER_ARRAYSIZE_MAX				2

#define ECRIO_TXN_SIP_T1_TIMER_INTERVAL					(500U)
#define ECRIO_TXN_SIP_T2_TIMER_INTERVAL					(4000U)
#define ECRIO_TXN_SIP_T4_TIMER_INTERVAL					(5000U)
#define ECRIO_TXN_SIP_D_TIMER_INTERVAL					(32000U)
#define ECRIO_TXN_SIP_MIN_D_TIMER_INTERVAL				(32000U)

#define	ECRIO_TXN_SIP_TCP_WAIT_TIMER_INTERVAL			(100U)

#define		ECRIO_TXN_TXN_STATE_NAME_NONE				(u_char *)"None state"
#define		ECRIO_TXN_TXN_STATE_NAME_CALLING			(u_char *)"Calling state"
#define		ECRIO_TXN_TXN_STATE_NAME_TRYING				(u_char *)"Trying state"
#define		ECRIO_TXN_TXN_STATE_NAME_PROCEEDING			(u_char *)"Proceeding state"
#define		ECRIO_TXN_TXN_STATE_NAME_PROGRESSING		(u_char *)"Progressing state"
#define		ECRIO_TXN_TXN_STATE_NAME_COMPLETED			(u_char *)"Completed state"
#define		ECRIO_TXN_TXN_STATE_NAME_CONFIRMED			(u_char *)"Confirmed state"
#define		ECRIO_TXN_TXN_STATE_NAME_TERMINATED			(u_char *)"Terminated state"

#define		ECRIO_TXN_TXN_TYPE_NAME_REGISTER_CLIENT		(u_char *)"REGISTER Client transaction"
#define		ECRIO_TXN_TXN_TYPE_NAME_MESSAGE_CLIENT		(u_char *)"MESSAGE Client transaction"
#define		ECRIO_TXN_TXN_TYPE_NAME_MESSAGE_SERVER		(u_char *)"MESSAGE Server transaction"
#define		ECRIO_TXN_TXN_TYPE_NAME_INVITE_CLIENT		(u_char *)"INVITE Client transaction"
#define		ECRIO_TXN_TXN_TYPE_NAME_INVITE_SERVER		(u_char *)"INVITE Server transaction"
#define		ECRIO_TXN_TXN_TYPE_NAME_CANCEL_CLIENT		(u_char *)"CANCEL Client transaction"
#define		ECRIO_TXN_TXN_TYPE_NAME_CANCEL_SERVER		(u_char *)"CANCEL Server transaction"
#define		ECRIO_TXN_TXN_TYPE_NAME_BYE_CLIENT			(u_char *)"BYE Client transaction"
#define		ECRIO_TXN_TXN_TYPE_NAME_BYE_SERVER			(u_char *)"BYE Server transaction"

#define		ECRIO_TXN_SIP_TIMER_A_NAME					(u_char *)"Timer A"
#define		ECRIO_TXN_SIP_TIMER_B_NAME					(u_char *)"Timer B"
#define		ECRIO_TXN_SIP_TIMER_D_NAME					(u_char *)"Timer D"
#define		ECRIO_TXN_SIP_TIMER_E_NAME					(u_char *)"Timer E"
#define		ECRIO_TXN_SIP_TIMER_F_NAME					(u_char *)"Timer F"
#define		ECRIO_TXN_SIP_TIMER_G_NAME					(u_char *)"Timer G"
#define		ECRIO_TXN_SIP_TIMER_H_NAME					(u_char *)"Timer H"
#define		ECRIO_TXN_SIP_TIMER_I_NAME					(u_char *)"Timer I"
#define		ECRIO_TXN_SIP_TIMER_J_NAME					(u_char *)"Timer J"
#define		ECRIO_TXN_SIP_TIMER_K_NAME					(u_char *)"Timer K"

#define		_ECRIO_TXN_TXN_NAME_STR_MAX					100U

#define		ECRIO_TXN_IncrementSipTimer(sipTimer)		(sipTimer << 1)
#define		ECRIO_TXN_DecrementSipTimer(sipTimer)		(sipTimer >> 1)

#define		ECRIO_TXN_MAX_UINT_LENGTH		8

#define     ECRIO_TXN_MAX_NO_OF_CHANNELS	1000U

typedef	enum
{
	_EcrioTXNStructType_None = 0,
	_EcrioTXNStructType_EcrioTXNInitParamStruct,
	_EcrioTXNStructType_EcrioTXNInputParamStruct,
	_EcrioTXNStructType_EcrioTXNOutputParamStruct,
	_EcrioTXNStructType_EcrioTXNTUNotificationDataStruct,
	_EcrioTXNStructType_EcrioTXNTxnIdentificationStruct,
	_EcrioTXNStructType_EcrioTXNTxnMatchingCommonParamStruct,
	_EcrioTXNStructType_EcrioTXNTxnMatchingExtraParamStruct,
	_EcrioTXNStructType_EcrioTXNTxnMatchingParamStruct,
	_EcrioTXNStructType_EcrioTXNSessionTxnMappingParamStruct,
	_EcrioTXNStructType_EcrioTXNTransportDetailsStruct,
	_EcrioTXNStructType_EcrioTXNOutputParamDetailsStruct,
	_EcrioTXNStructType_EcrioTXNTimerDetailsStruct,
	_EcrioTXNStructType_EcrioTXNTxnNodeStruct,
	_EcrioTXNStructType_EcrioTXNSentMsgDetailsStruct,
	_EcrioTXNStructType_EcrioTxnMgrTransportStruct,
	_EcrioTXNStructType_EcrioTxnMgrSignalingCommunicationStruct,
	_EcrioTXNStructType_EcrioTxnMgrSignalingCommunicationChannelStruct,
	_EcrioTXNStructType_EcrioTxnMgrSignalingCommunicationChannelInfoStruct,
	_EcrioTXNStructType_EcrioTXNDataStruct
} _EcrioTXNStructTypeEnum;

/**
*Enumeration for sip timer types for which callback functions defined
*/
typedef	enum
{
	EcrioTXNSipTimerTypeNone	= 0x0000,
	EcrioTXNSipTimerTypeA		= 0x0001,
	EcrioTXNSipTimerTypeB		= 0x0002,
	EcrioTXNSipTimerTypeD		= 0x0004,
	EcrioTXNSipTimerTypeE		= 0x0008,
	EcrioTXNSipTimerTypeF		= 0x0010,
	EcrioTXNSipTimerTypeG		= 0x0020,
	EcrioTXNSipTimerTypeH		= 0x0040,

	EcrioTXNSipTimerTypeI = 0x0080,

	EcrioTXNSipTimerTypeJ	= 0x0100,
	EcrioTXNSipTimerTypeK	= 0x0200,
} EcrioTXNSipTimerTypeEnum;

typedef struct
{
	s_int32	absoluteDuration;
	TIMERHANDLE	timerID;
	BoolEnum isRunning;
	EcrioTXNSipTimerTypeEnum sipTimerType;
} _EcrioTXNTimerDetailsStruct;

typedef struct tag_TXN_Node
{
	EcrioTXNTxnMatchingParamStruct *pTxnMatchingParam;
	EcrioTXNTxnMatchingParamStruct *pTxnMatchingParamACK;
	EcrioTXNSessionTxnMappingParamStruct *pSessionTxnMappingParam;
	EcrioTXNTxnStateEnum txnState;
	u_char *pMsg;
	u_int32	msgLen;
	_EcrioTXNTimerDetailsStruct	timerDetails[TXN_NODE_TIMER_ARRAYSIZE_MAX];
	BoolEnum bRetryEnabled;
	s_int32	retransmissionContext;
	EcrioTXNTransportDetailsStruct transportDetails;
	void	*pSMSRetryData;	
	u_int16	SmsRetryCount;
	void *pTxnData;
	struct	tag_TXN_Node *pPrev;
	struct	tag_TXN_Node *pNext;
	u_int32	uChannelIndex;
} _EcrioTXNTxnNodeStruct;

typedef	struct
{
	EcrioTXNInitParamStruct	*pTxnInitParam;
	_EcrioTXNTxnNodeStruct *pTxnListHead;
	_EcrioTXNTxnNodeStruct *pTxnListTail;
} _EcrioTXNDataStruct;

void _EcrioTXNStructRelease
(
	_EcrioTXNDataStruct	*pTxnData,
	void **ppInputStruct,
	_EcrioTXNStructTypeEnum	structType,
	BoolEnum bRelease
);

_EcrioTXNTxnNodeStruct *_EcrioTXNFindMatchingTxn
(
	_EcrioTXNDataStruct	*pTxnData,
	EcrioTXNTxnMatchingParamStruct *pTxnMatchingParam,
	EcrioTXNTxnMatchingCriteriaEnum	txnMatchingCriteria,
	EcrioTXNReturnCodeEnum *pTxnRetCode
);

EcrioTXNReturnCodeEnum _EcrioTXNMatchCommonMatchingParam
(
	_EcrioTXNDataStruct	*pTxnData,
	EcrioTXNTxnMatchingParamStruct *pTxnMatchingParamStored,
	EcrioTXNTxnMatchingParamStruct *pTxnMatchingParamInput,
	EcrioTXNTxnMatchingCriteriaEnum	txnMatchingCriteria
);

EcrioTXNReturnCodeEnum _EcrioTXNDeleteAllTxnNode
(
	_EcrioTXNDataStruct	*pTxnData
);

_EcrioTXNTxnNodeStruct *_EcrioTXNCreateTxnNode
(
	_EcrioTXNDataStruct	*pTxnData,
	EcrioTXNInputParamStruct *pTxnInputParam,
	u_int32	uChannelIndex,
	EcrioTXNReturnCodeEnum *pTxnRetCode
);

_EcrioTXNTxnNodeStruct *_EcrioTXNCreateTxn
(
	_EcrioTXNDataStruct	*pTxnData,
	EcrioTXNInputParamStruct *pTxnInputParam,
	u_int32	uChannelIndex,
	EcrioTXNReturnCodeEnum *pTxnRetCode
);

EcrioTXNReturnCodeEnum _EcrioTXNDeleteTxnNode
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode
);

EcrioTXNReturnCodeEnum _EcrioTXNDeleteTxn
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode
);

u_int32 _EcrioTXNGetRunningTimerIndex
(
	_EcrioTXNTxnNodeStruct *pTxnNode,
	EcrioTXNSipTimerTypeEnum sipTimerType
);

u_int32 _EcrioTXNGetFreeTimerIndex
(
	_EcrioTXNTxnNodeStruct *pTxnNode
);

u_int32 *_EcrioTXNGetRunningTimerIds
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode,
	u_int16	*pRunningTimerCount
);

EcrioTXNReturnCodeEnum _EcrioTXNStartTxnTimerInternal
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode,
	EcrioTXNSipTimerTypeEnum sipTimerType
);

EcrioTXNReturnCodeEnum _EcrioTXNStartTxnTimer
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode,
	EcrioTXNTimerTypeEnum timerType
);

EcrioTXNReturnCodeEnum _EcrioTXNStopTxnTimerInternal
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode,
	EcrioTXNSipTimerTypeEnum sipTimerType
);

EcrioTXNReturnCodeEnum _EcrioTXNStopTxnTimer
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode,
	EcrioTXNTimerTypeEnum timerType
);

EcrioTXNReturnCodeEnum _EcrioTXNUpdateSessionTxnMappingParam
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode,
	EcrioTXNInputParamStruct *pTxnInputParam
);

EcrioTXNReturnCodeEnum _EcrioTXNUpdateBasicTxnMatchingParam
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode,
	EcrioTXNInputParamStruct *pTxnInputParam
);

EcrioTXNReturnCodeEnum _EcrioTXNCopyTxnMatchingParamACK
(
	_EcrioTXNDataStruct	*pTxnData,
	EcrioTXNInputParamStruct *pTxnInputParam,
	_EcrioTXNTxnNodeStruct *pTxnNode
);

EcrioTXNReturnCodeEnum _EcrioTXNAllocateCopyTxnMatchingParam
(
	_EcrioTXNDataStruct	*pTxnData,
	EcrioTXNTxnMatchingParamStruct *pSrcTxnMatchingParam,
	EcrioTXNTxnMatchingParamStruct **ppDestTxnMatchingParam
);

EcrioTXNReturnCodeEnum _EcrioTXNAllocateCopySessionTxnMappingParam
(
	_EcrioTXNDataStruct	*pTxnData,
	EcrioTXNSessionTxnMappingParamStruct *pSrcSessionTxnMappingParam,
	EcrioTXNSessionTxnMappingParamStruct **ppDestSessionTxnMappingParam
);

u_char *_EcrioTXNGetTxnStateName
(
	EcrioTXNTxnStateEnum txnState
);

u_char *_EcrioTXNGetTxnName
(
	EcrioTXNTxnIdentificationStruct	txnIdentifier
);

u_char *_EcrioTXNGetSipTimerName
(
	_EcrioTXNDataStruct	*pTxnData,
	EcrioTXNSipTimerTypeEnum sipTimerType
);

void _EcrioTXNTimerACallback
(
	void *pCallbackData,
	TIMERHANDLE timerID
);
void _EcrioTXNTimerBCallback
(
	void *pCallbackData,
	TIMERHANDLE timerID
);
void _EcrioTXNTimerDCallback
(
	void *pCallbackData,
	TIMERHANDLE timerID
);
void _EcrioTXNTimerECallback
(
	void *pCallbackData,
	TIMERHANDLE timerID
);
void _EcrioTXNTimerFCallback
(
	void *pCallbackData,
	TIMERHANDLE timerID
);
void _EcrioTXNTimerGCallback
(
	void *pCallbackData,
	TIMERHANDLE timerID
);
void _EcrioTXNTimerHCallback
(
	void *pCallbackData,
	TIMERHANDLE timerID
);
void _EcrioTXNTimerICallback
(
	void *pCallbackData,
	TIMERHANDLE timerID
);
void _EcrioTXNTimerJCallback
(
	void *pCallbackData,
	TIMERHANDLE timerID
);
void _EcrioTXNTimerKCallback
(
	void *pCallbackData,
	TIMERHANDLE timerID
);

EcrioTXNReturnCodeEnum _EcrioTXNGetSipTimerType
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode,
	EcrioTXNTimerTypeEnum timerType,
	BoolEnum bCheckTransState,
	EcrioTXNSipTimerTypeEnum *pSipTimerType
);

EcrioTXNReturnCodeEnum _EcrioTXNGetTimerType
(
	_EcrioTXNDataStruct	*pTxnData,
	EcrioTXNSipTimerTypeEnum sipTimerType,
	EcrioTXNTimerTypeEnum *pTimerType
);

EcrioTXNReturnCodeEnum _EcrioTXNGenerateTUNotificationData
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode,
	EcrioTXNTUNotificationDataStruct **ppTxnTuNotificationData,
	BoolEnum bUseAckParam
);

EcrioTXNReturnCodeEnum _EcrioTXNHandleTimerExpiryRetrans
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode
);

/* This function returns dummy timer ID as recieved from upper layer, used for backward compatibility */
u_int32 _EcrioTXNGetDummyTimerID
(
	_EcrioTXNDataStruct	*pTxnData,
	EcrioTXNSipTimerTypeEnum sipTimer
);

#ifdef ENABLE_QCMAPI
u_int32	_EcrioTxnMgrSendDataToCommunicationChannel
(
	_EcrioTXNDataStruct *pTxnData,
	u_char *pData,
	u_int32 uDataLength,
	u_int32	uChannelIndex,
	EcrioTXNMessageTypeEnum	messageType,
	u_char *pCallId,
	BoolEnum bIsResponse
);
#else
u_int32	_EcrioTxnMgrSendDataToCommunicationChannel
(
	_EcrioTXNDataStruct *pTxnData,
	u_char *pData,
	u_int32 uDataLength,
	u_int32	uChannelIndex
);
#endif

u_int32	_EcrioTxnMgrModifyReceivedOnChannelIndex
(
	_EcrioTXNDataStruct *pTxnData, 
	u_int32	remoteSendingPort, 
	u_int32 receivedOnChannelId
);

BoolEnum _EcrioTxnMgrCheckRequestReceivedOnProperIPsec
(
	_EcrioTXNDataStruct	*pTxnData, 
	u_int32	remoteSendingPort, 
	u_int32 receivedOnChannelId, 
	BoolEnum	bCheckState
);

BoolEnum _EcrioTxnMgrCheckResponseReceivedOnProperIPsec
(
	_EcrioTXNDataStruct *pTxnData,
	u_int32	remoteSendingPort,
	u_int32 receivedOnChannelId,
	u_int32	sendingChannelId,
	u_char	*pMethodName,
	u_int32	responseCode
);

#endif /*	_ECRIO_TXN_INTERNAL_H_*/
