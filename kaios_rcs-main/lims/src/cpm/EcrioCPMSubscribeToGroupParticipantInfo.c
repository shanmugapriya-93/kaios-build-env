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
#include "EcrioSigMgrSubscribe.h"

/**
* This function can be called to subscribe participant information connected
* to indicated group chat session ID.
*
* @param[in] hCPMHandle			The CPM handle.
* @param[in] pSessionId			Session identifier.
* @param[in] pGroupSessionId	The ID of the group chat session that
*								should be affected.
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMSubscribeToGroupParticipantInfo
(
	CPMHANDLE hCPMHandle,
	char *pSessionId,
	u_char *pGroupSessionId
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	u_int32 uSigMgrError = ECRIO_SIGMGR_NO_ERROR;
	u_int32 uExpire = 0;
	EcrioCPMContextStruct *pContext = NULL;
	EcrioCPMSessionStruct *pSession = NULL;
	EcrioSigMgrConversationsIdStruct convId = { 0 };
	EcrioSigMgrNameAddrWithParamsStruct toHeader = { 0 };
	EcrioSigMgrSendSubscribeStruct tSendStruct = { 0 };
	char *pCallId = NULL;
	u_char *pEvent = NULL;
	u_char *pAccept = NULL;

	if (hCPMHandle == NULL || pSessionId == NULL || pGroupSessionId == NULL)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	/** Get the CPM structure from the handle */
	pContext = (EcrioCPMContextStruct *)hCPMHandle;

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pContext->bIsRelayEnabled == Enum_TRUE)
		ec_CPM_MapGetData(pContext, (u_char *)pSessionId, (void**)&pCallId);
	else
		pCallId = (char*)pSessionId;

	/** Get the session structure corresponding to the session id */
	EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, (u_char *)pCallId, (void**)&pSession);

	if (pSession == NULL)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpSession not found for pSessionId=%s",
			__FUNCTION__, __LINE__, pSessionId);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto EndTag;
	}

	if (pSession->state != EcrioCPMSessionState_Active)
	{
		uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tCurrent session is not in Active State...",
			__FUNCTION__, __LINE__);
		goto EndTag;
	}

	uExpire = EcrioSigMgrGetSubscriptionExpireInterval(pContext->hEcrioSigMgrHandle);
	if (uExpire == 0)
	{
		uExpire = 3600;
	}

	/* For Header Support */
	uSigMgrError = EcrioSigMgrPopulateToHeader(pContext->hEcrioSigMgrHandle, pGroupSessionId, &toHeader);
	if (uSigMgrError != ECRIO_SIGMGR_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\t EcrioSigMgrPopulateToHeader() failed with error= %u",
			__FUNCTION__, __LINE__, uSigMgrError);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto EndTag;
	}

	if (pSession->pConvId != NULL)
	{
		convId.pContributionId = pSession->pConvId->pContributionId;
		convId.pConversationId = pSession->pConvId->pConversationId;
		convId.pInReplyToContId = pSession->pConvId->pInReplyToContId;
	}

	pEvent = (u_char*)"conference";
	pAccept = (u_char*)"application/conference-info+xml";

	tSendStruct.pTo = &toHeader;
	tSendStruct.uEventCount = 1;
	tSendStruct.ppEventList = &pEvent;
	tSendStruct.uAcceptCount = 1;
	tSendStruct.ppAcceptList = &pAccept;
	tSendStruct.uExpiry = uExpire;
	tSendStruct.ppCallId = &pSession->pGrpChatSubSessId;
	//tSendStruct.pConvId = &convId;
	tSendStruct.pConvId = NULL;
	tSendStruct.bPrivacy = Enum_FALSE;
	tSendStruct.pMsgBody = NULL;

	uError = ec_CPM_PopulateFeatureTags(pContext, EcrioCPMContentTypeEnum_FileTransferOverHTTP, Enum_FALSE, Enum_TRUE, Enum_FALSE, Enum_FALSE, Enum_FALSE, &tSendStruct.pFetaureTags);
	if (uError != ECRIO_CPM_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_PopulateFeatureTags() error=%u",
			__FUNCTION__, __LINE__, uError);
		goto EndTag;
	}

	uError = EcrioSigMgrSendSubscribe(pContext->hEcrioSigMgrHandle, &tSendStruct);
	if (uSigMgrError != ECRIO_SIGMGR_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\t EcrioSigMgrSendSubscribe() failed with error= %u",
			 __FUNCTION__, __LINE__, uError);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto EndTag;
	}

	/* Store the session data into hash map for that call ID */
	EcrioSigMgrSetAppData(pContext->hEcrioSigMgrHandle, pSession->pGrpChatSubSessId, (void*)pSession);

EndTag:
	EcrioSigMgrReleaseUriStruct(pContext->hEcrioSigMgrHandle, &toHeader.nameAddr.addrSpec);

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return uError;
}

/**
* This function can be called to subscribe participant information connected
* to indicated group chat session ID before ending Group Chat session.
*
* @param[in] hCPMHandle			The CPM handle.
* @param[in] pSessionId			Session identifier.
* @param[in] pGroupSessionId	The ID of the group chat session that
*								should be affected.
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMUnSubscribeToGroupParticipantInfo
(
	CPMHANDLE hCPMHandle,
	char *pSessionId,
	u_char *pGroupSessionId
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	u_int32 uSigMgrError = ECRIO_SIGMGR_NO_ERROR;
	EcrioCPMContextStruct *pContext = NULL;
	EcrioCPMSessionStruct *pSession = NULL;
	EcrioSigMgrNameAddrWithParamsStruct toHeader = { 0 };
	EcrioSigMgrSendSubscribeStruct tSendStruct = { 0 };
	char *pCallId = NULL;
	u_char *pEvent = NULL;
	u_char *pAccept = NULL;

	if (hCPMHandle == NULL || pSessionId == NULL || pGroupSessionId == NULL)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	/** Get the CPM structure from the handle */
	pContext = (EcrioCPMContextStruct *)hCPMHandle;

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pContext->bIsRelayEnabled == Enum_TRUE)
		ec_CPM_MapGetData(pContext, (u_char *)pSessionId, (void**)&pCallId);
	else
		pCallId = (char*)pSessionId;

	/** Get the session structure corresponding to the session id */
	EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, (u_char *)pCallId, (void**)&pSession);

	if (pSession == NULL)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpSession not found for pSessionId=%s",
			__FUNCTION__, __LINE__, pSessionId);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto EndTag;
	}

	if (pSession->state != EcrioCPMSessionState_Active)
	{
		uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tCurrent session is not in Active State...",
			__FUNCTION__, __LINE__);
		goto EndTag;
	}

	/* For Header Support */
	uSigMgrError = EcrioSigMgrPopulateToHeader(pContext->hEcrioSigMgrHandle, pGroupSessionId, &toHeader);
	if (uSigMgrError != ECRIO_SIGMGR_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\t EcrioSigMgrPopulateToHeader() failed with error= %u",
			__FUNCTION__, __LINE__, uSigMgrError);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto EndTag;
	}

	pEvent = (u_char*)"conference";
	pAccept = (u_char*)"application/conference-info+xml";

	tSendStruct.pTo = &toHeader;
	tSendStruct.ppCallId = &pSession->pGrpChatSubSessId;
	tSendStruct.uExpiry = 0;
	tSendStruct.uEventCount = 1;
	tSendStruct.ppEventList = &pEvent;
	tSendStruct.uAcceptCount = 1;
	tSendStruct.ppAcceptList = &pAccept;
	tSendStruct.pConvId = NULL;
	tSendStruct.bPrivacy = Enum_FALSE;
	tSendStruct.pMsgBody = NULL;
	uSigMgrError = EcrioSigMgrSendSubscribe(pContext->hEcrioSigMgrHandle, &tSendStruct);
	if (uSigMgrError != ECRIO_SIGMGR_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\t EcrioSigMgrSendSubscribe() failed with error= %u",
			 __FUNCTION__, __LINE__, uError);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto EndTag;
	}

EndTag:
	EcrioSigMgrReleaseUriStruct(pContext->hEcrioSigMgrHandle, &toHeader.nameAddr.addrSpec);

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return uError;
}
