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

#include "EcrioCPM.h"
#include "EcrioCPMInternal.h"

u_int32 EcrioCPMInit
(
	EcrioCPMParamStruct *pCPMParam,
	EcrioCPMCallbackStruct *pCPMCallback,
	void *hSigMgrHandle,
	CPMHANDLE *hCPMHandle
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	EcrioCPMContextStruct *pContext = NULL;
	u_int32	sigMgrError = ECRIO_SIGMGR_NO_ERROR;
	u_int32	sdpError = ECRIO_SDP_NO_ERROR;
	EcrioSigMgrEngineCallbackStruct sigMgrCB = { 0 };
	LOGHANDLE hLogHandle = NULL;
	EcrioSDPConfigStruct tSDPConfig = { 0 };
	CPIMConfigStruct cpimConfig = { 0 };
	IMDNConfigStruct imdnConfig = { 0 };
//	MSRPConfigStruct msrpConfig = { 0 };
//	MSRPCallbackStruct msrpCB = { 0 };
	u_char msrpRequestURI[256] = { 0 };
	
	if (pCPMParam == NULL || pCPMCallback == NULL)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	hLogHandle = pCPMParam->hLogHandle;
	CPMLOGI(hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pal_MemoryAllocate(sizeof(EcrioCPMContextStruct), (void **)hCPMHandle);
	if (NULL == *hCPMHandle)
	{
		uError = ECRIO_CPM_MEMORY_ERROR;

		CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
			__FUNCTION__, __LINE__, uError);

		*hCPMHandle = NULL;
		goto Error_Level_01;
	}

	pal_MemorySet(*hCPMHandle, 0, sizeof(EcrioCPMContextStruct));
	pContext = (EcrioCPMContextStruct *)*hCPMHandle;

	/** Keeping reference to persistent members */
	pContext->hLogHandle = hLogHandle;
	pContext->callbackStruct = *pCPMCallback;
	pContext->hEcrioSigMgrHandle = hSigMgrHandle;
	pContext->pPublicIdentity = pCPMParam->pPublicIdentity;
	pContext->pLocalDomain = pCPMParam->pHomeDomain;
	pContext->pDeviceId = pCPMParam->pDeviceId;
	pContext->pDisplayName = pCPMParam->pDisplayName;
	pContext->bSendRingingResponse = pCPMParam->bSendRingingResponse;
	pContext->pOOMObject = pCPMParam->pOOMObject;
	pContext->bIsRelayEnabled = pCPMParam->bIsRelayEnabled;
	pContext->pRelayServerIP = pCPMParam->pRelayServerIP;
	pContext->uRelayServerPort = pCPMParam->uRelayServerPort;
	pContext->bEnablePAI = pCPMParam->bEnablePAI;
	pContext->pPANI = pCPMParam->pPANI;
	pContext->bMsrpOverTLS = pCPMParam->bMsrpOverTLS;

	pal_MemoryAllocate(ECRIO_CPM_MAX_COMMON_BUFFER_SIZE, (void **)&pContext->pString);
	if (NULL == pContext->pString)
	{
		uError = ECRIO_CPM_MEMORY_ERROR;

		CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
			__FUNCTION__, __LINE__, uError);

		goto Error_Level_01;
	}

	pal_MemorySet(pContext->pString, 0, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE);

	sigMgrCB.infoCallbackFn = CPMSigMgrInfoCallback;
	sigMgrCB.statusCallbackFn = CPMSigMgrStatusCallback;
	sigMgrCB.pCallbackData = pContext;

	sigMgrError = EcrioSigMgrSetCallbacks(pContext->hEcrioSigMgrHandle, &sigMgrCB, EcrioSigMgrCallbackRegisteringModule_CPM);
	if (sigMgrError != ECRIO_SIGMGR_NO_ERROR)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrSetCallbacks() error=%u",
			__FUNCTION__, __LINE__, sigMgrError);
		goto Error_Level_01;
	}

	/** Initialize SDP module */
	tSDPConfig.logHandle = hLogHandle;
	tSDPConfig.pUsername = (unsigned char *)pCPMParam->pPublicIdentity;
	tSDPConfig.ip.eAddressType = ECRIO_SDP_ADDRESS_TYPE_NONE;
	tSDPConfig.ip.pAddress = NULL;

	/** Fixed-length buffer for instance requirements from the SDP layer. */
	tSDPConfig.uAllocationSize = ECRIO_CPM_SDP_INSTANCE_SIZE;
	pal_MemoryAllocate(tSDPConfig.uAllocationSize, (void **)&pContext->pSDPInstance);
	if (NULL == pContext->pSDPInstance)
	{
		uError = ECRIO_CPM_MEMORY_ERROR;

		CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
			__FUNCTION__, __LINE__, uError);

		pContext->pSDPInstance = NULL;
		goto Error_Level_01;
	}

	tSDPConfig.pAllocated = pContext->pSDPInstance;

	pContext->hSDPHandle = EcrioSDPInit(&tSDPConfig, &sdpError);
	if (sdpError != ECRIO_SDP_NO_ERROR || pContext->hSDPHandle == NULL)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSDPInit() error=%u",
			__FUNCTION__, __LINE__, sigMgrError);
		goto Error_Level_01;
	}

	/*Initialize CPIM module */
	cpimConfig.logHandle = hLogHandle;
	cpimConfig.uAllocationSize = ECRIO_CPM_CPIM_INSTANCE_SIZE;
	pal_MemoryAllocate(cpimConfig.uAllocationSize, (void **)&pContext->pCPIMInstance);
	if (NULL == pContext->pCPIMInstance)
	{
		uError = ECRIO_CPM_MEMORY_ERROR;

		CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	cpimConfig.pAllocated = pContext->pCPIMInstance;

	pContext->hCPIMHandle = EcrioCPIMInit(&cpimConfig, &uError);
	if (uError != ECRIO_CPIM_NO_ERROR)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioCPIMInit() error=%u",
			__FUNCTION__, __LINE__, sigMgrError);
		goto Error_Level_01;
	}

	/*Initialize IMDN module*/
	imdnConfig.logHandle = hLogHandle;
	imdnConfig.uAllocationSize = ECRIO_CPM_IMDN_ALLOCATION_SIZE;
	pal_MemoryAllocate(imdnConfig.uAllocationSize, (void **)&pContext->pIMDNInstance);
	if (NULL == pContext->pIMDNInstance)
	{
		uError = ECRIO_CPM_MEMORY_ERROR;

		CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	imdnConfig.pAllocated = pContext->pIMDNInstance;

	pContext->hIMDNHandle = EcrioIMDNInit(&imdnConfig, &uError);
	if (uError != ECRIO_CPIM_NO_ERROR)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioIMDNInit() error=%u",
			__FUNCTION__, __LINE__, sigMgrError);
		goto Error_Level_01;
	}

#if 0
	/*Initialize MSRP module*/
	msrpConfig.logHandle = hLogHandle;
	msrpConfig.pal = pCPMParam->pal;
	msrpConfig.uAllocationSize = ECRIO_CPM_MSRP_ALLOCATION_SIZE;
	pal_MemoryAllocate(msrpConfig.uAllocationSize, (void **)&pContext->pMSRPInstance);
	if (NULL == pContext->pMSRPInstance)
	{
		uError = ECRIO_CPM_MEMORY_ERROR;

		CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	msrpConfig.pAllocated = pContext->pMSRPInstance;

	msrpCB.pCallbackData = pContext;
	msrpCB.pMSRPGetPropertyCallbackFn = pCPMCallback->pMSRPGetPropertyCallback;
	msrpCB.pMSRPFileCallbackFn = ec_CPM_MSRPFileCallback;
	msrpCB.pMSRPTextCallbackFn = ec_CPM_MSRPTextCallback;
	msrpCB.pMSRPAuthCallbackFn = ec_CPM_MSRPAuthCallback;

	if (0 >= pal_SStringPrintf((char*)msrpRequestURI, 256, "msrps://%s:%d;tcp", pCPMParam->pRelayServerIP, pCPMParam->uRelayServerPort))
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;

		CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tString Copy uError.",
			__FUNCTION__, __LINE__);
		goto Error_Level_01;
	}
	msrpConfig.pRequestUri = pal_StringCreate(msrpRequestURI, pal_StringLength(msrpRequestURI));
	msrpConfig.pUserName = pal_StringCreate((const u_char*)pCPMParam->pPrivateIdentity, pal_StringLength((const u_char*)pCPMParam->pPrivateIdentity));
	msrpConfig.pPassword = pal_StringCreate((const u_char*)pCPMParam->pPassword, pal_StringLength((const u_char*)pCPMParam->pPassword));

	pContext->hMSRPHandle = EcrioMSRPInit(&msrpConfig, &msrpCB, &uError);
	if (uError != ECRIO_CPIM_NO_ERROR )
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioMSRPInit() error=%u",
			__FUNCTION__, __LINE__, sigMgrError);
		goto Error_Level_01;
	}
#endif

	/* Init the hash map */
	ec_CPM_MapInit(&pContext->hHashMap);

Error_Level_01:
	CPMLOGI(hLogHandle, KLogTypeFuncExit, "%s:%u\tuError=%u", __FUNCTION__, __LINE__, uError);

//	if (msrpConfig.pRequestUri != NULL)
//	{
//		pal_MemoryFree((void **)&(msrpConfig.pRequestUri));
//	}
//	if (msrpConfig.pUserName != NULL)
//	{
//		pal_MemoryFree((void **)&(msrpConfig.pUserName));
//	}
//	if (msrpConfig.pPassword != NULL)
//	{
//		pal_MemoryFree((void **)&(msrpConfig.pPassword));
//	}

	if ((uError != ECRIO_CPM_NO_ERROR) && (pContext != NULL))
	{
		pal_MemoryFree((void **)&(pContext->pSDPInstance));
		pal_MemoryFree((void **)&(pContext->pCPIMInstance));
		pal_MemoryFree((void **)&(pContext->pIMDNInstance));
		pal_MemoryFree((void **)&(pContext->pMSRPInstance));
		pal_MemoryFree((void **)&(pContext->pString));

		if (pContext->hCPIMHandle)
		{
			EcrioCPIMDeinit(pContext->hCPIMHandle);
			pContext->hCPIMHandle = NULL;
		}

		if (pContext->hIMDNHandle)
		{
			EcrioIMDNDeinit(pContext->hIMDNHandle);
			pContext->hIMDNHandle = NULL;
		}

		if (*hCPMHandle)
		{
			pal_MemoryFree((void **)hCPMHandle);
		}

		*hCPMHandle = NULL;
	}

	return uError;
}

#if 0
u_int32 EcrioCPMUpdate
(
	EcrioCPMParamStruct *pCPMParam,
	void *hSigMgrHandle,
	CPMHANDLE hCPMHandle
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	EcrioCPMContextStruct *pContext = NULL;
	u_int32	sdpError = ECRIO_SDP_NO_ERROR;
	LOGHANDLE hLogHandle = NULL;
	
	if (pCPMParam == NULL)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	hLogHandle = pCPMParam->hLogHandle;
	CPMLOGI(hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pContext = (EcrioCPMContextStruct *)hCPMHandle;

	/** Keeping reference to persistent members */
	pContext->pPublicIdentity = pCPMParam->pPublicIdentity;
	pContext->pLocalDomain = pCPMParam->pHomeDomain;
	pContext->pDeviceId = pCPMParam->pDeviceId;
	pContext->pPANI = pCPMParam->pPANI;

	sdpError = EcrioSDPUpdateUserName(pContext->hSDPHandle, (unsigned char *)pCPMParam->pPublicIdentity);
	if (sdpError != ECRIO_SDP_NO_ERROR)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSDPUpdateUserName() error=%u",
			__FUNCTION__, __LINE__, sdpError);
		goto Error_Level_01;
	}

	uError = EcrioMSRPUpdateUserName(pContext->hMSRPHandle, pCPMParam->pPrivateIdentity);
	if (uError != ECRIO_CPIM_NO_ERROR )
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioMSRPUpdateUserName() error=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

Error_Level_01:
	CPMLOGI(hLogHandle, KLogTypeFuncExit, "%s:%u\tuError=%u", __FUNCTION__, __LINE__, uError);

	return uError;
}
#endif
