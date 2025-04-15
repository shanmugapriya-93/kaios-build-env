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

#include "EcrioSUESigMgrCallBacks.h"
#include "EcrioSUESigMgr.h"

#include "EcrioSigMgrInit.h"
#include "EcrioSigMgrRegister.h"

/******************************************************************************

                Internal Function _EcrioSUESigMgrHandle

******************************************************************************/

/**
 * This callback function is called by SigMgr to get its handle which is stored in SUE.
 * _EcrioSUEGlobalDataStruct is passed as input.
 *
 * @param[in] pCallbackData			Pointer to the SUE Handle.
 * @return                          The SigMgr handle.
 */
SIGMGRHANDLE _EcrioSUESigMgrHandle
(
	void *pCallbackData
)
{
	_EcrioSUEGlobalDataStruct *pSUEGlobalData = NULL;
	SIGMGRHANDLE pSigMgrHandle = NULL;

	if (!pCallbackData)
	{
		return NULL;
	}

	pSUEGlobalData = (_EcrioSUEGlobalDataStruct *)(pCallbackData);

	pSigMgrHandle = pSUEGlobalData->pSigMgrHandle;
	return (SIGMGRHANDLE)pSigMgrHandle;
}

/******************************************************************************

                Internal Function _EcrioSUESigMgrInfoCallback

******************************************************************************/

/**
 * This callback function is called by SigMgr to notify SUE for error,warning or information.
 * This function acts based on the event notified with detailed data, eg, re-registration timer expiry,
 * registration failed etc.
 *
 * @param[in] infoType				The type of notification, whether it is error,warning or information.
 * @param[in] infoCode				The specific code to identify the event.
 * @param[in] pData					The data corresponding to the specific code.
 * @param[in] pCallbackData			Pointer to the SUE Handle.
 * @return                          None.
 */
void _EcrioSUESigMgrInfoCallback
(
	EcrioSigMgrInfoTypeEnum infoType,
	s_int32 infoCode,
	EcrioSigMgrInfoStruct *pData,
	void *pCallbackData
)
{
	u_int32	uError = ECRIO_SUE_NO_ERROR;
	_EcrioSUEGlobalDataStruct *pSUEGlobalData = NULL;
	EcrioSUENotifyStruct *pSUENotificationData = NULL;
	_EcrioSUEInternalNetworkStateEnum eNetworkState = _ECRIO_SUE_INTERNAL_NETWORK_STATE_ENUM_None;

	// _EcrioSUEInternalRegistrationStateEnum			eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_None;

	pData = pData;

	if (!pCallbackData)
	{
		return;
	}

	pSUEGlobalData = (_EcrioSUEGlobalDataStruct *)(pCallbackData);

	SUELOGI( pSUEGlobalData->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__ );

	// eRegistrationState = pSUEGlobalData->eRegistrationState;
	eNetworkState = pSUEGlobalData->eNetworkState;

	pal_MemoryAllocate(sizeof(EcrioSUENotifyStruct), (void **)&pSUENotificationData);
	if (!pSUENotificationData)
	{
		uError = ECRIO_SUE_INSUFFICIENT_MEMORY_ERROR;
		goto EndTag;
	}


	if (infoType == EcrioSigMgrError)
	{
		switch (infoCode)
		{
			case ECRIO_SIGMGR_CRITCAL_ERROR_NEED_TO_EXIT:
			case ECRIO_SIG_MGR_REGISTRATION_FAILED:
			case ECRIO_SIGMGR_REGISTRATION_TIME_EXPIRED:
			{
				pSUENotificationData->eNotificationType = ECRIO_SUE_NOTIFICATION_ENUM_ExitSUE;
				pSUENotificationData->u.eExitReason = ECRIO_SUE_EXIT_REASON_ENUM_RegistrationFailed;

				pSUEGlobalData->eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_RegistrationFailed;
				ECRIO_SUE_LOG_REGISTRATION_STATE_CHANGE(pSUEGlobalData,
					(u_char *)"_EcrioSUESigMgrInfoCallback",
					_ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_RegistrationFailed);
			}
			break;

			case ECRIO_SIGMGR_MORE_THEN_TWO_AUTH_RECEIVED:
			{
			}
			break;

			case ECRIO_SIG_MGR_SIG_SOCKET_ERROR:
			{
				pSUENotificationData->eNotificationType = ECRIO_SUE_NOTIFICATION_ENUM_SocketError;

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
			break;

			default:
			{
			}
			break;
		}
	}
	else
	{
		/*TODO: */
	}

	if (pSUENotificationData && pSUENotificationData->eNotificationType != ECRIO_SUE_NOTIFICATION_ENUM_None)
	{
		SUELOGV( pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\t --> Notifying Shell --> pSUENotificationData = %x", __FUNCTION__, __LINE__, pSUENotificationData);

		pSUEGlobalData->callbackStruct.pNotifySUECallbackFn(pSUEGlobalData->callbackStruct.pCallbackData, (void *)pSUENotificationData);
	}

	if (infoType == EcrioSigMgrInformation && infoCode == ECRIO_SIGMGR_RE_REG_TIMER_EXPIRES)
	{
		BoolEnum *pbSendReReg = (BoolEnum *)pData;

		if (((pSUEGlobalData->eEngineState == _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Running) ||
			(pSUEGlobalData->eEngineState == _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Initialized)) &&
			(eNetworkState == _ECRIO_SUE_INTERNAL_NETWORK_STATE_ENUM_Connected) &&
			(pbSendReReg && Enum_TRUE == *pbSendReReg))
		{
			SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\t --> Re-Registration Timer "		"expired --> Sending ReRegister Request", __FUNCTION__, __LINE__ );
			uError = EcrioSUESigMgrSendRegister(pSUEGlobalData,
				EcrioSigMgrRegisterRequestType_Reg);

			if (uError != ECRIO_SUE_NO_ERROR)
			{
				if (uError == ECRIO_SUE_PAL_SOCKET_ERROR)
				{
					SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\tReceived socket error while re-registaering. Notifying socket error to upper layer.", __FUNCTION__, __LINE__);
					pSUENotificationData->eNotificationType = ECRIO_SUE_NOTIFICATION_ENUM_SocketError;
					pSUEGlobalData->callbackStruct.pNotifySUECallbackFn(pSUEGlobalData->callbackStruct.pCallbackData, (void *)pSUENotificationData);
				}
				SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\t --> Error Sending "		"ReRegister Request. error = %x.", __FUNCTION__, __LINE__, uError);
				goto EndTag;
			}

			ECRIO_SUE_LOG_REGISTRATION_STATE_CHANGE(pSUEGlobalData,
				(u_char *)"_EcrioSUESigMgrInfoCallback",
				_ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_ReRegistering);

			pSUEGlobalData->eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_ReRegistering;

			ECRIO_SUE_LOG_ALL_STATES(pSUEGlobalData, (u_char *)"_EcrioSUESigMgrInfoCallback");
		}
	}

EndTag:

	if (pSUEGlobalData->bExitSUE == Enum_TRUE)
	{
		_EcrioSUEEngineNotifySUEExit(pSUEGlobalData);
	}

	if (pSUENotificationData)
	{
		_EcrioSUEStructRelease((void **)&pSUENotificationData, ECRIO_SUE_STRUCT_ENUM_EcrioSUENotifyStruct, Enum_TRUE);
		pSUENotificationData = NULL;
	}

	SUELOGI( pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u\t Exit --> %u", __FUNCTION__, __LINE__, uError);
}

/******************************************************************************

                Internal Function _EcrioSUESigMgrStatusCallback

******************************************************************************/

/**
 * This callback function is called by SigMgr to notify SUE for received sip message, timeout timer
 * expiry and notification from SAC and SUEExtn.
 * This function acts based on the event notified with detailed data.
 *
 * @param[in] pSigMgrMessageStruct	The pointer to the data provided by SigMgr corresponding to the event.
 * @param[in] pCallbackData			Pointer to the SUE Handle.
 * @return                          None.
 */
void _EcrioSUESigMgrStatusCallback
(
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct,
	void *pCallbackData
)
{
	u_int32	uError = ECRIO_SUE_NO_ERROR;
	_EcrioSUEGlobalDataStruct *pSUEGlobalData = NULL;
	EcrioSUENotifyStruct *pSUENotificationData = NULL;

	pSUEGlobalData = (_EcrioSUEGlobalDataStruct *)(pCallbackData);

	if (!pSUEGlobalData)
	{
		return;
	}
	SUELOGI( pSUEGlobalData->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__ );

	if (!pSigMgrMessageStruct)
	{
		goto Cleanup_Tag;
	}

	switch (pSigMgrMessageStruct->msgCmd)
	{
		case EcrioSigMgrRegisterReponse:
		case EcrioSigMgrDeRegisterReponse:
		case EcrioSigMgrSubscribeResponseNotification:
		case EcrioSigMgrNotifyRequestNotification:
		{
			uError = _EcrioSUEConstructNotificationStructFromSigMgrMessageStruct
				(
				pSUEGlobalData,
				pSigMgrMessageStruct,
				&pSUENotificationData
				);
			if (uError != ECRIO_SUE_NO_ERROR)
			{
				goto Cleanup_Tag;
			}
		}
		break;

		default:
		{
		}
		break;
	}

	switch (pSigMgrMessageStruct->msgCmd)
	{
		case EcrioSigMgrRegisterReponse:
		{
			EcrioSigMgrSipMessageStruct *pRegResp = NULL;

			pRegResp = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;

			if (pRegResp)
			{
				uError = _EcrioSUEHandleRegisterResponse(pSUEGlobalData, pRegResp);
				if (uError != ECRIO_SUE_NO_ERROR)
				{
					goto Cleanup_Tag;
				}
			}
		}
		break;

		case EcrioSigMgrDeRegisterReponse:
		{
			EcrioSigMgrSipMessageStruct *pRegResp = NULL;

			pRegResp = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;

			if (pRegResp)
			{
				uError = _EcrioSUEHandleDeRegisterResponse(pSUEGlobalData, pRegResp);
				if (uError != ECRIO_SUE_NO_ERROR)
				{
					goto Cleanup_Tag;
				}
			}
		}
		break;

		case EcrioSigMgrSubscribeResponseNotification:
		{
			EcrioSigMgrSipMessageStruct *pSubResp = NULL;

			pSubResp = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;

			if (pSubResp)
			{
				uError = _EcrioSUEHandleSubscribeResponse(pSUEGlobalData, pSubResp);
				if (uError != ECRIO_SUE_NO_ERROR)
				{
					goto Cleanup_Tag;
				}
			}
		}
		break;

		case EcrioSigMgrNotifyRequestNotification:
		{
			EcrioSigMgrSipMessageStruct *pNotifyReq = NULL;

			pNotifyReq = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;

			if (pNotifyReq)
			{
				uError = _EcrioSUEHandleNotifyRequest(pSUEGlobalData, pNotifyReq);
				if (uError != ECRIO_SUE_NO_ERROR)
				{
					goto Cleanup_Tag;
				}
			}
		}
		break;

		case EcrioSigMgrIPSecSecurityServerNotification:
		{
			EcrioSipIPsecNegotiatedParamStruct *pIPSecNeg = NULL;

			pIPSecNeg = (EcrioSipIPsecNegotiatedParamStruct *)pSigMgrMessageStruct->pData;

			if (pIPSecNeg)
			{
				uError = _EcrioSUEHandleIPSecSecurityServerNotif(pSUEGlobalData, pIPSecNeg, &pSUENotificationData);
				if (uError != ECRIO_SUE_NO_ERROR)
				{
					goto Cleanup_Tag;
				}
			}
		}
		break;

		default:
			break;
	}

Cleanup_Tag:

	if (pSUENotificationData && pSUENotificationData->eNotificationType != ECRIO_SUE_NOTIFICATION_ENUM_None)
	{
		SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\t --> Notifying Shell --> pSUENotificationData = %x", __FUNCTION__, __LINE__, pSUENotificationData);

		pSUEGlobalData->callbackStruct.pNotifySUECallbackFn(pSUEGlobalData->callbackStruct.pCallbackData, (void *)pSUENotificationData);
	}

	if (pSUEGlobalData->bExitSUE == Enum_TRUE)
	{
		_EcrioSUEEngineNotifySUEExit(pSUEGlobalData);
	}

	if (pSUENotificationData)
	{
		_EcrioSUEStructRelease((void **)&pSUENotificationData, ECRIO_SUE_STRUCT_ENUM_EcrioSUENotifyStruct, Enum_TRUE);
		pSUENotificationData = NULL;
	}

	SUELOGI( pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u\t Exit --> %u", __FUNCTION__, __LINE__, uError);
}
