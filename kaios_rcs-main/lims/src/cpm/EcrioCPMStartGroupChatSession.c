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
* This API can be called to start group chat session. The pDest will point to
* Conference-Factory URI or Group Chat Session ID which indicated by the Conference
* Factory Application. By specifying the Group Chat Session ID that has already
* been created session, this assume a role of Join the Group Chat Session.
*
* ppSessionId will be returned which contains the CPM session ID of the internal
* group chat session, even before the CPM / group chat session is actually established.
* This is NOT a Group Chat Session ID which is notified from Conference-Factory
* Application.
*
* @param[in] hCPMHandle			The CPM handle.
* @param[in] pGroupChat			The pointer to EcrioCPMStartSessionStruct.
* @param[in/out] ppSessionId	The ID of the group chat session that should be
*								affected.
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMStartGroupChatSession
(
	CPMHANDLE hCPMHandle,
	EcrioCPMStartSessionStruct *pGroupChat,
	char **ppSessionId
)
{
	u_int32 uError = ECRIO_CPM_NO_ERROR;
	EcrioCPMContextStruct *pContext = NULL;
	EcrioCPMSessionStruct *pCPMSession = NULL;

	if (hCPMHandle == NULL)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	pContext = (EcrioCPMContextStruct*)hCPMHandle;

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pal_MemoryAllocate(sizeof(EcrioCPMSessionStruct), (void**)&pCPMSession);
	if (pCPMSession == NULL)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
			__FUNCTION__, __LINE__);
		uError = ECRIO_CPM_MEMORY_ERROR;
		goto END;
	}
	pCPMSession->eTerminationReason = EcrioSessionTerminationReason_None;
	pCPMSession->bIsStartMSRPSession = Enum_FALSE;

	if (pContext->bIsRelayEnabled == Enum_TRUE)
	{
		uError = ec_CPM_StartMSRPAuthProcess(pContext, pGroupChat, pCPMSession, ppSessionId);
	}
	else
	{
		uError = ec_CPM_StartSession(pContext, pGroupChat, pCPMSession, ppSessionId);
	}

END:
	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uError;
}

BoolEnum EcrioCPMIsGroupChatSession
(
		CPMHANDLE hCPMHandle,
		char *pSessionId
)
{
	BoolEnum isGroupChat = Enum_FALSE;
	EcrioCPMContextStruct *pContext = NULL;
	EcrioCPMSessionStruct *pSession = NULL;
	char *pCallId = NULL;

	if (hCPMHandle == NULL)
	{
		return Enum_INVALID;
	}

	/** Get the CPM structure from the handle */
	pContext = (EcrioCPMContextStruct *)hCPMHandle;

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pContext->bIsRelayEnabled == Enum_TRUE)
		ec_CPM_MapGetData(pContext, (u_char *)pSessionId, (void**)&pCallId);
	else
		pCallId = pSessionId;

	/** Get the session structure corresponding to the session id */
	EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, (u_char *)pCallId, (void**)&pSession);

	if (pSession == NULL)
	{
		isGroupChat = Enum_INVALID;
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpSession not found for pSessionId=%s",
				__FUNCTION__, __LINE__, pSessionId);
		goto EndTag;
	}

	isGroupChat = pSession->bIsGroupChat;
	if(isGroupChat == Enum_FALSE && pSession->pGroupSessionId != NULL)
		isGroupChat = Enum_TRUE;

EndTag:
	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return isGroupChat;
}
