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

#include "EcrioSigMgrCommon.h"
#include "EcrioSigMgrInit.h"
#include "EcrioSigMgrRegister.h"
#include "EcrioSUEInternalFunctions.h"
#include "EcrioSUESigMgrCallBacks.h"
#include "EcrioSUESigMgr.h"

u_int32 EcrioSUESetNetworkParam
(
	SUEENGINEHANDLE pSUEHandle,
	EcrioSUENetworkStateEnums eNetworkState,
	EcrioSUENetworkInfoStruct *pNetworkInfo
)
{
	u_int32	uError = ECRIO_SUE_NO_ERROR;
	u_int32	tlsConnectionError = ECRIO_SUE_NO_ERROR;
	_EcrioSUEGlobalDataStruct *pSUEGlobalData = NULL;
	EcrioSigMgrNetworkInfoStruct networkInfo = { 0 };
	BoolEnum bCreateChannels = Enum_FALSE;
	u_char counter = 0;

	if (pSUEHandle == NULL)
	{
		uError = ECRIO_SUE_INSUFFICIENT_DATA_ERROR;
		return uError;
	}

	pSUEGlobalData = (_EcrioSUEGlobalDataStruct *)pSUEHandle;

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncEntry,
		"%s:%u", __FUNCTION__, __LINE__);

	if (eNetworkState == EcrioSUENetworkState_LTEConnected)
	{
		if (pSUEGlobalData->pTransportAddressStruct)
		{
			if (pal_StringCompare(pSUEGlobalData->pTransportAddressStruct->pLocalIP, pNetworkInfo->pLocalIp) ||
				pal_StringCompare(pSUEGlobalData->pTransportAddressStruct->ppPCSCFList[pSUEGlobalData->pTransportAddressStruct->curPCSCFIndex], pNetworkInfo->ppRemoteIPs[0]) ||
				pSUEGlobalData->pTransportAddressStruct->uLocalPort != pNetworkInfo->uLocalPort ||
				pSUEGlobalData->pTransportAddressStruct->uPCSCFPort != pNetworkInfo->uRemotePort ||
				pSUEGlobalData->pTransportAddressStruct->uPCSCFClientPort != pNetworkInfo->uRemoteClientPort)
			{
				if (pSUEGlobalData->pTransportAddressStruct->pLocalIP)
				{
					pal_MemoryFree((void **)&pSUEGlobalData->pTransportAddressStruct->pLocalIP);
				}

				bCreateChannels = Enum_TRUE;
			}
		}
		else
		{
			pal_MemoryAllocate(sizeof(_EcrioSUETransportAddressStruct), (void **)&pSUEGlobalData->pTransportAddressStruct);
			if (!pSUEGlobalData->pTransportAddressStruct)
			{
				uError = ECRIO_SUE_INSUFFICIENT_MEMORY_ERROR;
				goto Error_Level_01;
			}

			bCreateChannels = Enum_TRUE;
		}

		if (bCreateChannels == Enum_TRUE)
		{
			pSUEGlobalData->pTransportAddressStruct->pLocalIP = pal_StringCreate(pNetworkInfo->pLocalIp, pal_StringLength(pNetworkInfo->pLocalIp));
			pSUEGlobalData->pTransportAddressStruct->curPCSCFIndex = 0;
			if (pNetworkInfo->uNoOfRemoteIps > 0)
			{
				/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
				if (pal_UtilityDataOverflowDetected(pNetworkInfo->uNoOfRemoteIps, sizeof(u_char *)) == Enum_TRUE)
				{
					uError = ECRIO_SUE_INSUFFICIENT_MEMORY_ERROR;
					goto Error_Level_01;
				}

				pal_MemoryAllocate(sizeof(u_char *) * pNetworkInfo->uNoOfRemoteIps, (void **)&pSUEGlobalData->pTransportAddressStruct->ppPCSCFList);
				if (!pSUEGlobalData->pTransportAddressStruct->ppPCSCFList)
				{
					uError = ECRIO_SUE_INSUFFICIENT_MEMORY_ERROR;
					goto Error_Level_01;
				}
				pSUEGlobalData->pTransportAddressStruct->uNoPCSCF = pNetworkInfo->uNoOfRemoteIps;

				for (counter = 0; counter < pNetworkInfo->uNoOfRemoteIps; counter++)
				{
					pSUEGlobalData->pTransportAddressStruct->ppPCSCFList[counter] = pal_StringCreate(pNetworkInfo->ppRemoteIPs[counter], pal_StringLength(pNetworkInfo->ppRemoteIPs[counter]));
				}
			}

			pSUEGlobalData->pTransportAddressStruct->uLocalPort = pNetworkInfo->uLocalPort;
			pSUEGlobalData->pTransportAddressStruct->uPCSCFPort = pNetworkInfo->uRemotePort;
			pSUEGlobalData->pTransportAddressStruct->uPCSCFTLSPort = pNetworkInfo->uRemoteTLSPort;
			pSUEGlobalData->pTransportAddressStruct->uPCSCFClientPort = pNetworkInfo->uRemoteClientPort;
			pSUEGlobalData->pTransportAddressStruct->bIsIPv6 = pNetworkInfo->bIsIPv6;
		}
		pSUEGlobalData->pTransportAddressStruct->bIsProxyRouteEnabled = pNetworkInfo->bIsProxyRouteEnabled;
		if(pSUEGlobalData->pTransportAddressStruct->bIsProxyRouteEnabled == Enum_TRUE)
		{
			pSUEGlobalData->pTransportAddressStruct->pProxyRouteAddress = pal_StringCreate((const u_char*)pNetworkInfo->pProxyRouteAddress,
					pal_StringLength((const u_char*)pNetworkInfo->pProxyRouteAddress));
			if(pSUEGlobalData->pTransportAddressStruct->pProxyRouteAddress == NULL)
			{
				uError = ECRIO_SUE_INSUFFICIENT_MEMORY_ERROR;
				goto Error_Level_01;
			}
			pSUEGlobalData->pTransportAddressStruct->uProxyRoutePort = pNetworkInfo->uProxyRoutePort;
		} else{
			pSUEGlobalData->pTransportAddressStruct->pProxyRouteAddress = NULL;
			pSUEGlobalData->pTransportAddressStruct->uProxyRoutePort = 0;
		}
	}
	else if (eNetworkState == EcrioSUENetworkState_NotConnected)
	{
		/*Stop all transactions timers*/
		EcrioSigMgrDeleteAllTransaction(pSUEGlobalData->pSigMgrHandle);

		/*stop registraton retry timer*/
		if (pSUEGlobalData->bRegReAttemptTimerStarted == Enum_TRUE)
		{
			pal_TimerStop(pSUEGlobalData->uRegReAttemptTimerId);
			pSUEGlobalData->uRegReAttemptTimerId = NULL;
			pSUEGlobalData->bRegReAttemptTimerStarted = Enum_FALSE;
		}

		/*Stop the reregister timer*/
		EcrioSigMgrStopRegistrationTimer(pSUEGlobalData->pSigMgrHandle, EcrioSigMgrReRegisterTimer, pSUEGlobalData->pRegisterIdentifier);

		/*stop subscription retry timer*/
		if (pSUEGlobalData->bSubReAttemptTimerStarted == Enum_TRUE)
		{
			pal_TimerStop(pSUEGlobalData->uSubReAttemptTimerId);
			pSUEGlobalData->uSubReAttemptTimerId = NULL;
			pSUEGlobalData->bSubReAttemptTimerStarted = Enum_FALSE;
		}
	}

	if(pNetworkInfo != NULL) {
		networkInfo.pLocalIp = pSUEGlobalData->pTransportAddressStruct->pLocalIP;
		networkInfo.pRemoteIP = pSUEGlobalData->pTransportAddressStruct->ppPCSCFList[pSUEGlobalData->pTransportAddressStruct->curPCSCFIndex];
		networkInfo.uLocalPort = pSUEGlobalData->pTransportAddressStruct->uLocalPort;
		networkInfo.uRemotePort = pSUEGlobalData->pTransportAddressStruct->uPCSCFPort;
		networkInfo.uRemoteTLSPort = pSUEGlobalData->pTransportAddressStruct->uPCSCFTLSPort;
		networkInfo.uRemoteClientPort = pSUEGlobalData->pTransportAddressStruct->uPCSCFClientPort;
		networkInfo.bIsIPv6 = pSUEGlobalData->pTransportAddressStruct->bIsIPv6;
		networkInfo.bIsProxyRouteEnabled = pSUEGlobalData->pTransportAddressStruct->bIsProxyRouteEnabled;
		networkInfo.pProxyRouteAddress = pSUEGlobalData->pTransportAddressStruct->pProxyRouteAddress;
		networkInfo.uProxyRoutePort = pSUEGlobalData->pTransportAddressStruct->uProxyRoutePort;

		switch (pNetworkInfo->uStatus) {
			case ECRIO_SUE_Network_Status_Success:
				networkInfo.uStatus = EcrioSigMgrNetworkStatus_Success;
				break;
			case ECRIO_SUE_Network_Status_VoLTE_Off:
				networkInfo.uStatus = EcrioSigMgrNetworkStatus_VoLTE_Off;
				break;
			case ECRIO_SUE_Network_Status_IPsec_Established:
				networkInfo.uStatus = EcrioSigMgrNetworkStatus_IPsec_Established;
				break;
			case ECRIO_SUE_Network_Status_IPsec_Failed:
				networkInfo.uStatus = EcrioSigMgrNetworkStatus_IPsec_Failed;
				break;
			case ECRIO_SUE_Netwotk_Status_IPsec_Disconneted:
				networkInfo.uStatus = EcrioSigMgrNetwotkStatus_IPsec_Disconneted;
				break;
			case ECRIO_SUE_Network_Status_IPsec_Lost:
				networkInfo.uStatus = EcrioSigMgrNetworkStatus_IPsec_Lost;
				break;
			case ECRIO_SUE_Network_Status_Dedicated_Bearer_Connected:
				networkInfo.uStatus = EcrioSigMgrNetworkStatus_Dedicated_Bearer_Connected;
				break;
			case ECRIO_SUE_Network_Status_Dedicated_Bearer_Disconnected:
				networkInfo.uStatus = EcrioSigMgrNetworkStatus_Dedicated_Bearer_Disconnected;
				break;
			case ECRIO_SUE_Network_Status_Dedicated_Bearer_Failed:
				networkInfo.uStatus = EcrioSigMgrNetworkStatus_Dedicated_Bearer_Failed;
				break;
		}
		uError = EcrioSigMgrSetNetworkParam(pSUEGlobalData->pSigMgrHandle, (EcrioSigMgrNetworkStateEnums)eNetworkState, &networkInfo);
	}
	else
		uError = EcrioSigMgrSetNetworkParam(pSUEGlobalData->pSigMgrHandle, (EcrioSigMgrNetworkStateEnums)eNetworkState, NULL);

	if (uError != ECRIO_SIGMGR_NO_ERROR)
	{
		SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
			"%s:%u\tEcrioSigMgrSetNetworkParam() failed with error = %u",
			__FUNCTION__, __LINE__, uError);
		if (uError != ECRIO_SIGMGR_TLS_CONNECTION_FAILURE)
		{
			if (uError == ECRIO_SIG_MGR_SIG_SOCKET_ERROR)
				uError = ECRIO_SUE_PAL_SOCKET_ERROR;
			else
				uError = ECRIO_SUE_UA_ENGINE_ERROR;
			goto Error_Level_01;
		}
		else
		{
			uError = ECRIO_SIGMGR_NO_ERROR;
			tlsConnectionError = ECRIO_SIGMGR_TLS_CONNECTION_FAILURE;
		}
	}

	if (bCreateChannels == Enum_TRUE && (pNetworkInfo != NULL && pNetworkInfo->uLocalPort == 0))
	{
		pNetworkInfo->uLocalPort = networkInfo.uLocalPort;
		pSUEGlobalData->pTransportAddressStruct->uLocalPort = networkInfo.uLocalPort;
	}

	if (eNetworkState == EcrioSUENetworkState_LTEConnected)
	{
		pSUEGlobalData->eNetworkState = _ECRIO_SUE_INTERNAL_NETWORK_STATE_ENUM_Connected;
	}
	else
	{
		pSUEGlobalData->eNetworkState = _ECRIO_SUE_INTERNAL_NETWORK_STATE_ENUM_Disconnected;
	}

Error_Level_01:
	if (tlsConnectionError != ECRIO_SIGMGR_NO_ERROR)
		uError = ECRIO_SUE_TLS_SOCKET_ERROR;

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uError);

	return uError;
}

#ifdef ENABLE_QCMAPI
void EcrioSUESetRegistrationState
(
	SUEENGINEHANDLE	pSUEHandle,
	unsigned int eRegistrationState
)
{
	_EcrioSUEGlobalDataStruct *pSUEGlobalData = (_EcrioSUEGlobalDataStruct *)pSUEHandle;

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncEntry,
		"%s:%u", __FUNCTION__, __LINE__);

	pSUEGlobalData->eRegistrationState = (_EcrioSUEInternalRegistrationStateEnum)eRegistrationState;

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u\t",
		__FUNCTION__, __LINE__);
}

void EcrioSUESetRegistrationStateToNone
(
	SUEENGINEHANDLE	pSUEHandle
)
{
	_EcrioSUEGlobalDataStruct *pSUEGlobalData = (_EcrioSUEGlobalDataStruct *)pSUEHandle;

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncEntry,
		"%s:%u", __FUNCTION__, __LINE__);

	if (pSUEGlobalData->eRegistrationState == _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_DeRegistered ||
		pSUEGlobalData->eRegistrationState == _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_DeRegistrationFailed)
	{
		ECRIO_SUE_LOG_REGISTRATION_STATE_CHANGE(pSUEGlobalData, (u_char *)"_EcrioSUEHandleRegisterResponse", _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_None);
		pSUEGlobalData->eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_None;
	}

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u\t",
		__FUNCTION__, __LINE__);
}

void EcrioSUESetRegistrationStateToRegistering
(
	SUEENGINEHANDLE	pSUEHandle
)
{
	_EcrioSUEGlobalDataStruct *pSUEGlobalData = (_EcrioSUEGlobalDataStruct *)pSUEHandle;

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncEntry,
		"%s:%u", __FUNCTION__, __LINE__);

	if (pSUEGlobalData->eRegistrationState == _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_Registered)
	{
		ECRIO_SUE_LOG_REGISTRATION_STATE_CHANGE(pSUEGlobalData, (u_char *)"_EcrioSUEHandleRegisterResponse", _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_ReRegistering);
		pSUEGlobalData->eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_ReRegistering;
	}
	else
	{
		ECRIO_SUE_LOG_REGISTRATION_STATE_CHANGE(pSUEGlobalData, (u_char *)"_EcrioSUEHandleRegisterResponse", _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_Registering);
		pSUEGlobalData->eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_Registering;
	}

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u\t",
		__FUNCTION__, __LINE__);
}
#endif
