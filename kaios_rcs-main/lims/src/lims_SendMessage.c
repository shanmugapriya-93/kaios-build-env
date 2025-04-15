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
* @file lims_SendMessage
* @brief Implementation of lims module send text message with/without suggested chiplist functionality.
*/

#include "lims.h"
#include "lims_internal.h"

/**
* This API can be called to send messages within a CPM session.
* Message within session includes, simple text, Rich card, Suggested Request, Suggested Response
* and JSON payload of File Transfer over HTTP.
*
* @param[in] handle			   The lims instance handle.
* @param[in] pSendMessage	   Pointer to lims_SendMessageStruct.
* @param[in,out] ppMessageId   Message Identifier.
*
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
*/
u_int32 lims_SendMessage
(
	LIMSHANDLE handle,
	lims_SendMessageStruct* pSendMessage,
	char **ppMessageId
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
#if defined(ENABLE_RCS)
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
	EcrioCPMSendMessageStruct cpmSendMsg = { 0 };
	u_char *pBuf = NULL;
	u_int32 uLen = 0;
	EcrioCPMBufferStruct imdnBuf = { 0 };

#endif //ENABLE_RCS

	lims_moduleStruct *m = NULL;

	if (handle == NULL)
	{
		return LIMS_INVALID_PARAMETER1;
	}

	m = (lims_moduleStruct *)handle;

	pal_MutexGlobalLock(m->pal);

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
#if defined(ENABLE_RCS)
	if (pSendMessage == NULL)
	{
		uLimsError = LIMS_INVALID_PARAMETER2;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t pSendMessage is NULL",
			__FUNCTION__, __LINE__);
		goto ERR_None;
	}

	if (pSendMessage->pMessage == NULL || pSendMessage->pMessage->pIMDNMsgId == NULL)
	{
		uLimsError = LIMS_INVALID_PARAMETER2;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t pIMDNMsgId is NULL",
			__FUNCTION__, __LINE__);
		goto ERR_None;
	}

	/*Populate base message*/
	cpmSendMsg.pSessionId = pSendMessage->pSessionId;
	cpmSendMsg.baseMsg.eTrafficType = pSendMessage->pMessage->eTrafficType;
	cpmSendMsg.baseMsg.imdnConfig = pSendMessage->pMessage->imdnConfig;
	cpmSendMsg.baseMsg.pIMDNMsgId = pSendMessage->pMessage->pIMDNMsgId;
	cpmSendMsg.baseMsg.pBotSuggestion = pSendMessage->pMessage->pBotSuggestion;
	cpmSendMsg.baseMsg.eContentType = pSendMessage->pMessage->eContentType;
	/* Code to add the IMDN details */
	if (pSendMessage->pMessage->eContentType == EcrioCPMContentTypeEnum_IMDN)
	{
		if ((NULL != pSendMessage->pMessage->message.pIMDN) && (NULL != pSendMessage->pMessage->message.pIMDN->pCPMIMDispoNotifBody))
		{
			/* Use 1st entry in dispostion notification body. Sender sends only 1 IMDN message at a time. */
			uCPMError = EcrioCPMFormIMDN(m->pCpmHandle, pSendMessage->pMessage->message.pIMDN, &uLen, &pBuf, 0);
			if (uCPMError != ECRIO_CPM_NO_ERROR)
			{
				uLimsError = LIMS_CPM_FORMIMDN_ERROR;
				LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t EcrioCPMFormIMDN is failed",
					__FUNCTION__, __LINE__);
				goto ERR_None;
			}
			imdnBuf.pMessage = pBuf;
			imdnBuf.uMessageLen = uLen;
			cpmSendMsg.baseMsg.message.pBuffer = &imdnBuf;
			cpmSendMsg.baseMsg.pDestURI = (char*)pal_StringCreate((u_char*)pSendMessage->pMessage->message.pIMDN->pDestURI,
				pal_StringLength((u_char*)pSendMessage->pMessage->message.pIMDN->pDestURI));
		}
	}
	else if (pSendMessage->pMessage->eContentType == EcrioCPMContentTypeEnum_Text ||
		pSendMessage->pMessage->eContentType == EcrioCPMContentTypeEnum_FileTransferOverHTTP ||
		pSendMessage->pMessage->eContentType == EcrioCPMContentTypeEnum_PushLocation)
	{
		cpmSendMsg.baseMsg.message.pBuffer = pSendMessage->pMessage->message.pBuffer;
	}
	else if (pSendMessage->pMessage->eContentType == EcrioCPMContentTypeEnum_Composing)
	{
		cpmSendMsg.baseMsg.message.pComposing = pSendMessage->pMessage->message.pComposing;
		//cpmSendMsg.baseMsg.pDestURI = pal_StringCreate(pSendMessage->pMessage->message.pComposing->pDestURI,
			//pal_StringLength(pSendMessage->pMessage->message.pComposing->pDestURI));
	}
	else if (pSendMessage->pMessage->eContentType == EcrioCPMContentTypeEnum_RichCard)
	{
		cpmSendMsg.baseMsg.pBotSuggestion = pSendMessage->pMessage->pBotSuggestion;
		cpmSendMsg.baseMsg.message.pBuffer = pSendMessage->pMessage->message.pBuffer;
	}

	uCPMError = EcrioCPMSendMessage(m->pCpmHandle, &cpmSendMsg, (u_char **)ppMessageId);
	if (uCPMError != ECRIO_CPM_NO_ERROR)
	{
		uLimsError = LIMS_CPM_SENDMESSAGE_ERROR;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t EcrioCPMSendMessage is failed",
			__FUNCTION__, __LINE__);
		goto ERR_None;
	}
#else
	LIMSLOGW(m->logHandle, KLogTypeGeneral, "%s:%u-  CPM feature disabled from this build", __FUNCTION__, __LINE__);
	uLimsError = LIMS_CPM_FEATURE_NOT_SUPPORTED;
	goto ERR_None;
#endif //ENABLE_RCS

ERR_None:

    if(cpmSendMsg.baseMsg.pDestURI != NULL)
        pal_MemoryFree((void**)&cpmSendMsg.baseMsg.pDestURI);

	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);

	pal_MutexGlobalUnlock(m->pal);

	return uLimsError;
}
