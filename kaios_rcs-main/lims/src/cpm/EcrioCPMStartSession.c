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

u_int32 EcrioCPMStartSession
(
	CPMHANDLE hCPMHandle,
	EcrioCPMStartSessionStruct *pStartSession,
	char **ppSessionId
)
{
	u_int32 uError = ECRIO_CPM_NO_ERROR;
	EcrioCPMContextStruct *pContext = NULL;
	EcrioCPMSessionStruct *pCPMSession = NULL;

	if (hCPMHandle == NULL)
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;

	pContext = (EcrioCPMContextStruct*)hCPMHandle;

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pal_MemoryAllocate(sizeof(EcrioCPMSessionStruct), (void**)&pCPMSession);
	if (NULL == pCPMSession)
	{
		uError = ECRIO_CPM_MEMORY_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
			__FUNCTION__, __LINE__, uError);
		return uError;
	}
	pCPMSession->eTerminationReason = EcrioSessionTerminationReason_None;
	pCPMSession->bIsStartMSRPSession = Enum_FALSE;

	if (pContext->bIsRelayEnabled == Enum_TRUE)
		uError = ec_CPM_StartMSRPAuthProcess(pContext, pStartSession, pCPMSession, ppSessionId);
	else
		uError = ec_CPM_StartSession(pContext, pStartSession, pCPMSession, ppSessionId);

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uError;
}

u_int32 ec_CPM_StartSession
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMStartSessionStruct *pStartSession,
	EcrioCPMSessionStruct *pCPMSession,
	char **ppSessionId
)
{
	u_int32 uError = ECRIO_CPM_NO_ERROR;
	u_int32 uSigError = ECRIO_SIGMGR_NO_ERROR;
	u_int32 uCPIMError = ECRIO_CPIM_NO_ERROR;
	u_int32 contentTypes = 0;
	u_int32 i = 0;
	EcrioSDPMSRPAttributeSetupEnum eLocalSetup = ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_NONE;
	BoolEnum bIsLargeMode = Enum_FALSE;
	u_char *pPath = NULL;
	u_char *pDest = NULL;
	u_char *pSigMgrCallID = NULL;
	LOGHANDLE hLogHandle = NULL;
	EcrioSigMgrSipMessageStruct inviteReq = { .eMethodType = EcrioSipMessageTypeNone };
	EcrioSigMgrMandatoryHeaderStruct mandatoryHeader = { 0 };
	EcrioSigMgrNameAddrWithParamsStruct toHeader = { 0 };
	EcrioSigMgrMessageBodyStruct msgBody = { .messageBodyType = EcrioSigMgrMessageBodySDP };
	EcrioSigMgrUnknownMessageBodyStruct unknownMsgBody = { 0 };
	EcrioCPMConversationsIdStruct *pConvId = NULL;
	EcrioSigMgrConversationsIdStruct conversationsId = { 0 };
	EcrioCPMConnectionInfoStruct *pConnStruct = NULL;
	hLogHandle = pContext->hLogHandle;
	u_char *pSdpBuf = NULL;
	u_char *pListBuf = NULL;
	u_char *pCpimBuf = NULL;
	u_char *pMultiBuf = NULL;
	u_char *pBoundary = NULL;
	u_int32 uSdpLen = 0;
	u_int32 uCpimLen = 0;
	u_int32 uListLen = 0;
	u_int32 uMultiLen = 0;
	EcrioCPMMultipartMessageStruct multiMsg = { 0 };
	EcrioSigMgrParamStruct *pParams = NULL;
	EcrioSigMgrParamStruct param = { 0 };
	EcrioCPMMessageStruct msg = { 0 };
	CPIMMessageStruct cpimMessage = { 0 };
	EcrioCPMChatbotPrivacyEnum eBotPrivacy = EcrioCPMChatbotPrivacy_None;

	CPMLOGI(hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pContext->bIsRelayEnabled == Enum_TRUE)
	{
		pDest = pCPMSession->relayData.pDest;
		pConvId = pCPMSession->pConvId;
		pConnStruct = &pCPMSession->relayData.localConnInfo;
		contentTypes = pCPMSession->contentTypes;
		bIsLargeMode = pCPMSession->bIsLargeMessage;
		eLocalSetup = ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_ACTPASS;
		pCPMSession->bPrivacy = pCPMSession->relayData.bPrivacy;
		eBotPrivacy = pCPMSession->eBotPrivacy;
	}
	else
	{
		pDest = pStartSession->pDest;
		pConvId = pStartSession->pConvId;
		pConnStruct = &pStartSession->connStruct;
		contentTypes = pStartSession->contentTypes;
		bIsLargeMode = pStartSession->bIsLargeMode;
		/* Set as actpass as we are not sure if MT want to be passive or active. */
		eLocalSetup = ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_ACTPASS;
		pCPMSession->bIsLargeMessage = pStartSession->bIsLargeMode;
		if (bIsLargeMode == Enum_TRUE)
		{
			eLocalSetup = ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_ACTIVE;
		}
		pCPMSession->bPrivacy = pStartSession->bPrivacy;

		/* Book keep for future use */
		pCPMSession->contentTypes = pStartSession->contentTypes;

		pal_MemoryAllocate(sizeof(EcrioCPMConversationsIdStruct), (void **)&pCPMSession->pConvId);
		if (pCPMSession->pConvId != NULL)
		{
			pCPMSession->pConvId->pContributionId = pal_StringCreate((const u_char*)pStartSession->pConvId->pContributionId, 
				pal_StringLength((const u_char*)pStartSession->pConvId->pContributionId));
			pCPMSession->pConvId->pConversationId = pal_StringCreate((const u_char*)pStartSession->pConvId->pConversationId, 
				pal_StringLength((const u_char*)pStartSession->pConvId->pConversationId));
			if (pStartSession->pConvId->pInReplyToContId != NULL)
			{
				pCPMSession->pConvId->pInReplyToContId = pal_StringCreate((const u_char*)pStartSession->pConvId->pInReplyToContId, 
					pal_StringLength((const u_char*)pStartSession->pConvId->pInReplyToContId));
			}
		}
		else
		{
			uError = ECRIO_CPM_MEMORY_ERROR;
			CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uError);
			goto Error_Level_01;
		}

		if (pStartSession->pFile != NULL)
		{
			pal_MemoryAllocate(sizeof(EcrioCPMFileDescriptorStruct), (void **)&pCPMSession->pFile);
			if (pCPMSession->pFile != NULL)
			{
			    pCPMSession->pFile->pFilePath = (char *)pal_StringCreate((const u_char*)pStartSession->pFile->pFilePath, pal_StringLength((const u_char*)pStartSession->pFile->pFilePath));
				pCPMSession->pFile->pFileName = (char *)pal_StringCreate((const u_char*)pStartSession->pFile->pFileName, pal_StringLength((const u_char*)pStartSession->pFile->pFileName));
				pCPMSession->pFile->pFileType = (char *)pal_StringCreate((const u_char*)pStartSession->pFile->pFileType, pal_StringLength((const u_char*)pStartSession->pFile->pFileType));
				pCPMSession->pFile->uFileSize = pStartSession->pFile->uFileSize;
				pCPMSession->pFile->pFileTransferId = (char *)pal_StringCreate((const u_char*)pStartSession->pFile->pFileTransferId, pal_StringLength((const u_char*)pStartSession->pFile->pFileTransferId));
				pCPMSession->pFile->pIMDNMsgId = (char *)pal_StringCreate((const u_char*)pStartSession->pFile->pIMDNMsgId, pal_StringLength((const u_char*)pStartSession->pFile->pIMDNMsgId));
				pCPMSession->pFile->imdnConfig = pStartSession->pFile->imdnConfig;
				pCPMSession->pFile->pAppData = pStartSession->pFile->pAppData;
			}
			else
			{
				uError = ECRIO_CPM_MEMORY_ERROR;
				CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
					__FUNCTION__, __LINE__, uError);
				goto Error_Level_01;
			}
		}

		pCPMSession->bIsGroupChat = pStartSession->bIsGroupChat;
		pCPMSession->bIsClosedGroupChat = pStartSession->bIsClosed;
		pCPMSession->bIsChatbot = pStartSession->bIsChatbot;
		pCPMSession->eBotPrivacy = pStartSession->eBotPrivacy;
		eBotPrivacy = pCPMSession->eBotPrivacy;
		if (pStartSession->pSubject != NULL)
		{
			pCPMSession->pSubject = (char *)pal_StringCreate((const u_char*)pStartSession->pSubject, pal_StringLength((const u_char*)pStartSession->pSubject));
		}
		if (pStartSession->pList != NULL)
		{
			pal_MemoryAllocate(sizeof(EcrioCPMParticipantListStruct), (void **)&pCPMSession->pList);
			if (pCPMSession->pList == NULL)
			{
				uError = ECRIO_CPM_MEMORY_ERROR;
				CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
					__FUNCTION__, __LINE__, uError);
				goto Error_Level_01;
			}
			pCPMSession->pList->uNumOfUsers = pStartSession->pList->uNumOfUsers;

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected(pCPMSession->pList->uNumOfUsers, sizeof(char*)) == Enum_TRUE)
			{
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto Error_Level_01;
			}

			pal_MemoryAllocate(sizeof(char*) * pCPMSession->pList->uNumOfUsers, (void **)&pCPMSession->pList->ppUri);
			if (pCPMSession->pList->ppUri == NULL)
			{
				uError = ECRIO_CPM_MEMORY_ERROR;
				CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
					__FUNCTION__, __LINE__, uError);
				goto Error_Level_01;
			}
			for (i = 0; i < pCPMSession->pList->uNumOfUsers; i++)
			{
				pCPMSession->pList->ppUri[i] = (char *)pal_StringCreate((const u_char*)pStartSession->pList->ppUri[i], pal_StringLength((const u_char*)pStartSession->pList->ppUri[i]));
			}
		}
	}
	pCPMSession->state = EcrioCPMSessionState_Unused;
	
	pCPMSession->pCallId = NULL; //index shall be populated

	uSigError = EcrioSigMgrPopulateToHeader(pContext->hEcrioSigMgrHandle, pDest, &toHeader);
	if (uSigError != ECRIO_SIGMGR_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrPopulateToHeader() error=%u",
			__FUNCTION__, __LINE__, uSigError);

		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}

	mandatoryHeader.pTo = &toHeader;
	inviteReq.pMandatoryHdrs = &mandatoryHeader;
	
	uError = ec_CPM_PopulateFeatureTags(pContext, contentTypes, bIsLargeMode, pCPMSession->bIsGroupChat, Enum_FALSE, Enum_FALSE, Enum_FALSE, &inviteReq.pFetaureTags);
	if (uError != ECRIO_CPM_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_PopulateFeatureTags() error=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	if (pConvId)
	{
		conversationsId.pContributionId = pConvId->pContributionId;
		conversationsId.pConversationId = pConvId->pConversationId;
		conversationsId.pInReplyToContId = pConvId->pInReplyToContId;
		inviteReq.pConvId = &conversationsId;
	}
	
	/* Check if Relay is enabled or not */
	if (pContext->bIsRelayEnabled == Enum_FALSE)
	{
		BoolEnum bIsFileTransfer = Enum_FALSE;
		/*Create MSRP session*/
		/* Use random port only if uLocalPort is set to 0 */
		if (pConnStruct->uLocalPort == 0)
			pConnStruct->uLocalPort = ec_CPM_GenerateRandomPort();

		if (pStartSession->pFile != NULL)
		{
			if (pStartSession->pFile->bIsCPIMWrapped != Enum_TRUE)
				bIsFileTransfer = Enum_TRUE;
		}

		uError = ec_CPM_CreateMSRPSession(pContext, pConnStruct, pCPMSession, &pPath, bIsFileTransfer);
		if (uError != ECRIO_CPM_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_CreateMSRPSession() error=%u",
				__FUNCTION__, __LINE__, uError);
			goto Error_Level_01;
		}
	}
	else /* If relay is enabled, path is already computed */
	{
		pPath = pCPMSession->relayData.pLocalPath;
		pCPMSession->relayData.pLocalPath = NULL;
	}
	
		/* Check and Populate optional header. */
	if (inviteReq.pOptionalHeaderList == NULL)
	{
		uSigError = EcrioSigMgrCreateOptionalHeaderList(pContext->hEcrioSigMgrHandle, &inviteReq.pOptionalHeaderList);
		if (uSigError != ECRIO_SIGMGR_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSigMgrCreateOptionalHeaderList() error=%u",
				__FUNCTION__, __LINE__, uSigError);

			uError = ECRIO_CPM_MEMORY_ERROR;
			goto Error_Level_01;
		}
	}

	/* Populate optional supported header*/
	{
		u_char *ppSupported[] = { (u_char *)ECRIO_SIG_MGR_SESSION_PATH_EXTENSION, (u_char *)ECRIO_SIG_MGR_SESSION_TIMER_EXTENSION , (u_char *)"sec-agree"};

		uSigError = EcrioSigMgrAddOptionalHeader(pContext->hEcrioSigMgrHandle, inviteReq.pOptionalHeaderList,
			EcrioSipHeaderTypeSupported, (sizeof(ppSupported) / sizeof(u_char *)), ppSupported, 0, NULL, NULL);
		if (uSigError != ECRIO_SIGMGR_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSigMgrCreateOptionalHeaderList() error=%u",
				__FUNCTION__, __LINE__, uSigError);

			uError = ECRIO_CPM_INTERNAL_ERROR;
			goto Error_Level_01;
		}
	}

	/** Add Subject header */
	if (pCPMSession->pSubject != NULL)
	{
		uSigError = EcrioSigMgrAddOptionalHeader(pContext->hEcrioSigMgrHandle, inviteReq.pOptionalHeaderList,
			EcrioSipHeaderTypeSubject, 1, (u_char**)&pCPMSession->pSubject, 0, NULL, NULL);
		if (uSigError != ECRIO_SIGMGR_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSigMgrCreateOptionalHeaderList() error=%u",
				__FUNCTION__, __LINE__, uSigError);

			uError = ECRIO_CPM_INTERNAL_ERROR;
			goto Error_Level_01;
		}
	}

	/** Add Require header */
	if (pCPMSession->pList != NULL &&
		(pCPMSession->pList->ppUri != NULL && pCPMSession->pList->uNumOfUsers > 0))
	{
		u_char *ppRequired[] = { (u_char *)"recipient-list-invite" };

		uSigError = EcrioSigMgrAddOptionalHeader(pContext->hEcrioSigMgrHandle, inviteReq.pOptionalHeaderList,
			EcrioSipHeaderTypeRequire, (sizeof(ppRequired) / sizeof(u_char *)), ppRequired, 0, NULL, NULL);
		if (uSigError != ECRIO_SIGMGR_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSigMgrCreateOptionalHeaderList() error=%u",
				__FUNCTION__, __LINE__, uSigError);

			uError = ECRIO_CPM_INTERNAL_ERROR;
			goto Error_Level_01;
		}
	}

	/** Add Privacy header */
	if (eBotPrivacy != EcrioCPMChatbotPrivacy_None)
	{
		u_char *ppPrivacyTk[] = { (u_char *)"tk" };
		u_char *ppPrivacyTklink[] = { (u_char *)"tklink" };

		if (eBotPrivacy == EcrioCPMChatbotPrivacy_Request)
		{
			uSigError = EcrioSigMgrAddOptionalHeader(pContext->hEcrioSigMgrHandle, inviteReq.pOptionalHeaderList,
				EcrioSipHeaderTypePrivacy, (sizeof(ppPrivacyTk) / sizeof(u_char *)), ppPrivacyTk, 0, NULL, NULL);
		}
		else if (eBotPrivacy == EcrioCPMChatbotPrivacy_Link)
		{
			uSigError = EcrioSigMgrAddOptionalHeader(pContext->hEcrioSigMgrHandle, inviteReq.pOptionalHeaderList,
				EcrioSipHeaderTypePrivacy, (sizeof(ppPrivacyTklink) / sizeof(u_char *)), ppPrivacyTklink, 0, NULL, NULL);
		}

		if (uSigError != ECRIO_SIGMGR_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSigMgrCreateOptionalHeaderList() error=%u",
				__FUNCTION__, __LINE__, uSigError);

			uError = ECRIO_CPM_INTERNAL_ERROR;
			goto Error_Level_01;
		}
	}

	pal_MemorySet((void*)&pCPMSession->localSDP, 0, sizeof(EcrioSDPInformationStruct));
	/** Create SDP session */
	uError = ec_CPM_CreateSDPSession(pContext, pCPMSession, &pCPMSession->localSDP);
	if (uError != ECRIO_CPM_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_CreateSDPSession() error=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	/** Populate SDP message body */
	unknownMsgBody.pBuffer = NULL;
	if (pContext->bIsRelayEnabled == Enum_TRUE)
	{
		uError = ec_CPM_FillInviteSdp(pContext,
			pCPMSession,
			ECRIO_SDP_EVENT_INITIAL_OFFER,
			(u_char *)pContext->pRelayServerIP,
			pContext->uRelayServerPort,
			pConnStruct->isIPv6,
			contentTypes,
			pPath,
			eLocalSetup,
			&pCPMSession->localSDP,
			&unknownMsgBody);
		if (uError != ECRIO_CPM_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_FillInviteSdp() error=%u",
				__FUNCTION__, __LINE__, uSigError);

			goto Error_Level_02;
		}
	}
	else
	{
		uError = ec_CPM_FillInviteSdp(pContext,
			pCPMSession,
			ECRIO_SDP_EVENT_INITIAL_OFFER,
			pConnStruct->pLocalIp,
			pConnStruct->uLocalPort,
			pConnStruct->isIPv6,
			contentTypes,
			pPath,
			eLocalSetup,
			&pCPMSession->localSDP,
			&unknownMsgBody);
		if (uError != ECRIO_CPM_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_FillInviteSdp() error=%u",
				__FUNCTION__, __LINE__, uSigError);

			goto Error_Level_02;
		}
	}

	if ((pCPMSession->pList != NULL && (pCPMSession->pList->ppUri != NULL && pCPMSession->pList->uNumOfUsers > 0)) ||
		(pCPMSession->pFile != NULL && pCPMSession->bIsLargeMessage != Enum_TRUE))
	{
		/** Store SDP buffer pointer */
		pSdpBuf = unknownMsgBody.pBuffer;
		unknownMsgBody.pBuffer = NULL;
		uSdpLen = unknownMsgBody.bufferLength;
		unknownMsgBody.bufferLength = 0;

		if (pCPMSession->pFile != NULL)
		{
			/** Build CPIM message */
			msg.message.pBuffer = NULL;
			msg.pIMDNMsgId = pCPMSession->pFile->pIMDNMsgId;
			msg.imdnConfig = pCPMSession->pFile->imdnConfig;
			msg.eContentType = EcrioCPMContentTypeEnum_None;

			uError = ec_CPM_PopulateCPIMHeaders(pContext, NULL, NULL, NULL, &msg, &cpimMessage);
			if (uError != ECRIO_CPM_NO_ERROR)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_PopulateCPIMHeaders() error=%u",
					__FUNCTION__, __LINE__, uError);

				goto Error_Level_01;
			}

			/** Set MSRP Content-Type */
			cpimMessage.buff[0].eContentType = CPIMContentType_Specified;
			cpimMessage.buff[0].pContentType = (u_char*)pCPMSession->pFile->pFileType;
			cpimMessage.uNumOfBuffers = 1;

			if (pContext->pPANI != NULL)
			{
				cpimMessage.pPANI = pContext->pPANI;
			}

			pCpimBuf = EcrioCPIMForm(pContext->hCPIMHandle, &cpimMessage, &uCpimLen, &uCPIMError);
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioCPIMForm failed, uError=%u",
					__FUNCTION__, __LINE__, uCPIMError);
				uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
				goto Error_Level_01;
			}
		}
		else
		{
			/** Build resource-list XML */
			uError = ec_CPM_FormResourceListXML(pContext, pCPMSession->pList, EcrioCPMParticipantType_None, (char**)&pListBuf, &uListLen);
			if (uError != ECRIO_CPM_NO_ERROR)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_FormResourceListXML() error=%u",
					__FUNCTION__, __LINE__, uSigError);

				goto Error_Level_02;
			}
		}

		/** Populate multipart message body */
		multiMsg.uNumOfBody = 2;
		pal_MemoryAllocate((u_int32)sizeof(EcrioCPMMultipartBodyStruct*) * 2, (void **)&multiMsg.ppBody);
		if (multiMsg.ppBody == NULL)
		{
			uError = ECRIO_CPM_MEMORY_ERROR;
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uError);
			goto Error_Level_02;
		}
		pal_MemoryAllocate((u_int32)sizeof(EcrioCPMMultipartBodyStruct), (void **)&multiMsg.ppBody[0]);
		if (multiMsg.ppBody[0] == NULL)
		{
			uError = ECRIO_CPM_MEMORY_ERROR;
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uError);
			goto Error_Level_02;
		}
		pal_MemoryAllocate((u_int32)sizeof(EcrioCPMMultipartBodyStruct), (void **)&multiMsg.ppBody[1]);
		if (multiMsg.ppBody[1] == NULL)
		{
			uError = ECRIO_CPM_MEMORY_ERROR;
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uError);
			goto Error_Level_02;
		}
		multiMsg.ppBody[0]->pContentType = (u_char *)"application/sdp";
		multiMsg.ppBody[0]->uContentLength = uSdpLen;
		multiMsg.ppBody[0]->pMessageBody = pSdpBuf;
		if (pCPMSession->pFile != NULL)
		{
			multiMsg.ppBody[1]->pContentType = (u_char *)"message/cpim";
			multiMsg.ppBody[1]->uContentLength = uCpimLen;
			multiMsg.ppBody[1]->pMessageBody = pCpimBuf;
		}
		else
		{
			multiMsg.ppBody[1]->pContentType = (u_char *)"application/resource-lists+xml";
			multiMsg.ppBody[1]->uContentLength = uListLen;
			multiMsg.ppBody[1]->pMessageBody = pListBuf;
		}

		uError = ec_CPM_FormMultipartMessage(pContext, &multiMsg, (char**)&pMultiBuf, &uMultiLen, (char**)&pBoundary);
		if (uError != ECRIO_CPM_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_FormMultipartMessage() error=%u",
				__FUNCTION__, __LINE__, uSigError);

			goto Error_Level_02;
		}

		/** Populate unknown message body */
		unknownMsgBody.pBuffer = pMultiBuf;
		unknownMsgBody.bufferLength = uMultiLen;
		param.pParamName = (u_char*)"boundary";
		param.pParamValue = pBoundary;
		pParams = &param;

		/** Content-Type : multipart/mixed */
		unknownMsgBody.contentType.numParams = 1;
		unknownMsgBody.contentType.ppParams = &pParams;
		unknownMsgBody.contentType.pHeaderValue = (u_char *)"multipart/mixed";
	}
	else
	{
		/** Content-Type : application-sdp */
		unknownMsgBody.contentType.numParams = 0;
		unknownMsgBody.contentType.ppParams = NULL;
		unknownMsgBody.contentType.pHeaderValue = (u_char *)"application/sdp";
	}

	msgBody.messageBodyType = EcrioSigMgrMessageBodyUnknown;
	msgBody.pMessageBody = &unknownMsgBody;
	inviteReq.pMessageBody = &msgBody;
	inviteReq.bPrivacy = pCPMSession->bPrivacy;

	/*Routing support*/
	inviteReq.eModuleId = EcrioSigMgrCallbackRegisteringModule_CPM;

	uSigError = EcrioSigMgrSendInvite(pContext->hEcrioSigMgrHandle, &inviteReq, &pSigMgrCallID);
	if ((ECRIO_SIGMGR_NO_ERROR != uSigError) || (NULL == pSigMgrCallID))
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrSendInvite() error=%u",
			__FUNCTION__, __LINE__, uSigError);

		if (uSigError == ECRIO_SIG_MGR_SIG_SOCKET_ERROR)
			uError = ECRIO_CPM_INTERNAL_ERROR;
		else
			uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_02;
	}

	pCPMSession->pCallId = pal_StringCreate(pSigMgrCallID, pal_StringLength(pSigMgrCallID));
	pCPMSession->state = EcrioCPMSessionState_Starting;

	/* Set the local connection time TCP connection based on a=setup: parameter in SDP */
	if (eLocalSetup == ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_ACTIVE /*||
		eLocalSetup == ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_ACTPASS*/)
	{
		if (pContext->bMsrpOverTLS == Enum_TRUE)
		{
			pCPMSession->eLocalConnectionType = MSRPConnectionType_TLS_Client;
		}
		else
		{
			pCPMSession->eLocalConnectionType = MSRPConnectionType_TCP_Client;
		}
	}
	else if (eLocalSetup == ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_PASSIVE)
	{
		pCPMSession->eLocalConnectionType = MSRPConnectionType_TCP_Server;
	}

	if (*ppSessionId == NULL)
	{
		*ppSessionId = (char *)pal_StringCreate(pCPMSession->pCallId, pal_StringLength(pCPMSession->pCallId));
		/* In this case callId and application callID will remain same as relay is disabled */
		pCPMSession->relayData.pAppCallId = pal_StringCreate(pCPMSession->pCallId, pal_StringLength(pCPMSession->pCallId));
		CPMLOGD(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tSessionID returned %s",
				__FUNCTION__, __LINE__, pCPMSession->pCallId);
	}

	/* MSRP Session setup based on https://tools.ietf.org/html/rfc4975#section-5.4,
	https://tools.ietf.org/html/rfc4975#section-4 and http://www.qtc.jp/3GPP/Specs/24247-910.pdf @ page 33 item #29 */
	if (pCPMSession->eLocalConnectionType == MSRPConnectionType_TCP_Server && pContext->bIsRelayEnabled == Enum_FALSE)
	{
		uError = ec_CPM_StartMSRPSession(pContext, pCPMSession);
		if (uError != ECRIO_CPM_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_StartMSRPSession() error=%u",
				__FUNCTION__, __LINE__, uError);
			goto Error_Level_01;
		}
	}

	/* Store the session data into hash map for that call ID */
	EcrioSigMgrSetAppData(pContext->hEcrioSigMgrHandle, pCPMSession->pCallId, (void*)pCPMSession);

	if (pContext->bIsRelayEnabled == Enum_TRUE)
		ec_CPM_MapSetData(pContext, (u_char *)*ppSessionId, (void*)pCPMSession->pCallId);

	goto Error_Level_01;

Error_Level_02:

	ec_CPM_ReleaseCPMSession(pContext, pCPMSession);

Error_Level_01:

	if (pListBuf)
	{
		pal_MemoryFree((void**)&pListBuf);
	}
	if (pMultiBuf)
	{
		pal_MemoryFree((void**)&pMultiBuf);
	}
	if (pBoundary)
	{
		pal_MemoryFree((void**)&pBoundary);
	}
	if (multiMsg.ppBody)
	{
		pal_MemoryFree((void**)&multiMsg.ppBody[0]);
		pal_MemoryFree((void**)&multiMsg.ppBody[1]);
		pal_MemoryFree((void**)&multiMsg.ppBody);
	}

	pal_MemoryFree((void**)&pPath);
	EcrioSigMgrReleaseUriStruct(pContext->hEcrioSigMgrHandle, &toHeader.nameAddr.addrSpec);

	if (inviteReq.numPPreferredIdentity > 1)
	{
		for (int i = 0; i < inviteReq.numPPreferredIdentity; i++)
		{
			EcrioSigMgrNameAddrStruct* pPrefferedIdentity = inviteReq.ppPPreferredIdentity[i];
			EcrioSigMgrReleaseUriStruct(pContext->hEcrioSigMgrHandle, &pPrefferedIdentity->addrSpec);
			if (pPrefferedIdentity->pDisplayName)
			{
				pal_MemoryFree((void**)&pPrefferedIdentity->pDisplayName);
			}

			pal_MemoryFree((void**)&inviteReq.ppPPreferredIdentity[i]);
		}

		pal_MemoryFree((void**)&inviteReq.ppPPreferredIdentity);
		inviteReq.numPPreferredIdentity = 0;
	}

	CPMLOGI(hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uError;
}

u_int32 ec_CPM_StartMSRPAuthProcess
(
	EcrioCPMContextStruct *pCPMContext,
	EcrioCPMStartSessionStruct *pStartSession,
	EcrioCPMSessionStruct *pCPMSession,
	char **ppSessionId
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	u_int32 i = 0;
	MSRPHostStruct msrpRemoteHost = { 0 };
	u_char remotePath[256] = { 0 };
	u_char *pPath = NULL;
	MSRPStreamParamStruct msrpParam = { 0 };
	u_char* pUcString = NULL;
	u_int32 ucStringLength = 0;

	CPMLOGI(pCPMContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/*Create MSRP session*/
	if (pCPMContext->bMsrpOverTLS == Enum_TRUE)
	{
		pCPMSession->eLocalConnectionType = MSRPConnectionType_TLS_Client;
	}
	else
	{
		pCPMSession->eLocalConnectionType = MSRPConnectionType_TCP_Client;
	}

	uError = ec_CPM_CreateMSRPSession(pCPMContext, &pStartSession->connStruct, pCPMSession, &pPath, Enum_FALSE/*pStartSession->pFile ? Enum_TRUE : Enum_FALSE*/);
	if (uError != ECRIO_CPM_NO_ERROR)
	{
		CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_CreateMSRPSession() error=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	/* Book keep the infomartion required to start session. 
	This info will be used when we receive auth success callback from MSRP lib */
	pCPMSession->bIsLargeMessage = pStartSession->bIsLargeMode;
	pCPMSession->contentTypes = pStartSession->contentTypes;

	pal_MemoryAllocate(sizeof(EcrioCPMConversationsIdStruct), (void **)&pCPMSession->pConvId);
	if (pCPMSession->pConvId != NULL)
	{
		pCPMSession->pConvId->pContributionId = pal_StringCreate((const u_char*)pStartSession->pConvId->pContributionId, pal_StringLength((const u_char*)pStartSession->pConvId->pContributionId));
		pCPMSession->pConvId->pConversationId = pal_StringCreate((const u_char*)pStartSession->pConvId->pConversationId, pal_StringLength((const u_char*)pStartSession->pConvId->pConversationId));
		if (pStartSession->pConvId->pInReplyToContId != NULL)
			pCPMSession->pConvId->pInReplyToContId = pal_StringCreate((const u_char*)pStartSession->pConvId->pInReplyToContId, pal_StringLength((const u_char*)pStartSession->pConvId->pInReplyToContId));
	}
	else
	{
		uError = ECRIO_CPM_MEMORY_ERROR;
		CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	if (pStartSession->pFile != NULL)
	{
		pal_MemoryAllocate(sizeof(EcrioCPMFileDescriptorStruct), (void **)&pCPMSession->pFile);
		if (pCPMSession->pFile != NULL)
		{
            pCPMSession->pFile->pFilePath = (char *)pal_StringCreate((const u_char*)pStartSession->pFile->pFilePath, pal_StringLength((const u_char*)pStartSession->pFile->pFilePath));
			pCPMSession->pFile->pFileName = (char *)pal_StringCreate((const u_char*)pStartSession->pFile->pFileName, pal_StringLength((const u_char*)pStartSession->pFile->pFileName));
			pCPMSession->pFile->pFileType = (char *)pal_StringCreate((const u_char*)pStartSession->pFile->pFileType, pal_StringLength((const u_char*)pStartSession->pFile->pFileType));
			pCPMSession->pFile->uFileSize = pStartSession->pFile->uFileSize;
			pCPMSession->pFile->pFileTransferId = (char *)pal_StringCreate((const u_char*)pStartSession->pFile->pFileTransferId, pal_StringLength((const u_char*)pStartSession->pFile->pFileTransferId));
			pCPMSession->pFile->pIMDNMsgId = (char *)pal_StringCreate((const u_char*)pStartSession->pFile->pIMDNMsgId, pal_StringLength((const u_char*)pStartSession->pFile->pIMDNMsgId));
			pCPMSession->pFile->imdnConfig = pStartSession->pFile->imdnConfig;
			pCPMSession->pFile->pAppData = pStartSession->pFile->pAppData;
		}
		else
		{
			uError = ECRIO_CPM_MEMORY_ERROR;
			CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uError);
			goto Error_Level_01;
		}
	}

	pCPMSession->bIsGroupChat = pStartSession->bIsGroupChat;
	pCPMSession->bIsClosedGroupChat = pStartSession->bIsClosed;
	pCPMSession->bIsChatbot = pStartSession->bIsChatbot;
	pCPMSession->eBotPrivacy = pStartSession->eBotPrivacy;
	if (pStartSession->pSubject != NULL)
	{
		pCPMSession->pSubject = (char *)pal_StringCreate((const u_char*)pStartSession->pSubject, pal_StringLength((const u_char*)pStartSession->pSubject));
	}
	if (pStartSession->pList != NULL)
	{
		pal_MemoryAllocate(sizeof(EcrioCPMParticipantListStruct), (void **)&pCPMSession->pList);
		if (NULL == pCPMSession->pList)
		{
			uError = ECRIO_CPM_MEMORY_ERROR;
			CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uError);
			goto Error_Level_01;
		}
		pCPMSession->pList->uNumOfUsers = pStartSession->pList->uNumOfUsers;

		/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
		if (pal_UtilityDataOverflowDetected(pCPMSession->pList->uNumOfUsers, sizeof(char*)) == Enum_TRUE)
		{
			uError = ECRIO_CPM_MEMORY_ERROR;
			goto Error_Level_01;
		}

		pal_MemoryAllocate(sizeof(char*) * pCPMSession->pList->uNumOfUsers, (void **)&pCPMSession->pList->ppUri);
		if (NULL == pCPMSession->pList->ppUri)
		{
			uError = ECRIO_CPM_MEMORY_ERROR;
			CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uError);
			goto Error_Level_01;
		}
		for (i = 0; i < pCPMSession->pList->uNumOfUsers; i++)
		{
			pCPMSession->pList->ppUri[i] = (char *)pal_StringCreate((const u_char*)pStartSession->pList->ppUri[i], pal_StringLength((const u_char*)pStartSession->pList->ppUri[i]));
		}
	}

	pCPMSession->relayData.pDest = pal_StringCreate((const u_char*)pStartSession->pDest, pal_StringLength((const u_char*)pStartSession->pDest));
	pCPMSession->relayData.pLocalPath = pPath;
	pCPMSession->relayData.localConnInfo.pLocalIp = pal_StringCreate((const u_char*)pStartSession->connStruct.pLocalIp, pal_StringLength((const u_char*)pStartSession->connStruct.pLocalIp));
	pCPMSession->relayData.localConnInfo.uLocalPort = pStartSession->connStruct.uLocalPort;
	pCPMSession->relayData.localConnInfo.isIPv6 = pStartSession->connStruct.isIPv6;
	pCPMSession->relayData.bPrivacy = pStartSession->bPrivacy;
	
	if (pCPMContext->bMsrpOverTLS == Enum_TRUE)
	{
		msrpParam.connType = MSRPConnectionType_TLS_Client;
	}
	else
	{
		msrpParam.connType = MSRPConnectionType_TCP_Client;
	}
	
	msrpParam.uBuffSize = ECRIO_CPM_MRSP_RECV_BUFFER_SIZE;
	pal_MemoryAllocate(msrpParam.uBuffSize, (void **)&pCPMSession->pMSRPRecvBuffer);
	if (NULL == pCPMSession->pMSRPRecvBuffer)
	{
		uError = ECRIO_CPM_MEMORY_ERROR;

		CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	msrpParam.pRecvBuff = pCPMSession->pMSRPRecvBuffer;

	uError = EcrioMSRPStreamSetParam(pCPMSession->hMSRPSessionHandle, &msrpParam);
	if (uError != ECRIO_MSRP_NO_ERROR)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}	
	
	/* Set MSRP Relay IP and Port details here */
	msrpRemoteHost.bIPv6 = Enum_FALSE;
	if (NULL == pal_StringNCopy((u_char *)msrpRemoteHost.ip, 64, (const u_char *)pCPMContext->pRelayServerIP,
		pal_StringLength((const u_char *)pCPMContext->pRelayServerIP)))
	{
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto Error_Level_01;
	}
	msrpRemoteHost.port = (u_int16)pCPMContext->uRelayServerPort;

	if (0 >= pal_SStringPrintf((char*)remotePath, 256, "msrps://%s:%d;tcp", msrpRemoteHost.ip, msrpRemoteHost.port))
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}
	uError = EcrioMSRPSetRemoteMSRPAddress(pCPMSession->hMSRPSessionHandle, (char *)remotePath);
	if (uError != ECRIO_MSRP_NO_ERROR)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}

	uError = EcrioMSRPAuthStart(pCPMSession->hMSRPSessionHandle);
	if (uError != ECRIO_MSRP_NO_ERROR)
	{
		CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioMSRPAuthStart() uError=%u",
			__FUNCTION__, __LINE__, uError);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}

	pCPMSession->bIsStartMSRPSession = Enum_TRUE;

	uError = ec_CPM_GetSessionId(&pUcString);
	if (uError != ECRIO_CPM_NO_ERROR)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}
	ucStringLength = pal_StringLength(pUcString);

	*ppSessionId = (char *)pal_StringCreate((const u_char*)pUcString, ucStringLength);
	pCPMSession->relayData.pAppCallId = pal_StringCreate((const u_char*)pUcString, ucStringLength);
	pal_MemoryFree((void **)&pUcString);

	pCPMSession->pCallId = NULL;

Error_Level_01:
	CPMLOGI(pCPMContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uError;
}


/**
* This function is called to get the application data.
* @param[in] hCPMHandle			The CPM handle.
* @param[in] pSessionId			The ID of the session
* @param[in/out] ppAppData		Pointer to application data
*/
u_int32 EcrioCPMGetFileAppData
(
		CPMHANDLE hCPMHandle,
		char *pSessionId,
		void **ppAppData
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	EcrioCPMContextStruct *pContext = NULL;
	EcrioCPMSessionStruct *pSession = NULL;
	LOGHANDLE hLogHandle = NULL;
	char *pCallId = NULL;

	if (hCPMHandle == NULL)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	/** Get the CPM structure from the handle */
	pContext = (EcrioCPMContextStruct *)hCPMHandle;

	hLogHandle = pContext->hLogHandle;

	CPMLOGI(hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pContext->bIsRelayEnabled == Enum_TRUE)
		ec_CPM_MapGetData(pContext, (u_char *)pSessionId, (void**)&pCallId);
	else
		pCallId = pSessionId;

	/** Get the session structure corresponding to the session id */
	EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, (u_char *)pCallId, (void**)&pSession);

	if (pSession == NULL)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tpSession not found for pSessionId=%s",
				__FUNCTION__, __LINE__, pSessionId);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto ErrTag;

	}

	if(pSession->pFile != NULL)
	{
		*ppAppData = pSession->pFile->pAppData;
		if(pSession->pFile->pAppData == NULL)
		{
			CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tpSession->pFile->pAppData is NULL",
			__FUNCTION__, __LINE__);
		}
		else
		{
			CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tpSession->pFile->pAppData is not NULL",
				__FUNCTION__, __LINE__);
		}
	}
	else
	{
		CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tpSession->pFile is NULL",
				__FUNCTION__, __LINE__);
	}

	ErrTag:

	pSession = NULL;

	CPMLOGI(hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return uError;
}
