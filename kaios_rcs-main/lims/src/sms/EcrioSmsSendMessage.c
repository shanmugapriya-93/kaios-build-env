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

/** API for Sending a SMS using SMS ALC. This API must be used once the SMS
    is initialsed.

*@ Param IN ECRIOSMSHANDLE hEcrioSMS  -  The SMS Handle.
*@ Param IN u_char* pUserID					-  SMS Receiver Id
*@ Param IN u_int32 dwMessageLen            -  Message Length.
*@ Param IN u_char pMessage                  -  Message to be sent .
*@ Param OUT u_char** ppMsgId				-  Message id given by UAE.
*@ return u_int32: Indicating the send status ECRIO_SMS_SUCCESS or
                                              a non-zero value indicating reason for failure.
**/
u_int32 EcrioSmsSendMessage
(
	ECRIOSMSHANDLE hEcrioSMS,
	EcrioSmsMessageStruct *pMsgStruct
)
{
	EcrioSMSContext *pContext = NULL;
	u_int32 uSigError = ECRIO_SIGMGR_NO_ERROR;
	u_int32 uSMSError = ECRIO_SMS_NO_ERROR;
	EcrioSigMgrSipMessageStruct messageReq = { .eMethodType = EcrioSipMessageTypeNone };
	EcrioSigMgrSipMessageStruct *pMessageReq = NULL;
	EcrioSigMgrMandatoryHeaderStruct *pMandatoryHeader = NULL;
	EcrioSigMgrNameAddrWithParamsStruct toHeader = { 0 };
	EcrioSigMgrMessageBodyStruct *pMessageBodyStruct = NULL;
	EcrioSigMgrUnknownMessageBodyStruct *pUnknownBody = NULL;
	u_char* pContentType = NULL;
	u_char *pEncoding = NULL;
	u_char **ppVal = NULL;

	if (NULL == hEcrioSMS)
	{
		return ECRIO_SMS_NOT_INITIALIZED_ERROR;
	}

	pContext = (EcrioSMSContext *)hEcrioSMS;

	SMSLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pMsgStruct == NULL)
	{
		SMSLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpMsgStruct is NULL", __FUNCTION__, __LINE__);
		uSMSError = ECRIO_SMS_INSUFFICIENT_DATA_ERROR;
		goto Error_Level_01;
	}

	/** Populate toHeader in tel uri format */
	uSigError = EcrioSigMgrFillUriStruct(pContext->hEcrioSigMgrHandle,
		&toHeader.nameAddr.addrSpec,
		pMsgStruct->pDest, EcrioSigMgrURISchemeTEL);
	if (uSigError != ECRIO_SIGMGR_NO_ERROR)
	{
		SMSLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrFillUriStruct() error=%u",
			__FUNCTION__, __LINE__, uSigError);

		uSMSError = ECRIO_SMS_INTERNAL_ERROR;
		goto Error_Level_01;
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void**)&pMandatoryHeader);
	if (pMandatoryHeader == NULL)
	{
		SMSLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		uSMSError = ECRIO_SMS_MEMORY_ERROR;
		goto Error_Level_01;
	}

	pMandatoryHeader->pTo = &toHeader;
	messageReq.pMandatoryHdrs = pMandatoryHeader;

	switch (pMsgStruct->format)
	{
	case ECRIO_SMS_MSG_FORMAT_3GPP:
	{
		pContentType = ECRIO_SMS_CONTENTTYPE_3GPPSMS;
	}
	break;

	case ECRIO_SMS_MSG_FORMAT_3GPP2:
	default:
	{
		pContentType = ECRIO_SMS_CONTENTTYPE_3GPP2SMS;
	}
	break;
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrUnknownMessageBodyStruct), (void**)&pUnknownBody);
	if (pUnknownBody == NULL)
	{
		SMSLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		uSMSError = ECRIO_SMS_MEMORY_ERROR;
		goto Error_Level_01;
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrMessageBodyStruct), (void**)&pMessageBodyStruct);
	if (pMessageBodyStruct == NULL)
	{
		SMSLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		uSMSError = ECRIO_SMS_MEMORY_ERROR;
		goto Error_Level_01;
	}

	pUnknownBody->contentType.numParams = 0;
	pUnknownBody->contentType.ppParams = NULL;
	pUnknownBody->contentType.pHeaderValue = pal_StringCreate(pContentType, pal_StringLength(pContentType));
	messageReq.pMessageBody = pMessageBodyStruct;
	messageReq.pMessageBody->messageBodyType = EcrioSigMgrMessageBodyUnknown;
	messageReq.pMessageBody->pMessageBody = pUnknownBody;
	pUnknownBody->pBuffer = pal_StringCreate(pMsgStruct->pMessage, pal_StringLength(pMsgStruct->pMessage));
	pUnknownBody->bufferLength = pMsgStruct->uMsgLen;

	messageReq.eModuleId = EcrioSigMgrCallbackRegisteringModule_SMS;

	/** Check and Populate optional header. */
	if (messageReq.pOptionalHeaderList == NULL)
	{
		uSigError = EcrioSigMgrCreateOptionalHeaderList(pContext->hEcrioSigMgrHandle, &messageReq.pOptionalHeaderList);
		if (uSigError != ECRIO_SIGMGR_NO_ERROR)
		{
			SMSLOGE(pContext->hLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSigMgrCreateOptionalHeaderList() error=%u",
				__FUNCTION__, __LINE__, uSigError);

			uSMSError = ECRIO_SMS_INTERNAL_ERROR;
			goto Error_Level_01;
		}
	}

	/** Populate optional Content-Transfer-Encoding header */
	{
		switch (pMsgStruct->eEncoding)
		{
			case EcrioSmsContentTransferEncoding_Binary:
			{
				pEncoding = ECRIO_SMS_CONTENT_TRANSFER_ENCODING_BINARY;
			}
			break;

			default:
			{
				pEncoding = NULL;
			}
			break;
		}

		if (pEncoding != NULL)
		{
			if (ppVal == NULL)
			{
				pal_MemoryAllocate(1 * sizeof(u_char *), (void **)&ppVal);
				if (ppVal == NULL)
				{
					SMSLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
						__FUNCTION__, __LINE__);
					uSMSError = ECRIO_SMS_MEMORY_ERROR;
					goto Error_Level_01;
				}
			}
			ppVal[0] = pal_StringCreate(pEncoding, pal_StringLength(pEncoding));

			uSigError = EcrioSigMgrAddOptionalHeader(pContext->hEcrioSigMgrHandle, messageReq.pOptionalHeaderList,
				EcrioSipHeaderTypeContentTransferEncoding, 1, ppVal, 0, NULL, NULL);
			if (uSigError != ECRIO_SIGMGR_NO_ERROR)
			{
				SMSLOGE(pContext->hLogHandle, KLogTypeGeneral,
					"%s:%u\tEcrioSigMgrCreateOptionalHeaderList() error=%u",
					__FUNCTION__, __LINE__, uSigError);

				uSMSError = ECRIO_SMS_INTERNAL_ERROR;
				goto Error_Level_01;
			}

			pal_MemoryFree((void **)&ppVal[0]);
			ppVal[0] = NULL;
		}
	}

	uSigError = EcrioSigMgrSendInstantMessage(pContext->hEcrioSigMgrHandle, &messageReq);
	if (uSigError != ECRIO_SIGMGR_NO_ERROR)
	{
		/* Set SigMgr-error to ALC specific error */
		uSMSError = ECRIO_SMS_INTERNAL_ERROR;

		SMSLOGE(pContext->hLogHandle, KLogTypeGeneral,
			"%s:%u\tEcrioSigMgrSendInstantMessage() uSigError=%u, uSMSError=%u",
			__FUNCTION__, __LINE__, uSigError, uSMSError);

		goto Error_Level_01;
	}

Error_Level_01:

	if (ppVal)
	{
		if (ppVal[0])
		{
			pal_MemoryFree((void **)&ppVal[0]);
		}
		pal_MemoryFree((void **)&ppVal);
	}

	EcrioSigMgrReleaseUriStruct(pContext->hEcrioSigMgrHandle, &toHeader.nameAddr.addrSpec);
	pMessageReq = &messageReq;
	pMessageReq->pMandatoryHdrs->pTo = NULL;
	pMessageReq->pMessageBody = NULL;
	EcrioSigMgrStructRelease(pContext->hEcrioSigMgrHandle, EcrioSigMgrStructType_SipMessage, (void**)&pMessageReq, Enum_FALSE);

	SMSLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u\tError=%u",
		__FUNCTION__, __LINE__, uSMSError);

	return uSMSError;
}
