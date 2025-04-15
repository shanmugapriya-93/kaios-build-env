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

#ifndef	_ECRIO_SUE_INTERNAL_FUNCTIONS_H_
#define _ECRIO_SUE_INTERNAL_FUNCTIONS_H_

#include "EcrioPAL.h"
#include "EcrioSigMgrCommon.h"
#include "EcrioSigMgrRegister.h"
#include "EcrioSigMgrSubscribe.h"

#include "EcrioSUEInternalDataTypes.h"
#include "EcrioSUESigMgr.h"

/* The build project should define ENABLE_LOG to enable logging using the Ecrio
   PAL logging mechanism. */
#ifdef ENABLE_LOG
#define SUELOGV(a, b, c, ...)		pal_LogMessage((a), KLogLevelVerbose, KLogComponentSUE | KLogComponent_Engine, (b), (c),##__VA_ARGS__);
#define SUELOGI(a, b, c, ...)		pal_LogMessage((a), KLogLevelInfo, KLogComponentSUE | KLogComponent_Engine, (b), (c),##__VA_ARGS__);
#define SUELOGD(a, b, c, ...)		pal_LogMessage((a), KLogLevelDebug, KLogComponentSUE | KLogComponent_Engine, (b), (c),##__VA_ARGS__);
#define SUELOGW(a, b, c, ...)		pal_LogMessage((a), KLogLevelWarning, KLogComponentSUE | KLogComponent_Engine, (b), (c),##__VA_ARGS__);
#define SUELOGE(a, b, c, ...)		pal_LogMessage((a), KLogLevelError, KLogComponentSUE | KLogComponent_Engine, (b), (c),##__VA_ARGS__);
#else
#define SUELOGV(a, b, c, ...)
#define SUELOGI(a, b, c, ...)
#define SUELOGD(a, b, c, ...)
#define SUELOGW(a, b, c, ...)
#define SUELOGE(a, b, c, ...)
#endif	// ENABLE_LOG

#define ECRIO_SUE_2XX_RESPONSE		2U
#define ECRIO_SUE_3XX_RESPONSE		3U
#define ECRIO_SUE_4XX_RESPONSE		4U
#define ECRIO_SUE_5XX_RESPONSE		5U
#define ECRIO_SUE_6XX_RESPONSE		6U

#if (!defined(ANDROID) && !defined(QNX) && !defined(__linux__) && !defined(__MACH__) && !defined(__APPLE__))
#define		ECRIO_SUE_LOG_INVALID_OPERATION_ERROR(pSUEGlobalData, FuncName) \
	SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\tThe operation is not a valid operation in the current states", __FUNCTION__, __LINE__ )
/*
pal_LogMessage	\
    (			\
        pSUEGlobalData->pLogHandle,KLogError,		\
        "__Engine__ "##FuncName##"() --> The operation is not a valid operation in the current states\r\n"	\
    )
*/
#define		ECRIO_SUE_LOG_IGNORE_EVENT(pSUEGlobalData, FuncName, eType) \
	_EcrioSUELogSigMgrMessageCmdEnum(pSUEGlobalData->pLogHandle, eType); \
	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\tThe EVENT is skipped \
			Current %s state is %s Current %s state is %s Current %s state is %s", __FUNCTION__, __LINE__, \
	_EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Engine), \
	_EcrioSUEGetInternalEngineStateString(pSUEGlobalData->eEngineState), \
	_EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Network), \
	_EcrioSUEGetInternalNetworkStateString(pSUEGlobalData->eNetworkState), \
	_EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Registration), \
	_EcrioSUEGetInternalRegistrationStateString(pSUEGlobalData->eRegistrationState) \
	)
/*
pal_LogMessage	\
    (			\
        pSUEGlobalData->pLogHandle,KLogInformation,		\
        "__Engine__ "##FuncName##"() --> The EVENT is skipped \
        Current %s state is %s Current %s state is %s Current %s state is %s\r\n",	\
        _EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Engine),	\
        _EcrioSUEGetInternalEngineStateString(pSUEGlobalData->eEngineState),		\
        _EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Network),	\
        _EcrioSUEGetInternalNetworkStateString(pSUEGlobalData->eNetworkState),		\
        _EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Registration),	\
        _EcrioSUEGetInternalRegistrationStateString(pSUEGlobalData->eRegistrationState)	\
    )
*/

#define		ECRIO_SUE_LOG_ALL_STATES(pSUEGlobalData, FuncName) \
	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeState, "%s:%u\tCurrent %s state is %s Current %s state is %s Current %s state is %s", \
	__FUNCTION__, __LINE__, \
	_EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Engine), \
	_EcrioSUEGetInternalEngineStateString(pSUEGlobalData->eEngineState), \
	_EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Network), \
	_EcrioSUEGetInternalNetworkStateString(pSUEGlobalData->eNetworkState), \
	_EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Registration), \
	_EcrioSUEGetInternalRegistrationStateString(pSUEGlobalData->eRegistrationState) \
	)
/*
pal_LogMessage	\
    (			\
        pSUEGlobalData->pLogHandle,KLogInformation,		\
        "__Engine__ "##FuncName##"() --> Current %s state is %s Current %s state is %s Current %s state is %s\r\n",	\
        _EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Engine),	\
        _EcrioSUEGetInternalEngineStateString(pSUEGlobalData->eEngineState),		\
        _EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Network),	\
        _EcrioSUEGetInternalNetworkStateString(pSUEGlobalData->eNetworkState),		\
        _EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Registration),	\
        _EcrioSUEGetInternalRegistrationStateString(pSUEGlobalData->eRegistrationState)	\
    )
*/
#define		ECRIO_SUE_LOG_ENGINE_STATE_CHANGE(pSUEGlobalData, FuncName, newState) \
	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeState, "%s:%u\t%s state is changing from %s to %s", \
	__FUNCTION__, __LINE__, \
	_EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Engine), \
	_EcrioSUEGetInternalEngineStateString(pSUEGlobalData->eEngineState), \
	_EcrioSUEGetInternalEngineStateString(newState) \
	)
/*
pal_LogMessage	\
    (			\
        pSUEGlobalData->pLogHandle,KLogInformation,		\
        "__Engine__ "##FuncName##"() --> %s state is changing from %s to %s\r\n",	\
        _EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Engine),	\
        _EcrioSUEGetInternalEngineStateString(pSUEGlobalData->eEngineState),		\
        _EcrioSUEGetInternalEngineStateString(newState)		\
    )
*/
#define		ECRIO_SUE_LOG_NETWORK_STATE_CHANGE(pSUEGlobalData, FuncName, newState) \
	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeState, "%s:%u\t%s state is changing from %s to %s", \
	__FUNCTION__, __LINE__, \
	_EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Network), \
	_EcrioSUEGetInternalNetworkStateString(pSUEGlobalData->eNetworkState), \
	_EcrioSUEGetInternalNetworkStateString(newState) \
	)
/*
pal_LogMessage	\
    (			\
        pSUEGlobalData->pLogHandle,KLogInformation,		\
        "__Engine__ "##FuncName##"() --> %s state is changing from %s to %s\r\n",	\
        _EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Network),	\
        _EcrioSUEGetInternalNetworkStateString(pSUEGlobalData->eNetworkState),		\
        _EcrioSUEGetInternalNetworkStateString(newState)		\
    )
*/
#define		ECRIO_SUE_LOG_REGISTRATION_STATE_CHANGE(pSUEGlobalData, FuncName, newState) \
	SUELOGD(pSUEGlobalData->pLogHandle, KLogTypeState, "%s:%u\t%s state is changing from %s to %s", \
	__FUNCTION__, __LINE__, \
	_EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Registration), \
	_EcrioSUEGetInternalRegistrationStateString(pSUEGlobalData->eRegistrationState), \
	_EcrioSUEGetInternalRegistrationStateString(newState) \
	)
/*
pal_LogMessage	\
    (			\
        pSUEGlobalData->pLogHandle,KLogInformation,		\
        "__Engine__ "##FuncName##"() --> %s state is changing from %s to %s\r\n",	\
        _EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Registration),	\
        _EcrioSUEGetInternalRegistrationStateString(pSUEGlobalData->eRegistrationState),		\
        _EcrioSUEGetInternalRegistrationStateString(newState)		\
    )
*/
#else /*!defined(ANDROID) && !defined(QNX) && !defined(__linux__) && !defined(__MACH__)  && !defined(__APPLE__)*/
void ECRIO_SUE_LOG_INVALID_OPERATION_ERROR
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	u_char *FuncName
);

void ECRIO_SUE_LOG_IGNORE_EVENT
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	u_char *FuncName,
	EcrioSigMgrMessageCmdEnum eType
);

void ECRIO_SUE_LOG_ALL_STATES
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	u_char *FuncName
);

void ECRIO_SUE_LOG_ENGINE_STATE_CHANGE
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	u_char *FuncName,
	_EcrioSUEInternalEngineStateEnum newState
);

void ECRIO_SUE_LOG_NETWORK_STATE_CHANGE
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	u_char *FuncName,
	_EcrioSUEInternalNetworkStateEnum newState
);

void ECRIO_SUE_LOG_REGISTRATION_STATE_CHANGE
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	u_char *FuncName,
	_EcrioSUEInternalRegistrationStateEnum newState
);
#endif /*!defined(ANDROID) && !defined(QNX) && !defined(__linux__) && !defined(__MACH__) && !defined(__APPLE__) */

/******************************************************************************

                Functions declarations start

******************************************************************************/

/*---------------- Main SUE operations start --------------------------------*/

u_int32 _EcrioSUEExit
(
	_EcrioSUEGlobalDataStruct **ppSUEGlobalData
);

u_int32 _EcrioSUEStart
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData
);

u_int32 _EcrioSUEStop
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData
);

/*----------------- Main SUE operations end ----------------------------------*/

/*----------------- Structure operations start -------------------------------*/

u_int32 _EcrioSUEStructRelease
(
	void **ppData,
	u_int32	uDataType,
	BoolEnum bReleaseParent
);

/*----------------- Structure operations end  -------------------------------*/

/* ---------------- SUE utility functions start ------------------------------*/

u_int32 _EcrioSUEEngineNotifySUEExit
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData
);

u_int32 _EcrioSUESendInitialRegister
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	BoolEnum bNextPCSCF
);

/** \brief This function is used to detach the LTE Network
 *
 * @param[in] uTimerId			- Timer ID value
 * @param[in] pTimerCBData		- Timer CB Data.
 */
void _EcrioSUERegReAttemptTimer
(
	void *pCallbackData,
	TIMERHANDLE uTimerId
);

void _EcrioSUESubscribeReAttemptTimer
(
	void *pCallbackData,
	TIMERHANDLE uTimerId
);

/* To handle incoming REGISTER response */
u_int32 _EcrioSUEHandleRegisterResponse
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	EcrioSigMgrSipMessageStruct *pRegResp
);

u_int32 _EcrioSUEHandleDeRegisterResponse
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	EcrioSigMgrSipMessageStruct *pRegResp
);

u_int32 _EcrioSUEHandleRegRetryEvent
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	EcrioOOMRetryValuesStruct *pRetryStruct
);

u_int32 _EcrioSUEHandleSubscriptionRetryEvent
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	EcrioOOMRetryValuesStruct *pRetryStruct
);

u_int32 _EcrioSUEConstructNotificationStructFromSigMgrMessageStruct
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct,
	EcrioSUENotifyStruct **ppSUENotificationData
);

u_char *_EcrioSUEGetStateTypeString
(
	_EcrioSUEInternalStateTypeEnum eType
);

u_char *_EcrioSUEGetInternalEngineStateString
(
	_EcrioSUEInternalEngineStateEnum eType
);

u_char *_EcrioSUEGetInternalNetworkStateString
(
	_EcrioSUEInternalNetworkStateEnum eType
);

u_char *_EcrioSUEGetInternalRegistrationStateString
(
	_EcrioSUEInternalRegistrationStateEnum eType
);

void _EcrioSUELogSigMgrMessageCmdEnum
(
	void *pLogHandle,
	EcrioSigMgrMessageCmdEnum eType
);

void _EcrioSUERetryReRegistrationTimerCallback
(
	u_int32 timerId,
	void *pTimerCBData
);

u_int32 _EcrioSUESigMgrSendSubscribe
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	u_int32 uExpiry
);

u_int32 _EcrioSUEHandleSubscribeResponse
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	EcrioSigMgrSipMessageStruct *pSubResp
);

u_int32 _EcrioSUEHandleNotifyRequest
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	EcrioSigMgrSipMessageStruct *pNotifyReq
);

u_int32	_EcrioSUEParseRegEventXML
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	EcrioSigMgrSipMessageStruct *pNotifyReq,
	EcrioSUENotifyNotifyRequestStruct **ppNotifyRequest
);

u_int32 _EcrioSUESubscribe
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData
);

u_int32 _EcrioSUEHandleIPSecSecurityServerNotif
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	EcrioSipIPsecNegotiatedParamStruct *pIPSecNeg,
	EcrioSUENotifyStruct **ppSUENotificationData
);

/* ---------------- SUE utility functions end  -------------------------------*/

/******************************************************************************

                Functions declarations end

******************************************************************************/

#endif /*  _ECRIO_SUE_INTERNAL_FUNCTIONS_H_    */
