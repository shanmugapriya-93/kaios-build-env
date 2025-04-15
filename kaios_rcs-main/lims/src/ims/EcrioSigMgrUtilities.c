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
#include "EcrioSigMgrSocketCallbacks.h"
#include "EcrioSigMgrUtilities.h"
#include "EcrioSigMgrIMSLibHandler.h"
#include "EcrioTXN.h"
#include "EcrioDSListInterface.h"

const u_int32 uKHName = 32;

KHASH_MAP_INIT_STR(uKHName, void*);

/**
*	Create string if the destination string is null else concatenate
*	the string. The destination string has to be released memory by the user.
*/
u_int32 _EcrioSigMgrStringCreate
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pSrcString,
	u_char **ppDestString
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR, oldLength = 0;
	u_int32 stringLength = 0;
	u_int32 memLength = 0;

	(void)pSigMgr;

	if (pSrcString == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	stringLength = pal_StringLength(pSrcString) + 1;
	if (*ppDestString == NULL)
	{
		pal_MemoryAllocate(stringLength, (void **)ppDestString);
		memLength = stringLength;
	}
	else
	{
		oldLength = pal_StringLength(*ppDestString);
		pal_MemoryReallocate(oldLength + stringLength, (void **)ppDestString);
		memLength = oldLength + stringLength;
	}

	if (*ppDestString == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	if (NULL == pal_StringNConcatenate(*ppDestString, memLength, pSrcString, pal_StringLength(pSrcString)))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tString Copy error",
			__FUNCTION__, __LINE__);

		error = ECRIO_SIGMGR_STRING_ERROR;
		goto Error_Level_01;
	}

	error = ECRIO_SIGMGR_NO_ERROR;
	goto Error_Level_01;

Error_Level_01:
	return error;
}

/**
*	Create string if the destination string is null else concatenate
*	the string upto the len. The destination string has to be released memory by the user.
*/
u_int32 _EcrioSigMgrStringNCreate
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pSrcString,
	u_char **ppDestString,
	u_int32 len
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR, oldLength = 0;
	u_int32 stringLength = 0;
	u_int32 destLength = 0;

	(void)pSigMgr;

	if ((pSrcString == NULL) || (ppDestString == NULL))
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	stringLength = pal_StringLength(pSrcString);
	if (stringLength < len)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	stringLength = len + 1;

	if (*ppDestString == NULL)
	{
		pal_MemoryAllocate(stringLength, (void **)ppDestString);
		destLength = stringLength;
	}
	else
	{
		oldLength = pal_StringLength(*ppDestString);
		pal_MemoryReallocate(oldLength + stringLength, (void **)ppDestString);
		destLength = oldLength + stringLength;
	}

	if (*ppDestString == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	if (NULL == pal_StringNConcatenate(*ppDestString, destLength, pSrcString, len))
	{
		if (oldLength == 0)
		{
			pal_MemoryFree((void **)&ppDestString);
		}

		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tString Copy error.",
			__FUNCTION__, __LINE__);

		error = ECRIO_SIGMGR_STRING_ERROR;
		goto Error_Level_01;
	}

	error = ECRIO_SIGMGR_NO_ERROR;
	goto Error_Level_01;

Error_Level_01:
	return error;
}

/**
*	Modify the via header with protocol version and port number.
*/

/**
*	Remove Double Quotes.
*/
u_char *_EcrioSigMgrStringUnquote
(
	u_char *pIn_String
)
{
	BoolEnum Result = Enum_TRUE;
	u_int32 inLength = 0;
	u_int32 outLength = 0;
	u_int32 i = 0;
	u_char *pTempStr = NULL;
	u_char *pOut_string = NULL;

	if (pIn_String == NULL)
	{
		return NULL;
	}

	inLength = (u_int32)pal_StringLength(pIn_String);
	if (!inLength)
	{
		return NULL;
	}

	Result = _EcrioSigMgrStringIsQuoted( pIn_String );

	if (Result == Enum_FALSE)
	{
		/* String is already Unquoted, copy input to output */
		pal_MemoryAllocate(inLength + 1, (void **)&pOut_string);
		if (pOut_string == NULL)
		{
			return NULL;
		}

		if (NULL == pal_StringNCopy( pOut_string, inLength + 1, pIn_String, inLength ))
		{
			pal_MemoryFree((void **)&pOut_string);
			return NULL;
		}
		return pOut_string;
	}

	if (inLength < 2)
	{
		return NULL;
	}

	outLength = inLength - (u_int32)2;	// Two less since removing '"'
	if (!outLength)
	{
		/* String is '""' */
		return NULL;
	}

	pal_MemoryAllocate(outLength + 1, (void **)&pOut_string);
	if (pOut_string == NULL)
	{
		return NULL;
	}

	pTempStr = pOut_string;

	for (i = 0; i < inLength; i++)
	{
		if (pIn_String[i] != '"')
		{
			*pTempStr++ = pIn_String[i];
		}
	}

	*pTempStr = '\0';

	return pOut_string;
}

/**
* Determine if string is Quoted
* Dont call this function when Input is NULL,
* That check should be done by the Caller
*/
BoolEnum _EcrioSigMgrStringIsQuoted
(
	u_char *pIn_String
)
{
	u_char *Tst_String;
	u_int32 strlen = 0;

	if (pIn_String == NULL)
	{
		return Enum_FALSE;
	}

	Tst_String = pIn_String;
	strlen = pal_StringLength(Tst_String);
	if ((*Tst_String == '"') && (Tst_String[strlen - 1] == '"'))
	{
		return Enum_TRUE;
	}
	else
	{
		return Enum_FALSE;
	}
}

/**************************************************************************

Function:		_EcrioSigMgrCopyRouteSet()

Purpose:		Stores source route set values to destination route set.

Description:	Stores source route set values to destination route set.

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
                EcrioSigMgrNameAddrWithParamsStruct* ppSrcRouteSet - Source Route Set Values.
OutPut:
                 EcrioSigMgrNameAddrWithParamsStruct*** pppDestRouteSet - Destination Set Values

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrCopyRouteSet
(
	EcrioSigMgrStruct *pSigMgr,
	u_int16 routeSetCount,
	EcrioSigMgrNameAddrWithParamsStruct **ppSrcRouteSet,
	EcrioSigMgrNameAddrWithParamsStruct ***pppDestRouteSet
)
{
	u_int32 i = 0, error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrNameAddrWithParamsStruct **ppDestRouteSet = NULL;

	if (pSigMgr == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (ppSrcRouteSet == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;

		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data passed, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_02;
	}

	pal_MemoryAllocate(routeSetCount * sizeof(EcrioSigMgrNameAddrWithParamsStruct *), (void **)&ppDestRouteSet);
	if (ppDestRouteSet == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_02;
	}

	for (i = 0; i < routeSetCount; ++i)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct), (void **)&(ppDestRouteSet[i]));
		if (ppDestRouteSet[i] == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_02;
		}

		pal_MemorySet(ppDestRouteSet[i], 0, sizeof(EcrioSigMgrNameAddrWithParamsStruct));

		error = _EcrioSigMgrPopulateNameAddr(pSigMgr,
			&ppSrcRouteSet[i]->nameAddr, &ppDestRouteSet[i]->nameAddr);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_02;
		}

		ppDestRouteSet[i]->numParams = ppSrcRouteSet[i]->numParams;
		if (ppDestRouteSet[i]->numParams > 0)
		{
			error = _EcrioSigMgrCopyParam(pSigMgr, ppDestRouteSet[i]->numParams,
				ppSrcRouteSet[i]->ppParams, &(ppDestRouteSet[i]->ppParams));
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error_Level_02;
			}
		}
	}

	*pppDestRouteSet = ppDestRouteSet;
	error = ECRIO_SIGMGR_NO_ERROR;

Error_Level_02:
	if ((error != ECRIO_SIGMGR_NO_ERROR) && (ppDestRouteSet != NULL))
	{
		for (i = 0; i < routeSetCount; ++i)
		{
			EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_NameAddrWithParams,
				(void **)&ppDestRouteSet[i], Enum_TRUE);
		}

		pal_MemoryFree((void **)&ppDestRouteSet);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

Error_Level_01:
	return error;
}

/***********************************************************************************************

Function:		_EcrioSigMgrReleaseRouteSet()

Purpose:		To release the memory allocated for the route set.

Description:	This internal function deinitializes the route set structure and frees the memory
                for the internal members of the route set.

Input:			IN EcrioSigMgrStruct* pSigMgr - Handle to Signaling Manager instance.
                IN u_int16 routeSetCount - Route set count
                IN EcrioSigMgrNameAddrWithParamsStruct** ppRouteSet - Route set

Returns:			error code.
*************************************************************************************************/
u_int32 _EcrioSigMgrReleaseRouteSet
(
	EcrioSigMgrStruct *pSigMgr,
	u_int16 routeSetCount,
	EcrioSigMgrNameAddrWithParamsStruct **ppRouteSet
)
{
	u_int32 i = 0;

	if (!ppRouteSet)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (routeSetCount > 0)
	{
		for (i = 0; i < routeSetCount; ++i)
		{
			EcrioSigMgrStructRelease((SIGMGRHANDLE)pSigMgr,
				EcrioSigMgrStructType_NameAddrWithParams, (void **)&ppRouteSet[i], Enum_TRUE);
		}

		pal_MemoryFree((void **)&ppRouteSet);
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

/**
* Genrate tag parameter.
*/
u_int32 _EcrioSigMgrGenerateTag
(
	u_char **ppTag
)
{
	u_char *pTag = NULL;
	u_int32 randNo = 0;
	u_char randStr[15];

	/* generate a random number for the tag */
	randNo = pal_UtilityRandomNumber();
	if (0 >= pal_NumToString(randNo, randStr, 15))
	{
		return ECRIO_SIGMGR_STRING_ERROR;
	}

	pal_MemoryAllocate(pal_StringLength(randStr) + 1, (void **)&pTag);
	if (pTag == NULL)
	{
		return ECRIO_SIGMGR_NO_MEMORY;
	}

	if (NULL == pal_StringNCopy(pTag, pal_StringLength(randStr) + 1, randStr, pal_StringLength(randStr)))
	{
		pal_MemoryFree((void **)&pTag);
		return ECRIO_SIGMGR_STRING_ERROR;
	}

	*ppTag = pTag;
	return ECRIO_SIGMGR_NO_ERROR;
}

/**
* Generate call-id.
*/
u_int32 _EcrioSigMgrGenerateCallId
(
	u_char *pUserId,
	u_char **ppCallId
)
{
	u_char *pCallId = NULL;
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 totalLen = 0, randNo = 0;
	u_char randStr[15] = "";
	u_char temp[2] = {"-"};

	/* check the validity of incoming pointers */
	if (!pUserId)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	totalLen = pal_StringLength(pUserId);

	randNo = pal_UtilityRandomNumber();
	if (0 >= pal_NumToString(randNo, randStr, 15))
	{
		error = ECRIO_SIGMGR_STRING_ERROR;
		goto Error_Level_01;
	}		
	totalLen += pal_StringLength(randStr);
	totalLen += 3;	/* to accomodate "-" and null characters */

	pal_MemoryAllocate(totalLen, (void **)&pCallId);
	if (pCallId == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	/* copy the random number, user-ID to the target string buffer */
	if (NULL == pal_StringNCopy(pCallId, totalLen, randStr, pal_StringLength(randStr)))
	{
		pal_MemoryFree((void **)&pCallId);
		error = ECRIO_SIGMGR_STRING_ERROR;
		goto Error_Level_01;
	}
	if (NULL == pal_StringNConcatenate(pCallId, totalLen - pal_StringLength(pCallId), temp, pal_StringLength(temp)))
	{
		pal_MemoryFree((void **)&pCallId);
		error = ECRIO_SIGMGR_STRING_ERROR;
		goto Error_Level_01;
	}
	if (NULL == pal_StringNConcatenate(pCallId, totalLen - pal_StringLength(pCallId), pUserId, pal_StringLength(pUserId)))
	{
		pal_MemoryFree((void **)&pCallId);
		error = ECRIO_SIGMGR_STRING_ERROR;
		goto Error_Level_01;
	}

	*ppCallId = pCallId;

Error_Level_01:
	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrGenerateBranchParam()

Purpose:		Generates branch parameter.

Description:	Generates branch parameter.

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.

OutPut:			u_char** ppBranch - branch string


Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrGenerateBranchParam
(
	EcrioSigMgrStruct *pSigMgr,
	u_char **ppBranch
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR, randNo = 0, totalLen = 0;
	u_char branch[] = ECRIO_SIG_MGR_BRANCH_INITIAT_STR;
	u_char randStr1[15], randStr2[15], randStr3[15];
	u_char *pBranch = NULL;

	(void)pSigMgr;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tppBranch=%p",
		__FUNCTION__, __LINE__, ppBranch);

	/* generate a random number and insert it
	in the target string buffer */
	randNo = pal_UtilityRandomNumber();
	if (0 >= pal_NumToString(randNo, randStr1, 15))
	{
		error = ECRIO_SIGMGR_STRING_ERROR;
		goto Error_Level_01;
	}
	randNo = pal_UtilityRandomNumber();
	if (0 >= pal_NumToString(randNo, randStr2, 15))
	{
		error = ECRIO_SIGMGR_STRING_ERROR;
		goto Error_Level_01;
	}
	randNo = pal_UtilityRandomNumber();
	if (0 >= pal_NumToString(randNo, randStr3, 15))
	{
		error = ECRIO_SIGMGR_STRING_ERROR;
		goto Error_Level_01;
	}

	totalLen = pal_StringLength(branch) + pal_StringLength(randStr1) + pal_StringLength(randStr2) +
		pal_StringLength(randStr3) + 1;

	pal_MemoryAllocate(totalLen, (void **)&pBranch);
	if (pBranch == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	if (NULL == pal_StringNCopy(pBranch, totalLen, branch, pal_StringLength(branch)))
	{
		pal_MemoryFree((void **)&pBranch);
		error = ECRIO_SIGMGR_STRING_ERROR;
		goto Error_Level_01;
	}
	if (NULL == pal_StringNConcatenate(pBranch, totalLen - pal_StringLength(pBranch), randStr1, pal_StringLength(randStr1)))
	{
		pal_MemoryFree((void **)&pBranch);
		error = ECRIO_SIGMGR_STRING_ERROR;
		goto Error_Level_01;
	}
	if (NULL == pal_StringNConcatenate(pBranch, totalLen - pal_StringLength(pBranch), randStr2, pal_StringLength(randStr2)))
	{
		pal_MemoryFree((void **)&pBranch);
		error = ECRIO_SIGMGR_STRING_ERROR;
		goto Error_Level_01;
	}
	if (NULL == pal_StringNConcatenate(pBranch, totalLen - pal_StringLength(pBranch), randStr3, pal_StringLength(randStr3)))
	{
		pal_MemoryFree((void **)&pBranch);
		error = ECRIO_SIGMGR_STRING_ERROR;
		goto Error_Level_01;
	}

	*ppBranch = pBranch;

Error_Level_01:
#if 0
	if ((error != ECRIO_SIGMGR_NO_ERROR) && (pBranch != NULL))
	{
		pal_MemoryFree((void **)&pBranch);
	}
#endif
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrUpdateRequestNonceCount()

Purpose:		Update the nonce count to Authorization or
                Proxy-Authorization header.

Description:	Update the nonce count to Authorization or
                Proxy-Authorization header.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                u_int16 numProxy - no. of proxy header.
                EcrioSigMgrAuthorizationStruct** ppProxyAuth - list of Proxy-Authorization header.
                u_int32 nonceCount - nonce count.
                EcrioSigMgrAuthorizationStruct* pAuth - Authorization header.

OutPut:			ppProxyAuth/pAuth.


Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrUpdateRequestNonceCount
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrAuthorizationStruct *pAuth,
	u_int32 nonceCount
)
{
	u_int32 ncLength = ECRIO_SIG_MGR_MAX_NC_LENGTH, i = 0;
	u_char ncString[ECRIO_SIGMGR_BUFFER_SIZE_10] = "";
	u_char pTempString[ECRIO_SIGMGR_BUFFER_SIZE_10] = "";

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (0 >= pal_SNumPrintf((char *)ncString, 10, "%x", nonceCount))
	{
		return ECRIO_SIGMGR_STRING_ERROR;
	}

	for (i = 0; i < (ncLength - pal_StringLength(ncString)); ++i)
	{
		if (NULL == pal_StringNConcatenate(pTempString, ECRIO_SIGMGR_BUFFER_SIZE_10 - pal_StringLength(pTempString), (u_char *)"0", pal_StringLength((const u_char*)"0")))
		{
			return ECRIO_SIGMGR_STRING_ERROR;
		}
	}

	if (NULL == pal_StringNConcatenate(pTempString, ECRIO_SIGMGR_BUFFER_SIZE_10 - pal_StringLength(pTempString), ncString, pal_StringLength(ncString)))
	{
		return ECRIO_SIGMGR_STRING_ERROR;
	}

	if (pAuth != NULL)
	{
		if (pAuth->pCountNonce != NULL)
		{
			pal_MemoryFree((void **)&pAuth->pCountNonce);
		}

		/* As per RFC 2617 nonce-count MUST be specified if a qop directive is sent and
		** MUST NOT be specified if the server did not send a qop directive in the
		** WWW-Authenticate header field.
		*/
		if (pAuth->pQoP)
		{
			_EcrioSigMgrStringCreate(pSigMgr, pTempString, &pAuth->pCountNonce);
		}
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:		_EcrioSigMgrPopulateAuthorizationDetails()

Purpose:		Stores Source Authorization details to destination authorization details..

Description:	Stores Source Authorization details to destination authorization details..

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
                EcrioSigMgrAuthorizationStruct* pSrcAuth -  Source Authorization details.
                EcrioSigMgrAuthorizationStruct* pDstAuth -  Destination Authorization details

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrPopulateAuthorizationDetails
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrAuthorizationStruct *pSrcAuth,
	EcrioSigMgrAuthorizationStruct *pDstAuth
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
		"%s:%u\tpSigMgr=%p, pSrcAuth=%p, pDstAuth=%p",
		__FUNCTION__, __LINE__, pSigMgr, pSrcAuth, pDstAuth);

	if ((pSrcAuth == NULL) || (pDstAuth == NULL))
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	// Store Authentication scheme
	if (pSrcAuth->pAuthenticationScheme != NULL)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcAuth->pAuthenticationScheme,
			&(pDstAuth->pAuthenticationScheme));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	// Store User Name
	if (pSrcAuth->pUserName != NULL)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcAuth->pUserName,
			&(pDstAuth->pUserName));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	// Store realm Value
	if (pSrcAuth->pRealm != NULL)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcAuth->pRealm,
			&(pDstAuth->pRealm));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	// Store Nonce Value
	if (pSrcAuth->pNonce != NULL)
	{
		if (pal_StringLength(pSrcAuth->pNonce))
		{
			_EcrioSigMgrStringCreate(pSigMgr, pSrcAuth->pNonce,
				&(pDstAuth->pNonce));
		}
		else
		{
			pal_MemoryAllocate(sizeof(u_char), (void **)&(pDstAuth->pNonce));
			if (pDstAuth->pNonce)
			{
				pDstAuth->pNonce[0] = '\0';
			}
		}

		if (pDstAuth->pNonce == NULL)
		{
			goto Error_Level_01;
		}
	}

	// Store Opaque Value
	if (pSrcAuth->pOpaque != NULL)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcAuth->pOpaque,
			&(pDstAuth->pOpaque));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	// Store URI
	if (pSrcAuth->pURI != NULL)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcAuth->pURI,
			&(pDstAuth->pURI));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	// Store Response
	if (pSrcAuth->pResponse != NULL)
	{
		if (pal_StringLength(pSrcAuth->pResponse))
		{
			_EcrioSigMgrStringCreate(pSigMgr, pSrcAuth->pResponse,
				&(pDstAuth->pResponse));
		}
		else
		{
			pal_MemoryAllocate(sizeof(u_char), (void **)&(pDstAuth->pResponse));
			if (pDstAuth->pResponse)
			{
				pDstAuth->pResponse[0] = '\0';
			}
		}

		if (pDstAuth->pResponse == NULL)
		{
			goto Error_Level_01;
		}
	}

	// Store CNonce Value
	if (pSrcAuth->pCNonce != NULL)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcAuth->pCNonce,
			&(pDstAuth->pCNonce));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	// Store Quality of Protection
	if (pSrcAuth->pQoP != NULL)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcAuth->pQoP,
			&(pDstAuth->pQoP));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	// Store Algo type parameter
	pDstAuth->authAlgo = pSrcAuth->authAlgo;

	// Store Nonce Count
	if (pSrcAuth->pCountNonce != NULL)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcAuth->pCountNonce,
			&(pDstAuth->pCountNonce));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrPopulateViaHdr()

Purpose:		Stores Via header values from source Via to destination Via.

Description:	Stores Via header values from source Via to destination Via

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
                EcrioSigMgrViaStruct* pSrcVia - Source Via
                EcrioSigMgrViaStruct* pDstVia - Destination Via.

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrPopulateViaHdr
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrViaStruct *pSrcVia,
	EcrioSigMgrViaStruct *pDstVia
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR, i = 0;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpSigMgr=%p, pSrcVia=%p, pDstVia=%p",
		__FUNCTION__, __LINE__, pSigMgr, pSrcVia, pDstVia);

	if ((pSrcVia == NULL) || (pDstVia == NULL))
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	// store sip version
	if (pSrcVia->pSipVersion != NULL)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcVia->pSipVersion,
			&(pDstVia->pSipVersion));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}
	}

	// store branch param
	if (pSrcVia->pBranch != NULL)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcVia->pBranch,
			&(pDstVia->pBranch));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}
	}

	// copy parameters
	if (pSrcVia->numParams > 0)
	{
		pDstVia->numParams = pSrcVia->numParams;

		/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
		if (pal_UtilityDataOverflowDetected(pDstVia->numParams, sizeof(EcrioSigMgrParamStruct *)) == Enum_TRUE)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pal_MemoryAllocate(pDstVia->numParams * sizeof(EcrioSigMgrParamStruct *), (void **)&pDstVia->ppParams);
		if (pDstVia->ppParams == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		for (i = 0; i < pDstVia->numParams; ++i)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&pDstVia->ppParams[i]);
			if (pDstVia->ppParams[i] == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			_EcrioSigMgrStringCreate(pSigMgr, pSrcVia->ppParams[i]->pParamName,
				&(pDstVia->ppParams[i]->pParamName));
			_EcrioSigMgrStringCreate(pSigMgr, pSrcVia->ppParams[i]->pParamValue,
				&(pDstVia->ppParams[i]->pParamValue));
		}
	}

	// copy ip address
	if (pSrcVia->pIPaddr.pIPAddr != NULL)
	{
		/*pal_MemoryAllocate(sizeof(EcrioSigMgrIPAddrStruct), (void **)&pDstVia->pIPaddr);
		if (pDstVia->pIPaddr == NULL)
		{
		    error = ECRIO_SIGMGR_NO_MEMORY;
		    goto Error_Level_01;
		}*/

		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcVia->pIPaddr.pIPAddr,
			&(pDstVia->pIPaddr.pIPAddr));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pDstVia->pIPaddr.port = pSrcVia->pIPaddr.port;
	}

	// copy transport type
	pDstVia->transport = pSrcVia->transport;

	error = ECRIO_SIGMGR_NO_ERROR;

Error_Level_01:
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		_EcrioSigMgrReleaseViaStruct(pSigMgr, pDstVia);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrAddHdr()

Purpose:		Stores source header values to destination header values.

Description:	Stores source header values to destination header values.

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
                EcrioSigMgrHeaderStruct* pSrcHdr - Source Header Values.
OutPut:
                EcrioSigMgrHeaderStruct** ppDstHdr - Destination Header Values.

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrAddHdr
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrHeaderStruct *pSrcHdr,
	EcrioSigMgrHeaderStruct **ppDstHdr
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR, j = 0;
	EcrioSigMgrHeaderStruct *pHdr = NULL;
	EcrioSigMgrHeaderValueStruct *pSrcHdrValue = NULL;
	EcrioSigMgrHeaderValueStruct *pHdrValue = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
		"%s:%u\tpSigMgr=%p, pSrcHdr=%p, ppDstHdr=%p",
		__FUNCTION__, __LINE__, pSigMgr, pSrcHdr, ppDstHdr);

	if ((pSrcHdr == NULL) || (ppDstHdr == NULL))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data passed", __FUNCTION__, __LINE__);

		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	if ((pSrcHdr->numHeaderValues == 0) ||
		(pSrcHdr->ppHeaderValues == NULL))
	{
		goto Error_Level_01;
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderStruct), (void **)&pHdr);
	if (pHdr == NULL)
	{
		error = ECRIO_SIGMGR_NO_ERROR;
		goto Error_Level_01;
	}

	if (pSrcHdr->pHeaderName != NULL)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcHdr->pHeaderName,
			&(pHdr->pHeaderName));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	pHdr->numHeaderValues = pSrcHdr->numHeaderValues;

	/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
	if (pal_UtilityDataOverflowDetected(pHdr->numHeaderValues, sizeof(EcrioSigMgrHeaderValueStruct *)) == Enum_TRUE)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderValueStruct *) * pHdr->numHeaderValues, (void **)&pHdr->ppHeaderValues);
	if (pHdr->ppHeaderValues == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	for (j = 0; j < pHdr->numHeaderValues; ++j)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderValueStruct), (void **)&pHdr->ppHeaderValues[j]);
		if (pHdr->ppHeaderValues[j] == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pSrcHdrValue = pSrcHdr->ppHeaderValues[j];
		pHdrValue = pHdr->ppHeaderValues[j];

		error = _EcrioSigMgrStringCreate(pSigMgr, (u_char *)pSrcHdrValue->pHeaderValue,
			(u_char **)&(pHdrValue->pHeaderValue));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}

		if (pSrcHdrValue->numParams > 0)
		{
			error = _EcrioSigMgrCopyParam(pSigMgr, pSrcHdrValue->numParams,
				pSrcHdrValue->ppParams, &pHdrValue->ppParams);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error_Level_01;
			}

			pHdrValue->numParams = pSrcHdrValue->numParams;
		}
	}

	*ppDstHdr = pHdr;
	error = ECRIO_SIGMGR_NO_ERROR;

Error_Level_01:
	if ((error != ECRIO_SIGMGR_NO_ERROR) && (pHdr != NULL))
	{
		EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_Header,
			(void **)&pHdr, Enum_TRUE);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrAddValuesToHdr()

Purpose:		Add given header values to header.

Description:	Add given header values to header.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                u_int32 numValues - number of header values
                u_char** ppValues - List of header values

OutPut:			EcrioSigMgrHeaderStruct* pHdr - Header to which values are
                to be added

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrAddValuesToHdr
(
	EcrioSigMgrStruct *pSigMgr,
	u_int16 numValues,
	u_char **ppValues,
	EcrioSigMgrHeaderStruct *pHdr
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 i = 0, existingValues = 0;
	EcrioSigMgrHeaderValueStruct *pHdrVal = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pHdr == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}	

	if (pHdr->numHeaderValues == 0)
	{
		/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
		if (pal_UtilityDataOverflowDetected(numValues, sizeof(EcrioSigMgrHeaderValueStruct *)) == Enum_TRUE)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderValueStruct *) * numValues, (void **)&pHdr->ppHeaderValues);
		if (pHdr->ppHeaderValues == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pHdr->numHeaderValues = numValues;
	}
	else
	{
		/* Check arithmetic overflow - do not use the ArithmeticOverflowDetected() API for u_int16 variable. */
		if ((pHdr->numHeaderValues + numValues) > USHRT_MAX)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
		if (pal_UtilityDataOverflowDetected((pHdr->numHeaderValues + numValues), sizeof(EcrioSigMgrHeaderValueStruct *)) == Enum_TRUE)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		error = pal_MemoryReallocate(sizeof(EcrioSigMgrHeaderValueStruct *) * (pHdr->numHeaderValues + numValues), (void **)&pHdr->ppHeaderValues);
		if (error != KPALErrorNone)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		existingValues = pHdr->numHeaderValues;
		pHdr->numHeaderValues =
			(u_int16)(pHdr->numHeaderValues + numValues);
	}

	for (i = 0; i < numValues; ++i)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderValueStruct), (void **)&pHdr->ppHeaderValues[existingValues + i]);
		if (pHdr->ppHeaderValues[existingValues + i] == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pHdrVal = pHdr->ppHeaderValues[existingValues + i];

		pal_MemorySet(pHdrVal, 0, sizeof(EcrioSigMgrHeaderValueStruct));

		if (ppValues[i])
		{
			error = _EcrioSigMgrStringCreate(pSigMgr, ppValues[i],
				(u_char **)&(pHdrVal->pHeaderValue));
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error_Level_01;
			}
		}
	}

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrAddValueParamsToHdr
(
	EcrioSigMgrStruct *pSigMgr,
	u_int16 numValues,
	u_char **ppValues,
	u_int16 numParams,
	u_char **ppParamsName,
	u_char **ppParamsValue,
	EcrioSigMgrHeaderStruct *pHdr
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 i = 0, existingValues = 0, j = 0;
	EcrioSigMgrHeaderValueStruct *pHdrVal = NULL;

	EcrioSigMgrParamStruct *pParamVal = NULL;

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
// quiet		__FUNCTION__, __LINE__);

	if (pHdr == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}	

	if (pHdr->numHeaderValues == 0)
	{
		/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
		if (pal_UtilityDataOverflowDetected(numValues, sizeof(EcrioSigMgrHeaderValueStruct *)) == Enum_TRUE)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderValueStruct *) * numValues, (void **)&pHdr->ppHeaderValues);
		if (pHdr->ppHeaderValues == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pHdr->numHeaderValues = numValues;
	}
	else
	{
		/* Check arithmetic overflow - do not use the ArithmeticOverflowDetected() API for u_int16 variable. */
		if ((pHdr->numHeaderValues + numValues) > USHRT_MAX)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
		if (pal_UtilityDataOverflowDetected((pHdr->numHeaderValues + numValues), sizeof(EcrioSigMgrHeaderValueStruct *)) == Enum_TRUE)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		error = pal_MemoryReallocate(sizeof(EcrioSigMgrHeaderValueStruct *) * (pHdr->numHeaderValues + numValues), (void **)&pHdr->ppHeaderValues);
		if (error != KPALErrorNone)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		existingValues = pHdr->numHeaderValues;
		pHdr->numHeaderValues =
			(u_int16)(pHdr->numHeaderValues + numValues);
	}

	for (i = 0; i < numValues; ++i)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderValueStruct), (void **)&pHdr->ppHeaderValues[existingValues + i]);
		if (pHdr->ppHeaderValues[existingValues + i] == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pHdrVal = pHdr->ppHeaderValues[existingValues + i];

		pal_MemorySet(pHdrVal, 0, sizeof(EcrioSigMgrHeaderValueStruct));

		if (ppValues[i])
		{
			error = _EcrioSigMgrStringCreate(pSigMgr, ppValues[i],
				(u_char **)&(pHdrVal->pHeaderValue));
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error_Level_01;
			}
		}

		if (numParams)
		{
			pHdrVal->numParams = numParams;

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected(numParams, sizeof(EcrioSigMgrParamStruct *)) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
				goto Error_Level_01;
			}

			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct *) * numParams, (void **)&pHdrVal->ppParams);
			if (pHdrVal->ppParams == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			for (j = 0; j < numParams; ++j)
			{
				pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&pHdrVal->ppParams[j]);
				if (pHdrVal->ppParams[j] == NULL)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					goto Error_Level_01;
				}

				pParamVal = pHdrVal->ppParams[j];

				if (ppParamsName)
				{
					if (ppParamsName[j])
					{
						error = _EcrioSigMgrStringCreate(pSigMgr, ppParamsName[j],
							(u_char **)&(pParamVal->pParamName));
						if (error != ECRIO_SIGMGR_NO_ERROR)
						{
							goto Error_Level_01;
						}
					}
				}

				if (ppParamsValue)
				{
					if (ppParamsValue[j])
					{
						error = _EcrioSigMgrStringCreate(pSigMgr, ppParamsValue[j],
							(u_char **)&(pParamVal->pParamValue));
						if (error != ECRIO_SIGMGR_NO_ERROR)
						{
							goto Error_Level_01;
						}
					}
				}
			}
		}
	}

Error_Level_01:
// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
// quiet		__FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrCopyParam()

Purpose:		Stores source parameter values to destination parameter values .

Description:	Stores source parameter values to destination parameter values .

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
                EcrioSigMgrParamStruct** ppSrcParamSet - Source Parameters
OutPut:
                EcrioSigMgrParamStruct*** pppDestParamSet - Destination Parameters

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrCopyParam
(
	EcrioSigMgrStruct *pSigMgr,
	u_int16 paramCount,
	EcrioSigMgrParamStruct **ppSrcParamSet,
	EcrioSigMgrParamStruct ***pppDestParamSet
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrParamStruct **ppTempParamSet = NULL;
	u_int32 i = 0;

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
// quiet		__FUNCTION__, __LINE__);

	if ((ppSrcParamSet == NULL) || (pppDestParamSet == NULL))
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;

		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data passed, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_02;
	}

	pal_MemoryAllocate(paramCount * sizeof(EcrioSigMgrParamStruct *), (void **)&ppTempParamSet);
	if (NULL == ppTempParamSet)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_02;
	}

	for (i = 0; i < paramCount; ++i)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&(ppTempParamSet[i]));
		if (ppTempParamSet[i] == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() failed",
				__FUNCTION__, __LINE__);

			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_02;
		}

		if (ppSrcParamSet[i]->pParamName != NULL)
		{
			error = _EcrioSigMgrStringCreate(pSigMgr, ppSrcParamSet[i]->pParamName,
				&(ppTempParamSet[i]->pParamName));
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_02;
			}
		}
		else
		{
			ppTempParamSet[i]->pParamName = NULL;
		}

		if (ppSrcParamSet[i]->pParamValue != NULL)
		{
			error = _EcrioSigMgrStringCreate(pSigMgr, ppSrcParamSet[i]->pParamValue,
				&(ppTempParamSet[i]->pParamValue));
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_02;
			}
		}
		else
		{
			ppTempParamSet[i]->pParamValue = NULL;
		}
	}

	*pppDestParamSet = ppTempParamSet;
Error_Level_02:
	if ((error != ECRIO_SIGMGR_NO_ERROR) && (ppTempParamSet != NULL))
	{
		for (i = 0; i < paramCount; ++i)
		{
			EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_Params,
				(void **)&ppTempParamSet[i], Enum_TRUE);
		}

		pal_MemoryFree((void **)&ppTempParamSet);
	}

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
// quiet		__FUNCTION__, __LINE__);

	return error;
}

u_int32 _EcrioSigMgrAllocateAndPopulateUri
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUriStruct *pSrcUri,
	EcrioSigMgrUriStruct **ppDstUri
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pal_MemoryAllocate(sizeof(EcrioSigMgrUriStruct), (void **)ppDstUri);
	if (*ppDstUri == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto EndTag;
	}

	error = _EcrioSigMgrPopulateUri(pSigMgr, pSrcUri, *ppDstUri);

EndTag:

	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		pal_MemoryFree((void **)ppDstUri);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrPopulateUri()

Purpose:		Stores uri from source uri to destination uri.

Description:	Stores uri from source uri to destination uri.

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
                EcrioSigMgrUriStruct* pSrcUri - Source URI.
                EcrioSigMgrUriStruct* pDstUri - Destination URI.

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrPopulateUri
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUriStruct *pSrcUri,
	EcrioSigMgrUriStruct *pDstUri
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
// quiet		__FUNCTION__, __LINE__);

	/* Store uri scheme */
	pDstUri->uriScheme = pSrcUri->uriScheme;

	/* Store uri */
	if (pSrcUri->uriScheme == EcrioSigMgrURISchemeSIP)
	{
		EcrioSigMgrSipURIStruct *pSrcSipUri = pSrcUri->u.pSipUri;
		EcrioSigMgrSipURIStruct *pDstSipUri = NULL;

		if (pSrcSipUri == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tInsufficient data passed", __FUNCTION__, __LINE__);

			error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
			goto Error_Level_01;
		}

		pal_MemoryAllocate(sizeof(EcrioSigMgrSipURIStruct), (void **)&pDstSipUri);
		if (pDstSipUri == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() failed", __FUNCTION__, __LINE__);

			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		error = _EcrioSigMgrPopulateSipUri(pSigMgr, pSrcSipUri, pDstSipUri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			if (pDstSipUri->pPassword)
			{
				pal_MemoryFree((void **)&pDstSipUri->pPassword);
			}
			if (pDstSipUri)
			{
				pal_MemoryFree((void **)&pDstSipUri);
			}
			goto Error_Level_01;
		}

		pDstUri->u.pSipUri = pDstSipUri;
		pDstSipUri = NULL;
	}
	else if (pSrcUri->uriScheme == EcrioSigMgrURISchemeTEL)
	{
		EcrioSigMgrTelURIStruct *pSrcTelUri = pSrcUri->u.pTelUri;
		EcrioSigMgrTelURIStruct *pDstTelUri = NULL;

		if (pSrcTelUri == NULL)
		{
			error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
			goto Error_Level_01;
		}

		pal_MemoryAllocate(sizeof(EcrioSigMgrTelURIStruct), (void **)&pDstTelUri);
		if (pDstTelUri == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() failed", __FUNCTION__, __LINE__);

			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		error = _EcrioSigMgrPopulateTelUri(pSigMgr, pSrcTelUri, pDstTelUri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			pal_MemoryFree((void **)&pDstTelUri);
			goto Error_Level_01;
		}

		pDstUri->u.pTelUri = pDstTelUri;
		pDstTelUri = NULL;
	}
	else if (pSrcUri->uriScheme == EcrioSigMgrURISchemeCustom)
	{
		if (pSrcUri->u.pAbsUri == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tInsufficient data passed", __FUNCTION__, __LINE__);

			error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
			goto Error_Level_01;
		}

		pDstUri->u.pAbsUri = NULL;
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcUri->u.pAbsUri, &pDstUri->u.pAbsUri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}
	else
	{
		error = ECRIO_SIGMGR_INVALID_URI_SCHEME;
	}

	error = ECRIO_SIGMGR_NO_ERROR;

Error_Level_01:
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		_EcrioSigMgrReleaseUriStruct(pSigMgr, pDstUri);
	}

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
// quiet		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrPopulateTelParams
(
	EcrioSigMgrStruct *pSigMgr,
	u_int16	srcNumTelParams,
	EcrioSigMgrTelParStruct **ppSrcTelParams,
	u_int16	*pDstNumTelParams,
	EcrioSigMgrTelParStruct ***pppDstTelParams
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 i = 0;
	EcrioSigMgrTelParStruct **ppDstTelParams = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
	if (pal_UtilityDataOverflowDetected(srcNumTelParams, sizeof(EcrioSigMgrTelParStruct *)) == Enum_TRUE)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error;
	}

	pal_MemoryAllocate(srcNumTelParams * sizeof(EcrioSigMgrTelParStruct *), (void **)&ppDstTelParams);
	if (ppDstTelParams == NULL)
	{
		goto Error;
	}

	for (i = 0; i < srcNumTelParams; i++)
	{
		pal_MemoryAllocate(srcNumTelParams * sizeof(EcrioSigMgrTelParStruct), (void **)&ppDstTelParams[i]);
		if (ppDstTelParams[i] == NULL)
		{
			goto Error;
		}

		ppDstTelParams[i]->telParType = ppSrcTelParams[i]->telParType;
		if (ppSrcTelParams[i]->telParType == EcrioSigMgrTelParExtension)
		{
			error = _EcrioSigMgrStringCreate(pSigMgr, ppSrcTelParams[i]->u.pExtension,
				&(ppDstTelParams[i]->u.pExtension));
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error;
			}
		}
		else if (ppSrcTelParams[i]->telParType == EcrioSigMgrTelParIsdnSubAddress)
		{
			error = _EcrioSigMgrStringCreate(pSigMgr, ppSrcTelParams[i]->u.pIsdnSubaddress,
				&(ppDstTelParams[i]->u.pIsdnSubaddress));
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error;
			}
		}
		else if (ppSrcTelParams[i]->telParType == EcrioSigMgrTelParParameter)
		{
			if (ppSrcTelParams[i]->u.pParameter)
			{
				pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&ppDstTelParams[i]->u.pParameter);
				if (ppDstTelParams[i]->u.pParameter == NULL)
				{
					goto Error;
				}

				if (ppSrcTelParams[i]->u.pParameter->pParamName)
				{
					error = _EcrioSigMgrStringCreate(pSigMgr, ppSrcTelParams[i]->u.pParameter->pParamName,
						&(ppDstTelParams[i]->u.pParameter->pParamName));
					if (error != ECRIO_SIGMGR_NO_ERROR)
					{
						goto Error;
					}
				}

				if (ppSrcTelParams[i]->u.pParameter->pParamValue)
				{
					error = _EcrioSigMgrStringCreate(pSigMgr, ppSrcTelParams[i]->u.pParameter->pParamValue,
						&(ppDstTelParams[i]->u.pParameter->pParamValue));
					if (error != ECRIO_SIGMGR_NO_ERROR)
					{
						goto Error;
					}
				}
			}
		}
	}

	*pDstNumTelParams = srcNumTelParams;
	*pppDstTelParams = ppDstTelParams;
	goto End;
Error:
	for (i = 0; i < srcNumTelParams; i++)
	{
		if (ppDstTelParams != NULL)
		{
			if (ppDstTelParams[i] != NULL)
			{
				if (ppDstTelParams[i]->u.pExtension != NULL)
				{
					pal_MemoryFree((void**) &ppDstTelParams[i]->u.pExtension);
				}
				else if (ppDstTelParams[i]->u.pIsdnSubaddress != NULL)
				{
					pal_MemoryFree((void**)&ppDstTelParams[i]->u.pIsdnSubaddress);
				}
				else if (ppDstTelParams[i]->u.pParameter != NULL)
				{
					if (ppDstTelParams[i]->u.pParameter->pParamName != NULL)
						pal_MemoryFree((void**)&ppDstTelParams[i]->u.pParameter->pParamName);

					if (ppDstTelParams[i]->u.pParameter->pParamValue != NULL)
						pal_MemoryFree((void**)&ppDstTelParams[i]->u.pParameter->pParamValue);
				}
				pal_MemoryFree((void**)&ppDstTelParams[i]);
			}
			pal_MemoryFree((void**)&ppDstTelParams);
		}
	}
End:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u error:%d",
		__FUNCTION__, __LINE__, error);
	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrPopulateTelUri()

Purpose:		Stores SIP URI values from source SIP URI to destination SIP URI.

Description:	Stores SIP URI header values from source SIP URI to destination SIP URI

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
EcrioSigMgrTelURIStruct* pSrcSipUri - Source SIP URI
EcrioSigMgrTelURIStruct* pDstSipUri - Destination SIP URI.

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrPopulateTelUri
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrTelURIStruct *pSrcTelUri,
	EcrioSigMgrTelURIStruct *pDstTelUri
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
		"%s:%u\tpSigMgr=%p, pSrcSipUri=%p, pDstUri=%p",
		__FUNCTION__, __LINE__, pSigMgr, pSrcTelUri, pDstTelUri);

	pDstTelUri->subscriberType = pSrcTelUri->subscriberType;
	if (pSrcTelUri->subscriberType == EcrioSigMgrTelSubscriberGlobal)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrTelGlobalNumberStruct), (void **)&pDstTelUri->u.pGlobalNumber);
		if (pDstTelUri->u.pGlobalNumber == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t memory error pDstTelUri->u.pGlobalNumber empty",
				__FUNCTION__, __LINE__);
			goto Error_Level_01;
		}

		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcTelUri->u.pGlobalNumber->pGlobalNumberDigits,
			&(pDstTelUri->u.pGlobalNumber->pGlobalNumberDigits));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t memory allocate and copy failed with error code:%d",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		if (pSrcTelUri->u.pGlobalNumber->numTelPar && pSrcTelUri->u.pGlobalNumber->ppTelPar)
		{
			error = _EcrioSigMgrPopulateTelParams(pSigMgr, pSrcTelUri->u.pGlobalNumber->numTelPar, pSrcTelUri->u.pGlobalNumber->ppTelPar,
				&pDstTelUri->u.pGlobalNumber->numTelPar, &pDstTelUri->u.pGlobalNumber->ppTelPar);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error_Level_01;
			}
		}
	}
	else if (pSrcTelUri->subscriberType == EcrioSigMgrTelSubscriberLocal)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrTelLocalNumberStruct), (void **)&pDstTelUri->u.pLocalNumber);
		if (pDstTelUri->u.pLocalNumber == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcTelUri->u.pLocalNumber->pLocalNumberDigits,
			&(pDstTelUri->u.pLocalNumber->pLocalNumberDigits));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}

		if (pSrcTelUri->u.pLocalNumber->numTelPar1 && pSrcTelUri->u.pLocalNumber->ppTelPar1)
		{
			error = _EcrioSigMgrPopulateTelParams(pSigMgr, pSrcTelUri->u.pLocalNumber->numTelPar1, pSrcTelUri->u.pLocalNumber->ppTelPar1,
				&pDstTelUri->u.pLocalNumber->numTelPar1, &pDstTelUri->u.pLocalNumber->ppTelPar1);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error_Level_01;
			}
		}

		if (pSrcTelUri->u.pLocalNumber->pContext)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrTelContextStruct), (void **)&pDstTelUri->u.pLocalNumber->pContext);
			if (pDstTelUri->u.pLocalNumber->pContext == NULL)
			{
				goto Error_Level_01;
			}

			pDstTelUri->u.pLocalNumber->pContext->contextType = pSrcTelUri->u.pLocalNumber->pContext->contextType;
			if (pSrcTelUri->u.pLocalNumber->pContext->u.pDomainName)
			{
				error = _EcrioSigMgrStringCreate(pSigMgr, pSrcTelUri->u.pLocalNumber->pContext->u.pDomainName,
					&(pDstTelUri->u.pLocalNumber->pContext->u.pDomainName));// work for both domain name and pGlobalNoDigitsInContext
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					goto Error_Level_01;
				}
			}
		}

		if (pSrcTelUri->u.pLocalNumber->numTelPar2 && pSrcTelUri->u.pLocalNumber->ppTelPar2)
		{
			error = _EcrioSigMgrPopulateTelParams(pSigMgr, pSrcTelUri->u.pLocalNumber->numTelPar2, pSrcTelUri->u.pLocalNumber->ppTelPar2,
				&pDstTelUri->u.pLocalNumber->numTelPar2, &pDstTelUri->u.pLocalNumber->ppTelPar2);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error_Level_01;
			}
		}
	}

Error_Level_01:
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		_EcrioSigMgrReleaseTelUriStruct(pSigMgr, pDstTelUri);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrPopulateSipUri()

Purpose:		Stores SIP URI values from source SIP URI to destination SIP URI.

Description:	Stores SIP URI header values from source SIP URI to destination SIP URI

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
                EcrioSigMgrSipURIStruct* pSrcSipUri - Source SIP URI
                EcrioSigMgrSipURIStruct* pDstSipUri - Destination SIP URI.

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrPopulateSipUri
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipURIStruct *pSrcSipUri,
	EcrioSigMgrSipURIStruct *pDstSipUri
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 i = 0;

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
// quiet		__FUNCTION__, __LINE__);

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
// quiet		"%s:%u\tpSigMgr=%p, pSrcSipUri=%p, pDstUri=%p",
// quiet		__FUNCTION__, __LINE__, pSigMgr, pSrcSipUri, pDstSipUri);

	/* store user id */
	if (pSrcSipUri->pUserId != NULL)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcSipUri->pUserId,
			&(pDstSipUri->pUserId));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	/* store password */
	if (pSrcSipUri->pPassword != NULL)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcSipUri->pPassword,
			&(pDstSipUri->pPassword));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	/* store domain */
	if (pSrcSipUri->pDomain != NULL)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcSipUri->pDomain,
			&(pDstSipUri->pDomain));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	/* Store ip address */
	if ((pSrcSipUri->pIPAddr != NULL) && (pSrcSipUri->pIPAddr->pIPAddr != NULL))
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrIPAddrStruct), (void **)&pDstSipUri->pIPAddr);
		if (pDstSipUri->pIPAddr == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pDstSipUri->pIPAddr->port = pSrcSipUri->pIPAddr->port;
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcSipUri->pIPAddr->pIPAddr,
			&(pDstSipUri->pIPAddr->pIPAddr));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	/* Stores URI parameters */
	if (pSrcSipUri->numURIParams > 0)
	{
		pDstSipUri->numURIParams = pSrcSipUri->numURIParams;

		/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
		if (pal_UtilityDataOverflowDetected(pDstSipUri->numURIParams, sizeof(EcrioSigMgrParamStruct *)) == Enum_TRUE)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pal_MemoryAllocate(pDstSipUri->numURIParams * sizeof(EcrioSigMgrParamStruct *), (void **)&pDstSipUri->ppURIParams);
		if (pDstSipUri->ppURIParams == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		for (i = 0; i < pDstSipUri->numURIParams; ++i)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&pDstSipUri->ppURIParams[i]);
			if (pDstSipUri->ppURIParams[i] == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pDstSipUri->ppURIParams[i]->pParamName = NULL;
			pDstSipUri->ppURIParams[i]->pParamValue = NULL;
			if (pSrcSipUri->ppURIParams[i]->pParamName != NULL)
			{
				error = _EcrioSigMgrStringCreate(pSigMgr, pSrcSipUri->ppURIParams[i]->pParamName,
					&(pDstSipUri->ppURIParams[i]->pParamName));
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					goto Error_Level_01;
				}
			}

			if (pSrcSipUri->ppURIParams[i]->pParamValue != NULL)
			{
				error = _EcrioSigMgrStringCreate(pSigMgr, pSrcSipUri->ppURIParams[i]->pParamValue,
					&(pDstSipUri->ppURIParams[i]->pParamValue));
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					goto Error_Level_01;
				}
			}
		}
	}

	/* store URI headers */
	if (pSrcSipUri->numURIHeaders > 0)
	{
		pDstSipUri->numURIHeaders = pSrcSipUri->numURIHeaders;

		/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
		if (pal_UtilityDataOverflowDetected(pDstSipUri->numURIHeaders, sizeof(EcrioSigMgrParamStruct *)) == Enum_TRUE)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pal_MemoryAllocate(pDstSipUri->numURIHeaders * sizeof(EcrioSigMgrParamStruct *), (void **)&pDstSipUri->ppURIHeaders);
		if (pDstSipUri->ppURIHeaders == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		for (i = 0; i < pDstSipUri->numURIHeaders; ++i)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&pDstSipUri->ppURIHeaders[i]);
			if (pDstSipUri->ppURIHeaders[i] == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pDstSipUri->ppURIHeaders[i]->pParamName = NULL;
			pDstSipUri->ppURIHeaders[i]->pParamValue = NULL;
			if (pSrcSipUri->ppURIHeaders[i]->pParamName != NULL)
			{
				error = _EcrioSigMgrStringCreate(pSigMgr, pSrcSipUri->ppURIHeaders[i]->pParamName,
					&(pDstSipUri->ppURIHeaders[i]->pParamName));
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					goto Error_Level_01;
				}
			}

			if (pSrcSipUri->ppURIHeaders[i]->pParamValue != NULL)
			{
				error = _EcrioSigMgrStringCreate(pSigMgr, pSrcSipUri->ppURIHeaders[i]->pParamValue,
					&(pDstSipUri->ppURIHeaders[i]->pParamValue));
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					goto Error_Level_01;
				}
			}
		}
	}

Error_Level_01:
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		_EcrioSigMgrReleaseSipUriStruct(pSigMgr, pDstSipUri);
	}

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
// quiet		__FUNCTION__, __LINE__);

	return error;
}

u_int32 _EcrioSigMgrAllocateAndPopulateNameAddr
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrNameAddrStruct *pSrcNameAddr,
	EcrioSigMgrNameAddrStruct **ppDstNameAddr
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrStruct), (void **)ppDstNameAddr);
	if (*ppDstNameAddr == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto EndTag;
	}

	error = _EcrioSigMgrPopulateNameAddr(pSigMgr, pSrcNameAddr, *ppDstNameAddr);

EndTag:

	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		pal_MemoryFree((void **)ppDstNameAddr);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrPopulateNameAddr()

Purpose:		Stores Source NameAddr details to destination NameAddr details.

Description:	Stores Source NameAddr details to destination NameAddr details.

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
                EcrioSigMgrNameAddrStruct* pSrcNameAddr - Source NameAddr details.
                EcrioSigMgrNameAddrStruct* pDstNameAddr - Destination NameAddr details

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrPopulateNameAddr
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrNameAddrStruct *pSrcNameAddr,
	EcrioSigMgrNameAddrStruct *pDstNameAddr
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	/* Copy Display Name */
	if (pSrcNameAddr->pDisplayName)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcNameAddr->pDisplayName,
			&pDstNameAddr->pDisplayName);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	/* Copy Addr-spec */
	error = _EcrioSigMgrPopulateUri(pSigMgr, &pSrcNameAddr->addrSpec,
		&pDstNameAddr->addrSpec);

Error_Level_01:
	if ((error != ECRIO_SIGMGR_NO_ERROR) && (pDstNameAddr != NULL))
	{
		_EcrioSigMgrReleaseNameAddrStruct(pSigMgr, pDstNameAddr);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrAllocateAndPopulateNameAddrWithParams
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrNameAddrWithParamsStruct *pSrcNameAddrParams,
	EcrioSigMgrNameAddrWithParamsStruct **ppDstNameAddrParams
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrNameAddrWithParamsStruct *pDstNameAddrParams = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct), (void **)&pDstNameAddrParams);
	if (pDstNameAddrParams == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto EndTag;
	}

	error = _EcrioSigMgrPopulateNameAddrWithParams(pSigMgr, pSrcNameAddrParams, pDstNameAddrParams);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		goto EndTag;
	}

	*ppDstNameAddrParams = pDstNameAddrParams;
	pDstNameAddrParams = NULL;

EndTag:

	if (pDstNameAddrParams != NULL)
	{
		pal_MemoryFree((void **)&pDstNameAddrParams);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrPopulateNameAddrWithParams()

Purpose:		Stores Source NameAddr+Params details to destination.

Description:	Stores Source NameAddr+Params details to destination.

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
                EcrioSigMgrNameAddrWithParamsStruct* pSrcNameAddrParams -
                    Source NameAddr+Params details.
                EcrioSigMgrNameAddrWithParamsStruct* pDstNameAddrParams -
                    Destination NameAddr+Params details

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrPopulateNameAddrWithParams
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrNameAddrWithParamsStruct *pSrcNameAddrParams,
	EcrioSigMgrNameAddrWithParamsStruct *pDstNameAddrParams
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	/* Copy Name-Addr */
	error = _EcrioSigMgrPopulateNameAddr(pSigMgr, &pSrcNameAddrParams->nameAddr,
		&pDstNameAddrParams->nameAddr);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		goto Error_Level_01;
	}

	/* Copy Params */
	if (pSrcNameAddrParams->numParams > 0)
	{
		error = _EcrioSigMgrCopyParam(pSigMgr, pSrcNameAddrParams->numParams,
			pSrcNameAddrParams->ppParams, &pDstNameAddrParams->ppParams);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}

		pDstNameAddrParams->numParams = pSrcNameAddrParams->numParams;
	}

Error_Level_01:
	if ((error != ECRIO_SIGMGR_NO_ERROR) && (pDstNameAddrParams != NULL))
	{
		_EcrioSigMgrReleaseNameAddrWithParamsStruct(pSigMgr,
			pDstNameAddrParams);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrPopulateContact()

Purpose:		Stores uri from source uri to destination uri.

Description:	Stores uri from source uri to destination uri.

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
                EcrioSigMgrUriStruct* pSrcUri - Source URI.
                EcrioSigMgrUriStruct* pDstUri - Destination URI.

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrPopulateContact
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrContactStruct *pSrcUri,
	EcrioSigMgrContactStruct *pDstUri
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR, i = 0;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
		"%s:%u\tpSigMgr=%p, pSrcUri=%p, pDstUri=%p",
		__FUNCTION__, __LINE__, pSigMgr, pSrcUri, pDstUri);

	if (pSrcUri == NULL || pSrcUri->numContactUris < 1)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data passed", __FUNCTION__, __LINE__);

		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		return error;
	}

	pDstUri->numContactUris = pSrcUri->numContactUris;

	pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct *) * (pDstUri->numContactUris),
		(void **)&pDstUri->ppContactDetails);
	if (pDstUri->ppContactDetails == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() failed", __FUNCTION__, __LINE__);

		error = ECRIO_SIGMGR_NO_MEMORY;
		return error;
	}

	for (i = 0; i < pDstUri->numContactUris; i++)
	{
		error = _EcrioSigMgrAllocateAndPopulateNameAddrWithParams(pSigMgr, pSrcUri->ppContactDetails[i], &pDstUri->ppContactDetails[i]);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrPopulateNameAddrWithParams() error=%u",
				__FUNCTION__, __LINE__, error);

			goto Error_Level_01;
		}
	}

Error_Level_01:
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		_EcrioSigMgrReleaseContactStruct(pSigMgr, pDstUri);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrValidateUri ()

Purpose:		Validate URI.

Description:	Validation of Sip/Tel URI.

Input:		EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance
Input:		EcrioSigMgrUriStruct* pUri - URI info;
Output:		BoolEnum** bValid - Is valid or not.

Returns:	error code

**************************************************************************/
u_int32 _EcrioSigMgrValidateUri
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUriStruct *pUri,
	BoolEnum *bValid
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	(void)pSigMgr;

	*bValid = Enum_TRUE;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	switch (pUri->uriScheme)
	{
		case EcrioSigMgrURISchemeSIP:
		{
			if ((pUri->u.pSipUri == NULL) ||
				(pUri->u.pSipUri->pUserId == NULL))
			{
				*bValid = Enum_FALSE;
				goto Error_Level_01;
			}

			if ((pUri->u.pSipUri->pDomain == NULL) &&
				((pUri->u.pSipUri->pIPAddr == NULL) ||
				(pUri->u.pSipUri->pIPAddr->pIPAddr == NULL)))
			{
				*bValid = Enum_FALSE;
				goto Error_Level_01;
			}
		}
		break;

		case EcrioSigMgrURISchemeTEL:
		{
			if (pUri->u.pTelUri == NULL)
			{
				*bValid = Enum_FALSE;
				goto Error_Level_01;
			}

			if ((pUri->u.pTelUri->subscriberType == EcrioSigMgrTelSubscriberGlobal) &&
				((pUri->u.pTelUri->u.pGlobalNumber == NULL) ||
				(pUri->u.pTelUri->u.pGlobalNumber->pGlobalNumberDigits == NULL)))
			{
				*bValid = Enum_FALSE;
				goto Error_Level_01;
			}
			else if ((pUri->u.pTelUri->subscriberType == EcrioSigMgrTelSubscriberLocal) &&
				((pUri->u.pTelUri->u.pLocalNumber == NULL) ||
				(pUri->u.pTelUri->u.pLocalNumber->pLocalNumberDigits == NULL) ||
				(pUri->u.pTelUri->u.pLocalNumber->pContext == NULL) ||
				(pUri->u.pTelUri->u.pLocalNumber->pContext->u.pDomainName == NULL)))
			{
				*bValid = Enum_FALSE;
				goto Error_Level_01;
			}
		}
		break;

		case EcrioSigMgrURISchemeCustom:
		{
			if (pUri->u.pAbsUri == NULL)
			{
				*bValid = Enum_FALSE;
				goto Error_Level_01;
			}
		}
		break;

		default:
		{
			*bValid = Enum_FALSE;
		}
		break;
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return error;
}

/********************************************************************************************************************
Function:		_EcrioSigMgrCreateSessionId()

Purpose:		Create the session id value from the given Call-Id.

Description:	Create the session id value from the given Call-Id.

Input:			EcrioSigMgrStruct* pSigMgr - Handle to signaling manager.
                u_char* pCallId - pointer to the string of Call-Id.

Output:			u_char** ppSessionId - Session ID.

Returns:		error code.
***********************************************************************************************************************/

/**************************************************************************

Function:		EcrioSigMgrFormSipURI ()

Purpose:		to generate a SipURI.

Description:	The API will generate valid SIP URI using the data passed
                by caller

Input:			SIGMGRHANDLE sigMgrHandle - Handle to signaling manager
                EcrioSigMgrSipURIStruct* pSipUriInfo - Uri Info;
Output:			u_char** ppSipUri - SIP URI

Returns:		Error Code

**************************************************************************/
u_int32 EcrioSigMgrFormSipURI
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSipURIStruct *pSipUriInfo,
	u_char **ppSipUri
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrUriStruct uri = { .uriScheme = EcrioSigMgrURISchemeNone };

	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
		"%s:%u\tsigMgrHandle=%p, pSipUriInfo=%p",
		__FUNCTION__, __LINE__, sigMgrHandle, pSipUriInfo);

	if ((pSipUriInfo == NULL) || (ppSipUri == NULL))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInvalid data passed", __FUNCTION__, __LINE__);

		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	uri.uriScheme = EcrioSigMgrURISchemeSIP;
	uri.u.pSipUri = pSipUriInfo;

	error = _EcrioSigMgrFormUri(pSigMgr, &uri, ppSipUri);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrFormUri() error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return error;
}


/**************************************************************************

Function:		EcrioSigMgrFormTelURI ()

Purpose:		to generate a TelURI.

Description:	The API will generate valid TEL URI using the data passed
by caller

Input:			SIGMGRHANDLE sigMgrHandle - Handle to signaling manager
EcrioSigMgrTelURIStruct* pTelUri - Uri Info;
Output:			u_char** ppUri - TEL URI

Returns:		Error Code

**************************************************************************/
u_int32 EcrioSigMgrFormTelURI
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrTelURIStruct *pTelUri,
	u_char **ppUri
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrUriStruct uri = { .uriScheme = EcrioSigMgrURISchemeNone };

	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
		"%s:%u\tsigMgrHandle=%p, pSipUriInfo=%p",
		__FUNCTION__, __LINE__, sigMgrHandle, pTelUri);

	if ((pTelUri == NULL) || (ppUri == NULL))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInvalid data passed", __FUNCTION__, __LINE__);

		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	uri.uriScheme = EcrioSigMgrURISchemeTEL;
	uri.u.pTelUri = pTelUri;

	error = _EcrioSigMgrFormUri(pSigMgr, &uri, ppUri);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrFormUri() error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return error;
}




/********************************************************************************************************************
Function:		_EcrioSigMgrGetParamValByName()

Purpose:		Retrieve Named Parameter from Header structure.

Description:	Retrieve Named Parameter from Header Structure.

Input:			EcrioSigMgrStruct* pSigMgr - Handle to signaling manager.
                EcrioSigMgrParamStruct** ppParams - pointer to the array of pointers to the Parameter structure.
                u_int32 noOfParameters - Number of Parameters in the Structure.
                u_char* pParameterName - Parameter Nae whose value to be retrieved.

Output:			u_char** ppParameterVal - Parameter Value.

Returns:		error code.
***********************************************************************************************************************/
u_int32 _EcrioSigMgrGetParamValByName
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrParamStruct **ppParams,
	u_int16 noOfParameters,
	u_char *pParameterName,
	u_char **ppParameterVal
)
{
	u_int32	error = ECRIO_SIGMGR_PARAMETER_NOT_FOUND;
	u_int32 index = 0;

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
// quiet		__FUNCTION__, __LINE__);

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
// quiet		"%s:%u\tpSigMgr=%p, ppParams=%p, pParameterName=%p, ppParameterVal=%p",
// quiet		__FUNCTION__, __LINE__, pSigMgr, ppParams, pParameterName, ppParameterVal);

	if (ppParams != NULL)
	{
		for (index = 0; index < noOfParameters; ++index)
		{
			if (pal_StringICompare(ppParams[index]->pParamName, pParameterName) == 0)
			{
				error = _EcrioSigMgrStringCreate(pSigMgr, ppParams[index]->pParamValue, ppParameterVal);

				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrStringCreate() for parameter value, error=%u",
						__FUNCTION__, __LINE__, error);
				}

				break;
			}
		}
	}

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
// quiet		__FUNCTION__, __LINE__);

	return error;
}

/**************************************************************************
Function:		_EcrioSigMgrGetHeaderValue()

Purpose:		Retrieve Header from Header structure.

Description:	Retrieve Header Value from Header Structure.

Input:			EcrioSigMgrStruct* pSigMgr - Handle to signaling manager.
                EcrioSigMgrHeaderStruct* pHdrStruct - Header Structure.

Output:			u_char** ppHeaderVal - Header Value.

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrGetHeaderValue
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrHeaderStruct *pHdrStruct,
	u_char **ppHeaderVal
)
{
	u_int32	error = ECRIO_SIGMGR_HEADER_VAL_NOT_FOUND;
	u_int32	hdrValueIndex = 0;

	if ((pSigMgr == NULL) || (ppHeaderVal == NULL))
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
		"%s:%u\tpSigMgr=%p, pHdrStruct=%p, ppHeaderVal=%p",
		__FUNCTION__, __LINE__, pSigMgr, pHdrStruct, ppHeaderVal);

	if (pHdrStruct != NULL)
	{
		for (hdrValueIndex = 0; hdrValueIndex < pHdrStruct->numHeaderValues; ++hdrValueIndex)
		{
			if (pHdrStruct->ppHeaderValues != NULL)
			{
				if (pHdrStruct->ppHeaderValues[hdrValueIndex]->pHeaderValue != NULL)
				{
					error = _EcrioSigMgrStringCreate(pSigMgr, pHdrStruct->ppHeaderValues[hdrValueIndex]->pHeaderValue,
						ppHeaderVal);

					if (error != ECRIO_SIGMGR_NO_ERROR)
					{
						SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
							"%s:%u\t_EcrioSigMgrStringCreate() for Header Value, error=%u",
							__FUNCTION__, __LINE__, error);
					}

					goto Error_Level_01;
				}
			}
		}
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return error;
}

/**************************************************************************

Function:		EcrioSigMgrCopyHeader()

Purpose:		copy header struct from source to destination.

Description:	copy header struct from source to destination.

Input:			SIGMGRHANDLE* pSigMgrHandle - Handle to signaling manager.
                EcrioSigMgrHeaderStruct* pSrcHdrStruct - Header Structure.

Output:			EcrioSigMgrHeaderStruct* pDestHdrStruct - Header Structure.

Returns:		error code.
**************************************************************************/
u_int32 EcrioSigMgrCopyHeader
(
	SIGMGRHANDLE pSigMgrHandle,
	EcrioSigMgrHeaderStruct *pSrcHdrStruct,
	EcrioSigMgrHeaderStruct *pDestHdrStruct
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrHeaderStruct *pHdrStruct	= NULL;

	if (pSigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		goto Error_Level_01;
	}	/* if */

	pSigMgr = (EcrioSigMgrStruct *)pSigMgrHandle;
	if ((pDestHdrStruct == NULL) || (pSrcHdrStruct == NULL))
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_02;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
		"%s:%u\tpSigMgr=%p, pHdrStruct=%p, pDestHdrStruct=%p",
		__FUNCTION__, __LINE__, pSigMgr, pHdrStruct, pDestHdrStruct);

	error = _EcrioSigMgrAddHdr(pSigMgrHandle, pSrcHdrStruct, &pHdrStruct);

	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		goto Error_Level_02;
	}	/* if */

	pDestHdrStruct->pHeaderName = pHdrStruct->pHeaderName;
	pDestHdrStruct->numHeaderValues	= pHdrStruct->numHeaderValues;
	pDestHdrStruct->ppHeaderValues = pHdrStruct->ppHeaderValues;

Error_Level_02:
	if (pHdrStruct != NULL)
	{
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			EcrioSigMgrStructRelease(pSigMgrHandle, EcrioSigMgrStructType_Header, (void **)&pHdrStruct, Enum_TRUE);
		}
		else
		{
			pal_MemoryFree((void **)&pHdrStruct);
		}
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

Error_Level_01:
	return error;
}

/**************************************************************************
Function:		EcrioSigMgrCreateMatchingParam()

Purpose:		Create matching parameter.

Output:			u_char** ppMatchingParam - Parameters.

Returns:		error code.
**************************************************************************/
u_int32 EcrioSigMgrCreateMatchingParam
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pBranch,
	EcrioSigMgrHeaderStruct *pEventHeader,
	u_char **ppMatchingParam
)
{
	u_char *pEventName = NULL;
	u_char *pEventId = NULL;
	u_char *pMatchingParam = NULL;
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	error = _EcrioSigMgrStringCreate(pSigMgr, pBranch, &pMatchingParam);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		goto Error_Level_01;
	}

	error = _EcrioSigMgrStringCreate(pSigMgr, (u_char *)ECRIO_SIG_MGR_SIP_SEMICOLON_SYMBOL,
		&pMatchingParam);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		pal_MemoryFree((void **)&pMatchingParam);
		goto Error_Level_01;
	}

	_EcrioSigMgrGetHeaderValue(pSigMgr, pEventHeader, &pEventName);
	error = _EcrioSigMgrStringCreate(pSigMgr, pEventName, &pMatchingParam);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		pal_MemoryFree((void **)&pEventName);
		pal_MemoryFree((void **)&pMatchingParam);
		goto Error_Level_01;
	}

	error = _EcrioSigMgrStringCreate(pSigMgr, (u_char *)ECRIO_SIG_MGR_SIP_SEMICOLON_SYMBOL,
		&pMatchingParam);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		pal_MemoryFree((void **)&pEventName);
		pal_MemoryFree((void **)&pMatchingParam);
		goto Error_Level_01;
	}

	if ((pEventHeader != NULL) &&
		(pEventHeader->ppHeaderValues != NULL) &&
		(pEventHeader->ppHeaderValues[0] != NULL) &&
		(pEventHeader->ppHeaderValues[0]->numParams > 0))
	{
		_EcrioSigMgrGetParamValByName(pSigMgr, pEventHeader->ppHeaderValues[0]->ppParams,
			pEventHeader->ppHeaderValues[0]->numParams, (u_char *)ECRIO_SIG_MGR_ID_PARAM,
			&pEventId);
	}

	error = _EcrioSigMgrStringCreate(pSigMgr, pEventId, &pMatchingParam);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		pal_MemoryFree((void **)&pEventName);
		pal_MemoryFree((void **)&pEventId);
		pal_MemoryFree((void **)&pMatchingParam);
		goto Error_Level_01;
	}

	*ppMatchingParam = pMatchingParam;
	/* deallocating memory for local variable*/
	pal_MemoryFree((void **)&pEventName);
	pal_MemoryFree((void **)&pEventId);

Error_Level_01:
	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrAddServiceRoute

Purpose:		To add service-route to SigMgr

Description:	This API is used to set the pre-loaded route contained in
                the Service-Route header field of a REGISTER response, in
                Signaling Manager. It is invoked on receiving a 2xx final
                response for any outgoing Register request. It shall
                overwrite any existing Service-Route value if present due
                to prior registration.

Input:			EcrioSigMgrStruct* pSigMgr - signaling manager instance
                EcrioSigMgrRouteStruct* pServiceRoute - service route

Output:			None

Returns:		Error Code

**************************************************************************/
u_int32 _EcrioSigMgrAddServiceRoute
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrRouteStruct *pServiceRoute
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	(void)pSigMgr;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
		"%s:%u\tpSigMgr=%p, pServiceRoute=%p",
		__FUNCTION__, __LINE__, pSigMgr, pServiceRoute);

	if ((pServiceRoute == NULL) || !(pServiceRoute->numRoutes))
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;

		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data passed", __FUNCTION__, __LINE__);

		goto Error_Level_01;
	}

	error = ECRIO_SIGMGR_NO_ERROR;

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrClearServiceRoute

Purpose:		To reset existing service-route in SigMgr

Description:	This API is used to reset the Service-Route values present
                in Signaling Manager if any. It is invoked in the following
                scenarios:
                    a)	Registration failure [non-2xx final response].
                    b)	Registration failure [Timeout].
                    c)	On successful De-Registration.

Input:			EcrioSigMgrStruct* pSigMgr - signaling manager instance

Output:			None

Returns:		Error Code

**************************************************************************/
u_int32 _EcrioSigMgrClearServiceRoute
(
	EcrioSigMgrStruct *pSigMgr
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	if (pSigMgr == NULL)
	{
		return ECRIO_SIGMGR_BAD_HANDLE;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpSigMgr=%p",
		__FUNCTION__, __LINE__, pSigMgr);

	/* Release any existing service route values */
	if (pSigMgr->pServiceRoute)
	{
		(void)_EcrioSigMgrReleaseRouteStruct(pSigMgr, pSigMgr->pServiceRoute);
		pal_MemoryFree((void **)&pSigMgr->pServiceRoute);
	}

	error = ECRIO_SIGMGR_NO_ERROR;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return error;
}

/****************************************************************************************
Function:		_EcrioSigMgrCopyCredentials()

Purpose:		Populate the Authorization/Proxy-Authorization header from the Shared
                Credential structure

Description:	Internal API to Populate the Authorization/Proxy-Authorization header
                from the Shared	Credential structure. If the Shared Credentials is
                updated from 401/407 challenge response (see
                _EcrioSigMgrUpdateSharedCredentialsFrom401407Resp), then it is used as is;
                else the Shared Credential is updated with the stored credential in SUEExtn
                and then used.
                The authCount/proxyAuthcount is incremented by 1 if the Shared Credentials
                contains a non-empty nonce which is then used to populate the nc-parameter.
                It also computes the MD5 response parameter (as per 4fc 2617) per
                Authorization/Proxy-Authorization header.

Input:			EcrioSigMgrStruct* pSigMgr - Handle to signaling manager.
                u_char *pMethod - SIP Method Type of the Request
                u_char *pEntityBody - Message body if present in the Request. Required for
                  qop. (Future use).

Output:			u_int16	*pDestNumProxyAuth - number of Proxy-Authorization headers
                EcrioSigMgrAuthorizationStruct ***pppDestProxyAuth - pointer to list of
                  Proxy-Authorization headers
                EcrioSigMgrAuthorizationStruct **ppDestAuth - pointer to Authorization
                  header

Returns:		error code.
                    - ECRIO_SIGMGR_NO_ERROR on success

*****************************************************************************************/
u_int32 _EcrioSigMgrCopyCredentials
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pMethod,
	u_char *pEntityBody
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;

#ifndef ENABLE_QCMAPI
	if ((pSigMgr == NULL) || (pSigMgr->pSharedCredentials == NULL))
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	/* If Shared Credentials is updated from 401/407 Challenge response, use it */
	if (pSigMgr->pSharedCredentials->bUpdatedFromChallengedResp == Enum_TRUE)
	{
		/* reset flag to indicate that the credentials have to be fetched from SUEExtn
		** for subsequent usage.
		*/
		pSigMgr->pSharedCredentials->bUpdatedFromChallengedResp = Enum_FALSE;
	}

	if (pSigMgr->pSharedCredentials->pAuth != NULL)
	{
		/* Increment the auth count by 1 if the nonce is non-empty */
		if (pSigMgr->pSharedCredentials->pAuth->pNonce && pal_StringLength(pSigMgr->pSharedCredentials->pAuth->pNonce))
		{
			pSigMgr->pSharedCredentials->authCount++;
			/* Update the nc parameter in the Authorization header based on the authCount value */
			if (pSigMgr->pSharedCredentials->authCount)
			{
				error = _EcrioSigMgrUpdateRequestNonceCount(pSigMgr,
					pSigMgr->pSharedCredentials->pAuth,
					pSigMgr->pSharedCredentials->authCount);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					goto EndTag;
				}
			}

			/* Compute and store the MD5 response parameter (as per rfc 2617) in the Authorization header */
			error = _EcrioSigMgrUpdateAuthResponse(pSigMgr, pSigMgr->pSharedCredentials->pAuth,
				pMethod, pEntityBody, pSigMgr->pSharedCredentials->pAuth->pAKARes);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto EndTag;
			}
		}
	}

EndTag:
#endif

	return error;
}

/****************************************************************************************
Function:		_EcrioSigMgrUpdateSharedCredentialsFrom401407Resp()

Purpose:		Store the Authentication challenge received in the 401/407 response

Description:	Internal API to update the UA Shared Credentials structure with the
                Authentication challenge information received in the incoming 401/407
                response.
                If AKA is the digest algorithm, then it will also fetch and	store the
                AKA response from PAL.
                It sets a flag to indicate that the Shared Credentials has been
                updated with the challege response and should be used while resending
                the request with credentials.
                It will reset the authCount or proxyAuthcount to 0.

Input:			EcrioSigMgrStruct* pSigMgr - Handle to signaling manager.
                u_int16	srcNumProxyAuth - number of Proxy-Authentication header present
                  in challenge
                EcrioSigMgrAuthenticationStruct	**ppSrcProxyAuth - list of
                  Proxy-Authentication headers present in 407 challenge response
                EcrioSigMgrAuthenticationStruct	*pSrcAuth - WWW-Authenticate header
                  present in 401 challenge response
                u_char *pRequestUri - Request URI of the Request that is challenged.
                u_char *pEntityBody - Message Body if present in the Request for qop.
                  (future use)
                EcrioSigMgrMethodTypeEnum eMethod - SIP Method type of the Request that
                  is challenged.
                u_int32	responseCode - whether 401 or 407 SIP challenge response

Output:			None.

Returns:		error code.
                    - ECRIO_SIGMGR_NO_ERROR on success

*****************************************************************************************/
u_int32	_EcrioSigMgrUpdateSharedCredentialsFrom401407Resp
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrAuthenticationStruct	*pSrcAuth,
	u_char *pRequestUri,
	u_char *pEntityBody,
	EcrioSigMgrMethodTypeEnum eMethod,
	u_int32	responseCode
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrSharedCredentialsStruct *pSharedCredentials = NULL;

	if (!pSigMgr)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		goto EndTag;
	}

	/* For 401 response, Authentication information MUST NOT be NULL */
	if (responseCode == ECRIO_SIGMGR_RESPONSE_CODE_UNAUTHORIZED)
	{
		if (!pSrcAuth)
		{
			goto EndTag;
		}
	}

	pSharedCredentials = pSigMgr->pSharedCredentials;

	// if(pSrcAuth)
	if (pSrcAuth)// && (eMethod == EcrioSigMgrMethodRegister))
	{
		/* Retrieve and store the new Authentication challenge received in 401 response */
		error = _EcrioSigMgrGetAuthCredential(pSigMgr,
			pSrcAuth,
			0,											// check it
			pRequestUri,
			pEntityBody,
			eMethod,
			&(pSharedCredentials->pAuth)
			);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto EndTag;
		}
		else
		{
			/* If the digest algorithm is AKA, then fetch and store the AKA response
			** (RES/AUTS, CK and IK) using PAL.
			*/
			/* Support AKAv1 and AKAv2 using configuration */
			/* Check MAC error before proceeding further */
			if (EcrioSipAuthAlgorithmAKAv1 == pSharedCredentials->pAuth->authAlgo ||
				EcrioSipAuthAlgorithmAKAv2 == pSharedCredentials->pAuth->authAlgo)
			{
				error = _EcrioSigMgrGetAKARes(pSigMgr,
					pSharedCredentials->pAuth->pNonce,
					pSharedCredentials->pAuth->authAlgo,
					&pSharedCredentials->pAuth->pAKARes);
				if (error != ECRIO_SIGMGR_NO_ERROR && error != ECRIO_SIGMGR_AKA_MAC_FAILURE)
				{
					goto EndTag;
				}
			}

			pSigMgr->pSharedCredentials->authCount = 0;
		}

		pSharedCredentials->bUpdatedFromChallengedResp = Enum_TRUE;
	}

EndTag:

	return error;
}

/*
Internal function to set Received Channel Index and the port
through which the server sent the message. */
u_int32 _EcrioSigMgrSetReceivedOnChannelIndexAndRemoteSendingPort
(
	void *sigMgrHandle,
	u_int32 channelId,
	u_int32	remotePort
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR, i = 0;
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioTxnMgrSignalingCommunicationStruct	*pCommStruct = NULL;
	BoolEnum bFound = Enum_FALSE;

	if (sigMgrHandle == NULL)
	{
		return ECRIO_SIGMGR_BAD_HANDLE;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
// quiet		__FUNCTION__, __LINE__);

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
// quiet		"%s:%u\tsigMgrHandle=%p, channelId = %d",
// quiet		__FUNCTION__, __LINE__, sigMgrHandle, channelId);

	if (NULL == pSigMgr->pSigMgrTransportStruct)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto EndTag;
	}

	/*	Check whether a channel with same ID and on which data can be sent out
	is present in the existing set if found and set the channel ID in the
	appropriate data structures.
	*/

	/*if(channelId > pSigMgr->pSigMgrSACStruct->pCommunicationStruct->noOfChannels)
	{
	pCommStruct = pSigMgr->pSigMgrSACStruct->pEmergencyCommStruct;
	pSigMgr->eActivePDNType = EcrioSigMgrPDNTypeEmergency;
	}
	else
	{
	pCommStruct = pSigMgr->pSigMgrSACStruct->pCommunicationStruct;
	pSigMgr->eActivePDNType = EcrioSigMgrPDNTypeIMS;
	}*/

	if (pSigMgr->pSigMgrTransportStruct->pCommunicationStruct == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tCommunication structure is NULL",
			__FUNCTION__, __LINE__);

		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto EndTag;
	}

	pCommStruct = pSigMgr->pSigMgrTransportStruct->pCommunicationStruct;
	if (pCommStruct != NULL)
	{
		for (i = 0; i < pCommStruct->noOfChannels; i++)
		{
			if (pCommStruct->pChannels[i].channelIndex == channelId)
			{
				bFound = Enum_TRUE;
				break;
			}
		}
	}

	if (bFound == Enum_FALSE)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tFailed as there is NO MATCHING Channel with the same channel Index and//or on which data can be sent out!",
			__FUNCTION__, __LINE__);

		error = ECRIO_SIGMGR_SAC_INVALID_CHANNEL;
		goto EndTag;
	}

	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tFound channel match: channelId: %d, index:%d remoteSendingPort:%u",
		__FUNCTION__, __LINE__, channelId, i, remotePort);

	pSigMgr->pSigMgrTransportStruct->receivedOnChannelIndex = channelId;
	pSigMgr->pSigMgrTransportStruct->remoteSendingPort = remotePort;
	pSigMgr->pSigMgrTransportStruct->bIPsecEnabled = pSigMgr->bIPSecEnabled;

	/*	Set the TXN Manager data. */
	error = EcrioTXNSetTransportData(pSigMgr->pTransactionMgrHndl, pSigMgr->pSigMgrTransportStruct);
	if (error != EcrioTXNNoError)
	{
		error = ECRIO_SIGMGR_TRANSACTIONMGR_ERROR;
		goto EndTag;
	}

EndTag:
// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
// quiet		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* Check expires parameter in Contact details structure.
*/
u_int32 _EcrioSigMgrFindExpiryInNameAddrStuct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrNameAddrWithParamsStruct *pContactDetails,
	BoolEnum *pIsHdrParam,
	BoolEnum *pIsZeroValue,
	BoolEnum *pExpireFound,
	u_int32	*pExpiresVal
)
{
	u_int32	i = 0, j = 0, error = ECRIO_SIGMGR_NO_ERROR;

	pSigMgr = pSigMgr;
	*pExpireFound = Enum_FALSE;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	for (i = 0; i < pContactDetails->numParams; i++)
	{
		if (0 == (pal_StringICompare(pContactDetails->ppParams[i]->pParamName,
			(u_char *)ECRIO_SIG_MGR_EXPIRES_PARAM)))
		{
			if (0 == (pal_StringICompare(pContactDetails->ppParams[i]->pParamValue,
				(u_char *)ECRIO_SIG_MGR_SIP_ZERO_STRING)))
			{
				*pIsZeroValue = Enum_TRUE;
			}
			else
			{
				*pExpiresVal = pal_StringToNum(pContactDetails->ppParams[i]->pParamValue, NULL);
			}

			*pIsHdrParam = Enum_TRUE;
			*pExpireFound = Enum_TRUE;
			goto End_Tag;
		}
	}

	if (pContactDetails->nameAddr.addrSpec.uriScheme == EcrioSigMgrURISchemeSIP)
	{
		if (pContactDetails->nameAddr.addrSpec.u.pSipUri)
		{
			for (j = 0; j < pContactDetails->nameAddr.addrSpec.u.pSipUri->numURIParams; j++)
			{
				if (0 == (pal_StringICompare(pContactDetails->nameAddr.addrSpec.u.pSipUri->ppURIParams[j]->pParamName,
					(u_char *)ECRIO_SIG_MGR_EXPIRES_PARAM)))
				{
					if (0 == (pal_StringICompare(pContactDetails->nameAddr.addrSpec.u.pSipUri->ppURIParams[j]->pParamValue,
						(u_char *)ECRIO_SIG_MGR_SIP_ZERO_STRING)))
					{
						*pIsZeroValue = Enum_TRUE;
					}
					else
					{
						*pExpiresVal = pal_StringToNum(pContactDetails->nameAddr.addrSpec.u.pSipUri->ppURIParams[j]->pParamValue, NULL);
					}

					*pIsHdrParam = Enum_FALSE;
					*pExpireFound = Enum_TRUE;
					goto End_Tag;
				}
			}
		}
	}

End_Tag:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**
* Add expires parameter to Contact header.
*/
u_int32 _EcrioSigMgrAddExpiryParam
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pUserName,
	BoolEnum bIsHdrParam,
	EcrioSigMgrRegisterContactInfoStruct *pContactHdrs
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrSipURIStruct *pSipUri = NULL;
	u_char *pParamName = NULL, *pParamValue = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pContactHdrs->pUserContactInfo->nameAddr.addrSpec.uriScheme == EcrioSigMgrURISchemeSIP)
	{
		if (bIsHdrParam)
		{
			pContactHdrs->pUserContactInfo->numParams += 1;
			pSipUri = pContactHdrs->pUserContactInfo->nameAddr.addrSpec.u.pSipUri;
			if (!pSipUri)
			{
				error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
				goto Error_Level_01;
			}

			if (pUserName)
			{
				error = _EcrioSigMgrStringCreate(pSigMgr, pUserName,
					&(pSipUri->pUserId));
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					goto Error_Level_01;
				}
			}

			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct *), (void **)&(pContactHdrs->pUserContactInfo->ppParams));
			if (pContactHdrs->pUserContactInfo->ppParams == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&(pContactHdrs->pUserContactInfo->ppParams[0]));
			if (pContactHdrs->pUserContactInfo->ppParams[0] == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pal_MemorySet(pContactHdrs->pUserContactInfo->ppParams[0], 0, sizeof(EcrioSigMgrParamStruct));

			pal_MemoryAllocate(pal_StringLength((u_char *)ECRIO_SIG_MGR_EXPIRES_PARAM) + 1, (void **)&(pContactHdrs->pUserContactInfo->ppParams[0]->pParamName));
			if (pContactHdrs->pUserContactInfo->ppParams[0]->pParamName == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pal_MemoryAllocate(pal_StringLength((u_char *)ECRIO_SIG_MGR_SIP_ZERO_STRING) + 1, (void **)&(pContactHdrs->pUserContactInfo->ppParams[0]->pParamValue));
			if (pContactHdrs->pUserContactInfo->ppParams[0]->pParamValue == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pParamName = pContactHdrs->pUserContactInfo->ppParams[0]->pParamName;
			pParamValue = pContactHdrs->pUserContactInfo->ppParams[0]->pParamValue;
		}
		else/*Expiry will be added in the SIP URI parameter*/
		{
			pSipUri = pContactHdrs->pUserContactInfo->nameAddr.addrSpec.u.pSipUri;
			if (!pSipUri)
			{
				error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
				goto Error_Level_01;
			}

			pSipUri->numURIParams = 1;
			if (pUserName)
			{
				error = _EcrioSigMgrStringCreate(pSigMgr, pUserName,
					&(pSipUri->pUserId));
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					goto Error_Level_01;
				}
			}

			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct *), (void **)&(pSipUri->ppURIParams));
			if (pSipUri->ppURIParams == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&(pSipUri->ppURIParams[0]));
			if (pSipUri->ppURIParams[0] == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pal_MemorySet(pSipUri->ppURIParams[0], 0, sizeof(EcrioSigMgrParamStruct));
			pal_MemoryAllocate(pal_StringLength((u_char *)ECRIO_SIG_MGR_EXPIRES_PARAM) + 1, (void **)&(pSipUri->ppURIParams[0]->pParamName));
			if (pSipUri->ppURIParams[0]->pParamName == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pal_MemoryAllocate(pal_StringLength((u_char *)ECRIO_SIG_MGR_SIP_ZERO_STRING) + 1, (void **)&(pSipUri->ppURIParams[0]->pParamValue));
			if (pSipUri->ppURIParams[0]->pParamValue == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pParamName = pSipUri->ppURIParams[0]->pParamName;
			pParamValue = pSipUri->ppURIParams[0]->pParamValue;
		}

		if (NULL == pal_StringNCopy(pParamName, pal_StringLength((u_char *)ECRIO_SIG_MGR_EXPIRES_PARAM) + 1, (u_char *)ECRIO_SIG_MGR_EXPIRES_PARAM, pal_StringLength((u_char *)ECRIO_SIG_MGR_EXPIRES_PARAM)))
		{
			pal_MemoryFree((void **)&pParamName);
			pal_MemoryFree((void **)&pParamValue);
			error = ECRIO_SIGMGR_STRING_ERROR;
			goto Error_Level_01;
		}
		if (NULL == pal_StringNCopy(pParamValue, pal_StringLength((u_char *)ECRIO_SIG_MGR_SIP_ZERO_STRING) + 1, (u_char *)ECRIO_SIG_MGR_SIP_ZERO_STRING, pal_StringLength((u_char *)ECRIO_SIG_MGR_SIP_ZERO_STRING)))
		{
			pal_MemoryFree((void **)&pParamName);
			pal_MemoryFree((void **)&pParamValue);
			error = ECRIO_SIGMGR_STRING_ERROR;
			goto Error_Level_01;
		}
	}

Error_Level_01:
	if ((error != ECRIO_SIGMGR_NO_ERROR) && (pSipUri != NULL))
	{
		_EcrioSigMgrReleaseIPAddrStruct(pSigMgr, pSipUri->pIPAddr);
		pSipUri->pIPAddr = NULL;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_char *_EcrioSigMgrTokenize
(
	u_char *pStart,
	u_char *pEnd,
	u_char dLimiter
)
{
	u_int32 quoteFound = 0;

	while (pStart <= pEnd)
	{
		if ((*pStart == '"') && (quoteFound == 0))
		{
			pStart++;
			quoteFound++;
			continue;
		}

		if ((*pStart == '"') && (quoteFound != 0))
		{
			pStart++;
			quoteFound--;
			continue;
		}

		if ((*pStart == dLimiter) && (quoteFound == 0))
		{
			return pStart;
		}

		pStart++;
	}

	return NULL;
}

u_char *_EcrioSigMgrTokenizeQuotedString
(
	u_char *pStart,
	u_char *pEnd,
	u_char dLimiter
)
{
	u_int32 quoteFound = 0;

	while (pStart <= pEnd)
	{
		if ((*pStart == '"') && (quoteFound == 0))
		{
			pStart++;
			quoteFound++;
			continue;
		}

		if ((*pStart == '"') && (quoteFound != 0))
		{
			pStart++;
			quoteFound--;
			continue;
		}

		/*quoted-pair = "\" (%x00-09 / %x0B-0C/ %x0E-7F)*/
		if (*pStart == '\\')
		{
			if ((*(pStart + 1) >= 0x0E && *(pStart + 1) <= 0x7F) ||
				(*(pStart + 1) <= 0x09) ||
				(*(pStart + 1) == 0x0B) ||
				(*(pStart + 1) == 0x0C))
			{
				pStart++;
			}
		}

		if ((*pStart == dLimiter) && (quoteFound == 0))
		{
			return pStart;
		}

		pStart++;
	}

	return NULL;
}

u_char *_EcrioSigMgrStripLeadingLWS
(
	u_char *pStart,
	u_char *pEnd
)
{
	/** Prevent out-of-bounds readings */
	while ((pStart <= pEnd) && ((*pStart == ' ') || (*pStart == '\t')))
	{
		pStart++;
	}

	/** If pStart > pEnd from the beginning or as a result of incrementing, return pEnd. */
	if (pStart > pEnd)
	{
		return pEnd;
	}
	/** Otherwise return the pointer with stripped whitespace. */
	else
	{
		return pStart;
	}
}

u_char *_EcrioSigMgrStripTrailingLWS
(
	u_char *pStart,
	u_char *pEnd
)
{
	/** Prevent out-of-bounds readings */
	while ((pEnd >= pStart) && ((*pEnd == ' ') || (*pEnd == '\t')))
	{
		pEnd--;
	}

	/** If pEnd < pStart from the beginning or as a result of decrementing, return pStart. */
	if (pEnd < pStart)
	{
		return pStart;
	}
	/** Otherwise return the pointer with stripped whitespace. */
	else
	{
		return pEnd;
	}
}

u_int32 EcrioSigMgrCopyNameAddrWithParamStruct
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrNameAddrWithParamsStruct	*pSrcNameAddrWithParams,
	EcrioSigMgrNameAddrWithParamsStruct	**ppDestNameAddrWithParams
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrNameAddrWithParamsStruct *pDestNameAddrWithParams = NULL;

	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if ((pSrcNameAddrWithParams == NULL) || (ppDestNameAddrWithParams == NULL))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInvalid data passed", __FUNCTION__, __LINE__);

		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	error = _EcrioSigMgrAllocateAndPopulateNameAddrWithParams(pSigMgr, pSrcNameAddrWithParams, &pDestNameAddrWithParams);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrPopulateNameAddrWithParams() error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	*ppDestNameAddrWithParams = pDestNameAddrWithParams;
	pDestNameAddrWithParams = NULL;

Error_Level_01:

	if (pDestNameAddrWithParams != NULL)
	{
		EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_NameAddrWithParams,
			(void **)&pDestNameAddrWithParams, Enum_TRUE);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return error;
}

u_int32 _EcrioSigMgrExtractHostPort
(
	u_char *pStart,
	u_char *pEnd,
	u_char **ppHost,
	u_int16	*pPort
)
{
	u_int32	err = ECRIO_SIGMGR_NO_ERROR;
	u_char *Host = NULL;
	u_int32	tmpPort = 0;
	u_char *pCurrentPos = NULL;
	u_char *tmp = NULL;

	/*first Check whether it is an IPV6 address
	*
	*/
	pStart = _EcrioSigMgrStripLeadingLWS(pStart, pEnd);
	pEnd = _EcrioSigMgrStripTrailingLWS(pStart, pEnd);
	if (*pStart == '[')
	{
		pCurrentPos = _EcrioSigMgrTokenize(pStart, pEnd, ']');
		/** Boundary checking */
		if (pCurrentPos == NULL || pCurrentPos == pEnd)
		{
			err = ECRIO_SIGMGR_INVALID_URI_SCHEME;
			goto error;
		}
		else
		{
			{
				/*If it is a valid IPV6 Now Store IPV6Address and Port
				*/
				tmp = _EcrioSigMgrTokenize(pCurrentPos + 1, pEnd, ':');
				if (tmp == NULL)
				{
					/*This means only Host part is present
					*/
					u_int32 strLen = 0;
					pStart = pStart + 1;
					pEnd = pEnd - 1;

					/** Boundary checking; if input string has "[]" which pStart indicates '[' and pEnd indicates ']'
					 *  and no port number is present, then going through this path, but this is an invalid string so
					 *  we need to return error.
					 */
					if (pStart > pEnd)
					{
						err = ECRIO_SIGMGR_INVALID_URI_SCHEME;
						goto error;
					}

					strLen = (u_int32)(pEnd - pStart + 1);
					pal_MemoryAllocate((strLen + 1), (void **)&Host);
					if (NULL == Host)
					{
						return ECRIO_SIGMGR_NO_MEMORY;	/* -mustfreefresh flag warning */
					}

					if (NULL == pal_StringNCopy(Host, strLen + 1, pStart, strLen))
					{
						err = ECRIO_SIGMGR_STRING_ERROR;
						goto error;
					}
				}
				/** Boundary checking; If ':' is last char in string, this is invalid URI scheme. */
				else if (tmp == pEnd)
				{
					err = ECRIO_SIGMGR_INVALID_URI_SCHEME;
					goto error;
				}
				else
				{
					/* Both host and port present. */
					u_char *pHostEnd = NULL;
					u_char *pPortBegin = NULL;
					u_int32	strLen = 0;
					pHostEnd = _EcrioSigMgrStripTrailingLWS(pStart, tmp - 1);
					pStart = pStart + 1;
					pHostEnd = pHostEnd - 1;
					strLen = (u_int32)(pHostEnd - pStart + 1);

					pal_MemoryAllocate((strLen + 1), (void **)&Host);
					if (NULL == Host)
					{
						return ECRIO_SIGMGR_NO_MEMORY;	/* -mustfreefresh flag warning */
					}

					if (NULL == pal_StringNCopy(Host, strLen + 1, pStart, strLen))
					{
						err = ECRIO_SIGMGR_STRING_ERROR;
						goto error;
					}
					pPortBegin = _EcrioSigMgrStripLeadingLWS(tmp + 1, pEnd);
					tmpPort = (u_int16)pal_StringToNum(pPortBegin, &pEnd);
					if (tmpPort > 65535)
					{
						goto error;
					}

					*pPort = (u_int16)tmpPort;
				}
			}
		}
	}
	else
	{
		/*Try to extract Host and Port if it is a normal address.
		*/
		tmp = _EcrioSigMgrTokenize(pStart, pEnd, ':');
		if (tmp == NULL)
		{
			/*This means only Host part is present
			*/
			u_int32	strLen = 0;
			strLen = (u_int32)(pEnd - pStart + 1);
			pal_MemoryAllocate((strLen + 1), (void **)&Host);
			if (NULL == Host)
			{
				return ECRIO_SIGMGR_NO_MEMORY;	/* -mustfreefresh flag warning */
			}

			if (NULL == pal_StringNCopy(Host, strLen + 1, pStart, strLen))
			{
				err = ECRIO_SIGMGR_STRING_ERROR;
				goto error;
			}
		}
		/** Boundary checking; If ':' is last char in string, this is invalid URI scheme. */
		else if (tmp == pEnd)
		{
			err = ECRIO_SIGMGR_INVALID_URI_SCHEME;
			goto error;
		}
		else
		{
			u_char *pHostEnd = NULL;
			u_char *pPortBegin = NULL;
			u_int32	strLen = 0;

			pHostEnd = _EcrioSigMgrStripTrailingLWS(pStart, tmp - 1);
			strLen = (u_int32)(pHostEnd - pStart + 1);
			pal_MemoryAllocate((strLen + 1), (void **)&Host);
			if (NULL == Host)
			{
				return ECRIO_SIGMGR_NO_MEMORY;	/* -mustfreefresh flag warning */
			}

			if (NULL == pal_StringNCopy(Host, strLen + 1, pStart, strLen))
			{
				err = ECRIO_SIGMGR_STRING_ERROR;
				goto error;
			}
			pPortBegin = _EcrioSigMgrStripLeadingLWS(tmp + 1, pEnd);
			tmpPort = (u_int16)pal_StringToNum(pPortBegin, &pEnd);
			if (tmpPort > 65535)
			{
				goto error;
			}

			*pPort = (u_int16)tmpPort;
		}
	}

	if (Host)
	{
		*ppHost = Host;
	}

	return err;

error:
	pal_MemoryFree((void **)&Host);
	return err;
}

/**************************************************************************

Function:		_EcrioSigMgrAppendServiceRouteToRouteSet

Purpose:		To add service-route to SigMgr

Description:	This API is used to append the service route to the
existing Route set. It is invoked by the UA Engine to add
the service route values to the route set while constructing
an initial outgoing request.

Input:			EcrioSigMgrStruct* pSigMgr - signaling manager instance

Output:			EcrioSigMgrRouteStruct** ppRouteSet - updated route set

Returns:		Error Code

**************************************************************************/
u_int32 _EcrioSigMgrAppendServiceRouteToRouteSet
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrRouteStruct **ppRouteSet
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 i = 0;

	EcrioSigMgrRouteStruct *pServiceRoute = NULL;
	EcrioSigMgrRouteStruct *pRouteSet = NULL;
	EcrioSigMgrNameAddrWithParamsStruct *pRoute = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
		"%s:%u\tpSigMgr=%p, ppRouteSet=%p",
		__FUNCTION__, __LINE__, pSigMgr, ppRouteSet);

	if (ppRouteSet == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;

		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data passed", __FUNCTION__, __LINE__);

		goto Error_Level_01;
	}

	if ((pSigMgr->pServiceRoute == NULL) ||
		(pSigMgr->pServiceRoute->ppRouteDetails == NULL) ||
		!(pSigMgr->pServiceRoute->numRoutes))
	{
		error = ECRIO_SIGMGR_NO_ERROR;
		goto Error_Level_01;
	}

	pRouteSet = *ppRouteSet;
	pServiceRoute = pSigMgr->pServiceRoute;

	if (pRouteSet == NULL)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrRouteStruct), (void **)&pRouteSet);
		if (pRouteSet == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() error=%u",
				__FUNCTION__, __LINE__, error);

			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}
	}

	if (pRouteSet->numRoutes > 0)
	{
		/** Check for variable overflow - exclude too large value */
		if (pRouteSet->numRoutes + pServiceRoute->numRoutes > USHRT_MAX)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		u_int32 newLen = (pRouteSet->numRoutes + pServiceRoute->numRoutes) *
			(sizeof(EcrioSigMgrNameAddrWithParamsStruct *));

		/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
		if (pal_UtilityDataOverflowDetected(newLen, sizeof(EcrioSigMgrNameAddrWithParamsStruct *)) == Enum_TRUE)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		error = pal_MemoryReallocate(newLen, (void **)&pRouteSet->ppRouteDetails);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryReallocate() error=%u",
				__FUNCTION__, __LINE__, error);

			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}
	}
	else
	{
		/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
		if (pal_UtilityDataOverflowDetected(pServiceRoute->numRoutes, sizeof(EcrioSigMgrNameAddrWithParamsStruct *)) == Enum_TRUE)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pal_MemoryAllocate(pServiceRoute->numRoutes * sizeof(EcrioSigMgrNameAddrWithParamsStruct *), (void **)&pRouteSet->ppRouteDetails);
		if (pRouteSet->ppRouteDetails == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() error=%u",
				__FUNCTION__, __LINE__, error);

			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}
	}

	for (i = 0; i < pServiceRoute->numRoutes; i++)
	{
		if (pServiceRoute->ppRouteDetails[i])
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct), (void **)&pRouteSet->ppRouteDetails[pRouteSet->numRoutes]);
			if (pRouteSet->ppRouteDetails[pRouteSet->numRoutes] == NULL)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryAllocate() error=%u",
					__FUNCTION__, __LINE__, error);

				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			pRoute = pRouteSet->ppRouteDetails[pRouteSet->numRoutes];
			pRouteSet->numRoutes++;

			error = _EcrioSigMgrPopulateNameAddr(pSigMgr,
				&(pServiceRoute->ppRouteDetails[i]->nameAddr),
				&pRoute->nameAddr);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrPopulateNameAddr() populate service routes, error=%u",
					__FUNCTION__, __LINE__, error);

				goto Error_Level_01;
			}

			if (pServiceRoute->ppRouteDetails[i]->numParams > 0)
			{
				error = _EcrioSigMgrCopyParam(pSigMgr,
					pServiceRoute->ppRouteDetails[i]->numParams,
					pServiceRoute->ppRouteDetails[i]->ppParams,
					&(pRoute->ppParams));
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\t_EcrioSigMgrCopyParam() populate service routes params, error=%u",
						__FUNCTION__, __LINE__, error);

					goto Error_Level_01;
				}

				pRoute->numParams = pServiceRoute->ppRouteDetails[i]->numParams;
			}
		}
	}

	*ppRouteSet = pRouteSet;
	error = ECRIO_SIGMGR_NO_ERROR;

Error_Level_01:
	if ((error != ECRIO_SIGMGR_NO_ERROR) &&
		(pRouteSet))
	{
		(void)_EcrioSigMgrReleaseRouteStruct(pSigMgr, pRouteSet);
		pal_MemoryFree((void **)&pRouteSet);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrPopulateAuthenticationDetails()

Purpose:		Stores Source Authentication details to destination Authentication details..

Description:	Stores Source Authentication details to destination Authentication details..

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
EcrioSigMgrAuthenticationStruct* pSrcAuth - Source Authentication details.
EcrioSigMgrAuthenticationStruct* pDstAuth - Destination Authentication details

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrPopulateAuthenticationDetails
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrAuthenticationStruct *pSrcAuth,
	EcrioSigMgrAuthenticationStruct *pDstAuth
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR, i = 0;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if ((pSrcAuth == NULL) || (pDstAuth == NULL))
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	// Store Authentication scheme
	if (pSrcAuth->pAuthenticationScheme != NULL)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcAuth->pAuthenticationScheme,
			&(pDstAuth->pAuthenticationScheme));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	// Store realm Value
	if (pSrcAuth->pRealm != NULL)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcAuth->pRealm,
			&(pDstAuth->pRealm));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	// Store Domain Name
	if (pSrcAuth->pDomain != NULL)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcAuth->pDomain,
			&(pDstAuth->pDomain));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	// Store Nonce Value
	if (pSrcAuth->pNonce != NULL)
	{
		if (pal_StringLength(pSrcAuth->pNonce))
		{
			_EcrioSigMgrStringCreate(pSigMgr, pSrcAuth->pNonce,
				&(pDstAuth->pNonce));
		}
		else
		{
			pal_MemoryAllocate(sizeof(u_char), (void **)&(pDstAuth->pNonce));
			if (pDstAuth->pNonce)
			{
				pDstAuth->pNonce[0] = '\0';
			}
		}

		if (pDstAuth->pNonce == NULL)
		{
			goto Error_Level_01;
		}
	}

	// Store Opaque Value
	if (pSrcAuth->pOpaque != NULL)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcAuth->pOpaque,
			&(pDstAuth->pOpaque));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	pDstAuth->bStale = pSrcAuth->bStale;
	pDstAuth->authAlgo = pSrcAuth->authAlgo;

	if (pSrcAuth->countQoP > 0)
	{
		pal_MemoryAllocate(sizeof(u_char *) * (pSrcAuth->countQoP), (void **)&pDstAuth->ppListOfQoP);
		if (pDstAuth->ppListOfQoP == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pDstAuth->countQoP = pSrcAuth->countQoP;

		for (i = 0; i < pSrcAuth->countQoP; ++i)
		{
			error = _EcrioSigMgrStringCreate(pSigMgr, pSrcAuth->ppListOfQoP[i],
				&(pDstAuth->ppListOfQoP[i]));
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error_Level_01;
			}
		}
	}

Error_Level_01:

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
s_int32 _EcrioSigMgrCompareOptionalHeaderFunc
(
	void *pData1,
	void *pData2,
	void *pCallbackData
)
{
	s_int32 retVal = 1;
	EcrioSigMgrStruct *pSigMgr = (EcrioSigMgrStruct *)pCallbackData;
	EcrioSipHeaderTypeEnum *pHdrType = (EcrioSipHeaderTypeEnum *)pData2;
	EcrioSigMgrHeaderStruct *pHeaderStruct = (EcrioSigMgrHeaderStruct *)pData1;

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

	if (pHeaderStruct->eHdrType == *pHdrType)
	{
		retVal = 0;
	}

Error_Level_01:

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
void _EcrioSigMgrDestroyOptionalHeaderListFunc
(
	void *pData,
	void *pCallbackData
)
{
	EcrioSigMgrHeaderStruct *pHeader = (EcrioSigMgrHeaderStruct *)pData;
	EcrioSigMgrStruct *pSigMgr = (EcrioSigMgrStruct *)pCallbackData;

	if (pSigMgr == NULL)
	{
		return;
	}

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
// quiet		__FUNCTION__, __LINE__);

	if (pHeader == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data", __FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	_EcrioSigMgrReleaseHdrStruct(pSigMgr, pHeader);
	pal_MemoryFree((void **)&pHeader);

Error_Level_01:
// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
// quiet		__FUNCTION__, __LINE__);
	return;
}

u_int32 EcrioSigMgrCreateOptionalHeaderList
(
	SIGMGRHANDLE sigMgrHandle,
	void **ppOptionalHeaderList
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;

	if (sigMgrHandle == NULL || ppOptionalHeaderList == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	pSigMgr = (EcrioSigMgrStruct *)sigMgrHandle;

//	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
//		__FUNCTION__, __LINE__);

	error = EcrioDSListCreate(
		pSigMgr->pLogHandle,
		_EcrioSigMgrCompareOptionalHeaderFunc,
		_EcrioSigMgrDestroyOptionalHeaderListFunc, pSigMgr,
		(void **)ppOptionalHeaderList);

	if (error != ECRIO_DS_NO_ERROR)
	{
// @todo Refactor this function. Return NULL if can't create. Let caller decide how to handle.
//		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
//			"%s:%u\tEcrioDSListCreate() for ppOptionalHeaderList, error=%u",
//			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_CRITCAL_ERROR_NEED_TO_EXIT;
	}

	return error;
}

u_int32 EcrioSigMgrGetOptionalHeader
(
	SIGMGRHANDLE sigMgrHandle,
	void *pOptionalHeaderList,
	EcrioSipHeaderTypeEnum eHdrType,
	EcrioSigMgrHeaderStruct **ppOptHeader
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	if (sigMgrHandle == NULL || ppOptHeader == NULL || pOptionalHeaderList == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

//	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
//		__FUNCTION__, __LINE__);

	error = EcrioDSListFindData(pOptionalHeaderList, (void *)&eHdrType, (void **)ppOptHeader);
	if (error != ECRIO_DS_NO_ERROR)
	{
// @todo Refactor this function! Don't return anything.
// Caller should just check ppOptHeader for NULL or not.
// Only caller can make decision if "signaling manager needs to exit"!
//		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
//			"%s:%u\tEcrioDSListFindData() for pOptionalHeaderList, error=%u",
//			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_CRITCAL_ERROR_NEED_TO_EXIT;
	}

	return error;
}

u_int32 _EcrioSigMgrInsertOptionalHeader
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList,
	EcrioSigMgrHeaderStruct *pOptHeader
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	if (pOptHeader == NULL || pOptionalHeaderList == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	error = EcrioDSListInsertData(pOptionalHeaderList, (void *)pOptHeader);
	if (error != ECRIO_DS_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tEcrioDSListInsertData() for pOptionalHeaderList, error=%u",
			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_CRITCAL_ERROR_NEED_TO_EXIT;
	}

	return error;
}

u_int32 EcrioSigMgrReleaseOptionalHeaderList
(
	SIGMGRHANDLE sigMgrHandle,
	void **ppOptionalHeaderList
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;

	if (sigMgrHandle == NULL || ppOptionalHeaderList == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	pSigMgr = (EcrioSigMgrStruct *)sigMgrHandle;

	error = EcrioDSListDestroy(ppOptionalHeaderList);
	if (error != ECRIO_DS_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tEcrioDSListDestroy() for pOptionalHeaderList, error=%u",
			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_CRITCAL_ERROR_NEED_TO_EXIT;
	}

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrAddValuesToHdr()

Purpose:		Add given header values to header.

Description:	Add given header values to header.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
u_int32 numValues - number of header values
u_char** ppValues - List of header values

OutPut:			EcrioSigMgrHeaderStruct* pHdr - Header to which values are
to be added

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrAddUserAgentHeader
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_char **ppVal = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pOptionalHeaderList == NULL || pSigMgr->pSignalingInfo->pUserAgent == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	pal_MemoryAllocate(1 * sizeof(u_char *), (void **)&ppVal);
	if (ppVal == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	ppVal[0] = pal_StringCreate((u_char *)pSigMgr->pSignalingInfo->pUserAgent, pal_StringLength((u_char *)pSigMgr->pSignalingInfo->pUserAgent));
	if (ppVal[0] == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pOptionalHeaderList, EcrioSipHeaderTypeUserAgent, 1,
		ppVal, 0, NULL, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

Error_Level_01:

	if (ppVal)
	{
		if (ppVal[0])
		{
			pal_MemoryFree((void **)&ppVal[0]);
		}

		pal_MemoryFree((void **)&ppVal);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrAddPublishIfMatchHdr
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList,
	u_char *pETag
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_char **ppVal = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pOptionalHeaderList == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	pal_MemoryAllocate(1 * sizeof(u_char *), (void **)&ppVal);
	ppVal[0] = pal_StringCreate((u_char *)pETag, pal_StringLength(pETag));
	error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pOptionalHeaderList, EcrioSipHeaderTypeSipIfMatch, 1,
		ppVal, 0, NULL, NULL);

Error_Level_01:
	if (ppVal)
	{
		if (ppVal[0])
		{
			pal_MemoryFree((void **)&ppVal[0]);
		}
		pal_MemoryFree((void **)&ppVal);
	}
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);
	return error;
}

u_int32 _EcrioSigMgrAddPublishEventHdr
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_char **ppVal = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pOptionalHeaderList == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	pal_MemoryAllocate(1 * sizeof(u_char *), (void **)&ppVal);
	ppVal[0] = pal_StringCreate((u_char *)_ECRIO_PUBLISH_EVENT, pal_StringLength(_ECRIO_PUBLISH_EVENT));
	error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pOptionalHeaderList, EcrioSipHeaderTypeEvent, 1,
		ppVal, 0, NULL, NULL);

Error_Level_01:
	if (ppVal)
	{
		if (ppVal[0])
		{
			pal_MemoryFree((void **)&ppVal[0]);
		}
		pal_MemoryFree((void **)&ppVal);
	}
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);
	return error;
}

u_int32 _EcrioSigMgrAddAllowHeader
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pOptionalHeaderList == NULL || pSigMgr->pSignalingInfo->ppSupportedMethod == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pOptionalHeaderList, EcrioSipHeaderTypeAllow, pSigMgr->pSignalingInfo->numSupportedMethods,
		pSigMgr->pSignalingInfo->ppSupportedMethod, 0, NULL, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}


BoolEnum _EcrioSigMgrIsEarlyDialogTermination
(
EcrioSigMgrStruct *pSigMgr,
u_char *pCallId
)
{
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;

	ec_MapGetKeyData(pSigMgr->hHashMap, pCallId, (void**)&pDialogNode);
	if (pDialogNode && pDialogNode->pForkedMsgHead)
	{
		return pDialogNode->pForkedMsgHead->bIsEarlyDialogTermination;
	}
	return Enum_FALSE;
}

/* Create the Reason value/param*/
/* RFC - 3326

Reason            =  "Reason" HCOLON reason-value *(COMMA reason-value)
reason-value      =  protocol *(SEMI reason-params)
protocol          =  "SIP" / "Q.850" / token
reason-params     =  protocol-cause / reason-text
/ reason-extension
protocol-cause    =  "cause" EQUAL cause
cause             =  1*DIGIT
reason-text       =  "text" EQUAL quoted-string
reason-extension  =  generic-param

*/
u_int32 _EcrioSigMgrAddReasonHeader
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList,
	u_char * pCallId
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_char **ppReasonParam = NULL;
	u_char **ppReasonVal = NULL;
	u_char **ppVal = NULL;
	u_int16 i = 0;
	BoolEnum bIsEarlyDialogTermination = Enum_FALSE;
	EcrioSigMgrCallbackRegisteringModuleEnums eModule = EcrioSigMgrCallbackRegisteringModule_MoIP;
	u_char *pReasonProto = NULL;
	u_char *pReasonCause = NULL;
	u_char *pReasonText = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pOptionalHeaderList == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	/*Check the request initiator module*/
	eModule = _EcrioSigMgrGetModuleId(pSigMgr, pCallId);
	bIsEarlyDialogTermination = _EcrioSigMgrIsEarlyDialogTermination(pSigMgr, pCallId);

	pal_MemoryAllocate(1 * sizeof(u_char *), (void **)&ppVal);
	if (NULL == ppVal)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeMemory,
			"%s:%u\tpal_MemoryAllocate() for ppVal returned NULL", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	if (pSigMgr->pOOMObject->ec_oom_GetReasonHeaderValues(&pReasonProto, &pReasonCause, &pReasonText))
	{
		if (pReasonProto != NULL)
		{
			ppVal[0] = pal_StringCreate((u_char *)pReasonProto, pal_StringLength((u_char *)pReasonProto));
		}
		if (pReasonCause != NULL)
		{
			pal_MemoryAllocate(2 * sizeof(u_char *), (void **)&ppReasonParam);
			if (ppReasonParam == NULL)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeMemory,
					"%s:%u\tpal_MemoryAllocate() for ppReasonParam returned NULL", __FUNCTION__, __LINE__);
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
			ppReasonParam[0] = pal_StringCreate((u_char *)pReasonCause, pal_StringLength((u_char *)pReasonCause));

			if (pReasonText != NULL)
			{
				ppReasonParam[1] = pal_StringCreate((u_char *)pReasonText, pal_StringLength((u_char *)pReasonText));
			}
			else
			{
				ppReasonParam[1] = NULL;
			}
		}
	}
	else
	{
		ppVal[0] = pal_StringCreate((u_char *)ECRIO_SIG_MGR_BYE_REASON_PROTOCOL, pal_StringLength((u_char *)ECRIO_SIG_MGR_BYE_REASON_PROTOCOL));

		pal_MemoryAllocate(2 * sizeof(u_char *), (void **)&ppReasonParam);
		if (ppReasonParam == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeMemory,
				"%s:%u\tpal_MemoryAllocate() for ppReasonParam returned NULL", __FUNCTION__, __LINE__);
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}
		ppReasonParam[0] = pal_StringCreate((u_char *)ECRIO_SIG_MGR_BYE_REASON_CAUSE, pal_StringLength((u_char *)ECRIO_SIG_MGR_BYE_REASON_CAUSE));
		ppReasonParam[1] = pal_StringCreate((u_char *)ECRIO_SIG_MGR_BYE_REASON_TEXT, pal_StringLength((u_char *)ECRIO_SIG_MGR_BYE_REASON_TEXT));

		pal_MemoryAllocate(2 * sizeof(u_char *), (void **)&ppReasonVal);
		if (ppReasonVal == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeMemory,
				"%s:%u\tpal_MemoryAllocate() for ppReasonVal returned NULL", __FUNCTION__, __LINE__);
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}
		ppReasonVal[0] = pal_StringCreate((u_char *)ECRIO_SIG_MGR_BYE_REASON_CAUSE_VAL, pal_StringLength((u_char *)ECRIO_SIG_MGR_BYE_REASON_CAUSE_VAL));

		if (eModule == EcrioSigMgrCallbackRegisteringModule_CPM)
		{
			ppReasonVal[1] = pal_StringCreate((u_char *)ECRIO_SIG_MGR_BYE_REASON_TEXT_VAL_CPM, pal_StringLength((u_char *)ECRIO_SIG_MGR_BYE_REASON_TEXT_VAL_CPM));
		}
		else if (bIsEarlyDialogTermination)
		{
			ppReasonVal[1] = pal_StringCreate((u_char *)ECRIO_SIG_MGR_BYE_REASON_TEXT_VAL_NEW_DIALOG, pal_StringLength((u_char *)ECRIO_SIG_MGR_BYE_REASON_TEXT_VAL_NEW_DIALOG));
		}
		else
		{
			ppReasonVal[1] = pal_StringCreate((u_char *)ECRIO_SIG_MGR_BYE_REASON_TEXT_VAL_MOIP, pal_StringLength((u_char *)ECRIO_SIG_MGR_BYE_REASON_TEXT_VAL_MOIP));
		}
	}

	error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pOptionalHeaderList, EcrioSipHeaderTypeReason, 1, ppVal, 2, ppReasonParam, ppReasonVal);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

Error_Level_01:

	if (ppVal)
	{
		for (i = 0; i < 1; i++)
		{
			if (ppVal[i])
			{
				pal_MemoryFree((void **)&ppVal[i]);
			}
		}

		pal_MemoryFree((void **)&ppVal);
	}

	if (ppReasonParam)
	{
		for (i = 0; i < 2; i++)
		{
			if (ppReasonParam[i])
			{
				pal_MemoryFree((void **)&ppReasonParam[i]);
			}
		}

		pal_MemoryFree((void **)&ppReasonParam);
	}

	if (ppReasonVal)
	{
		for (i = 0; i < 2; i++)
		{
			if (ppReasonVal[i])
			{
				pal_MemoryFree((void **)&ppReasonVal[i]);
			}
		}

		pal_MemoryFree((void **)&ppReasonVal);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrAddP_AccessNWInfoHeader
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_char **ppVal = NULL;

	u_char accessType[64] = { 0 };
	u_char accessInfo[64] = { 0 };
	u_char accessInfoHeaderValue[132] = { 0 }; /* accessType;accessInfo */
	u_int8 len = 0, len1 = 0;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pOptionalHeaderList == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	/* Generate header */
	pal_MemoryAllocate(1 * sizeof(u_char *), (void **)&ppVal);
	if (ppVal == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	(pSigMgr->limsCallbackStruct.getPropertyCallbackFn)(EcrioSipPropertyName_ACCESSTYPE, EcrioSipPropertyType_STRING, accessType,
		pSigMgr->limsCallbackStruct.pCallbackData);

	len = pal_StringLength((u_char *)&accessType[0]);
	if (len == 0)
	{
		SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t No Access Type Available, can not add P-ANI header", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_NO_ERROR;
		goto Error_Level_01;
	}

	(pSigMgr->limsCallbackStruct.getPropertyCallbackFn)(EcrioSipPropertyName_ACCESSINFO_VALUE, EcrioSipPropertyType_STRING, accessInfo,
		pSigMgr->limsCallbackStruct.pCallbackData);

	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tAccessType = %s",
		__FUNCTION__, __LINE__, &accessType[0]);
	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tAccessInfo = %s",
		__FUNCTION__, __LINE__, &accessInfo[0]);


	if (NULL == pal_StringNCopy(&accessInfoHeaderValue[0], len + 1, (u_char *)&accessType[0], len))
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}
	len1 = pal_StringLength((u_char *)";");
	len = len + len1;
	if (NULL == pal_StringNConcatenate(&accessInfoHeaderValue[0], len + 1, (u_char *)";", len1))
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}
	len1 = pal_StringLength((u_char *)&accessInfo[0]);
	len = len + len1;
	if (NULL == pal_StringNConcatenate(&accessInfoHeaderValue[0], len + 1, (u_char *)&accessInfo[0], len1))
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tAccessInfoHeaderValue = %s",
		__FUNCTION__, __LINE__, &accessInfoHeaderValue[0]);

	ppVal[0] = pal_StringCreate((u_char*)&accessInfoHeaderValue[0], pal_StringLength((u_char*)&accessInfoHeaderValue[0]));
	if (ppVal[0] == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pOptionalHeaderList, EcrioSipHeaderTypeP_AccessNetworkInfo, 1,
		ppVal, 0, NULL, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

Error_Level_01:

	if (ppVal)
	{
		if (ppVal[0])
		{
			pal_MemoryFree((void **)&ppVal[0]);
		}
		pal_MemoryFree((void **)&ppVal);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}


u_int32 _EcrioSigMgrAddP_LastAccessNWInfoHeader
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_char **ppVal = NULL;

	u_char lastAccessType[64] = { 0 };
	u_char lastAccessInfo[64] = { 0 };
	u_char lastAccessInfoHeaderValue[132] = { 0 }; /* lastAaccessType;lastAccessInfo */
	u_int8 len = 0,len1 = 0;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pOptionalHeaderList == NULL)	
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}
	
	/* Generate header */
	pal_MemoryAllocate(1 * sizeof(u_char *), (void **)&ppVal);
	if (ppVal == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	(pSigMgr->limsCallbackStruct.getPropertyCallbackFn)(EcrioSipPropertyName_LAST_ACCESSTYPE, EcrioSipPropertyType_STRING, lastAccessType,
		pSigMgr->limsCallbackStruct.pCallbackData);

	len = pal_StringLength((u_char *)&lastAccessType[0]);
	if (len == 0)
	{
		SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t No Last Access Type Available, can not add P-LANI header",__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_NO_ERROR;
		goto Error_Level_01;
	}

	(pSigMgr->limsCallbackStruct.getPropertyCallbackFn)(EcrioSipPropertyName_LAST_ACCESSINFO_VALUE, EcrioSipPropertyType_STRING, lastAccessInfo,
		pSigMgr->limsCallbackStruct.pCallbackData);

	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tLast AccessType = %s",
		__FUNCTION__, __LINE__, &lastAccessType[0]);
	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tLast AccessInfo = %s",
		__FUNCTION__, __LINE__, &lastAccessInfo[0]);

	
	if (NULL == pal_StringNCopy(&lastAccessInfoHeaderValue[0], len + 1, (u_char *)&lastAccessType[0], len))
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}
	len1 = pal_StringLength((u_char *)";");
	len = len + len1;
	if (NULL == pal_StringNConcatenate(&lastAccessInfoHeaderValue[0], len + 1, (u_char *)";", len1))
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}
	len1 = pal_StringLength((u_char *)&lastAccessInfo[0]);
	len = len + len1;
	if (NULL == pal_StringNConcatenate(&lastAccessInfoHeaderValue[0], len + 1, (u_char *)&lastAccessInfo[0], len1))
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tLastAccessInfoHeaderValue = %s",
		__FUNCTION__, __LINE__, &lastAccessInfoHeaderValue[0]);

	ppVal[0] = pal_StringCreate((u_char*)&lastAccessInfoHeaderValue[0], pal_StringLength((u_char*)&lastAccessInfoHeaderValue[0]));
	if (ppVal[0] == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}
	

	error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pOptionalHeaderList, EcrioSipHeaderTypeP_LastAccessNetworkInfo, 1,
		ppVal, 0, NULL, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

Error_Level_01:

	if (ppVal)
	{
		if (ppVal[0])
		{
			pal_MemoryFree((void **)&ppVal[0]);
		}
		pal_MemoryFree((void **)&ppVal);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 EcrioSigMgrAddOptionalHeader
(
	SIGMGRHANDLE sigMgrHandle,
	void *pOptionalHeaderList,
	EcrioSipHeaderTypeEnum eHeaderType,
	u_int16 numValues,
	u_char **ppValues,
	u_int16 numParams,
	u_char **ppParamN,
	u_char **ppParamV
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;

	if (sigMgrHandle == NULL || pOptionalHeaderList == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	pSigMgr = (EcrioSigMgrStruct *)sigMgrHandle;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pOptionalHeaderList, eHeaderType, numValues, ppValues, numParams, ppParamN, ppParamV);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}
u_int32 _EcrioSigMgrAddOptionalHeader
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList,
	EcrioSipHeaderTypeEnum eHeaderType,
	u_int16 numValues,
	u_char **ppValues,
	u_int16 numParams,
	u_char **ppParamN,
	u_char **ppParamV
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrHeaderStruct *pHeader = NULL;

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
// quiet		__FUNCTION__, __LINE__);

	if (pOptionalHeaderList == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderStruct), (void **)&pHeader);
	if (pHeader == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tECRIO_ALLOCATE_MEM() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	error = _EcrioSigMgrAddValueParamsToHdr(pSigMgr, numValues, ppValues, numParams,
		ppParamN, ppParamV, pHeader);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t__EcrioSigMgrAddValueParamsToHdr() Add optional header (%u), error=%u",
			__FUNCTION__, __LINE__, eHeaderType, error);

		goto Error_Level_01;
	}

	pHeader->eHdrType = eHeaderType;
	error = _EcrioSigMgrInsertOptionalHeader(pSigMgr, pOptionalHeaderList, pHeader);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrInsertOptionalHeader() Add optional header (%u), error=%u",
			__FUNCTION__, __LINE__, eHeaderType, error);

		goto Error_Level_01;
	}

Error_Level_01:
	pHeader = NULL;
// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
// quiet		__FUNCTION__, __LINE__, error);

	return error;
}

BoolEnum EcrioSigMgrCheckPrackSupport
(
	SIGMGRHANDLE hSigMgrHandle,
	void *pOptionalHeaderList,
	BoolEnum *bRequire
)
{
	// u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrHeaderStruct *pHeader = NULL;
	u_int32	i = 0;
	BoolEnum bPrackSupported = Enum_FALSE;

	pSigMgr = (EcrioSigMgrStruct *)hSigMgrHandle;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pOptionalHeaderList == NULL)
	{
		// error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	if (bRequire)
	{
		*bRequire = Enum_FALSE;
	}

	EcrioSigMgrGetOptionalHeader(pSigMgr, pOptionalHeaderList, EcrioSipHeaderTypeRequire, &pHeader);
	if (pHeader != NULL)
	{
		for (i = 0; i < pHeader->numHeaderValues; i++)
		{
			if (pHeader->ppHeaderValues && pHeader->ppHeaderValues[i])
			{
				if (pal_SubString(pHeader->ppHeaderValues[i]->pHeaderValue, (u_char *)ECRIO_SIG_MGR_PRACK_EXTENSION))
				{
					if (bRequire)
					{
						*bRequire = Enum_TRUE;
					}

					bPrackSupported = Enum_TRUE;
					break;
				}
			}
		}

		pHeader = NULL;
	}

	if (bPrackSupported != Enum_TRUE)
	{
		EcrioSigMgrGetOptionalHeader(pSigMgr, pOptionalHeaderList, EcrioSipHeaderTypeSupported, &pHeader);
		if (pHeader != NULL)
		{
			for (i = 0; i < pHeader->numHeaderValues; i++)
			{
				if (pHeader->ppHeaderValues && pHeader->ppHeaderValues[i])
				{
					if (pal_SubString(pHeader->ppHeaderValues[i]->pHeaderValue, (u_char *)ECRIO_SIG_MGR_PRACK_EXTENSION))
					{
						bPrackSupported = Enum_TRUE;
						break;
					}
				}
			}

			pHeader = NULL;
		}
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, bPrackSupported);

	return bPrackSupported;
}

BoolEnum EcrioSigMgrCheckTimerSupport
(
	SIGMGRHANDLE hSigMgrHandle,
	void *pOptionalHeaderList
)
{
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrHeaderStruct *pHeader = NULL;
	u_int32	i = 0;
	BoolEnum bTimerSupported = Enum_FALSE;

	pSigMgr = (EcrioSigMgrStruct *)hSigMgrHandle;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pOptionalHeaderList == NULL)
	{
		goto Error_Level_01;
	}

	EcrioSigMgrGetOptionalHeader(pSigMgr, pOptionalHeaderList, EcrioSipHeaderTypeSupported, &pHeader);
	if (pHeader != NULL)
	{
		for (i = 0; i < pHeader->numHeaderValues; i++)
		{
			if (pHeader->ppHeaderValues && pHeader->ppHeaderValues[i])
			{
				if (pal_SubString(pHeader->ppHeaderValues[i]->pHeaderValue, (u_char *)ECRIO_SIG_MGR_SESSION_TIMER_EXTENSION))
				{
					bTimerSupported = Enum_TRUE;
					break;
				}
			}
		}

		pHeader = NULL;
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\ttimer supported=%s",
		__FUNCTION__, __LINE__, bTimerSupported == Enum_TRUE ? "yes" : "no");

	return bTimerSupported;
}

u_int32	_EcrioSigMgrExtractRSeqFromRAck
(
	u_char *pRAckStr,
	u_int32	*pRseq
)
{
	u_int32	uError = ECRIO_SIGMGR_NO_ERROR;

	*pRseq = pal_StringConvertToUNum(pRAckStr, NULL, 10);

	return uError;
}

u_int32	_EcrioSigMgrExtractResponseCode
(
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData
)
{
	u_int32	uResponseCode = 0;

	if (pal_StringNCompare(pUsageData->pMsg, (u_char *)"SIP/2.0", 7) == 0)
	{
		uResponseCode = pal_StringToNum(pUsageData->pMsg + 8, NULL);
	}

	return uResponseCode;
}

u_int32	_EcrioSigMgrUpdateRSeqInProvResp
(
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData
)
{
	u_int32	uError = ECRIO_SIGMGR_NO_ERROR;
	u_char *pRseqValStart = NULL;
	u_char *pRseqValEnd = NULL;
	u_char *pNewMsg = NULL;
	u_char newRseqStr[11] = { 0 };
	u_int32	oldRseqLen = 0;
	u_int32	newRseqLen = 0;
	u_int32	rseqValStartIndex = 0;

	if (pUsageData->pPrackDetails)
	{
		/* Check arithmetic overflow */
		if (pal_UtilityArithmeticOverflowDetected(pUsageData->pPrackDetails->uRSeqArrayCount, 1) == Enum_TRUE)
		{
			return ECRIO_SIGMGR_NO_MEMORY;
		}

		/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
		if (pal_UtilityDataOverflowDetected((pUsageData->pPrackDetails->uRSeqArrayCount + 1), sizeof(_EcrioSigMgrRSeqStruct)) == Enum_TRUE)
		{
			return ECRIO_SIGMGR_NO_MEMORY;
		}

		(pUsageData->pPrackDetails->uLastRseq)++;
		if (KPALErrorNone != pal_MemoryReallocate((pUsageData->pPrackDetails->uRSeqArrayCount + 1) * sizeof(_EcrioSigMgrRSeqStruct), (void **)&pUsageData->pPrackDetails->pRseqArray))
		{
			return ECRIO_SIGMGR_NO_MEMORY;
		}
		pUsageData->pPrackDetails->pRseqArray[pUsageData->pPrackDetails->uRSeqArrayCount].uRseq = pUsageData->pPrackDetails->uLastRseq;
		pUsageData->pPrackDetails->pRseqArray[pUsageData->pPrackDetails->uRSeqArrayCount].bAcknowledged = Enum_FALSE;

		(pUsageData->pPrackDetails->uRSeqArrayCount)++;

		if (0 >= pal_NumToString(pUsageData->pPrackDetails->uLastRseq, newRseqStr, 11))
		{
			return ECRIO_SIGMGR_STRING_ERROR;
		}
	}

	pRseqValStart = pal_SubString(pUsageData->pMsg, (u_char *)"RSeq:");
	if (pRseqValStart)
	{
		pRseqValStart += 5;

		while (*pRseqValStart == ' ')
		{
			pRseqValStart++;
		}

		pRseqValEnd = pRseqValStart;

		while (*pRseqValEnd >= '0' && *pRseqValEnd <= '9')
		{
			pRseqValEnd++;
		}

		oldRseqLen = (u_int32)(pRseqValEnd - pRseqValStart);
		newRseqLen = pal_StringLength(newRseqStr);

		if (newRseqLen > oldRseqLen)
		{
			rseqValStartIndex = (u_int32)(pRseqValStart - pUsageData->pMsg);
			pal_MemoryAllocate(pal_StringLength(pUsageData->pMsg) + (newRseqLen - oldRseqLen) + 1, (void **)&pNewMsg);
			if (pNewMsg == NULL)
			{
				return ECRIO_SIGMGR_NO_MEMORY;
			}

			if (NULL == pal_StringNCopy(pNewMsg, rseqValStartIndex + 1, pUsageData->pMsg, rseqValStartIndex))
			{
				pal_MemoryFree((void **)&pNewMsg);
				return ECRIO_SIGMGR_STRING_ERROR;
			}

			if (NULL == pal_StringNCopy(&pNewMsg[rseqValStartIndex + newRseqLen], (pal_StringLength(pUsageData->pMsg) + (newRseqLen - oldRseqLen) + 1) - rseqValStartIndex, &pUsageData->pMsg[rseqValStartIndex + oldRseqLen], pal_StringLength(pUsageData->pMsg) - (rseqValStartIndex + oldRseqLen)))
			{
				pal_MemoryFree((void **)&pNewMsg);
				return ECRIO_SIGMGR_STRING_ERROR;
			}
			pal_MemoryCopy((void *)&pNewMsg[rseqValStartIndex], newRseqLen, newRseqStr, newRseqLen);

			pal_MemoryFree((void **)&pUsageData->pMsg);
			pUsageData->pMsg = pNewMsg;
			pNewMsg = NULL;

			pUsageData->msgLength += (newRseqLen - oldRseqLen);
		}
		else
		{
			pal_MemoryCopy((void *)pRseqValStart, newRseqLen, newRseqStr, newRseqLen);
		}
	}

	return uError;
}


BoolEnum EcrioSigMgrCheckPEarlyMedia//P-Early-Media
(
	SIGMGRHANDLE hSigMgrHandle,
	void *pOptionalHeaderList,
	EcrioSigMgrEarlyMediaStateEnum *EarlyMediaState
)
{
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrHeaderStruct *pHeader = NULL;
	u_int32	i = 0;

	pSigMgr = (EcrioSigMgrStruct *)hSigMgrHandle;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pOptionalHeaderList == NULL)
	{
		goto Error_Level_01;
	}

	if (EarlyMediaState)
	{
		*EarlyMediaState = EcrioSigMgrEarlyMediaStateNone;
	}
	else
	{
		return Enum_FALSE;
	}

	EcrioSigMgrGetOptionalHeader(pSigMgr, pOptionalHeaderList, EcrioSipHeaderTypeP_EarlyMedia, &pHeader);
	if (pHeader != NULL)
	{
		for (i = 0; i < pHeader->numHeaderValues; i++)
		{
			if (pHeader->ppHeaderValues && pHeader->ppHeaderValues[i])
			{
				if(pal_SubString(pHeader->ppHeaderValues[i]->pHeaderValue, (u_char *)"sendrecv") 
					|| pal_SubString(pHeader->ppHeaderValues[i]->pHeaderValue, (u_char *)"gated"))
				{
					*EarlyMediaState = EcrioSigMgrEarlyMediaStateSendrecv;
					break;
				}
				else if(pal_SubString(pHeader->ppHeaderValues[i]->pHeaderValue, (u_char *)"sendonly"))
				{
					*EarlyMediaState = EcrioSigMgrEarlyMediaStateSendOnly;
					break;
				}
				else if(pal_SubString(pHeader->ppHeaderValues[i]->pHeaderValue, (u_char *)"inactive"))
				{
					*EarlyMediaState = EcrioSigMgrEarlyMediaStateInactive;
					break;
				}
				else if (pal_SubString(pHeader->ppHeaderValues[i]->pHeaderValue, (u_char *)"recvonly"))
				{
					*EarlyMediaState = EcrioSigMgrEarlyMediaStateRecvOnly;
					break;
				}
				else
				{
					*EarlyMediaState = EcrioSigMgrEarlyMediaStateNone;
				}
						
			}
		}

		pHeader = NULL;
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, i);

	if(*EarlyMediaState != EcrioSigMgrEarlyMediaStateNone)
	{
		return Enum_TRUE;
	}
	else
	{
		return Enum_FALSE;
	}
}


u_int32 EcrioSigMgrFillUriStruct
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrUriStruct *pUri,
	u_char *pUserID,
	EcrioSigMgrURISchemesEnum eUriType
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrSipURIStruct *pToSipUri = NULL;
	EcrioSigMgrTelURIStruct *pToTelUri = NULL;

	if (sigMgrHandle == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	pSigMgr = (EcrioSigMgrStruct *)sigMgrHandle;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (eUriType == EcrioSigMgrURISchemeSIP)
	{
		pUri->uriScheme = EcrioSigMgrURISchemeSIP;

		pal_MemoryAllocate(sizeof(EcrioSigMgrSipURIStruct), (void **)&pUri->u.pSipUri);
		if (pUri->u.pSipUri == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto End;
		}

		pToSipUri = pUri->u.pSipUri;
		pToSipUri->pDomain = pal_StringCreate(pSigMgr->pSignalingInfo->pHomeDomain, pal_StringLength(pSigMgr->pSignalingInfo->pHomeDomain));
		if (pToSipUri->pDomain == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto End;
		}
		if (pUserID != NULL)
		{
			pToSipUri->pUserId = pal_StringCreate(pUserID, pal_StringLength(pUserID));
			if (pToSipUri->pUserId == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto End;
			}
		}
		else{
			pToSipUri->pUserId = pal_StringCreate(pSigMgr->pSignalingInfo->pFromSipURI->u.pSipUri->pUserId, pal_StringLength(pSigMgr->pSignalingInfo->pFromSipURI->u.pSipUri->pUserId));
		}
	}
	else if (eUriType == EcrioSigMgrURISchemeTEL)
	{
		pUri->uriScheme = EcrioSigMgrURISchemeTEL;

		pal_MemoryAllocate(sizeof(EcrioSigMgrTelURIStruct), (void **)&pUri->u.pTelUri);
		if (pUri->u.pTelUri == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto End;
		}

		pToTelUri = pUri->u.pTelUri;
		if (*pUserID == '+')
		{
			pToTelUri->subscriberType = EcrioSigMgrTelSubscriberGlobal;
			pal_MemoryAllocate(sizeof(EcrioSigMgrTelGlobalNumberStruct), (void **)&pToTelUri->u.pGlobalNumber);
			if (pToTelUri->u.pGlobalNumber == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto End;
			}

			pToTelUri->u.pGlobalNumber->pGlobalNumberDigits = pal_StringCreate(pUserID + 1, pal_StringLength(pUserID + 1));
		}
		else
		{
			pToTelUri->subscriberType = EcrioSigMgrTelSubscriberLocal;
			pal_MemoryAllocate(sizeof(EcrioSigMgrTelLocalNumberStruct), (void **)&pToTelUri->u.pLocalNumber);
			if (pToTelUri->u.pLocalNumber == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto End;
			}

			pToTelUri->u.pLocalNumber->pLocalNumberDigits = pal_StringCreate(pUserID, pal_StringLength(pUserID));
			pal_MemoryAllocate(sizeof(EcrioSigMgrTelContextStruct), (void **)&pToTelUri->u.pLocalNumber->pContext);
			if (pToTelUri->u.pLocalNumber->pContext == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto End;
			}

			pToTelUri->u.pLocalNumber->pContext->contextType = EcrioSigMgrTelContextDomainName;
			pToTelUri->u.pLocalNumber->pContext->u.pDomainName = pal_StringCreate(pSigMgr->pSignalingInfo->pHomeDomain, pal_StringLength(pSigMgr->pSignalingInfo->pHomeDomain));
		}
	}

End:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 EcrioSigMgrGetOriginator
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrNameAddrStruct *pUri,
	u_char **ppUserID
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_char *pUserId = NULL;
	EcrioSigMgrStruct *pSigMgr = (EcrioSigMgrStruct *)sigMgrHandle;

	(void)pSigMgr;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pUri != NULL &&
		pUri->addrSpec.uriScheme == EcrioSigMgrURISchemeTEL)
	{
		if (pUri->addrSpec.u.pTelUri->subscriberType == EcrioSigMgrTelSubscriberGlobal)
		{
			pal_MemoryAllocate(((sizeof(u_char) * pal_StringLength(pUri->addrSpec.u.pTelUri->u.pGlobalNumber->pGlobalNumberDigits)) + 2), (void **)&(pUserId));
			if (pUserId == NULL ||
				pUri->addrSpec.u.pTelUri->u.pGlobalNumber->pGlobalNumberDigits == NULL)
			{
				goto End;
			}

			if (NULL == pal_StringNCopy((u_char *)pUserId, pal_StringLength(pUri->addrSpec.u.pTelUri->u.pGlobalNumber->pGlobalNumberDigits) + 2, (const u_char *)"+", pal_StringLength((const u_char*)"+")))
			{
				pal_MemoryFree((void **)&pUserId);
				goto End;
			}

			if (NULL == pal_StringNConcatenate((u_char *)pUserId, pal_StringLength(pUri->addrSpec.u.pTelUri->u.pGlobalNumber->pGlobalNumberDigits) + 2 - 1, (const u_char *)pUri->addrSpec.u.pTelUri->u.pGlobalNumber->pGlobalNumberDigits, pal_StringLength(pUri->addrSpec.u.pTelUri->u.pGlobalNumber->pGlobalNumberDigits)))
			{
				pal_MemoryFree((void **)&pUserId);
				goto End;				
			}
		}
		else if (pUri->addrSpec.u.pTelUri->subscriberType == EcrioSigMgrTelSubscriberLocal)
		{
			pUserId = pal_StringCreate(pUri->addrSpec.u.pTelUri->u.pLocalNumber->pLocalNumberDigits, pal_StringLength(pUri->addrSpec.u.pTelUri->u.pLocalNumber->pLocalNumberDigits));
			if (pUserId == NULL)
			{
				goto End;
			}
		}
	}
	else if (pUri != NULL && pUri->addrSpec.uriScheme == EcrioSigMgrURISchemeSIP)
	{
		pUserId = pal_StringCreate(pUri->addrSpec.u.pSipUri->pUserId, pal_StringLength(pUri->addrSpec.u.pSipUri->pUserId));
	}

	*ppUserID = pUserId;
	pUserId = NULL;
End:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrCopySipMessage
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSrcStruct,
	EcrioSigMgrSipMessageStruct *pDstStruct
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if ((pSrcStruct == NULL) || (pDstStruct == NULL))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data passed", __FUNCTION__, __LINE__);

		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}	/* if */

	if (pSrcStruct->pMandatoryHdrs)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&pDstStruct->pMandatoryHdrs);
		if (pDstStruct->pMandatoryHdrs == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}	/* if */

		error = _EcrioSigMgrPopulateManHdrs(pSigMgr, pSrcStruct->pMandatoryHdrs,
			pDstStruct->pMandatoryHdrs);

		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}	/* if */
	}

	if (pSrcStruct->pReasonPhrase)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcStruct->pReasonPhrase,
			&pDstStruct->pReasonPhrase);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	pDstStruct->contentLength = pSrcStruct->contentLength;
	pDstStruct->eMethodType = pSrcStruct->eMethodType;
	pDstStruct->eReqRspType = pSrcStruct->eReqRspType;
	pDstStruct->isRequestInValid = pSrcStruct->isRequestInValid;
	pDstStruct->responseCode = pSrcStruct->responseCode;
	pDstStruct->statusCode = pSrcStruct->statusCode;

	pDstStruct->pFetaureTags = pSrcStruct->pFetaureTags;
	pDstStruct->pConvId = pSrcStruct->pConvId;
	pDstStruct->bPrivacy = pSrcStruct->bPrivacy;
	pDstStruct->eModuleId = pSrcStruct->eModuleId;

	if (pSrcStruct->pMessageBody != NULL)
	{
		EcrioSigMgrUnknownMessageBodyStruct *pUnknownBody = NULL;
		pal_MemoryAllocate(sizeof(EcrioSigMgrMessageBodyStruct),
			(void **)&pDstStruct->pMessageBody);
		if (pDstStruct->pMessageBody == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pDstOptionalHdrs->pMessageBody, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		pal_MemoryAllocate(sizeof(EcrioSigMgrUnknownMessageBodyStruct),
			(void **)&pDstStruct->pMessageBody->pMessageBody);
		if (pDstStruct->pMessageBody->pMessageBody == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pDstOptionalHdrs->pMessageBody, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		pUnknownBody = (EcrioSigMgrUnknownMessageBodyStruct *)pDstStruct->pMessageBody->pMessageBody;

		pUnknownBody->bufferLength = ((EcrioSigMgrUnknownMessageBodyStruct *)pSrcStruct->pMessageBody->pMessageBody)->bufferLength;

		pal_MemoryAllocate(pUnknownBody->bufferLength, (void **)&pUnknownBody->pBuffer);
		if (pUnknownBody->pBuffer == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pUnknownBody->pBuffer, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		pal_MemoryCopy(pUnknownBody->pBuffer, pUnknownBody->bufferLength,
			((EcrioSigMgrUnknownMessageBodyStruct *)pSrcStruct->pMessageBody->pMessageBody)->pBuffer, pUnknownBody->bufferLength);

		error = _EcrioSigMgrStringCreate(pSigMgr,
			((EcrioSigMgrUnknownMessageBodyStruct *)pSrcStruct->pMessageBody->pMessageBody)->contentType.pHeaderValue,
			&pUnknownBody->contentType.pHeaderValue);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}

		if (((EcrioSigMgrUnknownMessageBodyStruct *)pSrcStruct->pMessageBody->pMessageBody)->contentType.numParams > 0 &&
			((EcrioSigMgrUnknownMessageBodyStruct *)pSrcStruct->pMessageBody->pMessageBody)->contentType.ppParams != NULL)
		{
			pUnknownBody->contentType.numParams = ((EcrioSigMgrUnknownMessageBodyStruct *)pSrcStruct->pMessageBody->pMessageBody)->contentType.numParams;

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected(pUnknownBody->contentType.numParams, sizeof(EcrioSigMgrParamStruct*)) == Enum_TRUE)
			{
				error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
				goto Error_Level_01;
			}

			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct*) * pUnknownBody->contentType.numParams,
				(void **)&pUnknownBody->contentType.ppParams);
			if (pUnknownBody->contentType.ppParams == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryAllocate() for pUnknownBody->contentType.ppParams, error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}

			u_int32 i;
			for (i = 0; i < pUnknownBody->contentType.numParams; i++)
			{
				pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&pUnknownBody->contentType.ppParams[i]);
				if (pUnknownBody->contentType.ppParams[i] == NULL)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tpal_MemoryAllocate() for pUnknownBody->contentType.ppParams[i], error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}
				error = _EcrioSigMgrStringCreate(pSigMgr,
					((EcrioSigMgrUnknownMessageBodyStruct *)pSrcStruct->pMessageBody->pMessageBody)->contentType.ppParams[i]->pParamName,
					&pUnknownBody->contentType.ppParams[i]->pParamName);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					goto Error_Level_01;
				}
				error = _EcrioSigMgrStringCreate(pSigMgr,
					((EcrioSigMgrUnknownMessageBodyStruct *)pSrcStruct->pMessageBody->pMessageBody)->contentType.ppParams[i]->pParamValue,
					&pUnknownBody->contentType.ppParams[i]->pParamValue);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					goto Error_Level_01;
				}
			}
		}
		else
		{
			pUnknownBody->contentType.numParams = 0;
			pUnknownBody->contentType.ppParams = NULL;
		}
	}	/* if */

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return error;
}


u_int32 _EcrioSigMgrCopySipMessageBody
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrMessageBodyStruct *pSrcStruct,
	EcrioSigMgrMessageBodyStruct *pDstStruct
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrUnknownMessageBodyStruct *pUnknownBody = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if ((pSrcStruct == NULL) || (pDstStruct == NULL))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data passed", __FUNCTION__, __LINE__);

		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}	/* if */	

	
	pal_MemoryAllocate(sizeof(EcrioSigMgrUnknownMessageBodyStruct),
		(void **)&pDstStruct->pMessageBody);
	if (pDstStruct->pMessageBody == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for pDstOptionalHdrs->pMessageBody, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}


	pUnknownBody = (EcrioSigMgrUnknownMessageBodyStruct *)pDstStruct->pMessageBody;

	pUnknownBody->bufferLength = ((EcrioSigMgrUnknownMessageBodyStruct *)pSrcStruct->pMessageBody)->bufferLength;

	pal_MemoryAllocate(pUnknownBody->bufferLength, (void **)&pUnknownBody->pBuffer);
	if (pUnknownBody->pBuffer == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for pUnknownBody->pBuffer, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	pal_MemoryCopy(pUnknownBody->pBuffer, pUnknownBody->bufferLength,
		((EcrioSigMgrUnknownMessageBodyStruct *)pSrcStruct->pMessageBody)->pBuffer, pUnknownBody->bufferLength);

	if (((EcrioSigMgrUnknownMessageBodyStruct *)pSrcStruct->pMessageBody)->contentType.pHeaderValue != NULL)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr,
			((EcrioSigMgrUnknownMessageBodyStruct *)pSrcStruct->pMessageBody)->contentType.pHeaderValue,
			&pUnknownBody->contentType.pHeaderValue);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}
	else
	{
		error = _EcrioSigMgrStringCreate(pSigMgr,
			(u_char *)"application/sdp",
			&pUnknownBody->contentType.pHeaderValue);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	if (((EcrioSigMgrUnknownMessageBodyStruct *)pSrcStruct->pMessageBody)->contentType.numParams > 0 &&
		((EcrioSigMgrUnknownMessageBodyStruct *)pSrcStruct->pMessageBody)->contentType.ppParams != NULL)
	{
		pUnknownBody->contentType.numParams = ((EcrioSigMgrUnknownMessageBodyStruct *)pSrcStruct->pMessageBody)->contentType.numParams;

		/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
		if (pal_UtilityDataOverflowDetected(pUnknownBody->contentType.numParams, sizeof(EcrioSigMgrParamStruct*)) == Enum_TRUE)
		{
			error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
			goto Error_Level_01;
		}

		pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct*) * pUnknownBody->contentType.numParams,
			(void **)&pUnknownBody->contentType.ppParams);
		if (pUnknownBody->contentType.ppParams == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pUnknownBody->contentType.ppParams, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		u_int32 i;
		for (i = 0; i < pUnknownBody->contentType.numParams; i++)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&pUnknownBody->contentType.ppParams[i]);
			if (pUnknownBody->contentType.ppParams[i] == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryAllocate() for pUnknownBody->contentType.ppParams[i], error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}
			error = _EcrioSigMgrStringCreate(pSigMgr,
				((EcrioSigMgrUnknownMessageBodyStruct *)pSrcStruct->pMessageBody)->contentType.ppParams[i]->pParamName,
				&pUnknownBody->contentType.ppParams[i]->pParamName);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error_Level_01;
			}
			error = _EcrioSigMgrStringCreate(pSigMgr,
				((EcrioSigMgrUnknownMessageBodyStruct *)pSrcStruct->pMessageBody)->contentType.ppParams[i]->pParamValue,
				&pUnknownBody->contentType.ppParams[i]->pParamValue);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error_Level_01;
			}
		}
	}
	else
	{
		pUnknownBody->contentType.numParams = 0;
		pUnknownBody->contentType.ppParams = NULL;
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return error;
}


u_int32 _EcrioSigMgrPopulateManHdrs
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrMandatoryHeaderStruct *pSrcManHdrs,
	EcrioSigMgrMandatoryHeaderStruct *pDstManHdrs
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR, i = 0;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams,
		"%s:%u\tpSigMgr=%p, pSrcManHdrs=%p, pDstManHdrs=%p",
		__FUNCTION__, __LINE__, pSigMgr, pSrcManHdrs, pDstManHdrs);

	if ((pSrcManHdrs == NULL) || (pDstManHdrs == NULL))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data passed", __FUNCTION__, __LINE__);

		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	// Stores Request URI
	if (pSrcManHdrs->pRequestUri != NULL)
	{
		if (pDstManHdrs->pRequestUri != NULL)
		{
			EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_Uri,
				(void **)&pDstManHdrs->pRequestUri, Enum_FALSE);
		}
		else
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrUriStruct), (void **)&pDstManHdrs->pRequestUri);
			if (pDstManHdrs->pRequestUri == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
		}

		error = _EcrioSigMgrPopulateUri(pSigMgr, pSrcManHdrs->pRequestUri,
			(pDstManHdrs->pRequestUri));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	// Stores Callid
	if (pSrcManHdrs->pCallId != NULL)
	{
		if (pDstManHdrs->pCallId != NULL)
		{
			pal_MemoryFree((void **)&pDstManHdrs->pCallId);
		}

		error = _EcrioSigMgrStringCreate(pSigMgr, pSrcManHdrs->pCallId,
			&(pDstManHdrs->pCallId));
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	// Stores TO uri
	if (pSrcManHdrs->pTo != NULL)
	{
		if (pDstManHdrs->pTo != NULL)
		{
			EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_NameAddrWithParams,
				(void **)&(pDstManHdrs->pTo), Enum_FALSE);
		}
		else
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct),
				(void **)&pDstManHdrs->pTo);
			if (pDstManHdrs->pTo == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
		}

		error = _EcrioSigMgrPopulateNameAddrWithParams(pSigMgr,
			pSrcManHdrs->pTo, pDstManHdrs->pTo);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	// Stores From uri
	if (pSrcManHdrs->pFrom != NULL)
	{
		if (pDstManHdrs->pFrom != NULL)
		{
			EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_NameAddrWithParams,
				(void **)&(pDstManHdrs->pFrom), Enum_FALSE);
		}
		else
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct),
				(void **)&pDstManHdrs->pFrom);
			if (pDstManHdrs->pFrom == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
		}

		error = _EcrioSigMgrPopulateNameAddrWithParams(pSigMgr,
			pSrcManHdrs->pFrom, pDstManHdrs->pFrom);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	// store cSeq number
	if (pSrcManHdrs->CSeq != 0)
	{
		pDstManHdrs->CSeq = pSrcManHdrs->CSeq;
	}

	// store max forward value
	if (pSrcManHdrs->maxForwards != 0)
	{
		pDstManHdrs->maxForwards = pSrcManHdrs->maxForwards;
	}

	// store Via header
	if (pSrcManHdrs->numVia > 0)
	{
		if (pDstManHdrs->numVia > 0)
		{
			for (i = 0; i < pDstManHdrs->numVia; ++i)
			{
				EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_Via,
					(void **)&(pDstManHdrs->ppVia[i]), Enum_TRUE);
			}

			pal_MemoryFree((void **)&pDstManHdrs->ppVia);
		}

		/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
		if (pal_UtilityDataOverflowDetected(pSrcManHdrs->numVia, sizeof(EcrioSigMgrViaStruct *)) == Enum_TRUE)
		{
			error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
			goto Error_Level_01;
		}

		pal_MemoryAllocate(sizeof(EcrioSigMgrViaStruct *) * (pSrcManHdrs->numVia),
			(void **)&pDstManHdrs->ppVia);
		if (pDstManHdrs->ppVia == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pDstManHdrs->numVia = pSrcManHdrs->numVia;

		for (i = 0; i < pDstManHdrs->numVia; ++i)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrViaStruct),
				(void **)&pDstManHdrs->ppVia[i]);
			if (pDstManHdrs->ppVia[i] == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}

			error = _EcrioSigMgrPopulateViaHdr(pSigMgr,
				pSrcManHdrs->ppVia[i], pDstManHdrs->ppVia[i]);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error_Level_01;
			}
		}
	}

	error = ECRIO_SIGMGR_NO_ERROR;

Error_Level_01:
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		_EcrioSigMgrReleaseManHdrStruct(pSigMgr, pDstManHdrs);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return error;
}

#if 0
/**************************************************************************
Function:		_EcrioSigMgrCopyACKCredentials()

Purpose:		Copy credentials.

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrCopyACKCredentials
(
	EcrioSigMgrStruct *pSigMgr,
	u_int16	*pDestNumProxyAuth,
	EcrioSigMgrAuthorizationStruct ***pppDestProxyAuth,
	EcrioSigMgrAuthorizationStruct **ppDestAuth,
	u_int16	srcNumProxyAuth,
	EcrioSigMgrAuthorizationStruct **ppSrcProxyAuth,
	EcrioSigMgrAuthorizationStruct *pSrcAuth
)
{
	u_int32	i = 0, error = ECRIO_SIGMGR_NO_ERROR;

	if (!pSigMgr)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto EndTag;
	}

	if (ppDestAuth)
	{
		if (pSrcAuth)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrAuthorizationStruct),
				(void **)ppDestAuth);
			if (ppDestAuth == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto EndTag;
			}

			error = _EcrioSigMgrPopulateAuthorizationDetails(pSigMgr,
				pSrcAuth,
				*ppDestAuth);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto EndTag;
			}
		}
	}

	if (pppDestProxyAuth && pDestNumProxyAuth)
	{
		if (ppSrcProxyAuth && srcNumProxyAuth)
		{
			pal_MemoryAllocate(srcNumProxyAuth * sizeof(EcrioSigMgrAuthorizationStruct *),
				(void **)pppDestProxyAuth);
			if (pppDestProxyAuth == NULL)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto EndTag;
			}

			for (i = 0; i < srcNumProxyAuth; i++)
			{
				if (ppSrcProxyAuth[i])
				{
					pal_MemoryAllocate(sizeof(EcrioSigMgrAuthorizationStruct),
						(void **)&((*pppDestProxyAuth)[i]));
					if ((*pppDestProxyAuth)[i] == NULL)
					{
						error = ECRIO_SIGMGR_NO_MEMORY;
						goto EndTag;
					}

					error = _EcrioSigMgrPopulateAuthorizationDetails(pSigMgr,
						ppSrcProxyAuth[i],
						(*pppDestProxyAuth)[i]);
					if (error != ECRIO_SIGMGR_NO_ERROR)
					{
						goto EndTag;
					}
				}
			}
		}

		*pDestNumProxyAuth = srcNumProxyAuth;
	}

EndTag:

	return error;
}
#endif

u_int32 _EcrioSigMgrAddLocalContactUri
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrContactStruct **ppUri,
	BoolEnum bPrivacy
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrContactStruct *pContactUri = NULL;
	EcrioSigMgrNameAddrWithParamsStruct *pNameAddr = NULL;
	EcrioSigMgrSipURIStruct *pSipUri = NULL;
	EcrioSigMgrParamStruct **ppParams = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pal_MemoryAllocate(sizeof(EcrioSigMgrContactStruct), (void **)&pContactUri);
	if (pContactUri == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	pContactUri->numContactUris = 1;
	pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct *),
		(void **)&pContactUri->ppContactDetails);
	if (pContactUri->ppContactDetails == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct),
		(void **)&pContactUri->ppContactDetails[0]);
	if (pContactUri->ppContactDetails[0] == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	pNameAddr = pContactUri->ppContactDetails[0];

	if ((pSigMgr->pSignalingInfo->pDisplayName != NULL) && (bPrivacy == Enum_FALSE))
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSigMgr->pSignalingInfo->pDisplayName,
			&pNameAddr->nameAddr.pDisplayName);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	pNameAddr->nameAddr.addrSpec.uriScheme = EcrioSigMgrURISchemeSIP;
	pal_MemoryAllocate(sizeof(EcrioSigMgrSipURIStruct),
		(void **)&pNameAddr->nameAddr.addrSpec.u.pSipUri);
	if (pNameAddr->nameAddr.addrSpec.u.pSipUri == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	pSipUri = pNameAddr->nameAddr.addrSpec.u.pSipUri;
#if 0
	if (pSigMgr->pSignalingInfo->uriScheme == EcrioSigMgrURISchemeSIP)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSigMgr->pSignalingInfo->pUserId,
			&pSipUri->pUserId);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}
	else if (pSigMgr->pSignalingInfo->uriScheme == EcrioSigMgrURISchemeTEL)	/* New_UA */
	{
		u_char *pTelUri = NULL;
		EcrioSigMgrUriStruct uri = { .uriScheme = EcrioSigMgrURISchemeNone };

		uri.uriScheme = EcrioSigMgrURISchemeTEL;
		// uri.u.pTelUri = pSigMgr->pSignalingInfo->p;
		error = _EcrioSigMgrFormUri(pSigMgr, &uri, &pTelUri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}

		error = _EcrioSigMgrStringCreate(pSigMgr, (pTelUri + 4),/* ignore tel: */
			&pSipUri->pUserId);
		pal_MemoryFree((void **)&pTelUri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}
	else
	{
		error = ECRIO_SIGMGR_INVALID_URI_SCHEME;
		goto Error_Level_01;
	}
#else
	EcrioSigMgrNameAddrWithParamsStruct *pNameAddrParams = NULL;
	EcrioSigMgrGetDefaultPUID(pSigMgr, EcrioSigMgrURISchemeSIP, &pNameAddrParams);
	if (pNameAddrParams != NULL &&
		pNameAddrParams->nameAddr.addrSpec.u.pSipUri->pUserId != NULL)
	{		
		error = _EcrioSigMgrStringCreate(pSigMgr, pNameAddrParams->nameAddr.addrSpec.u.pSipUri->pUserId,
			&pSipUri->pUserId);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}		

		EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_NameAddrWithParams, \
			(void **)&pNameAddrParams, Enum_TRUE);
	}
	else
	{
		SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tFailed to fetch UserID from PAI. Populate using signaling info.",
			__FUNCTION__, __LINE__);
		error = _EcrioSigMgrStringCreate(pSigMgr, pSigMgr->pSignalingInfo->pUserId,
			&pSipUri->pUserId);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}
#endif
	pal_MemoryAllocate(sizeof(EcrioSigMgrIPAddrStruct), (void **)&pSipUri->pIPAddr);
	if (pSipUri->pIPAddr == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	error = _EcrioSigMgrStringCreate(pSigMgr, pSigMgr->pNetworkInfo->pLocalIp,
		&pSipUri->pIPAddr->pIPAddr);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		goto Error_Level_01;
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
				pSipUri->pIPAddr->port = pChannels->pChannelInfo->localPort;
			}
		}
	}
	else
	{
		pSipUri->pIPAddr->port = (u_int16)pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[0].pChannelInfo->localPort;
	}

	if ((pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE) ||
		(pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_FALSE))
	{
		pSipUri->numURIParams = 1;
		pal_MemoryAllocate(pSipUri->numURIParams*sizeof(EcrioSigMgrParamStruct*), (void**)&pSipUri->ppURIParams);
		if (pSipUri->ppURIParams != NULL)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void**)&pSipUri->ppURIParams[0]);
			if (pSipUri->ppURIParams[0] != NULL)
			{
				pSipUri->ppURIParams[0]->pParamName = pal_StringCreate((const u_char*)"transport", pal_StringLength((const u_char*)"transport"));
							
				if (pSigMgr->pSignalingInfo->bEnableTls == Enum_TRUE && pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex != 0)
					pSipUri->ppURIParams[0]->pParamValue = pal_StringCreate((const u_char*)"TLS", pal_StringLength((const u_char*)"TLS"));
				else
					pSipUri->ppURIParams[0]->pParamValue = pal_StringCreate((const u_char*)"UDP", pal_StringLength((const u_char*)"UDP"));
			}
			else
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
		}
		else
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}
	}
	else if ((pSigMgr->pSignalingInfo->bEnableUdp == Enum_FALSE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE))
	{
		pSipUri->numURIParams = 1;
		pal_MemoryAllocate(pSipUri->numURIParams*sizeof(EcrioSigMgrParamStruct*), (void**)&pSipUri->ppURIParams);
		if (pSipUri->ppURIParams != NULL)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void**)&pSipUri->ppURIParams[0]);
			if (pSipUri->ppURIParams[0] != NULL)
			{
				pSipUri->ppURIParams[0]->pParamName = pal_StringCreate((const u_char*)"transport", pal_StringLength((const u_char*)"transport"));
				pSipUri->ppURIParams[0]->pParamValue = pal_StringCreate((const u_char*)"TCP", pal_StringLength((const u_char*)"TCP"));
			}
			else
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
		}
		else
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}
	}

	error = _EcrioSigMgrCopyParam(pSigMgr, pSigMgr->pSignalingInfo->numFeatureTagParams, pSigMgr->pSignalingInfo->ppFeatureTagParams, &ppParams);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		goto Error_Level_01;
	}

	pNameAddr->ppParams = ppParams;
	ppParams = NULL;
	pNameAddr->numParams = pSigMgr->pSignalingInfo->numFeatureTagParams;
	/*
	(pSigMgr->limsCallbackStruct.getPropertyCallbackFn)(EcrioSigMgrPropertyName_DEVICE_ID, EcrioSigMgrPropertyType_STRING, deviceID, pSigMgr->limsCallbackStruct.pCallbackData);
	if (pal_StringLength(deviceID) != 0)
	{

	    pal_StringCopy(str, (u_char*)"\"<");
	    pal_StringConcatenate(str, (u_char*)"urn:gsma:imei:");
	    pal_StringNConcatenate(str, deviceID, 8);
	    pal_StringConcatenate(str, (u_char*)"-");
	    pal_StringNConcatenate(str, deviceID + 8, 6);
	    pal_StringConcatenate(str, (u_char*)"-");
	    pal_StringConcatenate(str, deviceID + 14);
	    pal_StringConcatenate(str, (u_char*)">\"");

	    pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct*), (void**)&ppParams);
	    if (ppParams == NULL)
	    {
	        error = ECRIO_SIGMGR_NO_MEMORY;
	        goto Error_Level_01;
	    }
	    pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void**)&ppParams[0]);
	    if (ppParams[0] == NULL)
	    {
	        error = ECRIO_SIGMGR_NO_MEMORY;
	        goto Error_Level_01;
	    }
	    ppParams[0]->pParamName = pal_StringCreate(ECRIO_SIGMGR_SIP_INSTANCE_PARAM_NAME, pal_StringLength(ECRIO_SIGMGR_SIP_INSTANCE_PARAM_NAME));
	    if (ppParams[0]->pParamName == NULL)
	    {
	        error = ECRIO_SIGMGR_NO_MEMORY;
	        goto Error_Level_01;
	    }

	    ppParams[0]->pParamValue = pal_StringCreate(str, pal_StringLength(str));
	    if (ppParams[0]->pParamValue == NULL)
	    {
	        error = ECRIO_SIGMGR_NO_MEMORY;
	        goto Error_Level_01;
	    }

	    pNameAddr->ppParams = ppParams;
	    ppParams = NULL;
	    pNameAddr->numParams = 1;
	}
	*/
	*ppUri = pContactUri;

Error_Level_01:
	if ((error != ECRIO_SIGMGR_NO_ERROR) && (pContactUri != NULL))
	{
		EcrioSigMgrStructRelease((SIGMGRHANDLE)pSigMgr, EcrioSigMgrStructType_Contact,
			(void **)&pContactUri, Enum_TRUE);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return ECRIO_SIGMGR_NO_ERROR;
}

u_char *_EcrioSigMgrFindTagParam
(
	EcrioSigMgrStruct *pSigMgr,
	u_int32	numParams,
	EcrioSigMgrParamStruct **ppParams
)
{
	u_int32	i = 0;
	u_char *pTag = NULL;

	// @todo Do we need pSigMgr parameter in this function? Maybe for future logging?
	(void)pSigMgr;

	if (ppParams)
	{
		for (i = 0; i < numParams; i++)
		{
			if (ppParams[i] && ppParams[i]->pParamName && pal_StringCompare(ppParams[i]->pParamName, ECRIO_SIG_MGR_SIP_TAG_STRING) == 0)
			{
				pTag = ppParams[i]->pParamValue;
			}
		}
	}

	return pTag;
}

u_int32	_EcrioSigMgrAppendParam
(
	EcrioSigMgrStruct *pSigMgr,
	u_int16	*pNumParams,
	EcrioSigMgrParamStruct ***pppParams,
	u_char *pName,
	u_char *pValue
)
{
	u_int32	uError = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrParamStruct **ppParams = NULL;

	if (pSigMgr == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	ppParams = *pppParams;

	/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
	if (pal_UtilityDataOverflowDetected((*pNumParams), sizeof(EcrioSigMgrParamStruct *)) == Enum_TRUE)
	{
		uError = ECRIO_SIGMGR_NO_MEMORY;
		goto EndTag;
	}

	if (ppParams == NULL)
	{
		pal_MemoryAllocate((*pNumParams + 1) * sizeof(EcrioSigMgrParamStruct *), (void **)&ppParams);
	}
	else
	{
		pal_MemoryReallocate((*pNumParams + 1) * sizeof(EcrioSigMgrParamStruct *), (void **)&ppParams);
	}

	if (ppParams == NULL)
	{
		uError = ECRIO_SIGMGR_NO_MEMORY;
		goto EndTag;
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&ppParams[*pNumParams]);
	if (ppParams[*pNumParams] == NULL)
	{
		pal_MemoryFree((void **)&ppParams);
		uError = ECRIO_SIGMGR_NO_MEMORY;
		goto EndTag;
	}

	ppParams[*pNumParams]->pParamName = pal_StringCreate(pName, pal_StringLength(pName));
	if (pValue)
	{
		ppParams[*pNumParams]->pParamValue = pal_StringCreate(pValue, pal_StringLength(pValue));
	}

	*pppParams = ppParams;
	(*pNumParams)++;

EndTag:
	ppParams = NULL;
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return uError;
}

u_int32	EcrioSigMgrGetDefaultPUID
(
	SIGMGRHANDLE hSigMgrHanddle,
	EcrioSigMgrURISchemesEnum ePreferredUriType,
	EcrioSigMgrNameAddrWithParamsStruct	**ppDefaultPUID
)
{
	u_int32	uError = ECRIO_SIGMGR_NO_ERROR;
	u_int32 i = 0;
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrNameAddrWithParamsStruct	*pDefaultPUID = NULL;
	BoolEnum bFound = Enum_FALSE;

	pSigMgr = (EcrioSigMgrStruct *)hSigMgrHanddle;
	if(ppDefaultPUID == NULL || pSigMgr == NULL)
		return ECRIO_SIGMGR_INVALID_ARGUMENTS;

	if ((pSigMgr->pSignalingInfo->numPAssociatedURIs != 0) && (pSigMgr->pSignalingInfo->ppPAssociatedURI != NULL))
	{
		for (i = 0; i < pSigMgr->pSignalingInfo->numPAssociatedURIs; i++)
		{
			if (ePreferredUriType != EcrioSigMgrURISchemeNone && ePreferredUriType == pSigMgr->pSignalingInfo->ppPAssociatedURI[i]->nameAddr.addrSpec.uriScheme)
			{
				bFound = Enum_TRUE;
				break;
			}
		}
	}

	if (bFound == Enum_TRUE && i < pSigMgr->pSignalingInfo->numPAssociatedURIs)
	{
		uError = _EcrioSigMgrAllocateAndPopulateNameAddrWithParams(pSigMgr, pSigMgr->pSignalingInfo->ppPAssociatedURI[i], &pDefaultPUID);
		if (uError != ECRIO_SIGMGR_NO_ERROR)
		{
			*ppDefaultPUID = NULL;
			uError = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		}
		else
			*ppDefaultPUID = pDefaultPUID;
	} else{
		*ppDefaultPUID = NULL;
		uError = ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	return uError;
}

u_int32 EcrioSigMgrGetSubscriptionExpireInterval
(
	SIGMGRHANDLE hSigMgrHanddle
)
{
	EcrioSigMgrStruct *pSigMgr = NULL;

	pSigMgr = (EcrioSigMgrStruct *)hSigMgrHanddle;
	if ((pSigMgr != NULL) && (pSigMgr->pSignalingInfo != NULL))
	{
		return pSigMgr->pSignalingInfo->subscriptionExpires;
	}

	return 0;
}

u_int32 _EcrioSigMgrBase64Encode
(
	u_char **ppBase64Data,
	u_char *pSrcData,
	u_int32 srcDataLen,
	u_int32 base64DataLen
)
{
	/* basic initializations */
	u_int32	i = 0;
	u_int32	uCnt = 0;
	u_char *pTempBase64Data = NULL;
	u_char EncodeTable[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	if (!pSrcData || (srcDataLen == 0) || !ppBase64Data || !base64DataLen)
	{
		return 1;
	}

	// base64DataLen = Base64EncodeLength(srcDataLen);

	pal_MemoryAllocate(base64DataLen + 1, (void **)&pTempBase64Data);
	if (NULL == pTempBase64Data)
	{
		return 1;
	}

	pTempBase64Data[base64DataLen] = 0;

	pal_MemorySet(pTempBase64Data, '=', base64DataLen);
	*ppBase64Data = pTempBase64Data;

	i = 0;

	while (i < (srcDataLen - (srcDataLen % 3)))
	{
		/** Check for OOB reads */
		uCnt += 4;
		if (base64DataLen < uCnt)
		{
			pal_MemoryFree((void **)&pTempBase64Data);
			return 1;
		}

		*pTempBase64Data++ = EncodeTable[pSrcData[i] >> 2];
		*pTempBase64Data++ = EncodeTable[((pSrcData[i] & 0x03) << 4) | (pSrcData[i + 1] >> 4)];
		*pTempBase64Data++ = EncodeTable[((pSrcData[i + 1] & 0x0f) << 2) | (pSrcData[i + 2] >> 6)];
		*pTempBase64Data++ = EncodeTable[pSrcData[i + 2] & 0x3f];
		i += 3;
	}

	if (srcDataLen - i)
	{
		/** Check for OOB reads */
		uCnt += 1;
		if (base64DataLen < uCnt)
		{
			pal_MemoryFree((void **)&pTempBase64Data);
			return 1;
		}

		*pTempBase64Data++ = EncodeTable[pSrcData[i] >> 2];

		if (srcDataLen - i == 1)
		{
			/** Check for OOB reads */
			uCnt += 1;
			if (base64DataLen < uCnt)
			{
				pal_MemoryFree((void **)&pTempBase64Data);
				return 1;
			}

			*pTempBase64Data = EncodeTable[(pSrcData[i] & 0x03) << 4];
		}
		else
		{
			/** Check for OOB reads */
			uCnt += 2;
			if (base64DataLen < uCnt)
			{
				pal_MemoryFree((void **)&pTempBase64Data);
				return 1;
			}

			*pTempBase64Data++ = EncodeTable[((pSrcData[i] & 0x03) << 4) | (pSrcData[i + 1] >> 4)];
			*pTempBase64Data = EncodeTable[(pSrcData[i + 1] & 0x0f) << 2];
		}
	}

	return 0;
}

u_int32 _EcrioSigMgrBase64EncodeRequireBufferLength
(
	u_int32 len
)
{
	/* return ((len+2)/3 * 4 + 1) */
	return 4 * (len / 3 + ((len % 3) ? 1 : 0));
}

u_int32 _EcrioSigMgrBase64Decode
(
	u_char **ppDecodedData,
	u_char *pBase64Data,
	u_int32 base64DataLen,
	u_int32 decodedDataLen
)
{
	/* basic initializations */
	u_char *pTempDecodedData = NULL;
	u_char *pTemp = NULL;
	u_char c = 0, d = 0;
	u_char DecodeTable[123];
	u_char pPad[] = "===";
	u_char CharecterCount = 0;
	u_int32	i = 0, j = 0;

	if (!pBase64Data || (base64DataLen == 0) || !ppDecodedData || !decodedDataLen)
	{
		return 1;
	}

	pTemp = _EcrioSigMgrFindCharInString(pBase64Data, pPad[0]);
	if (pTemp != NULL)
	{
		if ((u_int32)(pTemp - pBase64Data) < (base64DataLen - 3))
		{
			return 1;
		}
		else if (pal_StringNCompare(pTemp, pPad + 3 - (pBase64Data + base64DataLen - pTemp), pBase64Data + base64DataLen - pTemp))
		{
			return 1;
		}
	}

	for (CharecterCount = 0; CharecterCount < 123; CharecterCount++)
	{
		DecodeTable[CharecterCount] = 0x80;
	}

	for (CharecterCount = 'A'; CharecterCount <= 'Z'; CharecterCount++)
	{
		DecodeTable[CharecterCount] = (CharecterCount - 'A');
	}

	for (CharecterCount = 'a'; CharecterCount <= 'z'; CharecterCount++)
	{
		DecodeTable[CharecterCount] = 26 + (CharecterCount - 'a');
	}

	for (CharecterCount = '0'; CharecterCount <= '9'; CharecterCount++)
	{
		DecodeTable[CharecterCount] = 52 + (CharecterCount - '0');
	}

	DecodeTable['+'] = 62;
	DecodeTable['/'] = 63;
	DecodeTable['='] = 0;

	// decodedDataLen = Base64DecodeLength(base64DataLen);
	pal_MemoryAllocate(decodedDataLen + 1, (void **)&pTempDecodedData);
	if (NULL == pTempDecodedData)
	{
		return 0;
	}

	pTempDecodedData[decodedDataLen] = '\0';

	pTemp = pBase64Data;
	i = 0;

	c = *pTemp++;

	while ((c != '\0') && i < base64DataLen && j < decodedDataLen)
	{
		/** Boundary check */
		if (c >= 123)
		{
			pal_MemoryFree((void **)&pTempDecodedData);
			*ppDecodedData = NULL;
			return 1;
		}
		else if (c == pPad[0])
		{
			break;
		}

		d = DecodeTable[c];

		switch (i % 4)
		{
			case 0:
			{
				pTempDecodedData[j] = d << 2;
			}
			break;

			case 1:
			{
				pTempDecodedData[j++] |= d >> 4;
				pTempDecodedData[j] = (d & 0x0f) << 4;
			}
			break;

			case 2:
			{
				pTempDecodedData[j++] |= d >> 2;
				pTempDecodedData[j] = (d & 0x03) << 6;
			}
			break;

			case 3:
				pTempDecodedData[j++] |= d;
		}

		i++;
		c = *pTemp++;
	}

	*ppDecodedData = pTempDecodedData;
	return 0;
}

u_char *_EcrioSigMgrFindCharInString
(
	u_char *pString,
	u_char chr
)
{
	u_int32 i = 0;
	u_int32 stringLength = pal_StringLength(pString);

	for (i = 0; i < stringLength; i++)
	{
		if (pString[i] == chr)
		{
			return pString + i;
		}
	}

	return 0;
}

u_int32 _EcrioSigMgrBase64DecodeRequireBufferLength
(
	u_int32 len
)
{
	return 3 * (len / 4);
}

u_int32 _EcrioSigMgrAddSessionExpiresHeader
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList,
	BoolEnum isSessionRefresh,
	BoolEnum isRefresher,
	u_int32 uRefreshValue
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_char uExpStr[8] = { 0 };
	u_char **ppVal = NULL;
	u_char **ppSessionExpiresParam = NULL;
	u_char **ppSessionExpiresVal = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pOptionalHeaderList == NULL || pSigMgr->pSignalingInfo == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	if (0 >= pal_NumToString(uRefreshValue, uExpStr, 8))
	{
		error = ECRIO_SIGMGR_STRING_ERROR;
		goto Error_Level_01;
	}
	pal_MemoryAllocate(1 * sizeof(u_char *), (void **)&ppVal);
	if (ppVal == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}
	ppVal[0] = pal_StringCreate(uExpStr, pal_StringLength(uExpStr));

	if (isSessionRefresh == Enum_TRUE)
	{
		pal_MemoryAllocate(sizeof(u_char *), (void **)&ppSessionExpiresParam);
		if (ppSessionExpiresParam == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}
		ppSessionExpiresParam[0] = pal_StringCreate((u_char *)ECRIO_SIG_MGR_SESSION_REFRESHER_PARAM, pal_StringLength((u_char *)ECRIO_SIG_MGR_SESSION_REFRESHER_PARAM));

		pal_MemoryAllocate(sizeof(u_char *), (void **)&ppSessionExpiresVal);
		if (ppSessionExpiresVal == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}
		if (isRefresher == Enum_TRUE)
		{
			ppSessionExpiresVal[0] = pal_StringCreate((u_char *)ECRIO_SIG_MGR_SESSION_REFRESHER_UAS, pal_StringLength((u_char *)ECRIO_SIG_MGR_SESSION_REFRESHER_UAS));
		}
		else
		{
			ppSessionExpiresVal[0] = pal_StringCreate((u_char *)ECRIO_SIG_MGR_SESSION_REFRESHER_UAC, pal_StringLength((u_char *)ECRIO_SIG_MGR_SESSION_REFRESHER_UAC));
		}

		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pOptionalHeaderList, EcrioSipHeaderTypeSessionExpires, 1, ppVal, 1, ppSessionExpiresParam, ppSessionExpiresVal);
	}
	else
	{
		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pOptionalHeaderList, EcrioSipHeaderTypeSessionExpires, 1, ppVal, 0, NULL, NULL);
	}

	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

Error_Level_01:

	if (ppVal)
	{
		if (ppVal[0])
		{
			pal_MemoryFree((void **)&ppVal[0]);
		}

		pal_MemoryFree((void **)&ppVal);
	}

	if (ppSessionExpiresParam)
	{
		if (ppSessionExpiresParam[0])
		{
			pal_MemoryFree((void **)&ppSessionExpiresParam[0]);
		}

		pal_MemoryFree((void **)&ppSessionExpiresParam);
	}

	if (ppSessionExpiresVal)
	{
		if (ppSessionExpiresVal[0])
		{
			pal_MemoryFree((void **)&ppSessionExpiresVal[0]);
		}

		pal_MemoryFree((void **)&ppSessionExpiresVal);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrAddMinSEHeader
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList,
	u_int32 uMinSEValue
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_char uExpStr[8] = { 0 };
	u_char **ppVal = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pOptionalHeaderList == NULL || pSigMgr->pSignalingInfo == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	if (uMinSEValue != 0)
	{
		if (0 >= pal_NumToString(uMinSEValue, uExpStr, 8))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			goto Error_Level_01;
		}
	}
	else
	{
		if (0 >= pal_NumToString(pSigMgr->pSignalingInfo->minSe, uExpStr, 8))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			goto Error_Level_01;
		}
	}
	pal_MemoryAllocate(1 * sizeof(u_char *), (void **)&ppVal);
	if (ppVal == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}
	ppVal[0] = pal_StringCreate(uExpStr, pal_StringLength(uExpStr));

	error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pOptionalHeaderList, EcrioSipHeaderTypeMinSE, 1, ppVal, 0, NULL, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

Error_Level_01:

	if (ppVal)
	{
		if (ppVal[0])
		{
			pal_MemoryFree((void **)&ppVal[0]);
		}

		pal_MemoryFree((void **)&ppVal);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrAddP_EarlyMediaHeader
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessageStruct
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int16 numHeaderValues = 0;
	u_char **ppVal = NULL;

	u_char *pEarlyMedia = NULL;

	void *pOptionalHeaderList;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pSipMessageStruct == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	pOptionalHeaderList = pSipMessageStruct->pOptionalHeaderList;

	if (pOptionalHeaderList == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	/* Generate header */
	numHeaderValues = 1;

	pal_MemoryAllocate(numHeaderValues * sizeof(u_char *), (void **)&ppVal);
	if (ppVal == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	pEarlyMedia = _EcrioSigMgrGetFeatureTagValue(pSipMessageStruct->pFetaureTags, EcrioSipHeaderTypeP_EarlyMedia);

	if (pEarlyMedia)
	{
		ppVal[0] = pal_StringCreate(pEarlyMedia, pal_StringLength(pEarlyMedia));
	}

	if (ppVal[0] == NULL)
	{
		ppVal[0] = pal_StringCreate(ECRIO_SIGMGR_LIMS_3GPP_P_EARLY_MEDIA_VALUE_STRING, pal_StringLength(ECRIO_SIGMGR_LIMS_3GPP_P_EARLY_MEDIA_VALUE_STRING));
	}

	error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pOptionalHeaderList, EcrioSipHeaderTypeP_EarlyMedia, numHeaderValues,
		ppVal, 0, NULL, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

Error_Level_01:

	if (ppVal)
	{
		if (ppVal[0])
		{
			pal_MemoryFree((void **)&ppVal[0]);
		}

		pal_MemoryFree((void **)&ppVal);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}


u_int32 _EcrioSigMgrAddP_PreferredServiceHeader
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessageStruct
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int16 numHeaderValues = 0;
	u_char **ppVal = NULL;

	u_char *pPreferedService = NULL;

	void *pOptionalHeaderList;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pSipMessageStruct == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	pOptionalHeaderList = pSipMessageStruct->pOptionalHeaderList;

	if (pOptionalHeaderList == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	/* Generate header */
	numHeaderValues = 1;

	pal_MemoryAllocate(numHeaderValues * sizeof(u_char *), (void **)&ppVal);
	if (ppVal == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	pPreferedService = _EcrioSigMgrGetFeatureTagValue(pSipMessageStruct->pFetaureTags, EcrioSipHeaderTypeP_PreferredService);

	if (pPreferedService)
	{
		ppVal[0] = pal_StringCreate(pPreferedService, pal_StringLength(pPreferedService));
	}

	if (ppVal[0] == NULL)
	{
		ppVal[0] = pal_StringCreate(ECRIO_SIGMGR_LIMS_3GPP_P_PREFERRED_SERVICE_VALUE_STRING, pal_StringLength(ECRIO_SIGMGR_LIMS_3GPP_P_PREFERRED_SERVICE_VALUE_STRING));
	}

	error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pOptionalHeaderList, EcrioSipHeaderTypeP_PreferredService, numHeaderValues,
		ppVal, 0, NULL, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

Error_Level_01:

	if (ppVal)
	{
		if (ppVal[0])
		{
			pal_MemoryFree((void **)&ppVal[0]);
		}

		pal_MemoryFree((void **)&ppVal);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrAddAccptContactHeader
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessageStruct
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int16 numHeaderValues = 0;
	u_char **ppVal = NULL;
	u_char *pAcceptContact = NULL;
	void *pOptionalHeaderList;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pSipMessageStruct == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	pOptionalHeaderList = pSipMessageStruct->pOptionalHeaderList;

	if (pOptionalHeaderList == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	/* Generate header */
	numHeaderValues = 1;

	pal_MemoryAllocate(numHeaderValues * sizeof(u_char *), (void **)&ppVal);
	if (ppVal == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	pAcceptContact = _EcrioSigMgrGetFeatureTagValue(pSipMessageStruct->pFetaureTags, EcrioSipHeaderTypeAcceptContact);

	if (pAcceptContact)
	{
		u_char *pStar = (u_char*)"*";
		ppVal[0] = pal_StringCreate(pAcceptContact, pal_StringLength(pAcceptContact));
		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pOptionalHeaderList, EcrioSipHeaderTypeAcceptContact, numHeaderValues,
			&pStar, 1, ppVal, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	/*if (ppVal[0] == NULL)
	{
		ppVal[0] = pal_StringCreate(ECRIO_SIGMGR_LIMS_3GPP_ACCEPT_COONTACT_FEATURE_TAG_VALUE_STRING, pal_StringLength(ECRIO_SIGMGR_LIMS_3GPP_ACCEPT_COONTACT_FEATURE_TAG_VALUE_STRING));
	}*/

	/*error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pOptionalHeaderList, EcrioSipHeaderTypeAcceptContact, numHeaderValues,
		ppVal, 0, NULL, NULL);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}*/

Error_Level_01:

	if (ppVal)
	{
		if (ppVal[0])
		{
			pal_MemoryFree((void **)&ppVal[0]);
		}

		pal_MemoryFree((void **)&ppVal);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}


u_char *_EcrioSigMgrGetFeatureTagValue
(
	EcrioSigMgrFeatureTagStruct *pFetaureTags,
	EcrioSipHeaderTypeEnum eHeaderType
)
{
	u_int16 i = 0;

	if (!pFetaureTags)
	{
		return NULL;
	}

	for (i = 0; i < pFetaureTags->uNumberOfTags; i++)
	{
		if (eHeaderType == _EcrioSigMgrGetHeaderTypeFromString(pFetaureTags->ppFeatureTagHeaderValue[i]->pParamName))
		{
			return pFetaureTags->ppFeatureTagHeaderValue[i]->pParamValue;
		}
	}

	return NULL;
}

u_int32 _EcrioSigMgrAddRequestDispositionHeader
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int16 numHeaderValues = 0;
	u_char **ppVal = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pOptionalHeaderList == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	/* Generate header */
	numHeaderValues = 1;

	pal_MemoryAllocate(numHeaderValues * sizeof(u_char *), (void **)&ppVal);
	if (ppVal == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	// TODO - get the value from OOM module. At present hard-coding here

	if (ppVal[0] == NULL)
	{
		ppVal[0] = pal_StringCreate((const u_char *)ECRIO_SIGMGR_REQUEST_DISPOSITION_VALUE_NO_FORK, pal_StringLength((const u_char *)ECRIO_SIGMGR_REQUEST_DISPOSITION_VALUE_NO_FORK));
	}

	error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pOptionalHeaderList, EcrioSipHeaderTypeRequestDisposition, numHeaderValues,
		ppVal, 0, NULL, NULL);

	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

Error_Level_01:

	if (ppVal)
	{
		if (ppVal[0])
		{
			pal_MemoryFree((void **)&ppVal[0]);
		}

		pal_MemoryFree((void **)&ppVal);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);
	return error;
}

EcrioSigMgrRouteStruct *_EcrioSigMgrAddRouteHeader
(
	EcrioSigMgrStruct *pSigMgr
)
{
	EcrioSigMgrSipURIStruct	*pSipURIStruct = NULL;
	EcrioSigMgrIPAddrStruct *pIpAddrStruct = NULL;
	EcrioSigMgrRouteStruct *pRouteSet = NULL;

	if (pSigMgr == NULL)
	{
		return NULL;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pal_MemoryAllocate(sizeof(EcrioSigMgrRouteStruct), (void **)&pRouteSet);
	if (pRouteSet == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for pRouteSet",
			__FUNCTION__, __LINE__);
		goto Error_Level_02;
	}

	pRouteSet->numRoutes = 1;
	pal_MemoryAllocate(pRouteSet->numRoutes * sizeof(EcrioSigMgrNameAddrWithParamsStruct *), (void **)&pRouteSet->ppRouteDetails);
	if (pRouteSet->ppRouteDetails == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for pRouteSet->ppRouteDetails",
			__FUNCTION__, __LINE__);
		goto Error_Level_02;
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct), (void **)&pRouteSet->ppRouteDetails[0]);
	if (pRouteSet->ppRouteDetails[0] == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for pRouteSet->ppRouteDetails[0]",
			__FUNCTION__, __LINE__);
		goto Error_Level_02;
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrSipURIStruct), (void **)&pSipURIStruct);
	if (pSipURIStruct == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for pSipURIStruct",
			__FUNCTION__, __LINE__);
		goto Error_Level_02;
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrIPAddrStruct), (void **)&pIpAddrStruct);
	if (pIpAddrStruct == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for pIpAddrStruct",
			__FUNCTION__, __LINE__);
		goto Error_Level_02;
	}

	pIpAddrStruct->pIPAddr = pal_StringCreate(pSigMgr->pNetworkInfo->pRemoteIP, pal_StringLength(pSigMgr->pNetworkInfo->pRemoteIP));
	pIpAddrStruct->port = (u_int16)pSigMgr->pNetworkInfo->uRemotePort;
	pSipURIStruct->pIPAddr = pIpAddrStruct;

	pSipURIStruct->numURIParams = 1;

	pal_MemoryAllocate(pSipURIStruct->numURIParams * sizeof(EcrioSigMgrParamStruct *), (void **)&pSipURIStruct->ppURIParams);
	if (pSipURIStruct->ppURIParams == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for pSipURIStruct->ppURIParams",
			__FUNCTION__, __LINE__);
		goto Error_Level_02;
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&pSipURIStruct->ppURIParams[0]);
	if (pSipURIStruct->ppURIParams[0] == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for pSipURIStruct->ppURIParams",
			__FUNCTION__, __LINE__);
		goto Error_Level_02;
	}

	pSipURIStruct->ppURIParams[0]->pParamName = pal_StringCreate((u_char *)"lr", pal_StringLength((u_char *)"lr"));

	pRouteSet->ppRouteDetails[0]->nameAddr.addrSpec.uriScheme = EcrioSigMgrURISchemeSIP;
	pRouteSet->ppRouteDetails[0]->nameAddr.addrSpec.u.pSipUri = pSipURIStruct;

	goto Error_Level_01;
Error_Level_02:
	EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_SipURI,
		(void **)&(pSipURIStruct), Enum_FALSE);

	EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_Route,
		(void **)&(pRouteSet), Enum_FALSE);

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
	return pRouteSet;
}

u_int32 _EcrioSigMgrAddUserPhoneParam
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUriStruct *pUri
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrSipURIStruct *pSipUri = NULL;
	u_int16 index = 0;
	BoolEnum bIsUserFound = Enum_FALSE;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pUri == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	if (pUri->uriScheme == EcrioSigMgrURISchemeSIP)
	{
		pSipUri = pUri->u.pSipUri;

		if (pSipUri->numURIParams > 0)
		{
			for (index = 0; index < pSipUri->numURIParams; index++)
			{
				if (pal_StringCompare(pSipUri->ppURIParams[index]->pParamName, (u_char *)ECRIO_SIGMGR_SIP_URI_PARAM_NAME) == 0)
				{
					bIsUserFound = Enum_TRUE;
					break;
				}
			}
		}

		if (bIsUserFound == Enum_TRUE)
		{
			SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tuser=phone parameter is already added",	__FUNCTION__, __LINE__);
			/*Already added just return from this function*/
			error = ECRIO_SIGMGR_NO_ERROR;
			goto Error_Level_01;
		}

		pSipUri->numURIParams++;

		/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
		if (pal_UtilityDataOverflowDetected(pSipUri->numURIParams, sizeof(EcrioSigMgrParamStruct *)) == Enum_TRUE)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		if (pSipUri->ppURIParams == NULL)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct *),
				(void **)&(pSipUri->ppURIParams));
		}
		else
		{
			pal_MemoryReallocate(sizeof(EcrioSigMgrParamStruct *) * (pSipUri->numURIParams),
				(void **)&(pSipUri->ppURIParams));
		}

		if (pSipUri->ppURIParams == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		for (index = 0; index < pSipUri->numURIParams; index++)
		{
			if (pSipUri->ppURIParams[index] == NULL)
			{
				pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct),
					(void **)&(pSipUri->ppURIParams[index]));
				if (pSipUri->ppURIParams[index] == NULL)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					goto Error_Level_01;
				}
				break;
			}
		}

		if (index >= pSipUri->numURIParams)
		{
			error = ECRIO_SIGMGR_UNKNOWN_ERROR;
			goto Error_Level_01;
		}

		error = _EcrioSigMgrStringCreate(pSigMgr, (u_char *)ECRIO_SIGMGR_SIP_URI_PARAM_NAME,
			&pSipUri->ppURIParams[index]->pParamName);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_Params, (void **)(pSipUri->ppURIParams), Enum_TRUE);
			goto Error_Level_01;
		}

		error = _EcrioSigMgrStringCreate(pSigMgr, (u_char *)ECRIO_SIGMGR_SIP_URI_PARAM_VALUE,
			&pSipUri->ppURIParams[index]->pParamValue);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_Params, (void **)(pSipUri->ppURIParams), Enum_TRUE);
			goto Error_Level_01;
		}
	}
	else
	{
		/*TODO:*/
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32	EcrioSigMgrGetFeatureTag
(
	SIGMGRHANDLE sigMgrHandle,
	u_char *pFeatureTagName,
	u_char **ppFeatureTagValue
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	u_int16 index = 0;

	/* check for incoming parameters*/
	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry,
		"%s:%u", __FUNCTION__, __LINE__);

	if (!pFeatureTagName || !ppFeatureTagValue)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tinput datas are NULL",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	if (pSigMgr->pSignalingInfo == NULL ||
		pSigMgr->pSignalingInfo->ppFeatureTagParams == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tSigMgr not initialized",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_NOT_INITIALIZED;
		goto Error_Level_01;
	}

	for (index = 0; index < pSigMgr->pSignalingInfo->numFeatureTagParams; index++)
	{
		if (pal_StringCompare(pFeatureTagName, pSigMgr->pSignalingInfo->ppFeatureTagParams[index]->pParamName) == 0)
		{
			*ppFeatureTagValue = pSigMgr->pSignalingInfo->ppFeatureTagParams[index]->pParamValue;
			break;
		}
	}

	if (index == pSigMgr->pSignalingInfo->numFeatureTagParams)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tFeature tag not found",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_UNKNOWN_ERROR;
		goto Error_Level_01;
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

EcrioSigMgrCallbackRegisteringModuleEnums _EcrioSigMgrGetModuleId
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pCallId
)
{
	//EcrioSigMgrCallbackRegisteringModuleEnums *pModule = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;

	ec_MapGetKeyData(pSigMgr->hHashMap, pCallId, (void**)&pDialogNode);
	if (pDialogNode)
	{
		return pDialogNode->eModule;
	}
	return 0;
}

/*Store the module id. so that sigmgr can route the incoming message with same call id*/
u_int32 _EcrioSigMgrStoreModuleRoutingInfo
(
	EcrioSigMgrStruct *pSigMgr,
	u_char* pCallId,
	EcrioSigMgrCallbackRegisteringModuleEnums eModuleId
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	ec_MapGetKeyData(pSigMgr->hHashMap, pCallId, (void**)&pDialogNode);
	if (pDialogNode)
	{
		pDialogNode->eModule = eModuleId;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

u_int32 _EcrioSigMgrGetConversationsIdHeader
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessage
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrHeaderStruct *pConvHeader = NULL;
	EcrioSigMgrHeaderStruct *pContHeader = NULL;
	EcrioSigMgrHeaderStruct *pInReplyToHeader = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pal_MemoryAllocate(sizeof(EcrioSigMgrConversationsIdStruct), (void **)&pSipMessage->pConvId);
	if (pSipMessage->pConvId == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for pSipMessage->pId, error=%u",
			__FUNCTION__, __LINE__, error);

		goto Error_Level_01;
	}

	EcrioSigMgrGetOptionalHeader(pSigMgr, pSipMessage->pOptionalHeaderList,
		EcrioSipHeaderTypeConversationID, &pConvHeader);
	if (pConvHeader != NULL)
	{
		pSipMessage->pConvId->pConversationId = pal_StringCreate(pConvHeader->ppHeaderValues[0]->pHeaderValue,
			pal_StringLength(pConvHeader->ppHeaderValues[0]->pHeaderValue));
	}

	EcrioSigMgrGetOptionalHeader(pSigMgr, pSipMessage->pOptionalHeaderList,
		EcrioSipHeaderTypeContributionID, &pContHeader);
	if (pContHeader != NULL)
	{
		pSipMessage->pConvId->pContributionId = pal_StringCreate(pContHeader->ppHeaderValues[0]->pHeaderValue,
			pal_StringLength(pContHeader->ppHeaderValues[0]->pHeaderValue));
	}

	EcrioSigMgrGetOptionalHeader(pSigMgr, pSipMessage->pOptionalHeaderList,
		EcrioSipHeaderTypeInReplyToContributionID, &pInReplyToHeader);
	if (pInReplyToHeader != NULL)
	{
		pSipMessage->pConvId->pInReplyToContId = pal_StringCreate(pInReplyToHeader->ppHeaderValues[0]->pHeaderValue,
			pal_StringLength(pInReplyToHeader->ppHeaderValues[0]->pHeaderValue));
	}

	if (pConvHeader == NULL && pContHeader == NULL && pInReplyToHeader == NULL)
	{
		pal_MemoryFree((void**)&pSipMessage->pConvId);
	}
	
Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

EcrioSigMgrCallbackRegisteringModuleEnums _EcrioSigMgrFindModuleId
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessage
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR, i = 0;
	EcrioSigMgrHeaderStruct *pHeader = NULL;
	EcrioSigMgrParamStruct *pParam = NULL;
	u_char* pFound = NULL;
	EcrioSigMgrCallbackRegisteringModuleEnums eModuleId = EcrioSigMgrCallbackRegisteringModule_LIMS;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	EcrioSigMgrGetOptionalHeader(pSigMgr, pSipMessage->pOptionalHeaderList,
		EcrioSipHeaderTypeAcceptContact, &pHeader);
	if (pHeader != NULL)
	{
		if (pHeader->ppHeaderValues[0]->ppParams != NULL)
		{
			for (i = 0; i < pHeader->ppHeaderValues[0]->numParams; i++)
			{
				pParam = pHeader->ppHeaderValues[0]->ppParams[i];
				
				if (pal_StringCompare(pParam->pParamName, (u_char*)"+g.3gpp.icsi-ref") == 0)
				{
					pFound = pal_SubString(pParam->pParamValue, (u_char*)"icsi.oma.cpm.");
					if (pFound != NULL)
					{
						eModuleId = EcrioSigMgrCallbackRegisteringModule_CPM;
						break;
					}
				}

				if (pal_StringCompare(pParam->pParamName, (u_char*)"+g.3gpp.iari-ref") == 0)
				{
					pFound = pal_SubString(pParam->pParamValue, (u_char*)"iari.rcs.");
					if (pFound != NULL)
					{
						eModuleId = EcrioSigMgrCallbackRegisteringModule_CPM;
						break;
					}
				}
			}
		}
	}

	if (pFound == NULL && pSipMessage->pContentType != NULL)
	{
		u_char* pContentType = pSipMessage->pContentType->ppHeaderValues[0]->pHeaderValue;
		if (pal_StringCompare(pContentType, (u_char*)"message/cpim") == 0)
		{
			eModuleId = EcrioSigMgrCallbackRegisteringModule_CPM;
		}		
	}
	else if (pSipMessage->pMessageBody != NULL)
	{
		if (pSipMessage->pMessageBody->messageBodyType == EcrioSigMgrMessageBodyUnknown ||
			pSipMessage->pMessageBody->messageBodyType == EcrioSigMgrMessageBodySDP)
		{
			u_char* pBuf = ((EcrioSigMgrUnknownMessageBodyStruct*)pSipMessage->pMessageBody->pMessageBody)->pBuffer;
			if (pal_StringFindSubString(pBuf, (u_char*)"accept-types:message/cpim") != NULL)
				eModuleId = EcrioSigMgrCallbackRegisteringModule_CPM;
		}	
	}

	if (eModuleId == EcrioSigMgrCallbackRegisteringModule_LIMS)
	{
		switch (pSipMessage->eMethodType)
		{
		case EcrioSipMessageTypeRegister:
		case EcrioSipMessageTypeNotify:
			eModuleId = EcrioSigMgrCallbackRegisteringModule_SUE; //todo: Event header needs to be serached for NOTIFY
			break;
		case EcrioSipMessageTypeMessage:
			eModuleId = EcrioSigMgrCallbackRegisteringModule_SMS; 
			break;
		case EcrioSipMessageTypeInvite:
		case EcrioSipMessageTypeAck:
		case EcrioSipMessageTypeCancel:
		case EcrioSipMessageTypeBye:
		case EcrioSipMessageTypePrack:
		case EcrioSipMessageTypeUpdate:
			eModuleId = EcrioSigMgrCallbackRegisteringModule_MoIP;
			break;
		default:
			break;
		}
	}


	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return eModuleId;
}


u_int32 _EcrioSigMgrAddP_PreferrdIdHeader
(
	EcrioSigMgrStruct *pSigMgr, 
	EcrioSigMgrSipMessageStruct *pSipMessageStruct
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrNameAddrWithParamsStruct *pNameAddrParams = NULL;
	
	/*Populate P-Prefered-Identity from PAU header. If PAU not present then populate this from public userid.*/
	pSipMessageStruct->numPPreferredIdentity = 1;
	pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrStruct *), (void **)&pSipMessageStruct->ppPPreferredIdentity);
	if (pSipMessageStruct->ppPPreferredIdentity == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\tpal_MemoryAllocate() for pTempInviteReq->ppPPreferredIdentity, error=%u",
		__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrStruct), (void **)&pSipMessageStruct->ppPPreferredIdentity[0]);
	if (pSipMessageStruct->ppPPreferredIdentity[0] == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\tpal_MemoryAllocate() for pTempInviteReq->ppPPreferredIdentity, error=%u",
		__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	error = EcrioSigMgrGetDefaultPUID(pSigMgr, EcrioSigMgrURISchemeTEL, &pNameAddrParams);
	if (error == ECRIO_SIGMGR_NO_ERROR && pNameAddrParams != NULL)
	{
		/* Copy Name-Addr */
		pSipMessageStruct->ppPPreferredIdentity[0]->pDisplayName = pNameAddrParams->nameAddr.pDisplayName;
		pNameAddrParams->nameAddr.pDisplayName = NULL;
		pSipMessageStruct->ppPPreferredIdentity[0]->addrSpec.uriScheme = pNameAddrParams->nameAddr.addrSpec.uriScheme;
		pSipMessageStruct->ppPPreferredIdentity[0]->addrSpec.u.pSipUri = pNameAddrParams->nameAddr.addrSpec.u.pSipUri;
		pNameAddrParams->nameAddr.addrSpec.u.pSipUri = NULL;

		EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_NameAddrWithParams, \
		(void **)&pNameAddrParams, Enum_TRUE);
	}
	else
	{
		pSipMessageStruct->ppPPreferredIdentity[0]->addrSpec.uriScheme = EcrioSigMgrURISchemeSIP;
		pal_MemoryAllocate(sizeof(EcrioSigMgrSipURIStruct), (void **)&(pSipMessageStruct->ppPPreferredIdentity[0]->addrSpec.u.pSipUri));
		if (pSipMessageStruct->ppPPreferredIdentity[0]->addrSpec.u.pSipUri == NULL)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() for pSipUri, error=%u",
				__FUNCTION__, __LINE__, error);
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}

		pal_MemorySet(pSipMessageStruct->ppPPreferredIdentity[0]->addrSpec.u.pSipUri, 0, sizeof(EcrioSigMgrSipURIStruct));

		error = _EcrioSigMgrStringCreate(pSigMgr,
			pSigMgr->pSignalingInfo->pHomeDomain,
			&pSipMessageStruct->ppPPreferredIdentity[0]->addrSpec.u.pSipUri->pDomain);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrStringCreate() for pDomain, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		error = _EcrioSigMgrStringCreate(pSigMgr,
			pSigMgr->pSignalingInfo->pUserId,
			&pSipMessageStruct->ppPPreferredIdentity[0]->addrSpec.u.pSipUri->pUserId);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrStringCreate() for pUserId, error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	if (pSipMessageStruct->bPrivacy == Enum_TRUE)
	{
		if (pSipMessageStruct->ppPPreferredIdentity[0]->pDisplayName != NULL)
		{
			pal_MemoryFree((void**)&pSipMessageStruct->ppPPreferredIdentity[0]->pDisplayName);
		}
	}
	else
	{
		if ((pSipMessageStruct->ppPPreferredIdentity[0]->pDisplayName == NULL) && (pSigMgr->pSignalingInfo->pDisplayName != NULL))
		{
			pSipMessageStruct->ppPPreferredIdentity[0]->pDisplayName = pal_StringCreate(pSigMgr->pSignalingInfo->pDisplayName,
				pal_StringLength(pSigMgr->pSignalingInfo->pDisplayName));
		}
	}

Error_Level_01:
	if ((error != ECRIO_SIGMGR_NO_ERROR) && (pSipMessageStruct->ppPPreferredIdentity != NULL))
	{
		EcrioSigMgrStructRelease((SIGMGRHANDLE)pSigMgr, EcrioSigMgrStructType_NameAddr,
			(void **)&pSipMessageStruct->ppPPreferredIdentity, Enum_TRUE);
	}

	return error;
}

//Adding Conversation ID and Contribution ID to Optional Header.
u_int32 _EcrioSigMgrAddConversationsIDHeader
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessageStruct
)
{

	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	if (pSipMessageStruct->pConvId->pConversationId)
	{
		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pSipMessageStruct->pOptionalHeaderList, EcrioSipHeaderTypeConversationID, 1,
			&pSipMessageStruct->pConvId->pConversationId, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	if (pSipMessageStruct->pConvId->pContributionId)
	{
		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pSipMessageStruct->pOptionalHeaderList, EcrioSipHeaderTypeContributionID, 1,
			&pSipMessageStruct->pConvId->pContributionId, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	if (pSipMessageStruct->pConvId->pInReplyToContId)
	{
		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pSipMessageStruct->pOptionalHeaderList, EcrioSipHeaderTypeInReplyToContributionID, 1,
			&pSipMessageStruct->pConvId->pInReplyToContId, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
					__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}


	Error_Level_01:
	return error;
}

u_int32 EcrioSigMgrSetAppData
(
	SIGMGRHANDLE sigMgrHandle,
	u_char* pCallId,
	void* pAppData
)
{
	u_int32	uError = ECRIO_SIGMGR_NO_ERROR;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	EcrioSigMgrStruct *pSigMgr = NULL;

	if (sigMgrHandle == NULL)
	{
		uError = ECRIO_SIGMGR_BAD_HANDLE;
		return uError;
	}
	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	if ((pCallId == NULL) || (pAppData == NULL))
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	ec_MapGetKeyData(pSigMgr->hHashMap, pCallId, (void **)&pDialogNode);
	if (pDialogNode)
	{
		pDialogNode->pAppData = pAppData;
	}
	else
	{
		uError = ECRIO_SIGMGR_ID_NOT_FOUND;
	}

	return uError;
}


u_int32 EcrioSigMgrCheckUpdateSessionRefresh
(
	SIGMGRHANDLE sigMgrHandle,
	u_char* pCallId,
	BoolEnum* pUpdateAllow
)
{
	u_int32	uError = ECRIO_SIGMGR_NO_ERROR;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	EcrioSigMgrStruct *pSigMgr = NULL;
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;

	if (sigMgrHandle == NULL)
	{
		return ECRIO_SIGMGR_BAD_HANDLE;
	}	
	if ((pCallId == NULL) || (pUpdateAllow == NULL))
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}
	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);
	ec_MapGetKeyData(pSigMgr->hHashMap, pCallId, (void **)&pDialogNode);
	if (pDialogNode !=NULL )
	{
		pInviteUsage = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
		if (pInviteUsage !=NULL)
		{
			*pUpdateAllow = pInviteUsage->isUpdateAllow;
		}
	}
	else
	{
		return ECRIO_SIGMGR_ID_NOT_FOUND;
	}
	return ECRIO_SIGMGR_NO_ERROR;	
}

u_int32 EcrioSigMgrGetAppData
(
	SIGMGRHANDLE sigMgrHandle,
	u_char* pCallId,
	void** ppAppData
)
{
	u_int32	uError = ECRIO_SIGMGR_NO_ERROR;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	EcrioSigMgrStruct *pSigMgr = NULL;

	if (sigMgrHandle == NULL)
	{
		uError = ECRIO_SIGMGR_BAD_HANDLE;
		return uError;
	}
	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	if ((pCallId == NULL) || (ppAppData == NULL))
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	ec_MapGetKeyData(pSigMgr->hHashMap, pCallId, (void **)&pDialogNode);
	if (pDialogNode)
	{
		*ppAppData = pDialogNode->pAppData;
	}
	else
	{
		uError = ECRIO_SIGMGR_ID_NOT_FOUND;
	}

	return uError;
}

u_int32 EcrioSigMgrGetNextAppData
(
	SIGMGRHANDLE sigMgrHandle,
	void** ppAppData
)
{
	u_int32	uError = ECRIO_SIGMGR_NO_ERROR;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	EcrioSigMgrStruct *pSigMgr = NULL;

	if (sigMgrHandle == NULL)
	{
		uError = ECRIO_SIGMGR_BAD_HANDLE;
		return uError;
	}
	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	if (ppAppData == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	ec_MapGetNextData(pSigMgr->hHashMap, (void**)&pDialogNode);
	if (pDialogNode)
	{
		*ppAppData = pDialogNode->pAppData;
		ec_MapDeleteKeyData(pSigMgr->hHashMap, pDialogNode->pCallId);
	}
	else
	{
		uError = ECRIO_SIGMGR_ID_NOT_FOUND;
		*ppAppData = NULL;
	}

	return uError;
}

u_int32 ec_MapInit(MAPHANDLE* ppHandle)
{
	khash_t(uKHName) *h = kh_init(uKHName);

	*ppHandle = (MAPHANDLE)h;

	return 0;
}

u_int32 ec_MapInsertData(MAPHANDLE pHandle, u_char* key, void* pData)
{
	s_int32 error = 0;
	khiter_t iterator;

	khash_t(uKHName) *h = (khash_t(uKHName) *)pHandle;

	// Add a value Key to the hashtable

	//TODO: Handle error value

	iterator = kh_put(uKHName, h, (kh_cstr_t)key, &error);

	kh_value(h, iterator) = (void*)pData; // set the value of the key

	return error;
}

u_int32 ec_MapGetKeyData(MAPHANDLE pHandle, u_char* key, void** ppData)
{
	u_int32 error = 0;
	khiter_t iterator;

	khash_t(uKHName) *h = (khash_t(uKHName) *)pHandle;

	// Retrieve the value for key "apple"
	iterator = kh_get(uKHName, h, (kh_cstr_t)key);  // first have to get iterator

	if (iterator != kh_end(h)) 
	{
		// iterator will be equal to kh_end if key not present

		*ppData = (void*)kh_val(h, iterator); // next have to fetch  the actual value

		error = 0;
	}
	else
	{
		error = 1;
	}
	return error;
}

u_int32 ec_MapGetNextData(MAPHANDLE pHandle, void** ppData)
{
	khiter_t iterator;

	khash_t(uKHName) *h = (khash_t(uKHName) *)pHandle;

	for (iterator = kh_begin(h); iterator != kh_end(h); ++iterator) {
		if (kh_exist(h,iterator)) {
			*ppData = kh_value(h, iterator);
			break;
		}
	}

	return 0;
}

/* Note - ppData need to be deleted separately by caller */
u_int32 ec_MapDeleteKeyData(MAPHANDLE pHandle, u_char* key)
{
	u_int32 error = 0;
	khiter_t iterator;

	khash_t(uKHName) *h = (khash_t(uKHName) *)pHandle;

	// Retrieve the value for key "apple"

	iterator = kh_get(uKHName, h, (kh_cstr_t)key);  // first have to get iterator

	if (iterator != kh_end(h)) 
	{  
		kh_del(uKHName, h, iterator); // Delete the key and data
		error = 0;
	}
	else
	{
		error = 1;
	}

	return error;
}

u_int32 ec_MapDeInit(MAPHANDLE pHandle)
{
	khash_t(uKHName) *h = (khash_t(uKHName) *)pHandle;

	kh_destroy(uKHName, h);

	return 0;
}

EcrioSigMgrURISchemesEnum EcrioSigMgrGetUriType
(
	SIGMGRHANDLE sigMgrHandle,
	u_char *pUri
)
{
	EcrioSigMgrURISchemesEnum uriType = EcrioSigMgrURISchemeNone;
	EcrioSigMgrStruct *pSigMgr = NULL;

	if (pUri == NULL)
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;

	pSigMgr = (EcrioSigMgrStruct *)sigMgrHandle;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pal_StringNCompare(pUri, (u_char*)"sip:", 4) == 0)
	{
		uriType = EcrioSigMgrURISchemeSIP;
	}
	else if (pal_StringNCompare(pUri, (u_char*)"tel:", 4) == 0)
	{
		uriType = EcrioSigMgrURISchemeTEL;
	}
	else
	{
		uriType = EcrioSigMgrURISchemeCustom;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",__FUNCTION__, __LINE__);

	return uriType;
}

u_int32 EcrioSigMgrPopulateToHeader
(
	SIGMGRHANDLE sigMgrHandle,
	u_char *pUri,
	EcrioSigMgrNameAddrWithParamsStruct *pTo
)
{
	u_int32	uError = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrURISchemesEnum uriType = EcrioSigMgrURISchemeNone;
	EcrioSigMgrStruct *pSigMgr = NULL;

	if (pUri == NULL || pTo == NULL)
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;

	pSigMgr = (EcrioSigMgrStruct *)sigMgrHandle;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	uriType = EcrioSigMgrGetUriType(pSigMgr, pUri);

	if (uriType == EcrioSigMgrURISchemeSIP ||
		uriType == EcrioSigMgrURISchemeTEL)
	{
		uError = _EcrioSigMgrParseUri(pSigMgr, pUri, &pTo->nameAddr.addrSpec);
	}
	else
	{
		/*It is a MDN. Make it as a tel URI*/
		uError = EcrioSigMgrFillUriStruct(pSigMgr, &pTo->nameAddr.addrSpec, pUri, EcrioSigMgrURISchemeTEL);
	}
	
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, uError);

	return uError;
}

u_char* _EcrioSigMgrCreateUdpKeepAlivePacket
(
	LOGHANDLE pLogHandle,
	u_char* pPublicIdentity
)
{
	u_int32 uPktLength = 0;
	u_char* pUdpKeepAlivePkt = NULL;
	(void)pPublicIdentity;

	SIGMGRLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	uPktLength = pal_StringLength((const u_char*)"\r\n\r\n") + 1;;
		//pal_StringLength("UDP-PING ") + 
		//pal_StringLength(pPublicIdentity) + 
		//pal_StringLength((const u_char*)"\r\nl:0\r\n\r\n") + 1;

	pal_MemoryAllocate(sizeof(u_char)*uPktLength, (void**)&pUdpKeepAlivePkt);
	if (pUdpKeepAlivePkt != NULL)
	{
		if (NULL == pal_StringNCopy(pUdpKeepAlivePkt, uPktLength, (const u_char*)"\r\n\r\n", uPktLength-1))
		{
			pal_MemoryFree((void **)&pUdpKeepAlivePkt);
			return NULL;
		}
		//pal_StringCopy(pUdpKeepAlivePkt, "UDP-PING ");
		//pal_StringConcatenate(pUdpKeepAlivePkt, pPublicIdentity);
		//pal_StringConcatenate(pUdpKeepAlivePkt, (const u_char*)"\r\nl:0\r\n\r\n");
	}

	SIGMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u",__FUNCTION__, __LINE__);
	return pUdpKeepAlivePkt;
}

u_int32 _EcrioSigMgrStartUdpKeepAlive
(
	EcrioSigMgrStruct *pSigMgr
)
{
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;
	u_int32 uPFDerror = KPALErrorNone;
	TimerStartConfigStruct timerConfig = { 0 };

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pSigMgr->pSignalingInfo->bEnableNoActivityTimer == Enum_TRUE && pSigMgr->bUdpKeepAliveTimerStarted == Enum_FALSE)
	{
		// check if the timer is already running
		pSigMgr->pUdpKeepAlivePkt = _EcrioSigMgrCreateUdpKeepAlivePacket(pSigMgr->pLogHandle, NULL);

		// send packet to network

		//pal_SocketSendData(pSigMgr->pSigMgrTransportStruct->sendingChannelIndex, pSigMgr->pUdpKeepAlivePkt, pal_StringLength(pSigMgr->pUdpKeepAlivePkt));

		// start the timer

		timerConfig.uPeriodicInterval = 0;
		timerConfig.pCallbackData = (void *)pSigMgr;
		timerConfig.bEnableGlobalMutex = Enum_TRUE;
		timerConfig.uInitialInterval = pSigMgr->pSignalingInfo->uNoActivityTimerInterval;
		timerConfig.timerCallback = _EcrioSigMgrUdpKeepAliveCallback;
		uPFDerror = pal_TimerStart(pSigMgr->pal, &timerConfig, &pSigMgr->udpKeepAliveTimerId);
		if (uPFDerror == KPALErrorNone)
		{
			pSigMgr->bUdpKeepAliveTimerStarted = Enum_TRUE;
		}
		else
		{
			pSigMgr->bUdpKeepAliveTimerStarted = Enum_FALSE;
			uError = ECRIO_SIGMGR_TIMER_ERROR;
		}
	}
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",__FUNCTION__, __LINE__, uError);
	return uError;
}

u_int32 _EcrioSigMgrStopUdpKeepAlive
(
	EcrioSigMgrStruct *pSigMgr
)
{
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;
	u_int32 uPFDerror = KPALErrorNone;
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);
	if (pSigMgr->bUdpKeepAliveTimerStarted == Enum_TRUE)
	{
		uPFDerror = pal_TimerStop(pSigMgr->udpKeepAliveTimerId);
		if (uPFDerror == KPALErrorNone)
		{
			pSigMgr->bUdpKeepAliveTimerStarted = Enum_FALSE;
		}
		else
		{
			uError = ECRIO_SIGMGR_TIMER_ERROR;
		}
	}
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u", __FUNCTION__, __LINE__, uError);
	return uError;
}

void _EcrioSigMgrUdpKeepAliveCallback
(
	void *pData,
	TIMERHANDLE timerID
)
{
	EcrioSigMgrStruct *pSigMgr = NULL;
	u_int32 uPFDerror = KPALErrorNone;
	if (pData != NULL)
	{
		TimerStartConfigStruct timerConfig = { 0 };
		pSigMgr = (EcrioSigMgrStruct*)pData;
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
			__FUNCTION__, __LINE__);

		// check the start timer flag
		if (pSigMgr->bUdpKeepAliveTimerStarted == Enum_TRUE)
		{
			// stop the timer.
			uPFDerror = pal_TimerStop(timerID);

			// send the packet.
			pal_SocketSendData(pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[1].hChannelHandle, pSigMgr->pUdpKeepAlivePkt, pal_StringLength(pSigMgr->pUdpKeepAlivePkt));

			timerConfig.uPeriodicInterval = 0;
			timerConfig.pCallbackData = (void *)pSigMgr;
			timerConfig.bEnableGlobalMutex = Enum_TRUE;
			timerConfig.uInitialInterval = pSigMgr->pSignalingInfo->uNoActivityTimerInterval;
			timerConfig.timerCallback = _EcrioSigMgrUdpKeepAliveCallback;
			uPFDerror = pal_TimerStart(pSigMgr->pal, &timerConfig, &pSigMgr->udpKeepAliveTimerId);
			if (uPFDerror == KPALErrorNone)
			{
				pSigMgr->bUdpKeepAliveTimerStarted = Enum_TRUE;
			}
		}
		else
		{
			// ignore
			return;
		}
	}
	else
	{
		return;
	}
}


u_int32 _EcrioSigMgrCreateSecurityClientHdr
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList
)
{
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;
	u_int16 index = 0;
	u_int16 uEalgoCount = 0;
	u_int16 uAlgoCount = 0;

	EcrioSigMgrIPSecStruct *pIPsecStruct = NULL;
	EcrioSipIPsecParamStruct *pIPsecParams = NULL;

	if ((pSigMgr == NULL) || (pOptionalHeaderList == NULL))
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if ((pSigMgr->pIPSecDetails != NULL) && (pSigMgr->pIPSecDetails->pIPsecParams != NULL))
	{
		pIPsecStruct = pSigMgr->pIPSecDetails;
		pIPsecParams = pIPsecStruct->pIPsecParams;

		/* Purely internal API - argument validation is not done here */
		if ((!pIPsecParams->uPort_uc) || (!pIPsecParams->uPort_us) ||
			(!pIPsecParams->uSpi_uc) || (!pIPsecParams->uSpi_us) ||
			(!pIPsecParams->uAlgoCount) || (!pIPsecParams->ppAlgo))
		{
			uError = ECRIO_SIGMGR_INSUFFICIENT_DATA;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tInsufficient data for IPSecParams error=%u",
				__FUNCTION__, __LINE__, uError);
			goto EndTag;
		}
		else
		{
			u_char *ppSupported[] = { (u_char *)ECRIO_SIG_MGR_HEADER_NAME_SECURITY_CLIENT_VALUE };
			u_int16	numSupported = 0, count = 0, numParams = 0;
			u_char *pParamN[6] = { 0 };
			u_char *pParamV[6] = { 0 };
			u_char value1[15] = { 0 }, value2[15] = { 0 }, value3[15] = { 0 }, value4[15] = { 0 };
			
			numSupported = (u_int16)(sizeof(ppSupported) / sizeof(u_char *));

			if (pIPsecParams->uEncryptCount > 0)
			{
				count = pIPsecParams->uAlgoCount * pIPsecParams->uEncryptCount;
			}
			else
			{
				count = pIPsecParams->uAlgoCount;
			}
			for (index = 0; index < count; index++)
			{
				if (pIPsecParams->uEncryptCount > 0)
					numParams = 6; /* alg, ealg, port-c, port-s, spi-c, spi-s */
				else
					numParams = 5; /* alg, port-c, port-s, spi-c, spi-s */

				pParamN[0] = (u_char*)ECRIO_SIG_MGR_PARAM_NAME_PORTC;
				if (0 >= pal_NumToString(pSigMgr->pIPSecDetails->pIPsecParams->uPort_uc, value1, 15))
				{
					uError = ECRIO_SIGMGR_STRING_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tString Copy error",
						__FUNCTION__, __LINE__);

					goto EndTag;
				}
				pParamV[0] = value1;

				pParamN[1] = (u_char*)ECRIO_SIG_MGR_PARAM_NAME_PORTS;

				if (0 >= pal_NumToString(pSigMgr->pIPSecDetails->pIPsecParams->uPort_us, value2, 15))
				{
					uError = ECRIO_SIGMGR_STRING_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tString Copy error",
						__FUNCTION__, __LINE__);

					goto EndTag;
				}
				pParamV[1] = value2;

				pParamN[2] = (u_char*)ECRIO_SIG_MGR_PARAM_NAME_SPIC;
				if (0 >= pal_NumToString(pSigMgr->pIPSecDetails->pIPsecParams->uSpi_uc, value3, 15))
				{
					uError = ECRIO_SIGMGR_STRING_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tString Copy error",
						__FUNCTION__, __LINE__);

					goto EndTag;
				}
				pParamV[2] = value3;

				pParamN[3] = (u_char*)ECRIO_SIG_MGR_PARAM_NAME_SPIS;
				if (0 >= pal_NumToString(pSigMgr->pIPSecDetails->pIPsecParams->uSpi_us, value4, 15))
				{
					uError = ECRIO_SIGMGR_STRING_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tString Copy error",
						__FUNCTION__, __LINE__);

					goto EndTag;
				}
				pParamV[3] = value4;

				if (uAlgoCount < pIPsecParams->uAlgoCount)
				{
					if ((pIPsecParams->ppAlgo[uAlgoCount]) &&
						('\0' != pIPsecParams->ppAlgo[uAlgoCount]->pValue[0]))
					{
						pParamN[4] = (u_char*)ECRIO_SIG_MGR_PARAM_NAME_ALG;
						pParamV[4] = (u_char*)pIPsecStruct->pIPsecParams->ppAlgo[index]->pValue;
					}
					++uAlgoCount;
				}
				
				if (uEalgoCount < pIPsecParams->uEncryptCount &&
					pIPsecParams->uEncryptCount != 0)
				{
					if ((pIPsecParams->ppEncrypt[uEalgoCount]) &&
						('\0' != pIPsecParams->ppEncrypt[uEalgoCount]->pValue[0]))
					{
						pParamN[5] = (u_char*)ECRIO_SIG_MGR_PARAM_NAME_ENCRYPT_ALG;
						pParamV[5] = (u_char*)pIPsecStruct->pIPsecParams->ppEncrypt[index]->pValue;
					}
				}

				if (uAlgoCount >= pIPsecParams->uAlgoCount)
				{
					uAlgoCount = 0;
					if (pIPsecParams->uEncryptCount > 1)
						uEalgoCount++;
					else
						uEalgoCount = 0;
				}

				uError = _EcrioSigMgrAddOptionalHeader(pSigMgr, pOptionalHeaderList, EcrioSipHeaderTypeSecurityClient, numSupported,
					ppSupported, numParams, pParamN, pParamV);
				if (uError != ECRIO_SIGMGR_NO_ERROR)
				{
					goto EndTag;
				}

			}
		}
	}
	else
	{
		uError = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tInsufficient data for IPSec error=%u",
			__FUNCTION__, __LINE__, uError);

		goto EndTag;
	}

EndTag:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return uError;
}

u_int32 _EcrioSigMgrCreateSecurityVerifyHdr
(
	EcrioSigMgrStruct *pSigMgr,
	void **ppOptionalHeaderList
)
{
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrIPSecStruct *pIPsecStruct = NULL;
	EcrioSipIPsecNegotiatedParamStruct *pIPsecNegParams = NULL;

	if (pSigMgr == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (*ppOptionalHeaderList == NULL)
	{
		/* Create optional header object */
		uError = EcrioSigMgrCreateOptionalHeaderList(pSigMgr, ppOptionalHeaderList);
		if (uError != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrCreateOptionalHeaderList() error=%u",
				__FUNCTION__, __LINE__, uError);

			uError = ECRIO_SIGMGR_NO_MEMORY;
			goto EndTag;
		}
	}


	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pSigMgr->pSignalingInfo->pSecurityVerify != NULL)
	{
		u_int16 numHeaderValues = 0;
		u_char **ppVal = NULL;

		numHeaderValues = 1;

		pal_MemoryAllocate(numHeaderValues * sizeof(u_char *), (void **)&ppVal);
		if (ppVal == NULL)
		{
			uError = ECRIO_SIGMGR_NO_MEMORY;
			goto EndTag;
		}

		ppVal[0] = pal_StringCreate(pSigMgr->pSignalingInfo->pSecurityVerify, pal_StringLength(pSigMgr->pSignalingInfo->pSecurityVerify));
		if ((pal_StringLength(pSigMgr->pSignalingInfo->pSecurityVerify) > 0) && (ppVal[0] == NULL))
		{
			uError = ECRIO_SIGMGR_NO_MEMORY;
			pal_MemoryFree((void **)&ppVal);
			goto EndTag;
		}

		uError = _EcrioSigMgrAddOptionalHeader(pSigMgr, *ppOptionalHeaderList, EcrioSipHeaderTypeSecurityVerify, numHeaderValues,
			ppVal, 0, NULL, NULL);
		if (uError != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
				__FUNCTION__, __LINE__, uError);
		}

		if (ppVal)
		{
			if (ppVal[0])
			{
				pal_MemoryFree((void **)&ppVal[0]);
			}

			pal_MemoryFree((void **)&ppVal);
		}
	}
	else
	{
	if ((pSigMgr->pIPSecDetails != NULL) && (pSigMgr->pIPSecDetails->pIPsecNegParams != NULL) &&
		(pSigMgr->pIPSecDetails->eIPSecState != ECRIO_SIP_IPSEC_STATE_ENUM_None))
	{
		pIPsecStruct = pSigMgr->pIPSecDetails;
		pIPsecNegParams = pIPsecStruct->pIPsecNegParams;

		/* Purely internal API - argument validation is not done here */
		if ((!pIPsecNegParams->uPort_uc) || (!pIPsecNegParams->uPort_us) ||
			(!pIPsecNegParams->uSpi_uc) || (!pIPsecNegParams->uSpi_us) ||
			(!pIPsecNegParams->uPort_pc) || (!pIPsecNegParams->uPort_ps) ||
			(!pIPsecNegParams->uSpi_pc) || (!pIPsecNegParams->uSpi_ps))
		{
			uError = ECRIO_SIGMGR_INSUFFICIENT_DATA;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tInsufficient data for IPSecParams error=%u",
				__FUNCTION__, __LINE__, uError);
			goto EndTag;
		}
		else
		{
			u_char *ppSupported[] = { (u_char *)ECRIO_SIG_MGR_HEADER_NAME_SECURITY_CLIENT_VALUE };
			u_int16	numSupported = 0, numParams = 0, i = 0;
			u_char *pParamN[6] = { 0 };
			u_char *pParamV[6] = { 0 };
			u_char value1[15] = { 0 }, value2[15] = { 0 }, value3[15] = { 0 }, value4[15] = { 0 };

			numSupported = (u_int16)(sizeof(ppSupported) / sizeof(u_char *));

			pParamN[i] = (u_char*)ECRIO_SIG_MGR_PARAM_NAME_PORTC;
			if (0 >= pal_NumToString(pIPsecNegParams->uPort_pc, value1, 15))
			{
				uError = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tString Copy error",
					__FUNCTION__, __LINE__);

				goto EndTag;
			}
			pParamV[i] = value1;
			i++;

			pParamN[i] = (u_char*)ECRIO_SIG_MGR_PARAM_NAME_PORTS;
			if (0 >= pal_NumToString(pIPsecNegParams->uPort_ps, value2, 15))
			{
				uError = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tString Copy error",
					__FUNCTION__, __LINE__);

				goto EndTag;
			}
			pParamV[i] = value2;
			i++;

			pParamN[i] = (u_char*)ECRIO_SIG_MGR_PARAM_NAME_SPIC;
			if (0 >= pal_NumToString(pIPsecNegParams->uSpi_pc, value3, 15))
			{
				uError = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tString Copy error",
					__FUNCTION__, __LINE__);

				goto EndTag;
			}
			pParamV[i] = value3;
			i++;

			pParamN[i] = (u_char*)ECRIO_SIG_MGR_PARAM_NAME_SPIS;
			if (0 >= pal_NumToString(pIPsecNegParams->uSpi_ps, value4, 15))
			{
				uError = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tString Copy error",
					__FUNCTION__, __LINE__);

				goto EndTag;
			}
			pParamV[i] = value4;
			i++;

			if (pIPsecNegParams->pAlgo != NULL)
			{
				pParamN[i] = (u_char*)ECRIO_SIG_MGR_PARAM_NAME_ALG;
				pParamV[i] = pIPsecNegParams->pAlgo;
				i++;
			}
			if (pIPsecNegParams->pEncrypt != NULL)
			{
				pParamN[i] = (u_char*)ECRIO_SIG_MGR_PARAM_NAME_ENCRYPT_ALG;
				pParamV[i] = pIPsecNegParams->pEncrypt;
				i++;
			}

			numParams = i;

			uError = _EcrioSigMgrAddOptionalHeader(pSigMgr, *ppOptionalHeaderList, EcrioSipHeaderTypeSecurityVerify, numSupported,
				ppSupported, numParams, pParamN, pParamV);
			if (uError != ECRIO_SIGMGR_NO_ERROR)
			{
				goto EndTag;
			}
		}
	}
	else
	{
		uError = ECRIO_SIGMGR_NO_ERROR;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tSecurity-Server is not received, hence not adding this header", __FUNCTION__, __LINE__);

		goto EndTag;
		}
	}

EndTag:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return uError;
}

u_int32 _EcrioSigMgrCreateTempIPSecCommunicationChannel
(
	EcrioSigMgrStruct *pSigMgr, 
	EcrioTxnMgrSignalingCommunicationChannelInfoStruct	*pChannelInfo
)
{
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;

	SOCKETHANDLE pSock = NULL;
	SocketCreateConfigStruct socketCreateConfig = { .protocolVersion = ProtocolVersion_IPv4, .protocolType = ProtocolType_UDP, .socketType = SocketType_Default, .socketMediaType = SocketMediaType_Default };
	SocketCallbackFnStruct fnStruct = { 0 };
	BoolEnum bCallGetLocalHost = Enum_FALSE;

	if (pSigMgr == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	if (pChannelInfo == NULL || pChannelInfo->pLocalIp == NULL || pChannelInfo->pRemoteIp == NULL)
	{
		uError = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto EndTag;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\t- local-ip = %s local-port=%u remote-ip=%s remote-port=%u socket-type=%u", __FUNCTION__, __LINE__,
		pChannelInfo->pLocalIp ? pChannelInfo->pLocalIp : (u_char *)"null",
		pChannelInfo->localPort,
		pChannelInfo->pRemoteIp ? pChannelInfo->pRemoteIp : (u_char *)"null",
		pChannelInfo->remotePort,
		pChannelInfo->eSocketType);

	socketCreateConfig.protocolVersion = (pSigMgr->pNetworkInfo->bIsIPv6) ? ProtocolVersion_IPv6 : ProtocolVersion_IPv4;
	socketCreateConfig.protocolType = pChannelInfo->eSocketType;
	socketCreateConfig.socketType = SocketType_Default;
	socketCreateConfig.socketMediaType = SocketMediaType_Default;
	socketCreateConfig.bufferSize = 4096;
	socketCreateConfig.bEnableGlobalMutex = Enum_TRUE;

	if (pChannelInfo->eSocketType != ProtocolType_TCP_Server)
	{
		fnStruct.pAcceptCallbackFn = NULL;
	}
	else
	{
		fnStruct.pAcceptCallbackFn = _EcrioSigMgrSocketAcceptCB;
	}
	fnStruct.pErrorCallbackFn = _EcrioSigMgrSocketErrorCB;
	fnStruct.pReceiveCallbackFn = _EcrioSigMgrSocketReceieveCB;
	fnStruct.pCallbackFnData = pSigMgr;

	uError = pal_SocketCreate(pSigMgr->pal, &socketCreateConfig, &fnStruct, &pSock);
	if (uError != KPALErrorNone)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t- pal_SocketCreate() failed ... Error = %d", __FUNCTION__, __LINE__, uError);
		uError = ECRIO_SIG_MGR_SIG_SOCKET_ERROR;
		goto EndTag;
	}

	if (pChannelInfo->localPort == 0)
	{
		bCallGetLocalHost = Enum_TRUE;
	}

	uError = pal_SocketSetLocalHost(pSock, pChannelInfo->pLocalIp, (u_int16)pChannelInfo->localPort);
	if (uError != KPALErrorNone)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t- pal_SocketSetLocalHost() failed ... Error = %d", __FUNCTION__, __LINE__, uError);
		uError = ECRIO_SIG_MGR_SIG_SOCKET_ERROR;
		goto EndTag;
	}

	if (pChannelInfo->eSocketType != ProtocolType_TCP_Server)
	{
		uError = pal_SocketSetRemoteHost(pSock, pChannelInfo->pRemoteIp, (u_int16)pChannelInfo->remotePort);
		if (uError != KPALErrorNone)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t- pal_SocketSetRemoteHost() failed ... Error = %d", __FUNCTION__, __LINE__, uError);
			uError = ECRIO_SIG_MGR_SIG_SOCKET_ERROR;
			goto EndTag;
		}
	}

	/* Use socket open extension API only if proxy routing enabled */
	if(pChannelInfo->bIsProxyRouteEnabled == Enum_TRUE)
		uError = pal_SocketOpenEx(pSock);
	else
		uError = pal_SocketOpen(pSock);
	if (uError != KPALErrorNone)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t- pal_SocketOpen() failed ... Error = %d", __FUNCTION__, __LINE__, uError);
		uError = ECRIO_SIG_MGR_SIG_SOCKET_ERROR;
		goto EndTag;
	}

	if (bCallGetLocalHost == Enum_TRUE)
	{
		u_int16 port = 0;
		u_char localIp[52];	// maximum length of IPV6 address plus some extra bytes

		uError = pal_SocketGetLocalHost(pSock, (u_char *)&localIp, &port);
		if (uError != KPALErrorNone)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t- pal_SocketGetLocalHost() failed ... Error = %d", __FUNCTION__, __LINE__, uError);
			uError = ECRIO_SIG_MGR_SIG_SOCKET_ERROR;
			goto EndTag;
		}

		pChannelInfo->localPort = port;
		SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tpChannelInfo-> pRemoteIp:%s remotePort:%d pLocalIp:%s localPort:%d", __FUNCTION__, __LINE__, pChannelInfo->pRemoteIp, pChannelInfo->remotePort, pChannelInfo->pLocalIp, pChannelInfo->localPort);
	}
	_EcrioSigMgrTerminateChannel(pSigMgr, pSock);
	pSock = NULL;


EndTag:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return uError;
}


u_int32 _EcrioSigMgrExtractSecurityServerFromRegisterResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct	*pRegResp,
	BoolEnum *bValidSecurityServer
)
{
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;
	u_int32 i = 0, j = 0;

	EcrioSigMgrHeaderStruct *pHeader = NULL;
	EcrioSipHeaderTypeEnum eHdrType;
	EcrioSipIPsecNegotiatedParamStruct *pIPsecNegParams = NULL;
	BoolEnum bMatchFound = Enum_FALSE;

	if ((pRegResp->eReqRspType != EcrioSigMgrSIPResponse) || (pRegResp->pOptionalHeaderList == NULL))
	{
		uError = ECRIO_SIGMGR_INVALID_OPERATION;
		return uError;
	}

	pIPsecNegParams = pSigMgr->pIPSecDetails->pIPsecNegParams;
	eHdrType = EcrioSipHeaderTypeSecurityServer;
	uError = EcrioDSListFindData(pRegResp->pOptionalHeaderList, (void *)&eHdrType, (void **)&pHeader);
	if (uError != ECRIO_DS_NO_ERROR)
	{
		// @todo Refactor this function! Don't return anything.
		// Caller should just check ppOptHeader for NULL or not.
		// Only caller can make decision if "signaling manager needs to exit"!
		//		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
		//			"%s:%u\tEcrioDSListFindData() for pOptionalHeaderList, error=%u",
		//			__FUNCTION__, __LINE__, error);
		uError = ECRIO_SIGMGR_CRITCAL_ERROR_NEED_TO_EXIT;
	}

	if (pHeader == NULL)
	{
		goto EndTag;
	}

	for (i = 0; i < pHeader->numHeaderValues; i++)
	{
		if (pal_StringCompare((const u_char*)pHeader->pHeaderName, (const u_char*)ECRIO_SIG_MGR_HEADER_NAME_SECURITY_SERVER) == 0)
		{
			for (j = 0; j < pHeader->ppHeaderValues[i]->numParams; j++)
			{
				if (pal_StringICompare(pHeader->ppHeaderValues[i]->ppParams[j]->pParamName, (u_char*)ECRIO_SIG_MGR_PARAM_NAME_PORTC) == 0)
				{
					pIPsecNegParams->uPort_pc = pal_StringConvertToUNum(pHeader->ppHeaderValues[i]->ppParams[j]->pParamValue, NULL, 10);
				}
				else if (pal_StringICompare(pHeader->ppHeaderValues[i]->ppParams[j]->pParamName, (u_char*)ECRIO_SIG_MGR_PARAM_NAME_PORTS) == 0)
				{
					pIPsecNegParams->uPort_ps = pal_StringConvertToUNum(pHeader->ppHeaderValues[i]->ppParams[j]->pParamValue, NULL, 10);
				}
				else if (pal_StringICompare(pHeader->ppHeaderValues[i]->ppParams[j]->pParamName, (u_char*)ECRIO_SIG_MGR_PARAM_NAME_SPIC) == 0)
				{
					pIPsecNegParams->uSpi_pc = pal_StringConvertToUNum(pHeader->ppHeaderValues[i]->ppParams[j]->pParamValue, NULL, 10);
				}
				else if (pal_StringICompare(pHeader->ppHeaderValues[i]->ppParams[j]->pParamName, (u_char*)ECRIO_SIG_MGR_PARAM_NAME_SPIS) == 0)
				{
					pIPsecNegParams->uSpi_ps = pal_StringConvertToUNum(pHeader->ppHeaderValues[i]->ppParams[j]->pParamValue, NULL, 10);
				}
				else if (pal_StringICompare(pHeader->ppHeaderValues[i]->ppParams[j]->pParamName, (u_char*)ECRIO_SIG_MGR_PARAM_NAME_ALG) == 0)
				{
					if (pHeader->ppHeaderValues[i]->ppParams[j]->pParamValue != NULL)
					{
						_EcrioSigMgrStringCreate(pSigMgr, pHeader->ppHeaderValues[i]->ppParams[j]->pParamValue, &pIPsecNegParams->pAlgo);
					}
				}
				else if (pal_StringICompare(pHeader->ppHeaderValues[i]->ppParams[j]->pParamName, (u_char*)ECRIO_SIG_MGR_PARAM_NAME_ENCRYPT_ALG) == 0)
				{
					if (pHeader->ppHeaderValues[i]->ppParams[j]->pParamValue != NULL)
					{
						_EcrioSigMgrStringCreate(pSigMgr, pHeader->ppHeaderValues[i]->ppParams[j]->pParamValue, &pIPsecNegParams->pEncrypt);
					}
				}
			}
		}
		else
		{
			*bValidSecurityServer = Enum_FALSE;
		}

		if ((pIPsecNegParams->uPort_pc == 0) || (pIPsecNegParams->uPort_ps == 0) || (pIPsecNegParams->uSpi_pc == 0) || (pIPsecNegParams->uSpi_ps == 0))
		{
			*bValidSecurityServer = Enum_FALSE;
		}
		if (pIPsecNegParams->pAlgo != NULL)
		{
			i = 0;
			for (i = 0; i < pSigMgr->pIPSecDetails->pIPsecParams->uAlgoCount; i++)
			{
				if (pal_StringICompare((const u_char *)pSigMgr->pIPSecDetails->pIPsecParams->ppAlgo[i]->pValue, pIPsecNegParams->pAlgo))
				{
					bMatchFound = Enum_TRUE;
					break;
				}
			}
		}
		if (bMatchFound == Enum_FALSE)
		{
			*bValidSecurityServer = Enum_FALSE;
		}

		if (pIPsecNegParams->pEncrypt != NULL)
		{
			i = 0;
			for (i = 0; i < pSigMgr->pIPSecDetails->pIPsecParams->uEncryptCount; i++)
			{
				if (pal_StringICompare((const u_char *)pSigMgr->pIPSecDetails->pIPsecParams->ppEncrypt[i]->pValue, pIPsecNegParams->pEncrypt))
				{
					bMatchFound = Enum_TRUE;
					break;
				}
			}
		}
		if (bMatchFound == Enum_FALSE)
		{
			*bValidSecurityServer = Enum_FALSE;
		}

		*bValidSecurityServer = Enum_TRUE;


	}
	pSigMgr->pIPSecDetails->eIPSecState = ECRIO_SIP_IPSEC_STATE_ENUM_Temporary;

EndTag:
	return uError;
}


u_int32 _EcrioSigMgrFreeIPSecNegParams
(
	EcrioSigMgrStruct *pSigMgr
)
{
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;
	EcrioSipIPsecNegotiatedParamStruct *pIPsecNegParams = NULL;

	if ((pSigMgr == NULL) || (pSigMgr->pIPSecDetails == NULL))
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pSigMgr->pIPSecDetails->pIPsecNegParams == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	pIPsecNegParams = pSigMgr->pIPSecDetails->pIPsecNegParams;

	if (pIPsecNegParams->pAlgo)
	{
		pal_MemoryFree((void **)&pIPsecNegParams->pAlgo);
	}
	if (pIPsecNegParams->pCK)
	{
		pal_MemoryFree((void **)&pIPsecNegParams->pCK);
	}
	if (pIPsecNegParams->pEncrypt)
	{
		pal_MemoryFree((void **)&pIPsecNegParams->pEncrypt);
	}
	if (pIPsecNegParams->pIK)
	{
		pal_MemoryFree((void **)&pIPsecNegParams->pIK);
	}
	if (pIPsecNegParams->pLocalIp)
	{
		pal_MemoryFree((void **)&pIPsecNegParams->pLocalIp);
	}
	if (pIPsecNegParams->pRemoteIp)
	{
		pal_MemoryFree((void **)&pIPsecNegParams->pRemoteIp);
	}

	pIPsecNegParams->uCKSize = 0;
	pIPsecNegParams->uIKSize = 0;
	pIPsecNegParams->uPort_pc = 0;
	pIPsecNegParams->uPort_ps = 0;
	pIPsecNegParams->uPort_uc = 0;
	pIPsecNegParams->uPort_us = 0;
	pIPsecNegParams->uSpi_pc = 0;
	pIPsecNegParams->uSpi_ps = 0;
	pIPsecNegParams->uSpi_uc = 0;
	pIPsecNegParams->uSpi_us = 0;

	pSigMgr->pIPSecDetails->pIPsecNegParams = NULL;

	return uError;
}

u_int32 _EcrioSigMgrAddLocalContactUriWithoutFeatureTags
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrContactStruct **ppUri
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrContactStruct *pContactUri = NULL;
	EcrioSigMgrNameAddrWithParamsStruct *pNameAddr = NULL;
	EcrioSigMgrSipURIStruct *pSipUri = NULL;
	EcrioSigMgrParamStruct **ppParams = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pal_MemoryAllocate(sizeof(EcrioSigMgrContactStruct), (void **)&pContactUri);
	if (pContactUri == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	pContactUri->numContactUris = 1;
	pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct *),
		(void **)&pContactUri->ppContactDetails);
	if (pContactUri->ppContactDetails == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct),
		(void **)&pContactUri->ppContactDetails[0]);
	if (pContactUri->ppContactDetails[0] == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	pNameAddr = pContactUri->ppContactDetails[0];

	if (pSigMgr->pSignalingInfo->pDisplayName != NULL)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSigMgr->pSignalingInfo->pDisplayName,
			&pNameAddr->nameAddr.pDisplayName);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}

	pNameAddr->nameAddr.addrSpec.uriScheme = EcrioSigMgrURISchemeSIP;
	pal_MemoryAllocate(sizeof(EcrioSigMgrSipURIStruct),
		(void **)&pNameAddr->nameAddr.addrSpec.u.pSipUri);
	if (pNameAddr->nameAddr.addrSpec.u.pSipUri == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	pSipUri = pNameAddr->nameAddr.addrSpec.u.pSipUri;
#if 0
	if (pSigMgr->pSignalingInfo->uriScheme == EcrioSigMgrURISchemeSIP)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pSigMgr->pSignalingInfo->pUserId,
			&pSipUri->pUserId);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}
	else if (pSigMgr->pSignalingInfo->uriScheme == EcrioSigMgrURISchemeTEL)	/* New_UA */
	{
		u_char *pTelUri = NULL;
		EcrioSigMgrUriStruct uri = { .uriScheme = EcrioSigMgrURISchemeNone };

		uri.uriScheme = EcrioSigMgrURISchemeTEL;
		// uri.u.pTelUri = pSigMgr->pSignalingInfo->p;
		error = _EcrioSigMgrFormUri(pSigMgr, &uri, &pTelUri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}

		error = _EcrioSigMgrStringCreate(pSigMgr, (pTelUri + 4),/* ignore tel: */
			&pSipUri->pUserId);
		pal_MemoryFree((void **)&pTelUri);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}
	else
	{
		error = ECRIO_SIGMGR_INVALID_URI_SCHEME;
		goto Error_Level_01;
	}
#else
	EcrioSigMgrNameAddrWithParamsStruct *pNameAddrParams = NULL;
	EcrioSigMgrGetDefaultPUID(pSigMgr, EcrioSigMgrURISchemeSIP, &pNameAddrParams);
	if (pNameAddrParams != NULL &&
		pNameAddrParams->nameAddr.addrSpec.u.pSipUri->pUserId != NULL)
	{
		error = _EcrioSigMgrStringCreate(pSigMgr, pNameAddrParams->nameAddr.addrSpec.u.pSipUri->pUserId,
			&pSipUri->pUserId);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}

		EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_NameAddrWithParams, \
			(void **)&pNameAddrParams, Enum_TRUE);
	}
	else
	{
		SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tFailed to fetch UserID from PAI. Populate using signaling info.",
			__FUNCTION__, __LINE__);
		error = _EcrioSigMgrStringCreate(pSigMgr, pSigMgr->pSignalingInfo->pUserId,
			&pSipUri->pUserId);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error_Level_01;
		}
	}
#endif
	pal_MemoryAllocate(sizeof(EcrioSigMgrIPAddrStruct), (void **)&pSipUri->pIPAddr);
	if (pSipUri->pIPAddr == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	error = _EcrioSigMgrStringCreate(pSigMgr, pSigMgr->pNetworkInfo->pLocalIp,
		&pSipUri->pIPAddr->pIPAddr);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		goto Error_Level_01;
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
				pSipUri->pIPAddr->port = pChannels->pChannelInfo->localPort;
			}
		}
	}
	else
	{
		pSipUri->pIPAddr->port = (u_int16)pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[0].pChannelInfo->localPort;
	}

#ifndef ENABLE_MTK
	if (pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_FALSE)
	{
		pSipUri->numURIParams = 1;
		pal_MemoryAllocate(pSipUri->numURIParams*sizeof(EcrioSigMgrParamStruct*), (void**)&pSipUri->ppURIParams);
		if (pSipUri->ppURIParams != NULL)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void**)&pSipUri->ppURIParams[0]);
			if (pSipUri->ppURIParams[0] != NULL)
			{
				pSipUri->ppURIParams[0]->pParamName = pal_StringCreate((const u_char*)"transport", pal_StringLength((const u_char*)"transport"));
				pSipUri->ppURIParams[0]->pParamValue = pal_StringCreate((const u_char*)"UDP", pal_StringLength((const u_char*)"UDP"));
			}
			else
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
		}
		else
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}
	}
	else if ((pSigMgr->pSignalingInfo->bEnableUdp == Enum_FALSE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE))
	{
		pSipUri->numURIParams = 1;
		pal_MemoryAllocate(pSipUri->numURIParams*sizeof(EcrioSigMgrParamStruct*), (void**)&pSipUri->ppURIParams);
		if (pSipUri->ppURIParams != NULL)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void**)&pSipUri->ppURIParams[0]);
			if (pSipUri->ppURIParams[0] != NULL)
			{
				pSipUri->ppURIParams[0]->pParamName = pal_StringCreate((const u_char*)"transport", pal_StringLength((const u_char*)"transport"));
				pSipUri->ppURIParams[0]->pParamValue = pal_StringCreate((const u_char*)"TCP", pal_StringLength((const u_char*)"TCP"));
			}
			else
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
		}
		else
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}
	}
#else	/* Temp Fix - Forcefully add transport=tcp. */
	{
		pSipUri->numURIParams = 1;
		pal_MemoryAllocate(pSipUri->numURIParams*sizeof(EcrioSigMgrParamStruct*), (void**)&pSipUri->ppURIParams);
		if (pSipUri->ppURIParams != NULL)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void**)&pSipUri->ppURIParams[0]);
			if (pSipUri->ppURIParams[0] != NULL)
			{
				pSipUri->ppURIParams[0]->pParamName = pal_StringCreate((const u_char*)"transport", pal_StringLength((const u_char*)"transport"));
				pSipUri->ppURIParams[0]->pParamValue = pal_StringCreate((const u_char*)"tcp", pal_StringLength((const u_char*)"tcp"));
			}
			else
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				goto Error_Level_01;
			}
		}
		else
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error_Level_01;
		}
	}
#endif
	
	pNameAddr->ppParams = NULL;
	pNameAddr->numParams = 1;
	
	pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct *)*1, (void**)&pNameAddr->ppParams);
	if (pNameAddr->ppParams == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void**)&pNameAddr->ppParams[0]);
	if (pNameAddr->ppParams[0] == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}
	pNameAddr->ppParams[0]->pParamName = pal_StringCreate((const u_char*)"mobility", pal_StringLength((const u_char*)"mobility"));
	pNameAddr->ppParams[0]->pParamValue = pal_StringCreate((const u_char*)"\"mobile\"", pal_StringLength((const u_char*)"\"mobile\""));

	*ppUri = pContactUri;

Error_Level_01:
	if ((error != ECRIO_SIGMGR_NO_ERROR) && (pContactUri != NULL))
	{
		EcrioSigMgrStructRelease((SIGMGRHANDLE)pSigMgr, EcrioSigMgrStructType_Contact,
			(void **)&pContactUri, Enum_TRUE);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:		EcrioSigMgrSetUserAgent()

Purpose:		Sets the User-Agent header value

Description:	The function should be invoked to overwrite the User-AGent
                header value in Signaling Manager.

Input:			SIGMGRHANDLE pSigMgrHandle - Signaling Manager Instance.
				u_char* pUserAgent*        - User-Agent Header Values.

OutPut:         None.

Returns:		error code.
**************************************************************************/
u_int32 EcrioSigMgrSetUserAgent
(
	SIGMGRHANDLE sigMgrHandle,
	u_char* pUserAgent
)
{
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	_EcrioSigMgrInternalSignalingInfoStruct *pSignalingInfo = NULL;

	if (sigMgrHandle == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	pSigMgr = (EcrioSigMgrStruct*)sigMgrHandle;
	pSignalingInfo = pSigMgr->pSignalingInfo;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pUserAgent == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data passed", __FUNCTION__, __LINE__);
		uError = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	if (pSignalingInfo != NULL)
	{
		if (pSignalingInfo->pUserAgent != NULL)
		{
			pal_MemoryFree((void**)&pSignalingInfo->pUserAgent);
		}
		pSignalingInfo->pUserAgent = pal_StringCreate((u_char *)pUserAgent, pal_StringLength((u_char *)pUserAgent));
	}
	else
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpSignalingInfo is NULL", __FUNCTION__, __LINE__);

		uError = ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}	
Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
	return uError;
}