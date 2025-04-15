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
#include "EcrioSigMgrOptionsInternal.h"

/*****************************************************************************
                Internal API Defnision Section - Begin
*****************************************************************************/

/*****************************************************************************

  Function:		_EcrioSigMgrSendOptionsResponse()

  Purpose:		Respond to query for capabilities.

  Description:	Creates and sends a options response.

  Input:		EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                EcrioSigMgrMessageResponseStruct* pOptionsResp - Options
                response Headers.

  Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrSendOptionsResponseI
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pOptionsResp
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	u_int32	reqLen = 0;
	u_char *pReqData = NULL;
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };
	EcrioSigMgrCommonInfoStruct	cmnInfo = { 0 };

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	error = _EcrioSigMgrPopulateMandatoryHeaders(pSigMgr, pOptionsResp,
		EcrioSigMgrSIPResponse, Enum_TRUE, NULL);

	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateMandatoryHeaders() for pIMSManHdrs, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_02;
	}

	error = _EcrioSigMgrPopulateCommonInfo(pSigMgr, pOptionsResp->pMandatoryHdrs, &cmnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateCommonInfo() for pCmnInfo, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_02;
	}

	cmnInfo.responseCode = pOptionsResp->responseCode;
	error = _EcrioSigMgrUpdateMessageRequestStruct(pSigMgr, pOptionsResp, Enum_FALSE);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tOptional Header Creation error, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}


	/* Add Contact */
	if (pOptionsResp->pContact == NULL)
	{
		error = _EcrioSigMgrAddLocalContactUri(pSigMgr, &pOptionsResp->pContact, pOptionsResp->bPrivacy);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddLocalContactUri() for pOptionalHdrs->pContact, error=%u", __FUNCTION__, __LINE__, error);

			goto Error_Level_02;
		}
	}

	eTransportType = pOptionsResp->pMandatoryHdrs->ppVia[0]->transport;
	
	/* Construct Options response */
	error = _EcrioSigMgrSipMessageForm(pSigMgr, EcrioSigMgrSipMessageModeFullName, pOptionsResp, &pReqData, &reqLen);
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

	cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_OPTIONS;
	cmnInfo.role = EcrioSigMgrRoleUAS;

	txnInfo.currentContext = ECRIO_SIGMGR_OPTIONS_RESPONSE_SEND;
	txnInfo.retransmitContext = ECRIO_SIGMGR_OPTIONS_RESPONSE_RESEND;

	txnInfo.deleteflag = Enum_FALSE;
	txnInfo.messageType = EcrioSigMgrMessageNonInviteResponse;
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

u_int32 _EcrioSigMgrHandleOptionsRequest
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pOptionsReqNtf,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	u_int32 reasonCode,
	u_char *pReasonPhrase

)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int16 i = 0;
	EcrioSigMgrMessageStruct message = { .msgCmd = EcrioSigMgrMessageNone };
	EcrioSigMgrMandatoryHeaderStruct *pTempManHdr = NULL;

	if (pSigMgr == NULL)
	{
		return ECRIO_SIGMGR_BAD_HANDLE;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if ((pOptionsReqNtf == NULL) || (pCmnInfo == NULL))
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;

		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data", __FUNCTION__, __LINE__);

		goto Error_Level_01;
	}

	//check wether the optional header contains Conversation and Contribution ids, if contains, add it to pid.
	_EcrioSigMgrGetConversationsIdHeader(pSigMgr, pOptionsReqNtf);

	pTempManHdr = pOptionsReqNtf->pMandatoryHdrs;
	if ((pTempManHdr == NULL) || (pTempManHdr->pTo == NULL))
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;

		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tInsufficient data", __FUNCTION__, __LINE__);

		goto Error_Level_01;
	}

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

	message.msgCmd = EcrioSigMgrOptionsRequestNotification;
	message.pData = (void *)pOptionsReqNtf;

	if (pOptionsReqNtf->isRequestInValid == Enum_TRUE)
	{
		error = _EcrioSigMgrSendOptionsErrorResponse(pSigMgr, pOptionsReqNtf,
			pReasonPhrase, reasonCode);
	}
	else
	{

		// Optoins received NOTIFY app
		message.msgCmd = EcrioSigMgrOptionsRequestNotification;
		message.pData = (void *)pOptionsReqNtf;
		_EcrioSigMgrUpdateStatusUCE(pSigMgr, &message);
	}

Error_Level_01:

	if (pOptionsReqNtf->pConvId)
	{
		pal_MemoryFree((void**)&pOptionsReqNtf->pConvId->pContributionId);
		pal_MemoryFree((void**)&pOptionsReqNtf->pConvId->pConversationId);
		pal_MemoryFree((void**)&pOptionsReqNtf->pConvId->pInReplyToContId);
		pal_MemoryFree((void**)&pOptionsReqNtf->pConvId);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit,
		"%s:%u\terror=%u", __FUNCTION__, __LINE__, error);

	return error;
}
/*****************************************************************************

  Function:		_EcrioSigMgrSendOptionsErrorResponse()

  Purpose:		Respond to IM Request on Failure conditions.

  Description:	Creates and sends a Options Response.

  Input:		EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                EcrioSigMgrSipMessageStruct* pOptionsReq - Options
                request Headers.
                u_char* pReasonPhrase - Error phrase
                u_int32 reasonCode - status code for error response

  Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrSendOptionsErrorResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pOptionsReq,
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
	sipMessage.eMethodType = EcrioSipMessageTypeOptions;
	sipMessage.eReqRspType = EcrioSigMgrSIPResponse;

	error = _EcrioSigMgrUpdateMessageRequestStruct(pSigMgr, &sipMessage, Enum_FALSE);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tOptional Header Creation error, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}
	sipMessage.pMandatoryHdrs = pOptionsReq->pMandatoryHdrs;

	error = _EcrioSigMgrPopulateCommonInfo(pSigMgr, sipMessage.pMandatoryHdrs, &cmnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateCommonInfo() for pCmnInfo, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	cmnInfo.responseCode = sipMessage.responseCode;

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

	cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_OPTIONS;
	cmnInfo.role = EcrioSigMgrRoleUAS;

	txnInfo.currentContext = ECRIO_SIGMGR_OPTIONS_RESPONSE_SEND;
	txnInfo.retransmitContext = ECRIO_SIGMGR_OPTIONS_RESPONSE_RESEND;

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

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}


/*****************************************************************************

Function:            _EcrioSigMgrSendOptionsResponse()

Purpose:             Respond to query for capabilities.

Description: Creates and sends a options response.

Input:               EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
EcrioSigMgrMessageResponseStruct* pOptionsResp - Options
response Headers.

Returns:             error code.
*****************************************************************************/
u_int32 _EcrioSigMgrSendOptionsResponseImpl
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pOptionsResp
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 reqLen = 0;
	u_char *pReqData = NULL;
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };
	EcrioSigMgrCommonInfoStruct     cmnInfo = { 0 };

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pOptionsResp->eMethodType = EcrioSipMessageTypeOptions;
	pOptionsResp->eReqRspType = EcrioSigMgrSIPResponse;

	error = _EcrioSigMgrPopulateMandatoryHeaders(pSigMgr, pOptionsResp,
		EcrioSigMgrSIPResponse, Enum_TRUE, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateMandatoryHeaders() for pIMSManHdrs, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_02;
	}

	error = _EcrioSigMgrPopulateCommonInfo(pSigMgr, pOptionsResp->pMandatoryHdrs, &cmnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateCommonInfo() for pCmnInfo, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_02;
	}

	cmnInfo.responseCode = pOptionsResp->responseCode;
	error = _EcrioSigMgrUpdateMessageRequestStruct(pSigMgr, pOptionsResp, Enum_FALSE);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tOptional Header Creation error, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}


	/* Add Contact */
	if (pOptionsResp->pContact == NULL)
	{
		error = _EcrioSigMgrAddLocalContactUri(pSigMgr, &pOptionsResp->pContact, pOptionsResp->bPrivacy);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddLocalContactUri() for pOptionalHdrs->pContact, error=%u", __FUNCTION__, __LINE__, error);

			goto Error_Level_02;
		}
	}

	eTransportType = pOptionsResp->pMandatoryHdrs->ppVia[0]->transport;

	/* Construct Options response */
	error = _EcrioSigMgrSipMessageForm(pSigMgr, EcrioSigMgrSipMessageModeFullName, pOptionsResp, &pReqData, &reqLen);
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

	cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_OPTIONS;
	cmnInfo.role = EcrioSigMgrRoleUAS;

	txnInfo.currentContext = ECRIO_SIGMGR_OPTIONS_RESPONSE_SEND;
	txnInfo.retransmitContext = ECRIO_SIGMGR_OPTIONS_RESPONSE_RESEND;

	txnInfo.deleteflag = Enum_FALSE;
	txnInfo.messageType = EcrioSigMgrMessageNonInviteResponse;
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

Function:            _EcrioSigMgrHandleOptionsRequest()

Purpose:             Handles the query for capabilities of the remote UA.

Description: Handles an incoming options request.

Input:               EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
EcrioSigMgrSipMessageStruct* pOptionsReqNtf -
Options request Headers.
EcrioSigMgrCommonInfoStruct* pCmnInfo - Common headers
u_char* pReasonPhrase - Reason Phrase for options response
u_int32 reasonCode - Reason Code for options response

Returns:             error code.
*****************************************************************************/
u_int32 _EcrioSigMgrSendOptionsResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pOptionsReqNtf
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrSipMessageStruct messageResp = { .eMethodType = EcrioSipMessageTypeNone };


	if (pSigMgr == NULL)
	{
		return ECRIO_SIGMGR_BAD_HANDLE;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pOptionsReqNtf == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;

		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data", __FUNCTION__, __LINE__);

		goto Error_Level_01;
	}

	//check wether the optional header contains Conversation and Contribution ids, if contains, add it to pid.
	_EcrioSigMgrGetConversationsIdHeader(pSigMgr, pOptionsReqNtf);
	
	messageResp.eMethodType = EcrioSipMessageTypeOptions;
	messageResp.pReasonPhrase = (u_char *)ECRIO_SIGMGR_RESPONSE_OK;
	messageResp.responseCode = ECRIO_SIGMGR_RESPONSE_CODE_OK;
	messageResp.pMandatoryHdrs = pOptionsReqNtf->pMandatoryHdrs;
	//get the conversation and contribution id from request
	messageResp.pConvId = pOptionsReqNtf->pConvId;

	messageResp.pFetaureTags = pOptionsReqNtf->pFetaureTags;

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

	// send repsonse to n/w
	error = _EcrioSigMgrSendOptionsResponseImpl(pSigMgr, &messageResp);

	messageResp.pReasonPhrase = NULL;
	messageResp.pMandatoryHdrs = NULL;
	messageResp.pFetaureTags = NULL;
	_EcrioSigMgrReleaseSipMessage(pSigMgr, &messageResp);

Error_Level_01:

	if (pOptionsReqNtf->pConvId)
	{
		pal_MemoryFree((void**)&pOptionsReqNtf->pConvId->pContributionId);
		pal_MemoryFree((void**)&pOptionsReqNtf->pConvId->pConversationId);
		pal_MemoryFree((void**)&pOptionsReqNtf->pConvId->pInReplyToContId);
		pal_MemoryFree((void**)&pOptionsReqNtf->pConvId);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit,
		"%s:%u\terror=%u", __FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************
                    API Definition Section - End
*****************************************************************************/
