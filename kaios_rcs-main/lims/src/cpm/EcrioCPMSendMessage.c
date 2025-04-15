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

#include "EcrioCPM.h"
#include "EcrioCPMInternal.h"

u_int32 EcrioCPMSendMessage
(
	CPMHANDLE hCPMHandle,
	EcrioCPMSendMessageStruct* pTextMessage,
	u_char **ppMessageId
)
{
	u_int32 uError = ECRIO_CPM_NO_ERROR;
	u_int32 uCPIMError = ECRIO_CPIM_NO_ERROR;
	EcrioCPMContextStruct *pContext = NULL;
	u_int32 uLen = 0;
	u_char *pBuf = NULL;
	LOGHANDLE hLogHandle = NULL;
	EcrioCPMSessionStruct* pSession = NULL;
	CPIMMessageStruct cpimMessage = { 0 };
	MSRPTextMessageStruct msrpMessage = { 0 };
	u_char *pCallId = NULL;
	/* NULL --> sip:anonymous@anonymous.invalid will be populated. */
	u_char* pFromAddr = NULL;
	u_char* pToAddr = NULL;
	u_char* pDisplayName = NULL;
	EcrioSigMgrNameAddrWithParamsStruct *pUID = NULL;
	EcrioSigMgrNameAddrWithParamsStruct toHeader = { 0 };
	EcrioCPMBufferStruct composingBuf = { 0 };

	if ((NULL == hCPMHandle) || (NULL == pTextMessage))
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	/** Get the CPM structure from the handle */
	pContext = (EcrioCPMContextStruct *)hCPMHandle;

	hLogHandle = pContext->hLogHandle;

	CPMLOGI(hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);	

	if (pContext->bIsRelayEnabled == Enum_TRUE)
		ec_CPM_MapGetData(pContext, (u_char *)pTextMessage->pSessionId, (void**)&pCallId);
	else
		pCallId = (u_char *)pTextMessage->pSessionId;

	EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, pCallId, (void**)&pSession);

	if (pSession == NULL)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tpSession not found for pSessionId=%s",
			__FUNCTION__, __LINE__, pCallId);
		goto Error_Level_01;
	}

	if (pSession->state != EcrioCPMSessionState_Active)
	{
		uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;

		CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tInvalid sessionId uError=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}
	
	if ((EcrioCPMContentTypeEnum_Composing == pTextMessage->baseMsg.eContentType && pSession->bIsGroupChat == Enum_TRUE) ||
			EcrioCPMContentTypeEnum_Composing != pTextMessage->baseMsg.eContentType)
	{
		/* IsComposing for GroupChat Only. Send isComposing wrapped in CPIM using MSRP */
		if (EcrioCPMContentTypeEnum_Composing == pTextMessage->baseMsg.eContentType &&
			pSession->bIsGroupChat == Enum_TRUE)
		{
			msrpMessage.type = MSRPMessageType_Composition;
			msrpMessage.u.comp.pContent = pTextMessage->baseMsg.message.pComposing->pContent;

			switch (pTextMessage->baseMsg.message.pComposing->eComposingState)
			{
			case EcrioCPMComposingType_Idle:
			{
				msrpMessage.u.comp.state = MSRPIsComposingState_Idle;
			}
			break;

			case EcrioCPMComposingType_Active:
			{
				msrpMessage.u.comp.state = MSRPIsComposingState_Active;
			}
			break;
			default:
			{
				msrpMessage.u.comp.state = MSRPIsComposingState_None;
			}
			}
			msrpMessage.u.comp.uRefresh = pTextMessage->baseMsg.message.pComposing->uRefresh;
			msrpMessage.u.comp.pLastActive = pTextMessage->baseMsg.message.pComposing->pLastActive;
			EcrioMSRPFormComposingXMLDocument(pSession->hMSRPSessionHandle, &msrpMessage.u.comp, &pBuf, &uLen);
			/*Wrap isComposing within CPIM message */
			composingBuf.pMessage = pBuf;
			composingBuf.uMessageLen = uLen;
			pTextMessage->baseMsg.message.pBuffer = &composingBuf;
		}

		/* Fill From and To with valid value for IMDN content-type OR for group chat messages.*/
		if (pTextMessage->baseMsg.eContentType == EcrioCPMContentTypeEnum_IMDN || 
			pTextMessage->baseMsg.eContentType == EcrioCPMContentTypeEnum_Composing ||
			pSession->bIsGroupChat == Enum_TRUE)
		{
			u_int32 error = ECRIO_SIGMGR_NO_ERROR;
			u_char* pUri = NULL;

			if (pSession->bIsGroupChat == Enum_TRUE)
				pToAddr = pal_StringCreate((u_char*)pTextMessage->baseMsg.pDestURI, pal_StringLength((u_char*)pTextMessage->baseMsg.pDestURI));
			else
			{
				error = EcrioSigMgrPopulateToHeader(pContext->hEcrioSigMgrHandle, (u_char*)pTextMessage->baseMsg.pDestURI, &toHeader);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrPopulateToHeader() error=%u",
						__FUNCTION__, __LINE__, error);

					pToAddr = NULL;
				}
				else
				{
					if (toHeader.nameAddr.addrSpec.uriScheme == EcrioSigMgrURISchemeSIP)
					{
						EcrioSigMgrFormSipURI(pContext->hEcrioSigMgrHandle, toHeader.nameAddr.addrSpec.u.pSipUri, &pUri);
						pToAddr = pUri;
					}
					else if (toHeader.nameAddr.addrSpec.uriScheme == EcrioSigMgrURISchemeTEL)
					{
						EcrioSigMgrFormTelURI(pContext->hEcrioSigMgrHandle, toHeader.nameAddr.addrSpec.u.pTelUri, &pUri);
						pToAddr = pUri;
					}
					else
						pToAddr = pal_StringCreate((u_char*)pTextMessage->baseMsg.pDestURI, pal_StringLength((u_char*)pTextMessage->baseMsg.pDestURI));

					pUID = &toHeader;
					EcrioSigMgrStructRelease(pContext->hEcrioSigMgrHandle, EcrioSigMgrStructType_NameAddr, (void **)(&pUID), Enum_FALSE);
				}
			}

			pUri = NULL;
			error = EcrioSigMgrGetDefaultPUID(pContext->hEcrioSigMgrHandle, EcrioSigMgrURISchemeTEL, &pUID);
			if (error != ECRIO_SIGMGR_NO_ERROR || pUID == NULL)
			{
				CPMLOGE(hLogHandle, KLogTypeGeneral,
					"%s:%u\tEcrioSigMgrGetDefaultPUID(TEL_URI) populate To uri, error=%u",
					__FUNCTION__, __LINE__, error);
				error = EcrioSigMgrGetDefaultPUID(pContext->hEcrioSigMgrHandle, EcrioSigMgrURISchemeSIP, &pUID);
				if (error != ECRIO_SIGMGR_NO_ERROR || pUID == NULL)
				{
					CPMLOGE(hLogHandle, KLogTypeGeneral,
							"%s:%u\tEcrioSigMgrGetDefaultPUID(SIP_URI) populate To uri, error=%u",
							__FUNCTION__, __LINE__, error);
					uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
					goto Error_Level_01;
				}
				else
				{
					error = EcrioSigMgrFormSipURI(pContext->hEcrioSigMgrHandle, pUID->nameAddr.addrSpec.u.pSipUri, &pUri);
					if (error != ECRIO_SIGMGR_NO_ERROR)
					{
						CPMLOGE(hLogHandle, KLogTypeGeneral,
							"%s:%u\tEcrioSigMgrFormSipURI() returned with failure, error=%u",
							__FUNCTION__, __LINE__, error);
						uError = ECRIO_CPM_INTERNAL_ERROR;
						goto Error_Level_01;
					}
				}
			}
			else
			{
				error = EcrioSigMgrFormTelURI(pContext->hEcrioSigMgrHandle, pUID->nameAddr.addrSpec.u.pTelUri, &pUri);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					CPMLOGE(hLogHandle, KLogTypeGeneral,
						"%s:%u\tEcrioSigMgrFormTelURI() returned with failure, error=%u",
						__FUNCTION__, __LINE__, error);
					uError = ECRIO_CPM_INTERNAL_ERROR;
					goto Error_Level_01;
				}
			}
			if (pContext->pDisplayName != NULL)
			{
				pDisplayName = pContext->pDisplayName;
			}
			else
			{
				pDisplayName = pUID->nameAddr.pDisplayName;
			}
			pFromAddr = pUri;
		}
		
		/*Populate CPIM Message*/
		uError = ec_CPM_PopulateCPIMHeaders(pContext, pFromAddr, pDisplayName, pToAddr,
			&pTextMessage->baseMsg, &cpimMessage);
		if (uError != ECRIO_CPM_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_PopulateCPIMHeaders() error=%u",
				__FUNCTION__, __LINE__, uError);

			goto Error_Level_01;
		}

		if (pContext->pPANI != NULL)
		{
			cpimMessage.pPANI = pContext->pPANI;
		}

		pBuf = EcrioCPIMForm(pContext->hCPIMHandle, &cpimMessage, &uLen, &uCPIMError);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioCPIMForm failed, uError=%u",
				__FUNCTION__, __LINE__, uCPIMError);
			uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
			goto Error_Level_01;
		}
	}
	
	if ((EcrioCPMContentTypeEnum_Text == pTextMessage->baseMsg.eContentType) ||
		(EcrioCPMContentTypeEnum_IMDN == pTextMessage->baseMsg.eContentType) ||
		(EcrioCPMContentTypeEnum_RichCard == pTextMessage->baseMsg.eContentType) ||
		(EcrioCPMContentTypeEnum_SuggestedChipList == pTextMessage->baseMsg.eContentType) ||
		(EcrioCPMContentTypeEnum_SuggestionResponse == pTextMessage->baseMsg.eContentType) ||
		(EcrioCPMContentTypeEnum_FileTransferOverHTTP == pTextMessage->baseMsg.eContentType) ||
		(EcrioCPMContentTypeEnum_Composing == pTextMessage->baseMsg.eContentType && pSession->bIsGroupChat == Enum_TRUE) ||
		(EcrioCPMContentTypeEnum_PushLocation == pTextMessage->baseMsg.eContentType))
	{
		/*Send message using MSRP session*/
		msrpMessage.type = MSRPMessageType_CPIM;
		msrpMessage.u.text.pMsg = pBuf;
		msrpMessage.u.text.uMsgLen = uLen;
	}
	/* IsComposing for 1-1 Chat Only. Send isComposing in MSRP packet */
	else if (EcrioCPMContentTypeEnum_Composing == pTextMessage->baseMsg.eContentType &&
			pSession->bIsGroupChat == Enum_FALSE)
	{
		msrpMessage.type = MSRPMessageType_Composition;
		msrpMessage.u.comp.pContent = pTextMessage->baseMsg.message.pComposing->pContent;
		
		switch (pTextMessage->baseMsg.message.pComposing->eComposingState)
		{
			case EcrioCPMComposingType_Idle:
			{
				msrpMessage.u.comp.state = MSRPIsComposingState_Idle;
			}
			break;

			case EcrioCPMComposingType_Active:
			{
				msrpMessage.u.comp.state = MSRPIsComposingState_Active;
			}
			break;
			default:
			{
				msrpMessage.u.comp.state = MSRPIsComposingState_None;
			}
		}		
		msrpMessage.u.comp.uRefresh = pTextMessage->baseMsg.message.pComposing->uRefresh;
		msrpMessage.u.comp.pLastActive = pTextMessage->baseMsg.message.pComposing->pLastActive;
	}

	if (EcrioMSRPSendTextMessage(pSession->hMSRPSessionHandle, &msrpMessage) != ECRIO_MSRP_NO_ERROR)
	{
		CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioMSRPSendTextMessage failed", __FUNCTION__, __LINE__);
		uError = ECRIO_CPM_MSRP_ERROR;
		goto Error_Level_01;
	}

	/*store MSRP Message id */
	if (msrpMessage.pMessageId && ppMessageId)
	{
		*ppMessageId = pal_StringCreate((u_char*)msrpMessage.pMessageId, pal_StringLength((u_char*)msrpMessage.pMessageId));
	}

Error_Level_01:
	if (pFromAddr != NULL)
		pal_MemoryFree((void**)&pFromAddr);

	if (pToAddr != NULL)
		pal_MemoryFree((void**)&pToAddr);

	if (pUID != NULL)
		EcrioSigMgrStructRelease(pContext->hEcrioSigMgrHandle, EcrioSigMgrStructType_NameAddr, (void **)(&pUID), Enum_TRUE);

	CPMLOGI(hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uError;
}
