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
#include "EcrioBase64.h"

u_int32 ec_CPM_Get_Bits(u_int32 serviceId, u_int32 bitswanted) 
{

	u_int32 k;
	u_int32 mask = 0, masked_bit = 0, bit_val = 0, len = 0;
	for (k = 0; k<bitswanted; k++) {
		mask = 1 << k;
		masked_bit = serviceId & mask;
		bit_val = masked_bit >> k;
		if (bit_val)
			len++;
	}
	return len;
}



u_int32 ec_CPM_FillInviteSdp
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMSessionStruct *pCPMSession,
	EcrioSDPEventTypeEnum eEvent,
	u_char *pLocalIp,
	u_int32 uLocalPort,
	BoolEnum isIPv6,
	EcrioCPMContentTypeEnum eContentType,
	u_char *pPath,
	EcrioSDPMSRPAttributeSetupEnum eSetup,
	EcrioSDPInformationStruct *pInfo,
	EcrioSigMgrUnknownMessageBodyStruct *pUnknownMsgBody
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	u_int32 uSDPError = ECRIO_SDP_NO_ERROR;
	EcrioCPMSessionStruct *pSession = NULL;
	EcrioSDPStringStruct tSDP = { 0 };

	pSession = pCPMSession;

	pInfo->eEvent = eEvent;

	pInfo->localIp.eAddressType = (isIPv6 == Enum_TRUE) ? ECRIO_SDP_ADDRESS_TYPE_IP6 : ECRIO_SDP_ADDRESS_TYPE_IP4;
	pInfo->localIp.pAddress = pLocalIp;

	/** Only MSRP session is allowed */
	pInfo->uNumOfLocalMedia = 1;

	pInfo->localStream[0].eMediaType = ECRIO_SDP_MEDIA_TYPE_MSRP;
	pInfo->localStream[0].uMediaPort = (u_int16)uLocalPort;

	if (pContext->bMsrpOverTLS == Enum_TRUE)
	{
		pInfo->localStream[0].eProtocol = ECRIO_SDP_MEDIA_PROTOCOL_TLS_MSRP;
	}
	else
	{
		pInfo->localStream[0].eProtocol = ECRIO_SDP_MEDIA_PROTOCOL_TCP_MSRP;
	}
	
	pInfo->localStream[0].uNumOfPayloads = 0;

	/*Populate SDP info*/
	pInfo->localStream[0].eDirection = ECRIO_SDP_MEDIA_DIRECTION_SENDRECV;
	pInfo->localStream[0].u.msrp.eAcceptTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_NONE;
	pInfo->localStream[0].u.msrp.eAcceptWrappedTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_NONE;

	if (((pSession->contentTypes & EcrioCPMContentTypeEnum_FileTransferOverMSRP) == EcrioCPMContentTypeEnum_FileTransferOverMSRP) &&
		pCPMSession->bIsLargeMessage == Enum_FALSE)
	{
		/** Direction of file transfer is fixed (sender:sendonly, receiver:recvonly) for now. */
		if (eEvent == ECRIO_SDP_EVENT_INITIAL_OFFER)
		{
			pInfo->localStream[0].eDirection = ECRIO_SDP_MEDIA_DIRECTION_SENDONLY;
		}
		else
		{
			pInfo->localStream[0].eDirection = ECRIO_SDP_MEDIA_DIRECTION_RECVONLY;
		}

		pInfo->localStream[0].u.msrp.eAcceptTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_GENERAL;
		pInfo->localStream[0].u.msrp.eAcceptWrappedTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_WILDCARD;

		pInfo->localStream[0].u.msrp.fileSelector.pName = (u_char *)pCPMSession->pFile->pFileName;
		pInfo->localStream[0].u.msrp.fileSelector.pType = (u_char *)pCPMSession->pFile->pFileType;
		pInfo->localStream[0].u.msrp.fileSelector.uSize = pCPMSession->pFile->uFileSize;
		pInfo->localStream[0].u.msrp.pFileTransferId = (u_char *)pCPMSession->pFile->pFileTransferId;
	}
	else if (((pSession->contentTypes & EcrioCPMContentTypeEnum_FileTransferOverMSRP) == EcrioCPMContentTypeEnum_FileTransferOverMSRP) &&
		pCPMSession->bIsLargeMessage == Enum_TRUE)
	{
		if (eEvent == ECRIO_SDP_EVENT_INITIAL_OFFER)
		{
			pInfo->localStream[0].eDirection = ECRIO_SDP_MEDIA_DIRECTION_SENDONLY;
		}
		else
		{
			pInfo->localStream[0].eDirection = ECRIO_SDP_MEDIA_DIRECTION_RECVONLY;
		}
		pInfo->localStream[0].u.msrp.eAcceptTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_CPIM;
		if (pCPMSession->pFile->pFileType != NULL)
		{
			pInfo->localStream[0].u.msrp.eAcceptWrappedTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_WILDCARD;
			pInfo->localStream[0].u.msrp.fileSelector.pName = (u_char *)pCPMSession->pFile->pFileName;
			pInfo->localStream[0].u.msrp.fileSelector.pType = (u_char *)pCPMSession->pFile->pFileType;
		}
		else
		{
			pInfo->localStream[0].u.msrp.eAcceptWrappedTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_WILDCARD;
			pInfo->localStream[0].u.msrp.fileSelector.pName = NULL;
			pInfo->localStream[0].u.msrp.fileSelector.pType = NULL;
		}

		pInfo->localStream[0].u.msrp.fileSelector.uSize = pCPMSession->pFile->uFileSize;
		pInfo->localStream[0].u.msrp.pFileTransferId = (u_char *)pCPMSession->pFile->pFileTransferId;
	}
	else
	{
		if (pCPMSession->bIsLargeMessage == Enum_TRUE)
		{
			if (eEvent == ECRIO_SDP_EVENT_INITIAL_OFFER)
			{
				pInfo->localStream[0].eDirection = ECRIO_SDP_MEDIA_DIRECTION_SENDONLY;
			}
			else
			{
				pInfo->localStream[0].eDirection = ECRIO_SDP_MEDIA_DIRECTION_RECVONLY;
			}
		pInfo->localStream[0].u.msrp.eAcceptTypes |= ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_CPIM;
			pInfo->localStream[0].u.msrp.eAcceptWrappedTypes |= ECRIO_SDP_MSRP_MEDIA_TYPE_TEXT_PLAIN;
		}
		else
		{
			pInfo->localStream[0].u.msrp.eAcceptTypes |= ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_CPIM;

			if ((eContentType & EcrioCPMContentTypeEnum_Composing) == EcrioCPMContentTypeEnum_Composing)
			{
				if (pCPMSession->bIsGroupChat == Enum_TRUE)
				{
					pInfo->localStream[0].u.msrp.eAcceptWrappedTypes |= ECRIO_SDP_MSRP_MEDIA_TYPE_IM_ISCOMPOSING_XML;
				}
				else
				{
					pInfo->localStream[0].u.msrp.eAcceptTypes |= ECRIO_SDP_MSRP_MEDIA_TYPE_IM_ISCOMPOSING_XML;
				}
			}

			if ((eContentType & EcrioCPMContentTypeEnum_Text) == EcrioCPMContentTypeEnum_Text)
				pInfo->localStream[0].u.msrp.eAcceptWrappedTypes |= ECRIO_SDP_MSRP_MEDIA_TYPE_TEXT_PLAIN;

			if ((eContentType & EcrioCPMContentTypeEnum_IMDN) == EcrioCPMContentTypeEnum_IMDN)
				pInfo->localStream[0].u.msrp.eAcceptWrappedTypes |= ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_IMDN_XML;

			if ((eContentType & EcrioCPMContentTypeEnum_FileTransferOverHTTP) == EcrioCPMContentTypeEnum_FileTransferOverHTTP)
				pInfo->localStream[0].u.msrp.eAcceptWrappedTypes |= ECRIO_SDP_MSRP_MEDIA_TYPE_HTTP_FILE_TRANSFER;

			if ((eContentType & EcrioCPMContentTypeEnum_RichCard) == EcrioCPMContentTypeEnum_RichCard)
				pInfo->localStream[0].u.msrp.eAcceptWrappedTypes |= ECRIO_SDP_MSRP_MEDIA_TYPE_RICHCARD;

			if ((eContentType & EcrioCPMContentTypeEnum_SuggestedChipList) == EcrioCPMContentTypeEnum_SuggestedChipList)
				pInfo->localStream[0].u.msrp.eAcceptWrappedTypes |= ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTED_CHIPLIST;

			if ((eContentType & EcrioCPMContentTypeEnum_RichCard) == EcrioCPMContentTypeEnum_RichCard
				|| (eContentType & EcrioCPMContentTypeEnum_SuggestedChipList) == EcrioCPMContentTypeEnum_SuggestedChipList)
			{
				pInfo->localStream[0].u.msrp.eAcceptWrappedTypes |= ECRIO_SDP_MSRP_MEDIA_TYPE_MULTIPART_MIXED;
			}

			if ((eContentType & EcrioCPMContentTypeEnum_GroupData) == EcrioCPMContentTypeEnum_GroupData)
				pInfo->localStream[0].u.msrp.eAcceptWrappedTypes |= ECRIO_SDP_MSRP_MEDIA_TYPE_GROUPDATA;

			if ((eContentType & EcrioCPMContentTypeEnum_PushLocation) == EcrioCPMContentTypeEnum_PushLocation)
			{
				pInfo->localStream[0].u.msrp.eAcceptWrappedTypes |= ECRIO_SDP_MSRP_MEDIA_TYPE_PUSH_LOCATION;;
			}

			if ((eContentType & EcrioCPMContentTypeEnum_SuggestionResponse) == EcrioCPMContentTypeEnum_SuggestionResponse)
			{
				pInfo->localStream[0].u.msrp.eAcceptWrappedTypes |= ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTION_RESPONSE;
			}

			if ((eContentType & EcrioCPMContentTypeEnum_SharedClientData) == EcrioCPMContentTypeEnum_SharedClientData)
			{
				pInfo->localStream[0].u.msrp.eAcceptWrappedTypes |= ECRIO_SDP_MSRP_MEDIA_TYPE_SHARED_CLIENT_DATA;
			}
		}
	}


	pInfo->localStream[0].u.msrp.pPath = pPath;
	pInfo->localStream[0].u.msrp.eSetup = eSetup;
	pInfo->localStream[0].u.msrp.bCEMA = Enum_TRUE;
	if (eEvent == ECRIO_SDP_EVENT_INITIAL_ANSWER)
	{
		pInfo->localStream[0].u.msrp.bIsClosed = pSession->remoteSDP.remoteStream[0].u.msrp.bIsClosed;
		pSession->bIsClosedGroupChat = pSession->remoteSDP.remoteStream[0].u.msrp.bIsClosed;
	}
	else
	{
		pInfo->localStream[0].u.msrp.bIsClosed = pCPMSession->bIsClosedGroupChat;
	}
	


	tSDP.pData = pSession->pSDPStringBuffer;
	tSDP.uContainerSize = ECRIO_CPM_SDP_STRING_SIZE;
	tSDP.uSize = 0;

	uSDPError = EcrioSDPSessionGenerateSDPString(pSession->hSDPSessionHandle, pInfo, &tSDP);
	if (uSDPError != ECRIO_SDP_NO_ERROR)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSDPSessionGenerateSDPString() uSDPError=%u",
			__FUNCTION__, __LINE__, uSDPError);
		goto Error_Level_01;
	}

	pUnknownMsgBody->bufferLength = tSDP.uSize;
	pUnknownMsgBody->pBuffer = tSDP.pData;

Error_Level_01:

	return uError;
}

u_int32 ec_CPM_GetSessionId
(
	u_char **ppStr
)
{
	u_int32 uEncodedLen = 0;
	u_int32 uError = KPALErrorNone;
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
	u_char *pEncodedStr = NULL;
	u_char *pRandBin = NULL;

	pRandBin =  pal_UtilityRandomBin16();
	uError = _EcrioIMSLibraryBase64Encode(&pEncodedStr, pRandBin, ECRIO_CPM_STRING_SIZE_16, &uEncodedLen);
	if (uError != KPALErrorNone)
	{
		uCPMError = ECRIO_CPM_INTERNAL_ERROR;
		goto Done;
	}

	*ppStr = pEncodedStr;

Done:
	if (pRandBin != NULL)
	{
		pal_MemoryFree((void**)&pRandBin);
	}
	return uCPMError;
}

#if 0
u_int32 ec_CPM_GenerateMSRPPath
(
	EcrioCPMContextStruct *pContextStruct,
	MSRPHostStruct *pHost,
	u_char **ppPath
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	u_int32 uSize = 0;
	u_char* pPath = NULL;
	u_char ucString[9] = { 0 };
	BoolEnum bIsBracketRequired = Enum_TRUE;
	u_char* pUcString = NULL;

	if (pal_StringFindSubString((const u_char *)pHost->ip, (const u_char *)"[") != NULL) {
		bIsBracketRequired = Enum_FALSE;
	}

	uSize = 128;

	pal_MemoryAllocate(uSize, (void **)&pPath);
	if (NULL == pPath)
	{
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}
	pal_MemorySet(pPath, 0, uSize);

	if (pContextStruct->bMsrpOverTLS == Enum_TRUE)
	{
		if (NULL == pal_StringNCopy(pPath, uSize, (u_char *)"msrps://", pal_StringLength((u_char *)"msrps://")))
		{
			uError = ECRIO_CPM_MEMORY_ERROR;
			goto Error;
		}
	}
	else
	{
		if (NULL == pal_StringNCopy(pPath, uSize, (u_char *)"msrp://", pal_StringLength((u_char *)"msrp://")))
		{
			uError = ECRIO_CPM_MEMORY_ERROR;
			goto Error;
		}
	}

	if (pHost->bIPv6 == Enum_TRUE && bIsBracketRequired == Enum_TRUE)
	{
		if (NULL == pal_StringNConcatenate(pPath, uSize - pal_StringLength((const u_char *)pPath), (u_char *)"[", 1))
		{
			uError = ECRIO_CPM_MEMORY_ERROR;
			goto Error;
		}
	}
	if (NULL == pal_StringNConcatenate(pPath, uSize - pal_StringLength((const u_char *)pPath), (const u_char *)pHost->ip, pal_StringLength((const u_char *)pHost->ip)))
	{
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}
	if (pHost->bIPv6 == Enum_TRUE && bIsBracketRequired == Enum_TRUE)
	{
		if (NULL == pal_StringNConcatenate(pPath, uSize - pal_StringLength((const u_char *)pPath), (u_char *)"]", 1))
		{
			uError = ECRIO_CPM_MEMORY_ERROR;
			goto Error;
		}
	}
	if (NULL == pal_StringNConcatenate(pPath, uSize - pal_StringLength((const u_char *)pPath), (u_char *)":", 1))
	{
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}
	if (0 >= pal_NumToString(pHost->port, ucString, 9))
	{
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}
	if (NULL == pal_StringNConcatenate(pPath, uSize - pal_StringLength((const u_char *)pPath), ucString, pal_StringLength(ucString)))
	{
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}
	if (NULL == pal_StringNConcatenate(pPath, uSize - pal_StringLength((const u_char *)pPath), (u_char *)"/", 1))
	{
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}

	uError = ec_CPM_GetSessionId(&pUcString);
	if (uError != ECRIO_CPM_NO_ERROR)
	{
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}

	if (NULL == pal_StringNConcatenate(pPath, uSize - pal_StringLength((const u_char *)pPath), pUcString, pal_StringLength(pUcString)))
	{
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}

	if (NULL == pal_StringNConcatenate(pPath, uSize - pal_StringLength((const u_char *)pPath), (u_char *)";tcp", pal_StringLength((u_char *)";tcp")))
	{
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto Error;
	}

	*ppPath = pPath;

	goto END;

Error:
	pal_MemoryFree((void**)&pPath);
	*ppPath = NULL;

END:
	if (pUcString != NULL)
	{
		pal_MemoryFree((void**)&pUcString);
	}
	
	return uError;
}
#endif

void ec_CPM_getRandomString
(
	u_char *pStr,
	u_int32 uLength
)
{
	u_int32 i = 0;
	u_char seed[64] = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	for (i = 0; i < uLength; i++)
	{
		*(pStr + i) = seed[pal_UtilityRandomNumber() % 62];
	}
}


u_int32 ec_CPM_PopulateFeatureTags
(
	EcrioCPMContextStruct *pContext,
	u_int32 contentTypes,
	BoolEnum bIsLargeMode,
	BoolEnum bIsGroupChat,
	BoolEnum bIsPagerMode,
	BoolEnum bIsChatbotSA,
	BoolEnum bDeleteToken,
	EcrioSigMgrFeatureTagStruct **ppFeatureTags
)
{
	/*contentTypes 0 means pager message*/
	u_int32	uError = ECRIO_CPM_NO_ERROR, uLen = 0;
	BoolEnum isChatBot = Enum_FALSE, isFToverHTTP = Enum_FALSE, isFToverMSRP = Enum_FALSE, isPushLocation = Enum_FALSE;
	BoolEnum isPrivMngCmd = Enum_FALSE, isSpamReportMsg = Enum_FALSE;
	EcrioSigMgrFeatureTagStruct* pFetaureTags = NULL;
	u_char *pCommon = NULL, *pContact = NULL;
	u_char *pICSIValue = NULL, *pIARIValue = NULL;
	u_char *pPService = NULL;
	u_int32 uICSILen = 0, uIARILen = 0, uChatBotLen = 0;

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (NULL == ppFeatureTags)
	{
		uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;

		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tppFeatureTags is NULL, uError=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	if (contentTypes == EcrioCPMContentTypeEnum_FileTransferOverMSRP)
	{
		isFToverMSRP = Enum_TRUE;
	}
	else
	{
		if (((contentTypes & EcrioCPMContentTypeEnum_SuggestedChipList) == EcrioCPMContentTypeEnum_SuggestedChipList)
			|| ((contentTypes & EcrioCPMContentTypeEnum_SuggestionResponse) == EcrioCPMContentTypeEnum_SuggestionResponse)
			|| ((contentTypes & EcrioCPMContentTypeEnum_RichCard) == EcrioCPMContentTypeEnum_RichCard))
		{
			isChatBot = Enum_TRUE;
		}
		else if ((contentTypes & EcrioCPMContentTypeEnum_PrivacyManagementCommand) == EcrioCPMContentTypeEnum_PrivacyManagementCommand)
		{
			isChatBot = Enum_TRUE;
			isPrivMngCmd = Enum_TRUE;
		}
		else if ((contentTypes & EcrioCPMContentTypeEnum_SpamReportMessage) == EcrioCPMContentTypeEnum_SpamReportMessage)
		{
			isChatBot = Enum_TRUE;
			isSpamReportMsg = Enum_TRUE;
		}

		if ((contentTypes & EcrioCPMContentTypeEnum_FileTransferOverHTTP) == \
			EcrioCPMContentTypeEnum_FileTransferOverHTTP)
		{
			isFToverHTTP = Enum_TRUE;
		}

		if ((contentTypes & EcrioCPMContentTypeEnum_PushLocation) == EcrioCPMContentTypeEnum_PushLocation)
		{
			isPushLocation = Enum_TRUE;
		}
	}

	/** 3 headers for providing feature-tag are support (including Contact header) */
	pal_MemoryAllocate(sizeof(EcrioSigMgrFeatureTagStruct), (void **)&pFetaureTags);
	if (pFetaureTags == NULL)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto MEM_ERR;
	}

	pFetaureTags->uNumberOfTags = 3;

	pal_MemoryAllocate((pFetaureTags->uNumberOfTags) * sizeof(EcrioSigMgrParamStruct *),
		(void **)&pFetaureTags->ppFeatureTagHeaderValue);
	if (pFetaureTags->ppFeatureTagHeaderValue == NULL)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto MEM_ERR;
	}
	pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&pFetaureTags->ppFeatureTagHeaderValue[0]);
	if (pFetaureTags->ppFeatureTagHeaderValue[0] == NULL)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto MEM_ERR;
	}
	pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&pFetaureTags->ppFeatureTagHeaderValue[1]);
	if (pFetaureTags->ppFeatureTagHeaderValue[1] == NULL)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto MEM_ERR;
	}
	pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&pFetaureTags->ppFeatureTagHeaderValue[2]);
	if (pFetaureTags->ppFeatureTagHeaderValue[2] == NULL)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto MEM_ERR;
	}

	/** Feature-tag format for Accept-Contact headers */
	pal_MemorySet(pContext->pString, 0, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE);
	pICSIValue = pContext->pString;

	if (NULL == pal_StringNCopy(pICSIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE, ECRIO_CPM_3GPP_ICSI_PREFIX_REF, 
		pal_StringLength(ECRIO_CPM_3GPP_ICSI_PREFIX_REF)))
	{
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto MEM_ERR;
	}
	CPM_STRING_CONCATENATE(pICSIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pICSIValue), ECRIO_CPM_3GPP_DOUBLE_QUOTE,
		pal_StringLength(ECRIO_CPM_3GPP_DOUBLE_QUOTE), uError, MEM_ERR);

	/*contentTypes 0 means pager mode message*/
	if (contentTypes == 0)
	{
		if (bDeleteToken == Enum_TRUE)
		{
			CPM_STRING_CONCATENATE(pICSIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pICSIValue), ECRIO_CPM_3GPP_FEATURE_SYSTEMMSG,
				pal_StringLength(ECRIO_CPM_3GPP_FEATURE_SYSTEMMSG), uError, MEM_ERR);
			CPM_STRING_CONCATENATE(pICSIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pICSIValue), ECRIO_CPM_3GPP_DOUBLE_QUOTE,
				pal_StringLength(ECRIO_CPM_3GPP_DOUBLE_QUOTE), uError, MEM_ERR);
		}
		else
		{
			CPM_STRING_CONCATENATE(pICSIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pICSIValue), ECRIO_CPM_3GPP_FEATURE_PAGERMSG,
				pal_StringLength(ECRIO_CPM_3GPP_FEATURE_PAGERMSG), uError, MEM_ERR);
			CPM_STRING_CONCATENATE(pICSIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pICSIValue), ECRIO_CPM_3GPP_DOUBLE_QUOTE,
				pal_StringLength(ECRIO_CPM_3GPP_DOUBLE_QUOTE), uError, MEM_ERR);
		}
	}
	else
	{
		if (bIsLargeMode)
		{
			CPM_STRING_CONCATENATE(pICSIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pICSIValue), ECRIO_CPM_3GPP_FEATURE_LARGEMSG,
				pal_StringLength(ECRIO_CPM_3GPP_FEATURE_LARGEMSG), uError, MEM_ERR);
			CPM_STRING_CONCATENATE(pICSIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pICSIValue), ECRIO_CPM_3GPP_DOUBLE_QUOTE,
				pal_StringLength(ECRIO_CPM_3GPP_DOUBLE_QUOTE), uError, MEM_ERR);
		}
		else
		{
			if (isFToverMSRP == Enum_TRUE)
			{
				CPM_STRING_CONCATENATE(pICSIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pICSIValue), ECRIO_CPM_3GPP_FEATURE_FT_OVER_MSRP,
					pal_StringLength(ECRIO_CPM_3GPP_FEATURE_FT_OVER_MSRP), uError, MEM_ERR);
			}
			else
			{
				CPM_STRING_CONCATENATE(pICSIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pICSIValue), ECRIO_CPM_3GPP_FEATURE_SESSION,
					pal_StringLength(ECRIO_CPM_3GPP_FEATURE_SESSION), uError, MEM_ERR);
			}
			CPM_STRING_CONCATENATE(pICSIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pICSIValue), ECRIO_CPM_3GPP_DOUBLE_QUOTE,
				pal_StringLength(ECRIO_CPM_3GPP_DOUBLE_QUOTE), uError, MEM_ERR);
		}
	}

	if (isFToverHTTP == Enum_TRUE)
	{
		if (pIARIValue == NULL)
		{
			pIARIValue = pContext->pString + pal_StringLength(pICSIValue) + 2;
			if (NULL == pal_StringNCopy(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE, ECRIO_CPM_3GPP_IARI_PREFIX_REF,
				pal_StringLength(ECRIO_CPM_3GPP_IARI_PREFIX_REF)))
			{
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto MEM_ERR;
			}

			CPM_STRING_CONCATENATE(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pIARIValue), ECRIO_CPM_3GPP_DOUBLE_QUOTE,
				pal_StringLength(ECRIO_CPM_3GPP_DOUBLE_QUOTE), uError, MEM_ERR);
		}
		else
		{
			CPM_STRING_CONCATENATE(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pIARIValue), ECRIO_CPM_3GPP_COMMA,
				pal_StringLength(ECRIO_CPM_3GPP_COMMA), uError, MEM_ERR);
		}
		
		CPM_STRING_CONCATENATE(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pIARIValue), ECRIO_CPM_3GPP_FEATURE_FILE_TRANSFER,
			pal_StringLength(ECRIO_CPM_3GPP_FEATURE_FILE_TRANSFER), uError, MEM_ERR);
	}

	if (isChatBot == Enum_TRUE)
	{
		if (pIARIValue == NULL)
		{
			pIARIValue = pContext->pString + pal_StringLength(pICSIValue) + 2;
			if (NULL == pal_StringNCopy(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE, ECRIO_CPM_3GPP_IARI_PREFIX_REF,
				pal_StringLength(ECRIO_CPM_3GPP_IARI_PREFIX_REF)))
			{
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto MEM_ERR;
			}
			CPM_STRING_CONCATENATE(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pIARIValue), ECRIO_CPM_3GPP_DOUBLE_QUOTE,
				pal_StringLength(ECRIO_CPM_3GPP_DOUBLE_QUOTE), uError, MEM_ERR);
		}
		else
		{
			CPM_STRING_CONCATENATE(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pIARIValue), ECRIO_CPM_3GPP_COMMA,
				pal_StringLength(ECRIO_CPM_3GPP_COMMA), uError, MEM_ERR);
		}

		if (isPrivMngCmd)
		{
			CPM_STRING_CONCATENATE(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pIARIValue), ECRIO_CPM_3GPP_FEATURE_ALIAS,
				pal_StringLength(ECRIO_CPM_3GPP_FEATURE_ALIAS), uError, MEM_ERR);
		}
		else
		{
			CPM_STRING_CONCATENATE(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pIARIValue), ECRIO_CPM_3GPP_FEATURE_CHATBOT,
				pal_StringLength(ECRIO_CPM_3GPP_FEATURE_CHATBOT), uError, MEM_ERR);
		}
	}

	if (bIsChatbotSA == Enum_TRUE)
	{
		if (pIARIValue == NULL)
		{
			pIARIValue = pContext->pString + pal_StringLength(pICSIValue) + 2;
			if (NULL == pal_StringNCopy(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE, ECRIO_CPM_3GPP_IARI_PREFIX_REF,
				pal_StringLength(ECRIO_CPM_3GPP_IARI_PREFIX_REF)))
			{
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto MEM_ERR;
			}
			CPM_STRING_CONCATENATE(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pIARIValue), ECRIO_CPM_3GPP_DOUBLE_QUOTE,
				pal_StringLength(ECRIO_CPM_3GPP_DOUBLE_QUOTE), uError, MEM_ERR);
		}
		else
		{
			CPM_STRING_CONCATENATE(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pIARIValue), ECRIO_CPM_3GPP_COMMA,
				pal_StringLength(ECRIO_CPM_3GPP_COMMA), uError, MEM_ERR);
		}

		CPM_STRING_CONCATENATE(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pIARIValue), ECRIO_CPM_3GPP_FEATURE_CHATBOT_STANDALONE,
			pal_StringLength(ECRIO_CPM_3GPP_FEATURE_CHATBOT_STANDALONE), uError, MEM_ERR);

		/** Set isChatBot flag to true to add the Chatbot Role feature-tag */
		isChatBot = Enum_TRUE;
	}

	if (isPushLocation)
	{
		if (pIARIValue == NULL)
		{
			pIARIValue = pContext->pString + pal_StringLength(pICSIValue) + 2;
			if (NULL == pal_StringNCopy(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE, ECRIO_CPM_3GPP_IARI_PREFIX_REF,
				pal_StringLength(ECRIO_CPM_3GPP_IARI_PREFIX_REF)))
			{
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto MEM_ERR;
			}
			CPM_STRING_CONCATENATE(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pIARIValue), ECRIO_CPM_3GPP_DOUBLE_QUOTE,
				pal_StringLength(ECRIO_CPM_3GPP_DOUBLE_QUOTE), uError, MEM_ERR);
		}
		else
		{
			CPM_STRING_CONCATENATE(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pIARIValue), ECRIO_CPM_3GPP_COMMA,
				pal_StringLength(ECRIO_CPM_3GPP_COMMA), uError, MEM_ERR);
		}

		CPM_STRING_CONCATENATE(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pIARIValue), ECRIO_CPM_3GPP_FEATURE_PUSH_LOCATION,
			pal_StringLength(ECRIO_CPM_3GPP_FEATURE_PUSH_LOCATION), uError, MEM_ERR);
	}

	if (bDeleteToken == Enum_TRUE)
	{
		if (pIARIValue == NULL)
		{
			pIARIValue = pContext->pString + pal_StringLength(pICSIValue) + 2;
			if (NULL == pal_StringNCopy(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE, ECRIO_CPM_3GPP_IARI_PREFIX_REF,
				pal_StringLength(ECRIO_CPM_3GPP_IARI_PREFIX_REF)))
			{
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto MEM_ERR;
			}
			CPM_STRING_CONCATENATE(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pIARIValue), ECRIO_CPM_3GPP_DOUBLE_QUOTE,
				pal_StringLength(ECRIO_CPM_3GPP_DOUBLE_QUOTE), uError, MEM_ERR);
		}
		else
		{
			CPM_STRING_CONCATENATE(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pIARIValue), ECRIO_CPM_3GPP_COMMA,
				pal_StringLength(ECRIO_CPM_3GPP_COMMA), uError, MEM_ERR);
		}
		CPM_STRING_CONCATENATE(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pIARIValue), ECRIO_CPM_3GPP_FEATURE_DEL_TK,
			pal_StringLength(ECRIO_CPM_3GPP_FEATURE_DEL_TK), uError, MEM_ERR);
	}

	
	if (pIARIValue)
	{
		CPM_STRING_CONCATENATE(pIARIValue, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pIARIValue), ECRIO_CPM_3GPP_DOUBLE_QUOTE,
			pal_StringLength(ECRIO_CPM_3GPP_DOUBLE_QUOTE), uError, MEM_ERR);
	}

	uICSILen = pal_StringLength(pICSIValue);
	uIARILen = pal_StringLength(pIARIValue);
	/** RCC.07 3.6.2.3 Chatbot role - isbot feature tag is added by chatbot role, not by client */
	if (isPrivMngCmd || isSpamReportMsg)
	{
		uChatBotLen = pal_StringLength(ECRIO_CPM_3GPP_FEATURE_BOTVERSION);
	}
	else if(bDeleteToken == Enum_FALSE)
	{
		uChatBotLen = pal_StringLength(ECRIO_CPM_3GPP_FEATURE_BOTVERSION) + pal_StringLength(ECRIO_CPM_3GPP_FEATURE_ISBOT);
	}

	uLen = uICSILen + uIARILen + uChatBotLen; //extra 2 bytes for IARI

	/** Feature-tag format for Contact headers */
	pCommon = pContext->pString + uLen + 4;

#ifndef ENABLE_QCMAPI
	if (0 >= pal_SNumPrintf((char *)pCommon, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE, (char *)ECRIO_CPM_3GPP_INSTANCE_VALUE, pContext->pDeviceId))
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tCopy string error.",
			__FUNCTION__, __LINE__);
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto MEM_ERR;
	}

	uLen += pal_StringLength(pCommon);
	uLen += pal_StringLength(ECRIO_CPM_3GPP_SIP_INSTANCE) + 2 + 4;
#endif

	/* Include mobility="mobile" feature-tag*/
	uLen += pal_StringLength((u_char*)";mobility=\"mobile\";");

	pal_MemoryAllocate(uLen, (void **)&pContact);
	if (pContact == NULL)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto MEM_ERR;
	}
	pal_MemorySet(pContact, 0, uLen);

#ifndef ENABLE_QCMAPI
	if (NULL == pal_StringNCopy(pContact, uLen, ECRIO_CPM_3GPP_SIP_INSTANCE, pal_StringLength(ECRIO_CPM_3GPP_SIP_INSTANCE)))
	{
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto MEM_ERR;
	}
	CPM_STRING_CONCATENATE(pContact, uLen - pal_StringLength(pContact), ECRIO_CPM_3GPP_EQUAL, pal_StringLength(ECRIO_CPM_3GPP_EQUAL), uError, MEM_ERR);
	CPM_STRING_CONCATENATE(pContact, uLen - pal_StringLength(pContact), pCommon, pal_StringLength(pCommon), uError, MEM_ERR);
	CPM_STRING_CONCATENATE(pContact, uLen - pal_StringLength(pContact), ECRIO_CPM_3GPP_SEMICOLON, pal_StringLength(ECRIO_CPM_3GPP_SEMICOLON), uError, MEM_ERR);
#endif

	CPM_STRING_CONCATENATE(pContact, uLen - pal_StringLength(pContact), pICSIValue, pal_StringLength(pICSIValue), uError, MEM_ERR);
	if (pIARIValue)
	{
		CPM_STRING_CONCATENATE(pContact, uLen - pal_StringLength(pContact), ECRIO_CPM_3GPP_SEMICOLON, pal_StringLength(ECRIO_CPM_3GPP_SEMICOLON), uError, MEM_ERR);
		CPM_STRING_CONCATENATE(pContact, uLen - pal_StringLength(pContact), pIARIValue, pal_StringLength(pIARIValue), uError, MEM_ERR);
	}

	CPM_STRING_CONCATENATE(pContact, uLen - pal_StringLength(pContact), (u_char*)";mobility=\"mobile\"", pal_StringLength((u_char*)";mobility=\"mobile\""), uError, MEM_ERR);

	if (isChatBot)
	{
		/** RCC.07 3.6.2.3 Chatbot role - isbot feature tag is added by chatbot role, not by client */
		/* if (!(isPrivMngCmd || isSpamReportMsg))
		{
			pal_StringNConcatenate(pContact, ECRIO_CPM_3GPP_SEMICOLON);
			pal_StringNConcatenate(pContact, ECRIO_CPM_3GPP_FEATURE_ISBOT);
		} */
		CPM_STRING_CONCATENATE(pContact, uLen - pal_StringLength(pContact), ECRIO_CPM_3GPP_SEMICOLON, pal_StringLength(ECRIO_CPM_3GPP_SEMICOLON), uError, MEM_ERR);
		CPM_STRING_CONCATENATE(pContact, uLen - pal_StringLength(pContact), ECRIO_CPM_3GPP_FEATURE_BOTVERSION, pal_StringLength(ECRIO_CPM_3GPP_FEATURE_BOTVERSION), uError, MEM_ERR);
	}

	pFetaureTags->ppFeatureTagHeaderValue[0]->pParamName = pal_StringCreate(
		ECRIO_CPM_ACCEPT_CONTACT_HEADER_STRING, pal_StringLength(ECRIO_CPM_ACCEPT_CONTACT_HEADER_STRING));

	uLen = uIARILen + uICSILen + uChatBotLen + 4; // 4 for 4 semicolon
#ifndef ENABLE_QCMAPI
	if (isSpamReportMsg != Enum_TRUE)
	{
		uLen += pal_StringLength((u_char*)";require;explicit") + 2;
	}
#endif
	if (isPrivMngCmd || isSpamReportMsg)
	{
		uLen += pal_StringLength(ECRIO_CPM_3GPP_FEATURE_SYSTEMMSG) + 2;
	}

	pal_MemoryAllocate(uLen, (void **)&pFetaureTags->ppFeatureTagHeaderValue[0]->pParamValue);
	if (pFetaureTags->ppFeatureTagHeaderValue[0]->pParamValue == NULL)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto MEM_ERR;
	}

	if (NULL == pal_StringNCopy(pFetaureTags->ppFeatureTagHeaderValue[0]->pParamValue, uLen, pICSIValue, pal_StringLength(pICSIValue)))
	{
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto MEM_ERR;
	}

	if (pIARIValue)
	{
		CPM_STRING_CONCATENATE(pFetaureTags->ppFeatureTagHeaderValue[0]->pParamValue,
			uLen - pal_StringLength(pFetaureTags->ppFeatureTagHeaderValue[0]->pParamValue),
			ECRIO_CPM_3GPP_SEMICOLON, pal_StringLength(ECRIO_CPM_3GPP_SEMICOLON), uError, MEM_ERR);
		CPM_STRING_CONCATENATE(pFetaureTags->ppFeatureTagHeaderValue[0]->pParamValue,
			uLen - pal_StringLength(pFetaureTags->ppFeatureTagHeaderValue[0]->pParamValue),
			pIARIValue, pal_StringLength(pIARIValue), uError, MEM_ERR);
	}
		

	if (isChatBot)
	{
		/** RCC.07 3.6.2.3 Chatbot role - isbot feature tag is added by chatbot role, not by client */
		if (isPrivMngCmd || isSpamReportMsg)
		{
			CPM_STRING_CONCATENATE(pFetaureTags->ppFeatureTagHeaderValue[0]->pParamValue,
				uLen - pal_StringLength(pFetaureTags->ppFeatureTagHeaderValue[0]->pParamValue),
				ECRIO_CPM_3GPP_SEMICOLON, pal_StringLength(ECRIO_CPM_3GPP_SEMICOLON), uError, MEM_ERR);
			CPM_STRING_CONCATENATE(pFetaureTags->ppFeatureTagHeaderValue[0]->pParamValue,
				uLen - pal_StringLength(pFetaureTags->ppFeatureTagHeaderValue[0]->pParamValue),
				ECRIO_CPM_3GPP_FEATURE_SYSTEMMSG, pal_StringLength(ECRIO_CPM_3GPP_FEATURE_SYSTEMMSG), uError, MEM_ERR);
		}
		/* else
		{
			pal_StringNConcatenate(pFetaureTags->ppFeatureTagHeaderValue[0]->pParamValue, ECRIO_CPM_3GPP_SEMICOLON);
			pal_StringNConcatenate(pFetaureTags->ppFeatureTagHeaderValue[0]->pParamValue, ECRIO_CPM_3GPP_FEATURE_ISBOT);
		} */
		CPM_STRING_CONCATENATE(pFetaureTags->ppFeatureTagHeaderValue[0]->pParamValue,
			uLen - pal_StringLength(pFetaureTags->ppFeatureTagHeaderValue[0]->pParamValue),
			ECRIO_CPM_3GPP_SEMICOLON, pal_StringLength(ECRIO_CPM_3GPP_SEMICOLON), uError, MEM_ERR);
		CPM_STRING_CONCATENATE(pFetaureTags->ppFeatureTagHeaderValue[0]->pParamValue,
			uLen - pal_StringLength(pFetaureTags->ppFeatureTagHeaderValue[0]->pParamValue),
			ECRIO_CPM_3GPP_FEATURE_BOTVERSION, pal_StringLength(ECRIO_CPM_3GPP_FEATURE_BOTVERSION), uError, MEM_ERR);
	}

#ifndef ENABLE_QCMAPI
	if (isFToverMSRP == Enum_FALSE)
	{
		if (isSpamReportMsg != Enum_TRUE)
		{
			CPM_STRING_CONCATENATE(pFetaureTags->ppFeatureTagHeaderValue[0]->pParamValue,
				uLen - pal_StringLength(pFetaureTags->ppFeatureTagHeaderValue[0]->pParamValue),
				(u_char*)";require;explicit", pal_StringLength((u_char*)";require;explicit"), uError, MEM_ERR);
		}
	}
#endif

	//pFetaureTags->ppFeatureTagHeaderValue[0]->pParamValue = pal_StringCreate(pContact, pal_StringLength(pContact));

	pFetaureTags->ppFeatureTagHeaderValue[1]->pParamName = pal_StringCreate(
		ECRIO_CPM_CONTACT_HEADER_STRING, pal_StringLength(ECRIO_CPM_CONTACT_HEADER_STRING));
	pFetaureTags->ppFeatureTagHeaderValue[1]->pParamValue = pContact;

	/** Feature-tag format for P-Preferred-Service header */
#if 0
	uLen = pal_StringLength(pICSIValue) + 1;
	pal_MemoryAllocate(uLen, (void **)&pPService);

	pal_StringNCopy(pPService, pICSIValue);

	pFetaureTags->ppFeatureTagHeaderValue[2]->pParamName = pal_StringCreate(
		ECRIO_CPM_P_PREFERRED_SERVICE_HEADER_STRING, pal_StringLength(ECRIO_CPM_P_PREFERRED_SERVICE_HEADER_STRING));
	pFetaureTags->ppFeatureTagHeaderValue[2]->pParamValue = pPService;
#endif

    pFetaureTags->ppFeatureTagHeaderValue[2]->pParamName = pal_StringCreate(
            ECRIO_CPM_P_PREFERRED_SERVICE_HEADER_STRING,
            pal_StringLength(ECRIO_CPM_P_PREFERRED_SERVICE_HEADER_STRING));

	if (bIsGroupChat) {
        pFetaureTags->ppFeatureTagHeaderValue[2]->pParamValue = pal_StringCreate(
				ECRIO_CPM_P_PREFERRED_SERVICE_3GPP_FEATURE_GROUP_SESSION,
                pal_StringLength(ECRIO_CPM_P_PREFERRED_SERVICE_3GPP_FEATURE_GROUP_SESSION));
    }
	else if (isPrivMngCmd || isSpamReportMsg)
	{
		pFetaureTags->ppFeatureTagHeaderValue[2]->pParamValue = pal_StringCreate(
				ECRIO_CPM_P_PREFERRED_SERVICE_3GPP_FEATURE_SYSTEMMSG,
				pal_StringLength(ECRIO_CPM_P_PREFERRED_SERVICE_3GPP_FEATURE_SYSTEMMSG));
	}
	else if(bIsPagerMode)
	{
		pFetaureTags->ppFeatureTagHeaderValue[2]->pParamValue = pal_StringCreate(
				ECRIO_CPM_P_PREFERRED_SERVICE_3GPP_FEATURE_PAGERMSG,
				pal_StringLength(ECRIO_CPM_P_PREFERRED_SERVICE_3GPP_FEATURE_PAGERMSG));
	}
	else if(bIsLargeMode)
	{
		pFetaureTags->ppFeatureTagHeaderValue[2]->pParamValue = pal_StringCreate(
				ECRIO_CPM_P_PREFERRED_SERVICE_3GPP_FEATURE_LARGEMSG,
				pal_StringLength(ECRIO_CPM_P_PREFERRED_SERVICE_3GPP_FEATURE_LARGEMSG));
	}
	else if (bDeleteToken == Enum_TRUE)
	{
		pFetaureTags->ppFeatureTagHeaderValue[2]->pParamValue = pal_StringCreate(
			ECRIO_CPM_P_PREFERRED_SERVICE_3GPP_FEATURE_SYSTEMMSG, 
			pal_StringLength(ECRIO_CPM_P_PREFERRED_SERVICE_3GPP_FEATURE_SYSTEMMSG));
	}
	else 
	{
		pFetaureTags->ppFeatureTagHeaderValue[2]->pParamValue = pal_StringCreate(
				ECRIO_CPM_P_PREFERRED_SERVICE_3GPP_FEATURE_SESSION,
				pal_StringLength(ECRIO_CPM_P_PREFERRED_SERVICE_3GPP_FEATURE_SESSION));
	}

	*ppFeatureTags = pFetaureTags;

	goto END;

MEM_ERR:

	if (pFetaureTags != NULL)
	{
		if (pFetaureTags->ppFeatureTagHeaderValue != NULL)
		{
			int i;
			for (i = 0; i < pFetaureTags->uNumberOfTags; i++)
			{
				if (pFetaureTags->ppFeatureTagHeaderValue[i] != NULL)
				{
					if (pFetaureTags->ppFeatureTagHeaderValue[i]->pParamName != NULL)
					{
						pal_MemoryFree((void**)&pFetaureTags->ppFeatureTagHeaderValue[i]->pParamName);
					}
					if (pFetaureTags->ppFeatureTagHeaderValue[i]->pParamValue != NULL)
					{
						pal_MemoryFree((void**)&pFetaureTags->ppFeatureTagHeaderValue[i]->pParamValue);
					}
					pal_MemoryFree((void**)&pFetaureTags->ppFeatureTagHeaderValue[i]);
				}
			}
			pal_MemoryFree((void**)&pFetaureTags->ppFeatureTagHeaderValue);
		}
		pal_MemoryFree((void**)&pFetaureTags);
	}

END:
Error_Level_01:

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uError;
}

u_int32 ec_CPM_PopulateCPIMHeaders
(
	EcrioCPMContextStruct *pContext,
	u_char* pFromAddr,
	u_char* pFromDispName,
	u_char* pDest,
	EcrioCPMMessageStruct *pMsgStruct,
	CPIMMessageStruct *pCPIMMessage
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	EcrioDateAndTimeStruct dateAndTime = { 0 };
	
	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	switch (pMsgStruct->eTrafficType)
	{
	case EcrioCPMTrafficTypeAdvertisement:
		pCPIMMessage->trfType = CPIMTrafficType_Advertisement;
		break;
	case EcrioCPMTrafficTypePayment:
		pCPIMMessage->trfType = CPIMTrafficType_Payment;
		break;
	case EcrioCPMTrafficTypePremium:
		pCPIMMessage->trfType = CPIMTrafficType_Premium;
		break;
	case EcrioCPMTrafficTypeSubscription:
		pCPIMMessage->trfType = CPIMTrafficType_Subscription;
		break;
	case EcrioCPMTrafficTypePlugin:
		pCPIMMessage->trfType = CPIMTrafficType_Plugin;
		break;
	default:
		pCPIMMessage->trfType = CPIMTrafficType_None;
		break;
	}

	pCPIMMessage->dnType = CPIMDispoNotifReq_None;

	if ((pMsgStruct->imdnConfig & EcrioCPMIMDispositionConfigPositiveDelivery) == EcrioCPMIMDispositionConfigPositiveDelivery)
		pCPIMMessage->dnType |= CPIMDispoNotifReq_Positive;

	if ((pMsgStruct->imdnConfig & EcrioCPMIMDispositionConfigNegativeDelivery) == EcrioCPMIMDispositionConfigNegativeDelivery)
		pCPIMMessage->dnType |= CPIMDispoNotifReq_Negative;

	if ((pMsgStruct->imdnConfig & EcrioCPMIMDispositionConfigDisplay) == EcrioCPMIMDispositionConfigDisplay)
		pCPIMMessage->dnType |= CPIMDispoNotifReq_Display;

	pal_MemorySet(pContext->pString, 0, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE);

	if (pDest != NULL)
	{
		EcrioSigMgrURISchemesEnum uriType = EcrioSigMgrURISchemeNone;

		uriType = EcrioSigMgrGetUriType(pContext->hEcrioSigMgrHandle, pDest);

		if (uriType == EcrioSigMgrURISchemeSIP)
		{
			pCPIMMessage->pToAddr = pDest;
			pCPIMMessage->pDateTime = pContext->pString;
		}
		else if (uriType == EcrioSigMgrURISchemeTEL)
		{
			/*pCPIMMessage->pToAddr = pContext->pString;

			pal_StringNCopy(pCPIMMessage->pToAddr, (const u_char *)"sip:");
			if(pal_StringFindSubString((const u_char*)pDest, (const u_char*)"phone-context") == NULL)
				pal_StringNConcatenate(pCPIMMessage->pToAddr, pDest + 4); //truncate 'tel:'
			else
			{
				u_char* pTemp = pal_StringFindSubString((const u_char*)pDest, (const u_char*)";");
				pal_StringNConcatenate(pCPIMMessage->pToAddr, pDest + 4, (pTemp-(pDest+4)));
			}
			pal_StringNConcatenate(pCPIMMessage->pToAddr, (const u_char *)"@");
			pal_StringNConcatenate(pCPIMMessage->pToAddr, pContext->pLocalDomain);
			pCPIMMessage->pDateTime = pContext->pString + pal_StringLength(pCPIMMessage->pToAddr) + 2;*/
			pCPIMMessage->pToAddr = pDest;
			pCPIMMessage->pDateTime = pContext->pString;
		}
		else
		{
			pCPIMMessage->pToAddr = pContext->pString;
			if (NULL == pal_StringNCopy(pCPIMMessage->pToAddr, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE, (const u_char *)"sip:", 
				pal_StringLength((const u_char *)"sip:")))
			{
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto END;
			}
			if (NULL == pal_StringNConcatenate(pCPIMMessage->pToAddr, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pCPIMMessage->pToAddr), pDest, pal_StringLength(pDest)))
			{
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto END;
			}
			if (NULL == pal_StringNConcatenate(pCPIMMessage->pToAddr, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pCPIMMessage->pToAddr), (const u_char *)"@", 
				pal_StringLength((const u_char *)"@")))
			{
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto END;
			}
			if (NULL == pal_StringNConcatenate(pCPIMMessage->pToAddr, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE - pal_StringLength(pCPIMMessage->pToAddr), pContext->pLocalDomain, 
				pal_StringLength(pContext->pLocalDomain)))
			{
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto END;
			}
			pCPIMMessage->pDateTime = pContext->pString + pal_StringLength(pCPIMMessage->pToAddr) + 2;
		}		
	}
	else
	{
		pCPIMMessage->pToAddr = NULL;
		pCPIMMessage->pDateTime = pContext->pString;
	}

	/*ISO8601 style (yyyyMMdd'T'HHmmss.SSSZ)*/	
	pal_UtilityGetDateAndTime(&dateAndTime);
	if (0 >= pal_StringSNPrintf((char *)pCPIMMessage->pDateTime, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE, (char *)"%04u-%02u-%02uT%02u:%02u:%02uZ", dateAndTime.year, dateAndTime.month, \
		dateAndTime.day, dateAndTime.hour, dateAndTime.minute, dateAndTime.second))
	{
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto END;
	}

	pCPIMMessage->pFromAddr = pFromAddr;
	pCPIMMessage->pDisplayName = pFromDispName;
	
	pCPIMMessage->pMsgId = (u_char *)pMsgStruct->pIMDNMsgId;

	if ((NULL != pMsgStruct->message.pBuffer) && (pMsgStruct->message.pBuffer->pMessage))
	{
		pCPIMMessage->buff[pCPIMMessage->uNumOfBuffers].pMsgBody = pMsgStruct->message.pBuffer->pMessage;
		pCPIMMessage->buff[pCPIMMessage->uNumOfBuffers].uMsgLen = pMsgStruct->message.pBuffer->uMessageLen;
	}
	switch (pMsgStruct->eContentType)
	{
	case EcrioCPMContentTypeEnum_Text:
	{
		pCPIMMessage->buff[pCPIMMessage->uNumOfBuffers].eContentType = CPIMContentType_Text;
		pCPIMMessage->uNumOfBuffers++;
	}
	break;
	case EcrioCPMContentTypeEnum_FileTransferOverHTTP:
	{
		pCPIMMessage->buff[pCPIMMessage->uNumOfBuffers].eContentType = CPIMContentType_FileTransferOverHTTP;
		pCPIMMessage->uNumOfBuffers++;
	}
	break;
	case EcrioCPMContentTypeEnum_RichCard:
	{
		pCPIMMessage->buff[pCPIMMessage->uNumOfBuffers].eContentType = CPIMContentType_RichCard;
		pCPIMMessage->uNumOfBuffers++;
	}
	break;
	case EcrioCPMContentTypeEnum_IMDN:
	{
		pCPIMMessage->buff[pCPIMMessage->uNumOfBuffers].eContentType = CPIMContentType_IMDN;
		pCPIMMessage->uNumOfBuffers++;
	}
	break;
	case EcrioCPMContentTypeEnum_GroupData:
	{
		pCPIMMessage->buff[pCPIMMessage->uNumOfBuffers].eContentType = CPIMContentType_CpmGroupData;
		pCPIMMessage->uNumOfBuffers++;
	}
	break;
	case EcrioCPMContentTypeEnum_Composing:
	{
		pCPIMMessage->buff[pCPIMMessage->uNumOfBuffers].eContentType = CPIMContentType_Specified;
		pCPIMMessage->buff[pCPIMMessage->uNumOfBuffers].pContentType = pal_StringCreate((u_char*)"application/im-iscomposing+xml", pal_StringLength((u_char*)"application/im-iscomposing+xml"));
		pCPIMMessage->uNumOfBuffers++;
	}
	break;
	case EcrioCPMContentTypeEnum_PushLocation:
	{
		pCPIMMessage->buff[pCPIMMessage->uNumOfBuffers].eContentType = CPIMContentType_PushLocation;
		pCPIMMessage->uNumOfBuffers++;
	}
	break;
	default:
		break;
	}

	if (pMsgStruct->pBotSuggestion != NULL)
	{
		pCPIMMessage->buff[pCPIMMessage->uNumOfBuffers].pMsgBody = pMsgStruct->pBotSuggestion->pJson;
		pCPIMMessage->buff[pCPIMMessage->uNumOfBuffers].uMsgLen = pMsgStruct->pBotSuggestion->uJsonLen;
		pCPIMMessage->buff[pCPIMMessage->uNumOfBuffers].eContentType = (pMsgStruct->pBotSuggestion->bIsBotSuggestionResponse) ? CPIMContentType_SuggestionResponse : CPIMContentType_SuggestedChipList;
		pCPIMMessage->uNumOfBuffers++;
	}

END:
	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uError;
}

#if 0
u_int32 ec_CPM_HandleCPIMMessage
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMSessionStruct *pSession,
	MSRPTextMessageStruct *pMSRPText,
	u_char *pSessionId,
	EcrioCPMNotifStruct* pCPMNotifStruct
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	u_int32 uCPIMError = ECRIO_CPIM_NO_ERROR;
	u_int32 counter = 0, imdnConfig = EcrioCPMIMDispositionConfigNone;
	u_int32 uIMDNBodyCnt = 0;
	u_int32 uIMDNBufferIndex = 0;
	BoolEnum bHandled = Enum_FALSE;
	CPIMMessageStruct cpimMessage = { 0 };
	CPIMMessageBufferStruct *pBuffStruct = NULL;
	EcrioCPMSessionMessageStruct *pIncSessionMsg = NULL;
	EcrioCPMGroupDataRequestStruct* pDmStruct = NULL;
	EcrioCPMBotSuggestionStruct *pBotSuggestion = NULL;
	EcrioCPMGroupChatIconStruct* pGroupIcon = NULL;
	EcrioCPMTrafficTypeEnum eTrafficType = EcrioCPMTrafficTypeNone;
	EcrioCPMParsedXMLStruct *pParsedXml = NULL;

	EcrioCPMIMDispoNotifStruct *pImdispNotif = NULL;

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	uCPIMError = EcrioCPIMHandler(pContext->hCPIMHandle, &cpimMessage, pMSRPText->u.text.pMsg, pMSRPText->u.text.uMsgLen);
	if (uCPIMError != ECRIO_CPIM_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioCPIMHandler failed, uError=%u",
			__FUNCTION__, __LINE__, uCPIMError);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}

	/*Populate IMDN Disposition-Notification*/
	imdnConfig = EcrioCPMIMDispositionConfigNone;
	if ((cpimMessage.dnType & CPIMDispoNotifReq_Negative) == CPIMDispoNotifReq_Negative)
	{
		imdnConfig |= EcrioCPMIMDispositionConfigNegativeDelivery;
	}

	if ((cpimMessage.dnType & CPIMDispoNotifReq_Positive) == CPIMDispoNotifReq_Positive)
	{
		imdnConfig |= EcrioCPMIMDispositionConfigPositiveDelivery;
	}

	if ((cpimMessage.dnType & CPIMDispoNotifReq_Display) == CPIMDispoNotifReq_Display)
	{
		imdnConfig |= EcrioCPMIMDispositionConfigDisplay;
	}

	switch (cpimMessage.trfType)
	{
	case CPIMTrafficType_Advertisement:
		eTrafficType = EcrioCPMTrafficTypeAdvertisement;
		break;
	case CPIMTrafficType_Payment:
		eTrafficType = EcrioCPMTrafficTypePayment;
		break;
	case CPIMTrafficType_Premium:
		eTrafficType = EcrioCPMTrafficTypePremium;
		break;
	case CPIMTrafficType_Subscription:
		eTrafficType = EcrioCPMTrafficTypeSubscription;
		break;
	case CPIMTrafficType_Plugin:
		eTrafficType = EcrioCPMTrafficTypePlugin;
		break;
	default:
		break;
	}

	/** In this section, handle messages with support multipart bodies. However,
	 *  our framework does not assume multiple messages of the same content type
	 *  except for IMDN messages. if there are multiple bodies of content type
	 *  other than IMDN messages, only the first content is valid, and the rest
	 *  will be discarded.
	 */
	 /* Get list of IMDN messages in the incoming buffer */
	for (counter = 0; counter < cpimMessage.uNumOfBuffers; counter++)
	{
		pBuffStruct = &cpimMessage.buff[counter];
		if(cpimMessage.buff[counter].eContentType == CPIMContentType_IMDN)
		{
			uIMDNBodyCnt++;
		}
	}
	/* Cap maximum IMDN message to be handled */
	if(uIMDNBodyCnt > ECRIO_CPM_MAX_IMDN_MESSAGE)
		uIMDNBodyCnt = ECRIO_CPM_MAX_IMDN_MESSAGE;

	/*Populate message*/
	for (counter = 0; counter < cpimMessage.uNumOfBuffers; counter++)
	{
		pBuffStruct = &cpimMessage.buff[counter];

		switch (pBuffStruct->eContentType)
		{
		case CPIMContentType_Text:
		{
			if (pIncSessionMsg != NULL)
			{
				CPMLOGI(pContext->hLogHandle, KLogTypeGeneral,
					"%s:%u\tMultiple bodies were detected for the same content type and will be discarded.",
					__FUNCTION__, __LINE__);
				break;
			}

			pal_MemoryAllocate(sizeof(EcrioCPMSessionMessageStruct), (void**)&pIncSessionMsg);
			if (pIncSessionMsg == NULL)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto Error_Level_01;
			}
			pIncSessionMsg->pSessionId = (char *)pal_StringCreate(pSessionId, pal_StringLength(pSessionId));
			pal_MemoryAllocate(sizeof(EcrioCPMMessageStruct), (void**)&pIncSessionMsg->pMessage);
			if (pIncSessionMsg->pMessage == NULL)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto Error_Level_01;
			}
			pIncSessionMsg->pMessage->imdnConfig = imdnConfig;
			pIncSessionMsg->pMessage->pIMDNMsgId = (char *)cpimMessage.pMsgId;
			pIncSessionMsg->pSenderURI = (char *)cpimMessage.pFromAddr;
			pIncSessionMsg->pDisplayName = (char *)cpimMessage.pDisplayName;

			pal_MemoryAllocate(sizeof(EcrioCPMBufferStruct), (void**)&pIncSessionMsg->pMessage->message.pBuffer);
			if (pIncSessionMsg->pMessage->message.pBuffer == NULL)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto Error_Level_01;
			}
			pIncSessionMsg->pMessage->message.pBuffer->pMessage = pBuffStruct->pMsgBody;
			pIncSessionMsg->pMessage->message.pBuffer->uMessageLen = pBuffStruct->uMsgLen;
			pIncSessionMsg->pMessage->message.pBuffer->uByteRangeStart = 0;
			pIncSessionMsg->pMessage->message.pBuffer->uByteRangeEnd = 0;
			pIncSessionMsg->pMessage->message.pBuffer->pDate = cpimMessage.pDateTime;

			pIncSessionMsg->pMessage->eTrafficType = eTrafficType;

			pIncSessionMsg->pMessage->pBotSuggestion = NULL;
			pIncSessionMsg->pMessageId = (char *)pal_StringCreate(pMSRPText->pMessageId, pal_StringLength(pMSRPText->pMessageId));

			pIncSessionMsg->pMessage->eContentType = EcrioCPMContentTypeEnum_Text;
			/** Notify the upper layer. */
			pCPMNotifStruct->eNotifCmd = EcrioCPM_Notif_IncomingSessionMessage;
			pCPMNotifStruct->pNotifData = pIncSessionMsg;
		}
		break;
		case CPIMContentType_IMDN:
		{			
			IMDNDispoNotifStruct iMDN = { 0 };

			/** If more than the maximum number of IMDN messages are received, discard the rest. */
			if (uIMDNBufferIndex == ECRIO_CPM_MAX_IMDN_MESSAGE)
			{
				CPMLOGI(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tNo more IMDN messages will be handled: %d messages",
					__FUNCTION__, __LINE__, ECRIO_CPM_MAX_IMDN_MESSAGE);
				break;
			}

			EcrioIMDNHandler(pContext->hIMDNHandle, &iMDN, pBuffStruct->pMsgBody, pBuffStruct->uMsgLen);
			if (ECRIO_IMDN_NO_ERROR != uError)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioIMDNHandler() error=%u",
					__FUNCTION__, __LINE__, uError);

				uError = ECRIO_CPM_INTERNAL_ERROR;
				goto Error_Level_01;
			}

			if (pImdispNotif == NULL)
			{
				/** First IMDN message body */
				pal_MemoryAllocate(sizeof(EcrioCPMIMDispoNotifStruct), (void**)&pImdispNotif);
				if (pImdispNotif == NULL)
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
						__FUNCTION__, __LINE__);
					uError = ECRIO_CPM_MEMORY_ERROR;
					goto Error_Level_01;
				}
				pImdispNotif->pDisplayName = (char*)pal_StringCreate(cpimMessage.pDisplayName, pal_StringLength(cpimMessage.pDisplayName));
				pImdispNotif->pSenderURI = (char *)cpimMessage.pFromAddr;
				pImdispNotif->pDestURI = (char *)cpimMessage.pToAddr;
				pal_MemoryAllocate(sizeof(EcrioCPMIMDispoNotifBodyStruct)*uIMDNBodyCnt, (void**)&pImdispNotif->pCPMIMDispoNotifBody);
				if (pImdispNotif->pCPMIMDispoNotifBody == NULL)
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
						__FUNCTION__, __LINE__);
					uError = ECRIO_CPM_MEMORY_ERROR;
					goto Error_Level_01;
				}
			}

			pImdispNotif->pCPMIMDispoNotifBody[uIMDNBufferIndex].pIMDNMsgId = iMDN.pMsgId;
			pImdispNotif->pCPMIMDispoNotifBody[uIMDNBufferIndex].pRecipientUri = iMDN.pRecipientUri;
			pImdispNotif->pCPMIMDispoNotifBody[uIMDNBufferIndex].pOriginalRecipientUri = iMDN.pOriginalRecipientUri;
			pImdispNotif->pCPMIMDispoNotifBody[uIMDNBufferIndex].pDate = iMDN.pDateTime;

			/* map the delivery enum */
			switch (iMDN.eDelivery)
			{
			case IMDNDeliveryNotif_Delivered:
			{
				pImdispNotif->pCPMIMDispoNotifBody[uIMDNBufferIndex].eDelivery = EcrioCPMIMDNDeliveryNotif_Delivered;
			}
			break;
			case IMDNDeliveryNotif_Failed:
			{
				pImdispNotif->pCPMIMDispoNotifBody[uIMDNBufferIndex].eDelivery = EcrioCPMIMDNDeliveryNotif_Failed;
			}
			break;
			case IMDNDeliveryNotif_Forbidden:
			{
				pImdispNotif->pCPMIMDispoNotifBody[uIMDNBufferIndex].eDelivery = EcrioCPMIMDNDeliveryNotif_Forbidden;
			}
			break;
			case IMDNDeliveryNotif_Error:
			{
				pImdispNotif->pCPMIMDispoNotifBody[uIMDNBufferIndex].eDelivery = EcrioCPMIMDNDeliveryNotif_Error;
			}
			break;
			default:
			{
				pImdispNotif->pCPMIMDispoNotifBody[uIMDNBufferIndex].eDelivery = EcrioCPMIMDNDeliveryNotif_None;
			}
			break;
			}
			/* map the Disposition enum */
			switch (iMDN.eDisNtf)
			{
			case IMDNDisNtfTypeReq_Delivery:
			{
				pImdispNotif->pCPMIMDispoNotifBody[uIMDNBufferIndex].eDisNtf = EcrioCPMIMDispositionNtfTypeReq_Delivery;
			}
			break;
			case IMDNDisNtfTypeReq_Display:
			{
				pImdispNotif->pCPMIMDispoNotifBody[uIMDNBufferIndex].eDisNtf = EcrioCPMIMDispositionNtfTypeReq_Display;
			}
			break;
			default:
			{
				pImdispNotif->pCPMIMDispoNotifBody[uIMDNBufferIndex].eDisNtf = EcrioCPMIMDispositionNtfTypeReq_None;
			}
			break;
			}
			/* map the display enum */
			switch (iMDN.eDisplay)
			{
			case IMDNDisplayNotif_Displayed:
			{
				pImdispNotif->pCPMIMDispoNotifBody[uIMDNBufferIndex].eDisplay = EcrioCPMIMDNDisplayNotif_Displayed;
			}
			break;
			case IMDNDisplayNotif_Forbidden:
			{
				pImdispNotif->pCPMIMDispoNotifBody[uIMDNBufferIndex].eDisplay = EcrioCPMIMDNDisplayNotif_Forbidden;
			}
			break;
			case IMDNDisplayNotif_Error:
			{
				pImdispNotif->pCPMIMDispoNotifBody[uIMDNBufferIndex].eDisplay = EcrioCPMIMDNDisplayNotif_Error;
			}
			break;
			default:
			{
				pImdispNotif->pCPMIMDispoNotifBody[uIMDNBufferIndex].eDisplay = EcrioCPMIMDNDisplayNotif_None;
			}
			break;
			}

			uIMDNBufferIndex++;
			pImdispNotif->uNumOfNotifBody = uIMDNBufferIndex;
			pCPMNotifStruct->eNotifCmd = EcrioCPM_Notif_IMDNDisposition;
			pCPMNotifStruct->pNotifData = (void*)pImdispNotif;
		}
		break;
		case CPIMContentType_FileTransferOverHTTP:
		{
			if (pIncSessionMsg != NULL)
			{
				CPMLOGI(pContext->hLogHandle, KLogTypeGeneral,
					"%s:%u\tMultiple bodies were detected for the same content type and will be discarded.",
					__FUNCTION__, __LINE__);
				break;
			}

			pal_MemoryAllocate(sizeof(EcrioCPMSessionMessageStruct), (void**)&pIncSessionMsg);
			if (pIncSessionMsg == NULL)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto Error_Level_01;
			}
			pIncSessionMsg->pSessionId = (char *)pal_StringCreate(pSessionId, pal_StringLength(pSessionId));
			pal_MemoryAllocate(sizeof(EcrioCPMMessageStruct), (void**)&pIncSessionMsg->pMessage);
			if (pIncSessionMsg->pMessage == NULL)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto Error_Level_01;
			}
			pIncSessionMsg->pMessage->imdnConfig = imdnConfig;
			pIncSessionMsg->pMessage->pIMDNMsgId = (char *)cpimMessage.pMsgId;
			pIncSessionMsg->pSenderURI = (char *)cpimMessage.pFromAddr;
			if (cpimMessage.pDisplayName != NULL)
			{
				pIncSessionMsg->pDisplayName = (char *)pal_StringCreate(cpimMessage.pDisplayName, pal_StringLength(cpimMessage.pDisplayName));
			}

			pal_MemoryAllocate(sizeof(EcrioCPMBufferStruct), (void**)&pIncSessionMsg->pMessage->message.pBuffer);
			if (pIncSessionMsg->pMessage->message.pBuffer == NULL)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto Error_Level_01;
			}
			pIncSessionMsg->pMessage->message.pBuffer->pMessage = pBuffStruct->pMsgBody;
			pIncSessionMsg->pMessage->message.pBuffer->uMessageLen = pBuffStruct->uMsgLen;
			pIncSessionMsg->pMessage->message.pBuffer->uByteRangeStart = 0;
			pIncSessionMsg->pMessage->message.pBuffer->uByteRangeEnd = 0;
			pIncSessionMsg->pMessage->message.pBuffer->pDate = cpimMessage.pDateTime;

			pIncSessionMsg->pMessage->eTrafficType = eTrafficType;

			pIncSessionMsg->pMessage->pBotSuggestion = NULL;
			pIncSessionMsg->pMessageId = (char *)pal_StringCreate(pMSRPText->pMessageId, pal_StringLength(pMSRPText->pMessageId));

			pIncSessionMsg->pMessage->eContentType = EcrioCPMContentTypeEnum_FileTransferOverHTTP;
			/** Notify the upper layer. */
			pCPMNotifStruct->eNotifCmd = EcrioCPM_Notif_IncomingSessionMessage;
			pCPMNotifStruct->pNotifData = pIncSessionMsg;
		}
		break;
		case CPIMContentType_RichCard:
		{
			if (pIncSessionMsg != NULL)
			{
				CPMLOGI(pContext->hLogHandle, KLogTypeGeneral,
					"%s:%u\tMultiple bodies were detected for the same content type and will be discarded.",
					__FUNCTION__, __LINE__);
				break;
			}

			pal_MemoryAllocate(sizeof(EcrioCPMSessionMessageStruct), (void**)&pIncSessionMsg);
			if (pIncSessionMsg == NULL)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto Error_Level_01;
			}

			pIncSessionMsg->pMessageId = (char *)pal_StringCreate(pMSRPText->pMessageId, pal_StringLength(pMSRPText->pMessageId));
			pIncSessionMsg->pSessionId = (char *)pal_StringCreate(pSessionId, pal_StringLength(pSessionId));
			pIncSessionMsg->pSenderURI = (char *)cpimMessage.pFromAddr;

			pal_MemoryAllocate(sizeof(EcrioCPMMessageStruct), (void**)&pIncSessionMsg->pMessage);
			if (pIncSessionMsg->pMessage == NULL)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto Error_Level_01;
			}
			pIncSessionMsg->pMessage->eTrafficType = eTrafficType;
			pIncSessionMsg->pMessage->imdnConfig = imdnConfig;
			pIncSessionMsg->pMessage->pBotSuggestion = NULL;
			pIncSessionMsg->pMessage->pIMDNMsgId = (char *)cpimMessage.pMsgId;

			pal_MemoryAllocate(sizeof(EcrioCPMBufferStruct), (void**)&pIncSessionMsg->pMessage->message.pBuffer);
			if (pIncSessionMsg->pMessage->message.pBuffer == NULL)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto Error_Level_01;
			}
			pIncSessionMsg->pMessage->message.pBuffer->pMessage = pBuffStruct->pMsgBody;
			pIncSessionMsg->pMessage->message.pBuffer->uMessageLen = pBuffStruct->uMsgLen;
			pIncSessionMsg->pMessage->message.pBuffer->uByteRangeStart = 0;
			pIncSessionMsg->pMessage->message.pBuffer->uByteRangeEnd = 0;
			pIncSessionMsg->pMessage->message.pBuffer->pDate = cpimMessage.pDateTime;

			pIncSessionMsg->pMessage->eContentType = EcrioCPMContentTypeEnum_RichCard;

			/** Notify the upper layer. */
			pCPMNotifStruct->eNotifCmd = EcrioCPM_Notif_RichCardReceived;
			pCPMNotifStruct->pNotifData = pIncSessionMsg;
		}
		break;
		case CPIMContentType_SuggestedChipList:
		{
			if (pBotSuggestion != NULL)
			{
				CPMLOGI(pContext->hLogHandle, KLogTypeGeneral,
					"%s:%u\tMultiple bodies were detected for the same content type and will be discarded.",
					__FUNCTION__, __LINE__);
				break;
			}

			pal_MemoryAllocate(sizeof(EcrioCPMBotSuggestionStruct), (void**)&pBotSuggestion);
			if (pBotSuggestion == NULL)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto Error_Level_01;
			}
			pBotSuggestion->bIsBotSuggestionResponse = Enum_FALSE;
			pBotSuggestion->pJson = pBuffStruct->pMsgBody;
			pBotSuggestion->uJsonLen = pBuffStruct->uMsgLen;
		}
		break;
		case CPIMContentType_SuggestionResponse:
		{
			if (pBotSuggestion != NULL)
			{
				CPMLOGI(pContext->hLogHandle, KLogTypeGeneral,
					"%s:%u\tMultiple bodies were detected for the same content type and will be discarded.",
					__FUNCTION__, __LINE__);
				break;
			}

			pal_MemoryAllocate(sizeof(EcrioCPMBotSuggestionStruct), (void**)&pBotSuggestion);
			if (pBotSuggestion == NULL)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto Error_Level_01;
			}
			pBotSuggestion->bIsBotSuggestionResponse = Enum_TRUE;
			pBotSuggestion->pJson = pBuffStruct->pMsgBody;
			pBotSuggestion->uJsonLen = pBuffStruct->uMsgLen;
		}
		break;
		case CPIMContentType_CpmGroupData:
		{
			BoolEnum bMatch = Enum_FALSE;

			uError = ec_CPM_ParseXmlDocument(pContext, &pParsedXml, (char*)pBuffStruct->pMsgBody, pBuffStruct->uMsgLen);
			if (ECRIO_CPM_NO_ERROR != uError)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_ParseXmlDocument() error=%u",
					__FUNCTION__, __LINE__, uError);

				uError = ECRIO_CPM_INTERNAL_ERROR;
				goto Error_Level_01;
			}
			bMatch = ec_CPM_FindXmlElementNameAndValue(pParsedXml, (u_char *)"request", NULL);
			if (bMatch == Enum_TRUE)
			{
				BoolEnum bRet = Enum_FALSE;
				
				if (pDmStruct != NULL)
				{
					CPMLOGI(pContext->hLogHandle, KLogTypeGeneral,
						"%s:%u\tMultiple bodies were detected for the same content type and will be discarded.",
						__FUNCTION__, __LINE__);
					break;
				}

				pal_MemoryAllocate(sizeof(EcrioCPMGroupDataRequestStruct), (void**)&pDmStruct);
				if (pDmStruct == NULL)
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
						__FUNCTION__, __LINE__);
					uError = ECRIO_CPM_MEMORY_ERROR;
					goto Error_Level_01;
				}
				CPMLOGD(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tGroup Data Matched",
						__FUNCTION__, __LINE__);

				pDmStruct->type = EcrioCPMGroupDataMngType_None;

				bRet = ec_CPM_FindXmlAttributeNameAndValue(pParsedXml, (u_char *)"target", (u_char *)"icon");
				if (Enum_TRUE == bRet)
				{
					u_char* pValue = NULL;
					pValue = ec_CPM_GetXmlAttributeValue(pParsedXml, (u_char *)"action");
					if (pal_StringCompare((u_char *)pValue, (u_char *)"set") == 0)
					{
						pDmStruct->icon.type = EcrioCPMGroupDataActionType_Set;
					}
					else if (pal_StringCompare((u_char *)pValue, (u_char *)"delete") == 0)
					{
						pDmStruct->icon.type = EcrioCPMGroupDataActionType_Delete;
					}
					else
					{
						pDmStruct->icon.type = EcrioCPMGroupDataActionType_None;
					}
					
					pValue = NULL;
					pValue = ec_CPM_GetXmlAttributeValue(pParsedXml, (u_char *)"icon-uri");
					if (pValue != NULL)
					{
						pDmStruct->icon.pUri = pal_StringCreate((u_char*)pValue, pal_StringLength((u_char*)pValue));
					}
					else
					{
						pValue = ec_CPM_GetXmlAttributeValue(pParsedXml, (u_char*)"file-info");
						if (pValue != NULL)
						{
							pDmStruct->icon.pFileInfo = pal_StringCreate((u_char*)pValue, pal_StringLength((u_char*)pValue));
						}
					}
					pDmStruct->type = EcrioCPMGroupDataMngType_Icon;

					CPMLOGD(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tGroup Data ICON",
							__FUNCTION__, __LINE__);
				}
				bRet = ec_CPM_FindXmlAttributeNameAndValue(pParsedXml, (u_char *)"target", (u_char *)"subject");
				if (Enum_TRUE == bRet)
				{
					u_char* pValue = NULL;

					if (pDmStruct->type == EcrioCPMGroupDataMngType_Icon)
						pDmStruct->type = EcrioCPMGroupDataMngType_Multi;
					else
						pDmStruct->type = EcrioCPMGroupDataMngType_Subject;

					pValue = ec_CPM_GetXmlAttributeValue(pParsedXml, (u_char *)"action");
					if (pal_StringCompare((u_char *)pValue, (u_char *)"set") == 0)
					{
						pDmStruct->subject.type = EcrioCPMGroupDataActionType_Set;
					}
					else if (pal_StringCompare((u_char *)pValue, (u_char *)"delete") == 0)
					{
						pDmStruct->subject.type = EcrioCPMGroupDataActionType_Delete;
					}
					else
					{
						pDmStruct->subject.type = EcrioCPMGroupDataActionType_None;
					}

					pValue = NULL;
					pValue = ec_CPM_GetXmlAttributeValue(pParsedXml, (u_char *)"subject");
					if (pValue != NULL)
					{
						pDmStruct->subject.pText = pal_StringCreate((u_char*)pValue, pal_StringLength((u_char*)pValue));
					}
					CPMLOGD(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tGroup Data SUBJECT",
							__FUNCTION__, __LINE__);
				}

				bRet = ec_CPM_FindXmlAttributeNameAndValue(pParsedXml, (u_char *)"target", (u_char *)"role");
				if (Enum_TRUE == bRet)
				{
					u_char* pValue = NULL;
					if (pDmStruct->type == EcrioCPMGroupDataMngType_Icon ||
						pDmStruct->type == EcrioCPMGroupDataMngType_Subject)
						pDmStruct->type = EcrioCPMGroupDataMngType_Multi;
					else
						pDmStruct->type = EcrioCPMGroupDataMngType_Role;

					pValue = ec_CPM_GetXmlAttributeValue(pParsedXml, (u_char *)"action");
					if (pal_StringCompare((u_char *)pValue, (u_char *)"set") == 0)
					{
						pDmStruct->role.type = EcrioCPMGroupDataActionType_Set;
					}
					else if (pal_StringCompare((u_char *)pValue, (u_char *)"move") == 0)
					{
						pDmStruct->role.type = EcrioCPMGroupDataActionType_Move;
					}
					else
					{
						pDmStruct->role.type = EcrioCPMGroupDataActionType_None;
					}

					pValue = ec_CPM_GetXmlAttributeValue(pParsedXml, (u_char *)"user-role");
					if (pValue != NULL)
						pDmStruct->role.pUserRole = pal_StringCreate((u_char*)pValue, pal_StringLength((u_char*)pValue));
					else
						pDmStruct->role.pUserRole = NULL;

					pValue = ec_CPM_GetXmlAttributeValue(pParsedXml, (u_char *)"participant");
					if (pValue != NULL)
						pDmStruct->role.pHostUri = pal_StringCreate((u_char*)pValue, pal_StringLength((u_char*)pValue));
					else
						pDmStruct->role.pHostUri = NULL;

					CPMLOGD(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tGroup Data Role",
						__FUNCTION__, __LINE__);
				}

				pDmStruct->pSessionId = (char*)pal_StringCreate(pSessionId, pal_StringLength(pSessionId));
				pCPMNotifStruct->eNotifCmd = EcrioCPM_Notif_GroupDataStatus;
				pCPMNotifStruct->pNotifData = (void*)pDmStruct;
			}
			else
			{
				/*bMatch = ec_CPM_FindXmlElementNameAndValue(pParsedXml, (u_char *)"response-code", (u_char*)"200");
				if (bMatch == Enum_TRUE)
				{
					pCPMNotifStruct->eNotifCmd = EcrioCPM_Notif_ModifyGroupChatSuccess;
				}
				else
				{
					pCPMNotifStruct->eNotifCmd = EcrioCPM_Notif_ModifyGroupChatFailure;
				}
				pCPMNotifStruct->pNotifData = pSessionId;*/
				CPMLOGD(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tGroup Data Mismatched",
						__FUNCTION__, __LINE__);
			}
		}
		break;
		case CPIMContentType_Specified:
		{
			CPMLOGD(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tCPIMContentType_Specified - Buffer Content-Type: %s",
				__FUNCTION__, __LINE__, pBuffStruct->pContentType);
			if (pal_StringFindSubString(pBuffStruct->pContentType, (u_char*)"application/im-iscomposing+xml") != NULL)
			{
				EcrioCPMSessionMessageStruct *pSessionMsgStruct = NULL;
				MSRPTextMessageStruct *pMSRPText = NULL;

				MSRPIsComposingStruct composing = { 0 };
				EcrioCPMComposingStruct *pComposing = NULL;
				u_int32 uStrLen = 0;

				if (bHandled == Enum_TRUE)
				{
					CPMLOGI(pContext->hLogHandle, KLogTypeGeneral,
						"%s:%u\tMultiple bodies were detected for the same content type and will be discarded.",
						__FUNCTION__, __LINE__);
					break;
				}

				EcrioMSRPParseComposingXMLDocument(pSession->hMSRPSessionHandle, pBuffStruct->pMsgBody, pBuffStruct->uMsgLen, &composing);

				pal_MemoryAllocate(sizeof(EcrioCPMComposingStruct), (void**)&pComposing);
				if (pComposing == NULL)
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
						__FUNCTION__, __LINE__);
					uError = ECRIO_CPM_MEMORY_ERROR;
					goto Error_Level_01;
				}
				if (composing.pContent)
				{
					pComposing->pContent = pal_StringCreate(composing.pContent, pal_StringLength(composing.pContent));
					pal_MemoryFree((void**)&composing.pContent);
				}
				if (composing.pLastActive)
				{
					pComposing->pLastActive = pal_StringCreate(composing.pLastActive, pal_StringLength(composing.pLastActive));
					pal_MemoryFree((void**)&composing.pLastActive);
				}
				pComposing->uRefresh = composing.uRefresh;

				switch (composing.state)
				{
				case MSRPIsComposingState_Idle:
				{
					pComposing->eComposingState = EcrioCPMComposingType_Idle;
				}
				break;
				case MSRPIsComposingState_Active:
				{
					pComposing->eComposingState = EcrioCPMComposingType_Active;
				}
				break;
				default:
					pComposing->eComposingState = EcrioCPMComposingType_None;
					break;
				}

				if (cpimMessage.pDisplayName != NULL)
				{
					pComposing->pDisplayName = (char*)pal_StringCreate(cpimMessage.pDisplayName, pal_StringLength(cpimMessage.pDisplayName));
				}
				pComposing->pDestURI = (char*)pal_StringCreate(cpimMessage.pFromAddr, pal_StringLength(cpimMessage.pFromAddr));
				pal_MemoryAllocate(sizeof(EcrioCPMSessionMessageStruct), (void**)&pSessionMsgStruct);
				if (pSessionMsgStruct == NULL)
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
						__FUNCTION__, __LINE__);
					uError = ECRIO_CPM_MEMORY_ERROR;
					goto Error_Level_01;
				}
				pal_MemorySet((void*)pSessionMsgStruct, 0, sizeof(EcrioCPMSessionMessageStruct));
				pal_MemoryAllocate(sizeof(EcrioCPMMessageStruct), (void**)&pSessionMsgStruct->pMessage);
				if (pSessionMsgStruct->pMessage == NULL)
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
						__FUNCTION__, __LINE__);
					uError = ECRIO_CPM_MEMORY_ERROR;
					goto Error_Level_01;
				}
				pal_MemorySet((void*)pSessionMsgStruct->pMessage, 0, sizeof(EcrioCPMMessageStruct));
				pSessionMsgStruct->pMessage->message.pComposing = pComposing;
				pSessionMsgStruct->pMessage->eContentType = EcrioCPMContentTypeEnum_Composing;
				pSessionMsgStruct->pSessionId = (char *)pal_StringCreate(pSessionId, pal_StringLength(pSessionId));
				pSessionMsgStruct->pMessageId = NULL;
				pCPMNotifStruct->eNotifCmd = EcrioCPM_Notif_IMComposing;
				pCPMNotifStruct->pNotifData = pSessionMsgStruct;

				bHandled = Enum_TRUE;
			}
			else if (pal_StringFindSubString(pBuffStruct->pContentDisposition, (u_char*)"icon") != NULL)
			{
				if (pBuffStruct->pMsgBody != NULL)
				{
					pal_MemoryAllocate(sizeof(EcrioCPMGroupChatIconStruct), (void**)&pGroupIcon);
					if (pGroupIcon == NULL)
					{
						CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
							__FUNCTION__, __LINE__);
						uError = ECRIO_CPM_MEMORY_ERROR;
						goto Error_Level_01;
					}

					pal_MemorySet((void*)pGroupIcon, 0, sizeof(EcrioCPMGroupChatIconStruct));

					pGroupIcon->pContentType = (char*)pal_StringCreate(pBuffStruct->pContentType, pal_StringLength(pBuffStruct->pContentType));
					pGroupIcon->pContentTypeTransferEncoding = (char*)pal_StringCreate(pBuffStruct->pContentTransferEncoding, pal_StringLength(pBuffStruct->pContentTransferEncoding));

					pGroupIcon->uDataLength = pBuffStruct->uMsgLen;
					pal_MemoryAllocate(pBuffStruct->uMsgLen, (void**)&pGroupIcon->pData);
					if (pGroupIcon->pData == NULL)
					{
						pal_MemoryFree((void**)&pGroupIcon->pContentType);
						pal_MemoryFree((void**)&pGroupIcon);

						CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
							__FUNCTION__, __LINE__);
						uError = ECRIO_CPM_MEMORY_ERROR;
						goto Error_Level_01;
					}

					pal_MemoryCopy(pGroupIcon->pData, pGroupIcon->uDataLength, pBuffStruct->pMsgBody, pBuffStruct->uMsgLen);

					if (pDmStruct != NULL && pDmStruct->type == EcrioCPMGroupDataMngType_Icon && pDmStruct->icon.pFileInfo != NULL)
					{
						pGroupIcon->pFileInfo = (char*)pDmStruct->icon.pFileInfo;
						pDmStruct->icon.pFileInfo = NULL;
						if (pDmStruct->pSessionId != NULL)
						{
							pal_MemoryFree((void**)&pDmStruct->pSessionId);
						}

						pal_MemoryFree((void**)&pDmStruct);
					}

					/** Notify the upper layer. */
					pGroupIcon->pSenderURI = (char*)pal_StringCreate(cpimMessage.pFromAddr, pal_StringLength(cpimMessage.pFromAddr));
					pGroupIcon->pSessionId = (char*)pal_StringCreate(pSessionId, pal_StringLength(pSessionId));
					pCPMNotifStruct->eNotifCmd = EcrioCPM_Notif_GroupChatIcon;
					pCPMNotifStruct->pNotifData = pGroupIcon;
				}
			}
			else
			{
				if (pIncSessionMsg != NULL)
				{
					CPMLOGI(pContext->hLogHandle, KLogTypeGeneral,
						"%s:%u\tMultiple bodies were detected for the same content type and will be discarded.",
						__FUNCTION__, __LINE__);
					break;
				}

				pal_MemoryAllocate(sizeof(EcrioCPMSessionMessageStruct), (void**)&pIncSessionMsg);
				if (pIncSessionMsg == NULL)
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
						__FUNCTION__, __LINE__);
					uError = ECRIO_CPM_MEMORY_ERROR;
					goto Error_Level_01;
				}
				pIncSessionMsg->pSessionId = (char *)pal_StringCreate(pSessionId, pal_StringLength(pSessionId));
				pIncSessionMsg->pMessageId = (char *)pal_StringCreate(cpimMessage.pMsgId, pal_StringLength(cpimMessage.pMsgId));
				pIncSessionMsg->pTransactionId = NULL;

				pal_MemoryAllocate(sizeof(EcrioCPMMessageStruct), (void**)&pIncSessionMsg->pMessage);
				if (pIncSessionMsg->pMessage == NULL)
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
						__FUNCTION__, __LINE__);
					uError = ECRIO_CPM_MEMORY_ERROR;
					goto Error_Level_01;
				}
				pal_MemorySet((void*)pIncSessionMsg->pMessage, 0, sizeof(EcrioCPMMessageStruct));

				pIncSessionMsg->pMessage->eContentType = EcrioCPMContentTypeEnum_FileTransferOverMSRP;

				pal_MemoryAllocate(sizeof(EcrioCPMBufferStruct), (void**)&pIncSessionMsg->pMessage->message.pBuffer);
				if (pIncSessionMsg->pMessage->message.pBuffer == NULL)
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
						__FUNCTION__, __LINE__);
					uError = ECRIO_CPM_MEMORY_ERROR;
					goto Error_Level_01;
				}
				pIncSessionMsg->pMessage->message.pBuffer->pMessage = pBuffStruct->pMsgBody;
				pIncSessionMsg->pMessage->message.pBuffer->uMessageLen = pBuffStruct->uMsgLen;
				pIncSessionMsg->pMessage->message.pBuffer->uByteRangeStart = 1;
				pIncSessionMsg->pMessage->message.pBuffer->uByteRangeEnd = pBuffStruct->uMsgLen;
				pIncSessionMsg->pMessage->message.pBuffer->pDate = cpimMessage.pDateTime;
				pIncSessionMsg->pMessage->message.pBuffer->pContentType = pBuffStruct->pContentType;
				pIncSessionMsg->pMessage->imdnConfig = imdnConfig;

				/** Notify the upper layer. */
				pCPMNotifStruct->eNotifCmd = EcrioCPM_Notif_IncomingSessionMessage;
				pCPMNotifStruct->pNotifData = pIncSessionMsg;
			}
		}
		break;
#if 0
		case CPIMContentType_IsComposing:
		{
			EcrioCPMSessionMessageStruct *pSessionMsgStruct = NULL;
			EcrioCPMComposingStruct *pComposing = NULL;
			u_int32 uStrLen = 0;

			uError = ec_CPM_ParseXmlDocument(pContext, &pParsedXml, (char*)pBuffStruct->pMsgBody, pBuffStruct->uMsgLen);
			if (ECRIO_CPM_NO_ERROR != uError)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_ParseXmlDocument() error=%u",
					__FUNCTION__, __LINE__, uError);

				uError = ECRIO_CPM_INTERNAL_ERROR;
				goto Error_Level_01;
			}

			pal_MemoryAllocate(sizeof(EcrioCPMComposingStruct), (void**)&pComposing);
			pComposing->eComposingState = EcrioCPMComposingType_None;

			if (pal_StringCompare((u_char *)pParsedXml->pElementName, (u_char*)"isComposing") == 0)
			{
				for (i = 0; i < pParsedXml->uNumOfChild; i++)
				{
					pStruct = (EcrioCPMParsedXMLStruct *)pParsedXml->ppChild[i];

					/** state */
					if (pal_StringCompare((u_char *)pStruct->pElementName, (u_char*)"state") == 0)
					{
						if (pal_StringCompare((u_char *)pStruct->pElementValue, (u_char*)"idle") == 0)
						{
							pComposing->eComposingState = EcrioCPMComposingType_Idle;
						}
						else if (pal_StringCompare((u_char *)pStruct->pElementValue, (u_char*)"active") == 0)
						{
							pComposing->eComposingState = EcrioCPMComposingType_Active;
						}
						else
						{
							pComposing->eComposingState = EcrioCPMComposingType_None;
						}
					}
					/** refresh */
					else if (pal_StringCompare((u_char *)pStruct->pElementName, (u_char*)"refresh") == 0)
					{
						pComposing->uRefresh = pal_StringToNum((const u_char*)pStruct->pElementValue, NULL);
					}
					/** contenttype */
					else if (pal_StringCompare((u_char *)pStruct->pElementName, (u_char*)"contenttype") == 0)
					{
						pComposing->pContent = pal_StringCreate(pStruct->pElementValue, pal_StringLength(pStruct->pElementValue));
					}
					/** lastactive */
					else if (pal_StringCompare((u_char *)pStruct->pElementName, (u_char*)"lastactive") == 0)
					{
						pComposing->pLastActive = pal_StringCreate(pStruct->pElementValue, pal_StringLength(pStruct->pElementValue));
					}
				}
			}

			pal_MemoryAllocate(sizeof(EcrioCPMSessionMessageStruct), (void**)&pSessionMsgStruct);
			pal_MemorySet((void*)pSessionMsgStruct, 0, sizeof(EcrioCPMSessionMessageStruct));
			pal_MemoryAllocate(sizeof(EcrioCPMMessageStruct), (void**)&pSessionMsgStruct->pMessage);
			pal_MemorySet((void*)pSessionMsgStruct->pMessage, 0, sizeof(EcrioCPMMessageStruct));
			pSessionMsgStruct->pMessage->message.pComposing = pComposing;
			pSessionMsgStruct->pMessage->eContentType = EcrioCPMContentTypeEnum_Composing;
			pSessionMsgStruct->pSessionId = (char *)pal_StringCreate(pSessionId, pal_StringLength(pSessionId));
			uStrLen = (u_int32)(pal_StringLength(pMSRPText->pMessageId));
			pal_MemoryAllocate((uStrLen + 1), (void **)&(pSessionMsgStruct->pMessageId));
			pal_MemoryCopy(pSessionMsgStruct->pMessageId, pMSRPText->pMessageId, uStrLen);
			pSessionMsgStruct->pSenderURI = (char *)cpimMessage.pFromAddr;
			pCPMNotifStruct->eNotifCmd = EcrioCPM_Notif_IMComposing;
			pCPMNotifStruct->pNotifData = pSessionMsgStruct;
		}
		break;
#endif

		case CPIMContentType_PushLocation:
		{
			if (pIncSessionMsg != NULL)
			{
				CPMLOGI(pContext->hLogHandle, KLogTypeGeneral,
					"%s:%u\tMultiple bodies were detected for the same content type and will be discarded.",
					__FUNCTION__, __LINE__);
				break;
			}

			pal_MemoryAllocate(sizeof(EcrioCPMSessionMessageStruct), (void**)&pIncSessionMsg);
			if (pIncSessionMsg == NULL)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto Error_Level_01;
			}
			pIncSessionMsg->pSessionId = (char *)pal_StringCreate(pSessionId, pal_StringLength(pSessionId));
			pal_MemoryAllocate(sizeof(EcrioCPMMessageStruct), (void**)&pIncSessionMsg->pMessage);
			if (pIncSessionMsg->pMessage == NULL)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto Error_Level_01;
			}
			pIncSessionMsg->pMessage->imdnConfig = imdnConfig;
			pIncSessionMsg->pMessage->pIMDNMsgId = (char *)cpimMessage.pMsgId;
			pIncSessionMsg->pSenderURI = (char *)cpimMessage.pFromAddr;

			pal_MemoryAllocate(sizeof(EcrioCPMBufferStruct), (void**)&pIncSessionMsg->pMessage->message.pBuffer);
			if (pIncSessionMsg->pMessage->message.pBuffer == NULL)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto Error_Level_01;
			}
			pIncSessionMsg->pMessage->message.pBuffer->pMessage = pBuffStruct->pMsgBody;
			pIncSessionMsg->pMessage->message.pBuffer->uMessageLen = pBuffStruct->uMsgLen;
			pIncSessionMsg->pMessage->message.pBuffer->uByteRangeStart = 0;
			pIncSessionMsg->pMessage->message.pBuffer->uByteRangeEnd = 0;
			pIncSessionMsg->pMessage->message.pBuffer->pDate = cpimMessage.pDateTime;

			pIncSessionMsg->pMessage->eTrafficType = eTrafficType;

			pIncSessionMsg->pMessage->pBotSuggestion = NULL;
			pIncSessionMsg->pMessageId = (char *)pal_StringCreate(pMSRPText->pMessageId, pal_StringLength(pMSRPText->pMessageId));

			/* The same content type may be received for both normal MSRP message and file transfer message (CPIM Wrapped). */
			if ((pSession->contentTypes & EcrioCPMContentTypeEnum_FileTransferOverMSRP) == EcrioCPMContentTypeEnum_FileTransferOverMSRP)
			{
				pIncSessionMsg->pMessage->eContentType = EcrioCPMContentTypeEnum_FileTransferOverMSRP;
			}
			else 
			{
				pIncSessionMsg->pMessage->eContentType = EcrioCPMContentTypeEnum_PushLocation;
			}
						
			/** Notify the upper layer. */
			pCPMNotifStruct->eNotifCmd = EcrioCPM_Notif_IncomingSessionMessage;
			pCPMNotifStruct->pNotifData = pIncSessionMsg;
		}
		break;

		default:
			break;
		}
	}

	if (pBotSuggestion)
	{
		if ((pCPMNotifStruct->eNotifCmd == EcrioCPM_Notif_IncomingSessionMessage) ||
			(pCPMNotifStruct->eNotifCmd == EcrioCPM_Notif_RichCardReceived))
		{
			if (pIncSessionMsg->pMessage == NULL)
			{
				pal_MemoryAllocate(sizeof(EcrioCPMMessageStruct), (void**)&pIncSessionMsg->pMessage);
				if (pIncSessionMsg->pMessage == NULL)
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
						__FUNCTION__, __LINE__);
					uError = ECRIO_CPM_MEMORY_ERROR;
					goto Error_Level_01;
				}
			}

			pIncSessionMsg->pMessage->pBotSuggestion = pBotSuggestion;
		}

		/* This means we had received only bot suggestion request or response only */
		if (cpimMessage.uNumOfBuffers == 1)
		{
			pIncSessionMsg->pMessageId = (char *)pal_StringCreate(pMSRPText->pMessageId, pal_StringLength(pMSRPText->pMessageId));
			pIncSessionMsg->pSessionId = (char *)pal_StringCreate(pSessionId, pal_StringLength(pSessionId));
			pIncSessionMsg->pSenderURI = (char *)cpimMessage.pFromAddr;
			pIncSessionMsg->pMessage->eTrafficType = eTrafficType;
			pIncSessionMsg->pMessage->imdnConfig = imdnConfig;
			pIncSessionMsg->pMessage->pIMDNMsgId = (char *)cpimMessage.pMsgId;
		}
	}

Error_Level_01:
	if (pParsedXml != NULL)
	{
		ec_CPM_ReleaseParsedXmlStruct(&pParsedXml, Enum_TRUE);
	}
	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uError;
}
#endif

u_int32 ec_CPM_ReleaseCPMNotifStruct
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMNotifStruct* pCPMNotifStruct
)
{
	(void)pContext;

	switch (pCPMNotifStruct->eNotifCmd)
	{
	case EcrioCPM_Notif_IncomingSessionMessage:
	case EcrioCPM_Notif_RichCardReceived:
	{
		EcrioCPMSessionMessageStruct* pIncSessionMsg = (EcrioCPMSessionMessageStruct*)pCPMNotifStruct->pNotifData;

		pal_MemoryFree((void**)&pIncSessionMsg->pSessionId);
		pal_MemoryFree((void**)&pIncSessionMsg->pMessage->message.pBuffer);
		pal_MemoryFree((void**)&pIncSessionMsg->pMessage->pBotSuggestion);
		pal_MemoryFree((void**)&pIncSessionMsg->pMessage);
		pal_MemoryFree((void**)&pIncSessionMsg->pMessageId);
		pal_MemoryFree((void**)&pIncSessionMsg->pTransactionId);
		if (pIncSessionMsg->pDisplayName)
		{
			pal_MemoryFree((void**)&pIncSessionMsg->pDisplayName);
		}
		pal_MemoryFree((void**)&pIncSessionMsg);
	}
	break;
	case EcrioCPM_Notif_SessionMessageSendSuccess:
	case EcrioCPM_Notif_SessionMessageSendFailure:
	{
		EcrioCPMSessionIDStruct * pSessionId = (EcrioCPMSessionIDStruct*)pCPMNotifStruct->pNotifData;
		pal_MemoryFree((void**)&pSessionId->pSessionId);
		pal_MemoryFree((void**)&pSessionId->pMessageId);
		pal_MemoryFree((void**)&pSessionId->pTransactionId);
		pal_MemoryFree((void**)&pSessionId->pDescription);
	}
	break;
	case EcrioCPM_Notif_IMDNDisposition:
	{
		EcrioCPMIMDispoNotifStruct * pImdispNotif = (EcrioCPMIMDispoNotifStruct*)pCPMNotifStruct->pNotifData;
		if (pImdispNotif)
		{
			/*if (pImdispNotif->pIMDNMsgId)
			{
				pal_MemoryFree((void**)&pImdispNotif->pIMDNMsgId);
			}
			if (pImdispNotif->pDestURI)
			{
				pal_MemoryFree((void**)&pImdispNotif->pDestURI);
			}
			if (pImdispNotif->pDate)
			{
				pal_MemoryFree((void**)&pImdispNotif->pDate);
			}*/
			if (pImdispNotif->pCPMIMDispoNotifBody) 
			{
				pal_MemoryFree((void**)&pImdispNotif->pCPMIMDispoNotifBody);
			}
			if (pImdispNotif->pDisplayName)
			{
				pal_MemoryFree((void**)&pImdispNotif->pDisplayName);
			}
			pal_MemoryFree((void**)&pImdispNotif);
		}
	}
	break;
	case EcrioCPM_Notif_IMComposing:
	{
		EcrioCPMComposingStruct *pComposing = NULL;
		EcrioCPMSessionMessageStruct* pIncSessionMsg = (EcrioCPMSessionMessageStruct*)pCPMNotifStruct->pNotifData;

		if (pIncSessionMsg->pMessage != NULL)
		{
			pComposing = pIncSessionMsg->pMessage->message.pComposing;
			if (pComposing != NULL)
			{
				if (pComposing->pDisplayName != NULL)
				{
					pal_MemoryFree((void**)&pComposing->pDisplayName);
				}
				pal_MemoryFree((void**)&pComposing->pContent);
				pal_MemoryFree((void**)&pComposing->pLastActive);
				pal_MemoryFree((void**)&pComposing->pDestURI);
			}
			pal_MemoryFree((void**)&pComposing);
			pal_MemoryFree((void**)&pIncSessionMsg->pMessage);
		}

		pal_MemoryFree((void**)&pIncSessionMsg->pSessionId);
		pal_MemoryFree((void**)&pIncSessionMsg->pMessageId);
		pal_MemoryFree((void**)&pIncSessionMsg);
	}
	break;
	case EcrioCPM_Notif_GroupChatIcon:
	{
		EcrioCPMGroupChatIconStruct* pIcon = (EcrioCPMGroupChatIconStruct*)pCPMNotifStruct->pNotifData;

		pal_MemoryFree((void**)&pIcon->pSessionId);
		pal_MemoryFree((void**)&pIcon->pContentType);
		pal_MemoryFree((void**)&pIcon->pData);
		pal_MemoryFree((void**)&pIcon->pFileInfo);
		pal_MemoryFree((void**)&pIcon->pSenderURI);
		pal_MemoryFree((void**)&pIcon);
	}
	break;
	default:
		break;
	}

	return 0;
}

#if 0
u_int32 ec_CPM_CreateMSRPSession
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMConnectionInfoStruct *pConn,
	EcrioCPMSessionStruct *pCPMSession,
	u_char** ppPath,
	BoolEnum bIsFileTransfer
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	u_int32 uMSRPError = ECRIO_MSRP_NO_ERROR;
	MSRPSessionConfigStruct msrpSessionConfig = { 0 };
	MSRPHostStruct msrphost = { 0 };
	u_char *pPath = NULL;

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	msrpSessionConfig.handle = pContext->hMSRPHandle;
	msrpSessionConfig.connType = pCPMSession->eLocalConnectionType;
	if (bIsFileTransfer == Enum_FALSE)
	{
		msrpSessionConfig.type = MSRPType_TextMessage;
	}
	else
	{
		msrpSessionConfig.type = MSRPType_FileTransfer;
	}
	msrpSessionConfig.pAppData = pCPMSession; //call-id shall be provided later

	msrpSessionConfig.localHost.bIPv6 = pConn->isIPv6;
	if (NULL == pal_StringNCopy((u_char *)msrpSessionConfig.localHost.ip, 64, pConn->pLocalIp, pal_StringLength(pConn->pLocalIp)))
	{
		uError = ECRIO_CPM_MEMORY_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
			__FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	msrpSessionConfig.localHost.port = pConn->uLocalPort;	

	msrpSessionConfig.uAllocationSize = ECRIO_CPM_MSRP_ALLOCATION_SIZE;
	pal_MemoryAllocate(msrpSessionConfig.uAllocationSize, (void **)&pCPMSession->pMSRPSessionInstance);
	if (NULL == pCPMSession->pMSRPSessionInstance)
	{
		uError = ECRIO_CPM_MEMORY_ERROR;

		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	msrpSessionConfig.pAllocated = pCPMSession->pMSRPSessionInstance;

	if (pContext->bIsRelayEnabled)
	{
		msrpSessionConfig.remoteHost.bIPv6 = pConn->isIPv6;
		if (NULL == pal_StringNCopy((u_char *)msrpSessionConfig.remoteHost.ip, 64, (const u_char *)pContext->pRelayServerIP,
			pal_StringLength((const u_char *)pContext->pRelayServerIP)))
		{
			uError = ECRIO_CPM_MEMORY_ERROR;
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
				__FUNCTION__, __LINE__);
			goto Error_Level_01;
		}
		msrpSessionConfig.remoteHost.port = (u_int16)pContext->uRelayServerPort;
	}

	msrpSessionConfig.bIsRelayEnabled = pContext->bIsRelayEnabled;

	/* Populate proxy route info */
	msrpSessionConfig.bIsProxyRouteEnabled = pContext->proxyParam.bIsProxyRouteEnabled;
	msrpSessionConfig.pProxyRouteAddress = pContext->proxyParam.pProxyRouteAddress;
	msrpSessionConfig.uProxyRoutePort = pContext->proxyParam.uProxyRoutePort;
	
	/* Set fingerprint information */

	pCPMSession->hMSRPSessionHandle = EcrioMSRPSessionCreate(&msrpSessionConfig, &uMSRPError);
	if (uMSRPError != ECRIO_MSRP_NO_ERROR || pCPMSession->hMSRPSessionHandle == NULL)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioMSRPSessionCreate() uError=%u",
			__FUNCTION__, __LINE__, uMSRPError);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}

	uMSRPError = EcrioMSRPGetLocalHost(pCPMSession->hMSRPSessionHandle, &msrphost);
	if (uMSRPError != ECRIO_MSRP_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioMSRPGetLocalHost() uError=%u",
			__FUNCTION__, __LINE__, uMSRPError);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}

	if (pConn->uLocalPort == 0)
		pConn->uLocalPort = msrphost.port;

	uError = ec_CPM_GenerateMSRPPath(pContext, &msrphost, &pPath);
	if (uError != ECRIO_SDP_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_GenerateMSRPPath() error=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	uMSRPError = EcrioMSRPSetLocalMSRPAddress(pCPMSession->hMSRPSessionHandle, (char *)pPath);
	if (uMSRPError != ECRIO_MSRP_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioMSRPSetLocalMSRPAddress() uError=%u",
			__FUNCTION__, __LINE__, uMSRPError);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}

	*ppPath = pPath;

Error_Level_01:
	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uError;
}

u_int32 ec_CPM_StartMSRPSession
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMSessionStruct *pSession
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	u_int32 uMSRPError = ECRIO_MSRP_NO_ERROR;
	MSRPHostStruct msrpRemoteHost = { 0 };
	MSRPStreamParamStruct msrpParam = { 0 };

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pSession->bIsStartMSRPSession == Enum_TRUE)
	{
		/** Skip the MSRP session start process since MSRP session has already exists */
		CPMLOGI(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMSRP session has already started.", __FUNCTION__, __LINE__);
		uMSRPError = ECRIO_MSRP_NO_ERROR;
		goto Error_Level_01;
	}

	/* Check if connection type is Client OR Server and set the remote details. 
	Note - Remote details is available only if SDP nego is complete.
	This info will be used by MSRP lib for verifying incoming MSRP packets */
	if (pSession->eLocalConnectionType == MSRPConnectionType_TCP_Client || 
		pSession->eLocalConnectionType == MSRPConnectionType_TLS_Client)
	{
		msrpRemoteHost.bIPv6 = pSession->remoteSDPInfo.isIPv6;
		if (NULL == pal_StringNCopy((u_char *)msrpRemoteHost.ip, 64, pSession->remoteSDPInfo.pRemoteIp,
			pal_StringLength(pSession->remoteSDPInfo.pRemoteIp)))
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
				__FUNCTION__, __LINE__);
			uError = ECRIO_CPM_MEMORY_ERROR;
			goto Error_Level_01;
		}
		msrpRemoteHost.port = pSession->remoteSDPInfo.uRemotePort;

		uMSRPError = EcrioMSRPSetRemoteHost(pSession->hMSRPSessionHandle, &msrpRemoteHost);
		if (uMSRPError != ECRIO_MSRP_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioMSRPSetRemoteHost() uError=%u",
				__FUNCTION__, __LINE__, uMSRPError);
			uError = ECRIO_CPM_INTERNAL_ERROR;
			goto Error_Level_01;
		}

		uMSRPError = EcrioMSRPSetRemoteMSRPAddress(pSession->hMSRPSessionHandle, (char *)pSession->remoteSDPInfo.pRemotePath);
		if (uMSRPError != ECRIO_MSRP_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioMSRPSetRemoteMSRPAddress() uError=%u",
				__FUNCTION__, __LINE__, uMSRPError);
			uError = ECRIO_CPM_INTERNAL_ERROR;
			goto Error_Level_01;
		}
	}

	if (pSession->eLocalConnectionType == MSRPConnectionType_TLS_Client)
	{
		/* Check the media transport protocol */
		if (pSession->remoteSDP.remoteStream[0].eProtocol != ECRIO_SDP_MEDIA_PROTOCOL_TLS_MSRP)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMedia protocol TCP/TLS/MSRP not found!",
				__FUNCTION__, __LINE__);
			uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
			goto Error_Level_01;
		}

		uMSRPError = EcrioMSRPSetFingerPrint(pSession->hMSRPSessionHandle,(u_char *)pSession->remoteSDP.remoteStream[0].u.msrp.pFingerPrint);
		if (uMSRPError != ECRIO_MSRP_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioMSRPSetRemoteMSRPAddress() uError=%u",
				__FUNCTION__, __LINE__, uMSRPError);
			uError = ECRIO_CPM_INTERNAL_ERROR;
			goto Error_Level_01;
		}

	}
	
	msrpParam.connType = pSession->eLocalConnectionType;	
	msrpParam.uBuffSize = ECRIO_CPM_MRSP_RECV_BUFFER_SIZE;
	pal_MemoryAllocate(msrpParam.uBuffSize, (void **)&pSession->pMSRPRecvBuffer);
	if (NULL == pSession->pMSRPRecvBuffer)
	{
		uError = ECRIO_CPM_MEMORY_ERROR;

		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	msrpParam.pRecvBuff = pSession->pMSRPRecvBuffer;

	uMSRPError = EcrioMSRPStreamSetParam(pSession->hMSRPSessionHandle, &msrpParam);
	if (uMSRPError != ECRIO_MSRP_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioMSRPStreamSetParam() uError=%u",
			__FUNCTION__, __LINE__, uMSRPError);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}

	if (pContext->bIsRelayEnabled == Enum_TRUE)
		EcrioMSRPSessionSendEmptyPacket(pSession->hMSRPSessionHandle, Enum_TRUE);
	else
		EcrioMSRPSessionSendEmptyPacket(pSession->hMSRPSessionHandle, Enum_TRUE);
	
	uMSRPError = EcrioMSRPSessionStart(pSession->hMSRPSessionHandle);
	if (uMSRPError != ECRIO_MSRP_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioMSRPSessionStart() uError=%u",
		__FUNCTION__, __LINE__, uMSRPError);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}

	pSession->bIsStartMSRPSession = Enum_TRUE;

Error_Level_01:
	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uError;
}

u_int32 ec_CPM_CreateSDPSession
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMSessionStruct *pCPMSession,
	EcrioSDPInformationStruct *pSDPInfo
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	u_int32 uSDPError = ECRIO_SDP_NO_ERROR;
	//EcrioSDPInformationStruct tInfo = { .eEvent = ECRIO_SDP_EVENT_NONE };
	

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Create SDP session */
	pal_MemoryAllocate(ECRIO_CPM_SDP_SESSION_INSTANCE_SIZE, (void **)&pCPMSession->pSDPSessionInstance);
	if (NULL == pCPMSession->pSDPSessionInstance)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
			__FUNCTION__, __LINE__, uError);
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto Error_Level_01;
	}

	pCPMSession->hSDPSessionHandle = EcrioSDPSessionCreate(pContext->hSDPHandle,
		ECRIO_CPM_SDP_SESSION_INSTANCE_SIZE,
		pCPMSession->pSDPSessionInstance,
		&uSDPError);
	if (uSDPError != ECRIO_SDP_NO_ERROR || pCPMSession->hSDPSessionHandle == NULL)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSDPSessionCreate() error=%u",
			__FUNCTION__, __LINE__, uSDPError);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}

	pal_MemoryAllocate(ECRIO_CPM_SDP_INFO_BUFFER_SIZE, (void **)&pCPMSession->pSDPInformationBuffer);
	if (NULL == pCPMSession->pSDPInformationBuffer)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() error.",
			__FUNCTION__, __LINE__);
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto Error_Level_01;
	}

	pal_MemoryAllocate(ECRIO_CPM_SDP_INFO_BUFFER_SIZE, (void **)&pCPMSession->pSDPWorkingBuffer);
	if (NULL == pCPMSession->pSDPWorkingBuffer)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() error.",
			__FUNCTION__, __LINE__);
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto Error_Level_01;
	}

	pal_MemoryAllocate(ECRIO_CPM_SDP_STRING_SIZE, (void **)&pCPMSession->pSDPStringBuffer);
	if (NULL == pCPMSession->pSDPStringBuffer)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() error.",
			__FUNCTION__, __LINE__);
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto Error_Level_01;
	}


	pSDPInfo->strings.pData = pCPMSession->pSDPInformationBuffer;
	pSDPInfo->strings.uContainerSize = ECRIO_CPM_SDP_INFO_BUFFER_SIZE;
	pSDPInfo->strings.uSize = 0;

Error_Level_01:
	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uError;
}
#endif

u_char* ec_FormCPIMFromHeaderValue
(
	EcrioCPMContextStruct *pContext,
	u_char* pFromAddr
)
{
	u_char pFromWithFeature[256] = { 0 };
	u_char* pCPIMBodyFromHdr = NULL;

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	if (pContext->pDeviceId != NULL && pFromAddr != NULL)
	{
		if (NULL == pal_StringNCopy(pFromWithFeature, 256, pFromAddr, pal_StringLength(pFromAddr)))
		{
			return NULL;
		}
		if (NULL == pal_StringNConcatenate(pFromWithFeature, 256 - pal_StringLength(pFromWithFeature), (const u_char *)"?Accept-Contact=+sip.instance%3D%22%3Curn:gsma:imei:",
			pal_StringLength((const u_char *)"?Accept-Contact=+sip.instance%3D%22%3Curn:gsma:imei:")))
		{
			return NULL;
		}
		if (NULL == pal_StringNConcatenate(pFromWithFeature, 256 - pal_StringLength(pFromWithFeature), pContext->pDeviceId, pal_StringLength(pContext->pDeviceId)))
		{
			return NULL;
		}
		if (NULL == pal_StringNConcatenate(pFromWithFeature, 256 - pal_StringLength(pFromWithFeature), (const u_char *)"%3E%22%3Brequire%3Bexplicit", 
			pal_StringLength((const u_char *)"%3E%22%3Brequire%3Bexplicit")))
		{
			return NULL;
		}
		pCPIMBodyFromHdr = pal_StringCreate(pFromWithFeature, pal_StringLength(pFromWithFeature));
	}
	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return pCPIMBodyFromHdr;
}

u_int16 ec_CPM_GenerateRandomPort()
{
	return (u_int16)(ECRIO_CPM_MSRP_BASE_PORT + (pal_UtilityRandomNumber() % (0xFFFF - ECRIO_CPM_MSRP_BASE_PORT)));
}

BoolEnum ec_CPM_FindXmlElementNameAndValue
(
	EcrioCPMParsedXMLStruct *pParsedXml,
	u_char *pElemName,
	u_char *pElemValue
)
{
	BoolEnum bRet = Enum_FALSE;
	u_int32 i = 0;
	EcrioCPMParsedXMLStruct *pStruct = NULL;

	do
	{
		if (pal_StringCompare((u_char *)pParsedXml->pElementName, pElemName) == 0)
		{
			if (pal_StringCompare((u_char *)pParsedXml->pElementValue, pElemValue) == 0)
			{
				return Enum_TRUE;
			}
		}

		if (pParsedXml->ppChild != NULL && pParsedXml->ppChild[i] != NULL)
		{
			pStruct = (EcrioCPMParsedXMLStruct *)pParsedXml->ppChild[i];
			bRet = ec_CPM_FindXmlElementNameAndValue(pStruct, pElemName, pElemValue);
			if (bRet == Enum_TRUE)
			{
				return bRet;
			}
		}

		i++;
	} while (i < pParsedXml->uNumOfChild);

	return bRet;
}

BoolEnum ec_CPM_FindXmlAttributeNameAndValue
(
	EcrioCPMParsedXMLStruct *pParsedXml,
	u_char *pAttrName,
	u_char *pAttrValue
)
{
	BoolEnum bRet = Enum_FALSE;
	u_int32 i = 0, j = 0;
	EcrioCPMParsedXMLStruct *pStruct = NULL;

	do
	{
		do
		{
			if ((i < pParsedXml->uNumOfAttr) && (pParsedXml->ppAttr[i]) && (pParsedXml->ppAttr[i]->pAttrName) &&
				(pal_StringCompare((u_char *)pParsedXml->ppAttr[i]->pAttrName, pAttrName) == 0))
			{
				if ((pParsedXml->ppAttr[i]->pAttrValue) && (pal_StringCompare((u_char *)pParsedXml->ppAttr[i]->pAttrValue, pAttrValue) == 0))
				{
					return Enum_TRUE;
				}
			}
			i++;
		} while (i < pParsedXml->uNumOfAttr);

		if (pParsedXml->ppChild != NULL && pParsedXml->ppChild[j] != NULL)
		{
			pStruct = (EcrioCPMParsedXMLStruct *)pParsedXml->ppChild[j];
			bRet = ec_CPM_FindXmlAttributeNameAndValue(pStruct, pAttrName, pAttrValue);
			if (bRet == Enum_TRUE)
			{
				return bRet;
			}
		}
		j++;
	} while (j < pParsedXml->uNumOfChild);
	return bRet;
}

u_char* ec_CPM_GetXmlAttributeValue
(
	EcrioCPMParsedXMLStruct *pParsedXml,
	u_char *pElemName
)
{
	u_int32 i = 0;
	EcrioCPMParsedXMLStruct *pStruct = NULL;

	do
	{
		if (pal_StringCompare((u_char *)pParsedXml->pElementName, pElemName) == 0)
		{
			return (u_char *)pParsedXml->pElementValue;
		}

		if (pParsedXml->ppChild != NULL && pParsedXml->ppChild[i] != NULL)
		{
			u_char *pElemValue = NULL;
			pStruct = (EcrioCPMParsedXMLStruct *)pParsedXml->ppChild[i];
			pElemValue = ec_CPM_GetXmlAttributeValue(pStruct, pElemName);
			if (pElemValue != NULL)
			{
				return pElemValue;
			}
		}

		i++;
	} while (i < pParsedXml->uNumOfChild);

	return NULL;
}

u_int32 ec_CPM_FillGroupChatInfo
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMGroupChatInfoStruct *pInfo,
	EcrioCPMParsedXMLStruct *pParsedXml
)
{
	u_int32 i = 0, j = 0, k = 0, l = 0, m = 0;
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	EcrioCPMParsedXMLStruct *pStruct = NULL, *pTmp1 = NULL, *pTmp2 = NULL, *pTmp3 = NULL;
	EcrioCPMParsedXMLAttrStruct *pAttr = NULL;
	EcrioCPMGroupChatUsersStruct *pUsers = NULL;
	BoolEnum bIsYourown = Enum_FALSE;

	if (pal_StringCompare((u_char *)pParsedXml->pElementName, (u_char*)"conference-info") == 0)
	{
		/** conference document state */
		for (i = 0; i < pParsedXml->uNumOfAttr; i++)
		{
			pAttr = pParsedXml->ppAttr[i];
			if (pal_StringCompare((u_char *)pAttr->pAttrName, (u_char*)"state") == 0)
			{
				if (pal_StringCompare((u_char *)pAttr->pAttrValue, (u_char*)"full") == 0)
				{
					pInfo->eState = EcrioCPMGroupChatConferenceState_Full;
				}
				else if (pal_StringCompare((u_char *)pAttr->pAttrValue, (u_char*)"partial") == 0)
				{
					pInfo->eState = EcrioCPMGroupChatConferenceState_Partial;
				}
				else
				{
					pInfo->eState = EcrioCPMGroupChatConferenceState_None;
				}
				break;
			}
		}

		for (i = 0; i < pParsedXml->uNumOfChild; i++)
		{
			pStruct = (EcrioCPMParsedXMLStruct *)pParsedXml->ppChild[i];

			/** Host Info */
			if (pal_StringCompare((u_char *)pStruct->pElementName, (u_char*)"host-info") == 0)
			{
				for (j = 0; j < pStruct->uNumOfChild; j++)
				{
					/** display-text */
					pTmp1 = (EcrioCPMParsedXMLStruct *)pStruct->ppChild[j];
					if (pal_StringCompare((u_char *)pTmp1->pElementName, (u_char*)"display-text") == 0)
					{
						pInfo->hostInfo.pDisplayText = (char *)pal_StringCreate((u_char *)pTmp1->pElementValue,
							pal_StringLength((u_char *)pTmp1->pElementValue));
					}
					/** web-page */
					if (pal_StringCompare((u_char *)pTmp1->pElementName, (u_char*)"web-page") == 0)
					{
						pInfo->hostInfo.pWebPage = (char *)pal_StringCreate((u_char *)pTmp1->pElementValue,
							pal_StringLength((u_char *)pTmp1->pElementValue));
					}
					/** uri */
					if (pal_StringCompare((u_char *)pTmp1->pElementName, (u_char*)"uris") == 0)
					{
						EcrioCPMParsedXMLStruct *pEntry, *pUri;
						for (u_int32 urisCount = 0; urisCount < pTmp1->uNumOfChild; urisCount++)
						{
							/** entry */
							pEntry = (EcrioCPMParsedXMLStruct *)pTmp1->ppChild[urisCount];
							if (pal_StringCompare((u_char *)pEntry->pElementName, (u_char*)"entry") == 0)
							{
								pUri = pEntry->ppChild[0];
								/** Allocate URI-list */
								if (pInfo->hostInfo.uNumOfUsers == 0)
								{
									pal_MemoryAllocate((u_int32)sizeof(char*), (void **)&pInfo->hostInfo.ppUri);
								}
								else
								{
									/* Check arithmetic overflow */
									if (pal_UtilityArithmeticOverflowDetected(pInfo->hostInfo.uNumOfUsers, 1) == Enum_TRUE)
									{
										uError = ECRIO_CPM_MEMORY_ERROR;
										goto Error_Level_01;
									}

									/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
									if (pal_UtilityDataOverflowDetected((pInfo->hostInfo.uNumOfUsers + 1), sizeof(char*)) == Enum_TRUE)
									{
										uError = ECRIO_CPM_MEMORY_ERROR;
										goto Error_Level_01;
									}
									pal_MemoryReallocate((u_int32)((pInfo->hostInfo.uNumOfUsers + 1) * sizeof(char*)), (void **)&pInfo->hostInfo.ppUri);
								}
								if (NULL == pInfo->hostInfo.ppUri)
								{
									uError = ECRIO_CPM_INTERNAL_ERROR;
									CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
										__FUNCTION__, __LINE__, uError);
									goto Error_Level_01;
								}
								pInfo->hostInfo.ppUri[pInfo->hostInfo.uNumOfUsers] = (char *)pal_StringCreate((u_char *)pUri->pElementValue,
									pal_StringLength((u_char *)pUri->pElementValue));
								pInfo->hostInfo.uNumOfUsers++;
							}
						}
					}
				}
			}
			/** conference-description */
			else if (pal_StringCompare((u_char *)pStruct->pElementName, (u_char*)"conference-description") == 0)
			{
				for (j = 0; j < pStruct->uNumOfChild; j++)
				{
					/** subject */
					pTmp1 = (EcrioCPMParsedXMLStruct *)pStruct->ppChild[j];
					if (pal_StringCompare((u_char *)pTmp1->pElementName, (u_char*)"subject") == 0)
					{
						pInfo->desc.pSubject = (char *)pal_StringCreate((u_char *)pTmp1->pElementValue,
							pal_StringLength((u_char *)pTmp1->pElementValue));
					}
					/** subject-ext */
					else if (pal_StringCompare((u_char *)pTmp1->pElementName, (u_char*)"subject-ext") == 0)
					{
						for (k = 0; k < pTmp1->uNumOfChild; k++)
						{
							/** participant */
							pTmp2 = (EcrioCPMParsedXMLStruct *)pTmp1->ppChild[k];
							if (pal_StringCompare((u_char *)pTmp2->pElementName, (u_char*)"participant") == 0)
							{
								pInfo->desc.pSubCngUser = (char *)pal_StringCreate((u_char *)pTmp2->pElementValue,
									pal_StringLength((u_char *)pTmp2->pElementValue));
							}
							/** timestamp */
							else if (pal_StringCompare((u_char *)pTmp2->pElementName, (u_char*)"timestamp") == 0)
							{
								pInfo->desc.pSubCngDate = (char *)pal_StringCreate((u_char *)pTmp2->pElementValue,
									pal_StringLength((u_char *)pTmp2->pElementValue));
							}
						}
					}
					/** icon */
					else if (pal_StringCompare((u_char *)pTmp1->pElementName, (u_char*)"icon") == 0)
					{
						for (k = 0; k < pTmp1->uNumOfChild; k++)
						{
							/** participant */
							pTmp2 = (EcrioCPMParsedXMLStruct *)pTmp1->ppChild[k];
							if (pal_StringCompare((u_char *)pTmp2->pElementName, (u_char*)"participant") == 0)
							{
								pInfo->desc.pIconCngUser = (char *)pal_StringCreate((u_char *)pTmp2->pElementValue,
									pal_StringLength((u_char *)pTmp2->pElementValue));
							}
							/** timestamp */
							else if (pal_StringCompare((u_char *)pTmp2->pElementName, (u_char*)"timestamp") == 0)
							{
								pInfo->desc.pIconCngDate = (char *)pal_StringCreate((u_char *)pTmp2->pElementValue,
									pal_StringLength((u_char *)pTmp2->pElementValue));
							}
							/** source */
							else if (pal_StringCompare((u_char *)pTmp2->pElementName, (u_char*)"source") == 0)
							{
								for (l = 0; l < pTmp2->uNumOfChild; l++)
								{
									pTmp3 = (EcrioCPMParsedXMLStruct *)pTmp2->ppChild[l];

									/** icon-uri */
									if (pal_StringCompare((u_char *)pTmp3->pElementName, (u_char*)"icon-uri") == 0)
									{
										pInfo->desc.pIconUri = (char *)pal_StringCreate((u_char *)pTmp3->pElementValue,
											pal_StringLength((u_char *)pTmp3->pElementValue));
									}
									/** file-info */
									else if (pal_StringCompare((u_char *)pTmp3->pElementName, (u_char*)"file-info") == 0)
									{
										pInfo->desc.pIconInfo = (char *)pal_StringCreate((u_char *)pTmp3->pElementValue,
											pal_StringLength((u_char *)pTmp3->pElementValue));
									}
								}
							}
						}
					}
				}
			}
			/** users */
			else if (pal_StringCompare((u_char *)pStruct->pElementName, (u_char*)"users") == 0)
			{
				for (j = 0; j < pStruct->uNumOfChild; j++)
				{
					/** Note: we assume users element has only one child 'user' */
					pTmp1 = (EcrioCPMParsedXMLStruct *)pStruct->ppChild[j];
					if (pal_StringCompare((u_char *)pTmp1->pElementName, (u_char*)"user") != 0)
					{
						uError = ECRIO_CPM_INTERNAL_ERROR;
						CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tInvalid element",
							__FUNCTION__, __LINE__);
						goto Error_Level_01;
					}

					/** Populate participant uri info from <user entity> instead of <endpoint entity> */
					/** Alloc user */
					pal_MemoryAllocate((u_int32)sizeof(EcrioCPMGroupChatUsersStruct), (void **)&pUsers);
					if (NULL == pUsers)
					{
						uError = ECRIO_CPM_INTERNAL_ERROR;
						CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
							__FUNCTION__, __LINE__, uError);
						goto Error_Level_01;
					}

					if (pInfo->uNumOfUsers == 0)
					{
						pal_MemoryAllocate((u_int32)sizeof(EcrioCPMGroupChatUsersStruct*), (void **)&pInfo->ppUsers);
					}
					else
					{
						/* Check arithmetic overflow */
						if (pal_UtilityArithmeticOverflowDetected(pInfo->uNumOfUsers, 1) == Enum_TRUE)
						{
							uError = ECRIO_CPM_MEMORY_ERROR;
							goto Error_Level_01;
						}

						/** Check for memory overflow - exclude if memory to be allocated exceeds u_int32 */
						if (pal_UtilityDataOverflowDetected(pInfo->uNumOfUsers+1, sizeof(EcrioCPMGroupChatUsersStruct*)) == Enum_TRUE)
						{
							uError = ECRIO_CPM_INTERNAL_ERROR;
							goto Error_Level_01;
						}
						pal_MemoryReallocate((u_int32)((pInfo->uNumOfUsers + 1) * sizeof(EcrioCPMGroupChatUsersStruct*)), (void **)&pInfo->ppUsers);
					}
					if (NULL == pInfo->ppUsers)
					{
						uError = ECRIO_CPM_INTERNAL_ERROR;
						CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
							__FUNCTION__, __LINE__, uError);
						goto Error_Level_01;
					}
					pInfo->ppUsers[pInfo->uNumOfUsers] = pUsers;
					pInfo->uNumOfUsers++;

					/** Fill URI */
					for (k = 0; k < pTmp1->uNumOfAttr; k++)
					{
						pAttr = pTmp1->ppAttr[k];
						if (pal_StringCompare((u_char *)pAttr->pAttrName, (u_char*)"entity") == 0)
						{
							/** Extract user part from URI */
							u_int32 uLength = 0;
							u_int32 uStartCur = 0;
							u_char *pCur = NULL;

							if (pal_StringNCompare((u_char *)pAttr->pAttrValue, (u_char*)"sip:", 4) == 0)
							{
								uStartCur = 4;
								pCur = pal_SubString(pAttr->pAttrValue, (u_char *)"@");
								if (pCur != NULL)
								{
									uLength = pal_StringLength((u_char *)(pAttr->pAttrValue + uStartCur)) - pal_StringLength(pCur);
								}
								else
								{
									uLength = pal_StringLength((u_char *)(pAttr->pAttrValue + uStartCur));
								}
							}
							else if (pal_StringNCompare((u_char *)pAttr->pAttrValue, (u_char*)"tel:", 4) == 0)
							{
								uStartCur = 4;
								uLength = pal_StringLength((u_char *)(pAttr->pAttrValue + uStartCur));
							}
							else
							{
								uStartCur = 0;
								uLength = pal_StringLength((u_char *)pAttr->pAttrValue);
							}

							pUsers->pUri = (char *)pal_StringCreate((u_char *)(pAttr->pAttrValue + uStartCur), uLength);
							break;
						}
					}

					for (k = 0; k < pTmp1->uNumOfChild; k++)
					{
						/** endpoint */
						pTmp2 = (EcrioCPMParsedXMLStruct *)pTmp1->ppChild[k];
						if (pal_StringCompare((u_char *)pTmp2->pElementName, (u_char*)"endpoint") == 0)
						{
							/** Fill users information */
							for (l = 0; l < pTmp2->uNumOfChild; l++)
							{
								pTmp3 = (EcrioCPMParsedXMLStruct *)pTmp2->ppChild[l];

								/** Fill display-text */
								if (pal_StringCompare((u_char *)pTmp3->pElementName, (u_char*)"display-text") == 0)
								{
									pUsers->pDisplayText = (char *)pal_StringCreate((u_char *)pTmp3->pElementValue,
										pal_StringLength((u_char *)pTmp3->pElementValue));
								}
								/** Fill status */
								else if (pal_StringCompare((u_char *)pTmp3->pElementName, (u_char*)"status") == 0)
								{
									if (pal_StringCompare((u_char *)pTmp3->pElementValue, (u_char*)"connected") == 0)
									{
										pUsers->status = EcrioCPMGroupChatUserStatus_Connected;
									}
									else if (pal_StringCompare((u_char *)pTmp3->pElementValue, (u_char*)"disconnected") == 0)
									{
										pUsers->status = EcrioCPMGroupChatUserStatus_Disconnected;
									}
									else if (pal_StringCompare((u_char *)pTmp3->pElementValue, (u_char*)"pending") == 0)
									{
										pUsers->status = EcrioCPMGroupChatUserStatus_Pending;
									}
									else
									{
										pUsers->status = EcrioCPMGroupChatUserStatus_None;
									}
								}
								/** Fill disconnection-method */
								else if (pal_StringCompare((u_char *)pTmp3->pElementName, (u_char*)"disconnection-method") == 0)
								{
									if (pal_StringCompare((u_char *)pTmp3->pElementValue, (u_char*)"departed") == 0)
									{
										pUsers->disMethod = EcrioCPMGroupChatDisconnMethod_Departed;
									}
									else if (pal_StringCompare((u_char *)pTmp3->pElementValue, (u_char*)"booted") == 0)
									{
										pUsers->disMethod = EcrioCPMGroupChatDisconnMethod_Booted;
									}
									else if (pal_StringCompare((u_char *)pTmp3->pElementValue, (u_char*)"failed") == 0)
									{
										pUsers->disMethod = EcrioCPMGroupChatDisconnMethod_Failed;
									}
									else
									{
										pUsers->disMethod = EcrioCPMGroupChatDisconnMethod_None;
									}
								}
							}
						}
						else if (pal_StringCompare((u_char *)pTmp2->pElementName, (u_char*)"roles") == 0)
						{
							for (m = 0; m < pTmp2->uNumOfChild; m++)
							{
								pTmp3 = (EcrioCPMParsedXMLStruct *)pTmp2->ppChild[m];
								if (pal_StringICompare((u_char *)pTmp3->pElementValue, (u_char*)"administrator") == 0)
								{
									pUsers->isHost = Enum_TRUE;
								}
								else
								{
									pUsers->isHost = Enum_FALSE;
								}
							}
						}
					}
				}
			}
		}
	}

Error_Level_01:
	return uError;
}

u_int32 ec_CPM_FillResourceLists
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMParticipantListStruct *pList,
	EcrioCPMParsedXMLStruct *pParsedXml
)
{
	u_int32 i = 0, j = 0, k = 0;
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	EcrioCPMParsedXMLStruct *pStruct = NULL, *pTmp1 = NULL;
	EcrioCPMParsedXMLAttrStruct *pAttr = NULL;
	char *pUri = NULL;

	/** resource-lists */
	if (pal_StringCompare((u_char *)pParsedXml->pElementName, (u_char*)"resource-lists") == 0)
	{
		for (i = 0; i < pParsedXml->uNumOfChild; i++)
		{
			pStruct = (EcrioCPMParsedXMLStruct *)pParsedXml->ppChild[i];

			/** list */
			if (pal_StringCompare((u_char *)pStruct->pElementName, (u_char*)"list") == 0)
			{
				for (j = 0; j < pStruct->uNumOfChild; j++)
				{
					pTmp1 = (EcrioCPMParsedXMLStruct *)pStruct->ppChild[j];

					/** entry */
					if (pal_StringCompare((u_char *)pTmp1->pElementName, (u_char*)"entry") == 0)
					{
						/** Allocate URI-list */
						if (pList->uNumOfUsers == 0)
						{
							pal_MemoryAllocate((u_int32)sizeof(char*), (void **)&pList->ppUri);
						}
						else
						{
							/* Check arithmetic overflow */
							if (pal_UtilityArithmeticOverflowDetected(pList->uNumOfUsers, 1) == Enum_TRUE)
							{
								uError = ECRIO_CPM_MEMORY_ERROR;
								goto Error_Level_01;
							}

							/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
							if (pal_UtilityDataOverflowDetected((pList->uNumOfUsers + 1), sizeof(char*)) == Enum_TRUE)
							{
								uError = ECRIO_CPM_MEMORY_ERROR;
								goto Error_Level_01;
							}
							pal_MemoryReallocate((u_int32)((pList->uNumOfUsers + 1) * sizeof(char*)), (void **)&pList->ppUri);
						}
						if (NULL == pList->ppUri)
						{
							uError = ECRIO_CPM_INTERNAL_ERROR;
							CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
								__FUNCTION__, __LINE__, uError);
							goto Error_Level_01;
						}

						/** Fill URI */
						for (k = 0; k < pTmp1->uNumOfAttr; k++)
						{
							pAttr = pTmp1->ppAttr[k];
							if (pal_StringCompare((u_char *)pAttr->pAttrName, (u_char*)"uri") == 0)
							{
								/** Extract user part from URI */
								u_int32 uLength = 0;
								u_int32 uStartCur = 0;
								u_char *pCur = NULL;

								if (pal_StringNCompare((u_char *)pAttr->pAttrValue, (u_char*)"sip:", 4) == 0)
								{
									uStartCur = 4;
									pCur = pal_SubString(pAttr->pAttrValue, (u_char *)"@");
									if (pCur != NULL)
									{
										uLength = pal_StringLength((u_char *)(pAttr->pAttrValue + uStartCur)) - pal_StringLength(pCur);
									}
									else
									{
										uLength = pal_StringLength((u_char *)(pAttr->pAttrValue + uStartCur));
									}
								}
								else if (pal_StringNCompare((u_char *)pAttr->pAttrValue, (u_char*)"tel:", 4) == 0)
								{
									uStartCur = 4;
									uLength = pal_StringLength((u_char *)(pAttr->pAttrValue + uStartCur));
								}

								pUri = (char *)pal_StringCreate((u_char *)(pAttr->pAttrValue + uStartCur), uLength);
								break;
							}
						}

						pList->ppUri[pList->uNumOfUsers] = pUri;
						pList->uNumOfUsers++;
					}
				}
			}
		}
	}

Error_Level_01:
	return uError;
}

BoolEnum ec_CPM_StringIsQuoted
(
	u_char *pIn_String
)
{
	u_char *Tst_String;
	u_int32 strlen = 0;

	if (pIn_String == NULL)
	{
		return Enum_FALSE;
	}

	Tst_String = pIn_String;
	strlen = pal_StringLength(Tst_String);
	if ((*Tst_String == '"') && (Tst_String[strlen - 1] == '"'))
	{
		return Enum_TRUE;
	}
	else
	{
		return Enum_FALSE;
	}
}

u_char *ec_CPM_StringUnquote
(
	u_char *pIn_String
)
{
	BoolEnum Result = Enum_TRUE;
	u_int32 inLength = 0;
	u_int32 outLength = 0;
	u_int32 i = 0;
	u_char *pTempStr = NULL;
	u_char *pOut_string = NULL;

	if (pIn_String == NULL)
	{
		return NULL;
	}

	inLength = (u_int32)pal_StringLength(pIn_String);
	if (!inLength)
	{
		return NULL;
	}

	Result = ec_CPM_StringIsQuoted(pIn_String);

	if (Result == Enum_FALSE)
	{
		/* String is already Unquoted, copy input to output */
		pal_MemoryAllocate(inLength + 1, (void **)&pOut_string);
		if (pOut_string == NULL)
		{
			return NULL;
		}

		if (NULL == pal_StringNCopy(pOut_string, inLength + 1, pIn_String, pal_StringLength(pIn_String)))
		{
			return NULL;
		}

		return pOut_string;
	}

	if (inLength < 2)
	{
		return NULL;
	}

	outLength = inLength - (u_int32)2;	// Two less since removing '"'
	if (!outLength)
	{
		/* String is '""' */
		return NULL;
	}

	pal_MemoryAllocate(outLength + 1, (void **)&pOut_string);
	if (pOut_string == NULL)
	{
		return NULL;
	}

	pTempStr = pOut_string;

	for (i = 0; i < inLength; i++)
	{
		if (pIn_String[i] != '"')
		{
			*pTempStr++ = pIn_String[i];
		}
	}

	*pTempStr = '\0';

	return pOut_string;
}
