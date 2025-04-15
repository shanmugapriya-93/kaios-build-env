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
#include "EcrioSigMgrCallbacks.h"
#include "EcrioSigMgrIMSLibHandler.h"
#include "EcrioSigMgrTransactionHandler.h"
#include "EcrioDSListInterface.h"
#include "EcrioSigMgrPublishInternal.h"
#include "EcrioCPMDataTypes.h"

/*
+-----------+-------+---------------+---------------+
| Operation | Body? | SIP-If-Match? | Expires Value |
+-----------+-------+---------------+---------------+
| Initial   | yes   | no            | > 0           |
| Refresh   | no    | yes           | > 0           |
| Modify    | yes   | yes           | > 0           |
| Remove    | no    | yes           | 0             |
+-----------+-------+---------------+---------------+
*/

u_int32 _EcrioSigMgrRespValidateStateTakeAction
(
EcrioSigMgrStruct *pSigMgr,
EcrioSigMgrSipMessageStruct *pMessageResp,
EcrioSigMgrPublishEvents ePubEvents,
EcrioSigMgrSipMessageStruct *pSendMssg
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	_EcrioSigMgrInternalSignalingInfoStruct *pSigInfo = (_EcrioSigMgrInternalSignalingInfoStruct*)pSigMgr->pSignalingInfo;
	EcrioSigMgrPublishInfo* pPubInfo = (EcrioSigMgrPublishInfo*)pSigInfo->pPublishInfo;

	if (pPubInfo == NULL)
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;

	switch (ePubEvents)
	{
	case ECRIO_SIG_MGR_PUBLISH_EVENT_2xx_Received:
	{
		switch (pPubInfo->eState)
		{
		case ECRIO_SIG_MGR_PUBLISH_STATE_Initial:
		case ECRIO_SIG_MGR_PUBLISH_STATE_Refresh:
		case ECRIO_SIG_MGR_PUBLISH_STATE_Modify:
		{
			EcrioSigMgrHeaderStruct *pEtagHeader = NULL;
			EcrioSigMgrGetOptionalHeader(pSigMgr, pMessageResp->pOptionalHeaderList,
				EcrioSipHeaderTypeSipEtag, &pEtagHeader);
			if (pEtagHeader)
			{
				if (pPubInfo->SipETag)
					pal_MemoryFree((void**)&pPubInfo->SipETag);
				// As Etag shud have one header value index 0 used here
				_EcrioSigMgrStringCreate(pSigMgr, pEtagHeader->ppHeaderValues[0]->pHeaderValue, &pPubInfo->SipETag);

				pMessageResp->pSipETag = pPubInfo->SipETag;

			}
			if (pMessageResp->pExpires)
				pPubInfo->minExpires = *(pMessageResp->pExpires);
			pPubInfo->eState = ECRIO_SIG_MGR_PUBLISH_STATE_Established;
			_EcrioSigMgrStartRePublishTimer(pSigMgr, pMessageResp, pSigMgr->pSignalingInfo->pPublishInfo);
		}
		break;
		case ECRIO_SIG_MGR_PUBLISH_STATE_Remove:
		{
			if (pPubInfo->bIsPubExpireTimerRunning == Enum_TRUE)
			{
				pal_TimerStop(pPubInfo->uPubExpireTimerID);
				pPubInfo->uPubExpireTimerID = NULL;
				pPubInfo->bIsPubExpireTimerRunning = Enum_FALSE;
			}

			_EcrioSigMgrPublishInfoStruct(&pPubInfo);
			pSigInfo->pPublishInfo = NULL;
		}
		break;
		default:
		{
		}
		}
	}
	break;
	case ECRIO_SIG_MGR_PUBLISH_EVENT_412_Received:
	{
		switch (pPubInfo->eState)
		{
		case ECRIO_SIG_MGR_PUBLISH_STATE_Initial: // However its not possible to get this response in this state TODO - Bivash
		case ECRIO_SIG_MGR_PUBLISH_STATE_Refresh:
		case ECRIO_SIG_MGR_PUBLISH_STATE_Modify:
		{
			if (pPubInfo)
			{
				if (pPubInfo && pMessageResp->pMinExpires)
					pPubInfo->minExpires = *(pMessageResp->pMinExpires);
				if (pPubInfo->SipETag)
				{
					pal_MemoryFree((void**)&pPubInfo->SipETag);
					pPubInfo->SipETag = NULL;
				}



				EcrioSigMgrMessageBodyStruct msgBody = { .messageBodyType = EcrioSigMgrMessageBodyUnknown };
				EcrioSigMgrUnknownMessageBodyStruct unknownMsgBody = { 0 };
				EcrioSigMgrSipMessageStruct messageReq = { .eMethodType = EcrioSipMessageTypeNone };
				EcrioSigMgrSipMessageStruct *pMessageReq = NULL;

				unknownMsgBody.bufferLength = pPubInfo->uMssgLen;
				unknownMsgBody.pBuffer = pPubInfo->pMssgBody;
				unknownMsgBody.contentType.numParams = 0;
				unknownMsgBody.contentType.ppParams = NULL;
				unknownMsgBody.contentType.pHeaderValue = (u_char*)"application/pidf+xml";
				messageReq.pMessageBody = &msgBody;
				msgBody.messageBodyType = EcrioSigMgrMessageBodyUnknown;
				msgBody.pMessageBody = &unknownMsgBody;

				messageReq.eModuleId = EcrioSigMgrCallbackRegisteringModule_UCE;
				_EcrioSigMgrSendPublish(pSigMgr, &messageReq);


			}
		}
		break;
		case ECRIO_SIG_MGR_PUBLISH_STATE_Remove:
		{
			if (pPubInfo->bIsPubExpireTimerRunning == Enum_TRUE)
			{
				pal_TimerStop(pPubInfo->uPubExpireTimerID);
				pPubInfo->uPubExpireTimerID = NULL;
				pPubInfo->bIsPubExpireTimerRunning = Enum_FALSE;
			}
			_EcrioSigMgrPublishInfoStruct(&pPubInfo);
			pSigInfo->pPublishInfo = NULL;
		}
		break;
		default: 
		{
		}
		}
	}
	break;
	case ECRIO_SIG_MGR_PUBLISH_EVENT_423_Received:
	{
		switch (pPubInfo->eState)
		{
		case ECRIO_SIG_MGR_PUBLISH_STATE_Initial:
		case ECRIO_SIG_MGR_PUBLISH_STATE_Refresh:
		case ECRIO_SIG_MGR_PUBLISH_STATE_Modify:
		{
			if (pPubInfo)
			{
				if (pPubInfo && pMessageResp->pMinExpires){
					pPubInfo->minExpires = *(pMessageResp->pMinExpires);

					pSigInfo->publishExpires = *(pMessageResp->pMinExpires);

				}



				EcrioSigMgrMessageBodyStruct msgBody = { .messageBodyType = EcrioSigMgrMessageBodyUnknown };
				EcrioSigMgrUnknownMessageBodyStruct unknownMsgBody = { 0 };
				EcrioSigMgrSipMessageStruct messageReq = { .eMethodType = EcrioSipMessageTypeNone };
				EcrioSigMgrSipMessageStruct *pMessageReq = NULL;

				unknownMsgBody.bufferLength = pPubInfo->uMssgLen;
				unknownMsgBody.pBuffer = pPubInfo->pMssgBody;
				unknownMsgBody.contentType.numParams = 0;
				unknownMsgBody.contentType.ppParams = NULL;
				unknownMsgBody.contentType.pHeaderValue = (u_char*)"application/pidf+xml";
				messageReq.pMessageBody = &msgBody;
				msgBody.messageBodyType = EcrioSigMgrMessageBodyUnknown;
				msgBody.pMessageBody = &unknownMsgBody;

				messageReq.eModuleId = EcrioSigMgrCallbackRegisteringModule_UCE;
				_EcrioSigMgrSendPublish(pSigMgr, &messageReq);
			}
		}
		break;
		case ECRIO_SIG_MGR_PUBLISH_STATE_Remove:
		{
			if (pPubInfo->bIsPubExpireTimerRunning == Enum_TRUE)
			{
				pal_TimerStop(pPubInfo->uPubExpireTimerID);
				pPubInfo->uPubExpireTimerID = NULL;
				pPubInfo->bIsPubExpireTimerRunning = Enum_FALSE;
			}

			_EcrioSigMgrPublishInfoStruct(&pPubInfo);
			pSigInfo->pPublishInfo = NULL;
		}
		break;
		default:
		{
		}
		}
	}
	break;
	case ECRIO_SIG_MGR_PUBLISH_EVENT_4xx_6xx_Received:
	{
		switch (pPubInfo->eState)
		{
		case ECRIO_SIG_MGR_PUBLISH_STATE_Initial:
		{
			_EcrioSigMgrPublishInfoStruct(&pPubInfo);
			pSigInfo->pPublishInfo = NULL;
			error = ECRIO_SIGMGR_INVALID_STATUS_CODE;
		}
		break;
		default:
		{
			_EcrioSigMgrPublishInfoStruct(&pPubInfo);
			pSigInfo->pPublishInfo = NULL;
		}
		}
	}
	break;
	default:
		break;
	}
	return error;

}

u_int32 _EcrioSigMgrRequValidateStateTakeAction
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pMessageReq,
	EcrioSigMgrPublishEvents ePubEvents
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	_EcrioSigMgrInternalSignalingInfoStruct *pSigInfo = (_EcrioSigMgrInternalSignalingInfoStruct*)pSigMgr->pSignalingInfo;
	EcrioSigMgrPublishInfo* pPubInfo = NULL;

	switch (ePubEvents)
	{
	case ECRIO_SIG_MGR_PUBLISH_EVENT_Initiate:
	{
		if (pSigInfo->pPublishInfo)
		{
			return ECRIO_SIGMGR_INVALID_REQUEST;
		}
		else
		{
			EcrioSigMgrUnknownMessageBodyStruct *pUnMssgBody = (EcrioSigMgrUnknownMessageBodyStruct *)((EcrioSigMgrMessageBodyStruct *)pMessageReq->pMessageBody)->pMessageBody;;
			pal_MemoryAllocate(sizeof(EcrioSigMgrPublishInfo), (void**)&pPubInfo);
			if (pPubInfo == NULL)
			{
				return ECRIO_SIGMGR_NO_MEMORY;
			}
			pPubInfo->bIsPubExpireTimerRunning = Enum_FALSE;
			pPubInfo->minExpires = 0;
			pPubInfo->pMssgBody = pUnMssgBody->pBuffer;
			pPubInfo->pSigMgr = pSigMgr;
			pPubInfo->SipETag = NULL;
			pPubInfo->uMssgLen = pUnMssgBody->bufferLength;
			pPubInfo->uPubExpireTimerID = NULL;
			pPubInfo->eState = ECRIO_SIG_MGR_PUBLISH_STATE_Initial;
			pSigInfo->pPublishInfo = pPubInfo;

		}
	}
	break;
	case ECRIO_SIG_MGR_PUBLISH_EVENT_Modify:
	{
		EcrioSigMgrPublishInfo* pPubInfo = (EcrioSigMgrPublishInfo*)pSigInfo->pPublishInfo;

		if (pPubInfo != NULL)
		{
			switch (pPubInfo->eState)
			{
			case ECRIO_SIG_MGR_PUBLISH_STATE_Established:
			case ECRIO_SIG_MGR_PUBLISH_STATE_Modify:
			case ECRIO_SIG_MGR_PUBLISH_STATE_Remove:
			case ECRIO_SIG_MGR_PUBLISH_STATE_Refresh:
			{
				EcrioSigMgrUnknownMessageBodyStruct *pUnMssgBody = (EcrioSigMgrUnknownMessageBodyStruct *)((EcrioSigMgrMessageBodyStruct *)pMessageReq->pMessageBody)->pMessageBody;
				pPubInfo->eState = ECRIO_SIG_MGR_PUBLISH_STATE_Modify;
				if (pPubInfo->pMssgBody)
				{
					pal_MemoryFree((void**)&pPubInfo->pMssgBody);
				}
				pPubInfo->pMssgBody = NULL;
				pPubInfo->uMssgLen = 0;

				pPubInfo->pMssgBody = pUnMssgBody->pBuffer;
				pPubInfo->uMssgLen = pUnMssgBody->bufferLength;



				if (pMessageReq->pSipETag != NULL)
				{
					/* If the E-Tag does not match , the initial PUBLISH will be sent */

					if (pal_StringCompare((u_char *)pPubInfo->SipETag,(u_char*)pMessageReq->pSipETag) != 0)
					{
						if (pPubInfo->SipETag)
							pal_MemoryFree((void**)&pPubInfo->SipETag);

						pPubInfo->SipETag = NULL;

					}
				}


			}
			break;
			case ECRIO_SIG_MGR_PUBLISH_STATE_None:
			case ECRIO_SIG_MGR_PUBLISH_STATE_Initial:
			default:
			{
				return ECRIO_SIGMGR_INVALID_REQUEST;
			}
			break;
			}
		}
		else /* The scenario - when E-Tag is passed to UCE but no PUBLISH was exists , i.e. first time publish with some E-Tag value */
		{

			EcrioSigMgrUnknownMessageBodyStruct *pUnMssgBody = (EcrioSigMgrUnknownMessageBodyStruct *)((EcrioSigMgrMessageBodyStruct *)pMessageReq->pMessageBody)->pMessageBody;;
			pal_MemoryAllocate(sizeof(EcrioSigMgrPublishInfo), (void**)&pPubInfo);
			if (pPubInfo == NULL)
			{
				return ECRIO_SIGMGR_NO_MEMORY;
			}
			pPubInfo->bIsPubExpireTimerRunning = Enum_FALSE;
			pPubInfo->minExpires = 0;
			pPubInfo->pMssgBody = pUnMssgBody->pBuffer;
			pPubInfo->pSigMgr = pSigMgr;
		
			pPubInfo->SipETag = NULL;

			if (pMessageReq->pSipETag != NULL)
			{
				_EcrioSigMgrStringCreate(pSigMgr, pMessageReq->pSipETag, &pPubInfo->SipETag);
			}
			
			
			pPubInfo->uMssgLen = pUnMssgBody->bufferLength;
			pPubInfo->uPubExpireTimerID = NULL;
			pPubInfo->eState = ECRIO_SIG_MGR_PUBLISH_STATE_Initial;
			pSigInfo->pPublishInfo = pPubInfo;

		}
		

		
	}
	break;
	case ECRIO_SIG_MGR_PUBLISH_EVENT_Remove:
	{
		EcrioSigMgrPublishInfo* pPubInfo = (EcrioSigMgrPublishInfo*)pSigInfo->pPublishInfo;
		if (pPubInfo != NULL)
		{
			switch (pPubInfo->eState)
			{
			case ECRIO_SIG_MGR_PUBLISH_STATE_Established:
			case ECRIO_SIG_MGR_PUBLISH_STATE_Modify:
			case ECRIO_SIG_MGR_PUBLISH_STATE_Remove:
			case ECRIO_SIG_MGR_PUBLISH_STATE_Refresh:
			{
				pPubInfo->eState = ECRIO_SIG_MGR_PUBLISH_STATE_Remove;
				if (pPubInfo->pMssgBody)
				{
					pal_MemoryFree((void**)&pPubInfo->pMssgBody);
				}
				pPubInfo->pMssgBody = NULL;
				pPubInfo->uMssgLen = 0;
			}
			break;
			case ECRIO_SIG_MGR_PUBLISH_STATE_None:
			case ECRIO_SIG_MGR_PUBLISH_STATE_Initial:
			default:
			{
				return ECRIO_SIGMGR_INVALID_REQUEST;
			}
			break;
			}

		}
		else
			return ECRIO_SIGMGR_INVALID_REQUEST;

	}
	break;
	case ECRIO_SIG_MGR_PUBLISH_EVENT_Refresh:
	{
		EcrioSigMgrPublishInfo* pPubInfo = (EcrioSigMgrPublishInfo*)pSigInfo->pPublishInfo;
		if (pPubInfo != NULL)
		{
			switch (pPubInfo->eState)
			{
			case ECRIO_SIG_MGR_PUBLISH_STATE_Established:
			{
				pPubInfo->eState = ECRIO_SIG_MGR_PUBLISH_STATE_Refresh;
			}
			break;
			case ECRIO_SIG_MGR_PUBLISH_STATE_None:
			case ECRIO_SIG_MGR_PUBLISH_STATE_Initial:
			case ECRIO_SIG_MGR_PUBLISH_STATE_Modify:
			case ECRIO_SIG_MGR_PUBLISH_STATE_Remove:
			default:
			{
				return ECRIO_SIGMGR_INVALID_REQUEST;
			}
			break;
			}

		}
		else
			return ECRIO_SIGMGR_INVALID_REQUEST;
	}
	break;
	default:
		return ECRIO_SIGMGR_INVALID_REQUEST;
	}

	return error;
}

/*****************************************************************************
                    API Definition Section - Begin
*****************************************************************************/

/*****************************************************************************

  Function:		_EcrioSigMgrSendPublish()

  Purpose:		Initiates for publishing capabilities of the UA.

  Description:	Creates and sends a Message request.

  Input:		EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                EcrioSigMgrSipMessageStruct* pMessageReq - Message
                request Headers.

  Input/OutPut: SIGSESSIONHANDLE* pSessionHandle - Handle to session.

  Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrSendPublish
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pMessageReq
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 reqLen = 0, pubExpiry = 0;
	u_char *pReqData = NULL;
	EcrioSigMgrMandatoryHeaderStruct *pMandatoryHdrs = NULL;
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;
	EcrioSigMgrCommonInfoStruct cmnInfo = { 0 };
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };
	EcrioSigMgrPublishInfo* pPubInfo = NULL;
	u_int32 *pExpires = NULL;
	u_char *pAcceptValue = (u_char *)ECRIO_SIG_MGR_HEADER_VALUE_ACCEPT;

	//_EcrioSigMgrInternalSignalingInfoStruct *pSigInfo = (_EcrioSigMgrInternalSignalingInfoStruct*)pSigMgr->pSignalingInfo;
	pPubInfo = (EcrioSigMgrPublishInfo*)pSigMgr->pSignalingInfo->pPublishInfo;
	pubExpiry = pSigMgr->pSignalingInfo->publishExpires;
	
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);
	pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void**)&pMessageReq->pMandatoryHdrs);
	if (pMessageReq->pMandatoryHdrs == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}
	pMandatoryHdrs = pMessageReq->pMandatoryHdrs;
	
	if (EcrioSigMgrGetDefaultPUID(pSigMgr, EcrioSigMgrURISchemeTEL, &pMandatoryHdrs->pTo) != ECRIO_SIGMGR_NO_ERROR)
	{
		if (EcrioSigMgrGetDefaultPUID(pSigMgr, EcrioSigMgrURISchemeSIP, &pMandatoryHdrs->pTo) != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrGetDefaultPUID() returned error", __FUNCTION__, __LINE__);
			goto Error_Level_01;
		}
	}
	pal_MemoryAllocate(sizeof(EcrioSigMgrUriStruct), (void **)&pMandatoryHdrs->pRequestUri);
	if (pMandatoryHdrs->pRequestUri == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	error = _EcrioSigMgrPopulateUri(pSigMgr, &pMandatoryHdrs->pTo->nameAddr.addrSpec,
		pMandatoryHdrs->pRequestUri);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateUri() populate request uri, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}
	error = _EcrioSigMgrUpdateMessageRequestStruct(pSigMgr, pMessageReq,Enum_TRUE);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpMandatoryHdrs() Update Message info object with upper layer data, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}
	_EcrioSigMgrAddPublishEventHdr(pSigMgr, pMessageReq->pOptionalHeaderList);

	/* Add P-Access-Network-Info */
	if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypePublish, EcrioSipHeaderTypeP_AccessNetworkInfo) == Enum_TRUE)
	{
		error = _EcrioSigMgrAddP_AccessNWInfoHeader(pSigMgr, pMessageReq->pOptionalHeaderList);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddP_AccessNWInfoHeader() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}


	/* Add P-Last-Access-Network-Info */
	if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypePublish, EcrioSipHeaderTypeP_LastAccessNetworkInfo) == Enum_TRUE)
	{
		error = _EcrioSigMgrAddP_LastAccessNWInfoHeader(pSigMgr, pMessageReq->pOptionalHeaderList);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSigMgrAddP_LastAccessNWInfoHeader() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	/* Add Accept header */	
	error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pMessageReq->pOptionalHeaderList, EcrioSipHeaderTypeAccept, 1, &pAcceptValue, 0, NULL, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

#ifdef ENABLE_MTK //Adding Require and Proxy-Require with sec-agree 
	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\tTemp Fix - Adding Require and Proxy-Require with sec-agree",
		__FUNCTION__, __LINE__);

	if (pSigMgr->bIPSecEnabled == Enum_FALSE)
	{
		SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tTemp Fix - Adding Require with sec-agree",
			__FUNCTION__, __LINE__);
		u_char *pRequire = (u_char *)ECRIO_SIG_MGR_HEADER_VALUE_SEC_AGREE;

		if (pMessageReq->pOptionalHeaderList == NULL)
		{
			/* Create optional header object */
			error = EcrioSigMgrCreateOptionalHeaderList(pSigMgr, &pMessageReq->pOptionalHeaderList);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrCreateOptionalHeaderList() error=%u",
					__FUNCTION__, __LINE__, error);

				goto Error_Level_01;
			}
		}

		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pMessageReq->pOptionalHeaderList, EcrioSipHeaderTypeRequire, 1, &pRequire, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}

		SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tTemp Fix - Adding Proxy-Require with sec-agree",
			__FUNCTION__, __LINE__);
		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pMessageReq->pOptionalHeaderList, EcrioSipHeaderTypeProxyRequire, 1, &pRequire, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}
	}
#endif

	if (pPubInfo)
	{
		if (pPubInfo->SipETag)
			_EcrioSigMgrAddPublishIfMatchHdr(pSigMgr, pMessageReq->pOptionalHeaderList, pPubInfo->SipETag);
#if 0
		pal_MemoryAllocate(sizeof(u_int32), (void **)&pExpires);
		if (pExpires == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pExpires, error=%u",
				__FUNCTION__, __LINE__, error);
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}
		if (pPubInfo->eState != ECRIO_SIG_MGR_PUBLISH_STATE_Remove)
			*pExpires = (pubExpiry <= 0) ? _ECRIO_PUBLISH_EXPIRY : pubExpiry;
		else
			*pExpires = 0;
		pMessageReq->pExpires = pExpires;
		pExpires = NULL;
#else
		/* Include Expires=0 only for Delete publish */
		if (pPubInfo->eState == ECRIO_SIG_MGR_PUBLISH_STATE_Remove)
		{
			pal_MemoryAllocate(sizeof(u_int32), (void **)&pMessageReq->pExpires);
			if (pMessageReq->pExpires == NULL)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						   "%s:%u\tpal_MemoryAllocate() for pExpires, error=%u",
						   __FUNCTION__, __LINE__, error);
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			*(pMessageReq->pExpires) = 0;
		}
		else
			pMessageReq->pExpires = NULL;
#endif
	}
	++(pMandatoryHdrs->CSeq);
	/*	Support for Authorization header in all request	start	*/
	error = _EcrioSigMgrCopyCredentials(pSigMgr,
		(u_char *)ECRIO_SIG_MGR_METHOD_MESSAGE,
		NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrCopyCredentials() Create credentials for MESSAGE request, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}
	/*	Support for Authorization header in all request	end	*/
	error = _EcrioSigMgrPopulateMandatoryHeaders(pSigMgr, pMessageReq,
		EcrioSigMgrSIPRequest, Enum_FALSE, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateMandatoryHeaders() for pIMSManHdrs, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	if (pMessageReq->pContact == NULL)
	{
		error = _EcrioSigMgrAddLocalContactUriWithoutFeatureTags(pSigMgr, &pMessageReq->pContact);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddLocalContactUri() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}		
	}

	/** Add P-PreferredIdentity header */
	error = _EcrioSigMgrAddP_PreferrdIdHeader(pSigMgr, pMessageReq);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddP_preferredIdHeader() error=%u",
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
	pMessageReq->pRouteSet = _EcrioSigMgrAddRouteHeader(pSigMgr);
	pMessageReq->eMethodType = EcrioSipMessageTypePublish;
	pMessageReq->eReqRspType = EcrioSigMgrSIPRequest;
	/* Construct Message request */
	error = _EcrioSigMgrSipMessageForm(pSigMgr, EcrioSigMgrSipMessageModeFullName, pMessageReq, &pReqData, &reqLen);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSipMessageForm() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_IMS_LIB_ERROR;
		goto Error_Level_01;
	}
	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeSIP,
		"%s:%u\tOutgoing SIP message constructed:", __FUNCTION__, __LINE__);
	SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pReqData, reqLen);
	cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_PUBLISH;
	cmnInfo.role = EcrioSigMgrRoleUAC;
	txnInfo.currentContext = ECRIO_SIGMGR_MESSAGE_REQUEST;
	txnInfo.retransmitContext = ECRIO_SIGMGR_MESSAGE_REQUEST_RESEND;
	txnInfo.deleteflag = Enum_FALSE;
	txnInfo.messageType = EcrioSigMgrMessageNonInviteRequest;

	if (reqLen > pSigMgr->pSignalingInfo->uUdpMtu)
		txnInfo.transport = EcrioSigMgrTransportTCP;
	else
		txnInfo.transport = eTransportType;


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


	txnInfo.transport = eTransportType;
	txnInfo.pMsg = pReqData;
	txnInfo.msglen = reqLen;
	txnInfo.pSessionMappingStr = NULL;

	error = _EcrioSigMgrSendMsgToTXNHandler(pSigMgr, &cmnInfo, &txnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendMsgToTXNHandler() error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_02;
	}
	/*Module routing support*/
	_EcrioSigMgrStoreModuleRoutingInfo(pSigMgr, pMessageReq->pMandatoryHdrs->pCallId, pMessageReq->eModuleId);
	pPubInfo = (EcrioSigMgrPublishInfo*)pSigMgr->pSignalingInfo->pPublishInfo;
Error_Level_02:
	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);
	if (pReqData != NULL)
	{
		pal_MemoryFree((void **)&pReqData);
		pReqData = NULL;
		reqLen = 0;
	}
Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}
/*****************************************************************************

  Function:		_EcrioSigMgrHandlePublishResponse()

  Purpose:		Handles the Publlish respnse

  Description:	Handles an incoming Message response.

  Input:		EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                EcrioSigMgrMessageResponseStruct* pMessageRespNtf - Message
                response Headers.
                EcrioSigMgrCommonInfoStruct* pCmnInfo - Common headers

  Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrHandlePublishResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pMessageRespNtf,
	EcrioSigMgrCommonInfoStruct *pCmnInfo
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrMessageStruct message = { 0 };
	u_int32 statusCode = ECRIO_SIGMGR_INVALID_STATUS_CODE;
	BoolEnum bUpperNotif = Enum_FALSE;
	EcrioSigMgrPublishInfo* pPubInfo = pSigMgr->pSignalingInfo->pPublishInfo;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);
	if ((pMessageRespNtf == NULL) || (pCmnInfo == NULL))
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data", __FUNCTION__, __LINE__);
		goto Error_Level_01;
	}
	message.msgCmd = EcrioSigMgrPublishResponse;
	message.pData = (void *)pMessageRespNtf;
	statusCode = pMessageRespNtf->responseCode / 100;
	if (statusCode == ECRIO_SIGMGR_1XX_RESPONSE)
	{
		goto Error_Level_01;
	}


	if (statusCode == ECRIO_SIGMGR_2XX_RESPONSE)
	{
		error = _EcrioSigMgrRespValidateStateTakeAction(pSigMgr, pMessageRespNtf, ECRIO_SIG_MGR_PUBLISH_EVENT_2xx_Received, NULL);
		bUpperNotif = Enum_TRUE;
	} else if (statusCode == ECRIO_SIGMGR_3XX_RESPONSE){
	} else if (statusCode == ECRIO_SIGMGR_4XX_RESPONSE){

		if (pMessageRespNtf->responseCode == ECRIO_SIGMGR_RESPONSE_CODE_COND_REQEST_FAILED){
			error = _EcrioSigMgrRespValidateStateTakeAction(pSigMgr, pMessageRespNtf, ECRIO_SIG_MGR_PUBLISH_EVENT_412_Received, NULL);
			//bUpperNotif = Enum_TRUE;

		}
		else if (pMessageRespNtf->responseCode == 423){
			error = _EcrioSigMgrRespValidateStateTakeAction(pSigMgr, pMessageRespNtf, ECRIO_SIG_MGR_PUBLISH_EVENT_423_Received, NULL);
			//bUpperNotif = Enum_TRUE;
		}
		else{
			error = _EcrioSigMgrRespValidateStateTakeAction(pSigMgr, pMessageRespNtf, ECRIO_SIG_MGR_PUBLISH_EVENT_4xx_6xx_Received, NULL);
			bUpperNotif = Enum_TRUE;
		}
	}else if (statusCode == ECRIO_SIGMGR_5XX_RESPONSE || statusCode == ECRIO_SIGMGR_6XX_RESPONSE){
		error = _EcrioSigMgrRespValidateStateTakeAction(pSigMgr, pMessageRespNtf, ECRIO_SIG_MGR_PUBLISH_EVENT_4xx_6xx_Received,NULL);
		bUpperNotif = Enum_TRUE;
	}else {
		;
	}

	
#ifdef NOT_REQURE

	if (statusCode == ECRIO_SIGMGR_2XX_RESPONSE)
	{
		error = _EcrioSigMgrRespValidateStateTakeAction(pSigMgr, pMessageRespNtf, ECRIO_SIG_MGR_PUBLISH_EVENT_2xx_Received,NULL);
		bUpperNotif = Enum_TRUE;
	}
	else
	{
		if (
			pMessageRespNtf->responseCode != ECRIO_SIGMGR_RESPONSE_CODE_COND_REQEST_FAILED ||
			pMessageRespNtf->responseCode != ECRIO_SIGMGR_RESPONSE_CODE_INTERVAL_TOO_BRIEF
			)
		{
			bUpperNotif = Enum_TRUE;
			if (pPubInfo->bIsPubExpireTimerRunning == Enum_TRUE)
			{
				pal_TimerStop(pPubInfo->uPubExpireTimerID);
				pPubInfo->uPubExpireTimerID = NULL;
				pPubInfo->bIsPubExpireTimerRunning = Enum_FALSE;
			}

			_EcrioSigMgrPublishInfoStruct(&pPubInfo);
			pSigMgr->pSignalingInfo->pPublishInfo = NULL;
		}
		
		if (!bUpperNotif && pPubInfo->eState != ECRIO_SIG_MGR_PUBLISH_STATE_Remove)
		{
			
			EcrioSigMgrMessageBodyStruct msgBody = { .messageBodyType = EcrioSigMgrMessageBodyUnknown };
			EcrioSigMgrUnknownMessageBodyStruct unknownMsgBody = { 0 };
			EcrioSigMgrSipMessageStruct messageReq = { .eMethodType = EcrioSipMessageTypeNone };
			EcrioSigMgrSipMessageStruct *pMessageReq = NULL;
			
			unknownMsgBody.bufferLength = pPubInfo->uMssgLen;
			unknownMsgBody.pBuffer = pPubInfo->pMssgBody;
			unknownMsgBody.contentType.numParams = 0;
			unknownMsgBody.contentType.ppParams = NULL;
			unknownMsgBody.contentType.pHeaderValue = (u_char*)"application/pidf+xml";
			messageReq.pMessageBody = &msgBody;
			msgBody.messageBodyType = EcrioSigMgrMessageBodyUnknown;
			msgBody.pMessageBody = &unknownMsgBody;

			messageReq.eModuleId = EcrioSigMgrCallbackRegisteringModule_CPM;
			error = _EcrioSigMgrRespValidateStateTakeAction(pSigMgr, pMessageRespNtf, ECRIO_SIG_MGR_PUBLISH_EVENT_2xx_Received, &messageReq);
			_EcrioSigMgrSendPublish(pSigMgr, &messageReq);
			pMessageReq = &messageReq;
			EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_SipMessage, (void**)&pMessageReq, Enum_FALSE);

		}
	}

#endif

	// Notify to the Upperlayer
	if (bUpperNotif)
		_EcrioSigMgrUpdateStatusUCE(pSigMgr, &message);

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************
                    API Definition Section - End
*****************************************************************************/
