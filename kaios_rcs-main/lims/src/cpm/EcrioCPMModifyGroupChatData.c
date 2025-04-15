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

/**
* This function can be called to modify the group session data management
* related to the subject or icon or host. Change them according to the contents
* set in action.
*
* @param[in] hCPMHandle			The CPM handle.
* @param[in] pDataMng			Pointer to a EcrioCPMDataManagementStruct.
* @param[in] pSessionId			The ID of the CPM session that should be
*								affected.
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMModifyGroupChatData
(
	CPMHANDLE hCPMHandle,
	EcrioCPMDataManagementStruct *pDataMng,
	char *pSessionId
)
{
	u_int32 uError = ECRIO_CPM_NO_ERROR;
	u_int32 uCPIMError = ECRIO_CPIM_NO_ERROR;
	EcrioCPMContextStruct *pContext = NULL;
	u_int32 uLen = 0;
	u_int32 uXmlLen = 0;
	u_char *pBuf = NULL;
	char *pXmlBuf = NULL;
	EcrioCPMBufferStruct cpmBuf = { 0 };
	EcrioCPMSessionStruct* pSession = NULL;
	EcrioCPMGroupDataRequestStruct groupData = { .type = EcrioCPMGroupDataMngType_None };
	EcrioCPMMessageStruct msg = { 0 };
	CPIMMessageStruct cpimMessage = { 0 };
	MSRPTextMessageStruct msrpMessage = { 0 };
	u_char *pCallId = NULL;
	u_char msgId[17] = {0};

	if ((NULL == hCPMHandle) || (NULL == pDataMng))
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	/** Get the CPM structure from the handle */
	pContext = (EcrioCPMContextStruct *)hCPMHandle;

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);	

	if (pContext->bIsRelayEnabled == Enum_TRUE)
		ec_CPM_MapGetData(pContext, (u_char *)pSessionId, (void**)&pCallId);
	else
		pCallId = (u_char *)pSessionId;

	EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, pCallId, (void**)&pSession);

	if (pSession == NULL)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpSession not found for pSessionId=%s",
			__FUNCTION__, __LINE__, pCallId);
		goto Error_Level_01;
	}

	if (pSession->state != EcrioCPMSessionState_Active)
	{
		uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;

		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tInvalid sessionId uError=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	/** Fill XML message */
	if (pDataMng->subject.action != EcrioCPMDataActionType_None)
	{
		groupData.type = EcrioCPMGroupDataMngType_Subject;
		if (pDataMng->subject.action == EcrioCPMDataActionType_Set)
		{
			groupData.subject.type = EcrioCPMGroupDataActionType_Set;
			if (pDataMng->subject.pSubjectLine != NULL)
			{
				groupData.subject.pText = (u_char *)pDataMng->subject.pSubjectLine;
			}
		}
		else if (pDataMng->subject.action == EcrioCPMDataActionType_Delete)
		{
			groupData.subject.type = EcrioCPMGroupDataActionType_Delete;
			groupData.subject.pText = NULL;
		}
	}	
	else if (pDataMng->icon.action != EcrioCPMDataActionType_None)
	{
		groupData.type = EcrioCPMGroupDataMngType_Icon;
		if (pDataMng->icon.action == EcrioCPMDataActionType_Set)
		{
			groupData.icon.type = EcrioCPMGroupDataActionType_Set;
			if (pDataMng->icon.pIconUri != NULL)
			{
				groupData.icon.pUri = (u_char *)pDataMng->icon.pIconUri;
			}
			else if(pDataMng->icon.pFileInfo != NULL && pDataMng->icon.pFileInfo->pFileInfo != NULL)
			{
				groupData.icon.pFileInfo = pDataMng->icon.pFileInfo->pFileInfo;
			}
		}
		else if (pDataMng->icon.action == EcrioCPMDataActionType_Delete)
		{
			groupData.icon.type = EcrioCPMGroupDataActionType_Delete;
			groupData.icon.pUri = NULL;
		}
	}
	else if (pDataMng->host.action != EcrioCPMDataActionType_None)
	{
		groupData.role.type = EcrioCPMGroupDataActionType_Move;
		if (pDataMng->host.pHostUri != NULL)
		{
			groupData.role.pHostUri = (u_char *)pDataMng->host.pHostUri;
		}
		groupData.role.pUserRole = (u_char *)pDataMng->host.pUserRole;
		groupData.type = EcrioCPMGroupDataMngType_Role;
	}

	uError = ec_CPM_FormGroupSessionDataManagementXML(pContext, &groupData, &pXmlBuf, &uXmlLen);
	if (uError != ECRIO_CPM_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_FormGroupSessionDataManagementXML() error",
			__FUNCTION__, __LINE__);
		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}

	cpmBuf.pMessage = (u_char*)pXmlBuf;
	cpmBuf.uMessageLen = uXmlLen;
	cpmBuf.uByteRangeStart = 1;
	cpmBuf.uByteRangeEnd = uXmlLen;
	msg.message.pBuffer = &cpmBuf;
	pal_MemorySet((void*)&msgId[0], 0, 17);
	ec_CPM_getRandomString((u_char*)&msgId[0], 16);
	msg.pIMDNMsgId = (char*) &msgId[0];
	msg.imdnConfig = EcrioCPMIMDispositionConfigNone;
	msg.eContentType = EcrioCPMContentTypeEnum_GroupData;

	/** Populate CPIM Message */
	if (pContext->pOOMObject->ec_oom_IsAnonymousCPIMHeader())
	{
		uError = ec_CPM_PopulateCPIMHeaders(pContext, NULL, NULL, pSession->pGroupSessionId, &msg, &cpimMessage);
	}
	else
	{
		uError = ec_CPM_PopulateCPIMHeaders(pContext, pContext->pPublicIdentity, NULL, pSession->pGroupSessionId, &msg, &cpimMessage);
	}
	if (uError != ECRIO_CPM_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_PopulateCPIMHeaders() error=%u",
			__FUNCTION__, __LINE__, uError);

		goto Error_Level_01;
	}

	if (pDataMng->icon.action == EcrioCPMDataActionType_Set && pDataMng->icon.pFileInfo != NULL)
	{
		/* Populate Icon data */
		cpimMessage.buff[cpimMessage.uNumOfBuffers].eContentType = CPIMContentType_Specified;
		cpimMessage.buff[cpimMessage.uNumOfBuffers].pContentType = pDataMng->icon.pFileInfo->pContentType;
		cpimMessage.buff[cpimMessage.uNumOfBuffers].pMsgBody = pDataMng->icon.pFileInfo->pData;
		cpimMessage.buff[cpimMessage.uNumOfBuffers].uMsgLen = pDataMng->icon.pFileInfo->uDataLength;
		cpimMessage.buff[cpimMessage.uNumOfBuffers].pContentId = pDataMng->icon.pFileInfo->pFileInfo;
		cpimMessage.buff[cpimMessage.uNumOfBuffers].pContentDisposition = (u_char*)"icon";
		cpimMessage.buff[cpimMessage.uNumOfBuffers].pContentTransferEncoding = (u_char*)"binary";
		cpimMessage.uNumOfBuffers++;
	}

	if (pContext->pPANI != NULL)
	{
		cpimMessage.pPANI = pContext->pPANI;
	}

	pBuf = EcrioCPIMForm(pContext->hCPIMHandle, &cpimMessage, &uLen, &uCPIMError);
	if (uCPIMError != ECRIO_CPIM_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioCPIMForm failed, uError=%u",
			__FUNCTION__, __LINE__, uCPIMError);
		uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
		goto Error_Level_01;
	}

	/** Send message using MSRP session */
	msrpMessage.type = MSRPMessageType_CPIM;
	msrpMessage.u.text.pMsg = pBuf;
	msrpMessage.u.text.uMsgLen = uLen;

	if (EcrioMSRPSendTextMessage(pSession->hMSRPSessionHandle, &msrpMessage) != ECRIO_MSRP_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioMSRPSendTextMessage failed", __FUNCTION__, __LINE__);
		uError = ECRIO_CPM_MSRP_ERROR;
		goto Error_Level_01;
	}

Error_Level_01:
	if (pXmlBuf != NULL)
	{
		pal_MemoryFree((void**)&pXmlBuf);
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uError;
}
