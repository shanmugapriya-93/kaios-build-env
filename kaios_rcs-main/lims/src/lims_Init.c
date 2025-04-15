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

/**
* @file lims_Init.c
* @brief Implementation of lims module initialization functionality.
*/

#include "lims.h"
#include "lims_internal.h"
#include "EcrioIOTA.h"
#ifdef ENABLE_QCMAPI
void _lims_EventCallback
(
	void *pCallbackFnData,
	ConnectionEvents uEvent,
	void *pData
)
{
	lims_moduleStruct *m = (lims_moduleStruct *)pCallbackFnData;
	lims_NotifyTypeEnums notifyType = lims_Notify_Type_NONE;
	u_int32 uLimsError = LIMS_NO_ERROR;

	switch (uEvent)
	{
		case ConnectionEvent_NotRegistered:
		{
			LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tDEREGISTRATION SUCCESSFUL",
				__FUNCTION__, __LINE__);
			EcrioSUENotifyRegisterResponseStruct deRegRspStruct;
			deRegRspStruct.uRspCode = 200; 
			if (m->pSueHandle != NULL)
			{
				EcrioSUESetRegistrationState(m->pSueHandle, _lims_REGISTRATION_STATE_ENUM_DeRegistered);
			}
			m->moduleState = lims_Module_State_CONNECTED;
			notifyType = lims_Notify_Type_DEREGISTERED;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, notifyType, (void *)&deRegRspStruct, m->pCallback.pContext);
		}
		break;

		case ConnectionEvent_Registered:
		{
			if (m->moduleState == lims_Module_State_CONNECTED)
			{
				LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tREGISTRATION SUCCESSFUL", __FUNCTION__, __LINE__);
				EcrioSUESetRegistrationState(m->pSueHandle, _lims_REGISTRATION_STATE_ENUM_Registered);
				notifyType = lims_Notify_Type_REGISTERED;
				m->moduleState = lims_Module_State_REGISTERED;
				LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tChange State from CONNECTED to REGISTERED", __FUNCTION__, __LINE__);

				m->pCallback.pLimsCallback(lims_CallbackType_Status, notifyType, NULL, m->pCallback.pContext);
			}
			else if (m->moduleState == lims_Module_State_REGISTERED)
			{
				EcrioSUESetRegistrationState(m->pSueHandle, _lims_REGISTRATION_STATE_ENUM_Registered);
				notifyType = lims_Notify_Type_REGISTERED;
				LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tChange State from REGISTERED to REGISTERED", __FUNCTION__, __LINE__);

				m->pCallback.pLimsCallback(lims_CallbackType_Status, notifyType, NULL, m->pCallback.pContext);
			}
		}
		break;

		case ConnectionEvent_Allowed:
		{
			LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tService is Allowed", __FUNCTION__, __LINE__);
		}
		break;

		case ConnectionEvent_NotAllowed:
		{
			LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tService is Not Allowed", __FUNCTION__, __LINE__);
			if (m->pSueHandle != NULL)
			{
				EcrioSUESetRegistrationState(m->pSueHandle, _lims_REGISTRATION_STATE_ENUM_DeRegistered);
			}
			m->moduleState = lims_Module_State_CONNECTED;
			notifyType = lims_Notify_Type_SERVICE_NOTALLOWED;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, notifyType, NULL, m->pCallback.pContext);
		}
		break;

		case ConnectionEvent_ForcefulClose:
		{
			LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tService is Closed Forceful", __FUNCTION__, __LINE__);
			if (m->pSueHandle != NULL)
			{
				EcrioSUESetRegistrationState(m->pSueHandle, _lims_REGISTRATION_STATE_ENUM_DeRegistered);
			}
			m->moduleState = lims_Module_State_CONNECTED;
			notifyType = lims_Notify_Type_SERVICE_FORCEFUL_CLOSE;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, notifyType, NULL, m->pCallback.pContext);
		}
		break;

		case ConnectionEvent_TerminateConnection:
		{
			LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tService is Terminated", __FUNCTION__, __LINE__);
			if (m->pSueHandle != NULL)
			{
				EcrioSUESetRegistrationState(m->pSueHandle, _lims_REGISTRATION_STATE_ENUM_DeRegistered);
			}
			m->moduleState = lims_Module_State_CONNECTED;
			notifyType = lims_Notify_Type_SERVICE_TERMINATE_CONNECTION;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, notifyType, NULL, m->pCallback.pContext);
		}
		break;

		case ConnectionEvent_Created:
		{
			LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tService is Created", __FUNCTION__, __LINE__);
			QCMConfigStruct *configStruct = (QCMConfigStruct *)pData;
			lims_configInternalStruct pConfigInternalStruct = {};
			u_int32 uLimsError = LIMS_NO_ERROR;
			u_int32 uSueError = ECRIO_SUE_NO_ERROR;

			pConfigInternalStruct.pSipPublicUserId = configStruct->pSipPublicUserId;
			pConfigInternalStruct.pSipHomeDomain = configStruct->pSipHomeDomain;
			pConfigInternalStruct.pSipPrivateUserId = configStruct->pSipPrivateUserId;
			pConfigInternalStruct.pPANI = configStruct->pPANI;
			pConfigInternalStruct.pSecurityVerify = configStruct->pSecurityVerify;
			pConfigInternalStruct.pAssociatedUri = configStruct->pAssociatedUri;
			pConfigInternalStruct.pUriUserPart = configStruct->pUriUserPart;
			pConfigInternalStruct.pIMEI = configStruct->pIMEI;
			uLimsError = lims_InitInternal(m, &pConfigInternalStruct);
			if (uLimsError != LIMS_NO_ERROR)
			{
				LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tlims_InitInternal() failed with error code: uLimsError=%d",
					__FUNCTION__, __LINE__, uLimsError);
				goto ERROR;
			}
	
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->pSipOutBoundProxyName=%s", __FUNCTION__, __LINE__, configStruct->pSipOutBoundProxyName);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->uPCSCFClientPort=%u", __FUNCTION__, __LINE__, configStruct->uPCSCFClientPort);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->uPCSCFServerPort=%u", __FUNCTION__, __LINE__, configStruct->uPCSCFServerPort);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->pLocalHostIPAddress=%s", __FUNCTION__, __LINE__, configStruct->pLocalHostIPAddress);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->uUEClientPort=%u", __FUNCTION__, __LINE__, configStruct->uUEClientPort);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->uSipOutBoundProxyPort=%u", __FUNCTION__, __LINE__, configStruct->uSipOutBoundProxyPort);

			lims_NetworkConnectionStruct connectionDetails = {0};
			connectionDetails.uNoOfRemoteIps = 1;
			LIMS_ALLOC(sizeof(u_char*) * connectionDetails.uNoOfRemoteIps, (void **)&(connectionDetails.ppRemoteIPs), uLimsError, ERR_None, m->logHandle);
			LIMS_ALLOC_AND_COPY_STRING(configStruct->pSipOutBoundProxyName, connectionDetails.ppRemoteIPs[0], uLimsError, ERR_None, m->logHandle);
			if ((configStruct->uPCSCFServerPort == 0) && (configStruct->uPCSCFClientPort == 0))
			{
				connectionDetails.uRemotePort = configStruct->uSipOutBoundProxyPort;
			}
			else
			{
				connectionDetails.uRemotePort = configStruct->uPCSCFServerPort;
			}
			connectionDetails.uRemoteClientPort = configStruct->uPCSCFClientPort;
			LIMS_ALLOC_AND_COPY_STRING(configStruct->pLocalHostIPAddress, connectionDetails.pLocalIp, uLimsError, ERR_None, m->logHandle);
			connectionDetails.uLocalPort = configStruct->uUEClientPort;
			if (configStruct->uIPType == IPType_IPv4)
			{
				connectionDetails.eIPType = lims_Network_IP_Type_V4;
			}
			else
			{
				connectionDetails.eIPType = lims_Network_IP_Type_V6;
			}
			connectionDetails.uStatus = lims_Network_Status_Success;

			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tPublicIdentity=%s", __FUNCTION__, __LINE__, m->pConfig->pPublicIdentity);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tHomeDomain=%s", __FUNCTION__, __LINE__, m->pConfig->pHomeDomain);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tPrivateIdentity=%s", __FUNCTION__, __LINE__, m->pConfig->pPrivateIdentity);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tRemoteIPs=%s", __FUNCTION__, __LINE__, connectionDetails.ppRemoteIPs[0]);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tRemotePort=%u", __FUNCTION__, __LINE__, connectionDetails.uRemotePort);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tuRemoteClientPort=%u", __FUNCTION__, __LINE__, connectionDetails.uRemoteClientPort);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tLocalIp=%s", __FUNCTION__, __LINE__, connectionDetails.pLocalIp);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tuLocalPort=%u", __FUNCTION__, __LINE__, connectionDetails.uLocalPort);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\teIPType=%u", __FUNCTION__, __LINE__, connectionDetails.eIPType);

			uLimsError = lims_NetworkStateChangeInternal(m, lims_Network_PDN_Type_IMS, lims_Network_Connection_Type_LTE, &connectionDetails);
			if (uLimsError != LIMS_NO_ERROR)
			{
				LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tlims_NetworkStateChange() is failed, uLimsError=%u", __FUNCTION__, __LINE__, uLimsError);
			}

			if (connectionDetails.pLocalIp != NULL)
			{
				LIMS_FREE((void **)&connectionDetails.pLocalIp, m->logHandle);
			}

			if (connectionDetails.ppRemoteIPs != NULL)
			{
				unsigned int i;

				for (i = 0; i < connectionDetails.uNoOfRemoteIps; ++i)
				{
					if (connectionDetails.ppRemoteIPs[i])
					{
						LIMS_FREE((void **)&connectionDetails.ppRemoteIPs[i], m->logHandle);
					}
				}

				LIMS_FREE((void **)&connectionDetails.ppRemoteIPs, m->logHandle);
			}
			
			EcrioSUESetRegistrationStateToNone(m->pSueHandle);
			uLimsError = lims_cleanUpAndCopyFeatureTags(m, m->uFeatures);
			if (uLimsError != LIMS_NO_ERROR)
			{
				goto ERROR;
			}
			
			uSueError = EcrioSUESetFeatureTagParams(m->pSueHandle, m->uFeatureTagCount, m->ppFeatureTagStruct);
			if (uSueError != ECRIO_SUE_NO_ERROR)
			{
				uLimsError = LIMS_SUE_REGISTER_ERROR;
				goto ERROR;
			}

			EcrioSUESetRegistrationStateToRegistering(m->pSueHandle);

//			notifyType = lims_Notify_Type_CONNECTION_CREATED;
//			m->pCallback.pLimsCallback(lims_CallbackType_Status, notifyType, NULL, m->pCallback.pContext);
		}
		break;

		case ConnectionEvent_GetACSConfiguration:
		{
			LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tGet ACS Configuration", __FUNCTION__, __LINE__);
			QcmACSConfigStruct *configStruct = (QcmACSConfigStruct *)pData;
			LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\ttype=%u", __FUNCTION__, __LINE__, configStruct->uType);
			LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tpConfig=%s", __FUNCTION__, __LINE__, configStruct->pConfig);
			notifyType = lims_Notify_Type_GET_ACSCONFIGURATION;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, notifyType, pData, m->pCallback.pContext);
		}
		break;

		case ConnectionEvent_AutoconfigRequestStatus:
		{
			LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tAutoconfigRequestStatus is arrived", __FUNCTION__, __LINE__);
			u_int32 *status = (u_int32 *)pData;
			LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tstatus%u", __FUNCTION__, __LINE__, *status);
			notifyType = lims_Notify_Type_AUTOCONFIG_REQUESTSTATUS;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, notifyType, pData, m->pCallback.pContext);
		}
		break;

		case ConnectionEvent_GetConfiguration:
		{
			LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tGet Configuration", __FUNCTION__, __LINE__);
			QCMConfigStruct *configStruct = (QCMConfigStruct *)pData;
			lims_configInternalStruct pConfigInternalStruct = {};
			u_int32 uLimsError = LIMS_NO_ERROR;
			u_int32 uSueError = ECRIO_SUE_NO_ERROR;

			pConfigInternalStruct.pSipPublicUserId = configStruct->pSipPublicUserId;
			pConfigInternalStruct.pSipHomeDomain = configStruct->pSipHomeDomain;
			pConfigInternalStruct.pSipPrivateUserId = configStruct->pSipPrivateUserId;
			pConfigInternalStruct.pPANI = configStruct->pPANI;
			pConfigInternalStruct.pSecurityVerify = configStruct->pSecurityVerify;
			pConfigInternalStruct.pAssociatedUri = configStruct->pAssociatedUri;
			pConfigInternalStruct.pUriUserPart = configStruct->pUriUserPart;
			pConfigInternalStruct.pIMEI = configStruct->pIMEI;
			uLimsError = lims_updateParamters(m, &pConfigInternalStruct);
			if (uLimsError != LIMS_NO_ERROR)
			{
				LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tlims_InitInternal() failed with error code: uLimsError=%d",
					__FUNCTION__, __LINE__, uLimsError);
				goto ERROR;
			}

			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->pSipOutBoundProxyName=%s", __FUNCTION__, __LINE__, configStruct->pSipOutBoundProxyName);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->uPCSCFClientPort=%u", __FUNCTION__, __LINE__, configStruct->uPCSCFClientPort);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->uPCSCFServerPort=%u", __FUNCTION__, __LINE__, configStruct->uPCSCFServerPort);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->pLocalHostIPAddress=%s", __FUNCTION__, __LINE__, configStruct->pLocalHostIPAddress);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->uUEClientPort=%u", __FUNCTION__, __LINE__, configStruct->uUEClientPort);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->uSipOutBoundProxyPort=%u", __FUNCTION__, __LINE__, configStruct->uSipOutBoundProxyPort);

			lims_NetworkConnectionStruct connectionDetails = {0};
			connectionDetails.uNoOfRemoteIps = 1;
			LIMS_ALLOC(sizeof(u_char*) * connectionDetails.uNoOfRemoteIps, (void **)&(connectionDetails.ppRemoteIPs), uLimsError, ERR_None, m->logHandle);
			LIMS_ALLOC_AND_COPY_STRING(configStruct->pSipOutBoundProxyName, connectionDetails.ppRemoteIPs[0], uLimsError, ERR_None, m->logHandle);
			if ((configStruct->uPCSCFServerPort == 0) && (configStruct->uPCSCFClientPort == 0))
			{
				connectionDetails.uRemotePort = configStruct->uSipOutBoundProxyPort;
			}
			else
			{
				connectionDetails.uRemotePort = configStruct->uPCSCFServerPort;
			}
			connectionDetails.uRemoteClientPort = configStruct->uPCSCFClientPort;
			LIMS_ALLOC_AND_COPY_STRING(configStruct->pLocalHostIPAddress, connectionDetails.pLocalIp, uLimsError, ERR_None, m->logHandle);
			connectionDetails.uLocalPort = configStruct->uUEClientPort;
			if (configStruct->uIPType == IPType_IPv4)
			{
				connectionDetails.eIPType = lims_Network_IP_Type_V4;
			}
			else
			{
				connectionDetails.eIPType = lims_Network_IP_Type_V6;
			}
			connectionDetails.uStatus = lims_Network_Status_Success;

			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tPublicIdentity=%s", __FUNCTION__, __LINE__, m->pConfig->pPublicIdentity);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tHomeDomain=%s", __FUNCTION__, __LINE__, m->pConfig->pHomeDomain);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tPrivateIdentity=%s", __FUNCTION__, __LINE__, m->pConfig->pPrivateIdentity);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tRemoteIPs=%s", __FUNCTION__, __LINE__, connectionDetails.ppRemoteIPs[0]);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tRemotePort=%u", __FUNCTION__, __LINE__, connectionDetails.uRemotePort);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tuRemoteClientPort=%u", __FUNCTION__, __LINE__, connectionDetails.uRemoteClientPort);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tLocalIp=%s", __FUNCTION__, __LINE__, connectionDetails.pLocalIp);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tuLocalPort=%u", __FUNCTION__, __LINE__, connectionDetails.uLocalPort);
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\teIPType=%u", __FUNCTION__, __LINE__, connectionDetails.eIPType);

			uLimsError = lims_NetworkStateChangeInternal(m, lims_Network_PDN_Type_IMS, lims_Network_Connection_Type_LTE, &connectionDetails);
			if (uLimsError != LIMS_NO_ERROR)
			{
				LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tlims_NetworkStateChange() is failed, uLimsError=%u", __FUNCTION__, __LINE__, uLimsError);
			}

			if (connectionDetails.pLocalIp != NULL)
			{
				LIMS_FREE((void **)&connectionDetails.pLocalIp, m->logHandle);
			}

			if (connectionDetails.ppRemoteIPs != NULL)
			{
				unsigned int i;

				for (i = 0; i < connectionDetails.uNoOfRemoteIps; ++i)
				{
					if (connectionDetails.ppRemoteIPs[i])
					{
						LIMS_FREE((void **)&connectionDetails.ppRemoteIPs[i], m->logHandle);
					}
				}

				LIMS_FREE((void **)&connectionDetails.ppRemoteIPs, m->logHandle);
			}

			uLimsError = lims_cleanUpAndCopyFeatureTags(m, m->uFeatures);
			if (uLimsError != LIMS_NO_ERROR)
			{
				goto ERROR;
			}

			uSueError = EcrioSUESetFeatureTagParams(m->pSueHandle, m->uFeatureTagCount, m->ppFeatureTagStruct);
			if (uSueError != ECRIO_SUE_NO_ERROR)
			{
				uLimsError = LIMS_SUE_REGISTER_ERROR;
				goto ERROR;
			}
		}
		break;

		default:
			break;
	}

	goto ERR_None;

ERROR:
	LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tRegister is failed", __FUNCTION__, __LINE__);
	EcrioSUENotifyRegisterResponseStruct deRegRspStruct;
	deRegRspStruct.uRspCode = 200; 
	if (m->pSueHandle != NULL)
	{
		EcrioSUESetRegistrationState(m->pSueHandle, _lims_REGISTRATION_STATE_ENUM_DeRegistered);
	}
	m->moduleState = lims_Module_State_CONNECTED;
	notifyType = lims_Notify_Type_DEREGISTERED;
	m->pCallback.pLimsCallback(lims_CallbackType_Status, notifyType, (void *)&deRegRspStruct, m->pCallback.pContext);

ERR_None:

	return;
}
#endif

/**
* This function can be used to initialize an overall lims implementation
* instance. The lims definition assumes that only one lims instance can exist on
* a device at a time. This function acts synchronously, and upon return the
* lims instance should be prepared.
*
* It also initialize all the required underlying  modules..
*
* @param[in] pConfig			Pointer to a lims_ConfigStruct structure. Must
*								be valid. The caller owns the structure and the
*								lims implementation will consume or copy from it.
* @param[in] pCallback			Pointer to a structure that holds all the callback
*								pointers for asynchronous communications back to
*								the caller.
* @param[out] pError			Pointer to a u_int32 to place the result code in
*								upon return. Must be valid.
*
* @return Returns a valid lims instance handle. If NULL, pError will provide
* the error code. Callers only need to rely on the function return, it is not
* necessary to check both the return and pError. They will be guaranteed to
* be consistent.
*/
LIMSHANDLE lims_Init
(
	lims_ConfigStruct *pConfig,
	lims_CallbackStruct *pCallback,
	u_int32 *pError
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
	u_int32 uPalError = KPALErrorNone;
	lims_moduleStruct *m = NULL;
	EcrioSipTimerStruct timers;
#ifdef ENABLE_QCMAPI
	u_char *pFeature = NULL;
#endif
	/** Check parameter validation */
	if (pError == NULL)
	{
		return NULL;
	}

	if (pConfig == NULL)
	{
		*pError = LIMS_INVALID_PARAMETER1;
		return NULL;
	}

	if (pCallback == NULL)
	{
		*pError = LIMS_INVALID_PARAMETER2;
		return NULL;
	}

	*pError = LIMS_NO_ERROR;

	pal_MutexGlobalLock(pConfig->pal);

	LIMSLOGI(pConfig->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	LIMSLOGV(pConfig->logHandle, KLogTypeGeneral, "%s:%u\tEcrio iota version: %s", __FUNCTION__, __LINE__, ECRIO_IOTA_VERSION_STRING)
	LIMSLOGI(pConfig->logHandle, KLogTypeFuncParams, "%s:%u\tpConfig=%p, pCallback=%p, pError=%p",
		__FUNCTION__, __LINE__, pConfig, pCallback, pError);

	/** Allocate memory for the lims module instance structure. */
	LIMS_ALLOC(sizeof(lims_moduleStruct), (void **)&m, uLimsError, ERR_Fail, pConfig->logHandle);

	m->pSigMgrHandle = NULL;
	m->pSueHandle = NULL;
	m->pSmsHandle = NULL;
	m->pMoipHandle = NULL;
	m->pCpmHandle = NULL;
	m->pConfig = NULL;
	m->pConnection = NULL;
	m->uFeatureTagCount = 0;
	m->ppFeatureTagStruct = NULL;
	m->bSubscribed = Enum_FALSE;
	m->moduleState = lims_Module_State_INITIALIZING;
	m->bMoIPFeature = Enum_FALSE;
	m->bSmsFeature = Enum_FALSE;
	m->bCpmFeature = Enum_FALSE;

	LIMSLOGI(pConfig->logHandle, KLogTypeGeneral, "%s:%u\tChange State from UNINITIALIZED to INITIALIZING", __FUNCTION__, __LINE__);

	/* make a copy of callback function pointers */
	m->pCallback.pContext = pCallback->pContext;
	m->pCallback.pLimsCallback = pCallback->pLimsCallback;
	m->pCallback.pLimsGetPropertyCallback = pCallback->pLimsGetPropertyCallback;

	m->logHandle = pConfig->logHandle;
	m->pal = pConfig->pal;

	/* For non TLS configuration, UDP/TCP must be enabled */
	if (pConfig->bTLSEnabled == Enum_FALSE)
	{
		if (pConfig->bEnableTcp == Enum_FALSE && pConfig->bEnableUdp == Enum_FALSE)
		{
			LIMSLOGE(pConfig->logHandle, KLogTypeGeneral, "%s:%u\tboth bEnableTcp and bEnableUdp set to FALSE", __FUNCTION__, __LINE__);
			*pError = LIMS_INVALID_CHANNEL_SETTING;
			return NULL;
		}
	} /* For TLS enabled configuration, UDP/TCP must be disbaled */
	else
	{
		if (pConfig->bEnableTcp == Enum_TRUE || pConfig->bEnableUdp == Enum_TRUE)
		{
			LIMSLOGE(pConfig->logHandle, KLogTypeGeneral, "%s:%u\tTLS is enabled so, TCP/UDP is not allowed", __FUNCTION__, __LINE__);
			*pError = LIMS_INVALID_CHANNEL_SETTING;
			return NULL;
		}
	}

	/** Allocate memory for the lims module config structure and copy the parameters... */
	LIMS_ALLOC(sizeof(lims_ConfigStruct), (void **)&m->pConfig, uLimsError, ERR_Fail, pConfig->logHandle);
	if (pConfig->pPublicIdentity != NULL)
	{
		LIMS_ALLOC_AND_COPY_STRING(pConfig->pPublicIdentity, m->pConfig->pPublicIdentity, uLimsError, ERR_Fail, pConfig->logHandle);
	}
	if (pConfig->pHomeDomain != NULL)
	{
		LIMS_ALLOC_AND_COPY_STRING(pConfig->pHomeDomain, m->pConfig->pHomeDomain, uLimsError, ERR_Fail, pConfig->logHandle);
	}
	if (pConfig->pPrivateIdentity != NULL)
	{
		LIMS_ALLOC_AND_COPY_STRING(pConfig->pPrivateIdentity, m->pConfig->pPrivateIdentity, uLimsError, ERR_Fail, pConfig->logHandle);
	}
	if (pConfig->pPassword != NULL)
	{
		LIMS_ALLOC_AND_COPY_STRING(pConfig->pPassword, m->pConfig->pPassword, uLimsError, ERR_Fail, pConfig->logHandle);
	}
	if (pConfig->pUserAgent != NULL)
	{
		LIMS_ALLOC_AND_COPY_STRING(pConfig->pUserAgent, m->pConfig->pUserAgent, uLimsError, ERR_Fail, pConfig->logHandle);
	}
	if (pConfig->pDeviceId != NULL)
	{
		LIMS_ALLOC_AND_COPY_STRING(pConfig->pDeviceId, m->pConfig->pDeviceId, uLimsError, ERR_Fail, pConfig->logHandle);
	}
	if (pConfig->pPANI != NULL)
	{
		LIMS_ALLOC_AND_COPY_STRING(pConfig->pPANI, m->pConfig->pPANI, uLimsError, ERR_Fail, pConfig->logHandle);
	}

	m->pConfig->uRegExpireInterval = pConfig->uRegExpireInterval;
	m->pConfig->uSubExpireInterval = pConfig->uSubExpireInterval;
	m->pConfig->bSubscribeRegEvent = pConfig->bSubscribeRegEvent;
	m->pConfig->bUnSubscribeRegEvent = pConfig->bUnSubscribeRegEvent;
	m->pConfig->bSendRingingResponse = pConfig->bSendRingingResponse;
	m->pConfig->uPublishRefreshInterval = pConfig->uPublishRefreshInterval;
	m->pConfig->bisChatbot = pConfig->bisChatbot;
	m->pConfig->bCapabilityInfo = pConfig->bCapabilityInfo;
	m->pConfig->eAlgorithm = pConfig->eAlgorithm;
	m->pConfig->pOOMObject = pConfig->pOOMObject;
	m->pConfig->bIsRelayEnabled = pConfig->bIsRelayEnabled;
	m->pConfig->uRelayServerPort = pConfig->uRelayServerPort;
	m->pConfig->uMtuSize  = pConfig->uMtuSize;
	m->pConfig->uNoActivityTimerInterval = pConfig->uNoActivityTimerInterval;
	m->pConfig->bEnableUdp = pConfig->bEnableUdp;
	m->pConfig->bEnableTcp = pConfig->bEnableTcp;
	m->pConfig->bEnablePAI = pConfig->bEnablePAI;
	m->pConfig->bSupportedPrecondition = pConfig->bSupportedPrecondition;
	m->pConfig->bRequiredPrecondition = pConfig->bRequiredPrecondition;
	m->pConfig->bTLSEnabled = pConfig->bTLSEnabled;
	m->pConfig->bTLSPeerVerification = pConfig->bTLSPeerVerification;

	if (pConfig->pTLSCertificate != NULL)
	{
		LIMS_ALLOC_AND_COPY_STRING(pConfig->pTLSCertificate, m->pConfig->pTLSCertificate, uLimsError, ERR_Fail, pConfig->logHandle);
	}
	else
	{
		if (pConfig->bTLSPeerVerification)
		{
			LIMSLOGE(pConfig->logHandle, KLogTypeGeneral, "%s:%u\tlims_initSigMgrModule() SERTIFICATE NOT FOUND! PEAR VARIFICATION NOT POSSIBLE=%d",
				__FUNCTION__, __LINE__, uLimsError);

			m->pConfig->bTLSPeerVerification = Enum_FALSE;
		}
	}

	if (pConfig->pRelayServerIP != NULL)
	{
		LIMS_ALLOC_AND_COPY_STRING(pConfig->pRelayServerIP, m->pConfig->pRelayServerIP, uLimsError, ERR_Fail, pConfig->logHandle);
	}
#if defined(ENABLE_RCS)
	if (pConfig->pDisplayName != NULL)
	{
		LIMS_ALLOC_AND_COPY_STRING(pConfig->pDisplayName, m->pConfig->pDisplayName, uLimsError, ERR_Fail, pConfig->logHandle);
	}
#endif	// ENABLE_RCS

	m->pConfig->regModel.isRegistrationShared = pConfig->regModel.isRegistrationShared;
	if (m->pConfig->regModel.isRegistrationShared == Enum_TRUE)
	{
		if (pConfig->regModel.pRegSharedData != NULL)
		{
			LIMS_ALLOC_AND_COPY_STRING(pConfig->regModel.pRegSharedData, m->pConfig->regModel.pRegSharedData, uLimsError, ERR_Fail, pConfig->logHandle);
		}
	}

	if (m->pConfig->pOOMObject != NULL)
	{
		if (m->pConfig->pOOMObject->ec_oom_GetSipTimers(&timers) == KOOMErrorNone)
		{
			m->sipTimers.uTimerF = timers.uTimerF;
			m->sipTimers.uTimerT1 = timers.uTimerT1;
			m->sipTimers.uTimerT2 = timers.uTimerT2;
		}

		u_int32 uAllowMethods = m->pConfig->pOOMObject->ec_oom_GetAllowMethod();
		lims_getAndCopyAllowMethods(m, uAllowMethods);
	}

#ifndef ENABLE_QCMAPI
	/* Initialize the SigMgr module. */
	uLimsError = lims_initSigMgrModule(m);
	if (uLimsError != LIMS_NO_ERROR)
	{
		LIMSLOGE(pConfig->logHandle, KLogTypeGeneral, "%s:%u\tlims_initSigMgrModule() failed with error code: uLimsError=%d",
			__FUNCTION__, __LINE__, uLimsError);
		goto ERR_Fail;
	}

	/** Initialize the SUE module. */
	uLimsError = lims_initSueModule(m);
	if (uLimsError != LIMS_NO_ERROR)
	{
		LIMSLOGE(pConfig->logHandle, KLogTypeGeneral, "%s:%u\tlims_initSueEngine() failed with error code: uLimsError=%d",
			__FUNCTION__, __LINE__, uLimsError);
		goto ERR_Fail;
	}

#if defined(ENABLE_RCS)
	/** Initialize the CPM module.*/
	m->bCpmFeature = Enum_TRUE;
	uLimsError = lims_initCpmModule(m);
	if (uLimsError != LIMS_NO_ERROR)
	{
		LIMSLOGE(pConfig->logHandle, KLogTypeGeneral, "%s:%u\tlims_initCpmModule() failed with error code: uLimsError=%d",
			__FUNCTION__, __LINE__, uLimsError);
		goto ERR_Fail;
	}

#endif	// ENABLE_RCS
#endif

#ifdef ENABLE_QCMAPI
#if 0
	LIMS_ALLOC(1024, (void **)&pFeature, uLimsError, ERR_None, m->logHandle);
	pal_StringNCopy(pFeature, LIMS_3GPP_ICSI_FEATURE_TAG_STRING);
	pal_StringNConcatenate(pFeature, "=");
	pal_StringNConcatenate(pFeature, LIMS_3GPP_ICSI_PAGER_MODE_FEATURE_TAG_VALUE_STRING);
	pal_StringNConcatenate(pFeature, ";");
	pal_StringNConcatenate(pFeature, LIMS_3GPP_ICSI_FEATURE_TAG_STRING);
	pal_StringNConcatenate(pFeature, "=");
	pal_StringNConcatenate(pFeature, LIMS_3GPP_ICSI_LARGE_MODE_FEATURE_TAG_VALUE_STRING);
	pal_StringNConcatenate(pFeature, ";");
	pal_StringNConcatenate(pFeature, LIMS_3GPP_ICSI_FEATURE_TAG_STRING);
	pal_StringNConcatenate(pFeature, "=");
	pal_StringNConcatenate(pFeature, LIMS_3GPP_ICSI_CPM_SESSION_FEATURE_TAG_VALUE_STRING);
//	pal_StringNConcatenate(pFeature, ";");
//	pal_StringNConcatenate(pFeature, LIMS_3GPP_IARI_FEATURE_TAG_STRING);
//	pal_StringNConcatenate(pFeature, "=");
//	pal_StringNConcatenate(pFeature, LIMS_3GPP_IARI_FT_FEATURE_TAG_VALUE_STRING);
	uPalError = pal_QcmInit(m->pal, m->logHandle, &(m->qcmInstance), (u_char *)m->pConfig->regModel.pRegSharedData, pFeature, _lims_EventCallback, m, Enum_TRUE);
#endif
	uPalError = pal_QcmInit(m->pal, m->logHandle, &(m->qcmInstance), (u_char *)m->pConfig->regModel.pRegSharedData, _lims_EventCallback, m, Enum_TRUE);
	if (uPalError != KPALErrorNone)
	{
		LIMSLOGE(pConfig->logHandle, KLogTypeGeneral, "%s:%u\tpal_QcmInit() failed with error code: uPalError=%d",
			__FUNCTION__, __LINE__, uPalError);
		uLimsError = LIMS_INVALID_OPERATION;
		goto ERR_Fail;		
	}
#if 0
	if (pFeature)
	{
		LIMS_FREE((void **)&pFeature, m->logHandle);
	}
#endif
#endif

	m->moduleState = lims_Module_State_INITIALIZED;
	LIMSLOGI(pConfig->logHandle, KLogTypeGeneral, "%s:%u\tChange State from INITIALIZING to INITIALIZED", __FUNCTION__, __LINE__);

	pal_MutexGlobalUnlock(pConfig->pal);

	goto ERR_None;

ERR_Fail:
	if (m != NULL)
	{
		/** Only cleanup if something was actually allocated. */
		lims_cleanUpModuleStructure(m);

		LIMS_FREE((void **)&m, pConfig->logHandle);
		m = NULL;
	}

	*pError = uLimsError;

	pal_MutexGlobalUnlock(pConfig->pal);

ERR_None:
	LIMSLOGI(pConfig->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);
	return (LIMSHANDLE)m;
}
