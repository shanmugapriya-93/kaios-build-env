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
#include "EcrioSigMgrInstantMessage.h"
#include "EcrioSigMgrInstantMessageInternal.h"
#include "EcrioSigMgrUtilities.h"

/*****************************************************************************
                Internal API Declaration Section - Begin
*****************************************************************************/
extern u_int32 _EcrioSigMgrSendMessageErrorResponse
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

  Function:		  EcrioSigMgrSendInstantMessage()

  Purpose:		  Construct and Send an Instant Message.

  Description:	  Creates and sends a MESSAGE request.

  Input:		  SIGMGRHANDLE sigMgrHandle - Signaling Manager Instance.
                  EcrioSigMgrSipMessageStruct* pMessageReq - MESSAGE
                  request Headers.

  OutPut:		  none

  Returns:		  error code.
*****************************************************************************/
u_int32 EcrioSigMgrSendInstantMessage
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSipMessageStruct *pMessageReq
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;

	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry,
		"%s:%u\tsigMgrHandle = %x,pMessageStruct = %x",
		__FUNCTION__, __LINE__, sigMgrHandle, pMessageReq);

	if (pMessageReq == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpMessageReq is NULL",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_NOT_INITIALIZED;
		goto Error_Level_01;
	}

	if (pMessageReq->pMandatoryHdrs == NULL ||
		pMessageReq->pMandatoryHdrs->pTo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tMissing To header",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	error = _EcrioSigMgrSendMessage(pSigMgr, pMessageReq, 0);

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

	return error;
}


u_int32 EcrioSigMgrSendInstantMessageResponse
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSipMessageStruct *pMessageResp
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;

	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)sigMgrHandle;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u\tsigMgrHandle = %x,pMessageStruct = %x",
		__FUNCTION__, __LINE__, sigMgrHandle, pMessageResp);

	if (pMessageResp == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tpMessageResp is NULL",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_NOT_INITIALIZED;
		goto Error_Level_01;
	}

	if (pMessageResp->pMandatoryHdrs == NULL || pMessageResp->pMandatoryHdrs->pTo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tMissing To header",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	if (pMessageResp->responseCode == ECRIO_SIGMGR_RESPONSE_CODE_OK)
	{
		EcrioSigMgrSipMessageStruct messageResp = { .eMethodType = EcrioSipMessageTypeNone };

		messageResp.eMethodType = EcrioSipMessageTypeMessage;
		messageResp.pReasonPhrase = (u_char *)ECRIO_SIGMGR_RESPONSE_OK;
		messageResp.responseCode = ECRIO_SIGMGR_RESPONSE_CODE_OK;
		messageResp.pMandatoryHdrs = pMessageResp->pMandatoryHdrs;
		messageResp.pConvId = pMessageResp->pConvId;

		if (messageResp.pOptionalHeaderList == NULL)
		{
			error = EcrioSigMgrCreateOptionalHeaderList(pSigMgr, &messageResp.pOptionalHeaderList);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrCreateOptionalHeaderList() error=%u",
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
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t_EcrioSigMgrStringCreate() error=%u",
					__FUNCTION__, __LINE__, error);

				goto Error_Level_01;
			}
		}

		error = _EcrioSigMgrSendMessageResponse(pSigMgr, &messageResp);

		messageResp.pReasonPhrase = NULL;
		messageResp.pMandatoryHdrs = NULL;
		_EcrioSigMgrReleaseSipMessage(pSigMgr, &messageResp);
	}
	else
	{
		error = _EcrioSigMgrSendMessageErrorResponse(pSigMgr, pMessageResp,
			pMessageResp->pReasonPhrase, pMessageResp->responseCode);
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

	return error;
}


void _EcrioSigMgrSMSRetryTimerCallback
(
	void *pData,
	TIMERHANDLE timerID
)
{

	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	u_int32 uSigError = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrRetryMessageStruct		*pSMSRetryMessage;
	EcrioSigMgrUnknownMessageBodyStruct *pUnknownBody = NULL;
	EcrioSigMgrSipMessageStruct *pMessageReq= NULL;
	EcrioSigMgrSipMessageStruct messageReq = { .eMethodType = EcrioSipMessageTypeNone };
	u_char* pContentType = (u_char *)"application/vnd.3gpp2.sms";
	EcrioSigMgrNameAddrWithParamsStruct toHeader = { 0 };
	EcrioSigMgrMandatoryHeaderStruct *pMandatoryHeader = NULL;
	EcrioSigMgrMessageBodyStruct *pMessageBodyStruct = NULL;
	u_int32 count = 0;


	/* Stop the timer before proceeding further. */
	pal_TimerStop(timerID);

	if (NULL == pData)
	{
		return;
	}

	pSMSRetryMessage = (EcrioSigMgrRetryMessageStruct *)pData;

	pSigMgr = (EcrioSigMgrStruct *)pSMSRetryMessage->pSigMgrHandle;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\ttimerID=%u, pData=%p",
		__FUNCTION__, __LINE__, timerID, pSigMgr);

	/** Populate toHeader in tel uri format */
	uSigError = EcrioSigMgrFillUriStruct(pSigMgr,
		&toHeader.nameAddr.addrSpec,
		pSMSRetryMessage->pDest, EcrioSigMgrURISchemeTEL);
	
	if (uSigError != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrFillUriStruct() error=%u",
			__FUNCTION__, __LINE__, uSigError);
		goto Error_Level_01;
	}
	
	pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void**)&pMandatoryHeader);
	if (pMandatoryHeader == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	pMandatoryHeader->pTo = &toHeader;
	messageReq.pMandatoryHdrs = pMandatoryHeader;


	pal_MemoryAllocate(sizeof(EcrioSigMgrUnknownMessageBodyStruct), (void**)&pUnknownBody);
	if (pUnknownBody == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrMessageBodyStruct), (void**)&pMessageBodyStruct);
	if (pMessageBodyStruct == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	pUnknownBody->contentType.numParams = 0;
	pUnknownBody->contentType.ppParams = NULL;
	pUnknownBody->contentType.pHeaderValue = pal_StringCreate(pContentType, pal_StringLength(pContentType));
	messageReq.pMessageBody = pMessageBodyStruct;
	messageReq.pMessageBody->messageBodyType = EcrioSigMgrMessageBodyUnknown;
	messageReq.pMessageBody->pMessageBody = pUnknownBody;
	pUnknownBody->pBuffer = pal_StringCreate(pSMSRetryMessage->pUnknownBody, pal_StringLength(pSMSRetryMessage->pUnknownBody));
	pUnknownBody->bufferLength = pal_StringLength(pSMSRetryMessage->pUnknownBody);
	messageReq.eModuleId = EcrioSigMgrCallbackRegisteringModule_SMS;

	count = pSMSRetryMessage->SmsRetryCount;
	
	count++;
	
	error = _EcrioSigMgrSendMessage(pSigMgr, &messageReq, count);

Error_Level_01:

	EcrioSigMgrReleaseUriStruct(pSigMgr, &toHeader.nameAddr.addrSpec);
	pMessageReq = &messageReq;
	pMessageReq->pMandatoryHdrs->pTo = NULL;
	pMessageReq->pMessageBody = NULL;
	EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_SipMessage, (void**)&pMessageReq, Enum_FALSE);
	
	if(pSMSRetryMessage)
	{
		if(pSMSRetryMessage->pDest)
		{
			pal_MemoryFree((void**)&pSMSRetryMessage->pDest);
		}		
		pal_MemoryFree((void**)&pSMSRetryMessage);
	}


	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);
}



/*****************************************************************************
                    API Definition Section - End
*****************************************************************************/
