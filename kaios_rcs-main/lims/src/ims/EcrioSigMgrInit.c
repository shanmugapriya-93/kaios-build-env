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

#include "EcrioSigMgrCommon.h"
#include "EcrioSigMgrInit.h"
#include "EcrioSigMgrInternal.h"
#include "EcrioSigMgrUtilities.h"
#include "EcrioSigMgrIMSLibHandler.h"
#include "EcrioSigMgrTransactionHandler.h"

/*****************************************************************************
                Internal API Declaration Section - Begin
*****************************************************************************/

/*****************************************************************************
                Internal API Declaration Section - End
*****************************************************************************/

/*****************************************************************************
                    API Definition Section - Begin
*****************************************************************************/

/*****************************************************************************

Function:		EcrioSigMgrInit()

Purpose:		To initialize the Signaling Manager.

Description:	Initializes the Signaling manager. This Function is
                responsible for initializing IMS Lib, Transaction Manager
                and Dialog Manager.

Input:			EcrioSigMgrInitStruct* pSigMgrInitInfo - Struct containing
                the parameters required for SIGMgr initialization :
                  - moduleId - Module id
                  - pLogEngineHandle - Handle to Log engine
                  - timerX - timer X value, used for computing transaction
                  timer intervals.
                  - engineCallbackFnStruct - Pointer to Callback function
                  structure.

Output:			SIGMGRHANDLE* pSigMgrHandle - Handle to Signaling Manager
                instance

Returns:		error code.
*****************************************************************************/
u_int32 EcrioSigMgrInit
(
	EcrioSigMgrInitStruct *pSigMgrInitInfo,
	SIGMGRHANDLE *pSigMgrHandle
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	// EcrioSigMgrStruct *pSigMgr = NULL;

	/* Check For Input Parameters*/
	if (pSigMgrInitInfo == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	SIGMGRLOGI(pSigMgrInitInfo->pLogEngineHandle, KLogTypeFuncParams,
		"%s:%u\tpSigMgrInitInfo = %x , pSigMgrHandle = %x",
		__FUNCTION__, __LINE__, pSigMgrInitInfo, pSigMgrHandle);

	error = _EcrioSigMgrInit(pSigMgrInitInfo, pSigMgrHandle);
	if (error == ECRIO_SIGMGR_NO_ERROR)
	{
		// pSigMgr = (EcrioSigMgrStruct *)*(pSigMgrHandle);
	}

	SIGMGRLOGI(pSigMgrInitInfo->pLogEngineHandle, KLogTypeFuncExit,
		"%s:%u\t%u", __FUNCTION__, __LINE__, error);

Error_Level_01:
	return error;
}

/*****************************************************************************

Function:		EcrioSigMgrDeInit()

Purpose:		To Deinitialize the Signaling Manager.

Description:	DeInitializes the Signaling manager. This Function is
                responsible for Deinitializing IMS Lib, Transaction Manager
                and Dialog Manager.

Input:			SIGMGRHANDLE* pSigMgrHandle - Handle to Signaling Manager
                   instance.

Returns:		error code.
*****************************************************************************/
u_int32 EcrioSigMgrDeInit
(
	SIGMGRHANDLE *pSigMgrHandle
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	LOGHANDLE pLogEngineHandle = NULL;

	if ((pSigMgrHandle == NULL) || (*pSigMgrHandle == NULL))
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		goto Error_Level_01;
	}

	pSigMgr = (EcrioSigMgrStruct *)(*pSigMgrHandle);
	pLogEngineHandle = pSigMgr->pLogHandle;

	SIGMGRLOGI(pLogEngineHandle, KLogTypeFuncParams,
		"%s:%u\tpSigMgrHandle = %x",
		__FUNCTION__, __LINE__, pSigMgrHandle);

	error = _EcrioSigMgrDeInit(pSigMgr);

	*pSigMgrHandle = NULL;

	SIGMGRLOGI(pLogEngineHandle, KLogTypeFuncExit,
		"%s:%u\t%u", __FUNCTION__, __LINE__, error);

Error_Level_01:
	return error;
}

/**************************************************************************

Function:		EcrioSigMgrSetSignalingParam()

Purpose:		Sets the Signaling Parameters in Signaling Manager instance.

Description:	Sets the Signaling Parameters in Signaling Manager instance.

Input:			SIGMGRHANDLE sigMgrHandle - Signaling Manager Instance.
                EcrioSigMgrSignalingInfoStruct* pSignalingInfo - Signaling Parameters

OutPut:			None

Returns:		error code.
**************************************************************************/
u_int32 EcrioSigMgrSetSignalingParam
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSignalingInfoStruct *pSignalingInfo
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	BoolEnum bValid = Enum_FALSE;
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrUriStruct uri = { .uriScheme = EcrioSigMgrURISchemeNone };

	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		goto Error_Level_01;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry,
		"%s:%u", __FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
		"%s:%u\tsigMgrHandle = %x, SignalingInfo = %x",
		__FUNCTION__, __LINE__, sigMgrHandle, pSignalingInfo);

	if (pSignalingInfo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient Data Provided",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_02;
	}

	pSigMgr->customSIPTimers.T1 = pSignalingInfo->pCustomSIPTimers->T1;
	pSigMgr->customSIPTimers.T2 = pSignalingInfo->pCustomSIPTimers->T2;
	pSigMgr->customSIPTimers.T4 = pSignalingInfo->pCustomSIPTimers->T4;
	pSigMgr->customSIPTimers.TimerA = pSignalingInfo->pCustomSIPTimers->TimerA;
	pSigMgr->customSIPTimers.TimerB = pSignalingInfo->pCustomSIPTimers->TimerB;
	pSigMgr->customSIPTimers.TimerD = pSignalingInfo->pCustomSIPTimers->TimerD;
	pSigMgr->customSIPTimers.TimerE = pSignalingInfo->pCustomSIPTimers->TimerE;
	pSigMgr->customSIPTimers.TimerF = pSignalingInfo->pCustomSIPTimers->TimerF;
	pSigMgr->customSIPTimers.TimerG = pSignalingInfo->pCustomSIPTimers->TimerG;
	pSigMgr->customSIPTimers.TimerH = pSignalingInfo->pCustomSIPTimers->TimerH;
	pSigMgr->customSIPTimers.TimerI = pSignalingInfo->pCustomSIPTimers->TimerI;
	pSigMgr->customSIPTimers.TimerJ = pSignalingInfo->pCustomSIPTimers->TimerJ;
	pSigMgr->customSIPTimers.TimerK = pSignalingInfo->pCustomSIPTimers->TimerK;
	pSigMgr->customSIPTimers.TimerM = pSignalingInfo->pCustomSIPTimers->TimerM;

	/* initialize the Transaction Manager */
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\tInitialize Transaction Manager", __FUNCTION__, __LINE__);

	error = _EcrioSigMgrInitalizeTransactionMgr(pSigMgr);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrInitalizeTransactionMgr() error=%u",
			__FUNCTION__, __LINE__, error);

		if (ECRIO_SIGMGR_CUSTOM_TIMER_RANGE_ERROR != error)
		{
			error = ECRIO_SIGMGR_INIT_FAILED;
		}

		goto Error_Level_02;
	}

	if (pSignalingInfo->pPublicIdentity_msisdn == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient data passed for msisdn",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_02;
	}

	{
		error = _EcrioSigMgrParseUri(pSigMgr, pSignalingInfo->pPublicIdentity_msisdn, &uri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrStringCreate() for UserId, error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_02;
		}

		_EcrioSigMgrValidateUri(pSigMgr, &uri, &bValid);
		if (bValid == Enum_FALSE)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
				"%s:%u\tInsufficient data passed, pMSISDN = %u",
				__FUNCTION__, __LINE__, pSignalingInfo->pPublicIdentity_msisdn);
			error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
			goto Error_Level_02;
		}
	}

	if ((pSignalingInfo->pHomeDomain == NULL) ||
		(pSignalingInfo->pRegistrarDomain == NULL))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient data passed for domain  or pRegistrarDomain",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_02;
	}

	if (pSignalingInfo->pPrivateId == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient data passed, PrivateID",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_02;
	}

	error = _EcrioSigMgrSetSignalingParam(pSigMgr, pSignalingInfo, &uri);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\t_EcrioSigMgrSetSignalingParam() failed with error = %u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_02;
	}

	goto Error_Level_01;

Error_Level_02:

	_EcrioSigMgrReleaseUriStruct(pSigMgr, &uri);
	if (pSigMgr->pTransactionMgrHndl)
	{
		_EcrioSigMgrDeInitalizeTransactionMgr(pSigMgr);
		pSigMgr->pTransactionMgrHndl = NULL;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

Error_Level_01:
	return error;
}

/*****************************************************************************

Function:		EcrioSigMgrChangeSignalingParam()

Purpose:		Chenge signaling parameter at run time.

Description:	Update signaling param information which is stored in sigMgrHandle and also take appropriate action to Cloase previous sockets and open new one.

Input:		SIGMGRHANDLE sigMgrHandle - Signaling Manager Instance.
EcrioSigMgrSignalingChangeParamInfoStruct* pChangeParamInfo - This information will update.

Returns:		error code.
*****************************************************************************/
u_int32 EcrioSigMgrChangeSignalingParam
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSignalingChangeParamInfoStruct *pChangeParamInfo
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	_EcrioSigMgrInternalSignalingInfoStruct *pSignalingInfo = NULL;
	EcrioSigMgrCustomTimersStruct *pTempCustomSIPTimers = NULL;

	/* check for incoming parameters*/
	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry,
		"%s:%u", __FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
		"%s:%u\tsigMgrHandle = %x, pChangeParamInfo = %x",
		__FUNCTION__, __LINE__, sigMgrHandle, pChangeParamInfo);

	if (pSigMgr->pSignalingInfo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tSignaling Manager not initialized",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_NOT_INITIALIZED;
		goto Error_Level_01;
	}

	if (pChangeParamInfo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient data passed",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	pSignalingInfo = pSigMgr->pSignalingInfo;

	if (pChangeParamInfo->pCustomSIPTimers != NULL)
	{
		pTempCustomSIPTimers = pChangeParamInfo->pCustomSIPTimers;
		pSigMgr->customSIPTimers.T1 = pTempCustomSIPTimers->T1;
		pSigMgr->customSIPTimers.T2 = pTempCustomSIPTimers->T2;
		pSigMgr->customSIPTimers.T4 = pTempCustomSIPTimers->T4;
		pSigMgr->customSIPTimers.TimerA = pTempCustomSIPTimers->TimerA;
		pSigMgr->customSIPTimers.TimerB = pTempCustomSIPTimers->TimerB;
		pSigMgr->customSIPTimers.TimerD = pTempCustomSIPTimers->TimerD;
		pSigMgr->customSIPTimers.TimerE = pTempCustomSIPTimers->TimerE;
		pSigMgr->customSIPTimers.TimerF = pTempCustomSIPTimers->TimerF;
		pSigMgr->customSIPTimers.TimerG = pTempCustomSIPTimers->TimerG;
		pSigMgr->customSIPTimers.TimerH = pTempCustomSIPTimers->TimerH;
		pSigMgr->customSIPTimers.TimerI = pTempCustomSIPTimers->TimerI;
		pSigMgr->customSIPTimers.TimerJ = pTempCustomSIPTimers->TimerJ;
		pSigMgr->customSIPTimers.TimerK = pTempCustomSIPTimers->TimerK;
		pSigMgr->customSIPTimers.TimerM = pTempCustomSIPTimers->TimerM;

		error = _EcrioSigMgrChangeTimersInTransactionMgr(pSigMgr);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tError while changing the custom timers value in Transaction Manager. Error = %d",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	if (pChangeParamInfo->uMinSe != 0)
	{
		pSignalingInfo->minSe = pChangeParamInfo->uMinSe;
	}

	if (pChangeParamInfo->uSessionExpires != 0)
	{
		pSignalingInfo->sessionExpires = pChangeParamInfo->uSessionExpires;
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 EcrioSigMgrSetNetworkParam
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrNetworkStateEnums eNetworkState,
	EcrioSigMgrNetworkInfoStruct *pNetworkInfo
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 tlsConnectionError = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	BoolEnum bCreateChannels = Enum_FALSE;

	/* check for incoming parameters*/
	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry,
		"%s:%u", __FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
		"%s:%u\tsigMgrHandle = %x, eNetworkState = %d",
		__FUNCTION__, __LINE__, pSigMgr, eNetworkState);

	if (eNetworkState == EcrioSigMgrNetworkState_LTEConnected &&
		(pNetworkInfo == NULL || pNetworkInfo->pLocalIp == NULL || pNetworkInfo->pRemoteIP == NULL || pNetworkInfo->uRemotePort == 0))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tIPs are not passed",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	if (eNetworkState == EcrioSigMgrNetworkState_LTEConnected)
	{
		switch (pNetworkInfo->uStatus)
		{
			case EcrioSigMgrNetworkStatus_Success:
			case EcrioSigMgrNetworkStatus_VoLTE_Off:
			{
				if (pSigMgr->pNetworkInfo)
				{
					if (pal_StringCompare(pSigMgr->pNetworkInfo->pLocalIp, pNetworkInfo->pLocalIp) ||
						pal_StringCompare(pSigMgr->pNetworkInfo->pRemoteIP, pNetworkInfo->pRemoteIP) ||
						pSigMgr->pNetworkInfo->uLocalPort != pNetworkInfo->uLocalPort ||
						pSigMgr->pNetworkInfo->uRemotePort != pNetworkInfo->uRemotePort ||
						pSigMgr->pNetworkInfo->uRemoteClientPort != pNetworkInfo->uRemoteClientPort ||
						pSigMgr->pNetworkInfo->bIsIPv6 != pNetworkInfo->bIsIPv6)
					{
						_EcrioSigMgrTerminateCommunicationChannels(pSigMgr);

						if (pSigMgr->pNetworkInfo->pLocalIp)
						{
							pal_MemoryFree((void **)&pSigMgr->pNetworkInfo->pLocalIp);
						}

						if (pSigMgr->pNetworkInfo->pRemoteIP)
						{
							pal_MemoryFree((void **)&pSigMgr->pNetworkInfo->pRemoteIP);
						}

						bCreateChannels = Enum_TRUE;
					}
					else
					{
						if (pSigMgr->pSigMgrTransportStruct == NULL)
						{
							/* There was never a valid transport structure created, so force it. */
							// @todo Get a better solution here, either we have transport or we don't, and
							// either we want transport, or we don't!
							bCreateChannels = Enum_TRUE;
						}
					}
				}
				else
				{
					pal_MemoryAllocate(sizeof(EcrioSigMgrNetworkInfoStruct), (void **)&pSigMgr->pNetworkInfo);
					if (pSigMgr->pNetworkInfo == NULL)
					{
						error = ECRIO_SIGMGR_NO_MEMORY;

						SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
							"%s:%u\t allocation of pSigMgr->pNetworkInfo failed",
							__FUNCTION__, __LINE__);
						goto Error_Level_01;
					}

					bCreateChannels = Enum_TRUE;
				}

				if (bCreateChannels == Enum_TRUE)
				{
					pSigMgr->pNetworkInfo->pLocalIp = pal_StringCreate(pNetworkInfo->pLocalIp, pal_StringLength(pNetworkInfo->pLocalIp));
					pSigMgr->pNetworkInfo->pRemoteIP = pal_StringCreate(pNetworkInfo->pRemoteIP, pal_StringLength(pNetworkInfo->pRemoteIP));
					pSigMgr->pNetworkInfo->uLocalPort = pNetworkInfo->uLocalPort;
					pSigMgr->pNetworkInfo->uRemotePort = pNetworkInfo->uRemotePort;
					pSigMgr->pNetworkInfo->uRemoteTLSPort = pNetworkInfo->uRemoteTLSPort;
					pSigMgr->pNetworkInfo->uRemoteClientPort = pNetworkInfo->uRemoteClientPort;
					pSigMgr->pNetworkInfo->bIsIPv6 = pNetworkInfo->bIsIPv6;
					pSigMgr->pNetworkInfo->bIsProxyRouteEnabled = pNetworkInfo->bIsProxyRouteEnabled;
					if(pSigMgr->pNetworkInfo->bIsProxyRouteEnabled == Enum_TRUE) {
						pSigMgr->pNetworkInfo->pProxyRouteAddress = pal_StringCreate(
								pNetworkInfo->pProxyRouteAddress,
								pal_StringLength(pNetworkInfo->pProxyRouteAddress));
						pSigMgr->pNetworkInfo->uProxyRoutePort = pNetworkInfo->uProxyRoutePort;
					} else{
						pSigMgr->pNetworkInfo->pProxyRouteAddress = NULL;
						pSigMgr->pNetworkInfo->uProxyRoutePort = 0;
					}

					error = _EcrioSigMgrCreateCommunicationChannels(pSigMgr);
					if (error != ECRIO_SIGMGR_NO_ERROR)
					{
						if (error == ECRIO_SIGMGR_TLS_CONNECTION_FAILURE)
						{
							/* ignore the error and proceed */
							SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
								"%s:%u\t_EcrioSigMgrCreateCommunicationChannels failed for TLS connection with error = %u",
								__FUNCTION__, __LINE__, error);

							error = ECRIO_SIGMGR_NO_ERROR;
							tlsConnectionError = ECRIO_SIGMGR_TLS_CONNECTION_FAILURE;
						}
						else
						{
							SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
								"%s:%u\t_EcrioSigMgrCreateCommunicationChannels failed with error = %u",
								__FUNCTION__, __LINE__, error);

							goto Error_Level_01;
						}
					}

					if (pSigMgr->pSigMgrTransportStruct)
					{
						if (pNetworkInfo->uLocalPort == 0)
						{
							pSigMgr->pNetworkInfo->uLocalPort = pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[0].pChannelInfo->localPort;
							pNetworkInfo->uLocalPort = pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[0].pChannelInfo->localPort;
						}
					}
				}
				if (pSigMgr->pSigMgrTransportStruct)
					EcrioTXNSetTransportData(pSigMgr->pTransactionMgrHndl, pSigMgr->pSigMgrTransportStruct);
			}
			break;
			case EcrioSigMgrNetworkStatus_IPsec_Established:
			{
				if (pSigMgr->pIPSecDetails->eIPSecState == ECRIO_SIP_IPSEC_STATE_ENUM_Temporary)
				{
					error = _EcrioSigMgrCreateIPSecCommunicationChannels(pSigMgr);
					if (error != ECRIO_SIGMGR_NO_ERROR)
					{
						SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t_EcrioSigMgrCreateIPSecCommunicationChannels failed with error = %u",
							__FUNCTION__, __LINE__, error);

						goto Error_Level_01;
					}
					EcrioTXNSetTransportData(pSigMgr->pTransactionMgrHndl, pSigMgr->pSigMgrTransportStruct);
					if (pSigMgr->pIPSecDetails->ipsecRegId != 0)
					{
						EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo = NULL;
						error = _EcrioSigMgrGetUserRegInfoRegId(pSigMgr, (void*)&pSigMgr->pIPSecDetails->ipsecRegId, &pUserRegInfo);
						if (pUserRegInfo == NULL)
						{
							SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tINVALID registration Id passed, error=%u",
								__FUNCTION__, __LINE__, error);
							goto Error_Level_01;
						}

						if (pUserRegInfo->registrationState == EcrioSigMgrStateAuthenticationRequired)
						{
							error = _EcrioSigMgrSendRegister(pSigMgr, EcrioSigMgrRegisterRequestType_Reg, (void*)&pSigMgr->pIPSecDetails->ipsecRegId);
							if (error != ECRIO_SIGMGR_NO_ERROR)
							{
								SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t_EcrioSigMgrSendRegister() error=%u", __FUNCTION__, __LINE__, error);
								goto Error_Level_01;
							}
						}
					}
				}
			}
			break;
			case EcrioSigMgrNetworkStatus_IPsec_Failed:
			case EcrioSigMgrNetwotkStatus_IPsec_Disconneted:
			case EcrioSigMgrNetworkStatus_IPsec_Lost:
			{
				_EcrioSigMgrFreeIPSecNegParams(pSigMgr);
				pSigMgr->pIPSecDetails->eIPSecState = ECRIO_SIP_IPSEC_STATE_ENUM_None;
			}
			break;
			default:
			break;
		}
	}
	else
	{
		_EcrioSigMgrTerminateCommunicationChannels(pSigMgr);

		if (pSigMgr->bIPSecEnabled == Enum_TRUE)
		{
			_EcrioSigMgrTerminateIPSecCommunicationChannels(pSigMgr);
		}
		if (pSigMgr->pNetworkInfo->pLocalIp)
		{
			pal_MemoryFree((void **)&pSigMgr->pNetworkInfo->pLocalIp);
		}

		if (pSigMgr->pNetworkInfo->pRemoteIP)
		{
			pal_MemoryFree((void **)&pSigMgr->pNetworkInfo->pRemoteIP);
		}

		if (pSigMgr->pNetworkInfo->pProxyRouteAddress)
		{
			pal_MemoryFree((void **)&pSigMgr->pNetworkInfo->pProxyRouteAddress);
		}
	}

	pSigMgr->eNetworkState = eNetworkState;

Error_Level_01:
	if (tlsConnectionError != ECRIO_SIGMGR_NO_ERROR)
		error = ECRIO_SIGMGR_TLS_CONNECTION_FAILURE;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 EcrioSigMgrSetCallbacks
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrEngineCallbackStruct *pCallbackStruct,
	EcrioSigMgrCallbackRegisteringModuleEnums eCallbackRegisteringModule
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;

	/* check for incoming parameters*/
	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry,
// quiet		"%s:%u", __FUNCTION__, __LINE__);

	if (pCallbackStruct == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpCallbackStruct is NULL",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	if ((eCallbackRegisteringModule == EcrioSigMgrCallbackRegisteringModule_LIMS && pCallbackStruct->getPropertyCallbackFn == NULL) ||
		(eCallbackRegisteringModule == EcrioSigMgrCallbackRegisteringModule_SUE && (pCallbackStruct->infoCallbackFn == NULL || pCallbackStruct->statusCallbackFn == NULL)) ||
		(eCallbackRegisteringModule == EcrioSigMgrCallbackRegisteringModule_SMS && (pCallbackStruct->infoCallbackFn == NULL || pCallbackStruct->statusCallbackFn == NULL)) ||
		(eCallbackRegisteringModule == EcrioSigMgrCallbackRegisteringModule_MoIP && (pCallbackStruct->infoCallbackFn == NULL || pCallbackStruct->statusCallbackFn == NULL)) ||
		(eCallbackRegisteringModule == EcrioSigMgrCallbackRegisteringModule_CPM && (pCallbackStruct->infoCallbackFn == NULL || pCallbackStruct->statusCallbackFn == NULL)) ||
		(eCallbackRegisteringModule == EcrioSigMgrCallbackRegisteringModule_UCE && (pCallbackStruct->infoCallbackFn == NULL || pCallbackStruct->statusCallbackFn == NULL)))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tcallbacks are NULL",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	if (eCallbackRegisteringModule == EcrioSigMgrCallbackRegisteringModule_LIMS)
	{
		pSigMgr->limsCallbackStruct = *pCallbackStruct;
	}
	else if (eCallbackRegisteringModule == EcrioSigMgrCallbackRegisteringModule_SUE)
	{
		pSigMgr->sueCallbackStruct = *pCallbackStruct;
	}
	else if (eCallbackRegisteringModule == EcrioSigMgrCallbackRegisteringModule_SMS)
	{
		pSigMgr->smsCallbackStruct = *pCallbackStruct;
	}
	else if (eCallbackRegisteringModule == EcrioSigMgrCallbackRegisteringModule_MoIP)
	{
		pSigMgr->moipCallbackStruct = *pCallbackStruct;
	}
	else if (eCallbackRegisteringModule == EcrioSigMgrCallbackRegisteringModule_CPM)
	{
		pSigMgr->cpmCallbackStruct = *pCallbackStruct;
	}
	else if (eCallbackRegisteringModule == EcrioSigMgrCallbackRegisteringModule_UCE)
	{
		pSigMgr->uceCallbackStruct = *pCallbackStruct;
	}

Error_Level_01:

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
// quiet		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32	EcrioSigMgrSetFeatureTagParams
(
	SIGMGRHANDLE sigMgrHandle,
	u_int16	numFeatureTagParams,
	EcrioSigMgrParamStruct **ppFeatureTagParams
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;

	/* check for incoming parameters*/
	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry,
		"%s:%u", __FUNCTION__, __LINE__);

	if (!numFeatureTagParams || !ppFeatureTagParams)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tinput datas are NULL",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	pSigMgr->pSignalingInfo->numFeatureTagParams = numFeatureTagParams;
	pSigMgr->pSignalingInfo->ppFeatureTagParams = ppFeatureTagParams;

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		EcrioSigMgrSetIPSecParam()

Purpose:		Sets the IP Security Parameters in Signaling Manager instance.

Description:	Sets the IP Security Parameters in Signaling Manager instance.

Input:			SIGMGRHANDLE sigMgrHandle - Signaling Manager Instance.
				EcrioSipIPsecParamStruct* pSignalingInfo - Signaling Parameters

OutPut:			None

Returns:		error code.
**************************************************************************/
u_int32 EcrioSigMgrSetIPSecParam
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSipIPsecParamStruct *pIPSecParams
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	u_int32 len = 0;
	u_int16 i = 0;

	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		goto Error_Level_01;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pIPSecParams == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tinput datas are NULL", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	pSigMgr->bIPSecEnabled = Enum_TRUE;

	pal_MemoryAllocate(sizeof(EcrioSigMgrIPSecStruct), (void **)&pSigMgr->pIPSecDetails);
	if (pSigMgr->pIPSecDetails == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t allocation of pSigMgr->pIPSecDetails failed",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	pal_MemoryAllocate(sizeof(EcrioSipIPsecParamStruct), (void **)&pSigMgr->pIPSecDetails->pIPsecParams);
	if (pSigMgr->pIPSecDetails->pIPsecParams == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t allocation of pSigMgr->pIPSecDetails->pIPsecParams failed",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	pal_MemoryAllocate(sizeof(EcrioSipIPsecNegotiatedParamStruct), (void **)&pSigMgr->pIPSecDetails->pIPsecNegParams);
	if (pSigMgr->pIPSecDetails->pIPsecNegParams == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t allocation of pSigMgr->pIPSecDetails->pIPsecNegParams failed",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	{
		EcrioSipIPsecValueStruct **ppAlgo = NULL, **ppEncrypt = NULL;
		pSigMgr->pIPSecDetails->pIPsecParams->uAlgoCount = pIPSecParams->uAlgoCount;

		/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
		if (pal_UtilityDataOverflowDetected(pIPSecParams->uAlgoCount, sizeof(EcrioSipIPsecValueStruct*)) == Enum_TRUE)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pal_MemoryAllocate(sizeof(EcrioSipIPsecValueStruct*) * pIPSecParams->uAlgoCount , (void **)&ppAlgo);
		if (ppAlgo == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t allocation of ppAlgo failed",
				__FUNCTION__, __LINE__);
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}
		pSigMgr->pIPSecDetails->pIPsecParams->ppAlgo = ppAlgo;

		for (i = 0; i < pIPSecParams->uAlgoCount; i++)
		{
			pal_MemoryAllocate(sizeof(EcrioSipIPsecValueStruct), (void **)&ppAlgo[i]);
			if (ppAlgo[i] == NULL)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t allocation of ppAlgo[%d] failed",
					__FUNCTION__, __LINE__, i);
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
		    ppAlgo[i]->pValue  = (char *)pal_StringCreate((const u_char *)pIPSecParams->ppAlgo[i]->pValue, (s_int32)pal_StringLength((const u_char *)pIPSecParams->ppAlgo[i]->pValue));
		}

		pSigMgr->pIPSecDetails->pIPsecParams->uEncryptCount = pIPSecParams->uEncryptCount;

		/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
		if (pal_UtilityDataOverflowDetected(pIPSecParams->uEncryptCount, sizeof(EcrioSipIPsecValueStruct*)) == Enum_TRUE)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pal_MemoryAllocate(sizeof(EcrioSipIPsecValueStruct*)*pIPSecParams->uEncryptCount, (void **)&ppEncrypt);
		if (ppEncrypt == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t allocation of ppEncrypt failed",
				__FUNCTION__, __LINE__);
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}
		pSigMgr->pIPSecDetails->pIPsecParams->ppEncrypt = ppEncrypt;

		for (i = 0; i < pIPSecParams->uEncryptCount; i++)
		{
		    pal_MemoryAllocate(sizeof(EcrioSipIPsecValueStruct), (void **)&ppEncrypt[i]);
			if (ppEncrypt[i] == NULL)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t allocation of ppEncrypt[%d] failed",
					__FUNCTION__, __LINE__, i);
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			ppEncrypt[i]->pValue = (char *)pal_StringCreate((const u_char *)pIPSecParams->ppEncrypt[i]->pValue, (s_int32)pal_StringLength((const u_char *)pIPSecParams->ppEncrypt[i]->pValue));
		}
	}

	pSigMgr->pIPSecDetails->pIPsecParams->uPort_uc = pIPSecParams->uPort_uc;
	pSigMgr->pIPSecDetails->pIPsecParams->uPort_us = pIPSecParams->uPort_us;
	pSigMgr->pIPSecDetails->pIPsecParams->uSpi_uc = pIPSecParams->uSpi_uc;
	pSigMgr->pIPSecDetails->pIPsecParams->uSpi_us = pIPSecParams->uSpi_us;

	{
		EcrioSigMgrNetworkInfoStruct *pNetworkInfo = NULL;
		EcrioTxnMgrSignalingCommunicationStruct	*pCommunicationStruct = NULL;
		EcrioTxnMgrSignalingCommunicationChannelInfoStruct	*pChannelInfo = NULL;
		EcrioTxnMgrSignalingCommunicationChannelInfoStruct	tmpChannelInfo;

		pNetworkInfo = pSigMgr->pNetworkInfo;

		pCommunicationStruct = pSigMgr->pSigMgrTransportStruct->pCommunicationStruct;

		i = 0;
		for (i = 0; pCommunicationStruct->noOfChannels; i++)
		{
			if (pCommunicationStruct->pChannels[i].pChannelInfo != NULL)
			{
				u_int16 j = 0;
				for (j = 0; j < pCommunicationStruct->pChannels[i].channelIndex; j++)
				{
					if ((pCommunicationStruct->pChannels[i].pChannelInfo[j].eSocketType == ProtocolType_UDP) ||
						(pCommunicationStruct->pChannels[i].pChannelInfo[j].eSocketType == ProtocolType_TCP_Server))
					{
						pChannelInfo = &pCommunicationStruct->pChannels[i].pChannelInfo[j];
						break;
					}
				}
				if (pChannelInfo != NULL)
				{
					break;
				}
			}
		}

		// create UDP
		tmpChannelInfo.eSocketType = pChannelInfo->eSocketType;
		tmpChannelInfo.localPort = 0;
		tmpChannelInfo.pLocalIp = pChannelInfo->pLocalIp;
		tmpChannelInfo.pRemoteIp = pChannelInfo->pRemoteIp;
		tmpChannelInfo.remotePort = pChannelInfo->remotePort;

		error = _EcrioSigMgrCreateTempIPSecCommunicationChannel(pSigMgr, &tmpChannelInfo);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t-failed to create IPSec UDP Channel with error:%d", __FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
		pSigMgr->pIPSecDetails->pIPsecParams->uPort_us = tmpChannelInfo.localPort;

		if (pNetworkInfo->pLocalIp)
		{
			pSigMgr->pIPSecDetails->pIPsecNegParams->pLocalIp = pal_StringCreate(pNetworkInfo->pLocalIp, pal_StringLength(pNetworkInfo->pLocalIp));
		}
		if (pNetworkInfo->pRemoteIP)
		{
			pSigMgr->pIPSecDetails->pIPsecNegParams->pRemoteIp = pal_StringCreate(pNetworkInfo->pRemoteIP, pal_StringLength(pNetworkInfo->pRemoteIP));
		}

		if (pNetworkInfo->bIsIPv6 == Enum_FALSE)
		{
			pSigMgr->pIPSecDetails->pIPsecNegParams->eLocalIPType = EcrioSipNetwork_IP_Type_V4;
			pSigMgr->pIPSecDetails->pIPsecNegParams->eRemoteIPType = EcrioSipNetwork_IP_Type_V4;
		}
		else
		{
			pSigMgr->pIPSecDetails->pIPsecNegParams->eLocalIPType = EcrioSipNetwork_IP_Type_V6;
			pSigMgr->pIPSecDetails->pIPsecNegParams->eRemoteIPType = EcrioSipNetwork_IP_Type_V6;
		}
	}

	goto End_level;

Error_Level_01:
	if (pSigMgr->pIPSecDetails != NULL)
	{
		if (pSigMgr->pIPSecDetails->pIPsecParams != NULL)
		{
			if (pSigMgr->pIPSecDetails->pIPsecParams->ppAlgo != NULL)
			{
				for (i = 0; i < pSigMgr->pIPSecDetails->pIPsecParams->uAlgoCount; i++)
				{
					if (pSigMgr->pIPSecDetails->pIPsecParams->ppAlgo[i]->pValue != NULL)
					{
						pal_MemoryFree((void**)&pSigMgr->pIPSecDetails->pIPsecParams->ppAlgo[i]->pValue);
						pSigMgr->pIPSecDetails->pIPsecParams->ppAlgo[i]->pValue = NULL;
					}
				}
				pal_MemoryFree((void**)&pSigMgr->pIPSecDetails->pIPsecParams->ppAlgo);
				pSigMgr->pIPSecDetails->pIPsecParams->ppAlgo = NULL;
			}
			if (pSigMgr->pIPSecDetails->pIPsecParams->ppEncrypt != NULL)
			{
				for (i = 0; i < pSigMgr->pIPSecDetails->pIPsecParams->uEncryptCount; i++)
				{
					if (pSigMgr->pIPSecDetails->pIPsecParams->ppEncrypt[i]->pValue != NULL)
					{
						pal_MemoryFree((void**)&pSigMgr->pIPSecDetails->pIPsecParams->ppEncrypt[i]->pValue);
						pSigMgr->pIPSecDetails->pIPsecParams->ppEncrypt[i]->pValue = NULL;
					}
				}
				pal_MemoryFree((void**)&pSigMgr->pIPSecDetails->pIPsecParams->ppEncrypt);
				pSigMgr->pIPSecDetails->pIPsecParams->ppEncrypt = NULL;
			}
			pal_MemoryFree((void**)&pSigMgr->pIPSecDetails->pIPsecParams);
			pSigMgr->pIPSecDetails->pIPsecParams = NULL;
		}
		pal_MemoryFree((void**)&pSigMgr->pIPSecDetails);
		pSigMgr->pIPSecDetails = NULL;
	}
End_level:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u", __FUNCTION__, __LINE__, error);

	return error;
}

u_int32 EcrioSigMgrUpdateSignalingParam
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSignalingInfoStruct *pSignalingInfo
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	BoolEnum bValid = Enum_FALSE;
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrUriStruct uri = { .uriScheme = EcrioSigMgrURISchemeNone };

	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		goto Error_Level_01;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry,
		"%s:%u", __FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
		"%s:%u\tsigMgrHandle = %x, SignalingInfo = %x",
		__FUNCTION__, __LINE__, sigMgrHandle, pSignalingInfo);

	if (pSignalingInfo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient Data Provided",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_02;
	}

	if (pSignalingInfo->pPublicIdentity_msisdn == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient data passed for msisdn",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_02;
	}

	{
		error = _EcrioSigMgrParseUri(pSigMgr, pSignalingInfo->pPublicIdentity_msisdn, &uri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrStringCreate() for UserId, error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_02;
		}

		_EcrioSigMgrValidateUri(pSigMgr, &uri, &bValid);
		if (bValid == Enum_FALSE)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
				"%s:%u\tInsufficient data passed, pMSISDN = %u",
				__FUNCTION__, __LINE__, pSignalingInfo->pPublicIdentity_msisdn);
			error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
			goto Error_Level_02;
		}
	}

	if ((pSignalingInfo->pHomeDomain == NULL) ||
		(pSignalingInfo->pRegistrarDomain == NULL))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient data passed for domain  or pRegistrarDomain",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_02;
	}

	if (pSignalingInfo->pPrivateId == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient data passed, PrivateID",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_02;
	}

	if (pSigMgr->pSignalingInfo != NULL)
	{
		error = _EcrioSigMgrReleaseInternalSignalingInfoStruct(pSigMgr, pSigMgr->pSignalingInfo);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
				"%s:%u\t_EcrioSigMgrReleaseInternalSignalingInfoStruct() failed with error = %u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_02;
		}
	}

	error = _EcrioSigMgrSetSignalingParam(pSigMgr, pSignalingInfo, &uri);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\t_EcrioSigMgrSetSignalingParam() failed with error = %u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_02;
	}

	goto Error_Level_01;

Error_Level_02:

	_EcrioSigMgrReleaseUriStruct(pSigMgr, &uri);
	if (pSigMgr->pTransactionMgrHndl)
	{
		_EcrioSigMgrDeInitalizeTransactionMgr(pSigMgr);
		pSigMgr->pTransactionMgrHndl = NULL;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

Error_Level_01:
	return error;
}

/*****************************************************************************
                    API Definition Section - End
*****************************************************************************/
