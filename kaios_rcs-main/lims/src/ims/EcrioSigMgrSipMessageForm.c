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
#include "EcrioDSListInterface.h"

#define STRCAT(src, dest)									_EcrioSigMgrStringCreate(pSigMgr, src, dest)
#define STRCAT_BRACKET_OPEN(dest)							_EcrioSigMgrStringCreate(pSigMgr, (u_char *)"[", dest)
#define STRCAT_BRACKET_CLOSE(dest)							_EcrioSigMgrStringCreate(pSigMgr, (u_char *)"]", dest)
#define STRCAT_COLON(dest)									_EcrioSigMgrStringCreate(pSigMgr, (u_char *)":", dest)
#define STRCAT_SPACE(dest)									_EcrioSigMgrStringCreate(pSigMgr, (u_char *)" ", dest)
#define STRCAT_CRLF(dest)									_EcrioSigMgrStringCreate(pSigMgr, (u_char *)"\r\n", dest)
#define STRCAT_HEADERNAME(fullName, shortName, ppSipBuf)	_EcrioSigMgrFormHeaderName(pSigMgr, mode, fullName, shortName, ppSipBuf)

u_char *MethodTypeToString
(
	EcrioSipMessageTypeEnum eMethodType
)
{
	switch (eMethodType)
	{
		case EcrioSipMessageTypeRegister:
		{
			return (u_char *)"REGISTER";
		}

		case EcrioSipMessageTypeInvite:
		{
			return (u_char *)"INVITE";
		}

		case EcrioSipMessageTypeAck:
		{
			return (u_char *)"ACK";
		}

		case EcrioSipMessageTypeBye:
		{
			return (u_char *)"BYE";
		}

		case EcrioSipMessageTypeCancel:
		{
			return (u_char *)"CANCEL";
		}

		case EcrioSipMessageTypeMessage:
		{
			return (u_char *)"MESSAGE";
		}

		case EcrioSipMessageTypePrack:
		{
			return (u_char *)"PRACK";
		}

		case EcrioSipMessageTypeSubscribe:
		{
			return (u_char *)"SUBSCRIBE";
		}

		case EcrioSipMessageTypeNotify:
		{
			return (u_char *)"NOTIFY";
		}

		case EcrioSipMessageTypeUpdate:
		{
			return (u_char *)"UPDATE";
		}
		case EcrioSipMessageTypePublish:
		{
			return (u_char *)"PUBLISH";
		}
		case EcrioSipMessageTypeOptions:
		{
			return (u_char *)"OPTIONS";
		}
		case EcrioSipMessageTypeRefer:
		{
			return (u_char *)"REFER";
		}
		case EcrioSipMessageTypeInfo:
		{
			return (u_char *)"INFO";
		}
		default:
			return NULL;
	}
}

u_int32 _EcrioSigMgrFormHeaderName
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageModeEnum mode,
	u_char *pFullName,
	u_char *pShortName,
	u_char **ppSipBuf
)
{
	if (mode == EcrioSigMgrSipMessageModeFullName)
	{
		STRCAT(pFullName, ppSipBuf);
	}
	else if (mode == EcrioSigMgrSipMessageModeShortName)
	{
		STRCAT(pShortName, ppSipBuf);
	}

	return 0;
}

u_int32 _EcrioSigMgrFormParam
(
	EcrioSigMgrStruct *pSigMgr,
	u_int16 numParams,
	EcrioSigMgrParamStruct **ppParams,
	u_char *pSeparator,
	u_char *pLeadingSeptr,
	u_char **ppUri
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int16 paramIter = 0;
	EcrioSigMgrParamStruct *pURIParams = NULL;

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry,
// quiet		"%s:%u", __FUNCTION__, __LINE__);

	for (paramIter = 0; paramIter < numParams; paramIter++)
	{
		pURIParams = ppParams[paramIter];

		if (pURIParams->pParamName)
		{
			if (pLeadingSeptr)
			{
				STRCAT(pLeadingSeptr, ppUri);
				pLeadingSeptr = NULL;
			}
			else
			{
				STRCAT(pSeparator, ppUri);
			}

			STRCAT(pURIParams->pParamName, ppUri);
		}

		if (pURIParams->pParamValue)
		{
			STRCAT((u_char *)"=", ppUri);
			STRCAT(pURIParams->pParamValue, ppUri);
		}
	}

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrFormTelParams

Purpose:		To generate a NULL terminated URI string from SigMgr URI struct.

Description:	API will make use of IMS lib utility API to generate a NULL
terminated URI string from SigMgr URI struct.

Input:			EcrioSigMgrStruct* pSigMgr - signaling manager instance
u_int16	numTelParams - Tel Param count
EcrioSigMgrTelParStruct** ppTelParams - Tel Param array

Output:			u_char** ppUri - NULL terminated Uri String

Returns:		Error Code

**************************************************************************/
u_int32 _EcrioSigMgrFormTelParams
(
	EcrioSigMgrStruct *pSigMgr,
	u_int16	numTelParams,
	EcrioSigMgrTelParStruct **ppTelParams,
	u_char **ppUri
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 i = 0;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry,
		"%s:%u", __FUNCTION__, __LINE__);

	for (i = 0; i < numTelParams; i++)
	{
		if (ppTelParams[i])
		{
			if (ppTelParams[i]->telParType == EcrioSigMgrTelParParameter)
			{
				error = _EcrioSigMgrFormParam(pSigMgr, 1,
					&ppTelParams[i]->u.pParameter, (u_char *)";", NULL, ppUri);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					goto Error_Level_01;
				}
			}
			else if (ppTelParams[i]->telParType == EcrioSigMgrTelParExtension)
			{
				STRCAT((u_char *)";ext=", ppUri);
				if (ppTelParams[i]->u.pExtension)
				{
					STRCAT(ppTelParams[i]->u.pExtension, ppUri);
				}
			}
			else if (ppTelParams[i]->telParType == EcrioSigMgrTelParIsdnSubAddress)
			{
				STRCAT((u_char *)";isub=", ppUri);
				if (ppTelParams[i]->u.pIsdnSubaddress)
				{
					STRCAT(ppTelParams[i]->u.pIsdnSubaddress, ppUri);
				}
			}
		}
	}

	error = ECRIO_SIGMGR_NO_ERROR;

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit,
		"%s:%u\terror=%u", __FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrFormTelUri

Purpose:		To generate a NULL terminated URI string from SigMgr URI struct.

Description:	API will make use of IMS lib utility API to generate a NULL
terminated URI string from SigMgr URI struct.

Input:			EcrioSigMgrStruct* pSigMgr - signaling manager instance
EcrioSigMgrTelURIStruct* pTelUri - Tel Uri struct

Output:			u_char** ppUri - NULL terminated Uri String

Returns:		Error Code

**************************************************************************/
u_int32 _EcrioSigMgrFormTelUri
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrTelURIStruct *pTelUri,
	u_char **ppUri
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry,
		"%s:%u", __FUNCTION__, __LINE__);

	STRCAT((u_char *)"tel:", ppUri);
	if (pTelUri->subscriberType == EcrioSigMgrTelSubscriberGlobal)
	{
		if (pTelUri->u.pGlobalNumber)
		{
			if (pTelUri->u.pGlobalNumber->pGlobalNumberDigits)
			{
				STRCAT((u_char *)"+", ppUri);
				STRCAT(pTelUri->u.pGlobalNumber->pGlobalNumberDigits, ppUri);
			}

			error = _EcrioSigMgrFormTelParams(pSigMgr, pTelUri->u.pGlobalNumber->numTelPar, pTelUri->u.pGlobalNumber->ppTelPar, ppUri);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error_Level_01;
			}
		}
	}
	else if (pTelUri->subscriberType == EcrioSigMgrTelSubscriberLocal)
	{
		if (pTelUri->u.pLocalNumber)
		{
			if (pTelUri->u.pLocalNumber->pLocalNumberDigits)
			{
				STRCAT(pTelUri->u.pLocalNumber->pLocalNumberDigits, ppUri);
			}

			error = _EcrioSigMgrFormTelParams(pSigMgr, pTelUri->u.pLocalNumber->numTelPar1, pTelUri->u.pLocalNumber->ppTelPar1, ppUri);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error_Level_01;
			}

			if (pTelUri->u.pLocalNumber->pContext)
			{
				STRCAT((u_char *)";phone-context=", ppUri);
				if (pTelUri->u.pLocalNumber->pContext->contextType == EcrioSigMgrTelContextDomainName)
				{
					STRCAT(pTelUri->u.pLocalNumber->pContext->u.pDomainName, ppUri);
				}
				else if (pTelUri->u.pLocalNumber->pContext->contextType == EcrioSigMgrTelContextGlobalNumDigits)
				{
					STRCAT(pTelUri->u.pLocalNumber->pContext->u.pGlobalNoDigitsInContext, ppUri);
				}
			}

			error = _EcrioSigMgrFormTelParams(pSigMgr, pTelUri->u.pLocalNumber->numTelPar2, pTelUri->u.pLocalNumber->ppTelPar2, ppUri);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error_Level_01;
			}
		}
	}

	error = ECRIO_SIGMGR_NO_ERROR;

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit,
		"%s:%u\terror=%u", __FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrFormSipUri

Purpose:		To generate a NULL terminated URI string from SigMgr URI struct.

Description:	API will make use of IMS lib utility API to generate a NULL
terminated URI string from SigMgr URI struct.

Input:			EcrioSigMgrStruct* pSigMgr - signaling manager instance
EcrioSigMgrSipURIStruct* pSipUri - Sip Uri struct

Output:			u_char** ppUri - NULL terminated Uri String

Returns:		Error Code

**************************************************************************/
u_int32 _EcrioSigMgrFormSipUri
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipURIStruct *pSipUri,
	u_char **ppUri
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry,
// quiet		"%s:%u", __FUNCTION__, __LINE__);

	STRCAT((u_char *)"sip:", ppUri);

	if (pSipUri->pUserId)
	{
		STRCAT(pSipUri->pUserId, ppUri);
	}

	if (pSipUri->pPassword)
	{
		STRCAT_COLON(ppUri);
		STRCAT(pSipUri->pPassword, ppUri);
	}

	if (pSipUri->pUserId)
	{
		STRCAT((u_char *)"@", ppUri);
	}

	if (pSipUri->pDomain)
	{
		if (pal_StringFindSubString((const u_char *)pSipUri->pDomain, (const u_char *)":") != NULL)
		{
			// Put brackets around IPv6 IP.
			STRCAT_BRACKET_OPEN(ppUri);
			STRCAT(pSipUri->pDomain, ppUri);
			STRCAT_BRACKET_CLOSE(ppUri);
		}
		else
		{
			STRCAT(pSipUri->pDomain, ppUri);
		}
	}

	if (pSipUri->pIPAddr)
	{
		if (pal_StringFindSubString((const u_char *)pSipUri->pIPAddr->pIPAddr, (const u_char *)":") != NULL)
		{
			// Put brackets around IPv6 IP.
			STRCAT_BRACKET_OPEN(ppUri);
			STRCAT(pSipUri->pIPAddr->pIPAddr, ppUri);
			STRCAT_BRACKET_CLOSE(ppUri);
		}
		else
		{
			STRCAT(pSipUri->pIPAddr->pIPAddr, ppUri);
		}

		if (pSipUri->pIPAddr->port > 0)
		{
			char porttext[6] = { 0 };
			STRCAT_COLON(ppUri);
			if (0 >= pal_SStringPrintf(porttext, 6, "%u", pSipUri->pIPAddr->port))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tString Copy error",
					__FUNCTION__, __LINE__);

				goto Error_Level_01;
			}
			porttext[5] = '\0';
			STRCAT((u_char *)porttext, ppUri);
		}
	}

	if (pSipUri->ppURIParams)
	{
		// STRCAT((u_char*)";", ppUri);
		error = _EcrioSigMgrFormParam(pSigMgr, pSipUri->numURIParams,
			pSipUri->ppURIParams, (u_char *)";", NULL, ppUri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrStringCreate() for ppUri, error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}
	}

	if (pSipUri->ppURIHeaders)
	{
		// STRCAT((u_char*)"?", ppUri);
		error = _EcrioSigMgrFormParam(pSigMgr, pSipUri->numURIHeaders,
			pSipUri->ppURIHeaders, (u_char *)"&", (u_char *)"?", ppUri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrStringCreate() for ppUri, error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}
	}

	error = ECRIO_SIGMGR_NO_ERROR;

Error_Level_01:

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit,
// quiet		"%s:%u\terror=%u", __FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrFormUri

Purpose:		To generate a NULL terminated URI string from SigMgr URI struct.

Description:	API will make use of IMS lib utility API to generate a NULL
terminated URI string from SigMgr URI struct.

Input:			EcrioSigMgrStruct* pSigMgr - signaling manager instance
EcrioSigMgrUriStruct* pUriStruct - Uri struct

Output:			u_char** ppUri - NULL terminated Uri String

Returns:		Error Code

**************************************************************************/
u_int32 _EcrioSigMgrFormUri
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUriStruct *pUriStruct,
	u_char **ppUri
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry,
// quiet		"%s:%u", __FUNCTION__, __LINE__);

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
// quiet		"%s:%u\tpSigMgr=%p, pUriStruct=%p",
// quiet		__FUNCTION__, __LINE__, pSigMgr, pUriStruct);

	if (pUriStruct->uriScheme == EcrioSigMgrURISchemeSIP)
	{
		error = _EcrioSigMgrFormSipUri(pSigMgr, pUriStruct->u.pSipUri, ppUri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t_EcrioSigMgrFormSipUri() for ppUri, error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}
	}
	else if (pUriStruct->uriScheme == EcrioSigMgrURISchemeTEL)
	{
		error = _EcrioSigMgrFormTelUri(pSigMgr, pUriStruct->u.pTelUri, ppUri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t_EcrioSigMgrFormTelUri() for ppUri, error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}
	}
	else if (pUriStruct->uriScheme == EcrioSigMgrURISchemeCustom)
	{
		if (pUriStruct->u.pAbsUri == NULL)
		{
			error = ECRIO_SIGMGR_INSUFFICIENT_DATA;

			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpAbsUri is NULL", __FUNCTION__, __LINE__);

			goto Error_Level_01;
		}

		error = _EcrioSigMgrStringCreate(pSigMgr, pUriStruct->u.pAbsUri, ppUri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrStringCreate() for ppUri, error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}
	}
	else
	{
		error = ECRIO_SIGMGR_INVALID_URI_SCHEME;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tURI scheme is INVALID", __FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	error = ECRIO_SIGMGR_NO_ERROR;

Error_Level_01:

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit,
// quiet		"%s:%u\terror=%u", __FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrFormNameAddrWithParams

Purpose:		To generate a NULL terminated URI string from SigMgr NameAddrParam struct.

Description:	API will make use of IMS lib utility API to generate a NULL
terminated URI string from SigMgr URI struct.

Input:			EcrioSigMgrStruct* pSigMgr - signaling manager instance
                EcrioSigMgrNameAddrWithParamsStruct * pAddrWithParamsStruct

Output:			u_char** ppUri - NULL terminated Uri String

Returns:		Error Code

**************************************************************************/
u_int32 _EcrioSigMgrFormNameAddrWithParams
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrNameAddrWithParamsStruct *pAddrWithParamsStruct,
	u_char **ppUri
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrNameAddrStruct *pNameAddr = NULL;

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry,
// quiet		"%s:%u", __FUNCTION__, __LINE__);

	pNameAddr = &pAddrWithParamsStruct->nameAddr;

	if (pNameAddr->pDisplayName)
	{
		STRCAT(pNameAddr->pDisplayName, ppUri);
		STRCAT_SPACE(ppUri);
	}

	STRCAT((u_char *)"<", ppUri);
	error = _EcrioSigMgrFormUri(pSigMgr, &pNameAddr->addrSpec, ppUri);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		error = ECRIO_SIGMGR_UNKNOWN_ERROR;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrFormUri() error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	STRCAT((u_char *)">", ppUri);

	if (pAddrWithParamsStruct->ppParams)
	{
		error = _EcrioSigMgrFormParam(pSigMgr, pAddrWithParamsStruct->numParams,
			pAddrWithParamsStruct->ppParams, (u_char *)";", NULL, ppUri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			error = ECRIO_SIGMGR_UNKNOWN_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrFormUri() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}
	}

Error_Level_01:

	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		pal_MemoryFree((void **)ppUri);
		*ppUri = NULL;
	}

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit,
// quiet		"%s:%u\terror=%u", __FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrAddRunTimeContact

Purpose:		To generate a NULL terminated URI string from SigMgr NameAddrParam struct.

Description:	API will make use of IMS lib utility API to generate a NULL
terminated URI string from SigMgr URI struct.

Input:			EcrioSigMgrStruct* pSigMgr - signaling manager instance
EcrioSigMgrNameAddrWithParamsStruct * pAddrWithParamsStruct

Output:			u_char** ppUri - NULL terminated Uri String

Returns:		Error Code

**************************************************************************/
u_int32 _EcrioSigMgrAddRunTimeContact
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessage,
	EcrioSigMgrNameAddrWithParamsStruct *pAddrWithParamsStruct,
	u_char **ppUri
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrNameAddrStruct *pNameAddr = NULL;
	EcrioSigMgrParamStruct *pParamStruct = NULL;
	u_char *pContact = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry,
		"%s:%u", __FUNCTION__, __LINE__);

	pNameAddr = &pAddrWithParamsStruct->nameAddr;

	if (pNameAddr->pDisplayName)
	{
		STRCAT(pNameAddr->pDisplayName, ppUri);
		STRCAT_SPACE(ppUri);
	}

	STRCAT((u_char *)"<", ppUri);
	error = _EcrioSigMgrFormUri(pSigMgr, &pNameAddr->addrSpec, ppUri);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		error = ECRIO_SIGMGR_UNKNOWN_ERROR;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrFormUri() error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	STRCAT((u_char *)">", ppUri);

	if (pSipMessage->pFetaureTags)
	{
		/* This indicates overwritting of contact at run time */
		pContact = _EcrioSigMgrGetFeatureTagValue(pSipMessage->pFetaureTags, EcrioSipHeaderTypeContact);
		if (pContact)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&pParamStruct);
			if (pParamStruct == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			pal_MemorySet(pParamStruct, 0, sizeof(EcrioSigMgrParamStruct));
			pParamStruct->pParamName = pal_StringCreate(pContact, pal_StringLength(pContact));
			error = _EcrioSigMgrFormParam(pSigMgr, 1, &pParamStruct, (u_char *)";", NULL, ppUri);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				error = ECRIO_SIGMGR_UNKNOWN_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrFormUri() error=%u",
					__FUNCTION__, __LINE__, error);

				goto Error_Level_01;
			}
		}
		else
		{
			// Do nothing
		}
	}

Error_Level_01:

	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		pal_MemoryFree((void **)ppUri);
		*ppUri = NULL;
	}

	if (pParamStruct != NULL)
	{
		if (pParamStruct->pParamName != NULL)
		{
			pal_MemoryFree((void **)&pParamStruct->pParamName);
		}

		pal_MemoryFree((void **)&pParamStruct);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit,
		"%s:%u\terror=%u", __FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrFormHeaderStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrHeaderStruct *pHeader,
	u_char **ppSipBuf
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR, hdrIter = 0;
	EcrioSigMgrHeaderValueStruct *pHeaderValues = NULL;

	if (pHeader->ppHeaderValues)
	{
		u_char *pPtr = NULL;
		u_int32 len;

		for (hdrIter = 0; hdrIter < pHeader->numHeaderValues; hdrIter++)
		{
			pHeaderValues = pHeader->ppHeaderValues[hdrIter];

			if (hdrIter > 0)
			{
				STRCAT((u_char *)",", ppSipBuf);
			}

			STRCAT(pHeaderValues->pHeaderValue, ppSipBuf);

			if (pHeaderValues->ppParams)
			{
				error = _EcrioSigMgrFormParam(pSigMgr, pHeaderValues->numParams,
					pHeaderValues->ppParams, (u_char *)";", NULL, ppSipBuf);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					error = ECRIO_SIGMGR_UNKNOWN_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrFormUri() error=%u",
						__FUNCTION__, __LINE__, error);

					goto Error_Level_01;
				}
			}
		}

		pPtr = *ppSipBuf;
		len = pal_StringLength(pPtr);
		if (pPtr[len - 1] == ',')
		{
			pPtr[len - 1] = '\0';
		}

		STRCAT_CRLF(ppSipBuf);
	}

Error_Level_01:

	return error;
}

u_int32 _EcrioSigMgrFormMsgBody
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageModeEnum mode,
	EcrioSigMgrMessageBodyStruct *pMessageBody,
	u_char **ppSipBuf,
	u_int32 *pSipBufLen
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;

	if (pMessageBody->messageBodyType == EcrioSigMgrMessageBodyUnknown ||
		pMessageBody->messageBodyType == EcrioSigMgrMessageBodySDP)
	{
		char textexpires[6] = "";
		u_int32 tempLen = 0;
		EcrioSigMgrHeaderValueStruct *pContentType = NULL;
		EcrioSigMgrUnknownMessageBodyStruct *pUnknwnMsgBdy = (EcrioSigMgrUnknownMessageBodyStruct *)pMessageBody->pMessageBody;

		pContentType = &pUnknwnMsgBdy->contentType;
		STRCAT_HEADERNAME((u_char *)"Content-Type: ", (u_char *)"c: ", ppSipBuf);
		if (pContentType->pHeaderValue)
		{
			STRCAT((u_char *)pContentType->pHeaderValue, ppSipBuf);
			if (pContentType->ppParams)
			{
				error = _EcrioSigMgrFormParam(pSigMgr, pContentType->numParams,
					pContentType->ppParams, (u_char *)";", NULL, ppSipBuf);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					error = ECRIO_SIGMGR_UNKNOWN_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrFormParam() error=%u",
						__FUNCTION__, __LINE__, error);

					goto Error_Level_01;
				}
			}
		}

		STRCAT_CRLF(ppSipBuf);

		STRCAT_HEADERNAME((u_char *)"Content-Length: ", (u_char *)"l: ", ppSipBuf);
		if (0 >= pal_SNumPrintf(textexpires, 6, "%u", pUnknwnMsgBdy->bufferLength))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tString Cpy error",
				__FUNCTION__, __LINE__);

			goto Error_Level_01;
		}
		textexpires[5] = '\0';
		STRCAT((u_char *)textexpires, ppSipBuf);
		STRCAT_CRLF(ppSipBuf);

		if (pUnknwnMsgBdy->pBuffer)
		{
			u_char *pSipBuf = NULL;
			STRCAT_CRLF(ppSipBuf);
			pSipBuf = *ppSipBuf;

			tempLen = pal_StringLength(pSipBuf);

			/* Check arithmetic overflow */
			if (pal_UtilityArithmeticOverflowDetected(pUnknwnMsgBdy->bufferLength, 1) == Enum_TRUE)
			{
				return ECRIO_SIGMGR_NO_MEMORY;
			}
			if (pal_UtilityArithmeticOverflowDetected(tempLen, (pUnknwnMsgBdy->bufferLength + 1)) == Enum_TRUE)
			{
				return ECRIO_SIGMGR_NO_MEMORY;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((tempLen + pUnknwnMsgBdy->bufferLength + 1), sizeof(u_char)) == Enum_TRUE)
			{
				return ECRIO_SIGMGR_NO_MEMORY;
			}

			pal_MemoryReallocate((tempLen + pUnknwnMsgBdy->bufferLength + 1)*sizeof(u_char), (void **)&pSipBuf);
			if (pSipBuf == NULL)
			{
				return ECRIO_SIGMGR_NO_MEMORY;
			}
			pal_MemoryCopy( pSipBuf + tempLen, pUnknwnMsgBdy->bufferLength, pUnknwnMsgBdy->pBuffer, pUnknwnMsgBdy->bufferLength);

			*pSipBufLen = tempLen + pUnknwnMsgBdy->bufferLength;
			pSipBuf[*pSipBufLen] = 0;
			*ppSipBuf = pSipBuf;
		}
	}
	else
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

Error_Level_01:

	return error;
}

u_int32 _EcrioSigMgrFormMandatoryHdrs
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageModeEnum mode,
	EcrioSigMgrSipMessageStruct *pSipMessage,
	u_char **ppSipBuf
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrMandatoryHeaderStruct *pMandatoryHdrs = pSipMessage->pMandatoryHdrs;

	if ((pSipMessage->eReqRspType == EcrioSigMgrSIPRequest) && (pMandatoryHdrs->pRequestUri))
	{
		_EcrioSigMgrFormUri(pSigMgr, pMandatoryHdrs->pRequestUri, ppSipBuf);
		STRCAT_SPACE(ppSipBuf);
		STRCAT((u_char *)"SIP/2.0", ppSipBuf);
		STRCAT_CRLF(ppSipBuf);
	}

	if ((pSipMessage->eReqRspType == EcrioSigMgrSIPRequest) && (pMandatoryHdrs->maxForwards))
	{
		char clen[20] = "";
		if (0 >= pal_SStringPrintf(clen, 20, "%lu", pMandatoryHdrs->maxForwards))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tString Copy error",
				__FUNCTION__, __LINE__);

			goto Error_Level_01;
		}
		clen[20 - 1] = '\0';
		STRCAT_HEADERNAME((u_char *)"Max-Forwards: ", (u_char *)"Max-Forwards: ", ppSipBuf);
		STRCAT((u_char *)clen, ppSipBuf);
		STRCAT_CRLF(ppSipBuf);
	}

	if (pMandatoryHdrs->pCallId)
	{
		STRCAT_HEADERNAME((u_char *)"Call-ID: ", (u_char *)"i: ", ppSipBuf);
		STRCAT(pMandatoryHdrs->pCallId, ppSipBuf);
		STRCAT_CRLF(ppSipBuf);
	}

	if (pMandatoryHdrs->pFrom)
	{
		STRCAT_HEADERNAME((u_char *)"From: ", (u_char *)"f: ", ppSipBuf);
		_EcrioSigMgrFormNameAddrWithParams(pSigMgr, pMandatoryHdrs->pFrom, ppSipBuf);
		STRCAT_CRLF(ppSipBuf);
	}

	if (pMandatoryHdrs->pTo)
	{
		STRCAT_HEADERNAME((u_char *)"To: ", (u_char *)"t: ", ppSipBuf);
		_EcrioSigMgrFormNameAddrWithParams(pSigMgr, pMandatoryHdrs->pTo, ppSipBuf);
		STRCAT_CRLF(ppSipBuf);
	}

	if (pMandatoryHdrs->ppVia)
	{
		u_int16 Iterator = 0;
		char porttext[6] = "";
		EcrioSigMgrViaStruct *pVia = NULL;

		for (Iterator = 0; Iterator < pMandatoryHdrs->numVia; Iterator++)
		{
			pVia = pMandatoryHdrs->ppVia[Iterator];
			STRCAT_HEADERNAME((u_char *)"Via: ", (u_char *)"v: ", ppSipBuf);
			STRCAT(pVia->pSipVersion, ppSipBuf);
			if (pVia->transport == EcrioSigMgrTransportUDP)
			{
				STRCAT((u_char *)"/UDP ", ppSipBuf);
			}
			else if (pVia->transport == EcrioSigMgrTransportTCP)
			{
				STRCAT((u_char *)"/TCP ", ppSipBuf);
			}
			else if (pVia->transport == EcrioSigMgrTransportTLS)
			{
				STRCAT((u_char *)"/TLS ", ppSipBuf);
			}
			if (pal_StringFindSubString((const u_char *)pVia->pIPaddr.pIPAddr, (const u_char *)":") != NULL)
			{
				// Put brackets around IPv6 IP.
				STRCAT_BRACKET_OPEN(ppSipBuf);
				STRCAT(pVia->pIPaddr.pIPAddr, ppSipBuf);
				STRCAT_BRACKET_CLOSE(ppSipBuf);
			}
			else
			{
				STRCAT(pVia->pIPaddr.pIPAddr, ppSipBuf);
			}

			if (pVia->pIPaddr.port != 0xFFFF)
			{
				STRCAT_COLON(ppSipBuf);
				if (0 >= pal_SStringPrintf(porttext, 6, "%u", pVia->pIPaddr.port))
				{
					error = ECRIO_SIGMGR_STRING_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tString Copy error",
						__FUNCTION__, __LINE__);

					goto Error_Level_01;
				}
				porttext[5] = '\0';
				STRCAT((u_char *)porttext, ppSipBuf);
			}

			if (pVia->pBranch)
			{
				STRCAT((u_char *)";branch=", ppSipBuf);
				STRCAT(pVia->pBranch, ppSipBuf);
			}

			if (pVia->ppParams)
			{
				error = _EcrioSigMgrFormParam(pSigMgr, pVia->numParams,
					pVia->ppParams, (u_char *)";", NULL, ppSipBuf);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					error = ECRIO_SIGMGR_UNKNOWN_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrFormUri() error=%u",
						__FUNCTION__, __LINE__, error);

					goto Error_Level_01;
				}
			}

			STRCAT_CRLF(ppSipBuf);
		}
	}

Error_Level_01:

	return error;
}

u_int32 _EcrioSigMgrSipMessageForm
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageModeEnum mode,
	EcrioSigMgrSipMessageStruct *pSipMessage,
	u_char **ppSipBuf,
	u_int32 *pSipLen
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	u_char *pMethod = NULL, *pOut = NULL;
	u_int32 size = 0, hdrIter = 0;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pSipMessage == NULL || ppSipBuf == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient Data Provided",
			__FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	if (pSipMessage->pMandatoryHdrs == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient Data Provided",
			__FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	pMethod = MethodTypeToString(pSipMessage->eMethodType);

	if (pSipMessage->eReqRspType == EcrioSigMgrSIPRequest)
	{
		STRCAT(pMethod, &pOut);
		STRCAT_SPACE(&pOut);
	}
	else if (pSipMessage->eReqRspType == EcrioSigMgrSIPResponse)
	{
		char responseCode[4] = { 0 };

		STRCAT((u_char *)"SIP/2.0", &pOut);
		STRCAT_SPACE(&pOut);
		if (0 >= pal_SStringPrintf(responseCode, 4, "%03d", pSipMessage->responseCode))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tString Copy error",
				__FUNCTION__, __LINE__);

			goto Error_Level_01;
		}
		responseCode[3] = '\0';
		STRCAT((u_char *)responseCode, &pOut);
		STRCAT_SPACE(&pOut);
		STRCAT(pSipMessage->pReasonPhrase, &pOut);
		STRCAT_CRLF(&pOut);
	}

	error = _EcrioSigMgrFormMandatoryHdrs(pSigMgr, mode, pSipMessage, &pOut);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		error = ECRIO_SIGMGR_UNKNOWN_ERROR;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrFormUri() error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	if (pSipMessage->pMandatoryHdrs->CSeq > 0)
	{
		char cs[11] = "";
		if (0 >= pal_SStringPrintf(cs, 11, "%lu", pSipMessage->pMandatoryHdrs->CSeq))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tString Copy error",
				__FUNCTION__, __LINE__);

			goto Error_Level_01;
		}
		cs[11 - 1] = '\0';
		STRCAT_HEADERNAME((u_char *)"CSeq: ", (u_char *)"CSeq: ", &pOut);
		STRCAT((u_char *)cs, &pOut);
		STRCAT_SPACE(&pOut);
		STRCAT(pMethod, &pOut);
		STRCAT_CRLF(&pOut);
	}

	if (pSipMessage->pOptionalHeaderList)
	{
		EcrioSigMgrHeaderStruct *pHeader = NULL;
		EcrioDSListGetSize(pSipMessage->pOptionalHeaderList, &size);

		for (hdrIter = 1; hdrIter <= size; ++hdrIter)
		{
			EcrioDSListGetDataAt(pSipMessage->pOptionalHeaderList, hdrIter, (void **)&pHeader);
			if (pHeader == NULL)
			{
				continue;
			}

			switch (pHeader->eHdrType)
			{
				case EcrioSipHeaderTypeAccept:
				{
					STRCAT_HEADERNAME((u_char *)"Accept: ", (u_char *)"Accept: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeAcceptContact:
				{
					STRCAT_HEADERNAME((u_char *)"Accept-Contact: ", (u_char *)"Accept-Contact: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeAcceptEncoding:
				{
					STRCAT_HEADERNAME((u_char *)"Accept-Encoding: ", (u_char *)"Accept-Encoding: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeAcceptLanguage:
				{
					STRCAT_HEADERNAME((u_char *)"Accept-Language: ", (u_char *)"Accept-Language: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeAllow:
				{
					STRCAT_HEADERNAME((u_char *)"Allow: ", (u_char *)"Allow: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeAllowEvents:
				{
					STRCAT_HEADERNAME((u_char *)"Allow-Events: ", (u_char *)"Allow-Events: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeDate:
				{
					STRCAT_HEADERNAME((u_char *)"Date: ", (u_char *)"Date: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeErrorInfo:
				{
					STRCAT_HEADERNAME((u_char *)"Error-Info: ", (u_char *)"Error-Info: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeEvent:
				{
					STRCAT_HEADERNAME((u_char *)"Event: ", (u_char *)"Event: ", &pOut);
				}
				break;
				case EcrioSipHeaderTypeSipIfMatch:
				{
					STRCAT_HEADERNAME((u_char *)"SIP-If-Match: ", (u_char *)"SIP-If-Match: ", &pOut);
				}
				break;
				case EcrioSipHeaderTypeP_AccessNetworkInfo:
				{
					STRCAT_HEADERNAME((u_char *)"P-Access-Network-Info: ", (u_char *)"P-Access-Network-Info: ", &pOut);
				}
				break;
				case EcrioSipHeaderTypeP_LastAccessNetworkInfo:
				{
					STRCAT_HEADERNAME((u_char *)"P-Last-Access-Network-Info: ", (u_char *)"P-Last-Access-Network-Info: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeP_ChargingFunctionAddr:
				{
					STRCAT_HEADERNAME((u_char *)"P-Charging-Function-Addresses: ", (u_char *)"P-Charging-Function-Addresses: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeProxyRequire:
				{
					STRCAT_HEADERNAME((u_char *)"Proxy-Require: ", (u_char *)"Proxy-Require: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeRequire:
				{
					STRCAT_HEADERNAME((u_char *)"Require: ", (u_char *)"Require: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeRetryAfter:
				{
					STRCAT_HEADERNAME((u_char *)"Retry-After: ", (u_char *)"Retry-After: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeAuthenticationInfo:
				{
					STRCAT_HEADERNAME((u_char *)"Authentication-Info: ", (u_char *)"Authentication-Info: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeServer:
				{
					STRCAT_HEADERNAME((u_char *)"Server: ", (u_char *)"Server: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeSupported:
				{
					STRCAT_HEADERNAME((u_char *)"Supported: ", (u_char *)"k: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeSubject:
				{
					STRCAT_HEADERNAME((u_char *)"Subject: ", (u_char *)"Subject: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeSecurityClient:
				{
					STRCAT_HEADERNAME((u_char *)"Security-Client: ", (u_char *)"Security-Client: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeSecurityServer:
				{
					STRCAT_HEADERNAME((u_char *)"Security-Server: ", (u_char *)"Security-Server: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeSecurityVerify:
				{
					STRCAT_HEADERNAME((u_char *)"Security-Verify: ", (u_char *)"Security-Verify: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeSessionExpires:
				{
					STRCAT_HEADERNAME((u_char *)"Session-Expires: ", (u_char *)"x: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeMinSE:
				{
					STRCAT_HEADERNAME((u_char *)"Min-SE: ", (u_char *)"Min-SE: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeTimestamp:
				{
					STRCAT_HEADERNAME((u_char *)"Timestamp: ", (u_char *)"Timestamp: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeUnsupported:
				{
					STRCAT_HEADERNAME((u_char *)"Unsupported: ", (u_char *)"Unsupported: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeUserAgent:
				{
					STRCAT_HEADERNAME((u_char *)"User-Agent: ", (u_char *)"User-Agent: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeWarning:
				{
					STRCAT_HEADERNAME((u_char *)"Warning: ", (u_char *)"Warning: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypePrivacy:
				{
					STRCAT_HEADERNAME((u_char *)"Privacy: ", (u_char *)"Privacy: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeRAck:
				{
					STRCAT_HEADERNAME((u_char *)"RAck: ", (u_char *)"RAck: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeRequestDisposition:
				{
					STRCAT_HEADERNAME((u_char *)"Request-Disposition: ", (u_char *)"d: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeRSeq:
				{
					STRCAT_HEADERNAME((u_char *)"RSeq: ", (u_char *)"RSeq: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeContentTransferEncoding:
				{
					STRCAT_HEADERNAME((u_char *)"Content-Transfer-Encoding: ", (u_char *)"Content-Transfer-Encoding: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeContentEncoding:
				{
					STRCAT_HEADERNAME((u_char *)"Content-Encoding: ", (u_char *)"Content-Encoding: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeContentType:
				{
					STRCAT_HEADERNAME((u_char *)"Content-Type: ", (u_char *)"c: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeReason:
				{
					STRCAT_HEADERNAME((u_char *)"Reason: ", (u_char *)"Reason: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeContributionID:
				{
					STRCAT_HEADERNAME((u_char *)"Contribution-ID: ", (u_char *)"Contribution-ID: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeConversationID:
				{
					STRCAT_HEADERNAME((u_char *)"Conversation-ID: ", (u_char *)"Conversation-ID: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeInReplyToContributionID:
				{
					STRCAT_HEADERNAME((u_char *)"InReplyTo-Contribution-ID: ", (u_char *)"InReplyTo-Contribution-ID: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeP_PreferredService:
				{
					STRCAT_HEADERNAME((u_char *)"P-Preferred-Service: ", (u_char *)"P-Preferred-Service: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeP_EarlyMedia:
				{
					STRCAT_HEADERNAME((u_char *)"P-Early-Media: ", (u_char *)"P-Early-Media: ", &pOut);
				}
				break;
				
				case EcrioSipHeaderTypePriority:
				{
					STRCAT_HEADERNAME((u_char *)"Priority: ", (u_char *)"Priority: ", &pOut);
				}
				break;				

				case EcrioSipHeaderTypeContentID:
				{
					STRCAT_HEADERNAME((u_char *)"Content-ID: ", (u_char *)"Content-ID: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeContentDisposition:
				{
					STRCAT_HEADERNAME((u_char *)"Content-Disposition: ", (u_char *)"Content-Disposition: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeReferTo:
				{
					STRCAT_HEADERNAME((u_char *)"Refer-To: ", (u_char *)"Refer-To: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeReferSub:
				{
					STRCAT_HEADERNAME((u_char *)"Refer-Sub: ", (u_char *)"Refer-Sub: ", &pOut);
				}
				break;

				case EcrioSipHeaderTypeReferredBy:
				{
					STRCAT_HEADERNAME((u_char *)"Referred-By: ", (u_char *)"Referred-By: ", &pOut);
				}
				break;

				default:
				{
					STRCAT_HEADERNAME(pHeader->pHeaderName, pHeader->pHeaderName, &pOut);
				}
				break;
			}

			_EcrioSigMgrFormHeaderStruct(pSigMgr, pHeader, &pOut);
		}
	}

	if (pSipMessage->pContentType)
	{
		STRCAT_HEADERNAME((u_char *)"Content-Type: ", (u_char *)"c: ", &pOut);
		_EcrioSigMgrFormHeaderStruct(pSigMgr, pSipMessage->pContentType, &pOut);
	}

	if (pSipMessage->pContact)
	{
		u_int16 contactIter = 0;
		EcrioSigMgrNameAddrWithParamsStruct *pContactDetails = NULL;
		STRCAT_HEADERNAME((u_char *)"Contact: ", (u_char *)"m: ", &pOut);

		for (contactIter = 0; contactIter < pSipMessage->pContact->numContactUris; contactIter++)
		{
			pContactDetails = pSipMessage->pContact->ppContactDetails[contactIter];

			if (contactIter > 0)
			{
				STRCAT((u_char *)",", &pOut);
			}

			if (pSipMessage->pFetaureTags)
			{
				/* This indicates overwritting of contact at run time */
				if (_EcrioSigMgrGetFeatureTagValue(pSipMessage->pFetaureTags, EcrioSipHeaderTypeContact))
				{
					_EcrioSigMgrAddRunTimeContact(pSigMgr, pSipMessage, pContactDetails, &pOut);
					break;
				}
			}

			_EcrioSigMgrFormNameAddrWithParams(pSigMgr, pContactDetails, &pOut);
		}

		STRCAT_CRLF(&pOut);
	}

	if (pSipMessage->pRouteSet)
	{
		u_int16 iterator = 0;
		EcrioSigMgrNameAddrWithParamsStruct *pRouteDetails = NULL;
		STRCAT_HEADERNAME((u_char *)"Route: ", (u_char *)"Route: ", &pOut);

		for (iterator = 0; iterator < pSipMessage->pRouteSet->numRoutes; iterator++)
		{
			pRouteDetails = pSipMessage->pRouteSet->ppRouteDetails[iterator];

			if (iterator > 0)
			{
				STRCAT((u_char *)",", &pOut);
			}

			_EcrioSigMgrFormNameAddrWithParams(pSigMgr, pRouteDetails, &pOut);
		}

		STRCAT_CRLF(&pOut);
	}

	if (pSipMessage->pRecordRouteSet)
	{
		u_int16 iterator = 0;
		EcrioSigMgrNameAddrWithParamsStruct *pRecordRouteDetails = NULL;
		STRCAT_HEADERNAME((u_char *)"Record-Route: ", (u_char *)"Record-Route: ", &pOut);

		for (iterator = 0; iterator < pSipMessage->pRecordRouteSet->numRoutes; iterator++)
		{
			pRecordRouteDetails = pSipMessage->pRecordRouteSet->ppRouteDetails[iterator];

			if (iterator > 0)
			{
				STRCAT((u_char *)",", &pOut);
			}

			_EcrioSigMgrFormNameAddrWithParams(pSigMgr, pRecordRouteDetails, &pOut);
		}

		STRCAT_CRLF(&pOut);
	}

	if (pSipMessage->ppPPreferredIdentity)
	{
		u_int16 iterator = 0;
		EcrioSigMgrNameAddrStruct *pNameAddr = NULL;
		BoolEnum bIsFound = Enum_FALSE;
		STRCAT_HEADERNAME((u_char *)"P-Preferred-Identity: ", (u_char *)"P-Preferred-Identity: ", &pOut);

		for (iterator = 0; iterator < pSipMessage->numPPreferredIdentity; iterator++)
		{
			pNameAddr = pSipMessage->ppPPreferredIdentity[iterator];
			if (pNameAddr->addrSpec.uriScheme == EcrioSigMgrURISchemeTEL)
			{
				if (iterator > 0)
				{
					STRCAT((u_char *)",", &pOut);
				}

				if (pNameAddr->pDisplayName)
				{
					STRCAT(pNameAddr->pDisplayName, &pOut);
					STRCAT_SPACE(&pOut);
				}

				STRCAT((u_char *)"<", &pOut);
				error = _EcrioSigMgrFormUri(pSigMgr, &pNameAddr->addrSpec, &pOut);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					error = ECRIO_SIGMGR_UNKNOWN_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrFormUri() error=%u",
						__FUNCTION__, __LINE__, error);

					goto Error_Level_01;
				}

				STRCAT((u_char *)">", &pOut);

				bIsFound = Enum_TRUE;
			}
		}

		if (bIsFound != Enum_TRUE)
		{
			pNameAddr = NULL;
			for (iterator = 0; iterator < pSipMessage->numPPreferredIdentity; iterator++)
			{
				pNameAddr = pSipMessage->ppPPreferredIdentity[iterator];
				if (pNameAddr->addrSpec.uriScheme == EcrioSigMgrURISchemeSIP)
				{
			if (iterator > 0)
			{
				STRCAT((u_char *)",", &pOut);
			}

			if (pNameAddr->pDisplayName)
			{
				STRCAT(pNameAddr->pDisplayName, &pOut);
				STRCAT_SPACE(&pOut);
			}

			STRCAT((u_char *)"<", &pOut);
			error = _EcrioSigMgrFormUri(pSigMgr, &pNameAddr->addrSpec, &pOut);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				error = ECRIO_SIGMGR_UNKNOWN_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrFormUri() error=%u",
					__FUNCTION__, __LINE__, error);

				goto Error_Level_01;
			}

			STRCAT((u_char *)">", &pOut);
					bIsFound = Enum_TRUE;
				}
			}
		}

		STRCAT_CRLF(&pOut);
	}

	if (pSipMessage->pExpires)
	{
		char textexpires[8] = "";
		STRCAT_HEADERNAME((u_char *)"Expires: ", (u_char *)"Expires: ", &pOut);
		if (0 >= pal_SStringPrintf(textexpires, 8, "%u", *(pSipMessage->pExpires)))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tString Copy error",
				__FUNCTION__, __LINE__);

			goto Error_Level_01;
		}
		textexpires[7] = '\0';
		STRCAT((u_char *)textexpires, &pOut);
		STRCAT_CRLF(&pOut);
	}

	if (pSipMessage->pMinExpires)
	{
		char textexpires[6] = "";
		STRCAT_HEADERNAME((u_char *)"Min-Expires: ", (u_char *)"Min-Expires: ", &pOut);
		if (0 >= pal_SStringPrintf(textexpires, 6, "%u", *(pSipMessage->pMinExpires)))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tString Copy error",
				__FUNCTION__, __LINE__);

			goto Error_Level_01;
		}
		textexpires[5] = '\0';
		STRCAT((u_char *)textexpires, &pOut);
		STRCAT_CRLF(&pOut);
	}

	// if (pSipMessage->pContentLength)

	if (pSipMessage->pAuthorization)
	{
		/*Required parameters are formed only*/
		EcrioSigMgrAuthorizationStruct *pAuthorization = pSipMessage->pAuthorization;

		STRCAT_HEADERNAME((u_char *)"Authorization: ", (u_char *)"Authorization: ", &pOut);
		STRCAT(pAuthorization->pAuthenticationScheme, &pOut);
		STRCAT_SPACE(&pOut);
		STRCAT((u_char *)"realm=\"", &pOut);
		STRCAT(pAuthorization->pRealm, &pOut);
		STRCAT((u_char *)"\",", &pOut);
		STRCAT((u_char *)"nonce=\"", &pOut);
		STRCAT(pAuthorization->pNonce, &pOut);
		STRCAT((u_char *)"\",", &pOut);
		STRCAT((u_char *)"algorithm=", &pOut);

		switch (pAuthorization->authAlgo)
		{
			case EcrioSipAuthAlgorithmDefault:
			case EcrioSipAuthAlgorithmMD5:
			{
				STRCAT(ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_ALGORITHM_VALUE_MD5, &pOut);
			}
			break;

			case EcrioSipAuthAlgorithmAKAv1:
			{
				STRCAT(ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_ALGORITHM_VALUE_AKAv1_MD5, &pOut);
			}
			break;

			case EcrioSipAuthAlgorithmAKAv2:
			{
				STRCAT(ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_ALGORITHM_VALUE_AKAv2_MD5, &pOut);
			}
			break;

			case EcrioSipAuthAlgorithmUnknown:
			default:
				error = ECRIO_SIGMGR_INVALID_DATA;
				goto Error_Level_01;
		}

		STRCAT((u_char *)",", &pOut);
		STRCAT((u_char *)"username=\"", &pOut);
		STRCAT(pAuthorization->pUserName, &pOut);
		STRCAT((u_char *)"\",", &pOut);
		STRCAT((u_char *)"uri=\"", &pOut);
		STRCAT(pAuthorization->pURI, &pOut);
		STRCAT((u_char *)"\",", &pOut);
		STRCAT((u_char *)"response=\"", &pOut);
		STRCAT(pAuthorization->pResponse, &pOut);
		STRCAT((u_char *)"\"", &pOut);

		if (pAuthorization->pQoP)
		{
			STRCAT((u_char *)",", &pOut);
			STRCAT((u_char *)"qop=", &pOut);
			STRCAT(pAuthorization->pQoP, &pOut);
		}

		if (pAuthorization->pOpaque)
		{
			STRCAT((u_char *)",", &pOut);
			STRCAT((u_char *)"opaque=\"", &pOut);
			STRCAT(pAuthorization->pOpaque, &pOut);
			STRCAT((u_char *)"\"", &pOut);
		}

		if (pAuthorization->pCNonce)
		{
			STRCAT((u_char *)",", &pOut);
			STRCAT((u_char *)"cnonce=\"", &pOut);
			STRCAT(pAuthorization->pCNonce, &pOut);
			STRCAT((u_char *)"\"", &pOut);
		}

		if (pAuthorization->pCountNonce)
		{
			STRCAT((u_char *)",", &pOut);
			STRCAT((u_char *)"nc=", &pOut);
			STRCAT(pAuthorization->pCountNonce, &pOut);
		}

		STRCAT_CRLF(&pOut);
	}

	if (pSipMessage->pMessageBody != NULL)
	{
		error = _EcrioSigMgrFormMsgBody(pSigMgr, mode, pSipMessage->pMessageBody, &pOut, pSipLen);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			error = ECRIO_SIGMGR_UNKNOWN_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrFormMsgBody() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}
	}
	else
	{
		char textexpires[6] = "";
		STRCAT_HEADERNAME((u_char *)"Content-Length: ", (u_char *)"l: ", &pOut);
		if (0 >= pal_SNumPrintf(textexpires, 6, "%u", pSipMessage->contentLength))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tString Copy error",
				__FUNCTION__, __LINE__);

			goto Error_Level_01;
		}
		textexpires[5] = '\0';
		STRCAT((u_char *)textexpires, &pOut);
		STRCAT_CRLF(&pOut);

		STRCAT_CRLF(&pOut);
		*pSipLen = pal_StringLength(pOut);
		/**pSipLen = pal_StringLength(pOut) + 1;
		pal_MemoryReallocate(pSigMgr->moduleId, *pSipLen,
		    (void **)&pOut);
		pOut[*pSipLen - 1] = '\0';*/
	}

	*ppSipBuf = pOut;
	pOut = NULL;

Error_Level_01:

	if (pOut)
	{
		pal_MemoryFree((void **)&pOut);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

	return error;
}
