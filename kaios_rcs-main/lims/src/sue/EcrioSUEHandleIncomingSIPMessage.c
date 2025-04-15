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

#include "EcrioSUESigMgr.h"
#include "EcrioSUESigMgrCallBacks.h"

#include "EcrioSigMgrInternal.h"
#include "EcrioSigMgrRegister.h"
#include "EcrioSigMgrUtilities.h"

#include "yxml.h"

#define ECRIO_SUE_SCOPE_ID_SYMBOL    "%"

#define ALWAYS_TRUE(anything)    ((##anything) == (##anything))

/* Support for NAT - Start */
// static u_int32 _EcrioSUEEngineCreateNoOpPacket( _EcrioSUEGlobalDataStruct* pSUEGlobalData, u_char** ppNoOpPacket );
/* Support for NAT - End */

/*********************************************************************************

  Function:		_EcrioSUEHandleRegisterResponse()

  Purpose:		This API will be used by the SUE Engine to handle
                the registration response

  Description:	For negative final response it increament the failure count and if failure count
                reaches to its limit then for 404 response it sends an initial registration with
                IMSI public id if already tried with MSISDN public id, otherwise detach from LTE
                network for a specified time interval.For 403 and other failure response it detach
                from LTE network. For failure responses it delete the temporary IPsec.
                For 401 response it initiates IPsec establishment if IPsec is enabled otherwise
                sends the REGISTER with credential.
                For 200 response it transits the temporary IPsec to established state. It sets the
                current registration state as registered and notifies it to all deployed engines and
                also it sends SUBSCRIBE.

  Input:		_EcrioSUEGlobalDataStruct *pSUEGlobalData -		Handle to the
                                                                SUE Global Data

                EcrioSigMgrSipMessageStruct *pRegResp -	Pointer which
                                                                contains the
                                                                register response
                                                                details

  Returns:		One of the pre-defined Error Codes.
**********************************************************************************/
u_int32 _EcrioSUEHandleRegisterResponse
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	EcrioSigMgrSipMessageStruct *pRegResp
)
{
	u_int32	uError = ECRIO_SUE_NO_ERROR;

	if (!pRegResp || !pSUEGlobalData)
	{
		uError = ECRIO_SUE_INSUFFICIENT_DATA_ERROR;
		goto EndTag;
	}

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	SUELOGD( pSUEGlobalData->pLogHandle, KLogTypeState,
		"%s:%u\tRegistrationState = %d, RegResponseCode=%d",
		__FUNCTION__, __LINE__, pSUEGlobalData->eRegistrationState, pRegResp->responseCode);

	/*Ignore response while SUE is not running or deinitializing*/
	if (pSUEGlobalData->eEngineState != _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Running &&
		pSUEGlobalData->eEngineState != _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Deinitializing)
	{
		ECRIO_SUE_LOG_IGNORE_EVENT(pSUEGlobalData, (u_char *)"_EcrioSUEHandleRegisterResponse", EcrioSigMgrRegisterReponse);
		goto EndTag;
	}

	/*Ignore response when no network*/
	if (pSUEGlobalData->eNetworkState == _ECRIO_SUE_INTERNAL_NETWORK_STATE_ENUM_None)
	{
		ECRIO_SUE_LOG_IGNORE_EVENT(pSUEGlobalData, (u_char *)"_EcrioSUEHandleRegisterResponse", EcrioSigMgrRegisterReponse);
		goto EndTag;
	}

	/*Ignore unwanted response*/
	if (pSUEGlobalData->eRegistrationState != _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_Registering &&
		pSUEGlobalData->eRegistrationState != _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_ReRegistering)
	{
		ECRIO_SUE_LOG_IGNORE_EVENT(pSUEGlobalData, (u_char *)"_EcrioSUEHandleRegisterResponse", EcrioSigMgrRegisterReponse);
		goto EndTag;
	}

	switch (pRegResp->responseCode / 100)
	{
		case ECRIO_SUE_2XX_RESPONSE:
		{
			/* Reset state variable for register error response handler */
			pSUEGlobalData->uRegGeneralFailureCount = 0;

			ECRIO_SUE_LOG_REGISTRATION_STATE_CHANGE(pSUEGlobalData, (u_char *)"_EcrioSUEHandleRegisterResponse", _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_Registered);
			pSUEGlobalData->eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_Registered;
			ECRIO_SUE_LOG_ALL_STATES(pSUEGlobalData, (u_char *)"_EcrioSUEHandleRegisterResponse");


			_EcrioSigMgrStartUdpKeepAlive(pSUEGlobalData->pSigMgrHandle);

			/*Send SUBSCRIBE for reg event*/
			if (pSUEGlobalData->bSubscribeRegEvent == Enum_TRUE)
			{
				if (pSUEGlobalData->pSubscribeSessionId == NULL)
				{
					/*Send initial subscribe request*/
					SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\tSending SUBSCRIBE request for reg event.", __FUNCTION__, __LINE__);
					uError = _EcrioSUESubscribe(pSUEGlobalData);
					if (uError != ECRIO_SUE_NO_ERROR)
					{
						SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
							"%s:%u\tSubscription to reg event failed, error = %u", __FUNCTION__, __LINE__, uError);
						goto EndTag;
					}
				}
				else
				{
					/*Send re-subscribe request if resubscribe timer was expired in sigmgr but there was no network.*/
					EcrioSigMgrSubscribeState eSubState = EcrioSigMgrSubscribeStateNone;
					EcrioSigMgrGetSubscribeState(pSUEGlobalData->pSigMgrHandle, pSUEGlobalData->pSubscribeSessionId, &eSubState);

					if (eSubState == EcrioSigMgrSubscribeStateReSubcribeTimerExpired)
					{
						SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\tSending SUBSCRIBE request for reg event.", __FUNCTION__, __LINE__);
						uError = _EcrioSUESubscribe(pSUEGlobalData);
						if (uError != ECRIO_SUE_NO_ERROR)
						{
							SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
								"%s:%u\tSubscription to reg event failed, error = %u", __FUNCTION__, __LINE__, uError);
							goto EndTag;
						}
					}
				}
			}
		}
		break;

		case ECRIO_SUE_4XX_RESPONSE:
		case ECRIO_SUE_5XX_RESPONSE:
		case ECRIO_SUE_6XX_RESPONSE:
		default:
		{
			EcrioOOMRetryQueryStruct retryQueryStruct = { 0 };
			EcrioOOMRetryValuesStruct regRetryValueStruct = { 0 };
			EcrioSigMgrHeaderStruct *pRetryAfterHeader = NULL;
			_EcrioSUEInternalRegistrationStateEnum eRegState = pSUEGlobalData->eRegistrationState;

			SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\tError REGISTER reponse received.", __FUNCTION__, __LINE__);

			if (eRegState == _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_ReRegistering)
			{
				ECRIO_SUE_LOG_REGISTRATION_STATE_CHANGE(pSUEGlobalData, (u_char *)"_EcrioSUEHandleRegisterResponse", _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_Registered);
				pSUEGlobalData->eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_Registered;
			}
			else
			{
				ECRIO_SUE_LOG_REGISTRATION_STATE_CHANGE(pSUEGlobalData, (u_char *)"_EcrioSUEHandleRegisterResponse", _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_RegistrationFailed);
				pSUEGlobalData->eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_RegistrationFailed;
			}

			ECRIO_SUE_LOG_ALL_STATES(pSUEGlobalData, (u_char *)"_EcrioSUEHandleRegisterResponse");

			/* Increment the Throttling Counter */
			pSUEGlobalData->uRegGeneralFailureCount++;
			/*Populate OOM query structure*/
			if (eRegState == _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_ReRegistering)
			{
				retryQueryStruct.bIsInitialRequest = Enum_FALSE;
			}
			else
			{
				retryQueryStruct.bIsInitialRequest = Enum_TRUE;
			}

			retryQueryStruct.uRetryCount = pSUEGlobalData->uRegGeneralFailureCount;
			retryQueryStruct.uResponseCode = (u_int16)pRegResp->responseCode;

			/*get action from OOM*/
			if (pSUEGlobalData->pOOMObject != NULL)
			{
				pSUEGlobalData->pOOMObject->ec_oom_GetRegRetryValues(retryQueryStruct, &regRetryValueStruct);

				/*Check for retry after header. If found then over write the interval*/
				uError = EcrioSigMgrGetOptionalHeader(pSUEGlobalData->pSigMgrHandle, pRegResp->pOptionalHeaderList,
					EcrioSipHeaderTypeRetryAfter, &pRetryAfterHeader);
				if (uError == ECRIO_SUE_NO_ERROR && pRetryAfterHeader != NULL)
				{
					if (pRetryAfterHeader->ppHeaderValues && pRetryAfterHeader->ppHeaderValues[0]->pHeaderValue)
					{
						regRetryValueStruct.uRetryInterval = (u_int16)pal_StringConvertToUNum(pRetryAfterHeader->ppHeaderValues[0]->pHeaderValue, NULL, 10);
					}
				}
			}
			else
			{
				SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
					"%s:%u\tRegister Re-attempt not defined in OOM", __FUNCTION__, __LINE__);

				pSUEGlobalData->bExitSUE = Enum_TRUE;
				pSUEGlobalData->eExitReason = ECRIO_SUE_EXIT_REASON_ENUM_RegistrationFailed;
				goto EndTag;
			}

			uError = _EcrioSUEHandleRegRetryEvent(pSUEGlobalData, &regRetryValueStruct);
			if (uError != ECRIO_SUE_NO_ERROR)
			{
				SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSUEHandleRegRetryEvent failed, error = %u", __FUNCTION__, __LINE__, uError);
				goto EndTag;
			}
		}
		break;
	}

EndTag:

	if (NULL != pSUEGlobalData)
	{
		SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u\tuError=%u", __FUNCTION__, __LINE__, uError);
	}

	return uError;
}

/*
This function handles deregistartion responses.
For 200 response it delete all IPsecand sets the current registration
state as registered. If there is any pending register it sends the REGISTER.
It notifies SACALC to exit.
For 401 response it initiates IPsec establishment if IPsec is enabled otherwise
sends the REGISTER with credential.
*/
u_int32 _EcrioSUEHandleDeRegisterResponse
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	EcrioSigMgrSipMessageStruct *pRegResp
)
{
	u_int32	uError = ECRIO_SUE_NO_ERROR;
	/*BoolEnum										bUseStoredSIPParam = Enum_FALSE;
	EcrioSigMgrRegisterRequestEnum					eRegReqType = EcrioSigMgrRegisterRequestType_Reg;
	_EcrioSUEInternalRegistrationStateEnum			eInternalRegState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_None;*/
	_EcrioSUEInternalNetworkStateEnum eNetworkState = _ECRIO_SUE_INTERNAL_NETWORK_STATE_ENUM_None;
	_EcrioSUEInternalRegistrationStateEnum eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_None;

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (!pRegResp)
	{
		uError = ECRIO_SUE_INSUFFICIENT_DATA_ERROR;
		goto EndTag;
	}

	eNetworkState = pSUEGlobalData->eNetworkState;

	eRegistrationState = pSUEGlobalData->eRegistrationState;

	if (pRegResp->responseCode >= _ECRIO_SIP_RESPONSE_CODE_200)
	{
		if ((pSUEGlobalData->eEngineState != _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Running &&
			pSUEGlobalData->eEngineState != _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Deinitializing)
			||
			eNetworkState == _ECRIO_SUE_INTERNAL_NETWORK_STATE_ENUM_None
			||
			(eRegistrationState != _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_DeRegistering)
			)
		{
			ECRIO_SUE_LOG_IGNORE_EVENT(pSUEGlobalData, (u_char *)"_EcrioSUEHandleDeRegisterResponse", EcrioSigMgrDeRegisterReponse);
			goto EndTag;
		}
	}
	else
	{
		ECRIO_SUE_LOG_IGNORE_EVENT(pSUEGlobalData, (u_char *)"_EcrioSUEHandleDeRegisterResponse", EcrioSigMgrDeRegisterReponse);
		goto EndTag;
	}

	if ((pRegResp->responseCode >= _ECRIO_SIP_RESPONSE_CODE_200 && pRegResp->responseCode < _ECRIO_SIP_RESPONSE_CODE_300) ||
		(pRegResp->responseCode == _ECRIO_SIP_RESPONSE_CODE_501) || (pRegResp->responseCode == _ECRIO_SIP_RESPONSE_CODE_481))
	{
		ECRIO_SUE_LOG_REGISTRATION_STATE_CHANGE(pSUEGlobalData, (u_char *)"_EcrioSUEHandleDeRegisterResponse", _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_DeRegistered);

		pSUEGlobalData->eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_DeRegistered;

		_EcrioSigMgrStopUdpKeepAlive(pSUEGlobalData->pSigMgrHandle);

		ECRIO_SUE_LOG_ALL_STATES(pSUEGlobalData, (u_char *)"_EcrioSUEHandleDeRegisterResponse");

		if (pSUEGlobalData->eEngineState == _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Deinitializing)
		{
			pSUEGlobalData->bExitSUE = Enum_TRUE;
			pSUEGlobalData->eExitReason = ECRIO_SUE_EXIT_REASON_ENUM_Deregistered;
		}

		/*Temporary disabled*/
	}
	else
	{
		ECRIO_SUE_LOG_REGISTRATION_STATE_CHANGE(pSUEGlobalData, (u_char *)"_EcrioSUEHandleDeRegisterResponse", _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_DeRegistrationFailed);

		pSUEGlobalData->eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_DeRegistrationFailed;

		ECRIO_SUE_LOG_ALL_STATES(pSUEGlobalData, (u_char *)"_EcrioSUEHandleDeRegisterResponse");

		pSUEGlobalData->bExitSUE = Enum_TRUE;
		pSUEGlobalData->eExitReason = ECRIO_SUE_EXIT_REASON_ENUM_Deregistered;
	}

EndTag:
	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return uError;
}

/*Handle the reg retry actions from OOM*/
u_int32 _EcrioSUEHandleRegRetryEvent
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	EcrioOOMRetryValuesStruct *pRetryStruct
)
{
	u_int32 uError = ECRIO_SUE_NO_ERROR;
	TimerStartConfigStruct config = { 0 };

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	switch (pRetryStruct->eRetryType)
	{
		case EcrioOOMRetry_InitialRequest_SamePCSCF:
		{
			if (pRetryStruct->uRetryInterval == 0)
			{
				SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
					"%s:%u\tSend initial register to same PCSCF", __FUNCTION__, __LINE__);

				uError = _EcrioSUESendInitialRegister(pSUEGlobalData, Enum_FALSE);
				if (uError != ECRIO_SUE_NO_ERROR)
				{
					SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSUESendInitialRegisterSamePCSCF() uError=%u", __FUNCTION__, __LINE__, uError);
				}
			}
			else
			{
				pSUEGlobalData->eReAttempt = _ECRIO_SUE_INTERNAL_REATTEMPT_SAME_PCSCF;
			}
		}
		break;

		case EcrioOOMRetry_InitialRequest_NextPCSCF:
		{
			SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
				"%s:%u\tSend initial register to next PCSCF", __FUNCTION__, __LINE__);

			if (pRetryStruct->uRetryInterval == 0)
			{
				/*We are in socket receive thread. Same socket should not be terminated.
				So the socket will be terminated in timer thread*/
				pRetryStruct->uRetryInterval = 1;
			}

			pSUEGlobalData->eReAttempt = _ECRIO_SUE_INTERNAL_REATTEMPT_NEXT_PCSCF;
		}
		break;

		case EcrioOOMRetry_ReRequest_SamePCSCF:
		{
			if (pRetryStruct->uRetryInterval == 0)
			{
				SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
					"%s:%u\tSend reregister", __FUNCTION__, __LINE__);

				uError = EcrioSUESigMgrSendRegister(pSUEGlobalData,
					EcrioSigMgrRegisterRequestType_Reg);
				if (uError != ECRIO_SUE_NO_ERROR)
				{
					SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
						"%s:%u\tEcrioSUESigMgrSendRegister() uError=%u", __FUNCTION__, __LINE__, uError);
				}

				ECRIO_SUE_LOG_REGISTRATION_STATE_CHANGE(pSUEGlobalData, (u_char *)"_EcrioSUEHandleRegRetryEvent", _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_Registering);
				pSUEGlobalData->eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_Registering;
			}
			else
			{
				pSUEGlobalData->eReAttempt = _ECRIO_SUE_INTERNAL_REATTEMPT_REREQUEST;
			}
		}
		break;

		default:
		{
			SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
				"%s:%u\tReattempt in next power cycle", __FUNCTION__, __LINE__);
			pSUEGlobalData->bExitSUE = Enum_TRUE;
			pSUEGlobalData->eExitReason = ECRIO_SUE_EXIT_REASON_ENUM_RegistrationFailed;
		}
		break;
	}

	if (pRetryStruct->uRetryInterval > 0)
	{
		config.uInitialInterval = pRetryStruct->uRetryInterval * 1000;
		config.uPeriodicInterval = 0;
		config.timerCallback = _EcrioSUERegReAttemptTimer;
		config.pCallbackData = (void *)pSUEGlobalData;
		config.bEnableGlobalMutex = Enum_TRUE;

		uError = pal_TimerStart(pSUEGlobalData->pal, &config, &pSUEGlobalData->uRegReAttemptTimerId);
		if (uError == KPALErrorNone)
		{
			pSUEGlobalData->bRegReAttemptTimerStarted = Enum_TRUE;
			SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
				"%s:%u\tRegister Re attempt timer started (Throttling Timer) for interval value of %u secs.",
				__FUNCTION__, __LINE__, pRetryStruct->uRetryInterval);

			/*Stop the all register related timer*/
			EcrioSigMgrStopRegistrationTimer(pSUEGlobalData->pSigMgrHandle, EcrioSigMgrRegisterTimerAll, pSUEGlobalData->pRegisterIdentifier);

		}
		else
		{
			SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_TimerStart() Register Re attempt timer failed for interval value of %u secs, uError=%u",
				__FUNCTION__, __LINE__, pRetryStruct->uRetryInterval, uError);
			uError = ECRIO_SUE_PLATMAN_ERROR;
			pSUEGlobalData->bExitSUE = Enum_TRUE;
			pSUEGlobalData->eExitReason = ECRIO_SUE_EXIT_REASON_ENUM_RegistrationFailed;
		}
	}

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return uError;
}

/*Handle the subscribe retry actions from OOM*/
u_int32 _EcrioSUEHandleSubscribeRetryEvent
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	EcrioOOMRetryValuesStruct *pRetryStruct
)
{
	u_int32 uError = ECRIO_SUE_NO_ERROR;
	TimerStartConfigStruct config = { 0 };

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	switch (pRetryStruct->eRetryType)
	{
		case EcrioOOMRetry_InitialRequest_SamePCSCF:
		{
			if (pSUEGlobalData->pSubscribeSessionId)
			{
				EcrioSigMgrAbortSession(pSUEGlobalData->pSigMgrHandle, pSUEGlobalData->pSubscribeSessionId);
				//pal_MemoryFree((void **)&(pSUEGlobalData->pSubscribeSessionId));
				pSUEGlobalData->pSubscribeSessionId = NULL;
			}

			if (pRetryStruct->uRetryInterval == 0)
			{
				SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
					"%s:%u\tSend initial Subscribe", __FUNCTION__, __LINE__);

				uError = _EcrioSUESubscribe(pSUEGlobalData);
				if (uError != ECRIO_SUE_NO_ERROR)
				{
					SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSUESendInitialSubscribeSamePCSCF() uError=%u", __FUNCTION__, __LINE__, uError);
				}
			}
			else
			{
				pSUEGlobalData->eReAttempt = _ECRIO_SUE_INTERNAL_REATTEMPT_SAME_PCSCF;
			}
		}
		break;

		case EcrioOOMRetry_ReRequest_SamePCSCF:
		{
			if (pRetryStruct->uRetryInterval == 0)
			{
				SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
					"%s:%u\tSend resubscribe", __FUNCTION__, __LINE__);

				uError = _EcrioSUESubscribe(pSUEGlobalData);
				if (uError != ECRIO_SUE_NO_ERROR)
				{
					SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
						"%s:%u\tEcrioSUESendSubscribe() uError=%u", __FUNCTION__, __LINE__, uError);
				}
			}
			else
			{
				pSUEGlobalData->eReAttempt = _ECRIO_SUE_INTERNAL_REATTEMPT_REREQUEST;
			}
		}
		break;

		default:
		{
			SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
				"%s:%u\tReattempt in next power cycle", __FUNCTION__, __LINE__);
			pSUEGlobalData->bExitSUE = Enum_TRUE;
			pSUEGlobalData->eExitReason = ECRIO_SUE_EXIT_REASON_ENUM_SubscriptionFailed;
		}
		break;
	}

	if (pRetryStruct->uRetryInterval > 0)
	{
		config.uInitialInterval = pRetryStruct->uRetryInterval * 1000;
		config.uPeriodicInterval = 0;
		config.timerCallback = _EcrioSUESubscribeReAttemptTimer;
		config.pCallbackData = (void *)pSUEGlobalData;
		config.bEnableGlobalMutex = Enum_TRUE;

		uError = pal_TimerStart(pSUEGlobalData->pal, &config, &pSUEGlobalData->uSubReAttemptTimerId);
		if (uError == KPALErrorNone)
		{
			pSUEGlobalData->bSubReAttemptTimerStarted = Enum_TRUE;
			SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
				"%s:%u\tSubscribe Re attempt timer started (Throttling Timer) for interval value of %u secs.",
				__FUNCTION__, __LINE__, pRetryStruct->uRetryInterval);
		}
		else
		{
			SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_TimerStart() Subscribe Re attempt timer failed for interval value of %u secs, uError=%u",
				__FUNCTION__, __LINE__, pRetryStruct->uRetryInterval, uError);
			uError = ECRIO_SUE_PLATMAN_ERROR;
		}
	}

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return uError;
}

/*
This function handles SUBSCRIBE responses.
*/

/*
This function converts SigMgr's notification data for various signaling event to EcrioSUENotifyStruct
to notify SACALC. This function is called to for incoming sip messages and on timeout timer expiry.
*/
u_int32 _EcrioSUEConstructNotificationStructFromSigMgrMessageStruct
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct,
	EcrioSUENotifyStruct **ppSUENotificationData
)
{
	u_int32	uError = ECRIO_SUE_NO_ERROR;
	EcrioSUENotifyStruct *pSUENotificationData = NULL;
	_EcrioSUEInternalNetworkStateEnum eNetworkState = _ECRIO_SUE_INTERNAL_NETWORK_STATE_ENUM_None;
	_EcrioSUEInternalRegistrationStateEnum eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_None;

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__ );

	if (!pSigMgrMessageStruct || !ppSUENotificationData)
	{
		uError = ECRIO_SUE_INSUFFICIENT_DATA_ERROR;
		goto EndTag;
	}

	eNetworkState = pSUEGlobalData->eNetworkState;
	eRegistrationState = pSUEGlobalData->eRegistrationState;

	pal_MemoryAllocate(sizeof(EcrioSUENotifyStruct), (void **)&pSUENotificationData);
	if (!pSUENotificationData)
	{
		uError = ECRIO_SUE_INSUFFICIENT_MEMORY_ERROR;
		goto EndTag;
	}

	SUELOGV( pSUEGlobalData->pLogHandle, KLogTypeGeneral,
		"%s:%u\tpSigMgrMessageStruct->msgCmd=%u",
		__FUNCTION__, __LINE__, pSigMgrMessageStruct->msgCmd);

	_EcrioSUELogSigMgrMessageCmdEnum
	(
		pSUEGlobalData->pLogHandle,
		pSigMgrMessageStruct->msgCmd
	);

	switch (pSigMgrMessageStruct->msgCmd)
	{
		case EcrioSigMgrRegisterReponse:
		case EcrioSigMgrDeRegisterReponse:
		{
			EcrioSUENotifyRegisterResponseStruct *pRegisterResponse = NULL;
			EcrioSUENotifyRegisterResponseStruct *pDeregisterResponse = NULL;

			if (pSigMgrMessageStruct->msgCmd == EcrioSigMgrRegisterReponse)
			{
				EcrioSigMgrSipMessageStruct *pRegResp = NULL;

				pRegResp = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;

				if (pRegResp->responseCode >= _ECRIO_SIP_RESPONSE_CODE_200)
				{
					if ((pSUEGlobalData->eEngineState != _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Running &&
						pSUEGlobalData->eEngineState != _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Deinitializing)
						||
						eNetworkState == _ECRIO_SUE_INTERNAL_NETWORK_STATE_ENUM_None
						/*||
						(eRegistrationState != _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_Registering) */
						)
					{
						ECRIO_SUE_LOG_IGNORE_EVENT(pSUEGlobalData, (u_char *)"_EcrioSUEConstructNotificationStructFromSigMgrMessageStruct", EcrioSigMgrRegisterReponse);
						goto EndTag;
					}
				}
				else
				{
					ECRIO_SUE_LOG_IGNORE_EVENT(pSUEGlobalData, (u_char *)"_EcrioSUEConstructNotificationStructFromSigMgrMessageStruct", EcrioSigMgrRegisterReponse);
					goto EndTag;
				}

				// if (pRegResp->responseCode == 200)
				{
					pSUENotificationData->eNotificationType = ECRIO_SUE_NOTIFICATION_ENUM_RegisterResponse;
					pal_MemoryAllocate(sizeof(EcrioSUENotifyRegisterResponseStruct), (void **)&pRegisterResponse);
					if (!pRegisterResponse)
					{
						uError = ECRIO_SUE_INSUFFICIENT_MEMORY_ERROR;
						goto EndTag;
					}

					pRegisterResponse->uRspCode = pRegResp->responseCode;

					pSUENotificationData->u.pRegisterResponse = pRegisterResponse;
					pRegisterResponse = NULL;
				}
			}
			else if (pSigMgrMessageStruct->msgCmd == EcrioSigMgrDeRegisterReponse)
			{
				EcrioSigMgrSipMessageStruct *pRegResp = NULL;

				pRegResp = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;

				if (pRegResp->responseCode >= _ECRIO_SIP_RESPONSE_CODE_200)
				{
					if ((pSUEGlobalData->eEngineState != _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Running &&
						pSUEGlobalData->eEngineState != _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Deinitializing)
						||
						eNetworkState == _ECRIO_SUE_INTERNAL_NETWORK_STATE_ENUM_None
						||
						(eRegistrationState != _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_DeRegistering)
						)
					{
						ECRIO_SUE_LOG_IGNORE_EVENT(pSUEGlobalData, (u_char *)"_EcrioSUEConstructNotificationStructFromSigMgrMessageStruct", EcrioSigMgrDeRegisterReponse);
						goto EndTag;
					}
				}
				else
				{
					ECRIO_SUE_LOG_IGNORE_EVENT(pSUEGlobalData, (u_char *)"_EcrioSUEConstructNotificationStructFromSigMgrMessageStruct", EcrioSigMgrDeRegisterReponse);
					goto EndTag;
				}

				pSUENotificationData->eNotificationType = ECRIO_SUE_NOTIFICATION_ENUM_DeregisterResponse;
				pal_MemoryAllocate(sizeof(EcrioSUENotifyRegisterResponseStruct), (void **)&pDeregisterResponse);
				if (!pDeregisterResponse)
				{
					uError = ECRIO_SUE_INSUFFICIENT_MEMORY_ERROR;
					goto EndTag;
				}

				pDeregisterResponse->uRspCode = pRegResp->responseCode;

				pSUENotificationData->u.pDeregisterResponse = pDeregisterResponse;
				pDeregisterResponse = NULL;
			}
		}
		break;

		case EcrioSigMgrSubscribeResponseNotification:
		{
			EcrioSUENotifySubscribeResponseStruct *pSubscribeResponse = NULL;

			EcrioSigMgrSipMessageStruct *pSubResp = NULL;

			pSubResp = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;
			pSUENotificationData->eNotificationType = ECRIO_SUE_NOTIFICATION_ENUM_SubscribeResponse;
			pal_MemoryAllocate(sizeof(EcrioSUENotifySubscribeResponseStruct), (void **)&pSubscribeResponse);
			if (!pSubscribeResponse)
			{
				uError = ECRIO_SUE_INSUFFICIENT_MEMORY_ERROR;
				goto EndTag;
			}

			pSubscribeResponse->uRspCode = pSubResp->responseCode;
			if (pSubResp->pExpires != NULL)
			{
				pSubscribeResponse->uExpire = *pSubResp->pExpires;
			}

			pSUENotificationData->u.pSubscribeResponse = pSubscribeResponse;
			pSubscribeResponse = NULL;
		}
		break;

		case EcrioSigMgrNotifyRequestNotification:
		{
			EcrioSUENotifyNotifyRequestStruct *pNotifyRequest = NULL;

			EcrioSigMgrSipMessageStruct *pNotifyReq = NULL;

			pNotifyReq = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;

			if (pNotifyReq->pMessageBody)
			{
				uError = _EcrioSUEParseRegEventXML(pSUEGlobalData, pNotifyReq, &pNotifyRequest);
				if (pNotifyRequest == NULL)
				{
					SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\t _EcrioSUEParseRegEventXML() failed with error = %u", __FUNCTION__, __LINE__, uError);
					goto EndTag;
				}

				pSUENotificationData->eNotificationType = ECRIO_SUE_NOTIFICATION_ENUM_NotifyRequest;

				pSUENotificationData->u.pNotifyRequest = pNotifyRequest;
				pNotifyRequest = NULL;
			}
		}
		break;

		default:
		{
		}
	}

	*ppSUENotificationData = pSUENotificationData;
	pSUENotificationData = NULL;

EndTag:

	if (pSUENotificationData)
	{
		_EcrioSUEStructRelease((void **)&pSUENotificationData, ECRIO_SUE_STRUCT_ENUM_EcrioSUENotifyStruct, Enum_TRUE);
		pSUENotificationData = NULL;
	}

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__ );
	return uError;
}

/*Send initial register.. send it to next PCSCF if bNextPCSCF == Enum_TRUE else send it to same PCSCF*/
u_int32 _EcrioSUESendInitialRegister
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	BoolEnum bNextPCSCF
)
{

	u_int32 uError = ECRIO_SUE_NO_ERROR;
	EcrioSigMgrNetworkInfoStruct networkInfo = { 0 };

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (bNextPCSCF == Enum_TRUE)
	{
		/*Get the next PCSCF index*/
		pSUEGlobalData->pTransportAddressStruct->curPCSCFIndex++;

		/*if the IMS application used the last P-CSCF IP address provided by the network in the previous attempt,
		then the IMS application shall use the first P-CSCF IP address provided by the network for this attempt*/
		if (pSUEGlobalData->pTransportAddressStruct->curPCSCFIndex >= pSUEGlobalData->pTransportAddressStruct->uNoPCSCF)
		{
			pSUEGlobalData->pTransportAddressStruct->curPCSCFIndex = 0;
		}

		networkInfo.pLocalIp = pSUEGlobalData->pTransportAddressStruct->pLocalIP;
		networkInfo.pRemoteIP = pSUEGlobalData->pTransportAddressStruct->ppPCSCFList[pSUEGlobalData->pTransportAddressStruct->curPCSCFIndex];
		networkInfo.uLocalPort = pSUEGlobalData->pTransportAddressStruct->uLocalPort;
		networkInfo.uRemotePort = pSUEGlobalData->pTransportAddressStruct->uPCSCFPort;
		networkInfo.uRemoteTLSPort = pSUEGlobalData->pTransportAddressStruct->uPCSCFTLSPort;
		networkInfo.uRemoteClientPort = pSUEGlobalData->pTransportAddressStruct->uPCSCFClientPort;
		networkInfo.bIsIPv6 = pSUEGlobalData->pTransportAddressStruct->bIsIPv6;

		SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\tModify channels for new PCSCF %s", __FUNCTION__, __LINE__, networkInfo.pRemoteIP);

		uError = EcrioSigMgrSetNetworkParam(pSUEGlobalData->pSigMgrHandle, EcrioSigMgrNetworkState_LTEConnected, &networkInfo);
		if (uError != ECRIO_SIGMGR_NO_ERROR)
		{
			SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSigMgrSetNetworkParam() failed with error = %u",
				__FUNCTION__, __LINE__, uError);

			uError = ECRIO_SUE_UA_ENGINE_ERROR;
			goto Error_None;
		}
	}

	ECRIO_SUE_LOG_REGISTRATION_STATE_CHANGE(pSUEGlobalData, (u_char *)"_EcrioSUESendInitialRegisterNextPCSCF", _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_None);
	pSUEGlobalData->eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_None;

	SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\tSending intial register to next pcscf",
		__FUNCTION__, __LINE__);

	uError = EcrioSUESigMgrSendRegister(pSUEGlobalData,
		EcrioSigMgrRegisterRequestType_Reg);
	if (uError != ECRIO_SUE_NO_ERROR)
	{
		SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
			"%s:%u\tEcrioSUESigMgrSendRegister() uError=%u", __FUNCTION__, __LINE__, uError);
		goto Error_None;
	}

	ECRIO_SUE_LOG_REGISTRATION_STATE_CHANGE(pSUEGlobalData, (u_char *)"_EcrioSUESendInitialRegisterNextPCSCF", _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_Registering);
	pSUEGlobalData->eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_Registering;

	/*After successfully completing any new IMS registration 
	(as opposed to a re-registration), the device shall always request a new subscription to the reg events package*/

	if (pSUEGlobalData->pSubscribeSessionId)
	{
		EcrioSigMgrAbortSession(pSUEGlobalData->pSigMgrHandle, pSUEGlobalData->pSubscribeSessionId);
		//pal_MemoryFree((void **)&(pSUEGlobalData->pSubscribeSessionId));
		pSUEGlobalData->pSubscribeSessionId = NULL;
	}

Error_None:

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return uError;
}

/* Start - To Support LTE Requirement */
/* Ref Doc: LTE SMS DEVICE REQUIREMENTS, Version 6.0
    Section: 5.1.1.3 IMS REGISTRATION ERRORS */
/****************************************************************************

  Function:		_EcrioSUERegReAttemptTimer()

  Description:	This timer callback will modify remote transport based on the next p-cscf address
                for 408,503 REFISTER response, otherwise it sends an initial registration.

  Input:		TIMERHANDLE hTimer	- Timer Handle
                u_int32 uTimerId	- Timer ID value
                void* pTimerCBData	- Timer CB Data

  Returns:		One of the pre-defined Error Codes.
*****************************************************************************/
void _EcrioSUERegReAttemptTimer
(
	void *pCallbackData,
	TIMERHANDLE uTimerId
)
{
	u_int32 uError = ECRIO_SUE_NO_ERROR;
	_EcrioSUEGlobalDataStruct *pSUEGlobalData = NULL;

	/* Stop the timer before proceeding further. */
	pal_TimerStop(uTimerId);

	/* Validate the Timer Call Back data */
	if (pCallbackData == NULL)
	{
		return;
	}

	/* Assign Timer call back data to SUE Interface structure pointer */
	pSUEGlobalData = (_EcrioSUEGlobalDataStruct *)pCallbackData;

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	/* Validate the Timer ID value */
	if (pSUEGlobalData->uRegReAttemptTimerId != uTimerId)
	{
		goto Error_None;
	}

	SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\tResetting \"bRegReAttemptTimerStarted\" flag to False",
		__FUNCTION__, __LINE__);

	/* Set the bRegReAttemptTimerStarted flag to false */
	pSUEGlobalData->bRegReAttemptTimerStarted = Enum_FALSE;

	switch (pSUEGlobalData->eReAttempt)
	{
		case _ECRIO_SUE_INTERNAL_REATTEMPT_NEXT_PCSCF:
		{
			uError = _EcrioSUESendInitialRegister(pSUEGlobalData, Enum_TRUE);
		}
		break;

		case _ECRIO_SUE_INTERNAL_REATTEMPT_SAME_PCSCF:
		{
			uError = _EcrioSUESendInitialRegister(pSUEGlobalData, Enum_FALSE);
		}
		break;

		case _ECRIO_SUE_INTERNAL_REATTEMPT_REREQUEST:
		{
			SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
				"%s:%u\tSend reregister", __FUNCTION__, __LINE__);

			uError = EcrioSUESigMgrSendRegister(pSUEGlobalData,
				EcrioSigMgrRegisterRequestType_Reg);
			if (uError != ECRIO_SUE_NO_ERROR)
			{
				SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
					"%s:%u\tEcrioSUESigMgrSendRegister() uError=%u", __FUNCTION__, __LINE__, uError);
			}

			ECRIO_SUE_LOG_REGISTRATION_STATE_CHANGE(pSUEGlobalData, (u_char *)"_EcrioSUEHandleRegRetryEvent", _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_ReRegistering);
			pSUEGlobalData->eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_ReRegistering;
		}
		break;

		default:
		{
			pSUEGlobalData->bExitSUE = Enum_TRUE;
			pSUEGlobalData->eExitReason = ECRIO_SUE_EXIT_REASON_ENUM_RegistrationFailed;
			_EcrioSUEEngineNotifySUEExit(pSUEGlobalData);
		}
		break;
	}

	/*Reset eReAttempt*/
	pSUEGlobalData->eReAttempt = _ECRIO_SUE_INTERNAL_REATTEMPT_None;

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

Error_None:

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}

/****************************************************************************

Function:		_EcrioSUESubscribeReAttemptTimer()

Description:	This timer callback will modify remote transport.

Input:		TIMERHANDLE hTimer	- Timer Handle
u_int32 uTimerId	- Timer ID value
void* pTimerCBData	- Timer CB Data

Returns:		One of the pre-defined Error Codes.
*****************************************************************************/
void _EcrioSUESubscribeReAttemptTimer
(
	void *pCallbackData,
	TIMERHANDLE uTimerId
)
{
	u_int32 uError = ECRIO_SUE_NO_ERROR;
	_EcrioSUEGlobalDataStruct *pSUEGlobalData = NULL;

	/* Stop the timer before proceeding further. */
	pal_TimerStop(uTimerId);

	/* Validate the Timer Call Back data */
	if (pCallbackData == NULL)
	{
		return;
	}

	/* Assign Timer call back data to SUE Interface structure pointer */
	pSUEGlobalData = (_EcrioSUEGlobalDataStruct *)pCallbackData;

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	/* Validate the Timer ID value */
	if (pSUEGlobalData->uSubReAttemptTimerId != uTimerId)
	{
		goto Error_None;
	}

	SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\tResetting \"bSubscribeReAttemptTimerStarted\" flag to False",
		__FUNCTION__, __LINE__);

	/* Set the bSubReAttemptTimerStarted flag to false */
	pSUEGlobalData->bSubReAttemptTimerStarted = Enum_FALSE;

	switch (pSUEGlobalData->eReAttempt)
	{
		case _ECRIO_SUE_INTERNAL_REATTEMPT_SAME_PCSCF:
		{
			/*Release old subscription to fresh subscription*/
			if (pSUEGlobalData->pSubscribeSessionId != NULL)
			{
				EcrioSigMgrAbortSession(pSUEGlobalData->pSigMgrHandle, pSUEGlobalData->pSubscribeSessionId);
				//pal_MemoryFree((void **)&(pSUEGlobalData->pSubscribeSessionId));
				pSUEGlobalData->pSubscribeSessionId = NULL;
			}

			uError = _EcrioSUESubscribe(pSUEGlobalData);
			if (uError != ECRIO_SUE_NO_ERROR)
			{
				SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
					"%s:%u\tEcrioSUESendSubscribe() uError=%u", __FUNCTION__, __LINE__, uError);
			}
		}
		break;

		case _ECRIO_SUE_INTERNAL_REATTEMPT_REREQUEST:
		{
			SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
				"%s:%u\tSend resubscribe", __FUNCTION__, __LINE__);

			uError = _EcrioSUESubscribe(pSUEGlobalData);
			if (uError != ECRIO_SUE_NO_ERROR)
			{
				SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
					"%s:%u\tEcrioSUESigMgrSendSubscribe() uError=%u", __FUNCTION__, __LINE__, uError);
			}
		}
		break;

		default:
		{
			pSUEGlobalData->bExitSUE = Enum_TRUE;
			pSUEGlobalData->eExitReason = ECRIO_SUE_EXIT_REASON_ENUM_SubscriptionFailed;
			_EcrioSUEEngineNotifySUEExit(pSUEGlobalData);
		}
		break;
	}

	/*Reset eReAttempt*/
	pSUEGlobalData->eReAttempt = _ECRIO_SUE_INTERNAL_REATTEMPT_None;

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

Error_None:

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}

u_int32 _EcrioSUEHandleSubscribeResponse
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	EcrioSigMgrSipMessageStruct *pSubResp
)
{
	u_int32 uError = ECRIO_SUE_NO_ERROR;

	if (NULL == pSUEGlobalData)
	{
		return ECRIO_SUE_INVALID_INPUT_ERROR;
	}

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	switch (pSubResp->responseCode / 100)
	{
		case ECRIO_SUE_2XX_RESPONSE:
		{
			pSUEGlobalData->uSubGeneralFailureCount = 0;
			break;
		}

		case ECRIO_SUE_4XX_RESPONSE:
		case ECRIO_SUE_5XX_RESPONSE:
		case ECRIO_SUE_6XX_RESPONSE:
		default:
		{
			EcrioOOMRetryQueryStruct retryQueryStruct = { 0 };
			EcrioOOMRetryValuesStruct subRetryValueStruct = { 0 };
			EcrioSigMgrHeaderStruct *pRetryAfterHeader = NULL;
			EcrioSigMgrSubscribeState eSubState = EcrioSigMgrSubscribeStateNone;

			/* check for Re subscribe response  */
			EcrioSigMgrGetSubscribeState(pSUEGlobalData->pSigMgrHandle, pSubResp->pMandatoryHdrs->pCallId, &eSubState);

			SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\tError Subscribe reponse received.", __FUNCTION__, __LINE__);

			/* Increment the Throttling Counter */
			pSUEGlobalData->uSubGeneralFailureCount++;
			/*Populate OOM query structure*/
			retryQueryStruct.bIsInitialRequest = (eSubState == EcrioSigMgrSubscribeStateReSubcribeProgress) ? Enum_TRUE : Enum_FALSE;

			retryQueryStruct.uRetryCount = pSUEGlobalData->uSubGeneralFailureCount;
			retryQueryStruct.uResponseCode = (u_int16)pSubResp->responseCode;

			/*get action from OOM*/
			if (pSUEGlobalData->pOOMObject != NULL)
			{
				pSUEGlobalData->pOOMObject->ec_oom_GetSubRetryValues(retryQueryStruct, &subRetryValueStruct);

				/*Check for retry after header. If found then over write the interval*/
				uError = EcrioSigMgrGetOptionalHeader(pSUEGlobalData->pSigMgrHandle, pSubResp->pOptionalHeaderList,
					EcrioSipHeaderTypeRetryAfter, &pRetryAfterHeader);
				if (uError == ECRIO_SUE_NO_ERROR && pRetryAfterHeader != NULL)
				{
					if (pRetryAfterHeader->ppHeaderValues && pRetryAfterHeader->ppHeaderValues[0]->pHeaderValue)
					{
						subRetryValueStruct.uRetryInterval = (u_int16)pal_StringConvertToUNum(pRetryAfterHeader->ppHeaderValues[0]->pHeaderValue, NULL, 10);
					}
				}
			}
			else
			{
				SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
					"%s:%u\tSubscribe Re-attempt not defined in OOM", __FUNCTION__, __LINE__);

				pSUEGlobalData->bExitSUE = Enum_TRUE;
				pSUEGlobalData->eExitReason = ECRIO_SUE_EXIT_REASON_ENUM_SubscriptionFailed;
				goto EndTag;
			}

			uError = _EcrioSUEHandleSubscribeRetryEvent(pSUEGlobalData, &subRetryValueStruct);
			if (uError != ECRIO_SUE_NO_ERROR)
			{
				SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSUEHandleSubscribeRetryEvent failed, error = %u", __FUNCTION__, __LINE__, uError);
				goto EndTag;
			}

			break;
		}
	}

EndTag:

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u\tuError=%u", __FUNCTION__, __LINE__, uError);

	return uError;
}

u_int32 _EcrioSUEHandleNotifyRequest
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	EcrioSigMgrSipMessageStruct *pNotifyReq
)
{
	u_int32 uError = ECRIO_SUE_NO_ERROR;
	EcrioSigMgrHeaderStruct	*pSubscriptionState = NULL;
	BoolEnum bSubTerminated = Enum_FALSE;

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	EcrioSigMgrGetOptionalHeader(pSUEGlobalData->pSigMgrHandle, pNotifyReq->pOptionalHeaderList, EcrioSipHeaderTypeSubscriptionState, &pSubscriptionState);
	if (pSubscriptionState)
	{
		if (pSubscriptionState->ppHeaderValues && pSubscriptionState->ppHeaderValues[0] && pSubscriptionState->ppHeaderValues[0]->pHeaderValue)
		{
			if (pal_StringCompare(pSubscriptionState->ppHeaderValues[0]->pHeaderValue, (u_char *)ECRIO_SIG_MGR_SUBSCRPTION_STATE_TERMINATED) == 0)
			{
				bSubTerminated = Enum_TRUE;
			}
		}
	}

	uError = EcrioSigMgrSendNotifyResponse(pSUEGlobalData->pSigMgrHandle, pNotifyReq, ECRIO_SIGMGR_RESPONSE_OK, ECRIO_SIGMGR_RESPONSE_CODE_OK);
	if (uError != ECRIO_SIGMGR_NO_ERROR)
	{
		SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeFuncEntry, "%s:%u\tEcrioSigMgrSendNotifyResponse() failed with error = %u",
			__FUNCTION__, __LINE__, uError);
		uError = ECRIO_SUE_UA_ENGINE_ERROR;
		goto EndTag;
	}

	if (bSubTerminated == Enum_TRUE)
	{
		pSUEGlobalData->pSubscribeSessionId = NULL;
	}

EndTag:

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return uError;
}

u_int32	_EcrioSUEParseRegEventXML
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	EcrioSigMgrSipMessageStruct *pNotifyReq,
	EcrioSUENotifyNotifyRequestStruct **ppNotifyRequest
)
{
	enum
	{
		element_none			= 0,
		element_registration	= 1,
		element_contact			= 2,
		element_uri				= 4,
		element_unknown_param	= 8
	} prev_elements = element_none, curr_element = element_none;

	enum
	{
		attr_none		= 0,
		attr_aor		= 1,
		attr_state		= 2,
		attr_event		= 4,
		attr_expires	= 8,
		attr_retryafter = 16,
		attr_name		= 32
	} curr_attribute = attr_none;

	u_int32	uError = ECRIO_SUE_NO_ERROR;
	void *pBuffer = NULL;
	yxml_ret_t yxmlret;
	yxml_t x[1];
	u_int32 index = 0;
	u_char *pAttrval = NULL, *pContent = NULL, *pTemp = NULL;
	u_char size_buf[128];

	EcrioSUENotifyNotifyRequestStruct *pNotifyRequest = NULL;
	EcrioSigMgrMessageBodyStruct *pMessageBody = NULL;
	EcrioSigMgrUnknownMessageBodyStruct *pUnknownMsgBody = NULL;
	EcrioSigMgrNameAddrWithParamsStruct *pToHeader = NULL;
	u_char *pXMLBody = NULL;
	u_char *pSipInsVal = NULL;
	BoolEnum bAorFound = Enum_FALSE;

	uError = EcrioSigMgrGetDefaultPUID(pSUEGlobalData->pSigMgrHandle, EcrioSigMgrURISchemeSIP, &pToHeader);
	if (uError != ECRIO_SIGMGR_NO_ERROR)
	{
		SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\t EcrioSigMgrGetDefaultPUID() failed with error= %u", __FUNCTION__, __LINE__, uError);
		uError = ECRIO_SUE_UA_ENGINE_ERROR;
		goto EndTag;
	}

	pMessageBody = pNotifyReq->pMessageBody;
	if (pMessageBody->messageBodyType != EcrioSigMgrMessageBodyUnknown)
	{
		SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\t incorrect message body type",
			__FUNCTION__, __LINE__);
		uError = ECRIO_SUE_INCONSISTENT_DATA_ERROR;
		goto EndTag;
	}

	pUnknownMsgBody = (EcrioSigMgrUnknownMessageBodyStruct *)pMessageBody->pMessageBody;
	if (pUnknownMsgBody == NULL || pUnknownMsgBody->pBuffer == NULL)
	{
		SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\t message body is NULL",
			__FUNCTION__, __LINE__);
		uError = ECRIO_SUE_INSUFFICIENT_DATA_ERROR;
		goto EndTag;
	}

	pXMLBody = pUnknownMsgBody->pBuffer;

	pal_MemoryAllocate(2 * 1024, &pBuffer);
	if (pBuffer == NULL)
	{
		uError = ECRIO_SUE_INSUFFICIENT_MEMORY_ERROR;
		goto EndTag;
	}

	yxml_init(x, pBuffer, 2 * 1024);

	for (index = 0; index < pUnknownMsgBody->bufferLength; index++)
	{
		yxmlret = yxml_parse(x, (int)pXMLBody[index]);

		switch (yxmlret)
		{
			case YXML_OK:
			{
			}
			break;

			case YXML_ELEMSTART:
			{
				// elemstart
				// x->elem
				if (pal_StringCompare((u_char *)x->elem, _ECRIO_SUE_REGISTRATION_ELEMENT) == 0)
				{
					prev_elements = prev_elements | element_registration;
					curr_element = element_registration;
				}
				else if (pal_StringCompare((u_char *)x->elem, _ECRIO_SUE_CONTACT_ELEMENT) == 0)
				{
					prev_elements = prev_elements | element_contact;
					curr_element = element_contact;
				}
				else if (pal_StringCompare((u_char *)x->elem, _ECRIO_SUE_URI_ELEMENT) == 0)
				{
					prev_elements = prev_elements | element_uri;
					curr_element = element_uri;
					pContent = size_buf;
				}
				else if (pal_StringCompare((u_char *)x->elem, _ECRIO_SUE_UNKNOWN_ELEMENT) == 0)
				{
					prev_elements = prev_elements | element_unknown_param;
					curr_element = element_unknown_param;
				}

				// if (yxmlret & YXML_CONTENT) content
			}
			break;

			case YXML_ELEMEND:
			{
				// elemend
				if (pContent != NULL)
				{
					/*As pContent pointer is incremented on YXML_CONTENT,
					so get the actual content from size_buf.*/
					pContent = size_buf;

					if ((prev_elements & element_registration) == element_registration &&
						(prev_elements & element_contact) == element_contact)
					{
						if (curr_element == element_unknown_param)
						{
							/*sip.instance matching*/
							if (pSipInsVal != NULL)
							{
								if (pal_StringCompare(pContent, pSipInsVal) != 0)
								{
									SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\tsip.instance is not matched.",
										__FUNCTION__, __LINE__);
									uError = ECRIO_SUE_NO_ERROR;
									goto EndTag;
								}
							}
						}
					}

					pContent = NULL;
				}

				prev_elements = prev_elements ^ curr_element;
				curr_element = curr_element >> 1;
				if (curr_element == element_registration)
				{
					bAorFound = Enum_FALSE;
				}
			}
			break;

			case YXML_ATTRSTART:
			{
				// attrstart
				// x->attr
				if (pal_StringCompare((u_char *)x->attr, _ECRIO_SUE_AOR_ATTRIBUTE) == 0)
				{
					if (curr_element == element_registration)
					{
						curr_attribute = attr_aor;
					}
				}

				if (bAorFound == Enum_TRUE)
				{
					if (curr_element == element_contact)
					{
						if (pal_StringCompare((u_char *)x->attr, _ECRIO_SUE_STATE_ATTRIBUTE) == 0)
						{
							if ((prev_elements & element_registration) == element_registration)
							{
								curr_attribute = attr_state;
							}
						}
						else if (pal_StringCompare((u_char *)x->attr, _ECRIO_SUE_EVENT_ATTRIBUTE) == 0)
						{
							if ((prev_elements & element_registration) == element_registration)
							{
								curr_attribute = attr_event;
							}
						}
						else if (pal_StringCompare((u_char *)x->attr, _ECRIO_SUE_EXPIRES_ATTRIBUTE) == 0)
						{
							if ((prev_elements & element_registration) == element_registration)
							{
								curr_attribute = attr_expires;
							}
						}
						else if (pal_StringCompare((u_char *)x->attr, _ECRIO_SUE_RETRY_AFTER_ATTRIBUTE) == 0)
						{
							if ((prev_elements & element_registration) == element_registration)
							{
								curr_attribute = attr_retryafter;
							}
						}
					}

					if (curr_element == element_unknown_param)
					{
						if ((prev_elements & element_contact) == element_contact)
						{
							curr_attribute = attr_name;
						}
					}
				}

				if (curr_attribute != attr_none)
				{
					pAttrval = size_buf;
				}
			}
			break;

			case YXML_ATTREND:
			{
				// attrend
				if (pAttrval)
				{
					/*As pAttrval pointer is incremented on YXML_ATTRVAL,
					so get the actual attribute from size_buf.*/
					pAttrval = size_buf;

					switch (curr_attribute)
					{
						case attr_aor:
						{
							pAttrval = pal_SubString(pAttrval, (u_char *)":");
							if (pAttrval != NULL)
							{
								pAttrval++;
								if (!pal_StringNCompare(pAttrval, pToHeader->nameAddr.addrSpec.u.pSipUri->pUserId,
									pal_StringLength(pToHeader->nameAddr.addrSpec.u.pSipUri->pUserId)))
								{
									bAorFound = Enum_TRUE;
									if (pNotifyRequest == NULL)
									{
										pal_MemoryAllocate(sizeof(EcrioSUENotifyNotifyRequestStruct), (void **)&pNotifyRequest);
									}

									if (!pNotifyRequest)
									{
										uError = ECRIO_SUE_INSUFFICIENT_MEMORY_ERROR;
										goto EndTag;
									}
								}
							}
						}
						break;

						case attr_state:
						{
							if (pal_StringCompare(pAttrval, _ECRIO_SUE_ATTRIBUTE_VALUE_ACTIVE) == 0)
							{
								pNotifyRequest->eRegInfoContactState = ECRIO_SUE_REGINFO_CONTACT_STATE_ENUM_Active;
							}
							else if (pal_StringCompare(pAttrval, _ECRIO_SUE_ATTRIBUTE_VALUE_TERMINATED) == 0)
							{
								pNotifyRequest->eRegInfoContactState = ECRIO_SUE_REGINFO_CONTACT_STATE_ENUM_Terminated;
							}
						}
						break;

						case attr_event:
						{
							if (pal_StringCompare(pAttrval, _ECRIO_SUE_ATTRIBUTE_VALUE_REGISTERED) == 0)
							{
								pNotifyRequest->eRegInfoContactEvent = ECRIO_SUE_REGINFO_CONTACT_EVENT_ENUM_Registered;
							}
							else if (pal_StringCompare(pAttrval, _ECRIO_SUE_ATTRIBUTE_VALUE_CREATED) == 0)
							{
								pNotifyRequest->eRegInfoContactEvent = ECRIO_SUE_REGINFO_CONTACT_EVENT_ENUM_Created;
							}
							else if (pal_StringCompare(pAttrval, _ECRIO_SUE_ATTRIBUTE_VALUE_REFRESHED) == 0)
							{
								pNotifyRequest->eRegInfoContactEvent = ECRIO_SUE_REGINFO_CONTACT_EVENT_ENUM_Refreshed;
							}
							else if (pal_StringCompare(pAttrval, _ECRIO_SUE_ATTRIBUTE_VALUE_SHORTENED) == 0)
							{
								pNotifyRequest->eRegInfoContactEvent = ECRIO_SUE_REGINFO_CONTACT_EVENT_ENUM_Shortened;
							}
							else if (pal_StringCompare(pAttrval, _ECRIO_SUE_ATTRIBUTE_VALUE_EXPIRED) == 0)
							{
								pNotifyRequest->eRegInfoContactEvent = ECRIO_SUE_REGINFO_CONTACT_EVENT_ENUM_Expired;
							}
							else if (pal_StringCompare(pAttrval, _ECRIO_SUE_ATTRIBUTE_VALUE_DEACTIVATED) == 0)
							{
								pNotifyRequest->eRegInfoContactEvent = ECRIO_SUE_REGINFO_CONTACT_EVENT_ENUM_Deactivated;
							}
							else if (pal_StringCompare(pAttrval, _ECRIO_SUE_ATTRIBUTE_VALUE_PROBATION) == 0)
							{
								pNotifyRequest->eRegInfoContactEvent = ECRIO_SUE_REGINFO_CONTACT_EVENT_ENUM_Probation;
							}
							else if (pal_StringCompare(pAttrval, _ECRIO_SUE_ATTRIBUTE_VALUE_UNREGISTERED) == 0)
							{
								pNotifyRequest->eRegInfoContactEvent = ECRIO_SUE_REGINFO_CONTACT_EVENT_ENUM_Unregistered;
							}
							else if (pal_StringCompare(pAttrval, _ECRIO_SUE_ATTRIBUTE_VALUE_REJECTED) == 0)
							{
								pNotifyRequest->eRegInfoContactEvent = ECRIO_SUE_REGINFO_CONTACT_EVENT_ENUM_Rejected;
							}
						}
						break;

						case attr_expires:
						{
							pNotifyRequest->uExpires = pal_StringConvertToUNum(pAttrval, NULL, 10);
						}
						break;

						case attr_retryafter:
						{
							pNotifyRequest->uRetryAfter = pal_StringConvertToUNum(pAttrval, NULL, 10);
						}
						break;

						case attr_name:
						{
							if (pal_StringCompare(pAttrval, (u_char *)"+sip.instance") == 0)
							{
								EcrioSigMgrGetFeatureTag(pSUEGlobalData->pSigMgrHandle, (u_char *)"+sip.instance", &pSipInsVal);
								if (pSipInsVal != NULL)
								{
									pContent = size_buf;
								}
							}
						}
						break;

						default:
						{
						}
						break;
					}

					pAttrval = NULL;
				}

				// reset the curr_attribute
				curr_attribute = attr_none;
			}
			break;

			case YXML_CONTENT:
			{
				// content
				pTemp = (u_char*)x->data;
				if (bAorFound == Enum_TRUE)
				{
					if (pContent != NULL)
					{
						while (*pTemp && pContent < size_buf + sizeof(size_buf))
						{
							*(pContent++) = *(pTemp++);
						}

						if (pContent == size_buf + sizeof(size_buf))
						{
							// terminate the loop
							SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
								"%s:%u\tXML Parsing failed, too long content value, memeory error.", __FUNCTION__, __LINE__);
							index = pUnknownMsgBody->bufferLength;
						}
						else
						{
							*pContent = '\0';
						}
					}
				}
			}
			break;

			case YXML_PICONTENT:
			{
				// picontent
			}
			break;

			case YXML_ATTRVAL:
			{
				// attrval
				// x->data
				if ((pAttrval != NULL) &&
					(prev_elements & element_registration) == element_registration)
				{
					pTemp = (u_char*)x->data;

					while (*pTemp && pAttrval < size_buf + sizeof(size_buf))
					{
						*(pAttrval++) = *(pTemp++);
					}

					if (pAttrval == size_buf + sizeof(size_buf))
					{
						// terminate the loop
						// terminate the loop
						SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
							"%s:%u\tXML Parsing failed, too long attribute value, memeory error.", __FUNCTION__, __LINE__);
						index = pUnknownMsgBody->bufferLength;
					}
					else
					{
						*pAttrval = '\0';
					}
				}
			}
			break;

			case YXML_PISTART:
			{
				// pistart
				// x->pi
			}
			break;

			case YXML_PIEND:
			{
				// piend
			}
			break;

			default:
				// terminate the loop
				index = pUnknownMsgBody->bufferLength;
		}
	}

	if (yxml_eof(x) < 0)
	{
		SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\t XML Parsing failed.",
			__FUNCTION__, __LINE__);
		uError = ECRIO_SUE_XML_PARSER_ERROR;
		goto EndTag;
	}

	/*If sip.instance does not match then return NULL*/
	*ppNotifyRequest = pNotifyRequest;
	pNotifyRequest = NULL;

EndTag:

	if (pBuffer)
	{
		pal_MemoryFree((void**)&pBuffer);
	}

	if (pToHeader)
	{
		EcrioSigMgrStructRelease(pSUEGlobalData->pSigMgrHandle, EcrioSigMgrStructType_NameAddrWithParams, (void **)&pToHeader, Enum_TRUE);
	}

	if (pNotifyRequest)
	{
		pal_MemoryFree((void**)&pNotifyRequest);
	}

	return uError;
}


u_int32 _EcrioSUEHandleIPSecSecurityServerNotif
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData,
	EcrioSipIPsecNegotiatedParamStruct *pIPSecNeg,
	EcrioSUENotifyStruct **ppSUENotificationData
)
{
	u_int32 uError = ECRIO_SUE_NO_ERROR;
	EcrioSUENotifyStruct *pSUENotificationData = NULL;
	//EcrioSUENotifyIPSecNegotiatedStruct *pSUEIPSecNeg = NULL;

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (!pSUEGlobalData || !ppSUENotificationData)
	{
		uError = ECRIO_SUE_INSUFFICIENT_DATA_ERROR;
		goto EndTag;
	}

	pal_MemoryAllocate(sizeof(EcrioSUENotifyStruct), (void **)&pSUENotificationData);
	if (!pSUENotificationData)
	{
		uError = ECRIO_SUE_INSUFFICIENT_MEMORY_ERROR;
		goto EndTag;
	}

	*ppSUENotificationData = pSUENotificationData;

	pSUENotificationData->eNotificationType = ECRIO_SUE_NOTIFICATION_ENUM_EstablishIPSECSA;
	pSUENotificationData->u.pIPSecNotification = (void*)pIPSecNeg;

EndTag:

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return uError;
}