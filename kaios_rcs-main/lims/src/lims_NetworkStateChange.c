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
* @file lims_NetworkConnectionStateChange
* @brief Implementation of lims network connection state change functionality.
*/

#include "lims.h"
#include "lims_internal.h"

/**
* This function is used to update the current connected network bearer connection
* state to lims module. Caller monitors the bearer connection and update the connection
* change to lims module using this function. This function always assume that the first remote
* IP to be used for the current remote address and corresponding channels will be created.
*
* If there is no bearer connection, caller invokes this function along
* with the type lims_Network_Connection_Type_NONE and provide pConnectionDetails as NULL.
*
* @param[in] handle						Pointer to lims instance handle.
* @param[in] ePdnType					Connected PDN type.
* @param[in] eConnectionType			Network connection type enumeration.
* @param[in] pConnectionDetails			pointer to lims_NetworkConnectionStruct.
*
* @return Returns LIMS_NO_ERROR if successful, otherwise an error
* specific to the lims implementation.
* If returned LIMS_SOCKET_ERROR, application need to re-initalize lims by invoking
* lims_Deinit(), lims_Init() and lims_NetworkStateChange().
*/
u_int32 lims_NetworkStateChange
(
	LIMSHANDLE handle,
	lims_NetworkPDNTypeEnum	ePdnType,
	lims_NetworkConnectionTypeEnum eConnectionType,
	lims_NetworkConnectionStruct *pConnectionDetails
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
	u_int32 uSueError = ECRIO_SUE_NO_ERROR;
	u_int32 uCpmError = ECRIO_CPM_NO_ERROR;
	lims_moduleStruct *m = NULL;
	EcrioSUENetworkInfoStruct networkInfo = { 0 };
	EcrioCPMNetworkParamStruct cpmNetworkInfo = {0};

	// @note ePdnType is not used yet.
	(void)ePdnType;

	if (handle == NULL)
	{
		return LIMS_INVALID_PARAMETER1;
	}

	m = (lims_moduleStruct *)handle;

	pal_MutexGlobalLock(m->pal);

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

			m->moduleState = lims_Module_State_INITIALIZED;
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tMoved to lims_Module_State_INITIALIZED state",
				__FUNCTION__, __LINE__);

			goto ERR_None;
		}
	}

	switch (eConnectionType)
	{
		case lims_Network_Connection_Type_NONE:
		default:
		{
			uSueError = EcrioSUESetNetworkParam(m->pSueHandle, EcrioSUENetworkState_NotConnected, NULL);
			if (uSueError == ECRIO_SUE_NO_ERROR || uSueError == ECRIO_SUE_TLS_SOCKET_ERROR)   // Ignoring the TLS socket error
			{
				if (uSueError == ECRIO_SUE_TLS_SOCKET_ERROR)
					LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tChange State from CONNECTED to INITIALIZED without TLS ", __FUNCTION__, __LINE__);

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
			networkInfo.uRemoteTLSPort = m->pConnection->uRemoteTLSPort;
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

			/* Set CPM network info */
			cpmNetworkInfo.bIsProxyRouteEnabled = m->pConnection->isProxyRouteEnabled;
			cpmNetworkInfo.pProxyRouteAddress = (u_char*)m->pConnection->pProxyRouteAddress;
			cpmNetworkInfo.uProxyRoutePort = m->pConnection->uProxyRoutePort;
			uCpmError = EcrioCPMSetNetworkParam(m->pCpmHandle, &cpmNetworkInfo);
			if (uCpmError != ECRIO_CPM_NO_ERROR)
			{
				LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tEcrioCPMSetNetworkParam() returned with error %d", __FUNCTION__, __LINE__, uCpmError);
				uLimsError = LIMS_CPM_SET_NETWORK_PARAM_ERROR;
				goto ERR_None;
			}

			networkInfo.bIsProxyRouteEnabled = m->pConnection->isProxyRouteEnabled;
			networkInfo.pProxyRouteAddress = (u_char*)m->pConnection->pProxyRouteAddress;
			networkInfo.uProxyRoutePort = m->pConnection->uProxyRoutePort;
			uSueError = EcrioSUESetNetworkParam(m->pSueHandle, EcrioSUENetworkState_LTEConnected, &networkInfo);
			if (uSueError == ECRIO_SUE_TLS_SOCKET_ERROR)
			{
				LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tChange State from INITIALIZED to CONNECTED without TLS connection ", __FUNCTION__, __LINE__);
				uSueError = ECRIO_SUE_NO_ERROR;
			}

			if (uSueError == ECRIO_SUE_NO_ERROR)
			{
				m->moduleState = lims_Module_State_CONNECTED;
				LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tChange State from INITIALIZED to CONNECTED", __FUNCTION__, __LINE__);
			}
		}
		break;
	}

	if (uSueError != ECRIO_SUE_NO_ERROR)
	{
		if (uSueError == ECRIO_SUE_PAL_SOCKET_ERROR || uSueError == ECRIO_SUE_TLS_SOCKET_ERROR)
		{
			uLimsError = LIMS_SOCKET_ERROR;
		}
		else
		{ 
			uLimsError = LIMS_SUE_SET_NETWORK_PARAM_ERROR;
		}
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tEcrioSUESetNetworkParam() failed with error code: uSueError=%d",
			__FUNCTION__, __LINE__, uSueError);
	}

ERR_None:
	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);

	pal_MutexGlobalUnlock(m->pal);

	return uLimsError;
}
