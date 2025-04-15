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
#include "EcrioSigMgrSipMessage.h"
#include "EcrioSigMgrSubscribe.h"
#include "EcrioSigMgrSubscribeInternal.h"
#include "EcrioSigMgrReferInternal.h"
#include "EcrioSigMgrOptionsInternal.h"
/**************************************************************************
Function:		_EcrioSigMgrPopulateCommonInfo()


Purpose:		Populates the Common info from the request / response.

Description:	Populates the Common info from the request / response.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
Input:			EcrioIMSLibrarySignalingMandatoryStruct* pManHdrs  - IMS Mandatory struct.
Input:			EcrioSigMgrMandatoryHeaderStruct* pSrcManHdrs, -  Signaling Manager Mandatory struct.
Output:			EcrioSigMgrCommonInfoStruct* pCommonInfo - CommonInfoStruct.

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrPopulateCommonInfo
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrMandatoryHeaderStruct *pSrcManHdrs,
	EcrioSigMgrCommonInfoStruct *pCommonInfo
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry,
// quiet		"%s:%u", __FUNCTION__, __LINE__);

	if (pSrcManHdrs != NULL)
	{
		if ((pSrcManHdrs->pCallId != NULL) && (pCommonInfo->pCallId == NULL))
		{
			error = _EcrioSigMgrStringCreate(pSigMgr, pSrcManHdrs->pCallId,
				&pCommonInfo->pCallId);

			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrStringCreate() for pCallId, error=%u",
					__FUNCTION__, __LINE__, error);

				goto Error_Level_01;
			}
		}
		
		if ((pSrcManHdrs->pTo == NULL) || (pSrcManHdrs->pFrom == NULL))
		{
			error = ECRIO_SIGMGR_INVALID_REQUEST;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tIncoming Request/Response received is invalid. To or From header is missing/malformed, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		if ((pCommonInfo->pToTag == NULL) && (pSrcManHdrs->pTo->ppParams != NULL))
		{
			error = _EcrioSigMgrGetParamValByName(pSigMgr, pSrcManHdrs->pTo->ppParams,
				pSrcManHdrs->pTo->numParams, (u_char *)ECRIO_SIG_MGR_SIP_TAG_STRING,
				&pCommonInfo->pToTag);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrGetParamValByName() for pToTag, error=%u",
					__FUNCTION__, __LINE__, error);

				goto Error_Level_01;
			}
		}

		if ((pCommonInfo->pFromTag == NULL) && (pSrcManHdrs->pFrom->ppParams != NULL))
		{
			error = _EcrioSigMgrGetParamValByName(pSigMgr, pSrcManHdrs->pFrom->ppParams,
				pSrcManHdrs->pFrom->numParams, (u_char *)ECRIO_SIG_MGR_SIP_TAG_STRING,
				&pCommonInfo->pFromTag);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrGetParamValByName() for pFromTag, error=%u",
					__FUNCTION__, __LINE__, error);

				goto Error_Level_01;
			}
		}

		pCommonInfo->cSeq = pSrcManHdrs->CSeq;
	}

	if (pSrcManHdrs != NULL)
	{
		if ((pSrcManHdrs->ppVia != NULL) &&
			(pSrcManHdrs->ppVia[0]->pBranch != NULL) &&
			(pCommonInfo->pBranch == NULL))
		{
			error = _EcrioSigMgrStringCreate(pSigMgr,
				pSrcManHdrs->ppVia[0]->pBranch, &pCommonInfo->pBranch);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrStringCreate() for pBranch, error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}
	}

Error_Level_01:

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit,
// quiet		"%s:%u\terror=%u", __FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrParseUri

Purpose:		To parse a NULL terminated URI string into SigMgr URI struct.

Description:	API will make use of IMS lib utility API to parse a NULL
                terminated URI string into SigMgr URI struct.

Input:			EcrioSigMgrStruct* pSigMgr - signaling manager instance
                u_char* ppUri - NULL terminated Uri String

Output:			EcrioSigMgrUriStruct** ppUriStruct - Uri struct

Returns:		Error Code

**************************************************************************/
u_int32 _EcrioSigMgrParseUri
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pUri,
	EcrioSigMgrUriStruct *pUriStruct
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 len = pal_StringLength(pUri) - 1;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry,
		"%s:%u", __FUNCTION__, __LINE__);

	error = _EcrioSigMgrParseAddrSpec(pSigMgr, pUri, pUri + len, pUriStruct);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrParseAddrSpec() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit,
		"%s:%u\terror=%u", __FUNCTION__, __LINE__, error);

	return error;
}
/*****************************************************************************

Function:		_EcrioSigMgrUpdateMessageRequestStruct()

Purpose:		Updates header values in Message info object.

Description:	Creates Mandatory header, Optional header objects and store
/ updates header values from upper layer.

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
				EcrioSigMgrSipMessageStruct* pMessageReq - Message request
				BoolEnum bServiceRoute - service route present or not
Headers.

Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrUpdateMessageRequestStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pMessageReq,
	BoolEnum bServiceRoute
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	/* Create optional header object */
	if (pMessageReq->pOptionalHeaderList == NULL)
	{
		error = EcrioSigMgrCreateOptionalHeaderList(pSigMgr, &pMessageReq->pOptionalHeaderList);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSigMgrCreateOptionalHeaderList() error=%u",
				__FUNCTION__, __LINE__, error);

			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}
	}

	/* User-Agent support */
	if (pSigMgr->pOOMObject != NULL)
	{
		if (pMessageReq->eReqRspType == EcrioSigMgrSIPRequest)
		{

			if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(pMessageReq->eMethodType, EcrioSipHeaderTypeUserAgent))
			{
				error = _EcrioSigMgrAddUserAgentHeader(pSigMgr, pMessageReq->pOptionalHeaderList);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrAddUserAgentHeader() error=%u",
						__FUNCTION__, __LINE__, error);

					goto Error_Level_01;
				}
			}
		}
		else if (pMessageReq->eReqRspType == EcrioSigMgrSIPResponse)
		{
			/* Only for INVITE */
			if (pMessageReq->eMethodType == EcrioSipMessageTypeInvite)
			{
				switch (pMessageReq->responseCode / 100)
				{
					/* only handle 2xx response - ignore all */
					case ECRIO_SIGMGR_2XX_RESPONSE:
					{
						if (pSigMgr->pOOMObject->ec_oom_HasHeaderSupport(EcrioSipMessageTypeInvite_2xx, EcrioSipHeaderTypeUserAgent))
						{
							error = _EcrioSigMgrAddUserAgentHeader(pSigMgr, pMessageReq->pOptionalHeaderList);
							if (error != ECRIO_SIGMGR_NO_ERROR)
							{
								SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
									"%s:%u\t_EcrioSigMgrAddUserAgentHeader() error=%u",
									__FUNCTION__, __LINE__, error);
	
								goto Error_Level_01;
							}
						}
					}
					break;
					default:
						break;
				}
			}
		}
		else
		{
			; // Do nothing
		}
	}

	error = _EcrioSigMgrAddAllowHeader(pSigMgr, pMessageReq->pOptionalHeaderList);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddAllowHeader() error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	/* Add service route to Route Set */
	if (bServiceRoute)
	{
		error = _EcrioSigMgrAppendServiceRouteToRouteSet(pSigMgr,
			&pMessageReq->pRouteSet);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAppendServiceRouteToRouteSet() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}
/**************************************************************************

Function:		_EcrioSigMgrCompareUris

Purpose:		To compare two URIs

Description:	API will make use of IMS lib utility API to identify
                type of the URI and provides URI scheme of the input URI

Input:			EcrioSigMgrStruct* pSigMgr - signaling manager instance
                EcrioSigMgrUriStruct* pSrcUri - First URI
                EcrioSigMgrUriStruct* pDestUri - Second URI,

Output:			BoolEnum* result - Enum_TRUE if URI match

Returns:		Error Code

**************************************************************************/
u_int32 _EcrioSigMgrCompareUris
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUriStruct *pSrcUri,
	EcrioSigMgrUriStruct *pDestUri,
	BoolEnum *result
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	(void)pSigMgr;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry,
		"%s:%u", __FUNCTION__, __LINE__);

	if ((pSrcUri == NULL) || (pDestUri == NULL))
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;

		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data",	__FUNCTION__, __LINE__);

		goto Error_Level_01;
	}

	if (result == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data",	__FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	*result = Enum_FALSE;

	/* Check for URI scheme */
	if (pSrcUri->uriScheme != pDestUri->uriScheme)
	{
		*result = Enum_FALSE;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tURI schemes not matching", __FUNCTION__, __LINE__);

		goto Error_Level_01;
	}

	if (pSrcUri->uriScheme == EcrioSigMgrURISchemeSIP)
	{
		error = _EcrioSigMgrCompareSIPURI(pSrcUri->u.pSipUri, pDestUri->u.pSipUri, result);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrCompareSIPURI() error=%u",
				__FUNCTION__, __LINE__, error);

			*result = Enum_FALSE;
			error = ECRIO_SIGMGR_IMS_LIB_ERROR;
			goto Error_Level_01;
		}
	}
	else if (pSrcUri->uriScheme == EcrioSigMgrURISchemeTEL)
	{
		error = _EcrioSigMgrCompareTelURI(pSrcUri->u.pTelUri, pDestUri->u.pTelUri, result);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrCompareTELURI() error=%u",
				__FUNCTION__, __LINE__, error);

			*result = Enum_FALSE;
			error = ECRIO_SIGMGR_IMS_LIB_ERROR;
			goto Error_Level_01;
		}
	}
	else
	{
		*result = Enum_FALSE;
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;

		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data", __FUNCTION__, __LINE__);
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit,
		"%s:%u\terror=%u", __FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrPopulateMandatoryHeaders()

Purpose:		populates Mandatory headers for IMS lib.

Description:	populates Mandatory headers for IMS lib. using SigMgr req headers.
                a mandatory header is not present, it will be generated here and stored in
                SigMgr Mandatory headers.

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
                EcrioSigMgrMandatoryHeaderStruct* pSrcManHdrs - Mandatory headers provided by UAC / UAS
                EcrioIMSLibrarySignalingResultStruct *pResultStruct - if provided will represent mandatory headers
                are constructed for a response.
                BoolEnum generateTag - if Enum_TRUE will generate the to tag for Mandatory headers.
                u_char* branch - if present, this parameter will be used for first Via Value.

OutPut:			EcrioIMSLibrarySignalingMandatoryStruct* pDstIMSManHdrs - IMS Mandatory headers
                u_char** pMsg - generated request.
                u_int32* pMsgLen - generated request length.

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrPopulateMandatoryHeaders
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessage,
	EcrioSigMgrSIPMessageTypeEnum eMsgType,
	BoolEnum generateToTag,
	u_char *pBranch
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR, i = 0;
	BoolEnum generateFromTag = Enum_TRUE;
	EcrioSigMgrNetworkInfoStruct *pNetworkInfo = NULL;
	EcrioSigMgrMandatoryHeaderStruct *pSrcManHdrs = NULL;

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry,
// quiet		"%s:%u", __FUNCTION__, __LINE__);

	pSrcManHdrs = pSipMessage->pMandatoryHdrs;

	pNetworkInfo = pSigMgr->pNetworkInfo;
	if (pNetworkInfo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrGetPDNInfoStruct() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	if (eMsgType == EcrioSigMgrSIPRequest)
	{
		if (pSrcManHdrs->maxForwards == 0)
		{
			pSrcManHdrs->maxForwards = pSigMgr->pSignalingInfo->maxForwards;
		}
	}
	else
	{
	}

	if (pSrcManHdrs->pTo != NULL)
	{
		if (pSigMgr->pOOMObject->ec_oom_HasUserPhoneSupport(pSipMessage->eMethodType, EcrioSipHeaderTypeTo, EcrioSipURIType_SIP) == Enum_TRUE)
		{
			/* All outgoing SIP uri which contain telephone number in user info
			** part must have "user=phone" parameter as SIP uri parameter.
			** Hence following piece of code.
			*/
			error = _EcrioSigMgrAddUserPhoneParam(pSigMgr, &pSrcManHdrs->pTo->nameAddr.addrSpec);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddUserPhoneParam, error=%u", __FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}

		/* generate To tag if required. */
		for (i = 0; i < pSrcManHdrs->pTo->numParams; ++i)
		{
			if (pal_StringICompare(pSrcManHdrs->pTo->ppParams[i]->pParamName,
				(u_char*)ECRIO_SIG_MGR_SIP_TAG_STRING) == 0)
			{
				generateToTag = Enum_FALSE;
				break;
			}
		}
	}

	

	if (generateToTag == Enum_TRUE)
	{
		EcrioSigMgrParamStruct *pTagParam = NULL;

		if (pSrcManHdrs->pTo->numParams == 0)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct *) * 1, (void **)&pSrcManHdrs->pTo->ppParams);
			if (pSrcManHdrs->pTo->ppParams == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryAllocate() for pSrcManHdrs->pTo->ppParams, error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}

			pSrcManHdrs->pTo->numParams = 1;
		}
		else
		{
			/* Check arithmetic overflow - do not use the ArithmeticOverflowDetected() API for u_int16 variable. */
			if (pSrcManHdrs->pTo->numParams + 1 > USHRT_MAX)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((pSrcManHdrs->pTo->numParams + 1), sizeof(EcrioSigMgrParamStruct *)) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = pal_MemoryReallocate(
				sizeof(EcrioSigMgrParamStruct *) * (pSrcManHdrs->pTo->numParams + 1),
				(void **)&pSrcManHdrs->pTo->ppParams);
			if (error != KPALErrorNone)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryReallocate() for pSrcManHdrs->pTo->ppParams, error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}

			++(pSrcManHdrs->pTo->numParams);
		}

		i = pSrcManHdrs->pTo->numParams;
		pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&(pSrcManHdrs->pTo->ppParams[i - 1]));
		if (pSrcManHdrs->pTo->ppParams[i - 1] == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pSrcManHdrs->pTo->ppParams[i - 1], error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		pTagParam = pSrcManHdrs->pTo->ppParams[i - 1];
		pal_MemorySet(pTagParam, 0, sizeof(EcrioSigMgrParamStruct));

		error = _EcrioSigMgrStringCreate(pSigMgr, (u_char *)ECRIO_SIG_MGR_SIP_TAG_STRING,
			&pTagParam->pParamName);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrStringCreate() for pTagParam->pParamName, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		error = _EcrioSigMgrGenerateTag(&pTagParam->pParamValue);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrGenerateTag() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	/* From Header values */
	if (pSrcManHdrs->pFrom == NULL)
	{
		if (pSipMessage->bPrivacy != Enum_TRUE)
		{
#ifdef ENABLE_QCMAPI
			EcrioSigMgrURISchemesEnum eURIScheme = EcrioSigMgrURISchemeTEL;
#else
			EcrioSigMgrURISchemesEnum eURIScheme = EcrioSigMgrURISchemeSIP;
#endif
			/*Generate From header from P-Associated-URI. If there is no PAU then populate this from configured user id.*/
			error = EcrioSigMgrGetDefaultPUID(pSigMgr, eURIScheme, &pSrcManHdrs->pFrom);
			if (error != ECRIO_SIGMGR_NO_ERROR || pSrcManHdrs->pFrom == NULL)
			{
				pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct), (void **)&pSrcManHdrs->pFrom);
				if (pSrcManHdrs->pFrom == NULL)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tpal_MemoryAllocate() for pSrcManHdrs->pFrom, error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}

				if (pSigMgr->pSignalingInfo->uriScheme == eURIScheme)
				{
					EcrioSigMgrSipURIStruct *pSipUri = NULL;
					pSrcManHdrs->pFrom->nameAddr.addrSpec.uriScheme = eURIScheme;

					pal_MemoryAllocate(sizeof(EcrioSigMgrSipURIStruct), (void **)&(pSrcManHdrs->pFrom->nameAddr.addrSpec.u.pSipUri));
					if (pSrcManHdrs->pFrom->nameAddr.addrSpec.u.pSipUri == NULL)
					{
						error = ECRIO_SIGMGR_NO_MEMORY;
						SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
							"%s:%u\tpal_MemoryAllocate() for pSrcManHdrs->pFrom->nameAddr.addrSpec.u.pSipUri, error=%u",
							__FUNCTION__, __LINE__, error);
						goto Error_Level_01;
					}

					pSipUri = pSrcManHdrs->pFrom->nameAddr.addrSpec.u.pSipUri;

					error = _EcrioSigMgrStringCreate(pSigMgr, pSigMgr->pSignalingInfo->pHomeDomain,
						&pSipUri->pDomain);
					if (error != ECRIO_SIGMGR_NO_ERROR)
					{
						SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
							"%s:%u\t_EcrioSigMgrStringCreate() for pSipUri->pDomain, error=%u",
							__FUNCTION__, __LINE__, error);
						goto Error_Level_01;
					}

					error = _EcrioSigMgrStringCreate(pSigMgr, pSigMgr->pSignalingInfo->pUserId,
						&pSipUri->pUserId);
					if (error != ECRIO_SIGMGR_NO_ERROR)
					{
						SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
							"%s:%u\t_EcrioSigMgrStringCreate() for pSipUri->pUserId, error=%u",
							__FUNCTION__, __LINE__, error);
						goto Error_Level_01;
					}

					/* EcrioSipURITypeEnums is same as EcrioSigMgrURISchemesEnum, hence type casted directly */
					if (pSigMgr->pOOMObject->ec_oom_HasUserPhoneSupport(pSipMessage->eMethodType, EcrioSipHeaderTypeFrom, (EcrioSipURITypeEnums)eURIScheme) == Enum_TRUE)
					{
						/* All outgoing SIP uri which contain telephone number in user info
						** part must have "user=phone" parameter as SIP uri parameter.
						** Hence following piece of code.
						*/
						error = _EcrioSigMgrAddUserPhoneParam(pSigMgr, &pSrcManHdrs->pFrom->nameAddr.addrSpec);
						if (error != ECRIO_SIGMGR_NO_ERROR)
						{
							SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
								"%s:%u\t_EcrioSigMgrAddUserPhoneParam, error=%u", __FUNCTION__, __LINE__, error);
							goto Error_Level_01;
						}
					}
				}
				/*else if (pSigMgr->pSignalingInfo->uriScheme == EcrioSigMgrURISchemeTEL)
				{
				pSrcManHdrs->pFrom->nameAddr.addrSpec.uriScheme = EcrioSigMgrURISchemeTEL;

				error = _EcrioSigMgrPopulateTelUri(pSigMgr, pSigMgr->pSignalingInfo->pMSISDN,
				&(pSrcManHdrs->pFrom->nameAddr.addrSpec.u.pTelUri));
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrPopulateTelUri() error=%u",
				__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
				}
				}*/
				else
				{
					error = ECRIO_SIGMGR_INVALID_URI_SCHEME;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tFailed since INVALID_URI_SCHEME",
						__FUNCTION__, __LINE__);
					goto Error_Level_01;
				}
			}
		}
		else
		{
			/* Generate From header with an anonymous value. */
			pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct), (void **)&pSrcManHdrs->pFrom);
			if (pSrcManHdrs->pFrom == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryAllocate() for pSrcManHdrs->pFrom, error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}

			if (pSigMgr->pSignalingInfo->uriScheme == EcrioSigMgrURISchemeSIP)
			{
				EcrioSigMgrSipURIStruct *pSipUri = NULL;
				pSrcManHdrs->pFrom->nameAddr.addrSpec.uriScheme = EcrioSigMgrURISchemeSIP;

				error = _EcrioSigMgrStringCreate(pSigMgr, (u_char*)"Anonymous", &(pSrcManHdrs->pFrom->nameAddr.pDisplayName));
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrStringCreate() for pDisplayName, error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}

				pal_MemoryAllocate(sizeof(EcrioSigMgrSipURIStruct), (void **)&(pSrcManHdrs->pFrom->nameAddr.addrSpec.u.pSipUri));
				if (pSrcManHdrs->pFrom->nameAddr.addrSpec.u.pSipUri == NULL)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tpal_MemoryAllocate() for pSrcManHdrs->pFrom->nameAddr.addrSpec.u.pSipUri, error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}

				pSipUri = pSrcManHdrs->pFrom->nameAddr.addrSpec.u.pSipUri;

				error = _EcrioSigMgrStringCreate(pSigMgr, (u_char*)ECRIO_SIG_MGR_SIGNALING_ANONYMOUS_HOST, &pSipUri->pDomain);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrStringCreate() for pSipUri->pDomain, error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}

				error = _EcrioSigMgrStringCreate(pSigMgr, (u_char*)ECRIO_SIG_MGR_SIGNALING_ANONYMOUS, &pSipUri->pUserId);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrStringCreate() for pSipUri->pUserId, error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}

				if (pSigMgr->pOOMObject->ec_oom_HasUserPhoneSupport(pSipMessage->eMethodType, EcrioSipHeaderTypeFrom, EcrioSipURIType_SIP) == Enum_TRUE)
				{
					/* All outgoing SIP uri which contain telephone number in user info
					** part must have "user=phone" parameter as SIP uri parameter.
					** Hence following piece of code.
					*/
					error = _EcrioSigMgrAddUserPhoneParam(pSigMgr, &pSrcManHdrs->pFrom->nameAddr.addrSpec);
					if (error != ECRIO_SIGMGR_NO_ERROR)
					{
						SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
							"%s:%u\t_EcrioSigMgrAddUserPhoneParam, error=%u", __FUNCTION__, __LINE__, error);
						goto Error_Level_01;
					}
				}
			}
			/*else if (pSigMgr->pSignalingInfo->uriScheme == EcrioSigMgrURISchemeTEL)
			{
			}*/
			else
			{
				error = ECRIO_SIGMGR_INVALID_URI_SCHEME;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tFailed since INVALID_URI_SCHEME",
					__FUNCTION__, __LINE__);
				goto Error_Level_01;
			}
		}
	}

	if ((pSrcManHdrs->pFrom->nameAddr.pDisplayName == NULL) && (pSigMgr->pSignalingInfo->pDisplayName != NULL) && (pSipMessage->bPrivacy != Enum_TRUE))
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSigMgr->pSignalingInfo->pDisplayName,
			&(pSrcManHdrs->pFrom->nameAddr.pDisplayName));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrStringCreate() for pDisplayName, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	/* generate From tag if required. */
	for (i = 0; i < pSrcManHdrs->pFrom->numParams; ++i)
	{
		if (pal_StringICompare(pSrcManHdrs->pFrom->ppParams[i]->pParamName,
			(u_char *)ECRIO_SIG_MGR_SIP_TAG_STRING) == 0)
		{
			generateFromTag = Enum_FALSE;
			break;
		}
	}

	if (generateFromTag == Enum_TRUE)
	{
		EcrioSigMgrParamStruct *pTagParam = NULL;

		if (pSrcManHdrs->pFrom->numParams == 0)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct *) * 1, (void **)&pSrcManHdrs->pFrom->ppParams);
			if (pSrcManHdrs->pFrom->ppParams == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryAllocate() for pSrcManHdrs->pFrom->ppParams, error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}

			pSrcManHdrs->pFrom->numParams = 1;
		}
		else
		{
			/* Check arithmetic overflow - do not use the ArithmeticOverflowDetected() API for u_int16 variable. */
			if (pSrcManHdrs->pFrom->numParams + 1 > USHRT_MAX)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((pSrcManHdrs->pFrom->numParams + 1), sizeof(EcrioSigMgrParamStruct *)) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = pal_MemoryReallocate(
				sizeof(EcrioSigMgrParamStruct *) * (pSrcManHdrs->pFrom->numParams + 1),
				(void **)&pSrcManHdrs->pFrom->ppParams);
			if (error != KPALErrorNone)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryReallocate() for pSrcManHdrs->pFrom->ppParams, error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}

			++(pSrcManHdrs->pFrom->numParams);
		}

		i = pSrcManHdrs->pFrom->numParams;
		pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&(pSrcManHdrs->pFrom->ppParams[i - 1]));
		if (pSrcManHdrs->pFrom->ppParams[i - 1] == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pSrcManHdrs->pFrom->ppParams[i - 1], error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		pTagParam = pSrcManHdrs->pFrom->ppParams[i - 1];

		error = _EcrioSigMgrStringCreate(pSigMgr, (u_char *)ECRIO_SIG_MGR_SIP_TAG_STRING,
			&pTagParam->pParamName);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrStringCreate() for pTagParam->pParamName, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		error = _EcrioSigMgrGenerateTag(&pTagParam->pParamValue);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrGenerateTag() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	/* Via Header */
	if (pSrcManHdrs->numVia > 0)
	{
		/* Send rport only if IPSec is disabled */
		if (((eMsgType == EcrioSigMgrSIPRequest) && (pSigMgr->pIPSecDetails != NULL) && (pSigMgr->pIPSecDetails->eIPSecState == ECRIO_SIP_IPSEC_STATE_ENUM_None))
			|| ((eMsgType == EcrioSigMgrSIPRequest) && (pSigMgr->pIPSecDetails == NULL)))
		{
			if (pSrcManHdrs->ppVia[0]->numParams == 0)
			{
				pSrcManHdrs->ppVia[0]->numParams = 1;
				pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct *) * pSrcManHdrs->ppVia[0]->numParams, (void **)&pSrcManHdrs->ppVia[0]->ppParams);
				if (pSrcManHdrs->ppVia[0]->ppParams == NULL)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tpal_MemoryAllocate() for pSrcManHdrs->ppVia[0]->ppParams, error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}

				pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&pSrcManHdrs->ppVia[0]->ppParams[0]);
				if (pSrcManHdrs->ppVia[0]->ppParams[0] == NULL)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tpal_MemoryAllocate() for pSrcManHdrs->ppVia[0]->ppParams[0], error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}

				error = _EcrioSigMgrStringCreate(pSigMgr, (u_char *)"rport", &(pSrcManHdrs->ppVia[0]->ppParams[0]->pParamName));
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrStringCreate() for pSrcManHdrs->ppVia[0]->ppParams[0]->pParamName, error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}

				pSrcManHdrs->ppVia[0]->ppParams[0]->pParamValue = NULL;
			}
			else
			{
				pSrcManHdrs->ppVia[0]->numParams += 1;

				/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
				if (pal_UtilityDataOverflowDetected(pSrcManHdrs->ppVia[0]->numParams, sizeof(EcrioSigMgrParamStruct *)) == Enum_TRUE)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					goto Error_Level_01;
				}

				error = pal_MemoryReallocate(sizeof(EcrioSigMgrParamStruct *) * pSrcManHdrs->ppVia[0]->numParams,
					(void **)&pSrcManHdrs->ppVia[0]->ppParams);
				if (error != KPALErrorNone)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tpal_MemoryReallocate() for pSrcManHdrs->ppVia[0]->ppParams, error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}

				pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&pSrcManHdrs->ppVia[0]->ppParams[(pSrcManHdrs->ppVia[0]->numParams - 1)]);
				if (pSrcManHdrs->ppVia[0]->ppParams[(pSrcManHdrs->ppVia[0]->numParams - 1)] == NULL)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tpal_MemoryAllocate() for pSrcManHdrs->ppVia[0]->ppParams[(pSrcManHdrs->ppVia[0]->numParams-1)], error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}

				error = _EcrioSigMgrStringCreate(pSigMgr, (u_char *)"rport", &(pSrcManHdrs->ppVia[0]->ppParams[(pSrcManHdrs->ppVia[0]->numParams - 1)]->pParamName));
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrStringCreate() for pSrcManHdrs->ppVia[0]->ppParams[(pSrcManHdrs->ppVia[0]->numParams-1)]->pParamName, error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}

				pSrcManHdrs->ppVia[0]->ppParams[(pSrcManHdrs->ppVia[0]->numParams - 1)]->pParamValue = NULL;
			}
		}
	}
	else
	{
		pSrcManHdrs->numVia = 1;
		pal_MemoryAllocate(sizeof(EcrioSigMgrViaStruct *) * 1, (void **)&pSrcManHdrs->ppVia);
		if (pSrcManHdrs->ppVia == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pSrcManHdrs->ppVia, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		pal_MemoryAllocate(sizeof(EcrioSigMgrViaStruct), (void **)&pSrcManHdrs->ppVia[0]);
		if (pSrcManHdrs->ppVia[0] == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pSrcManHdrs->ppVia[0], error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		error = _EcrioSigMgrStringCreate(pSigMgr, (u_char *)ECRIO_SIG_MGR_SIP_VERSION,
			&pSrcManHdrs->ppVia[0]->pSipVersion);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrStringCreate() for pSipVersion, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		pSrcManHdrs->ppVia[0]->pIPaddr.port = (u_int16)pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[0].pChannelInfo->localPort;

		if (pSigMgr->pSignalingInfo->bEnableTls == Enum_TRUE && pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex != 0)
		{
			pSrcManHdrs->ppVia[0]->transport = EcrioSigMgrTransportTLS;
		}
		else if (pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE)
		{
			pSrcManHdrs->ppVia[0]->transport = EcrioSigMgrTransportUDP;
		}
		else if (pSigMgr->pSignalingInfo->bEnableUdp == Enum_FALSE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE)
		{
			pSrcManHdrs->ppVia[0]->transport = EcrioSigMgrTransportTCP;
		}

		if ((pSigMgr->bIPSecEnabled == Enum_TRUE) && (pSigMgr->pIPSecDetails != NULL) && (pSigMgr->pIPSecDetails->eIPSecState != ECRIO_SIP_IPSEC_STATE_ENUM_None))
		{
			u_int16 uIndex = 0;
			EcrioTxnMgrSignalingCommunicationChannelStruct *pChannels = NULL;

			if (pSigMgr->pSignalingInfo->bEnableUdp == Enum_FALSE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE)
			{
				uIndex = pSigMgr->pSigMgrTransportStruct->sendingIPsecTCPChannelIndex - 1;
			}
			else
			{
				uIndex = pSigMgr->pSigMgrTransportStruct->sendingIPsecUdpChannelIndex - 1;
			}

			pChannels = &pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[uIndex];
			if (pChannels != NULL)
			{
				if ((pChannels->pChannelInfo != NULL) && (pChannels->pChannelInfo->bIPsecProtected == Enum_TRUE))
				{
					pSrcManHdrs->ppVia[0]->pIPaddr.port = pChannels->pChannelInfo->localPort;
				}
			}
		}

		error = _EcrioSigMgrStringCreate(pSigMgr, (u_char *)pNetworkInfo->pLocalIp,
			&pSrcManHdrs->ppVia[0]->pIPaddr.pIPAddr);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrStringCreate() for pSrcManHdrs->ppVia[0]->pIPaddr.pIPAddr, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		if (pBranch == NULL)
		{
			error = _EcrioSigMgrGenerateBranchParam(pSigMgr, &(pSrcManHdrs->ppVia[0]->pBranch));
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u_EcrioSigMgrGenerateBranchParam() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}
		else
		{
			error = _EcrioSigMgrStringCreate(pSigMgr, pBranch, &(pSrcManHdrs->ppVia[0]->pBranch));
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrStringCreate() for pSrcManHdrs->ppVia[0]->pBranch, error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
		}

		/* Send rport only if IPSec is disabled */
		if (((eMsgType == EcrioSigMgrSIPRequest) && (pSigMgr->pIPSecDetails != NULL) && (pSigMgr->pIPSecDetails->eIPSecState == ECRIO_SIP_IPSEC_STATE_ENUM_None))
			|| ((eMsgType == EcrioSigMgrSIPRequest) && (pSigMgr->pIPSecDetails == NULL)))
		{
			pSrcManHdrs->ppVia[0]->numParams = 1;
			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct *) * pSrcManHdrs->ppVia[0]->numParams, (void **)&pSrcManHdrs->ppVia[0]->ppParams);
			if (pSrcManHdrs->ppVia[0]->ppParams == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryAllocate() for pSrcManHdrs->ppVia[0]->ppParams, error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}

			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&pSrcManHdrs->ppVia[0]->ppParams[0]);
			if (pSrcManHdrs->ppVia[0]->ppParams[0] == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryAllocate() for pSrcManHdrs->ppVia[0]->ppParams[0], error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}

			error = _EcrioSigMgrStringCreate(pSigMgr, (u_char *)"rport", &(pSrcManHdrs->ppVia[0]->ppParams[0]->pParamName));
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrStringCreate() for pSrcManHdrs->ppVia[0]->ppParams[0]->pParamName, error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}

			pSrcManHdrs->ppVia[0]->ppParams[0]->pParamValue = NULL;
		}
	}

	/* CallID Header */ /* TODO: Fill User-Id */
	if (pSrcManHdrs->pCallId == NULL)
	{
		error = _EcrioSigMgrGenerateCallId(
			(u_char *)"user-id", &pSrcManHdrs->pCallId);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrGenerateCallId() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

Error_Level_01:

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit,
// quiet		"%s:%u\terror=%u", __FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrIMSHandleMessage()

Purpose:		IMS handler function for incoming messages.

Description:	IMS handler function for incoming messages.

Input:			EcrioSigMgrStruct* pSigMgr - SigMgrI nstance.
                u_char* pReceiveData - received buffer.
                u_int32 receiveLength - received length.

OutPut:			None

Returns:		error code.
**************************************************************************/
void _EcrioSigMgrIMSHandleMessage
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pReceiveData,
	u_int32 receiveLength
)
{
	EcrioSigMgrSipMessageStruct *pSipMsg = NULL;
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	error = _EcrioSigMgrSipMessageParse(pSigMgr, pReceiveData, &pSipMsg, receiveLength);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		_EcrioSigMgrSendInfo(pSigMgr, EcrioSigMgrError, ECRIO_SIGMGR_BAD_MESSAGE_RECEIVED, NULL);
		goto Error_Level_01;
	}

	error = _EcrioSigMgrCheckTLSTransport(pSigMgr, pSipMsg);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tThe wrong transport in received TLS data!",
			__FUNCTION__, __LINE__);

		goto Error_Level_01;
	}

	if (pSipMsg->eReqRspType == EcrioSigMgrSIPRequest)
	{
		error = _EcrioSigMgrHandleIncomingRequests(pSigMgr, pSipMsg);
	}
	else
	{
		error = _EcrioSigMgrHandleIncomingResponses(pSigMgr, pSipMsg);
	}

Error_Level_01:

	if (pSipMsg != NULL)
	{
		_EcrioSigMgrReleaseSipMessage(pSigMgr, pSipMsg);
		pal_MemoryFree((void **)&pSipMsg);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);
}

/**************************************************************************

Function:		_EcrioSigMgrHandleIncomingRequests()

Purpose:		handler function for incoming SIP Requests.

Description:	handler function for incoming SIP Requests.

Input:			EcrioSigMgrStruct* pSigMgr - SigMgrI nstance.
EcrioSigMgrSipMessageStruct *pReceivedData -
parsed IMS data buffer.

OutPut:			None

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrHandleIncomingRequests
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessage
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrValidationStruct validateStruct = { 0 };

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pSipMessage->pMandatoryHdrs->pTo == NULL)
	{
		SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tTo header is missing", __FUNCTION__, __LINE__);
		_EcrioSigMgrSendInfo(pSigMgr, EcrioSigMgrError, ECRIO_SIGMGR_BAD_MESSAGE_RECEIVED, NULL);
		goto Error_Level_01;
	}

	if (pSipMessage->eMethodType == EcrioSipMessageTypeBye ||
		pSipMessage->eMethodType == EcrioSipMessageTypeAck ||
		pSipMessage->eMethodType == EcrioSipMessageTypePrack ||
		pSipMessage->eMethodType == EcrioSipMessageTypeNotify)
	{
		BoolEnum bToTagPresent = Enum_FALSE;
		u_int32 i = 0;
		EcrioSigMgrNameAddrWithParamsStruct *pTo = pSipMessage->pMandatoryHdrs->pTo;

		for (i = 0; i < pTo->numParams; i++)
		{
			if (pal_StringICompare(pTo->ppParams[i]->pParamName, \
				(u_char *)ECRIO_SIG_MGR_SIP_TAG_STRING) == 0)
			{
				bToTagPresent = Enum_TRUE;
				break;
			}
		}

		if (bToTagPresent == Enum_FALSE)
		{
			SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tTo tag not present", __FUNCTION__, __LINE__);

			if (pSipMessage->eMethodType != EcrioSipMessageTypeAck)
			{
				// _EcrioSigMgrSendBadResponse(pSigMgr, pReceivedData, pReceiveData, receiveLength);

				_EcrioSigMgrSendInfo(pSigMgr, EcrioSigMgrError, ECRIO_SIGMGR_BAD_MESSAGE_RECEIVED, NULL);
			}

			goto Error_Level_01;
		}
	}

	if ((pSipMessage->pMandatoryHdrs->pFrom == NULL) ||
		(pSipMessage->pMandatoryHdrs->pFrom->ppParams == NULL))
	{
		SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tFrom tag not present", __FUNCTION__, __LINE__);

		// _EcrioSigMgrSendBadResponse(pSigMgr, pReceivedData, pReceiveData, receiveLength);
		_EcrioSigMgrSendInfo(pSigMgr, EcrioSigMgrError, ECRIO_SIGMGR_BAD_MESSAGE_RECEIVED, NULL);
		goto Error_Level_01;
	}

	if (pSipMessage->pMandatoryHdrs->maxForwards > ECRIO_SIG_MGR_RANGE_MAX_FORWARDS)
	{
		SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tMax forward crossed ECRIO_SIG_MGR_RANGE_MAX_FORWARDS", __FUNCTION__, __LINE__);

		// _EcrioSigMgrSendBadResponse(pSigMgr, pReceivedData, pReceiveData, receiveLength);
		_EcrioSigMgrSendInfo(pSigMgr, EcrioSigMgrError, ECRIO_SIGMGR_BAD_MESSAGE_RECEIVED, NULL);
		goto Error_Level_01;
	}

	error = _EcrioSigMgrIMSCheckIsRetranMsg(pSigMgr, pSipMessage->pMandatoryHdrs,
		_EcrioSigMgrGetMthdName(pSipMessage->eMethodType), EcrioSigMgrRoleUAS, NULL, ECRIO_SIGMGR_INVALID_RESPONSE_CODE);
	if ((error == ECRIO_SIGMGR_NO_ERROR) || (error == ECRIO_SIGMGR_TRANSACTION_NOT_FOUND))
	{
		validateStruct.pSipMessage = pSipMessage;

		error = _EcrioSigMgrValidateIncomingRequests(pSigMgr, &validateStruct);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrValidateIncomingRequests() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}

		SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrValidateIncomingRequests() returns with no error, ReasonPhrase=%s, ResponseCode=%u",
			__FUNCTION__, __LINE__, validateStruct.reasonPhrase, validateStruct.responseCode);

		validateStruct.cmnInfo.role = EcrioSigMgrRoleUAS;

		switch (pSipMessage->eMethodType)
		{
			case EcrioSipMessageTypeMessage:
			{
				validateStruct.cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_MESSAGE;

				error = _EcrioSigMgrHandleMessageRequest(pSigMgr, pSipMessage,
					&validateStruct.cmnInfo, validateStruct.reasonPhrase, validateStruct.responseCode);
				//error = _EcrioSigMgrIMSHandleMessageRequest(pSigMgr, pSipMessage);
			}
			break;

			case EcrioSipMessageTypeInvite:
			{
				//error = _EcrioSigMgrIMSHandleInviteRequest(pSigMgr, pSipMessage);
				validateStruct.cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_INVITE;

				error = _EcrioSigMgrHandleInviteRequest(pSigMgr, pSipMessage,
					&validateStruct.cmnInfo, validateStruct.reasonPhrase, validateStruct.responseCode);
			}
			break;

			case EcrioSipMessageTypeBye:
			{
				//error = _EcrioSigMgrIMSHandleByeRequest(pSigMgr, pSipMessage);

				validateStruct.cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_BYE;

				error = _EcrioSigMgrHandleByeRequest(pSigMgr, pSipMessage, &validateStruct.cmnInfo,
					validateStruct.responseCode, validateStruct.reasonPhrase);
			}
			break;

			case EcrioSipMessageTypeCancel:
			{
				error = _EcrioSigMgrHandleCancelRequest(pSigMgr, pSipMessage, &validateStruct.cmnInfo,
					validateStruct.responseCode, validateStruct.reasonPhrase);
				//error = _EcrioSigMgrIMSHandleCancelRequest(pSigMgr, pSipMessage);
			}
			break;

			case EcrioSipMessageTypeAck:
			{
				validateStruct.cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_ACK;

				error = _EcrioSigMgrHandleAckRequest(pSigMgr, pSipMessage, &validateStruct.cmnInfo,
					validateStruct.responseCode, validateStruct.reasonPhrase);
				//error = _EcrioSigMgrIMSHandleAckRequest(pSigMgr, pSipMessage);
			}
			break;

			case EcrioSipMessageTypePrack:
			{
				validateStruct.cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_PRACK;

				error = _EcrioSigMgrHandlePrackRequest(pSigMgr, pSipMessage, &validateStruct.cmnInfo,
					validateStruct.responseCode, validateStruct.reasonPhrase);

				//error = _EcrioSigMgrIMSHandlePrackRequest(pSigMgr, pSipMessage);
			}
			break;
			case EcrioSipMessageTypeOptions:
			{
				validateStruct.cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_OPTIONS;
 
				error = _EcrioSigMgrHandleOptionsRequest(pSigMgr, pSipMessage,
					&validateStruct.cmnInfo,
					validateStruct.responseCode, validateStruct.reasonPhrase);

			}
			break;
			case EcrioSipMessageTypeNotify:
			{
				validateStruct.cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_NOTIFY;

				error = _EcrioSigMgrHandleNotifyRequest(pSigMgr, pSipMessage, &validateStruct.cmnInfo,
					validateStruct.responseCode, validateStruct.reasonPhrase);
				//error = _EcrioSigMgrIMSHandleNotifyRequest(pSigMgr, pSipMessage);
			}
			break;

			case EcrioSipMessageTypeUpdate:
			{
				validateStruct.cmnInfo.pMethodName = (u_char *)ECRIO_SIG_MGR_METHOD_UPDATE;

				error = _EcrioSigMgrHandleUpdateRequest(pSigMgr, pSipMessage, &validateStruct.cmnInfo,
					validateStruct.responseCode, validateStruct.reasonPhrase);

				//error = _EcrioSigMgrIMSHandleUpdateRequest(pSigMgr, pSipMessage);
			}
			break;

			default:
			{
				if (NULL == pal_StringNCopy(validateStruct.reasonPhrase, ECRIO_SIGMGR_BUFFER_SIZE_64, (u_char *)ECRIO_SIGMGR_RESPONSE_METHOD_NOT_ALLOWED, pal_StringLength((const u_char *)ECRIO_SIGMGR_RESPONSE_METHOD_NOT_ALLOWED)))
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tString Copy error",
						__FUNCTION__, __LINE__);
					error = ECRIO_SIGMGR_STRING_ERROR;

					goto Error_Level_01;					
				}
				validateStruct.responseCode = ECRIO_SIGMGR_RESPONSE_CODE_METHOD_NOT_ALLOWED;

				// generate 405(Method not allowed) response
				error = _EcrioSigMgrSendErrorResponse(pSigMgr, pSipMessage, 
					_EcrioSigMgrGetMthdName(pSipMessage->eMethodType), 
					validateStruct.responseCode, validateStruct.reasonPhrase);
			}
			break;
		}
	}
	

Error_Level_01:

	validateStruct.cmnInfo.pMethodName = NULL;
	_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &validateStruct.cmnInfo);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrHandleIncomingResponses()

Purpose:		IMS handler function for incoming responses.

Description:	IMS handler function for incoming responses.

Input:			EcrioSigMgrStruct* pSigMgr - SigMgrI nstance.
                EcrioSigMgrSipMessageStruct *pReceivedData -
                received data structure from IMS.

OutPut:			None

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrHandleIncomingResponses
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessage
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_char *pTxnSessionMatchingParam = NULL;
	BoolEnum bCommonInfoPopulated = Enum_FALSE;
	EcrioSigMgrCommonInfoStruct cmnInfo = { 0 };
	EcrioSigMgrHeaderStruct *pContentType = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);
	// check if message has been retransmitted
	error = _EcrioSigMgrIMSCheckIsRetranMsg(pSigMgr, pSipMessage->pMandatoryHdrs,
		_EcrioSigMgrGetMthdName(pSipMessage->eMethodType),
		EcrioSigMgrRoleUAC, &pTxnSessionMatchingParam, pSipMessage->responseCode);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		if (!((error == ECRIO_SIGMGR_TRANSACTION_NOT_FOUND) &&
			((pSipMessage->eMethodType == EcrioSipMessageTypeInvite) ||
			(pSipMessage->eMethodType == EcrioSipMessageTypeUpdate)) &&
			(pSipMessage->responseCode == ECRIO_SIGMGR_RESPONSE_CODE_OK)))
		{
			goto Error_Level_01;
		}
	}

	error = _EcrioSigMgrPopulateCommonInfo(pSigMgr, pSipMessage->pMandatoryHdrs, &cmnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateCommonInfo() for cmnInfo, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	cmnInfo.responseCode = pSipMessage->responseCode;
	bCommonInfoPopulated = Enum_TRUE;

	if (cmnInfo.responseCode != ECRIO_SIGMGR_RESPONSE_CODE_TRYING &&
		cmnInfo.pToTag == NULL)
	{
		_EcrioSigMgrSendInfo(pSigMgr, EcrioSigMgrError,
			ECRIO_SIGMGR_BAD_MESSAGE_RECEIVED, NULL);
		error = ECRIO_SIGMGR_BAD_MESSAGE_RECEIVED;

		goto Error_Level_01;
	}

	pContentType = pSipMessage->pContentType;
	pSipMessage->pContentType = NULL;

	if (pSipMessage->pMessageBody != NULL)
	{
		if (pSipMessage->pMessageBody->messageBodyType == EcrioSigMgrMessageBodyUnknown ||
			pSipMessage->pMessageBody->messageBodyType == EcrioSigMgrMessageBodySDP)
		{
			EcrioSigMgrUnknownMessageBodyStruct *pUnknownBody =
				(EcrioSigMgrUnknownMessageBodyStruct *)pSipMessage->pMessageBody->pMessageBody;

			if ((pContentType != NULL) && (pContentType->ppHeaderValues != NULL) &&
				(pContentType->ppHeaderValues[0] != NULL))
			{
				_EcrioSigMgrReleaseHdrValueStruct(pSigMgr, &pUnknownBody->contentType);
#if 0
				if (pUnknownBody->contentType.pHeaderValue != NULL)
				{
					pal_MemoryFree((void **)&(pUnknownBody->contentType.pHeaderValue));
				}
#endif // Release Header value Params also
				pUnknownBody->contentType = *(pContentType->ppHeaderValues[0]);
				pal_MemorySet((void *)pContentType->ppHeaderValues[0], 0, sizeof(EcrioSigMgrHeaderValueStruct));
			}
		}
	}

	switch (pSipMessage->eMethodType)
	{
		case EcrioSipMessageTypeRegister:
		{
			error = _EcrioSigMgrHandleRegResponse(pSigMgr, pSipMessage);
		}
		break;

		case EcrioSipMessageTypeMessage:
		{
			error = _EcrioSigMgrHandleMessageResponse(pSigMgr, pSipMessage, &cmnInfo);
		}
		break;

		case EcrioSipMessageTypeInvite:
		{
			error = _EcrioSigMgrHandleInviteResponse(pSigMgr, pSipMessage, &cmnInfo, Enum_FALSE);
		}
		break;
		case EcrioSipMessageTypePublish:
		{
			error = _EcrioSigMgrHandlePublishResponse(pSigMgr, pSipMessage, &cmnInfo);
		}
		break;
		case EcrioSipMessageTypeBye:
		{
			error = _EcrioSigMgrHandleByeResponse(pSigMgr, pSipMessage, &cmnInfo, Enum_FALSE);
		}
		break;

		case EcrioSipMessageTypeCancel:
		{
			error = _EcrioSigMgrHandleCancelResponse(pSigMgr, pSipMessage, &cmnInfo, Enum_FALSE);
		}
		break;

		case EcrioSipMessageTypePrack:
		{
			error = _EcrioSigMgrHandlePrackResponse(pSigMgr, pSipMessage, &cmnInfo, Enum_FALSE);
		}
		break;

		case EcrioSipMessageTypeSubscribe:
		{
			error = _EcrioSigMgrHandleSubscribeResponse(pSigMgr, pSipMessage, &cmnInfo, Enum_FALSE);
		}
		break;

		case EcrioSipMessageTypeUpdate:
		{
			error = _EcrioSigMgrHandleUpdateResponse(pSigMgr, pSipMessage, &cmnInfo, Enum_FALSE);
		}
		break;

		case EcrioSipMessageTypeRefer:
		{
			error = _EcrioSigMgrHandleReferResponse(pSigMgr, pSipMessage, &cmnInfo);
		}
		break;
		case EcrioSipMessageTypeOptions:
		{
			error = _EcrioSigMgrHandleOptionsResponse(pSigMgr, pSipMessage, &cmnInfo);
		}
		break;

		default:
		{
		}
		break;
	}

Error_Level_01:

	if (pTxnSessionMatchingParam != NULL)
	{
		pal_MemoryFree((void **)&pTxnSessionMatchingParam);
	}

	if (pContentType != NULL)
	{
		EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_Header,
			(void **)&pContentType, Enum_TRUE);
	}

	if (bCommonInfoPopulated == Enum_TRUE)
	{
		cmnInfo.pMethodName = NULL;
		_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr, &cmnInfo);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrIMSCheckIsRetranMsg()

Purpose:		Checks if a message has been retransmitted.

Description:	Checks if a message has been retransmitted.

Input:			EcrioSigMgrStruct* pSigMgr - SigMgrI nstance.
                EcrioIMSLibrarySignalingMandatoryStruct* pIMSManHdrs - IMS mandatory headers
                u_char* pMethod - method  name.
                EcrioSigMgrRoleEnum role - role UAC or UAS.

OutPut:			EcrioSigMgrRouteStruct** ppRouteHdr - route header.

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrIMSCheckIsRetranMsg
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrMandatoryHeaderStruct *pMandatoryHdrs,
	u_char *pMethod,
	EcrioSigMgrRoleEnum role,
	u_char **pTxnSessionMatchingString,
	u_int32 responseCode
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR, i = 0;
	EcrioSigMgrCommonInfoStruct commonInfo = {0};
	EcrioSigMgrTXNInfoStruct txnInfo = {0};
	BoolEnum isInvite = Enum_TRUE;
	u_char *pTempTxnSessionMatchingParam = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if ((pMethod == NULL) || (pMandatoryHdrs == NULL))
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	if (role == EcrioSigMgrRoleUAC)
	{
		if (pTxnSessionMatchingString == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tInsufficient data", __FUNCTION__, __LINE__);

			error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
			goto Error_Level_01;
		}
	}

	commonInfo.cSeq = pMandatoryHdrs->CSeq;
	commonInfo.pBranch = pMandatoryHdrs->ppVia[0]->pBranch;
	commonInfo.pCallId = pMandatoryHdrs->pCallId;
	commonInfo.role = role;
	commonInfo.pMethodName = pMethod;

	if ((pMandatoryHdrs->pTo != NULL) && (pMandatoryHdrs->pTo->ppParams != NULL))
	{
		/*u_int32 i = 0;*/
		for (i = 0; i < pMandatoryHdrs->pTo->numParams; i++)
		{
			if (!(pal_StringICompare(pMandatoryHdrs->pTo->ppParams[i]->pParamName,
				(u_char *)ECRIO_SIG_MGR_SIP_TAG_STRING)))
			{
				commonInfo.pToTag = pMandatoryHdrs->pTo->ppParams[i]->pParamValue;
				break;
			}
		}
	}

	if ((pMandatoryHdrs->pFrom != NULL) && (pMandatoryHdrs->pFrom->ppParams != NULL))
	{
		/*u_int32 i = 0;*/
		for (i = 0; i < pMandatoryHdrs->pFrom->numParams; i++)
		{
			if (!(pal_StringICompare(pMandatoryHdrs->pFrom->ppParams[i]->pParamName,
				(u_char *)ECRIO_SIG_MGR_SIP_TAG_STRING)))
			{
				commonInfo.pFromTag = pMandatoryHdrs->pFrom->ppParams[i]->pParamValue;
				break;
			}
		}
	}

	if (role == EcrioSigMgrRoleUAC)
	{
		commonInfo.responseCode = responseCode;
	}

	isInvite = ((pal_StringICompare(pMethod, (const u_char *)ECRIO_SIG_MGR_METHOD_INVITE) == 0) ||
		(pal_StringICompare(pMethod, (const u_char *)ECRIO_SIG_MGR_METHOD_ACK) == 0)) ? Enum_TRUE : Enum_FALSE;

	txnInfo.pSessionMappingStr = commonInfo.pCallId;

	if (role == EcrioSigMgrRoleUAC)
	{
		if (isInvite == Enum_TRUE)
		{
			txnInfo.messageType = EcrioSigMgrMessageInviteResponse;
		}
		else
		{
			txnInfo.messageType = EcrioSigMgrMessageNonInviteResponse;
		}
	}
	else
	{
		if (isInvite == Enum_TRUE)
		{
			txnInfo.messageType = EcrioSigMgrMessageInviteRequest;
		}
		else
		{
			txnInfo.messageType = EcrioSigMgrMessageNonInviteRequest;
		}
	}

	if (role == EcrioSigMgrRoleUAC)
	{
		/* get the maching session parameter string */
		_EcrioSigMgrGetTxnMappingParameter(pSigMgr,
			pMandatoryHdrs, pMethod, &pTempTxnSessionMatchingParam);
		*pTxnSessionMatchingString = pTempTxnSessionMatchingParam;
	}

	txnInfo.transport = pMandatoryHdrs->ppVia[0]->transport;

	error = _EcrioSigMgrSendMsgToTXNHandler(pSigMgr, &commonInfo, &txnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		goto Error_Level_01;
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrIMSHandleRegisterResponses()
Purpose:		IMS handler function for incoming  Register responses.

Description:	IMS handler function for incoming Register responses.

Input:			EcrioSigMgrStruct* pSigMgr - SigMgr Instance.
                EcrioSigMgrSipMessageStruct *pReceivedData -
                received data structure from IMS.

OutPut:			None

Returns:		error code.
**************************************************************************/

/*Newly added by Bb - Support for TCP START*/
u_int32 _EcrioSigMgrCreateTCPBasedBuffer
(
	EcrioSigMgrStruct *pSigMgr,
	u_char **ppMsgIn,
	EcrioSigMgrSIPMessageTypeEnum eMsgType
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_char *pMsg = *ppMsgIn;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	switch (eMsgType)
	{
		case EcrioSigMgrSIPRequest:
		{
			u_char *pTemp = NULL;
			u_char udpPort[16] = { 0 };
			u_char tcpPort[16] = { 0 };

			// changing via header transport=UDP to transport=TCP
			pTemp = pal_SubString(pMsg, (u_char *)"SIP/2.0/UDP");
			if (pTemp == NULL)
			{
				//If "SIP/2.0/UDP" is not present then no need to replace
				break;
			}
			pTemp += pal_StringLength((u_char *)"SIP/2.0/UDP");

			while (*pTemp != '/')
			{
				pTemp--;
			}

			pTemp++;
			pal_MemoryCopy((void *)pTemp, pal_StringLength((const u_char*)ECRIO_SIG_MGR_SIP_TCP_PARAM), (void *)ECRIO_SIG_MGR_SIP_TCP_PARAM, pal_StringLength((const u_char*)ECRIO_SIG_MGR_SIP_TCP_PARAM));

			// changing contact header transport=UDP to transport=TCP
			pTemp = NULL;
			pTemp = pal_SubString(pMsg, (u_char *)"transport=UDP");
			if (pTemp == NULL)
			{
				//If transport=UDP is not present then no need to replace
				break;
			}
			pTemp += pal_StringLength((u_char *)"transport=UDP");
			while (*pTemp != '=')
			{
				pTemp--;
			}

			pTemp++;
			pal_MemoryCopy((void *)pTemp, pal_StringLength((const u_char*)ECRIO_SIG_MGR_SIP_TCP_PARAM), (void *)ECRIO_SIG_MGR_SIP_TCP_PARAM, pal_StringLength((const u_char*)ECRIO_SIG_MGR_SIP_TCP_PARAM));
			
#ifndef ENABLE_QCMAPI
			// changing UDP local port to TCP local port if TCP local port and Udp local ports are different.
			if (pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[0].pChannelInfo->localPort != pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[1].pChannelInfo->localPort)
			{
				if (0 >= pal_StringSNPrintf((char *)&udpPort, 16, "%d", pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[0].pChannelInfo->localPort))
				{
					return ECRIO_SIGMGR_STRING_ERROR;
				}

				pTemp = NULL;
				pTemp = pal_SubString(pMsg, (u_char *)udpPort);
				pTemp += pal_StringLength((u_char *)udpPort);
				while (*pTemp != ':')
				{
					pTemp--;
				}
				pTemp++;
				if (0 >= pal_StringSNPrintf((char *)&tcpPort, 16, "%d", pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[1].pChannelInfo->localPort))
				{
					return ECRIO_SIGMGR_STRING_ERROR;
				}
				pal_MemoryCopy((void *)pTemp, pal_StringLength((const u_char*)tcpPort), (void *)tcpPort, pal_StringLength((const u_char*)tcpPort));

				pTemp = NULL;
				pTemp = pal_SubString(pMsg, (u_char *)udpPort);
				pTemp += pal_StringLength((u_char *)udpPort);
				while (*pTemp != ':')
				{
					pTemp--;
				}

				pTemp++;
				pal_MemoryCopy((void *)pTemp, pal_StringLength((const u_char*)tcpPort), (void *)tcpPort, pal_StringLength((const u_char*)tcpPort));
			}
#endif
		}
		break;

		default:
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tEerror=%u", __FUNCTION__, __LINE__, error);
			error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
			return error;
		}
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit,
		"%s:%u\terror=%u", __FUNCTION__, __LINE__, error);
	return error;
}

u_char *_EcrioSigMgrGetMthdName
(
	EcrioSipMessageTypeEnum methodType
)
{
	u_char *pMthdName = NULL;

	switch (methodType)
	{
		case EcrioSipMessageTypeRegister:
		{
			pMthdName = (u_char *)ECRIO_SIG_MGR_METHOD_REGISTER;
		}
		break;

		case EcrioSipMessageTypeMessage:
		{
			pMthdName = (u_char *)ECRIO_SIG_MGR_METHOD_MESSAGE;
		}
		break;

		case EcrioSipMessageTypeInvite:
		{
			pMthdName = (u_char *)ECRIO_SIG_MGR_METHOD_INVITE;
		}
		break;

		case EcrioSipMessageTypeBye:
		{
			pMthdName = (u_char *)ECRIO_SIG_MGR_METHOD_BYE;
		}
		break;

		case EcrioSipMessageTypeCancel:
		{
			pMthdName = (u_char *)ECRIO_SIG_MGR_METHOD_CANCEL;
		}
		break;

		case EcrioSipMessageTypeAck:
		{
			pMthdName = (u_char *)ECRIO_SIG_MGR_METHOD_ACK;
		}
		break;

		case EcrioSipMessageTypePrack:
		{
			pMthdName = (u_char *)ECRIO_SIG_MGR_METHOD_PRACK;
		}
		break;

		case EcrioSipMessageTypeSubscribe:
		{
			pMthdName = (u_char *)ECRIO_SIG_MGR_METHOD_SUBSCRIBE;
		}
		break;

		case EcrioSipMessageTypeNotify:
		{
			pMthdName = (u_char *)ECRIO_SIG_MGR_METHOD_NOTIFY;
		}
		break;

		case EcrioSipMessageTypeUpdate:
		{
			pMthdName = (u_char *)ECRIO_SIG_MGR_METHOD_UPDATE;
		}
		break; 
		case EcrioSipMessageTypePublish:
		{
			pMthdName = (u_char *)ECRIO_SIG_MGR_METHOD_PUBLISH;
		}
		break; 
		case EcrioSipMessageTypeOptions:
		{
			pMthdName = (u_char *)ECRIO_SIG_MGR_METHOD_OPTIONS;
		}
		break;
		case EcrioSipMessageTypeRefer:
		{
			pMthdName = (u_char *)ECRIO_SIG_MGR_METHOD_REFER;
		}
		break;
		case EcrioSipMessageTypeInfo:
		{
			pMthdName = (u_char *)ECRIO_SIG_MGR_METHOD_INFO;
		}
		break;
		default:
		{
			pMthdName = (u_char *)ECRIO_SIG_MGR_METHOD_NOT_SUPPORTED;
		}
		break;
	}

	return pMthdName;
}

/***************************************************

Function : _EcrioSigMgrCompareSIPURI

Purpose :  To compare two SIP URI internally.

Description :
This function is used to compare two SIP/ SIPS URI. And indicates if
any difference observed.

Input and Output Parameters :

Input :
EcrioSigMgrSipURIStruct		*pSipUriStruct1,
EcrioSigMgrSipURIStruct		*pSipUriStruct2,
BoolEnum*									pbMatchedUri

Return value :
u_int32								error code

************************************************************************************/
u_int32 _EcrioSigMgrCompareSIPURI
(
	EcrioSigMgrSipURIStruct *pSipUriStruct1,
	EcrioSigMgrSipURIStruct *pSipUriStruct2,
	BoolEnum *pbMatchedUri
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrSipURIStruct *pSipUri1;
	EcrioSigMgrSipURIStruct *pSipUri2;

	if (pSipUriStruct1 == NULL || pSipUriStruct2 == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	/* Swapping the larger Uri to be pSipUri1. */
	if (pSipUriStruct1->numURIParams > pSipUriStruct2->numURIParams)
	{
		pSipUri1 = pSipUriStruct1;
		pSipUri2 = pSipUriStruct2;
	}
	else
	{
		pSipUri2 = pSipUriStruct1;
		pSipUri1 = pSipUriStruct2;
	}

	/* As per RFC 3261:
	- URI uri-parameter components are compared as follows:
	- Any uri-parameter appearing in both URIs must match.
	- A user, ttl, or method uri-parameter appearing in only one
	URI never matches, even if it contains the default value.
	- A URI that includes an maddr parameter will not match a URI
	that contains no maddr parameter.
	- All other uri-parameters appearing in only one URI are
	ignored when comparing the URIs.
	*/

	/* @note The pal_String*Compare function will return a match if both pointer
	are NULL and no match if one pointer is NULL and the other isn't. This
	function will therefore return no match if the data doesn't agree. */

	if (pal_StringCompare(pSipUri1->pPassword, pSipUri2->pPassword) != 0)
	{
		goto NO_MATCH;
	}

	if (pal_StringCompare(pSipUri1->pUserId, pSipUri2->pUserId) != 0)
	{
		goto NO_MATCH;
	}

	if (pal_StringICompare(pSipUri1->pDomain, pSipUri2->pDomain) != 0)
	{
		goto NO_MATCH;
	}

	if ((pSipUri1->pIPAddr != NULL) && (pSipUri2->pIPAddr != NULL))
	{
		if (pal_StringICompare(pSipUri1->pIPAddr->pIPAddr, pSipUri2->pIPAddr->pIPAddr) != 0)
		{
			goto NO_MATCH;
		}

		if (pSipUri1->pIPAddr->port != pSipUri2->pIPAddr->port)
		{
			goto NO_MATCH;
		}
	}
	else
	{
		/* If only one is NULL, it can't be a match. */
		if (pSipUri1->pIPAddr != pSipUri2->pIPAddr)
		{
			goto NO_MATCH;
		}
	}

	/* Only check for potential parameter matches if there are any! */
	if (pSipUri1->numURIParams > 0)
	{
		// @todo How big can the URI index be??? Can we set to u_int8?
		// @todo Setting values here to 0 to avoid compiler warning, but really isn't needed.
		u_int16 index1;
		u_int16 index2;
		s_int16 transportIndex1 = -1;
		s_int16 transportIndex2 = -1;
		s_int16 userIndex1 = -1;
		s_int16 userIndex2 = -1;
		s_int16 ttlIndex1 = -1;
		s_int16 ttlIndex2 = -1;
		s_int16 methodIndex1 = -1;
		s_int16 methodIndex2 = -1;
		s_int16 maddrIndex1 = -1;
		s_int16 maddrIndex2 = -1;

		u_int32 uMatches = 0;

		/* Iterate through SipUri1 as the outer loop. */
		for (index1 = 0; index1 < pSipUri1->numURIParams; ++index1)
		{
			// @todo Should check pSipUri1->ppURIParams[index1] here for validity.

			switch (*pSipUri1->ppURIParams[index1]->pParamName)
			{
				case 't':
				case 'T':
				{
					/* If "Transport" present then no need to check again. */
					if ((transportIndex1 < 0) &&
						pal_StringICompare(pSipUri1->ppURIParams[index1]->pParamName, (u_char *)"Transport") == 0)
					{
						uMatches |= EcrioSigMgrPresentParam_transport1;
						transportIndex1 = index1;
					}
					/* If "ttl" present then no need to check again. */
					else if ((ttlIndex1 < 0) &&
						pal_StringICompare(pSipUri1->ppURIParams[index1]->pParamName, (u_char *)"ttl") == 0)
					{
						uMatches |= EcrioSigMgrPresentParam_ttl1;
						ttlIndex1 = index1;
					}
				}
				break;

				case 'u':
				case 'U':
				{
					/* If "user" present then no need to check again. */
					if ((userIndex1 < 0) &&
						pal_StringICompare(pSipUri1->ppURIParams[index1]->pParamName, (u_char *)"user") == 0)
					{
						uMatches |= EcrioSigMgrPresentParam_user1;
						userIndex1 = index1;
					}
				}
				break;

				case 'm':
				case 'M':
				{
					/* If "method" present then no need to check again. */
					if ((methodIndex1 < 0) &&
						pal_StringICompare(pSipUri1->ppURIParams[index1]->pParamName, (u_char *)"method") == 0)
					{
						uMatches |= EcrioSigMgrPresentParam_method1;
						methodIndex1 = index1;
					}
					/* If "maddr" present then no need to check again. */
					else if ((maddrIndex1 < 0) &&
						pal_StringICompare(pSipUri1->ppURIParams[index1]->pParamName, (u_char *)"maddr") == 0)
					{
						uMatches |= EcrioSigMgrPresentParam_maddr1;
						maddrIndex1 = index1;
					}
				}
				break;

				default:
				{
				}
				break;
			}

			/* Iterate through SipUri2 as the inner loop. */
			for (index2 = 0; index2 < pSipUri2->numURIParams; ++index2)
			{
				// @todo Should check pSipUri2->ppURIParams[index2] here for validity.

				switch (*pSipUri2->ppURIParams[index2]->pParamName)
				{
					case 't':
					case 'T':
					{
						/* If "Transport" present then no need to check again. */
						if ((transportIndex2 < 0) &&
							pal_StringICompare(pSipUri2->ppURIParams[index2]->pParamName, (u_char *)"Transport") == 0)
						{
							uMatches |= EcrioSigMgrPresentParam_transport2;
							transportIndex2 = index2;
						}
						/* If "ttl" present then no need to check again. */
						else if ((ttlIndex2 < 0) &&
							pal_StringICompare(pSipUri2->ppURIParams[index2]->pParamName, (u_char *)"ttl") == 0)
						{
							uMatches |= EcrioSigMgrPresentParam_ttl2;
							ttlIndex2 = index2;
						}
					}
					break;

					case 'u':
					case 'U':
					{
						/* If "user" present then no need to check again. */
						if ((userIndex2 < 0) &&
							pal_StringICompare(pSipUri2->ppURIParams[index2]->pParamName, (u_char *)"user") == 0)
						{
							uMatches |= EcrioSigMgrPresentParam_user2;
							userIndex2 = index2;
						}
					}
					break;

					case 'm':
					case 'M':
					{
						/* If "method" present then no need to check again. */
						if ((methodIndex2 < 0) &&
							pal_StringICompare(pSipUri2->ppURIParams[index2]->pParamName, (u_char *)"method") == 0)
						{
							uMatches |= EcrioSigMgrPresentParam_method2;
							methodIndex2 = index2;
						}
						/* If "maddr" present then no need to check again. */
						else if ((maddrIndex2 < 0) &&
							pal_StringICompare(pSipUri2->ppURIParams[index2]->pParamName, (u_char *)"maddr") == 0)
						{
							uMatches |= EcrioSigMgrPresentParam_maddr2;
							maddrIndex2 = index2;
						}
					}
					break;

					default:
					{
					}
					break;
				}

				if (pal_StringICompare(pSipUri1->ppURIParams[index1]->pParamName, pSipUri2->ppURIParams[index2]->pParamName) == 0)
				{
					if (pal_StringICompare(pSipUri1->ppURIParams[index1]->pParamValue,
						pSipUri2->ppURIParams[index2]->pParamValue) != 0)
					{
						goto NO_MATCH;
					}
				}
			}
		}

		/* In the following comparisons, we check the parameter values if they were
		present in both Uris. If they were present in only one, it is no match.
		If there were not present in either, it is not a match criteria. Note
		that the compiler should put in the bitwise or'd values. */

		/* Comparing "Transport". */
		if ((uMatches & (EcrioSigMgrPresentParam_transport1 | EcrioSigMgrPresentParam_transport2)) == (EcrioSigMgrPresentParam_transport1 | EcrioSigMgrPresentParam_transport2))
		{
			if (pal_StringICompare(pSipUri1->ppURIParams[transportIndex1]->pParamValue,
				pSipUri2->ppURIParams[transportIndex2]->pParamValue) != 0)
			{
				goto NO_MATCH;
			}
		}
		else if ((uMatches & (EcrioSigMgrPresentParam_transport1 | EcrioSigMgrPresentParam_transport2)) > 0)
		{
			goto NO_MATCH;
		}

		/* Comparing "ttl". */
		if ((uMatches & (EcrioSigMgrPresentParam_ttl1 | EcrioSigMgrPresentParam_ttl2)) == (EcrioSigMgrPresentParam_ttl1 | EcrioSigMgrPresentParam_ttl2))
		{
			if (pal_StringICompare(pSipUri1->ppURIParams[ttlIndex1]->pParamValue,
				pSipUri2->ppURIParams[ttlIndex2]->pParamValue) != 0)
			{
				goto NO_MATCH;
			}
		}
		else if ((uMatches & (EcrioSigMgrPresentParam_ttl1 | EcrioSigMgrPresentParam_ttl2)) > 0)
		{
			goto NO_MATCH;
		}

		/* Comparing "user". */
		if ((uMatches & (EcrioSigMgrPresentParam_user1 | EcrioSigMgrPresentParam_user2)) == (EcrioSigMgrPresentParam_user1 | EcrioSigMgrPresentParam_user2))
		{
			if (pal_StringICompare(pSipUri1->ppURIParams[userIndex1]->pParamValue,
				pSipUri2->ppURIParams[userIndex2]->pParamValue) != 0)
			{
				goto NO_MATCH;
			}
		}
		else if ((uMatches & (EcrioSigMgrPresentParam_user1 | EcrioSigMgrPresentParam_user2)) > 0)
		{
			goto NO_MATCH;
		}

		/* Comparing "method". */
		if ((uMatches & (EcrioSigMgrPresentParam_method1 | EcrioSigMgrPresentParam_method2)) == (EcrioSigMgrPresentParam_method1 | EcrioSigMgrPresentParam_method2))
		{
			if (pal_StringICompare(pSipUri1->ppURIParams[methodIndex1]->pParamValue,
				pSipUri2->ppURIParams[methodIndex2]->pParamValue) != 0)
			{
				goto NO_MATCH;
			}
		}
		else if ((uMatches & (EcrioSigMgrPresentParam_method1 | EcrioSigMgrPresentParam_method2)) > 0)
		{
			goto NO_MATCH;
		}

		/* Comparing "maddr". */
		if ((uMatches & (EcrioSigMgrPresentParam_maddr1 | EcrioSigMgrPresentParam_maddr2)) == (EcrioSigMgrPresentParam_maddr1 | EcrioSigMgrPresentParam_maddr2))
		{
			if (pal_StringICompare(pSipUri1->ppURIParams[maddrIndex1]->pParamValue,
				pSipUri2->ppURIParams[maddrIndex2]->pParamValue) != 0)
			{
				goto NO_MATCH;
			}
		}
		else if ((uMatches & (EcrioSigMgrPresentParam_maddr1 | EcrioSigMgrPresentParam_maddr2)) > 0)
		{
			goto NO_MATCH;
		}
	}

	*pbMatchedUri = Enum_TRUE;

	goto END;

NO_MATCH:

	*pbMatchedUri = Enum_FALSE;

END:

	return error;
}

/*********************************************************************************
Function : _EcrioSigMgrCompareTelURI

Purpose :  To compare two Tel URI.

Header:    EcrioSigMgrCommon.h


Description :
This function is used to compare two Tel URI. And indicates if
any difference observed.

Input and Output Parameters :

Input :
EcrioSigMgrTelURIStruct*   pTelURI1
EcrioSigMgrTelURIStruct*	pTelURI2
BoolEnum*								pbMatchedUri

Return value :
u_int32								error code
********************************************************************************/
u_int32 _EcrioSigMgrCompareTelURI
(
	EcrioSigMgrTelURIStruct *pTelURI1,
	EcrioSigMgrTelURIStruct *pTelURI2,
	BoolEnum *pbMatchedUri
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	if (pTelURI1 == NULL || pTelURI2 == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pTelURI1->subscriberType != pTelURI2->subscriberType)
	{
		error = ECRIO_SIGMGR_INVALID_DATA;
		*pbMatchedUri = Enum_FALSE;
		goto END;
	}

	switch (pTelURI1->subscriberType)
	{
		case EcrioSigMgrTelSubscriberGlobal:
		{
			error = _EcrioSigMgrCompareGlobalTelURI
					(pTelURI1->u.pGlobalNumber, pTelURI2->u.pGlobalNumber, pbMatchedUri);
		}
		break;

		case EcrioSigMgrTelSubscriberLocal:
		{
			error = _EcrioSigMgrCompareLocalTelURI
					(pTelURI1->u.pLocalNumber, pTelURI2->u.pLocalNumber, pbMatchedUri);
		}
		break;

		default:
		{
			*pbMatchedUri = Enum_FALSE;
			error = ECRIO_SIGMGR_INVALID_DATA;
		}
	}

END:
	return error;
}

u_int32 _EcrioSigMgrCompareTelURIParams
(
	u_int16 numTelPar1,
	EcrioSigMgrTelParStruct **ppTelPar1,
	u_int16 numTelPar2,
	EcrioSigMgrTelParStruct **ppTelPar2,
	BoolEnum *pbMatchedParams
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 uTelUriParamCount1 = 0;
	u_int32 uTelUriParamCount2 = 0;

	BoolEnum bContinued = Enum_FALSE;

	/* Verify No Of parameters */
	/* Should be of same no of parameters */

	if (numTelPar1 != numTelPar2)
	{
		error = ECRIO_SIGMGR_INVALID_DATA;
		goto EndTag;
	}

	for (uTelUriParamCount1 = 0; uTelUriParamCount1 < numTelPar1; uTelUriParamCount1++)
	{
		/* Compare Parameters */
		do
		{
			bContinued = Enum_FALSE;

			error = _EcrioSigMgrCompareTelUriParam((ppTelPar1[uTelUriParamCount1]), (ppTelPar2[uTelUriParamCount2]), &bContinued);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto EndTag;
			}

			uTelUriParamCount2++;

			if (bContinued == Enum_TRUE && uTelUriParamCount2 == numTelPar2)
			{
				error = ECRIO_SIGMGR_INVALID_DATA;
				goto EndTag;
			}
		} while (bContinued);

		uTelUriParamCount2 = 0;
	}

	*pbMatchedParams = Enum_TRUE;

EndTag:
	return error;
}

/*********************************************************************************
Function : _EcrioSigMgrCompareGlobalTelURI

Purpose :  To compare two Global Tel URI.

Header:    EcrioSigMgrCommon.h


Description :
This function is used to compare two Global Tel URI. And indicates if
any difference observed.

Input and Output Parameters :

Input :
EcrioSigMgrTelGlobalNumberStruct	*pGlobalNumber1
EcrioSigMgrTelGlobalNumberStruct	*pGlobalNumber2
BoolEnum							*pbMatchedUri

Return value :
u_int32								error code
********************************************************************************/
u_int32 _EcrioSigMgrCompareGlobalTelURI
(
	EcrioSigMgrTelGlobalNumberStruct *pGlobalNumber1,
	EcrioSigMgrTelGlobalNumberStruct *pGlobalNumber2,
	BoolEnum *pbMatchedUri
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;

	if (pGlobalNumber1 == NULL || pGlobalNumber2 == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto EndTag;
	}

	/* Compare Global No Digits*/
	error = _EcrioSigMgrCompareTelUriNumber
			(pGlobalNumber1->pGlobalNumberDigits, pGlobalNumber2->pGlobalNumberDigits);

	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		goto EndTag;
	}

	error = _EcrioSigMgrCompareTelURIParams(pGlobalNumber1->numTelPar, pGlobalNumber1->ppTelPar, pGlobalNumber2->numTelPar, pGlobalNumber2->ppTelPar, pbMatchedUri);
	if (*pbMatchedUri != Enum_TRUE)
	{
		goto EndTag;
	}

	*pbMatchedUri = Enum_TRUE;

EndTag:
	return error;
}

/*********************************************************************************
Function : _EcrioSigMgrCompareTelUriNumber

Purpose :  To compare two Tel URI number.

Header:    EcrioSigMgrCommon.h


Description :
This function is used to compare Tel URI Number. And indicates if
any difference observed.

Input and Output Parameters :

Input :
u_char									*pNumber1,
u_char									*pNumber2

Return value :
u_int32								error code
********************************************************************************/
u_int32 _EcrioSigMgrCompareTelUriNumber
(
	u_char *pNumber1,
	u_char *pNumber2
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	u_char *pTempNumber1 = NULL;
	u_char *pTempNumber2 = NULL;

	if (pNumber1 == NULL || pNumber2 == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto EndTag;
	}

	error = _EcrioSigMgrRemoveVisualSeparatorTelURI(pNumber1, &pTempNumber1);

	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		goto EndTag;
	}

	error = _EcrioSigMgrRemoveVisualSeparatorTelURI(pNumber2, &pTempNumber2);

	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		goto EndTag;
	}

	if (pal_StringICompare(pTempNumber1, pTempNumber2) != 0)
	{
		error = ECRIO_SIGMGR_INVALID_DATA;
	}

EndTag:

	if (pTempNumber1)
	{
		pal_MemoryFree((void **)&pTempNumber1);
	}

	if (pTempNumber2)
	{
		pal_MemoryFree((void **)&pTempNumber2);
	}

	return error;
}

/***************************************************

Function : _EcrioSigMgrRemoveVisualSeparatorTelURI

Purpose :  To remove visual separator from Tel Uri number.

Header:   EcrioSigMgrCommon.h


Description :
This function is used to Remove visual separator from tel URI.

Input and Output Parameters :

Input :
u_char												*pInStr,
u_char												**ppOutStr

Return value :
u_int32								error code

************************************************************************************/
u_int32 _EcrioSigMgrRemoveVisualSeparatorTelURI
(
	u_char *pInStr,
	u_char **ppOutStr
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;

	u_char *pEnd = NULL;
	u_char *pCurrentPos = NULL;
	u_char *pOutStr = NULL;

	u_int32	uLoopCount = 0;
	u_int32	index = 0;
	u_int32	uLength = 0;

	// u_int32	outStrLen = 0;

	if (!pInStr || !ppOutStr)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto EndTag;
	}

	/* Remove leading and trailing white space */
	pEnd = pInStr + (pal_StringLength(pInStr) - 1);
	pCurrentPos = _EcrioSigMgrStripLeadingLWS(pInStr, pEnd);
	pEnd = _EcrioSigMgrStripTrailingLWS(pInStr, pEnd);

	/* Get actual length */
	uLength = pEnd - pCurrentPos;

	pOutStr = pal_StringCreate(pCurrentPos, uLength + 1);

	if (!pOutStr)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto EndTag;
	}

	// outStrLen = uLength + 1;

	while (*pCurrentPos && (uLoopCount < uLength))
	{
		uLoopCount++;

		if (*pCurrentPos == '-' || *pCurrentPos == '.' || *pCurrentPos == '(' ||
			*pCurrentPos == ')')
		{
		}
		else
		{
			pOutStr[index++] = *(pCurrentPos);
		}

		pCurrentPos++;
	}

	pOutStr[index] = '\0';

	*ppOutStr = pOutStr;
	pOutStr = NULL;

EndTag:

	return error;
}

/*********************************************************************************
Function : _EcrioSigMgrCompareLocalTelURI

Purpose :  To compare two Local Tel URI.

Header:    EcrioSigMgrCommon.h


Description :
This function is used to compare two Local Tel URI. And indicates if
any difference observed.

Input and Output Parameters :

Input :
EcrioSigMgrTelLocalNumberStruct		*pLocalNumber1
EcrioSigMgrTelLocalNumberStruct		*pLocalNumber2
BoolEnum							*pbMatchedUri

Return value :
u_int32								error code
********************************************************************************/
u_int32 _EcrioSigMgrCompareLocalTelURI
(
	EcrioSigMgrTelLocalNumberStruct *pLocalNumber1,
	EcrioSigMgrTelLocalNumberStruct *pLocalNumber2,
	BoolEnum *pbMatchedUri
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;

	if (pLocalNumber1 == NULL || pLocalNumber2 == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto EndTag;
	}

	/* Compare Local No Digits*/
	error = _EcrioSigMgrCompareTelUriNumber
			(pLocalNumber1->pLocalNumberDigits, pLocalNumber2->pLocalNumberDigits);

	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		goto EndTag;
	}

	error = _EcrioSigMgrCompareTelURIParams(pLocalNumber1->numTelPar1, pLocalNumber1->ppTelPar1, pLocalNumber1->numTelPar1, pLocalNumber1->ppTelPar1, pbMatchedUri);
	if (*pbMatchedUri != Enum_TRUE)
	{
		goto EndTag;
	}

	error = _EcrioSigMgrCompareTelURIParams(pLocalNumber1->numTelPar2, pLocalNumber1->ppTelPar2, pLocalNumber1->numTelPar2, pLocalNumber1->ppTelPar2, pbMatchedUri);
	if (*pbMatchedUri != Enum_TRUE)
	{
		goto EndTag;
	}

	/* Check the context */
	error = _EcrioSigMgrCompareTelUriContext(pLocalNumber1->pContext, pLocalNumber2->pContext);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		goto EndTag;
	}

	*pbMatchedUri = Enum_TRUE;

EndTag:

	return error;
}

/*********************************************************************************
Function : _EcrioSigMgrCompareTelUriContext

Purpose :  To compare two Local Tel URI context part.

Header:    EcrioIMSLibSignalingSIPCommon.h


Description :
This function is used to compare two Tel URI context part. And indicates if
any difference observed.

Input and Output Parameters :

Input :
EcrioSigMgrTelContextStruct		*pContext1,
EcrioSigMgrTelContextStruct		*pContext2

Return value :
u_int32								error code
********************************************************************************/
u_int32 _EcrioSigMgrCompareTelUriContext
(
	EcrioSigMgrTelContextStruct *pContext1,
	EcrioSigMgrTelContextStruct *pContext2
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;

	if (pContext1 == NULL || pContext2 == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto EndTag;
	}

	if (pContext1->contextType != pContext2->contextType)
	{
		error = ECRIO_SIGMGR_INVALID_DATA;
		goto EndTag;
	}

	switch (pContext1->contextType)
	{
		case EcrioSigMgrTelContextDomainName:
		{
			/* Zero for Success */
			if (pContext1->u.pDomainName == NULL || pContext2->u.pDomainName == NULL)
			{
				error = ECRIO_SIGMGR_INVALID_DATA;
				goto EndTag;
			}

			if (pal_StringICompare(pContext1->u.pDomainName, pContext2->u.pDomainName))
			{
				error = ECRIO_SIGMGR_INVALID_DATA;
				goto EndTag;
			}
		}
		break;

		case EcrioSigMgrTelContextGlobalNumDigits:
		{
			/* Compare Global No Digits*/
			error = _EcrioSigMgrCompareTelUriNumber
					(pContext1->u.pGlobalNoDigitsInContext, pContext2->u.pGlobalNoDigitsInContext);
		}
		break;

		default:
			error = ECRIO_SIGMGR_INVALID_DATA;
	}

EndTag:

	return error;
}

/***************************************************

Function : _EcrioSigMgrCompareTelUriParam

Purpose :   to compare two Local Tel URI parameter.

Header:   EcrioSigMgrCommon.h


Description :
This function is used to compare Tel URI's Parameter. And indicates if
any difference observed.

Input and Output Parameters :

Input :
EcrioSigMgrTelParStruct		*pTelUriParam1,
EcrioSigMgrTelParStruct		*pTelUriParam2,
BoolEnum											*pbContinue

Return value :
u_int32								error code

************************************************************************************/
u_int32 _EcrioSigMgrCompareTelUriParam
(
	EcrioSigMgrTelParStruct *pTelUriParam1,
	EcrioSigMgrTelParStruct *pTelUriParam2,
	BoolEnum *pbContinue
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;

	if (pTelUriParam1 == NULL || pTelUriParam2 == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto EndTag;
	}

	if (pTelUriParam1->telParType != pTelUriParam2->telParType)
	{
		*pbContinue = Enum_TRUE;	/* Suggests that parameter may appear in other place within 2nd*/
		goto EndTag;
	}

	switch (pTelUriParam1->telParType)
	{
		case EcrioSigMgrTelParExtension:
		{
			if ((pTelUriParam1->u.pExtension && !pTelUriParam2->u.pExtension) ||
				(!pTelUriParam1->u.pExtension && pTelUriParam2->u.pExtension))
			{
				error = ECRIO_SIGMGR_INVALID_DATA;
				goto EndTag;
			}
			else if (pTelUriParam1->u.pExtension && pTelUriParam2->u.pExtension)
			{
				/* Zero is success */
				if (pal_StringICompare(pTelUriParam1->u.pExtension, pTelUriParam2->u.pExtension))
				{
					error = ECRIO_SIGMGR_INVALID_DATA;
					goto EndTag;
				}
			}
		}
		break;

		case EcrioSigMgrTelParIsdnSubAddress:
		{
			if ((pTelUriParam1->u.pIsdnSubaddress && !pTelUriParam2->u.pIsdnSubaddress) ||
				(!pTelUriParam1->u.pIsdnSubaddress && pTelUriParam2->u.pIsdnSubaddress))
			{
				error = ECRIO_SIGMGR_INVALID_DATA;
				goto EndTag;
			}
			else if (pTelUriParam1->u.pIsdnSubaddress && pTelUriParam2->u.pIsdnSubaddress)
			{
				/* Zero is success */
				if (pal_StringICompare(pTelUriParam1->u.pIsdnSubaddress, pTelUriParam2->u.pIsdnSubaddress))
				{
					error = ECRIO_SIGMGR_INVALID_DATA;
					goto EndTag;
				}
			}
		}
		break;

		case EcrioSigMgrTelParParameter:
		{
			if ((pTelUriParam1->u.pParameter)->pParamName == NULL ||
				(pTelUriParam2->u.pParameter)->pParamName == NULL)
			{
				error = ECRIO_SIGMGR_INVALID_DATA;
				goto EndTag;
			}

			/* Zero is success */
			if (pal_StringICompare((pTelUriParam1->u.pParameter)->pParamName, (pTelUriParam2->u.pParameter)->pParamName))
			{
				*pbContinue = Enum_TRUE;									/* Suggests that parameter may appear in other place within 2nd */
				goto EndTag;
			}

			if (((pTelUriParam1->u.pParameter)->pParamValue && !((pTelUriParam2->u.pParameter)->pParamValue)) ||
				(!((pTelUriParam1->u.pParameter)->pParamValue) && (pTelUriParam2->u.pParameter)->pParamValue))
			{
				error = ECRIO_SIGMGR_INVALID_DATA;
				goto EndTag;
			}
			else if ((pTelUriParam1->u.pParameter)->pParamValue && (pTelUriParam2->u.pParameter)->pParamValue)
			{
				/* Zero is success */
				if (pal_StringICompare((pTelUriParam1->u.pParameter)->pParamValue, (pTelUriParam2->u.pParameter)->pParamValue))
				{
					error = ECRIO_SIGMGR_INVALID_DATA;
					goto EndTag;
				}
			}
		}
		break;

		default:
		{
		}
		break;
	}

EndTag:

	return error;
}


/**************************************************************************

Function:		_EcrioSigMgrValidateIncomingRequests()

Purpose:		Validate an incoming Request as per RFC 3261.

Description:	Validate an incoming Request as per RFC 3261.

Input:			EcrioSigMgrStruct* pSigMgr - SigMgr Instance.
EcrioSigMgrValidationStruct *pValidate 

OutPut:			u_char* pReasonPhrase - Error Reason Phrase [if any], NULL
otherwise
u_int32* pResponseCode - Error response code [if any], else
ECRIO_SIGMGR_INVALID_RESPONSE_CODE if request valid.

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrValidateIncomingRequests
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrValidationStruct *pValidate
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrHeaderStruct *pContentType = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	error = _EcrioSigMgrPopulateCommonInfo(pSigMgr,
		pValidate->pSipMessage->pMandatoryHdrs, &pValidate->cmnInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateCommonInfo() for cmnInfo, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	if (pValidate->pSipMessage->pContentType)
	{
		pContentType = pValidate->pSipMessage->pContentType;
		pValidate->pSipMessage->pContentType = NULL;
	}

	if (pValidate->pSipMessage->pMessageBody &&
		pValidate->pSipMessage->pMessageBody->messageBodyType == EcrioSigMgrMessageBodyUnknown &&
		pValidate->pSipMessage->pMessageBody->pMessageBody)
	{
		if ((pContentType != NULL) && (pContentType->ppHeaderValues != NULL) &&
			(pContentType->ppHeaderValues[0] != NULL))
		{
			BoolEnum bValidContentType = Enum_FALSE;
			EcrioSigMgrUnknownMessageBodyStruct *pUnknownBody =
				(EcrioSigMgrUnknownMessageBodyStruct *)pValidate->pSipMessage->pMessageBody->pMessageBody;

			if (pContentType)
			{
				if (pUnknownBody->contentType.pHeaderValue != NULL)
				{
					_EcrioSigMgrReleaseHdrValueStruct(pSigMgr, &pUnknownBody->contentType);
				}
				pUnknownBody->contentType = *(pContentType->ppHeaderValues[0]);
				pal_MemorySet((void *)pContentType->ppHeaderValues[0], 0, sizeof(EcrioSigMgrHeaderValueStruct));
			}

			if (!(pal_StringCompare((u_char *)pUnknownBody->contentType.pHeaderValue, (u_char *)"application/vnd.3gpp2.sms")))
			{
				bValidContentType = Enum_TRUE;
			}
			else if (!(pal_StringCompare((u_char *)pUnknownBody->contentType.pHeaderValue, (u_char *)"application/vnd.3gpp.sms")))
			{
				bValidContentType = Enum_TRUE;
			}
			else if (!(pal_StringCompare((u_char *)pUnknownBody->contentType.pHeaderValue, (u_char *)"application/reginfo+xml")))
			{
				bValidContentType = Enum_TRUE;
			}
			else if (!(pal_StringCompare((u_char *)pUnknownBody->contentType.pHeaderValue, (u_char *)"application/sdp")))
			{
				bValidContentType = Enum_TRUE;
			}
			else if (!(pal_StringCompare((u_char *)pUnknownBody->contentType.pHeaderValue, (u_char *)"message/cpim")))
			{
				bValidContentType = Enum_TRUE;
			}
			else if (!(pal_StringCompare((u_char *)pUnknownBody->contentType.pHeaderValue, (u_char *)"application/vnd.oma.cpm-groupdata+xml")))
			{
				bValidContentType = Enum_TRUE;
			}
			else if (!(pal_StringCompare((u_char *)pUnknownBody->contentType.pHeaderValue, (u_char *)"application/vnd.gsma.rcsalias-mgmt+xml")))
			{
				bValidContentType = Enum_TRUE;
			}
			else if (!(pal_StringCompare((u_char *)pUnknownBody->contentType.pHeaderValue, (u_char *)"application/conference-info+xml")))
			{
				bValidContentType = Enum_TRUE;
			}
			else if (!(pal_StringCompare((u_char *)pUnknownBody->contentType.pHeaderValue, (u_char *)"application/resource-lists+xml")))
			{
				bValidContentType = Enum_TRUE;
			}
			else if (!(pal_StringCompare((u_char *)pUnknownBody->contentType.pHeaderValue, (u_char *)"application/pidf+xml")))
			{
				bValidContentType = Enum_TRUE;
			}
			else if (!(pal_StringCompare((u_char *)pUnknownBody->contentType.pHeaderValue, (u_char *)"multipart/mixed")))
			{
				bValidContentType = Enum_TRUE;
			}
			else if (!(pal_StringCompare((u_char *)pUnknownBody->contentType.pHeaderValue, (u_char *)"multipart/related")))
			{
				bValidContentType = Enum_TRUE;
			}

			if (!bValidContentType)
			{
				// pReasonPhrase[0] = (u_char*)ECRIO_SIGMGR_RESPONSE_UNSUPPORTED_MEDIA_TYPE;
				if (NULL == pal_StringNCopy(pValidate->reasonPhrase, ECRIO_SIGMGR_BUFFER_SIZE_64, (u_char *)ECRIO_SIGMGR_RESPONSE_UNSUPPORTED_MEDIA_TYPE, pal_StringLength((const u_char *)ECRIO_SIGMGR_RESPONSE_UNSUPPORTED_MEDIA_TYPE)))
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tString Copy error",
						__FUNCTION__, __LINE__);
					error = ECRIO_SIGMGR_STRING_ERROR;

					goto Error_Level_01;
				}
				pValidate->responseCode = ECRIO_SIGMGR_RESPONSE_CODE_UNSUPPORTED_MEDIA_TYPE;
				pValidate->pSipMessage->isRequestInValid = Enum_TRUE;
				goto Error_Level_01;
			}
		}
	}
	else
	{
		if (pValidate->pSipMessage->eMethodType == EcrioSipMessageTypeMessage)
		{
			if (NULL == pal_StringNCopy(pValidate->reasonPhrase, ECRIO_SIGMGR_BUFFER_SIZE_64, (u_char *)ECRIO_SIGMGR_RESPONSE_BAD_REQUEST, pal_StringLength((const u_char *)ECRIO_SIGMGR_RESPONSE_BAD_REQUEST)))
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tString Copy error",
					__FUNCTION__, __LINE__);
				error = ECRIO_SIGMGR_STRING_ERROR;

				goto Error_Level_01;
			}
			pValidate->responseCode = ECRIO_SIGMGR_RESPONSE_CODE_BAD_REQUEST;
			pValidate->pSipMessage->isRequestInValid = Enum_TRUE;
			goto Error_Level_01;
		}	
	}

	if (pValidate->pSipMessage->pMandatoryHdrs->ppVia)
	{
		/* Checking MAGIC COOKIE */
		if (pal_StringNCompare(pValidate->pSipMessage->pMandatoryHdrs->ppVia[0]->pBranch,
			(u_char *)ECRIO_SIG_MGR_BRANCH_INITIAT_STR, 7) != 0)
		{
			SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tECRIO_SIGMGR_RESPONSE_CODE_BAD_REQUEST", __FUNCTION__, __LINE__);

			/* Setting Response Code to 400 Bad Request */
			pValidate->responseCode = ECRIO_SIGMGR_RESPONSE_CODE_BAD_REQUEST;
			if (NULL == pal_StringNCopy(pValidate->reasonPhrase, ECRIO_SIGMGR_BUFFER_SIZE_64, (u_char *)ECRIO_SIGMGR_RESPONSE_BAD_REQUEST, pal_StringLength((const u_char *)ECRIO_SIGMGR_RESPONSE_BAD_REQUEST)))
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tString Copy error",
					__FUNCTION__, __LINE__);
				error = ECRIO_SIGMGR_STRING_ERROR;

				goto Error_Level_01;
			}
			pValidate->pSipMessage->isRequestInValid = Enum_TRUE;
			goto Error_Level_01;
		}
	}

	//Invalid checking - You can receive domain as IP:Port */
	//if (pValidate->pSipMessage->pMandatoryHdrs->pTo)
	//{
	//	EcrioSigMgrNameAddrWithParamsStruct *pTo = pValidate->pSipMessage->pMandatoryHdrs->pTo;
	//	EcrioSigMgrNameAddrWithParamsStruct *pPAssociatedURI = NULL;
	//	u_int32 iterator = 0;

	//	for (iterator = 0; iterator < pSigMgr->pSignalingInfo->numPAssociatedURIs; iterator++)
	//	{
	//		pPAssociatedURI = pSigMgr->pSignalingInfo->ppPAssociatedURI[iterator];

	//		_EcrioSigMgrCompareUris(pSigMgr,
	//			&pTo->nameAddr.addrSpec, &pPAssociatedURI->nameAddr.addrSpec, &result);
	//		if (result == Enum_TRUE)
	//		{
	//			break;
	//		}
	//	}

	//	if (result == Enum_FALSE)
	//	{
	//		/* Setting Response Code to 403  */
	//		pValidate->responseCode = ECRIO_SIGMGR_RESPONSE_CODE_FORBIDDEN;
	//		pal_StringCopy(pValidate->reasonPhrase, (u_char *)ECRIO_SIGMGR_RESPONSE_FORBIDDEN);
	//		pValidate->pSipMessage->isRequestInValid = Enum_TRUE;
	//		goto Error_Level_01;
	//	}
	//}

	if (pValidate->pSipMessage->pMandatoryHdrs->pRequestUri)
	{
		/*EcrioSigMgrUriStruct *pRequestUri = pSipMessage->pMandatoryHdrs->pRequestUri;
		EcrioSigMgrNameAddrWithParamsStruct* pPAssociatedURI = NULL;
		u_int32 iterator = 0;*/

		if (pValidate->pSipMessage->pMandatoryHdrs->pRequestUri->uriScheme != EcrioSigMgrURISchemeSIP &&
			pValidate->pSipMessage->pMandatoryHdrs->pRequestUri->uriScheme != EcrioSigMgrURISchemeTEL)
		{
			SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tECRIO_SIGMGR_RESPONSE_UNSUPPORTED_URI_SCHEME", __FUNCTION__, __LINE__);

			pValidate->responseCode = ECRIO_SIGMGR_RESPONSE_CODE_UNSUPPORTED_URI_SCHEME;
			if (NULL == pal_StringNCopy(pValidate->reasonPhrase, ECRIO_SIGMGR_BUFFER_SIZE_64, (u_char *)ECRIO_SIGMGR_RESPONSE_UNSUPPORTED_URI_SCHEME, pal_StringLength((const u_char *)ECRIO_SIGMGR_RESPONSE_UNSUPPORTED_URI_SCHEME)))
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tString Copy error",
					__FUNCTION__, __LINE__);
				error = ECRIO_SIGMGR_STRING_ERROR;

				goto Error_Level_01;
			}
			pValidate->pSipMessage->isRequestInValid = Enum_TRUE;
			goto Error_Level_01;
		}
	}

	pValidate->responseCode = ECRIO_SIGMGR_INVALID_RESPONSE_CODE;

Error_Level_01:

	if (pContentType != NULL)
	{
		EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_Header,
			(void **)&pContentType, Enum_TRUE);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}


/**************************************************************************

Function:		_EcrioSigMgrCheckTLSTransport()

Purpose:		function to check TLS transport in received TLS message.

Description:	TLS transport parameter checking in TLS messsaage.

Input:			EcrioSigMgrStruct* pSigMgr - SigMgrI nstance.
EcrioSigMgrSipMessageStruct *pReceivedData -
parsed IMS data buffer.

OutPut:			None

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrCheckTLSTransport
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessage
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrMandatoryHeaderStruct *pMandatoryHdrs = NULL;
	EcrioSigMgrViaStruct *pVia = NULL;
	EcrioSigMgrTransportEnum uTransport;

	if (pSigMgr == NULL)
		return ECRIO_SIGMGR_INVALID_DATA;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pSipMessage == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tpSipMessage is NULL", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INVALID_DATA;
		goto Error_Level_01;
	}

	if (!((pSigMgr->pSignalingInfo->bEnableTls == Enum_TRUE) && (pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex != 0)))
	{
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tReturning here as TLS is not enabled", __FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	if (pSipMessage != NULL)
	{
		pMandatoryHdrs = pSipMessage->pMandatoryHdrs;
	}
	else
	{
		error = ECRIO_SIGMGR_INVALID_DATA;
		goto Error_Level_01;
	}

	if (pMandatoryHdrs != NULL)
	{
		if (pMandatoryHdrs->ppVia != NULL)
		{
			pVia = pMandatoryHdrs->ppVia[0];
		}
	}
	else
	{
		error = ECRIO_SIGMGR_INVALID_DATA;
		goto Error_Level_01;
	}

	if (pVia != NULL)
	{
		uTransport = pVia->transport;
	}
	else
	{
		error = ECRIO_SIGMGR_INVALID_DATA;
		goto Error_Level_01;
	}

	if (uTransport != EcrioSigMgrTransportTLS)
	{
		error = ECRIO_SIGMGR_INVALID_DATA;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tDiscarding the received non-TLS message!", __FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u", __FUNCTION__, __LINE__, error);

	return error;
}
