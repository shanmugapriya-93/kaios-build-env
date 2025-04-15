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

#include "EcrioDSListInterface.h"
#include "EcrioSigMgrCommon.h"
#include "EcrioSigMgrInit.h"
#include "EcrioSigMgrInternal.h"
#include "EcrioSigMgrUtilities.h"
#include "EcrioSigMgrIMSLibHandler.h"
#include "EcrioSigMgrTransactionHandler.h"
#include "EcrioSigMgrDialogHandler.h"
#include "EcrioSigMgrSubscribe.h"
#include "EcrioSigMgrSubscribeInternal.h"

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

Function:		_EcrioSigMgrInit()

Purpose:		To initialize the Signaling Manager.

Description:	Initializes the Signaling manager. This Function is responsible
                for initializing IMS Lib, Transaction Manager and Dialog
                Manager.

Input:			LOGHANDLE pLogEngineHandle - Handle to Log Engine.
                EcrioEngineCallbackStruct* pEngineCallbackFnStruct - Pointer to
                    Callback function structure.

OutPut:			SIGMGRHANDLE* pSigMgrHandle - Handle to Signaling Manager
                    instance.

Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrInit
(
	EcrioSigMgrInitStruct *pSigMgrInitInfo,
	SIGMGRHANDLE *pSigMgrHandle
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;

	SIGMGRLOGI(pSigMgrInitInfo->pLogEngineHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	*pSigMgrHandle = NULL;

	pal_MemoryAllocate(sizeof(EcrioSigMgrStruct), (void **)&pSigMgr);
	if (pSigMgr == NULL)
	{
		SIGMGRLOGE(pSigMgrInitInfo->pLogEngineHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	/* Initialize the Signaling Manager with default values */
	pal_MemorySet(pSigMgr, 0, sizeof(EcrioSigMgrStruct));

	/* initialize the SigMgr manager memebers.*/
	pSigMgr->pal = pSigMgrInitInfo->pal;
	pSigMgr->pLogHandle = pSigMgrInitInfo->pLogEngineHandle;
	pSigMgr->pOOMObject = (EcrioOOMStruct *)pSigMgrInitInfo->pOOMObject;

	/*Routing support*/
	//ec_SigMgrHashMap_Create(ECRIO_SIGMGR_HASHMAP_INITIAL_SIZE, &pSigMgr->pHashMap);
	ec_MapInit(&pSigMgr->hHashMap);

	/* Set the SigMgrHandle */
	*pSigMgrHandle = pSigMgr;

	error = ECRIO_SIGMGR_NO_ERROR;

Error_Level_01:
	SIGMGRLOGI(pSigMgrInitInfo->pLogEngineHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);
	return error;
}

/*****************************************************************************

Function:		_EcrioSigMgrDeInit()

Purpose:		To Deinitialize the Signaling Manager.

Description:	DeInitializes the Signaling manager. This Function is
                responsible for Deinitializing IMS Lib, Transaction Manager
                and Dialog Manager.	It closes the signalling socket and frees
                the memory allocated for the data members of the signalling
                manager master structure.

Input:			IN EcrioSigMgrStruct* pSigMgr - Handle to Signaling Manager
                    instance.

Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrDeInit
(
	EcrioSigMgrStruct *pSigMgr
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	LOGHANDLE pLogEngineHandle = NULL;

	if (pSigMgr == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		goto Error_Level_01;
	}

	pSigMgr->bIsDeInitializing = Enum_TRUE;

	pLogEngineHandle = pSigMgr->pLogHandle;

	SIGMGRLOGI(pLogEngineHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	_EcrioSigMgrTerminateCommunicationChannels(pSigMgr);

	if (pSigMgr->bIPSecEnabled == Enum_TRUE)
	{
		_EcrioSigMgrTerminateIPSecCommunicationChannels(pSigMgr);
	}

	/*Deinitialize the transaction manager*/
	if (pSigMgr->pTransactionMgrHndl != NULL)
	{
		SIGMGRLOGI(pLogEngineHandle, KLogTypeGeneral,
			"%s:%u\tDeInit Transaction Manager", __FUNCTION__, __LINE__);

		/* SBC Support */
		if (pSigMgr->pSigMgrTransportStruct != NULL)
		{
			EcrioTXNStructRelease(pSigMgr->pTransactionMgrHndl, (void **)&(pSigMgr->pSigMgrTransportStruct), EcrioTXNStructType_EcrioTXNTransportStruct, Enum_TRUE);
			EcrioTXNResetSacData((pSigMgr->pTransactionMgrHndl), pSigMgr->pSigMgrTransportStruct);
			pSigMgr->pSigMgrTransportStruct = NULL;
		}

		error = _EcrioSigMgrDeInitalizeTransactionMgr(pSigMgr);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pLogEngineHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrDeInitalizeTransactionMgr() error=%u",
				__FUNCTION__, __LINE__, error);

			/* goto Error_Level_03; */
		}

		pSigMgr->pTransactionMgrHndl = NULL;
	}

	SIGMGRLOGV(pLogEngineHandle, KLogTypeGeneral,
		"%s:%u\tRelease Signaling Manager struct", __FUNCTION__, __LINE__);

	/*Routing support*/
	//ec_SigMgrHashMap_Destroy(&pSigMgr->pHashMap);
	ec_MapDeInit(pSigMgr->hHashMap);
	/* De-Init the Signaling thread */

	_EcrioSigMgrInternalStructRelease(pSigMgr, EcrioSigMgrStructType_SigMgr,
		(void **)&pSigMgr, Enum_TRUE);

Error_Level_01:
	SIGMGRLOGV(pLogEngineHandle, KLogTypeFuncExit,
		"%s:%u\t exit with error:%d", __FUNCTION__, __LINE__, error);
	return error;
}

/*****************************************************************************

Function:		_EcrioSigMgrSetSignalingParam()

Purpose:		Sets the Signaling Parameters in Signaling Manager instance.

Description:	Sets the Signaling Parameters in Signaling Manager instance.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                EcrioSigMgrSignalingInfoStruct* pSignalingInfo - Signaling
                    Parameters

OutPut:			None

Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrSetSignalingParam
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSignalingInfoStruct *pSignalingInfo,
	EcrioSigMgrUriStruct *pMSISDN
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	_EcrioSigMgrInternalSignalingInfoStruct signalingInfo = { 0 };
	u_char *pStart = NULL;
	u_char *pEnd = NULL;
	u_char *pCurrentPos = NULL;
	EcrioSigMgrUriStruct uri = { .uriScheme = EcrioSigMgrURISchemeNone };
	BoolEnum bValid = Enum_FALSE;
	u_char *pUri = NULL;
	u_int16 iterator = 0;
	EcrioSigMgrNameAddrStruct *pNameAddr = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpSigMgr=%p, pSignalingInfo=%p",
		__FUNCTION__, __LINE__, pSigMgr, pSignalingInfo);

	signalingInfo.pDisplayName = pSignalingInfo->pDisplayName;

	signalingInfo.pPrivateId = pSignalingInfo->pPrivateId;
	signalingInfo.pPassword = pSignalingInfo->pPassword;
	signalingInfo.pHomeDomain = pSignalingInfo->pHomeDomain;
	signalingInfo.pRegistrarDomain = pSignalingInfo->pRegistrarDomain;
	signalingInfo.pUserAgent = pSignalingInfo->pUserAgent;

	signalingInfo.uriScheme = EcrioSigMgrURISchemeSIP;
	signalingInfo.maxForwards = ECRIO_SIG_MGR_MAX_FORWARDS;

	signalingInfo.uUdpMtu = pSignalingInfo->uUdpMtu;

	signalingInfo.bEnableNoActivityTimer = pSignalingInfo->bEnableNoActivityTimer;
	signalingInfo.uNoActivityTimerInterval = pSignalingInfo->uNoActivityTimerInterval;

	signalingInfo.bEnableUdp = pSignalingInfo->bEnableUdp;
	signalingInfo.bEnableTcp = pSignalingInfo->bEnableTcp;

	signalingInfo.bWildCharInContact = pSignalingInfo->bWildCharInContact;

	signalingInfo.bEnabledRefreshSubscription = Enum_TRUE;

	signalingInfo.sessionExpires = pSignalingInfo->sessionExpires;
	signalingInfo.minSe = pSignalingInfo->minSe;

	signalingInfo.registrationExpires = pSignalingInfo->registrationExpires;
	signalingInfo.subscriptionExpires = pSignalingInfo->subscriptionExpires;
	signalingInfo.publishExpires = pSignalingInfo->publishExpires;
	signalingInfo.registerationTimerDelta = (signalingInfo.registrationExpires > 1200) ? 600 : signalingInfo.registrationExpires / 2;
	signalingInfo.bEnablePAUChecking = Enum_FALSE;
	signalingInfo.eAuthAlgorithm = pSignalingInfo->eAuthAlgorithm;
	signalingInfo.numSupportedMethods = pSignalingInfo->numSupportedMethods;
	signalingInfo.pPANI = pSignalingInfo->pPANI;
	signalingInfo.pSecurityVerify = pSignalingInfo->pSecurityVerify;
	signalingInfo.pAssociatedUri = pSignalingInfo->pAssociatedUri;
	signalingInfo.pUriUserPart = pSignalingInfo->pUriUserPart;
	signalingInfo.bEnableTls = pSignalingInfo->bTLSEnabled;
	signalingInfo.bTLSPeerVerification = pSignalingInfo->bTLSPeerVerification;
	signalingInfo.pTLSCertificate = pSignalingInfo->pTLSCertificate;

	if (pSignalingInfo->numSupportedMethods && pSignalingInfo->ppSupportedMethods)
	{
		signalingInfo.ppSupportedMethod = pSignalingInfo->ppSupportedMethods;
	}

	signalingInfo.pPAccessNWInfo = pSignalingInfo->pAccessNWInfo;
	if (pSigMgr->pSignalingInfo == NULL)
	{
		pal_MemoryAllocate(sizeof(_EcrioSigMgrInternalSignalingInfoStruct), (void **)&(pSigMgr->pSignalingInfo));
		if (pSigMgr->pSignalingInfo == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for Signaling Info, error=%u",
				__FUNCTION__, __LINE__, error);

			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_02;
		}
	}

	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tpAssociatedUri = %s, size=%u", __FUNCTION__, __LINE__, signalingInfo.pAssociatedUri, pal_StringLength(signalingInfo.pAssociatedUri));
	if (signalingInfo.pAssociatedUri != NULL)
	{
		pStart = signalingInfo.pAssociatedUri;
		pCurrentPos = signalingInfo.pAssociatedUri;
		signalingInfo.numPAssociatedURIs = 0;
		while (*pCurrentPos != '\0')
		{
			if (*pCurrentPos == '>')
			{
				pEnd = pCurrentPos;
			}
			else if (*pCurrentPos == '<')
			{
				pStart = pCurrentPos;
				pCurrentPos++;
				continue;					
			}
			else
			{
				pCurrentPos++;
				continue;
			}

			if (signalingInfo.ppPAssociatedURI == NULL)
			{
				pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct *), (void **)&signalingInfo.ppPAssociatedURI);
				if (signalingInfo.ppPAssociatedURI == NULL)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tpal_MemoryAllocate() for ppPAssociatedURI, error=%u",
						__FUNCTION__, __LINE__, error);

					error = ECRIO_SIGMGR_NO_MEMORY;
					goto Error_Level_02;
				}
			}
			else
			{
				/* Check arithmetic overflow - do not use the ArithmeticOverflowDetected() API for u_int16 variable. */
				if (signalingInfo.numPAssociatedURIs + 1 > USHRT_MAX)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					goto Error_Level_01;
				}

				/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
				if (pal_UtilityDataOverflowDetected((signalingInfo.numPAssociatedURIs + 1), sizeof(EcrioSigMgrNameAddrWithParamsStruct *)) == Enum_TRUE)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					goto Error_Level_01;
				}

				pal_MemoryReallocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct *) * (signalingInfo.numPAssociatedURIs + 1), \
					(void **)&signalingInfo.ppPAssociatedURI);
				if (signalingInfo.ppPAssociatedURI == NULL)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tpal_MemoryAllocate() for ppPAssociatedURI, error=%u",
						__FUNCTION__, __LINE__, error);

					error = ECRIO_SIGMGR_NO_MEMORY;
					goto Error_Level_02;
				}
			}

			pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct), (void **)&signalingInfo.ppPAssociatedURI[signalingInfo.numPAssociatedURIs]);
			if (signalingInfo.ppPAssociatedURI[signalingInfo.numPAssociatedURIs] == NULL)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryAllocate() for ppPAssociatedURI, error=%u",
					__FUNCTION__, __LINE__, error);

				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_02;
			}

			if (_EcrioSigMgrParseNameAddrWithParams(pSigMgr, pStart, pEnd, signalingInfo.ppPAssociatedURI[signalingInfo.numPAssociatedURIs]))
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrParseNameAddrWithParams() is failed",
					__FUNCTION__, __LINE__);

				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_02;
			}

			signalingInfo.numPAssociatedURIs++;
			pCurrentPos++;
		}

		for (iterator = 0; iterator < signalingInfo.numPAssociatedURIs; iterator++)
		{
			if (signalingInfo.ppPAssociatedURI[iterator] == NULL)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
					"%s:%u\tInsufficient data passed, signalingInfo.ppPAssociatedURI",
					__FUNCTION__, __LINE__);
				error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
				goto Error_Level_02;
			}			

			pNameAddr = &signalingInfo.ppPAssociatedURI[iterator]->nameAddr;
			if (pNameAddr->addrSpec.uriScheme == EcrioSigMgrURISchemeSIP)
			{
				error = _EcrioSigMgrFormUri(pSigMgr, &pNameAddr->addrSpec, &pUri);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					error = ECRIO_SIGMGR_UNKNOWN_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrFormUri() error=%u",
						__FUNCTION__, __LINE__, error);

					goto Error_Level_02;
				}
			}
		}

		error = _EcrioSigMgrParseUri(pSigMgr, pUri, &uri);
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
				"%s:%u\tInsufficient data passed, pUri = %u",
				__FUNCTION__, __LINE__, pUri);
			error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
			goto Error_Level_02;
		}

		signalingInfo.pUserId = pal_StringCreate(uri.u.pSipUri->pUserId, pal_StringLength(uri.u.pSipUri->pUserId));

		pal_MemoryAllocate(sizeof(EcrioSigMgrUriStruct), (void **)&signalingInfo.pFromSipURI);
		if (signalingInfo.pFromSipURI == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for signalingInfo.pFromSipURI, error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_02;
		}

		*(signalingInfo.pFromSipURI) = uri;

		if (pUri != NULL)
		{
			pal_MemoryFree((void **)&pUri);
		}

		if (pMSISDN)
		{
			_EcrioSigMgrReleaseUriStruct(pSigMgr, pMSISDN);
		}
	}
	else
	{
		signalingInfo.pUserId = pal_StringCreate(pMSISDN->u.pSipUri->pUserId, pal_StringLength(pMSISDN->u.pSipUri->pUserId));

		pal_MemoryAllocate(sizeof(EcrioSigMgrUriStruct), (void **)&signalingInfo.pFromSipURI);
		if (signalingInfo.pFromSipURI == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for signalingInfo.pFromSipURI, error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_02;
		}

		*(signalingInfo.pFromSipURI) = *pMSISDN;
		pal_MemorySet((void *)pMSISDN, 0, sizeof(EcrioSigMgrUriStruct));
	}

	*(pSigMgr->pSignalingInfo) = signalingInfo;

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\tParams set SUCCESSFULLLY", __FUNCTION__, __LINE__);

	goto Error_Level_01;

Error_Level_02:
	if (pUri != NULL)
	{
		pal_MemoryFree((void **)&pUri);
	}
	
	_EcrioSigMgrReleaseInternalSignalingInfoStruct(pSigMgr, &signalingInfo);

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}


#if 0
/*Routing support*/
/* create a new empty hash map */
void ec_SigMgrHashMap_Create(u_int32 size, EcrioSigMgrHashMap **ppHashMap)
{
	EcrioSigMgrHashMap *pHashMap = NULL;

	if (ppHashMap == NULL)
		return;

	pal_MemoryAllocate(sizeof(EcrioSigMgrHashMap), (void**)&pHashMap);
	if (pHashMap == NULL)
		return;
	pHashMap->n = 0;
	pHashMap->size = size;

	pal_MemoryAllocate(sizeof(struct elt*)* size, (void**)&pHashMap->ppTable);
	if (pHashMap->ppTable == NULL)
	{
		pal_MemoryFree((void**)&pHashMap);
		return;
	}

	*ppHashMap = pHashMap;
	return;
}

/* destroy a hash map */
void ec_SigMgrHashMap_Destroy(EcrioSigMgrHashMap **ppHashMap)
{
	u_int32 index = 0;
	EcrioSigMgrHashMap *pHashMap = NULL;
	struct elt *pElt = NULL;
	struct elt *pNext = NULL;

	if (ppHashMap == NULL)
		return;

	pHashMap = *ppHashMap;

	for (index = 0; index < pHashMap->size; index++)
	{
		for (pElt = pHashMap->ppTable[index]; pElt != NULL; pElt = pNext)
		{
			pNext = pElt->pNext;

			pal_MemoryFree((void**)&pElt->pKey);
			pal_MemoryFree((void**)&pElt->pValue);
			pal_MemoryFree((void**)&pElt);
		}
	}

	pal_MemoryFree((void**)&pHashMap->ppTable);
	pal_MemoryFree(&pHashMap);
	*ppHashMap = NULL;
}

static u_int64 hash_function(u_char* pKey)
{
	u_char* pStart = NULL;
	u_int64 hash = 0;
	char ucMULTIPLIER = 97;

	pStart = pKey;

	while (*pStart != '\0')
	{
		hash = (hash * ucMULTIPLIER + *pStart);
		pStart++;
	}

	return hash;
}

static void hashmap_grow(EcrioSigMgrHashMap *pHashMap)
{
	EcrioSigMgrHashMap *pHashMap2 = NULL, swap; /* new hashmap we'll create */
	struct elt* pElt = NULL;
	u_int32 i = 0;

	ec_SigMgrHashMap_Create(pHashMap->size * ECRIO_SIGMGR_HASHMAP_GROWTH_FACTOR, &pHashMap2);
	if (pHashMap2 == NULL)
		return;

	for (i = 0; i < pHashMap->size; i++)
	{
		for (pElt = pHashMap->ppTable[i]; pElt != NULL; pElt = pElt->pNext)
		{
			ec_SigMgrHashMap_Insert(pHashMap2, pElt->pKey, pElt->pValue);
		}
	}

	swap = *pHashMap;
	*pHashMap = *pHashMap2;
	*pHashMap2 = swap;

	ec_SigMgrHashMap_Destroy(&pHashMap2);
}

/* insert a new key-value pair into an existing hash map */
void ec_SigMgrHashMap_Insert(EcrioSigMgrHashMap *pHashMap, u_char *pKey, void* pValue)
{
	struct elt* pElt = NULL;
	u_int64 hashkey = 0;

	if (pHashMap == NULL || pKey == NULL || pValue == NULL)
	{
		return;
	}

	pal_MemoryAllocate(sizeof(struct elt), &pElt);
	if (pElt == NULL)
		return;

	pElt->pKey = pal_StringCreate(pKey, pal_StringLength(pKey));
	pElt->pValue = pValue;

	hashkey = hash_function(pKey) % pHashMap->size;

	pElt->pNext = pHashMap->ppTable[hashkey];
	pHashMap->ppTable[hashkey] = pElt;
	pHashMap->n++;

	if (pHashMap->n >= pHashMap->size * ECRIO_SIGMGR_HASHMAP_MAX_LOAD_FACTOR)
		hashmap_grow(pHashMap);
}

/*search the most recently inserted value associated with a key*/
/*or NULL if no matching key is present*/
void ec_SigMgrHashMap_Search(EcrioSigMgrHashMap *pHashMap, u_char *pKey, void** ppValue)
{
	struct elt* pElt = NULL;

	for (pElt = pHashMap->ppTable[hash_function(pKey) % pHashMap->size]; pElt != NULL; pElt = pElt->pNext)
	{
		if (pal_StringCompare(pKey, pElt->pKey) == 0)
		{
			*ppValue = pElt->pValue;
			break;
		}
	}

}

/*delete the most recently inserted record with the given key*/
/*if there is no such record, has no effect*/
void ec_SigMgrHashMap_Delete(EcrioSigMgrHashMap *pHashMap, u_char *pKey)
{
	struct elt* pElt = NULL;
	struct elt** ppPrev = NULL;

	for (ppPrev = &(pHashMap->ppTable[hash_function(pKey) % pHashMap->size]); *ppPrev != NULL; ppPrev = &((*ppPrev)->pNext))
	{
		if (pal_StringCompare(pKey, (*ppPrev)->pKey) == 0)
		{
			pElt = *ppPrev;
			*ppPrev = pElt->pNext;

			pal_MemoryFree((void**)&pElt->pKey);
			pal_MemoryFree((void**)&pElt->pValue);
			pal_MemoryFree((void**)&pElt);
			pHashMap->n--;
			break;
		}
	}
}
#endif
/*****************************************************************************
                    API Definition Section - End
*****************************************************************************/
