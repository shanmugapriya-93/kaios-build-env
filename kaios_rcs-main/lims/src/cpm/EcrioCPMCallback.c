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
* @file EcrioCPMCallback.c
* @brief Implementation of the Ecrio CPM Call back implementation functions.
*/

#include "EcrioCPM.h"
#include "EcrioCPMInternal.h"
#include "EcrioSigMgrSubscribe.h"
#include "EcriogZip.h"

void CPMSigMgrInfoCallback
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

void CPMSigMgrStatusCallback
(
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct,
	void *pCallbackData
)
{
	EcrioCPMContextStruct *pContext = NULL;

	if ((pCallbackData == NULL) || (pSigMgrMessageStruct == NULL))
	{
		return;
	}

	pContext = (EcrioCPMContextStruct *)pCallbackData;

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	switch (pSigMgrMessageStruct->msgCmd)
	{
		case EcrioSigMgrInstantMessageResponse:
		{
			EcrioCPMStandAloneMessageIDStruct cmpMsgIdStatus = { 0 };
			EcrioSigMgrSipMessageStruct* pMessage = NULL;
			EcrioCPMNotifStruct cpmNotifStruct = { 0 };
			EcrioSigMgrHeaderStruct *pWarningeHeader = NULL;

			CPMLOGW(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u",
				__FUNCTION__, __LINE__);

			pMessage = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;
			/* call the app callback */

			if (pMessage->statusCode == ECRIO_SIGMGR_RESPONSE_SUCCESS)
			{
				cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_PagerMessageSendSuccess;
			}
			else
			{
				cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_PagerMessageSendFailure;
			}

			EcrioSigMgrGetOptionalHeader(pContext->hEcrioSigMgrHandle, pMessage->pOptionalHeaderList,
				EcrioSipHeaderTypeWarning, &pWarningeHeader);
			if ((pWarningeHeader != NULL) && (pWarningeHeader->ppHeaderValues != NULL) && (pWarningeHeader->numHeaderValues == 3))
			{
				if ((pWarningeHeader->ppHeaderValues[0] != NULL) && (pWarningeHeader->ppHeaderValues[0]->pHeaderValue))
				{
					cmpMsgIdStatus.uWarningCode = pal_StringConvertToUNum(pWarningeHeader->ppHeaderValues[0]->pHeaderValue, NULL, 10);
				}
				if ((pWarningeHeader->ppHeaderValues[1] != NULL) && (pWarningeHeader->ppHeaderValues[1]->pHeaderValue))
				{
					cmpMsgIdStatus.pWarningHostName = (char*)pWarningeHeader->ppHeaderValues[1]->pHeaderValue;
				}
				if ((pWarningeHeader->ppHeaderValues[2] != NULL) && (pWarningeHeader->ppHeaderValues[2]->pHeaderValue))
				{
					cmpMsgIdStatus.pWarningText = (char*)pWarningeHeader->ppHeaderValues[2]->pHeaderValue;
				}
			}
			else
			{
				CPMLOGI(pContext->hLogHandle, KLogTypeGeneral, "%s:%uNo Warning header or invalid string",
					__FUNCTION__, __LINE__);
				if (pWarningeHeader != NULL)
				{
					if (pWarningeHeader->ppHeaderValues == NULL)
					{
						CPMLOGI(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tWarning header exist but ppHeaderValues is NULL",
							__FUNCTION__, __LINE__);
					}
					else
					{
						CPMLOGI(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tWarning header exist but number of header is %d",
							__FUNCTION__, __LINE__, pWarningeHeader->numHeaderValues);
					}
				}
			}

			if (pMessage->numPAssertedIdentities > 0)
			{
				cmpMsgIdStatus.uNumOfPAssertedIdentity = pMessage->numPAssertedIdentities;
				pal_MemoryAllocate(sizeof(u_char*) * cmpMsgIdStatus.uNumOfPAssertedIdentity, &cmpMsgIdStatus.ppPAssertedIdentity);
				if (cmpMsgIdStatus.ppPAssertedIdentity != NULL)
				{
					for (int i = 0; i < cmpMsgIdStatus.uNumOfPAssertedIdentity; i++)
					{
						if (pMessage->ppPAssertedIdentity[i]->addrSpec.uriScheme == EcrioSigMgrURISchemeSIP)
						{
							EcrioSigMgrFormSipURI(pContext->hEcrioSigMgrHandle,
								pMessage->ppPAssertedIdentity[i]->addrSpec.u.pSipUri, &cmpMsgIdStatus.ppPAssertedIdentity[i]);
						}
						else
						{
							EcrioSigMgrFormTelURI(pContext->hEcrioSigMgrHandle,
								pMessage->ppPAssertedIdentity[i]->addrSpec.u.pTelUri, &cmpMsgIdStatus.ppPAssertedIdentity[i]);
						}
					}
				}
			}

			cmpMsgIdStatus.pszReason = pMessage->pReasonPhrase;
			cmpMsgIdStatus.statusCode = pMessage->responseCode;
			cmpMsgIdStatus.pCallId = (char *)pMessage->pMandatoryHdrs->pCallId;
			cpmNotifStruct.pNotifData = (void*)&cmpMsgIdStatus;

			pContext->callbackStruct.pCPMNotificationCallbackFn((void *)pContext->callbackStruct.pCallbackData, 
				(void *)&cpmNotifStruct);
			cmpMsgIdStatus.pCallId = NULL;

			if (cmpMsgIdStatus.ppPAssertedIdentity)
			{
				for (int i = 0; i < cmpMsgIdStatus.uNumOfPAssertedIdentity; i++)
				{
					pal_MemoryFree((void**)&cmpMsgIdStatus.ppPAssertedIdentity[i]);
				}
				pal_MemoryFree((void**)&cmpMsgIdStatus.ppPAssertedIdentity);
			}
		}
		break;
		case EcrioSigMgrPublishResponse:
		{
			EcrioSigMgrSipMessageStruct* pMessage = NULL;
			EcrioCPMNotifStruct cpmNotifStruct = { 0 };

			CPMLOGW(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u",
				__FUNCTION__, __LINE__);

			pMessage = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;
			/* call the app callback */

			if (pMessage->responseCode == ECRIO_SIGMGR_RESPONSE_CODE_OK)
			{
				cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_PublishSendSuccess;
			}
			else
			{
				cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_PublishSendFailure;
			}

			pContext->callbackStruct.pCPMNotificationCallbackFn((void *)pContext->callbackStruct.pCallbackData,
				(void *)&cpmNotifStruct);		
		}
		break;
		case EcrioSigMgrInstantMessageRequestNotification:
		{
			ec_CPM_HandleInstantMessageRequest(pContext, pSigMgrMessageStruct);
		}
		break;

		default:
		{
			CPMLOGW(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tUnhandled command %u",
				__FUNCTION__, __LINE__, pSigMgrMessageStruct->msgCmd);
		}
		break;
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
}

void ec_CPM_HandlePrackRequest
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
)
{
	EcrioSigMgrSipMessageStruct	*pPrackReq = NULL;

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pPrackReq = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;

	EcrioSigMgrSendPrackResponse(pContext->hEcrioSigMgrHandle, pPrackReq, ECRIO_SIGMGR_RESPONSE_OK, ECRIO_SIGMGR_RESPONSE_CODE_OK);

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
}

/** Internal function called when an incoming BYE Request Notification is received */
void ec_CPM_HandleSessionEndRequest
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
)
{
	EcrioSigMgrSipMessageStruct	*pSessionEndNotifStruct = NULL;
	EcrioCPMSessionStruct *pSession = NULL;
	EcrioCPMNotifStruct cpmNotifStruct = { .eNotifCmd = EcrioCPM_Notif_None };
	EcrioCPMReasonStruct reasonStruct = { 0 };

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Find the current CPM session structure */

	pSessionEndNotifStruct = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;

	EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, pSessionEndNotifStruct->pMandatoryHdrs->pCallId, (void**)&pSession);
	if (pSession == NULL)
	{
		/** Print error and return */
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\t No matching session found for incoming BYE request",
			__FUNCTION__, __LINE__);

		return;
	}

	if (pContext->bIsRelayEnabled == Enum_TRUE)
		reasonStruct.pSessionId = (char *)pSession->relayData.pAppCallId;
	else
		reasonStruct.pSessionId = (char *)pSession->pCallId;

	/* Response code for handling BYE request is optional. for completeness
	 * sake, it is filled with 200 to mark it as successful BYE transaction */
	reasonStruct.uResponseCode = 200;
	reasonStruct.uReasonCause = pSessionEndNotifStruct->causeCode;
	reasonStruct.pReasonText = pSessionEndNotifStruct->pReasonPhrase;

	if (pSession->eTerminationReason == EcrioSessionTerminationReason_MSRPSocketFailure)
	{
		reasonStruct.uReasonCause = ECRIO_CPM_SESSION_TERMINATION_REASON_1001;
		reasonStruct.pReasonText = ECRIO_CPM_SESSION_TERMINATION_REASON_TEXT_1001;
	}

	/** Notify the upper layer. */
	cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_SessionEnded;
	cpmNotifStruct.pNotifData = &reasonStruct;

	pContext->callbackStruct.pCPMNotificationCallbackFn(pContext->callbackStruct.pCallbackData,
		&cpmNotifStruct);

	ec_CPM_MapDeleteKeyData(pContext->hHashMap, (u_char *)reasonStruct.pSessionId);

//	ec_CPM_ReleaseCPMSession(pContext, pSession);

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
}

#if 0
/** Internal function called when an incoming Invite request is received */
void ec_CPM_HandleInviteMessageRequest
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	u_int32 uSigMgrError = ECRIO_SIGMGR_NO_ERROR;
	u_int32	uSDPError = ECRIO_SDP_NO_ERROR;
	u_int32 uCPIMError = ECRIO_CPIM_NO_ERROR;
	u_char *pFrom = NULL;
	u_char *pGroupSessionId = NULL;
	EcrioSigMgrSipMessageStruct	*pInviteNotifStruct = NULL;
	EcrioSigMgrUnknownMessageBodyStruct *pUnknownBody = NULL;
	EcrioSDPStringStruct tSDP = {0};
	EcrioCPMSessionStruct *pSession = NULL;
	EcrioCPMNotifStruct cpmNotifStruct = { .eNotifCmd = EcrioCPM_Notif_None };
	EcrioCPMIncomingSessionStruct incomingStruct = { 0 };
	EcrioSDPMSRPMediaTypesEnum eAcceptWrappedTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_NONE;
	EcrioCPMConversationsIdStruct convIds = { 0 };
	EcrioCPMFileDescriptorStruct fileStruct = { 0 };
	EcrioSigMgrHeaderStruct *pPrivacyHeader = NULL;
	EcrioSigMgrHeaderStruct *pSubjectHeader = NULL;
	u_int32	i = 0, j = 0;
	char *pBoundaryStr = NULL;
	u_char *pBoundary = NULL;
	BoolEnum bIsMultipart = Enum_FALSE;
	BoolEnum bIsGroupChat = Enum_FALSE;
	BoolEnum bIsChatbot = Enum_FALSE, bTagChatbot = Enum_FALSE, bChatbotIARI = Enum_FALSE, bBotVer = Enum_FALSE;
	EcrioCPMMultipartMessageStruct *pMultiPart = NULL;
	EcrioCPMParsedXMLStruct *pParsedXml = NULL;
	EcrioCPMParticipantListStruct resource = { 0 };
	CPIMMessageStruct cpimMessage = { 0 };
	u_char *pSubject = NULL;
	EcrioCPMAliasingParamStruct *pAlias = NULL;
	u_char *pDisplayName = NULL;

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Find the current CPM session structure */

	pInviteNotifStruct = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;

	/** Get remote IP & port fropm SDP */
	if ((NULL == pInviteNotifStruct->pMessageBody) ||
		(NULL == pInviteNotifStruct->pMessageBody->pMessageBody))
	{
		uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
		goto Error_Level_01;
	}
	if (pInviteNotifStruct->pMandatoryHdrs == NULL)
	{
		pal_MemoryAllocate((u_int32)sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&pInviteNotifStruct->pMandatoryHdrs);
		if (pInviteNotifStruct->pMandatoryHdrs == NULL)
		{
			uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
			goto Error_Level_01;
		}
	}

	EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, pInviteNotifStruct->pMandatoryHdrs->pCallId, (void**)&pSession);
	if (pSession == NULL)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpSession not found for pCallId=%s",
			__FUNCTION__, __LINE__, pInviteNotifStruct->pMandatoryHdrs->pCallId);

		pal_MemoryAllocate((u_int32)sizeof(EcrioCPMSessionStruct), (void **)&pSession);
		if (pSession == NULL)
		{
			uError = ECRIO_CPM_MEMORY_ERROR;
			goto Error_Level_01;
		}
		pSession->eTerminationReason = EcrioSessionTerminationReason_None;
		pSession->bIsStartMSRPSession = Enum_FALSE;
		EcrioSigMgrSetAppData(pContext->hEcrioSigMgrHandle, pInviteNotifStruct->pMandatoryHdrs->pCallId, pSession);
	}
	if (EcrioCPMSessionState_Unused != pSession->state)
	{
		uError = ECRIO_CPM_BUSY_ERROR;

		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMaximum calls reached uError=%u", __FUNCTION__, __LINE__, uError);

		uError = ec_CPMSendInviteResponse(pContext, pInviteNotifStruct->pMandatoryHdrs->pCallId,
					ECRIO_SIGMGR_RESPONSE_BUSY_HERE_ESTABLISHING_ANOTHER_CALL, ECRIO_SIGMGR_RESPONSE_CODE_BUSY_HERE);
		if (uError != ECRIO_SIGMGR_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPMSendInviteResponse() failed with uError=%u",
				__FUNCTION__, __LINE__, uError);
			goto Error_Level_01;
		}
		
		goto Error_Level_01;
	}

#if 0 //Commenting out the 100 Trying as it is not expected by R&S
	/* Send 100 Trying before processing INVITE requst further */

	CPMLOGD(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tSending 100 Trying", __FUNCTION__, __LINE__);

	uError = ec_CPMSendInviteResponse(pContext, pInviteNotifStruct->pMandatoryHdrs->pCallId,
		ECRIO_SIGMGR_RESPONSE_TRYING, ECRIO_SIGMGR_RESPONSE_CODE_TRYING);
	if (uError != ECRIO_SIGMGR_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPMSendInviteResponse(100Trying) failed with uError=%u. Ignore and proceed further",
			__FUNCTION__, __LINE__, uError);
	}
#endif
	/*TODO: User-Agent matching*/


	pUnknownBody = (EcrioSigMgrUnknownMessageBodyStruct *)pInviteNotifStruct->pMessageBody->pMessageBody;
	if (NULL == pUnknownBody->pBuffer)
	{
		uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
		goto Error_Level_01;
	}

	/** Handle multipart message body */
	EcrioSigMgrHeaderValueStruct *pHdrValue = &pUnknownBody->contentType;
	if (pal_StringCompare(pHdrValue->pHeaderValue, (u_char *)"multipart/mixed") == 0)
	{
		bIsMultipart = Enum_TRUE;
		if (pHdrValue->ppParams && pHdrValue->ppParams[0])
		{
			for (i = 0; i < pHdrValue->numParams; i++)
			{
				if (pal_StringCompare(pHdrValue->ppParams[i]->pParamName, (u_char *)"boundary") == 0)
				{
					pBoundary = pHdrValue->ppParams[i]->pParamValue;
				}
			}
		}
	}

	if (bIsMultipart == Enum_TRUE && pBoundary != NULL)
	{
		pBoundaryStr = (char*)ec_CPM_StringUnquote(pBoundary);
		uError = ec_CPM_ParseMultipartMessage(pContext, &pMultiPart, (char*)pBoundaryStr,
			(char*)pUnknownBody->pBuffer, pUnknownBody->bufferLength);
		if (uError != ECRIO_CPM_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_ParseMultipartMessage() failed with uError=%u",
				__FUNCTION__, __LINE__, uError);
			goto Error_Level_01;
		}

		for (i = 0; i < pMultiPart->uNumOfBody; i++)
		{
			if (pal_StringCompare(pMultiPart->ppBody[i]->pContentType, (u_char *)"application/sdp") == 0)
			{
				tSDP.pData = pMultiPart->ppBody[i]->pMessageBody;
				tSDP.uContainerSize = ECRIO_CPM_SDP_STRING_SIZE;
				tSDP.uSize = pMultiPart->ppBody[i]->uContentLength;
			}
			else if (pal_StringCompare(pMultiPart->ppBody[i]->pContentType, (u_char *)"application/resource-lists+xml") == 0)
			{
				/** Parse message body for resource-list XML */
				uError = ec_CPM_ParseXmlDocument(pContext, &pParsedXml, (char*)pMultiPart->ppBody[i]->pMessageBody,
					pMultiPart->ppBody[i]->uContentLength);
				if (uError != ECRIO_CPM_NO_ERROR)
				{
					uError = ECRIO_CPM_INTERNAL_ERROR;
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_ParseXmlDocument() failed with uError=%u",
						__FUNCTION__, __LINE__, uError);
					goto Error_Level_01;
				}

				/** Fill resource-list XML */
				uError = ec_CPM_FillResourceLists(pContext, &resource, pParsedXml);
				if (uError != ECRIO_CPM_NO_ERROR)
				{
					uError = ECRIO_CPM_INTERNAL_ERROR;
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_FillResourceList() failed with uError=%u",
						__FUNCTION__, __LINE__, uError);
					goto Error_Level_01;
				}

				incomingStruct.pList = &resource;
			}
			else if (pal_StringCompare(pMultiPart->ppBody[i]->pContentType, (u_char *)"message/cpim") == 0)
			{
				uCPIMError = EcrioCPIMHandler(pContext->hCPIMHandle, &cpimMessage, pMultiPart->ppBody[i]->pMessageBody, pMultiPart->ppBody[i]->uContentLength);
				if (uCPIMError != ECRIO_CPIM_NO_ERROR)
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioCPIMHandler failed, uError=%u",
						__FUNCTION__, __LINE__, uCPIMError);
					uError = ECRIO_CPM_INTERNAL_ERROR;
					goto Error_Level_01;
				}
			}
		}
	}
	else
	{
		tSDP.pData = pUnknownBody->pBuffer;
		tSDP.uContainerSize = ECRIO_CPM_SDP_STRING_SIZE;
		tSDP.uSize = pUnknownBody->bufferLength;
	}

	pal_MemorySet((void*)&pSession->remoteSDP, 0, sizeof(EcrioSDPInformationStruct));
	/** Create SDP session */
	uError = ec_CPM_CreateSDPSession(pContext, pSession, &pSession->remoteSDP);
	if (uError != ECRIO_CPM_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_CreateSDPSession() error=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	uSDPError = EcrioSDPSessionHandleSDPString(pSession->hSDPSessionHandle, &pSession->remoteSDP, &tSDP);
	if (uSDPError != ECRIO_SDP_NO_ERROR)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSDPSessionHandleSDPString() uSDPError=%u",
			__FUNCTION__, __LINE__, uSDPError);

		goto Error_Level_02;
	}

	if ((pSession->remoteSDP.uNumOfRemoteMedia > 1) || (pSession->remoteSDP.remoteStream[0].eMediaType != ECRIO_SDP_MEDIA_TYPE_MSRP))
	{
		uError = ec_CPMSendInviteResponse(pContext, pInviteNotifStruct->pMandatoryHdrs->pCallId,
			ECRIO_SIGMGR_RESPONSE_UNSUPPORTED_MEDIA_TYPE, ECRIO_SIGMGR_RESPONSE_CODE_UNSUPPORTED_MEDIA_TYPE);
		if (uError != ECRIO_SIGMGR_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPMSendInviteResponse() failed with uError=%u",
				__FUNCTION__, __LINE__, uError);
			goto Error_Level_01;
		}

		goto Error_Level_02;
	}


	if (pSession->remoteSDP.remoteStream[0].eProtocol != ECRIO_SDP_MEDIA_PROTOCOL_TCP_MSRP &&
		pSession->remoteSDP.remoteStream[0].eProtocol != ECRIO_SDP_MEDIA_PROTOCOL_TLS_MSRP)
	{
		uError = ec_CPMSendInviteResponse(pContext, pInviteNotifStruct->pMandatoryHdrs->pCallId,
			ECRIO_SIGMGR_RESPONSE_NOT_ACCEPTABLE_HERE, ECRIO_SIGMGR_RESPONSE_CODE_NOT_ACCEPTABLE_HERE);
		if (uError != ECRIO_SIGMGR_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPMSendInviteResponse() failed with uError=%u",
				__FUNCTION__, __LINE__, uError);
			goto Error_Level_01;
		}
		
		goto Error_Level_02;
		
	}
	
	ec_CPM_PopulateRemoteMSRPInfo(pContext,pSession, pSession->remoteSDP);
	/* Check and overwrite the connection setup. If setup:actpass, then
	local setup:active i.e., local connection type is TCP client*/
	if (pSession->remoteSDP.remoteStream[0].u.msrp.eSetup == ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_ACTPASS || 
		pSession->remoteSDP.remoteStream[0].u.msrp.eSetup == ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_PASSIVE)
	{
		if (pContext->bMsrpOverTLS == Enum_TRUE)
		{
			pSession->eLocalConnectionType = MSRPConnectionType_TLS_Client;
		}
		else
		{
			pSession->eLocalConnectionType = MSRPConnectionType_TCP_Client;
		}
		
	}
	else if (pSession->remoteSDP.remoteStream[0].u.msrp.eSetup == ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_ACTIVE)
	{
		pSession->eLocalConnectionType = MSRPConnectionType_TCP_Server;
	}

	/** Check Contact header whether request is group chat / chatbot or not */
	if (pInviteNotifStruct->pContact)
	{
		for (i = 0; i < pInviteNotifStruct->pContact->numContactUris; i++)
		{
			if (pInviteNotifStruct->pContact->ppContactDetails[i])
			{
				for (j = 0; j < pInviteNotifStruct->pContact->ppContactDetails[i]->numParams; j++)
				{
					if (pInviteNotifStruct->pContact->ppContactDetails[i]->ppParams[j])
					{
						if (pal_StringCompare(pInviteNotifStruct->pContact->ppContactDetails[i]->ppParams[j]->pParamName, (u_char *)"isfocus") == 0)
						{
							bIsGroupChat = Enum_TRUE;
							if (pInviteNotifStruct->pContact->ppContactDetails[i]->nameAddr.addrSpec.uriScheme == EcrioSigMgrURISchemeSIP)
							{
								EcrioSigMgrSipURIStruct contactUri = {0};
								EcrioSigMgrIPAddrStruct ipAddr = {0};
								EcrioSigMgrNameAddrWithParamsStruct *pTempNameAddr = NULL;
								EcrioSigMgrCopyNameAddrWithParamStruct(pContext->hEcrioSigMgrHandle, pInviteNotifStruct->pContact->ppContactDetails[i], &pTempNameAddr);

								if (pTempNameAddr != NULL)
								{
									EcrioSigMgrFormSipURI(pContext->hEcrioSigMgrHandle, pTempNameAddr->nameAddr.addrSpec.u.pSipUri, &pGroupSessionId);
									EcrioSigMgrStructRelease(pContext->hEcrioSigMgrHandle, EcrioSigMgrStructType_NameAddrWithParams, (void **)&pTempNameAddr, Enum_TRUE);
								}
								else
								{
									CPMLOGD(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tFailed to populate the GroupID. Fallback on old logic",
										__FUNCTION__, __LINE__);
									contactUri.pUserId = pInviteNotifStruct->pContact->ppContactDetails[i]->nameAddr.addrSpec.u.pSipUri->pUserId;
									if (pInviteNotifStruct->pContact->ppContactDetails[i]->nameAddr.addrSpec.u.pSipUri->pIPAddr != NULL)
									{
										ipAddr.pIPAddr = pInviteNotifStruct->pContact->ppContactDetails[i]->nameAddr.addrSpec.u.pSipUri->pIPAddr->pIPAddr;
										ipAddr.port = pInviteNotifStruct->pContact->ppContactDetails[i]->nameAddr.addrSpec.u.pSipUri->pIPAddr->port;
									}
									else
									{
										ipAddr.pIPAddr = pInviteNotifStruct->pContact->ppContactDetails[i]->nameAddr.addrSpec.u.pSipUri->pDomain;
									}
									contactUri.pIPAddr = &ipAddr;
									EcrioSigMgrFormSipURI(pContext->hEcrioSigMgrHandle, &contactUri, &pGroupSessionId);
								}
								CPMLOGD(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tGroup Session ID: %s",
									__FUNCTION__, __LINE__, pGroupSessionId);
							}
							else
							{
								uSigMgrError = EcrioSigMgrGetOriginator(pContext->hEcrioSigMgrHandle, &pInviteNotifStruct->pContact->ppContactDetails[i]->nameAddr, &pGroupSessionId);
							}
							incomingStruct.pGroupSessionId = (char*)pGroupSessionId;
							if (pSession->pGroupSessionId)
								pal_MemoryFree((void **)&(pSession->pGroupSessionId));
							pSession->pGroupSessionId = pal_StringCreate((const u_char*)pGroupSessionId, pal_StringLength((const u_char*)pGroupSessionId));
							pSession->bIsGroupChat = Enum_TRUE;
							break;
						}
						else if (pal_StringCompare(pInviteNotifStruct->pContact->ppContactDetails[i]->ppParams[j]->pParamName, ECRIO_CPM_3GPP_FEATURE_ISBOT) == 0)
						{
							bTagChatbot = Enum_TRUE;
						}
						else if (pal_StringCompare(pInviteNotifStruct->pContact->ppContactDetails[i]->ppParams[j]->pParamName, (u_char*)"+g.3gpp.iari-ref") == 0)
						{
							if (pal_StringFindSubString(pInviteNotifStruct->pContact->ppContactDetails[i]->ppParams[j]->pParamValue, ECRIO_CPM_3GPP_FEATURE_CHATBOT) != NULL)
							{
								bChatbotIARI = Enum_TRUE;
							}
						}
						else if (pal_StringCompare(pInviteNotifStruct->pContact->ppContactDetails[i]->ppParams[j]->pParamName, (u_char*)"+g.gsma.rcs.botversion") == 0)
						{
							bBotVer = Enum_TRUE;
						}
					}
				}
			}
			if (bIsGroupChat == Enum_TRUE)
			{
				break;
			}
			else if (bTagChatbot == Enum_TRUE && bChatbotIARI == Enum_TRUE && bBotVer == Enum_TRUE)
			{
				bIsChatbot = Enum_TRUE;
				break;
			}
		}
	}

	/** Verify Privacy header */
	EcrioSigMgrGetOptionalHeader(pContext->hEcrioSigMgrHandle, pInviteNotifStruct->pOptionalHeaderList, EcrioSipHeaderTypePrivacy, &pPrivacyHeader);

	/** If Privacy header fields are indicated "id" or "header", then originating party ID is restricted. */
	if (pPrivacyHeader != NULL)
	{
		for (i = 0; i < pPrivacyHeader->numHeaderValues; i++)
		{
			if (pPrivacyHeader->ppHeaderValues && pPrivacyHeader->ppHeaderValues[i])
			{
				if ((pal_SubString(pPrivacyHeader->ppHeaderValues[i]->pHeaderValue, (u_char *)"id")) ||
					(pal_SubString(pPrivacyHeader->ppHeaderValues[i]->pHeaderValue, (u_char *)"header")))
				{
					pFrom = pal_StringCreate(ECRIO_CPM_CALLER_ID_RESTRICTED, pal_StringLength(ECRIO_CPM_CALLER_ID_RESTRICTED));
					pDisplayName = pal_StringCreate(ECRIO_CPM_CALLER_ID_RESTRICTED, pal_StringLength(ECRIO_CPM_CALLER_ID_RESTRICTED));
				}
			}
		}
	}
	if (pFrom == NULL)
	{
		/*Check if the incomming invite contains Referred-By header, if present, then add it */
		if (pInviteNotifStruct->ppReferredBy != NULL)
		{
			EcrioSigMgrGetOriginator(pContext->hEcrioSigMgrHandle, &pInviteNotifStruct->ppReferredBy[0]->nameAddr, &pFrom);
			if (pInviteNotifStruct->ppReferredBy[0]->nameAddr.pDisplayName != NULL)
			{
				pDisplayName = pal_StringCreate(pInviteNotifStruct->ppReferredBy[0]->nameAddr.pDisplayName,
					pal_StringLength(pInviteNotifStruct->ppReferredBy[0]->nameAddr.pDisplayName));
			}
		}
		/** OIP/OIR - Check P-Asserted-Identity header and flag of using it */
		if (pContext->bEnablePAI == Enum_TRUE)
		{
			if ((pInviteNotifStruct->ppPAssertedIdentity != NULL) && (pFrom == NULL))
			{
				EcrioSigMgrGetOriginator(pContext->hEcrioSigMgrHandle, pInviteNotifStruct->ppPAssertedIdentity[0], &pFrom);
				if ((pDisplayName == NULL) && (pInviteNotifStruct->ppPAssertedIdentity[0]->pDisplayName != NULL))
				{
					pDisplayName = pal_StringCreate(pInviteNotifStruct->ppPAssertedIdentity[0]->pDisplayName,
						pal_StringLength(pInviteNotifStruct->ppPAssertedIdentity[0]->pDisplayName));
				}
			}
		}
		/** If we don't support to see PAI header fields, then use From header field for originating party ID. */
		if (pFrom == NULL)
		{
			EcrioSigMgrGetOriginator(pContext->hEcrioSigMgrHandle, &pInviteNotifStruct->pMandatoryHdrs->pFrom->nameAddr, &pFrom);
		}
		if ((pDisplayName == NULL) && (pInviteNotifStruct->pMandatoryHdrs->pFrom->nameAddr.pDisplayName != NULL))
		{
			pDisplayName = pal_StringCreate(pInviteNotifStruct->pMandatoryHdrs->pFrom->nameAddr.pDisplayName,
				pal_StringLength(pInviteNotifStruct->pMandatoryHdrs->pFrom->nameAddr.pDisplayName));
		}
	}

	/** Check PAI header fields - if present, then store the tk param and aliason value */
	if (bIsChatbot == Enum_TRUE)
	{
		if (pInviteNotifStruct->ppPAssertedIdentity != NULL)
		{
			for (i = 0; i < pInviteNotifStruct->numPAssertedIdentities; i++)
			{
				uError = ec_CPM_GetAliasParamFromPAI(pContext, pInviteNotifStruct->ppPAssertedIdentity[i], &pAlias);
				if (pAlias != NULL)
				{
					break;
				}
			}
		}
	}

	if ((ECRIO_SIGMGR_NO_ERROR != uSigMgrError) || (NULL == pFrom))
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrGetOriginator() uSigMgrError=%u",
			__FUNCTION__, __LINE__, uSigMgrError);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_02;
	}

	if (pInviteNotifStruct->pConvId)
	{
		convIds.pConversationId = pInviteNotifStruct->pConvId->pConversationId;
		convIds.pContributionId = pInviteNotifStruct->pConvId->pContributionId;
		convIds.pInReplyToContId = pInviteNotifStruct->pConvId->pInReplyToContId;
		incomingStruct.pConvId = &convIds;

		/* Book keeping for future use */
		if (pSession->pConvId == NULL)
		{
			pal_MemoryAllocate(sizeof(EcrioCPMConversationsIdStruct), (void **)&pSession->pConvId);
			if (pSession->pConvId == NULL)
			{
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto Error_Level_02;
			}
		}
		else
		{
			pal_MemoryFree((void**)&pSession->pConvId->pContributionId);
			pal_MemoryFree((void**)&pSession->pConvId->pConversationId);
			pal_MemoryFree((void**)&pSession->pConvId->pInReplyToContId);
		}

		pSession->pConvId->pContributionId = pal_StringCreate((const u_char*)pInviteNotifStruct->pConvId->pContributionId, 
			pal_StringLength((const u_char*)pInviteNotifStruct->pConvId->pContributionId));
		pSession->pConvId->pConversationId = pal_StringCreate((const u_char*)pInviteNotifStruct->pConvId->pConversationId, 
			pal_StringLength((const u_char*)pInviteNotifStruct->pConvId->pConversationId));
		if (pInviteNotifStruct->pConvId->pInReplyToContId != NULL)
		{
			pSession->pConvId->pInReplyToContId = pal_StringCreate((const u_char*)pInviteNotifStruct->pConvId->pInReplyToContId,
				pal_StringLength((const u_char*)pInviteNotifStruct->pConvId->pInReplyToContId));
		}
	}

	if (pContext->bSendRingingResponse == Enum_TRUE)
	{
		/** Send 180 Ringing invite response -- is it better to send 183 Session Progress? */
		uError = ec_CPMSendInviteResponse(pContext, pInviteNotifStruct->pMandatoryHdrs->pCallId,
					ECRIO_SIGMGR_RESPONSE_RINGING, ECRIO_SIGMGR_RESPONSE_CODE_RINGING);
		if (uError != ECRIO_SIGMGR_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPMSendInviteResponse() failed with uError=%u",
				__FUNCTION__, __LINE__, uError);
			goto Error_Level_01;
		}
	}
	else
	{
		/** Send 100 Trying invite response -- to stop INVITE retransmission */
		uError = ec_CPMSendInviteResponse(pContext, pInviteNotifStruct->pMandatoryHdrs->pCallId,
			ECRIO_SIGMGR_RESPONSE_TRYING, ECRIO_SIGMGR_RESPONSE_CODE_TRYING);
		if (uError != ECRIO_SIGMGR_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPMSendInviteResponse() failed with uError=%u",
				__FUNCTION__, __LINE__, uError);
			goto Error_Level_01;
		}
	}

	pSession->bIsLargeMessage = ec_CPMIsLargeMessage(pContext, pInviteNotifStruct);
	pSession->state = EcrioCPMSessionState_Starting;
	pSession->pCallId = pal_StringCreate(pInviteNotifStruct->pMandatoryHdrs->pCallId, pal_StringLength(pInviteNotifStruct->pMandatoryHdrs->pCallId));
	if (pContext->bIsRelayEnabled == Enum_TRUE)
	{
		pSession->relayData.pAppCallId = pal_StringCreate(pInviteNotifStruct->pMandatoryHdrs->pCallId, pal_StringLength(pInviteNotifStruct->pMandatoryHdrs->pCallId));
		ec_CPM_MapSetData(pContext, pSession->relayData.pAppCallId, (void*)pSession->pCallId);
	}

	if (pDisplayName != NULL)
	{
		pSession->pPeerDisplayName = pal_StringCreate(pDisplayName, pal_StringLength(pDisplayName));
	}

	pInviteNotifStruct->pReasonPhrase = NULL;
	incomingStruct.pReqFrom = (char*)pFrom;
	incomingStruct.pSessionId = (char*)pSession->pCallId;
	incomingStruct.bIsGroupChat = bIsGroupChat;
	if (pSession->remoteSDP.remoteStream[0].u.msrp.bIsClosed == Enum_TRUE)
	{
		incomingStruct.bIsClosed = Enum_TRUE;
	}

	eAcceptWrappedTypes = pSession->remoteSDP.remoteStream[0].u.msrp.eAcceptWrappedTypes;
	if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_TEXT_PLAIN) == ECRIO_SDP_MSRP_MEDIA_TYPE_TEXT_PLAIN)
		incomingStruct.contentTypes |= EcrioCPMContentTypeEnum_Text;

	if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_IMDN_XML) == ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_IMDN_XML)
		incomingStruct.contentTypes |= EcrioCPMContentTypeEnum_IMDN;

	if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_IM_ISCOMPOSING_XML) == ECRIO_SDP_MSRP_MEDIA_TYPE_IM_ISCOMPOSING_XML)
		incomingStruct.contentTypes |= EcrioCPMContentTypeEnum_Composing;

	if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_HTTP_FILE_TRANSFER) == ECRIO_SDP_MSRP_MEDIA_TYPE_HTTP_FILE_TRANSFER)
		incomingStruct.contentTypes |= EcrioCPMContentTypeEnum_FileTransferOverHTTP;

	if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_RICHCARD) == ECRIO_SDP_MSRP_MEDIA_TYPE_RICHCARD)
		incomingStruct.contentTypes |= EcrioCPMContentTypeEnum_RichCard;

	if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTED_CHIPLIST) == ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTED_CHIPLIST)
		incomingStruct.contentTypes |= EcrioCPMContentTypeEnum_SuggestedChipList;

	if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTION_RESPONSE) == ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTION_RESPONSE)
		incomingStruct.contentTypes |= EcrioCPMContentTypeEnum_SuggestionResponse;

	if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_SHARED_CLIENT_DATA) == ECRIO_SDP_MSRP_MEDIA_TYPE_SHARED_CLIENT_DATA)
		incomingStruct.contentTypes |= EcrioCPMContentTypeEnum_SharedClientData;

	if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_WILDCARD) == ECRIO_SDP_MSRP_MEDIA_TYPE_WILDCARD)
		incomingStruct.contentTypes |= EcrioCPMContentTypeEnum_WildCard;

	if (bIsGroupChat == Enum_TRUE) {
		if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_GROUPDATA) == ECRIO_SDP_MSRP_MEDIA_TYPE_GROUPDATA)
			incomingStruct.contentTypes |= EcrioCPMContentTypeEnum_GroupData;
	}

	if (((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_WILDCARD) == ECRIO_SDP_MSRP_MEDIA_TYPE_WILDCARD &&
		pSession->remoteSDP.remoteStream[0].u.msrp.eAcceptTypes == ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_CPIM) ||
		((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_CONFERENCE) == ECRIO_SDP_MSRP_MEDIA_TYPE_CONFERENCE &&
		pSession->remoteSDP.remoteStream[0].u.msrp.eAcceptTypes == ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_CPIM))
	{
		CPMLOGD(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tWildcard data wrapped in CPIM",__FUNCTION__, __LINE__);
		if (pSession->remoteSDP.remoteStream[0].u.msrp.fileSelector.uSize != 0)
		{
			CPMLOGD(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tWildcard data is file", __FUNCTION__, __LINE__);
			incomingStruct.contentTypes = EcrioCPMContentTypeEnum_FileTransferOverMSRP;
			fileStruct.pFilePath = NULL;
			fileStruct.pFileName = NULL;
			fileStruct.pFileType = NULL;
			fileStruct.uFileSize = pSession->remoteSDP.remoteStream[0].u.msrp.fileSelector.uSize;
			fileStruct.pFileTransferId = NULL;
			fileStruct.pIMDNMsgId = NULL;
			fileStruct.imdnConfig = 0;

			incomingStruct.pFile = &fileStruct;

			/* Book keeping for future use */
			if (pSession->pFile == NULL)
			{
				pal_MemoryAllocate(sizeof(EcrioCPMFileDescriptorStruct), (void **)&pSession->pFile);
				if (pSession->pFile == NULL)
				{
					uError = ECRIO_CPM_MEMORY_ERROR;
					goto Error_Level_02;
				}
			}
			else
			{
				if (pSession->pFile->pFilePath != NULL)
				{
					pal_MemoryFree((void **)&(pSession->pFile->pFilePath));
				}

				if (pSession->pFile->pFileName != NULL)
				{
					pal_MemoryFree((void **)&(pSession->pFile->pFileName));
				}

				if (pSession->pFile->pFileType != NULL)
				{
					pal_MemoryFree((void **)&(pSession->pFile->pFileType));
				}

				if (pSession->pFile->pFileTransferId != NULL)
				{
					pal_MemoryFree((void **)&(pSession->pFile->pFileTransferId));
				}

				if (pSession->pFile->pIMDNMsgId != NULL)
				{
					pal_MemoryFree((void **)&(pSession->pFile->pIMDNMsgId));
				}
			}

			pSession->pFile->pFilePath = NULL;
			pSession->pFile->pFileName = NULL;
			pSession->pFile->pFileType = NULL;
			pSession->pFile->uFileSize = fileStruct.uFileSize;
			pSession->pFile->pFileTransferId = NULL;
			pSession->pFile->pIMDNMsgId = NULL;
			pSession->pFile->imdnConfig = 0;
		}
		else /* Its not FT over MSRP session */
		{
			incomingStruct.contentTypes = EcrioCPMContentTypeEnum_Text | EcrioCPMContentTypeEnum_IMDN |
				EcrioCPMContentTypeEnum_FileTransferOverHTTP | EcrioCPMContentTypeEnum_Composing;
		}
	}
	if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_PUSH_LOCATION) == ECRIO_SDP_MSRP_MEDIA_TYPE_PUSH_LOCATION)
		incomingStruct.contentTypes |= EcrioCPMContentTypeEnum_PushLocation;

	/** File Transfer */
	if (pSession->remoteSDP.remoteStream[0].u.msrp.eAcceptTypes == ECRIO_SDP_MSRP_MEDIA_TYPE_GENERAL)
	{
		u_int32 imdnConfig = EcrioCPMIMDispositionConfigNone;

		incomingStruct.contentTypes = EcrioCPMContentTypeEnum_FileTransferOverMSRP;

		fileStruct.pFileName = (char*)pSession->remoteSDP.remoteStream[0].u.msrp.fileSelector.pName;
		fileStruct.pFileType = (char*)pSession->remoteSDP.remoteStream[0].u.msrp.fileSelector.pType;
		fileStruct.uFileSize = pSession->remoteSDP.remoteStream[0].u.msrp.fileSelector.uSize;
		fileStruct.pFileTransferId = (char*)pSession->remoteSDP.remoteStream[0].u.msrp.pFileTransferId;
		fileStruct.pIMDNMsgId = (char*)cpimMessage.pMsgId;

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
		fileStruct.imdnConfig = imdnConfig;
		incomingStruct.pFile = &fileStruct;

		/* Book keeping for future use */
		if (pSession->pFile == NULL)
		{
			pal_MemoryAllocate(sizeof(EcrioCPMFileDescriptorStruct), (void **)&pSession->pFile);
			if (pSession->pFile == NULL)
			{
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto Error_Level_02;
			}
		}
		else
		{
			if (pSession->pFile->pFilePath != NULL)
			{
				pal_MemoryFree((void **)&(pSession->pFile->pFilePath));
			}

			if (pSession->pFile->pFileName != NULL)
			{
				pal_MemoryFree((void **)&(pSession->pFile->pFileName));
			}

			if (pSession->pFile->pFileType != NULL)
			{
				pal_MemoryFree((void **)&(pSession->pFile->pFileType));
			}

			if (pSession->pFile->pFileTransferId != NULL)
			{
				pal_MemoryFree((void **)&(pSession->pFile->pFileTransferId));
			}

			if (pSession->pFile->pIMDNMsgId != NULL)
			{
				pal_MemoryFree((void **)&(pSession->pFile->pIMDNMsgId));
			}
		}

		pSession->pFile->pFileName = NULL;
		pSession->pFile->pFileName = (char*)pal_StringCreate((const u_char*)fileStruct.pFileName, pal_StringLength((const u_char*)fileStruct.pFileName));
		pSession->pFile->pFileType = (char*)pal_StringCreate((const u_char*)fileStruct.pFileType, pal_StringLength((const u_char*)fileStruct.pFileType));
		pSession->pFile->uFileSize = fileStruct.uFileSize;
		pSession->pFile->pFileTransferId = (char*)pal_StringCreate((const u_char*)fileStruct.pFileTransferId, pal_StringLength((const u_char*)fileStruct.pFileTransferId));
		pSession->pFile->pIMDNMsgId = (char*)pal_StringCreate((const u_char*)fileStruct.pIMDNMsgId, pal_StringLength((const u_char*)fileStruct.pIMDNMsgId));
		pSession->pFile->imdnConfig = fileStruct.imdnConfig;
	}

	/* isComposing may not be wrapped content. Hence check under a=accept-types parameter */
	if ((pSession->remoteSDP.remoteStream[0].u.msrp.eAcceptTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_IM_ISCOMPOSING_XML) == ECRIO_SDP_MSRP_MEDIA_TYPE_IM_ISCOMPOSING_XML)
		incomingStruct.contentTypes |= EcrioCPMContentTypeEnum_Composing;

	/* Check if wild card is passed for a=accept-types and a=accept-wrapped-types. If so
	only include Text, IMDN, FT HTTP and Composing. */
	if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_WILDCARD) == ECRIO_SDP_MSRP_MEDIA_TYPE_WILDCARD &&
		pSession->remoteSDP.remoteStream[0].u.msrp.eAcceptTypes == ECRIO_SDP_MSRP_MEDIA_TYPE_WILDCARD)
	{
		incomingStruct.contentTypes = EcrioCPMContentTypeEnum_Text | EcrioCPMContentTypeEnum_IMDN |
			EcrioCPMContentTypeEnum_FileTransferOverHTTP | EcrioCPMContentTypeEnum_Composing;
	}
	else if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_WILDCARD) == ECRIO_SDP_MSRP_MEDIA_TYPE_WILDCARD &&
		(pSession->remoteSDP.remoteStream[0].u.msrp.eAcceptTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_CPIM) == ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_CPIM &&
		pSession->pFile == NULL)
	{
		incomingStruct.contentTypes = EcrioCPMContentTypeEnum_Text | EcrioCPMContentTypeEnum_IMDN |
			EcrioCPMContentTypeEnum_FileTransferOverHTTP | EcrioCPMContentTypeEnum_Composing;
	}
	/* Book keeping for future use */
	pSession->contentTypes = incomingStruct.contentTypes;

	if (bIsGroupChat == Enum_TRUE)
	{		
		EcrioSigMgrGetOptionalHeader(pContext->hEcrioSigMgrHandle, pInviteNotifStruct->pOptionalHeaderList, EcrioSipHeaderTypeSubject, &pSubjectHeader);
		if (pSubjectHeader != NULL)
		{
			pSubject = pal_StringCreate((const u_char*)pSubjectHeader->ppHeaderValues[0]->pHeaderValue,
				pal_StringLength((const u_char*)pSubjectHeader->ppHeaderValues[0]->pHeaderValue));
			incomingStruct.pGroupSubject = (char*)pSubject;
		}
		else
			incomingStruct.pGroupSubject = NULL;
	}

	incomingStruct.pAlias = pAlias;
	incomingStruct.bIsChatbot = bIsChatbot;
	incomingStruct.pDisplayName = (char*)pDisplayName;

	/** Notfiy upper layer */
	cpmNotifStruct.pNotifData = &incomingStruct;
	cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_IncomingSession;
	pContext->callbackStruct.pCPMNotificationCallbackFn(pContext->callbackStruct.pCallbackData,
		&cpmNotifStruct);

	pDisplayName = NULL;
	goto Error_Level_01;

Error_Level_02:

	ec_CPM_MapDeleteKeyData(pContext->hHashMap, (u_char*)pInviteNotifStruct->pMandatoryHdrs->pCallId);
	ec_CPM_ReleaseCPMSession(pContext, pSession);

Error_Level_01:

	if (NULL != pFrom)
	{
		pal_MemoryFree((void **)&pFrom);
	}

	if (NULL != pDisplayName)
	{
		pal_MemoryFree((void **)&pDisplayName);
	}

	if (NULL != pMultiPart)
	{
		for (i = 0; i < pMultiPart->uNumOfBody; i++)
		{
			if (pMultiPart->ppBody[i]->pContentType)
			{
				pal_MemoryFree((void **)&pMultiPart->ppBody[i]->pContentType);
			}
			if (pMultiPart->ppBody[i]->pMessageBody)
			{
				pal_MemoryFree((void **)&pMultiPart->ppBody[i]->pMessageBody);
			}
			pal_MemoryFree((void **)&pMultiPart->ppBody[i]);
		}
		pal_MemoryFree((void **)&pMultiPart->ppBody);
		pal_MemoryFree((void **)&pMultiPart);
	}

	if (NULL != resource.ppUri)
	{
		for (i = 0; i < resource.uNumOfUsers; i++)
		{
			if (resource.ppUri[i])
			{
				pal_MemoryFree((void **)&resource.ppUri[i]);
			}
		}
		pal_MemoryFree((void **)&resource.ppUri);
	}

	if (NULL != pParsedXml)
	{
		ec_CPM_ReleaseParsedXmlStruct(&pParsedXml, Enum_TRUE);
	}

	if (NULL != pGroupSessionId)
	{
		pal_MemoryFree((void **)&pGroupSessionId);
	}

	if (NULL != pSubject)
	{
		CPMLOGD(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tFree the Subject", __FUNCTION__, __LINE__);
		pal_MemoryFree((void **)&pSubject);
	}

	if (NULL != pBoundaryStr)
	{
		pal_MemoryFree((void **)&pBoundaryStr);
	}

	if (NULL != pAlias)
	{
		pal_MemoryFree((void **)&pAlias);
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u\tuError=%u", __FUNCTION__, __LINE__, uError);
}

/** Internal function called when an incoming Re-Invite request is received */
void ec_CPM_HandleReInviteMessageRequest
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	u_int32	uSDPError = ECRIO_SDP_NO_ERROR;
	EcrioSigMgrSipMessageStruct	*pInviteNotifStruct = NULL;
	EcrioSigMgrUnknownMessageBodyStruct *pUnknownBody = NULL;
	EcrioSigMgrMessageBodyStruct msgBody = { .messageBodyType = EcrioSigMgrMessageBodySDP };
	EcrioSigMgrUnknownMessageBodyStruct unknownMsgBody = { 0 };
	EcrioSDPStringStruct tSDP = { 0 };
	EcrioCPMSessionStruct *pSession = NULL;
	EcrioSigMgrSipMessageStruct inviteResp = { .eMethodType = EcrioSipMessageTypeNone };
	EcrioSigMgrSipMessageStruct *pInviteResp = NULL;
	u_int32	i = 0;
	char *pBoundaryStr = NULL;
	u_char *pBoundary = NULL;
	BoolEnum bIsMultipart = Enum_FALSE;
	EcrioCPMMultipartMessageStruct *pMultiPart = NULL;

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Find the current CPM session structure */

	pInviteNotifStruct = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;

	/** Get remote IP & port fropm SDP */
	if ((NULL == pInviteNotifStruct->pMessageBody) ||
		(NULL == pInviteNotifStruct->pMessageBody->pMessageBody))
	{
		uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
		goto Error_Level_01;
	}

	EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, pInviteNotifStruct->pMandatoryHdrs->pCallId, (void**)&pSession);
	if (pSession == NULL)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpSession not found for pCallId=%s",
			__FUNCTION__, __LINE__, pInviteNotifStruct->pMandatoryHdrs->pCallId);

		uError = ec_CPMSendInviteResponse(pContext, pInviteNotifStruct->pMandatoryHdrs->pCallId,
			ECRIO_SIGMGR_RESPONSE_CALL_TXN_NOT_EXIST, ECRIO_SIGMGR_RESPONSE_CODE_CALL_TXN_NOT_EXIST);
		if (uError != ECRIO_SIGMGR_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPMSendInviteResponse() failed with uError=%u",
				__FUNCTION__, __LINE__, uError);
		}
		goto Error_Level_01;
	}

	if (inviteResp.pMandatoryHdrs == NULL)
	{
		pal_MemoryAllocate((u_int32)sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&inviteResp.pMandatoryHdrs);
		if (inviteResp.pMandatoryHdrs == NULL)
		{
			uError = ECRIO_CPM_MEMORY_ERROR;
			goto Error_Level_01;
		}
	}
	inviteResp.pMandatoryHdrs->pCallId = pal_StringCreate(pInviteNotifStruct->pMandatoryHdrs->pCallId, pal_StringLength(pInviteNotifStruct->pMandatoryHdrs->pCallId));

	
	pUnknownBody = (EcrioSigMgrUnknownMessageBodyStruct *)pInviteNotifStruct->pMessageBody->pMessageBody;
	if (NULL == pUnknownBody->pBuffer)
	{
		uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
		goto Error_Level_01;
	}

	/** Handle multipart message body */
	EcrioSigMgrHeaderValueStruct *pHdrValue = &pUnknownBody->contentType;
	if (pal_StringCompare(pHdrValue->pHeaderValue, (u_char *)"multipart/mixed") == 0)
	{
		bIsMultipart = Enum_TRUE;
		if (pHdrValue->ppParams && pHdrValue->ppParams[0])
		{
			for (i = 0; i < pHdrValue->numParams; i++)
			{
				if (pal_StringCompare(pHdrValue->ppParams[i]->pParamName, (u_char *)"boundary") == 0)
				{
					pBoundary = pHdrValue->ppParams[i]->pParamValue;
				}
			}
		}
	}

	if (bIsMultipart == Enum_TRUE && pBoundary != NULL)
	{
		pBoundaryStr = (char*)ec_CPM_StringUnquote(pBoundary);
		uError = ec_CPM_ParseMultipartMessage(pContext, &pMultiPart, (char*)pBoundaryStr,
			(char*)pUnknownBody->pBuffer, pUnknownBody->bufferLength);
		if (uError != ECRIO_CPM_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_ParseMultipartMessage() failed with uError=%u",
				__FUNCTION__, __LINE__, uError);
			goto Error_Level_01;
		}

		for (i = 0; i < pMultiPart->uNumOfBody; i++)
		{
			if (pal_StringCompare(pMultiPart->ppBody[i]->pContentType, (u_char *)"application/sdp") == 0)
			{
				tSDP.pData = pMultiPart->ppBody[i]->pMessageBody;
				tSDP.uContainerSize = ECRIO_CPM_SDP_STRING_SIZE;
				tSDP.uSize = pMultiPart->ppBody[i]->uContentLength;
			}
		}
	}
	else
	{
		tSDP.pData = pUnknownBody->pBuffer;
		tSDP.uContainerSize = ECRIO_CPM_SDP_STRING_SIZE;
		tSDP.uSize = pUnknownBody->bufferLength;
	}

	uSDPError = EcrioSDPSessionHandleSDPString(pSession->hSDPSessionHandle, &pSession->remoteSDP, &tSDP);
	if (uSDPError != ECRIO_SDP_NO_ERROR)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSDPSessionHandleSDPString() uSDPError=%u",
			__FUNCTION__, __LINE__, uSDPError);

		goto Error_Level_01;
	}

	if ((pSession->remoteSDP.uNumOfRemoteMedia > 1) || (pSession->remoteSDP.remoteStream[0].eMediaType != ECRIO_SDP_MEDIA_TYPE_MSRP))
	{
		uError = ec_CPMSendInviteResponse(pContext, pInviteNotifStruct->pMandatoryHdrs->pCallId,
			ECRIO_SIGMGR_RESPONSE_UNSUPPORTED_MEDIA_TYPE, ECRIO_SIGMGR_RESPONSE_CODE_UNSUPPORTED_MEDIA_TYPE);
		if (uError != ECRIO_SIGMGR_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPMSendInviteResponse() failed with uError=%u",
				__FUNCTION__, __LINE__, uError);
		}
		goto Error_Level_01;
	}


	if (pSession->remoteSDP.remoteStream[0].eProtocol != ECRIO_SDP_MEDIA_PROTOCOL_TCP_MSRP &&
		pSession->remoteSDP.remoteStream[0].eProtocol != ECRIO_SDP_MEDIA_PROTOCOL_TLS_MSRP)
	{
		uError = ec_CPMSendInviteResponse(pContext, pInviteNotifStruct->pMandatoryHdrs->pCallId,
			ECRIO_SIGMGR_RESPONSE_NOT_ACCEPTABLE_HERE, ECRIO_SIGMGR_RESPONSE_CODE_NOT_ACCEPTABLE_HERE);
		if (uError != ECRIO_SIGMGR_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPMSendInviteResponse() failed with uError=%u",
				__FUNCTION__, __LINE__, uError);
		}
		goto Error_Level_01;
	}

	if (pSession->remoteSDP.eEvent == ECRIO_SDP_EVENT_IDENTICAL)
	{
		pSession->localSDP.eEvent = ECRIO_SDP_EVENT_IDENTICAL;

		pal_MemorySet((void*)pSession->pSDPInformationBuffer, 0, ECRIO_CPM_SDP_INFO_BUFFER_SIZE);
		pSession->localSDP.strings.pData = pSession->pSDPInformationBuffer;
		pSession->localSDP.strings.uContainerSize = ECRIO_CPM_SDP_INFO_BUFFER_SIZE;
		pSession->localSDP.strings.uSize = 0;

		/* Generate SDP Answer */
		pal_MemorySet((void*)pSession->pSDPStringBuffer, 0, ECRIO_CPM_SDP_STRING_SIZE);
		tSDP.pData = pSession->pSDPStringBuffer;
		tSDP.uContainerSize = ECRIO_CPM_SDP_STRING_SIZE;
		tSDP.uSize = 0;

		uSDPError = EcrioSDPSessionGenerateSDPString(pSession->hSDPSessionHandle, &pSession->localSDP, &tSDP);
		if (uSDPError != ECRIO_SDP_NO_ERROR)
		{
			uError = ECRIO_CPM_INTERNAL_ERROR;
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSDPSessionGenerateSDPString() uSDPError=%u",
				__FUNCTION__, __LINE__, uSDPError);

			goto Error_Level_01;
		}

		unknownMsgBody.bufferLength = tSDP.uSize;
		unknownMsgBody.pBuffer = tSDP.pData;
		/* content-type : application-sdp
		*/
		unknownMsgBody.contentType.numParams = 0;
		unknownMsgBody.contentType.ppParams = NULL;
		unknownMsgBody.contentType.pHeaderValue = (u_char *)"application/sdp";

		msgBody.messageBodyType = EcrioSigMgrMessageBodyUnknown;
		msgBody.pMessageBody = &unknownMsgBody;
		inviteResp.pMessageBody = &msgBody;

		inviteResp.pReasonPhrase = (u_char *)ECRIO_SIGMGR_RESPONSE_OK;
		inviteResp.responseCode = ECRIO_SIGMGR_RESPONSE_CODE_OK;

		uError = EcrioSigMgrSendInviteResponse(pContext->hEcrioSigMgrHandle, &inviteResp);
		if (uError != ECRIO_SIGMGR_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSigMgrSendInviteResponse() failed with uError=%u",
				__FUNCTION__, __LINE__, uError);
		}
	}

	goto Error_Level_01;

Error_Level_01:
	if (NULL != pMultiPart)
	{
		for (i = 0; i < pMultiPart->uNumOfBody; i++)
		{
			if (pMultiPart->ppBody[i]->pContentType)
			{
				pal_MemoryFree((void **)&pMultiPart->ppBody[i]->pContentType);
			}
			if (pMultiPart->ppBody[i]->pMessageBody)
			{
				pal_MemoryFree((void **)&pMultiPart->ppBody[i]->pMessageBody);
			}
			pal_MemoryFree((void **)&pMultiPart->ppBody[i]);
		}
		pal_MemoryFree((void **)&pMultiPart->ppBody);
	}

	if (NULL != pBoundaryStr)
	{
		pal_MemoryFree((void **)&pBoundaryStr);
	}

	inviteResp.pReasonPhrase = NULL;
	inviteResp.pMessageBody = NULL;
	pInviteResp = &inviteResp;
	EcrioSigMgrStructRelease(pContext->hEcrioSigMgrHandle, EcrioSigMgrStructType_SipMessage, (void **)&pInviteResp, Enum_FALSE);


	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u\tuError=%u", __FUNCTION__, __LINE__, uError);
}

/** Internal function called when an incoming Invite response is received */
void ec_CPM_HandleInviteMessageResponse
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	u_int32 uSigMgrError = ECRIO_SIGMGR_NO_ERROR;
	u_int32	uSDPError = ECRIO_SDP_NO_ERROR;
	EcrioSigMgrSipMessageStruct	*pInviteNotifStruct = NULL;
	EcrioCPMSessionStruct *pSession = NULL;
	EcrioCPMNotifStruct cpmNotifStruct = { .eNotifCmd = EcrioCPM_Notif_None };
	EcrioCPMAcceptedSessionStruct acceptedStruct = { 0 };
	EcrioCPMFailedSessionStruct failedStruct = { 0 };
	EcrioSDPStringStruct tSDP = { 0 };
	u_int32	i = 0, j = 0;
	char *pBoundaryStr = NULL;
	u_char *pBoundary = NULL;
	u_char *pGroupSessionId = NULL;
	BoolEnum bIsMultipart = Enum_FALSE;
	BoolEnum bIsGroupChat = Enum_FALSE;
	BoolEnum bIsChatbot = Enum_FALSE, bTagChatbot = Enum_FALSE, bChatbotIARI = Enum_FALSE, bBotVer = Enum_FALSE;
	EcrioCPMMultipartMessageStruct *pMultiPart = NULL;
	EcrioCPMAliasingParamStruct *pAlias = NULL;

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Find the current CPM session structure */

	pInviteNotifStruct = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;

	if (pInviteNotifStruct == NULL || pInviteNotifStruct->pMandatoryHdrs == NULL)
	{
		uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
		goto Error_Level_01;
	}

	EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, pInviteNotifStruct->pMandatoryHdrs->pCallId, (void**)&pSession);

	if (pSession == NULL)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpSession not found for pCallId=%s",
			__FUNCTION__, __LINE__, pInviteNotifStruct->pMandatoryHdrs->pCallId);
		goto Error_Level_01;
	}

	switch (pInviteNotifStruct->statusCode)
	{
		case ECRIO_SIGMGR_INVITE_IN_PROGRESS:
		{
			if (pInviteNotifStruct->responseCode != 100)
			{
				/*if (EcrioSigMgrCheckPrackSupport(pContext->hEcrioSigMgrHandle, pInviteNotifStruct->pOptionalHeaderList, NULL) == Enum_TRUE)
				{
					sipRequest.eMethodType = EcrioSipMessageTypePrack;
					sipRequest.eReqRspType = EcrioSigMgrSIPRequest;

					if (sipRequest.pMandatoryHdrs == NULL)
					{
						pal_MemoryAllocate((u_int32)sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&sipRequest.pMandatoryHdrs);
					}
					sipRequest.pMandatoryHdrs->pCallId = pSession->pCallId;
					uSigMgrError = EcrioSigMgrSendPrackRequest(pContext->hEcrioSigMgrHandle, &sipRequest);
					if (ECRIO_SIGMGR_NO_ERROR != uSigMgrError)
					{
						CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrSendPrackRequest() uSigMgrError=%u",
							__FUNCTION__, __LINE__, uSigMgrError);
						uError = ECRIO_CPM_INTERNAL_ERROR;
						goto Error_Level_01;
					}
				}*/

				acceptedStruct.contentTypes = EcrioCPMContentTypeEnum_None;
				cpmNotifStruct.eNotifCmd = (pInviteNotifStruct->responseCode == 180) ? EcrioCPM_Notif_SessionRinging : EcrioCPM_Notif_SessionProgress;
				if (pContext->bIsRelayEnabled == Enum_TRUE)
					acceptedStruct.pSessionId = (char *)pSession->relayData.pAppCallId;
				else
					acceptedStruct.pSessionId = (char *)pSession->pCallId;

				if (pSession->pPeerDisplayName != NULL)
				{
					pal_MemoryFree((void**)&pSession->pPeerDisplayName);
					pSession->pPeerDisplayName = NULL;
				}

				if (pInviteNotifStruct->ppPAssertedIdentity != NULL && pInviteNotifStruct->numPAssertedIdentities > 0)
				{
					u_int32 index = 0;
					for (index = 0; index < pInviteNotifStruct->numPAssertedIdentities; index++)
					{
						if (pInviteNotifStruct->ppPAssertedIdentity[index] != NULL &&
							pInviteNotifStruct->ppPAssertedIdentity[index]->pDisplayName != NULL)
						{
							pSession->pPeerDisplayName = pal_StringCreate(pInviteNotifStruct->ppPAssertedIdentity[index]->pDisplayName,
								pal_StringLength(pInviteNotifStruct->ppPAssertedIdentity[index]->pDisplayName));
							break;
						}
					}
				}
				else if (pInviteNotifStruct->pMandatoryHdrs->pTo != NULL &&
					pInviteNotifStruct->pMandatoryHdrs->pTo->nameAddr.pDisplayName != NULL)
				{
					pSession->pPeerDisplayName = pal_StringCreate(pInviteNotifStruct->pMandatoryHdrs->pTo->nameAddr.pDisplayName,
						pal_StringLength(pInviteNotifStruct->pMandatoryHdrs->pTo->nameAddr.pDisplayName));

				}
				acceptedStruct.pDisplayName = (char*)pSession->pPeerDisplayName;
				
				acceptedStruct.pConvId = NULL;
				cpmNotifStruct.pNotifData = &acceptedStruct;

				/** Notfiy upper layer */
				pContext->callbackStruct.pCPMNotificationCallbackFn(pContext->callbackStruct.pCallbackData,
					&cpmNotifStruct);
			}
		}
		break;

		case ECRIO_SIGMGR_RESPONSE_SUCCESS:
		{
			u_int32 uMSRPError = ECRIO_MSRP_NO_ERROR;
			EcrioSigMgrUnknownMessageBodyStruct *pUnknownBody = NULL;
			EcrioCPMConversationsIdStruct convIds = { 0 };
			EcrioCPMRemoteSDPInfoStruct* pRemoteMSRPInfo = NULL;
			EcrioSDPMSRPMediaTypesEnum eAcceptWrappedTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_NONE;
			BoolEnum bNotifySessionEstablish = Enum_TRUE;

			acceptedStruct.contentTypes = EcrioCPMContentTypeEnum_None;

			/* If DUT is MO with relay enabled, postpone session established notification till we receive 
			MSRP Empty packet from MT. */
			if (pContext->bIsRelayEnabled == Enum_TRUE && pSession->relayData.pDest != NULL)
				bNotifySessionEstablish = Enum_FALSE;

			acceptedStruct.bIsClosedGroupChat = pSession->bIsClosedGroupChat;
			cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_SessionEstablished;
			if (pContext->bIsRelayEnabled == Enum_TRUE)
				acceptedStruct.pSessionId = (char *)pSession->relayData.pAppCallId;
			else
				acceptedStruct.pSessionId = (char *)pSession->pCallId;

			if (pSession->pPeerDisplayName != NULL)
			{
				pal_MemoryFree((void**)&pSession->pPeerDisplayName);
				pSession->pPeerDisplayName = NULL;
			}

			if (pInviteNotifStruct->ppPAssertedIdentity != NULL && pInviteNotifStruct->numPAssertedIdentities > 0)
			{
				u_int32 index = 0;
				for (index = 0; index < pInviteNotifStruct->numPAssertedIdentities; index++)
				{
					if (pInviteNotifStruct->ppPAssertedIdentity[index] != NULL &&
						pInviteNotifStruct->ppPAssertedIdentity[index]->pDisplayName != NULL)
					{
						pSession->pPeerDisplayName = pal_StringCreate(pInviteNotifStruct->ppPAssertedIdentity[index]->pDisplayName,
							pal_StringLength(pInviteNotifStruct->ppPAssertedIdentity[index]->pDisplayName));
						break;
					}
				}
			}
			else if (pInviteNotifStruct->pMandatoryHdrs->pTo != NULL &&
				pInviteNotifStruct->pMandatoryHdrs->pTo->nameAddr.pDisplayName != NULL)
			{
				pSession->pPeerDisplayName = pal_StringCreate(pInviteNotifStruct->pMandatoryHdrs->pTo->nameAddr.pDisplayName,
					pal_StringLength(pInviteNotifStruct->pMandatoryHdrs->pTo->nameAddr.pDisplayName));
				
			} 
			acceptedStruct.pDisplayName = (char*)pSession->pPeerDisplayName;

			if (pInviteNotifStruct->pConvId)
			{
				acceptedStruct.pConvId = &convIds;
				convIds.pContributionId = pInviteNotifStruct->pConvId->pContributionId;
				convIds.pConversationId = pInviteNotifStruct->pConvId->pConversationId;
				convIds.pInReplyToContId = pInviteNotifStruct->pConvId->pInReplyToContId;
			}

			if ((NULL == pInviteNotifStruct->pMessageBody) || (NULL == pInviteNotifStruct->pMessageBody->pMessageBody))
			{
				uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
				goto Error_Level_01;
			}

			pUnknownBody = (EcrioSigMgrUnknownMessageBodyStruct *)pInviteNotifStruct->pMessageBody->pMessageBody;
			if (NULL == pUnknownBody->pBuffer)
			{
				uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
				goto Error_Level_01;
			}

			pal_MemorySet((void*)&pSession->remoteSDP, 0, sizeof(EcrioSDPInformationStruct));
			pSession->remoteSDP.strings.pData = pSession->pSDPInformationBuffer;
			pSession->remoteSDP.strings.uContainerSize = ECRIO_CPM_SDP_INFO_BUFFER_SIZE;
			pSession->remoteSDP.strings.uSize = 0;

			/** Check Contact header whether request is group chat / chatbot or not */
			if (pInviteNotifStruct->pContact)
			{
				for (i = 0; i < pInviteNotifStruct->pContact->numContactUris; i++)
				{
					if (pInviteNotifStruct->pContact->ppContactDetails[i])
					{
						for (j = 0; j < pInviteNotifStruct->pContact->ppContactDetails[i]->numParams; j++)
						{
							if (pInviteNotifStruct->pContact->ppContactDetails[i]->ppParams[j])
							{
								if (pal_StringCompare(pInviteNotifStruct->pContact->ppContactDetails[i]->ppParams[j]->pParamName, (u_char *)"isfocus") == 0)
								{
									bIsGroupChat = Enum_TRUE;
									if (pInviteNotifStruct->pContact->ppContactDetails[i]->nameAddr.addrSpec.uriScheme == EcrioSigMgrURISchemeSIP)
									{
										EcrioSigMgrSipURIStruct contactUri = {0};
										EcrioSigMgrIPAddrStruct ipAddr = {0};
										EcrioSigMgrNameAddrWithParamsStruct *pTempNameAddr = NULL;
										EcrioSigMgrCopyNameAddrWithParamStruct(pContext->hEcrioSigMgrHandle, pInviteNotifStruct->pContact->ppContactDetails[i], &pTempNameAddr);

										if (pTempNameAddr != NULL)
										{
											EcrioSigMgrFormSipURI(pContext->hEcrioSigMgrHandle, pTempNameAddr->nameAddr.addrSpec.u.pSipUri, &pGroupSessionId);
											EcrioSigMgrStructRelease(pContext->hEcrioSigMgrHandle, EcrioSigMgrStructType_NameAddrWithParams, (void **)&pTempNameAddr, Enum_TRUE);
										}
										else
										{
											CPMLOGD(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tFailed to populate the GroupID. Fallback on old logic",
												__FUNCTION__, __LINE__);
											contactUri.pUserId = pInviteNotifStruct->pContact->ppContactDetails[i]->nameAddr.addrSpec.u.pSipUri->pUserId;
											if (pInviteNotifStruct->pContact->ppContactDetails[i]->nameAddr.addrSpec.u.pSipUri->pIPAddr != NULL)
											{
												ipAddr.pIPAddr = pInviteNotifStruct->pContact->ppContactDetails[i]->nameAddr.addrSpec.u.pSipUri->pIPAddr->pIPAddr;
												ipAddr.port = pInviteNotifStruct->pContact->ppContactDetails[i]->nameAddr.addrSpec.u.pSipUri->pIPAddr->port;
											}
											else
											{
												ipAddr.pIPAddr = pInviteNotifStruct->pContact->ppContactDetails[i]->nameAddr.addrSpec.u.pSipUri->pDomain;
											}
											contactUri.pIPAddr = &ipAddr;
											EcrioSigMgrFormSipURI(pContext->hEcrioSigMgrHandle, &contactUri, &pGroupSessionId);
										}
										CPMLOGD(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tGroup Session ID: %s",
											__FUNCTION__, __LINE__, pGroupSessionId);
									}
									else
									{
										uSigMgrError = EcrioSigMgrGetOriginator(pContext->hEcrioSigMgrHandle, &pInviteNotifStruct->pContact->ppContactDetails[i]->nameAddr, &pGroupSessionId);
									}
									acceptedStruct.pGroupSessionId = (char*)pGroupSessionId;
									if (pSession->pGroupSessionId)
										pal_MemoryFree((void **)&(pSession->pGroupSessionId));
									pSession->pGroupSessionId = pal_StringCreate((const u_char*)pGroupSessionId, pal_StringLength((const u_char*)pGroupSessionId));
									break;
								}
								else if (pal_StringCompare(pInviteNotifStruct->pContact->ppContactDetails[i]->ppParams[j]->pParamName, ECRIO_CPM_3GPP_FEATURE_ISBOT) == 0)
								{
									bTagChatbot = Enum_TRUE;
								}
								else if (pal_StringCompare(pInviteNotifStruct->pContact->ppContactDetails[i]->ppParams[j]->pParamName, (u_char*)"+g.3gpp.iari-ref") == 0)
								{
									if (pal_StringFindSubString(pInviteNotifStruct->pContact->ppContactDetails[i]->ppParams[j]->pParamValue, ECRIO_CPM_3GPP_FEATURE_CHATBOT) != NULL)
									{
										bChatbotIARI = Enum_TRUE;
									}
								}
								else if (pal_StringCompare(pInviteNotifStruct->pContact->ppContactDetails[i]->ppParams[j]->pParamName, (u_char*)"+g.gsma.rcs.botversion") == 0)
								{
									bBotVer = Enum_TRUE;
								}
							}
						}
					}
					if (bIsGroupChat == Enum_TRUE)
					{
						break;
					}
					else if (bTagChatbot == Enum_TRUE && bChatbotIARI == Enum_TRUE && bBotVer == Enum_TRUE)
					{
						bIsChatbot = Enum_TRUE;
						break;
					}
				}
			}

			/** Handle multipart message body */
			EcrioSigMgrHeaderValueStruct *pHdrValue = &pUnknownBody->contentType;
			if (pal_StringCompare(pHdrValue->pHeaderValue, (u_char *)"multipart/mixed") == 0)
			{
				bIsMultipart = Enum_TRUE;
				if (pHdrValue->ppParams && pHdrValue->ppParams[0])
				{
					for (i = 0; i < pHdrValue->numParams; i++)
					{
						if (pal_StringCompare(pHdrValue->ppParams[i]->pParamName, (u_char *)"boundary") == 0)
						{
							pBoundary = pHdrValue->ppParams[i]->pParamValue;
						}
					}
				}
			}

			if (bIsMultipart == Enum_TRUE && pBoundary != NULL)
			{
				pBoundaryStr = (char*)ec_CPM_StringUnquote(pBoundary);
				uError = ec_CPM_ParseMultipartMessage(pContext, &pMultiPart, (char*)pBoundaryStr,
					(char*)pUnknownBody->pBuffer, pUnknownBody->bufferLength);
				if (uError != ECRIO_CPM_NO_ERROR)
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_ParseMultipartMessage() failed with uError=%u",
						__FUNCTION__, __LINE__, uError);
					goto Error_Level_01;
				}

				for (i = 0; i < pMultiPart->uNumOfBody; i++)
				{
					if (pal_StringCompare(pMultiPart->ppBody[i]->pContentType, (u_char *)"application/sdp") == 0)
					{
						tSDP.pData = pMultiPart->ppBody[i]->pMessageBody;
						tSDP.uContainerSize = ECRIO_CPM_SDP_STRING_SIZE;
						tSDP.uSize = pMultiPart->ppBody[i]->uContentLength;
					}
				}
			}
			else
			{
				tSDP.pData = pUnknownBody->pBuffer;
				tSDP.uContainerSize = ECRIO_CPM_SDP_STRING_SIZE;
				tSDP.uSize = pUnknownBody->bufferLength;
			}

			/** Handling received SDP */
			uSDPError = EcrioSDPSessionHandleSDPString(pSession->hSDPSessionHandle, &pSession->remoteSDP, &tSDP);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				uError = ECRIO_CPM_INTERNAL_ERROR;
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSDPSessionHandleSDPString() uSDPError=%u",
					__FUNCTION__, __LINE__, uSDPError);

				goto Error_Level_01;
			}

			ec_CPM_PopulateRemoteMSRPInfo(pContext,pSession, pSession->remoteSDP);
			/* Check and overwrite the connection setup. If relay is enabled and setup:actpass, then
			local setup:active i.e., local connection type is TCP client*/
			if (pSession->remoteSDP.remoteStream[0].u.msrp.eSetup == ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_ACTIVE &&
				pContext->bIsRelayEnabled == Enum_TRUE)
			{
				u_int16 length = 0;
				u_char *pRemotePath = NULL;
				u_char *pTemp = pSession->remoteSDPInfo.pRemotePath;
			
				if (pContext->bMsrpOverTLS == Enum_TRUE)
				{
					pSession->eLocalConnectionType = MSRPConnectionType_TLS_Client;
				}
				else
				{
					pSession->eLocalConnectionType = MSRPConnectionType_TCP_Client;
				}
				

				length = (u_int16)pal_StringLength(pTemp);
				length += (u_int16)pal_StringLength(pSession->relayData.pUsePath);
				length += 2; /* Space and null termination */

				pal_MemoryAllocate(length, (void**)&pRemotePath);
				if (NULL == pRemotePath)
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
						__FUNCTION__, __LINE__);
					uError = ECRIO_CPM_MEMORY_ERROR;
					goto Error_Level_01;
				}
				if (NULL == pal_StringNCopy(pRemotePath, length, pSession->relayData.pUsePath, pal_StringLength(pSession->relayData.pUsePath))) /* Add relay path */
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
					__FUNCTION__, __LINE__);
					uError = ECRIO_CPM_MEMORY_ERROR;
					goto Error_Level_01;
				}
				if (NULL == pal_StringNConcatenate(pRemotePath, length - pal_StringLength((u_char*)pRemotePath), (const u_char *)" ", 1))/* Space between 2 MSRP URL */
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
					__FUNCTION__, __LINE__);
					uError = ECRIO_CPM_MEMORY_ERROR;
					goto Error_Level_01;
				}
				if (NULL == pal_StringNConcatenate(pRemotePath, length - pal_StringLength((u_char*)pRemotePath), pTemp, pal_StringLength(pTemp))) /* Add local path */
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
					__FUNCTION__, __LINE__);
					uError = ECRIO_CPM_MEMORY_ERROR;
					goto Error_Level_01;
				}

				pRemoteMSRPInfo = &pSession->remoteSDPInfo;
				pRemoteMSRPInfo->pRemotePath = pSession->pSDPWorkingBuffer + pal_StringLength(pRemoteMSRPInfo->pRemoteIp) + 2;
				pal_MemorySet(pRemoteMSRPInfo->pRemotePath, 0,
					ECRIO_CPM_SDP_INFO_BUFFER_SIZE - (pal_StringLength(pRemoteMSRPInfo->pRemoteIp) + 2));
				if (NULL == pal_StringNCopy(pRemoteMSRPInfo->pRemotePath, ECRIO_CPM_SDP_INFO_BUFFER_SIZE - (pal_StringLength(pRemoteMSRPInfo->pRemoteIp) + 2), 
					pRemotePath, pal_StringLength(pRemotePath)))
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
					__FUNCTION__, __LINE__);
					uError = ECRIO_CPM_MEMORY_ERROR;
					goto Error_Level_01;
				}
				pal_MemoryFree((void**)&pRemotePath);
				pRemotePath = NULL;
			}
			else if(pSession->remoteSDP.remoteStream[0].u.msrp.eSetup == ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_ACTIVE &&
				pContext->bIsRelayEnabled == Enum_FALSE)
			{
				pSession->eLocalConnectionType = MSRPConnectionType_TCP_Server;
			}
			
			/* MSRP Session setup based on https://tools.ietf.org/html/rfc4975#section-5.4,
			https://tools.ietf.org/html/rfc4975#section-4 and http://www.qtc.jp/3GPP/Specs/24247-910.pdf @ page 33 item #29 */
			if ((pSession->eLocalConnectionType == MSRPConnectionType_TCP_Client || pSession->eLocalConnectionType == MSRPConnectionType_TLS_Client) &&
				pContext->bIsRelayEnabled == Enum_FALSE)
			{
				uError = ec_CPM_StartMSRPSession(pContext, pSession);
				if (uError != ECRIO_CPM_NO_ERROR)
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_StartMSRPSession() error=%u",
						__FUNCTION__, __LINE__, uError);
					if (pSession->bIsGroupChat == Enum_TRUE) {
						pSession->eTerminationReason = EcrioSessionTerminationReason_MSRPSocketFailure;
						EcrioSigMgrTerminateSession(pContext->hEcrioSigMgrHandle, pSession->pCallId,
							Enum_FALSE);
					}
					goto Error_Level_01;
				}
			}
			else /* Set remote details as DUT is acting as TCP server. This info will be used by MSRP lib for verifying incoming MSRP packets */
			{
				MSRPHostStruct msrpRemoteHost = { 0 };
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

				if (pSession->eLocalConnectionType == MSRPConnectionType_TCP_Server && pContext->bIsRelayEnabled == Enum_FALSE)
				{
					uError = ec_CPM_StartMSRPSession(pContext, pSession);
					if (uError != ECRIO_CPM_NO_ERROR)
					{
						CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_StartMSRPSession() error=%u",
							__FUNCTION__, __LINE__, uError);
						if(pSession->bIsGroupChat == Enum_TRUE) {
							pSession->eTerminationReason = EcrioSessionTerminationReason_MSRPSocketFailure;
							EcrioSigMgrTerminateSession(pContext->hEcrioSigMgrHandle, pSession->pCallId,
														Enum_FALSE);
						}
						goto Error_Level_01;
					}
				}
				else
				{
					pSession->bIsStartMSRPSession = Enum_TRUE;
				}

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

			/** Check PAI header fields - if present, then store the tk param and aliason value */
			if (bIsChatbot == Enum_TRUE)
			{
				if (pInviteNotifStruct->ppPAssertedIdentity != NULL)
				{
					for (i = 0; i < pInviteNotifStruct->numPAssertedIdentities; i++)
					{
						uError = ec_CPM_GetAliasParamFromPAI(pContext, pInviteNotifStruct->ppPAssertedIdentity[i], &pAlias);
						if (pAlias != NULL)
						{
							break;
						}
					}
				}
			}

			/** Chatbot role */
			if (bTagChatbot == Enum_TRUE)
			{
				acceptedStruct.bIsChatbotRole = Enum_TRUE;
			}
			else
			{
				acceptedStruct.bIsChatbotRole = Enum_FALSE;
			}

			eAcceptWrappedTypes = pSession->remoteSDP.remoteStream[0].u.msrp.eAcceptWrappedTypes;

			if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_TEXT_PLAIN) == ECRIO_SDP_MSRP_MEDIA_TYPE_TEXT_PLAIN)
				acceptedStruct.contentTypes |= EcrioCPMContentTypeEnum_Text;

			if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_IMDN_XML) == ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_IMDN_XML)
				acceptedStruct.contentTypes |= EcrioCPMContentTypeEnum_IMDN;

			if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_IM_ISCOMPOSING_XML) == ECRIO_SDP_MSRP_MEDIA_TYPE_IM_ISCOMPOSING_XML)
				acceptedStruct.contentTypes |= EcrioCPMContentTypeEnum_Composing;

			/*if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_MULTIPART_MIXED) == ECRIO_SDP_MSRP_MEDIA_TYPE_MULTIPART_MIXED)
				acceptedStruct.contentTypes |= EcrioCPMContentTypeEnum_Text;*/

			if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_HTTP_FILE_TRANSFER) == ECRIO_SDP_MSRP_MEDIA_TYPE_HTTP_FILE_TRANSFER)
				acceptedStruct.contentTypes |= EcrioCPMContentTypeEnum_FileTransferOverHTTP;

			if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_RICHCARD) == ECRIO_SDP_MSRP_MEDIA_TYPE_RICHCARD)
				acceptedStruct.contentTypes |= EcrioCPMContentTypeEnum_RichCard;

			if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTED_CHIPLIST) == ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTED_CHIPLIST)
				acceptedStruct.contentTypes |= EcrioCPMContentTypeEnum_SuggestedChipList;

			if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTION_RESPONSE) == ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTION_RESPONSE)
				acceptedStruct.contentTypes |= EcrioCPMContentTypeEnum_SuggestionResponse;

			if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_PUSH_LOCATION) == ECRIO_SDP_MSRP_MEDIA_TYPE_PUSH_LOCATION)
				acceptedStruct.contentTypes |= EcrioCPMContentTypeEnum_PushLocation;

			if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_SHARED_CLIENT_DATA) == ECRIO_SDP_MSRP_MEDIA_TYPE_SHARED_CLIENT_DATA)
				acceptedStruct.contentTypes |= EcrioCPMContentTypeEnum_SharedClientData;

			/*if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_WILDCARD) == ECRIO_SDP_MSRP_MEDIA_TYPE_WILDCARD)
				acceptedStruct.contentTypes |= EcrioCPMContentTypeEnum_Text;*/

			if (bIsGroupChat == Enum_TRUE) {
				if ((eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_GROUPDATA) == ECRIO_SDP_MSRP_MEDIA_TYPE_GROUPDATA)
					acceptedStruct.contentTypes |= EcrioCPMContentTypeEnum_GroupData;
			}

			/** File Transfer session */
			if (pSession->pFile != NULL)
			{
				acceptedStruct.contentTypes = EcrioCPMContentTypeEnum_FileTransferOverMSRP;
			}

			acceptedStruct.pAlias = pAlias;

			cpmNotifStruct.pNotifData = &acceptedStruct;

			pSession->state = EcrioCPMSessionState_Active;

			/* Book keeping for future use */
			pSession->contentTypes = acceptedStruct.contentTypes;

			/* Notify only if relay is disabled */
			if (bNotifySessionEstablish == Enum_TRUE)
				/** Notfiy upper layer */
				pContext->callbackStruct.pCPMNotificationCallbackFn(pContext->callbackStruct.pCallbackData,
				&cpmNotifStruct);
			
			break;
		}

		case ECRIO_SIGMGR_RESPONSE_REDIRECTED:
		{
		}
		break;

		case ECRIO_SIGMGR_REQUEST_FAILED:
		default:
		{
			if(pContext->bIsRelayEnabled == Enum_TRUE)
				failedStruct.pSessionId = (char *)pSession->relayData.pAppCallId;
			else
				failedStruct.pSessionId = (char *)pSession->pCallId;
				
			failedStruct.uResponseCode = pInviteNotifStruct->responseCode;
			failedStruct.eAction = EcrioCPMActionNone;
			/*handle 403 and 504 response*/

			//if (pInviteNotifStruct->responseCode == ECRIO_SIGMGR_RESPONSE_CODE_FORBIDDEN)
			{
				EcrioSigMgrHeaderStruct *pWarningeHeader = NULL;
				//check if 403 response contains Warning header.
				EcrioSigMgrGetOptionalHeader(pContext->hEcrioSigMgrHandle, pInviteNotifStruct->pOptionalHeaderList,
					EcrioSipHeaderTypeWarning, &pWarningeHeader);
				if ((pWarningeHeader != NULL) && (pWarningeHeader->ppHeaderValues!= NULL) && (pWarningeHeader->numHeaderValues == 3))
				{
					if ((pWarningeHeader->ppHeaderValues[0] != NULL) && (pWarningeHeader->ppHeaderValues[0]->pHeaderValue))
					{
						failedStruct.uWarningCode = pal_StringConvertToUNum(pWarningeHeader->ppHeaderValues[0]->pHeaderValue, NULL, 10);
					}
					if ((pWarningeHeader->ppHeaderValues[1] != NULL) && (pWarningeHeader->ppHeaderValues[1]->pHeaderValue))
					{
						failedStruct.pWarningHostName = (char*)pWarningeHeader->ppHeaderValues[1]->pHeaderValue;
					}
					if ((pWarningeHeader->ppHeaderValues[2] != NULL) && (pWarningeHeader->ppHeaderValues[2]->pHeaderValue))
					{
						failedStruct.pWarningText = (char*)pWarningeHeader->ppHeaderValues[2]->pHeaderValue;
					}
				}
				else
				{
					CPMLOGI(pContext->hLogHandle, KLogTypeGeneral, "%s:%uNo Warning header or invalid string",
						__FUNCTION__, __LINE__);
					if (pWarningeHeader != NULL)
					{
						if (pWarningeHeader->ppHeaderValues == NULL)
						{
							CPMLOGI(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tWarning header exist but ppHeaderValues is NULL",
								__FUNCTION__, __LINE__);
						}
						else
						{
							CPMLOGI(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tWarning header exist but number of header is %d",
								__FUNCTION__, __LINE__, pWarningeHeader->numHeaderValues);
						}
					}
				}
#if 0
				/* Code commented as this was old requirement */
				if (pWarningeHeader == NULL)
				{
					//if it does not contain warnning header set the action to EcrioCPMAction_sendRERegister
					failedStruct.eAction = EcrioCPMActionSendReRegister;
				}
#endif
			}

			//handle for 504 response
			if (pInviteNotifStruct->responseCode == ECRIO_SIGMGR_RESPONSE_CODE_SERVER_TIMEOUT)
			{
				BoolEnum bPresent = Enum_FALSE;
				//check wether the content length is >0 , 
				if (pInviteNotifStruct->contentLength > 0)
				{
					//it has got a message body
					if (pInviteNotifStruct->pMessageBody != NULL &&
						pInviteNotifStruct->pMessageBody->messageBodyType == EcrioSigMgrMessageBodyUnknown)
					{
						EcrioSigMgrUnknownMessageBodyStruct* pUnknwnMsg = (EcrioSigMgrUnknownMessageBodyStruct*)pInviteNotifStruct->pMessageBody->pMessageBody;
						if (pal_StringCompare(pUnknwnMsg->contentType.pHeaderValue, (u_char*)"application/3gpp-ims+xml") == 0)
						{
							bPresent = ec_CPM_Handle3gppIMSXML(pContext->hCPIMHandle, (u_char *)pUnknwnMsg->pBuffer);
							if (bPresent == Enum_TRUE)
							{
								failedStruct.eAction = EcrioCPMActionSendInitialRegister;
							}
						}
					}
				}
			}

			if (pInviteNotifStruct->numPAssertedIdentities > 0)
			{
				failedStruct.uNumOfPAssertedIdentity = pInviteNotifStruct->numPAssertedIdentities;
				pal_MemoryAllocate(sizeof(u_char*)* failedStruct.uNumOfPAssertedIdentity, &failedStruct.ppPAssertedIdentity);
				if (failedStruct.ppPAssertedIdentity == NULL)
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
						__FUNCTION__, __LINE__);
					uError = ECRIO_CPM_MEMORY_ERROR;
					goto Error_Level_01;
				}

				for (int i = 0; i < failedStruct.uNumOfPAssertedIdentity; i++)
				{
					if (pInviteNotifStruct->ppPAssertedIdentity[i]->addrSpec.uriScheme == EcrioSigMgrURISchemeSIP)
					{
						uSigMgrError = EcrioSigMgrFormSipURI(pContext->hEcrioSigMgrHandle, 
							pInviteNotifStruct->ppPAssertedIdentity[i]->addrSpec.u.pSipUri, &failedStruct.ppPAssertedIdentity[i]);
						if (ECRIO_SIGMGR_NO_ERROR != uSigMgrError)
						{
							CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrFormSipURI() uSigMgrError=%u",
								__FUNCTION__, __LINE__, uSigMgrError);
							pal_MemoryFree(&failedStruct.ppPAssertedIdentity);
							uError = ECRIO_CPM_INTERNAL_ERROR;
							goto Error_Level_01;
						}
					}
					else
					{
						uSigMgrError = EcrioSigMgrFormTelURI(pContext->hEcrioSigMgrHandle,
							pInviteNotifStruct->ppPAssertedIdentity[i]->addrSpec.u.pTelUri, &failedStruct.ppPAssertedIdentity[i]);
						if (ECRIO_SIGMGR_NO_ERROR != uSigMgrError)
						{
							CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrFormSipURI() uSigMgrError=%u",
								__FUNCTION__, __LINE__, uSigMgrError);
							pal_MemoryFree(&failedStruct.ppPAssertedIdentity);
							uError = ECRIO_CPM_INTERNAL_ERROR;
							goto Error_Level_01;
						}
					}
				}
			}
			
			cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_SessionConnectFailure;
			cpmNotifStruct.pNotifData = &failedStruct;
			pSession->state = EcrioCPMSessionState_Unused;
			pContext->callbackStruct.pCPMNotificationCallbackFn(pContext->callbackStruct.pCallbackData,
				&cpmNotifStruct);

			ec_CPM_MapDeleteKeyData(pContext->hHashMap, (u_char*)failedStruct.pSessionId);

			ec_CPM_ReleaseCPMSession(pContext, pSession);
			break;
		}
	}

Error_Level_01:
	if (NULL != pMultiPart)
	{
		for (i = 0; i < pMultiPart->uNumOfBody; i++)
		{
			if (pMultiPart->ppBody[i]->pContentType)
			{
				pal_MemoryFree((void **)&pMultiPart->ppBody[i]->pContentType);
			}
			if (pMultiPart->ppBody[i]->pMessageBody)
			{
				pal_MemoryFree((void **)&pMultiPart->ppBody[i]->pMessageBody);
			}
			pal_MemoryFree((void **)&pMultiPart->ppBody[i]);
		}
		pal_MemoryFree((void **)&pMultiPart->ppBody);
		pal_MemoryFree((void **)&pMultiPart);
	}

	if (NULL != pBoundaryStr)
	{
		pal_MemoryFree((void **)&pBoundaryStr);
	}

	if (NULL != pGroupSessionId)
	{
		pal_MemoryFree((void **)&pGroupSessionId);
	}

	if (NULL != pAlias)
	{
		pal_MemoryFree((void **)&pAlias);
	}

	if (failedStruct.ppPAssertedIdentity)
	{
		for (int i = 0; i < failedStruct.uNumOfPAssertedIdentity; i++)
		{
			pal_MemoryFree((void**)&failedStruct.ppPAssertedIdentity[i]);
		}

		pal_MemoryFree((void**)&failedStruct.ppPAssertedIdentity);
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u\tuError=%u", __FUNCTION__, __LINE__, uError);
}

/** Internal function called when an incoming Re-Invite response is received */
void ec_CPM_HandleReInviteMessageResponse
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	u_int32	uSDPError = ECRIO_SDP_NO_ERROR;
	EcrioSigMgrSipMessageStruct	*pInviteNotifStruct = NULL;
	EcrioCPMSessionStruct *pSession = NULL;
	EcrioCPMNotifStruct cpmNotifStruct = { .eNotifCmd = EcrioCPM_Notif_None };
	EcrioCPMFailedSessionStruct failedStruct = { 0 };
	EcrioSDPStringStruct tSDP = { 0 };
	u_int32	i = 0;
	char *pBoundaryStr = NULL;
	u_char *pBoundary = NULL;
	BoolEnum bIsMultipart = Enum_FALSE;
	EcrioCPMMultipartMessageStruct *pMultiPart = NULL;

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pInviteNotifStruct = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;
	if (pInviteNotifStruct == NULL || pInviteNotifStruct->pMandatoryHdrs == NULL)
	{
		uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
		goto Error_Level_01;
	}

	EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, pInviteNotifStruct->pMandatoryHdrs->pCallId, (void**)&pSession);

	if (pSession == NULL)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpSession not found for pCallId=%s",
			__FUNCTION__, __LINE__, pInviteNotifStruct->pMandatoryHdrs->pCallId);
		goto Error_Level_01;
	}

	switch (pInviteNotifStruct->statusCode)
	{
		case ECRIO_SIGMGR_INVITE_IN_PROGRESS:
		{
			/* No action required */
			CPMLOGD(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tReceived %d response for Re-INVITE. Ignore it.",
				__FUNCTION__, __LINE__, pInviteNotifStruct->statusCode);
		}
		break;

		case ECRIO_SIGMGR_RESPONSE_SUCCESS:
		{
			EcrioSigMgrUnknownMessageBodyStruct *pUnknownBody = NULL;

			if ((NULL == pInviteNotifStruct->pMessageBody) || (NULL == pInviteNotifStruct->pMessageBody->pMessageBody))
			{
				uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
				goto Error_Level_01;
			}

			pUnknownBody = (EcrioSigMgrUnknownMessageBodyStruct *)pInviteNotifStruct->pMessageBody->pMessageBody;
			if (NULL == pUnknownBody->pBuffer)
			{
				uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
				goto Error_Level_01;
			}

			pal_MemorySet((void*)&pSession->remoteSDP, 0, sizeof(EcrioSDPInformationStruct));
			pSession->remoteSDP.strings.pData = pSession->pSDPInformationBuffer;
			pSession->remoteSDP.strings.uContainerSize = ECRIO_CPM_SDP_INFO_BUFFER_SIZE;
			pSession->remoteSDP.strings.uSize = 0;

			/** Handle multipart message body */
			EcrioSigMgrHeaderValueStruct *pHdrValue = &pUnknownBody->contentType;
			if (pal_StringCompare(pHdrValue->pHeaderValue, (u_char *)"multipart/mixed") == 0)
			{
				bIsMultipart = Enum_TRUE;
				if (pHdrValue->ppParams && pHdrValue->ppParams[0])
				{
					for (i = 0; i < pHdrValue->numParams; i++)
					{
						if (pal_StringCompare(pHdrValue->ppParams[i]->pParamName, (u_char *)"boundary") == 0)
						{
							pBoundary = pHdrValue->ppParams[i]->pParamValue;
						}
					}
				}
			}

			if (bIsMultipart == Enum_TRUE && pBoundary != NULL)
			{
				pBoundaryStr = (char*)ec_CPM_StringUnquote(pBoundary);
				uError = ec_CPM_ParseMultipartMessage(pContext, &pMultiPart, (char*)pBoundaryStr,
					(char*)pUnknownBody->pBuffer, pUnknownBody->bufferLength);
				if (uError != ECRIO_CPM_NO_ERROR)
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_ParseMultipartMessage() failed with uError=%u",
						__FUNCTION__, __LINE__, uError);
					goto Error_Level_01;
				}

				for (i = 0; i < pMultiPart->uNumOfBody; i++)
				{
					if (pal_StringCompare(pMultiPart->ppBody[i]->pContentType, (u_char *)"application/sdp") == 0)
					{
						tSDP.pData = pMultiPart->ppBody[i]->pMessageBody;
						tSDP.uContainerSize = ECRIO_CPM_SDP_STRING_SIZE;
						tSDP.uSize = pMultiPart->ppBody[i]->uContentLength;
					}
				}
			}
			else
			{
				tSDP.pData = pUnknownBody->pBuffer;
				tSDP.uContainerSize = ECRIO_CPM_SDP_STRING_SIZE;
				tSDP.uSize = pUnknownBody->bufferLength;
			}

			/** Handling received SDP */
			uSDPError = EcrioSDPSessionHandleSDPString(pSession->hSDPSessionHandle, &pSession->remoteSDP, &tSDP);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				uError = ECRIO_CPM_INTERNAL_ERROR;
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSDPSessionHandleSDPString() uSDPError=%u",
					__FUNCTION__, __LINE__, uSDPError);

				goto Error_Level_01;
			}

			if (pSession->remoteSDP.eEvent == ECRIO_SDP_EVENT_IDENTICAL)
			{
				CPMLOGD(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tReceived identical SDP event",
					__FUNCTION__, __LINE__);
			}
			else
			{
				CPMLOGD(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tReceived %d SDP event",
					__FUNCTION__, __LINE__, pSession->remoteSDP.eEvent);
			}
			break;
		}

		case ECRIO_SIGMGR_RESPONSE_REDIRECTED:
		{
		}
		break;

		case ECRIO_SIGMGR_REQUEST_FAILED:
		default:
		{
			BoolEnum skipNotification = Enum_FALSE;

			if (pContext->bIsRelayEnabled == Enum_TRUE)
				failedStruct.pSessionId = (char *)pSession->relayData.pAppCallId;
			else
				failedStruct.pSessionId = (char *)pSession->pCallId;

			failedStruct.uResponseCode = pInviteNotifStruct->responseCode;
			failedStruct.eAction = EcrioCPMActionNone;


			switch (pInviteNotifStruct->responseCode)
			{
			case ECRIO_SIGMGR_RESPONSE_CODE_FORBIDDEN:
			{
				/*handle 403 and 504 response*/
				EcrioSigMgrHeaderStruct *pWarningeHeader = NULL;
				//check if 403 response contains Warning header.
				EcrioSigMgrGetOptionalHeader(pContext->hEcrioSigMgrHandle, pInviteNotifStruct->pOptionalHeaderList,
					EcrioSipHeaderTypeWarning, &pWarningeHeader);
				if (pWarningeHeader == NULL)
				{
					//if it does not contain warnning header set the action to EcrioCPMAction_sendRERegister
					failedStruct.eAction = EcrioCPMActionSendReRegister;
				}
			}
			break;
			case ECRIO_SIGMGR_RESPONSE_CODE_SERVER_TIMEOUT:
			{
				//handle for 504 response

				BoolEnum bPresent = Enum_FALSE;
				//check wether the content length is >0 , 
				if (pInviteNotifStruct->contentLength > 0)
				{
					//it has got a message body
					if (pInviteNotifStruct->pMessageBody != NULL &&
						pInviteNotifStruct->pMessageBody->messageBodyType == EcrioSigMgrMessageBodyUnknown)
					{
						EcrioSigMgrUnknownMessageBodyStruct* pUnknwnMsg = (EcrioSigMgrUnknownMessageBodyStruct*)pInviteNotifStruct->pMessageBody->pMessageBody;
						if (pal_StringCompare(pUnknwnMsg->contentType.pHeaderValue, (u_char*)"application/3gpp-ims+xml") == 0)
						{
							bPresent = ec_CPM_Handle3gppIMSXML(pContext->hCPIMHandle, (u_char *)pUnknwnMsg->pBuffer);
							if (bPresent == Enum_TRUE)
							{
								failedStruct.eAction = EcrioCPMActionSendInitialRegister;
							}
						}
					}
				}
			}
			break;
			case ECRIO_SIGMGR_RESPONSE_CODE_REQUEST_TIMEOUT:
			case ECRIO_SIGMGR_RESPONSE_CODE_CALL_TXN_NOT_EXIST:
			{
				/* If the session refresh request transaction times out or generates a 408 or 481 response, then the UAC sends a BYE request. */
				EcrioSigMgrTerminateSession(pContext->hEcrioSigMgrHandle, pSession->pCallId,
					Enum_FALSE);
				skipNotification = Enum_TRUE;
			}
			break;
			default:
				break;
			}
			
			if (skipNotification == Enum_FALSE)
			{
				cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_SessionConnectFailure;
				cpmNotifStruct.pNotifData = &failedStruct;
				pSession->state = EcrioCPMSessionState_Unused;
				pContext->callbackStruct.pCPMNotificationCallbackFn(pContext->callbackStruct.pCallbackData,
					&cpmNotifStruct);

				ec_CPM_MapDeleteKeyData(pContext->hHashMap, (u_char *)failedStruct.pSessionId);

				ec_CPM_ReleaseCPMSession(pContext, pSession);
			}
		}
		break;
	}

Error_Level_01:
	if (NULL != pMultiPart)
	{
		for (i = 0; i < pMultiPart->uNumOfBody; i++)
		{
			if (pMultiPart->ppBody[i]->pContentType)
			{
				pal_MemoryFree((void **)&pMultiPart->ppBody[i]->pContentType);
			}
			if (pMultiPart->ppBody[i]->pMessageBody)
			{
				pal_MemoryFree((void **)&pMultiPart->ppBody[i]->pMessageBody);
			}
			pal_MemoryFree((void **)&pMultiPart->ppBody[i]);
		}
		pal_MemoryFree((void **)&pMultiPart->ppBody);
	}

	if (NULL != pBoundaryStr)
	{
		pal_MemoryFree((void **)&pBoundaryStr);
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u\tuError=%u", __FUNCTION__, __LINE__, uError);
}

void ec_CPM_MSRPFileCallback
(
	MSRPFileTransferTypeEnum type,
	void *pData,
	void *pContext
)
{
	u_int32 uError = ECRIO_CPM_NO_ERROR;
	u_int32 uCPIMError = ECRIO_CPIM_NO_ERROR;
	u_int32 uStrLen = 0;
	LOGHANDLE hLogHandle = NULL;
	EcrioCPMSessionStruct *pSession = NULL;
	EcrioCPMContextStruct *pCPMContext = NULL;
	EcrioCPMNotifStruct cpmNotifStruct = { .eNotifCmd = EcrioCPM_Notif_None };
	EcrioCPMSessionMessageStruct *pIncSessionMsg = NULL;
	EcrioCPMSessionIDStruct sessionId = { 0 };
	EcrioCPMAcceptedSessionStruct acceptedStruct = { 0 };
	MSRPFileTransferStruct *pFileTransfer = NULL;
	u_char *pSessionId = NULL;
	CPIMMessageStruct cpimMessage = { 0 };

	if ((NULL == pData) || (NULL == pContext))
	{
		return;
	}

	/** Get the CPM structure from the handle */
	pCPMContext = (EcrioCPMContextStruct *)pContext;

	hLogHandle = pCPMContext->hLogHandle;

	CPMLOGI(hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pFileTransfer = (MSRPFileTransferStruct*)pData;
	pSession = (EcrioCPMSessionStruct*)pFileTransfer->pAppData;
	if (pSession == NULL)
	{
		CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tpSession is NULL...", __FUNCTION__, __LINE__);
		goto End_Tag;
	}

	if (pCPMContext->bIsRelayEnabled == Enum_TRUE)
		pSessionId = pSession->relayData.pAppCallId;
	else
		pSessionId = pSession->pCallId;

	if (pSession->bIsLargeMessage == Enum_TRUE && 
		(type == MSRPFileTransferType_Status && pFileTransfer->eStatus != MSRPFileTransferStatus_Empty))
	{
		uCPIMError = EcrioCPIMHandler(pCPMContext->hCPIMHandle, &cpimMessage, pFileTransfer->pData, pFileTransfer->uTotal);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioCPIMHandler failed, uError=%u",
				__FUNCTION__, __LINE__, uCPIMError);
			uError = ECRIO_CPM_INTERNAL_ERROR;
			goto End_Tag;
		}
		pFileTransfer->pData = cpimMessage.buff[0].pMsgBody;
		pFileTransfer->uByteRangeStart = 1;
		pFileTransfer->uByteRangeEnd = cpimMessage.buff[0].uMsgLen;
		pFileTransfer->uTotal = cpimMessage.buff[0].uMsgLen;
		pFileTransfer->pMessageId = cpimMessage.pMsgId;
		pFileTransfer->eStatus = MSRPFileTransferStatus_Complete;
	}
	switch (type)
	{
	case MSRPFileTransferType_Status:
	{
		switch (pFileTransfer->eStatus)
		{
		case MSRPFileTransferStatus_InProgress:
		case MSRPFileTransferStatus_Complete:
		case MSRPFileTransferStatus_Failed:
		{
			pal_MemoryAllocate(sizeof(EcrioCPMSessionMessageStruct), (void**)&pIncSessionMsg);
			if (NULL == pIncSessionMsg)
			{
				CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				goto End_Tag;
			}

            /** Notify the upper layer. */
            cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_IncomingSessionMessage;
            cpmNotifStruct.pNotifData = pIncSessionMsg;

			pIncSessionMsg->pSessionId = (char *)pal_StringCreate(pSessionId, pal_StringLength(pSessionId));
            pIncSessionMsg->pMessageId = (char *)pal_StringCreate(pFileTransfer->pMessageId, pal_StringLength(pFileTransfer->pMessageId));
            pIncSessionMsg->pTransactionId = (char *)pal_StringCreate(pFileTransfer->pTransactionId, pal_StringLength(pFileTransfer->pTransactionId));

			pal_MemoryAllocate(sizeof(EcrioCPMMessageStruct), (void**)&pIncSessionMsg->pMessage);
			if (NULL == pIncSessionMsg->pMessage)
			{
				CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				goto Error_Level_01;
			}
			pal_MemorySet((void*)pIncSessionMsg->pMessage, 0, sizeof(EcrioCPMMessageStruct));

			pIncSessionMsg->pMessage->eContentType = EcrioCPMContentTypeEnum_FileTransferOverMSRP;

			pal_MemoryAllocate(sizeof(EcrioCPMBufferStruct), (void**)&pIncSessionMsg->pMessage->message.pBuffer);
			if (NULL == pIncSessionMsg->pMessage->message.pBuffer)
			{
				CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				goto Error_Level_01;
			}
			pIncSessionMsg->pMessage->message.pBuffer->pMessage = pFileTransfer->pData;
			pIncSessionMsg->pMessage->message.pBuffer->uMessageLen = pFileTransfer->uByteRangeEnd - pFileTransfer->uByteRangeStart + 1;
			pIncSessionMsg->pMessage->message.pBuffer->uByteRangeStart = pFileTransfer->uByteRangeStart;
			pIncSessionMsg->pMessage->message.pBuffer->uByteRangeEnd = pFileTransfer->uByteRangeEnd;
		}
		break;
		case MSRPFileTransferStatus_Empty:
		{
			if (pCPMContext->bIsRelayEnabled == Enum_TRUE)
			{
				/* Populate infor from stored session info */
				acceptedStruct.pConvId = pSession->pConvId;
				acceptedStruct.pSessionId = (char *)pSession->relayData.pAppCallId;
				acceptedStruct.contentTypes = pSession->contentTypes;
				acceptedStruct.pDisplayName = (char*)pSession->pPeerDisplayName;

				cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_SessionEstablished;
				cpmNotifStruct.pNotifData = &acceptedStruct;
			}
		}
		break;
		default:
			break;
		}
	}
	break;
	case MSRPFileTransferType_SendSuccess:
	case MSRPFileTransferType_SendFailure:
	{
		pFileTransfer = (MSRPFileTransferStruct*)pData;
		pSession = (EcrioCPMSessionStruct*)pFileTransfer->pAppData;

		if (type == MSRPFileTransferType_SendSuccess)
			cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_SessionMessageSendSuccess;
		else
			cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_SessionMessageSendFailure;

        cpmNotifStruct.pNotifData = &sessionId;

		uStrLen = (u_int32)(pal_StringLength(pFileTransfer->pMessageId));
		if (uStrLen > 0)
		{
			pal_MemoryAllocate((uStrLen + 1), (void **)&(sessionId.pMessageId));
			if (NULL == sessionId.pMessageId)
			{
				CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				goto End_Tag;
			}
			pal_MemoryCopy(sessionId.pMessageId, (uStrLen + 1), pFileTransfer->pMessageId, uStrLen);
		}
		else
		{
			sessionId.pMessageId = NULL;
			CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpFileTransfer->pMessageId is NULL.",
					__FUNCTION__, __LINE__);
		}
		uStrLen = (u_int32)(pal_StringLength(pFileTransfer->pTransactionId));
		if (uStrLen > 0)
		{
			pal_MemoryAllocate((uStrLen + 1), (void **)&(sessionId.pTransactionId));
			if (NULL == sessionId.pTransactionId)
			{
				CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				goto Error_Level_01;
			}
			pal_MemoryCopy(sessionId.pTransactionId, (uStrLen + 1), pFileTransfer->pTransactionId, uStrLen);
		}
		else
		{
			sessionId.pTransactionId = NULL;
            CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpFileTransfer->pTransactionId is NULL.",
                    __FUNCTION__, __LINE__);
		}

		if (pCPMContext->bIsRelayEnabled == Enum_TRUE)
		{
			uStrLen = (u_int32)(pal_StringLength(pSession->relayData.pAppCallId));
			if (uStrLen > 0)
			{
				pal_MemoryAllocate((uStrLen + 1), (void **)&(sessionId.pSessionId));
				if (NULL == sessionId.pSessionId)
				{
					CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
						__FUNCTION__, __LINE__);
					goto Error_Level_01;
				}
				pal_MemoryCopy(sessionId.pSessionId, (uStrLen + 1), pSession->relayData.pAppCallId, uStrLen);
			}
			else
			{
				sessionId.pSessionId = NULL;
                CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpSession->relayData.pAppCallId is NULL.",
                        __FUNCTION__, __LINE__);
                goto Error_Level_01;
			}
		}
		else
		{		
			uStrLen = (u_int32)(pal_StringLength(pSession->pCallId));
			if (uStrLen > 0)
			{
				pal_MemoryAllocate((uStrLen + 1), (void **)&(sessionId.pSessionId));
				if (NULL == sessionId.pSessionId)
				{
					CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
						__FUNCTION__, __LINE__);
					goto Error_Level_01;
				}
				pal_MemoryCopy(sessionId.pSessionId, (uStrLen + 1), pSession->pCallId, uStrLen);
			}
			else
			{
				sessionId.pSessionId = NULL;
                CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpSession->pCallId is NULL.",
                        __FUNCTION__, __LINE__);
                goto Error_Level_01;
			}
		}

		uStrLen = (u_int32)(pal_StringLength(pFileTransfer->pDescription));
		if (uStrLen > 0)
		{
			pal_MemoryAllocate((uStrLen + 1), (void **)&(sessionId.pDescription));
			if (NULL == sessionId.pDescription)
			{
				CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				goto Error_Level_01;
			}
			pal_MemoryCopy(sessionId.pDescription, (uStrLen + 1), pFileTransfer->pDescription, uStrLen);
		}
		else
		{
			sessionId.pDescription = NULL;
            CPMLOGD(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpFileTransfer->pDescription is NULL.",
                    __FUNCTION__, __LINE__);
		}

		sessionId.uStatusCode = pFileTransfer->uStatusCode;
	}
	break;
	case MSRPFileTransferType_TCPSocketError:
	{
		if (pSession != NULL && (pSession->state == EcrioCPMSessionState_Starting ||
			pSession->state == EcrioCPMSessionState_Active))
		{
			CPMLOGD(hLogHandle, KLogTypeGeneral, "%s:%u\tTerminate session due to socket error",
				__FUNCTION__, __LINE__);

			/** Terminate the session */
			uError = EcrioSigMgrTerminateSession(pCPMContext->hEcrioSigMgrHandle, pSession->pCallId, Enum_FALSE);
			if (ECRIO_SIGMGR_NO_ERROR != uError)
			{
				CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrTerminateSession() uSigMgrError=%u ",
					__FUNCTION__, __LINE__, uError);
			}
			else
			{
				pSession->eTerminationReason = EcrioSessionTerminationReason_MSRPSocketFailure;
			}
		}
	}
	break;
	default:
		break;
	}

	if (cpmNotifStruct.eNotifCmd != EcrioCPM_Notif_None)
	{
		pCPMContext->callbackStruct.pCPMNotificationCallbackFn(pCPMContext->callbackStruct.pCallbackData,
			&cpmNotifStruct);
	}

Error_Level_01:
    ec_CPM_ReleaseCPMNotifStruct(pCPMContext, &cpmNotifStruct);

End_Tag:
	CPMLOGI(hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
}

void ec_CPM_MSRPTextCallback
(
	MSRPMessageTypeEnum type,
	void *pData,
	void *pContext
)
{
	u_int32 uError = ECRIO_CPM_NO_ERROR;
	LOGHANDLE hLogHandle = NULL;
	EcrioCPMSessionStruct *pSession = NULL;
	EcrioCPMContextStruct *pCPMContext = NULL;
	EcrioCPMNotifStruct cpmNotifStruct = { .eNotifCmd = EcrioCPM_Notif_None };
	EcrioCPMSessionIDStruct sessionId = { 0 };
	EcrioCPMAcceptedSessionStruct acceptedStruct = { 0 };

	if ((NULL == pData) || (NULL == pContext))
	{
		return;
	}

	/** Get the CPM structure from the handle */
	pCPMContext = (EcrioCPMContextStruct *)pContext;

	hLogHandle = pCPMContext->hLogHandle;

	CPMLOGI(hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	CPMLOGI(hLogHandle, KLogTypeGeneral, "%s:%u switch case type is %d", __FUNCTION__, __LINE__, type);

	switch (type)
	{
	case MSRPMessageType_SendSuccess:
	{
		MSRPTextMessageStruct *pMSRPText = NULL;
		u_int32 uStrLen = 0;
		pMSRPText = (MSRPTextMessageStruct*)pData;
		pSession = (EcrioCPMSessionStruct*)pMSRPText->pAppData;

		cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_SessionMessageSendSuccess;
        cpmNotifStruct.pNotifData = &sessionId;

		uStrLen = (u_int32)(pal_StringLength(pMSRPText->pMessageId));
		if (uStrLen > 0)
		{
			pal_MemoryAllocate((uStrLen + 1), (void **)&(sessionId.pMessageId));
			if (NULL == sessionId.pMessageId)
			{
				CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
                goto End_Tag;
			}
			pal_MemoryCopy(sessionId.pMessageId, (uStrLen + 1), pMSRPText->pMessageId, uStrLen);
		}
		else
		{
			sessionId.pMessageId = NULL;
            CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpMSRPText->pMessageId is NULL.",
                    __FUNCTION__, __LINE__);
		}

		if (pCPMContext->bIsRelayEnabled == Enum_TRUE)
		{
			uStrLen = (u_int32)(pal_StringLength(pSession->relayData.pAppCallId));
			if (uStrLen > 0)
			{
				pal_MemoryAllocate((uStrLen + 1), (void **)&(sessionId.pSessionId));
				if (NULL == sessionId.pSessionId)
				{
					CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
						__FUNCTION__, __LINE__);
                    goto Error_Level_01;
				}
				pal_MemoryCopy(sessionId.pSessionId, (uStrLen + 1), pSession->relayData.pAppCallId, uStrLen);
			}
			else
			{
				sessionId.pSessionId = NULL;
                CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpSession->relayData.pAppCallId is NULL.",
                        __FUNCTION__, __LINE__);
                goto Error_Level_01;
			}
		}
		else
		{		
			uStrLen = (u_int32)(pal_StringLength(pSession->pCallId));
			if (uStrLen > 0)
			{
				pal_MemoryAllocate((uStrLen + 1), (void **)&(sessionId.pSessionId));
				if (NULL == sessionId.pSessionId)
				{
					CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
						__FUNCTION__, __LINE__);
					goto Error_Level_01;
				}
				pal_MemoryCopy(sessionId.pSessionId, (uStrLen + 1), pSession->pCallId, uStrLen);
			}
			else
			{
				sessionId.pSessionId = NULL;
                CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpSession->pCallId is NULL.",
                        __FUNCTION__, __LINE__);
                goto Error_Level_01;
			}
		}

		sessionId.pDescription = NULL;
		sessionId.uStatusCode = 200;
	}
	break;
	case MSRPMessageType_SendFailure:
	{
		MSRPFailureResultStruct* pMSRPFailResult = NULL;
		u_int32 uStrLen = 0;

		pMSRPFailResult = (MSRPFailureResultStruct*)pData;
		pSession = (EcrioCPMSessionStruct*)pMSRPFailResult->pAppData;

		cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_SessionMessageSendFailure;
        cpmNotifStruct.pNotifData = &sessionId;
		
		uStrLen = (u_int32)(pal_StringLength(pMSRPFailResult->pMessageId));
		if (uStrLen > 0)
		{
			pal_MemoryAllocate((uStrLen + 1), (void **)&(sessionId.pMessageId));
			if (NULL == sessionId.pMessageId)
			{
				CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				goto End_Tag;
			}
			pal_MemoryCopy(sessionId.pMessageId, (uStrLen + 1), pMSRPFailResult->pMessageId, uStrLen);
		}
		else
		{
			sessionId.pMessageId = NULL;
            CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpMSRPText->pMessageId is NULL.",
                    __FUNCTION__, __LINE__);
		}

		if (pCPMContext->bIsRelayEnabled == Enum_TRUE)
		{
			uStrLen = (u_int32)(pal_StringLength(pSession->relayData.pAppCallId));
			if (uStrLen > 0)
			{
				pal_MemoryAllocate((uStrLen + 1), (void **)&(sessionId.pSessionId));
				if (NULL == sessionId.pSessionId)
				{
					CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
						__FUNCTION__, __LINE__);
					goto Error_Level_01;
				}
				pal_MemoryCopy(sessionId.pSessionId, (uStrLen + 1), pSession->relayData.pAppCallId, uStrLen);
			}
			else
			{
				sessionId.pSessionId = NULL;
                CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpSession->relayData.pAppCallId is NULL.",
                        __FUNCTION__, __LINE__);
                goto Error_Level_01;
			}
		}
		else
		{		
			uStrLen = (u_int32)(pal_StringLength(pSession->pCallId));
			if (uStrLen > 0)
			{
				pal_MemoryAllocate((uStrLen + 1), (void **)&(sessionId.pSessionId));
				if (NULL == sessionId.pSessionId)
				{
					CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
						__FUNCTION__, __LINE__);
					goto Error_Level_01;
				}
				pal_MemoryCopy(sessionId.pSessionId, (uStrLen + 1), pSession->pCallId, uStrLen);
			}
			else
			{
				sessionId.pSessionId = NULL;
                CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpSession->pCallId is NULL.",
                        __FUNCTION__, __LINE__);
                goto Error_Level_01;
			}
		}

		uStrLen = (u_int32)(pal_StringLength(pMSRPFailResult->pDescription));
		if (uStrLen > 0)
		{
			pal_MemoryAllocate((uStrLen + 1), (void **)&(sessionId.pDescription));
			if (NULL == sessionId.pDescription)
			{
				CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				goto Error_Level_01;
			}
			pal_MemoryCopy(sessionId.pDescription, (uStrLen + 1), pMSRPFailResult->pDescription, uStrLen);
		}
		else
		{
			sessionId.pDescription = NULL;
		}
		sessionId.uStatusCode = pMSRPFailResult->uStatusCode;
	}
	break;
	case MSRPMessageType_CPIM:
	{
		MSRPTextMessageStruct *pMSRPText = NULL;

		pMSRPText = (MSRPTextMessageStruct*)pData;
		pSession = (EcrioCPMSessionStruct*)pMSRPText->pAppData;
		if (pCPMContext->bIsRelayEnabled == Enum_TRUE)
			ec_CPM_HandleCPIMMessage(pCPMContext, pSession, pMSRPText, pSession->relayData.pAppCallId, &cpmNotifStruct);
		else
			ec_CPM_HandleCPIMMessage(pCPMContext, pSession, pMSRPText, pSession->pCallId, &cpmNotifStruct);
	}
	break;
	case MSRPMessageType_Composition:
	{
		EcrioCPMSessionMessageStruct *pSessionMsgStruct = NULL;
		MSRPTextMessageStruct *pMSRPText = NULL;

		MSRPIsComposingStruct composing = { 0 };
		EcrioCPMComposingStruct *pComposing = NULL;
		u_int32 uStrLen = 0;

		pMSRPText = (MSRPTextMessageStruct*)pData;
		pSession = (EcrioCPMSessionStruct*)pMSRPText->pAppData;

		composing = pMSRPText->u.comp;

		pal_MemoryAllocate(sizeof(EcrioCPMComposingStruct), (void**)&pComposing);
		if (NULL == pComposing)
		{
			CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
				__FUNCTION__, __LINE__);
			return;
		}
		if (composing.pContent)
		{
			pComposing->pContent = pal_StringCreate(composing.pContent, pal_StringLength(composing.pContent));
		}
		if (composing.pLastActive)
		{
			pComposing->pLastActive = pal_StringCreate(composing.pLastActive, pal_StringLength(composing.pLastActive));
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

		pal_MemoryAllocate(sizeof(EcrioCPMSessionMessageStruct), (void**)&pSessionMsgStruct);
		if (NULL == pSessionMsgStruct)
		{
			CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
				__FUNCTION__, __LINE__);
			return;
		}

        cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_IMComposing;
        cpmNotifStruct.pNotifData = pSessionMsgStruct;

		pal_MemorySet((void*)pSessionMsgStruct, 0, sizeof(EcrioCPMSessionMessageStruct));
		pal_MemoryAllocate(sizeof(EcrioCPMMessageStruct), (void**)&pSessionMsgStruct->pMessage);
		if (NULL == pSessionMsgStruct->pMessage)
		{
			CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
				__FUNCTION__, __LINE__);
			goto Error_Level_01;
		}
		pal_MemorySet((void*)pSessionMsgStruct->pMessage, 0, sizeof(EcrioCPMMessageStruct));
		pSessionMsgStruct->pMessage->message.pComposing = pComposing;
		pSessionMsgStruct->pMessage->eContentType = EcrioCPMContentTypeEnum_Composing;
		if (pCPMContext->bIsRelayEnabled == Enum_TRUE)
			pSessionMsgStruct->pSessionId = (char *)pal_StringCreate(pSession->relayData.pAppCallId, pal_StringLength(pSession->relayData.pAppCallId));
		else
			pSessionMsgStruct->pSessionId = (char *)pal_StringCreate(pSession->pCallId, pal_StringLength(pSession->pCallId));
		uStrLen = (u_int32)(pal_StringLength(pMSRPText->pMessageId));
		if (uStrLen > 0)
		{
			pal_MemoryAllocate((uStrLen + 1), (void **)&(pSessionMsgStruct->pMessageId));
			if (NULL == pSessionMsgStruct->pMessageId)
			{
				CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				goto Error_Level_01;
			}
			pal_MemoryCopy(pSessionMsgStruct->pMessageId, (uStrLen + 1), pMSRPText->pMessageId, uStrLen);
		}
		else
		{
			pSessionMsgStruct->pMessageId = NULL;
            CPMLOGE(pCPMContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpSessionMsgStruct->pMessageId is NULL.",
                    __FUNCTION__, __LINE__);
		}
	}
	break;
	case MSRPMessageType_Empty:
	{
		if (pCPMContext->bIsRelayEnabled == Enum_TRUE)
		{
			MSRPTextMessageStruct *pMSRPText = NULL;

			pMSRPText = (MSRPTextMessageStruct*)pData;
			pSession = (EcrioCPMSessionStruct*)pMSRPText->pAppData;

			/* Populate infor from stored session info */
			acceptedStruct.pConvId = pSession->pConvId;
			acceptedStruct.pSessionId = (char *)pSession->relayData.pAppCallId;
			acceptedStruct.contentTypes = pSession->contentTypes;
			acceptedStruct.pDisplayName = (char*)pSession->pPeerDisplayName;
			
			cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_SessionEstablished;
			cpmNotifStruct.pNotifData = &acceptedStruct;
		}
	}
	break;
	case MSRPMessageType_TCPSocketError:
	{
		MSRPTextMessageStruct *pMSRPText = NULL;

		pMSRPText = (MSRPTextMessageStruct*)pData;
		pSession = (EcrioCPMSessionStruct*)pMSRPText->pAppData;

		if (pSession != NULL && (pSession->state == EcrioCPMSessionState_Starting ||
			pSession->state == EcrioCPMSessionState_Active))
		{
			CPMLOGD(hLogHandle, KLogTypeGeneral, "%s:%u\tTerminate session due to socket error",
				__FUNCTION__, __LINE__);

			/** Terminate the session */
			uError = EcrioSigMgrTerminateSession(pCPMContext->hEcrioSigMgrHandle, pSession->pCallId, Enum_FALSE);
			if (ECRIO_SIGMGR_NO_ERROR != uError)
			{
				CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrTerminateSession() uSigMgrError=%u ",
					__FUNCTION__, __LINE__, uError);
			}
			else
			{
				pSession->eTerminationReason = EcrioSessionTerminationReason_MSRPSocketFailure;
			}
		}
	}
	break;
	case MSRPMessageType_SendTimeout:
	{
		MSRPTextMessageStruct *pMSRPText = NULL;

		pMSRPText = (MSRPTextMessageStruct*)pData;
		pSession = (EcrioCPMSessionStruct*)pMSRPText->pAppData;

		if (pSession != NULL/* && pSession->bIsGroupChat == Enum_TRUE*/) {
			CPMLOGD(hLogHandle, KLogTypeGeneral, "%s:%u\tTerminating session due to MSRPMessageType_SendTimeout", __FUNCTION__, __LINE__);
			pSession->eTerminationReason = EcrioSessionTerminationReason_MSRPSocketFailure;
			EcrioSigMgrTerminateSession(pCPMContext->hEcrioSigMgrHandle, pSession->pCallId,
				Enum_FALSE);
		}
		else
		{
			CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tMSRPMessageType_SendTimeout - pSession is NULL", __FUNCTION__, __LINE__);
		}
	}
	break;
	default:
		break;
	}

	if (cpmNotifStruct.eNotifCmd != EcrioCPM_Notif_None)
	{
		pCPMContext->callbackStruct.pCPMNotificationCallbackFn(pCPMContext->callbackStruct.pCallbackData,
			&cpmNotifStruct);
	}

Error_Level_01:
    ec_CPM_ReleaseCPMNotifStruct(pCPMContext, &cpmNotifStruct);

End_Tag:
    CPMLOGI(hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
}

void ec_CPM_MSRPAuthCallback
(
	MSRPAuthResultEnum type,
	void *pData,
	void *pContext
)
{
	LOGHANDLE hLogHandle = NULL;
	EcrioCPMSessionStruct *pSession = NULL;
	EcrioCPMContextStruct *pCPMContext = NULL;

	if ((NULL == pData) || (NULL == pContext))
	{
		return;
	}

	/** Get the CPM structure from the handle */
	pCPMContext = (EcrioCPMContextStruct *)pContext;

	hLogHandle = pCPMContext->hLogHandle;

	CPMLOGI(hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	switch (type)
	{
	case MSRPAuthResult_Success:
	{
		MSRPAuthResultStruct *pMSRPAuth = NULL;
		u_char *pPath = NULL;
		u_int32 length = 0;
		pMSRPAuth = (MSRPAuthResultStruct*)pData;
		pSession = (EcrioCPMSessionStruct*)pMSRPAuth->pAppData;
		CPMLOGD(hLogHandle, KLogTypeGeneral, "%s:%u\t MSRP Auth Result Successful", __FUNCTION__, __LINE__);
		CPMLOGD(hLogHandle, KLogTypeGeneral, "%s:%u\t Use-Path: %s", __FUNCTION__, __LINE__, pMSRPAuth->pUsePath);

		/* Start new session only for initial AUTH success response.
		TODO - If use-path is differs from previous one then update session with RE-INVITE. */
		if (pSession != NULL && pSession->state == EcrioCPMSessionState_Unused)
		{
			length = pal_StringLength(pMSRPAuth->pUsePath);
			length += pal_StringLength(pSession->relayData.pLocalPath);
			length += 2; /* Space and null termination */

			pal_MemoryAllocate(length, (void**)&pPath);
			if (NULL == pPath)
			{
				CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.", __FUNCTION__, __LINE__);
				return;
			}
			if (NULL == pal_StringNCopy(pPath, length, pMSRPAuth->pUsePath, pal_StringLength(pMSRPAuth->pUsePath))) /* Add relay path */
			{
				CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.", __FUNCTION__, __LINE__);
				return;
			}
			if (NULL == pal_StringNConcatenate(pPath, length - pal_StringLength((u_char*)pPath), (const u_char *)" ", 1))/* Space between 2 MSRP URL */
			{
				CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.", __FUNCTION__, __LINE__);
				return;
			}
			if (NULL == pal_StringNConcatenate(pPath, length - pal_StringLength((u_char*)pPath), pSession->relayData.pLocalPath, pal_StringLength(pSession->relayData.pLocalPath))) /* Add local path */
			{
				CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.", __FUNCTION__, __LINE__);
				return;
			}

			pal_MemoryFree((void**)&pSession->relayData.pLocalPath);
			pSession->relayData.pLocalPath = pPath;

			pSession->relayData.pUsePath = pal_StringCreate(pMSRPAuth->pUsePath, pal_StringLength(pMSRPAuth->pUsePath));

			/* Initiate Session as we have received use-path */
			ec_CPM_StartSession(pCPMContext, NULL, pSession, (char **)&pSession->relayData.pAppCallId);
		}
	}
	break;
	case MSRPAuthResult_Failure:
	{
		MSRPAuthResultStruct *pMSRPAuth = NULL;
		EcrioCPMNotifStruct cpmNotifStruct = { .eNotifCmd = EcrioCPM_Notif_None };
		EcrioCPMFailedSessionStruct failedStruct = { 0 };

		pMSRPAuth = (MSRPAuthResultStruct*)pData;
		pSession = (EcrioCPMSessionStruct*)pMSRPAuth->pAppData;

		CPMLOGD(hLogHandle, KLogTypeGeneral, "%s:%u\t MSRP Auth Result Failure", __FUNCTION__, __LINE__);
		if (pSession != NULL)
		{
			failedStruct.pSessionId = (char *)pSession->relayData.pAppCallId;
			failedStruct.uResponseCode = ECRIO_SIGMGR_RESPONSE_CODE_SERVER_INTERNAL_ERROR;
			failedStruct.eAction = EcrioCPMActionNone;

			cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_SessionConnectFailure;
			cpmNotifStruct.pNotifData = &failedStruct;
			pSession->state = EcrioCPMSessionState_Unused;
			pCPMContext->callbackStruct.pCPMNotificationCallbackFn(pCPMContext->callbackStruct.pCallbackData,
				&cpmNotifStruct);

			ec_CPM_MapDeleteKeyData(pCPMContext->hHashMap, (u_char *)failedStruct.pSessionId);

			ec_CPM_ReleaseCPMSession(pCPMContext, pSession);
		}
	}
	break;
	case MSRPAuthResult_None:
	{
		CPMLOGD(hLogHandle, KLogTypeGeneral, "%s:%u\t MSRP Auth Result None", __FUNCTION__, __LINE__);
	}
	break;
	}
}
#endif

BoolEnum ec_CPM_Handle3gppIMSXML(CPIM_HANDLE hLogHandle, u_char *pMsgBody)
{
	u_char *pAltService = NULL;
	u_char *pType = NULL;
	u_char *pRestoration = NULL;
	u_char *pAction = NULL;  
	u_char *pInitialReg = NULL;
	BoolEnum bResult = Enum_FALSE;

	CPMLOGI(hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pAltService = pal_StringFindSubString(pMsgBody, (const u_char *)"<alternative-service>");
	if (pAltService != NULL)
	{
		//<alternative-service> element is present,check for <type> element
		pType = pal_StringFindSubString(pAltService, (const u_char *)"<type>");
		if (pType != NULL)
		{
			//check for "restoration" value
			pRestoration = pal_StringFindSubString(pType, (const u_char *)"restoration");
			if (pRestoration == NULL)
			{
				bResult = Enum_FALSE;
			}
		}
		else
		{
			bResult = Enum_FALSE;
		}

		pAction = pal_StringFindSubString(pAltService, (const u_char *)"<action>");
		if (pAction != NULL)
		{
			pInitialReg = pal_StringFindSubString(pAction, (const u_char *)"initial-registration");
			if (pInitialReg == NULL)
			{
				bResult = Enum_FALSE;
			}
		}
		else
		{
			bResult = Enum_FALSE;
		}
		//everything is present.
		bResult = Enum_TRUE;
	}
	else
	{
		bResult = Enum_FALSE;
	}

	CPMLOGI(hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return bResult;
}

void ec_CPM_HandleInstantMessageRequest
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	u_int32 uSigMgrError = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrSipMessageStruct *pMessageReq = NULL;
	EcrioSigMgrUnknownMessageBodyStruct *pUnknownBody = NULL;
	EcrioCPMNotifStruct cpmNotifStruct = { .eNotifCmd = EcrioCPM_Notif_None };
	u_char *pFrom = NULL;
	EcrioSigMgrHeaderStruct *pHdr = NULL;
	EcrioCPMConversationsIdStruct convIdStruct = { 0 };
	EcrioCPMIncomingStandAloneMessageStruct incPagerMsg = { 0 };
	EcrioCPMPrivMngCmdRespStruct privMngCmdResp = { 0 };
	CPIMMessageStruct cpimMsgStruct = { 0 };
	EcrioCPMBufferStruct cpmBuffStruct = { 0 };
	EcrioCPMIMDispoNotifStruct imdispNotif = { 0 };
	EcrioCPMMessageStruct message = { 0 };
	BoolEnum bIsPrivMngResp = Enum_FALSE;
	EcrioCPMParsedXMLStruct *pParsedXml = NULL;
	u_char *pDisplayName = NULL;
	EcrioSigMgrHeaderStruct *pPrivacyHeader = NULL;
	u_int32 i = 0;
	BoolEnum bIsChatbotSA = Enum_FALSE;

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Find the current CPM session structure */

	pMessageReq = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;

	/** Get remote IP & port fropm SDP */
	if ((NULL == pMessageReq->pMessageBody) ||
		(NULL == pMessageReq->pMessageBody->pMessageBody))
	{
		uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
		goto Error_Level_01;
	}

	/** Verify Privacy header */
	EcrioSigMgrGetOptionalHeader(pContext->hEcrioSigMgrHandle, pMessageReq->pOptionalHeaderList, EcrioSipHeaderTypePrivacy, &pPrivacyHeader);

	/** If Privacy header fields are indicated "id" or "header", then originating party ID is restricted. */
	if (pPrivacyHeader != NULL)
	{
		for (i = 0; i < pPrivacyHeader->numHeaderValues; i++)
		{
			if (pPrivacyHeader->ppHeaderValues && pPrivacyHeader->ppHeaderValues[i])
			{
				if ((pal_SubString(pPrivacyHeader->ppHeaderValues[i]->pHeaderValue, (u_char *)"id")) ||
					(pal_SubString(pPrivacyHeader->ppHeaderValues[i]->pHeaderValue, (u_char *)"header")))
				{
					pFrom = pal_StringCreate(ECRIO_CPM_CALLER_ID_RESTRICTED, pal_StringLength(ECRIO_CPM_CALLER_ID_RESTRICTED));
					pDisplayName = pal_StringCreate(ECRIO_CPM_CALLER_ID_RESTRICTED, pal_StringLength(ECRIO_CPM_CALLER_ID_RESTRICTED));
				}
			}
		}
	}
	if (pFrom == NULL)
	{
		if (pContext->bEnablePAI == Enum_TRUE)
		{
			if (pMessageReq->ppPAssertedIdentity != NULL)
			{
				EcrioSigMgrGetOriginator(pContext->hEcrioSigMgrHandle, pMessageReq->ppPAssertedIdentity[0], &pFrom);
				if (pMessageReq->ppPAssertedIdentity[0]->pDisplayName != NULL)
				{
					pDisplayName = pal_StringCreate(pMessageReq->ppPAssertedIdentity[0]->pDisplayName,
						pal_StringLength(pMessageReq->ppPAssertedIdentity[0]->pDisplayName));
				}
			}
		}
		if (pFrom == NULL)
		{
			EcrioSigMgrGetOriginator(pContext->hEcrioSigMgrHandle, &pMessageReq->pMandatoryHdrs->pFrom->nameAddr, &pFrom);
		}
		if ((pDisplayName == NULL) && (pMessageReq->pMandatoryHdrs->pFrom->nameAddr.pDisplayName != NULL))
		{
			pDisplayName = pal_StringCreate(pMessageReq->pMandatoryHdrs->pFrom->nameAddr.pDisplayName,
				pal_StringLength(pMessageReq->pMandatoryHdrs->pFrom->nameAddr.pDisplayName));
		}

		if ((ECRIO_SIGMGR_NO_ERROR != uSigMgrError) || (NULL == pFrom))
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrGetOriginator() uSigMgrError=%u",
				__FUNCTION__, __LINE__, uSigMgrError);
			uError = ECRIO_CPM_INTERNAL_ERROR;
			goto Error_Level_01;
		}
	}

	/** Get conversation IDs this will populated in UAE*/
	EcrioSigMgrGetOptionalHeader(pContext->hEcrioSigMgrHandle,
		pMessageReq->pOptionalHeaderList,
		EcrioSipHeaderTypeConversationID,
		&pHdr);
	if (pHdr != NULL)
	{
		if (pHdr->ppHeaderValues && pHdr->ppHeaderValues[0] && pHdr->ppHeaderValues[0]->pHeaderValue)
		{
			convIdStruct.pConversationId = pHdr->ppHeaderValues[0]->pHeaderValue;
		}
	}

	EcrioSigMgrGetOptionalHeader(pContext->hEcrioSigMgrHandle,
		pMessageReq->pOptionalHeaderList,
		EcrioSipHeaderTypeContributionID,
		&pHdr);
	if (pHdr != NULL)
	{
		if (pHdr->ppHeaderValues && pHdr->ppHeaderValues[0] && pHdr->ppHeaderValues[0]->pHeaderValue)
		{
			convIdStruct.pContributionId = pHdr->ppHeaderValues[0]->pHeaderValue;
		}
	}

	EcrioSigMgrGetOptionalHeader(pContext->hEcrioSigMgrHandle,
		pMessageReq->pOptionalHeaderList,
		EcrioSipHeaderTypeInReplyToContributionID,
		&pHdr);
	if (pHdr != NULL)
	{
		if (pHdr->ppHeaderValues && pHdr->ppHeaderValues[0] && pHdr->ppHeaderValues[0]->pHeaderValue)
		{
			convIdStruct.pInReplyToContId = pHdr->ppHeaderValues[0]->pHeaderValue;
		}
	}

	/** Get Accept-Contact header value */
	EcrioSigMgrGetOptionalHeader(pContext->hEcrioSigMgrHandle,
		pMessageReq->pOptionalHeaderList,
		EcrioSipHeaderTypeAcceptContact,
		&pHdr);
	if (pHdr != NULL)
	{
		if (pHdr->ppHeaderValues[0]->ppParams != NULL)
		{
			for (i = 0; i < pHdr->ppHeaderValues[0]->numParams; i++)
			{
				if (pal_StringCompare(pHdr->ppHeaderValues[0]->ppParams[i]->pParamName, (u_char*)"+g.3gpp.iari-ref") == 0)
				{
					if (pal_StringFindSubString(pHdr->ppHeaderValues[0]->ppParams[i]->pParamValue, ECRIO_CPM_3GPP_FEATURE_CHATBOT_STANDALONE) != NULL)
					{
						bIsChatbotSA = Enum_TRUE;
						break;
					}
				}
			}
		}
	}

	pUnknownBody = (EcrioSigMgrUnknownMessageBodyStruct *)pMessageReq->pMessageBody->pMessageBody;
	if (NULL == pUnknownBody->pBuffer)
	{
		uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
		goto Error_Level_01;
	}

	/** Check Chatbot IARI */
	bIsPrivMngResp = ec_CPMIsPrivacyManagementResponse(pContext, pMessageReq);

	if (bIsPrivMngResp == Enum_TRUE)
	{
		/** Handle Privacy Management XML */
		if (pal_StringCompare(pUnknownBody->contentType.pHeaderValue, (u_char*)"application/vnd.gsma.rcsalias-mgmt+xml") == 0)
		{
			uError = ec_CPM_ParseXmlDocument(pContext, &pParsedXml, (char*)pUnknownBody->pBuffer, pUnknownBody->bufferLength);
			if (uError != ECRIO_CPM_NO_ERROR)
			{
				uError = ECRIO_CPM_INTERNAL_ERROR;
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_ParseXmlDocument() failed with uError=%u",
					__FUNCTION__, __LINE__, uError);
				goto Error_Level_01;
			}

			u_char* pValue = NULL;
			pValue = ec_CPM_GetXmlAttributeValue(pParsedXml, (u_char *)"Command-ID");
			if (pValue != NULL)
			{
				privMngCmdResp.pCommandID = (char*)pal_StringCreate((u_char*)pValue, pal_StringLength((u_char*)pValue));
			}

			pValue = NULL;
			pValue = ec_CPM_GetXmlAttributeValue(pParsedXml, (u_char *)"result");
			if (pal_StringCompare((u_char *)pValue, (u_char *)"on") == 0)
			{
				privMngCmdResp.eResp = EcrioCPMPrivMngCmdResp_On;
			}
			else if (pal_StringCompare((u_char *)pValue, (u_char *)"off") == 0)
			{
				privMngCmdResp.eResp = EcrioCPMPrivMngCmdResp_Off;
			}
			else if (pal_StringCompare((u_char *)pValue, (u_char *)"failure") == 0)
			{
				privMngCmdResp.eResp = EcrioCPMPrivMngCmdResp_Failure;
			}
			else
			{
				privMngCmdResp.eResp = EcrioCPMPrivMngCmdResp_None;
			}

			/** Notfiy upper layer */
			cpmNotifStruct.pNotifData = &privMngCmdResp;
			cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_PrivacyManagementCommands;
			pContext->callbackStruct.pCPMNotificationCallbackFn(pContext->callbackStruct.pCallbackData,
				&cpmNotifStruct);

			if (privMngCmdResp.pCommandID != NULL)
			{
				pal_MemoryFree((void **)&privMngCmdResp.pCommandID);
			}
		}
	}
	else
	{
		uError = EcrioCPIMHandler(pContext->hCPIMHandle, &cpimMsgStruct, pUnknownBody->pBuffer, pUnknownBody->bufferLength);
		if (ECRIO_CPIM_NO_ERROR != uError)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioCPIMHandler() error=%u",
				__FUNCTION__, __LINE__, uError);

			uError = ECRIO_CPM_INTERNAL_ERROR;
			goto Error_Level_01;
		}

		if (cpimMsgStruct.buff[0].eContentType == CPIMContentType_Text)
		{
			incPagerMsg.pConvId = &convIdStruct;
			incPagerMsg.pReqFrom = (char *)pFrom;

			/** Check CPIM-From header */
			if (bIsChatbotSA == Enum_TRUE)
			{
				EcrioSigMgrSipMessageStruct msgRsp = { .eMethodType = EcrioSipMessageTypeNone };
				BoolEnum bIsError = Enum_FALSE;

				msgRsp.eMethodType = EcrioSipMessageTypeMessage;
				msgRsp.pMandatoryHdrs = pMessageReq->pMandatoryHdrs;

				/** If does not contain the Chatbot Role feature-tag in CPIM-From header, then
				 *  return 606 error. */
				if (pal_StringFindSubString(cpimMsgStruct.pFromAddr, ECRIO_CPM_3GPP_FEATURE_ISBOT) == NULL)
				{
					msgRsp.pReasonPhrase = (u_char *)ECRIO_SIGMGR_RESPONSE_NOT_ACCEPTABLE_EVERYWHERE;
					msgRsp.responseCode = ECRIO_SIGMGR_RESPONSE_CODE_NOT_ACCEPTABLE_EVERYWHERE;
					bIsError = Enum_TRUE;
				}
				/** Otherwise, return 200 OK */
				else
				{
					msgRsp.pReasonPhrase = (u_char *)ECRIO_SIGMGR_RESPONSE_OK;
					msgRsp.responseCode = ECRIO_SIGMGR_RESPONSE_CODE_OK;
				}

				uSigMgrError = EcrioSigMgrSendInstantMessageResponse(pContext->hEcrioSigMgrHandle, &msgRsp);
				if (ECRIO_SIGMGR_NO_ERROR != uSigMgrError)
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrSendInstantMessageResponse() uSigMgrError=%u",
						__FUNCTION__, __LINE__, uSigMgrError);
					uError = ECRIO_CPM_INTERNAL_ERROR;
					goto Error_Level_01;
				}

				/** If error, do not notify the received message */
				if (bIsError == Enum_TRUE)
				{
					goto Error_Level_01;
				}
			}

			incPagerMsg.pMessage = &message;
			//incPagerMsg.imdnConfig
			incPagerMsg.pMessage->imdnConfig = EcrioCPMIMDispositionConfigNone;
			if ((cpimMsgStruct.dnType & CPIMDispoNotifReq_Negative) == CPIMDispoNotifReq_Negative)
			{
				incPagerMsg.pMessage->imdnConfig |= EcrioCPMIMDispositionConfigNegativeDelivery;
			}

			if ((cpimMsgStruct.dnType & CPIMDispoNotifReq_Positive) == CPIMDispoNotifReq_Positive)
			{
				incPagerMsg.pMessage->imdnConfig |= EcrioCPMIMDispositionConfigPositiveDelivery;
			}

			if ((cpimMsgStruct.dnType & CPIMDispoNotifReq_Display) == CPIMDispoNotifReq_Display)
			{
				incPagerMsg.pMessage->imdnConfig |= EcrioCPMIMDispositionConfigDisplay;
			}

			incPagerMsg.pMessage->pIMDNMsgId = (char *)cpimMsgStruct.pMsgId;
			incPagerMsg.pMessage->message.pBuffer = &cpmBuffStruct;
			incPagerMsg.pDisplayName = (char*)pDisplayName;

			incPagerMsg.bIsChatbot = bIsChatbotSA;

			cpmBuffStruct.pMessage = cpimMsgStruct.buff[0].pMsgBody;
			cpmBuffStruct.uMessageLen = cpimMsgStruct.buff[0].uMsgLen;
			cpmBuffStruct.pDate = cpimMsgStruct.pDateTime;

			/** Notfiy upper layer */
			cpmNotifStruct.pNotifData = &incPagerMsg;
			cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_PagerMessageReceived;
			pContext->callbackStruct.pCPMNotificationCallbackFn(pContext->callbackStruct.pCallbackData,
				&cpmNotifStruct);

		}
		/* It is assumed aggregation is done only for IMDN messages and multiple buffer is received
		with only IMDN messages. */
		else if (cpimMsgStruct.buff[0].eContentType == CPIMContentType_IMDN)
		{
			IMDNDispoNotifStruct iMDN = { 0 };

			imdispNotif.uNumOfNotifBody = cpimMsgStruct.uNumOfBuffers;

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected(cpimMsgStruct.uNumOfBuffers, sizeof(EcrioCPMIMDispoNotifBodyStruct)) == Enum_TRUE)
			{
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto Error_Level_01;
			}

			pal_MemoryAllocate(sizeof(EcrioCPMIMDispoNotifBodyStruct) * cpimMsgStruct.uNumOfBuffers, (void**)&imdispNotif.pCPMIMDispoNotifBody);
			if (imdispNotif.pCPMIMDispoNotifBody == NULL)
			{
				uError = ECRIO_CPM_MEMORY_ERROR;
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
					__FUNCTION__, __LINE__);
				goto Error_Level_01;
			}

			for (u_int32 counter = 0; counter < cpimMsgStruct.uNumOfBuffers; counter++)
			{
				pal_MemorySet((void*)&iMDN, 0, sizeof(IMDNDispoNotifStruct));
				uError = EcrioIMDNHandler(pContext->hIMDNHandle, &iMDN, cpimMsgStruct.buff[counter].pMsgBody, cpimMsgStruct.buff[counter].uMsgLen);
				if (ECRIO_IMDN_NO_ERROR != uError)
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioIMDNHandler() error=%u",
						__FUNCTION__, __LINE__, uError);

					uError = ECRIO_CPM_INTERNAL_ERROR;
					goto Error_Level_01;
				}

				imdispNotif.pCPMIMDispoNotifBody[counter].pIMDNMsgId = iMDN.pMsgId;

				/* map the delivery enum */
				switch (iMDN.eDelivery)
				{
				case IMDNDeliveryNotif_Delivered:
				{
					imdispNotif.pCPMIMDispoNotifBody[counter].eDelivery = EcrioCPMIMDNDeliveryNotif_Delivered;
				}
				break;
				case IMDNDeliveryNotif_Failed:
				{
					imdispNotif.pCPMIMDispoNotifBody[counter].eDelivery = EcrioCPMIMDNDeliveryNotif_Failed;
				}
				break;
				case IMDNDeliveryNotif_Forbidden:
				{
					imdispNotif.pCPMIMDispoNotifBody[counter].eDelivery = EcrioCPMIMDNDeliveryNotif_Forbidden;
				}
				break;
				case IMDNDeliveryNotif_Error:
				{
					imdispNotif.pCPMIMDispoNotifBody[counter].eDelivery = EcrioCPMIMDNDeliveryNotif_Error;
				}
				break;
				default:
				{
					imdispNotif.pCPMIMDispoNotifBody[counter].eDelivery = EcrioCPMIMDNDeliveryNotif_None;
				}
				break;
				}
				/* map the Disposition enum */
				switch (iMDN.eDisNtf)
				{
				case IMDNDisNtfTypeReq_Delivery:
				{
					imdispNotif.pCPMIMDispoNotifBody[counter].eDisNtf = EcrioCPMIMDispositionNtfTypeReq_Delivery;
				}
				break;
				case IMDNDisNtfTypeReq_Display:
				{
					imdispNotif.pCPMIMDispoNotifBody[counter].eDisNtf = EcrioCPMIMDispositionNtfTypeReq_Display;
				}
				break;
				default:
				{
					imdispNotif.pCPMIMDispoNotifBody[counter].eDisNtf = EcrioCPMIMDispositionNtfTypeReq_None;
				}
				break;
				}
				/* map the display enum */
				switch (iMDN.eDisplay)
				{
				case IMDNDisplayNotif_Displayed:
				{
					imdispNotif.pCPMIMDispoNotifBody[counter].eDisplay = EcrioCPMIMDNDisplayNotif_Displayed;
				}
				break;
				case IMDNDisplayNotif_Forbidden:
				{
					imdispNotif.pCPMIMDispoNotifBody[counter].eDisplay = EcrioCPMIMDNDisplayNotif_Forbidden;
				}
				break;
				case IMDNDisplayNotif_Error:
				{
					imdispNotif.pCPMIMDispoNotifBody[counter].eDisplay = EcrioCPMIMDNDisplayNotif_Error;
				}
				break;
				default:
				{
					imdispNotif.pCPMIMDispoNotifBody[counter].eDisplay = EcrioCPMIMDNDisplayNotif_None;
				}
				break;
				}
				imdispNotif.pCPMIMDispoNotifBody[counter].pRecipientUri = iMDN.pRecipientUri;
				imdispNotif.pCPMIMDispoNotifBody[counter].pOriginalRecipientUri = iMDN.pOriginalRecipientUri;
				imdispNotif.pCPMIMDispoNotifBody[counter].pDate = iMDN.pDateTime;
			}
			/* Who sent this IMDN message. */		
			imdispNotif.pDestURI = (char*)cpimMsgStruct.pToAddr;
			imdispNotif.pSenderURI = (char*)cpimMsgStruct.pFromAddr;

			/** Notfiy upper layer */
			cpmNotifStruct.pNotifData = &imdispNotif;
			cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_IMDNDisposition;
			pContext->callbackStruct.pCPMNotificationCallbackFn(pContext->callbackStruct.pCallbackData,
				&cpmNotifStruct);
		}
		else
		{
			/*TODO: Other content types handling*/
		}
	}

Error_Level_01:

	if (NULL != imdispNotif.pCPMIMDispoNotifBody)
	{
		pal_MemoryFree((void **)&imdispNotif.pCPMIMDispoNotifBody);
	}

	if (NULL != pFrom)
	{
		pal_MemoryFree((void **)&pFrom);
	}

	if (NULL != pDisplayName)
	{
		pal_MemoryFree((void **)&pDisplayName);
	}

	if (NULL != pParsedXml)
	{
		ec_CPM_ReleaseParsedXmlStruct(&pParsedXml, Enum_TRUE);
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u\tuError=%u", __FUNCTION__, __LINE__, uError);
}

#if 0
/*Function to populate RemoteMSRPInfo Struct*/
void ec_CPM_PopulateRemoteMSRPInfo
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMSessionStruct *pSession,
	EcrioSDPInformationStruct tInfo
)
{
	pSession->remoteSDPInfo.isIPv6 = tInfo.remoteIp.eAddressType == ECRIO_SDP_ADDRESS_TYPE_IP6 ? Enum_TRUE : Enum_FALSE;
	pal_MemorySet(pSession->pSDPWorkingBuffer, 0, ECRIO_CPM_SDP_INFO_BUFFER_SIZE);
	pSession->remoteSDPInfo.pRemoteIp = pSession->pSDPWorkingBuffer;
	if (NULL == pal_StringNCopy(pSession->remoteSDPInfo.pRemoteIp, ECRIO_CPM_SDP_INFO_BUFFER_SIZE, tInfo.remoteIp.pAddress, 
		pal_StringLength(tInfo.remoteIp.pAddress)))
	{
		return;
	}
	pSession->remoteSDPInfo.uRemotePort = tInfo.remoteStream[0].uMediaPort;
	pSession->remoteSDPInfo.pRemotePath = pSession->pSDPWorkingBuffer + pal_StringLength(pSession->remoteSDPInfo.pRemoteIp) + 2;
	if (NULL == pal_StringNCopy(pSession->remoteSDPInfo.pRemotePath, ECRIO_CPM_SDP_INFO_BUFFER_SIZE, tInfo.remoteStream[0].u.msrp.pPath,
		pal_StringLength(tInfo.remoteStream[0].u.msrp.pPath)))
	{
		return;
	}

	if (tInfo.remoteStream[0].u.msrp.eSetup == ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_ACTIVE ||
		tInfo.remoteStream[0].u.msrp.eSetup == ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_ACTPASS)
	{
		pSession->eLocalConnectionType = MSRPConnectionType_TCP_Server;
	}
	else
	{
		if (pContext->bMsrpOverTLS == Enum_TRUE)
		{
			pSession->eLocalConnectionType = MSRPConnectionType_TLS_Client;
		}
		else
		{
			pSession->eLocalConnectionType = MSRPConnectionType_TCP_Client;
		}
	}
	

	return;
}
#endif

BoolEnum ec_CPMIsLargeMessage
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrSipMessageStruct *pSipMessage
)
{
	u_int32 i = 0;
	EcrioSigMgrHeaderStruct *pHeader = NULL;
	BoolEnum result = Enum_FALSE;
	
	EcrioSigMgrGetOptionalHeader(pContext->hEcrioSigMgrHandle, pSipMessage->pOptionalHeaderList, EcrioSipHeaderTypeAcceptContact, &pHeader);
	if (pHeader != NULL)
	{
		if (pHeader->ppHeaderValues[0]->ppParams != NULL)
		{
			for (i = 0; i < pHeader->ppHeaderValues[0]->numParams; i++)
			{
				if (pal_StringCompare(pHeader->ppHeaderValues[0]->ppParams[i]->pParamName, (u_char*)"+g.3gpp.icsi-ref") == 0)
				{
					if (pal_StringFindSubString(pHeader->ppHeaderValues[0]->ppParams[i]->pParamValue, ECRIO_CPM_3GPP_FEATURE_LARGEMSG) != NULL)
					{
						result = Enum_TRUE;
						break;
					}					
				}
			}
		}
	}

	return result;
}

BoolEnum ec_CPMIsPrivacyManagementResponse
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrSipMessageStruct *pSipMessage
)
{
	u_int32 i = 0, j = 0;
	EcrioSigMgrHeaderStruct *pHeader = NULL;
	BoolEnum result = Enum_FALSE;

	/** Check Accept-Contact header. */
	EcrioSigMgrGetOptionalHeader(pContext->hEcrioSigMgrHandle, pSipMessage->pOptionalHeaderList, EcrioSipHeaderTypeAcceptContact, &pHeader);
	if (pHeader != NULL)
	{
		if (pHeader->ppHeaderValues[0]->ppParams != NULL)
		{
			for (i = 0; i < pHeader->ppHeaderValues[0]->numParams; i++)
			{
				if (pal_StringCompare(pHeader->ppHeaderValues[0]->ppParams[i]->pParamName, (u_char*)"+g.3gpp.iari-ref") == 0)
				{
					if (pal_StringFindSubString(pHeader->ppHeaderValues[0]->ppParams[i]->pParamValue, ECRIO_CPM_3GPP_FEATURE_ALIAS) != NULL)
					{
						result = Enum_TRUE;
						break;
					}
				}
			}
		}
	}

	/** If Chatbot IARI does not contain in Accept-Contact header, then check Contact header. */
	if (result != Enum_TRUE)
	{
		if (pSipMessage->pContact)
		{
			for (i = 0; i < pSipMessage->pContact->numContactUris; i++)
			{
				if (pSipMessage->pContact->ppContactDetails[i])
				{
					for (j = 0; j < pSipMessage->pContact->ppContactDetails[i]->numParams; j++)
					{
						if (pSipMessage->pContact->ppContactDetails[i]->ppParams[j])
						{
							if (pal_StringCompare(pSipMessage->pContact->ppContactDetails[i]->ppParams[j]->pParamName, (u_char *)"+g.3gpp.iari-ref") == 0)
							{
								if (pal_StringFindSubString(pSipMessage->pContact->ppContactDetails[i]->ppParams[j]->pParamValue, ECRIO_CPM_3GPP_FEATURE_ALIAS) != NULL)
								{
									result = Enum_TRUE;
									break;
								}
							}
						}
					}
				}
			}
		}
	}

	return result;
}

u_int32 ec_CPM_GetAliasParamFromPAI
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrNameAddrStruct *pPAI,
	EcrioCPMAliasingParamStruct **ppAlias
)
{
	u_int32 uError = ECRIO_CPM_NO_ERROR;
	u_int32 i = 0;
	EcrioCPMAliasingParamStruct *pAlias = NULL;

	if (pContext == NULL || pPAI == NULL || ppAlias == NULL)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tInvalid parameter.", __FUNCTION__, __LINE__);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto END;
	}

	if (pPAI->addrSpec.uriScheme == EcrioSigMgrURISchemeSIP)
	{
		EcrioSigMgrSipURIStruct *pSipUri = pPAI->addrSpec.u.pSipUri;
		if (pSipUri != NULL)
		{
			for (i = 0; i < pSipUri->numURIParams; i++)
			{
				if (pal_StringICompare(pSipUri->ppURIParams[i]->pParamName, (u_char *)"tk") == 0)
				{
					if (pAlias == NULL)
					{
						pal_MemoryAllocate(sizeof(EcrioCPMAliasingParamStruct), (void **)&pAlias);
						if (pAlias == NULL)
						{
							uError = ECRIO_CPM_MEMORY_ERROR;
							CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
								__FUNCTION__, __LINE__);
							goto END;
						}
						pal_MemorySet((void*)pAlias, 0, sizeof(EcrioCPMAliasingParamStruct));
					}

					if (pal_StringICompare(pSipUri->ppURIParams[i]->pParamValue, (u_char *)"on") == 0)
					{
						pAlias->eTkParam = EcrioCPMTkParam_On;
					}
					else if (pal_StringICompare(pSipUri->ppURIParams[i]->pParamValue, (u_char *)"off") == 0)
					{
						pAlias->eTkParam = EcrioCPMTkParam_Off;
					}
				}
				else if (pal_StringICompare(pSipUri->ppURIParams[i]->pParamName, (u_char *)"aliason") == 0)
				{
					if (pAlias == NULL)
					{
						pal_MemoryAllocate(sizeof(EcrioCPMAliasingParamStruct), (void **)&pAlias);
						if (pAlias == NULL)
						{
							uError = ECRIO_CPM_MEMORY_ERROR;
							CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
								__FUNCTION__, __LINE__);
							goto END;
						}
						pal_MemorySet((void*)pAlias, 0, sizeof(EcrioCPMAliasingParamStruct));
					}

					pAlias->bIsAliason = Enum_TRUE;
				}
			}
		}
	}
	else if (pPAI->addrSpec.uriScheme == EcrioSigMgrURISchemeTEL)
	{
		EcrioSigMgrTelURIStruct *pTelUri = pPAI->addrSpec.u.pTelUri;
		if (pTelUri != NULL)
		{
			if (pTelUri->subscriberType == EcrioSigMgrTelSubscriberGlobal)
			{
				for (i = 0; i < pTelUri->u.pGlobalNumber->numTelPar; i++)
				{
					if (pTelUri->u.pGlobalNumber->ppTelPar[i] != NULL &&
						pTelUri->u.pGlobalNumber->ppTelPar[i]->telParType == EcrioSigMgrTelParParameter)
					{
						if (pTelUri->u.pGlobalNumber->ppTelPar[i]->u.pParameter != NULL)
						{
							if (pal_StringICompare(pTelUri->u.pGlobalNumber->ppTelPar[i]->u.pParameter->pParamName, (u_char *)"tk") == 0)
							{
								if (pAlias == NULL)
								{
									pal_MemoryAllocate(sizeof(EcrioCPMAliasingParamStruct), (void **)&pAlias);
									if (pAlias == NULL)
									{
										uError = ECRIO_CPM_MEMORY_ERROR;
										CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
											__FUNCTION__, __LINE__);
										goto END;
									}
									pal_MemorySet((void*)pAlias, 0, sizeof(EcrioCPMAliasingParamStruct));
								}

								if (pal_StringICompare(pTelUri->u.pGlobalNumber->ppTelPar[i]->u.pParameter->pParamValue, (u_char *)"on") == 0)
								{
									pAlias->eTkParam = EcrioCPMTkParam_On;
								}
								else if (pal_StringICompare(pTelUri->u.pGlobalNumber->ppTelPar[i]->u.pParameter->pParamValue, (u_char *)"off") == 0)
								{
									pAlias->eTkParam = EcrioCPMTkParam_Off;
								}
							}
							else if (pal_StringICompare(pTelUri->u.pGlobalNumber->ppTelPar[i]->u.pParameter->pParamName, (u_char *)"aliason") == 0)
							{
								if (pAlias == NULL)
								{
									pal_MemoryAllocate(sizeof(EcrioCPMAliasingParamStruct), (void **)&pAlias);
									if (pAlias == NULL)
									{
										uError = ECRIO_CPM_MEMORY_ERROR;
										CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
											__FUNCTION__, __LINE__);
										goto END;
									}
									pal_MemorySet((void*)pAlias, 0, sizeof(EcrioCPMAliasingParamStruct));
								}

								pAlias->bIsAliason = Enum_TRUE;
							}
						}
					}
				}
			}
			else if (pTelUri->subscriberType == EcrioSigMgrTelSubscriberLocal)
			{
				for (i = 0; i < pTelUri->u.pLocalNumber->numTelPar2; i++)
				{
					if (pTelUri->u.pLocalNumber->ppTelPar2[i] != NULL &&
						pTelUri->u.pLocalNumber->ppTelPar2[i]->telParType == EcrioSigMgrTelParParameter)
					{
						if (pTelUri->u.pLocalNumber->ppTelPar2[i]->u.pParameter != NULL)
						{
							if (pal_StringICompare(pTelUri->u.pLocalNumber->ppTelPar2[i]->u.pParameter->pParamName, (u_char *)"tk") == 0)
							{
								if (pAlias == NULL)
								{
									pal_MemoryAllocate(sizeof(EcrioCPMAliasingParamStruct), (void **)&pAlias);
									if (pAlias == NULL)
									{
										uError = ECRIO_CPM_MEMORY_ERROR;
										CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
											__FUNCTION__, __LINE__);
										goto END;
									}
									pal_MemorySet((void*)pAlias, 0, sizeof(EcrioCPMAliasingParamStruct));
								}

								if (pal_StringICompare(pTelUri->u.pLocalNumber->ppTelPar2[i]->u.pParameter->pParamValue, (u_char *)"on") == 0)
								{
									pAlias->eTkParam = EcrioCPMTkParam_On;
								}
								else if (pal_StringICompare(pTelUri->u.pLocalNumber->ppTelPar2[i]->u.pParameter->pParamValue, (u_char *)"off") == 0)
								{
									pAlias->eTkParam = EcrioCPMTkParam_Off;
								}
							}
							else if (pal_StringICompare(pTelUri->u.pLocalNumber->ppTelPar2[i]->u.pParameter->pParamName, (u_char *)"aliason") == 0)
							{
								if (pAlias == NULL)
								{
									pal_MemoryAllocate(sizeof(EcrioCPMAliasingParamStruct), (void **)&pAlias);
									if (pAlias == NULL)
									{
										uError = ECRIO_CPM_MEMORY_ERROR;
										CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
											__FUNCTION__, __LINE__);
										goto END;
									}
									pal_MemorySet((void*)pAlias, 0, sizeof(EcrioCPMAliasingParamStruct));
								}

								pAlias->bIsAliason = Enum_TRUE;
							}
						}
					}
				}
			}
		}
	}

	if (pAlias != NULL)
	{
		*ppAlias = pAlias;
	}

END:
	return uError;
}

u_int32 ec_CPMSendInviteResponse
(
	EcrioCPMContextStruct *pContext,
	u_char *pCallId,
	u_char *pReasonPhrase,
	u_int32 eResponseCode
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	EcrioSigMgrSipMessageStruct* pInviteResp = NULL;

	pal_MemoryAllocate(sizeof(EcrioSigMgrSipMessageStruct), (void **)&pInviteResp);
	if (NULL == pInviteResp)
	{
		return ECRIO_CPM_MEMORY_ERROR;
	}

	pInviteResp->pReasonPhrase = pReasonPhrase;
	pInviteResp->responseCode = eResponseCode;

	if (pInviteResp->pMandatoryHdrs == NULL)
	{
		pal_MemoryAllocate((u_int32)sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&pInviteResp->pMandatoryHdrs);
		if (NULL == pInviteResp->pMandatoryHdrs)
		{
			pal_MemoryFree((void**)&pInviteResp);
			return ECRIO_CPM_MEMORY_ERROR;
		}
	}
	pInviteResp->pMandatoryHdrs->pCallId = pal_StringCreate(pCallId, pal_StringLength(pCallId));

	uError = EcrioSigMgrSendInviteResponse(pContext->hEcrioSigMgrHandle, pInviteResp);
	pInviteResp->pReasonPhrase = NULL;
	EcrioSigMgrStructRelease(pContext->hEcrioSigMgrHandle, EcrioSigMgrStructType_SipMessage, (void **)&pInviteResp, Enum_TRUE);
	return uError;
}

#if 0
/** Internal function called when an ACK request is received */
void ec_CPM_HandleAckMessageRequest
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	EcrioCPMSessionStruct *pSession = NULL;
	EcrioSigMgrSipMessageStruct *pAckReq = NULL;
	EcrioCPMNotifStruct cpmNotifStruct = { .eNotifCmd = EcrioCPM_Notif_None };
	EcrioCPMAcceptedSessionStruct acceptedStruct = { 0 };
	EcrioCPMConversationsIdStruct convIds = { 0 };

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pAckReq = (EcrioSigMgrSipMessageStruct*)pSigMgrMessageStruct->pData;
	if (pAckReq != NULL && pAckReq->pMandatoryHdrs != NULL)
	{
		EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, pAckReq->pMandatoryHdrs->pCallId, (void**)&pSession);
		if (pSession == NULL)
		{
			uError = ECRIO_CPM_INTERNAL_ERROR;
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpSession not found for pCallId=%s",
				__FUNCTION__, __LINE__, pAckReq->pMandatoryHdrs->pCallId);
			goto Error_Level_01;
		}
	}
	else
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpAckReq=NULL or pAckReq->pMandatoryHdrs=NULL",
			__FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	/* Do this only for active session i.e, INVITE/200 OK/ACK only */
	if (pSession->state == EcrioCPMSessionState_Active)
	{
		/* MSRP Session setup based on https://tools.ietf.org/html/rfc4975#section-5.4,
		https://tools.ietf.org/html/rfc4975#section-4 and http://www.qtc.jp/3GPP/Specs/24247-910.pdf @ page 33 item #29 */
		/* Check the setup state and if its active then try to setup MSRP sesstion as TCP Client. */
		if (pSession->eLocalConnectionType == MSRPConnectionType_TCP_Client || 
			pSession->eLocalConnectionType == MSRPConnectionType_TLS_Client)
		{
			uError = ec_CPM_StartMSRPSession(pContext, pSession);
			if (uError != ECRIO_CPM_NO_ERROR)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_StartMSRPSession() error=%u",
					__FUNCTION__, __LINE__, uError);
				goto Error_Level_01;
			}
		}

		/** Notfiy upper layer */
		acceptedStruct.pSessionId = (char *)pSession->pCallId;
		if (pAckReq->pConvId)
		{
			acceptedStruct.pConvId = &convIds;
			convIds.pContributionId = pAckReq->pConvId->pContributionId;
			convIds.pConversationId = pAckReq->pConvId->pConversationId;
			convIds.pInReplyToContId = pAckReq->pConvId->pInReplyToContId;
		}
		else
			acceptedStruct.pConvId = pSession->pConvId;

		if (pSession->bIsGroupChat == Enum_TRUE ||
			pSession->pGroupSessionId != NULL)
		{
			acceptedStruct.pGroupSessionId = (char *)pSession->pGroupSessionId;
		}
		acceptedStruct.bIsClosedGroupChat = pSession->bIsClosedGroupChat;
		acceptedStruct.contentTypes = pSession->contentTypes;
		acceptedStruct.pDisplayName = (char*)pSession->pPeerDisplayName;
		cpmNotifStruct.pNotifData = &acceptedStruct;
		cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_SessionEstablished;
		pContext->callbackStruct.pCPMNotificationCallbackFn(pContext->callbackStruct.pCallbackData,
			&cpmNotifStruct);
	}
	/* Release the session by handling ACK since we are returning an error response */
	else if (pSession->state == EcrioCPMSessionState_Unused)
	{
		goto Error_Level_01;
	}
	goto End_Tag;

Error_Level_01:
	if (pSession != NULL)
	{
		ec_CPM_MapDeleteKeyData(pContext->hHashMap, (u_char*)pAckReq->pMandatoryHdrs->pCallId);
		ec_CPM_ReleaseCPMSession(pContext, pSession);
	}
End_Tag:
	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u\tuError=%u", __FUNCTION__, __LINE__, uError);
}

/** Internal function called when an incoming BYE Response Notification is received */
void ec_CPM_HandleSessionEndResponse
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
)
{
	EcrioSigMgrSipMessageStruct	*pSessionEndNotifStruct = NULL;
	EcrioCPMSessionStruct *pSession = NULL;
	EcrioCPMNotifStruct cpmNotifStruct = { .eNotifCmd = EcrioCPM_Notif_None };
	EcrioCPMReasonStruct reasonStruct = { 0 };

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Find the current CPM session structure */

	pSessionEndNotifStruct = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;

	if (pSessionEndNotifStruct != NULL)
	{
		if ((pSessionEndNotifStruct->responseCode / 100) == 1)
		{
			/** No operation for 1xx BYE response */
			CPMLOGI(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\t%d/BYE response is received -- do not notify the received 1xx response to upper layer",
				__FUNCTION__, __LINE__, pSessionEndNotifStruct->responseCode);
			return;
		}

		if (pSessionEndNotifStruct->pMandatoryHdrs != NULL)
		{
			EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, pSessionEndNotifStruct->pMandatoryHdrs->pCallId, (void**)&pSession);
		}
	} else{
		/** Print error and return */
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\t pSessionEndNotifStruct is NULL",
				__FUNCTION__, __LINE__);

		return;
	}

	if (pSession == NULL)
	{
		/** Print error and return */
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\t No matching session found for incoming BYE request",
			__FUNCTION__, __LINE__);

		return;
	}

	if (pContext->bIsRelayEnabled == Enum_TRUE)
		reasonStruct.pSessionId = (char *)pSession->relayData.pAppCallId;
	else
		reasonStruct.pSessionId = (char *)pSession->pCallId;

	reasonStruct.uResponseCode = pSessionEndNotifStruct->responseCode;
	reasonStruct.uReasonCause = pSessionEndNotifStruct->causeCode;
	reasonStruct.pReasonText = NULL;

	if (pSession->eTerminationReason == EcrioSessionTerminationReason_MSRPSocketFailure)
	{
		reasonStruct.uReasonCause = ECRIO_CPM_SESSION_TERMINATION_REASON_1001;
		reasonStruct.pReasonText = ECRIO_CPM_SESSION_TERMINATION_REASON_TEXT_1001;
	}

	/** Notify the upper layer. */
	cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_SessionEnded;
	cpmNotifStruct.pNotifData = &reasonStruct;

	pContext->callbackStruct.pCPMNotificationCallbackFn(pContext->callbackStruct.pCallbackData,
		&cpmNotifStruct);

	ec_CPM_MapDeleteKeyData(pContext->hHashMap, (u_char *)reasonStruct.pSessionId);

	ec_CPM_ReleaseCPMSession(pContext, pSession);

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
}

u_int32 ec_CPM_HandleSessionRefreshRequest
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
)
{
	u_int32 uError = ECRIO_CPM_NO_ERROR;
	u_int32 uSigError = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrSipMessageStruct	*pInviteNotifStruct = NULL;
	EcrioSigMgrSipMessageStruct inviteReq = { .eMethodType = EcrioSipMessageTypeNone };
	EcrioSigMgrSipMessageStruct updateReq = { .eMethodType = EcrioSipMessageTypeNone };
	EcrioSigMgrMessageBodyStruct msgBody = { .messageBodyType = EcrioSigMgrMessageBodySDP };
	EcrioSigMgrUnknownMessageBodyStruct unknownMsgBody = { 0 };
	EcrioSDPStringStruct tSDP = { 0 };
	EcrioCPMSessionStruct *pSession = NULL;
	BoolEnum bUpdateAllow = Enum_FALSE;
	
	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pInviteNotifStruct = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;
	if (pInviteNotifStruct != NULL)
	{
		EcrioSigMgrCheckUpdateSessionRefresh(pContext->hEcrioSigMgrHandle, pInviteNotifStruct->pMandatoryHdrs->pCallId, &bUpdateAllow);
		if (bUpdateAllow == Enum_TRUE)
		{
			updateReq.eMethodType = EcrioSipMessageTypeUpdate;
			/* Send UPDATE for session refresh */
			uSigError = EcrioSigMgrSendUpdate(pContext->hEcrioSigMgrHandle,
				pInviteNotifStruct->pMandatoryHdrs->pCallId,
				&updateReq);
			if (ECRIO_SIGMGR_NO_ERROR != uSigError)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral,
					"%s:%u\tEcrioSigMgrSendUpdate() error=%u",
					__FUNCTION__, __LINE__, uSigError);
				uError = ECRIO_CPM_INTERNAL_ERROR;
				goto Error_Level_01;
			}
		}
		else
		{
			EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, pInviteNotifStruct->pMandatoryHdrs->pCallId, (void **)&pSession);
			if (pSession == NULL)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tSession not found for pCallId=%s",
					__FUNCTION__, __LINE__, pInviteNotifStruct->pMandatoryHdrs->pCallId);
				goto Error_Level_01;
			}

			/* Send INVITE for session refresh */
			pSession->localSDP.eEvent = ECRIO_SDP_EVENT_IDENTICAL;
			pal_MemorySet((void*)pSession->pSDPInformationBuffer, 0, ECRIO_CPM_SDP_INFO_BUFFER_SIZE);
			pSession->localSDP.strings.pData = pSession->pSDPInformationBuffer;
			pSession->localSDP.strings.uContainerSize = ECRIO_CPM_SDP_INFO_BUFFER_SIZE;
			pSession->localSDP.strings.uSize = 0;

			pal_MemorySet((void*)pSession->pSDPStringBuffer, 0, ECRIO_CPM_SDP_STRING_SIZE);
			tSDP.pData = pSession->pSDPStringBuffer;
			tSDP.uContainerSize = ECRIO_CPM_SDP_STRING_SIZE;
			tSDP.uSize = 0;

			uError = EcrioSDPSessionGenerateSDPString(pSession->hSDPSessionHandle, &pSession->localSDP, &tSDP);
			if (uError != ECRIO_SDP_NO_ERROR)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral,
					"%s:%u\tEcrioSDPSessionGenerateSDPString() uSDPError=%u",
					__FUNCTION__, __LINE__, uError);
				uError = ECRIO_CPM_INTERNAL_ERROR;
				goto Error_Level_01;
			}

			unknownMsgBody.bufferLength = tSDP.uSize;
			unknownMsgBody.pBuffer = tSDP.pData;

			/* content-type : application-sdp
			*/
			unknownMsgBody.contentType.numParams = 0;
			unknownMsgBody.contentType.ppParams = NULL;
			unknownMsgBody.contentType.pHeaderValue = (u_char *)"application/sdp";

			msgBody.messageBodyType = EcrioSigMgrMessageBodyUnknown;
			msgBody.pMessageBody = &unknownMsgBody;
			inviteReq.pMessageBody = &msgBody;
			inviteReq.eMethodType = EcrioSipMessageTypeInvite;

			uSigError = EcrioSigMgrSendInvite(pContext->hEcrioSigMgrHandle,
				&inviteReq,
				&pInviteNotifStruct->pMandatoryHdrs->pCallId);
			if (ECRIO_SIGMGR_NO_ERROR != uSigError)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral,
					"%s:%u\tEcrioSigMgrSendInvite() error=%u",
					__FUNCTION__, __LINE__, uSigError);

				uError = ECRIO_CPM_INTERNAL_ERROR;
				goto Error_Level_01;
			}
		}
	}
	else
	{
		uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\t pInviteNotifStruct is NULL",__FUNCTION__, __LINE__);
	}
 
Error_Level_01:

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uError;
}

void ec_CPM_HandleSubscribeResponse
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
)
{
	EcrioSigMgrSipMessageStruct	*pSubscribeRsp = NULL;
	EcrioCPMSessionStruct *pSession = NULL;
	EcrioCPMNotifStruct cpmNotifStruct = { .eNotifCmd = EcrioCPM_Notif_None };
	EcrioCPMReasonStruct reasonStruct = { 0 };

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Find the current CPM session structure */

	pSubscribeRsp = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;

	if (pSubscribeRsp != NULL)
	{
		EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, pSubscribeRsp->pMandatoryHdrs->pCallId, (void**)&pSession);
		if (pSession == NULL)
		{
			CPMLOGW(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tCouldn't find session handle from call-id:%s",
				__FUNCTION__, __LINE__, pSubscribeRsp->pMandatoryHdrs->pCallId);
			reasonStruct.pSessionId = NULL;
		}
		else
		{
			if (pContext->bIsRelayEnabled == Enum_TRUE)
				reasonStruct.pSessionId = (char *)pSession->relayData.pAppCallId;
			else
				reasonStruct.pSessionId = (char *)pSession->pCallId;
		}

		if ((pSubscribeRsp->responseCode / 100) == 2)
		{
			/** Success for 2xx SUBSCRIBE response */
			if (pSubscribeRsp->pExpires != NULL && *(pSubscribeRsp->pExpires) == 0)
			{
				cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_UnSubscribeSendSuccess;
			}
			else
			{
				cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_SubscribeSendSuccess;
			}
		}
		else if ((pSubscribeRsp->responseCode / 100) > 2)
		{
			/** Failure for 3xx/4xx/5xx/6xx SUBSCRIBE response */
			if (pSubscribeRsp->pExpires != NULL && *(pSubscribeRsp->pExpires) == 0)
			{
				cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_UnSubscribeSendFailure;
			}
			else
			{
				cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_SubscribeSendFailure;
			}
		}

		reasonStruct.uResponseCode = pSubscribeRsp->responseCode;
		reasonStruct.uReasonCause = pSubscribeRsp->responseCode;
		reasonStruct.pReasonText = NULL;
	

		/** Notify the upper layer. */
		cpmNotifStruct.pNotifData = &reasonStruct;

		pContext->callbackStruct.pCPMNotificationCallbackFn(pContext->callbackStruct.pCallbackData,
			&cpmNotifStruct);
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
}

void ec_CPM_HandleNotifyRequest
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
)
{
	u_int32 i = 0;
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	EcrioCPMParsedXMLStruct *pStruct = NULL;
	EcrioSigMgrSipMessageStruct	*pNotifyReq = NULL;
	EcrioSigMgrHeaderStruct *pHeader = NULL;
	EcrioCPMSessionStruct *pSession = NULL;
	EcrioCPMNotifStruct cpmNotifStruct = { .eNotifCmd = EcrioCPM_Notif_None };
	EcrioSigMgrUnknownMessageBodyStruct *pUnknownMsg = NULL;
	EcrioCPMGroupChatInfoStruct info = { 0 };
	BoolEnum bIsGzip = Enum_FALSE;
	u_char *pBuf = NULL;
	u_char *pData = NULL;
	char *pBoundaryStr = NULL;
	u_int32 uLen = 0;
	u_int32 uDataLen = 0;

	BoolEnum bIsMultipart = Enum_FALSE;
	u_char* pBoundary = NULL;
	EcrioCPMMultipartMessageStruct *pMultiPart = NULL;

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pNotifyReq = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;

	uError = EcrioSigMgrSendNotifyResponse(pContext->hEcrioSigMgrHandle, pNotifyReq, ECRIO_SIGMGR_RESPONSE_OK, ECRIO_SIGMGR_RESPONSE_CODE_OK);
	if (uError != ECRIO_SIGMGR_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrSendNotifyResponse() failed with error = %u",
			__FUNCTION__, __LINE__, uError);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}

	if (pNotifyReq->contentLength > 0)
	{
		if (pNotifyReq->pMessageBody != NULL &&
			pNotifyReq->pMessageBody->messageBodyType == EcrioSigMgrMessageBodyUnknown)
		{
			pUnknownMsg = (EcrioSigMgrUnknownMessageBodyStruct*)pNotifyReq->pMessageBody->pMessageBody;

			EcrioSigMgrGetOptionalHeader(pContext->hEcrioSigMgrHandle, pNotifyReq->pOptionalHeaderList, EcrioSipHeaderTypeContentEncoding, &pHeader);
			if (pHeader != NULL)
			{
				if (pHeader->ppHeaderValues != NULL && pHeader->ppHeaderValues[0] != NULL && pHeader->ppHeaderValues[0]->pHeaderValue != NULL)
				{
					if (pal_StringCompare(pHeader->ppHeaderValues[0]->pHeaderValue, (u_char*)"gzip") == 0)
					{
						bIsGzip = Enum_TRUE;
					}
				}
			}

			if (bIsGzip == Enum_TRUE)
			{
				/** Decompressing message body which compressed by gZip */
				uError = EcriogZipDeCompress(pUnknownMsg->pBuffer, pUnknownMsg->bufferLength, &pBuf, &uLen);
				if (uError != ECRIO_GZIP_NO_ERROR)
				{
					uError = ECRIO_CPM_INTERNAL_ERROR;
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tDecompress message body error",
						__FUNCTION__, __LINE__);
					goto Error_Level_01;
				}

				pData = pBuf;
				uDataLen = uLen;
			}
			else
			{
				pData = pUnknownMsg->pBuffer;
				uDataLen = pUnknownMsg->bufferLength;
			}

			/** Handle multipart message body */
			EcrioSigMgrHeaderValueStruct *pHdrValue = &pUnknownMsg->contentType;
			if (pal_StringCompare(pHdrValue->pHeaderValue, (u_char *)"multipart/mixed") == 0 ||
					pal_StringCompare(pHdrValue->pHeaderValue, (u_char *)"multipart/related") == 0)
			{
				bIsMultipart = Enum_TRUE;
				if (pHdrValue->ppParams && pHdrValue->ppParams[0])
				{
					for (i = 0; i < pHdrValue->numParams; i++)
					{
						if (pal_StringCompare(pHdrValue->ppParams[i]->pParamName, (u_char *)"boundary") == 0)
						{
							pBoundary = pHdrValue->ppParams[i]->pParamValue;
						}
					}
				}
			}

			if (bIsMultipart == Enum_TRUE && pBoundary != NULL)
			{
				pBoundaryStr = (char*)ec_CPM_StringUnquote(pBoundary);
				uError = ec_CPM_ParseMultipartMessage(pContext, &pMultiPart, (char*)pBoundaryStr, (char*)pData, uDataLen);
				if (uError != ECRIO_CPM_NO_ERROR)
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_ParseMultipartMessage() failed with uError=%u",
						__FUNCTION__, __LINE__, uError);
					goto Error_Level_01;
				}

				for (i = 0; i < pMultiPart->uNumOfBody; i++)
				{
					if (pal_StringCompare(pMultiPart->ppBody[i]->pContentType, (u_char *)"application/conference-info+xml") == 0)
					{
						/** Parse message body for conference-info XML */
						uError = ec_CPM_ParseXmlDocument(pContext, &pStruct, (char*)pMultiPart->ppBody[i]->pMessageBody, pMultiPart->ppBody[i]->uContentLength);
						if (uError != ECRIO_CPM_NO_ERROR)
						{
							uError = ECRIO_CPM_INTERNAL_ERROR;
							CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tXML parser for conference-info XML error",
								__FUNCTION__, __LINE__);
							goto Error_Level_01;
						}

						pal_MemorySet(&info, 0, sizeof(EcrioCPMGroupChatInfoStruct));

						uError = ec_CPM_FillGroupChatInfo(pContext, &info, pStruct);
						if (uError != ECRIO_CPM_NO_ERROR)
						{
							uError = ECRIO_CPM_INTERNAL_ERROR;
							CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tFill Group Chat Information error",
								__FUNCTION__, __LINE__);
							goto Error_Level_01;
						}
					}
					else if (pMultiPart->ppBody[i] != NULL && pMultiPart->ppBody[i]->pContentType != NULL)
					{
						if (info.pIconInfo == NULL)
						{
							pal_MemoryAllocate((u_int32)sizeof(EcrioCPMIconInfoStruct), (void **)&info.pIconInfo);
							if (info.pIconInfo == NULL)
							{
								uError = ECRIO_CPM_MEMORY_ERROR;
								CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
									__FUNCTION__, __LINE__);
								goto Error_Level_01;
							}

							info.pIconInfo->pContentType = pMultiPart->ppBody[i]->pContentType;
							info.pIconInfo->pContentDisposition = pMultiPart->ppBody[i]->pContentDisposition;
							info.pIconInfo->pContentId = pMultiPart->ppBody[i]->pContentId;
							info.pIconInfo->pContentTransferEncoding = pMultiPart->ppBody[i]->pContentTransferEncoding;
							info.pIconInfo->uDataLength = pMultiPart->ppBody[i]->uContentLength;
							info.pIconInfo->pData = pMultiPart->ppBody[i]->pMessageBody;
						}
					}
				}
			}
			else
			{
				if (pal_StringCompare(pUnknownMsg->contentType.pHeaderValue, (u_char*)"application/conference-info+xml") == 0)
				{
					/** Parse message body for conference-info XML */
					uError = ec_CPM_ParseXmlDocument(pContext, &pStruct, (char*)pData, uDataLen);
					if (uError != ECRIO_CPM_NO_ERROR)
					{
						uError = ECRIO_CPM_INTERNAL_ERROR;
						CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tXML parser for conference-info XML error",
							__FUNCTION__, __LINE__);
						goto Error_Level_01;
					}

					uError = ec_CPM_FillGroupChatInfo(pContext, &info, pStruct);
					if (uError != ECRIO_CPM_NO_ERROR)
					{
						uError = ECRIO_CPM_INTERNAL_ERROR;
						CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tFill Group Chat Information error",
							__FUNCTION__, __LINE__);
						goto Error_Level_01;
					}
				}
			}

			/** Get app data and find session id */
			EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, pNotifyReq->pMandatoryHdrs->pCallId, (void**)&pSession);
			if (pSession == NULL)
			{
				CPMLOGW(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tCouldn't find session handle from call-id:%s",
					__FUNCTION__, __LINE__, pNotifyReq->pMandatoryHdrs->pCallId);
				info.pSessionId = NULL;
			}
			else
			{
				if (pContext->bIsRelayEnabled == Enum_TRUE)
					info.pSessionId = (char *)pSession->relayData.pAppCallId;
				else
					info.pSessionId = (char *)pSession->pCallId;
			}

			/** Notfiy upper layer */
			cpmNotifStruct.pNotifData = &info;
			cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_GroupChatInfo;
			pContext->callbackStruct.pCPMNotificationCallbackFn(pContext->callbackStruct.pCallbackData,
				&cpmNotifStruct);
		}
	}

Error_Level_01:
	if (pStruct != NULL)
	{
		ec_CPM_ReleaseParsedXmlStruct(&pStruct, Enum_TRUE);
	}
	if (pBuf != NULL)
	{
		pal_MemoryFree((void**)&pBuf);
	}
	if (NULL != pBoundaryStr)
	{
		pal_MemoryFree((void **)&pBoundaryStr);
	}

	if (NULL != pMultiPart)
	{
		for (i = 0; i < pMultiPart->uNumOfBody; i++)
		{
			if (pMultiPart->ppBody[i]->pContentType)
			{
				pal_MemoryFree((void **)&pMultiPart->ppBody[i]->pContentType);
			}
			if (pMultiPart->ppBody[i]->pContentDisposition)
			{
				pal_MemoryFree((void **)&pMultiPart->ppBody[i]->pContentDisposition);
			}
			if (pMultiPart->ppBody[i]->pContentId)
			{
				pal_MemoryFree((void **)&pMultiPart->ppBody[i]->pContentId);
			}
			if (pMultiPart->ppBody[i]->pContentTransferEncoding)
			{
				pal_MemoryFree((void **)&pMultiPart->ppBody[i]->pContentTransferEncoding);
			}
			if (pMultiPart->ppBody[i]->pMessageBody)
			{
				pal_MemoryFree((void **)&pMultiPart->ppBody[i]->pMessageBody);
			}
			pal_MemoryFree((void **)&pMultiPart->ppBody[i]);
		}
		pal_MemoryFree((void **)&pMultiPart->ppBody);
		pal_MemoryFree((void **)&pMultiPart);
	}

	{
		if (info.desc.pSubject != NULL)
			pal_MemoryFree((void**)&info.desc.pSubject);
		if (info.desc.pSubCngUser != NULL)
			pal_MemoryFree((void**)&info.desc.pSubCngUser);
		if (info.desc.pSubCngDate != NULL)
			pal_MemoryFree((void**)&info.desc.pSubCngDate);
		if (info.desc.pIconUri != NULL)
			pal_MemoryFree((void**)&info.desc.pIconUri);
		if (info.desc.pIconInfo != NULL)
			pal_MemoryFree((void**)&info.desc.pIconInfo);
		if (info.desc.pIconCngUser != NULL)
			pal_MemoryFree((void**)&info.desc.pIconCngUser);
		if (info.desc.pIconCngDate != NULL)
			pal_MemoryFree((void**)&info.desc.pIconCngDate);

		if (info.uNumOfUsers != 0 && info.ppUsers != NULL)
		{
			for (i = 0; i < info.uNumOfUsers; i++)
			{
				if (info.ppUsers[i]->pUri != NULL)
					pal_MemoryFree((void**)&info.ppUsers[i]->pUri);
				if (info.ppUsers[i]->pDisplayText != NULL)
					pal_MemoryFree((void**)&info.ppUsers[i]->pDisplayText);
				pal_MemoryFree((void**)&info.ppUsers[i]);
			}
			pal_MemoryFree((void**)&info.ppUsers);
		}

		if (info.pIconInfo)
			pal_MemoryFree((void**)&info.pIconInfo);
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
}
#endif

void ec_CPM_HandleReferResponse
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
)
{
	EcrioSigMgrSipMessageStruct	*pReferRsp = NULL;
	EcrioCPMSessionStruct *pSession = NULL;
	EcrioCPMNotifStruct cpmNotifStruct = { .eNotifCmd = EcrioCPM_Notif_None };
	EcrioCPMAddRemoveParticipantResponseStruct reasonStruct = { 0 };

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Find the current CPM session structure */

	pReferRsp = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;

	if (pReferRsp != NULL)
	{
		if ((pReferRsp->responseCode / 100) == 2)
		{
			/** Success for 2xx REFER response */
			cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_ReferSendSuccess;
		}
		else if ((pReferRsp->responseCode / 100) > 2)
		{
			/** Failure for 3xx/4xx/5xx/6xx REFER response */
			cpmNotifStruct.eNotifCmd = EcrioCPM_Notif_ReferSendFailure;
		}

		/** Get app data and find session id */
		EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, pReferRsp->pMandatoryHdrs->pCallId, (void**)&pSession);
		if (pSession == NULL)
		{
			CPMLOGW(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tCouldn't find session handle from call-id:%s",
				__FUNCTION__, __LINE__, pReferRsp->pMandatoryHdrs->pCallId);
			reasonStruct.pSessionId = NULL;
		}
		else
		{
			if (pContext->bIsRelayEnabled == Enum_TRUE)
				reasonStruct.pSessionId = (char *)pSession->relayData.pAppCallId;
			else
				reasonStruct.pSessionId = (char *)pSession->pCallId;
		}

		/* Pass branch as refer id. */
		if (pReferRsp->pMandatoryHdrs != NULL &&
			pReferRsp->pMandatoryHdrs->ppVia != NULL &&
			pReferRsp->pMandatoryHdrs->ppVia[0] != NULL)
			reasonStruct.pReferId = (char*)pReferRsp->pMandatoryHdrs->ppVia[0]->pBranch;

		reasonStruct.uReasonCause = pReferRsp->responseCode;
		reasonStruct.pReasonText = NULL;

		/** Notify the upper layer. */
		cpmNotifStruct.pNotifData = &reasonStruct;

		pContext->callbackStruct.pCPMNotificationCallbackFn(pContext->callbackStruct.pCallbackData,
			&cpmNotifStruct);
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
}

void ec_CPM_HandleUpdateRequest
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
)
{
	u_int32	uError = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrSipMessageStruct	*pUpdateNotifStruct = NULL;
	EcrioSigMgrSipMessageStruct updateResp = { .eMethodType = EcrioSipMessageTypeNone };

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pUpdateNotifStruct = (EcrioSigMgrSipMessageStruct *)pSigMgrMessageStruct->pData;

	pal_MemoryAllocate((u_int32)sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&updateResp.pMandatoryHdrs);
	if(updateResp.pMandatoryHdrs != NULL) {
		updateResp.pMandatoryHdrs->pCallId = pal_StringCreate(pUpdateNotifStruct->pMandatoryHdrs->pCallId,
			pal_StringLength(pUpdateNotifStruct->pMandatoryHdrs->pCallId));
		updateResp.pMessageBody = NULL;

		/* Set response code */
		updateResp.responseCode = pUpdateNotifStruct->responseCode;
		updateResp.pReasonPhrase = pUpdateNotifStruct->pReasonPhrase;

		/* Populate Via using incoming UPDATE */
		updateResp.pMandatoryHdrs->numVia = pUpdateNotifStruct->pMandatoryHdrs->numVia;
		updateResp.pMandatoryHdrs->ppVia = pUpdateNotifStruct->pMandatoryHdrs->ppVia;

		uError = EcrioSigMgrSendUpdateResponse(pContext->hEcrioSigMgrHandle, &updateResp);
		if ( uError != ECRIO_SIGMGR_NO_ERROR) {
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral,
					"%s:%u\tEcrioSigMgrSendUpdateResponse() failed with uError=%u",
					__FUNCTION__, __LINE__, uError);
		}
		else
		{
			CPMLOGD(pContext->hLogHandle, KLogTypeGeneral,
					"%s:%u\tSent response to update request successfully",
					__FUNCTION__, __LINE__);
		}

		/* Free allocated memory */
		if(updateResp.pMandatoryHdrs->pCallId != NULL)
			pal_MemoryFree((void**)&updateResp.pMandatoryHdrs->pCallId);

		pal_MemoryFree((void**)&updateResp.pMandatoryHdrs);
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
}