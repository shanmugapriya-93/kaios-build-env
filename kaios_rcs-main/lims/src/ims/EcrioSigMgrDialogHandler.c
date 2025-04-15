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
#include "EcrioSigMgrDialogHandler.h"
#include "EcrioTXN.h"

/* This function will be called while sending or receiving the dialog establishing request (INVITE/SUBSCRIBE).
For outgoing request the pSipMessage must have the allocated pMandatoryHdrs and populated pTo, pFrom with from-tag, pCallId and CSeq.
For incoming request the pSipMessage must have the allocated pMandatoryHdrs and populated pTo, pFrom with from-tag, pCallId , CSeq and pContact and possibly pRecordRoute
This function will populate the local contact by itself. It will set the state as Initialized. */
_EcrioSigMgrDialogNodeStruct *_EcrioSigMgrCreateDialog
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct	*pSipMessage,
	BoolEnum bIncomingRequest,
	void *pDialogContext
)
{
	u_int32	uError = ECRIO_SIGMGR_NO_ERROR;
	_EcrioSigMgrDialogNodeStruct *pNode = NULL;
	_EcrioSigMgrDialogNodeStruct *pReturnNode = NULL;
	EcrioSigMgrNameAddrWithParamsStruct	*pLocalUri = NULL;
	EcrioSigMgrNameAddrWithParamsStruct	*pRemoteUri = NULL;
	_EcrioSigMgrForkedMessageListNodeStruct *pForkedMsgNode = NULL;
	u_char *pLocalTag = NULL;
	u_char *pRemoteTag = NULL;
	u_char *pTag = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u pSipMessage = %x pMandatoryHdrs = %x, bIncomingRequest = %d",
		__FUNCTION__, __LINE__, pSipMessage, pSipMessage->pMandatoryHdrs, bIncomingRequest);

	if (pSipMessage->eReqRspType != EcrioSigMgrSIPRequest || pSipMessage->pMandatoryHdrs == NULL)
	{
		uError = ECRIO_SIGMGR_INVALID_DATA;
		goto EndTag;
	}

	pal_MemoryAllocate(sizeof(_EcrioSigMgrForkedMessageListNodeStruct), (void **)&pForkedMsgNode);
	if (NULL == pForkedMsgNode)
	{
		uError = ECRIO_SIGMGR_NO_MEMORY;
		goto EndTag;
	}

	pal_MemoryAllocate(sizeof(_EcrioSigMgrDialogNodeStruct), (void **)&pNode);
	if (NULL == pNode)
	{
		uError = ECRIO_SIGMGR_NO_MEMORY;
		goto EndTag;
	}

	if (pSipMessage->pMandatoryHdrs->pTo == NULL || pSipMessage->pMandatoryHdrs->pFrom == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u Either To OR From header is NULL.",
			__FUNCTION__, __LINE__);
		uError = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto EndTag;
	}

	if (pSipMessage->pMandatoryHdrs->pCallId == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u Call-ID is NULL.",
			__FUNCTION__, __LINE__);
		uError = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto EndTag;
	}
		
	if (bIncomingRequest == Enum_TRUE)
	{
		pLocalUri = pSipMessage->pMandatoryHdrs->pTo;
		if (pSipMessage->pMandatoryHdrs->pTo->ppParams)
		{
			pLocalTag = _EcrioSigMgrFindTagParam(pSigMgr, pSipMessage->pMandatoryHdrs->pTo->numParams, pSipMessage->pMandatoryHdrs->pTo->ppParams);
		}

		pRemoteUri = pSipMessage->pMandatoryHdrs->pFrom;
		if (pSipMessage->pMandatoryHdrs->pFrom->ppParams)
		{
			pRemoteTag = _EcrioSigMgrFindTagParam(pSigMgr, pSipMessage->pMandatoryHdrs->pFrom->numParams, pSipMessage->pMandatoryHdrs->pFrom->ppParams);
		}

		pForkedMsgNode->uRemoteCSeq = pSipMessage->pMandatoryHdrs->CSeq;
		pNode->uLastReceivedInviteCSeq = pSipMessage->pMandatoryHdrs->CSeq;
		pNode->pRouteSet = pSipMessage->pRecordRouteSet;
		pSipMessage->pRecordRouteSet = NULL;

		pal_MemoryAllocate(sizeof(EcrioSigMgrContactStruct), (void **)&pForkedMsgNode->pRemoteContact);
		if (pForkedMsgNode->pRemoteContact == NULL)
		{
			uError = ECRIO_SIGMGR_NO_MEMORY;
			goto EndTag;
		}

		uError = _EcrioSigMgrPopulateContact(pSigMgr, pSipMessage->pContact, pForkedMsgNode->pRemoteContact);
		if (uError != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u Failed to populate Contact header. Error = %d",
				__FUNCTION__, __LINE__, uError);
			uError = ECRIO_SIGMGR_UNKNOWN_ERROR;
			goto EndTag;
		}
	}
	else
	{
		pRemoteUri = pSipMessage->pMandatoryHdrs->pTo;
		if (pSipMessage->pMandatoryHdrs->pTo->ppParams)
		{
			pRemoteTag = _EcrioSigMgrFindTagParam(pSigMgr, pSipMessage->pMandatoryHdrs->pTo->numParams, pSipMessage->pMandatoryHdrs->pTo->ppParams);
		}

		pLocalUri = pSipMessage->pMandatoryHdrs->pFrom;
		if (pSipMessage->pMandatoryHdrs->pFrom->ppParams)
		{
			pLocalTag = _EcrioSigMgrFindTagParam(pSigMgr, pSipMessage->pMandatoryHdrs->pFrom->numParams, pSipMessage->pMandatoryHdrs->pFrom->ppParams);
		}

		pNode->uLocalCSeq = pSipMessage->pMandatoryHdrs->CSeq;
		pNode->uLastSentInviteCSeq = pSipMessage->pMandatoryHdrs->CSeq;
	}

	uError = _EcrioSigMgrAddLocalContactUri(pSigMgr, &pNode->pLocalContact, Enum_FALSE);//We do not need to copy the display name here
	if (uError != ECRIO_SIGMGR_NO_ERROR)
	{
		goto EndTag;
	}

	uError = _EcrioSigMgrAllocateAndPopulateNameAddrWithParams(pSigMgr, pLocalUri, &pNode->pLocalUri);
	if (uError != ECRIO_SIGMGR_NO_ERROR)
	{
		goto EndTag;
	}

	uError = _EcrioSigMgrAllocateAndPopulateNameAddrWithParams(pSigMgr, pRemoteUri, &pNode->pRemoteUri);
	if (uError != ECRIO_SIGMGR_NO_ERROR)
	{
		goto EndTag;
	}

	if (bIncomingRequest == Enum_TRUE)
	{
		if (pLocalTag == NULL)
		{
			_EcrioSigMgrGenerateTag(&pTag);
			uError = _EcrioSigMgrAppendParam(pSigMgr, &pLocalUri->numParams, &pLocalUri->ppParams,
				(u_char *)ECRIO_SIG_MGR_SIP_TAG_STRING, pTag);

			SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tpLocalUri->numParams=%u",
				__FUNCTION__, __LINE__, pLocalUri->numParams);

			pLocalTag = pTag;
		}
	}

	if (pLocalTag)
	{
		pNode->pLocalTag = pal_StringCreate(pLocalTag, pal_StringLength(pLocalTag));
	}

	if (pRemoteTag)
	{
		pForkedMsgNode->pRemoteTag = pal_StringCreate(pRemoteTag, pal_StringLength(pRemoteTag));
	}

	pNode->eDialogType = (pSipMessage->eMethodType == EcrioSipMessageTypeInvite) ? _EcrioSigMgrDialogType_INVITE : _EcrioSigMgrDialogType_SUBSCRIBE;

	pNode->pCallId = pal_StringCreate(pSipMessage->pMandatoryHdrs->pCallId, pal_StringLength(pSipMessage->pMandatoryHdrs->pCallId));

	pNode->pDialogContext = pDialogContext;

	pForkedMsgNode->eDialogState = _EcrioSigMgrDialogState_Initialized;

	pForkedMsgNode->bIsEarlyDialogTermination = Enum_FALSE;

	pNode->eModule = EcrioSigMgrCallbackRegisteringModule_LIMS;

	pForkedMsgNode->pNext = NULL;
	if (pNode->pForkedMsgHead == NULL)
		pNode->pForkedMsgHead = pForkedMsgNode;
	else
	{
		pForkedMsgNode->pNext = pNode->pForkedMsgHead;
		pNode->pForkedMsgHead = pForkedMsgNode;
	}
	pNode->pCurrForkedMsgNode = pForkedMsgNode;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t Dialog %u created with c-id=%s l-tag=%s r-tag=%s l-cseq=%u r-cseq=%u",
		__FUNCTION__, __LINE__, pNode->eDialogType, pNode->pCallId, pLocalTag ? pLocalTag : (u_char *)"null", pRemoteTag ? pRemoteTag : (u_char *)"null", pNode->uLocalCSeq, pForkedMsgNode->uRemoteCSeq);

	pReturnNode = pNode;
	pNode = NULL;

EndTag:

	if (NULL != pTag)
	{
		pal_MemoryFree((void **)&pTag);
	}

	if (pNode)
	{
		_EcrioSigMgrReleaseDialogNodeStruct(pSigMgr, pNode);
		pal_MemoryFree((void **)&pNode);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t uError=%u",
		__FUNCTION__, __LINE__, uError);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\tpReturnNode=%x",
		__FUNCTION__, __LINE__, pReturnNode);
	return pReturnNode;
}

/* This function will be called while sending a request or response within the dialog.*/
u_int32	_EcrioSigMgePopulateHeaderFromDialogAndUpdateState
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct	*pSipMessage,
	_EcrioSigMgrDialogNodeStruct *pDialogNode
)
{
	u_int32	uError = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrNameAddrWithParamsStruct	**ppLocalUri = NULL;
	EcrioSigMgrNameAddrWithParamsStruct	**ppRemoteUri = NULL;
	u_char *pLocalTag = NULL;
	u_char *pRemoteTag = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u pSipMessage = %x pMandatoryHdrs = %x, eMethodType = %u eReqRspType=%u pDialogNode=%x",
		__FUNCTION__, __LINE__, pSipMessage, pSipMessage->pMandatoryHdrs, pSipMessage->eMethodType, pSipMessage->eReqRspType, pDialogNode);

	if (pDialogNode == NULL || pSipMessage == NULL || pSipMessage->pMandatoryHdrs == NULL)
	{
		uError = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto EndTag;
	}
	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u pSipMessage = %x pMandatoryHdrs = %x, eMethodType = %u eReqRspType=%u pDialogNode=%x",
		__FUNCTION__, __LINE__, pSipMessage, pSipMessage->pMandatoryHdrs, pSipMessage->eMethodType, pSipMessage->eReqRspType, pDialogNode);

	if (pDialogNode->pCurrForkedMsgNode == NULL)
	{
		uError = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto EndTag;
	}

	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u At start Node's state =%u c-id = %s, l-tag = %s r-tag=%s l-cseq=%u r-cseq=%u",
		__FUNCTION__, __LINE__, pDialogNode->pCurrForkedMsgNode->eDialogState, pDialogNode->pCallId,
		pDialogNode->pLocalTag ? pDialogNode->pLocalTag : (u_char *)"null",
		pDialogNode->pCurrForkedMsgNode->pRemoteTag ? pDialogNode->pCurrForkedMsgNode->pRemoteTag : (u_char *)"null", pDialogNode->uLocalCSeq, pDialogNode->pCurrForkedMsgNode->uRemoteCSeq);

	if (pSipMessage->eReqRspType == EcrioSigMgrSIPRequest)
	{
		if (pSipMessage->eMethodType == EcrioSipMessageTypeAck || pSipMessage->eMethodType == EcrioSipMessageTypeCancel)
		{
			pSipMessage->pMandatoryHdrs->CSeq = pDialogNode->uLastSentInviteCSeq;
		}
		else
		{
			pSipMessage->pMandatoryHdrs->CSeq = ++(pDialogNode->uLocalCSeq);
		}

		ppLocalUri = &pSipMessage->pMandatoryHdrs->pFrom;
		ppRemoteUri = &pSipMessage->pMandatoryHdrs->pTo;
	}
	else
	{
		if (pSipMessage->pMandatoryHdrs->CSeq == 0)
		{
			if (pSipMessage->eMethodType == EcrioSipMessageTypeInvite)
			{
				// Initial INVITE CSeq should not be changed due to arrival of PRACK/UPDATE at early dialog state
				pSipMessage->pMandatoryHdrs->CSeq = pDialogNode->uLastReceivedInviteCSeq;
			}
			else
			{
				pSipMessage->pMandatoryHdrs->CSeq = pDialogNode->pCurrForkedMsgNode->uRemoteCSeq;
			}
		}

		ppLocalUri = &pSipMessage->pMandatoryHdrs->pTo;
		ppRemoteUri = &pSipMessage->pMandatoryHdrs->pFrom;
	}

	if (ppLocalUri != NULL && *ppLocalUri == NULL)
	{
		uError = _EcrioSigMgrAllocateAndPopulateNameAddrWithParams(pSigMgr, pDialogNode->pLocalUri, ppLocalUri);
		if (uError != ECRIO_SIGMGR_NO_ERROR)
		{
			goto EndTag;
		}
	}

	pLocalTag = _EcrioSigMgrFindTagParam(pSigMgr, (*ppLocalUri)->numParams, (*ppLocalUri)->ppParams);
	if (pLocalTag == NULL)
	{
		uError = _EcrioSigMgrAppendParam(pSigMgr, &((*ppLocalUri)->numParams), &((*ppLocalUri)->ppParams),
			(u_char *)ECRIO_SIG_MGR_SIP_TAG_STRING, pDialogNode->pLocalTag);
		if (uError != ECRIO_SIGMGR_NO_ERROR)
		{
			goto EndTag;
		}

		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t(*ppLocalUri)->numParams=%u",
			__FUNCTION__, __LINE__, (*ppLocalUri)->numParams);
	}

	if (ppRemoteUri != NULL && *ppRemoteUri == NULL)
	{
		uError = _EcrioSigMgrAllocateAndPopulateNameAddrWithParams(pSigMgr, pDialogNode->pRemoteUri, ppRemoteUri);
		if (uError != ECRIO_SIGMGR_NO_ERROR || ppRemoteUri == NULL)
		{
			goto EndTag;
		}

		pRemoteTag = _EcrioSigMgrFindTagParam(pSigMgr, (*ppRemoteUri)->numParams, (*ppRemoteUri)->ppParams);
		if (pRemoteTag == NULL && pDialogNode->pCurrForkedMsgNode->pRemoteTag != NULL)
		{
			uError = _EcrioSigMgrAppendParam(pSigMgr, &((*ppRemoteUri)->numParams), &((*ppRemoteUri)->ppParams),
				(u_char *)ECRIO_SIG_MGR_SIP_TAG_STRING, pDialogNode->pCurrForkedMsgNode->pRemoteTag);
			if (uError != ECRIO_SIGMGR_NO_ERROR)
			{
				goto EndTag;
			}

			SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t(*ppRemoteUri)->numParams=%u",
				__FUNCTION__, __LINE__, (*ppRemoteUri)->numParams);
		}
	}
	if (pSipMessage->pMandatoryHdrs->pCallId == NULL)
	{
		pSipMessage->pMandatoryHdrs->pCallId = pal_StringCreate(pDialogNode->pCallId, pal_StringLength(pDialogNode->pCallId));
	}

	if (pSipMessage->pContact == NULL &&
		(pSipMessage->eMethodType == EcrioSipMessageTypeInvite || pSipMessage->eMethodType == EcrioSipMessageTypeSubscribe ||
		 pSipMessage->eMethodType == EcrioSipMessageTypeRefer))
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrContactStruct), (void **)&pSipMessage->pContact);
		if (pSipMessage->pContact == NULL)
		{
			uError = ECRIO_SIGMGR_NO_MEMORY;
			goto EndTag;
		}

		uError = _EcrioSigMgrPopulateContact(pSigMgr, pDialogNode->pLocalContact, pSipMessage->pContact);
		if (uError != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u Failed to populate Contact header. Error = %d",
				__FUNCTION__, __LINE__, uError);
			uError = ECRIO_SIGMGR_UNKNOWN_ERROR;
			goto EndTag;
		}
	}

	if (pSipMessage->eReqRspType == EcrioSigMgrSIPRequest)
	{
		if (pSipMessage->pMandatoryHdrs->pRequestUri == NULL)
		{
			EcrioSigMgrNameAddrWithParamsStruct *pSrcAddr = NULL;

			/* Populate from remote-contact if present else from remote-uri
			*/
			if (pDialogNode->pCurrForkedMsgNode->pRemoteContact && pSipMessage->eMethodType != EcrioSipMessageTypeCancel)
			{
				if (NULL != pDialogNode->pCurrForkedMsgNode->pRemoteContact->ppContactDetails)
				{
					pSrcAddr = pDialogNode->pCurrForkedMsgNode->pRemoteContact->ppContactDetails[0];
				}
			}
			else
			{
				pSrcAddr = pDialogNode->pRemoteUri;
			}

			if (NULL != pSrcAddr)
			{
				uError = _EcrioSigMgrAllocateAndPopulateUri(pSigMgr, &(pSrcAddr->nameAddr.addrSpec), &pSipMessage->pMandatoryHdrs->pRequestUri);
				if (uError != ECRIO_SIGMGR_NO_ERROR)
				{
					goto EndTag;
				}
			}
			else
			{
				uError = ECRIO_SIGMGR_INSUFFICIENT_DATA;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\tuError=%u",
					__FUNCTION__, __LINE__, uError);
				goto EndTag;
			}
		}
	}

	if (pSipMessage->eMethodType == EcrioSipMessageTypeInvite && pSipMessage->eReqRspType == EcrioSigMgrSIPResponse)
	{
		// pSipMessage->pRecordRouteSet = pDialogNode->pRouteSet;
		if (pDialogNode->pRouteSet)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrRouteStruct), (void **)&pSipMessage->pRecordRouteSet);
			if (pSipMessage->pRecordRouteSet == NULL)
			{
				uError = ECRIO_SIGMGR_NO_MEMORY;
				goto EndTag;
			}

			_EcrioSigMgrCopyRouteSet(pSigMgr, pDialogNode->pRouteSet->numRoutes, pDialogNode->pRouteSet->ppRouteDetails,
				&pSipMessage->pRecordRouteSet->ppRouteDetails);
			pSipMessage->pRecordRouteSet->numRoutes = pDialogNode->pRouteSet->numRoutes;
		}

		if (pDialogNode->pCurrForkedMsgNode->eDialogState != _EcrioSigMgrDialogState_Confirmed)
		{
			if (pSipMessage->responseCode > 100 && pSipMessage->responseCode < 200)
			{
				pDialogNode->pCurrForkedMsgNode->eDialogState = _EcrioSigMgrDialogState_Early;
			}
			else
			{
				pDialogNode->pCurrForkedMsgNode->eDialogState = _EcrioSigMgrDialogState_Confirmed;
				if (pDialogNode->bIsFirst200OKReceived != Enum_TRUE)
					pDialogNode->pAcceptedForkedMsgNode = pDialogNode->pCurrForkedMsgNode;
			}
		}
	}
	else
	{
		// pSipMessage->pRouteSet = pDialogNode->pRouteSet;
		if (pDialogNode->pRouteSet)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrRouteStruct), (void **)&pSipMessage->pRouteSet);
			if (pSipMessage->pRouteSet == NULL)
			{
				uError = ECRIO_SIGMGR_NO_MEMORY;
				goto EndTag;
			}

			_EcrioSigMgrCopyRouteSet(pSigMgr, pDialogNode->pRouteSet->numRoutes, pDialogNode->pRouteSet->ppRouteDetails,
				&pSipMessage->pRouteSet->ppRouteDetails);
			pSipMessage->pRouteSet->numRoutes = pDialogNode->pRouteSet->numRoutes;
		}
	}

	if (pSipMessage->eMethodType == EcrioSipMessageTypeInvite && pSipMessage->eReqRspType == EcrioSigMgrSIPRequest)
	{
		pDialogNode->uLastSentInviteCSeq = pSipMessage->pMandatoryHdrs->CSeq;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u At end Node's state =%u c-id = %s, l-tag = %s r-tag=%s l-cseq=%u r-cseq=%u",
		__FUNCTION__, __LINE__, pDialogNode->pCurrForkedMsgNode->eDialogState, pDialogNode->pCallId,
		pDialogNode->pLocalTag ? pDialogNode->pLocalTag : (u_char *)"null",
		pDialogNode->pCurrForkedMsgNode->pRemoteTag ? pDialogNode->pCurrForkedMsgNode->pRemoteTag : (u_char *)"null", pDialogNode->uLocalCSeq, pDialogNode->pCurrForkedMsgNode->uRemoteCSeq);

EndTag:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\tuError=%u",
		__FUNCTION__, __LINE__, uError);

	return uError;
}

_EcrioSigMgrDialogNodeStruct *EcrioSigMgrFindMatchedDialog
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pLocalTag,
	u_char *pRemoteTag,
	u_char *pCallId,
	u_int32 CSeq,
	EcrioSipMessageTypeEnum eMethodType,
	EcrioSigMgrSIPMessageTypeEnum eReqRspType,
	BoolEnum *pDialogMatched,
	BoolEnum *pForkedRequest,
	u_int32	*pResponseCode
)
{
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	_EcrioSigMgrForkedMessageListNodeStruct *pHeadNode = NULL;

	if (pSigMgr == NULL)
	{
		return NULL;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pDialogMatched == NULL || pForkedRequest == NULL || pResponseCode == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tOne of the input argument is NULL", __FUNCTION__, __LINE__);
		return NULL;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u eMethodType = %u eReqRspType=%u",
		__FUNCTION__, __LINE__, eMethodType, eReqRspType);

	*pDialogMatched = Enum_FALSE;
	*pForkedRequest = Enum_FALSE;
	*pResponseCode = 0;

	if (pCallId == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u The CallId is NULL.", __FUNCTION__, __LINE__);
		goto Level_01;
	}

	ec_MapGetKeyData(pSigMgr->hHashMap, pCallId, (void **)&pDialogNode);

	SIGMGRLOGD(pSigMgr->pLogHandle, KLogType_Fine, "%s:%u incoming message's c-id = %s l-tag = %s r-tag=%s cseq=%u",
		__FUNCTION__, __LINE__, pCallId, pLocalTag ? pLocalTag : (u_char *)"null", pRemoteTag ? pRemoteTag : (u_char *)"null", CSeq);

	if (pDialogNode)
	{
		if (eReqRspType == EcrioSigMgrSIPRequest && pLocalTag == NULL)
		{
			if (eMethodType != EcrioSipMessageTypeCancel)
			{
				*pDialogMatched = Enum_FALSE;
				*pForkedRequest = Enum_FALSE;
				*pResponseCode = 481;	// possibly request from a restarted server where call-id matches but to-tag not available

				SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u breaking loop as the non-CANCEL request not containing any to-tag but contain matching call-id, no further match expected",
					__FUNCTION__, __LINE__);
				goto Level_01;
			}
		}

		if (pDialogNode->pForkedMsgHead == NULL)
		{
			*pDialogMatched = Enum_FALSE;
			*pForkedRequest = Enum_FALSE;
			*pResponseCode = 481;

			SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u No matching dialog exist", __FUNCTION__, __LINE__);
			goto Level_01;
		}

		pDialogNode->pCurrForkedMsgNode = NULL;
		pHeadNode = pDialogNode->pForkedMsgHead;
		if (pHeadNode && pHeadNode->pNext)
		{
			while (pHeadNode)
			{
				if (pHeadNode->pRemoteTag)
				{
					if (pal_StringCompare(pRemoteTag, pHeadNode->pRemoteTag) == 0)
					{
						pDialogNode->pCurrForkedMsgNode = pHeadNode;
						break;
					}
				}
				pHeadNode = pHeadNode->pNext;
			}
		}
		else
		{
			/* If only one message received and 2nd one was forked. */
			/* if *pForkedRequest = Enum_TRUE, caller need to create new node and insert into
			pDialogNode->pForkedMsgHead and set pDialogNode->pCurrForkedMsgNode with new node. */
			if (pHeadNode->pRemoteTag != NULL && pal_StringCompare(pRemoteTag, pHeadNode->pRemoteTag) != 0)
			{
				*pDialogMatched = Enum_TRUE;
				*pForkedRequest = Enum_TRUE;
				*pResponseCode = 0;
			}
			pDialogNode->pCurrForkedMsgNode = pHeadNode;
		}

		if (pDialogNode->pCurrForkedMsgNode && *pForkedRequest != Enum_TRUE)
		{
			if (eReqRspType == EcrioSigMgrSIPRequest &&
				!(eMethodType == EcrioSipMessageTypeAck || eMethodType == EcrioSipMessageTypeCancel) &&
				pDialogNode->pCurrForkedMsgNode->uRemoteCSeq >= CSeq)
			{
				*pDialogMatched = Enum_TRUE;
				*pForkedRequest = Enum_FALSE;
				*pResponseCode = 500;

				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u The incoming request contain lower CSec value(%u) than stored r-cseq (%u), should be responded by 500",
					__FUNCTION__, __LINE__, CSeq, pDialogNode->pCurrForkedMsgNode->uRemoteCSeq);
			}
			else
			{

				*pDialogMatched = Enum_TRUE;
				*pForkedRequest = Enum_FALSE;
				*pResponseCode = 0;
			}
		}
		else /* No matching dialog found hence request is forked. */
		{
			/* if *pForkedRequest = Enum_TRUE, caller need to create new node and insert into
			pDialogNode->pForkedMsgHead and set pDialogNode->pCurrForkedMsgNode with new node. */
			
			*pDialogMatched = Enum_TRUE;
			*pForkedRequest = Enum_TRUE;
			*pResponseCode = 0;
		}
	}
	else
	{
		*pDialogMatched = Enum_FALSE;
		*pForkedRequest = Enum_FALSE;
		*pResponseCode = 481;

		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u No matching dialog exist",
			__FUNCTION__, __LINE__);
	}

Level_01:
	if (*pDialogMatched != Enum_TRUE)
	{
		pDialogNode = NULL;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t pDialogMatched=%u bForked=%u response code=%u",
		__FUNCTION__, __LINE__, *pDialogMatched, *pForkedRequest, *pResponseCode);
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\tpDialogNode=%x",
		__FUNCTION__, __LINE__, pDialogNode);

	return pDialogNode;
}

_EcrioSigMgrDialogNodeStruct *EcrioSigMgrMatchDialogAndUpdateState
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct	*pSipMessage,
	BoolEnum *pForkedRequest,
	u_int32	*pResponseCode
)
{
	u_int32	uError = ECRIO_SIGMGR_NO_ERROR;
	BoolEnum bDialogMatched = Enum_FALSE;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	_EcrioSigMgrForkedMessageListNodeStruct *pForkedMsgNode;
	u_char *pLocalTag = NULL;
	u_char *pRemoteTag = NULL;
	EcrioSigMgrRouteStruct *pRouteSet = NULL;
	u_int16	i = 0;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u pSipMessage = %x pMandatoryHdrs = %x, eMethodType = %u eReqRspType=%u ",
		__FUNCTION__, __LINE__, pSipMessage, pSipMessage->pMandatoryHdrs, pSipMessage->eMethodType, pSipMessage->eReqRspType);

	if (pSipMessage->eReqRspType == EcrioSigMgrSIPRequest)
	{
		if (pSipMessage->pMandatoryHdrs->pTo->ppParams)
		{
			pLocalTag = _EcrioSigMgrFindTagParam(pSigMgr, pSipMessage->pMandatoryHdrs->pTo->numParams, pSipMessage->pMandatoryHdrs->pTo->ppParams);
		}

		if (pSipMessage->pMandatoryHdrs->pFrom->ppParams)
		{
			pRemoteTag = _EcrioSigMgrFindTagParam(pSigMgr, pSipMessage->pMandatoryHdrs->pFrom->numParams, pSipMessage->pMandatoryHdrs->pFrom->ppParams);
		}
	}
	else
	{
		if (pSipMessage->pMandatoryHdrs->pTo->ppParams)
		{
			pRemoteTag = _EcrioSigMgrFindTagParam(pSigMgr, pSipMessage->pMandatoryHdrs->pTo->numParams, pSipMessage->pMandatoryHdrs->pTo->ppParams);
		}

		if (pSipMessage->pMandatoryHdrs->pFrom->ppParams)
		{
			pLocalTag = _EcrioSigMgrFindTagParam(pSigMgr, pSipMessage->pMandatoryHdrs->pFrom->numParams, pSipMessage->pMandatoryHdrs->pFrom->ppParams);
		}
	}

	pDialogNode = EcrioSigMgrFindMatchedDialog(pSigMgr, pLocalTag, pRemoteTag, pSipMessage->pMandatoryHdrs->pCallId,
		pSipMessage->pMandatoryHdrs->CSeq, pSipMessage->eMethodType, pSipMessage->eReqRspType, &bDialogMatched,
		pForkedRequest, pResponseCode);

	/* Check if the message is forked and create new node. */
	if (pDialogNode != NULL && *pForkedRequest == Enum_TRUE)
	{
		pal_MemoryAllocate(sizeof(_EcrioSigMgrForkedMessageListNodeStruct), (void **)&pForkedMsgNode);
		if (pForkedMsgNode == NULL)
		{
			// uError = ECRIO_SIGMGR_NO_MEMORY;
			goto EndTag;
		}

		pForkedMsgNode->pNext = pDialogNode->pForkedMsgHead;
		pDialogNode->pForkedMsgHead = pForkedMsgNode;
		pDialogNode->pCurrForkedMsgNode = pForkedMsgNode;
	}

	if (pDialogNode && pDialogNode->pCurrForkedMsgNode && bDialogMatched == Enum_TRUE && *pResponseCode == 0)
	{
		if (pSipMessage->eReqRspType == EcrioSigMgrSIPRequest)
		{
			if (pSipMessage->eMethodType != EcrioSipMessageTypeAck && pSipMessage->eMethodType != EcrioSipMessageTypeCancel)
			{
				pDialogNode->pCurrForkedMsgNode->uRemoteCSeq = pSipMessage->pMandatoryHdrs->CSeq;
			}

			if (pSipMessage->eMethodType == EcrioSipMessageTypeInvite)
			{
				if (pDialogNode->pCurrForkedMsgNode->pRemoteContact)
				{
					_EcrioSigMgrReleaseContactStruct(pSigMgr, pDialogNode->pCurrForkedMsgNode->pRemoteContact);
				}
				else
				{
					pal_MemoryAllocate(sizeof(EcrioSigMgrContactStruct), (void **)&pDialogNode->pCurrForkedMsgNode->pRemoteContact);
					if (pDialogNode->pCurrForkedMsgNode->pRemoteContact == NULL)
					{
						// uError = ECRIO_SIGMGR_NO_MEMORY;
						goto EndTag;
					}
				}

				uError = _EcrioSigMgrPopulateContact(pSigMgr, pSipMessage->pContact, pDialogNode->pCurrForkedMsgNode->pRemoteContact);
				if (uError != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u Failed to populate Contact header. Error = %d",
						__FUNCTION__, __LINE__, uError);
					goto EndTag;
				}

				pDialogNode->uLastReceivedInviteCSeq = pSipMessage->pMandatoryHdrs->CSeq;
			}
		}
		else
		{
			if (pSipMessage->eMethodType == EcrioSipMessageTypeInvite ||
				pSipMessage->eMethodType == EcrioSipMessageTypeSubscribe)
			{
				if (pSipMessage->pContact)
				{
					if (pDialogNode->pCurrForkedMsgNode->pRemoteContact)
					{
						_EcrioSigMgrReleaseContactStruct(pSigMgr, pDialogNode->pCurrForkedMsgNode->pRemoteContact);
					}
					else
					{
						pal_MemoryAllocate(sizeof(EcrioSigMgrContactStruct), (void **)&pDialogNode->pCurrForkedMsgNode->pRemoteContact);
						if (pDialogNode->pCurrForkedMsgNode->pRemoteContact == NULL)
						{
							// uError = ECRIO_SIGMGR_NO_MEMORY;
							goto EndTag;
						}
					}

					uError = _EcrioSigMgrPopulateContact(pSigMgr, pSipMessage->pContact, pDialogNode->pCurrForkedMsgNode->pRemoteContact);
					if (uError != ECRIO_SIGMGR_NO_ERROR)
					{
						SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u Failed to populate Contact header. Error = %d",
							__FUNCTION__, __LINE__, uError);
						goto EndTag;
					}
				}

				if (pSipMessage->eMethodType == EcrioSipMessageTypeInvite)
				{
					pDialogNode->uLastReceivedInviteCSeq = pSipMessage->pMandatoryHdrs->CSeq;
				}

				if (pDialogNode->pCurrForkedMsgNode->eDialogState != _EcrioSigMgrDialogState_Confirmed)
				{
					if (pDialogNode->pRouteSet)
					{
						_EcrioSigMgrReleaseRouteStruct(pSigMgr, pDialogNode->pRouteSet);
						pal_MemoryFree((void **)&pDialogNode->pRouteSet);
					}

					pRouteSet = pSipMessage->pRecordRouteSet;
					if (pRouteSet)
					{
						for (i = 0; i < pRouteSet->numRoutes / 2; i++)
						{
							EcrioSigMgrNameAddrWithParamsStruct *pTempRecordRoute = NULL;
							pTempRecordRoute = pRouteSet->ppRouteDetails[i];
							pRouteSet->ppRouteDetails[i] = pRouteSet->ppRouteDetails[pRouteSet->numRoutes - 1 - i];
							pRouteSet->ppRouteDetails[pRouteSet->numRoutes - 1 - i] = pTempRecordRoute;
						}
					}

					pSipMessage->pRecordRouteSet = NULL;
					pDialogNode->pRouteSet = pRouteSet;
					pRouteSet = NULL;

					if (pDialogNode->pCurrForkedMsgNode->eDialogState == _EcrioSigMgrDialogState_Initialized &&
						pSipMessage->responseCode > 100 &&
						pDialogNode->pCurrForkedMsgNode->pRemoteTag == NULL && pRemoteTag != NULL)
					{
						pDialogNode->pCurrForkedMsgNode->pRemoteTag = pal_StringCreate(pRemoteTag, pal_StringLength(pRemoteTag));
					}

					if (pSipMessage->responseCode > 100 && pSipMessage->responseCode < 200)
					{
						pDialogNode->pCurrForkedMsgNode->eDialogState = _EcrioSigMgrDialogState_Early;
					}
					else if (pSipMessage->responseCode >= 200)
					{
						pDialogNode->pCurrForkedMsgNode->eDialogState = _EcrioSigMgrDialogState_Confirmed;
					}
				}
			}
		}

		SIGMGRLOGD(pSigMgr->pLogHandle, KLogType_Fine, "%s:%u Dialog matched and state =%u c-id = %s, l-tag = %s r-tag=%s l-cseq=%u r-cseq=%u",
			__FUNCTION__, __LINE__, pDialogNode->pCurrForkedMsgNode->eDialogState, pDialogNode->pCallId,
			pDialogNode->pLocalTag ? pDialogNode->pLocalTag : (u_char *)"null",
			pDialogNode->pCurrForkedMsgNode->pRemoteTag ? pDialogNode->pCurrForkedMsgNode->pRemoteTag : (u_char *)"null", pDialogNode->uLocalCSeq, pDialogNode->pCurrForkedMsgNode->uRemoteCSeq);
	}

	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t bForked=%u resppose code=%u",
		__FUNCTION__, __LINE__, *pForkedRequest, *pResponseCode);
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\tpDialogNode=%x",
		__FUNCTION__, __LINE__, pDialogNode);

EndTag:
	return pDialogNode;
}

u_int32	_EcrioSigMgrDeleteDialog
(
EcrioSigMgrStruct *pSigMgr,
_EcrioSigMgrDialogNodeStruct *pDialogNode
)
{
	u_int32	uError = ECRIO_SIGMGR_NO_ERROR;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u pDialogNode = %x",
		__FUNCTION__, __LINE__, pDialogNode);

	if (pDialogNode == NULL)
	{
		uError = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto EndTag;
	}

	/* Transactions are deleted if only TCP enabled */
	if (pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE &&
		pSigMgr->pSignalingInfo->bEnableUdp == Enum_FALSE)
	{
			EcrioTXNReturnCodeEnum eTxnErr = EcrioTXNNoError;
			eTxnErr = EcrioTXNDeleteMatchedTransactions(pSigMgr->pTransactionMgrHndl,
				pDialogNode->pCallId);
			SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioTXNDeleteMatchedTransactions() returned with = %d", __FUNCTION__, __LINE__, eTxnErr);
	}

	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tDelete Key Data, pCallId = %x",
		 __FUNCTION__, __LINE__, pDialogNode->pCallId);
	ec_MapDeleteKeyData(pSigMgr->hHashMap, pDialogNode->pCallId);
	_EcrioSigMgrReleaseDialogNodeStruct(pSigMgr, pDialogNode);
	pal_MemoryFree((void **)&pDialogNode);

EndTag:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\tuError=%u",
		__FUNCTION__, __LINE__, uError);

	return uError;
}
