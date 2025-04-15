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

#include "EcrioSms.h"
#include "EcrioSmsInternal.h"

void SMSSigMgrInfoCallback
(
	EcrioSigMgrInfoTypeEnum infoType,
	s_int32 infoCode,
	EcrioSigMgrInfoStruct *pData,
	void *pCallbackData
)
{
	(void)infoType;
	(void)infoCode;
	(void)pData;
	(void)pCallbackData;
}

void SMSSigMgrStatusCallback
(
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct,
	void *pCallbackData
)
{
	EcrioSMSContext *pContextStruct = NULL;

	if ((pCallbackData == NULL) || (pSigMgrMessageStruct == NULL))
	{
		return;
	}

	pContextStruct = (EcrioSMSContext *)pCallbackData;

	SMSLOGI(pContextStruct->hLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	switch (pSigMgrMessageStruct->msgCmd)
	{
		case EcrioSigMgrInstantMessageResponse:	/* final response received */
		{_EcrioSMSHandleMessageResponse(pContextStruct, pSigMgrMessageStruct);
		}
		break;

		case EcrioSigMgrInstantMessageRequestNotification:	/* incoming MESSAGE received */
		{_EcrioSMSHandleMessageRequest(pContextStruct, pSigMgrMessageStruct);
		}
		break;

		default:
		{
			SMSLOGW(pContextStruct->hLogHandle, KLogTypeFuncExit, "%s:%u\tUnhandled command %d",
				__FUNCTION__, __LINE__, pSigMgrMessageStruct->msgCmd);
		}
		break;
	}

	SMSLOGI(pContextStruct->hLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}

void _EcrioSMSHandleMessageResponse
(
	EcrioSMSContext *pContextStruct,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
)
{
	EcrioSmsStatusStruct status = { 0 };
	EcrioSigMgrSipMessageStruct* pMessage = NULL;

	SMSLOGI(pContextStruct->hLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pMessage = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;
	if (pMessage == NULL)
	{
		goto End;
	}

	status.uStatusCode = pMessage->responseCode;


	/* call the app callback */
	pContextStruct->callbackStruct.pCallback(EcrioSmsMessage_Status_SEND,
		(void *)&status,
		(void *)pContextStruct->callbackStruct.pContext);

End:

	SMSLOGI(pContextStruct->hLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}

void _EcrioSMSHandleMessageRequest
(
	EcrioSMSContext *pContextStruct,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
)
{
	EcrioSmsMessageStruct *pRcvdMsgStruct = NULL;
	EcrioSigMgrUnknownMessageBodyStruct *pUnknownBody = NULL;
	EcrioSigMgrSipMessageStruct *pMessageReqNtf = NULL;
	EcrioSigMgrHeaderStruct *pHeader = NULL;
	u_int32 i;

	SMSLOGI(pContextStruct->hLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pMessageReqNtf = (EcrioSigMgrSipMessageStruct *)(pSigMgrMessageStruct->pData);
	if (pMessageReqNtf == NULL)
	{
		goto End;
	}

	pal_MemoryAllocate(sizeof(EcrioSmsMessageStruct), (void **)&pRcvdMsgStruct);
	if (pRcvdMsgStruct == NULL)
	{
		//error = ECRIO_SIGMGR_NO_MEMORY;
		goto End;
	}

	pUnknownBody = (EcrioSigMgrUnknownMessageBodyStruct *)(pMessageReqNtf->pMessageBody->pMessageBody);
	pRcvdMsgStruct->uMsgLen = pUnknownBody->bufferLength;
	if (pRcvdMsgStruct->uMsgLen == 0 || pUnknownBody->pBuffer == NULL)
	{
		goto End;
	}

	/* fill the received MESSAGE data into SMS pRcvdMsgStruct */
	pal_MemoryAllocate(pRcvdMsgStruct->uMsgLen + 1, (void **)&(pRcvdMsgStruct->pMessage));
	if (pRcvdMsgStruct->pMessage == NULL)
	{
		goto End;
	}

	if (KPALErrorNone != pal_MemoryCopy((void *)pRcvdMsgStruct->pMessage, pRcvdMsgStruct->uMsgLen + 1, (void *)pUnknownBody->pBuffer, pRcvdMsgStruct->uMsgLen))
	{
		SMSLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
			__FUNCTION__, __LINE__);
		goto End;
	}
	pRcvdMsgStruct->pMessage[pRcvdMsgStruct->uMsgLen] = '\0';

	if (!(pal_StringCompare((u_char *)pUnknownBody->contentType.pHeaderValue, ECRIO_SMS_CONTENTTYPE_3GPP2SMS)))
	{
		pRcvdMsgStruct->format = ECRIO_SMS_MSG_FORMAT_3GPP2;
	}
	else if (!(pal_StringCompare((u_char *)pUnknownBody->contentType.pHeaderValue, ECRIO_SMS_CONTENTTYPE_3GPPSMS)))
	{
		pRcvdMsgStruct->format = ECRIO_SMS_MSG_FORMAT_3GPP;
	}

	if (pMessageReqNtf->ppPAssertedIdentity)
	{
		EcrioSigMgrGetOriginator(pContextStruct->hEcrioSigMgrHandle, pMessageReqNtf->ppPAssertedIdentity[0], &pRcvdMsgStruct->pDest);
	}
	else
	{
		EcrioSigMgrGetOriginator(pContextStruct->hEcrioSigMgrHandle, &pMessageReqNtf->pMandatoryHdrs->pFrom->nameAddr, &pRcvdMsgStruct->pDest);
	}

	/** Verify Content-Transfer-Encoding header */
	pRcvdMsgStruct->eEncoding = EcrioSmsContentTransferEncoding_None;
	EcrioSigMgrGetOptionalHeader(pContextStruct->hEcrioSigMgrHandle, pMessageReqNtf->pOptionalHeaderList, EcrioSipHeaderTypeContentTransferEncoding, &pHeader);

	if (pHeader != NULL)
	{
		for (i = 0; i < pHeader->numHeaderValues; i++)
		{
			if (pHeader->ppHeaderValues && pHeader->ppHeaderValues[i])
			{
				/** Check whether Content-Transfer-Encoding header fields are indicated "binary". */
				if (pal_SubString(pHeader->ppHeaderValues[i]->pHeaderValue, ECRIO_SMS_CONTENT_TRANSFER_ENCODING_BINARY))
				{
					pRcvdMsgStruct->eEncoding = EcrioSmsContentTransferEncoding_Binary;
				}
			}
		}
	}

	SMSLOGI(pContextStruct->hLogHandle, KLogTypeFuncParams, "%s:%u\t notifying incoming MESSAGE with pUserId = %s dwMessageLen = %u eMsgFormatType = %d eEncoding = %s",
		__FUNCTION__, __LINE__,
		(const char *)pRcvdMsgStruct->pDest ? (const char *)pRcvdMsgStruct->pDest : "null",
		pRcvdMsgStruct->uMsgLen, pRcvdMsgStruct->format,
		pRcvdMsgStruct->eEncoding == EcrioSmsContentTransferEncoding_Binary ? "binary" : "(none)");

	/* call the app callback */
	pContextStruct->callbackStruct.pCallback(EcrioSmsMessage_Status_RECV,
		(void *)pRcvdMsgStruct,
		(void *)pContextStruct->callbackStruct.pContext);

End:

	if (pRcvdMsgStruct)
	{
		pal_MemoryFree((void **)&pRcvdMsgStruct->pDest);
		pal_MemoryFree((void **)&pRcvdMsgStruct->pMessage);
		pal_MemoryFree((void **)&pRcvdMsgStruct);
	}

	SMSLOGI(pContextStruct->hLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}
