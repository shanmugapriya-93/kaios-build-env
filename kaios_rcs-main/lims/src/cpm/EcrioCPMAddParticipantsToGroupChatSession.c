/******************************************************************************

Copyright (c) 2019-2020 Ecrio, Inc. All Rights Reserved.

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
#include "EcrioSigMgrRefer.h"

/**
* This API can be used to invite other principals to existing Group Chat session.
* Participants are specified in a list format and multiple URIs can be pointed.
*
* @param[in] hCPMHandle			The CPM handle.
* @param[in] pList				Pointer to a EcrioCPMParticipantListStruct.
* @param[in] pSessionId			The ID of the CPM session that should be
*								affected.
* @param[in] ppReferId			ID to idenitfy the REFER request associated
*								with request to add participant
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMAddParticipantsToGroupChatSession
(
	CPMHANDLE hCPMHandle,
	EcrioCPMParticipantListStruct *pList,
	char *pSessionId,
	char **ppReferId
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	EcrioCPMContextStruct *pContext = NULL;

	if (hCPMHandle == NULL)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	/** Get the CPM structure from the handle */
	pContext = (EcrioCPMContextStruct *)hCPMHandle;

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	uError = ec_CPM_HandleParticipantsToGroupChatSession(pContext, Enum_TRUE, pList, pSessionId, ppReferId);
	if (ECRIO_CPM_NO_ERROR != uError)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_HandleParticipantsToGroupChatSession() uError=%u ",
			__FUNCTION__, __LINE__, uError);
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uError;
}

u_int32 ec_CPM_HandleParticipantsToGroupChatSession
(
	EcrioCPMContextStruct *pContext,
	BoolEnum bIsAddParticipants,
	EcrioCPMParticipantListStruct *pList,
	char *pSessionId,
	char **ppReferId
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	u_int32 uSigMgrError = ECRIO_SIGMGR_NO_ERROR;
	u_int32 uLength = 0;
	EcrioCPMSessionStruct *pSession = NULL;
	EcrioSigMgrSipMessageStruct referReq = { .eMethodType = EcrioSipMessageTypeNone };
	EcrioSigMgrMessageBodyStruct msgBody = { .messageBodyType = EcrioSigMgrMessageBodySDP };
	EcrioSigMgrUnknownMessageBodyStruct unknownMsgBody = { 0 };
	EcrioSigMgrReferRecipientInfoStruct referInfo = { .eMethod = EcrioSigMgrReferMethodNone };
	EcrioSigMgrConversationsIdStruct convId = { 0 };
	char *pCallId = NULL;
	char *pData = NULL;
	u_char *pUri = NULL;
	u_char ucString[13] = { 0 };

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pContext->bIsRelayEnabled == Enum_TRUE)
		ec_CPM_MapGetData(pContext, (u_char *)pSessionId, (void**)&pCallId);
	else
		pCallId = pSessionId;

	/** Get the session structure corresponding to the session id */
	EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, (u_char *)pCallId, (void**)&pSession);

	if (pSession == NULL)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpSession not found for pSessionId=%s",
			__FUNCTION__, __LINE__, pSessionId);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto ErrTag;

	}

	if (pSession->state != EcrioCPMSessionState_Starting &&
		pSession->state != EcrioCPMSessionState_Active)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tCurrent session is not in Active State...",
			__FUNCTION__, __LINE__);

		uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
		goto ErrTag;
	}

	if (bIsAddParticipants == Enum_TRUE)
	{
		referInfo.eMethod = EcrioSigMgrReferMethodINVITE;
	}
	else
	{
		referInfo.eMethod = EcrioSigMgrReferMethodBYE;
	}

	if (pList->uNumOfUsers == 1)
	{
		referInfo.bIsMultiRecipient = Enum_FALSE;
		/** Add default "sip:" if ppUri[0] does not present URI scheme. */
		pal_MemoryAllocate(256, (void **)&pUri);
		if (pUri == NULL)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tCould not allocate memory...",
			__FUNCTION__, __LINE__);
			uError = ECRIO_CPM_MEMORY_ERROR;
			goto ErrTag;
		}
		pal_MemorySet(pUri, 0, 256);

		if ((pal_StringNCompare((u_char *)pList->ppUri[0], (u_char*)"sip:", 4) != 0) &&
			(pal_StringNCompare((u_char *)pList->ppUri[0], (u_char*)"tel:", 4) != 0))
		{
			if (NULL == pal_StringNCopy(pUri, 256, (const u_char*)"sip:", pal_StringLength((const u_char*)"sip:")))
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
				__FUNCTION__, __LINE__);
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto ErrTag;
			}
			if (NULL == pal_StringNConcatenate(pUri, 256 - pal_StringLength((u_char*)pUri), (u_char*)pList->ppUri[0], pal_StringLength((u_char*)pList->ppUri[0])))
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
				__FUNCTION__, __LINE__);
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto ErrTag;
			}
			if (NULL == pal_StringNConcatenate(pUri, 256 - pal_StringLength((u_char*)pUri), (u_char*)"@", pal_StringLength((u_char*)"@")))
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
				__FUNCTION__, __LINE__);
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto ErrTag;
			}
			if (NULL == pal_StringNConcatenate(pUri, 256 - pal_StringLength((u_char*)pUri), pContext->pLocalDomain, pal_StringLength(pContext->pLocalDomain)))
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
				__FUNCTION__, __LINE__);
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto ErrTag;
			}
		}
		else
		{
			if (NULL == pal_StringNCopy(pUri, 256, (const u_char *)pList->ppUri[0], pal_StringLength((const u_char *)pList->ppUri[0])))
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
				__FUNCTION__, __LINE__);
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto ErrTag;
			}
		}

		referInfo.pRecipient = pUri;
		referInfo.pContentId = NULL;

		referReq.pMessageBody = NULL;
	}
	else if (pList->uNumOfUsers > 1)
	{
		EcrioCPMParticipantTypeEnum eType = EcrioCPMParticipantType_None;

		referInfo.bIsMultiRecipient = Enum_TRUE;
		referInfo.pRecipient = NULL;
		pal_MemorySet(ucString, 0, 13);
		ec_CPM_getRandomString(ucString, 12);
		referInfo.pContentId = ucString;

		if (bIsAddParticipants == Enum_TRUE)
			eType = EcrioCPMParticipantType_Add;
		else
			eType = EcrioCPMParticipantType_Remove;

		/** Build MIME resource-list XML */
		uError = ec_CPM_FormResourceListXML(pContext, pList, eType, &pData, &uLength);
		if (uError != ECRIO_CPM_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_FormResourceListXML() error, error=%d",
				__FUNCTION__, __LINE__, uError);

			uError = ECRIO_CPM_INTERNAL_ERROR;
			goto ErrTag;
		}

		unknownMsgBody.bufferLength = uLength;
		unknownMsgBody.pBuffer = (u_char*)pData;

		/** Content-Type : application/resource-lists+xml */
		unknownMsgBody.contentType.numParams = 0;
		unknownMsgBody.contentType.ppParams = NULL;
		unknownMsgBody.contentType.pHeaderValue = (u_char *)"application/resource-lists+xml";

		msgBody.messageBodyType = EcrioSigMgrMessageBodyUnknown;
		msgBody.pMessageBody = &unknownMsgBody;
		referReq.pMessageBody = &msgBody;
	}
	else
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tNo participant...",
			__FUNCTION__, __LINE__);

		uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
		goto ErrTag;
	}

	referReq.eMethodType = EcrioSipMessageTypeRefer;
	if (pSession->pConvId != NULL)
	{
		convId.pContributionId = pSession->pConvId->pContributionId;
		convId.pConversationId = pSession->pConvId->pConversationId;
		convId.pInReplyToContId = pSession->pConvId->pInReplyToContId;
		referReq.pConvId = &convId;
	}

	/** Send REFER Request */
	uSigMgrError = EcrioSigMgrSendRefer(pContext->hEcrioSigMgrHandle, (u_char*)pCallId, &referInfo, &referReq, (u_char**)ppReferId);
	if (ECRIO_SIGMGR_NO_ERROR != uSigMgrError)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrSendRefer() error=%u",
			__FUNCTION__, __LINE__, uSigMgrError);

		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto ErrTag;
	}

ErrTag:
	pSession = NULL;

	if (pData != NULL)
	{
		pal_MemoryFree((void **)&pData);
	}

	if (pUri != NULL)
	{
		pal_MemoryFree((void **)&pUri);
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return uError;

}
