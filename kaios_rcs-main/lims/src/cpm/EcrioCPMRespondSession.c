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

u_int32 EcrioCPMRespondSession
(
	CPMHANDLE hCPMHandle,
	EcrioCPMRespondSessionStruct *pRespond
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	u_int32	uSigError = ECRIO_SIGMGR_NO_ERROR;
	u_int32 uSigMgrError = ECRIO_SIGMGR_NO_ERROR;
	EcrioCPMContextStruct *pContext = NULL;
	EcrioSigMgrSipMessageStruct* pInviteResp = NULL;
	EcrioSigMgrSipMessageStruct inviteResp = { .eMethodType = EcrioSipMessageTypeNone };
	EcrioSigMgrMessageBodyStruct msgBody = { .messageBodyType = EcrioSigMgrMessageBodySDP };
	EcrioSigMgrUnknownMessageBodyStruct unknownMsgBody = { 0 };
	LOGHANDLE hLogHandle = NULL;
	EcrioCPMSessionStruct *pSession = NULL;
	EcrioSigMgrConversationsIdStruct sigmgrConvId = { 0 };
	EcrioSDPMSRPAttributeSetupEnum eLocalSetup = ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_NONE;
	u_char* pPath = NULL;
	BoolEnum bIsFileTransfer = Enum_FALSE;

	if (hCPMHandle == NULL)
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;

	pContext = (EcrioCPMContextStruct*)hCPMHandle;

	hLogHandle = pContext->hLogHandle;

	CPMLOGI(hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, (u_char *)pRespond->pSessionId, (void**)&pSession);
	if (pSession == NULL)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tpSession not found for pSessionId=%s",
			__FUNCTION__, __LINE__, pRespond->pSessionId);
		goto Error_Level_02;
	}

	

	/** Get the session structure corresponding to the session id - not required
	  * Only one session allowed */
	if (EcrioCPM_SessionStatus_ACCEPT == pRespond->eStatus)
	{
		if (pSession->state != EcrioCPMSessionState_Starting)
		{
			uError = ECRIO_CPM_BUSY_ERROR;
			CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tMaximum sessions reached uError=%u",
				__FUNCTION__, __LINE__, uError);
			goto Error_Level_02;
		}

		pSession->contentTypes = pRespond->eContentType;

		if (pContext->bIsRelayEnabled == Enum_TRUE)
		{
			eLocalSetup = ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_ACTIVE;
			if (pContext->bMsrpOverTLS == Enum_TRUE)
			{
				pSession->eLocalConnectionType = MSRPConnectionType_TLS_Client;
			}
			else
			{
				pSession->eLocalConnectionType = MSRPConnectionType_TCP_Client;
			}
		}
		else
		{
			if (pContext->bMsrpOverTLS == Enum_TRUE)
			{
				eLocalSetup = (pSession->eLocalConnectionType == MSRPConnectionType_TLS_Client) ? ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_ACTIVE : ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_PASSIVE;
			}
			else
			{
				eLocalSetup = (pSession->eLocalConnectionType == MSRPConnectionType_TCP_Client) ? ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_ACTIVE : ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_PASSIVE;
			}

			if (pRespond->conn.uLocalPort == 0)
				pRespond->conn.uLocalPort = ec_CPM_GenerateRandomPort();
		}

		if (((pRespond->eContentType & EcrioCPMContentTypeEnum_FileTransferOverMSRP ) == EcrioCPMContentTypeEnum_FileTransferOverMSRP) &&
			pRespond->pFile != NULL)
		{
			if (pSession->pFile == NULL)
			{
				pal_MemoryAllocate(sizeof(EcrioCPMFileDescriptorStruct), (void **)&pSession->pFile);
				if (pSession->pFile == NULL)
				{
					CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
						__FUNCTION__, __LINE__);
					uError = ECRIO_CPM_MEMORY_ERROR;
					goto Error_Level_01;
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

			pSession->pFile->pFilePath = (char *)pal_StringCreate((const u_char*)pRespond->pFile->pFilePath, pal_StringLength((const u_char*)pRespond->pFile->pFilePath));
			pSession->pFile->pFileName = (char *)pal_StringCreate((const u_char*)pRespond->pFile->pFileName, pal_StringLength((const u_char*)pRespond->pFile->pFileName));
			pSession->pFile->pFileType = (char *)pal_StringCreate((const u_char*)pRespond->pFile->pFileType, pal_StringLength((const u_char*)pRespond->pFile->pFileType));
			pSession->pFile->uFileSize = pRespond->pFile->uFileSize;
			pSession->pFile->pFileTransferId = (char *)pal_StringCreate((const u_char*)pRespond->pFile->pFileTransferId, pal_StringLength((const u_char*)pRespond->pFile->pFileTransferId));
			pSession->pFile->pIMDNMsgId = (char *)pal_StringCreate((const u_char*)pRespond->pFile->pIMDNMsgId, pal_StringLength((const u_char*)pRespond->pFile->pIMDNMsgId));
			pSession->pFile->imdnConfig = pRespond->pFile->imdnConfig;
			pSession->pFile->pAppData = pRespond->pFile->pAppData;
			pRespond->pFile->pAppData = NULL;
		}

		/*  MSRP chunk size is 1 MB.*/
		if (pRespond->pFile != NULL && pRespond->pFile->uFileSize > (1 << 20))
		{
			bIsFileTransfer = Enum_TRUE;
		}
		/*Create MSRP session*/
		uError = ec_CPM_CreateMSRPSession(pContext, &pRespond->conn, pSession, &pPath, bIsFileTransfer);
		if (uError != ECRIO_CPM_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_CreateMSRPSession() error=%u",
				__FUNCTION__, __LINE__, uError);
			goto Error_Level_01;
		}

		pal_MemorySet((void*)&pSession->localSDP, 0, sizeof(EcrioSDPInformationStruct));
		/** Populate SDP message body */
		unknownMsgBody.pBuffer = NULL;
		pSession->localSDP.strings.pData = pSession->pSDPInformationBuffer;
		pSession->localSDP.strings.uContainerSize = ECRIO_CPM_SDP_INFO_BUFFER_SIZE;
		pSession->localSDP.strings.uSize = 0;

		if (pContext->bIsRelayEnabled != Enum_TRUE)
		{
			uError = ec_CPM_FillInviteSdp(pContext,
				pSession,
				ECRIO_SDP_EVENT_INITIAL_ANSWER,
				pRespond->conn.pLocalIp,
				pRespond->conn.uLocalPort,
				pRespond->conn.isIPv6,
				pRespond->eContentType,
				pPath,
				eLocalSetup,
				&pSession->localSDP,
				&unknownMsgBody);
			if (uError != ECRIO_CPM_NO_ERROR)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_FillInviteSdp() error=%u",
					__FUNCTION__, __LINE__, uError);

				goto Error_Level_01;
			}
		}
		else
		{
			uError = ec_CPM_FillInviteSdp(pContext,
				pSession,
				ECRIO_SDP_EVENT_INITIAL_ANSWER,
				(u_char *)pContext->pRelayServerIP,
				pContext->uRelayServerPort,
				pRespond->conn.isIPv6,
				pRespond->eContentType,
				pPath,
				eLocalSetup,
				&pSession->localSDP,
				&unknownMsgBody);
			if (uError != ECRIO_CPM_NO_ERROR)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_FillInviteSdp() error=%u",
					__FUNCTION__, __LINE__, uError);

				goto Error_Level_01;
			}
		}

		/** Content-Type : application-sdp */
		unknownMsgBody.contentType.numParams = 0;
		unknownMsgBody.contentType.ppParams = NULL;
		unknownMsgBody.contentType.pHeaderValue = (u_char *)"application/sdp";

		msgBody.messageBodyType = EcrioSigMgrMessageBodyUnknown;
		msgBody.pMessageBody = &unknownMsgBody;
		inviteResp.pMessageBody = &msgBody;

		inviteResp.responseCode = ECRIO_CPM_RESPONSE_SUCCESS;
		inviteResp.pReasonPhrase = ECRIO_CPM_RESPONSE_SUCCESS_PHRASE;

		if (pRespond->pConvId != NULL)
		{
			sigmgrConvId.pContributionId = pRespond->pConvId->pContributionId;
			sigmgrConvId.pConversationId = pRespond->pConvId->pConversationId;
			sigmgrConvId.pInReplyToContId = pRespond->pConvId->pInReplyToContId;
			inviteResp.pConvId = &sigmgrConvId;
		}
	}
	else
	{
		if (EcrioCPM_SessionStatus_TEMPORARILY_UNAVAILABLE == pRespond->eStatus)
		{
			inviteResp.responseCode = ECRIO_CPM_RESPONSE_TEMPORARILY_UNAVAILABLE;
			inviteResp.pReasonPhrase = ECRIO_CPM_RESPONSE_TEMPORARILY_UNAVAILABLE_PHRASE;
		}
		else if (EcrioCPM_SessionStatus_BUSY == pRespond->eStatus)
		{
			inviteResp.responseCode = ECRIO_CPM_RESPONSE_BUSY;
			inviteResp.pReasonPhrase = ECRIO_CPM_RESPONSE_BUSY_PHRASE;
		}
		else if (EcrioCPM_SessionStatus_NOT_ACCEPTABLE_HERE == pRespond->eStatus)
		{
			inviteResp.responseCode = ECRIO_CPM_RESPONSE_NOT_ACCEPTABLE_HERE;
			inviteResp.pReasonPhrase = ECRIO_CPM_RESPONSE_NOT_ACCEPTABLE_HERE_PHRASE;
		}
		else if (EcrioCPM_SessionStatus_DECLINED == pRespond->eStatus)
		{
			inviteResp.responseCode = ECRIO_CPM_RESPONSE_DECLINED;
			inviteResp.pReasonPhrase = ECRIO_CPM_RESPONSE_DECLINED_PHRASE;
		}
		else if (EcrioCPM_SessionStatus_NOT_ACCEPTABLE == pRespond->eStatus)
		{
			inviteResp.responseCode = ECRIO_CPM_RESPONSE_NOT_ACCEPTABLE;
			inviteResp.pReasonPhrase = ECRIO_CPM_RESPONSE_NOT_ACCEPTABLE_PHRASE;
		}
	}

	/** Create CPM feature-tag string */
	uError = ec_CPM_PopulateFeatureTags(pContext, pRespond->eContentType, pSession->bIsLargeMessage, Enum_FALSE, Enum_FALSE, Enum_FALSE, Enum_FALSE, &inviteResp.pFetaureTags);
	if (uError != ECRIO_CPM_NO_ERROR)
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_PopulateFeatureTags() error=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	if (inviteResp.pMandatoryHdrs == NULL)
	{
		pal_MemoryAllocate((u_int32)sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&inviteResp.pMandatoryHdrs);
		if (inviteResp.pMandatoryHdrs == NULL)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory allocation error.",
				__FUNCTION__, __LINE__);
			uError = ECRIO_CPM_MEMORY_ERROR;
			goto Error_Level_01;
		}
	}
	inviteResp.pMandatoryHdrs->pCallId = pal_StringCreate(pSession->pCallId, pal_StringLength(pSession->pCallId));
	uSigError = EcrioSigMgrSendInviteResponse(pContext->hEcrioSigMgrHandle,
		&inviteResp);
	if ((ECRIO_SIGMGR_NO_ERROR != uSigError))
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrSendInviteResponse() error=%u",
			__FUNCTION__, __LINE__, uSigError);

		if (uSigMgrError == ECRIO_SIG_MGR_SIG_SOCKET_ERROR)
			uError = ECRIO_CPM_SOCKET_ERROR;
		else
			uError = ECRIO_CPM_INTERNAL_ERROR;
		goto Error_Level_01;
	}

	if (EcrioCPM_SessionStatus_ACCEPT == pRespond->eStatus)
	{
		/** Populate sessioninfo struct if accepted */
		pSession->state = EcrioCPMSessionState_Active;

		/* MSRP Session setup based on https://tools.ietf.org/html/rfc4975#section-5.4,
		https://tools.ietf.org/html/rfc4975#section-4 and http://www.qtc.jp/3GPP/Specs/24247-910.pdf @ page 33 item #29 */
		if (pSession->eLocalConnectionType == MSRPConnectionType_TCP_Server)
		{
			MSRPHostStruct msrpRemoteHost = { 0 };

			uError = ec_CPM_StartMSRPSession(pContext, pSession);
			if (uError != ECRIO_CPM_NO_ERROR)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tec_CPM_StartMSRPSession() error=%u",
					__FUNCTION__, __LINE__, uError);
				goto Error_Level_01;
			}

			/* Set remote details as DUT is acting as TCP server. This info will be used by MSRP lib for verifying incoming MSRP packets */
			msrpRemoteHost.bIPv6 = pSession->remoteSDPInfo.isIPv6;
			if (NULL == pal_StringNCopy((u_char *)msrpRemoteHost.ip, 64, pSession->remoteSDPInfo.pRemoteIp, 
				pal_StringLength((const u_char *)pSession->remoteSDPInfo.pRemoteIp)))
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
					__FUNCTION__, __LINE__);
				uError = ECRIO_CPM_MEMORY_ERROR;
				goto Error_Level_01;
			}
			msrpRemoteHost.port = pSession->remoteSDPInfo.uRemotePort;

			uError = EcrioMSRPSetRemoteHost(pSession->hMSRPSessionHandle, &msrpRemoteHost);
			if (uError != ECRIO_MSRP_NO_ERROR)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioMSRPSetRemoteHost() uError=%u",
					__FUNCTION__, __LINE__, uError);
				uError = ECRIO_CPM_INTERNAL_ERROR;
				goto Error_Level_01;
			}

			uError = EcrioMSRPSetRemoteMSRPAddress(pSession->hMSRPSessionHandle, (char *)pSession->remoteSDPInfo.pRemotePath);
			if (uError != ECRIO_MSRP_NO_ERROR)
			{
				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioMSRPSetRemoteMSRPAddress() uError=%u",
					__FUNCTION__, __LINE__, uError);
				uError = ECRIO_CPM_INTERNAL_ERROR;
				goto Error_Level_01;
			}
		}
	}
	else
	{
		pSession->state = EcrioCPMSessionState_Unused;
	}

Error_Level_01:
	
	if (pPath)
	{
		pal_MemoryFree((void**)&pPath);
	}

	if (uError != ECRIO_CPM_NO_ERROR)
	{
		EcrioSigMgrAbortSession(pContext->hEcrioSigMgrHandle, pSession->pCallId);
		ec_CPM_ReleaseCPMSession(pContext, pSession);
	}

	pInviteResp = &inviteResp;
	inviteResp.pReasonPhrase = NULL;
	inviteResp.pMessageBody = NULL;
	EcrioSigMgrStructRelease(pContext->hEcrioSigMgrHandle, EcrioSigMgrStructType_SipMessage, (void**)&pInviteResp, Enum_FALSE);

Error_Level_02:

	CPMLOGI(hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return uError;
}
