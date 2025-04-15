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
#include "EcrioSigMgrIMSLibHandler.h"
#include "EcrioSigMgrTransactionHandler.h"
#include "EcrioDSListInterface.h"

/*****************************************************************************
                Internal API Declaration Section - Begin
*****************************************************************************/

/*****************************************************************************
                Internal API Declaration Section - End
*****************************************************************************/

/*****************************************************************************
                    API Definition Section - Begin
*****************************************************************************/

/*****************************************************************************

Function:		_EcrioSigMgrSendRegister()

Purpose:		Starts the registration / deregistration Process for a User.

Description:	Creates and sends a REGISTER request.

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
                EcrioSigMgrRegisterReqStruct* pRegReq - Registration request
                Headers.
                BoolEnum update - if Enum_TRUE, header will be update in the
                User reg info object and override message callback will not
                be generated. Enum_FALSE is used when re-registration is
                done from Signaling Manager internally; for all the rest of
                cases  'update' shall be Enum_TRUE.

Input / OutPut:	void* pRegisterIdentifier - Registration Identifier
                (call-id) of the register request.

Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrSendRegister
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrRegisterRequestEnum eRegReq,
	void *pRegisterIdentifier
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 reqLen = 0, i = 0;
	u_char *pReqData = NULL;
	EcrioSigMgrUriStruct reqUri = { .uriScheme = EcrioSigMgrURISchemeNone };
	EcrioSigMgrSipURIStruct	sipUri = { 0 };
	EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo = NULL;
	EcrioSigMgrMandatoryHeaderStruct *pMandatoryHdrs = NULL;
	EcrioSigMgrCommonInfoStruct commonInfo = { 0 };
	EcrioSigMgrTXNInfoStruct txnInfo = { 0 };
	EcrioSigMgrTransportEnum eTransportType = EcrioSigMgrTransportUDP;
	EcrioSigMgrNetworkInfoStruct *pNetworkInfo = NULL;
	EcrioSigMgrSipMessageStruct	regReq = { .eMethodType = EcrioSipMessageTypeNone };
	EcrioSigMgrSipMessageStruct *pRegReq = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpSigMgr=%p",
		__FUNCTION__, __LINE__, pSigMgr);

	pRegReq = &regReq;

	error = _EcrioSigMgrGetUserRegInfoRegId(pSigMgr,
		pRegisterIdentifier, &pUserRegInfo);
	if (pUserRegInfo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tINVALID registration Id passed, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	pNetworkInfo = pSigMgr->pNetworkInfo;
	if (pNetworkInfo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpNetworkInfo is NULL",
			__FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	if (!((pUserRegInfo->registrationState == EcrioSigMgrStateRegistered) ||
		(pUserRegInfo->registrationState == EcrioSigMgrStateInitialized) ||
		(pUserRegInfo->registrationState == EcrioSigMgrStateAuthenticationRequired) ||
		(pUserRegInfo->registrationState == EcrioSigMgrStateExpiresIntervalTooBrief)))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tRegistration request received while a registration is already is in progress, regId=%u, request type=%u",
			__FUNCTION__, __LINE__, *((u_int32 *)pRegisterIdentifier), eRegReq);
		error = ECRIO_SIGMGR_REG_ALREADY_IN_PROGRESS;
		goto Error_Level_01;
	}

	if ((eRegReq == EcrioSigMgrDeRegisterRequestType_DeReg) &&
		((pUserRegInfo->registrationState != EcrioSigMgrStateRegistered) &&
		(pUserRegInfo->registrationState != EcrioSigMgrStateAuthenticationRequired)))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tDeRegistration request received while a registration has not been done, regId=%u, request type=%u",
			__FUNCTION__, __LINE__, *((u_int32 *)pRegisterIdentifier), eRegReq);
		error = ECRIO_SIGMGR_INVALID_OPERATION;
		goto Error_Level_01;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\tStore/Update upper layer header values in User Reg info object for Reregistration with those values",
		__FUNCTION__, __LINE__);

	error = _EcrioSigMgrUpdateRegInfo(pSigMgr, eRegReq, pUserRegInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrUpdateRegInfo() Error updating User Reg Info object, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	error = _EcrioSigMgrPopulateContactUris(pSigMgr,
		pRegReq, pUserRegInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tFailed to populate contact URI, error=%u",
			__FUNCTION__, __LINE__, error);
		/*TODO: release memory for pUSerRegInfo*/
		error = ECRIO_SIG_MGR_REGISTRATION_FAILED;
		goto Error_Level_01;
	}

	error = _EcrioSigMgrUpdateExpirationTime(pSigMgr,
		pUserRegInfo, eRegReq);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrUpdateExpirationTime() error=%u",
			__FUNCTION__, __LINE__, error);
		/*TODO: release memory for pUSerRegInfo*/
		// error  = ECRIO_SIG_MGR_REGISTRATION_FAILED;
		error = ECRIO_SIGMGR_CRITCAL_ERROR_NEED_TO_EXIT;
		goto Error_Level_01;
	}

	pMandatoryHdrs = pUserRegInfo->pRegisterReq->pMandatoryHdrs;
	if (NULL == pMandatoryHdrs)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tpMandatoryHdrs is NULL", __FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	/* increment C Seq number*/
	pMandatoryHdrs->CSeq = (pUserRegInfo->cSeqNo)++;

	if (pMandatoryHdrs->pRequestUri == NULL)
	{
		reqUri.uriScheme = EcrioSigMgrURISchemeSIP;
		reqUri.u.pSipUri = &sipUri;
		reqUri.u.pSipUri->pDomain = pSigMgr->pSignalingInfo->pRegistrarDomain;

		error = _EcrioSigMgrAllocateAndPopulateUri(pSigMgr, &reqUri,
			&pMandatoryHdrs->pRequestUri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrPopulateUri() for pRequestUri, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	/*	Support for Authorization header in all request	start	*/
	if ((pRegReq && pRegReq->pAuthorization) ||
		(pSigMgr->pSharedCredentials == NULL) || (pSigMgr->pSharedCredentials->pAuth == NULL))
	{
		if (pSigMgr->pSharedCredentials)
		{
			error = EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_SharedCredentials, (void **)&(pSigMgr->pSharedCredentials), Enum_FALSE);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error_Level_01;
			}
		}
		else
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrSharedCredentialsStruct), (void **)&pSigMgr->pSharedCredentials);
			if (pSigMgr->pSharedCredentials == NULL)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryAllocate() for pSharedCredentials, error=%u",
					__FUNCTION__, __LINE__, error);
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
		}

		pal_MemoryAllocate(sizeof(EcrioSigMgrAuthorizationStruct), (void **)&pSigMgr->pSharedCredentials->pAuth);
		if (pSigMgr->pSharedCredentials->pAuth == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pAuth, error=%u",
				__FUNCTION__, __LINE__, error);
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		if (pRegReq && pRegReq->pAuthorization)
		{
			error = _EcrioSigMgrPopulateAuthorizationDetails(pSigMgr,
				pRegReq->pAuthorization,
				pSigMgr->pSharedCredentials->pAuth);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrPopulateAuthorizationDetails() for pAuth, error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}
		else
		{
			pSigMgr->pSharedCredentials->pAuth->authAlgo = pSigMgr->pSignalingInfo->eAuthAlgorithm;
			error = _EcrioSigMgrStringCreate(pSigMgr,
				ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_SCHEME,
				&pSigMgr->pSharedCredentials->pAuth->pAuthenticationScheme);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrStringCreate() for pAuthenticationScheme, error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}

			error = _EcrioSigMgrStringCreate(pSigMgr,
				pSigMgr->pSignalingInfo->pHomeDomain,
				&pSigMgr->pSharedCredentials->pAuth->pRealm);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrStringCreate() for pRealm, error=%u",
					__FUNCTION__, __LINE__, error);
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrFormUri(pSigMgr, pMandatoryHdrs->pRequestUri,
				&pSigMgr->pSharedCredentials->pAuth->pURI);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrFormUri() for pURI, error=%u",
					__FUNCTION__, __LINE__, error);
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrStringCreate(pSigMgr,
				pSigMgr->pSignalingInfo->pPrivateId,
				&pSigMgr->pSharedCredentials->pAuth->pUserName);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrStringCreate() for pUserName, error=%u",
					__FUNCTION__, __LINE__, error);
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			/* Fill empty string for Nonce */
			error = _EcrioSigMgrStringCreate(pSigMgr, (u_char *)"",
				&pSigMgr->pSharedCredentials->pAuth->pNonce);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrStringCreate() for pNonce, error=%u",
					__FUNCTION__, __LINE__, error);
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			/* Fill empty string for Response */
			error = _EcrioSigMgrStringCreate(pSigMgr, (u_char *)"",
				&pSigMgr->pSharedCredentials->pAuth->pResponse);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrStringCreate() for pResponse, error=%u",
					__FUNCTION__, __LINE__, error);
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
		}

		pSigMgr->pSharedCredentials->bUpdatedFromChallengedResp = Enum_TRUE;
	}

	/*	Support for Authorization header in all request	end	*/

	/*	Support for Authorization header in all request	start	*/
	if (pUserRegInfo && pUserRegInfo->pRegisterReq)
	{
		/* Populate the Authorization header details from the cached Shared Credentials
		** if updated from the 401/407 challeng response, or the retrieved Shared
		** Credentials from the stored credentials in SUE Extn. If the nonce value is
		** present, it also computes the digest response parameter and populates the
		** same in the Authorization header.
		*/
		error = _EcrioSigMgrCopyCredentials(pSigMgr,
			(u_char *)ECRIO_SIG_MGR_METHOD_REGISTER,
			NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrCopyCredentials() for REGISTER request, error=%u",
				__FUNCTION__, __LINE__, error);
			// error  = ECRIO_SIG_MGR_REGISTRATION_FAILED;
			error = ECRIO_SIGMGR_CRITCAL_ERROR_NEED_TO_EXIT;
			goto Error_Level_01;
		}

		/* referencing the pointer, it should be set to NULL after message creation instead of free. */
		pUserRegInfo->pRegisterReq->pAuthorization = pSigMgr->pSharedCredentials->pAuth;

		pUserRegInfo->pRegisterReq->eMethodType = EcrioSipMessageTypeRegister;
		pUserRegInfo->pRegisterReq->eReqRspType = EcrioSigMgrSIPRequest;
	}

	if (pMandatoryHdrs && pMandatoryHdrs->pFrom && pMandatoryHdrs->pFrom->ppParams)
	{
		for (i = 0; i < pMandatoryHdrs->pFrom->numParams; i++)
		{
			if (pMandatoryHdrs->pFrom->ppParams[i] &&
				pMandatoryHdrs->pFrom->ppParams[i]->pParamName &&
				pal_StringCompare(pMandatoryHdrs->pFrom->ppParams[i]->pParamName, (u_char *)"tag") == 0)
			{
				pal_MemoryFree((void **)&pMandatoryHdrs->pFrom->ppParams[i]->pParamValue);
				error = _EcrioSigMgrGenerateTag(&pMandatoryHdrs->pFrom->ppParams[i]->pParamValue);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrGenerateTag() error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}

				break;
			}
		}
	}

	error = _EcrioSigMgrPopulateMandatoryHeaders(pSigMgr,
		pUserRegInfo->pRegisterReq,
		EcrioSigMgrSIPRequest, Enum_FALSE, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateMandatoryHeaders() error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	error = _EcrioSigMgrPopulateCommonInfo(pSigMgr, pUserRegInfo->pRegisterReq->pMandatoryHdrs, &commonInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateCommonInfo() error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	error = _EcrioSigMgrSipMessageForm(pSigMgr, EcrioSigMgrSipMessageModeFullName, pUserRegInfo->pRegisterReq, &pReqData, &reqLen);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSipMessageForm() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_IMS_LIB_ERROR;
		goto Error_Level_01;
	}

	if (pSigMgr->pSignalingInfo->bEnableTls == Enum_TRUE && pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex != 0)
	{
		eTransportType = EcrioSigMgrTransportTLS;
	}
	else if ((pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE) && (reqLen > pSigMgr->pSignalingInfo->uUdpMtu))
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
	else if (pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableUdp == Enum_FALSE)
	{
		eTransportType = EcrioSigMgrTransportTCP;
	}

	// referenced pointer set to NULL
	pUserRegInfo->pRegisterReq->pAuthorization = NULL;

	if (((pUserRegInfo->pCallIDStruct == NULL) || (pUserRegInfo->pCallIDStruct->pCallID == NULL)) &&
		(pMandatoryHdrs->pCallId != NULL))
	{
		if (pUserRegInfo->pCallIDStruct == NULL)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrCallIDStruct), (void **)&pUserRegInfo->pCallIDStruct);
			if (pUserRegInfo->pCallIDStruct == NULL)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryAllocate() for pCallIDStruct, error=%u",
					__FUNCTION__, __LINE__, error);
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pUserRegInfo->pCallIDStruct->pSigMgr = pSigMgr;
		}

		error = _EcrioSigMgrStringCreate(pSigMgr, pMandatoryHdrs->pCallId,
			&pUserRegInfo->pCallIDStruct->pCallID);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrStringCreate() for pCallID, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	error = _EcrioSigMgrStringCreate(pSigMgr,
		(u_char *)ECRIO_SIG_MGR_METHOD_REGISTER, &commonInfo.pMethodName);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrStringCreate() for pMethodName, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	commonInfo.role = EcrioSigMgrRoleUAC;

	/* decide retransmission context*/
	if (pUserRegInfo->registrationState == EcrioSigMgrStateInitialized)
	{
		txnInfo.currentContext = ECRIO_SIGMGR_REGISTER_REQUEST;
		txnInfo.retransmitContext = ECRIO_SIGMGR_REGISTER_REQUEST_RESEND;
	}
	else if (eRegReq == EcrioSigMgrRegisterRequestType_Reg)
	{
		txnInfo.currentContext = ECRIO_SIGMGR_REREGISTER_REQUEST;
		txnInfo.retransmitContext = ECRIO_SIGMGR_REREGISTER_REQUEST_RESEND;
	}
	else if (eRegReq == EcrioSigMgrDeRegisterRequestType_DeReg)
	{
		txnInfo.currentContext = ECRIO_SIGMGR_DEREGISTER_REQUEST;
		txnInfo.retransmitContext = ECRIO_SIGMGR_DEREGISTER_REQUEST_RESEND;
	}

	txnInfo.deleteflag = Enum_FALSE;
	txnInfo.messageType = EcrioSigMgrMessageNonInviteRequest;
	txnInfo.transport = eTransportType;
	txnInfo.pSessionMappingStr = NULL;
	txnInfo.pMsg = pReqData;
	txnInfo.msglen = reqLen;

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeSIP,
		"%s:%u\tOutgoing SIP message constructed:", __FUNCTION__, __LINE__);
	SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pReqData, reqLen);

	/* send message to TXN handler*/
	error = _EcrioSigMgrSendMsgToTXNHandler(pSigMgr,
		&commonInfo, &txnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendMsgToTXNHandler() error=%u",
			__FUNCTION__, __LINE__, error);
		// error = ECRIO_SIGMGR_TRANSACTION_FAILED;
		pUserRegInfo->registrationState = EcrioSigMgrStateInitialized;

		if (error != ECRIO_SIG_MGR_SIG_SOCKET_ERROR)
			error = ECRIO_SIG_MGR_REGISTRATION_FAILED;

		goto Error_Level_01;
	}

	if (pUserRegInfo->registrationState == EcrioSigMgrStateAuthenticationRequired)
	{
		(pUserRegInfo->authRetryCount)++;
	}

	if (eRegReq == EcrioSigMgrRegisterRequestType_Reg)
	{
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeState, "%s:%u\t - Changing Signaling Manager Registration State from %s to %s", __FUNCTION__, __LINE__, _EcrioSigMgrGetRegistrationStateString(pUserRegInfo->registrationState), _EcrioSigMgrGetRegistrationStateString(EcrioSigMgrStateRegistrationInProgress));
		pUserRegInfo->registrationState =
			EcrioSigMgrStateRegistrationInProgress;
	}
	else if (eRegReq == EcrioSigMgrDeRegisterRequestType_DeReg)
	{
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeState, "%s:%u\t - Changing Signaling Manager Registration State from %s to %s", __FUNCTION__, __LINE__, _EcrioSigMgrGetRegistrationStateString(pUserRegInfo->registrationState), _EcrioSigMgrGetRegistrationStateString(EcrioSigMgrStateDeRegistrationInProgress));
		pUserRegInfo->registrationState =
			EcrioSigMgrStateDeRegistrationInProgress;

		if (pUserRegInfo->reregistrationTimerId != NULL)
		{
			pal_TimerStop(pUserRegInfo->reregistrationTimerId);
			pUserRegInfo->reregistrationTimerId = NULL;
		}

		if (pUserRegInfo->bIsRegExpireTimerRunning == Enum_TRUE)
		{
			pal_TimerStop(pUserRegInfo->uRegExpireTimerID);
			pUserRegInfo->uRegExpireTimerID = NULL;
			pUserRegInfo->bIsRegExpireTimerRunning = Enum_FALSE;
		}
	}

Error_Level_01:
	if (pReqData != NULL)
	{
		pal_MemoryFree((void **)&pReqData);
		pReqData = NULL;
		reqLen = 0;
	}

	if ((error != ECRIO_SIGMGR_NO_ERROR) &&
		(pUserRegInfo != NULL) &&
		(pUserRegInfo->registrationState ==	EcrioSigMgrStateInitialized))
	{
		_EcrioSigMgrUpdateUserRegInfoOnInitState(pSigMgr, pUserRegInfo, Enum_TRUE);
	}

	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &commonInfo);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************

Function:		_EcrioSigMgrCreateUserRegInfo()

Purpose:		Creates a user reg info object and add it to the list.

Description:	Creates a user reg info object and add it to the list.

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.

OutPut:			EcrioSigMgrUserRegisterInfoStruct** ppUserRegInfo - User reg
                info object.

Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrCreateUserRegInfo
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUserRegisterInfoStruct **ppUserRegInfo
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo = NULL;
	EcrioSigMgrRegisterInfoStruct *pRegInfoStruct = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpSigMgr=%p, ppUserRegInfo=%p",
		__FUNCTION__, __LINE__, pSigMgr, ppUserRegInfo);

	if ((pSigMgr->pRegisterInfo == NULL) || (ppUserRegInfo == NULL))
	{
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInvalid data passed, pSigMgr->pRegisterInfo=%x, ppUserRegInfo=%p",
			__FUNCTION__, __LINE__, pSigMgr->pRegisterInfo, ppUserRegInfo);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	*ppUserRegInfo = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\tAllocate memory for User Reg Info Object", __FUNCTION__, __LINE__);

	pal_MemoryAllocate(sizeof(EcrioSigMgrUserRegisterInfoStruct), (void **)&pUserRegInfo);
	if (pUserRegInfo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for pUserRegInfo, error=%u",
			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	pal_MemorySet((void *)pUserRegInfo, 0, sizeof(EcrioSigMgrUserRegisterInfoStruct));

	pUserRegInfo->regId = pal_UtilityRandomNumber();

	/* Initialize CSeqNo to 1 */
	pUserRegInfo->cSeqNo = 1;

	pRegInfoStruct = (EcrioSigMgrRegisterInfoStruct *)pSigMgr->pRegisterInfo;
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\tAdd User Reg Info object to DS List",
		__FUNCTION__, __LINE__);

	error = EcrioDSListInsertData(pRegInfoStruct->registerInformationList,
		(void *)pUserRegInfo);
	if (error != ECRIO_DS_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tEcrioDSListInsertData() for pUserRegInfo, error=%u",
			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_UNKNOWN_ERROR;
		goto Error_Level_02;
	}

	*ppUserRegInfo = pUserRegInfo;
	pUserRegInfo = NULL;

Error_Level_02:

Error_Level_01:
	if ((error != ECRIO_SIGMGR_NO_ERROR) && (pUserRegInfo != NULL))
	{
		(void)_EcrioSigMgrInternalStructRelease(pSigMgr,
			EcrioSigMgrStructType_UserRegisterInfo,
			(void **)&pUserRegInfo, Enum_TRUE);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);
	return error;
}

/*****************************************************************************

Function:		_EcrioSigMgrCompareRegInfoFunc()

Purpose:		Callback function to be used by list DS to compare two nodes.

Description:	Callback function to be used by list DS to compare two nodes.

Input:			void *pData1 - User Reginfo object.
                void *pData2 - User Reginfo object.
                void *pCallbackData - callback data usually owner of list.

OutPut:			None

Returns:		1 if not found else 0.
*****************************************************************************/
s_int32 _EcrioSigMgrCompareRegInfoFunc
(
	void *pData1,
	void *pData2,
	void *pCallbackData
)
{
	s_int32 retVal = 1;
	EcrioSigMgrUserRegisterInfoStruct *pRegData1 =
		(EcrioSigMgrUserRegisterInfoStruct *)pData1;
	EcrioSigMgrUserRegisterInfoStruct *pRegData2 =
		(EcrioSigMgrUserRegisterInfoStruct *)pData2;

	EcrioSigMgrStruct *pSigMgr = (EcrioSigMgrStruct *)pCallbackData;

	pSigMgr = pSigMgr;	/* Removing level-4 warning */

	if (pSigMgr != NULL)
	{
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
			__FUNCTION__, __LINE__);

		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpData1=%p, pData2=%p",
			__FUNCTION__, __LINE__, pData1, pData2);
	}

	if ((pData1 == NULL) || (pData2 == NULL))
	{
		if (pSigMgr != NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tInsufficient data", __FUNCTION__, __LINE__);
		}

		retVal = 1;
		goto Error_Level_01;
	}

	if (pRegData1->regId == pRegData2->regId)
	{
		retVal = 0;
	}

Error_Level_01:
	if (pSigMgr != NULL)
	{
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\tretval=%d",
			__FUNCTION__, __LINE__, retVal);
	}

	return retVal;
}

/*****************************************************************************

Function:		_EcrioSigMgrDestroyRegInfoFunc()

Purpose:		Callback function to be used by list DS to deletes a node.

Description:	Callback function to be used by list DS to deletes a node.

Input:			void *pData1 - User Reginfo object.
                void *pCallbackData - callback data usually owner of list.

Returns:		error code.
*****************************************************************************/
void _EcrioSigMgrDestroyRegInfoFunc
(
	void *pData,
	void *pCallbackData
)
{
	EcrioSigMgrUserRegisterInfoStruct *pRegData =
		(EcrioSigMgrUserRegisterInfoStruct *)pData;

	EcrioSigMgrStruct *pSigMgr = (EcrioSigMgrStruct *)pCallbackData;

	if (pSigMgr == NULL)
	{
		return;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpData=%p",
		__FUNCTION__, __LINE__, pData);

	if (pRegData == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data", __FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	_EcrioSigMgrInternalStructRelease(pSigMgr, EcrioSigMgrStructType_UserRegisterInfo,
		(void **)&pRegData, Enum_TRUE);

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}

/*****************************************************************************

Function:		_EcrioSigMgrRemoveUserRegInfo()

Purpose:		Function to removed a user reg info object from the list.

Description:	Function to removed a user reg info object from the list.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager instance.
                u_int32* pRegId - used to find a node in the list.

OutPut:			None

Returns:		error code
*****************************************************************************/
u_int32 _EcrioSigMgrRemoveUserRegInfo
(
	EcrioSigMgrStruct *pSigMgr,
	u_int32 *pRegId
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 i = 0, size = 0;
	EcrioSigMgrRegisterInfoStruct *pRegInfoStruct = NULL;
	EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpSigMgr=%p, pRegId=%p",
		__FUNCTION__, __LINE__, pSigMgr, pRegId);

	if (pRegId == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	pRegInfoStruct = (EcrioSigMgrRegisterInfoStruct *)pSigMgr->pRegisterInfo;
	if (pRegInfoStruct == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tRegistration Info List not present",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	error = EcrioDSListGetSize(pRegInfoStruct->registerInformationList,
		&size);
	if (error != ECRIO_DS_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tEcrioDSListGetSize() for registerInformationList, error=%u",
			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_UNKNOWN_ERROR;
		goto Error_Level_01;
	}

	for (i = 1; i <= size; i++)
	{
		error = EcrioDSListGetDataAt(pRegInfoStruct->registerInformationList,
			i, (void **)&pUserRegInfo);
		if ((pUserRegInfo != NULL) && (pUserRegInfo->regId == *pRegId))
		{
			(void)EcrioDSListRemoveDataAt(pRegInfoStruct->
				registerInformationList, i);
			break;
		}
	}

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************

  Function:		_EcrioSigMgrGetUserRegInfo()

  Purpose:		Gets the registration details from the Signaling Manager
                Instance.

  Description:	Iterates through the registration info list, compares each
                node's callid with the registration identifier. If comparison
                fails for every node, NULL is returned.

  Input:		EcrioSigMgrStruct* pSigMgr - SigMgr instance.
                void* ppRegistrationId - Registration Id.

  OutPut:		EcrioSigMgrUserRegisterInfoStruct** ppUserRegInfo -
                Registration info. object.

  Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrGetUserRegInfo
(
	EcrioSigMgrStruct *pSigMgr,
	void *pRegistrationId,
	EcrioSigMgrUserRegisterInfoStruct **ppUserRegInfo
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 i = 0, size = 0;
	EcrioSigMgrRegisterInfoStruct *pRegInfoStruct = NULL;
	EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
		"%s:%u\tpSigMgr=%p, pRegistrationId=%p",
		__FUNCTION__, __LINE__, pSigMgr, pRegistrationId);

	if ((pRegistrationId == NULL) || (ppUserRegInfo == NULL))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	*ppUserRegInfo = NULL;

	size = pal_StringLength(pRegistrationId);
	if (!size)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInvalid Registration Id", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	if (pSigMgr->pRegisterInfo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tRegistration Info List not present", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	pRegInfoStruct = (EcrioSigMgrRegisterInfoStruct *)pSigMgr->pRegisterInfo;

	error = EcrioDSListGetSize(pRegInfoStruct->registerInformationList,
		&size);
	if (error != ECRIO_DS_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tEcrioDSListGetSize() for registerInformationList, error=%u",
			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_UNKNOWN_ERROR;
		goto Error_Level_02;
	}

	for (i = 1; i <= size; ++i)
	{
		EcrioDSListGetDataAt(pRegInfoStruct->registerInformationList, i,
			(void **)&pUserRegInfo);

		if ((pUserRegInfo != NULL) && (pUserRegInfo->pRegisterReq != NULL) &&
			(pUserRegInfo->pRegisterReq->pMandatoryHdrs != NULL) &&
			(pal_StringCompare((u_char *)pRegistrationId, pUserRegInfo->
			pRegisterReq->pMandatoryHdrs->pCallId) == 0))
		{
			SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tUser reg object found pUserRegInfo=%p, callid=%s",
				__FUNCTION__, __LINE__, pUserRegInfo, (u_char *)pRegistrationId);
			*ppUserRegInfo = pUserRegInfo;
			break;
		}
	}

	if (*ppUserRegInfo == NULL)
	{
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tUser reg object NOT found callid=%s",
			__FUNCTION__, __LINE__, (u_char *)pRegistrationId);
		error = ECRIO_SIGMGR_ID_NOT_FOUND;
	}

Error_Level_02:

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************

Function:		_EcrioSigMgrUpdateRegInfo()

Purpose:		Updates header values in User reg info object.

Description:	Creates Mandatory header, Optional header objects and
                store / updates header values from upper layer.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                EcrioSigMgrRegisterReqStruct* pRegReq - Registration request
                Headers.
                EcrioSigMgrUserRegisterInfoStruct* pUserRegInfo - User reg
                info object.

Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrUpdateRegInfo
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrRegisterRequestEnum reqType,
	EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrMandatoryHeaderStruct *pMandatoryHdrs = NULL;
	u_char *ppSupported[] = { (u_char *)ECRIO_SIGMGR_SUPPORTED_VALUE_PATH };
	u_int16	numSupported = 0;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pUserRegInfo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tInsufficient data", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	/* moduleId = pSigMgr->moduleId; */

	/* Create register request structure.*/
	if (pUserRegInfo->pRegisterReq == NULL)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrSipMessageStruct), (void **)&(pUserRegInfo->pRegisterReq));
		if (pUserRegInfo->pRegisterReq == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pRegisterReq, error=%u",
				__FUNCTION__, __LINE__, error);
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}
	}

	pUserRegInfo->pRegisterReq->eMethodType = EcrioSipMessageTypeRegister;

	pMandatoryHdrs = pUserRegInfo->pRegisterReq->pMandatoryHdrs;
	/* creates mandatory header structure.*/
	if (pMandatoryHdrs == NULL)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&(pUserRegInfo->pRegisterReq->pMandatoryHdrs));
		pMandatoryHdrs = pUserRegInfo->pRegisterReq->pMandatoryHdrs;
		if (pMandatoryHdrs == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pMandatoryHdrs, error=%u",
				__FUNCTION__, __LINE__, error);
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}
	}

	if ((pUserRegInfo->pCallIDStruct != NULL) && (pUserRegInfo->pCallIDStruct->pCallID != NULL))
	{
		if (pMandatoryHdrs->pCallId != NULL)
		{
			pal_MemoryFree((void **)&pMandatoryHdrs->pCallId);
		}

		error = _EcrioSigMgrStringCreate(pSigMgr, pUserRegInfo->pCallIDStruct->pCallID,
			&pMandatoryHdrs->pCallId);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrStringCreate() for pCallId, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	/* if TO URI is not provided, store the local uri, since registration should be done*/
	/* for local party.*/
	if (pMandatoryHdrs->pTo == NULL)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct), (void **)&(pMandatoryHdrs->pTo));
		if (pMandatoryHdrs->pTo == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pTo, error=%u",
				__FUNCTION__, __LINE__, error);
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		if (pSigMgr->pSignalingInfo->pDisplayName != NULL)
		{
			error = _EcrioSigMgrStringCreate(pSigMgr,
				pSigMgr->pSignalingInfo->pDisplayName,
				&pMandatoryHdrs->pTo->nameAddr.pDisplayName);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrStringCreate() for pDisplayName, error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}

		{
			EcrioSigMgrSipURIStruct *pSipUri = NULL;

			pal_MemoryAllocate(sizeof(EcrioSigMgrSipURIStruct), (void **)&(pMandatoryHdrs->pTo->nameAddr.addrSpec.u.pSipUri));
			if (pMandatoryHdrs->pTo->nameAddr.addrSpec.u.pSipUri == NULL)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryAllocate() for pSipUri, error=%u",
					__FUNCTION__, __LINE__, error);
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pSipUri = pMandatoryHdrs->pTo->nameAddr.addrSpec.u.pSipUri;
			pal_MemorySet(pSipUri, 0, sizeof(EcrioSigMgrSipURIStruct));

			error = _EcrioSigMgrStringCreate(pSigMgr,
				pSigMgr->pSignalingInfo->pHomeDomain,
				&pSipUri->pDomain);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrStringCreate() for pDomain, error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}

			error = _EcrioSigMgrStringCreate(pSigMgr,
				pSigMgr->pSignalingInfo->pUserId,
				&pSipUri->pUserId);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrStringCreate() for pUserId, error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}

			pMandatoryHdrs->pTo->nameAddr.addrSpec.uriScheme =
				EcrioSigMgrURISchemeSIP;
		}
	}

	/* Create optional header object*/
	if (pUserRegInfo->pRegisterReq->pOptionalHeaderList == NULL)
	{
		error = _EcrioSigMgrUpdateMessageRequestStruct(pSigMgr, pUserRegInfo->pRegisterReq, Enum_FALSE);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tOptional Header Creation error, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		numSupported = (u_int16)(sizeof(ppSupported) / sizeof(u_char *));
		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pUserRegInfo->pRegisterReq->pOptionalHeaderList, EcrioSipHeaderTypeSupported, numSupported, ppSupported, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}

		if (pSigMgr->pOOMObject)
		{
			if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeRegister, EcrioSipHeaderTypeP_AccessNetworkInfo))
			{
				error = _EcrioSigMgrAddP_AccessNWInfoHeader(pSigMgr, pUserRegInfo->pRegisterReq->pOptionalHeaderList);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					goto Error_Level_01;
				}
			}

			if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeRegister, EcrioSipHeaderTypeP_LastAccessNetworkInfo))
			{
				error = _EcrioSigMgrAddP_LastAccessNWInfoHeader(pSigMgr, pUserRegInfo->pRegisterReq->pOptionalHeaderList);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					goto Error_Level_01;
				}
			}
		}
	}

	if ((pSigMgr->bIPSecEnabled == Enum_TRUE) && (pUserRegInfo->pRegisterReq->pOptionalHeaderList != NULL))
	{
		ppSupported[0] = (u_char *)ECRIO_SIG_MGR_HEADER_VALUE_SEC_AGREE;
		numSupported = (u_int16)(sizeof(ppSupported) / sizeof(u_char *));

		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pUserRegInfo->pRegisterReq->pOptionalHeaderList, EcrioSipHeaderTypeSupported, numSupported, ppSupported, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}

		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pUserRegInfo->pRegisterReq->pOptionalHeaderList, EcrioSipHeaderTypeRequire, numSupported, ppSupported, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}

		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pUserRegInfo->pRegisterReq->pOptionalHeaderList, EcrioSipHeaderTypeProxyRequire, numSupported, ppSupported, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}

		if (pSigMgr->pIPSecDetails->eIPSecState != ECRIO_SIP_IPSEC_STATE_ENUM_Temporary)
		{
			error = _EcrioSigMgrCreateSecurityClientHdr(pSigMgr, pUserRegInfo->pRegisterReq->pOptionalHeaderList);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error_Level_01;
			}
		}

		error = _EcrioSigMgrCreateSecurityVerifyHdr(pSigMgr, &pUserRegInfo->pRegisterReq->pOptionalHeaderList);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	pUserRegInfo->reqType = reqType;

Error_Level_01:
	if ((error != ECRIO_SIGMGR_NO_ERROR) && (pUserRegInfo != NULL))
	{
		/* Cleanup pUserRegInfo structure, but not pUserRegInfo pointer. */
		_EcrioSigMgrInternalStructRelease(pSigMgr,
			EcrioSigMgrStructType_UserRegisterInfo,
			(void **)&pUserRegInfo, Enum_FALSE);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************

Function:		_EcrioSigMgrReleaseRegInfo()

Purpose:		Function used to release structure which contains
                registration info list.

Description:	Function used to release structure which contains
                registration info list.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.

Output:			void** ppRegisterInfo - Registration info object.

Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrReleaseRegInfo
(
	EcrioSigMgrStruct *pSigMgr,
	void **ppRegisterInfo
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrRegisterInfoStruct *pRegisterInfoStruct = NULL;

	if (pSigMgr == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		goto Error_Level_01;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
		"%s:%u\tpSigMgr=%p, ppRegisterInfo=%p",
		__FUNCTION__, __LINE__, pSigMgr, ppRegisterInfo);

	if ((ppRegisterInfo == NULL) || (*ppRegisterInfo == NULL))
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data", __FUNCTION__, __LINE__);
		goto Error_Level_02;
	}

	pRegisterInfoStruct = (EcrioSigMgrRegisterInfoStruct *)(*ppRegisterInfo);

	/* Release the register info */
	_EcrioSigMgrInternalStructRelease(pSigMgr, EcrioSigMgrStructType_RegisterInfo,
		(void **)&(pRegisterInfoStruct), Enum_TRUE);

	*ppRegisterInfo = NULL;

Error_Level_02:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

Error_Level_01:
	return error;
}

/*****************************************************************************

Function:		_EcrioSigMgrHandleRegResponse()

Purpose:		Function used to handle registration response.

Description:	handles registration response, sends notification to upper
                layer, and updates registration states.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                EcrioSigMgrRegisterResponseStruct* pRegResp - Registration
                response Headers.

Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrHandleRegResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pRegResp
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	u_int32	responseCategory = 0, i = 0;
	u_int32 *pRegId = NULL;
	BoolEnum isDeRegResponse = Enum_FALSE;
	BoolEnum requestTerminated = Enum_FALSE;
	BoolEnum sendStatusCallback = Enum_TRUE;
	BoolEnum bPositiveResponse = Enum_FALSE;
	EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo = NULL;
	EcrioSigMgrMandatoryHeaderStruct *pManHdrs = NULL, *pUserManHdrs = NULL;
	EcrioSigMgrInfoStruct sigMgrInfo = { .eventType = EcrioSigMgrMessageNone };
	EcrioSigMgrMessageStruct message = { .msgCmd = EcrioSigMgrMessageNone };

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	responseCategory = pRegResp->responseCode / 100;
	if (responseCategory > ECRIO_SIGMGR_6XX_RESPONSE)
	{
		error = ECRIO_SIGMGR_BAD_MESSAGE_RECEIVED;
		goto Error_Level_01;
	}

	pManHdrs = pRegResp->pMandatoryHdrs;

	error = _EcrioSigMgrGetUserRegInfo(pSigMgr, pManHdrs->pCallId,
		&pUserRegInfo);
	if ((pUserRegInfo == NULL) || (pUserRegInfo->pRegisterReq == NULL))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrGetUserRegInfo() bad message received, error=%u",
			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_BAD_MESSAGE_RECEIVED;
		goto Error_Level_01;
	}

	pUserManHdrs = pUserRegInfo->pRegisterReq->pMandatoryHdrs;

	isDeRegResponse = _EcrioSigMgrIsDeRegResponse(pSigMgr, pRegResp);

	/* Populate the Notification struct */
	message.msgCmd = ((pUserRegInfo->reqType != EcrioSigMgrDeRegisterRequestType_DeReg) ?
		EcrioSigMgrRegisterReponse : EcrioSigMgrDeRegisterReponse);
	message.pData = (void *)pRegResp;

	/* Clear service route */
	if ((responseCategory != ECRIO_SIGMGR_1XX_RESPONSE) &&
		(message.msgCmd == EcrioSigMgrRegisterReponse))
	{
		if (pSigMgr->pServiceRoute)
		{
			_EcrioSigMgrClearServiceRoute(pSigMgr);
		}
	}

	if ((pUserManHdrs != NULL) && (pUserManHdrs->numVia > 0))
	{
		for (i = 0; i < pManHdrs->numVia; ++i)
		{
			EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_Via,
				(void **)&pUserManHdrs->ppVia[i], Enum_TRUE);
		}

		pal_MemoryFree((void **)&pUserManHdrs->ppVia);
		pUserManHdrs->numVia = 0;
	}

	/* 401 response received.*/
	if (pRegResp->responseCode == ECRIO_SIGMGR_RESPONSE_CODE_UNAUTHORIZED)
	{
		u_char *pRequestURI = NULL;

		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeState, "%s:%u\t - Changing Signaling Manager Registration State from %s to %s", __FUNCTION__, __LINE__, _EcrioSigMgrGetRegistrationStateString(pUserRegInfo->registrationState), _EcrioSigMgrGetRegistrationStateString(EcrioSigMgrStateAuthenticationRequired));
		pUserRegInfo->registrationState = EcrioSigMgrStateAuthenticationRequired;

		/*	Support for Authorization header in all request	start	*/
		/* Retrieve and store the WWW-Authenticate header received in the 401 Response */
		if (pUserManHdrs && pRegResp->pAuthentication)
		{
			error = _EcrioSigMgrFormUri(pSigMgr,
				pUserManHdrs->pRequestUri, &pRequestURI);
			if (pRequestURI)
			{
				/* Update the Shared Credentials Auth structure from the challenge
				** received in the 401 response. If the AKA authentication algorithm is
				** used, it also computes and stores the AKA response (RES/AUTS, CK and IK).
				** A flag is set so that the cached SharedCredential will be used while
				** sending the subsequent REGISTER with credential instead of fetching it
				** from SUEExtn.
				*/
				error = _EcrioSigMgrUpdateSharedCredentialsFrom401407Resp(
					pSigMgr,
					pRegResp->pAuthentication,
					pRequestURI,
					NULL,	/* entity-body not supported */
					EcrioSigMgrMethodRegister,
					ECRIO_SIGMGR_RESPONSE_CODE_UNAUTHORIZED);

				pal_MemoryFree((void **)&pRequestURI);
			}

			/* Check MAC error before proceeding further */
			if (error != ECRIO_SIGMGR_NO_ERROR && error != ECRIO_SIGMGR_AKA_MAC_FAILURE)
			{
				/* Error while storing Shared Credentials.. Treat 401 as other Failure Response */
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrUpdateSharedCredentialsFrom401407Resp() error=%u",
					__FUNCTION__, __LINE__, error);
				requestTerminated = Enum_TRUE;
				pUserRegInfo->authRetryCount = 0;
			}
		}
		else
		{
			/* WWW-Authenticate header not present in 401, treat as other failure response */
			requestTerminated = Enum_TRUE;
			pUserRegInfo->authRetryCount = 0;
			error = ECRIO_SIGMGR_CRITCAL_ERROR_NEED_TO_EXIT;
		}

		if (Enum_TRUE == pSigMgr->bIPSecEnabled)
		{
			//EcrioSipIPsecNegotiatedParamStruct agreedIPsecInfo = { NULL };
			BoolEnum bValidSecurityServer = Enum_FALSE;

			pSigMgr->pIPSecDetails->ipsecRegId = pUserRegInfo->regId;
			/* Check for existence of Security-Server header in 401 Response */
			error = _EcrioSigMgrExtractSecurityServerFromRegisterResponse(pSigMgr,
				pRegResp, &bValidSecurityServer);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error_Level_01;
			}

			if (bValidSecurityServer == Enum_TRUE)
			{
				/* Update negotiated details */
				pSigMgr->pIPSecDetails->pIPsecNegParams->uPort_uc = pSigMgr->pIPSecDetails->pIPsecParams->uPort_uc;
				pSigMgr->pIPSecDetails->pIPsecNegParams->uPort_us = pSigMgr->pIPSecDetails->pIPsecParams->uPort_us;
				pSigMgr->pIPSecDetails->pIPsecNegParams->uSpi_uc = pSigMgr->pIPSecDetails->pIPsecParams->uSpi_uc;
				pSigMgr->pIPSecDetails->pIPsecNegParams->uSpi_us = pSigMgr->pIPSecDetails->pIPsecParams->uSpi_us;
				message.msgCmd = EcrioSigMgrIPSecSecurityServerNotification;
				message.pData = (void *)pSigMgr->pIPSecDetails->pIPsecNegParams;
				_EcrioSigMgrUpdateStatus(pSigMgr, &message);
				goto Error_Level_01;
			}
		}
		/*	if 2nd 401 contains stale = FALSE then no further retry else 3rd REGISTER is sent,
		** if the 3rd 401 comes with stale = TRUE then further retry after a specified time
		** and the 4th 401 will be teated as the first 401
		*/
		if (pUserRegInfo->authRetryCount >= 2)
		{
			if (pRegResp->pAuthentication->bStale == Enum_FALSE)
			{
				SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t2nd 401 contains stale = FALSE then no further retry, so SUE will be exit.", __FUNCTION__, __LINE__);

				requestTerminated = Enum_TRUE;
				pUserRegInfo->authRetryCount = 0;
				error = ECRIO_SIGMGR_CRITCAL_ERROR_NEED_TO_EXIT;
			}
			else
			{
				SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t2 authorization has already been made, hence next retry after a specified time",
					__FUNCTION__, __LINE__);
				pUserRegInfo->authRetryCount = 0;	/* set the authRetryCount as zero so that the 4th challenge
													** will be treated as the 1st (or new) challenge.
													*/

				_EcrioSigMgrUpdateUserRegInfoOnInitState(pSigMgr, pUserRegInfo, Enum_TRUE);

				/* send 3rd auth challenge recieved information to the upper layer*/
				sigMgrInfo.eventType = EcrioSigMgrRegisterRequest;
				sigMgrInfo.pEventIdentifier = (void *)&pRegId;
				_EcrioSigMgrSendInfo(pSigMgr, EcrioSigMgrInformation,
					ECRIO_SIGMGR_MORE_THEN_TWO_AUTH_RECEIVED, &sigMgrInfo);

				goto Error_Level_01;
			}
		}
		else
		{
			if (requestTerminated != Enum_TRUE)
			{
				/* Notify upper layer about the 401 response.
				 * The Upper Layer will be responsible for Resending REGISTER.
				 */
				sendStatusCallback = Enum_FALSE;

				error = _EcrioSigMgrSendRegister(pSigMgr,
					isDeRegResponse == Enum_FALSE ? EcrioSigMgrRegisterRequestType_Reg : EcrioSigMgrDeRegisterRequestType_DeReg,
					(void *)&pUserRegInfo->regId);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrSendRegister() error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}

				// _EcrioSigMgrUpdateStatus(pSigMgr, &message);
			}
		}
	}
	else if (pRegResp->responseCode == ECRIO_SIGMGR_RESPONSE_CODE_INTERVAL_TOO_BRIEF)
	{
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeState, "%s:%u\t - Changing Signaling Manager Registration State from %s to %s", __FUNCTION__, __LINE__, _EcrioSigMgrGetRegistrationStateString(pUserRegInfo->registrationState), _EcrioSigMgrGetRegistrationStateString(EcrioSigMgrStateInitialized));
		pUserRegInfo->registrationState = EcrioSigMgrStateInitialized;

		if (pUserRegInfo->pRegisterReq && pUserRegInfo->pRegisterReq->pExpires && pRegResp->pMinExpires)
		{
			*(pUserRegInfo->pRegisterReq->pExpires) = *(pRegResp->pMinExpires);
			pSigMgr->pSignalingInfo->registrationExpires = *(pRegResp->pMinExpires);
		}

		{
			if (requestTerminated != Enum_TRUE)
			{
				/* Notify upper layer about the 401 response.
				* The Upper Layer will be responsible for Resending REGISTER.
				*/
				sendStatusCallback = Enum_FALSE;

				error = _EcrioSigMgrSendRegister(pSigMgr,
					isDeRegResponse == Enum_FALSE ? EcrioSigMgrRegisterRequestType_Reg : EcrioSigMgrDeRegisterRequestType_DeReg,
					(void *)&pUserRegInfo->regId);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrSendRegister() error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}

				// _EcrioSigMgrUpdateStatus(pSigMgr, &message);
			}
		}
	}
	else
	{
		pUserRegInfo->authRetryCount = 0;

		SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tRegResponse=%d", __FUNCTION__, __LINE__, responseCategory);

		switch (responseCategory)
		{
			/*case ECRIO_SIGMGR_1XX_RESPONSE:
			    pRegResp->status = ECRIO_SIGMGR_REQUEST_IN_PROGRESS;
			    break;*/

			case ECRIO_SIGMGR_2XX_RESPONSE:
			{
				if (pRegResp->pAuthenticationInfo)
				{
					EcrioSigMgrHeaderStruct *pAuthInfo = NULL;

					pAuthInfo = pRegResp->pAuthenticationInfo;
					if ((pAuthInfo->ppHeaderValues != NULL) &&
						(pAuthInfo->ppHeaderValues[0] != NULL) &&
						(pAuthInfo->ppHeaderValues[0]->ppParams != NULL))
					{
						EcrioSigMgrParamStruct *pParam = NULL;
						u_char *pTempNextNonce = NULL;
						u_char *pTempCNonce = NULL;
						u_char *pTempQoP = NULL;
						u_int32 uHeaderCount = 0;

						for (uHeaderCount = 0; uHeaderCount < pAuthInfo->numHeaderValues; uHeaderCount++)
						{
							for (i = 0; i < pAuthInfo->ppHeaderValues[uHeaderCount]->numParams; i++)
							{
								pParam = pAuthInfo->ppHeaderValues[uHeaderCount]->ppParams[i];

								if (!pParam || !pParam->pParamName || !pParam->pParamValue)
								{
									continue;
								}

								if (!pal_StringICompare(pParam->pParamName, (u_char *)"nextnonce"))
								{
									pTempNextNonce = pParam->pParamValue;
								}
								/* cnonce checking */
								else if (!pal_StringICompare(pParam->pParamName, (u_char *)"cnonce"))
								{
									pTempCNonce = pParam->pParamValue;
								}
								/* qop checking */
								else if (!pal_StringICompare(pParam->pParamName, (u_char *)"qop"))
								{
									pTempQoP = pParam->pParamValue;
								}
							}
						}

						/* Start - Support for Authentication-Info Header */
						if (pTempNextNonce)
						{
							/* delete NextNonce from Shared Credential */
							if (pSigMgr->pSharedCredentials->pAuth->pNonce != NULL)
							{
								pal_MemoryFree(
									(void **)&(pSigMgr->pSharedCredentials->pAuth->pNonce));
							}

							/* Update the nonce from nextnonce */
							pSigMgr->pSharedCredentials->pAuth->pNonce =
								_EcrioSigMgrStringUnquote(pTempNextNonce);
							if (pSigMgr->pSharedCredentials->pAuth->pNonce == NULL)
							{
								SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
									"%s:%u\t_EcrioSigMgrStringUnquote() for pNonce", __FUNCTION__, __LINE__);
								error = ECRIO_SIGMGR_NO_MEMORY;
								goto Error_Level_01;
							}

							/* Reset nc value for new nonce */
							pSigMgr->pSharedCredentials->authCount = 0;

							/* CNonce */
							if (pTempCNonce)
							{
								/* delete CNonce from Shared Credential */
								if (pSigMgr->pSharedCredentials->pAuth->pCNonce != NULL)
								{
									pal_MemoryFree(
										(void **)&(pSigMgr->pSharedCredentials->pAuth->pCNonce));
								}

								/* Update the cnonce to Shared Credential */
								pSigMgr->pSharedCredentials->pAuth->pCNonce =
									_EcrioSigMgrStringUnquote(pTempCNonce);
								if (pSigMgr->pSharedCredentials->pAuth->pCNonce == NULL)
								{
									SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
										"%s:%u\t_EcrioSigMgrStringUnquote() for pCNonce", __FUNCTION__, __LINE__);
									error = ECRIO_SIGMGR_NO_MEMORY;
									goto Error_Level_01;
								}
							}

							/* qop */
							if (pTempQoP)
							{
								/* delete qop from Shared Credential */
								if (pSigMgr->pSharedCredentials->pAuth->pQoP != NULL)
								{
									pal_MemoryFree(
										(void **)&(pSigMgr->pSharedCredentials->pAuth->pQoP));
								}

								/* Update the qop to Shared Credential */
								pSigMgr->pSharedCredentials->pAuth->pQoP =
									_EcrioSigMgrStringUnquote(pTempQoP);
								if (pSigMgr->pSharedCredentials->pAuth->pQoP == NULL)
								{
									SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
										"%s:%u\t_EcrioSigMgrStringUnquote() for pQoP", __FUNCTION__, __LINE__);
									error = ECRIO_SIGMGR_NO_MEMORY;
									goto Error_Level_01;
								}
							}
						}

						pTempNextNonce = NULL;
						pTempCNonce = NULL;
						pTempQoP = NULL;
						/* End - Support for Authentication-Info Header */
					}
				}

				if (pUserRegInfo->reqType != EcrioSigMgrRegisterRequestType_Fetching)
				{
					if (isDeRegResponse == Enum_FALSE)
					{
						/* Start No Activity Timer if not already started */
#ifdef SIG_NOP
						if ((pSigMgr->bEnableSBCNoOp == Enum_TRUE) &&
							(pSigMgr->noActivityTimerId == NULL))
						{
							_EcrioSigMgrStartNoActivityTimer(pSigMgr,
								pSigMgr->uNoActivityTimerInterval );
						}
#endif				// #ifdef SIG_NOP

						SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeState, "%s:%u\t - Changing Signaling Manager Registration State from %s to %s", __FUNCTION__, __LINE__, _EcrioSigMgrGetRegistrationStateString(pUserRegInfo->registrationState), _EcrioSigMgrGetRegistrationStateString(EcrioSigMgrStateRegistered));
						pUserRegInfo->registrationState = EcrioSigMgrStateRegistered;

						/* Update service route values */
						if (pRegResp->pServiceRoute != NULL)
						{
							_EcrioSigMgrAddServiceRoute(pSigMgr, pRegResp->pServiceRoute);
						}

						if (pRegResp->ppPAssociatedURI != NULL)
						{
							u_int32 iterator = 0;
							if (pSigMgr->pSignalingInfo->ppPAssociatedURI)
							{
								for (iterator = 0; iterator < pSigMgr->pSignalingInfo->numPAssociatedURIs; iterator++)
								{
									EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_NameAddrWithParams,
										(void **)&(pSigMgr->pSignalingInfo->ppPAssociatedURI[iterator]), Enum_TRUE);
								}

								pal_MemoryFree((void **)&(pSigMgr->pSignalingInfo->ppPAssociatedURI));
							}

							pSigMgr->pSignalingInfo->ppPAssociatedURI = pRegResp->ppPAssociatedURI;
							pRegResp->ppPAssociatedURI = NULL;
							pSigMgr->pSignalingInfo->numPAssociatedURIs = pRegResp->numPAssociatedURIs;
							pRegResp->numPAssociatedURIs = 0;
						}

						bPositiveResponse = Enum_TRUE;
						error = _EcrioSigMgrStartReRegTimer(pSigMgr,
							pRegResp,
							/* Memory Leak Fixing */
							// pManHdrs->pCallId, pUserRegInfo);
							// pUserRegInfo->pCallID, pUserRegInfo);
							pUserRegInfo->pCallIDStruct,
							pUserRegInfo,
							bPositiveResponse);
						if (error != ECRIO_SIGMGR_NO_ERROR)
						{
							requestTerminated = Enum_TRUE;
							SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
								"%s:%u\t_EcrioSigMgrStartReRegTimer() Error starting ReReg timer, error=%u",
								__FUNCTION__, __LINE__, error);
						}
					}
					else
					{
						/* Stop No Activity Timer if it is running */
#ifdef SIG_NOP
						if ((pSigMgr->bEnableSBCNoOp == Enum_TRUE) &&
							(pSigMgr->noActivityTimerId != NULL))
						{
							_EcrioSigMgrStopNoActivityTimer(pSigMgr);
						}
#endif				// #ifdef SIG_NOP
						if (pSigMgr->pServiceRoute)
						{
							_EcrioSigMgrClearServiceRoute(pSigMgr);
						}

						if (pUserRegInfo->registrationState == EcrioSigMgrStateDeRegistrationInProgress)
						{
							SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeState, "%s:%u\t - Changing Signaling Manager Registration State from %s to %s", __FUNCTION__, __LINE__, _EcrioSigMgrGetRegistrationStateString(pUserRegInfo->registrationState), _EcrioSigMgrGetRegistrationStateString(EcrioSigMgrStateInitialized));
							pUserRegInfo->registrationState = EcrioSigMgrStateInitialized;
						}
						else
						{
							requestTerminated = Enum_TRUE;
							SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
								"%s:%u\tRequest terminated", __FUNCTION__, __LINE__);
						}
					}
				}
				if (pSigMgr->bIPSecEnabled == Enum_TRUE)
				{
					if ((pUserRegInfo->registrationState == EcrioSigMgrStateRegistered) &&
						(pSigMgr->pIPSecDetails->eIPSecState == ECRIO_SIP_IPSEC_STATE_ENUM_Temporary))
					{
						pSigMgr->pIPSecDetails->eIPSecState = ECRIO_SIP_IPSEC_STATE_ENUM_Established;
					}
					else if ((pUserRegInfo->registrationState == EcrioSigMgrStateInitialized) &&
						(pSigMgr->pIPSecDetails->eIPSecState == ECRIO_SIP_IPSEC_STATE_ENUM_Established))
					{
						pSigMgr->pIPSecDetails->eIPSecState = ECRIO_SIP_IPSEC_STATE_ENUM_Expiring;
						_EcrioSigMgrFreeIPSecNegParams(pSigMgr);
					}
				}
			}
			break;

			case ECRIO_SIGMGR_4XX_RESPONSE:
			case ECRIO_SIGMGR_5XX_RESPONSE:
			case ECRIO_SIGMGR_6XX_RESPONSE:
			default:
			{
				if (isDeRegResponse == Enum_TRUE)
				{
					/* Stop No Activity Timer if it is running */
#ifdef SIG_NOP
					if ((pSigMgr->bEnableSBCNoOp == Enum_TRUE) &&
						(pSigMgr->noActivityTimerId != NULL))
					{
						_EcrioSigMgrStopNoActivityTimer(pSigMgr);
					}
#endif			// #ifdef SIG_NOP
				}

				// pRegResp->status = ECRIO_SIGMGR_REQUEST_FAILED;

				if (pUserRegInfo->reqType != EcrioSigMgrRegisterRequestType_Fetching)
				{
					/*pUserRegInfo->registrationState =
					    EcrioSigMgrStateInitialized;*/
					SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeState, "%s:%u\t - Changing Signaling Manager Registration State from %s to %s", __FUNCTION__, __LINE__, _EcrioSigMgrGetRegistrationStateString(pUserRegInfo->registrationState), _EcrioSigMgrGetRegistrationStateString(EcrioSigMgrStateInitialized));
					pUserRegInfo->registrationState =
						EcrioSigMgrStateInitialized;
				}
			}
			break;
		}

		/* Transactions are deleted if only TCP enabled */
		if (pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE &&
			pSigMgr->pSignalingInfo->bEnableUdp == Enum_FALSE)
		{
			if (pUserManHdrs != NULL && pUserManHdrs->pCallId != NULL)
			{
				EcrioTXNReturnCodeEnum eTxnErr = EcrioTXNNoError;
				eTxnErr = EcrioTXNDeleteMatchedTransactions(pSigMgr->pTransactionMgrHndl,
					pUserManHdrs->pCallId);
				SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tEcrioTXNDeleteMatchedTransactions() returned with = %d", __FUNCTION__, __LINE__, eTxnErr);
			}
		}
	}

	// pUserRegInfo->reqType = EcrioSigMgrRegisterRequestType_Reg;

	if (requestTerminated == Enum_TRUE)
	{
		sigMgrInfo.eventType = EcrioSigMgrRegisterRequest;
		sigMgrInfo.pEventIdentifier = (void *)&pRegId;
		_EcrioSigMgrUpdateUserRegInfoOnInitState(pSigMgr, pUserRegInfo, Enum_TRUE);

		SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\trequestTerminated=%d", __FUNCTION__, __LINE__, requestTerminated);

		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			_EcrioSigMgrSendInfo(pSigMgr, EcrioSigMgrError,
				error, &sigMgrInfo);
		}

		sendStatusCallback = Enum_FALSE;
	}

	if (sendStatusCallback == Enum_TRUE)
	{
		if (pUserRegInfo->registrationState == EcrioSigMgrStateInitialized)
		{
			error = _EcrioSigMgrUpdateUserRegInfoOnInitState(pSigMgr, pUserRegInfo, Enum_TRUE);
		}

		_EcrioSigMgrUpdateStatus(pSigMgr, &message);
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************

Function:		_EcrioSigMgrIsDeRegResponse()

Purpose:		decides if a response is for register request or deregister
                request.

Description:	decides if a response is for register request or deregister
                request.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                EcrioSigMgrRegisterCommonHeaderStruct* pCommonHdrs - Common hdrs

OutPut:			None

Returns:		error code.
*****************************************************************************/
BoolEnum _EcrioSigMgrIsDeRegResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pRegResp
)
{
	u_int32 expires = 0, j = 0, k = 0;
	BoolEnum isDeReg = Enum_FALSE;
	EcrioSigMgrNetworkInfoStruct *pNetworkInfo = NULL;

	pSigMgr = pSigMgr;	/* Removing level-4 warning */

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
// quiet		__FUNCTION__, __LINE__);

	pNetworkInfo = pSigMgr->pNetworkInfo;

	if (pRegResp->pExpires != NULL)
	{
		expires = *pRegResp->pExpires;
		if (expires == 0)
		{
			isDeReg = Enum_TRUE;
		}
	}
	else if (pRegResp->pContact != NULL)
	{
		for (j = 0; j < pRegResp->pContact->numContactUris; ++j)
		{
			EcrioSigMgrNameAddrWithParamsStruct *pContactUri = NULL;
			pContactUri = pRegResp->pContact->ppContactDetails[j];

			if (pContactUri != NULL)
			{
				if (pContactUri->nameAddr.addrSpec.uriScheme == EcrioSigMgrURISchemeSIP)
				{
					u_char *pIPAddr = NULL;
					u_int32 ipLen = 0;

					if (pContactUri->nameAddr.addrSpec.u.pSipUri)
					{
						if (pContactUri->nameAddr.addrSpec.u.pSipUri->pIPAddr &&
							pContactUri->nameAddr.addrSpec.u.pSipUri->pIPAddr->pIPAddr)
						{
							pIPAddr = pContactUri->nameAddr.addrSpec.u.pSipUri->pIPAddr->pIPAddr;
						}
						else if (pContactUri->nameAddr.addrSpec.u.pSipUri->pDomain)
						{
							pIPAddr = pContactUri->nameAddr.addrSpec.u.pSipUri->pDomain;
						}

						if (pIPAddr)
						{
							if (pIPAddr[0] == '[')
							{
								pIPAddr++;

								while (pIPAddr[ipLen] != ']' && pIPAddr[ipLen] != 0)
								{
									ipLen++;
								}
							}
							else
							{
								ipLen = pal_StringLength(pIPAddr);
							}
						}

						if (pNetworkInfo != NULL &&
							pNetworkInfo->pLocalIp &&
							pIPAddr &&
							pal_StringNCompare(pIPAddr, pNetworkInfo->pLocalIp, ipLen) != 0)
						{
							continue;
						}

						for (k = 0; k < pContactUri->nameAddr.addrSpec.u.pSipUri->numURIParams; ++k)
						{
							if ((pal_StringICompare(pContactUri->nameAddr.addrSpec.u.pSipUri->ppURIParams[k]->pParamName,
								(u_char *)ECRIO_SIG_MGR_EXPIRES_PARAM) == 0) &&
								(pContactUri->nameAddr.addrSpec.u.pSipUri->ppURIParams[k]->pParamValue != NULL))
							{
								expires = pal_StringToNum(pContactUri->nameAddr.addrSpec.u.pSipUri->ppURIParams[k]->pParamValue, NULL);
								if (expires == 0)
								{
									isDeReg = Enum_TRUE;
									break;
								}
							}
						}
					}
				}

				if (isDeReg != Enum_TRUE)
				{
					for (k = 0; k < pContactUri->numParams; ++k)
					{
						if ((pal_StringICompare(pContactUri->ppParams[k]->pParamName,
							(u_char *)ECRIO_SIG_MGR_EXPIRES_PARAM) == 0) &&
							(pContactUri->ppParams[k]->pParamValue != NULL))
						{
							expires = pal_StringToNum(pContactUri->ppParams[k]->
								pParamValue, NULL);
							if (expires == 0)
							{
								isDeReg = Enum_TRUE;
								break;
							}
						}
					}
				}
			}

			if (isDeReg == Enum_TRUE)
			{
				break;
			}
		}
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tisDeReg:%d",
		__FUNCTION__, __LINE__, isDeReg);

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
// quiet		__FUNCTION__, __LINE__);

	return isDeReg;
}

/*****************************************************************************

Function:		_EcrioSigMgrStartReRegTimer()

Purpose:		starts a reregistration timer for list of contact uris.

Description:	starts a reregistration timer for list of contact uris.

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
                EcrioSigMgrRegisterCommonHeaderStruct* pCommonHdrs - common
                headers.
                u_char* pCallId - callid.
                EcrioSigMgrUserRegisterInfoStruct* pUserRegInfo - user
                registration info object.

OutPut:			none.

Returns:		error code.
*****************************************************************************/
/*u_int32 _EcrioSigMgrStartReRegTimer(EcrioSigMgrStruct* pSigMgr,
        EcrioSigMgrRegisterCommonHeaderStruct* pCommonHdrs,
        u_char* pCallId, EcrioSigMgrUserRegisterInfoStruct* pUserRegInfo)*/
u_int32 _EcrioSigMgrStartReRegTimer
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pRegRsp,
	EcrioSigMgrCallIDStruct *pCallIdStruct,
	EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo,
	BoolEnum bPositiveResponse
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	u_int32 expires	= 0;
	u_int16 j = 0;
	BoolEnum bExpireFound = Enum_FALSE, bIsHdrParam = Enum_FALSE, bIsZeroValue = Enum_FALSE;
	TimerStartConfigStruct timerConfig;

	EcrioSigMgrSipMessageStruct *pRegCommonHdrs = NULL;
	// EcrioSigMgrSipMessageStruct *pCommonHdrs = pRegRsp->pResponseHdrs->pOptionalHdrs->pRegCommonHdrs;

	EcrioSigMgrNetworkInfoStruct *pNetworkInfo = NULL;
	u_char *pIPAddr = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pNetworkInfo = pSigMgr->pNetworkInfo;

	if (pUserRegInfo->pRegisterReq != NULL)
	{
		pRegCommonHdrs = pUserRegInfo->pRegisterReq;
	}

	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\tIs positive response received = %d",
		__FUNCTION__, __LINE__, bPositiveResponse);

	if (bPositiveResponse)
	{
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u bPositiveResponse is Enum_TRUE",
			__FUNCTION__, __LINE__);

		/* Stopping registration expire callback */
		if (pUserRegInfo->bIsRegExpireTimerRunning == Enum_TRUE)
		{
			pal_TimerStop(pUserRegInfo->uRegExpireTimerID);
			pUserRegInfo->uRegExpireTimerID = NULL;

			pUserRegInfo->bIsRegExpireTimerRunning = Enum_FALSE;
		}

		/* Release any contact hdrs present in User reg info Request hdrs.*/
		if ((pRegCommonHdrs != NULL) && (pRegCommonHdrs->pContact != NULL))
		{
			SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u pRegCommonHdrs, pContact are populated",
				__FUNCTION__, __LINE__);

			EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_Contact,
				(void **)&(pRegCommonHdrs->pContact), Enum_TRUE);
			pRegCommonHdrs->pContact = NULL;
		}

		if ((pRegCommonHdrs != NULL) && (pRegCommonHdrs->pExpires != NULL))
		{
			SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u pRegCommonHdrs, pExpires are populated",
				__FUNCTION__, __LINE__);

			if (pUserRegInfo->pExpires == NULL)
			{
				pal_MemoryAllocate(sizeof(u_int32), (void **)&pUserRegInfo->pExpires);
				if (pUserRegInfo->pExpires == NULL)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tpal_MemoryAllocate() for pUserRegInfo->pExpires, error=%u",
						__FUNCTION__, __LINE__, error);
					error = ECRIO_SIGMGR_NO_MEMORY;
					goto Error_Level_01;
				}
			}

			*pUserRegInfo->pExpires = *pRegCommonHdrs->pExpires;
			pal_MemoryFree((void **)&pRegCommonHdrs->pExpires);
			SIGMGRLOGD(pSigMgr->pLogHandle, KLogType_Fine, "%s:%u pExpires is %u",
				__FUNCTION__, __LINE__, *pUserRegInfo->pExpires);
		}

		if (pRegRsp->pContact != NULL)
		{
			SIGMGRLOGD(pSigMgr->pLogHandle, KLogType_Fine,
				"%s:%u\tNumber of contact URI received = %d",
				__FUNCTION__, __LINE__, pRegRsp->pContact->numContactUris);

			for (j = 0; j < pRegRsp->pContact->numContactUris; ++j)
			{
				u_char *pStoreIPAddr = NULL;
				u_char *pStoreLocalIPAddr = NULL;
				BoolEnum bLocalPortCheck = Enum_FALSE;
				u_int32 uLocalPort = 0;

				// Retrieve the expire value only if the contact is relevant to local IP
				if (pRegRsp->pContact->ppContactDetails[j]->nameAddr.addrSpec.uriScheme == EcrioSigMgrURISchemeSIP)
				{
					if (pRegRsp->pContact->ppContactDetails[j]->nameAddr.addrSpec.u.pSipUri->pIPAddr &&
						pRegRsp->pContact->ppContactDetails[j]->nameAddr.addrSpec.u.pSipUri->pIPAddr->pIPAddr)
					{
						pIPAddr = pRegRsp->pContact->ppContactDetails[j]->nameAddr.addrSpec.u.pSipUri->pIPAddr->pIPAddr;
						uLocalPort = pRegRsp->pContact->ppContactDetails[j]->nameAddr.addrSpec.u.pSipUri->pIPAddr->port;
						bLocalPortCheck = Enum_TRUE;
					}
					else if (pRegRsp->pContact->ppContactDetails[j]->nameAddr.addrSpec.u.pSipUri->pDomain)
					{
						pIPAddr = pRegRsp->pContact->ppContactDetails[j]->nameAddr.addrSpec.u.pSipUri->pDomain;
						bLocalPortCheck = Enum_TRUE;
					}
				}
				else
				{
					SIGMGRLOGD(pSigMgr->pLogHandle, KLogType_Fine,
						"%s:%u\tURI scheme is not SIP, skipping contact URI", __FUNCTION__, __LINE__);
					continue;
				}

				SIGMGRLOGD(pSigMgr->pLogHandle, KLogType_Fine,
					"%s:%u\tContact Header's IP = %s, PDN IP = %s",
					__FUNCTION__, __LINE__, pIPAddr, pNetworkInfo->pLocalIp);

				if (bLocalPortCheck == Enum_TRUE)
				{
					SIGMGRLOGD(pSigMgr->pLogHandle, KLogType_Fine,
						"%s:%u\tRequest Contact Local Port:%d Response Contact Local Port:%d",
						__FUNCTION__, __LINE__, uLocalPort, pNetworkInfo->uLocalPort);
				}

				pStoreIPAddr = pal_StringCreate(pIPAddr, pal_StringLength(pIPAddr));
				pStoreLocalIPAddr = pal_StringCreate(pNetworkInfo->pLocalIp, pal_StringLength(pNetworkInfo->pLocalIp));


				if (pStoreIPAddr != NULL && pStoreLocalIPAddr != NULL)
				{
					u_int32 ipLen = 0;
					BoolEnum bStoreIPAddrIncremented = Enum_FALSE;
					BoolEnum bStoreLocalIPAddrIncremented = Enum_FALSE;
					BoolEnum bMatched = Enum_TRUE;

					if (pStoreIPAddr[0] == '[')
					{
						pStoreIPAddr++;
						bStoreIPAddrIncremented = Enum_TRUE;

						while (pStoreIPAddr[ipLen] != ']' && pStoreIPAddr[ipLen] != 0)
						{
							ipLen++;
						}
					}
					else
					{
						ipLen = pal_StringLength(pStoreIPAddr);
					}

					if (pStoreLocalIPAddr[0] == '[')
					{
						pStoreLocalIPAddr++;
						bStoreLocalIPAddrIncremented = Enum_TRUE;
					}

					if ((pRegRsp->pContact->ppContactDetails[j]->nameAddr.addrSpec.uriScheme != EcrioSigMgrURISchemeSIP) ||
						(pIPAddr == NULL) || (pal_StringNCompare(pStoreIPAddr, pStoreLocalIPAddr, ipLen) != 0) || 
						(bLocalPortCheck == Enum_TRUE && uLocalPort != pNetworkInfo->uLocalPort))
					{
						bMatched = Enum_FALSE;
					}

					if (bStoreIPAddrIncremented == Enum_TRUE)
					{
						pStoreIPAddr--;
					}

					if (bStoreLocalIPAddrIncremented == Enum_TRUE)
					{
						pStoreLocalIPAddr--;
					}

					pal_MemoryFree((void **)&pStoreIPAddr);
					pal_MemoryFree((void **)&pStoreLocalIPAddr);

					if (bMatched == Enum_FALSE)
					{
						SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tIP address did not match",
							__FUNCTION__, __LINE__);
						continue;
					}
				}
				else
				{
					SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u IP is NULL",
						__FUNCTION__, __LINE__);
					continue;
				}

				error = _EcrioSigMgrFindExpiryInNameAddrStuct(pSigMgr,
					pRegRsp->pContact->ppContactDetails[j],
					&bIsHdrParam,
					&bIsZeroValue,
					&bExpireFound,
					&expires);
				if (bIsZeroValue)
				{
					SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tbIsZeroValue is FALSE",
						__FUNCTION__, __LINE__);
					continue;
				}
				break;
			}
		}

		if (!bExpireFound)
		{
			SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tExpires value in contact header missing, look for expires header",
				__FUNCTION__, __LINE__);

			if (pRegRsp->pExpires != NULL)
			{
				expires = *pRegRsp->pExpires;

				SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tValue for Expires header = %d",
					__FUNCTION__, __LINE__, expires);
			}
		}

		if (expires != 0)
		{
			timerConfig.uPeriodicInterval = 0;
			timerConfig.timerCallback = _EcrioSigMgrReRegistrationCallBack;
			timerConfig.pCallbackData = (void *)pCallIdStruct;
			timerConfig.bEnableGlobalMutex = Enum_TRUE;

			if (expires > 1200)
			{
				timerConfig.uInitialInterval = expires - 600;
			}
			else
			{
				timerConfig.uInitialInterval = expires / 2;

				// The below lower expiry limit comes from OOM. At this moment we are hard coding to 30 seconds.
				if (timerConfig.uInitialInterval < 30)
				{
					// setting the lower limit to prevent the excess network traffic.
					timerConfig.uInitialInterval = 30;
				}
			}

			/* expires calculated so far is in seconds, Shall update to milliseconds*/
			timerConfig.uInitialInterval *= 1000;

			SIGMGRLOGD(pSigMgr->pLogHandle, KLogType_Fine,
				"%s:%u\tStarting re-registration timer, interval:%u",
				__FUNCTION__, __LINE__, timerConfig.uInitialInterval);

			error = pal_TimerStart(pSigMgr->pal, &timerConfig, &pUserRegInfo->reregistrationTimerId);
			if (error != KPALErrorNone)
			{
				// @todo Not sure about freeing this here, really?
				if (pCallIdStruct != NULL)
				{
					pal_MemoryFree(
						(void **)&pCallIdStruct);
				}

				/* It is reusing input param pCommonHdrs */
				error = ECRIO_SIGMGR_TIMER_ERROR;
				goto Error_Level_01;
			}

			/* start reg expire timer */
			timerConfig.timerCallback = _EcrioSigMgrRegExpireCallBack;
			timerConfig.uInitialInterval = expires * 1000;

			SIGMGRLOGD(pSigMgr->pLogHandle, KLogType_Fine,
				"%s:%u\tStarting registration expire timer, interval:%u",
				__FUNCTION__, __LINE__, timerConfig.uInitialInterval);

			error = pal_TimerStart(pSigMgr->pal, &timerConfig, &pUserRegInfo->uRegExpireTimerID);
			if (error != KPALErrorNone)
			{
				// @todo Not sure about freeing this here, really?
				if (pCallIdStruct != NULL)
				{
					pal_MemoryFree(
						(void **)&pCallIdStruct);
				}

				error = ECRIO_SIGMGR_TIMER_ERROR;
				goto Error_Level_01;
			}
			else
			{
				pUserRegInfo->bIsRegExpireTimerRunning = Enum_TRUE;
			}
		}
	}

Error_Level_01:
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		if (pUserRegInfo->reregistrationTimerId != NULL)
		{
			pal_TimerStop(pUserRegInfo->reregistrationTimerId);
			pUserRegInfo->reregistrationTimerId = NULL;
		}

		if (pUserRegInfo->uRegExpireTimerID != NULL)
		{
			pal_TimerStop(pUserRegInfo->uRegExpireTimerID);
			pUserRegInfo->uRegExpireTimerID = NULL;
		}
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return error;
}

/*****************************************************************************
Function:		_EcrioSigMgrPopulateContactUris()

Purpose:		populates contact uris to user registration object's request
                hdrs.

Description:	populates contact uris to user registration object's request
                hdrs.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                EcrioSigMgrRegisterCommonHeaderStruct* pSrcCommonHdrs - Common
                Hdrs in the request from upper layer.
                EcrioSigMgrUserRegisterInfoStruct* pUserRegInfo - User Reginfo
                object.

Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrPopulateContactUris
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pRegReq,
	EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	// @todo Refactor to remove pRegReq if it is not needed.
	(void)pRegReq;

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
// quiet		__FUNCTION__, __LINE__);

	if (pUserRegInfo->pRegisterReq->pContact == NULL)
	{
		error = _EcrioSigMgrAddLocalContactUri(pSigMgr, &pUserRegInfo->pRegisterReq->pContact, pUserRegInfo->pRegisterReq->bPrivacy);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}

		if (pSigMgr->pOOMObject->ec_oom_HasUserPhoneSupport(EcrioSipMessageTypeRegister, EcrioSipHeaderTypeContact, EcrioSipURIType_SIP) == Enum_TRUE)
		{
			/* All outgoing SIP uri which contain telephone number in user info part must have "user=phone" parameter as SIP uri parameter.	*/
			error = _EcrioSigMgrAddUserPhoneParam(pSigMgr, &pUserRegInfo->pRegisterReq->pContact->ppContactDetails[0]->nameAddr.addrSpec);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddUserPhoneParam, error=%u", __FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}
	}

Error_Level_01:

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
// quiet		__FUNCTION__, __LINE__);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrUpdateExpirationTime()

Purpose:		Updates expiration interval on the basis of request type.

Description:	Updates expiration interval on the basis of request type.

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
                EcrioSigMgrUserRegisterInfoStruct* pUserRegInfo - User registration info
                EcrioSigMgrReqisterRequestEnum regReqType - request type.

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrUpdateExpirationTime
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo,
	EcrioSigMgrRegisterRequestEnum regReqType
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 expires = 0;

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
// quiet		__FUNCTION__, __LINE__);

	expires = ((regReqType == EcrioSigMgrRegisterRequestType_Reg) ?
		((pUserRegInfo->pRegisterReq->pExpires != NULL) ? *pUserRegInfo->pRegisterReq->pExpires :
		pSigMgr->pSignalingInfo->registrationExpires) : 0);

	if (pUserRegInfo->pRegisterReq->pExpires == NULL)
	{
		pal_MemoryAllocate(sizeof(u_int32), (void **)&pUserRegInfo->pRegisterReq->pExpires);
		if (pUserRegInfo->pRegisterReq->pExpires == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pExpires, error=%u",
				__FUNCTION__, __LINE__, error);
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		*pUserRegInfo->pRegisterReq->pExpires = expires;
	}

Error_Level_01:

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
// quiet		__FUNCTION__, __LINE__);

	return error;
}

/*****************************************************************************

Function:		_EcrioSigMgrReRegistrationCallBack()

Purpose:		Reregistration timer callback function

Description:	Sends reregistration request, if a registration is already
                is in progress then starts a new timer with registration timer
                delta time interval.

Input:			u_int32 timerID - Timer ID.
                void* pData - Timer Callback Data

Returns:		error code.
*****************************************************************************/
void _EcrioSigMgrReRegistrationCallBack
(
	void *pData,
	TIMERHANDLE timerID
)
{
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrUserRegisterInfoStruct *pUserInfo = NULL;
	//EcrioSigMgrInfoStruct sigMgrInfo;
	TimerStartConfigStruct timerConfig;
	BoolEnum bSendReReg = Enum_TRUE;
	EcrioSigMgrCallIDStruct *pCallIdStruct = NULL;

	/* Stop the timer before proceeding further. */
	pal_TimerStop(timerID);

	if (pData == NULL)
	{
		return;
	}

	pCallIdStruct = (EcrioSigMgrCallIDStruct *)pData;

	pSigMgr = pCallIdStruct->pSigMgr;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\ttimerID=%u, pData=%p",
		__FUNCTION__, __LINE__, timerID, pCallIdStruct->pCallID);

	if (pCallIdStruct->pCallID == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tcallid is NULL",
			__FUNCTION__, __LINE__);

		goto Error_Level_01;
	}
	else
	{
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tcallid=%s",
			__FUNCTION__, __LINE__, pCallIdStruct->pCallID);
	}

	_EcrioSigMgrGetUserRegInfo(pSigMgr, pCallIdStruct->pCallID, &pUserInfo);

	if ((pUserInfo->registrationState != EcrioSigMgrStateRegistered) &&
		(pUserInfo->registrationState != EcrioSigMgrStateInitialized))
	{
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\ttimer expires in EcrioSigMgrStateRegistered state",
			__FUNCTION__, __LINE__);

		if (pUserInfo->reregistrationTimerId == timerID)
		{
			timerConfig.uInitialInterval = pSigMgr->pSignalingInfo->registerationTimerDelta;
			timerConfig.uPeriodicInterval = 0;
			timerConfig.timerCallback = _EcrioSigMgrReRegistrationCallBack;
			timerConfig.pCallbackData = (void *)pCallIdStruct;
			timerConfig.bEnableGlobalMutex = Enum_TRUE;

			pal_TimerStart(pSigMgr->pal, &timerConfig, &pUserInfo->reregistrationTimerId);
		}

		bSendReReg = Enum_FALSE;
	}
	else
	{
		// pRegisterReq = pUserInfo->pRegisterReq;

		if (pUserInfo->reregistrationTimerId == timerID)
		{
			pUserInfo->reregistrationTimerId = NULL;
		}

		pUserInfo->reqType = EcrioSigMgrRegisterRequestType_Reg;

		bSendReReg = Enum_TRUE;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tnotifying SUE on re-reg timer expiry",
		__FUNCTION__, __LINE__);

	_EcrioSigMgrSendInfo(pSigMgr, EcrioSigMgrInformation,
		ECRIO_SIGMGR_RE_REG_TIMER_EXPIRES, (void *)&bSendReReg);

	/*SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tre-registering on re-reg timer expiry",
	    __FUNCTION__, __LINE__);
	error = _EcrioSigMgrSendRegister(pSigMgr, EcrioSigMgrRegisterRequestType_Reg, (void*)&pUserInfo->regId);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
	    SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t_EcrioSigMgrSendRegister) failed with error = %u",
	        __FUNCTION__, __LINE__, error);

	    goto Error_Level_01;
	}*/

Error_Level_01:
#if 0
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		sigMgrInfo.eventType = EcrioSigMgrRegisterRequest;

		if (pUserInfo)
		{
			sigMgrInfo.pEventIdentifier = (void *)&pUserInfo->regId;
		}

		_EcrioSigMgrSendInfo(pSigMgr, EcrioSigMgrError, error, &sigMgrInfo);
	}
#endif
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}

/*****************************************************************************

Function:		_EcrioSigMgrRegExpireCallBack()

Purpose:		Reregistration expire timer callback function

Description:	Sends registration expire info

Input:			u_int32 timerID - Timer ID.
                void* pData - Timer Callback Data

Returns:		error code.
*****************************************************************************/
void _EcrioSigMgrRegExpireCallBack
(
	void *pData,
	TIMERHANDLE timerID
)
{
	EcrioSigMgrCallIDStruct	*pCallIdStruct = NULL;
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo = NULL;

	/* Stop the timer before proceeding further. */
	pal_TimerStop(timerID);

	if (pData == NULL)
	{
		return;
	}

	pCallIdStruct = (EcrioSigMgrCallIDStruct *)pData;

	pSigMgr = pCallIdStruct->pSigMgr;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\ttimerID=%u, pData=%p",
		__FUNCTION__, __LINE__, timerID, pCallIdStruct->pCallID);

	if (pCallIdStruct->pCallID == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tcallid is NULL",
			__FUNCTION__, __LINE__);

		goto Error_Level_01;
	}
	else
	{
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tcallid=%s",
			__FUNCTION__, __LINE__, pCallIdStruct->pCallID);

		_EcrioSigMgrGetUserRegInfo(pSigMgr, pCallIdStruct->pCallID,
			&pUserRegInfo);
		if (pUserRegInfo != NULL)
		{
			pUserRegInfo->bIsRegExpireTimerRunning = Enum_FALSE;
			pUserRegInfo->uRegExpireTimerID = NULL;
		}
	}

	/* send error message to upper layer*/
	_EcrioSigMgrSendInfo(pSigMgr, EcrioSigMgrError,
		ECRIO_SIGMGR_REGISTRATION_TIME_EXPIRED, NULL);

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}

/*****************************************************************************

Function:		_EcrioSigMgrHandleRegTimeOut()

Purpose:		handles time out notification for TXN Managers.

Description:	sends update message callback to the upper layer and deletes
                the user reg info object to the upper layer.

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
                u_char* pCallId - callid

Returns:		error code.
*****************************************************************************/
void _EcrioSigMgrHandleRegTimeOut
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pCallId
)
{
	/* Start - Support for FQDN on 305 and 408 */
#ifdef _HANDLE_WITHOUT_FQDN
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
#endif	// _HANDLE_WITHOUT_FQDN
	/* End - Support for FQDN on 305 and 408 */
	EcrioSigMgrMessageStruct message = { .msgCmd = EcrioSigMgrMessageNone };
	EcrioSigMgrSipMessageStruct regResponse = { .eMethodType = EcrioSipMessageTypeNone };
	EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
		"%s:%u\tpCallId=%s", __FUNCTION__, __LINE__, pCallId);

	_EcrioSigMgrGetUserRegInfo(pSigMgr, (void *)pCallId, &pUserRegInfo);
	if (pUserRegInfo == NULL)
	{
		goto Error_Level_01;
	}

	pUserRegInfo->authRetryCount = 0;

	/* add reason phrase*/
	regResponse.pReasonPhrase = (u_char *)ECRIO_SIGMGR_RESPONSE_REQUEST_TIMEOUT;
	regResponse.responseCode = ECRIO_SIGMGR_RESPONSE_CODE_REQUEST_TIMEOUT;

	/* TODO: provide list of contact uris for which registration timeouts*/
	// message.msgCmd = EcrioSigMgrRegisterReponse;
	if (pUserRegInfo->reqType == EcrioSigMgrDeRegisterRequestType_DeReg)
	{
		message.msgCmd = EcrioSigMgrDeRegisterReponse;
	}
	else
	{
		message.msgCmd = EcrioSigMgrRegisterReponse;
	}

	message.pData = (void *)&regResponse;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeState, "%s:%u\t - Changing Signaling Manager Registration State from %s to %s", __FUNCTION__, __LINE__, _EcrioSigMgrGetRegistrationStateString(pUserRegInfo->registrationState), _EcrioSigMgrGetRegistrationStateString(EcrioSigMgrStateInitialized));
	pUserRegInfo->registrationState = EcrioSigMgrStateInitialized;
	_EcrioSigMgrUpdateStatus(pSigMgr, &message);

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}

/*****************************************************************************

Function:		_EcrioSigMgrInitRegObject()

Purpose:		Used for creating registartion info list and user registration
                object.

Description:	Used for creating registartion info list and user registration
                object.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.

OutPut:			void** ppRegisterIdentifier - Registration Identifier.

Returns:		error code.
******************************************************************************/
u_int32 _EcrioSigMgrInitRegObject
(
	EcrioSigMgrStruct *pSigMgr,
	void **ppRegisterIdentifier
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 *pRegId = NULL;
	EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	/* Create registration info list.*/
	if (pSigMgr->pRegisterInfo == NULL)
	{
		EcrioSigMgrRegisterInfoStruct *pRegInfoStruct = NULL;

		pal_MemoryAllocate(sizeof(EcrioSigMgrRegisterInfoStruct), (void **)&(pRegInfoStruct));
		if (pRegInfoStruct == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pRegInfoStruct, error=%u",
				__FUNCTION__, __LINE__, error);
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pal_MemorySet((void *)pRegInfoStruct, 0, sizeof(EcrioSigMgrRegisterInfoStruct));

		error = EcrioDSListCreate(
			pSigMgr->pLogHandle,
			_EcrioSigMgrCompareRegInfoFunc,
			_EcrioSigMgrDestroyRegInfoFunc, pSigMgr,
			(void **)&pRegInfoStruct->registerInformationList);

		if (error != ECRIO_DS_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioDSListCreate() for registerInformationList, error=%u",
				__FUNCTION__, __LINE__, error);
			pal_MemoryFree((void **)&pRegInfoStruct);
			// error  = ECRIO_SIG_MGR_REGISTRATION_FAILED;
			error = ECRIO_SIGMGR_CRITCAL_ERROR_NEED_TO_EXIT;
			goto Error_Level_01;
		}

		/* Assign once the structure is completely initialized. */
		pSigMgr->pRegisterInfo = pRegInfoStruct;
	}

	error = _EcrioSigMgrCreateUserRegInfo(pSigMgr, &pUserRegInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrCreateUserRegInfo() error=%u",
			__FUNCTION__, __LINE__, error);
		// error  = ECRIO_SIG_MGR_REGISTRATION_FAILED;

		error = ECRIO_SIGMGR_CRITCAL_ERROR_NEED_TO_EXIT;
		goto Error_Level_01;
	}

	pal_MemoryAllocate(sizeof(u_int32), (void **)&pRegId);
	if (pRegId == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for pRegId, error=%u",
			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	*pRegId = pUserRegInfo->regId;

	*ppRegisterIdentifier = (void *)(pRegId);
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeState, "%s:%u\t - Changing Signaling Manager Registration State from %s to %s", __FUNCTION__, __LINE__, _EcrioSigMgrGetRegistrationStateString(pUserRegInfo->registrationState), _EcrioSigMgrGetRegistrationStateString(EcrioSigMgrStateInitialized));
	pUserRegInfo->registrationState = EcrioSigMgrStateInitialized;
	error = ECRIO_SIGMGR_NO_ERROR;

Error_Level_01:
	if ((error != ECRIO_SIGMGR_NO_ERROR) && (pUserRegInfo != NULL))
	{
		_EcrioSigMgrRemoveUserRegInfo(pSigMgr, pRegId);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return error;
}

/*****************************************************************************

Function:		_EcrioSigMgrGetUserRegInfoRegId()

Purpose:		Used for getting registartion info object using reg id.

Description:	Used for getting registartion info object using reg id.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.

OutPut:			void** ppRegisterIdentifier - Registration Identifier.

Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrGetUserRegInfoRegId
(
	EcrioSigMgrStruct *pSigMgr,
	void *pRegistrationId,
	EcrioSigMgrUserRegisterInfoStruct **ppUserRegInfo
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 i = 0, size = 0;
	EcrioSigMgrRegisterInfoStruct *pRegInfoStruct = NULL;
	EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
		"%s:%u\tpSigMgr=%p, pRegistrationId=%p",
		__FUNCTION__, __LINE__, pSigMgr, pRegistrationId);

	if ((pSigMgr->pRegisterInfo == NULL) || (ppUserRegInfo == NULL))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInvalid data passed, pSigMgr->pRegisterInfo=%p, ppUserRegInfo=%p",
			__FUNCTION__, __LINE__, pSigMgr->pRegisterInfo, ppUserRegInfo);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	pRegInfoStruct = (EcrioSigMgrRegisterInfoStruct *)pSigMgr->pRegisterInfo;
	*ppUserRegInfo = NULL;

	EcrioDSListGetSize(pRegInfoStruct->registerInformationList, &size);

	for (i = 1; i <= size; ++i)
	{
		EcrioDSListGetDataAt(pRegInfoStruct->registerInformationList, i,
			(void **)&pUserRegInfo);

		if ((pUserRegInfo != NULL) &&
			(pUserRegInfo->regId == *((u_int32 *)pRegistrationId)))
		{
			SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tUser reg object found, pUserRegInfo=%p, regid=%u",
				__FUNCTION__, __LINE__, pUserRegInfo, *((u_int32 *)pRegistrationId));
			*ppUserRegInfo = pUserRegInfo;
			break;
		}
	}

	if (*ppUserRegInfo == NULL)
	{
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tUser reg object  not found callid=%u",
			__FUNCTION__, __LINE__, (u_int32 *)pRegistrationId);
		error = ECRIO_SIGMGR_ID_NOT_FOUND;
	}

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);
	return error;
}

/*****************************************************************************

Function:		_EcrioSigMgrUpdateUserRegInfoOnInitState()

Purpose:		Updated user registration information.

Description:	Updated user registration information.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                EcrioSigMgrUserRegisterInfoStruct* pUserRegInfo - User
                registration object.
                BoolEnum bDelateRegInfo - true will delete reg info

Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrUpdateUserRegInfoOnInitState
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo,
	BoolEnum bDelateRegInfo
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pUserRegInfo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	if (bDelateRegInfo == Enum_TRUE)
	{
		if (pUserRegInfo->pRegisterReq != NULL)
		{
			EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_SipMessage,
				(void **)&(pUserRegInfo->pRegisterReq), Enum_TRUE);
		}
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeState, "%s:%u\t - Changing Signaling Manager Registration State from %s to %s", __FUNCTION__, __LINE__, _EcrioSigMgrGetRegistrationStateString(pUserRegInfo->registrationState), _EcrioSigMgrGetRegistrationStateString(EcrioSigMgrStateInitialized));
	pUserRegInfo->registrationState = EcrioSigMgrStateInitialized;

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************
  Function:		_EcrioSigMgrGetRegistrationStateString

  Purpose:		Get EcrioSigMgrRegistrationStateEnum string

  Description:	Get the string for the given state.
                Used in logging.

  Input:		EcrioSigMgrRegistrationStateEnum	registrationState -

  Output:		none

  Returns:		u_char* - Signaling manager registration state

**************************************************************************/
u_char *_EcrioSigMgrGetRegistrationStateString
(
	EcrioSigMgrRegistrationStateEnum registrationState
)
{
	u_char *str = NULL;

	switch (registrationState)
	{
		case EcrioSigMgrStateUnInitialized:
		{
			str = (u_char *)"EcrioSigMgrStateUnInitialized";
		}
		break;
		case EcrioSigMgrStateInitialized:
		{
			str = (u_char *)"EcrioSigMgrStateInitialized";
		}
		break;
		case EcrioSigMgrStateRegistrationInProgress:
		{
			str = (u_char *)"EcrioSigMgrStateRegistrationInProgress";
		}
		break;
		case EcrioSigMgrStateProxyAuthenticationRequired:
		{
			str = (u_char *)"EcrioSigMgrStateProxyAuthenticationRequired";
		}
		break;
		case EcrioSigMgrStateAuthenticationRequired:
		{
			str = (u_char *)"EcrioSigMgrStateAuthenticationRequired";
		}
		break;
		case EcrioSigMgrStateRegistered:
		{
			str = (u_char *)"EcrioSigMgrStateRegistered";
		}
		break;
		case EcrioSigMgrStateExpiresIntervalTooBrief:
		{
			str = (u_char *)"EcrioSigMgrStateExpiresIntervalTooBrief";
		}
		break;
		case EcrioSigMgrStateDeRegistrationInProgress:
		{
			str = (u_char *)"EcrioSigMgrStateDeRegistrationInProgress";
		}
		break;
		case EcrioSigMgrRegisterRedirectionInProgress:
		{
			str = (u_char *)"EcrioSigMgrRegisterRedirectionInProgress";
		}
		break;
		default:
		{
			str = (u_char *)"Unspecified registration state";
		}
		break;
	}

	return str;
}

/*****************************************************************************
                    API Definition Section - End
*****************************************************************************/
