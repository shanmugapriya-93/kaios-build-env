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
#include "EcrioSigMgrInstantMessageInternal.h"
#include "EcrioCPMDataTypes.h"

/*****************************************************************************
                    API Definition Section - Begin
*****************************************************************************/

/*****************************************************************************

  Function:		_EcrioSigMgrSendMessage()

  Purpose:		Initiates the query for capabilities of the remote UA.

  Description:	Creates and sends a Message request.

  Input:		EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                EcrioSigMgrSipMessageStruct* pMessageReq - Message
                request Headers.

  Input/OutPut: SIGSESSIONHANDLE* pSessionHandle - Handle to session.

  Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrSendMessage
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pMessageReq,
	u_int32 retry_count
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 reqLen = 0;
	u_char *pReqData = NULL;
	EcrioSigMgrMandatoryHeaderStruct *pMandatoryHdrs = NULL;
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;
	EcrioSigMgrNetworkInfoStruct *pNetworkInfo = NULL;
	EcrioSigMgrCommonInfoStruct cmnInfo = { 0 };
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };

	EcrioSigMgrUnknownMessageBodyStruct *pUnknownBody = NULL;
	u_char *pBuffer= NULL;
	u_char* pContentType = (u_char *)"application/vnd.3gpp2.sms";
	
	
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pNetworkInfo = pSigMgr->pNetworkInfo;
	if (pNetworkInfo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpNetworkInfo is NULL", __FUNCTION__, __LINE__);

		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	pMandatoryHdrs = pMessageReq->pMandatoryHdrs;

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

		/*error  = ECRIO_SIGMGR_MESSAGE_FAILED;*/   /*return proper error code*/
		goto Error_Level_02;
	}

	/*if (pMandatoryHdrs->ppVia && pMandatoryHdrs->ppVia[0] && pMandatoryHdrs->ppVia[0]->pBranch)
	{
		pal_MemoryFree((void **)&(pMandatoryHdrs->ppVia[0]->pBranch));
		error = _EcrioSigMgrGenerateBranchParam(pSigMgr, &(pMandatoryHdrs->ppVia[0]->pBranch));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrGenerateBranchParam() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_02;
		}
	}*/

	/* Check for CSeq */
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

	error = _EcrioSigMgrPopulateCommonInfo(pSigMgr, pMandatoryHdrs, &cmnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateCommonInfo() error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	if (pMessageReq->pRouteSet == NULL)
	{
		pMessageReq->pRouteSet = _EcrioSigMgrAddRouteHeader(pSigMgr);
	}
#ifdef ENABLE_RCS
	//adding Converstion id and contribution id to optional headers
	if (pMessageReq->pConvId)
	{
		error = _EcrioSigMgrAddConversationsIDHeader(pSigMgr, pMessageReq);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrPopulateCommonInfo() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}
	}
#endif //ENABLE_RCS

	if (pSigMgr->pOOMObject)
	{
		if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeMessage, EcrioSipHeaderTypeP_AccessNetworkInfo))
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

		if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeMessage, EcrioSipHeaderTypeP_LastAccessNetworkInfo))
		{
			error = _EcrioSigMgrAddP_LastAccessNWInfoHeader(pSigMgr, pMessageReq->pOptionalHeaderList);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tEcrioSipHeaderTypeP_LastAccessNetworkInfo() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}

		if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeMessage, EcrioSipHeaderTypeRequestDisposition))
		{
			error = _EcrioSigMgrAddRequestDispositionHeader(pSigMgr, pMessageReq->pOptionalHeaderList);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddRequestDispositionHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}

		if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeMessage, EcrioSipHeaderTypeP_PreferredService))
		{
			error = _EcrioSigMgrAddP_PreferredServiceHeader(pSigMgr, pMessageReq);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddP_PreferredServiceHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}

		if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeMessage, EcrioSipHeaderTypeAcceptContact))
		{
			error = _EcrioSigMgrAddAccptContactHeader(pSigMgr, pMessageReq);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddAccptContactHeader() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}

		if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeMessage, EcrioSipHeaderTypeMinSE))
		{
			if (pSigMgr->pSignalingInfo->minSe != 0)
			{
				error = _EcrioSigMgrAddMinSEHeader(pSigMgr, pMessageReq->pOptionalHeaderList, pSigMgr->pSignalingInfo->minSe);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					goto Error_Level_01;
				}
			}
		}
	}

	if (pSigMgr->pSignalingInfo->pSecurityVerify != NULL)
	{
		u_char *pRequire = (u_char *)ECRIO_SIG_MGR_HEADER_VALUE_SEC_AGREE;

		error = _EcrioSigMgrCreateSecurityVerifyHdr(pSigMgr, &pMessageReq->pOptionalHeaderList);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pMessageReq->pOptionalHeaderList, EcrioSipHeaderTypeRequire, 1, &pRequire, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}
		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pMessageReq->pOptionalHeaderList, EcrioSipHeaderTypeProxyRequire, 1, &pRequire, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}
	}
	
	/* Check and add contact header for out going SIP MESSAGE method. */
	if (pMessageReq->pContact == NULL)
	{
		error = _EcrioSigMgrAddLocalContactUri(pSigMgr, &pMessageReq->pContact, pMessageReq->bPrivacy);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddLocalContactUri() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		if (pSigMgr->pOOMObject->ec_oom_HasUserPhoneSupport(EcrioSipMessageTypeMessage, EcrioSipHeaderTypeContact, EcrioSipURIType_SIP) == Enum_TRUE)
		{
			/* All outgoing SIP uri which contain telephone number in user info part must have "user=phone" parameter as SIP uri parameter.	*/
			error = _EcrioSigMgrAddUserPhoneParam(pSigMgr, &pMessageReq->pContact->ppContactDetails[0]->nameAddr.addrSpec);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddUserPhoneParam, error=%u", __FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}
	}

	error = _EcrioSigMgrAddP_PreferrdIdHeader(pSigMgr, pMessageReq);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddP_preferredIdHeader() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	pMessageReq->eMethodType = EcrioSipMessageTypeMessage;
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

	if (pSigMgr->pSignalingInfo->bEnableTls == Enum_TRUE && pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex != 0)
	{
		eTransportType = EcrioSigMgrTransportTLS;
	}
	else if ((pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE)&&(reqLen > pSigMgr->pSignalingInfo->uUdpMtu))
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


	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeSIP,
		"%s:%u\tOutgoing SIP message constructed:", __FUNCTION__, __LINE__);
	SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pReqData, reqLen);

	if (pMessageReq->pMessageBody != NULL)
	{
		pUnknownBody = pMessageReq->pMessageBody->pMessageBody;
		if (pUnknownBody->bufferLength != 0 && pal_StringICompare(pUnknownBody->contentType.pHeaderValue, (const u_char*)pContentType) == 0)
		{
			pBuffer = pal_StringCreate(pUnknownBody->pBuffer, pal_StringLength(pUnknownBody->pBuffer));
		}
	}

	cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_MESSAGE;
	cmnInfo.role = EcrioSigMgrRoleUAC;

	txnInfo.currentContext = ECRIO_SIGMGR_MESSAGE_REQUEST;
	txnInfo.retransmitContext = ECRIO_SIGMGR_MESSAGE_REQUEST_RESEND;
	txnInfo.deleteflag = Enum_FALSE;
	txnInfo.messageType = EcrioSigMgrMessageNonInviteRequest;
	txnInfo.transport = eTransportType;
	txnInfo.pMsg = pReqData;
	txnInfo.msglen = reqLen;
	txnInfo.pSessionMappingStr = NULL;
	txnInfo.pSMSRetryData = (void*)pBuffer;
	txnInfo.retry_count = retry_count;

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


/**************************************************************************

Function:		_EcrioSigMgrSMSRetryMsg()

**************************************************************************/
u_int32 _EcrioSigMgrSMSRetryMsg
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrMandatoryHeaderStruct *pMandatoryHdrs,
	u_char *pMethod,
	u_int32 responseCode
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR, i = 0;
	EcrioSigMgrCommonInfoStruct commonInfo = {0};
	EcrioSigMgrTXNInfoStruct txnInfo = {0};
	TimerStartConfigStruct timerConfig = { 0 };
	EcrioSigMgrRetryMessageStruct		*pSMSRetryMessage = NULL;
	TIMERHANDLE TimerId;
	u_char *pBuffer = NULL;
	u_int32 SmsRetryCount;
	EcrioOOMRetryQueryStruct retryQueryStruct = { 0 };
	EcrioOOMRetryValuesStruct smsRetryValueStruct = { 0 };
	

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if ((pMethod == NULL) || (pMandatoryHdrs == NULL))
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}


	commonInfo.cSeq = pMandatoryHdrs->CSeq;
	commonInfo.pBranch = pMandatoryHdrs->ppVia[0]->pBranch;
	commonInfo.pCallId = pMandatoryHdrs->pCallId;
	commonInfo.role = EcrioSigMgrRoleUAC;
	commonInfo.pMethodName = pMethod;

	if ((pMandatoryHdrs->pTo != NULL) && (pMandatoryHdrs->pTo->ppParams != NULL))
	{
		/*u_int32 i = 0;*/
		for (i = 0; i < pMandatoryHdrs->pTo->numParams; i++)
		{
			if (!(pal_StringICompare(pMandatoryHdrs->pTo->ppParams[i]->pParamName,
				(u_char *)ECRIO_SIG_MGR_SIP_TAG_STRING)))
			{
				commonInfo.pToTag = pMandatoryHdrs->pTo->ppParams[i]->pParamValue;
				break;
			}
		}
	}

	if ((pMandatoryHdrs->pFrom != NULL) && (pMandatoryHdrs->pFrom->ppParams != NULL))
	{
		/*u_int32 i = 0;*/
		for (i = 0; i < pMandatoryHdrs->pFrom->numParams; i++)
		{
			if (!(pal_StringICompare(pMandatoryHdrs->pFrom->ppParams[i]->pParamName,
				(u_char *)ECRIO_SIG_MGR_SIP_TAG_STRING)))
			{
				commonInfo.pFromTag = pMandatoryHdrs->pFrom->ppParams[i]->pParamValue;
				break;
			}
		}
	}


	commonInfo.responseCode = responseCode;


	txnInfo.pSessionMappingStr = commonInfo.pCallId;

	txnInfo.messageType = EcrioSigMgrMessageNonInviteResponse;


	txnInfo.transport = pMandatoryHdrs->ppVia[0]->transport;


	_EcrioSigMgrGetTxnSMSRetryParameter(pSigMgr, &commonInfo, &txnInfo, &pBuffer, &SmsRetryCount);
	if(pBuffer == NULL)
	{

		error = ECRIO_SIGMGR_UNKNOWN_ERROR;
		goto Error_Level_01;
					
	}



	if (pSigMgr->pOOMObject != NULL && pSigMgr->pOOMObject->ec_oom_GetSMSMessageRetryValues != NULL )
	{

		/*Populate OOM query structure*/
		retryQueryStruct.bIsInitialRequest = Enum_FALSE;
		retryQueryStruct.uRetryCount = SmsRetryCount;
		retryQueryStruct.uResponseCode = (u_int16)responseCode;
			
			
		pSigMgr->pOOMObject->ec_oom_GetSMSMessageRetryValues(retryQueryStruct, &smsRetryValueStruct);


	}

		// retry for the 3ggp2 sms
	if(smsRetryValueStruct.uRetryInterval == 0)
	{
		error = ECRIO_SIGMGR_UNKNOWN_ERROR;
		goto Error_Level_01;
	}

	
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u",
		__FUNCTION__, __LINE__);		
	
	pal_MemoryAllocate(sizeof(EcrioSigMgrRetryMessageStruct), (void**)&pSMSRetryMessage);
	if(pSMSRetryMessage != NULL)
	{
		
		pSMSRetryMessage->pUnknownBody = pal_StringCreate(pBuffer, pal_StringLength(pBuffer));
		pSMSRetryMessage->pSigMgrHandle = pSigMgr;
		
		if (pMandatoryHdrs->pTo->nameAddr.addrSpec.u.pTelUri != NULL)
		{
			if (pMandatoryHdrs->pTo->nameAddr.addrSpec.u.pTelUri->subscriberType == EcrioSigMgrTelSubscriberGlobal)
			{
				u_char 	pDestTemp[30];
				memset(pDestTemp, 0x00, 30);
				if (NULL == pal_StringNCopy(pDestTemp, 30, (const u_char *)"+", pal_StringLength((const u_char*)"+")))
				{
					error = ECRIO_SIGMGR_STRING_ERROR;
					goto Error_Level_01;
				}
				if (NULL == pal_StringNConcatenate(pDestTemp, 30 - 1, pMandatoryHdrs->pTo->nameAddr.addrSpec.u.pTelUri->u.pGlobalNumber->pGlobalNumberDigits, pal_StringLength(pMandatoryHdrs->pTo->nameAddr.addrSpec.u.pTelUri->u.pGlobalNumber->pGlobalNumberDigits)))
				{
					error = ECRIO_SIGMGR_STRING_ERROR;
					goto Error_Level_01;
				}
				pSMSRetryMessage->pDest = pal_StringCreate(pDestTemp, pal_StringLength(pDestTemp));
			}
			else
			{
				if(pMandatoryHdrs->pTo->nameAddr.addrSpec.u.pTelUri->u.pLocalNumber->pLocalNumberDigits != NULL)
				{
					pSMSRetryMessage->pDest = pal_StringCreate(pMandatoryHdrs->pTo->nameAddr.addrSpec.u.pTelUri->u.pLocalNumber->pLocalNumberDigits, pal_StringLength(pMandatoryHdrs->pTo->nameAddr.addrSpec.u.pTelUri->u.pLocalNumber->pLocalNumberDigits));
				}
			}
		}
	}
	else
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	pSMSRetryMessage->SmsRetryCount = SmsRetryCount+1;

	if(pSMSRetryMessage->pDest  == NULL)
	{
		error = ECRIO_SIGMGR_UNKNOWN_ERROR;
		goto Error_Level_01;
	}
	timerConfig.uInitialInterval = (u_int32)smsRetryValueStruct.uRetryInterval ;
	timerConfig.uPeriodicInterval = 0;
	timerConfig.bEnableGlobalMutex = Enum_TRUE;
	timerConfig.timerCallback = _EcrioSigMgrSMSRetryTimerCallback;
	timerConfig.pCallbackData = (void *)pSMSRetryMessage;
	/* Start  timer
	*/
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStarting SMS Retry Timer  %u ",
		__FUNCTION__, __LINE__, timerConfig.uInitialInterval);
	error = pal_TimerStart(pSigMgr->pal, &timerConfig, &TimerId);
	if (KPALErrorNone != error)
	{
		error = ECRIO_SIGMGR_TIMER_ERROR;
		goto Error_Level_01;
	}
	
	
	return error;

Error_Level_01:


	if(pSMSRetryMessage)
	{
		if(pSMSRetryMessage->pDest)
		{
			pal_MemoryFree((void**)&pSMSRetryMessage->pDest);
		}

		if(pSMSRetryMessage->pUnknownBody )
		{
			pal_MemoryFree((void**)&pSMSRetryMessage->pUnknownBody);
		}		
		
		pal_MemoryFree((void**)&pSMSRetryMessage);
	}		

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}


/*****************************************************************************

  Function:		_EcrioSigMgrHandleMessageResponse()

  Purpose:		Handles the query for capabilities of the remote UA.

  Description:	Handles an incoming Message response.

  Input:		EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                EcrioSigMgrMessageResponseStruct* pMessageRespNtf - Message
                response Headers.
                EcrioSigMgrCommonInfoStruct* pCmnInfo - Common headers

  Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrHandleMessageResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pMessageRespNtf,
	EcrioSigMgrCommonInfoStruct *pCmnInfo
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrMessageStruct message = { 0 };
	u_int32 statusCode = ECRIO_SIGMGR_INVALID_STATUS_CODE;
	BoolEnum bIsAllocatedLocally = Enum_FALSE;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if ((pMessageRespNtf == NULL) || (pCmnInfo == NULL))
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;

		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data", __FUNCTION__, __LINE__);

		goto Error_Level_01;
	}

	if(pMessageRespNtf->pMandatoryHdrs == NULL)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void**)&pMessageRespNtf->pMandatoryHdrs);
		if (pMessageRespNtf->pMandatoryHdrs == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}
		pMessageRespNtf->pMandatoryHdrs->pCallId = pCmnInfo->pCallId;
		bIsAllocatedLocally = Enum_TRUE;
	}
	else
	{
		if(pMessageRespNtf->pMandatoryHdrs->pCallId == NULL)
			pMessageRespNtf->pMandatoryHdrs->pCallId = pCmnInfo->pCallId;
	}
	message.msgCmd = EcrioSigMgrInstantMessageResponse;
	message.pData = (void *)pMessageRespNtf;

	statusCode = pMessageRespNtf->responseCode / 100;

	if (statusCode == ECRIO_SIGMGR_1XX_RESPONSE)
	{
		goto Error_Level_01;
	}

	if (statusCode == ECRIO_SIGMGR_2XX_RESPONSE)
	{
		pMessageRespNtf->statusCode = ECRIO_SIGMGR_RESPONSE_SUCCESS;
	}
#if 0
	// removing the retry mechanism as it will be done by Application
	else
	{
		error = _EcrioSigMgrSMSRetryMsg(pSigMgr, pMessageRespNtf->pMandatoryHdrs,
										_EcrioSigMgrGetMthdName(pMessageRespNtf->eMethodType),
									 	pMessageRespNtf->responseCode);
		if(error == ECRIO_SIGMGR_NO_ERROR )
		{
			goto Error_Level_01;
		}
		else
		{		
			pMessageRespNtf->statusCode = ECRIO_SIGMGR_REQUEST_FAILED;
		}
	}

	/*Check for Contribution and Conversation Id*/
	_EcrioSigMgrGetConversationsIdHeader(pSigMgr, pMessageRespNtf);
	if (pMessageRespNtf->pConvId)
	{		
		_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
	}
	else
	{
		_EcrioSigMgrUpdateStatusSMS(pSigMgr, &message);
	}
#else
	_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);

	if(bIsAllocatedLocally)
		pal_MemoryFree((void**)&pMessageRespNtf->pMandatoryHdrs);

#endif

	/*Routing support*/
	//ec_SigMgrHashMap_Delete(pSigMgr->hHashMap, pCmnInfo->pCallId);	

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}


/*****************************************************************************
                    API Definition Section - End
*****************************************************************************/
