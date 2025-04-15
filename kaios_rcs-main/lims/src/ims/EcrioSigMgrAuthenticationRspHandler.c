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
#include "EcrioSigMgrIMSLibHandler.h"
#include "EcrioSigMgrMD5.h"
#include "EcrioMD5.h"

#define ECRIO_SIGMGR_MAXIMUM_WWWAUTHENTICATE_NONCECOUNT_LENGTH		8
#define ECRIO_SIGMGR_AUTH_ZERO_STRING								"0"

#define _ECRIO_SIGMGR_SIGNALING_MAX_SINGLE_DIGIT_NUMBER				(9)	/* maximum single digit number */
#define _ECRIO_SIGMGR_SIGNALING_MAX_SINGLE_DIGIT_HEX_NUMBER			(15)/* maximum single digit hexa decimal number */

/**
* Enumeration of constants that is used to specify the type of QOP.
*/
typedef enum
{
	ECRIO_SIGMGR_QOP_NONE = 0,		/**< None. */
	ECRIO_SIGMGR_QOP_AUTH,			/**< For Auth. */
	ECRIO_SIGMGR_QOP_AUTH_INT		/**< For Auth_Int. */
} EcriosigMgrAuthorizationTypeEnum;
/*****************************************************************************
                    API Definition Section - Begin
*****************************************************************************/

u_int32 _EcrioSigMgrGenerateResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrAuthenticationStruct	*pAuthStruct,
	u_char *pMethodName,
	u_char *pDigestUriValue,
	u_char *pEntityBody,
	u_char *pUsername,
	u_char *pCnonceValue,
	u_char *pNcValue,
	u_char *pPassword,
	u_char passwordLength,
	u_char **ppDigestResponseOut,
	EcriosigMgrAuthorizationTypeEnum *pAuthTypeEnum
);

/**************************************************************************

Function:		_EcrioSigMgrUpdateAuthResponse()

Purpose:		To re-calculate the AKA/MD5 Response.

Description:	The Basic idea of this function is to calculate the new
                Response value (as per rfc 2617) based on the provided
                Auth Details. This function can be called when ever user
                wants to calculate the Response value for the Authorization
                header. The	output which is a Response string will be saved
                in the pAuth structure.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                u_char* pMethod - SIP Method name
                u_char* pEntityBody - entity Body for qop='auth-int'.
                Can be NULL

Input/Output:	EcrioSigMgrAuthorizationStruct* pAuth - Authorization
                structure where Response will be output.

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrUpdateAuthResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrAuthorizationStruct *pAuth,
	const u_char *pMethod,
	const u_char *pEntityBody,
	EcrioSipAuthStruct *pAuthResp
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 pwdLength = 0;
	u_char *pPasswd = NULL;

	EcrioSigMgrAuthenticationStruct tempAuth = { 0 };
	EcriosigMgrAuthorizationTypeEnum eAuthVal = ECRIO_SIGMGR_QOP_NONE;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	pAuthResp = pAuthResp;
	if (pSigMgr->pSignalingInfo == NULL)
	{
		error = ECRIO_SIGMGR_NOT_INITIALIZED;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient Data Provided",
			__FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	if ((pAuth == NULL) || (pAuth->pNonce == NULL) ||
		(!pal_StringLength(pAuth->pNonce)) ||
		(pSigMgr->pSignalingInfo->pPrivateId == NULL))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient Data Provided",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	if ((pAuth->pRealm == NULL) ||
		(pAuth->pAuthenticationScheme == NULL))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient Data Provided",
			__FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	if (pAuth->authAlgo == EcrioSipAuthAlgorithmUnknown)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tUnsupported Algorithm", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_AUTH_ERROR;
		goto Error_Level_01;
	}

	if (pAuth->pResponse != NULL)
	{
		pal_MemoryFree((void **)&pAuth->pResponse);
	}

	/*populate the Authentication structure*/
	tempAuth.authAlgo = pAuth->authAlgo;
	tempAuth.pAuthenticationScheme = pAuth->pAuthenticationScheme;
	tempAuth.pRealm = pAuth->pRealm;
	tempAuth.pNonce = pAuth->pNonce;
	tempAuth.pOpaque = pAuth->pOpaque;

	if (pAuth->pQoP != NULL)
	{
		tempAuth.countQoP = 1;
		pal_MemoryAllocate(sizeof(u_char *) * 1, (void **)&tempAuth.ppListOfQoP);
		if (tempAuth.ppListOfQoP == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tpal_MemoryAllocate() failed with error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		tempAuth.ppListOfQoP[0] = pAuth->pQoP;
	}
	else
	{
		tempAuth.countQoP = 0;
		tempAuth.ppListOfQoP = NULL;
	}

	/* Support AKAv1 and AKAv2 using configuration */
	if (pAuth->authAlgo == EcrioSipAuthAlgorithmAKAv1 ||
		pAuth->authAlgo == EcrioSipAuthAlgorithmAKAv2)	/* AKA Algorithm */
	{
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t-AKA-MD5 Algorithm",
			__FUNCTION__, __LINE__);
		if (pAuthResp)
		{
			if (pAuth->pAuts != NULL)
			{
				pal_MemoryFree((void **)&pAuth->pAuts);
			}

			if (!pAuthResp->pRes && !pAuthResp->pAuts)
			{
				/* MAC Failure - response will be empty ("") as per 3GPP 24.229 section 5.1.1.5.3 */
				_EcrioSigMgrStringCreate(pSigMgr, (u_char *)"", &pAuth->pResponse);
				goto Error_Level_01;
			}
			else
			{
				if (pAuthResp->pAuts == NULL)
				{
					SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tAuth RES Length = %d", __FUNCTION__, __LINE__, pAuthResp->uResSize);
					/* Calculate the response parameter by setting the password=AKA RES */
					pal_MemoryAllocate((u_int32)(pAuthResp->uResSize), (void **)&pPasswd);
					if (pPasswd == NULL)
					{
						error = ECRIO_SIGMGR_NO_MEMORY;
						SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeMemory,
							"%s:%u\tECRIO_ALLOCATE_MEM() failed with error=%u",
							__FUNCTION__, __LINE__, error);
						goto Error_Level_01;
					}

					pal_MemoryCopy((void *)pPasswd, (u_int32)pAuthResp->uResSize, (const void *)pAuthResp->pRes, (u_int32)pAuthResp->uResSize);
					pwdLength = pAuthResp->uResSize;

					SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tPassword = %s", __FUNCTION__, __LINE__, pPasswd);
				}
				else
				{
					_EcrioSigMgrStringCreate(pSigMgr, pAuthResp->pAuts, &pAuth->pAuts);
					/*  Note that when the AUTS is present, the included "response"
					parameter is calculated using an empty password (password of ""),
					instead of a RES. - RFC 3310 Section 3.2 */
					_EcrioSigMgrStringCreate(pSigMgr, (u_char *)"", &pPasswd);
					pwdLength = 0;
				}
			}
		}
		else
		{
			_EcrioSigMgrStringCreate(pSigMgr, (u_char *)"", &pPasswd);
			pwdLength = 0;
		}
	}
	else/* MD5 Algorithm */
	/* Support only MD5 using configuration */
	{
		SIGMGRLOGD(pSigMgr->pLogHandle, KLogType_Fine,
			"%s:%u\t-MD5 Algorithm",
			__FUNCTION__, __LINE__);
		if (pSigMgr->pSignalingInfo->pPassword)
		{
			pPasswd =
				_EcrioSigMgrStringUnquote(pSigMgr->pSignalingInfo->pPassword);
			pwdLength = pal_StringLength(pPasswd);
		}
	}

	if (pPasswd == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tError obtaining password error = %u",
			__FUNCTION__, __LINE__, error);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	/* Calculate the digest response */
	error = _EcrioSigMgrGenerateResponse(
		pSigMgr,
		&tempAuth,
		(u_char *)pMethod,
		pAuth->pURI,
		(u_char *)pEntityBody,
		pAuth->pUserName,
		pAuth->pCNonce,
		pAuth->pCountNonce,
		pPasswd,
		(u_char)pwdLength,
		&pAuth->pResponse,
		&eAuthVal);

	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		error = ECRIO_SIGMGR_IMS_LIB_ERROR;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrGenerateResponse error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

Error_Level_01:

	if (pPasswd != NULL)
	{
		pal_MemoryFree((void **)&pPasswd);
		pPasswd = NULL;
	}

	if (tempAuth.ppListOfQoP != NULL)
	{
		pal_MemoryFree((void **)&tempAuth.ppListOfQoP);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************

Function:		_EcrioSigMgrGetAuthCredentialFromIMS ()

Purpose:		to generate the Authorization credentials for 401/407 challenge

Description:	This function is used to obtain the Authorization or
                Proxy-Authorization header field in response to	a 401/407
                challenge.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager instance.
                EcrioSigMgrAuthenticationStruct* pAuthResponse - challenge
                found in 401/407 response.
                u_int32 nonceCount - nonce-count.
                u_char* pRequestURI - URI param in Auth.
                u_char* pEntityBody - entity-body for qop=auth-int
                EcrioSigMgrMethodTypeEnum eMethod - request method type

Output:			EcrioSigMgrAuthorizationStruct** ppAuthRequest - Authorization
                or Proxy-Authorization header field

Returns:		Error Code

*****************************************************************************/
u_int32 _EcrioSigMgrGetAuthCredential
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrAuthenticationStruct *pAuthResponse,
	u_int32 nonceCount,
	const u_char *pRequestURI,
	const u_char *pEntityBody,
	EcrioSigMgrMethodTypeEnum eMethod,
	EcrioSigMgrAuthorizationStruct **ppAuthRequest
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_char ncString[10] = {(u_char)0};
	//u_char *pCNonce = NULL;
	u_char *pMethod = NULL;
	EcrioSigMgrAuthorizationStruct *pAuth = NULL;

	pEntityBody = pEntityBody;

	if (pSigMgr == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pSigMgr->pSignalingInfo == NULL)
	{
		error = ECRIO_SIGMGR_NOT_INITIALIZED;
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u\tInsufficient Data",
			__FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	/* Check for validity of input arguments */
	if ((pAuthResponse == NULL) || (ppAuthRequest == NULL) ||
		(pRequestURI == NULL) || (eMethod == EcrioSigMgrMethodNone) ||
		(pSigMgr->pSignalingInfo->pPrivateId == NULL))
	{
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry,
			"%s:%u\tInsufficient Data", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;

		goto Error_Level_01;
	}

	/* The nonce, realm and authentication scheme MUST NOT be NULL */
	if ((pAuthResponse->pNonce == NULL) || (pAuthResponse->pRealm == NULL) ||
		(pAuthResponse->pAuthenticationScheme == NULL))
	{
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry,
			"%s:%u\tInsufficient Data", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;

		goto Error_Level_01;
	}

	if (pAuthResponse->authAlgo == EcrioSipAuthAlgorithmUnknown)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t-Unsupported Algorithm", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_AUTH_ERROR;

		goto Error_Level_01;
	}

	/* nc value in 8 digit Hex value */
	if (0 >= pal_SNumPrintf((char *)ncString, 10, "%08x", nonceCount))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tString Copy error.", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_STRING_ERROR;

		goto Error_Level_01;
	}

	pAuth = *ppAuthRequest;
	/* algorithm value */
	pAuth->authAlgo = pAuthResponse->authAlgo;

	if (pAuth->pNonce)
	{
		pal_MemoryFree((void **)&pAuth->pNonce);
	}

	/* nonce value */
	pAuth->pNonce = _EcrioSigMgrStringUnquote(pAuthResponse->pNonce);
	if (!pAuth->pNonce)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrStringUnquote returned error= %u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\t-Nonce: %s", __FUNCTION__, __LINE__,
		pAuth->pNonce);

	if (pAuth->pRealm)
	{
		pal_MemoryFree((void **)&pAuth->pRealm);
	}

	/* realm value */
	pAuth->pRealm = _EcrioSigMgrStringUnquote(pAuthResponse->pRealm);
	if (!pAuth->pRealm)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrStringUnquote returned error= %u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\t-Nonce: %s", __FUNCTION__, __LINE__,
		pAuthResponse->pRealm);

	if (pAuth->pURI)
	{
		pal_MemoryFree((void **)&pAuth->pURI);
	}

	/* uri value */
	pAuth->pURI = _EcrioSigMgrStringUnquote((u_char *)pRequestURI);
	if (!pAuth->pURI)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrStringUnquote returned error= %u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\tRequest URI: %s", __FUNCTION__, __LINE__,
		pAuth->pURI);
	/* opaque value */
	if (pAuthResponse->pOpaque != NULL)
	{
		if (pAuth->pOpaque)
		{
			pal_MemoryFree((void **)&pAuth->pOpaque);
		}

		pAuth->pOpaque =
			_EcrioSigMgrStringUnquote(pAuthResponse->pOpaque);
		if (!pAuth->pOpaque)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrStringUnquote returned error= %u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}

		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tOpaque URI: %s", __FUNCTION__, __LINE__,
			pAuth->pOpaque);
	}

	/* method */
	switch (eMethod)
	{
		case EcrioSigMgrMethodRegister:
		{
			pMethod = (u_char *)ECRIO_SIG_MGR_METHOD_REGISTER;
		}
		break;

		case EcrioSigMgrMethodMessage:
		{
			pMethod = (u_char *)ECRIO_SIG_MGR_METHOD_MESSAGE;
		}
		break;

		default:
		{
		}
		break;
	}

	pMethod = pMethod;
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\tmethod: %s", __FUNCTION__, __LINE__,
		pMethod);
	/* cnonce value */
	if (pAuth->pCNonce == NULL)
	{
		pAuth->pCNonce = _EcrioSigMgrStringUnquote(ECRIO_SIG_MGR_AUTHORIZATION_CNONCE_VALUE);
		if (!pAuth->pCNonce)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrStringUnquote returned error= %u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\tCNonce: %s", __FUNCTION__, __LINE__,
		pAuth->pCNonce);

	/* Populate the qop */
	if (pAuthResponse->ppListOfQoP != NULL)
	{
		u_int16 i = 0, index = 0;
		BoolEnum bValidQoP = Enum_FALSE;

		for (i = 0; i < pAuthResponse->countQoP; i++)
		{
			if (!(pal_StringCompare(pAuthResponse->ppListOfQoP[i],
				(u_char *)"auth")))
			{
				bValidQoP = Enum_TRUE;
				index = i;
				break;
			}
			/* Support for "auth-int" */
			else if (!(pal_StringCompare(pAuthResponse->ppListOfQoP[i],
				(u_char *)"auth-int")))
			{
				bValidQoP = Enum_TRUE;
				index = i;
			}
		}

		if (bValidQoP == Enum_TRUE)
		{
			if (pAuth->pQoP)
			{
				pal_MemoryFree((void **)&pAuth->pQoP);
			}

			error = _EcrioSigMgrStringCreate(pSigMgr,
				pAuthResponse->ppListOfQoP[index],
				&pAuth->pQoP);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrStringCreate() error=%u",
					__FUNCTION__, __LINE__, error);
				goto Error_Level_01;
			}

			SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tqop: %s", __FUNCTION__, __LINE__,
				pAuth->pQoP);
		}
		else
		{
			error = ECRIO_SIGMGR_AUTH_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tsignatling Manager Auth Error error= %u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	/* nc value */
	if (pAuth->pCountNonce)
	{
		pal_MemoryFree((void **)&pAuth->pCountNonce);
	}

	error = _EcrioSigMgrStringCreate(pSigMgr, (u_char *)ncString,
		&pAuth->pCountNonce);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrStringCreate() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\tnc: %s", __FUNCTION__, __LINE__,
		pAuth->pCountNonce);

	error = ECRIO_SIGMGR_NO_ERROR;

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************
                    API Definition Section - End
*****************************************************************************/

/**************************************************************************

Function:		_EcrioSigMgr_GetHexAscii ()

Purpose:		To obtain the ascii equivalent of a number

Description:	This function returns the ascii equivalent of a number

Input:		u_char num - the number to be converted

Output:		u_char* ascii - pointer to buffer where the value is stored

Returns:		Error code

**************************************************************************/
u_int32 _EcrioSigMgr_GetHexAscii
(
	u_char num,
	u_char *ascii
)
{
	/* /check for valid input pointer */
	if (ascii == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	/* / checks if the number is more than a single hex digit */
	if (num > (u_char)_ECRIO_SIGMGR_SIGNALING_MAX_SINGLE_DIGIT_HEX_NUMBER)
	{
		/* // returns error code  */
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	/* / checks if the number is bigger than the single digit in decimal number  */
	if (num > (u_char)_ECRIO_SIGMGR_SIGNALING_MAX_SINGLE_DIGIT_NUMBER)
	{
		/* // generate A through F in the hex number system. */
		*ascii = (u_char)((u_char)'a' + num - (u_char)10);
	}
	else
	{
		/* // generates ascii equivalent of decimal numbers */
		*ascii = (num + (u_char)0x30);
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:   _EcrioSIGMGRLibrary_IToHexAscii ()

Purpose:	To convert an integer into a hex ascii string value

Description:
This function converts the input integer into a hex ascii
string value. For example. the value 0x34 will be converted
into the string value "43".

Input:		u_int32 num - the number to be converted
u_int32 maxHexAsciiBufLen - the maximum size of the output buffer

Output:		u_char* hexAscii - pointer to buffer where the string value is
to be stored
u_int32* hexAsciiLen - pointer to variable where the actual length
of the hex ascii string is returned

Returns:	error code

**************************************************************************/
u_int32 _EcrioSigMgr_IToHexAscii
(
	u_int32 num,
	u_char *hexAscii,
	u_int32 maxHexAsciiBufLen,
	u_int32 *hexAsciiLen
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int8 remainder = (u_int8)0;
	u_int32 tempNum = num, quotient = (u_int32)0, offset = (u_int32)0;
	u_int32 i, loopCount = (u_int32)1, asciiIndex = 0;

	if ((hexAscii == NULL) || (hexAsciiLen == NULL))
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	/* /repeat the process untill the number is less then 15( max hex number )
	to calculate the loopCount.*/
	while (tempNum > 15)
	{
		/* // find the quotient when dividing the number by 16( hexa decimal number system ) */
		quotient = tempNum / 16;
		/* // calculate the remainder */
		remainder = (u_int8)(tempNum - (quotient * 16));
		/* // increament the loop counter */
		loopCount++;
		/* // store the quotient to temp to repeat the process. */
		tempNum = quotient;
	}

	/* / get the original number */
	tempNum = num;

	/* / loop through for loopCount times to get the hex numbers */
	for (i = 0; i < loopCount; i++)
	{
		/* // find the quotient when dividing the number by 16( hexa decimal number system ) */
		quotient = tempNum / 16;
		/* // calculate the remainder */
		remainder = (u_int8)(tempNum - (quotient * 16));

		/* // store the quotient to temp to repeat the process. */
		tempNum = quotient;
		/* //check for the availability of the buffer size  */
		if ((asciiIndex + 2) < maxHexAsciiBufLen)
		{
			/* /// find the offset to copy the generated ascii in reverse order */
			offset = asciiIndex + loopCount - 1 - i;
			/* /// given a number below 15, this function generates ascii equivalent. The ascii
			data is stored in the given buffer.*/
			error = _EcrioSigMgr_GetHexAscii(remainder, (hexAscii + offset));
		}
		else	/* //if the buffer size if less */
		{
			/* /// return error code */
			error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		}

		/* // check for error condition */
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			/* /// set the offset of 0 and stop the process.  */
			loopCount = 0;
			break;
		}
	}

	/* / store the  resultant data size */
	asciiIndex = loopCount;
	/* / set the last character to null*/
	hexAscii[asciiIndex] = (u_char)0;
	/* / store the resultant length */
	*hexAsciiLen = asciiIndex;
	return error;
}

/*******************************************************************

Function:		_EcrioSigMgrExtractQop()

Purpose:		To measure the quality of protection(Qop).

Description:	This function takes the input SIP parameter, and counts the number
of Qop separated by "," and their values.

Input:		u_char *pQop

Output:		u_int32 *pNoOfQop
u_char ***pppQopValues

Returns:		Error code
***************************************************************************/
u_int32 _EcrioSigMgrExtractQop
(
	u_char *pQop,
	u_int32 *pNoOfQop,
	u_char ***pppQopValues
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_char temp[256] = { 0 };
	u_int32	len = 0, i = 0, j = 0;

	if (pQop == NULL || pNoOfQop == NULL || pppQopValues == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	len = (u_int32)pal_StringLength(pQop);

	if (len == 0)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	*pNoOfQop = 0;
	*pppQopValues = NULL;

	for (i = 0; i < len; i++)
	{
		if (*(pQop + i) == ',' || i == (len - 1))
		{
			if (j != 0)
			{
				(*pNoOfQop)++;

				if (*pppQopValues == NULL)
				{
					pal_MemoryAllocate(sizeof(u_char *), (void **)(pppQopValues));
					if (*pppQopValues == NULL)
					{
						return ECRIO_SIGMGR_NO_MEMORY;
					}
				}
				else
				{
					/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
					if (pal_UtilityDataOverflowDetected(*pNoOfQop, sizeof(u_char *)) == Enum_TRUE)
					{
						return ECRIO_SIGMGR_NO_MEMORY;
					}
					
					pal_MemoryReallocate(((*pNoOfQop) * sizeof(u_char *)), (void **)(pppQopValues));
					if (*pppQopValues == NULL)
					{
						return ECRIO_SIGMGR_NO_MEMORY;
					}
				}

				if (i == (len - 1))
				{
					temp[j] = *(pQop + i);
					j++;
				}

				temp[j] = '\0';

				(*pppQopValues)[*pNoOfQop - 1] = pal_StringCreate(temp, pal_StringLength(temp));
				if ((*pppQopValues)[*pNoOfQop - 1] == NULL)
				{
					return ECRIO_SIGMGR_NO_MEMORY;
				}

				j = 0;
			}
			else
			{
				for (j = 0; j < (*pNoOfQop); j++)
				{
					pal_MemoryFree((void **)&((*pppQopValues)[j]));
				}

				pal_MemoryFree((void **)pppQopValues);

				return ECRIO_SIGMGR_IMS_LIB_ERROR;
			}
		}
		else
		{
			temp[j] = *(pQop + i);
			j++;
		}
	}

	return error;
}

/**************************************************************************

Function:		EcrioSigMgrMD5String ()

Purpose:		to generate the MD5 hashed equivalent of an input string

Description:	This function generates the 128-bit (16 byte) MD5 hashed
equivalent of the input string, using the MD5 algorithm. In
this function, the string is first converted into a 128-bit number,
and then the number is hex-transcribed into a string.

Input:		u_char* string - pointer to the string to be hashed

Input/Output:	u_char* md5Result - pointer to string where result is returned

Returns:		None

**************************************************************************/
void EcrioSigMgrMD5String
(
	u_char *string,
	u_int32 stringLength,
	u_char *md5Result
)
{
	/* basic initializations */
	md5_state_t state;
	md5_byte_t digest[16] = "";
	u_int32 i = 0;
	u_int8 resultStrIndex = (u_int8)0;
	u_int32 len = 0;
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;

	_EcrioSigMgr_md5_init(&state);
	_EcrioSigMgr_md5_append(&state, (const md5_byte_t *)string, stringLength /*pal_StringLength (string)*/);
	_EcrioSigMgr_md5_finish(&state, digest);

	/* loops through each byte and generates a string */
	for (i = 0; i < 16; i++)
	{
		if (digest[i] == (md5_byte_t)0)
		{
			/* if the  byte is 0 append "00" to the string*/
			md5Result[resultStrIndex] = (u_char)'0';
			resultStrIndex++;
			md5Result[resultStrIndex] = (u_char)'0';
			resultStrIndex++;
		}
		else if (digest[i] <= (md5_byte_t)15)
		{
			/* if the byte is less then 15 add "0" and hex decimal string value
			to the resultant string */
			md5Result[resultStrIndex] = (u_char)'0';
			resultStrIndex++;
			error = _EcrioSigMgr_IToHexAscii((u_int32)digest[i], md5Result + resultStrIndex, 20, &len);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				return;
			}

			resultStrIndex = (u_int8)(resultStrIndex + (u_char)len);
		}
		else
		{
			/* /// or else the hexa decimal string value
			is added to the resultant string*/
			error = _EcrioSigMgr_IToHexAscii((u_int32)digest[i], md5Result + resultStrIndex, 20, &len);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				return;
			}

			resultStrIndex = (u_int8)(resultStrIndex + (u_char)len);
		}
	}

	/* / end the md5 resultant string with null */
	*(md5Result + resultStrIndex) = 0;
}

/***************************************************************************
Function :	_EcrioSigMgrGenerateResponse

Purpose :   To form the authentication response and filling up the pAuthStruct
appropriately.

Description :
This internal function is responsible for forming the authentication
response and filling up the pAuthStruct(Authentication structure)
appropriately.

Input and Output Parameters :

Input :
u_char*				pPrivateUserIdentity
u_char*				pRequestURI
u_char*				pCnonce
u_int32				nc

Output :
EcriosigMgrHdrValueWithParamStruct*	pAuthStruct

Return value :
u_int32								indicates Success or Failure

***********************************************************************************/
u_int32 _EcrioSigMgrGenerateResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrAuthenticationStruct	*pAuthStruct,
	u_char *pMethodName,
	u_char *pDigestUriValue,
	u_char *pEntityBody,
	u_char *pUsername,
	u_char *pCnonceValue,
	u_char *pNcValue,
	u_char *pPassword,
	u_char passwordLength,
	u_char **ppDigestResponseOut,
	EcriosigMgrAuthorizationTypeEnum *pAuthTypeEnum
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR, noOfQop = 0, i = 0, lengthStringOut2 = 0;
	u_int32 lengthStringOut1 = 0, lengthTempResFinal = 0, lengthOUT = 0, lengthpTempString = 0;
	u_int32 uSize = 0;
	u_char *pStringOut2 = NULL, *pStringOut1 = NULL, *pTempRes1 = NULL, *pTempRes2 = NULL, *pTempResFinal = NULL, *pTempString = NULL;
	u_char *pNonce = NULL, *pAlgorithm = NULL, *pRealm = NULL, *pQop = NULL, *pOUT = NULL, *pTempOut = NULL;
	u_char **ppQopValues = NULL;
	BoolEnum bAuthIntPresent = Enum_FALSE;

	// @todo Should we check pSigMgr == NULL? We don't if this function can only be called if it is not NULL.

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pAuthStruct == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

		return error;
	}

	if (pMethodName == NULL || pDigestUriValue == NULL || pUsername == NULL || pPassword == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

		return error;
	}

	if (pAuthStruct->pNonce)
	{
		pNonce = pAuthStruct->pNonce;
	}

	switch (pAuthStruct->authAlgo)
	{
		case EcrioSipAuthAlgorithmDefault:
		{
		}
		break;

		case EcrioSipAuthAlgorithmMD5:
		{
			pAlgorithm = ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_ALGORITHM_VALUE_MD5;
		}
		break;

		case EcrioSipAuthAlgorithmAKAv1:
		{
			pAlgorithm = ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_ALGORITHM_VALUE_AKAv1_MD5;
		}
		break;

		case EcrioSipAuthAlgorithmAKAv2:
		{
			pAlgorithm = ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_ALGORITHM_VALUE_AKAv2_MD5;
		}
		break;

		case EcrioSipAuthAlgorithmUnknown:
		default:
			error = ECRIO_SIGMGR_INVALID_DATA;
	}

	if (pAuthStruct->pRealm)
	{
		pRealm = pAuthStruct->pRealm;
	}

	/* Only qop="auth" is currently supported */
	if (pAuthStruct->ppListOfQoP != NULL)
	{
		u_int16 i = 0, index = 0;
		BoolEnum bValidQoP = Enum_FALSE;

		for (i = 0; i < pAuthStruct->countQoP; i++)
		{
			if (!(pal_StringCompare(pAuthStruct->ppListOfQoP[i],
				(u_char *)"auth")))
			{
				bValidQoP = Enum_TRUE;
				index = i;
				break;
			}
			/* Support for "auth-int" */
			else if (!(pal_StringCompare(pAuthStruct->ppListOfQoP[i],
				(u_char *)"auth-int")))
			{
				bValidQoP = Enum_TRUE;
				index = i;
			}
		}

		if (bValidQoP == Enum_TRUE)
		{
			error = _EcrioSigMgrStringCreate(pSigMgr, pAuthStruct->ppListOfQoP[index], &pQop);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioSigMgrStringCreate() for pQop, error=%u",
					__FUNCTION__, __LINE__, error);
			}
		}
		else
		{
			error = ECRIO_SIGMGR_AUTH_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tSignaling Manager Auth", __FUNCTION__, __LINE__);
		}
	}

	if (pNonce == NULL || pRealm == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

		goto END;
	}

	if (pQop != NULL)
	{
		error = _EcrioSigMgrExtractQop(pQop, &noOfQop, &ppQopValues);

		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

			goto END;
		}
		else
		{
			for (i = 0; i < noOfQop; i++)
			{
				if (pal_StringCompare(ppQopValues[i], (u_char *)"auth-int") == 0)
				{
					bAuthIntPresent = Enum_TRUE;
					*pAuthTypeEnum = ECRIO_SIGMGR_QOP_AUTH_INT;
				}
				else if (pal_StringCompare(ppQopValues[i], (u_char *)"auth") == 0)
				{
					*pAuthTypeEnum = ECRIO_SIGMGR_QOP_AUTH;
					pal_MemoryFree((void **)(&ppQopValues[i]));
					break;
				}
				else
				{
					*pAuthTypeEnum = ECRIO_SIGMGR_QOP_NONE;
				}

				pal_MemoryFree((void **)(&ppQopValues[i]));
			}

			pal_MemoryFree((void **)&(ppQopValues));
		}
	}

	if (pQop != NULL)
	{
		if (*pAuthTypeEnum != ECRIO_SIGMGR_QOP_AUTH && bAuthIntPresent == Enum_TRUE)	/*If (pQop == AUTH-INT)*/
		{
			/*
			CHECK Input PARAM is not NULL
			pStringOut2 = Concatenate ( Method Name ?봡igest-uri-value ?봯OUT);
			*/
			if (pEntityBody && pal_StringLength(pEntityBody))
			{
				pal_MemoryAllocate(33 * sizeof(u_char), (void **)&pTempOut);
				if (!pTempOut)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

					goto END;
				}

				pal_MemorySet(pTempOut, 0, 33);

				EcrioSigMgrMD5String(pEntityBody, pal_StringLength(pEntityBody), pTempOut);

				lengthStringOut2 = (u_int32)pal_StringLength(pMethodName) + (u_int32)pal_StringLength(pDigestUriValue) + (u_int32)pal_StringLength(pTempOut) + 2 + 1;
			}
			else
			{
				lengthStringOut2 = (u_int32)pal_StringLength(pMethodName) + (u_int32)pal_StringLength(pDigestUriValue) + 1 + 1;
			}

			pal_MemoryAllocate(lengthStringOut2 * sizeof(u_char), (void **)&pStringOut2);
			if (!pStringOut2)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

				goto END;
			}
			uSize = lengthStringOut2 * sizeof(u_char);

			pal_MemorySet(pStringOut2, 0, lengthStringOut2);
			if (NULL == pal_StringNConcatenate(pStringOut2, uSize - pal_StringLength((const u_char *)pStringOut2), pMethodName, pal_StringLength((const u_char *)pMethodName)))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

				goto END;
			}
			if (NULL == pal_StringNConcatenate(pStringOut2, uSize - pal_StringLength((const u_char *)pStringOut2), (u_char *)":", pal_StringLength((const u_char*)":")))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

				goto END;
			}
			if (NULL == pal_StringNConcatenate(pStringOut2, uSize - pal_StringLength((const u_char *)pStringOut2), pDigestUriValue, pal_StringLength((const u_char *)pDigestUriValue)))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

				goto END;
			}
			if (pEntityBody && pal_StringLength(pEntityBody))
			{
				if (NULL == pal_StringNConcatenate(pStringOut2, uSize - pal_StringLength((const u_char *)pStringOut2), (u_char *)":", pal_StringLength((const u_char*)":")))
				{
					error = ECRIO_SIGMGR_STRING_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

					goto END;
				}
				if (NULL == pal_StringNConcatenate(pStringOut2, uSize - pal_StringLength((const u_char *)pStringOut2), pTempOut, pal_StringLength((const u_char *)pTempOut)))
				{
					error = ECRIO_SIGMGR_STRING_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

					goto END;
				}
			}
		}
		else
		{
			/*pStringOut2 = Concatenate ( Method Name ?봡igest-uri-value);*/
			lengthStringOut2 = (u_int32)pal_StringLength(pMethodName) + (u_int32)pal_StringLength(pDigestUriValue) + 1 + 1;
			pal_MemoryAllocate(lengthStringOut2 * sizeof(u_char), (void **)&pStringOut2);
			if (!pStringOut2)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

				goto END;
			}
			uSize = lengthStringOut2 * sizeof(u_char);

			pal_MemorySet(pStringOut2, 0, lengthStringOut2);
			if (NULL == pal_StringNConcatenate(pStringOut2, uSize - pal_StringLength((const u_char *)pStringOut2), pMethodName, pal_StringLength((const u_char *)pMethodName)))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

				goto END;
			}
			if (NULL == pal_StringNConcatenate(pStringOut2, uSize - pal_StringLength((const u_char *)pStringOut2), (u_char *)":", pal_StringLength((const u_char*)":")))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

				goto END;
			}
			if (NULL == pal_StringNConcatenate(pStringOut2, uSize - pal_StringLength((const u_char *)pStringOut2), pDigestUriValue, pal_StringLength((const u_char *)pDigestUriValue)))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

				goto END;
			}
		}
	}

	if (pAlgorithm != NULL)
	{
		if (pal_StringICompare(pAlgorithm, (u_char *)"MD5-sess") == 0)
		{
			/*
			if (!RES)
			Return ERROR;

			CHECK Input PARAM is not NULL
			pTempString  = Concatenate (username-value[Unquoted]  ?Realm-
			value[unquoted] ?RES);
			pStringOut1  = Concatenate (pTempString  ":" [Unquoted]nonce-value)
			":" [Unquoted]cnonce-value)
			*/
			if (pRealm)
			{
				lengthpTempString = (u_int32)pal_StringLength(pUsername) + (u_int32)passwordLength /*pal_StringLength(pPassword)*/ + (u_int32)pal_StringLength(pRealm) + 2 + 1;
				pal_MemoryAllocate(lengthpTempString * sizeof(u_char), (void **)&pTempString);
				if (!pTempString)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

					goto END;
				}
				uSize = lengthpTempString * sizeof(u_char);

				pal_MemorySet(pTempString, 0, lengthpTempString);
				if (NULL == pal_StringNConcatenate(pTempString, uSize - pal_StringLength((const u_char *)pTempString), pUsername, pal_StringLength((const u_char *)pUsername)))
				{
					error = ECRIO_SIGMGR_STRING_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

					goto END;
				}
				if (NULL == pal_StringNConcatenate(pTempString, uSize - pal_StringLength((const u_char *)pTempString), (u_char *)":", pal_StringLength((const u_char*)":")))
				{
					error = ECRIO_SIGMGR_STRING_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

					goto END;
				}
				if (NULL == pal_StringNConcatenate(pTempString, uSize - pal_StringLength((const u_char *)pTempString), pRealm, pal_StringLength((const u_char *)pRealm)))
				{
					error = ECRIO_SIGMGR_STRING_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

					goto END;
				}
				if (NULL == pal_StringNConcatenate(pTempString, uSize - pal_StringLength((const u_char *)pTempString), (u_char *)":", pal_StringLength((const u_char*)":")))
				{
					error = ECRIO_SIGMGR_STRING_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

					goto END;
				}

				lengthpTempString = pal_StringLength(pTempString);
				if (passwordLength > 0)	/* If Password is not empty string i.e. "" */
				{
					pal_MemoryCopy((void *)(pTempString + lengthpTempString), passwordLength, pPassword, passwordLength);
				}

				lengthpTempString += passwordLength;
			}
#if 0
			else
			{
				lengthpTempString = (u_int32)pal_StringLength(pUsername) + (u_int32)passwordLength /*pal_StringLength(pPassword)*/ + 1 + 1;
				pal_MemoryAllocate(lengthpTempString * sizeof(u_char), (void **)&pTempString);
				if (!pTempString)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

					goto END;
				}

				pal_MemorySet(pTempString, 0, lengthpTempString);
				pal_StringConcatenate(pTempString, pUsername);
				pal_StringConcatenate(pTempString, (u_char *)":");

				lengthpTempString = pal_StringLength(pTempString);
				if (passwordLength > 0)	/* If Password is not empty string i.e. "" */
				{
					pal_MemoryCopy((void *)(pTempString + lengthpTempString), pPassword, passwordLength);
				}

				lengthpTempString += passwordLength;
			}
#endif

			pal_MemoryAllocate(33 * sizeof(u_char), (void **)&pOUT);
			if (!pOUT)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

				goto END;
			}

			pal_MemorySet(pOUT, 0, 33);

			EcrioSigMgrMD5String(pTempString, lengthpTempString, pOUT);

			/*				pStringOut1  = Concatenate (pOUT  ":" [Unquoted]nonce-value)
			":" [Unquoted]cnonce-value)
			*/
			lengthOUT = (u_int32)pal_StringLength(pOUT) +
				(u_int32)pal_StringLength(pNonce) + 1 + 1;

			if (pCnonceValue)
			{
				lengthOUT = lengthOUT + (u_int32)pal_StringLength(pCnonceValue) + 1;
			}

			pal_MemoryAllocate(lengthOUT * sizeof(u_char), (void **)&pStringOut1);
			if (!pStringOut1)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

				goto END;
			}
			uSize = lengthOUT * sizeof(u_char);

			pal_MemorySet(pStringOut1, 0, lengthOUT);

			if (NULL == pal_StringNConcatenate(pStringOut1, uSize - pal_StringLength((const u_char *)pStringOut1), pOUT, pal_StringLength((const u_char *)pOUT)))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

				goto END;
			}
			if (NULL == pal_StringNConcatenate(pStringOut1, uSize - pal_StringLength((const u_char *)pStringOut1), (u_char *)":", pal_StringLength((const u_char*)":")))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

				goto END;
			}
			if (NULL == pal_StringNConcatenate(pStringOut1, uSize - pal_StringLength((const u_char *)pStringOut1), pNonce, pal_StringLength((const u_char *)pNonce)))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

				goto END;
			}

			if (pCnonceValue)
			{
				if (NULL == pal_StringNConcatenate(pStringOut1, uSize - pal_StringLength((const u_char *)pStringOut1), (u_char *)":", pal_StringLength((const u_char*)":")))
				{
					error = ECRIO_SIGMGR_STRING_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

					goto END;
				}
				if (NULL == pal_StringNConcatenate(pStringOut1, uSize - pal_StringLength((const u_char *)pStringOut1), pCnonceValue, pal_StringLength((const u_char *)pCnonceValue)))
				{
					error = ECRIO_SIGMGR_STRING_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

					goto END;
				}
			}

			lengthStringOut1 = lengthOUT;
		}
		else
		{
			/*
			CHECK Input PARAM is not NULL
			pStringOut1   = Concatenate ([Unquoted]username-value ":"
			[Unquoted]realm-value ":" passwd );
			*/

			if (pRealm)
			{
				lengthStringOut1 = (u_int32)pal_StringLength(pUsername) + (u_int32)passwordLength /*pal_StringLength(pPassword)*/ + (u_int32)pal_StringLength(pRealm) + 2 + 1;
				pal_MemoryAllocate(lengthStringOut1 * sizeof(u_char), (void **)&pStringOut1);
				if (!pStringOut1)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

					goto END;
				}
				uSize = lengthStringOut1 * sizeof(u_char);

				pal_MemorySet(pStringOut1, 0, lengthStringOut1);
				if (NULL == pal_StringNConcatenate(pStringOut1, uSize - pal_StringLength((const u_char *)pStringOut1), pUsername, pal_StringLength((const u_char *)pUsername)))
				{
					error = ECRIO_SIGMGR_STRING_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

					goto END;
				}
				if (NULL == pal_StringNConcatenate(pStringOut1, uSize - pal_StringLength((const u_char *)pStringOut1), (u_char *)":", pal_StringLength((const u_char*)":")))
				{
					error = ECRIO_SIGMGR_STRING_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

					goto END;
				}
				if (NULL == pal_StringNConcatenate(pStringOut1, uSize - pal_StringLength((const u_char *)pStringOut1), pRealm, pal_StringLength((const u_char *)pRealm)))
				{
					error = ECRIO_SIGMGR_STRING_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

					goto END;
				}
				if (NULL == pal_StringNConcatenate(pStringOut1, uSize - pal_StringLength((const u_char *)pStringOut1), (u_char *)":", pal_StringLength((const u_char*)":")))
				{
					error = ECRIO_SIGMGR_STRING_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

					goto END;
				}

				lengthStringOut1 = pal_StringLength(pStringOut1);
				if (passwordLength > 0)	/* If Password is not empty string i.e. "" */
				{
					pal_MemoryCopy((void *)(pStringOut1 + lengthStringOut1), passwordLength, pPassword, passwordLength);
				}

				lengthStringOut1 += (passwordLength + 1);
			}
#if 0
			else
			{
				lengthStringOut1 = (u_int32)pal_StringLength(pUsername) + (u_int32)passwordLength /*pal_StringLength(pPassword)*/ + 1 + 1;
				pal_MemoryAllocate(lengthStringOut1 * sizeof(u_char), (void **)&pStringOut1);
				if (!pStringOut1)
				{
					error = ECRIO_SIGMGR_NO_MEMORY;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

					goto END;
				}

				pal_MemorySet(pStringOut1, 0, lengthStringOut1);
				if (NULL == pal_StringConcatenate(pStringOut1, pUsername);
				if (NULL == pal_StringConcatenate(pStringOut1, (u_char *)":");

				lengthStringOut1 = pal_StringLength(pStringOut1);
				if (passwordLength > 0)	/* If Password is not empty string i.e. "" */
				{
					pal_MemoryCopy((void *)(pStringOut1 + lengthStringOut1), pPassword, passwordLength);
				}

				lengthStringOut1 += (passwordLength + 1);
			}
#endif
		}
	}
	else
	{
		/*
		CHECK Input PARAM is not NULL
		pStringOut1   = Concatenate ([Unquoted]username-value ":"
		[Unquoted]realm-value ":" passwd );
		*/

		if (pRealm)
		{
			lengthStringOut1 = (u_int32)pal_StringLength(pUsername) + (u_int32)passwordLength /*pal_StringLength(pPassword)*/ + (u_int32)pal_StringLength(pRealm) + 2 + 1;
			pal_MemoryAllocate(lengthStringOut1 * sizeof(u_char), (void **)&pStringOut1);
			if (!pStringOut1)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

				goto END;
			}
			uSize = lengthStringOut1 * sizeof(u_char);

			pal_MemorySet(pStringOut1, 0, lengthStringOut1);
			if (NULL == pal_StringNConcatenate(pStringOut1, uSize - pal_StringLength((const u_char *)pStringOut1), pUsername, pal_StringLength((const u_char *)pUsername)))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

				goto END;
			}
			if (NULL == pal_StringNConcatenate(pStringOut1, uSize - pal_StringLength((const u_char *)pStringOut1), (u_char *)":", pal_StringLength((const u_char*)":")))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

				goto END;
			}
			if (NULL == pal_StringNConcatenate(pStringOut1, uSize - pal_StringLength((const u_char *)pStringOut1), pRealm, pal_StringLength((const u_char *)pRealm)))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

				goto END;
			}
			if (NULL == pal_StringNConcatenate(pStringOut1, uSize - pal_StringLength((const u_char *)pStringOut1), (u_char *)":", pal_StringLength((const u_char*)":")))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

				goto END;
			}

			lengthStringOut1 = pal_StringLength(pStringOut1);
			if (passwordLength > 0)	/* If Password is not empty string i.e. "" */
			{
				pal_MemoryCopy((void *)(pStringOut1 + lengthStringOut1), passwordLength, pPassword, passwordLength);
			}

			lengthStringOut1 += (passwordLength + 1);
		}
#if 0
		else
		{
			lengthStringOut1 = (u_int32)pal_StringLength(pUsername) + (u_int32)passwordLength /*pal_StringLength(pPassword)*/ + 1 + 1;
			pal_MemoryAllocate(lengthStringOut1 * sizeof(u_char), (void **)&pStringOut1);
			if (!pStringOut1)
			{
				error = ECRIO_SIGMGR_NO_MEMORY;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

				goto END;
			}

			pal_MemorySet(pStringOut1, 0, lengthStringOut1);
			pal_StringConcatenate(pStringOut1, pUsername);
			pal_StringConcatenate(pStringOut1, (u_char *)":");

			lengthStringOut1 = pal_StringLength(pStringOut1);
			if (passwordLength > 0)	/* If Password is not empty string i.e. "" */
			{
				pal_MemoryCopy((void *)(pStringOut1 + lengthStringOut1), pPassword, passwordLength);
			}

			lengthStringOut1 += (passwordLength + 1);
		}
#endif
	}

	if (pQop)
	{
		pal_MemoryAllocate(33 * sizeof(u_char), (void **)&pTempRes1);
		if (!pTempRes1)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

			goto END;
		}

		pal_MemorySet(pTempRes1, 0, 33);

		EcrioSigMgrMD5String(pStringOut1, lengthStringOut1 - 1, pTempRes1);

		pal_MemoryAllocate(33 * sizeof(u_char), (void **)&pTempRes2);
		if (!pTempRes2)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

			goto END;
		}

		pal_MemorySet(pTempRes2, 0, 33);

		EcrioSigMgrMD5String(pStringOut2, lengthStringOut2 - 1, pTempRes2);

		lengthTempResFinal = (u_int32)pal_StringLength(pTempRes1) +
			(u_int32)pal_StringLength(pNonce);
		if (pNcValue)
		{
			lengthTempResFinal = lengthTempResFinal + (u_int32)pal_StringLength(pNcValue) + 1;
		}

		if (pCnonceValue)
		{
			lengthTempResFinal = lengthTempResFinal + (u_int32)pal_StringLength(pCnonceValue) + 1;
		}

		if (pQop)
		{
			lengthTempResFinal = lengthTempResFinal + (u_int32)pal_StringLength(pQop) + 1;
		}

		lengthTempResFinal = lengthTempResFinal + (u_int32)pal_StringLength(pTempRes2) + 2 + 1;
		pal_MemoryAllocate(lengthTempResFinal * sizeof(u_char), (void **)&pTempResFinal);
		if (!pTempResFinal)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

			goto END;
		}
		uSize = lengthTempResFinal * sizeof(u_char);

		pal_MemorySet(pTempResFinal, 0, lengthTempResFinal);

		if (NULL == pal_StringNConcatenate(pTempResFinal, uSize - pal_StringLength((const u_char *)pTempResFinal), pTempRes1, pal_StringLength((const u_char *)pTempRes1)))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

			goto END;
		}
		if (NULL == pal_StringNConcatenate(pTempResFinal, uSize - pal_StringLength((const u_char *)pTempResFinal), (u_char *)":", pal_StringLength((const u_char*)":")))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

			goto END;
		}
		if (NULL == pal_StringNConcatenate(pTempResFinal, uSize - pal_StringLength((const u_char *)pTempResFinal), pNonce, pal_StringLength((const u_char *)pNonce)))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

			goto END;
		}

		if (pNcValue)
		{
			if (NULL == pal_StringNConcatenate(pTempResFinal, uSize - pal_StringLength((const u_char *)pTempResFinal), (u_char *)":", pal_StringLength((const u_char*)":")))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

				goto END;
			}
			if (NULL == pal_StringNConcatenate(pTempResFinal, uSize - pal_StringLength((const u_char *)pTempResFinal), pNcValue, pal_StringLength((const u_char *)pNcValue)))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

				goto END;
			}
		}

		if (pCnonceValue)
		{
			if (NULL == pal_StringNConcatenate(pTempResFinal, uSize - pal_StringLength((const u_char *)pTempResFinal), (u_char *)":", pal_StringLength((const u_char*)":")))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

				goto END;
			}
			if (NULL == pal_StringNConcatenate(pTempResFinal, uSize - pal_StringLength((const u_char *)pTempResFinal), pCnonceValue, pal_StringLength((const u_char *)pCnonceValue)))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

				goto END;
			}
		}

		if (pQop)
		{
			if (NULL == pal_StringNConcatenate(pTempResFinal, uSize - pal_StringLength((const u_char *)pTempResFinal), (u_char *)":", pal_StringLength((const u_char*)":")))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

				goto END;
			}
			if (NULL == pal_StringNConcatenate(pTempResFinal, uSize - pal_StringLength((const u_char *)pTempResFinal), pQop, pal_StringLength((const u_char *)pQop)))
			{
				error = ECRIO_SIGMGR_STRING_ERROR;
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

				goto END;
			}
		}

		if (NULL == pal_StringNConcatenate(pTempResFinal, uSize - pal_StringLength((const u_char *)pTempResFinal), (u_char *)":", pal_StringLength((const u_char*)":")))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

			goto END;
		}
		if (NULL == pal_StringNConcatenate(pTempResFinal, uSize - pal_StringLength((const u_char *)pTempResFinal), pTempRes2, pal_StringLength((const u_char *)pTempRes2)))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

			goto END;
		}
		pal_MemoryAllocate(33 * sizeof(u_char), (void **)ppDigestResponseOut);
		if (!(*ppDigestResponseOut))
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

			goto END;
		}

		pal_MemorySet(*ppDigestResponseOut, 0, 33);

		EcrioSigMgrMD5String(pTempResFinal, lengthTempResFinal - 1, *ppDigestResponseOut);
	}
	else
	{
		/*pStringOut2 = Concatenate ( Method Name ?봡igest-uri-value);*/

		lengthStringOut2 = (u_int32)pal_StringLength(pMethodName) + (u_int32)pal_StringLength(pDigestUriValue) + 1 + 1;
		pal_MemoryAllocate(lengthStringOut2 * sizeof(u_char), (void **)&pStringOut2);
		if (!pStringOut2)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

			goto END;
		}
		uSize = lengthStringOut2 * sizeof(u_char);

		pal_MemorySet(pStringOut2, 0, lengthStringOut2);
		if (NULL == pal_StringNConcatenate(pStringOut2, uSize - pal_StringLength((const u_char *)pStringOut2), pMethodName, pal_StringLength((const u_char *)pMethodName)))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

			goto END;
		}
		if (NULL == pal_StringNConcatenate(pStringOut2, uSize - pal_StringLength((const u_char *)pStringOut2), (u_char *)":", pal_StringLength((const u_char*)":")))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

			goto END;
		}
		if (NULL == pal_StringNConcatenate(pStringOut2, uSize - pal_StringLength((const u_char *)pStringOut2), pDigestUriValue, pal_StringLength((const u_char *)pDigestUriValue)))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

			goto END;
		}
		pal_MemoryAllocate(33 * sizeof(u_char), (void **)&pTempRes1);
		if (!pTempRes1)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

			goto END;
		}

		pal_MemorySet(pTempRes1, 0, 33);

		EcrioSigMgrMD5String(pStringOut1, lengthStringOut1 - 1, pTempRes1);

		pal_MemoryAllocate(33 * sizeof(u_char), (void **)&pTempRes2);
		if (!pTempRes2)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

			goto END;
		}

		pal_MemorySet(pTempRes2, 0, 33);

		EcrioSigMgrMD5String(pStringOut2, lengthStringOut2 - 1, pTempRes2);

		lengthTempResFinal = (u_int32)pal_StringLength(pTempRes1) + (u_int32)pal_StringLength(pNonce) + (u_int32)pal_StringLength(pTempRes2) + 2 + 1;
		pal_MemoryAllocate(lengthTempResFinal * sizeof(u_char), (void **)&pTempResFinal);
		if (!pTempResFinal)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

			goto END;
		}
		uSize = lengthTempResFinal * sizeof(u_char);

		pal_MemorySet(pTempResFinal, 0, lengthTempResFinal);

		if (NULL == pal_StringNConcatenate(pTempResFinal, uSize - pal_StringLength((const u_char *)pTempResFinal), pTempRes1, pal_StringLength((const u_char *)pTempRes1)))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

			goto END;
		}
		if (NULL == pal_StringNConcatenate(pTempResFinal, uSize - pal_StringLength((const u_char *)pTempResFinal), (u_char *)":", pal_StringLength((const u_char*)":")))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

			goto END;
		}
		if (NULL == pal_StringNConcatenate(pTempResFinal, uSize - pal_StringLength((const u_char *)pTempResFinal), pNonce, pal_StringLength((const u_char *)pNonce)))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

			goto END;
		}
		if (NULL == pal_StringNConcatenate(pTempResFinal, uSize - pal_StringLength((const u_char *)pTempResFinal), (u_char *)":", pal_StringLength((const u_char*)":")))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

			goto END;
		}
		if (NULL == pal_StringNConcatenate(pTempResFinal, uSize - pal_StringLength((const u_char *)pTempResFinal), pTempRes2, pal_StringLength((const u_char *)pTempRes2)))
		{
			error = ECRIO_SIGMGR_STRING_ERROR;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> String Copy error.", __FUNCTION__, __LINE__);

			goto END;
		}
		pal_MemoryAllocate(33 * sizeof(u_char), (void **)ppDigestResponseOut);
		if (!(*ppDigestResponseOut))
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t -> Signaling parser error occurred,error = %u ", __FUNCTION__, __LINE__, error);

			goto END;
		}

		pal_MemorySet(*ppDigestResponseOut, 0, 33);

		EcrioSigMgrMD5String(pTempResFinal, lengthTempResFinal - 1, *ppDigestResponseOut);
	}

END:

	SIGMGRLOGD(pSigMgr->pLogHandle, KLogType_Fine, "%s:%u\t -> AKAv1-MD5 Details ", __FUNCTION__, __LINE__);
	SIGMGRLOGD(pSigMgr->pLogHandle, KLogType_Fine, "%s:%u\tA1 = %s ", __FUNCTION__, __LINE__, pStringOut1);
	SIGMGRLOGD(pSigMgr->pLogHandle, KLogType_Fine, "%s:%u\tH(A1) = %s ", __FUNCTION__, __LINE__, pTempRes1);
	SIGMGRLOGD(pSigMgr->pLogHandle, KLogType_Fine, "%s:%u\tA2 = %s ", __FUNCTION__, __LINE__, pStringOut2);
	SIGMGRLOGD(pSigMgr->pLogHandle, KLogType_Fine, "%s:%u\tH(A2) = %s ", __FUNCTION__, __LINE__, pTempRes2);
	SIGMGRLOGD(pSigMgr->pLogHandle, KLogType_Fine, "%s:%u\tH(A1):nonce-value:nc-value:cnonce-value:qop-values:H(A2)= ", __FUNCTION__, __LINE__);
	SIGMGRLOGD(pSigMgr->pLogHandle, KLogType_Fine, "%s:%u\t%s", __FUNCTION__, __LINE__, pTempResFinal);
	SIGMGRLOGD(pSigMgr->pLogHandle, KLogType_Fine, "%s:%u\tH(H(A1):nonce-value:nc-value:cnonce-value:qop-values:H(A2)) = %s ", __FUNCTION__, __LINE__, *ppDigestResponseOut);

	if (pStringOut1 != NULL)
	{
		pal_MemoryFree((void **)&pStringOut1);
		pStringOut1 = NULL;
	}

	if (pStringOut2 != NULL)
	{
		pal_MemoryFree((void **)&pStringOut2);
		pStringOut2 = NULL;
	}

	if (pTempRes1 != NULL)
	{
		pal_MemoryFree((void **)&pTempRes1);
		pTempRes1 = NULL;
	}

	if (pTempRes2 != NULL)
	{
		pal_MemoryFree((void **)&pTempRes2);
		pTempRes2 = NULL;
	}

	if (pTempResFinal != NULL)
	{
		pal_MemoryFree((void **)&pTempResFinal);
		pTempResFinal = NULL;
	}

	if (pQop != NULL)
	{
		pal_MemoryFree((void **)&pQop);
		pQop = NULL;
	}

	if (pTempString)
	{
		pal_MemoryFree((void **)&pTempString);
		pTempString = NULL;
	}

	if (pOUT)
	{
		pal_MemoryFree((void **)&pOUT);
		pOUT = NULL;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t -> exit with error= %u", __FUNCTION__, __LINE__, error);

	return error;
}

/* Generate AKA RES from the auth challenge */
/*****************************************************************************

Function:		_EcrioSigMgrGetAKARes ()

Purpose:		to obtain AKA RES from the auth challenge

Description:	This function is used to obtain the AKA RES/CK/IK or AUTS
value using the nonce received in the server auth challenge.
The PFD API DRVGetAuthResponseCode will be used for this
purpose.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager instance.
uchar* pNonce - nonce value received in the challenge found
in 401/407 response.

Output:			EcrioPhoneAuthenticationStruct** ppAuthResponse - AKA RES/CK/IK
with optional AUTS.

Returns:		Error Code

*****************************************************************************/
u_int32 _EcrioSigMgrGetAKARes
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pNonce,					/* Nonce value contained in the challenge */
	EcrioSipAuthAlgorithmEnum eAuthAlgo,/*	EcrioSipAuthAlgorithmAKAv1 or EcrioSipAuthAlgorithmAKAv2	*/
	EcrioSipAuthStruct **ppAuthResponse
)											/* Output: AKA RES, CK, IK, AUTS */
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 encodeLength = 0;
	u_char *pAUTS = NULL;
	EcrioSipAuthStruct *pAuthResponse = NULL;
	EcrioSipPropertyNameEnums propName = EcrioSipPropertyName_AKAV1;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if ((ppAuthResponse == NULL) || (pNonce == NULL) || (pNonce[0] == '\0'))
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient Data Provided",
			__FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	if (eAuthAlgo != EcrioSipAuthAlgorithmAKAv1 && eAuthAlgo != EcrioSipAuthAlgorithmAKAv2)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeFuncParams,
			"%s:%u\tInsufficient Data Provided",
			__FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	*ppAuthResponse = NULL;

	pal_MemoryAllocate(sizeof(EcrioSipAuthStruct), (void **)&pAuthResponse);
	if (!pAuthResponse)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Level_01;
	}

	/* Parse the nonce string to its respective components - RAND, AUTN, ServerData (rfc 3310) */
	error = _EcrioSigMgrDecomposeAKANonce(pNonce, pAuthResponse);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tError populating Nonce",
			__FUNCTION__, __LINE__);
		goto Error_Level_01;
	}

	/* This is used to set the Algorithm type */
	// This needs to be taken from Config file
	/* Set the AKA version using IPsec status */
	/*if(pSigMgr->pSignalingInfo->bIPsecEnabled == Enum_TRUE)
	pAuthResponse->uAlgorithmType = ECRIO_PHONE_AUTH_ALGORITHM_AKAv1;
	else
	pAuthResponse->uAlgorithmType = ECRIO_PHONE_AUTH_ALGORITHM_AKAv2;*/
	if (eAuthAlgo == EcrioSipAuthAlgorithmAKAv2)
	{
		propName = EcrioSipPropertyName_AKAV2;
	}
	else
	{
		propName = EcrioSipPropertyName_AKAV1;
	}

	/* Check MAC error before proceeding further. If other failure
	DeInit the Phone module. */
	(pSigMgr->limsCallbackStruct.getPropertyCallbackFn)(propName, EcrioSipPropertyType_AUTH, pAuthResponse,
		pSigMgr->limsCallbackStruct.pCallbackData);
	if (!pAuthResponse->pRes && !pAuthResponse->pAuts)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tMAC Failure", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_AKA_MAC_FAILURE;
	}

	/* Check AKA response length */
	if (ECRIO_SIGMGR_NO_ERROR == error)
	{
		/* The length of RES is at most 128 bits and at least 32 bits */
		if (eAuthAlgo == EcrioSipAuthAlgorithmAKAv2)
		{
			if (((pAuthResponse->uResSize < 16) || (pAuthResponse->uResSize > 128)) &&
				(pAuthResponse->pAuts == NULL))	// AKAv2-MD5
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tUnable to get AKAv2 Response string",
					__FUNCTION__, __LINE__);
				error = ECRIO_SIGMGR_AKA_FAILURE;
				goto Error_Level_01;
			}
		}
		else if (eAuthAlgo == EcrioSipAuthAlgorithmAKAv1)
		{
			if (((pAuthResponse->uResSize < 4) || (pAuthResponse->uResSize > 16)) &&
				(pAuthResponse->pAuts == NULL))	// AKAv1-MD5
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
					"%s:%u\tUnable to get AKAv1 Response string",
					__FUNCTION__, __LINE__);
				error = ECRIO_SIGMGR_AKA_FAILURE;
				goto Error_Level_01;
			}
		}
	}

	/* As per RFC 3310, auts in Authorization header is Base64 encoded
	value of the AUTS value returned while computing AKA RES. */
	/* The length of AUTS is 112 bits (14 Bytes) */
	if ((pAuthResponse->uAutsSize == ECRIO_SIG_MGR_AKA_AUTS_LENGTH) && (pAuthResponse->pAuts != NULL))
	{
		encodeLength = _EcrioSigMgrBase64EncodeRequireBufferLength(pAuthResponse->uAutsSize);
		error = _EcrioSigMgrBase64Encode(&pAUTS, pAuthResponse->pAuts, pAuthResponse->uAutsSize, encodeLength);
		if ((error != ECRIO_SIGMGR_NO_ERROR) ||
			(pAUTS == NULL) || !(encodeLength))
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tError while Base64 Encode of AUTS Buffer",
				__FUNCTION__, __LINE__);
			error = ECRIO_SIGMGR_AKA_FAILURE;
			goto Error_Level_01;
		}

		pal_MemoryFree((void **)&pAuthResponse->pAuts);
		pAuthResponse->pAuts = pAUTS;
		pAUTS = NULL;
		pAuthResponse->uAutsSize = encodeLength;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\tReslen: %u",
		__FUNCTION__, __LINE__,
		pAuthResponse->uResSize);
	if (pAuthResponse->uResSize)
	{
		SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeGeneral, pAuthResponse->pRes, pAuthResponse->uResSize);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\tReslen: %u",
		__FUNCTION__, __LINE__,
		pAuthResponse->uAutsSize);
	if (pAuthResponse->uAutsSize)
	{
		SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeGeneral, pAuthResponse->pAuts, pAuthResponse->uAutsSize);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\tIKlen: %u",
		__FUNCTION__, __LINE__,
		pAuthResponse->uIKSize);

	if (pAuthResponse->uIKSize)
	{
		SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeGeneral, pAuthResponse->pIK, pAuthResponse->uIKSize);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\tCKlen: %u",
		__FUNCTION__, __LINE__,
		pAuthResponse->uCKSize);

	if (pAuthResponse->uCKSize)
	{
		SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeGeneral, pAuthResponse->pCK, pAuthResponse->uCKSize);
	}

	*ppAuthResponse = pAuthResponse;
	pAuthResponse = NULL;

Error_Level_01:

	if (pAuthResponse)
	{
		_EcrioSigMgrReleaseAKAStruct(pSigMgr, pAuthResponse);
		pal_MemoryFree((void **)&pAuthResponse);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return error;
}

u_int32 _EcrioSigMgrDecomposeAKANonce
(
	u_char *pNonce,
	EcrioSipAuthStruct *pAuthResponse
)
{
	u_int32 nonceLength = 0;
	u_int32 decodeLength = 0;
	u_int32 valReturn = 0;
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_char *pDecodeNonce = NULL;

	nonceLength = pal_StringLength(pNonce);
	decodeLength = _EcrioSigMgrBase64DecodeRequireBufferLength(nonceLength);
	/* RAND: 16 Bytes + AUTN: 16 Bytes */
	/* Therefore atleast minimum of 32 bytes of data MUST be present in decoded nonce */
	if (decodeLength < 32)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	valReturn = _EcrioSigMgrBase64Decode(&pDecodeNonce, pNonce, nonceLength, decodeLength);
	if (valReturn != 0 || pDecodeNonce == NULL)
	{
		return ECRIO_SIGMGR_INVALID_DATA;
	}

	pal_MemoryAllocate(ECRIO_SIG_MGR_AKA_RAND_LENGTH, (void **)&(pAuthResponse->pRand));
	if (NULL == pAuthResponse->pRand)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto ErrorTag;
	}

	/* Extract first 16 bytes for RAND */
	pal_MemoryCopy((void *)pAuthResponse->pRand, ECRIO_SIG_MGR_AKA_RAND_LENGTH, (void *)pDecodeNonce, ECRIO_SIG_MGR_AKA_RAND_LENGTH);
	pAuthResponse->uRandSize = ECRIO_SIG_MGR_AKA_RAND_LENGTH;

	pal_MemoryAllocate(ECRIO_SIG_MGR_AKA_AUTN_LENGTH, (void **)&(pAuthResponse->pAutn));
	if (NULL == pAuthResponse->pAutn)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto ErrorTag;
	}

	/* Extract next 16 bytes for AUTN */
	pal_MemoryCopy((void *)pAuthResponse->pAutn, ECRIO_SIG_MGR_AKA_AUTN_LENGTH, (void *)(pDecodeNonce + ECRIO_SIG_MGR_AKA_RAND_LENGTH), ECRIO_SIG_MGR_AKA_AUTN_LENGTH);
	pAuthResponse->uAutnSize = ECRIO_SIG_MGR_AKA_AUTN_LENGTH;

ErrorTag:
	pal_MemoryFree((void **)&pDecodeNonce);
	pDecodeNonce = NULL;

	return error;
}
