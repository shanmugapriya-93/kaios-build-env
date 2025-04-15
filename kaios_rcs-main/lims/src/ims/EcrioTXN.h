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

#ifndef		_ECRIO_TXN_H_
#define		_ECRIO_TXN_H_

#include "EcrioTxnMgrSAC.h"

#define ECRIO_TXN_TIMER_CL_HANDING_ENABLED    1U

/**********************************************************************************************/

#include "EcrioPAL.h"

/**********************************************************************************************/
/*	Customization enabling flags	*/
// #define		ECRIO_TXN_CUSTOMIZED_ACK_HANDING_ENABLED 0
/*	#define		ECRIO_TXN_TIMER_CL_HANDING_ENABLED	*/
/*	#define		ECRIO_TXN_TIMER_I_HANDING_ENABLED	*/
// #define		ECRIO_TXN_TIMER_B_STOPPING_ON_PROVISIONAL_RESPONSE_ENABLED

/**********************************************************************************************/

/**********************************************************************************************/

/*	Constants declaration start	*/

#define	ECRIO_TXN_METHOD_NAME_INVITE	(u_char *)"INVITE"
#define	ECRIO_TXN_METHOD_NAME_ACK		(u_char *)"ACK"
#define	ECRIO_TXN_METHOD_NAME_CANCEL	(u_char *)"CANCEL"

#define ECRIO_TXN_INVALID_DUMMY_ID		0xFFFFFFFFU

#define ECRIO_TXN_SAC_ASYNC_MODE		DISABLE

/*	Constants declaration end	*/

/**********************************************************************************************/

/**********************************************************************************************/

/*	enum declaration start	*/

typedef	enum
{
	EcrioTXNNoError = 0,
	EcrioTXNInsufficientMemoryError,
	EcrioTXNInsufficientDataError,
	EcrioTXNTimerError,
	EcrioTXNTransportError,
	EcrioTXNUnknownMsgReceivedError,
	EcrioTXNMsgReceivedInWrongStateError,
	EcrioTXNNoMatchingTxnFoundError,
	EcrioTXNRetransmittedMsgReceivedError,
	EcrioTXNUnknownError,
	EcrioTXNSigCompDecompressFailError,
	EcrioTXNSigCompCompressFailError,
	EcrioTXNSigCompDeInitializeError,
	EcrioTXNMessageNotProperlyProtected
} EcrioTXNReturnCodeEnum;

typedef	enum
{
	EcrioTXNNoneTimer		= 0,
	EcrioTXNRetransmitTimer = 1,
	EcrioTXNTimeoutTimer	= 2,
	EcrioTXNWaitTimer		= 4,
} EcrioTXNTimerTypeEnum;

typedef	enum
{
	EcrioTXNTxnTypeNone = 0,
	EcrioTXNTxnTypeClientInvite,
	EcrioTXNTxnTypeClientNonInvite,
	EcrioTXNTxnTypeServerInvite,
	EcrioTXNTxnTypeServerNonInvite,
} EcrioTXNTxnTypeEnum;

typedef	enum
{
	EcrioTXNTxnStateNone = 0,
	EcrioTXNTxnStateCalling,
	EcrioTXNTxnStateTrying,
	EcrioTXNTxnStateProceeding,
	EcrioTXNTxnStateProgressing,
	EcrioTXNTxnStateCompleted,
	EcrioTXNTxnStateConfirmed,
	EcrioTXNTxnStateTerminated
} EcrioTXNTxnStateEnum;

typedef	enum
{
	EcrioTXNTxnMatchingCriteriaDefault		= 0,
	EcrioTXNTxnMatchingCriteriaCallId		= 1,
	EcrioTXNTxnMatchingCriteriaToTag		= 2,
	EcrioTXNTxnMatchingCriteriaFromTag		= 4,
	EcrioTXNTxnMatchingCriteriaCSeq			= 8,
	EcrioTXNTxnMatchingCriteriaResponseCode = 16,
	EcrioTXNTxnMatchingCriteriaExtra		= 32
} EcrioTXNTxnMatchingCriteriaEnum;

typedef	enum
{
	EcrioTXNTransportTypeNone = 0,
	EcrioTXNTransportTypeUDP,
	EcrioTXNTransportTypeTCP,
	EcrioTXNTransportTypeTLS
} EcrioTXNTransportTypeEnum;

typedef	enum
{
	EcrioTXNMessageTypeNone = 0,
	EcrioTXNMessageTypeRequest,
	EcrioTXNMessageTypeResponse
} EcrioTXNMessageTypeEnum;

typedef	enum
{
	EcrioTXNStructType_None = 0,
	EcrioTXNStructType_EcrioTXNInitParamStruct,
	EcrioTXNStructType_EcrioTXNInputParamStruct,
	EcrioTXNStructType_EcrioTXNOutputParamStruct,
	EcrioTXNStructType_EcrioTXNTUNotificationDataStruct,
	EcrioTXNStructType_EcrioTXNSentMsgDetailsStruct,
	EcrioTXNStructType_EcrioTXNTxnIdentificationStruct,
	EcrioTXNStructType_EcrioTXNTxnMatchingCommonParamStruct,
	EcrioTXNStructType_EcrioTXNTxnMatchingExtraParamStruct,
	EcrioTXNStructType_EcrioTXNTxnMatchingParamStruct,
	EcrioTXNStructType_EcrioTXNSessionTxnMappingParamStruct,
	EcrioTXNStructType_EcrioTXNTransportDetailsStruct,
	EcrioTXNStructType_EcrioTXNOutputParamDetailsStruct,
	EcrioTXNStructType_EcrioTXNTransportStruct,
	EcrioTXNStructType_EcrioTxnMgrSignalingCommunicationStruct,
	EcrioTXNStructType_EcrioTxnMgrSignalingCommunicationChannelStruct,
	EcrioTXNStructType_EcrioTxnMgrSignalingCommunicationChannelInfoStruct,
} EcrioTXNStructTypeEnum;

/*	enum declaration end	*/

/**********************************************************************************************/

/**********************************************************************************************/

/*	structure declaration start	*/

typedef							void (*EcrioTXNNotificationFunction)
(
	void *
);

typedef	struct
{
	EcrioTXNNotificationFunction pTimerANotificationFn;
	EcrioTXNNotificationFunction pTimerBNotificationFn;
	EcrioTXNNotificationFunction pTimerDNotificationFn;
	EcrioTXNNotificationFunction pTimerENotificationFn;
	EcrioTXNNotificationFunction pTimerFNotificationFn;
	EcrioTXNNotificationFunction pTimerGNotificationFn;
	EcrioTXNNotificationFunction pTimerHNotificationFn;
	EcrioTXNNotificationFunction pTimerINotificationFn;
	EcrioTXNNotificationFunction pTimerJNotificationFn;
	EcrioTXNNotificationFunction pTimerKNotificationFn;
} EcrioTXNTimerExpiryNotificationFunctionStruct;

typedef struct
{
	/* All values should be non negative, 0 denotes usage of default value as per RFC */
	u_int32 T1;	/* Range - non negative - */
	u_int32 T2;
	u_int32 T4;
	u_int32 TimerA;
	u_int32 TimerB;
	u_int32 TimerD;
	u_int32 TimerE;
	u_int32 TimerF;
	u_int32 TimerG;
	u_int32 TimerH;
	u_int32 TimerI;
	u_int32 TimerJ;
	u_int32 TimerK;
} EcrioTXNCustomTimersStruct;

/* This structure is kept just for the backward compatibility and this should not be used in future developments */

typedef	struct
{
	u_int32	timerAId;
	u_int32	timerBId;
	u_int32	timerDId;
	u_int32	timerEId;
	u_int32	timerFId;
	u_int32	timerGId;
	u_int32	timerHId;

	u_int32	timerIId;

	u_int32	timerJId;
	u_int32	timerKId;
} EcrioTXNTimerIdStruct;

typedef	struct
{
	void *pal;
	void *pLogHandle;
	u_int32	timerX;
	EcrioTXNTimerIdStruct timerIds;									/* Just kept for backward compatiblity */
	EcrioTXNTimerExpiryNotificationFunctionStruct timerExpiryNotificationFunctions;

	void *pTimerNotificationData;

	void *pTxnTransportStruct;												/* To hold sac related info */

	EcrioTXNCustomTimersStruct customSIPTimers;
} EcrioTXNInitParamStruct;

typedef	struct
{
	u_char *pMethodName;
	EcrioTXNTxnTypeEnum	txnType;
} EcrioTXNTxnIdentificationStruct;

typedef	struct
{
	u_char *pBranchParam;
	EcrioTXNTxnIdentificationStruct	txnIdentifier;
	u_int32	responseCode;
	u_char *pCallId;
	u_char *pToTag;
	u_char *pFromTag;
	u_int32	cSeq;
} EcrioTXNTxnMatchingCommonParamStruct;

typedef	struct
{
	EcrioTXNTxnMatchingCommonParamStruct commonMatchingParam;
} EcrioTXNTxnMatchingParamStruct;

typedef	struct
{
	u_char *pSessionTxnMappingParamString;
} EcrioTXNSessionTxnMappingParamStruct;

typedef	struct
{
	EcrioTXNTransportTypeEnum transportType;
	u_char *pRemoteHost;
	u_int16	port;
} EcrioTXNTransportDetailsStruct;

typedef	struct
{
	EcrioTXNTxnMatchingParamStruct txnMatchingParam;
	EcrioTXNTxnMatchingCriteriaEnum	txnMatchingCriteria;
	EcrioTXNSessionTxnMappingParamStruct sessionTxnMappingParam;
	EcrioTXNMessageTypeEnum	messageType;
	u_char *pMsg;
	u_int32	msgLen;
	void	*pSMSRetryData;	
	u_int16	SmsRetryCount;
	EcrioTXNTransportDetailsStruct transportDetails;
	s_int16	currentContext;
	s_int16	retransmitContext;
	u_int32	uReceivedChannelIndex;
	u_int32	uRemoteSendingPort;
} EcrioTXNInputParamStruct;

typedef	struct
{
	EcrioTXNTxnMatchingParamStruct *pTxnMatchingParam;
	EcrioTXNSessionTxnMappingParamStruct *pSessionTxnMappingParam;
	/*
	EcrioTXNTxnStateEnum								txnState;
	u_int16												noOfRunningTimers;
	u_int32												*pRunningTimerIds;*/
} EcrioTXNOutputParamDetailsStruct;

typedef	struct
{
	u_int16	noOfOutputParamDetails;
	EcrioTXNOutputParamDetailsStruct **ppOutputParamaDetails;
} EcrioTXNOutputParamStruct;

typedef	struct
{
	EcrioTXNTxnMatchingParamStruct *pTxnMatchingParam;
	EcrioTXNSessionTxnMappingParamStruct *pSessionTxnMappingParam;
	u_int16	noOfRunningTimers;
	u_int32	*pRunningTimerIds;
} EcrioTXNTUNotificationDataStruct;

typedef	struct
{
	void *pTxnCBData;
	void *pAppCBData;
} EcrioTXNAppCBStruct;

/*	structure declaration end	*/

/**********************************************************************************************/

/**********************************************************************************************/

/*	API declaration start	*/

void EcrioTUNotifyError
(
	EcrioTXNReturnCodeEnum txnRetCode,
	void *pData
);

EcrioTXNReturnCodeEnum EcrioTXNInit
(
	void **ppTxnData,
	EcrioTXNInitParamStruct	*pTxnInitParam
);

EcrioTXNReturnCodeEnum EcrioTXNExit
(
	void **ppTxnData
);

void EcrioTXNStructRelease
(
	void *pTxnData,
	void **ppInputStruct,
	EcrioTXNStructTypeEnum structType,
	BoolEnum bRelease
);

EcrioTXNReturnCodeEnum EcrioTXNSendRequestToClientTxnHandler
(
	void *pTxn,
	EcrioTXNInputParamStruct *pTxnInputParam,
	BoolEnum bDeleteFlag,
	BoolEnum bIPSecFlag
);

EcrioTXNReturnCodeEnum EcrioTXNSendResponseToClientTxnHandler
(
	void *pTxn,
	EcrioTXNInputParamStruct *pTxnInputParam,
	BoolEnum bDeleteFlag
);

EcrioTXNReturnCodeEnum EcrioTXNSendRequestToServerTxnHandler
(
	void *pTxn,
	EcrioTXNInputParamStruct *pTxnInputParam,
	BoolEnum bDeleteFlag
);

EcrioTXNReturnCodeEnum EcrioTXNSendResponseToServerTxnHandler
(
	void *pTxn,
	EcrioTXNInputParamStruct *pTxnInputParam,
	BoolEnum bDeleteFlag
);

EcrioTXNReturnCodeEnum EcrioTXNSetChannelIndexInTXNNode
(
	void *pTxn,
	EcrioTXNInputParamStruct *pTxnInputParam,
	u_int32 uChannelIndex
);

EcrioTXNReturnCodeEnum EcrioTXNDeleteTxnAll
(
	void *pTxn
);

EcrioTXNReturnCodeEnum EcrioTXNDeleteTxnMatchedSessionMatchedTxnIdentifier
(
	void *pTxn,
	u_char *pSessionTxnMappingParamString,
	u_char *pViaBranch,
	EcrioTXNTxnIdentificationStruct	*pTxnIdentifier
);

EcrioTXNReturnCodeEnum EcrioTXNDeleteMatchedTransactions
(
	void *pTxn,
	u_char *pCallID
);

BoolEnum EcrioTXNIsAnyTxnExistMatchedSessionMatchedTxnIdentifier
(
	void *pTxn,
	u_char *pSessionTxnMappingParamString,
	EcrioTXNTxnIdentificationStruct	*pTxnIdentifier
);

EcrioTXNReturnCodeEnum EcrioTXNGetDetailsOfAllTxnMatchedTxnIdentifier
(
	void *pTxn,
	EcrioTXNTxnIdentificationStruct	*pTxnIdentifier,
	EcrioTXNOutputParamStruct *pOutputParamStruct
);

EcrioTXNReturnCodeEnum EcrioTXNDisableRetransmissionMatchedSessionMatchedTxnIdentifier
(
	void *pTxn,
	u_char *pSessionTxnMappingParamString,
	EcrioTXNTxnIdentificationStruct	*pTxnIdentifier
);

EcrioTXNReturnCodeEnum EcrioTXNSendMessageDirectToTransport
(
	void *pTxn,
	EcrioTXNInputParamStruct *pTxnInputParam
);

EcrioTXNReturnCodeEnum EcrioTXNValidateAndStoreCustomTimerValues
(
	EcrioTXNCustomTimersStruct *pCustomSIPTimers
);

EcrioTXNReturnCodeEnum EcrioTXNChangeCustomTimerValues
(
	void *pTxn,
	EcrioTXNCustomTimersStruct *pCustomSIPTimers
);
/* Adding SAC support */
EcrioTXNReturnCodeEnum EcrioTXNSetTransportData
(
	void *pTxn,
	EcrioTxnMgrTransportStruct *pTxnTransportStruct
);
EcrioTXNReturnCodeEnum EcrioTXNResetSacData
(
	void *pTxn,
	EcrioTxnMgrTransportStruct *pTxnTransportStruct
);

void  EcrioTXNFindMatchingTxnSMSRetryData
(
	void *pTxn,
	EcrioTXNTxnMatchingParamStruct *pTxnMatchingParam,
	EcrioTXNTxnMatchingCriteriaEnum	txnMatchingCriteria,
	EcrioTXNReturnCodeEnum *pTxnRetCode,
	u_char **			pSMSRetrydata,
	u_int32*				SmsRetryCount
);


/*Add ends */
/*	API declaration end	*/

/**********************************************************************************************/

#endif	// _ECRIO_TXN_H_
