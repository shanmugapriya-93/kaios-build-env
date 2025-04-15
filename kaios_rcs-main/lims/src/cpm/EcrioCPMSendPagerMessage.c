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

u_int32 EcrioCPMSendPagerModeMessage
(
	CPMHANDLE hCPMHandle,
	EcrioCPMPagerMessageStruct* pPagerMessage,
	u_char **ppCallId
)
{
	u_int32 uError = ECRIO_CPM_NO_ERROR;
	u_int32 uSigError = ECRIO_SIGMGR_NO_ERROR;
	u_char* pMessageBody = NULL;
	u_int32 mssgLen = 0;
	LOGHANDLE hLogHandle = NULL;
	CPIMMessageStruct cpimStruct = { 0 };
	EcrioCPMContextStruct *pContext = NULL;
	EcrioSigMgrSipMessageStruct messageReq = { .eMethodType = EcrioSipMessageTypeNone };
	EcrioSigMgrSipMessageStruct *pMessageReq = NULL;
	EcrioSigMgrNameAddrWithParamsStruct toHeader = { 0 };
	EcrioSigMgrMessageBodyStruct msgBody = { .messageBodyType = EcrioSigMgrMessageBodyUnknown };
	EcrioSigMgrUnknownMessageBodyStruct unknownMsgBody = { 0 };
	EcrioSigMgrConversationsIdStruct pid = { 0 };
	u_char* pCPIMMsgBodyFromHdr = NULL;
	/* NULL --> sip:anonymous@anonymous.invalid will be populated. */
	u_char* pFromAddr = NULL;
	u_char* pToAddr = NULL;
	EcrioSigMgrNameAddrWithParamsStruct *pUID = NULL;

	if (NULL == hCPMHandle)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	pContext = (EcrioCPMContextStruct *)hCPMHandle;
	hLogHandle = pContext->hLogHandle;

	CPMLOGI(hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	uSigError = EcrioSigMgrPopulateToHeader(pContext->hEcrioSigMgrHandle, pPagerMessage->pDestUri, &toHeader);
	if (uSigError != ECRIO_SIGMGR_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrPopulateToHeader() error=%u",
			__FUNCTION__, __LINE__, uSigError);

		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void**)&messageReq.pMandatoryHdrs);
	if (messageReq.pMandatoryHdrs == NULL)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto Error_Level_01;
	}
	messageReq.pMandatoryHdrs->pTo = &toHeader;

	if (pPagerMessage->bIsChatbot == Enum_TRUE)
	{
		uError = ec_CPM_PopulateFeatureTags(pContext, 0, Enum_FALSE, Enum_FALSE, 
			pPagerMessage->bDeleteChatBotToken ? Enum_FALSE : Enum_TRUE, pPagerMessage->bDeleteChatBotToken ? Enum_FALSE : Enum_TRUE, 
			pPagerMessage->bDeleteChatBotToken, &messageReq.pFetaureTags);
	}
	else
	{
		uError = ec_CPM_PopulateFeatureTags(pContext, 0, Enum_FALSE, Enum_FALSE, Enum_TRUE, Enum_FALSE, Enum_FALSE, &messageReq.pFetaureTags);
	}

	if (uError != ECRIO_CPM_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_PopulateFeatureTags() error=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	/*Populate CPIM Message*/
	if (pPagerMessage->bDeleteChatBotToken == Enum_FALSE && pContext->pOOMObject->ec_oom_IsAnonymousCPIMHeader() 
		&& pPagerMessage->baseMsg.eContentType != EcrioCPMContentTypeEnum_IMDN)
	{
		uError = ec_CPM_PopulateCPIMHeaders(pContext, NULL, NULL, NULL, &pPagerMessage->baseMsg, &cpimStruct);
	}
	else if (pPagerMessage->bDeleteChatBotToken == Enum_FALSE && pPagerMessage->baseMsg.eContentType == EcrioCPMContentTypeEnum_IMDN)
	{
		/* Fill From and To with valid value for IMDN content-type. */
		u_char* pUri = NULL;

		/* Fill To-Header of CPIM Payload */
		pToAddr = NULL;
		if (toHeader.nameAddr.addrSpec.uriScheme == EcrioSigMgrURISchemeSIP)
		{
			EcrioSigMgrFormSipURI(pContext->hEcrioSigMgrHandle, toHeader.nameAddr.addrSpec.u.pSipUri, &pUri);
			pToAddr = pal_StringCreate(pUri, pal_StringLength(pUri));
			pal_MemoryFree((void**)&pUri);
		}
		else if (toHeader.nameAddr.addrSpec.uriScheme == EcrioSigMgrURISchemeTEL)
		{
			EcrioSigMgrFormTelURI(pContext->hEcrioSigMgrHandle, toHeader.nameAddr.addrSpec.u.pTelUri, &pUri);
			pToAddr = pal_StringCreate(pUri, pal_StringLength(pUri));
			pal_MemoryFree((void**)&pUri);
		}
		else
			pToAddr = pal_StringCreate(pPagerMessage->pDestUri, pal_StringLength(pPagerMessage->pDestUri));

		/* Fill From - Header of CPIM Payload */
		pFromAddr = NULL;
		pUri = NULL;
		uError = EcrioSigMgrGetDefaultPUID(pContext->hEcrioSigMgrHandle, EcrioSigMgrURISchemeTEL, &pUID);
		if (uError != ECRIO_SIGMGR_NO_ERROR || pUID == NULL)
		{
			CPMLOGE(hLogHandle, KLogTypeGeneral,
					"%s:%u\tEcrioSigMgrGetDefaultPUID(TEL_URI) populate To uri, error=%u",
					__FUNCTION__, __LINE__, uError);
			uError = EcrioSigMgrGetDefaultPUID(pContext->hEcrioSigMgrHandle, EcrioSigMgrURISchemeSIP, &pUID);
			if (uError != ECRIO_SIGMGR_NO_ERROR || pUID == NULL)
			{
				CPMLOGE(hLogHandle, KLogTypeGeneral,
						"%s:%u\tEcrioSigMgrGetDefaultPUID(SIP_URI) populate To uri, error=%u",
						__FUNCTION__, __LINE__, uError);
				uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
				goto Error_Level_01;
			}
			else
			{
				uError = EcrioSigMgrFormSipURI(pContext->hEcrioSigMgrHandle, pUID->nameAddr.addrSpec.u.pSipUri, &pUri);
				if (uError != ECRIO_SIGMGR_NO_ERROR)
				{
					CPMLOGE(hLogHandle, KLogTypeGeneral,
						"%s:%u\tEcrioSigMgrFormSipURI() returned with failure, error=%u",
						__FUNCTION__, __LINE__, uError);
					uError = ECRIO_CPM_INTERNAL_ERROR;
					goto Error_Level_01;
				}
			}
		}
		else
		{
			uError = EcrioSigMgrFormTelURI(pContext->hEcrioSigMgrHandle, pUID->nameAddr.addrSpec.u.pTelUri, &pUri);
			if (uError != ECRIO_SIGMGR_NO_ERROR)
			{
				CPMLOGE(hLogHandle, KLogTypeGeneral,
					"%s:%u\tEcrioSigMgrFormTelURI() returned with failure, error=%u",
					__FUNCTION__, __LINE__, uError);
				uError = ECRIO_CPM_INTERNAL_ERROR;
				goto Error_Level_01;
			}
		}

		pFromAddr = pal_StringCreate(pUri, pal_StringLength(pUri));
		pal_MemoryFree((void**)&pUri);

		/*Populate CPIM Message*/
		uError = ec_CPM_PopulateCPIMHeaders(pContext, pFromAddr, NULL, pToAddr,
			&pPagerMessage->baseMsg, &cpimStruct);
		if (uError != ECRIO_CPM_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_PopulateCPIMHeaders() error=%u",
				__FUNCTION__, __LINE__, uError);

			goto Error_Level_01;
		}
	}
	else
	{
		if (pPagerMessage->bDeleteChatBotToken == Enum_FALSE)
		{
			pCPIMMsgBodyFromHdr = ec_FormCPIMFromHeaderValue(pContext, pContext->pPublicIdentity);
			if (pCPIMMsgBodyFromHdr != NULL)
				uError = ec_CPM_PopulateCPIMHeaders(pContext, pCPIMMsgBodyFromHdr, NULL, pPagerMessage->pDestUri, &pPagerMessage->baseMsg, &cpimStruct);
			else
				uError = ec_CPM_PopulateCPIMHeaders(pContext, pContext->pPublicIdentity, NULL, pPagerMessage->pDestUri, &pPagerMessage->baseMsg, &cpimStruct);
		}
	}
	
	if (uError != ECRIO_CPM_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_PopulateCPIMHeaders() error=%u",
			__FUNCTION__, __LINE__, uError);

		goto Error_Level_01;
	}

	if (pContext->pPANI != NULL)
	{
		cpimStruct.pPANI = pContext->pPANI;
	}

	if (pPagerMessage->bDeleteChatBotToken == Enum_FALSE)
	{
		pMessageBody = EcrioCPIMForm(
			pContext->hCPIMHandle,
			&cpimStruct,
			&mssgLen,
			&uError);
		if (ECRIO_CPIM_NO_ERROR != uError)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioCPIMForm() error=%u",
				__FUNCTION__, __LINE__, uError);

			uError = ECRIO_CPM_INTERNAL_ERROR;
			goto Error_Level_01;
		}

		unknownMsgBody.bufferLength = mssgLen;
		unknownMsgBody.pBuffer = pMessageBody;
		unknownMsgBody.contentType.numParams = 0;
		unknownMsgBody.contentType.ppParams = NULL;
		unknownMsgBody.contentType.pHeaderValue = (u_char*)"message/cpim";
		// Here we need to call STACK API to generate the Message MIME content body which will return as buffer!
		messageReq.eModuleId = EcrioSigMgrCallbackRegisteringModule_CPM;

		if (pPagerMessage->pConvId != NULL)
		{
			messageReq.pConvId = &pid;
			messageReq.pConvId->pConversationId = pPagerMessage->pConvId->pConversationId;
			messageReq.pConvId->pContributionId = pPagerMessage->pConvId->pContributionId;
			messageReq.pConvId->pInReplyToContId = pPagerMessage->pConvId->pInReplyToContId;
		}

		messageReq.pMessageBody = &msgBody;
		msgBody.messageBodyType = EcrioSigMgrMessageBodyUnknown;
		msgBody.pMessageBody = &unknownMsgBody;
	}

	uError = EcrioSigMgrSendInstantMessage(pContext->hEcrioSigMgrHandle, &messageReq);
	if (ECRIO_SIGMGR_NO_ERROR != uSigError)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrSendPagerModeMessage() error=%u",
			__FUNCTION__, __LINE__, uSigError);

		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}
	/*Allocate memory for Call Id , used in upper layer */
	if ((messageReq.pMandatoryHdrs || messageReq.pMandatoryHdrs->pCallId)  && ppCallId)
	{
		*ppCallId = pal_StringCreate(messageReq.pMandatoryHdrs->pCallId, pal_StringLength(messageReq.pMandatoryHdrs->pCallId));
	}
Error_Level_01:

	EcrioSigMgrReleaseUriStruct(pContext->hEcrioSigMgrHandle, &toHeader.nameAddr.addrSpec);
	pMessageReq = &messageReq;
	if (pMessageReq->pMandatoryHdrs != NULL)
	{
		pMessageReq->pMandatoryHdrs->pTo = NULL;
	}
	pMessageReq->pMessageBody = NULL;
	EcrioSigMgrStructRelease(pContext->hEcrioSigMgrHandle, EcrioSigMgrStructType_SipMessage, (void**)&pMessageReq, Enum_FALSE);
	if (pCPIMMsgBodyFromHdr != NULL)
	{
		pal_MemoryFree((void**)&pCPIMMsgBodyFromHdr);
		pCPIMMsgBodyFromHdr = NULL;
	}
	if (pUID != NULL)
	{
		EcrioSigMgrStructRelease(pContext->hEcrioSigMgrHandle, EcrioSigMgrStructType_NameAddr, (void **)&pUID, Enum_TRUE);
	}
	if (pToAddr != NULL)
	{
		pal_MemoryFree((void**)&pToAddr);
	}
	if (pFromAddr != NULL)
	{
		pal_MemoryFree((void**)&pFromAddr);
	}
	CPMLOGI(hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uError;
}


u_int32 EcrioCPMGetPagerModeMessageSize
(
	CPMHANDLE hCPMHandle,
	EcrioCPMPagerMessageStruct* pPagerMessage,
	u_int32* pMsgSize
)
{
	u_int32 uError = ECRIO_CPM_NO_ERROR;
	u_int32 uSigError = ECRIO_SIGMGR_NO_ERROR;
	u_char* pMessageBody = NULL;
	u_int32 mssgLen = 0;
	LOGHANDLE hLogHandle = NULL;
	CPIMMessageStruct cpimStruct = { 0 };
	EcrioCPMContextStruct *pContext = NULL;
	u_char* pCPIMMsgBodyFromHdr = NULL;

	if (NULL == hCPMHandle)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	pContext = (EcrioCPMContextStruct *)hCPMHandle;
	hLogHandle = pContext->hLogHandle;

	CPMLOGI(hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/*Populate CPIM Message*/
	if (pContext->pOOMObject->ec_oom_IsAnonymousCPIMHeader())
	{
		uError = ec_CPM_PopulateCPIMHeaders(pContext, NULL, NULL, NULL, &pPagerMessage->baseMsg, &cpimStruct);
	}
	else
	{
		pCPIMMsgBodyFromHdr = ec_FormCPIMFromHeaderValue(pContext, pContext->pPublicIdentity);
		if (pCPIMMsgBodyFromHdr != NULL)
		{
			uError = ec_CPM_PopulateCPIMHeaders(pContext, pCPIMMsgBodyFromHdr, NULL, pPagerMessage->pDestUri, &pPagerMessage->baseMsg, &cpimStruct);
		}
		else
		{
			uError = ec_CPM_PopulateCPIMHeaders(pContext, pContext->pPublicIdentity, NULL, pPagerMessage->pDestUri, &pPagerMessage->baseMsg, &cpimStruct);
		}
	}

	if (uError != ECRIO_CPM_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_PopulateCPIMHeaders() error=%u",
			__FUNCTION__, __LINE__, uError);

		goto Error_Level_01;
	}

	if (pContext->pPANI != NULL)
	{
		cpimStruct.pPANI = pContext->pPANI;
	}

	pMessageBody = EcrioCPIMForm(
		pContext->hCPIMHandle,
		&cpimStruct,
		pMsgSize,
		&uError);
	if (ECRIO_CPIM_NO_ERROR != uError)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioCPIMForm() error=%u",
			__FUNCTION__, __LINE__, uError);

		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}

Error_Level_01:

	if (pCPIMMsgBodyFromHdr != NULL)
	{
		pal_MemoryFree((void**)&pCPIMMsgBodyFromHdr);
		pCPIMMsgBodyFromHdr = NULL;
	}
	CPMLOGI(hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uError;
}
