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

/**
* @file lims_SendPagerMessage
* @brief Implementation of lims module send CPIM message to another user usin SIP protocol.
*/

#include "lims.h"
#include "lims_internal.h"

/**
* This API can be called to send stand alone mode instant message. The pTarget will
* point to a target CPM User. Default URI type lims assumes internally TEL URI.
*
* @param[in] handle				The lims instance handle.
* @param[in] pStandAloneMsg		Pointer to lims_StandAloneMessageStruct.
* @param[in,out] ppCallId		Pointer to unique call Id.
*
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
* If returned LIMS_SOCKET_ERROR, application need to re-initalize lims by invoking
* lims_Deinit(), lims_Init(), lims_NetworkStateChange() and lims_Register().
*/
u_int32 lims_SendStandAloneMessage
(
	LIMSHANDLE handle,
	lims_StandAloneMessageStruct* pPagerMsg,
	char **ppCallId
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
#if defined(ENABLE_RCS)
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
	EcrioCPMPagerMessageStruct cpmPagerMsgStruct = { 0 };
	u_char sipUri[LIMS_SIP_URI_LENGTH] = { 0 };
#endif 
	lims_moduleStruct *m = NULL;

	if (handle == NULL)
	{
		return LIMS_INVALID_PARAMETER1;
	}

	m = (lims_moduleStruct *)handle;

	pal_MutexGlobalLock(m->pal);

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

#if defined(ENABLE_RCS)
	if (pPagerMsg == NULL)
	{
		uLimsError = LIMS_INVALID_PARAMETER2;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t pMsgDetails is NULL",
			__FUNCTION__, __LINE__);
		goto ERR_None;
	}
	
	if (pPagerMsg->pDestUri == NULL ||
		(pPagerMsg->pMessage == NULL && pPagerMsg->bDeleteChatBotToken != Enum_TRUE))
	{
		uLimsError = LIMS_INVALID_PARAMETER2;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t pMsgDetails structure members are NULL",
			__FUNCTION__, __LINE__);
		goto ERR_None;

	}

	if (m->moduleState == lims_Module_State_REGISTERED)
	{
		u_char *pBuf = NULL;
		u_int32 uLen = 0;
		EcrioCPMBufferStruct imdnBuf = { 0 };

		cpmPagerMsgStruct.pDestUri = (u_char *)pPagerMsg->pDestUri;
		cpmPagerMsgStruct.pConvId = pPagerMsg->pConvId;
		cpmPagerMsgStruct.bDeleteChatBotToken = pPagerMsg->bDeleteChatBotToken;

		if (pPagerMsg->pMessage != NULL)
		{
			cpmPagerMsgStruct.baseMsg.eTrafficType = EcrioCPMTrafficTypeNone;
			cpmPagerMsgStruct.baseMsg.imdnConfig = pPagerMsg->pMessage->imdnConfig;
			cpmPagerMsgStruct.baseMsg.pIMDNMsgId = pPagerMsg->pMessage->pIMDNMsgId;
		}

		/* Code to add the IMDN details */
		if (pPagerMsg->pMessage != NULL && pPagerMsg->pMessage->eContentType == EcrioCPMContentTypeEnum_IMDN)
		{
			if ((NULL != pPagerMsg->pMessage->message.pIMDN) && (NULL != pPagerMsg->pMessage->message.pIMDN->pCPMIMDispoNotifBody))
			{
				/* Use 1st entry in dispostion notification body. Sender sends only 1 IMDN message at a time. */
				uCPMError = EcrioCPMFormIMDN(m->pCpmHandle, pPagerMsg->pMessage->message.pIMDN, &uLen, &pBuf, 0);
				if (uCPMError != ECRIO_CPM_NO_ERROR)
				{
					uLimsError = LIMS_CPM_FORMIMDN_ERROR;
					LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t EcrioCPMFormIMDN is failed",
						__FUNCTION__, __LINE__);
					goto ERR_None;
				}
				imdnBuf.pMessage = pBuf;
				imdnBuf.uMessageLen = uLen;
				cpmPagerMsgStruct.baseMsg.message.pBuffer = &imdnBuf;				
				cpmPagerMsgStruct.baseMsg.eContentType = EcrioCPMContentTypeEnum_IMDN;
				cpmPagerMsgStruct.baseMsg.pDestURI = (char*)pal_StringCreate((u_char*)pPagerMsg->pMessage->pDestURI,
					pal_StringLength((u_char*)pPagerMsg->pMessage->pDestURI));
			}	
		}
		else if (pPagerMsg->pMessage != NULL && (pPagerMsg->pMessage->eContentType == EcrioCPMContentTypeEnum_Text ||
					pPagerMsg->pMessage->eContentType == EcrioCPMContentTypeEnum_None))
		{
			cpmPagerMsgStruct.baseMsg.message.pBuffer = pPagerMsg->pMessage->message.pBuffer;
			cpmPagerMsgStruct.baseMsg.eContentType = EcrioCPMContentTypeEnum_Text;
		}

		if (pal_StringNCompare((u_char*)pPagerMsg->pDestUri, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			if (pal_StringFindSubString((u_char*)pPagerMsg->pDestUri, (u_char*)"@"))
			{
				u_int16 count = 4;
				u_int16 len = (u_int16)pal_StringLength((u_char*)pPagerMsg->pDestUri);
				if (NULL == pal_StringNCopy(sipUri, LIMS_SIP_URI_LENGTH, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")))
				{
					uLimsError = LIMS_NO_MEMORY;
					LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t Memory copy error.",
						__FUNCTION__, __LINE__);
					goto ERR_None;
				}
				while (count < len)
				{
					if (!pal_StringCompare((u_char*)pPagerMsg->pDestUri + count, (u_char*)"@"))
					{
						if (NULL == pal_StringNConcatenate(sipUri, LIMS_SIP_URI_LENGTH - pal_StringLength((u_char*)sipUri), (u_char*)"@", pal_StringLength((u_char*)"@")))
						{
							uLimsError = LIMS_NO_MEMORY;
							LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t Memory copy error.",
								__FUNCTION__, __LINE__);
							goto ERR_None;
						}
						if (count + 1 >= len)
						{
							if (NULL == pal_StringNConcatenate(sipUri, LIMS_SIP_URI_LENGTH - pal_StringLength((u_char*)sipUri), (u_char*)m->pConfig->pHomeDomain, pal_StringLength((u_char*)m->pConfig->pHomeDomain)))
							{
								uLimsError = LIMS_NO_MEMORY;
								LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t Memory copy error.",
									__FUNCTION__, __LINE__);
								goto ERR_None;
							}
							break;
						}
					}
					if (NULL == pal_StringNConcatenate(sipUri, LIMS_SIP_URI_LENGTH - pal_StringLength((u_char*)sipUri), (u_char*)pPagerMsg->pDestUri + count, 1))
					{
						uLimsError = LIMS_NO_MEMORY;
						LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t Memory copy error.",
							__FUNCTION__, __LINE__);
						goto ERR_None;
					}
					count++;
				}
				cpmPagerMsgStruct.pDestUri = sipUri;
			}
			else
			{
				if (NULL == pal_StringNCopy(sipUri, LIMS_SIP_URI_LENGTH, (u_char*)pPagerMsg->pDestUri, pal_StringLength((u_char*)pPagerMsg->pDestUri)))
				{
					uLimsError = LIMS_NO_MEMORY;
					LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t Memory copy error.",
						__FUNCTION__, __LINE__);
					goto ERR_None;
				}
				if (NULL == pal_StringNConcatenate(sipUri, LIMS_SIP_URI_LENGTH - pal_StringLength((u_char*)sipUri), (u_char*)"@", pal_StringLength((u_char*)"@")))
				{
					uLimsError = LIMS_NO_MEMORY;
					LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t Memory copy error.",
						__FUNCTION__, __LINE__);
					goto ERR_None;
				}
				if (NULL == pal_StringNConcatenate(sipUri, LIMS_SIP_URI_LENGTH - pal_StringLength((u_char*)sipUri), (u_char*)m->pConfig->pHomeDomain, pal_StringLength((u_char*)m->pConfig->pHomeDomain)))
				{
					uLimsError = LIMS_NO_MEMORY;
					LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t Memory copy error.",
						__FUNCTION__, __LINE__);
					goto ERR_None;
				}
				cpmPagerMsgStruct.pDestUri = sipUri;
			}
		}
		else
		{
			cpmPagerMsgStruct.pDestUri = (u_char*)pPagerMsg->pDestUri;
		}

		cpmPagerMsgStruct.bIsChatbot = pPagerMsg->bIsChatbot;

		uCPMError = EcrioCPMSendPagerModeMessage(m->pCpmHandle, &cpmPagerMsgStruct, (u_char **)ppCallId);
		if (uCPMError != ECRIO_CPM_NO_ERROR)
		{
			uLimsError = LIMS_CPM_SENDMESSAGE_ERROR;
			LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t EcrioSmsSendMessage() failed with error:%d",
				__FUNCTION__, __LINE__, uCPMError);
			goto ERR_None;
		}
	}
	else
	{
		uLimsError = LIMS_INVALID_OPERATION;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t lims is not in REGISTERED state",
			__FUNCTION__, __LINE__);
		goto ERR_None;
	}

#else
	LIMSLOGW(m->logHandle, KLogTypeGeneral, "%s:%u-  CPM feature disabled from this build", __FUNCTION__, __LINE__);
	uLimsError = LIMS_CPM_FEATURE_NOT_SUPPORTED;
	goto ERR_None;
#endif //ENABLE_RCS

ERR_None:

	if(cpmPagerMsgStruct.baseMsg.pDestURI != NULL)
		pal_MemoryFree((void**)&cpmPagerMsgStruct.baseMsg.pDestURI);

	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);

	pal_MutexGlobalUnlock(m->pal);

	return uLimsError;
}

/**
* This API is used to get the size of the CPIM payload that need to be sent out.
*
* @param[in] handle				The lims instance handle.
* @param[in] pStandAloneMsg		Pointer to lims_StandAloneMessageStruct.
* @param[out] pMsgSize			Pointer to size of the message.
*
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
* If returned LIMS_SOCKET_ERROR, application need to re-initalize lims by invoking
* lims_Deinit(), lims_Init(), lims_NetworkStateChange() and lims_Register().
*/
u_int32 lims_GetStandAloneMessageSize
(
	LIMSHANDLE handle,
	lims_StandAloneMessageStruct* pPagerMsg,
	u_int32* pMsgSize
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
#if defined(ENABLE_RCS)
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
	EcrioCPMPagerMessageStruct cpmPagerMsgStruct = { 0 };
#endif 
	lims_moduleStruct *m = NULL;

	if (handle == NULL)
	{
		return LIMS_INVALID_PARAMETER1;
	}

	m = (lims_moduleStruct *)handle;

	pal_MutexGlobalLock(m->pal);

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

#if defined(ENABLE_RCS)
	if (pPagerMsg == NULL)
	{
		uLimsError = LIMS_INVALID_PARAMETER2;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t pPagerMsg is NULL",
			__FUNCTION__, __LINE__);
		goto ERR_None;
	}

	if (pMsgSize == NULL)
	{
		uLimsError = LIMS_INVALID_PARAMETER3;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t pMsgSize is NULL",
			__FUNCTION__, __LINE__);
		goto ERR_None;
	}

	if (pPagerMsg->pDestUri == NULL ||
		pPagerMsg->pMessage == NULL)
	{
		uLimsError = LIMS_INVALID_PARAMETER2;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t pMsgDetails structure members are NULL",
			__FUNCTION__, __LINE__);
		goto ERR_None;

	}

	if (m->moduleState == lims_Module_State_REGISTERED /*&&
		pPagerMsg->pMessage->eContentType == EcrioCPMContentTypeEnum_Text*/)
	{
		cpmPagerMsgStruct.pDestUri = (u_char *)pPagerMsg->pDestUri;
		cpmPagerMsgStruct.pConvId = pPagerMsg->pConvId;
		cpmPagerMsgStruct.baseMsg.eTrafficType = EcrioCPMTrafficTypeNone;
		cpmPagerMsgStruct.baseMsg.imdnConfig = pPagerMsg->pMessage->imdnConfig;
		cpmPagerMsgStruct.baseMsg.pIMDNMsgId = pPagerMsg->pMessage->pIMDNMsgId;
		cpmPagerMsgStruct.baseMsg.message.pBuffer = pPagerMsg->pMessage->message.pBuffer;
		cpmPagerMsgStruct.baseMsg.eContentType = EcrioCPMContentTypeEnum_Text;

		uCPMError = EcrioCPMGetPagerModeMessageSize(m->pCpmHandle, &cpmPagerMsgStruct, pMsgSize);
		if (uCPMError != ECRIO_CPM_NO_ERROR)
		{
			uLimsError = LIMS_CPM_SENDMESSAGE_ERROR;
			LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t EcrioCPMGetPagerModeMessageSize() failed with error:%d",
				__FUNCTION__, __LINE__, uCPMError);
			goto ERR_None;
		}
	}
	else
	{
		uLimsError = LIMS_INVALID_OPERATION;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t lims is not in REGISTERED state OR ContentType != EcrioCPMContentTypeEnum_Text",
			__FUNCTION__, __LINE__);
		goto ERR_None;
	}

#else
	LIMSLOGW(m->logHandle, KLogTypeGeneral, "%s:%u-  CPM feature disabled from this build", __FUNCTION__, __LINE__);
	uLimsError = LIMS_CPM_FEATURE_NOT_SUPPORTED;
	goto ERR_None;
#endif //ENABLE_RCS

ERR_None:

	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);

	pal_MutexGlobalUnlock(m->pal);

	return uLimsError;
}