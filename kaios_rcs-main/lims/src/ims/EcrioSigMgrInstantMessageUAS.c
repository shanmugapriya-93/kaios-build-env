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

/*****************************************************************************
                Internal API Declaration Section - Begin
*****************************************************************************/
/**
*API to send automatic error response to incoming IM request.
*@Param IN EcrioSigMgrStruct* pSigMgr - Pointer to signaling manager.
*@Param IN EcrioSigMgrSipMessageStruct* pMessageReq - incoming MESSAGE request info.
*@Param IN u_char* pReasonPhrase - reason phrase of error Response.
*@Param IN u_int32 reasonCode - status code of error Response
*@Return	u_int32 Error code
*/
u_int32 _EcrioSigMgrSendMessageErrorResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pMessageReq,
	u_char *pReasonPhrase,
	u_int32 reasonCode
);
/*****************************************************************************
                Internal API Declaration Section - End
*****************************************************************************/

/*****************************************************************************
                    API Definition Section - Begin
*****************************************************************************/

/*****************************************************************************

  Function:		_EcrioSigMgrSendMessageResponse()

  Purpose:		Respond to query for capabilities.

  Description:	Creates and sends a options response.

  Input:		EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                EcrioSigMgrMessageResponseStruct* pMessageResp - Message
                response Headers.

  Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrSendMessageResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pMessageResp
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	u_int32	reqLen = 0;
	u_char *pReqData = NULL;
	// EcrioSigMgrMandatoryHeaderStruct *pTempManHdrs = NULL;
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };
	EcrioSigMgrCommonInfoStruct	cmnInfo = { 0 };

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pMessageResp->eMethodType = EcrioSipMessageTypeMessage;
	pMessageResp->eReqRspType = EcrioSigMgrSIPResponse;

	error = _EcrioSigMgrPopulateMandatoryHeaders(pSigMgr, pMessageResp,
		EcrioSigMgrSIPResponse, Enum_TRUE, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateMandatoryHeaders() for pIMSManHdrs, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_02;
	}

	error = _EcrioSigMgrPopulateCommonInfo(pSigMgr, pMessageResp->pMandatoryHdrs, &cmnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateCommonInfo() for pCmnInfo, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_02;
	}

	cmnInfo.responseCode = pMessageResp->responseCode;
	error = _EcrioSigMgrUpdateMessageRequestStruct(pSigMgr, pMessageResp, Enum_FALSE);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tOptional Header Creation error, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}

	eTransportType = pMessageResp->pMandatoryHdrs->ppVia[0]->transport;

	/* Construct Message response */
	error = _EcrioSigMgrSipMessageForm(pSigMgr, EcrioSigMgrSipMessageModeFullName, pMessageResp, &pReqData, &reqLen);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSipMessageForm() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_IMS_LIB_ERROR;
		goto Error_Level_02;
	}

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeSIP, "%s:%u\tOutgoing SIP message constructed:",
		__FUNCTION__, __LINE__);

	SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pReqData, reqLen);

	cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_MESSAGE;
	cmnInfo.role = EcrioSigMgrRoleUAS;

	txnInfo.currentContext = ECRIO_SIGMGR_MESSAGE_RESPONSE_SEND;
	txnInfo.retransmitContext = ECRIO_SIGMGR_MESSAGE_RESPONSE_RESEND;

	txnInfo.deleteflag = Enum_FALSE;
	txnInfo.messageType = EcrioSigMgrMessageNonInviteResponse;
	txnInfo.transport = eTransportType;
	txnInfo.pMsg = pReqData;
	txnInfo.msglen = reqLen;
	txnInfo.pSessionMappingStr = NULL;

	// pTempManHdrs = pMessageResp->pMandatoryHdrs;

	error = _EcrioSigMgrSendMsgToTXNHandler(pSigMgr, &cmnInfo, &txnInfo);

	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendMsgToTXNHandler() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_MESSAGE_FAILED;
		goto Error_Level_02;
	}

Error_Level_02:
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

/*****************************************************************************

  Function:		_EcrioSigMgrHandleMessageRequest()

  Purpose:		Handles the query for capabilities of the remote UA.

  Description:	Handles an incoming options request.

  Input:		EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                EcrioSigMgrSipMessageStruct* pMessageReqNtf -
                Message request Headers.
                EcrioSigMgrCommonInfoStruct* pCmnInfo - Common headers
                u_char* pReasonPhrase - Reason Phrase for options response
                u_int32 reasonCode - Reason Code for options response

  Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrHandleMessageRequest
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pMessageReqNtf,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	u_char *pReasonPhrase,
	u_int32 reasonCode
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int16 i = 0;
	EcrioSigMgrMessageStruct message = { .msgCmd = EcrioSigMgrMessageNone };
	EcrioSigMgrMandatoryHeaderStruct *pTempManHdr = NULL;
	EcrioSigMgrHeaderStruct *pHdr = NULL;
	BoolEnum bIsChatbotSA = Enum_FALSE;

	if (pSigMgr == NULL)
	{
		return ECRIO_SIGMGR_BAD_HANDLE;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if ((pMessageReqNtf == NULL) || (pCmnInfo == NULL))
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;

		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data", __FUNCTION__, __LINE__);

		goto Error_Level_01;
	}

	//check wether the optional header contains Conversation and Contribution ids, if contains, add it to pid.
	_EcrioSigMgrGetConversationsIdHeader(pSigMgr, pMessageReqNtf);

	pTempManHdr = pMessageReqNtf->pMandatoryHdrs;
	if (pCmnInfo->pToTag == NULL)
	{
		error = _EcrioSigMgrGenerateTag(&pCmnInfo->pToTag);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrGenerateTag() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}

		i = pTempManHdr->pTo->numParams;

		if (i == 0)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct *) * 1, (void **)&pTempManHdr->pTo->ppParams);
			if (pTempManHdr->pTo->ppParams == NULL)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryAllocate() error=%u",
					__FUNCTION__, __LINE__, error);

				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			++i;
		}
		else
		{
			/* Check arithmetic overflow - do not use the ArithmeticOverflowDetected() API for u_int16 variable. */
			if (i + 1 > USHRT_MAX)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((i + 1), sizeof(EcrioSigMgrParamStruct *)) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = pal_MemoryReallocate(sizeof(EcrioSigMgrParamStruct *) * (i + 1), (void **)&pTempManHdr->pTo->ppParams);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryReallocate() error=%u",
					__FUNCTION__, __LINE__, error);

				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			++i;
		}

		pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&pTempManHdr->pTo->ppParams[i - 1]);
		if (pTempManHdr->pTo->ppParams[i - 1] == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() error=%u",
				__FUNCTION__, __LINE__, error);

			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		error = _EcrioSigMgrStringCreate(pSigMgr,
			(u_char *)ECRIO_SIG_MGR_SIP_TAG_STRING,
			&pTempManHdr->pTo->ppParams[i - 1]->pParamName);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrStringCreate() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}

		error = _EcrioSigMgrStringCreate(pSigMgr, pCmnInfo->pToTag,
			&pTempManHdr->pTo->ppParams[i - 1]->pParamValue);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrStringCreate() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}

		pTempManHdr->pTo->numParams = (u_int16)i;
	}



	message.msgCmd = EcrioSigMgrInstantMessageRequestNotification;
	message.pData = (void *)pMessageReqNtf;

	if (pMessageReqNtf->isRequestInValid == Enum_TRUE)
	{
		error = _EcrioSigMgrSendMessageErrorResponse(pSigMgr, pMessageReqNtf,
			pReasonPhrase, reasonCode);
	}
	else
	{
		EcrioSigMgrSipMessageStruct messageResp = { .eMethodType = EcrioSipMessageTypeNone };

		messageResp.eMethodType = EcrioSipMessageTypeMessage;
		messageResp.pReasonPhrase = (u_char *)ECRIO_SIGMGR_RESPONSE_OK;
		messageResp.responseCode = ECRIO_SIGMGR_RESPONSE_CODE_OK;
		messageResp.pMandatoryHdrs = pMessageReqNtf->pMandatoryHdrs;
		//get the conversation and contribution id from request
		messageResp.pConvId = pMessageReqNtf->pConvId;


		if (messageResp.pOptionalHeaderList == NULL)
		{
			error = EcrioSigMgrCreateOptionalHeaderList(pSigMgr, &messageResp.pOptionalHeaderList);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tEcrioSigMgrCreateOptionalHeaderList() error=%u",
					__FUNCTION__, __LINE__, error);

				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
		}
#ifdef ENABLE_RCS
		//populate the optional header with pId in request
		if (messageResp.pConvId)
		{
			error = _EcrioSigMgrAddConversationsIDHeader(pSigMgr, &messageResp);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrStringCreate() error=%u",
					__FUNCTION__, __LINE__, error);

				goto Error_Level_01;
			}
		}
#endif //ENABLE_RCS
		_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
#if 0
		if (_EcrioSigMgrFindModuleId(pSigMgr, pMessageReqNtf) == EcrioSigMgrCallbackRegisteringModule_CPM)
		{
			/** Get Accept-Contact header value */
			EcrioSigMgrGetOptionalHeader(pSigMgr, pMessageReqNtf->pOptionalHeaderList, EcrioSipHeaderTypeAcceptContact, &pHdr);
			if (pHdr != NULL)
			{
				if (pHdr->ppHeaderValues[0]->ppParams != NULL)
				{
					for (i = 0; i < pHdr->ppHeaderValues[0]->numParams; i++)
					{
						if (pal_StringCompare(pHdr->ppHeaderValues[0]->ppParams[i]->pParamName, (u_char*)"+g.3gpp.iari-ref") == 0)
						{
							if (pal_StringFindSubString(pHdr->ppHeaderValues[0]->ppParams[i]->pParamValue,
								(u_char*)"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot.sa") != NULL)
							{
								bIsChatbotSA = Enum_TRUE;
								break;
							}
						}
					}
				}
			}

			_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);
		}
		else
		{
			_EcrioSigMgrUpdateStatusSMS(pSigMgr, &message);
		}
#endif
		//_EcrioSigMgrUpdateStatusSMS(pSigMgr, &message);

		/** No send a response when received MESSAGE Request from Chatbot; in this case,
		 *  CPM module will send a response. */
		if (bIsChatbotSA == Enum_FALSE)
		{
			error = _EcrioSigMgrSendMessageResponse(pSigMgr, &messageResp);
		}

		messageResp.pReasonPhrase = NULL;
		messageResp.pMandatoryHdrs = NULL;
		_EcrioSigMgrReleaseSipMessage(pSigMgr, &messageResp);
	}

Error_Level_01:

	if (pMessageReqNtf->pConvId)
	{
		pal_MemoryFree((void**)&pMessageReqNtf->pConvId->pContributionId);
		pal_MemoryFree((void**)&pMessageReqNtf->pConvId->pConversationId);
		pal_MemoryFree((void**)&pMessageReqNtf->pConvId->pInReplyToContId);
		pal_MemoryFree((void**)&pMessageReqNtf->pConvId);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit,
		"%s:%u\terror=%u", __FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************

  Function:		_EcrioSigMgrSendMessageErrorResponse()

  Purpose:		Respond to IM Request on Failure conditions.

  Description:	Creates and sends a Message Response.

  Input:		EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                EcrioSigMgrSipMessageStruct* pMessageReq - Message
                request Headers.
                u_char* pReasonPhrase - Error phrase
                u_int32 reasonCode - status code for error response

  Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrSendMessageErrorResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pMessageReq,
	u_char *pReasonPhrase,
	u_int32 reasonCode
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	u_char *pRespData = NULL;
	u_int32	respLen = 0;
	EcrioSigMgrCommonInfoStruct	cmnInfo = { 0 };
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;
	EcrioSigMgrSipMessageStruct	sipMessage = { .eMethodType = EcrioSipMessageTypeNone };

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	sipMessage.responseCode = reasonCode;
	sipMessage.pReasonPhrase = pReasonPhrase;
	sipMessage.eMethodType = EcrioSipMessageTypeMessage;
	sipMessage.eReqRspType = EcrioSigMgrSIPResponse;

	error = _EcrioSigMgrUpdateMessageRequestStruct(pSigMgr, &sipMessage, Enum_FALSE);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tOptional Header Creation error, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}
	sipMessage.pMandatoryHdrs = pMessageReq->pMandatoryHdrs;

	error = _EcrioSigMgrPopulateCommonInfo(pSigMgr, sipMessage.pMandatoryHdrs, &cmnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateCommonInfo() for pCmnInfo, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	cmnInfo.responseCode = sipMessage.responseCode;
	/*if (pMessageReq->pRequestHdrs->pOptionalHdrs)
	{
	    if (pMessageReq->pRequestHdrs->pOptionalHdrs->pCommonHdrs)
	    {
	        sipMessage.pRequire = pMessageReq->pRequestHdrs->pOptionalHdrs->pCommonHdrs->pRequire;
	        sipMessage.pDate = pMessageReq->pRequestHdrs->pOptionalHdrs->pCommonHdrs->pDate;
	        sipMessage.pTimeStamp = pMessageReq->pRequestHdrs->pOptionalHdrs->pCommonHdrs->pTimeStamp;
	        sipMessage.pPAccessNetworkInfo = pMessageReq->pRequestHdrs->pOptionalHdrs->pCommonHdrs->pPAccessNetworkInfo;
	        sipMessage.pPChargingFunctionAddresses = pMessageReq->pRequestHdrs->pOptionalHdrs->pCommonHdrs->pPChargingFunctionAddresses;
	    }
	}*/


	eTransportType = sipMessage.pMandatoryHdrs->ppVia[0]->transport;

	/* Construct Message response */
	error = _EcrioSigMgrSipMessageForm(pSigMgr, EcrioSigMgrSipMessageModeFullName, &sipMessage, &pRespData, &respLen);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSipMessageForm() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_IMS_LIB_ERROR;
		goto Error_Level_01;
	}

	cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_MESSAGE;
	cmnInfo.role = EcrioSigMgrRoleUAS;

	txnInfo.currentContext = ECRIO_SIGMGR_MESSAGE_RESPONSE_SEND;
	txnInfo.retransmitContext = ECRIO_SIGMGR_MESSAGE_RESPONSE_RESEND;

	txnInfo.deleteflag = Enum_FALSE;
	txnInfo.messageType = EcrioSigMgrMessageNonInviteResponse;
	txnInfo.transport = eTransportType;
	txnInfo.pMsg = pRespData;
	txnInfo.msglen = respLen;
	txnInfo.pSessionMappingStr = NULL;

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeSIP, "%s:%u\tOutgoing SIP message constructed:",
		__FUNCTION__, __LINE__);

	SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pRespData, respLen);

	error = _EcrioSigMgrSendMsgToTXNHandler(pSigMgr, &cmnInfo, &txnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendMsgToTXNHandler() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_MESSAGE_FAILED;
		goto Error_Level_01;
	}

Error_Level_01:
	if (pRespData != NULL)
	{
		pal_MemoryFree((void **)&pRespData);
		pRespData = NULL;
		respLen = 0;
	}

	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);

	sipMessage.pReasonPhrase = NULL;
	sipMessage.pMandatoryHdrs = NULL;
	_EcrioSigMgrReleaseSipMessage(pSigMgr, &sipMessage);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************
                    API Definition Section - End
*****************************************************************************/
