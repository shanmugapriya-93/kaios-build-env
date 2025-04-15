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

#include "EcrioSigMgrCommon.h"
#include "EcrioSigMgrInit.h"
#include "EcrioSigMgrInternal.h"
#include "EcrioSigMgrUtilities.h"
#include "EcrioSigMgrCallbacks.h"
#include "EcrioSigMgrOptions.h"
#include "EcrioSigMgrOptionsInternal.h"
#include "EcrioSigMgrTransactionHandler.h"


/*****************************************************************************
                    API Definition Section - Begin
*****************************************************************************/


/*****************************************************************************

  Function:		_EcrioSigMgrSendOptionsRequest()

  Purpose:		Construct and Send an Options Message.

  Description:	Creates and sends a OPTNIOS request.

  Input:		EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                u_char* pCallId - Call Id.
                EcrioSigMgrSipMessageStruct* pOptionsReq - Message request Headers.

  Input/OutPut: none.

  Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrSendOptionsRequest
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pCallId,
	EcrioSigMgrSipMessageStruct *pOptionsReq,
	u_char *pDest
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pCallId != NULL) /* within dialog */
	{
		ec_MapGetKeyData(pSigMgr->hHashMap, pCallId, (void **)&pDialogNode);
		pUsageData = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;

		/* Check and Populate optional header. */
		if (pOptionsReq->pOptionalHeaderList == NULL)
		{
			error = EcrioSigMgrCreateOptionalHeaderList(pSigMgr, &pOptionsReq->pOptionalHeaderList);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tEcrioSigMgrCreateOptionalHeaderList() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}

		/* Operation starts here */
		pOptionsReq->eMethodType = EcrioSipMessageTypeOptions;
		error = _EcrioSigMgrSendInDialogSipRequests(pSigMgr, pOptionsReq, pDialogNode);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrSendInDialogSipRequests() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}
	else /* without dialog */
	{
		error = _EcrioSigMgrSendOptionsImpl(pSigMgr, pOptionsReq, pDest);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrSendOptionsImpl() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}


/*****************************************************************************

Function:		_EcrioSigMgrSendOptionsImpl()

Purpose:		Initiates for publishing capabilities of the UA.

Description:	Creates and sends a Message request.

Input:		EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
EcrioSigMgrSipMessageStruct* pMessageReq - Message
request Headers.

Input/OutPut: SIGSESSIONHANDLE* pSessionHandle - Handle to session.

Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrSendOptionsImpl
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pMessageReq,
	u_char *pDest
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 reqLen = 0, pubExpiry = 0;
	u_char *pReqData = NULL;
	EcrioSigMgrMandatoryHeaderStruct *pMandatoryHdrs = NULL;
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;
	EcrioSigMgrCommonInfoStruct cmnInfo = { 0 };
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };
	u_int32 *pExpires = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);
	pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void**)&pMessageReq->pMandatoryHdrs);
	if (pMessageReq->pMandatoryHdrs == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				   "%s:%u\tpal_MemoryAllocate() error.",
				   __FUNCTION__, __LINE__);

		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}
	pMandatoryHdrs = pMessageReq->pMandatoryHdrs;

	pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct), (void **)&pMandatoryHdrs->pTo);
	if (pMandatoryHdrs->pTo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				   "%s:%u\tpal_MemoryAllocate() error.",
				   __FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	/** Populate toHeader in tel uri format */
	error = EcrioSigMgrFillUriStruct(pSigMgr,
			&pMandatoryHdrs->pTo->nameAddr.addrSpec,
		pDest, EcrioSigMgrURISchemeSIP);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tEcrioSigMgrGetDefaultPUID() populate To uri, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}
	
	pal_MemoryAllocate(sizeof(EcrioSigMgrUriStruct), (void **)&pMandatoryHdrs->pRequestUri);
	if (pMandatoryHdrs->pRequestUri == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}
	
	error = _EcrioSigMgrPopulateUri(pSigMgr, &pMandatoryHdrs->pTo->nameAddr.addrSpec,
		pMandatoryHdrs->pRequestUri);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateUri() populate request uri, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}
	
	error = _EcrioSigMgrUpdateMessageRequestStruct(pSigMgr, pMessageReq, Enum_TRUE);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpMandatoryHdrs() Update Message info object with upper layer data, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_02;
	}

	/* Add P-Access-Network-Info */
	if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeOptions, EcrioSipHeaderTypeP_AccessNetworkInfo) == Enum_TRUE)
	{
		error = _EcrioSigMgrAddP_AccessNWInfoHeader(pSigMgr, pMessageReq->pOptionalHeaderList);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddP_AccessNWInfoHeader() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeOptions, EcrioSipHeaderTypeP_LastAccessNetworkInfo))
	{
		error = _EcrioSigMgrAddP_LastAccessNWInfoHeader(pSigMgr, pMessageReq->pOptionalHeaderList);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSipHeaderTypeP_LastAccessNetworkInfo() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}


	++(pMandatoryHdrs->CSeq);
	/*	Support for Authorization header in all request	start	*/
	error = _EcrioSigMgrCopyCredentials(pSigMgr,
		(u_char *)ECRIO_SIG_MGR_METHOD_OPTIONS,
		NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrCopyCredentials() Create credentials for MESSAGE request, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}
	/*	Support for Authorization header in all request	end	*/
	error = _EcrioSigMgrPopulateMandatoryHeaders(pSigMgr, pMessageReq,
		EcrioSigMgrSIPRequest, Enum_FALSE, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateMandatoryHeaders() for pIMSManHdrs, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	if (pMessageReq->pContact == NULL)
	{
		error = _EcrioSigMgrAddLocalContactUriWithoutFeatureTags(pSigMgr, &pMessageReq->pContact);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddLocalContactUri() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	/** Add P-PreferredIdentity header */
	error = _EcrioSigMgrAddP_PreferrdIdHeader(pSigMgr, pMessageReq);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddP_preferredIdHeader() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	error = _EcrioSigMgrPopulateCommonInfo(pSigMgr, pMandatoryHdrs, &cmnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateCommonInfo() error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}
	pMessageReq->pRouteSet = _EcrioSigMgrAddRouteHeader(pSigMgr);
	pMessageReq->eMethodType = EcrioSipMessageTypeOptions;
	pMessageReq->eReqRspType = EcrioSigMgrSIPRequest;

	/* Construct Message request */
	error = _EcrioSigMgrSipMessageForm(pSigMgr, EcrioSigMgrSipMessageModeFullName, pMessageReq, &pReqData, &reqLen);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSipMessageForm() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_IMS_LIB_ERROR;
		goto Error_Level_01;
	}
	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeSIP,
		"%s:%u\tOutgoing SIP message constructed:", __FUNCTION__, __LINE__);
	SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pReqData, reqLen);
	cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_OPTIONS;
	cmnInfo.role = EcrioSigMgrRoleUAC;
	txnInfo.currentContext = ECRIO_SIGMGR_OPTIONS_REQUEST;
	txnInfo.retransmitContext = ECRIO_SIGMGR_OPTIONS_REQUEST_RESEND;
	txnInfo.deleteflag = Enum_FALSE;
	txnInfo.messageType = EcrioSigMgrMessageNonInviteRequest;

	if (reqLen > pSigMgr->pSignalingInfo->uUdpMtu)
		txnInfo.transport = EcrioSigMgrTransportTCP;
	else
		txnInfo.transport = eTransportType;

	if (pSigMgr->pSignalingInfo->bEnableTls == Enum_TRUE && pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex != 0)
	{
		eTransportType = EcrioSigMgrTransportTLS;
	}
	else if ((pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE) && (reqLen > pSigMgr->pSignalingInfo->uUdpMtu))
	{
		error = _EcrioSigMgrCreateTCPBasedBuffer(pSigMgr, &pReqData, EcrioSigMgrSIPRequest);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrCreateTCPBasedBuffer() error=%u",
				__FUNCTION__, __LINE__, error);

			error = ECRIO_SIGMGR_IMS_LIB_ERROR;
			goto Error_Level_01;
		}

		eTransportType = EcrioSigMgrTransportTCP;
	}
	else if (pSigMgr->pSignalingInfo->bEnableUdp == Enum_FALSE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE)
	{
		eTransportType = EcrioSigMgrTransportTCP;
	}

	txnInfo.transport = eTransportType;
	txnInfo.pMsg = pReqData;
	txnInfo.msglen = reqLen;
	txnInfo.pSessionMappingStr = NULL;

	error = _EcrioSigMgrSendMsgToTXNHandler(pSigMgr, &cmnInfo, &txnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendMsgToTXNHandler() error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_02;
	}
	/*Module routing support*/
	_EcrioSigMgrStoreModuleRoutingInfo(pSigMgr, pMessageReq->pMandatoryHdrs->pCallId, pMessageReq->eModuleId);

Error_Level_02:
	cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);
	if (pReqData != NULL)
	{
		pal_MemoryFree((void **)&pReqData);
		pReqData = NULL;
		reqLen = 0;
	}
Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************

  Function:		_EcrioSigMgrHandleOptionsResponse()

  Purpose:		Handles the Options respnse

  Description:	Handles an incoming OPTNIOS response.

  Input:		EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                EcrioSigMgrMessageResponseStruct* pMessageRespNtf - Message
                response Headers.
                EcrioSigMgrCommonInfoStruct* pCmnInfo - Common headers

  Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrHandleOptionsResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pMessageRespNtf,
	EcrioSigMgrCommonInfoStruct *pCmnInfo
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrMessageStruct message = { 0 };
	u_int32 statusCode = ECRIO_SIGMGR_INVALID_STATUS_CODE;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);
	if ((pMessageRespNtf == NULL) || (pCmnInfo == NULL))
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data", __FUNCTION__, __LINE__);
		goto Error_Level_01;
	}
	message.msgCmd = EcrioSigMgrOptionsResponseNotification;
	message.pData = (void *)pMessageRespNtf;
	statusCode = pMessageRespNtf->responseCode / 100;
	
	if (statusCode == ECRIO_SIGMGR_1XX_RESPONSE)
		goto Error_Level_01; /* No error */
	else
		_EcrioSigMgrUpdateStatusUCE(pSigMgr, &message);

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************
                    API Definition Section - End
*****************************************************************************/
