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
#include "EcrioSigMgrTransactionHandler.h"
#include "EcrioSigMgrCallbacks.h"
#include "EcrioSigMgrSubscribe.h"
#include "EcrioSigMgrSubscribeInternal.h"

u_int32 EcrioSigMgrSendSubscribe
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSendSubscribeStruct *pStruct
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	EcrioSigMgrStruct *pSigMgr = NULL;

	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	if (pStruct == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u\t",
		__FUNCTION__, __LINE__);

	if (pSigMgr->pSignalingInfo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tSignaling Manager not initialized",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_NOT_INITIALIZED;
		goto Error_Level_01;
	}

	//if (*pSessionHandle == NULL)
	{
		BoolEnum bValid = Enum_FALSE;

		if (pStruct->pTo == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tMissing To header",
				__FUNCTION__, __LINE__);
			error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
			goto Error_Level_01;
		}

		if (pStruct->uEventCount == 0 || pStruct->ppEventList == NULL ||
			pStruct->uAcceptCount == 0 || pStruct->ppAcceptList == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tMissing Event or Accept header",
				__FUNCTION__, __LINE__);
			error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
			goto Error_Level_01;
		}

		error = _EcrioSigMgrValidateUri(pSigMgr,
			&(pStruct->pTo->nameAddr.addrSpec),
			&bValid);
		if ((error != ECRIO_SIGMGR_NO_ERROR) || (bValid == Enum_FALSE))
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tInvalid To URI",
				__FUNCTION__, __LINE__);
			error = ECRIO_SIGMGR_INVALID_DATA;
			goto Error_Level_01;
		}
	}

	error = _EcrioSigMgrSendSubscribe(pSigMgr, pStruct);

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 EcrioSigMgrGetSubscribeState
(
	SIGMGRHANDLE sigMgrHandle,
	u_char *pCallId,
	EcrioSigMgrSubscribeState *pState
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	_EcrioSigMgrSubscribeUsageInfoStruct *pSubscribeUsage = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogHandle = NULL;

	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u\t",
		__FUNCTION__, __LINE__);

	if (pState == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpState is NULL", __FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}
	
	ec_MapGetKeyData(pSigMgr->hHashMap, pCallId, (void **)&pDialogHandle);

	pSubscribeUsage = (_EcrioSigMgrSubscribeUsageInfoStruct *)pDialogHandle->pDialogContext;

	*pState = pSubscribeUsage->eState;

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrSendSubscribe
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSendSubscribeStruct *pStruct
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	_EcrioSigMgrSubscribeUsageInfoStruct *pSubscribeUsage = NULL;
	EcrioSigMgrSipMessageStruct sipReqInfo = { .eMethodType = EcrioSipMessageTypeNone };

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (*(pStruct->ppCallId) == NULL)
	{
		pal_MemoryAllocate(sizeof(_EcrioSigMgrSubscribeUsageInfoStruct), (void **)&pSubscribeUsage);
		if (pSubscribeUsage == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pInviteUsage, error=%u",
				__FUNCTION__, __LINE__, error);
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		error = _EcrioSigMgrInitializeSubscribeUsage(pSigMgr, pSubscribeUsage, pStruct);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrHandleUACEventInitialized() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_02;
		}

		if (pStruct->pMsgBody != NULL)
		{
			pSubscribeUsage->pSubscribeReq->pMessageBody = pStruct->pMsgBody;
		}
		else
		{
			pSubscribeUsage->pSubscribeReq->pMessageBody = NULL;
		}

		error = _EcrioSigMgrSendInitialSubscribe(pSigMgr, pSubscribeUsage, pStruct->uExpiry, pStruct->ppCallId);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrSendSubscribeToNetwork() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_02;
		}
		goto Error_Level_01;
	}
	else
	{
		_EcrioSigMgrDialogNodeStruct *pDialogHandle = NULL;

		ec_MapGetKeyData(pSigMgr->hHashMap, *(pStruct->ppCallId), (void **)&pDialogHandle);
		pSubscribeUsage = (_EcrioSigMgrSubscribeUsageInfoStruct *)pDialogHandle->pDialogContext;

		if (pStruct->uExpiry == 0)
		{
			if (pSubscribeUsage->pSubscribeReq->pExpires)
			{
				*(pSubscribeUsage->pSubscribeReq->pExpires) = pStruct->uExpiry;
			}
		}

		if (pStruct->pMsgBody != NULL)
		{
			pSubscribeUsage->pSubscribeReq->pMessageBody = pStruct->pMsgBody;
		}
		else
		{
			pSubscribeUsage->pSubscribeReq->pMessageBody = NULL;
		}

		sipReqInfo.eMethodType = EcrioSipMessageTypeSubscribe;
		error = _EcrioSigMgrSendInDialogSipRequests(pSigMgr, &sipReqInfo, pDialogHandle);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrSendInDialogSipRequests() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
		goto Error_Level_01;
	}
Error_Level_02:

	if (pSubscribeUsage != NULL)
	{
		_EcrioSigMgrReleaseSubscribeUsageInfoStruct(pSigMgr, pSubscribeUsage);
		pal_MemoryFree((void **)&pSubscribeUsage);
		pSubscribeUsage = NULL;
	}

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrInitializeSubscribeUsage
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrSubscribeUsageInfoStruct *pSubscribeUsage,
	EcrioSigMgrSendSubscribeStruct *pStruct
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrSipMessageStruct *pTempSubscribeReq = NULL;
	EcrioSigMgrMandatoryHeaderStruct *pTempMandatoryHdrs = NULL;
	EcrioSigMgrUriStruct *pReqUri = NULL;
	u_char *ppSupported[] = { (u_char *)ECRIO_SIG_MGR_SESSION_TIMER_EXTENSION };
	u_int16	numSupported = 0;
	u_char **ppVal = NULL;
	BoolEnum bIsSubToConf = Enum_FALSE;
	BoolEnum bIsSubToPresence = Enum_FALSE;


	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	/* create temporary invite request object so that it can be used in case of 401 / 407 response.*/
	pal_MemoryAllocate(sizeof(EcrioSigMgrSipMessageStruct), (void **)&pTempSubscribeReq);
	if (pTempSubscribeReq == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for pTempSubscribeReq, error=%u",
			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	pTempSubscribeReq->eMethodType = EcrioSipMessageTypeSubscribe;
	pTempSubscribeReq->eReqRspType = EcrioSigMgrSIPRequest;

	pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&pTempSubscribeReq->pMandatoryHdrs);
	if (pTempSubscribeReq->pMandatoryHdrs == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for pTempSubscribeReq->pMandatoryHdrs, error=%u",
			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	pTempMandatoryHdrs = pTempSubscribeReq->pMandatoryHdrs;

	error = _EcrioSigMgrAllocateAndPopulateNameAddrWithParams(pSigMgr, pStruct->pTo, &pTempMandatoryHdrs->pTo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAllocateAndPopulateNameAddrWithParams() failed, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	if (pTempSubscribeReq->pRouteSet == NULL &&
		pSigMgr->pSignalingInfo->pRouteSet != NULL)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrRouteStruct), (void **)&pTempSubscribeReq->pRouteSet);
		if (pTempSubscribeReq->pRouteSet == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pTempOptionalHdrs->pRouteSet, error=%u",
				__FUNCTION__, __LINE__, error);
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pTempSubscribeReq->pRouteSet->numRoutes = pSigMgr->pSignalingInfo->pRouteSet->numRoutes;
		error = _EcrioSigMgrCopyRouteSet(pSigMgr, pTempSubscribeReq->pRouteSet->numRoutes,
			pSigMgr->pSignalingInfo->pRouteSet->ppRouteDetails, &pTempSubscribeReq->pRouteSet->ppRouteDetails);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrCopyRouteSet() for ppRouteDetails, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	error = _EcrioSigMgrAppendServiceRouteToRouteSet(pSigMgr,
		&pTempSubscribeReq->pRouteSet);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAppendServiceRouteToRouteSet() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	if (pTempSubscribeReq->pContact == NULL)
	{
		error = _EcrioSigMgrAddLocalContactUri(pSigMgr, &pTempSubscribeReq->pContact, pTempSubscribeReq->bPrivacy);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddLocalContactUri() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		if (pSigMgr->pOOMObject->ec_oom_HasUserPhoneSupport(EcrioSipMessageTypeSubscribe, EcrioSipHeaderTypeContact, EcrioSipURIType_SIP) == Enum_TRUE)
		{
			/* All outgoing SIP uri which contain telephone number in user info part must have "user=phone" parameter as SIP uri parameter.	*/
			error = _EcrioSigMgrAddUserPhoneParam(pSigMgr, &pTempSubscribeReq->pContact->ppContactDetails[0]->nameAddr.addrSpec);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddUserPhoneParam, error=%u", __FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}
	}

	error = _EcrioSigMgrUpdateMessageRequestStruct(pSigMgr, pTempSubscribeReq, Enum_FALSE);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tOptional Header Creation error, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeSubscribe, EcrioSipHeaderTypeP_AccessNetworkInfo) == Enum_TRUE)
	{
		error = _EcrioSigMgrAddP_AccessNWInfoHeader(pSigMgr, pTempSubscribeReq->pOptionalHeaderList);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddP_AccessNWInfoHeader() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeSubscribe, EcrioSipHeaderTypeP_LastAccessNetworkInfo))
	{
		error = _EcrioSigMgrAddP_LastAccessNWInfoHeader(pSigMgr, pTempSubscribeReq->pOptionalHeaderList);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSipHeaderTypeP_LastAccessNetworkInfo() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	numSupported = (u_int16)(sizeof(ppSupported) / sizeof(u_char *));
	error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSubscribeReq->pOptionalHeaderList, EcrioSipHeaderTypeSupported, numSupported, ppSupported, 0, NULL, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSubscribeReq->pOptionalHeaderList, EcrioSipHeaderTypeEvent, (u_int16)pStruct->uEventCount, pStruct->ppEventList, 0, NULL, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSubscribeReq->pOptionalHeaderList, EcrioSipHeaderTypeAccept, (u_int16)pStruct->uAcceptCount, pStruct->ppAcceptList, 0, NULL, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}
		/** Supporting gZip feature */
	{
		u_char *pGzip = (u_char *)"gzip";
		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSubscribeReq->pOptionalHeaderList, EcrioSipHeaderTypeAcceptEncoding, 1, &pGzip, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	/** Add P-PreferredIdentity header */
	error = _EcrioSigMgrAddP_PreferrdIdHeader(pSigMgr, pTempSubscribeReq);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddP_preferredIdHeader() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	if (pSigMgr->pOOMObject->ec_oom_HasUserPhoneSupport(EcrioSipMessageTypeSubscribe, EcrioSipHeaderTypeP_PreferredIdentity, EcrioSipURIType_SIP) == Enum_TRUE)
	{
		/* All outgoing SIP uri which contain telephone number in user info part must have "user=phone" parameter as SIP uri parameter.	*/
		error = _EcrioSigMgrAddUserPhoneParam(pSigMgr, &pTempSubscribeReq->ppPPreferredIdentity[0]->addrSpec);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddUserPhoneParam, error=%u", __FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	/** Add Privacy header if provide privacy */
	if (pStruct->bPrivacy == Enum_TRUE)
	{
		u_char *ppPrivacy[] = {0};

		ppPrivacy[0] =  (u_char *)"id" ;
		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSubscribeReq->pOptionalHeaderList, EcrioSipHeaderTypePrivacy, 
			(sizeof(ppPrivacy) / sizeof(u_char *)), ppPrivacy, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	if (pStruct->pConvId)
	{
		pTempSubscribeReq->pConvId = pStruct->pConvId;
		error = _EcrioSigMgrAddConversationsIDHeader(pSigMgr, pTempSubscribeReq);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddConversationsIDHeader() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	/* Disable adding user=phone and P-Preferred-Service for event conference only */
	if (pStruct->uEventCount > 0)
	{
		u_int32 index = 0;
		for (index = 0; index < pStruct->uEventCount; index++)
		{
			if (pal_StringCompare((const u_char*)pStruct->ppEventList[index], (const u_char*)"conference") == 0)
			{
				bIsSubToConf = Enum_TRUE;
				break;
			}
		}
	}

	if (pTempSubscribeReq->pMandatoryHdrs->pRequestUri == NULL)
	{
		pReqUri = &pTempMandatoryHdrs->pTo->nameAddr.addrSpec;

		/* TODO: Support strict routing */
		pal_MemoryAllocate(sizeof(EcrioSigMgrUriStruct), (void **)&pTempMandatoryHdrs->pRequestUri);
		if (pTempMandatoryHdrs->pRequestUri == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pTempMandatoryHdrs->pRequestUri, error=%u",
				__FUNCTION__, __LINE__, error);
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		error = _EcrioSigMgrPopulateUri(pSigMgr, pReqUri,
			pTempMandatoryHdrs->pRequestUri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrPopulateUri() populate request uri, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		if (bIsSubToConf == Enum_FALSE)
		{
			/* All outgoing SIP uri which contain telephone number in user info part must have "user=phone" parameter as SIP uri parameter.	*/
			error = _EcrioSigMgrAddUserPhoneParam(pSigMgr, pTempMandatoryHdrs->pRequestUri);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddUserPhoneParam, error=%u", __FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}
	}

	if (bIsSubToConf == Enum_TRUE)
	{	
		/* Add P-Preferred-Service headfer */
		pal_MemoryAllocate(sizeof(u_char *), (void **)&ppVal);
		if (ppVal == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeMemory,
				"%s:%u\tpal_MemoryAllocate() for ppVal returned NULL", __FUNCTION__, __LINE__);
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		ppVal[0] = pal_StringCreate((const u_char *)"urn:urn-7:3gpp-service.ims.icsi.oma.cpm.session.group", pal_StringLength((const u_char *)"urn:urn-7:3gpp-service.ims.icsi.oma.cpm.session.group"));
		if (ppVal[0] == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeMemory,
				"%s:%u\tpal_MemoryAllocate() for ppVal[0] returned NULL", __FUNCTION__, __LINE__);
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		if (pTempSubscribeReq->pOptionalHeaderList == NULL)
		{
			/* Create optional header object */
			error = EcrioSigMgrCreateOptionalHeaderList(pSigMgr, &pTempSubscribeReq->pOptionalHeaderList);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrCreateOptionalHeaderList() error=%u",
					__FUNCTION__, __LINE__, error);

				goto Error_Level_01;
			}
		}		

		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSubscribeReq->pOptionalHeaderList, EcrioSipHeaderTypeP_PreferredService, 1,
			ppVal, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		/* Populate Accept-Contact Header */
		pal_MemoryFree((void **)&ppVal[0]);
		ppVal[0] = pal_StringCreate((const u_char *)"*;+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session\"", pal_StringLength((const u_char *)"*;+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session\""));

		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pTempSubscribeReq->pOptionalHeaderList, EcrioSipHeaderTypeAcceptContact, 1,
			ppVal, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		/* Copy the feature tags */
		if (pStruct->pFetaureTags != NULL)
		{
			pTempSubscribeReq->pFetaureTags = pStruct->pFetaureTags;
			pStruct->pFetaureTags = NULL;
		}
	}

	/* Disable adding feature-tags in contact header for event presence only */
	if (pStruct->uEventCount > 0)
	{
		u_int32 index = 0;
		for (index = 0; index < pStruct->uEventCount; index++)
		{
			if (pal_StringCompare((const u_char*)pStruct->ppEventList[index], (const u_char*)"presence") == 0)
			{
				bIsSubToPresence = Enum_TRUE;
				break;
			}
		}
	}

	if (bIsSubToPresence == Enum_TRUE)
	{
		/* Check and remove feature tags */
		if (pTempSubscribeReq->pContact != NULL &&
			pTempSubscribeReq->pContact->ppContactDetails != NULL &&
			pTempSubscribeReq->pContact->ppContactDetails[0] != NULL &&
			pTempSubscribeReq->pContact->ppContactDetails[0]->numParams > 0)
		{
			u_int32 index = 0;
			for (index = 0; index < pTempSubscribeReq->pContact->ppContactDetails[0]->numParams; index++)
			{
				if (pTempSubscribeReq->pContact->ppContactDetails[0]->ppParams[index] != NULL)
				{
					_EcrioSigMgrReleaseParamsStruct(pSigMgr, (pTempSubscribeReq->pContact->ppContactDetails[0]->ppParams[index]));
					pal_MemoryFree((void **)&(pTempSubscribeReq->pContact->ppContactDetails[0]->ppParams[index]));
				}
			}
			pal_MemoryFree((void **)&(pTempSubscribeReq->pContact->ppContactDetails[0]->ppParams));
			pTempSubscribeReq->pContact->ppContactDetails[0]->ppParams = NULL;
			pTempSubscribeReq->pContact->ppContactDetails[0]->numParams = 0;
		}
	}

	pSubscribeUsage->pSubscribeReq = pTempSubscribeReq;
	pSubscribeUsage->pSigMgr = pSigMgr;
	error = ECRIO_SIGMGR_NO_ERROR;

Error_Level_01:
	if (ppVal)
	{
		if (ppVal[0])
		{
			pal_MemoryFree((void **)&ppVal[0]);
		}

		pal_MemoryFree((void **)&ppVal);
	}

	if ((error != ECRIO_SIGMGR_NO_ERROR) && (pTempSubscribeReq != NULL))
	{
		EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_SipMessage, (void **)&pTempSubscribeReq,
			Enum_TRUE);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrSendInitialSubscribe
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrSubscribeUsageInfoStruct *pSubscribeUsage,
	u_int32	uExpiry,
	u_char** ppCallId
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrSipMessageStruct *pTempSubscribeReq = NULL;
	EcrioSigMgrMandatoryHeaderStruct *pMandatoryHdrs = NULL;
	u_int32 reqLen = 0;
	u_char *pReqData = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	EcrioSigMgrCommonInfoStruct cmnInfo = { 0 };
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };
	// EcrioSigMgrUriStruct *pReqUri = NULL;
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;
	u_int32 *pExpires = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pTempSubscribeReq = pSubscribeUsage->pSubscribeReq;
	pMandatoryHdrs = pTempSubscribeReq->pMandatoryHdrs;
	// pReqUri = pTempSubscribeReq->pMandatoryHdrs->pRequestUri;

	++(pTempSubscribeReq->pMandatoryHdrs->CSeq);

	pal_MemoryAllocate(sizeof(u_int32), (void **)&pExpires);
	if (pExpires == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for pExpires, error=%u",
			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	*pExpires = uExpiry;
	pTempSubscribeReq->pExpires = pExpires;
	pExpires = NULL;

	if (pMandatoryHdrs->ppVia && pMandatoryHdrs->ppVia[0]
		&& pMandatoryHdrs->ppVia[0]->pBranch == NULL)
	{
		error = _EcrioSigMgrGenerateBranchParam(pSigMgr, &(pMandatoryHdrs->ppVia[0]->pBranch));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrGenerateBranchParam() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	pTempSubscribeReq->eMethodType = EcrioSipMessageTypeSubscribe;
	pTempSubscribeReq->eReqRspType = EcrioSigMgrSIPRequest;

	error = _EcrioSigMgrPopulateMandatoryHeaders(pSigMgr, pTempSubscribeReq,
		EcrioSigMgrSIPRequest, Enum_FALSE, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateMandatoryHeaders() for pIMSManHdrs, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	error = _EcrioSigMgrPopulateCommonInfo(pSigMgr, pMandatoryHdrs, &cmnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateCommonInfo() error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	/* Construct Message request */
	error = _EcrioSigMgrSipMessageForm(pSigMgr, EcrioSigMgrSipMessageModeFullName, pTempSubscribeReq, &pReqData, &reqLen);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSipMessageForm() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_IMS_LIB_ERROR;
		goto Error_Level_01;
	}

	if (pSigMgr->pSignalingInfo->bEnableTls == Enum_TRUE && pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex != 0)
	{
		eTransportType = EcrioSigMgrTransportTLS;
	}
	else if ((pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE) && (reqLen > pSigMgr->pSignalingInfo->uUdpMtu))
	{
		error = _EcrioSigMgrCreateTCPBasedBuffer(pSigMgr, &pReqData, EcrioSigMgrSIPRequest);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrCreateTCPBasedBuffer() error=%u",
				__FUNCTION__, __LINE__, error);

			error = ECRIO_SIGMGR_IMS_LIB_ERROR;
			goto Error_Level_01;
		}

		eTransportType = EcrioSigMgrTransportTCP;
	}
	else if (pSigMgr->pSignalingInfo->bEnableUdp == Enum_FALSE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE)
	{
		eTransportType = EcrioSigMgrTransportTCP;
	}

	cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_SUBSCRIBE;
	cmnInfo.role = EcrioSigMgrRoleUAC;

	txnInfo.currentContext = ECRIO_SIGMGR_SUBSCRIBE_SEND;
	txnInfo.retransmitContext = ECRIO_SIGMGR_SUBSCRIBE_RESEND;
	txnInfo.deleteflag = Enum_FALSE;
	txnInfo.messageType = EcrioSigMgrMessageNonInviteRequest;
	txnInfo.transport = eTransportType;
	txnInfo.pMsg = pReqData;
	txnInfo.msglen = reqLen;

	txnInfo.pSessionMappingStr = pTempSubscribeReq->pMandatoryHdrs->pCallId;

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeSIP,
		"%s:%u\tOutgoing SIP message constructed:", __FUNCTION__, __LINE__);
	SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pReqData, reqLen);

	if (NULL != pSubscribeUsage->pBranch)
	{
		pal_MemoryFree((void **)&pSubscribeUsage->pBranch);
	}

	pSubscribeUsage->pBranch = cmnInfo.pBranch;

	pDialogNode = _EcrioSigMgrCreateDialog(pSigMgr, pTempSubscribeReq, Enum_FALSE, pSubscribeUsage);
	if (pDialogNode == NULL)
	{
		error = ECRIO_SIGMGR_DIALOGMGR_ERROR;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrCreateDialog() error=%u",
			__FUNCTION__, __LINE__, error);
		pSubscribeUsage->pBranch = NULL;

		goto Error_Level_01;
	}

	ec_MapInsertData(pSigMgr->hHashMap, pDialogNode->pCallId, (void*)pDialogNode);

	error = _EcrioSigMgrSendMsgToTXNHandler(pSigMgr, &cmnInfo, &txnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendMsgToTXNHandler() error=%u",
			__FUNCTION__, __LINE__, error);
		/* Resolving crash issue in the error condition */
		pSubscribeUsage->pBranch = NULL;

		goto Error_Level_01;
	}

	cmnInfo.pBranch = NULL;

	*ppCallId = pDialogNode->pCallId;

Error_Level_01:

	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);
	if (pReqData != NULL)
	{
		pal_MemoryFree((void **)&pReqData);
		pReqData = NULL;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrHandleSubscribeResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSubscribeResp,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	BoolEnum bRequestTimedout
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	_EcrioSigMgrSubscribeUsageInfoStruct *pSubscribeUsage = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogHandle = NULL;
	BoolEnum bIsForked = Enum_FALSE;
	u_int32 responseCode = ECRIO_SIGMGR_INVALID_RESPONSE_CODE;
	EcrioSigMgrMessageStruct message = { .msgCmd = EcrioSigMgrMessageNone };
	u_int32	uExpires = 0;
	TimerStartConfigStruct timerConfig = { 0 };
	EcrioSigMgrHeaderStruct	*pEvent = NULL;
	BoolEnum bIsConference = Enum_FALSE;
	BoolEnum bIsPresence = Enum_FALSE;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pCmnInfo->pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_SUBSCRIBE;

	if (bRequestTimedout != Enum_TRUE)
	{
		pDialogHandle = EcrioSigMgrMatchDialogAndUpdateState(pSigMgr,
			pSubscribeResp, &bIsForked, &responseCode);
		if (pDialogHandle == NULL)
		{
			error = ECRIO_SIGMGR_DIALOGMGR_ERROR;
			goto Error_Level_01;
		}
	}
	else
	{
		BoolEnum bDialogMatched = Enum_FALSE;
		pDialogHandle = EcrioSigMgrFindMatchedDialog(pSigMgr,
			pCmnInfo->pFromTag, pCmnInfo->pToTag, pCmnInfo->pCallId, pCmnInfo->cSeq, pSubscribeResp->eMethodType,
			pSubscribeResp->eReqRspType, &bDialogMatched, &bIsForked, &responseCode);
		if (pDialogHandle == NULL)
		{
			error = ECRIO_SIGMGR_DIALOGMGR_ERROR;
			goto Error_Level_01;
		}
	}

	pSubscribeUsage = (_EcrioSigMgrSubscribeUsageInfoStruct *)pDialogHandle->pDialogContext;
	if (pSubscribeUsage == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInvite usage not found () INVITE response dropped, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	if ((pSubscribeUsage->pSubscribeReq != NULL) && (pSubscribeUsage->pSubscribeReq->pMandatoryHdrs != NULL))
	{
		if (pSubscribeResp->pMandatoryHdrs != NULL)
		{
			pSubscribeResp->pMandatoryHdrs->maxForwards = pSubscribeUsage->pSubscribeReq->pMandatoryHdrs->maxForwards;
		}
		else
		{
			error = ECRIO_SIGMGR_DIALOG_NOT_EXISTS;
			goto Error_Level_01;
		}

		pal_MemoryAllocate(sizeof(EcrioSigMgrUriStruct), (void **)&pSubscribeResp->pMandatoryHdrs->pRequestUri);
		if (pSubscribeResp->pMandatoryHdrs->pRequestUri == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		error = _EcrioSigMgrPopulateUri(pSigMgr, pSubscribeUsage->pSubscribeReq->pMandatoryHdrs->pRequestUri, 
			pSubscribeResp->pMandatoryHdrs->pRequestUri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	if ((pSubscribeUsage->pSubscribeReq != NULL) && (pSubscribeUsage->pSubscribeReq->pOptionalHeaderList != NULL))
	{
		EcrioSigMgrGetOptionalHeader((SIGMGRHANDLE)pSigMgr, pSubscribeUsage->pSubscribeReq->pOptionalHeaderList, EcrioSipHeaderTypeEvent, &pEvent);
		if (pEvent)
		{
			if (pEvent->ppHeaderValues)
			{
				u_int32 i;
				for (i = 0; i < pEvent->numHeaderValues; i++)
				{
					if (pEvent->ppHeaderValues[i] && pEvent->ppHeaderValues[i]->pHeaderValue)
					{
						if (pal_StringCompare(pEvent->ppHeaderValues[i]->pHeaderValue, (u_char *)"presence") == 0)
						{
							bIsPresence = Enum_TRUE;
							break;
						}
						else if (pal_StringCompare(pEvent->ppHeaderValues[i]->pHeaderValue, (u_char *)"conference") == 0)
						{
							bIsConference = Enum_TRUE;
							break;
						}
					}
				}
			}
		}
	}

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tSubscribe Response Code=%d",
		__FUNCTION__, __LINE__, pSubscribeResp->responseCode);

	EcrioSigMgrGetOptionalHeader((SIGMGRHANDLE)pSigMgr, pSubscribeResp->pOptionalHeaderList, EcrioSipHeaderTypeEvent, &pEvent);
	if (pEvent)
	{
		if (pEvent->ppHeaderValues)
		{
			u_int32 i;
			for (i = 0; i < pEvent->numHeaderValues; i++)
			{
				if (pEvent->ppHeaderValues[i] && pEvent->ppHeaderValues[i]->pHeaderValue)
				{
					if (pal_StringCompare(pEvent->ppHeaderValues[i]->pHeaderValue, (u_char *)"conference") == 0)
					{
						bIsConference = Enum_TRUE;
						break;
					}
					else if (pal_StringCompare(pEvent->ppHeaderValues[i]->pHeaderValue, (u_char *)"presence") == 0)
					{
						bIsPresence = Enum_TRUE;
						break;
					}
				}
			}
		}
	}
	if (bIsConference == Enum_FALSE)
	{
		pEvent = NULL;
		EcrioSigMgrGetOptionalHeader((SIGMGRHANDLE)pSigMgr, pSubscribeResp->pOptionalHeaderList, EcrioSipHeaderTypeAllowEvents, &pEvent);
		if (pEvent)
		{
			if (pEvent->ppHeaderValues)
			{
				u_int32 i;
				for (i = 0; i < pEvent->numHeaderValues; i++)
				{
					if (pEvent->ppHeaderValues[i] && pEvent->ppHeaderValues[i]->pHeaderValue)
					{
						if (pal_StringCompare(pEvent->ppHeaderValues[i]->pHeaderValue, (u_char *)"conference") == 0)
						{
							bIsConference = Enum_TRUE;
							break;
						}
					}
				}
			}
		}
	}

	/* Perform refresh only for conference event. */
	if (pSubscribeResp->pExpires != NULL && bIsPresence != Enum_TRUE)
	{
		uExpires = *(pSubscribeResp->pExpires);
		if (uExpires > 0)
		{
			//uExpires = (uExpires > 1200) ? uExpires - 600 : uExpires / 2;
			uExpires = (uExpires * 80)/100; //Choose 80% to refresh
			if (pSubscribeUsage->refreshTimerId)
			{
				SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStoping SUBSCRIBE refresh timer",
					__FUNCTION__, __LINE__);
				pal_TimerStop(pSubscribeUsage->refreshTimerId);
			}

			timerConfig.bEnableGlobalMutex = Enum_TRUE;
			timerConfig.pCallbackData = pDialogHandle;
			timerConfig.timerCallback = _EcrioSigMgrRefreshSubscribeCallback;
			timerConfig.uInitialInterval = uExpires * 1000;
			timerConfig.uPeriodicInterval = 0;

			SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStarting SUBSCRIBE refresh timer for interval %u ",
				__FUNCTION__, __LINE__, timerConfig.uInitialInterval);

			error = pal_TimerStart(pSigMgr->pal, &timerConfig, &pSubscribeUsage->refreshTimerId);
			if (KPALErrorNone != error)
			{
				error = ECRIO_SIGMGR_TIMER_ERROR;
				goto Error_Level_01;
			}
		}
		else
		{
			if (pSubscribeUsage->refreshTimerId)
			{
				SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStoping SUBSCRIBE refresh timer as expire is 0",
					__FUNCTION__, __LINE__);
				pal_TimerStop(pSubscribeUsage->refreshTimerId);
			}

			pSubscribeUsage->refreshTimerId = NULL;
		}
	}

	switch (pSubscribeResp->responseCode / 100)
	{
		case ECRIO_SIGMGR_2XX_RESPONSE:
		{
			pSubscribeResp->statusCode = ECRIO_SIGMGR_RESPONSE_SUCCESS;
			message.msgCmd = EcrioSigMgrSubscribeResponseNotification;

			message.pData = (void *)pSubscribeResp;

			if (bIsConference == Enum_TRUE)
			{
				_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
			}
			else if (bIsPresence == Enum_TRUE)
			{
				_EcrioSigMgrUpdateStatusUCE(pSigMgr, &message);
			}
			else
			{
				_EcrioSigMgrUpdateStatus(pSigMgr, &message);
			}

			pSubscribeUsage->eState = EcrioSigMgrSubscribeStateActive;
		}
		break;

		case ECRIO_SIGMGR_3XX_RESPONSE:
		case ECRIO_SIGMGR_4XX_RESPONSE:
		case ECRIO_SIGMGR_5XX_RESPONSE:
		case ECRIO_SIGMGR_6XX_RESPONSE:
		default:
		{
			pSubscribeResp->statusCode = ECRIO_SIGMGR_REQUEST_FAILED;
			message.msgCmd = EcrioSigMgrSubscribeResponseNotification;
			message.pData = (void *)pSubscribeResp;

			if (bIsConference == Enum_TRUE)
			{
				_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
			}
			else if (bIsPresence == Enum_TRUE)
			{
				_EcrioSigMgrUpdateStatusUCE(pSigMgr, &message);
			}
			else
			{
				_EcrioSigMgrUpdateStatus(pSigMgr, &message);
			}
		}
		break;
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrHandleNotifyRequest
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pNotifyReq,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	u_int32 responseCode,
	u_char *pReasonPhrase
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrMessageStruct message = { .msgCmd = EcrioSigMgrMessageNone };
	_EcrioSigMgrSubscribeUsageInfoStruct *pSubscribeUsage = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogHandle = NULL;
	BoolEnum bIsForked = Enum_FALSE;
	u_int32 respCode = 0;
	EcrioSigMgrHeaderStruct	*pSubscriptionState = NULL;
	EcrioSigMgrHeaderStruct	*pEvent = NULL;
	BoolEnum bIsConference = Enum_FALSE;
	BoolEnum bIsPresence = Enum_FALSE;

	(void)pCmnInfo;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pDialogHandle = EcrioSigMgrMatchDialogAndUpdateState(pSigMgr, pNotifyReq, &bIsForked, &respCode);
	if (pDialogHandle != NULL)
	{
		pSubscribeUsage = (_EcrioSigMgrSubscribeUsageInfoStruct *)pDialogHandle->pDialogContext;

		if (pNotifyReq->isRequestInValid == Enum_FALSE)
		{
			responseCode = ECRIO_SIGMGR_RESPONSE_CODE_OK;
			pReasonPhrase = (u_char *)ECRIO_SIGMGR_RESPONSE_OK;
		}
	}
	else
	{
		responseCode = ECRIO_SIGMGR_RESPONSE_CODE_CALL_TXN_NOT_EXIST;
		pReasonPhrase = (u_char *)ECRIO_SIGMGR_RESPONSE_CALL_TXN_NOT_EXIST;
	}

	if (responseCode != 200)
	{
		error = _EcrioSigMgrGenerateNotifyResp(pSigMgr, pNotifyReq, pSubscribeUsage,
			pReasonPhrase, responseCode);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrGenerateByeResp() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}
	else
	{
		message.msgCmd = EcrioSigMgrNotifyRequestNotification;
		message.pData = (void *)pNotifyReq;

		EcrioSigMgrGetOptionalHeader((SIGMGRHANDLE)pSigMgr, pNotifyReq->pOptionalHeaderList, EcrioSipHeaderTypeEvent, &pEvent);
		if (pEvent)
		{
			if (pEvent->ppHeaderValues)
			{
				u_int32 i;
				for (i = 0; i < pEvent->numHeaderValues; i++)
				{
					if (pEvent->ppHeaderValues[i] && pEvent->ppHeaderValues[i]->pHeaderValue)
					{
						if (pal_StringCompare(pEvent->ppHeaderValues[i]->pHeaderValue, (u_char *)"conference") == 0)
						{
							bIsConference = Enum_TRUE;
						}
						else if (pal_StringCompare(pEvent->ppHeaderValues[i]->pHeaderValue, (u_char *)"presence") == 0)
						{
							bIsPresence = Enum_TRUE;
						}
					}
				}
			}
		}

		if (bIsConference == Enum_TRUE)
		{
			_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
		}
		else if (bIsPresence == Enum_TRUE)
		{
			_EcrioSigMgrUpdateStatusUCE(pSigMgr, &message);

			EcrioSigMgrGetOptionalHeader((SIGMGRHANDLE)pSigMgr, pNotifyReq->pOptionalHeaderList, EcrioSipHeaderTypeSubscriptionState, &pSubscriptionState);
			if (pSubscriptionState)
			{
				if (pSubscriptionState->ppHeaderValues && pSubscriptionState->ppHeaderValues[0] && pSubscriptionState->ppHeaderValues[0]->pHeaderValue)
				{
					if (pal_StringCompare(pSubscriptionState->ppHeaderValues[0]->pHeaderValue, (u_char *)ECRIO_SIG_MGR_SUBSCRPTION_STATE_TERMINATED) == 0)
					{
						_EcrioSigMgrDeleteDialog(pSigMgr, pDialogHandle);
						pDialogHandle = NULL;

						_EcrioSigMgrReleaseSubscribeUsageInfoStruct(pSigMgr, pSubscribeUsage);
						pal_MemoryFree((void **)&pSubscribeUsage);
					}
				}
			}

		}
		else
		{
			_EcrioSigMgrUpdateStatus(pSigMgr, &message);

			EcrioSigMgrGetOptionalHeader((SIGMGRHANDLE)pSigMgr, pNotifyReq->pOptionalHeaderList, EcrioSipHeaderTypeSubscriptionState, &pSubscriptionState);
			if (pSubscriptionState)
			{
				if (pSubscriptionState->ppHeaderValues && pSubscriptionState->ppHeaderValues[0] && pSubscriptionState->ppHeaderValues[0]->pHeaderValue)
				{
					if (pal_StringCompare(pSubscriptionState->ppHeaderValues[0]->pHeaderValue, (u_char *)ECRIO_SIG_MGR_SUBSCRPTION_STATE_TERMINATED) == 0)
					{
						_EcrioSigMgrDeleteDialog(pSigMgr, pDialogHandle);
						pDialogHandle = NULL;

						_EcrioSigMgrReleaseSubscribeUsageInfoStruct(pSigMgr, pSubscribeUsage);
						pal_MemoryFree((void **)&pSubscribeUsage);
					}
				}
			}
		}
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrGenerateNotifyResp
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pNotifyReq,
	_EcrioSigMgrSubscribeUsageInfoStruct *pUsageData,
	u_char *pReasonPhrase,
	u_int32 responseCode
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrSipMessageStruct	notifyResp = { .eMethodType = EcrioSipMessageTypeNone };
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	(void)pUsageData;

	pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&notifyResp.pMandatoryHdrs);
	if (notifyResp.pMandatoryHdrs == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tECRIO_ALLOCATE_MEM() for byeResp.pMandatoryHdrs, error=%u",
			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_02;
	}

	error = _EcrioSigMgrPopulateManHdrs(pSigMgr, pNotifyReq->pMandatoryHdrs,
		notifyResp.pMandatoryHdrs);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateManHdrs() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}

	notifyResp.responseCode = responseCode;
	notifyResp.pReasonPhrase = pReasonPhrase;
	notifyResp.eMethodType = EcrioSipMessageTypeNotify;
	notifyResp.eReqRspType = EcrioSigMgrSIPResponse;

	ec_MapGetKeyData(pSigMgr->hHashMap, pNotifyReq->pMandatoryHdrs->pCallId, (void **)&pDialogNode);

	error = _EcrioSigMgePopulateHeaderFromDialogAndUpdateState(pSigMgr, &notifyResp, pDialogNode);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgePopulateHeaderFromDialogAndUpdateState() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}

	error = _EcrioSigMgrSendNotifyResponse(pSigMgr, &notifyResp);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendPrackResponse() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}

Error_Level_02:
	notifyResp.pReasonPhrase = NULL;
	_EcrioSigMgrReleaseSipMessage(pSigMgr, &notifyResp);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrSendNotifyResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pNotifyResp
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 respLen = 0;
	EcrioSigMgrCommonInfoStruct cmnInfo = { 0 };
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };
	u_char *pRespData = NULL;
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	error = _EcrioSigMgrPopulateCommonInfo(pSigMgr, pNotifyResp->pMandatoryHdrs, &cmnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateCommonInfo() for pCmnInfo, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	cmnInfo.responseCode = pNotifyResp->responseCode;

	eTransportType = pNotifyResp->pMandatoryHdrs->ppVia[0]->transport;

	error = _EcrioSigMgrSipMessageForm(pSigMgr, EcrioSigMgrSipMessageModeFullName, pNotifyResp, &pRespData, &respLen);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSipMessageForm() error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_NOTIFY;
	cmnInfo.role = EcrioSigMgrRoleUAS;
	cmnInfo.responseCode = pNotifyResp->responseCode;

	txnInfo.currentContext = ECRIO_SIGMGR_NOTIFY_RESPONSE_SEND;
	txnInfo.retransmitContext = ECRIO_SIGMGR_NOTIFY_RESPONSE_RESEND;

	txnInfo.deleteflag = Enum_FALSE;
	txnInfo.messageType = EcrioSigMgrMessageNonInviteResponse;
	txnInfo.transport = eTransportType;
	txnInfo.pMsg = pRespData;
	txnInfo.msglen = respLen;
	txnInfo.pSessionMappingStr = NULL;

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeSIP,
		"%s:%u\tOutgoing SIP message constructed:", __FUNCTION__, __LINE__);
	SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pRespData, respLen);

	error = _EcrioSigMgrSendMsgToTXNHandler(pSigMgr, &cmnInfo, &txnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendMsgToTXNHandler() error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	error = ECRIO_SIGMGR_NO_ERROR;
	goto Error_Level_01;

Error_Level_01:

	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);
	if (pRespData != NULL)
	{
		pal_MemoryFree((void **)&pRespData);
		pRespData = NULL;
		respLen = 0;
	}

	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 EcrioSigMgrSendNotifyResponse
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSipMessageStruct *pNotifyReq,
	u_char *pReasonPhrase,
	u_int32	responseCode
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	_EcrioSigMgrSubscribeUsageInfoStruct *pSubscribeUsage = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogHandle = NULL;

	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pNotifyReq == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient Data Provided pPrackReq = %x",
			__FUNCTION__, __LINE__, pNotifyReq);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	ec_MapGetKeyData(pSigMgr->hHashMap, pNotifyReq->pMandatoryHdrs->pCallId, (void **)&pDialogHandle);
	pSubscribeUsage = (_EcrioSigMgrSubscribeUsageInfoStruct *)pDialogHandle->pDialogContext;

	error = _EcrioSigMgrGenerateNotifyResp(pSigMgr, pNotifyReq, pSubscribeUsage,
		pReasonPhrase, responseCode);

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

void _EcrioSigMgrRefreshSubscribeCallback
(
	void *pData,
	TIMERHANDLE timerID
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogHandle = NULL;
	_EcrioSigMgrSubscribeUsageInfoStruct *pSubscribeUsageData = NULL;
	EcrioSigMgrSipMessageStruct sipReqInfo = { .eMethodType = EcrioSipMessageTypeNone };

	/* Stop the timer before proceeding further. */
	pal_TimerStop(timerID);

	pDialogHandle = (_EcrioSigMgrDialogNodeStruct *)pData;
	if (pDialogHandle == NULL)
	{
		return;
	}

	pSubscribeUsageData = (_EcrioSigMgrSubscribeUsageInfoStruct *)pDialogHandle->pDialogContext;
	if (pSubscribeUsageData == NULL)
	{
		return;
	}

	pSubscribeUsageData->refreshTimerId = NULL;

	pSigMgr = pSubscribeUsageData->pSigMgr;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pSigMgr->eNetworkState == EcrioSigMgrNetworkState_NotConnected)
	{
		pSubscribeUsageData->eState = EcrioSigMgrSubscribeStateReSubcribeTimerExpired;
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tReSubcribe Timer Expired",	__FUNCTION__, __LINE__);
	}
	else
	{
		sipReqInfo.eMethodType = EcrioSipMessageTypeSubscribe;
		error = _EcrioSigMgrSendInDialogSipRequests(pSigMgr, &sipReqInfo, pDialogHandle);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrSendInDialogSipRequests() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		pSubscribeUsageData->eState = EcrioSigMgrSubscribeStateReSubcribeProgress;
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);
}
