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
* @file lims_Register.c
* @brief Implementation of lims module registration functionality.
*/

#include "lims.h"
#include "lims_internal.h"

/**
* This function is used to initialize the signaling manager module, set the signaling parameter
* and callback function.
*/
u_int32 lims_initSigMgrModule
(
	lims_moduleStruct *m
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
	u_int32 uSigMgrError = ECRIO_SIGMGR_NO_ERROR;
	SIGMGRHANDLE pSigMgrHandle = NULL;
	EcrioSigMgrInitStruct sigMgrInitStruct;
	EcrioSigMgrSignalingInfoStruct sigMgrSignalingInfoStruct = { 0 };
	EcrioSigMgrEngineCallbackStruct pSigMgrEngineCallbackStruct = { 0 };
	EcrioSigMgrCustomTimersStruct customTimer = { 0 };
	EcrioSipSessionTimerStruct timers = { 0 };

	m->pSigMgrHandle = NULL;

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	sigMgrInitStruct.pal = m->pal;
	sigMgrInitStruct.pLogEngineHandle = m->logHandle;		/** setting the log handle to signaling manager. */
	sigMgrInitStruct.pOOMObject = m->pConfig->pOOMObject;	/** setting the Operator Object Model instance to signaling manager. */

	/* Initialize the signaling manager module */
	uSigMgrError = EcrioSigMgrInit(&sigMgrInitStruct, &pSigMgrHandle);
	if (uSigMgrError != ECRIO_SIGMGR_NO_ERROR)
	{
		uLimsError = LIMS_SIGMGR_INIT_ERROR;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrInit() failed with error code: SigMgrError=%d",
			__FUNCTION__, __LINE__, uSigMgrError);
		goto ERR_Fail;
	}

	m->pSigMgrHandle = pSigMgrHandle;	/** store the signaling manager handle in module structure for other modules use.*/

	sigMgrSignalingInfoStruct.pPublicIdentity_imsi = NULL;	// For now we are using only one public Id. Assuming it is always MSISDN based
	sigMgrSignalingInfoStruct.pPublicIdentity_msisdn = (u_char *)m->pConfig->pPublicIdentity;
	sigMgrSignalingInfoStruct.pHomeDomain = (u_char *)m->pConfig->pHomeDomain;
	sigMgrSignalingInfoStruct.pPassword = (u_char *)m->pConfig->pPassword;
	sigMgrSignalingInfoStruct.pRegistrarDomain = (u_char *)m->pConfig->pHomeDomain;
	sigMgrSignalingInfoStruct.pUserAgent = (u_char *)m->pConfig->pUserAgent;
	sigMgrSignalingInfoStruct.pPrivateId = (u_char *)m->pConfig->pPrivateIdentity;
	sigMgrSignalingInfoStruct.bWildCharInContact = Enum_TRUE;
	sigMgrSignalingInfoStruct.registrationExpires = m->pConfig->uRegExpireInterval;
	sigMgrSignalingInfoStruct.subscriptionExpires = m->pConfig->uSubExpireInterval;
	sigMgrSignalingInfoStruct.publishExpires = m->pConfig->uPublishRefreshInterval;
	if (m->pConfig->uNoActivityTimerInterval > 0)
		sigMgrSignalingInfoStruct.bEnableNoActivityTimer = Enum_TRUE;
	else
		sigMgrSignalingInfoStruct.bEnableNoActivityTimer = Enum_FALSE;

	sigMgrSignalingInfoStruct.bEnableUdp = m->pConfig->bEnableUdp;
	sigMgrSignalingInfoStruct.bEnableTcp = m->pConfig->bEnableTcp;
	
	sigMgrSignalingInfoStruct.uNoActivityTimerInterval = m->pConfig->uNoActivityTimerInterval;
	/* get the session timers */
	m->pConfig->pOOMObject->ec_oom_GetSessionTimers(&timers);
	sigMgrSignalingInfoStruct.minSe = timers.uTimerMinSE;
	sigMgrSignalingInfoStruct.sessionExpires = timers.uTimerSE;/* seconds */
	if (m->pConfig->uMtuSize > 0)
		sigMgrSignalingInfoStruct.uUdpMtu = m->pConfig->uMtuSize;
	else
		sigMgrSignalingInfoStruct.uUdpMtu = 1300; // default value as per RFC 3261.
	sigMgrSignalingInfoStruct.eAuthAlgorithm = m->pConfig->eAlgorithm;
	sigMgrSignalingInfoStruct.numSupportedMethods = m->uSupportedMethodsCount;
	sigMgrSignalingInfoStruct.ppSupportedMethods = m->ppSupportedMethods;
	sigMgrSignalingInfoStruct.pDisplayName = (u_char *)m->pConfig->pDisplayName;
	sigMgrSignalingInfoStruct.pPANI = (u_char *)m->pConfig->pPANI;
	sigMgrSignalingInfoStruct.pSecurityVerify = (u_char *)m->pConfig->pSecurityVerify;
	sigMgrSignalingInfoStruct.pAssociatedUri = (u_char *)m->pConfig->pAssociatedUri;
	sigMgrSignalingInfoStruct.pUriUserPart = (u_char *)m->pConfig->pUriUserPart;
	sigMgrSignalingInfoStruct.bTLSEnabled = m->pConfig->bTLSEnabled;
	sigMgrSignalingInfoStruct.bTLSPeerVerification = m->pConfig->bTLSPeerVerification;
	sigMgrSignalingInfoStruct.pTLSCertificate = m->pConfig->pTLSCertificate;

	// TODO timer values will be updated from OOM...
	customTimer.T1 = m->sipTimers.uTimerT1;
	customTimer.T2 = m->sipTimers.uTimerT2;
	customTimer.TimerF = m->sipTimers.uTimerF;
	customTimer.T4 = 5000;
	customTimer.TimerA = customTimer.T1;
	customTimer.TimerB = 55 * customTimer.T1;
	customTimer.TimerD = 32000;
	customTimer.TimerE = customTimer.T1;
	customTimer.TimerG = customTimer.T1;
	customTimer.TimerH = 64 * customTimer.T1;
	customTimer.TimerI = 5000;
	customTimer.TimerJ = 64 * customTimer.T1;
	customTimer.TimerK = 5000;
	customTimer.TimerM = 64 * customTimer.T1;
	sigMgrSignalingInfoStruct.pCustomSIPTimers = &customTimer;

	/* set signaling parameters */
	uSigMgrError = EcrioSigMgrSetSignalingParam(pSigMgrHandle, &sigMgrSignalingInfoStruct);
	if (uSigMgrError != ECRIO_SIGMGR_NO_ERROR)
	{
		uLimsError = LIMS_SIGMGR_SET_PARAM_ERROR;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrSetSignalingParam() failed with error code: SigMgrError=%d",
			__FUNCTION__, __LINE__, uSigMgrError);
		goto ERR_Fail;
	}

	/* set the get property calback function pointer */
	pSigMgrEngineCallbackStruct.getPropertyCallbackFn = (EcrioSigMgrGetPropertyCallback)m->pCallback.pLimsGetPropertyCallback;
	pSigMgrEngineCallbackStruct.pCallbackData = m->pCallback.pContext;

	/* set callback function and register lims module in sigmgr. */
	uSigMgrError = EcrioSigMgrSetCallbacks(pSigMgrHandle, &pSigMgrEngineCallbackStruct, EcrioSigMgrCallbackRegisteringModule_LIMS);
	if (uSigMgrError != ECRIO_SIGMGR_NO_ERROR)
	{
		uLimsError = LIMS_SIGMGR_SET_CALLBACK_ERROR;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrSetSignalingParam() failed with error code: SigMgrError=%d",
			__FUNCTION__, __LINE__, uSigMgrError);
		goto ERR_Fail;
	}

	goto ERR_None;

ERR_Fail:
	if (pSigMgrHandle != NULL)
	{
		EcrioSigMgrDeInit(&m->pSigMgrHandle);
		m->pSigMgrHandle = NULL;
	}

	// We don't need to free here... all these values are just references..lims_Deinit() take care this.

ERR_None:
	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);
	return uLimsError;
}

/**
* This function is used to deinitializes the signaling manager module...
*/
u_int32 lims_deInitSigMgrModule
(
	lims_moduleStruct *m
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
	u_int32 uSigMgrError = ECRIO_SIGMGR_NO_ERROR;
	SIGMGRHANDLE pSigMgrHandle = m->pSigMgrHandle;

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	uSigMgrError = EcrioSigMgrDeInit(&pSigMgrHandle);
	if (uSigMgrError != ECRIO_SIGMGR_NO_ERROR)
	{
		uLimsError = LIMS_SIGMGR_DEINIT_ERROR;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrSetSignalingParam() failed with error code: SigMgrError=%d",
			__FUNCTION__, __LINE__, uSigMgrError);
	}
	else
	{
		m->pSigMgrHandle = NULL;
	}

	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);
	return uLimsError;
}

/**
*	This function is used to initializes the sue module.
*/
u_int32 lims_initSueModule
(
	lims_moduleStruct *m
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
	u_int32 uSueError = ECRIO_SUE_NO_ERROR;
	EcrioSUEParamStruct sueParamStruct;
	EcrioSUECallbackStruct callbackStruct;
	SIGMGRHANDLE pSigMgrHandle = m->pSigMgrHandle;

	m->pSueHandle = NULL;

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	sueParamStruct.pal = m->pal;
	sueParamStruct.pLogHandle = m->logHandle;
	sueParamStruct.pOOMObject = m->pConfig->pOOMObject;
	sueParamStruct.bSubscribeRegEvent = m->pConfig->bSubscribeRegEvent;
	sueParamStruct.bUnSubscribeRegEvent = m->pConfig->bUnSubscribeRegEvent;
	callbackStruct.pCallbackData = m;
	callbackStruct.pNotifySUECallbackFn = lims_sueStatusCallbackHandler;

	/* Initialize the SUE module */
	uSueError = EcrioSUEInit(&sueParamStruct, &callbackStruct, pSigMgrHandle, &m->pSueHandle);
	if (uSueError != ECRIO_SUE_NO_ERROR)
	{
		uLimsError = LIMS_SUE_INIT_ERROR;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tEcrioSUEInit() failed with error code: uSueError=%d",
			__FUNCTION__, __LINE__, uSueError);
		goto ERR_None;
	}

ERR_None:
	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);
	return uLimsError;
}

/**
*	This function is used to deinitializes the sue module.
*/
u_int32 lims_deInitSueModule
(
	lims_moduleStruct *m
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
	u_int32 uSueError = ECRIO_SUE_NO_ERROR;

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	uSueError = EcrioSUEDeinit(&m->pSueHandle);
	if (uSueError != ECRIO_SUE_NO_ERROR)
	{
		uLimsError = LIMS_SUE_DEINIT_ERROR;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tEcrioSUEDeinit() failed with error code: uSueError=%d",
			__FUNCTION__, __LINE__, uSueError);
	}
	else
	{
		m->pSueHandle = NULL;
	}

	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);
	return uLimsError;
}

#if defined(ENABLE_RCS)
/**
* This function is used to initialize the cpm module.
*/
u_int32 lims_initCpmModule
(
	lims_moduleStruct *m
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
	EcrioCPMParamStruct pCPMParamStruct = { 0 };
	EcrioCPMCallbackStruct callback = { 0 };

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	pCPMParamStruct.hLogHandle = m->logHandle;
	pCPMParamStruct.pal = m->pal;
	pCPMParamStruct.pHomeDomain = (u_char *)m->pConfig->pHomeDomain;
	pCPMParamStruct.pPublicIdentity = (u_char *)m->pConfig->pPublicIdentity;
	pCPMParamStruct.pPrivateIdentity = (u_char *)m->pConfig->pPrivateIdentity;
	pCPMParamStruct.pPassword = (u_char *)m->pConfig->pPassword;
	pCPMParamStruct.pDeviceId = (u_char *)m->pConfig->pDeviceId;
	pCPMParamStruct.pDisplayName = (u_char *)m->pConfig->pDisplayName;
	pCPMParamStruct.bSendRingingResponse = m->pConfig->bSendRingingResponse;
	pCPMParamStruct.bEnablePAI = m->pConfig->bEnablePAI;
	pCPMParamStruct.pOOMObject = m->pConfig->pOOMObject;
	pCPMParamStruct.bIsRelayEnabled = m->pConfig->bIsRelayEnabled;
	pCPMParamStruct.pRelayServerIP = m->pConfig->pRelayServerIP;
	pCPMParamStruct.uRelayServerPort = m->pConfig->uRelayServerPort;
	pCPMParamStruct.pPANI = (u_char*)m->pConfig->pPANI;
	pCPMParamStruct.bMsrpOverTLS = m->pConfig->bTLSEnabled;

	callback.pCPMNotificationCallbackFn = lims_cpmStatusCallbackHandler;
//	callback.pMSRPGetPropertyCallback = (EcrioMSRPGetPropertyCallback)m->pCallback.pLimsGetPropertyCallback;
	callback.pCallbackData = m;

	uCPMError = EcrioCPMInit(&pCPMParamStruct, &callback, m->pSigMgrHandle, &m->pCpmHandle);
	if (uCPMError != ECRIO_CPM_NO_ERROR)
	{
		uLimsError = LIMS_CPM_INIT_ERROR;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tEcrioCPMInit() failed with error code: uCPMError=%d",
			__FUNCTION__, __LINE__, uCPMError);
		goto ERR_None;
	}

ERR_None:
	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);
	return uLimsError;
}

/**
* This function is used to deinitialize the cpm module.
*/
u_int32 lims_deInitCpmModule
(
	lims_moduleStruct *m
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	uCPMError = EcrioCPMDeinit(&m->pCpmHandle);
	if (uCPMError != ECRIO_CPM_NO_ERROR)
	{
		uLimsError = LIMS_CPM_DEINIT_ERROR;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tEcrioCPMDeinit() failed with error code: uCPMError=%d",
			__FUNCTION__, __LINE__, uCPMError);
		goto ERR_None;
	}
	else
	{
		m->pCpmHandle = NULL;
	}

ERR_None:
	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);
	return uLimsError;
}
#endif	// ENABLE_RCS




/**
* This function is used to clean up the module structure
*/
u_int32 lims_cleanUpModuleStructure
(
	lims_moduleStruct *m
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
	u_int32 counter = 0;

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

#if defined(ENABLE_RCS)
	if (m->pCpmHandle != NULL)
	{
		uLimsError = lims_deInitCpmModule(m);
		if (uLimsError != LIMS_NO_ERROR)
		{
			LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tlims_deInitCpmModule() failed with error code: uLimsError=%d",
				__FUNCTION__, __LINE__, uLimsError);
		}

		m->pCpmHandle = NULL;
	}


#endif	// ENABLE_RCS

	if (m->pSueHandle != NULL)
	{
		uLimsError = lims_deInitSueModule(m);
		if (uLimsError != LIMS_NO_ERROR)
		{
			LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tlims_deInitSueEngine() failed with error code: uLimsError=%d",
				__FUNCTION__, __LINE__, uLimsError);
		}

		m->pSueHandle = NULL;
	}

	if (m->pSigMgrHandle != NULL)
	{
		uLimsError = lims_deInitSigMgrModule(m);
		if (uLimsError != LIMS_NO_ERROR)
		{
			LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tlims_deInitSigMgrModule() failed with error code: uLimsError=%d",
				__FUNCTION__, __LINE__, uLimsError);
		}

		m->pSigMgrHandle = NULL;
	}

	if (m->pConfig != NULL)
	{
		if (m->pConfig->pPublicIdentity != NULL)
		{
			LIMS_FREE((void **)&m->pConfig->pPublicIdentity, m->logHandle);
			m->pConfig->pPublicIdentity = NULL;
		}

		if (m->pConfig->pHomeDomain != NULL)
		{
			LIMS_FREE((void **)&m->pConfig->pHomeDomain, m->logHandle);
			m->pConfig->pHomeDomain = NULL;
		}

		if (m->pConfig->pPassword != NULL)
		{
			LIMS_FREE((void **)&m->pConfig->pPassword, m->logHandle);
			m->pConfig->pPassword = NULL;
		}

		if (m->pConfig->pPrivateIdentity != NULL)
		{
			LIMS_FREE((void **)&m->pConfig->pPrivateIdentity, m->logHandle);
			m->pConfig->pPrivateIdentity = NULL;
		}

		if (m->pConfig->pUserAgent != NULL)
		{
			LIMS_FREE((void **)&m->pConfig->pUserAgent, m->logHandle);
			m->pConfig->pUserAgent = NULL;
		}

		if (m->pConfig->pDeviceId != NULL)
		{
			LIMS_FREE((void **)&m->pConfig->pDeviceId, m->logHandle);
			m->pConfig->pDeviceId = NULL;
		}

		if (m->pConfig->pDisplayName != NULL)
		{
			LIMS_FREE((void **)&m->pConfig->pDisplayName, m->logHandle);
			m->pConfig->pDisplayName = NULL;
		}

		if (m->pConfig->pRelayServerIP != NULL)
		{
			LIMS_FREE((void **)&m->pConfig->pRelayServerIP, m->logHandle);
			m->pConfig->pRelayServerIP = NULL;
		}

		/*if (m->pConfig->cpm.pUsername != NULL)
		{
			LIMS_FREE((void **)&m->pConfig->cpm.pUsername, m->logHandle);
			m->pConfig->cpm.pUsername = NULL;
		}

		if (m->pConfig->cpm.pDeviceId != NULL)
		{
			LIMS_FREE((void **)&m->pConfig->cpm.pDeviceId, m->logHandle);
			m->pConfig->cpm.pDeviceId = NULL;
		}

		if (m->pConfig->cpm.pDisplayName != NULL)
		{
			LIMS_FREE((void **)&m->pConfig->cpm.pDisplayName, m->logHandle);
			m->pConfig->cpm.pDisplayName = NULL;
		}*/


		if (m->pConfig->regModel.pRegSharedData != NULL)
		{
			LIMS_FREE((void **)&m->pConfig->regModel.pRegSharedData, m->logHandle);
			m->pConfig->regModel.pRegSharedData = NULL;
		}

		if (m->pConfig->pPANI != NULL)
		{
			LIMS_FREE((void **)&m->pConfig->pPANI, m->logHandle);
			m->pConfig->pPANI = NULL;
		}

		if (m->pConfig->pSecurityVerify != NULL)
		{
			LIMS_FREE((void **)&m->pConfig->pSecurityVerify, m->logHandle);
			m->pConfig->pSecurityVerify = NULL;
		}

		if (m->pConfig->pAssociatedUri != NULL)
		{
			LIMS_FREE((void **)&m->pConfig->pAssociatedUri, m->logHandle);
			m->pConfig->pAssociatedUri = NULL;
		}

		if (m->pConfig->pUriUserPart != NULL)
		{
			LIMS_FREE((void **)&m->pConfig->pUriUserPart, m->logHandle);
			m->pConfig->pUriUserPart = NULL;
		}

		if (m->pConfig->pTLSCertificate != NULL)
		{
			LIMS_FREE((void **)&m->pConfig->pTLSCertificate, m->logHandle);
			m->pConfig->pTLSCertificate = NULL;
		}

		LIMS_FREE((void **)&m->pConfig, m->logHandle);
		m->pConfig = NULL;
	}

	if (m->pConnection != NULL)
	{
		/* Free all remote Ip addresses memory allocation. */
		for (counter = 0; counter < m->pConnection->uNoOfRemoteIps; counter++)
		{
			if (m->pConnection->ppRemoteIPs[counter] != NULL)
			{
				LIMS_FREE((void **)&m->pConnection->ppRemoteIPs[counter], m->logHandle);
				m->pConnection->ppRemoteIPs[counter] = NULL;
			}
		}

		if (m->pConnection->ppRemoteIPs != NULL)
		{
			LIMS_FREE((void **)&m->pConnection->ppRemoteIPs, m->logHandle);
			m->pConnection->ppRemoteIPs = NULL;
		}
		m->pConnection->uNoOfRemoteIps = 0;

		/* Free local IP address */
		if (m->pConnection->pLocalIp != NULL)
		{
			LIMS_FREE((void **)&m->pConnection->pLocalIp, m->logHandle);
			m->pConnection->pLocalIp = NULL;
		}

		LIMS_FREE((void **)&m->pConnection, m->logHandle);
		m->pConnection = NULL;
	}

	if ((m->uSupportedMethodsCount > 0) && (m->ppSupportedMethods != NULL))
	{
		for (counter = 0; counter < m->uSupportedMethodsCount; counter++)
		{
			LIMS_FREE((void **)&m->ppSupportedMethods[counter], m->logHandle);
			m->ppSupportedMethods[counter] = NULL;
		}

		LIMS_FREE((void **)&m->ppSupportedMethods, m->logHandle);
		m->ppSupportedMethods = NULL;
	}

	lims_cleanUpFeatureTags(m);

	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);
	return uLimsError;
}

/* Function is used to clean up the memory allocated for feature tag in module structure. */
void lims_cleanUpFeatureTags
(
	lims_moduleStruct *m
)
{
	u_int16 uCount = 0;

	if (m->uFeatureTagCount > 0)
	{
		if (m->ppFeatureTagStruct != NULL)
		{
			for (uCount = 0; uCount < m->uFeatureTagCount; uCount++)
			{
				if (m->ppFeatureTagStruct[uCount] != NULL)
				{
					if (m->ppFeatureTagStruct[uCount]->pParamName != NULL)
					{
						LIMS_FREE((void **)&m->ppFeatureTagStruct[uCount]->pParamName, m->logHandle);
						m->ppFeatureTagStruct[uCount]->pParamName = NULL;
					}

					if (m->ppFeatureTagStruct[uCount]->pParamValue != NULL)
					{
						LIMS_FREE((void **)&m->ppFeatureTagStruct[uCount]->pParamValue, m->logHandle);
						m->ppFeatureTagStruct[uCount]->pParamValue = NULL;
					}

					LIMS_FREE((void **)&m->ppFeatureTagStruct[uCount], m->logHandle);
					m->ppFeatureTagStruct[uCount] = NULL;
				}
			}

			LIMS_FREE((void **)&m->ppFeatureTagStruct, m->logHandle);
			m->ppFeatureTagStruct = NULL;
		}

		m->uFeatureTagCount = 0;
	}
}

static void ec_AddSameFeatureTag(lims_moduleStruct *m, u_char* pSource, u_int32 uSrcLen, const u_char *pValue)
{
	u_int32 uSize = 0;
	u_char *pEnd = NULL;
	(void)m;
	
	uSize = pal_StringLength(pSource);
	pEnd = (uSize > 1) ? (pSource + uSize - 1) : pSource;

	if (*pEnd == '"')
	{
		*pEnd = ',';
		pEnd++;
		pValue++;
	}

	if (NULL == pal_StringNConcatenate(pEnd, uSrcLen, pValue, pal_StringLength(pValue)))
	{
		return;
	}

	return;
}

/**
* This function is used to clean up the memory allocated for feature tags in module structure and
* existing feature tags and copy the new ones into module structure.
*/
u_int32 lims_cleanUpAndCopyFeatureTags
(
	lims_moduleStruct *m,
	u_int32 uFeatures
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
	u_int16 uCount = 0;
	u_int32 uOOMError = KOOMErrorNone;
	u_char *pOtherFeatureTag = NULL;
	u_char *pVolteFeatureTag = NULL;
	u_int32 uSize = 0;
	u_char *pICSIRef = NULL;
	u_char *pIARIRef = NULL;

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/* find out what features are set */
	BoolEnum bSms = uFeatures & lims_Feature_IMS_Sms ? Enum_TRUE : Enum_FALSE;
	BoolEnum bVolte = uFeatures & lims_Feature_IMS_VoLTE ? Enum_TRUE : Enum_FALSE;
	BoolEnum bPager = uFeatures & lims_Feature_CPM_PagerMode ? Enum_TRUE : Enum_FALSE;
	BoolEnum bLarge = uFeatures & lims_Feature_CPM_LargeMode ? Enum_TRUE : Enum_FALSE;
	BoolEnum bChat = uFeatures & lims_Feature_CPM_Chat ? Enum_TRUE : Enum_FALSE;
	BoolEnum bFT = uFeatures & lims_Feature_CPM_FT_Http ? Enum_TRUE : Enum_FALSE;
	BoolEnum bChatbot = uFeatures & lims_Feature_CPM_Chatbot ? Enum_TRUE : Enum_FALSE;
	BoolEnum bIMDNAgreegation = uFeatures & lims_Feature_CPM_IMDN_Aggregation ? Enum_TRUE : Enum_FALSE;
	BoolEnum bGeoPush = uFeatures & lims_Feature_CPM_GeoLocation ? Enum_TRUE : Enum_FALSE;

	if ((bSms == Enum_TRUE) && (m->bSmsFeature == Enum_FALSE))
	{
		LIMSLOGE(m->logHandle, KLogTypeFuncEntry, "%s:%u SMS feature disabled. ", __FUNCTION__, __LINE__);
		return LIMS_SMS_FEATURE_NOT_SUPPORTED;
	}

	if ((bVolte == Enum_TRUE) && (m->bMoIPFeature == Enum_FALSE))
	{
		LIMSLOGE(m->logHandle, KLogTypeFuncEntry, "%s:%u MoIP feature disabled. ", __FUNCTION__, __LINE__);
		return LIMS_MOIP_FEATURE_NOT_SUPPORTED;
	}

	if ((bPager == Enum_TRUE || bLarge == Enum_TRUE || bFT == Enum_TRUE || bChat == Enum_TRUE || bChatbot == Enum_TRUE ) && (m->bCpmFeature == Enum_FALSE))
	{
		LIMSLOGE(m->logHandle, KLogTypeFuncEntry, "%s:%u CPM features disabled. ", __FUNCTION__, __LINE__);
		return LIMS_CPM_FEATURE_NOT_SUPPORTED;
	}

	/* check if the feature tags are already configured in the module structure.
	If yes clean up the existing ones and copy the new ones. */
	lims_cleanUpFeatureTags(m);
	m->uFeatures = uFeatures;

	if (bSms == Enum_TRUE)
		uCount++;	//for +g.3gpp.smsip feature tag

	if (bVolte == Enum_TRUE || bPager == Enum_TRUE || bLarge == Enum_TRUE || bChat == Enum_TRUE)
	{
		uCount++;			//for +g.3gpp.icsi-ref values, separated by comma
		LIMS_ALLOC(LIMS_COMMON_BUFFER_SIZE, (void **)&pICSIRef, uLimsError, ERR_MemoryFail, m->logHandle);
		pal_MemorySet(pICSIRef, 0, LIMS_COMMON_BUFFER_SIZE);
	}
		

	if (bChatbot == Enum_TRUE || bFT == Enum_TRUE || bGeoPush == Enum_TRUE)
	{
		uCount++; //extra for +g.3gpp.iari-ref values, separated by comma
		LIMS_ALLOC(LIMS_COMMON_BUFFER_SIZE, (void **)&pIARIRef, uLimsError, ERR_MemoryFail, m->logHandle);
		pal_MemorySet(pIARIRef, 0, LIMS_COMMON_BUFFER_SIZE);
	}
		

	if (bChatbot)
		uCount += 2;	//bot version and isbot

	if (bIMDNAgreegation)
		uCount += 1;	//IMDN Agreegation

	/* uCount is the number of feature tag which application wants to support.
	    There are some standard feature tags which is required by default as per IR.92 and 3GPP TS 24.229.*/
	m->uFeatureTagCount = uCount + 1;	// extra for +sip.instance feature tag.

	/* Invoke OOM function to get the other customized feature codes for register request. */
	if (m->pConfig->pOOMObject != NULL)
	{
		uSize = 8;
other_loop:
		/* allocate the memory for requested size. */
		LIMS_ALLOC(uSize + 1, (void **)&pOtherFeatureTag, uLimsError, ERR_MemoryFail, m->logHandle);
		uOOMError = m->pConfig->pOOMObject->ec_ooms_GetOtherFeatureTags(EcrioSipMessageTypeRegister, EcrioSipHeaderTypeContact, pOtherFeatureTag, &uSize);
		if (uOOMError == KOOMErrorNone)
		{
			if (uSize > 0)
			{
				m->uFeatureTagCount = m->uFeatureTagCount + 1;
			}
			else
			{
				// there is no other customized feature tags. free up the initially allocated for pOtherFeatureTag
				LIMS_FREE((void **)&pOtherFeatureTag, m->logHandle);
			}
		}
		else if (uOOMError == KOOMInvalidSize)
		{
			/* free memory and allocate the required size and invoke the callback again */
			LIMS_FREE((void **)&pOtherFeatureTag, m->logHandle);
			goto other_loop;
		}

		// next find is there any customized volte feature tags....
		uSize = 16;
volte_loop:
		/* allocate the memory for requested size. */
		LIMS_ALLOC(uSize + 1, (void **)&pVolteFeatureTag, uLimsError, ERR_MemoryFail, m->logHandle);
		uOOMError = m->pConfig->pOOMObject->ec_oom_GetVolteFeatureTags(EcrioSipMessageTypeRegister, EcrioSipHeaderTypeContact, pVolteFeatureTag, &uSize);
		if (uOOMError == KOOMErrorNone)
		{
			if (uSize > 0)
			{
				m->uFeatureTagCount = m->uFeatureTagCount + 1;
			}
			else
			{
				// there is no volte customized feature tags. free up the initially allocated for pVolteFeatureTag
				LIMS_FREE((void **)&pVolteFeatureTag, m->logHandle);
			}
		}
		else if (uOOMError == KOOMInvalidSize)
		{
			/* free memory and allocate the required size and invoke the callback again */
			LIMS_FREE((void **)&pVolteFeatureTag, m->logHandle);
			goto volte_loop;
		}
	}

	/* Allocate the memory*/
	LIMS_ALLOC(m->uFeatureTagCount * sizeof(EcrioSUESipParamStruct *), (void **)&m->ppFeatureTagStruct, uLimsError, ERR_MemoryFail, m->logHandle);
	if (m->ppFeatureTagStruct != NULL)
	{
		/* Allocate the memory for first feature tag which is default one. Here it is sip.instance */
		uCount = 0;
		LIMS_ALLOC(sizeof(EcrioSUESipParamStruct), (void **)&m->ppFeatureTagStruct[uCount], uLimsError, ERR_MemoryFail, m->logHandle);
		if (m->ppFeatureTagStruct[uCount] != NULL)
		{
			u_char instanceValue[64];
			if (0 >= pal_SNumPrintf((char *)instanceValue, 64, LIMS_SIP_INSTANCE_FEATURE_TAG_VALUE_STRING, m->pConfig->pDeviceId))
			{
				uLimsError = LIMS_NO_MEMORY;
				LIMSLOGE(m->logHandle, KLogTypeMemory, "%s:%u\t String copy error.",
					__FUNCTION__, __LINE__);
				goto ERR_MemoryFail;
			}
			m->ppFeatureTagStruct[uCount]->pParamName = pal_StringCreate(LIMS_SIP_INSTANCE_FEATURE_TAG_STRING, pal_StringLength(LIMS_SIP_INSTANCE_FEATURE_TAG_STRING));
			m->ppFeatureTagStruct[uCount]->pParamValue = pal_StringCreate(instanceValue, pal_StringLength(instanceValue));
		}

		/* If SMS feature is set. */
		if (bSms)
		{
			LIMS_ALLOC(sizeof(EcrioSUESipParamStruct), (void **)&m->ppFeatureTagStruct[++uCount], uLimsError, ERR_MemoryFail, m->logHandle);
			if (m->ppFeatureTagStruct[uCount] != NULL)
			{
				m->ppFeatureTagStruct[uCount]->pParamName = pal_StringCreate(LIMS_SMS_SMSIP_FEATURE_TAG_STRING, pal_StringLength(LIMS_SMS_SMSIP_FEATURE_TAG_STRING));
				m->ppFeatureTagStruct[uCount]->pParamValue = NULL;
			}
		}

		/* Populate +g.3gpp.icsi-ref for VoLTE feature. */
		if (bVolte)
		{
			ec_AddSameFeatureTag(m, pICSIRef, LIMS_COMMON_BUFFER_SIZE - pal_StringLength(pICSIRef), LIMS_3GPP_ICSI_MMTEL_FEATURE_TAG_VALUE_STRING);
			/*LIMS_ALLOC(sizeof(EcrioSUESipParamStruct), (void **)&m->ppFeatureTagStruct[++uCount], uLimsError, ERR_MemoryFail, m->logHandle);
			if (m->ppFeatureTagStruct[uCount] != NULL)
			{
				m->ppFeatureTagStruct[uCount]->pParamName = pal_StringCreate(LIMS_3GPP_ICSI_FEATURE_TAG_STRING, pal_StringLength(LIMS_3GPP_ICSI_FEATURE_TAG_STRING));
				m->ppFeatureTagStruct[uCount]->pParamValue = pal_StringCreate(LIMS_3GPP_ICSI_MMTEL_FEATURE_TAG_VALUE_STRING, pal_StringLength(LIMS_3GPP_ICSI_MMTEL_FEATURE_TAG_VALUE_STRING));
			}*/
		}

		/* If Pager Mode Message feature is set. */
		if (bPager)
		{
			ec_AddSameFeatureTag(m, pICSIRef, LIMS_COMMON_BUFFER_SIZE - pal_StringLength(pICSIRef), LIMS_3GPP_ICSI_PAGER_MODE_FEATURE_TAG_VALUE_STRING);
		}

		/* If Large Message Mode feature is set. */
		if (bLarge)
		{
			ec_AddSameFeatureTag(m, pICSIRef, LIMS_COMMON_BUFFER_SIZE - pal_StringLength(pICSIRef), LIMS_3GPP_ICSI_LARGE_MODE_FEATURE_TAG_VALUE_STRING);
			//ec_AddSameFeatureTag(m, pICSIRef, LIMS_3GPP_ICSI_DEFERRED_FEATURE_TAG_VALUE_STRING);
		}

		/* If Chat feature is set. */
		if (bChat)
		{
			ec_AddSameFeatureTag(m, pICSIRef, LIMS_COMMON_BUFFER_SIZE - pal_StringLength(pICSIRef), LIMS_3GPP_ICSI_CPM_SESSION_FEATURE_TAG_VALUE_STRING);
			//ec_AddSameFeatureTag(m, pIARIRef, LIMS_3GPP_IARI_CPM_CHAT_FEATURE_TAG_VALUE_STRING);
		}

		// If File Transfer feature set
		if (bFT)
		{
			ec_AddSameFeatureTag(m, pIARIRef, LIMS_COMMON_BUFFER_SIZE - pal_StringLength(pIARIRef), LIMS_3GPP_IARI_FT_FEATURE_TAG_VALUE_STRING);
		}

		// If Chatbot feature set
		if (bChatbot)
		{
			ec_AddSameFeatureTag(m, pIARIRef, LIMS_COMMON_BUFFER_SIZE - pal_StringLength(pIARIRef), LIMS_3GPP_IARI_CHATBOT_FEATURE_TAG_VALUE_STRING);
			LIMS_ALLOC(sizeof(EcrioSUESipParamStruct), (void **)&m->ppFeatureTagStruct[++uCount], uLimsError, ERR_MemoryFail, m->logHandle);
			if (m->ppFeatureTagStruct[uCount] != NULL)
			{
				m->ppFeatureTagStruct[uCount]->pParamName = pal_StringCreate(LIMS_3GPP_CHATBOT_VERSION_FEATURE_TAG_STRING, pal_StringLength(LIMS_3GPP_CHATBOT_VERSION_FEATURE_TAG_STRING));
				m->ppFeatureTagStruct[uCount]->pParamValue = pal_StringCreate((const u_char*)"\"#=1\"", pal_StringLength((const u_char*)"\"#=1\""));
			}

			LIMS_ALLOC(sizeof(EcrioSUESipParamStruct), (void **)&m->ppFeatureTagStruct[++uCount], uLimsError, ERR_MemoryFail, m->logHandle);
			if (m->ppFeatureTagStruct[uCount] != NULL)
			{
				m->ppFeatureTagStruct[uCount]->pParamName = pal_StringCreate(LIMS_3GPP_ISCHATBOT_FEATURE_TAG_STRING, pal_StringLength(LIMS_3GPP_ISCHATBOT_FEATURE_TAG_STRING));
				m->ppFeatureTagStruct[uCount]->pParamValue = NULL;
			}
		}

		if (bGeoPush)
		{
			ec_AddSameFeatureTag(m, pIARIRef, LIMS_COMMON_BUFFER_SIZE - pal_StringLength(pIARIRef), LIMS_3GPP_IARI_GEOPUSH_FEATURE_TAG_VALUE_STRING);
		}
		if (pVolteFeatureTag != NULL)
		{
			LIMS_ALLOC(sizeof(EcrioSUESipParamStruct), (void **)&m->ppFeatureTagStruct[++uCount], uLimsError, ERR_MemoryFail, m->logHandle);
			if (m->ppFeatureTagStruct[uCount] != NULL)
			{
				m->ppFeatureTagStruct[uCount]->pParamName = pal_StringCreate(pVolteFeatureTag, pal_StringLength(pVolteFeatureTag));
				m->ppFeatureTagStruct[uCount]->pParamValue = NULL;
			}
		}

		if (pICSIRef != NULL && *pICSIRef != '\0')
		{
			LIMS_ALLOC(sizeof(EcrioSUESipParamStruct), (void **)&m->ppFeatureTagStruct[++uCount], uLimsError, ERR_MemoryFail, m->logHandle);
			if (m->ppFeatureTagStruct[uCount] != NULL)
			{
				m->ppFeatureTagStruct[uCount]->pParamName = pal_StringCreate(LIMS_3GPP_ICSI_FEATURE_TAG_STRING, pal_StringLength(LIMS_3GPP_ICSI_FEATURE_TAG_STRING));
				m->ppFeatureTagStruct[uCount]->pParamValue = pal_StringCreate(pICSIRef, pal_StringLength(pICSIRef));		
			}

			LIMS_FREE((void **)&pICSIRef, m->logHandle);
		}

		if (pIARIRef != NULL && *pIARIRef != '\0')
		{
			LIMS_ALLOC(sizeof(EcrioSUESipParamStruct), (void **)&m->ppFeatureTagStruct[++uCount], uLimsError, ERR_MemoryFail, m->logHandle);
			if (m->ppFeatureTagStruct[uCount] != NULL)
			{
				m->ppFeatureTagStruct[uCount]->pParamName = pal_StringCreate(LIMS_3GPP_IARI_FEATURE_TAG_STRING, pal_StringLength(LIMS_3GPP_IARI_FEATURE_TAG_STRING));
				m->ppFeatureTagStruct[uCount]->pParamValue = pal_StringCreate(pIARIRef, pal_StringLength(pIARIRef));
			}

			LIMS_FREE((void **)&pIARIRef, m->logHandle);
		}

		// If other feature tags are present just add it here..
		if (pOtherFeatureTag != NULL)
		{
			LIMS_ALLOC(sizeof(EcrioSUESipParamStruct), (void **)&m->ppFeatureTagStruct[++uCount], uLimsError, ERR_MemoryFail, m->logHandle);
			if (m->ppFeatureTagStruct[uCount] != NULL)
			{
				m->ppFeatureTagStruct[uCount]->pParamName = pal_StringCreate(pOtherFeatureTag, pal_StringLength(pOtherFeatureTag));
				m->ppFeatureTagStruct[uCount]->pParamValue = NULL;
			}
		}

		// Add IMDN Agreegation
		if (bIMDNAgreegation)
		{
			LIMS_ALLOC(sizeof(EcrioSUESipParamStruct), (void **)&m->ppFeatureTagStruct[++uCount], uLimsError, ERR_MemoryFail, m->logHandle);
			if (m->ppFeatureTagStruct[uCount] != NULL)
			{
				m->ppFeatureTagStruct[uCount]->pParamName = pal_StringCreate(LIMS_IMDN_AGREEGATION_VALUE_STRING, pal_StringLength(LIMS_IMDN_AGREEGATION_VALUE_STRING));
				m->ppFeatureTagStruct[uCount]->pParamValue = NULL;
			}
		}
	}
	else
	{
		LIMSLOGE(m->logHandle, KLogTypeMemory, "%s:%u\t insufficient memory to allocate m->ppFeatureTagStruct: memory alloc size=%d",
			__FUNCTION__, __LINE__, m->uFeatureTagCount * sizeof(EcrioSUESipParamStruct *));
		goto ERR_MemoryFail;
	}

	goto ERR_None;

ERR_MemoryFail:
	lims_cleanUpFeatureTags(m);

	if (pICSIRef)
	{
		LIMS_FREE((void **)&pICSIRef, m->logHandle);
	}

	if (pIARIRef)
	{
		LIMS_FREE((void **)&pIARIRef, m->logHandle);
	}
		
	uLimsError = LIMS_NO_MEMORY;

ERR_None:

	/* free pOtherFeatureTag pointer if it is not NULL..*/
	if (pOtherFeatureTag != NULL)
	{
		LIMS_FREE((void **)&pOtherFeatureTag, m->logHandle);
	}

	/* free pVolteFeatureTag pointer if it is not NULL.. */
	if (pVolteFeatureTag != NULL)
	{
		LIMS_FREE((void **)&pVolteFeatureTag, m->logHandle);
	}

	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);
	return uLimsError;
}

/**
*
*/
u_int32 lims_getAndCopyAllowMethods
(
	lims_moduleStruct *m,
	u_int32 uMethods
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
	char **pptmpSupportedMethods = NULL;
	u_int16 uCount = 0;

	BoolEnum bUpdate = uMethods & EcrioSipMessageTypeUpdate ? Enum_TRUE : Enum_FALSE;
	BoolEnum bPrack = uMethods & EcrioSipMessageTypePrack ? Enum_TRUE : Enum_FALSE;
	BoolEnum bInvite = uMethods & EcrioSipMessageTypeInvite ? Enum_TRUE : Enum_FALSE;
	BoolEnum bNotify = uMethods & EcrioSipMessageTypeNotify ? Enum_TRUE : Enum_FALSE;
	BoolEnum bAck = uMethods & EcrioSipMessageTypeAck ? Enum_TRUE : Enum_FALSE;
	BoolEnum bBye = uMethods & EcrioSipMessageTypeBye ? Enum_TRUE : Enum_FALSE;
	BoolEnum bCancel = uMethods & EcrioSipMessageTypeCancel ? Enum_TRUE : Enum_FALSE;
	BoolEnum bMessage = uMethods & EcrioSipMessageTypeMessage ? Enum_TRUE : Enum_FALSE;
	BoolEnum bOptions = uMethods & EcrioSipMessageTypeOptions ? Enum_TRUE : Enum_FALSE;

	/* find out supported method count. BK way */
	for (uCount = 0; uMethods; uCount++)
	{
		uMethods &= uMethods - 1;	// clear the least significant bit set
	}

	m->uSupportedMethodsCount = uCount;
	LIMS_ALLOC(sizeof(u_char *) * m->uSupportedMethodsCount, (void **)&pptmpSupportedMethods, uLimsError, ERR_Fail, m->logHandle);
	uCount = 0;

	/* If bUpdate is Enum_TRUE, add the UPDATE method . */
	if (bUpdate)
	{
		LIMS_ALLOC_AND_COPY_STRING("UPDATE", pptmpSupportedMethods[uCount++], uLimsError, ERR_Fail, m->logHandle);
	}

	/* If bPrack is Enum_TRUE, add the PRACK method . */
	if (bPrack)
	{
		LIMS_ALLOC_AND_COPY_STRING("PRACK", pptmpSupportedMethods[uCount++], uLimsError, ERR_Fail, m->logHandle);
	}

	/* If bInvite is Enum_TRUE, add the INVITE method . */
	if (bInvite)
	{
		LIMS_ALLOC_AND_COPY_STRING("INVITE", pptmpSupportedMethods[uCount++], uLimsError, ERR_Fail, m->logHandle);
	}

	/* If bNotify is Enum_TRUE, add the NOTIFY method . */
	if (bNotify)
	{
		LIMS_ALLOC_AND_COPY_STRING("NOTIFY", pptmpSupportedMethods[uCount++], uLimsError, ERR_Fail, m->logHandle);
	}

	/* If bAck is Enum_TRUE, add the ACK method . */
	if (bAck)
	{
		LIMS_ALLOC_AND_COPY_STRING("ACK", pptmpSupportedMethods[uCount++], uLimsError, ERR_Fail, m->logHandle);
	}

	/* If bBye is Enum_TRUE, add the BYE method . */
	if (bBye)
	{
		LIMS_ALLOC_AND_COPY_STRING("BYE", pptmpSupportedMethods[uCount++], uLimsError, ERR_Fail, m->logHandle);
	}

	/* If bCancel is Enum_TRUE, add the CANCEL method . */
	if (bCancel)
	{
		LIMS_ALLOC_AND_COPY_STRING("CANCEL", pptmpSupportedMethods[uCount++], uLimsError, ERR_Fail, m->logHandle);
	}

	/* If bMessage is Enum_TRUE, add the CANCEL method . */
	if (bMessage)
	{
		LIMS_ALLOC_AND_COPY_STRING("MESSAGE", pptmpSupportedMethods[uCount++], uLimsError, ERR_Fail, m->logHandle);
	}

	/* If bOptions is Enum_TRUE, add the OPTIONS method . */
	if (bOptions)
	{
		LIMS_ALLOC_AND_COPY_STRING("OPTIONS", pptmpSupportedMethods[uCount++], uLimsError, ERR_Fail, m->logHandle);
	}


	m->ppSupportedMethods = (u_char **)pptmpSupportedMethods;
	pptmpSupportedMethods = NULL;
	goto ERR_None;

ERR_Fail:
	if (pptmpSupportedMethods != NULL)
	{
		for (uCount = 0; uCount < m->uSupportedMethodsCount; uCount++)
		{
			if (pptmpSupportedMethods[uCount] != NULL)
			{
				LIMS_FREE((void **)&pptmpSupportedMethods[uCount], m->logHandle);
			}
		}

		LIMS_FREE((void **)&pptmpSupportedMethods, m->logHandle);
	}

ERR_None:
	return uLimsError;
}

/**
* This function is used to clean up the connection detail structure and copy
* the new connection details received from the caller.
*/
u_int32 lims_cleanUpAndCopyConnectionDetails
(
	lims_moduleStruct *m,
	lims_NetworkConnectionStruct *pConnectionDetails
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
	u_int32 counter = 0;

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (m->pConnection != NULL)
	{
		// only update the IP addresses which got modified otherwise just keep the old ones in the structure..
		/* Free all remote Ip addresses memory allocation. */
		for (counter = 0; counter < m->pConnection->uNoOfRemoteIps; counter++)
		{
			if (m->pConnection->ppRemoteIPs[counter] != NULL)
			{
				LIMS_FREE((void **)&m->pConnection->ppRemoteIPs[counter], m->logHandle);
				m->pConnection->ppRemoteIPs[counter] = NULL;
			}
		}

		if (m->pConnection->ppRemoteIPs != NULL)
		{
			LIMS_FREE((void **)&m->pConnection->ppRemoteIPs, m->logHandle);
			m->pConnection->ppRemoteIPs = NULL;
		}
        m->pConnection->uNoOfRemoteIps = 0;

		/* Free local IP address */
		if (m->pConnection->pLocalIp != NULL)
		{
			LIMS_FREE((void **)&m->pConnection->pLocalIp, m->logHandle);
			m->pConnection->pLocalIp = NULL;
		}

		/* Free proxy IP address */
		if (m->pConnection->pProxyRouteAddress != NULL)
		{
			LIMS_FREE((void **)&m->pConnection->pProxyRouteAddress, m->logHandle);
			m->pConnection->pProxyRouteAddress = NULL;
		}

		/* Copy the local Ip */
		LIMS_ALLOC_AND_COPY_STRING(pConnectionDetails->pLocalIp, m->pConnection->pLocalIp, uLimsError, ERR_Fail, m->logHandle);

		/* Copy the remote Ips */
		LIMS_ALLOC(sizeof(u_char *) * pConnectionDetails->uNoOfRemoteIps, (void **)&m->pConnection->ppRemoteIPs, uLimsError, ERR_Fail, m->logHandle);
		m->pConnection->uNoOfRemoteIps = pConnectionDetails->uNoOfRemoteIps;

		for (counter = 0; counter < pConnectionDetails->uNoOfRemoteIps; counter++)
		{
			LIMS_ALLOC_AND_COPY_STRING(pConnectionDetails->ppRemoteIPs[counter], m->pConnection->ppRemoteIPs[counter], uLimsError, ERR_Fail, m->logHandle);
		}

		m->pConnection->uRemotePort = pConnectionDetails->uRemotePort;
		m->pConnection->uRemoteTLSPort = pConnectionDetails->uRemoteTLSPort;
		m->pConnection->uRemoteClientPort = pConnectionDetails->uRemoteClientPort;
		m->pConnection->uLocalPort = pConnectionDetails->uLocalPort;
		m->pConnection->uStatus = pConnectionDetails->uStatus;
		m->pConnection->eIPType = pConnectionDetails->eIPType;
	}
	else
	{
		LIMS_ALLOC(sizeof(lims_NetworkConnectionStruct), (void **)&m->pConnection, uLimsError, ERR_Fail, m->logHandle);
		/* Copy the local Ip */
		LIMS_ALLOC_AND_COPY_STRING(pConnectionDetails->pLocalIp, m->pConnection->pLocalIp, uLimsError, ERR_Fail, m->logHandle);

		/* Copy the remote Ips */
		if (pConnectionDetails->uNoOfRemoteIps > 0)
		{
			LIMS_ALLOC(sizeof(u_char *) * pConnectionDetails->uNoOfRemoteIps, (void **)&m->pConnection->ppRemoteIPs, uLimsError, ERR_Fail, m->logHandle);
			m->pConnection->uNoOfRemoteIps = pConnectionDetails->uNoOfRemoteIps;

			for (counter = 0; counter < pConnectionDetails->uNoOfRemoteIps; counter++)
			{
				LIMS_ALLOC_AND_COPY_STRING(pConnectionDetails->ppRemoteIPs[counter], m->pConnection->ppRemoteIPs[counter], uLimsError, ERR_Fail, m->logHandle);
			}
		}

		m->pConnection->uRemotePort = pConnectionDetails->uRemotePort;
		m->pConnection->uRemoteTLSPort = pConnectionDetails->uRemoteTLSPort;
		m->pConnection->uRemoteClientPort = pConnectionDetails->uRemoteClientPort;
		m->pConnection->uLocalPort = pConnectionDetails->uLocalPort;
		m->pConnection->uStatus = pConnectionDetails->uStatus;
		m->pConnection->eIPType = pConnectionDetails->eIPType;
	}

	/* Copy Proxy route info */
	m->pConnection->isProxyRouteEnabled = pConnectionDetails->isProxyRouteEnabled;
	if(m->pConnection->isProxyRouteEnabled == Enum_TRUE) {
		LIMS_ALLOC_AND_COPY_STRING(pConnectionDetails->pProxyRouteAddress,
								   m->pConnection->pProxyRouteAddress, uLimsError, ERR_Fail,
								   m->logHandle);
		m->pConnection->uProxyRoutePort = pConnectionDetails->uProxyRoutePort;
	} else{
		m->pConnection->pProxyRouteAddress = NULL;
		m->pConnection->uProxyRoutePort = 0;
	}

ERR_Fail:
	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);
	return uLimsError;
}

#ifdef ENABLE_QCMAPI
u_int32 lims_InitInternal
(
	lims_moduleStruct *m,
	lims_configInternalStruct *pStruct
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	LIMSLOGI(m->logHandle, KLogTypeFuncParams,
		"%s:%u\tlims_moduleStruct = %x, pStruct=%p",
		__FUNCTION__, __LINE__, m, pStruct);

	if (pStruct->pSipPublicUserId == NULL || pStruct->pSipHomeDomain == NULL || pStruct->pSipPrivateUserId == NULL)
	{
		return LIMS_INVALID_PARAMETER1;
	}

	if (m->pConfig->pPublicIdentity != NULL)
	{
		LIMS_FREE((void **)&m->pConfig->pPublicIdentity, m->logHandle);
		m->pConfig->pPublicIdentity = NULL;
	}
	LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->pSipPublicUserId=%s", __FUNCTION__, __LINE__, pStruct->pSipPublicUserId);
	LIMS_ALLOC_AND_COPY_STRING(pStruct->pSipPublicUserId, m->pConfig->pPublicIdentity, uLimsError, ERR_None, m->logHandle);

	if (m->pConfig->pHomeDomain != NULL)
	{
		LIMS_FREE((void **)&m->pConfig->pHomeDomain, m->logHandle);
		m->pConfig->pHomeDomain = NULL;
	}
	LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->pSipHomeDomain=%s", __FUNCTION__, __LINE__, pStruct->pSipHomeDomain);
	LIMS_ALLOC_AND_COPY_STRING(pStruct->pSipHomeDomain, m->pConfig->pHomeDomain, uLimsError, ERR_None, m->logHandle);

	if (m->pConfig->pPrivateIdentity != NULL)
	{
		LIMS_FREE((void **)&m->pConfig->pPrivateIdentity, m->logHandle);
		m->pConfig->pPrivateIdentity = NULL;
	}
	LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->pSipPrivateUserId=%s", __FUNCTION__, __LINE__, pStruct->pSipPrivateUserId);
	LIMS_ALLOC_AND_COPY_STRING(pStruct->pSipPrivateUserId, m->pConfig->pPrivateIdentity, uLimsError, ERR_None, m->logHandle);

	if (m->pConfig->pPANI != NULL)
	{
		LIMS_FREE((void **)&m->pConfig->pPANI, m->logHandle);
		m->pConfig->pPANI = NULL;
	}
	if (pStruct->pPANI != NULL)
	{
		LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->pPANI=%s", __FUNCTION__, __LINE__, pStruct->pPANI);
		LIMS_ALLOC_AND_COPY_STRING(pStruct->pPANI, m->pConfig->pPANI, uLimsError, ERR_None, m->logHandle);
	}

	if (m->pConfig->pSecurityVerify != NULL)
	{
		LIMS_FREE((void **)&m->pConfig->pSecurityVerify, m->logHandle);
		m->pConfig->pSecurityVerify = NULL;
	}
	if (pStruct->pSecurityVerify != NULL)
	{
		LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->pSecurityVerify=%s", __FUNCTION__, __LINE__, pStruct->pSecurityVerify);
		LIMS_ALLOC_AND_COPY_STRING(pStruct->pSecurityVerify, m->pConfig->pSecurityVerify, uLimsError, ERR_None, m->logHandle);
	}
	if (m->pConfig->pAssociatedUri != NULL)
	{
		LIMS_FREE((void **)&m->pConfig->pAssociatedUri, m->logHandle);
		m->pConfig->pAssociatedUri = NULL;
	}
	if (pStruct->pAssociatedUri != NULL)
	{
		LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->pAssociatedUri=%s", __FUNCTION__, __LINE__, pStruct->pAssociatedUri);
		LIMS_ALLOC_AND_COPY_STRING(pStruct->pAssociatedUri, m->pConfig->pAssociatedUri, uLimsError, ERR_None, m->logHandle);
	}

	if (m->pConfig->pUriUserPart != NULL)
	{
		LIMS_FREE((void **)&m->pConfig->pUriUserPart, m->logHandle);
		m->pConfig->pUriUserPart = NULL;
	}
	if (pStruct->pUriUserPart != NULL)
	{
		LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->pUriUserPart=%s", __FUNCTION__, __LINE__, pStruct->pUriUserPart);
		LIMS_ALLOC_AND_COPY_STRING(pStruct->pUriUserPart, m->pConfig->pUriUserPart, uLimsError, ERR_None, m->logHandle);
	}

	if (m->pConfig->pDeviceId != NULL)
	{
		LIMS_FREE((void **)&m->pConfig->pDeviceId, m->logHandle);
		m->pConfig->pDeviceId = NULL;
	}
	if (pStruct->pIMEI!= NULL)
	{
		LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->pIMEI=%s", __FUNCTION__, __LINE__, pStruct->pIMEI);
		LIMS_ALLOC_AND_COPY_STRING(pStruct->pIMEI, m->pConfig->pDeviceId, uLimsError, ERR_None, m->logHandle);
	}

	/* Initialize the SigMgr module. */
	uLimsError = lims_initSigMgrModule(m);
	if (uLimsError != LIMS_NO_ERROR)
	{
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tlims_initSigMgrModule() failed with error code: uLimsError=%d",
			__FUNCTION__, __LINE__, uLimsError);
		goto ERR_Fail;
	}

	/** Initialize the SUE module. */
	uLimsError = lims_initSueModule(m);
	if (uLimsError != LIMS_NO_ERROR)
	{
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tlims_initSueEngine() failed with error code: uLimsError=%d",
			__FUNCTION__, __LINE__, uLimsError);
		goto ERR_Fail;
	}

#if defined(ENABLE_RCS)
	/** Initialize the CPM module.*/
	m->bCpmFeature = Enum_TRUE;
	uLimsError = lims_initCpmModule(m);
	if (uLimsError != LIMS_NO_ERROR)
	{
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tlims_initCpmModule() failed with error code: uLimsError=%d",
			__FUNCTION__, __LINE__, uLimsError);
		goto ERR_Fail;
	}
#endif	// ENABLE_RCS

	goto ERR_None;

ERR_Fail:
ERR_None:
	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);
	return uLimsError;
}

u_int32 lims_NetworkStateChangeInternal
(
	lims_moduleStruct *m,
	lims_NetworkPDNTypeEnum	ePdnType,
	lims_NetworkConnectionTypeEnum eConnectionType,
	lims_NetworkConnectionStruct *pConnectionDetails
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
	u_int32 uSueError = ECRIO_SUE_NO_ERROR;
	EcrioSUENetworkInfoStruct networkInfo = { 0 };

	// @note ePdnType is not used yet.
	(void)ePdnType;

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (m->moduleState == lims_Module_State_UNINITIALIZED || m->moduleState == lims_Module_State_INITIALIZING)
	{
		uLimsError = LIMS_INVALID_STATE;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tcurrent state is UNINITIALIZED or INITIALIZING",
			__FUNCTION__, __LINE__);
		goto ERR_None;
	}

	if (pConnectionDetails == NULL && m->moduleState == lims_Module_State_INITIALIZED)
	{
		uLimsError = LIMS_INVALID_STATE;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tcurrent state is UNINITIALIZED or INITIALIZING",
			__FUNCTION__, __LINE__);
		goto ERR_None;
	}

	/* Make a copy of pConnectionDetails*/
	if (pConnectionDetails != NULL)
	{
		uLimsError = lims_cleanUpAndCopyConnectionDetails(m, pConnectionDetails);
		if (uLimsError != LIMS_NO_ERROR)
		{
			LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tClean up existing connection details and copy new connection details failed with error code:%d",
				__FUNCTION__, __LINE__, uLimsError);
			goto ERR_None;
		}
	}

	switch (eConnectionType)
	{
		case lims_Network_Connection_Type_NONE:
		default:
		{
			uSueError = EcrioSUESetNetworkParam(m->pSueHandle, EcrioSUENetworkState_NotConnected, NULL);
			if (uSueError == ECRIO_SUE_NO_ERROR)
			{
				m->moduleState = lims_Module_State_INITIALIZED;
#ifdef ENABLE_LOG
				lims_moduleStateEnum prevState = m->moduleState;
				if (prevState == lims_Module_State_CONNECTED)
				{
					LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tChange State from CONNECTED to INITIALIZED", __FUNCTION__, __LINE__);
				}
				else if (prevState == lims_Module_State_REGISTERED)
				{
					LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tChange State from REGISTERED to INITIALIZED", __FUNCTION__, __LINE__);
				}
				else
				{
					LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tChange State from NOT CONNECTED to INITIALIZED", __FUNCTION__, __LINE__);
				}
#endif
			}
		}
		break;

		case lims_Network_Connection_Type_WIFI:
		case lims_Network_Connection_Type_LTE:
		{
			networkInfo.pLocalIp = (u_char *)m->pConnection->pLocalIp;
			networkInfo.ppRemoteIPs = (u_char **)m->pConnection->ppRemoteIPs;
			networkInfo.uLocalPort = m->pConnection->uLocalPort;
			networkInfo.uRemotePort = m->pConnection->uRemotePort;
			networkInfo.uRemoteClientPort = m->pConnection->uRemoteClientPort;
			networkInfo.uNoOfRemoteIps = m->pConnection->uNoOfRemoteIps;
			networkInfo.bIsIPv6 = (m->pConnection->eIPType == lims_Network_IP_Type_V6) ? Enum_TRUE : Enum_FALSE;
			switch (m->pConnection->uStatus)
			{
			case lims_Network_Status_Success:
				networkInfo.uStatus = ECRIO_SUE_Network_Status_Success;
				break;				
			case lims_Network_Status_VoLTE_Off:
				networkInfo.uStatus = ECRIO_SUE_Network_Status_VoLTE_Off;
				break;
			case lims_Network_Status_IPsec_Established:
				networkInfo.uStatus = ECRIO_SUE_Network_Status_IPsec_Established;
				break;
			case lims_Network_Status_IPsec_Failed:
				networkInfo.uStatus = ECRIO_SUE_Network_Status_IPsec_Failed;
				break;
			case lims_Netwotk_Status_IPsec_Disconneted:
				networkInfo.uStatus = ECRIO_SUE_Netwotk_Status_IPsec_Disconneted;
				break;
			case lims_Network_Status_IPsec_Lost:
				networkInfo.uStatus = ECRIO_SUE_Network_Status_IPsec_Lost;
				break;
			case lims_Network_Status_Dedicated_Bearer_Connected:
				networkInfo.uStatus = ECRIO_SUE_Network_Status_Dedicated_Bearer_Connected;
				break;
			case lims_Network_Status_Dedicated_Bearer_Disconnected:
				networkInfo.uStatus = ECRIO_SUE_Network_Status_Dedicated_Bearer_Disconnected;
				break;
			case lims_Network_Status_Dedicated_Bearer_Failed:
				networkInfo.uStatus = ECRIO_SUE_Network_Status_Dedicated_Bearer_Failed;
				break;
			}

			uSueError = EcrioSUESetNetworkParam(m->pSueHandle, EcrioSUENetworkState_LTEConnected, &networkInfo);
			if (uSueError == ECRIO_SUE_NO_ERROR)
			{
				if (m->moduleState != lims_Module_State_REGISTERED)
				{
					m->moduleState = lims_Module_State_CONNECTED;
					LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tChange State from INITIALIZED to CONNECTED", __FUNCTION__, __LINE__);
				}
			}
		}
		break;
	}

	if (uSueError != ECRIO_SUE_NO_ERROR)
	{
		if (uSueError == ECRIO_SUE_PAL_SOCKET_ERROR)
			uLimsError = LIMS_SOCKET_ERROR;
		else
			uLimsError = LIMS_SUE_SET_NETWORK_PARAM_ERROR;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tEcrioSUESetNetworkParam() failed with error code: uSueError=%d",
			__FUNCTION__, __LINE__, uSueError);
	}

ERR_None:
	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);

	return uLimsError;
}

u_int32 lims_updateSigMgrParameters
(
	lims_moduleStruct *m
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
	u_int32 uSigMgrError = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrSignalingInfoStruct sigMgrSignalingInfoStruct = { 0 };
	EcrioSigMgrCustomTimersStruct customTimer = { 0 };
	EcrioSipSessionTimerStruct timers = { 0 };

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	sigMgrSignalingInfoStruct.pPublicIdentity_imsi = NULL;	// For now we are using only one public Id. Assuming it is always MSISDN based
	sigMgrSignalingInfoStruct.pPublicIdentity_msisdn = (u_char *)m->pConfig->pPublicIdentity;
	sigMgrSignalingInfoStruct.pHomeDomain = (u_char *)m->pConfig->pHomeDomain;
	sigMgrSignalingInfoStruct.pPassword = (u_char *)m->pConfig->pPassword;
	sigMgrSignalingInfoStruct.pRegistrarDomain = (u_char *)m->pConfig->pHomeDomain;
	sigMgrSignalingInfoStruct.pUserAgent = (u_char *)m->pConfig->pUserAgent;
	sigMgrSignalingInfoStruct.pPrivateId = (u_char *)m->pConfig->pPrivateIdentity;
	sigMgrSignalingInfoStruct.bWildCharInContact = Enum_TRUE;
	sigMgrSignalingInfoStruct.registrationExpires = m->pConfig->uRegExpireInterval;
	sigMgrSignalingInfoStruct.subscriptionExpires = m->pConfig->uSubExpireInterval;
	sigMgrSignalingInfoStruct.publishExpires = m->pConfig->uPublishRefreshInterval;
	if (m->pConfig->uNoActivityTimerInterval > 0)
		sigMgrSignalingInfoStruct.bEnableNoActivityTimer = Enum_TRUE;
	else
		sigMgrSignalingInfoStruct.bEnableNoActivityTimer = Enum_FALSE;

	sigMgrSignalingInfoStruct.bEnableUdp = m->pConfig->bEnableUdp;
	sigMgrSignalingInfoStruct.bEnableTcp = m->pConfig->bEnableTcp;
	
	sigMgrSignalingInfoStruct.uNoActivityTimerInterval = m->pConfig->uNoActivityTimerInterval;
	/* get the session timers */
	m->pConfig->pOOMObject->ec_oom_GetSessionTimers(&timers);
	sigMgrSignalingInfoStruct.minSe = timers.uTimerMinSE;
	sigMgrSignalingInfoStruct.sessionExpires = timers.uTimerSE;/* seconds */
	if (m->pConfig->uMtuSize > 0)
		sigMgrSignalingInfoStruct.uUdpMtu = m->pConfig->uMtuSize;
	else
		sigMgrSignalingInfoStruct.uUdpMtu = 1300; // default value as per RFC 3261.
	sigMgrSignalingInfoStruct.eAuthAlgorithm = m->pConfig->eAlgorithm;
	sigMgrSignalingInfoStruct.numSupportedMethods = m->uSupportedMethodsCount;
	sigMgrSignalingInfoStruct.ppSupportedMethods = m->ppSupportedMethods;
	sigMgrSignalingInfoStruct.pDisplayName = (u_char *)m->pConfig->pDisplayName;
	sigMgrSignalingInfoStruct.pPANI = (u_char *)m->pConfig->pPANI;
	sigMgrSignalingInfoStruct.pSecurityVerify = (u_char *)m->pConfig->pSecurityVerify;
	sigMgrSignalingInfoStruct.pAssociatedUri = (u_char *)m->pConfig->pAssociatedUri;
	sigMgrSignalingInfoStruct.pUriUserPart = (u_char *)m->pConfig->pUriUserPart;

	// TODO timer values will be updated from OOM...
	customTimer.T1 = m->sipTimers.uTimerT1;
	customTimer.T2 = m->sipTimers.uTimerT2;
	customTimer.TimerF = m->sipTimers.uTimerF;
	customTimer.T4 = 5000;
	customTimer.TimerA = customTimer.T1;
	customTimer.TimerB = 55 * customTimer.T1;
	customTimer.TimerD = 32000;
	customTimer.TimerE = customTimer.T1;
	customTimer.TimerG = customTimer.T1;
	customTimer.TimerH = 64 * customTimer.T1;
	customTimer.TimerI = 5000;
	customTimer.TimerJ = 64 * customTimer.T1;
	customTimer.TimerK = 5000;
	customTimer.TimerM = 64 * customTimer.T1;
	sigMgrSignalingInfoStruct.pCustomSIPTimers = &customTimer;

	/* set signaling parameters */
	uSigMgrError = EcrioSigMgrUpdateSignalingParam(m->pSigMgrHandle, &sigMgrSignalingInfoStruct);
	if (uSigMgrError != ECRIO_SIGMGR_NO_ERROR)
	{
		uLimsError = LIMS_SIGMGR_SET_PARAM_ERROR;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrSetSignalingParam() failed with error code: SigMgrError=%d",
			__FUNCTION__, __LINE__, uSigMgrError);
		goto ERR_Fail;
	}

	goto ERR_None;

ERR_Fail:
	// We don't need to free here... all these values are just references..lims_Deinit() take care this.

ERR_None:
	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);
	return uLimsError;
}

#if defined(ENABLE_RCS)
u_int32 lims_updateCpmParamters
(
	lims_moduleStruct *m
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
	EcrioCPMParamStruct pCPMParamStruct = { 0 };
	EcrioCPMCallbackStruct callback = { 0 };

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	pCPMParamStruct.hLogHandle = m->logHandle;
	pCPMParamStruct.pal = m->pal;
	pCPMParamStruct.pHomeDomain = (u_char *)m->pConfig->pHomeDomain;
	pCPMParamStruct.pPublicIdentity = (u_char *)m->pConfig->pPublicIdentity;
	pCPMParamStruct.pPrivateIdentity = (u_char *)m->pConfig->pPrivateIdentity;
	pCPMParamStruct.pPassword = (u_char *)m->pConfig->pPassword;
	pCPMParamStruct.pDeviceId = (u_char *)m->pConfig->pDeviceId;
	pCPMParamStruct.pDisplayName = (u_char *)m->pConfig->pDisplayName;
	pCPMParamStruct.bSendRingingResponse = m->pConfig->bSendRingingResponse;
	pCPMParamStruct.bEnablePAI = m->pConfig->bEnablePAI;
	pCPMParamStruct.pOOMObject = m->pConfig->pOOMObject;
	pCPMParamStruct.bIsRelayEnabled = m->pConfig->bIsRelayEnabled;
	pCPMParamStruct.pRelayServerIP = m->pConfig->pRelayServerIP;
	pCPMParamStruct.uRelayServerPort = m->pConfig->uRelayServerPort;
	pCPMParamStruct.pPANI = m->pConfig->pPANI;

	callback.pCPMNotificationCallbackFn = lims_cpmStatusCallbackHandler;
	callback.pMSRPGetPropertyCallback = (EcrioMSRPGetPropertyCallback)m->pCallback.pLimsGetPropertyCallback;
	callback.pCallbackData = m;

	uCPMError = EcrioCPMUpdate(&pCPMParamStruct, m->pSigMgrHandle, m->pCpmHandle);
	if (uCPMError != ECRIO_CPM_NO_ERROR)
	{
		uLimsError = LIMS_CPM_INIT_ERROR;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tEcrioCPMInit() failed with error code: uCPMError=%d",
			__FUNCTION__, __LINE__, uCPMError);
		goto ERR_None;
	}

ERR_None:
	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);
	return uLimsError;
}
#endif

u_int32 lims_updateParamters
(
	lims_moduleStruct *m,
	lims_configInternalStruct *pStruct
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	LIMSLOGI(m->logHandle, KLogTypeFuncParams,
		"%s:%u\tlims_moduleStruct = %x, pStruct=%p",
		__FUNCTION__, __LINE__, m, pStruct);

	if (pStruct->pSipPublicUserId == NULL || pStruct->pSipHomeDomain == NULL || pStruct->pSipPrivateUserId == NULL)
	{
		return LIMS_INVALID_PARAMETER1;
	}

	if (m->pConfig->pPublicIdentity != NULL)
	{
		LIMS_FREE((void **)&m->pConfig->pPublicIdentity, m->logHandle);
		m->pConfig->pPublicIdentity = NULL;
	}
	LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->pSipPublicUserId=%s", __FUNCTION__, __LINE__, pStruct->pSipPublicUserId);
	LIMS_ALLOC_AND_COPY_STRING(pStruct->pSipPublicUserId, m->pConfig->pPublicIdentity, uLimsError, ERR_None, m->logHandle);

	if (m->pConfig->pHomeDomain != NULL)
	{
		LIMS_FREE((void **)&m->pConfig->pHomeDomain, m->logHandle);
		m->pConfig->pHomeDomain = NULL;
	}
	LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->pSipHomeDomain=%s", __FUNCTION__, __LINE__, pStruct->pSipHomeDomain);
	LIMS_ALLOC_AND_COPY_STRING(pStruct->pSipHomeDomain, m->pConfig->pHomeDomain, uLimsError, ERR_None, m->logHandle);

	if (m->pConfig->pPrivateIdentity != NULL)
	{
		LIMS_FREE((void **)&m->pConfig->pPrivateIdentity, m->logHandle);
		m->pConfig->pPrivateIdentity = NULL;
	}
	LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->pSipPrivateUserId=%s", __FUNCTION__, __LINE__, pStruct->pSipPrivateUserId);
	LIMS_ALLOC_AND_COPY_STRING(pStruct->pSipPrivateUserId, m->pConfig->pPrivateIdentity, uLimsError, ERR_None, m->logHandle);

	if (m->pConfig->pPANI != NULL)
	{
		LIMS_FREE((void **)&m->pConfig->pPANI, m->logHandle);
		m->pConfig->pPANI = NULL;
	}
	if (pStruct->pPANI != NULL)
	{
		LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->pPANI=%s", __FUNCTION__, __LINE__, pStruct->pPANI);
		LIMS_ALLOC_AND_COPY_STRING(pStruct->pPANI, m->pConfig->pPANI, uLimsError, ERR_None, m->logHandle);
	}

	if (m->pConfig->pSecurityVerify != NULL)
	{
		LIMS_FREE((void **)&m->pConfig->pSecurityVerify, m->logHandle);
		m->pConfig->pSecurityVerify = NULL;
	}
	if (pStruct->pSecurityVerify != NULL)
	{
		LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->pSecurityVerify=%s", __FUNCTION__, __LINE__, pStruct->pSecurityVerify);
		LIMS_ALLOC_AND_COPY_STRING(pStruct->pSecurityVerify, m->pConfig->pSecurityVerify, uLimsError, ERR_None, m->logHandle);
	}
	if (m->pConfig->pAssociatedUri != NULL)
	{
		LIMS_FREE((void **)&m->pConfig->pAssociatedUri, m->logHandle);
		m->pConfig->pAssociatedUri = NULL;
	}
	if (pStruct->pAssociatedUri != NULL)
	{
		LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->pAssociatedUri=%s", __FUNCTION__, __LINE__, pStruct->pAssociatedUri);
		LIMS_ALLOC_AND_COPY_STRING(pStruct->pAssociatedUri, m->pConfig->pAssociatedUri, uLimsError, ERR_None, m->logHandle);
	}

	if (m->pConfig->pUriUserPart != NULL)
	{
		LIMS_FREE((void **)&m->pConfig->pUriUserPart, m->logHandle);
		m->pConfig->pUriUserPart = NULL;
	}
	if (pStruct->pUriUserPart != NULL)
	{
		LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->pUriUserPart=%s", __FUNCTION__, __LINE__, pStruct->pUriUserPart);
		LIMS_ALLOC_AND_COPY_STRING(pStruct->pUriUserPart, m->pConfig->pUriUserPart, uLimsError, ERR_None, m->logHandle);
	}

	if (m->pConfig->pDeviceId != NULL)
	{
		LIMS_FREE((void **)&m->pConfig->pDeviceId, m->logHandle);
		m->pConfig->pDeviceId = NULL;
	}
	if (pStruct->pIMEI!= NULL)
	{
		LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tconfigStruct->pIMEI=%s", __FUNCTION__, __LINE__, pStruct->pIMEI);
		LIMS_ALLOC_AND_COPY_STRING(pStruct->pIMEI, m->pConfig->pDeviceId, uLimsError, ERR_None, m->logHandle);
	}

	/* Initialize the SigMgr module. */
	uLimsError = lims_updateSigMgrParameters(m);
	if (uLimsError != LIMS_NO_ERROR)
	{
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tlims_initSigMgrModule() failed with error code: uLimsError=%d",
			__FUNCTION__, __LINE__, uLimsError);
		goto ERR_Fail;
	}

#if defined(ENABLE_RCS)
	/** Initialize the CPM module.*/
	m->bCpmFeature = Enum_TRUE;
	uLimsError = lims_updateCpmParamters(m);
	if (uLimsError != LIMS_NO_ERROR)
	{
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tlims_initCpmModule() failed with error code: uLimsError=%d",
			__FUNCTION__, __LINE__, uLimsError);
		goto ERR_Fail;
	}
#endif	// ENABLE_RCS

	goto ERR_None;

ERR_Fail:
ERR_None:
	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);
	return uLimsError;
}
#endif
