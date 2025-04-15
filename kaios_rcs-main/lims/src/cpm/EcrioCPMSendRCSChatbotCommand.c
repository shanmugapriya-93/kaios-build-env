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


u_int32 EcrioCPMSendPrivacyManagementCommand
(
	CPMHANDLE hCPMHandle,
	EcrioCPMPrivacyManagementCommandsStruct *pCmd,
	char **ppCallId
)
{
	EcrioCPMContextStruct *pContext = NULL;
	LOGHANDLE hLogHandle = NULL;
	u_int32 uError = ECRIO_CPM_NO_ERROR;
	u_int32 uSigError = ECRIO_SIGMGR_NO_ERROR;
	u_int32 uMsgLen = 0;
	EcrioSigMgrSipMessageStruct messageReq = { .eMethodType = EcrioSipMessageTypeNone };
	EcrioSigMgrSipMessageStruct *pMessageReq = NULL;
	EcrioSigMgrMessageBodyStruct msgBody = { .messageBodyType = EcrioSigMgrMessageBodyUnknown };
	EcrioSigMgrUnknownMessageBodyStruct unknownMsgBody = { 0 };
	EcrioSigMgrNameAddrWithParamsStruct toHeader = { 0 };
	u_char* pMessageBody = NULL;

	if (hCPMHandle == NULL)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	pContext = (EcrioCPMContextStruct *)hCPMHandle;
	hLogHandle = pContext->hLogHandle;

	CPMLOGI(hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Populate header */
	uSigError = EcrioSigMgrPopulateToHeader(pContext->hEcrioSigMgrHandle, (u_char *)pCmd->pChatbot, &toHeader);
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

	uError = ec_CPM_PopulateFeatureTags(pContext, EcrioCPMContentTypeEnum_PrivacyManagementCommand, Enum_FALSE, Enum_FALSE, Enum_TRUE, Enum_FALSE, Enum_FALSE, &messageReq.pFetaureTags);
	if (uError != ECRIO_CPM_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_PopulateFeatureTags() error=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	/** Build XML body */
	uError = ec_CPM_FormPrivacyManagementCommandXML(pContext, pCmd->pCommandID, pCmd->eAction, (char **)&pMessageBody, &uMsgLen);
	if (uError != ECRIO_CPM_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_FormPrivacyManagementCommandXML() error=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	unknownMsgBody.bufferLength = uMsgLen;
	unknownMsgBody.pBuffer = pMessageBody;
	unknownMsgBody.contentType.numParams = 0;
	unknownMsgBody.contentType.ppParams = NULL;
	unknownMsgBody.contentType.pHeaderValue = (u_char*)"application/vnd.gsma.rcsalias-mgmt+xml";

	messageReq.eModuleId = EcrioSigMgrCallbackRegisteringModule_CPM;
	messageReq.pMessageBody = &msgBody;

	msgBody.messageBodyType = EcrioSigMgrMessageBodyUnknown;
	msgBody.pMessageBody = &unknownMsgBody;

	uError = EcrioSigMgrSendInstantMessage(pContext->hEcrioSigMgrHandle, &messageReq);
	if (ECRIO_SIGMGR_NO_ERROR != uSigError)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrSendPagerModeMessage() error=%u",
			__FUNCTION__, __LINE__, uSigError);

		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}

	/** Allocate memory for Call Id , used in upper layer */
	if ((messageReq.pMandatoryHdrs || messageReq.pMandatoryHdrs->pCallId) && ppCallId)
	{
		*ppCallId = (char *)pal_StringCreate(messageReq.pMandatoryHdrs->pCallId, pal_StringLength(messageReq.pMandatoryHdrs->pCallId));
	}

Error_Level_01:

	EcrioSigMgrReleaseUriStruct(pContext->hEcrioSigMgrHandle, &toHeader.nameAddr.addrSpec);
	pMessageReq = &messageReq;
	pMessageReq->pMandatoryHdrs->pTo = NULL; 
	pMessageReq->pMessageBody = NULL;
	EcrioSigMgrStructRelease(pContext->hEcrioSigMgrHandle, EcrioSigMgrStructType_SipMessage, (void**)&pMessageReq, Enum_FALSE);

	if (pMessageBody != NULL)
	{
		pal_MemoryFree((void**)&pMessageBody);
		pMessageBody = NULL;
	}

	CPMLOGI(hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uError;
}


u_int32 EcrioCPMSendSpamReportMessage
(
	CPMHANDLE hCPMHandle,
	EcrioCPMSpamReportMessageStruct *pMsg,
	char **ppCallId
)
{
	EcrioCPMContextStruct *pContext = NULL;
	LOGHANDLE hLogHandle = NULL;
	u_int32 uError = ECRIO_CPM_NO_ERROR;
	u_int32 uSigError = ECRIO_SIGMGR_NO_ERROR;
	u_int32 uMsgLen = 0;
	EcrioSigMgrSipMessageStruct messageReq = { .eMethodType = EcrioSipMessageTypeNone };
	EcrioSigMgrSipMessageStruct *pMessageReq = NULL;
	EcrioSigMgrMessageBodyStruct msgBody = { .messageBodyType = EcrioSigMgrMessageBodyUnknown };
	EcrioSigMgrUnknownMessageBodyStruct unknownMsgBody = { 0 };
	EcrioSigMgrNameAddrWithParamsStruct toHeader = { 0 };
	u_char* pMessageBody = NULL;

	if (hCPMHandle == NULL)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	pContext = (EcrioCPMContextStruct *)hCPMHandle;
	hLogHandle = pContext->hLogHandle;

	CPMLOGI(hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Populate header */
	uSigError = EcrioSigMgrPopulateToHeader(pContext->hEcrioSigMgrHandle, (u_char *)pMsg->pChatbot, &toHeader);
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

	uError = ec_CPM_PopulateFeatureTags(pContext, EcrioCPMContentTypeEnum_SpamReportMessage, Enum_FALSE, Enum_FALSE, Enum_TRUE, Enum_FALSE, Enum_FALSE, &messageReq.pFetaureTags);
	if (uError != ECRIO_CPM_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_PopulateFeatureTags() error=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	/** Build XML body */
	uError = ec_CPM_FormSpamReportMessageXML(pContext, pMsg, (char **)&pMessageBody, &uMsgLen);
	if (uError != ECRIO_CPM_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_FormSpamReportMessageXML() error=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	unknownMsgBody.bufferLength = uMsgLen;
	unknownMsgBody.pBuffer = pMessageBody;
	unknownMsgBody.contentType.numParams = 0;
	unknownMsgBody.contentType.ppParams = NULL;
	unknownMsgBody.contentType.pHeaderValue = (u_char*)"application/vnd.gsma.rcsspam-report+xml";

	messageReq.eModuleId = EcrioSigMgrCallbackRegisteringModule_CPM;
	messageReq.pMessageBody = &msgBody;

	msgBody.messageBodyType = EcrioSigMgrMessageBodyUnknown;
	msgBody.pMessageBody = &unknownMsgBody;

	uError = EcrioSigMgrSendInstantMessage(pContext->hEcrioSigMgrHandle, &messageReq);
	if (ECRIO_SIGMGR_NO_ERROR != uSigError)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrSendPagerModeMessage() error=%u",
			__FUNCTION__, __LINE__, uSigError);

		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}

	/** Allocate memory for Call Id , used in upper layer */
	if ((messageReq.pMandatoryHdrs || messageReq.pMandatoryHdrs->pCallId) && ppCallId)
	{
		*ppCallId = (char *)pal_StringCreate(messageReq.pMandatoryHdrs->pCallId, pal_StringLength(messageReq.pMandatoryHdrs->pCallId));
	}

Error_Level_01:

	EcrioSigMgrReleaseUriStruct(pContext->hEcrioSigMgrHandle, &toHeader.nameAddr.addrSpec);
	pMessageReq = &messageReq;
	pMessageReq->pMandatoryHdrs->pTo = NULL; 
	pMessageReq->pMessageBody = NULL;
	EcrioSigMgrStructRelease(pContext->hEcrioSigMgrHandle, EcrioSigMgrStructType_SipMessage, (void**)&pMessageReq, Enum_FALSE);

	if (pMessageBody != NULL)
	{
		pal_MemoryFree((void**)&pMessageBody);
		pMessageBody = NULL;
	}

	CPMLOGI(hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uError;
}

