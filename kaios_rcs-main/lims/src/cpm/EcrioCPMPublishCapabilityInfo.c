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
#include "EcrioPresenceLib.h"

u_int32 EcrioCPMPublishCapabilityInfo
(
	CPMHANDLE hCPMHandle,
	EcrioCPMPublishServiceEnum eServices,
	EcrioCPMPublishEvent eEvents
)
{
	u_int32 uSigError = ECRIO_SIGMGR_NO_ERROR, uError = ECRIO_CPM_NO_ERROR;
	u_char* pMessageBody = NULL;
	LOGHANDLE hLogHandle = NULL;
	EcrioCPMContextStruct *pContext = NULL;
	EcrioSigMgrSipMessageStruct messageReq = { .eMethodType = EcrioSipMessageTypeNone }, *pMessageReq = NULL;
	EcrioSigMgrMessageBodyStruct msgBody = { .messageBodyType = EcrioSigMgrMessageBodyUnknown };
	EcrioSigMgrUnknownMessageBodyStruct unknownMsgBody = { 0 };
	EcrioSigMgrPublishEvents ePubEvents = ECRIO_SIG_MGR_PUBLISH_EVENT_None;
	EcrioPresLibPresenceInfoStruct	presInfo = { 0 }, *pPresInfo = NULL;
	COREXDMLIBHANDLE pCoreXdmHandle = NULL;
	PRESLIBHANDLE  pPresenceHandle = NULL;
	pContext = (EcrioCPMContextStruct *)hCPMHandle;
	hLogHandle = pContext->hLogHandle;
	if (NULL == hCPMHandle)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}
	CPMLOGI(hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	switch (eEvents)
	{
	case EcrioCPM_Publish_Initiate:
	case EcrioCPM_Publish_Modify:
	{
		if (eEvents == EcrioCPM_Publish_Initiate)
		{
			ePubEvents = ECRIO_SIG_MGR_PUBLISH_EVENT_Initiate;
		}
		else
		{
			ePubEvents = ECRIO_SIG_MGR_PUBLISH_EVENT_Modify;
		}

		//Populate PRESENCE Message and call corresponding API of Presence Library
		uError = ec_CPM_PresenceInit(pContext, &pPresenceHandle, &pCoreXdmHandle);
		if (uError != ECRIO_CPM_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_PresenceInit() error=%u",
				__FUNCTION__, __LINE__, uError);

			goto Error_Level_01;
		}
		uError = ec_CPM_FillPresencePidfInfo(pContext, &pPresenceHandle, &presInfo, eServices, BITSWANTED);
		if (uError != ECRIO_CPM_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_FillPresencePidfInfo() error=%u",
				__FUNCTION__, __LINE__, uError);

			goto Error_Level_01;
		}
		pMessageBody = EcrioPresLibXMLBuildUpdatePresence(
			pPresenceHandle, &presInfo, &uError);
		if (pMessageBody == NULL || uError)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioPresLibXMLBuildUpdatePresence() error=%u",
				__FUNCTION__, __LINE__, uError);
			uError = ECRIO_CPM_INTERNAL_ERROR;
			goto Error_Level_01;
		}
		unknownMsgBody.bufferLength = pal_StringLength(pMessageBody);
		unknownMsgBody.pBuffer = pMessageBody;
		unknownMsgBody.contentType.numParams = 0;
		unknownMsgBody.contentType.ppParams = NULL;
		unknownMsgBody.contentType.pHeaderValue = (u_char*)"application/pidf+xml";
		messageReq.eModuleId = EcrioSigMgrCallbackRegisteringModule_CPM;
		messageReq.pMessageBody = &msgBody;
		msgBody.messageBodyType = EcrioSigMgrMessageBodyUnknown;
		msgBody.pMessageBody = &unknownMsgBody;
		pPresInfo = &presInfo;
	}
	break;
	case EcrioCPM_Publish_Remove:
	{
		unknownMsgBody.bufferLength = 0;
		unknownMsgBody.pBuffer = NULL;
		unknownMsgBody.contentType.numParams = 0;
		unknownMsgBody.contentType.ppParams = NULL;
		unknownMsgBody.contentType.pHeaderValue = NULL;
		ePubEvents = ECRIO_SIG_MGR_PUBLISH_EVENT_Remove;
		messageReq.eModuleId = EcrioSigMgrCallbackRegisteringModule_CPM;
	}
	break;
	default:
		break;
	}
	uError = EcrioSigMgrSendPublish(pContext->hEcrioSigMgrHandle, &messageReq, ePubEvents);
	if (ECRIO_SIGMGR_NO_ERROR != uSigError)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrSendPublish() error=%u",
			__FUNCTION__, __LINE__, uSigError);

		uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}
	CPMLOGI(hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

Error_Level_01:

	if (pPresInfo){
		EcrioPresLibStructRelease((void **)&pPresInfo, ECRIO_PRESENCE_LIB_PresenceInfoStruct, Enum_FALSE);
	}
	pMessageReq = &messageReq;
	pMessageReq->pMessageBody = NULL;
	EcrioSigMgrStructRelease(pContext->hEcrioSigMgrHandle, EcrioSigMgrStructType_SipMessage, (void**)&pMessageReq, Enum_FALSE);
	ec_CPM_PresenceExit(pContext, &pPresenceHandle, &pCoreXdmHandle);
	return uError;
}

