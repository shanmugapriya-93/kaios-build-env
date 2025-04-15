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

#include "EcrioPAL.h"

#include "EcrioSUEInternalFunctions.h"
#include "EcrioSigMgrInit.h"

/*Newly Added by Bivash -END*/

u_int32 _EcrioSUEEngineNotifySUEExit
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData
)
{
	u_int32	uError = ECRIO_SUE_NO_ERROR;
	EcrioSUENotifyStruct *pSUENotificationData = NULL;

	if (pSUEGlobalData->bExitSUE == Enum_TRUE)
	{
		pal_MemoryAllocate(sizeof(EcrioSUENotifyStruct), (void **)&pSUENotificationData);
		if (!pSUENotificationData)
		{
			uError = ECRIO_SUE_INSUFFICIENT_MEMORY_ERROR;
			goto EndTag;
		}

		pSUENotificationData->eNotificationType = ECRIO_SUE_NOTIFICATION_ENUM_ExitSUE;
		pSUENotificationData->u.eExitReason = pSUEGlobalData->eExitReason;
		pSUEGlobalData->bExitSUE = Enum_FALSE;
		pSUEGlobalData->eExitReason = ECRIO_SUE_EXIT_REASON_ENUM_None;

		/*Stop all transactions timers*/
		EcrioSigMgrDeleteAllTransaction(pSUEGlobalData->pSigMgrHandle);

		/*stop registraton retry timer*/
		if (pSUEGlobalData->bRegReAttemptTimerStarted == Enum_TRUE)
		{
			pal_TimerStop(pSUEGlobalData->uRegReAttemptTimerId);
			pSUEGlobalData->uRegReAttemptTimerId = NULL;
			pSUEGlobalData->bRegReAttemptTimerStarted = Enum_FALSE;
		}

		/*Stop the all register related timer*/
		EcrioSigMgrStopRegistrationTimer(pSUEGlobalData->pSigMgrHandle, EcrioSigMgrRegisterTimerAll, pSUEGlobalData->pRegisterIdentifier);

		/*stop subscription retry timer*/
		if (pSUEGlobalData->bSubReAttemptTimerStarted == Enum_TRUE)
		{
			pal_TimerStop(pSUEGlobalData->uSubReAttemptTimerId);
			pSUEGlobalData->uSubReAttemptTimerId = NULL;
			pSUEGlobalData->bSubReAttemptTimerStarted = Enum_FALSE;
		}
	}

	if (pSUENotificationData && pSUENotificationData->eNotificationType != ECRIO_SUE_NOTIFICATION_ENUM_None)
	{
		SUELOGV( pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\t --> Notifying Shell --> pSUENotificationData = %x", __FUNCTION__, __LINE__, pSUENotificationData);

		pSUEGlobalData->callbackStruct.pNotifySUECallbackFn(pSUEGlobalData->callbackStruct.pCallbackData, (void *)pSUENotificationData);
	}

EndTag:

	if (pSUENotificationData)
	{
		_EcrioSUEStructRelease((void **)&pSUENotificationData, ECRIO_SUE_STRUCT_ENUM_EcrioSUENotifyStruct, Enum_TRUE);
		pSUENotificationData = NULL;
	}

	return uError;
}

u_char *_EcrioSUEGetStateTypeString
(
	_EcrioSUEInternalStateTypeEnum eType
)
{
	u_char *pString = NULL;

	switch (eType)
	{
		case _ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Engine:
		{
			pString = _ECRIO_SUE_INTERNAL_STATE_TYPE_ENGINE;
		}
		break;

		case _ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Network:
		{
			pString = _ECRIO_SUE_INTERNAL_STATE_TYPE_NETWORK;
		}
		break;

		case _ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Registration:
		{
			pString = _ECRIO_SUE_INTERNAL_STATE_TYPE_REGISTRATION;
		}
		break;

		default:
		{
			pString = _ECRIO_SUE_INTERNAL_STATE_NONE;
		}
		break;
	}

	return pString;
}

u_char *_EcrioSUEGetInternalEngineStateString
(
	_EcrioSUEInternalEngineStateEnum eType
)
{
	u_char *pString = NULL;

	switch (eType)
	{
		case _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Initialized:
		{
			pString = _ECRIO_SUE_INTERNAL_ENGINE_STATE_INITIALIZED;
		}
		break;

		case _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Running:
		{
			pString = _ECRIO_SUE_INTERNAL_ENGINE_STATE_RUNNING;
		}
		break;

		case _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Deinitializing:
		{
			pString = _ECRIO_SUE_INTERNAL_ENGINE_STATE_DEINITIALIZING;
		}
		break;

		case _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Deinitialized:
		{
			pString = _ECRIO_SUE_INTERNAL_ENGINE_STATE_DEINITILIZED;
		}
		break;

		default:
		{
			pString = _ECRIO_SUE_INTERNAL_STATE_NONE;
		}
		break;
	}

	return pString;
}

u_char *_EcrioSUEGetInternalNetworkStateString
(
	_EcrioSUEInternalNetworkStateEnum eType
)
{
	u_char *pString = NULL;

	switch (eType)
	{
		case _ECRIO_SUE_INTERNAL_NETWORK_STATE_ENUM_Connected:
		{
			pString = _ECRIO_SUE_INTERNAL_NETWORK_STATE_CONNECTED;
		}
		break;

		case _ECRIO_SUE_INTERNAL_NETWORK_STATE_ENUM_Disconnected:
		{
			pString = _ECRIO_SUE_INTERNAL_NETWORK_STATE_DISCONNECTED;
		}
		break;

		default:
		{
			pString = _ECRIO_SUE_INTERNAL_STATE_NONE;
		}
		break;
	}

	return pString;
}

u_char *_EcrioSUEGetInternalRegistrationStateString
(
	_EcrioSUEInternalRegistrationStateEnum eType
)
{
	u_char *pString = NULL;

	switch (eType)
	{
		case _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_Registering:
		{
			pString = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_REGISTERING;
		}
		break;

		case _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_Registered:
		{
			pString = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_REGISTERED;
		}
		break;

		case _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_RegistrationFailed:
		{
			pString = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_REGISTRATION_FAILED;
		}
		break;

		case _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_ReRegistering:
		{
			pString = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_REREGISTERING;
		}
		break;

		case _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_DeRegistering:
		{
			pString = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_DE_REGISTERING;
		}
		break;

		case _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_DeRegistered:
		{
			pString = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_DE_REGISTERED;
		}
		break;

		case _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_DeRegistrationFailed:
		{
			pString = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_DE_REGISTRATION_FAILED;
		}
		break;

		default:
		{
			pString = _ECRIO_SUE_INTERNAL_STATE_NONE;
		}
		break;
	}

	return pString;
}

void _EcrioSUELogSigMgrMessageCmdEnum
(
	void *pLogHandle,
	EcrioSigMgrMessageCmdEnum eType
)
{
	(void)pLogHandle;

	switch (eType)
	{
		case EcrioSigMgrMessageNone:
		{
			SUELOGV( pLogHandle, KLogTypeGeneral, "%s:%u\t	__Engine__ _EcrioSigMgrMessageCmdEnum IS --> EcrioSigMgrMessageNone ", __FUNCTION__, __LINE__ );
		}
		break;

		case EcrioSigMgrRegisterRequest:
		{
			SUELOGV( pLogHandle, KLogTypeGeneral, "%s:%u\t	__Engine__ _EcrioSigMgrMessageCmdEnum IS --> EcrioSigMgrRegisterRequest ", __FUNCTION__, __LINE__ );
		}
		break;

		case EcrioSigMgrRegisterReponse:
		{
			SUELOGV( pLogHandle, KLogTypeGeneral, "%s:%u\t	__Engine__ _EcrioSigMgrMessageCmdEnum IS --> EcrioSigMgrRegisterReponse ", __FUNCTION__, __LINE__ );
		}
		break;

		case EcrioSigMgrDeRegisterRequest:
		{
			SUELOGV( pLogHandle, KLogTypeGeneral, "%s:%u\t	__Engine__ _EcrioSigMgrMessageCmdEnum IS --> EcrioSigMgrDeRegisterRequest ", __FUNCTION__, __LINE__ );
		}
		break;

		case EcrioSigMgrDeRegisterReponse:
		{
			SUELOGV( pLogHandle, KLogTypeGeneral, "%s:%u\t	__Engine__ _EcrioSigMgrMessageCmdEnum IS --> EcrioSigMgrDeRegisterReponse ", __FUNCTION__, __LINE__ );
		}
		break;

		case EcrioSigMgrNotifyRequestNotification:
		{
			SUELOGV(pLogHandle, KLogTypeGeneral, "%s:%u\t	__Engine__ _EcrioSigMgrMessageCmdEnum IS --> EcrioSigMgrNotifyRequestNotification ", __FUNCTION__, __LINE__);
		}
		break;

		default:
		{
			SUELOGV( pLogHandle, KLogTypeGeneral, "%s:%u\t	__Engine__ _EcrioSigMgrMessageCmdEnum IS --> UNKNOWN. ", __FUNCTION__, __LINE__ );
		}
		break; 
	}
}

#if defined(ANDROID) || defined(QNX) || defined(__linux__) || defined(__MACH__)

void ECRIO_SUE_LOG_INVALID_OPERATION_ERROR
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	u_char *FuncName
)
{
	SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeState, "%s:%u\t%s() --> The operation is not a valid operation in the current states",
		__FUNCTION__, __LINE__, FuncName);
}

void ECRIO_SUE_LOG_IGNORE_EVENT
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	u_char *FuncName,
	EcrioSigMgrMessageCmdEnum eType
)
{
	_EcrioSUELogSigMgrMessageCmdEnum(pSUEGlobalData->pLogHandle, eType);
	SUELOGD(pSUEGlobalData->pLogHandle, KLogTypeState, "%s:%u\t%s() --> The EVENT is skipped. Current %s state is %s Current %s state is %s Current %s state is %s",
		__FUNCTION__, __LINE__, FuncName,
		_EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Engine),
		_EcrioSUEGetInternalEngineStateString(pSUEGlobalData->eEngineState),
		_EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Network),
		_EcrioSUEGetInternalNetworkStateString(pSUEGlobalData->eNetworkState),
		_EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Registration),
		_EcrioSUEGetInternalRegistrationStateString(pSUEGlobalData->eRegistrationState));
}

void ECRIO_SUE_LOG_ALL_STATES
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	u_char *FuncName
)
{
	SUELOGD(pSUEGlobalData->pLogHandle, KLogTypeState, "%s:%u\t%s() --> Current %s state is %s Current %s state is %s Current %s state is %s",
		__FUNCTION__, __LINE__, FuncName,
		_EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Engine),
		_EcrioSUEGetInternalEngineStateString(pSUEGlobalData->eEngineState),
		_EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Network),
		_EcrioSUEGetInternalNetworkStateString(pSUEGlobalData->eNetworkState),
		_EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Registration),
		_EcrioSUEGetInternalRegistrationStateString(pSUEGlobalData->eRegistrationState));
}

void ECRIO_SUE_LOG_ENGINE_STATE_CHANGE
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	u_char *FuncName,
	_EcrioSUEInternalEngineStateEnum newState
)
{
	SUELOGD(pSUEGlobalData->pLogHandle, KLogTypeState, "%s:%u\t%s() --> %s state is changing from %s to %s",
		__FUNCTION__, __LINE__, FuncName,
		_EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Engine),
		_EcrioSUEGetInternalEngineStateString(pSUEGlobalData->eEngineState),
		_EcrioSUEGetInternalEngineStateString(newState));
}

void ECRIO_SUE_LOG_NETWORK_STATE_CHANGE
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	u_char *FuncName,
	_EcrioSUEInternalNetworkStateEnum newState
)
{
	SUELOGD(pSUEGlobalData->pLogHandle, KLogTypeState, "%s:%u\t%s() --> %s state is changing from %s to %s",
		__FUNCTION__, __LINE__, FuncName,
		_EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Network),
		_EcrioSUEGetInternalNetworkStateString(pSUEGlobalData->eNetworkState),
		_EcrioSUEGetInternalNetworkStateString(newState));
}

void ECRIO_SUE_LOG_REGISTRATION_STATE_CHANGE
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	u_char *FuncName,
	_EcrioSUEInternalRegistrationStateEnum newState
)
{
	SUELOGD(pSUEGlobalData->pLogHandle, KLogTypeState, "%s:%u\t%s() --> %s state is changing from %s to %s",
		__FUNCTION__, __LINE__, FuncName,
		_EcrioSUEGetStateTypeString(_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Registration),
		_EcrioSUEGetInternalRegistrationStateString(pSUEGlobalData->eRegistrationState),
		_EcrioSUEGetInternalRegistrationStateString(newState));
}

#endif /* defined(ANDROID) || defined(QNX)  ||defined(__linux__)*/
