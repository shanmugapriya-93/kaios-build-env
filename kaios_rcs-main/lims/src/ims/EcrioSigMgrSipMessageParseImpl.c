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

/****************************************************************************
** FUNCTION: _EcrioSigMgrParseCSeqHeader
** DESCRIPTION: This function parses the CSeq Header
**
** PARAMETERS:
**				pStart(IN)		- The starting point in the input buffer
**								  from where this parser can start parsing.
**				pEnd(IN)		- The position in the input buffer till
**								  which this parser should parse.
**				hdr(IN)			- The particular header for which this
**								  parser is being invoked (required since
**								  multiple similar headers could be parsed
**								  by the same function).
**				pOutHdr(OUT)	- A void structure with the relevant
**								  headers filled.
**				pErr(OUT)		- Error value if any
**
** NOTE:
****************************************************************************/
u_int32	_EcrioSigMgrParseCSeqHeader
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSigMgrSipMessageStruct *pSipMessage
)
{
	u_char *pCurrentPos = pStart;
	u_char *pToken;
	u_char *pTemp;
	u_char *pMaxAllowedCseqStr = (u_char *)"4294967295";
	u_int32	MaxAllowedCseqLen = 10, i = 0;
	u_int32	size;
	u_char *pMethod = NULL;
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;

	(void)pSigMgr;

	/* Skip leading LWS */
	pCurrentPos = _EcrioSigMgrStripLeadingLWS(pCurrentPos, pEnd);

	/* Skip trailing LWS */
	pEnd = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pEnd);

	/* Tokenize for SP */
	pToken = _EcrioSigMgrTokenize(pCurrentPos, pEnd, ' ');	/* -mustfreefresh flag warning */
	/** Boundary checking */
	if (NULL == pToken || pToken == pEnd)
	{
		return ECRIO_SIGMGR_IMS_LIB_ERROR;
	}

	/*
	* Validate and store the Seq Number part
	*/
	pTemp = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pToken - 1);	/* -mustfreefresh flag warning */
	/* matching cseq string vs max allowed cseq-string = "4294967295" */
	if ((u_int32)(pTemp - pCurrentPos) > MaxAllowedCseqLen - 1)
	{
		return ECRIO_SIGMGR_IMS_LIB_ERROR;
	}
	else if ((u_int32)(pTemp - pCurrentPos) == MaxAllowedCseqLen - 1)
	{
		for (i = 0; i < MaxAllowedCseqLen; i++)
		{
			if (*(u_char *)(pCurrentPos + i) < *(u_char *)(pMaxAllowedCseqStr + i))
			{
				break;
			}
			else if (*(u_char *)(pCurrentPos + i) > *(u_char *)(pMaxAllowedCseqStr + i))
			{
				return ECRIO_SIGMGR_IMS_LIB_ERROR;
			}
		}
	}

	if (pSipMessage->pMandatoryHdrs == NULL)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&pSipMessage->pMandatoryHdrs);
		if (pSipMessage->pMandatoryHdrs == NULL)
		{
			return ECRIO_SIGMGR_NO_MEMORY;
		}
	}

	pSipMessage->pMandatoryHdrs->CSeq = pal_StringConvertToUNum(pCurrentPos, &pTemp, 10);

	/*
	* Store the Method after stripping leading LWS
	*/

	pTemp = _EcrioSigMgrStripLeadingLWS(pToken + 1, pEnd);	/* -mustfreefresh flag warning */

	size = (u_int32)(pEnd - pTemp + 1);
	if (0 == size)	/* PC-LINT: variable 'size' is unsigned so it can't be less than 0 */
	{
		/*
		* No method present
		*/

		uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
		goto Error;
	}

	pMethod = pal_StringCreate(pTemp, size);
	pSipMessage->eMethodType = _EcrioSigMgrGetMethodTypeFromString(pMethod);
	pal_MemoryFree((void **)&pMethod);

	return ECRIO_SIGMGR_NO_ERROR;

Error:
	if (pSipMessage->pMandatoryHdrs != NULL)
	{
		pal_MemoryFree((void **)&pSipMessage->pMandatoryHdrs);
	}

	return uError;
}

/****************************************************************************
** FUNCTION: _EcrioSigMgrParseNameAddrWithParams
** DESCRIPTION: This function parses the NameAddrspec
**
** PARAMETERS:
**				pStart(IN)		- The starting point in the input buffer
**								  from where this parser can start parsing.
**				pEnd(IN)		- The position in the input buffer till
**								  which this parser should parse.
**				pNameAddrParam(OUT)	- The EcrioSigMgrNameAddrWithParamsStruct
**								  structure with the relevant
**								  headers filled.
**				pErr(OUT		- Error value if any
**
** NOTE:
****************************************************************************/
u_int32 _EcrioSigMgrParseNameAddrWithParams
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd, \
	EcrioSigMgrNameAddrWithParamsStruct *pNameAddrParam
)
{
	u_char *pCurrentPos = pStart;
	u_char *pToken;
	u_char *pAddrSpecEnd = NULL;
	u_int32	size;
	u_char *pTemp2 = NULL;
	u_char *pExtraCharsBegin = NULL, *pExtraCharsEnd = NULL;
	u_char *pEndUserinfo = NULL;
	BoolEnum raquotExpected = Enum_FALSE;
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;

	while (*pCurrentPos == '\t' || *pCurrentPos == ':')
	{
		pCurrentPos++;
	}

	/* Skip leading LWS */
	pCurrentPos = _EcrioSigMgrStripLeadingLWS(pCurrentPos, pEnd);

	/* Skip trailing LWS */
	pEnd = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pEnd);

	/* Now pCurrentPos points to a non-LWS character */

	/* Tokenize for < */
	pToken = _EcrioSigMgrTokenizeQuotedString(pCurrentPos, pEnd, '<');
	if (NULL == pToken)
	{
		/*
		* Display name is not present
		*/
		pNameAddrParam->nameAddr.pDisplayName = NULL;
	}
	/** Boundary checking */
	else if (pToken == pEnd)
	{
		return ECRIO_SIGMGR_INVALID_URI_SCHEME;
	}
	else
	{
		/*
		* Display name may or may not be present. If display name
		* is of non-zero length, store it.
		*/

		if (pCurrentPos != pToken)
		{
			u_char *pTempLWS;

			pTempLWS = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pToken - 1);
			size = (u_int32)(pTempLWS - pCurrentPos + 1);
			if (size > 0)
			{
				/*
				* Display name is present
				*/
				_EcrioSigMgrStringNCreate(pSigMgr, pCurrentPos, &pNameAddrParam->nameAddr.pDisplayName, size);
			}
			else
			{
				pNameAddrParam->nameAddr.pDisplayName = NULL;
			}
		}
		else
		{
			pNameAddrParam->nameAddr.pDisplayName = NULL;
		}

		/*
		* Advance pCurrentPos
		*/

		pCurrentPos = pToken + 1;

		/*
		* LAQUOT found, now look for RAQUOT
		*/

		raquotExpected = Enum_TRUE;
	}	/* end of else if disp-name present */

	/*
	* Now pCurrentPos points to start of addr-spec.
	* Next, determine end of addr-spec
	*/

	pToken = _EcrioSigMgrTokenize(pCurrentPos, pEnd, '>');
	if (NULL != pToken)
	{
		if (Enum_FALSE == raquotExpected)
		{
			/*
			* RAQUOT found whereas LAQUOT was never encountered.
			* Raise parser error.
			*/
			uError = ECRIO_SIGMGR_INVALID_URI_SCHEME;
			goto Error;
		}

		pAddrSpecEnd = pToken - 1;

		/*
		* Store the position where ">" is found. Required later for
		* checking for presence of dangling characters after the end of
		* addr-spec.
		*/

		pTemp2 = pToken;
	}
	/** Boundary checking */
	else if (pToken == pEnd)
	{
		uError = ECRIO_SIGMGR_INVALID_URI_SCHEME;
		goto Error;
	}
	else
	{
		if (Enum_TRUE == raquotExpected)
		{
			/*
			* RAQUOT expected but not found.
			* Raise parser error.
			*/
			uError = ECRIO_SIGMGR_INVALID_URI_SCHEME;
			goto Error;
		}

		/*
		* In this case end of addr-spec is as yet undetermined.
		*/
	}

	/*
	* Check userinfo is present or not
	* Because in the userinfo ';' are allowed
	*/
	if (NULL == pToken)
	{
		pEndUserinfo = _EcrioSigMgrTokenize(pCurrentPos, pEnd, '@');
	}

	/*
	* Search for parameters
	*/

	pToken = _EcrioSigMgrTokenize((NULL == pToken) ? ((NULL == pEndUserinfo) ? pCurrentPos : pEndUserinfo) : (pToken + 1),
		pEnd, ';');	/* -mustfreefresh flag warning */
	if (NULL == pToken)
	{
		/*
		* No from-params present
		*/

		if (NULL == pAddrSpecEnd)
		{
			pAddrSpecEnd = pEnd;
		}
	}
	else
	{
		if (NULL == pAddrSpecEnd)
		{
			pAddrSpecEnd = pToken - 1;
		}
	}

	if ((NULL != pTemp2) && (pTemp2 < pEnd))
	{
		/*
		* Determine beginning and end of such dangling chars.
		*/

		pExtraCharsBegin = pTemp2 + 1;
		pExtraCharsEnd = (NULL == pToken) ? pEnd : pToken - 1;
		pExtraCharsBegin = _EcrioSigMgrStripLeadingLWS(pExtraCharsBegin, pExtraCharsEnd);
		pExtraCharsEnd = _EcrioSigMgrStripTrailingLWS(pExtraCharsBegin, pExtraCharsEnd);

		/*
		* If non-white space dangling chars found, throw an error.
		*/

		if ((0 < (pExtraCharsEnd - pExtraCharsBegin + 1)) && (1 < (pExtraCharsEnd - pExtraCharsBegin + 1)))
		{
			uError = ECRIO_SIGMGR_INVALID_URI_SCHEME;
			goto Error;
		}
	}

	/*
	* Invoke address spec parser
	*/

	if (_EcrioSigMgrParseAddrSpec(pSigMgr, pCurrentPos, pAddrSpecEnd, \
		&pNameAddrParam->nameAddr.addrSpec))
	{
			uError = ECRIO_SIGMGR_INVALID_URI_SCHEME;
			goto Error;
	}

	if (pNameAddrParam->nameAddr.addrSpec.uriScheme != EcrioSigMgrURISchemeSIP)
	{
		if (pExtraCharsBegin != NULL)
		{
			pCurrentPos = pExtraCharsBegin;
		}
		else if (pTemp2 != NULL)
		{
			pExtraCharsBegin = pTemp2 + 1;
			pExtraCharsEnd = (NULL == pToken) ? pEnd : pToken - 1;
			pExtraCharsBegin = _EcrioSigMgrStripLeadingLWS(pExtraCharsBegin, pExtraCharsEnd);
			if (pExtraCharsBegin != NULL)
			{
				pCurrentPos = pExtraCharsBegin;
			}
		}
	}

	if (pNameAddrParam->nameAddr.addrSpec.uriScheme == EcrioSigMgrURISchemeSIP)
	{
		if (pNameAddrParam->nameAddr.addrSpec.u.pSipUri->numURIParams > 0)
		{
			pCurrentPos = pAddrSpecEnd;
		}
	}

	if (NULL != pToken)
	{
		/*
		* Params expected; call param parser.
		*/

		pCurrentPos = pToken + 1;
		/** Boundary checking */
		if (pCurrentPos > pEnd)
		{
			uError = ECRIO_SIGMGR_INVALID_URI_SCHEME;
			goto Error;
		}

		if (_EcrioSigMgrParseSipParam(pSigMgr, &(pNameAddrParam->numParams), &(pNameAddrParam->ppParams), pCurrentPos, \
			pEnd, Enum_FALSE, ';'))
		{
			uError = ECRIO_SIGMGR_INVALID_URI_SCHEME;
			goto Error;
		}
	}

	return ECRIO_SIGMGR_NO_ERROR;
	
Error:
	if (pNameAddrParam != NULL)
	{
		if (pNameAddrParam->nameAddr.pDisplayName != NULL)
		{
			pal_MemoryFree((void **)&pNameAddrParam->nameAddr.pDisplayName);
			pNameAddrParam->nameAddr.pDisplayName = NULL;
		}
		EcrioSigMgrReleaseUriStruct(pSigMgr, &pNameAddrParam->nameAddr.addrSpec);
	}
	return uError;
}

/****************************************************************************
** FUNCTION: _EcrioSigMgrParseNameAddr
** DESCRIPTION: This function parses the NameAddrspec
**
** PARAMETERS:
**				pStart(IN)		- The starting point in the input buffer
**								  from where this parser can start parsing.
**				pEnd(IN)		- The position in the input buffer till
**								  which this parser should parse.
**				pNameAddrParam(OUT)	- The EcrioSigMgrNameAddrWithParamsStruct
**								  structure with the relevant
**								  headers filled.
**				pErr(OUT		- Error value if any
**
** NOTE:
****************************************************************************/
u_int32 _EcrioSigMgrParseNameAddr
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd, \
	EcrioSigMgrNameAddrStruct *pNameAddr
)
{
	u_char *pCurrentPos = pStart;
	u_char *pToken;
	u_char *pAddrSpecEnd = NULL;
	u_int32	size;
	u_char *pTemp2 = NULL;
	u_char *pExtraCharsBegin = NULL, *pExtraCharsEnd = NULL;
	u_char *pEndUserinfo = NULL;
	BoolEnum raquotExpected = Enum_FALSE;
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;

	while (*pCurrentPos == '\t' || *pCurrentPos == ':')
	{
		pCurrentPos++;
	}

	/* Skip leading LWS */
	pCurrentPos = _EcrioSigMgrStripLeadingLWS(pCurrentPos, pEnd);

	/* Skip trailing LWS */
	pEnd = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pEnd);

	/* Now pCurrentPos points to a non-LWS character */

	/* Tokenize for < */
	pToken = _EcrioSigMgrTokenizeQuotedString(pCurrentPos, pEnd, '<');
	if (NULL == pToken)
	{
		/*
		* Display name is not present
		*/
		pNameAddr->pDisplayName = NULL;
	}
	/** Boundary checking */
	else if (pToken == pEnd)
	{
		return ECRIO_SIGMGR_INVALID_URI_SCHEME;
	}
	else
	{
		/*
		* Display name may or may not be present. If display name
		* is of non-zero length, store it.
		*/

		if (pCurrentPos != pToken)
		{
			u_char *pTempLWS;

			pTempLWS = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pToken - 1);
			size = (u_int32)(pTempLWS - pCurrentPos + 1);
			if (size > 0)
			{
				/*
				* Display name is present
				*/
				_EcrioSigMgrStringNCreate(pSigMgr, pCurrentPos, &pNameAddr->pDisplayName, size);
			}
			else
			{
				pNameAddr->pDisplayName = NULL;
			}
		}
		else
		{
			pNameAddr->pDisplayName = NULL;
		}

		/*
		* Advance pCurrentPos
		*/

		pCurrentPos = pToken + 1;

		/*
		* LAQUOT found, now look for RAQUOT
		*/

		raquotExpected = Enum_TRUE;
	}	/* end of else if disp-name present */

	/*
	* Now pCurrentPos points to start of addr-spec.
	* Next, determine end of addr-spec
	*/

	pToken = _EcrioSigMgrTokenize(pCurrentPos, pEnd, '>');
	if (NULL != pToken)
	{
		if (Enum_FALSE == raquotExpected)
		{
			/*
			* RAQUOT found whereas LAQUOT was never encountered.
			* Raise parser error.
			*/

			uError = ECRIO_SIGMGR_INVALID_URI_SCHEME;
			goto Error;
		}

		pAddrSpecEnd = pToken - 1;

		/*
		* Store the position where ">" is found. Required later for
		* checking for presence of dangling characters after the end of
		* addr-spec.
		*/

		pTemp2 = pToken;
	}
	/** Boundary checking */
	else if (pToken == pEnd)
	{
		return ECRIO_SIGMGR_INVALID_URI_SCHEME;
	}
	else
	{
		if (Enum_TRUE == raquotExpected)
		{
			/*
			* RAQUOT expected but not found.
			* Raise parser error.
			*/

			uError = ECRIO_SIGMGR_INVALID_URI_SCHEME;
			goto Error;
		}

		/*
		* In this case end of addr-spec is as yet undetermined.
		*/
	}

	/*
	* Check userinfo is present or not
	* Because in the userinfo ';' are allowed
	*/
	if (NULL == pToken)
	{
		pEndUserinfo = _EcrioSigMgrTokenize(pCurrentPos, pEnd, '@');
	}

	/*
	* Search for parameters
	*/

	pToken = _EcrioSigMgrTokenize((NULL == pToken) ? ((NULL == pEndUserinfo) ? pCurrentPos : pEndUserinfo) : (pToken + 1),
		pEnd, ';');	/* -mustfreefresh flag warning */
	if (NULL == pToken)
	{
		/*
		* No from-params present
		*/

		if (NULL == pAddrSpecEnd)
		{
			pAddrSpecEnd = pEnd;
		}
	}
	else
	{
		if (NULL == pAddrSpecEnd)
		{
			pAddrSpecEnd = pToken - 1;
		}
	}

	if ((NULL != pTemp2) && (pTemp2 < pEnd))
	{
		/*
		* Determine beginning and end of such dangling chars.
		*/

		pExtraCharsBegin = pTemp2 + 1;
		pExtraCharsEnd = (NULL == pToken) ? pEnd : pToken - 1;
		pExtraCharsBegin = _EcrioSigMgrStripLeadingLWS(pExtraCharsBegin, pExtraCharsEnd);
		pExtraCharsEnd = _EcrioSigMgrStripTrailingLWS(pExtraCharsBegin, pExtraCharsEnd);

		/*
		* If non-white space dangling chars found, throw an error.
		*/

		if ((0 < (pExtraCharsEnd - pExtraCharsBegin + 1)) && (1 < (pExtraCharsEnd - pExtraCharsBegin + 1)))
		{
			uError = ECRIO_SIGMGR_INVALID_URI_SCHEME;
			goto Error;
		}
	}

	/*
	* Invoke address spec parser
	*/

	if (_EcrioSigMgrParseAddrSpec(pSigMgr, pCurrentPos, pAddrSpecEnd, &pNameAddr->addrSpec))
	{
		uError = ECRIO_SIGMGR_INVALID_URI_SCHEME;
		goto Error;
	}

	if (pNameAddr->addrSpec.uriScheme != EcrioSigMgrURISchemeSIP)
	{
		/*if (pExtraCharsBegin != NULL)
		{
			pCurrentPos = pExtraCharsBegin;
		}
		else */if (pTemp2 != NULL)
		{
			pExtraCharsBegin = pTemp2 + 1;
			pExtraCharsEnd = (NULL == pToken) ? pEnd : pToken - 1;
			pExtraCharsBegin = _EcrioSigMgrStripLeadingLWS(pExtraCharsBegin, pExtraCharsEnd);
			/*if (pExtraCharsBegin != NULL)
			{
				pCurrentPos = pExtraCharsBegin;
			}*/
		}
	}

	return ECRIO_SIGMGR_NO_ERROR;

Error:
	if (NULL != pNameAddr->pDisplayName)
	{
		pal_MemoryFree((void **)&pNameAddr->pDisplayName);
	}

	return uError;
}

/****************************************************************************
** FUNCTION: _EcrioSigMgrParseStringHeader
** DESCRIPTION: This function parses the Allow, Supported, Unsupported, Allow-
**				Events, Require, Call-Id and other Headers.
**
** PARAMETERS:
**				pStart(IN)		- The starting point in the input buffer
**								  from where this parser can start parsing.
**				pEnd(IN)		- The position in the input buffer till
**								  which this parser should parse.
**				hdr(IN)			- The particular header for which this
**								  parser is being invoked (required since
**								  multiple similar headers could be parsed
**								  by the same function).
**				pOutHdr(OUT)	- A void structure with the relevant
**								  headers filled.
**				pErr(OUT)		- Error value if any
**
** NOTE:
****************************************************************************/
u_int32	_EcrioSigMgrParseStringHeader
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSipHeaderTypeEnum dType,
	EcrioSigMgrSipMessageStruct *pSipMessage
)
{
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;
	u_char *pCurrentPos = pStart;
	EcrioSigMgrHeaderStruct	*pHdr = NULL;
	u_int16 numParams = 0;
	EcrioSigMgrParamStruct **ppParams = NULL;
	u_char *pHeaderValue = NULL;

//	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
//		__FUNCTION__, __LINE__);

	if (dType == EcrioSipHeaderTypeCallId ||
		dType == EcrioSipHeaderTypeContentType)
	{
		if (dType == EcrioSipHeaderTypeContentType)
		{
			if (pSipMessage->pContentType == NULL)
			{
				pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderStruct), (void **)&pHdr);
				if (pHdr == NULL)
				{
					return ECRIO_SIGMGR_NO_MEMORY;
				}

				pSipMessage->pContentType = pHdr;
				pHdr->eHdrType = dType;
			}
			else
			{
				pHdr = pSipMessage->pContentType;
			}
		}
	}
	else
	{
		if (pSipMessage->pOptionalHeaderList == NULL)
		{
			uError = EcrioSigMgrCreateOptionalHeaderList(pSigMgr, &pSipMessage->pOptionalHeaderList);
			if (uError != ECRIO_SIGMGR_NO_ERROR)
			{
				return uError;
			}
		}

		uError = EcrioSigMgrGetOptionalHeader(pSigMgr, pSipMessage->pOptionalHeaderList, dType, &pHdr);
		if (uError != ECRIO_SIGMGR_NO_ERROR)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderStruct), (void **)&pHdr);
			if (pHdr == NULL)
			{
				return ECRIO_SIGMGR_NO_MEMORY;
			}

			_EcrioSigMgrInsertOptionalHeader(pSigMgr, pSipMessage->pOptionalHeaderList, pHdr);
			pHdr->eHdrType = dType;
		}
	}

	/*
	* Skip leading LWS
	*/

	pCurrentPos = _EcrioSigMgrStripLeadingLWS(pCurrentPos, pEnd);

	/*
	* Skip trailing LWS
	*/

	pEnd = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pEnd);

	if (dType != EcrioSipHeaderTypeCallId && pHdr != NULL)
	{
		if (pHdr->ppHeaderValues == NULL)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderValueStruct *), (void **)&pHdr->ppHeaderValues);
			if (NULL == pHdr->ppHeaderValues)
			{
				return ECRIO_SIGMGR_NO_MEMORY;
			}
		}
		else
		{
			/* Check arithmetic overflow - do not use the ArithmeticOverflowDetected() API for u_int16 variable. */
			if (pHdr->numHeaderValues + 1 > USHRT_MAX)
			{
				return ECRIO_SIGMGR_NO_MEMORY;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((pHdr->numHeaderValues + 1), sizeof(EcrioSigMgrHeaderValueStruct *)) == Enum_TRUE)
			{
				return ECRIO_SIGMGR_NO_MEMORY;
			}

			pal_MemoryReallocate(sizeof(EcrioSigMgrHeaderValueStruct *) * (pHdr->numHeaderValues + 1),
				(void **)&pHdr->ppHeaderValues);
			if (NULL == pHdr->ppHeaderValues)
			{
				return ECRIO_SIGMGR_NO_MEMORY;
			}
		}

		pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderValueStruct), (void **)&pHdr->ppHeaderValues[pHdr->numHeaderValues]);
		if (NULL == pHdr->ppHeaderValues[pHdr->numHeaderValues])
		{
			uError = ECRIO_SIGMGR_NO_MEMORY;
			goto Error;
		}
	}

	/*
	* If the header is "Supported" or "Allow", allow empty header body.
	*/

	if (((0 == (pEnd - pCurrentPos + 1))) ||
		((0 == (pEnd - pCurrentPos + 1)) && (EcrioSipHeaderTypeAllow == dType || EcrioSipHeaderTypeSupported == dType)))
	{
		if (pHdr != NULL && pHdr->ppHeaderValues != NULL)
		{
			pHdr->ppHeaderValues[pHdr->numHeaderValues]->pHeaderValue = NULL;
		}
		else
		{
			uError = ECRIO_SIGMGR_NO_MEMORY;
			goto Error;
		}
	}
	else
	{
		if ((dType == EcrioSipHeaderTypeMinSE) || (dType == EcrioSipHeaderTypeExpires) || (dType == EcrioSipHeaderTypeMinExpires) \
			|| (dType == EcrioSipHeaderTypeSessionExpires) || (dType == EcrioSipHeaderTypeAcceptContact) || (dType == EcrioSipHeaderTypeSubscriptionState) \
			|| (dType == EcrioSipHeaderTypeContentType))
		{
			u_char *pToken = NULL;

			pToken = _EcrioSigMgrTokenize(pCurrentPos, pEnd, ';');
			/** Boundary checking */
			if (NULL != pToken && pToken != pEnd)
			{
				if (_EcrioSigMgrParseSipParam(pSigMgr, &numParams, &ppParams, pToken + 1, pEnd, Enum_FALSE, ';'))
				{
					uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
					goto Error;
				}

				pEnd = pToken - 1;										/* -mustfreefresh flag warning */
				pEnd = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pEnd);	/* -mustfreefresh flag warning */
			}
		}

		_EcrioSigMgrStringNCreate(pSigMgr, pCurrentPos,
			&pHeaderValue, (pEnd - pCurrentPos + 1));
		if (NULL == pHeaderValue)
		{
			uError = ECRIO_SIGMGR_NO_MEMORY;
			goto Error;
		}
	}

	if (dType == EcrioSipHeaderTypeCallId)
	{
		if (pSipMessage->pMandatoryHdrs->pCallId != NULL)
		{
			pal_MemoryFree((void **)&pSipMessage->pMandatoryHdrs->pCallId);
		}
		pSipMessage->pMandatoryHdrs->pCallId = (void *)pHeaderValue;
		pHeaderValue = NULL;
	}
	else if (pHdr != NULL)
	{
		pHdr->ppHeaderValues[pHdr->numHeaderValues]->pHeaderValue = pHeaderValue;
		pHeaderValue = NULL;

		if (ppParams != NULL)
		{
			pHdr->ppHeaderValues[pHdr->numHeaderValues]->numParams = numParams;
			pHdr->ppHeaderValues[pHdr->numHeaderValues]->ppParams = ppParams;
		}

		pHdr->numHeaderValues++;
	}

//	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
//		__FUNCTION__, __LINE__);
	return ECRIO_SIGMGR_NO_ERROR;

Error:
	if (ppParams != NULL)
	{
		u_int16 i = 0;
		for (i = 0; i < numParams; ++i)
		{
			_EcrioSigMgrReleaseParamsStruct(pSigMgr, ppParams[i]);
			pal_MemoryFree((void **)&(ppParams[i]));
		}
		pal_MemoryFree((void **)&ppParams);
	}

	if (NULL != pHdr && NULL != pHdr->ppHeaderValues && NULL != pHdr->ppHeaderValues[pHdr->numHeaderValues])
	{
		pal_MemoryFree((void **)&pHdr->ppHeaderValues[pHdr->numHeaderValues]);
	}

	return uError;
}

/****************************************************************************
** FUNCTION: _EcrioSigMgrParseAuthHeaders
** DESCRIPTION: This function parses the Authorization, Proxy-Authorization,
**				Authenticate and WWW-Authenticate Headers.
**
** PARAMETERS:
**				pStart(IN)		- The starting point in the input buffer
**								  from where this parser can start parsing.
**				pEnd(IN)		- The position in the input buffer till
**								  which this parser should parse.
**				hdr(IN)			- The particular header for which this
**								  parser is being invoked (required since
**								  multiple similar headers could be parsed
**								  by the same function).
**				pOutHdr(OUT)	- A void structure with the relevant
**								  headers filled.
**				pErr(OUT)		- Error value if any
**
** NOTE:
****************************************************************************/
u_int32	_EcrioSigMgrParseAuthHeaders
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSigMgrAuthenticationStruct **ppAuth
)
{
	u_char *pCurrentPos = pStart;
	u_char *pToken;
	u_char *pPrevToken;
	u_char *pTemp, *pSchemeEnd;
	u_int16	numParams = 0, i = 0;
	EcrioSigMgrParamStruct **ppParams = NULL;
	EcrioSigMgrAuthenticationStruct *pAuth = *ppAuth;
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;
	u_char *pUnquote = NULL;

	if (pAuth == NULL)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrAuthenticationStruct), (void **)&pAuth);
		if (NULL == pAuth)
		{
			uError = ECRIO_SIGMGR_NO_MEMORY;
			goto Error;
		}
	}

	/* Skip leading LWS */
	pCurrentPos = _EcrioSigMgrStripLeadingLWS(pCurrentPos, pEnd);

	/* Skip trailing LWS */
	pEnd = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pEnd);

	/* Now pCurrentPos points to a non-LWS character */

	pPrevToken = _EcrioSigMgrTokenize(pCurrentPos, pEnd, ',');

	pTemp = _EcrioSigMgrTokenize(pCurrentPos,
		(NULL == pPrevToken) ? pEnd : pPrevToken - 1, '=');
	if (NULL == pTemp)
	{
		/*
		* '=' is mandatory
		*/
		uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
		goto Error;
	}

	/*
	* Strip trailing LWS
	*/

	pTemp = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pTemp - 1);

	/*
	* Tokenize for SP
	*/

	pToken = _EcrioSigMgrTokenize(pCurrentPos, pTemp, ' ');
	if (NULL == pToken)
	{
		/*
		* Scheme is not present;
		* Raise parser error.
		*/
		uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
		goto Error;
	}

	/*
	* Store scheme after stripping trailing LWS.
	*/

	pSchemeEnd = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pToken - 1);

	_EcrioSigMgrStringNCreate(pSigMgr, pCurrentPos,
		&pAuth->pAuthenticationScheme, (pSchemeEnd - pCurrentPos + 1));
	if (pAuth->pAuthenticationScheme == NULL)
	{
		uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
		goto Error;
	}

	/*
	* Invoke param parser
	*/

	if (_EcrioSigMgrParseSipParam(pSigMgr, &numParams, &ppParams, pToken + 1, pEnd, \
		Enum_FALSE, ','))
	{
		uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
		goto Error;
	}

	for (i = 0; i < numParams; i++)
	{
		pUnquote = _EcrioSigMgrStringUnquote(ppParams[i]->pParamName);
		/* Realm param */
		if (pal_StringICompare(pUnquote,
			(u_char *)ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_REALM) == 0)
		{
			_EcrioSigMgrStringCreate(pSigMgr, ppParams[i]->pParamValue,
				&pAuth->pRealm);
		}
		/* Nonce param */
		else if (pal_StringICompare(pUnquote,
			(u_char *)ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_NONCE) == 0)
		{
			_EcrioSigMgrStringCreate(pSigMgr, ppParams[i]->pParamValue,
				&pAuth->pNonce);
		}
		/* Domain param */
		else if (pal_StringICompare(pUnquote,
			(u_char *)ECRIO_SIG_MGR_DOMAIN_PARAM) == 0)
		{
			_EcrioSigMgrStringCreate(pSigMgr, ppParams[i]->pParamValue,
				&pAuth->pDomain);
		}
		/* Opaque param */
		else if (pal_StringICompare(pUnquote,
			(u_char *)ECRIO_SIG_MGR_OPAQUE_PARAM) == 0)
		{
			if (ppParams[i]->pParamValue != NULL)
			{
				_EcrioSigMgrStringCreate(pSigMgr, ppParams[i]->pParamValue,
					&pAuth->pOpaque);
			}
			else
			{
				_EcrioSigMgrStringCreate(pSigMgr, (u_char *)"",
					&pAuth->pOpaque);
			}
		}
		/* Stale param */
		else if (pal_StringICompare(pUnquote,
			(u_char *)ECRIO_SIG_MGR_STALE_PARAM) == 0)
		{
			if (pal_StringICompare(ppParams[i]->pParamValue,
				(u_char *)"FALSE") == 0)
			{
				pAuth->bStale = Enum_FALSE;
			}
			else if (pal_StringICompare(ppParams[i]->pParamValue,
				(u_char *)"TRUE") == 0)
			{
				pAuth->bStale = Enum_TRUE;
			}
		}
		/* Support AKAv1 and AKAv2 using configuration */
		/* Algorithm param */
		else if (pal_StringICompare(pUnquote,
			(u_char *)ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_ALGORITHM_NAME) == 0)
		{
			u_char *pUnquoteVal = _EcrioSigMgrStringUnquote(ppParams[i]->pParamValue);
			if (pal_StringICompare(pUnquoteVal,
				(u_char *)ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_ALGORITHM_VALUE_AKAv1_MD5) == 0)
			{
				pAuth->authAlgo = EcrioSipAuthAlgorithmAKAv1;
			}
			else if (pal_StringICompare(pUnquoteVal,
				(u_char *)ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_ALGORITHM_VALUE_AKAv2_MD5) == 0)
			{
				pAuth->authAlgo = EcrioSipAuthAlgorithmAKAv2;
			}
			else if (pal_StringICompare(pUnquoteVal,
				(u_char *)ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_ALGORITHM_VALUE_MD5) == 0)
			{
				pAuth->authAlgo = EcrioSipAuthAlgorithmMD5;
			}
			else
			{
				pAuth->authAlgo = EcrioSipAuthAlgorithmUnknown;
			}
			pal_MemoryFree((void **)&pUnquoteVal);
		}
		/* Qop param */
		else if (pal_StringICompare(pUnquote,
			(u_char *)ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_QOP) == 0)
		{
			u_char *pLastIndex = NULL, *pIndex = NULL, *pTemp = NULL;
			u_int32 length = 0;

			if (pAuth->countQoP > 0 && pAuth->ppListOfQoP != NULL)
			{
				pal_MemoryReallocate(sizeof(u_char *) * (pAuth->countQoP + 1), (void **)&pAuth->ppListOfQoP);
				if (pAuth->ppListOfQoP == NULL)
				{
					uError = ECRIO_SIGMGR_NO_MEMORY;
					goto Error;
				}
				pAuth->countQoP++;
			}
			else
			{
				pal_MemoryAllocate(sizeof(u_char *) * 1, (void **)&pAuth->ppListOfQoP);
				if (NULL == pAuth->ppListOfQoP)
				{
					uError = ECRIO_SIGMGR_NO_MEMORY;
					goto Error;
				}
				pAuth->countQoP = 1;
			}

			pLastIndex = _EcrioSigMgrStringUnquote(ppParams[i]->pParamValue);
			pTemp = pLastIndex;
			do
			{
				u_char *pQop = NULL;

				pIndex = pal_SubString(pLastIndex, (u_char *)ECRIO_SIG_MGR_SIP_COMMA_SYMBOL);
				if (pIndex == NULL)
				{
					length = pal_StringLength(pLastIndex);
				}
				else
				{
					length = (u_int32)(pIndex - pLastIndex);
				}

				pal_MemoryAllocate(length + 1, (void **)&pAuth->ppListOfQoP[pAuth->countQoP - 1]);
				if (NULL == pAuth->ppListOfQoP[pAuth->countQoP - 1])
				{
					uError = ECRIO_SIGMGR_NO_MEMORY;
					goto Error;
				}

				pQop = pAuth->ppListOfQoP[pAuth->countQoP - 1];
				pal_MemoryCopy(pQop, length, pLastIndex, length);
				pQop[length] = 0x00;

				if (pIndex != NULL)
				{
					/* Check arithmetic overflow - do not use the ArithmeticOverflowDetected() API for u_int16 variable. */
					if (pAuth->countQoP + 1 > USHRT_MAX)
					{
						uError = ECRIO_SIGMGR_NO_MEMORY;
						goto Error;
					}

					/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
					if (pal_UtilityDataOverflowDetected((pAuth->countQoP + 1), sizeof(u_char *)) == Enum_TRUE)
					{
						uError = ECRIO_SIGMGR_NO_MEMORY;
						goto Error;
					}

					pal_MemoryReallocate(sizeof(u_char *) * (pAuth->countQoP + 1), (void **)&pAuth->ppListOfQoP);
					if (pAuth->ppListOfQoP == NULL)
					{
						uError = ECRIO_SIGMGR_NO_MEMORY;
						goto Error;
					}

					++(pAuth->countQoP);
					pLastIndex = pIndex + 1;					
				}
			} while (pIndex != NULL);
			pal_MemoryFree((void **)&pTemp);
		}
		if (pUnquote != NULL)
		{
			pal_MemoryFree((void **)&pUnquote);
		}
	}

	*ppAuth = pAuth;
	goto End;
Error:
	if (*ppAuth == NULL && pAuth != NULL)
	{
		u_int32 index = 0;
		if (pAuth->pAuthenticationScheme != NULL)
		{
			pal_MemoryFree((void**)&pAuth->pAuthenticationScheme);
		}
		if (pAuth->pDomain != NULL)
		{
			pal_MemoryFree((void**)&pAuth->pDomain);
		}
		if (pAuth->pNonce != NULL)
		{
			pal_MemoryFree((void**)&pAuth->pNonce);
		}
		if (pAuth->pRealm != NULL)
		{
			pal_MemoryFree((void**)&pAuth->pRealm);
		}
		if (pAuth->pOpaque != NULL)
		{
			pal_MemoryFree((void**)&pAuth->pOpaque);
		}
		
		if (pAuth->ppListOfQoP != NULL)
		{
			for (index = 0; index < pAuth->countQoP; index++)
			{
				if (pAuth->ppListOfQoP[index] != NULL)
				{
					pal_MemoryFree((void**)&pAuth->ppListOfQoP[index]);
				}
			}
			pal_MemoryFree((void**)&pAuth->ppListOfQoP);
		}
		pal_MemoryFree((void**)&pAuth);
	}
End:
	for (i = 0; i < numParams; i++)
	{
		if (NULL != ppParams[i]->pParamName)
		{
			pal_MemoryFree((void **)&ppParams[i]->pParamName);
		}
		if (NULL != ppParams[i]->pParamValue)
		{
			pal_MemoryFree((void **)&ppParams[i]->pParamValue);
		}
		if (NULL != ppParams[i])
		{
			pal_MemoryFree((void **)&ppParams[i]);
		}
	}

	if (NULL != ppParams)
	{
		pal_MemoryFree((void **)&ppParams);
	}

	return uError;
}

/****************************************************************************
** FUNCTION: _EcrioSigMgrParseContentLengthHeader
** DESCRIPTION: This function parses the Content-Length, RSeq, Max-Forwards,
**				Min-SE and Session-Expires headers.
**
** PARAMETERS:
**				pStart(IN)		- The starting point in the input buffer
**								  from where this parser can start parsing.
**				pEnd(IN)		- The position in the input buffer till
**								  which this parser should parse.
**				hdr(IN)			- The particular header for which this
**								  parser is being invoked (required since
**								  multiple similar headers could be parsed
**								  by the same function).
**				pOutHdr(OUT)	- A void structure with the relevant
**								  headers filled.
**				pErr(OUT)		- Error value if any
**
** NOTE:
****************************************************************************/
u_int32	_EcrioSigMgrParseContentLengthHeader
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	void *pOutHdr,
	EcrioSipHeaderTypeEnum dType
)
{
	u_char *pCurrentPos = pStart;
	EcrioSigMgrSipMessageStruct *pSipMessage = NULL;

	(void)pSigMgr;

	pSipMessage = (EcrioSigMgrSipMessageStruct *)(pOutHdr);

	/*
	* Skip leading and trailing LWS
	*/

	pCurrentPos = _EcrioSigMgrStripLeadingLWS(pCurrentPos, pEnd);
	pEnd = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pEnd);

	/*
	* This parser is used to parse Content-Length, RSeq, Max-Forwards,
	* Min-SE and Session-Expires headers. Of these, the Min-SE and
	* Session-Expires headers can have parameters in the header
	* body. Thus, we need to look for params for these two types of
	* headers.
	*/
	if (pCurrentPos > pEnd)
	{
		/*
		* Length field not present
		*/

		return ECRIO_SIGMGR_IMS_LIB_ERROR;
	}

	switch (dType)
	{
		case EcrioSipHeaderTypeMinSE:
		{
			if (pSipMessage->pMinExpires == NULL)
			{
				pal_MemoryAllocate(sizeof(u_int32), (void**)&pSipMessage->pMinExpires);
				if (NULL == pSipMessage->pMinExpires)
				{
					return ECRIO_SIGMGR_NO_MEMORY;
				}
			}

			*(pSipMessage->pMinExpires) = pal_StringConvertToUNum(pCurrentPos, &pEnd, 10);
		}
		break;

		case EcrioSipHeaderTypeMinExpires:
		{
			if (pSipMessage->pMinExpires == NULL)
			{
				pal_MemoryAllocate(sizeof(u_int32), (void**)&pSipMessage->pMinExpires);
				if (NULL == pSipMessage->pMinExpires)
				{
					return ECRIO_SIGMGR_NO_MEMORY;
				}
			}

			*(pSipMessage->pMinExpires) = pal_StringConvertToUNum(pCurrentPos, &pEnd, 10);
		}
		break;

		case EcrioSipHeaderTypeExpires:
		{
			if (pSipMessage->pExpires == NULL)
			{
				pal_MemoryAllocate(sizeof(u_int32), (void**)&pSipMessage->pExpires);
				if (NULL == pSipMessage->pExpires)
				{
					return ECRIO_SIGMGR_NO_MEMORY;
				}
			}

			*(pSipMessage->pExpires) = pal_StringConvertToUNum(pCurrentPos, &pEnd, 10);
		}
		break;

		case EcrioSipHeaderTypeContentLength:
		{
			pSipMessage->contentLength = pal_StringConvertToUNum(pCurrentPos, &pEnd, 10);
		}
		break;

		case EcrioSipHeaderTypeMaxForward:
		{
			pSipMessage->pMandatoryHdrs->maxForwards = pal_StringConvertToUNum(pCurrentPos, &pEnd, 10);
		}
		break;

		default:
		{
		}
	}

	/*
	* Validate and store the length/resp-num field
	*/

	return ECRIO_SIGMGR_NO_ERROR;
}	/* end of function _EcrioSigMgrParseContentLengthHeader */

/****************************************************************************
** FUNCTION: _EcrioSigMgrParseViaHeader
** DESCRIPTION: This function parses the Via Header.
**
** PARAMETERS:
**				pStart(IN)		- The starting point in the input buffer
**								  from where this parser can start parsing.
**				pEnd(IN)		- The position in the input buffer till
**								  which this parser should parse.
**				hdr(IN)			- The particular header for which this
**								  parser is being invoked (required since
**								  multiple similar headers could be parsed
**								  by the same function).
**				pOutHdr(OUT)	- A void structure with the relevant
**								  headers filled.
**				pErr(OUT)		- Error value if any
**
** NOTE:
****************************************************************************/
u_int32	_EcrioSigMgrParseViaHeader
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	void *pOutHdr
)
{
	u_char *pCurrentPos = pStart;
	u_char *pEndOfSendProtocol = NULL;
	u_char *pTmp = NULL;
	u_char *pTmpEnd = NULL;
	u_char *pSentProto = NULL;
	EcrioSigMgrViaStruct *pViaHdr = NULL;
	u_char *pSlash = NULL;
	u_char *pSlash2 = NULL;
	u_char *pColon = NULL;
	u_int32 Length = 0;
	u_int32 maxLength = 0;
	u_int32 i = 0;

	pViaHdr = (EcrioSigMgrViaStruct *)(pOutHdr);

	/* setting the port to invalid so that if we receive domain name then we need not include the port */
	pViaHdr->pIPaddr.port = 0xFFFF;

	pCurrentPos = _EcrioSigMgrStripLeadingLWS(pCurrentPos, pEnd);
	pEnd = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pEnd);
	pTmpEnd = pEnd;

	pTmp = pEnd;

	maxLength = (u_int32)(pEnd-pStart);

	/*
	* Validate the sent-protocol field.
	* Currently the only validation being done is checking that the
	* sent-proto field has indeed 3 components as required.
	*/

	pSlash = _EcrioSigMgrTokenize(pCurrentPos, pTmp - 1, '/');
	if ((NULL == pSlash) || ((pTmp - 1) == pSlash))
	{
		return ECRIO_SIGMGR_IMS_LIB_ERROR;
	}

	pTmpEnd = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pSlash - 1);
	Length = pTmpEnd - pCurrentPos + 2;
	pSlash2 = _EcrioSigMgrTokenize(pSlash + 1, pTmp - 1, '/');
	if ((NULL == pSlash2) || ((pTmp - 1) == pSlash2))
	{
		return ECRIO_SIGMGR_IMS_LIB_ERROR;
	}

	pTmpEnd = _EcrioSigMgrStripTrailingLWS(pSlash + 1, pSlash2 - 1);
	pSentProto = _EcrioSigMgrStripLeadingLWS(pSlash + 1, pSlash2 - 1);

	_EcrioSigMgrStringNCreate(pSigMgr, pCurrentPos,
		&pViaHdr->pSipVersion, (pTmpEnd - pCurrentPos + 1));
	if (NULL == pViaHdr->pSipVersion)
	{
		return ECRIO_SIGMGR_NO_MEMORY;
	}

	Length += pTmpEnd - pSentProto + 2;
	pSentProto = _EcrioSigMgrStripLeadingLWS(pSlash2 + 1, pTmp - 1);
	pTmpEnd = _EcrioSigMgrTokenize(pSentProto, pTmp - 1, ' ');
	if ((NULL == pTmpEnd) || ((pTmp - 1) == pTmpEnd))
	{
		return ECRIO_SIGMGR_IMS_LIB_ERROR;
	}

	Length = pTmpEnd - pSentProto;

	if (pal_StringNICompare(pSentProto, (u_char *)ECRIO_SIG_MGR_SIP_UDP_PARAM, Length) == 0)
	{
		pViaHdr->transport = EcrioSigMgrTransportUDP;
	}
	else if (pal_StringNICompare(pSentProto, (u_char *)ECRIO_SIG_MGR_SIP_TCP_PARAM, Length) == 0)
	{
		pViaHdr->transport = EcrioSigMgrTransportTCP;
	}
	else if (pal_StringNICompare(pSentProto, (u_char *)ECRIO_SIG_MGR_SIP_TLS_PARAM, Length) == 0)
	{
		pViaHdr->transport = EcrioSigMgrTransportTLS;
	}
	else
	{
		return ECRIO_SIGMGR_TRANSPORT_UNSUPPORTED;
	}

	Length = 0;
	pEndOfSendProtocol = pTmpEnd + 1;
	pCurrentPos = _EcrioSigMgrTokenize(pEndOfSendProtocol, pTmp - 1, ';');	/* -mustfreefresh flag warning */
	if (NULL == pCurrentPos)
	{
		pEndOfSendProtocol = _EcrioSigMgrStripLeadingLWS(pEndOfSendProtocol, pTmp);
		pTmp = _EcrioSigMgrStripTrailingLWS(pEndOfSendProtocol, pTmp);
		Length = (pTmp - pEndOfSendProtocol) + 1;
		pColon = _EcrioSigMgrTokenize(pEndOfSendProtocol, pTmp - 1, ':');
		if (NULL != pColon)
		{
			pTmpEnd = _EcrioSigMgrStripTrailingLWS(pEndOfSendProtocol, pColon - 1);
			Length = (pTmpEnd - pEndOfSendProtocol) + 2;
			pTmpEnd = _EcrioSigMgrStripLeadingLWS(pColon + 1, pTmp);
			Length += ((pTmp - pTmpEnd) + 1);
			_EcrioSigMgrExtractHostPort(pEndOfSendProtocol, pTmp, &pViaHdr->pIPaddr.pIPAddr, &pViaHdr->pIPaddr.port);
		}
		else
		{
			if(Length <= maxLength && Length > 0) {
				pal_MemoryAllocate(Length*sizeof(u_char), (void **) &pViaHdr->pIPaddr.pIPAddr);
				if (NULL == pViaHdr->pIPaddr.pIPAddr) {
					return ECRIO_SIGMGR_NO_MEMORY;    /* -mustfreefresh flag warning */
				}

				if (NULL == pal_StringNCopy(pViaHdr->pIPaddr.pIPAddr, Length, pEndOfSendProtocol,
											Length - 1)) {
					pal_MemoryFree((void **) &pViaHdr->pIPaddr.pIPAddr);
					return ECRIO_SIGMGR_STRING_ERROR;
				}
				pViaHdr->pIPaddr.pIPAddr[Length - 1] = 0x00;
			} else{
				return ECRIO_SIGMGR_NO_MEMORY;
			}
		}

		if (NULL == pViaHdr->pIPaddr.pIPAddr)
		{
			return ECRIO_SIGMGR_IMS_LIB_ERROR;	/* -mustfreefresh flag warning */
		}
	}
	else
	{
		/*
		* Store Send by.
		* Check whether the next character after ';' is not a
		* space cr or lf. Store  the via params.
		*/

		/* Moving one char backward not to include ; */
		pCurrentPos--;				/* -mustfreefresh flag warning */
		pSentProto = pCurrentPos;	/* -mustfreefresh flag warning */
		pCurrentPos = _EcrioSigMgrStripTrailingLWS(pEndOfSendProtocol, pCurrentPos);
		pEndOfSendProtocol = _EcrioSigMgrStripLeadingLWS(pEndOfSendProtocol, pCurrentPos);	/* -mustfreefresh flag warning */
		Length = pCurrentPos - pEndOfSendProtocol + 1;
		pColon = _EcrioSigMgrTokenize(pEndOfSendProtocol, pCurrentPos - 1, ':');
		if (NULL != pColon)
		{
			pTmpEnd = _EcrioSigMgrStripTrailingLWS(pEndOfSendProtocol, pColon - 1);
			Length = pTmpEnd - pEndOfSendProtocol + 2;
			pTmpEnd = _EcrioSigMgrStripLeadingLWS(pColon + 1, pCurrentPos);
			Length += pCurrentPos - pTmpEnd + 1;
			_EcrioSigMgrExtractHostPort(pEndOfSendProtocol, pTmp, &pViaHdr->pIPaddr.pIPAddr, &pViaHdr->pIPaddr.port);
		}
		else
		{
            if(Length <= maxLength && Length > 0) {
                pal_MemoryAllocate(Length*sizeof(u_char), (void **) &pViaHdr->pIPaddr.pIPAddr);
                if (NULL == pViaHdr->pIPaddr.pIPAddr) {
                    return ECRIO_SIGMGR_NO_MEMORY;    /* -mustfreefresh flag warning */
                }

                if (NULL == pal_StringNCopy(pViaHdr->pIPaddr.pIPAddr, Length, pEndOfSendProtocol,
                                            Length - 1)) {
                    pal_MemoryFree((void **) &pViaHdr->pIPaddr.pIPAddr);
                    return ECRIO_SIGMGR_STRING_ERROR;
                }
                pViaHdr->pIPaddr.pIPAddr[Length - 1] = 0x00;
            }
            else{
                return ECRIO_SIGMGR_NO_MEMORY;
            }
		}

		if (NULL == pViaHdr->pIPaddr.pIPAddr)
		{
			return ECRIO_SIGMGR_IMS_LIB_ERROR;
		}

		if (pViaHdr->pIPaddr.pIPAddr == NULL)
		{
			return ECRIO_SIGMGR_IMS_LIB_ERROR;
		}

		pCurrentPos = pSentProto;
		pCurrentPos = pCurrentPos + 2;
		pCurrentPos = _EcrioSigMgrStripLeadingLWS(pCurrentPos, pTmp);
		if ((*pCurrentPos == '\r') || (*pCurrentPos == '\n'))
		{
			return ECRIO_SIGMGR_IMS_LIB_ERROR;
		}
	}

	if (NULL != pCurrentPos)
	{
		u_int32 iterator = 0;
		/*pSentProto = pCurrentPos;

		pEndOfSendProtocol = _EcrioSigMgrTokenize(pCurrentPos, pTmp - 1, ';');
		if (pEndOfSendProtocol)
		{
		    pCurrentPos = _EcrioSigMgrStripLeadingLWS(pCurrentPos, pEndOfSendProtocol);
		    pTmpEnd = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pEndOfSendProtocol);

		    pColon = _EcrioSigMgrTokenize(pCurrentPos, pTmpEnd - 1, '=');
		    if (pColon)
		    {
		        pCurrentPos = _EcrioSigMgrStripLeadingLWS(pColon + 1, pTmpEnd);
		        pTmpEnd = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pTmpEnd - 1);

		        _EcrioSigMgrStringNCreate(pSigMgr, pCurrentPos,
		            &pViaHdr->pBranch, (pTmpEnd - pCurrentPos + 1));
		        if (NULL == pViaHdr->pSipVersion)
		        {
		            return ECRIO_SIGMGR_NO_MEMORY;
		        }
		    }

		    pCurrentPos = pEndOfSendProtocol + 1;
		}*/

		if (_EcrioSigMgrParseSipParam(pSigMgr, &pViaHdr->numParams,
			&pViaHdr->ppParams, pCurrentPos, pTmp, Enum_FALSE, ';'))
		{
			return ECRIO_SIGMGR_IMS_LIB_ERROR;
		}

		for (iterator = 0; iterator < pViaHdr->numParams; iterator++)
		{
			if (pal_StringICompare(pViaHdr->ppParams[iterator]->pParamName, (u_char *)"branch") == 0)
			{
				pViaHdr->pBranch = pViaHdr->ppParams[iterator]->pParamValue;
				pViaHdr->ppParams[iterator]->pParamValue = NULL;
				pal_MemoryFree((void **)&pViaHdr->ppParams[iterator]->pParamName);
				pViaHdr->ppParams[iterator]->pParamName = NULL;
				pal_MemoryFree((void **)&pViaHdr->ppParams[iterator]);

				for (i = iterator + 1; i < pViaHdr->numParams; i++)
				{
					pViaHdr->ppParams[i - 1] = pViaHdr->ppParams[i];
				}

				pViaHdr->numParams--;
				if (pViaHdr->numParams)
				{
					/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
					if (pal_UtilityDataOverflowDetected(pViaHdr->numParams, sizeof(EcrioSigMgrParamStruct *)) == Enum_TRUE)
					{
						return ECRIO_SIGMGR_NO_MEMORY;
					}

					pal_MemoryReallocate((pViaHdr->numParams) * sizeof(EcrioSigMgrParamStruct *), (void **)&pViaHdr->ppParams);
					if (NULL == pViaHdr->ppParams)
					{
						return ECRIO_SIGMGR_NO_MEMORY;
					}
				}
				else
				{
					pal_MemoryFree((void **)&pViaHdr->ppParams);
				}

				break;
			}
		}
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

/****************************************************************************
** FUNCTION: _EcrioSigMgrParseContactHeader
** DESCRIPTION: This function parses the Contact  Header.
**
** PARAMETERS:
**				pStart(IN)		- The starting point in the input buffer
**								  from where this parser can start parsing.
**				pEnd(IN)		- The position in the input buffer till
**								  which this parser should parse.
**				pOutHdr(OUT)	- A void structure with the relevant
**								  headers filled.
**				pErr(OUT)		- Error value if any
**
** NOTE:
****************************************************************************/
u_int32	_EcrioSigMgrParseContactHeader
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSigMgrContactStruct **ppContactHeader
)
{
	u_char *pCurrentPos = NULL;
	EcrioSigMgrContactStruct *pContactHeader = *ppContactHeader;
	EcrioSigMgrNameAddrWithParamsStruct *pNameAddrWithParams = NULL;
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;

	pCurrentPos = _EcrioSigMgrStripLeadingLWS(pStart, pEnd);
	pEnd = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pEnd);

	if (pContactHeader == NULL)
	{
		if (pal_MemoryAllocate(sizeof(EcrioSigMgrContactStruct), (void **)&pContactHeader) != KPALErrorNone)
		{
			return ECRIO_SIGMGR_NO_MEMORY;
		}

		*ppContactHeader = pContactHeader;
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct), (void **)&pNameAddrWithParams);
	if (NULL == pNameAddrWithParams)
	{
		return ECRIO_SIGMGR_NO_MEMORY;
	}

	if (_EcrioSigMgrParseNameAddrWithParams(pSigMgr, pCurrentPos, pEnd, pNameAddrWithParams))
	{
		uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
		goto Error;
	}
	else
	{
		if (pContactHeader->ppContactDetails == NULL)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct *), (void **)&pContactHeader->ppContactDetails);
			if (NULL == pContactHeader->ppContactDetails)
			{
				uError = ECRIO_SIGMGR_NO_MEMORY;
				goto Error;
			}
		}
		else
		{
			/* Check arithmetic overflow - do not use the ArithmeticOverflowDetected() API for u_int16 variable. */
			if (pContactHeader->numContactUris + 1 > USHRT_MAX)
			{
				uError = ECRIO_SIGMGR_NO_MEMORY;
				goto Error;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((pContactHeader->numContactUris + 1), sizeof(EcrioSigMgrNameAddrWithParamsStruct *)) == Enum_TRUE)
			{
				uError = ECRIO_SIGMGR_NO_MEMORY;
				goto Error;
			}

			pal_MemoryReallocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct *) * (pContactHeader->numContactUris + 1), (void **)&pContactHeader->ppContactDetails);
			if (NULL == pContactHeader->ppContactDetails)
			{
				uError = ECRIO_SIGMGR_NO_MEMORY;
				goto Error;
			}
		}

		pContactHeader->ppContactDetails[pContactHeader->numContactUris] = pNameAddrWithParams;
		pContactHeader->numContactUris += 1;
	}

	return ECRIO_SIGMGR_NO_ERROR;

Error:
	if (pNameAddrWithParams != NULL)
	{
		_EcrioSigMgrReleaseNameAddrWithParamsStruct(pSigMgr, pNameAddrWithParams);
		pal_MemoryFree((void **)&pNameAddrWithParams);
	}

	return uError;
}

u_int32	_EcrioSigMgrParseRouteHeader
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSigMgrRouteStruct **ppRouteSet
)
{
	u_char *pCurrentPos = NULL;
	EcrioSigMgrRouteStruct *pRouteSet = *ppRouteSet;
	EcrioSigMgrNameAddrWithParamsStruct *pNameAddrWithParams = NULL;
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;

	pCurrentPos = _EcrioSigMgrStripLeadingLWS(pStart, pEnd);
	pEnd = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pEnd);

	if (pRouteSet == NULL)
	{
		if (pal_MemoryAllocate(sizeof(EcrioSigMgrContactStruct), (void **)&pRouteSet) != KPALErrorNone)
		{
			return ECRIO_SIGMGR_NO_MEMORY;
		}
		*ppRouteSet = pRouteSet;
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct), (void **)&pNameAddrWithParams);
	if (NULL == pNameAddrWithParams)
	{
		return ECRIO_SIGMGR_NO_MEMORY;
	}

	if (_EcrioSigMgrParseNameAddrWithParams(pSigMgr, pCurrentPos, pEnd, pNameAddrWithParams))
	{
		uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
		goto Error;
	}
	else
	{
		if (pRouteSet->ppRouteDetails == NULL)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct *), (void **)&pRouteSet->ppRouteDetails);
			if (NULL == pRouteSet->ppRouteDetails)
			{
				uError = ECRIO_SIGMGR_NO_MEMORY;
				goto Error;
			}
		}
		else
		{
			/* Check arithmetic overflow - do not use the ArithmeticOverflowDetected() API for u_int16 variable. */
			if (pRouteSet->numRoutes + 1 > USHRT_MAX)
			{
				uError = ECRIO_SIGMGR_NO_MEMORY;
				goto Error;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((pRouteSet->numRoutes + 1), sizeof(EcrioSigMgrNameAddrWithParamsStruct *)) == Enum_TRUE)
			{
				uError = ECRIO_SIGMGR_NO_MEMORY;
				goto Error;
			}

			pal_MemoryReallocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct *) * (pRouteSet->numRoutes + 1), (void **)&pRouteSet->ppRouteDetails);
			if (NULL == pRouteSet->ppRouteDetails)
			{
				uError = ECRIO_SIGMGR_NO_MEMORY;
				goto Error;
			}
		}

		pRouteSet->ppRouteDetails[pRouteSet->numRoutes] = pNameAddrWithParams;
		pRouteSet->numRoutes += 1;
	}

	return ECRIO_SIGMGR_NO_ERROR;

Error:
	if (pNameAddrWithParams != NULL)
	{
		_EcrioSigMgrReleaseNameAddrWithParamsStruct(pSigMgr, pNameAddrWithParams);
		pal_MemoryFree((void **)&pNameAddrWithParams);
	}

	return uError;
}

u_int32	_EcrioSigMgrParseReasonHeader
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSigMgrSipMessageStruct *pSipMessage
)
{
	u_int16 numParams = 0;
	u_int32 i = 0;
	EcrioSigMgrParamStruct **ppParams = NULL;

	if (_EcrioSigMgrParseSipParam(pSigMgr, &numParams, &ppParams, pStart, pEnd, Enum_FALSE, ';'))
	{
		return ECRIO_SIGMGR_IMS_LIB_ERROR;
	}

	for (i = 0; i < numParams; i++)
	{
		if (pal_StringICompare(ppParams[i]->pParamName, (u_char *)"cause") == 0)
		{
			pSipMessage->causeCode = pal_StringToNum(ppParams[i]->pParamValue, NULL);
		}
		else if (pal_StringICompare(ppParams[i]->pParamName, (u_char *)"text") == 0)
		{
			_EcrioSigMgrStringCreate(pSigMgr, ppParams[i]->pParamValue, &pSipMessage->pReasonPhrase);
		}
	}

	for (i = 0; i < numParams; i++)
	{
		pal_MemoryFree((void **)&ppParams[i]->pParamName);
		pal_MemoryFree((void **)&ppParams[i]->pParamValue);
		pal_MemoryFree((void **)&ppParams[i]);
	}

	pal_MemoryFree((void **)&ppParams);

	return ECRIO_SIGMGR_NO_ERROR;
}

/****************************************************************************
** FUNCTION: _EcrioSigMgrParseAuthHeaders
** DESCRIPTION: This function parses the Authorization, Proxy-Authorization,
**				Authenticate and WWW-Authenticate Headers.
**
** PARAMETERS:
**				pStart(IN)		- The starting point in the input buffer
**								  from where this parser can start parsing.
**				pEnd(IN)		- The position in the input buffer till
**								  which this parser should parse.
**				hdr(IN)			- The particular header for which this
**								  parser is being invoked (required since
**								  multiple similar headers could be parsed
**								  by the same function).
**				pOutHdr(OUT)	- A void structure with the relevant
**								  headers filled.
**				pErr(OUT)		- Error value if any
**
** NOTE:
****************************************************************************/
u_int32	_EcrioSigMgrParseAuthenticationInfoHeaders
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSigMgrHeaderStruct **ppAuthenticationInfo
)
{
	u_char *pCurrentPos = pStart;
	EcrioSigMgrHeaderStruct *pAuth = *ppAuthenticationInfo;
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;

	if (pAuth == NULL)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderStruct), (void **)&pAuth);
		if (NULL == pAuth)
		{
			return ECRIO_SIGMGR_NO_MEMORY;
		}

		*ppAuthenticationInfo = pAuth;
	}

	/* Skip leading LWS */
	pCurrentPos = _EcrioSigMgrStripLeadingLWS(pCurrentPos, pEnd);

	/* Skip trailing LWS */
	pEnd = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pEnd);

	/* Now pCurrentPos points to a non-LWS character */

	if (pAuth->ppHeaderValues == NULL)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderValueStruct *), (void **)&pAuth->ppHeaderValues);
		if (NULL == pAuth->ppHeaderValues)
		{
			return ECRIO_SIGMGR_NO_MEMORY;
		}
	}
	else
	{
		/* Check arithmetic overflow - do not use the ArithmeticOverflowDetected() API for u_int16 variable. */
		if (pAuth->numHeaderValues + 1 > USHRT_MAX)
		{
			return ECRIO_SIGMGR_NO_MEMORY;
		}

		/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
		if (pal_UtilityDataOverflowDetected((pAuth->numHeaderValues + 1), sizeof(EcrioSigMgrHeaderValueStruct *)) == Enum_TRUE)
		{
			return ECRIO_SIGMGR_NO_MEMORY;
		}

		pal_MemoryReallocate(sizeof(EcrioSigMgrHeaderValueStruct *) * (pAuth->numHeaderValues + 1), (void **)&pAuth->ppHeaderValues);
		if (NULL == pAuth->ppHeaderValues)
		{
			return ECRIO_SIGMGR_NO_MEMORY;
		}
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderValueStruct), (void **)&pAuth->ppHeaderValues[pAuth->numHeaderValues]);
	if (NULL == pAuth->ppHeaderValues[pAuth->numHeaderValues])
	{
		return ECRIO_SIGMGR_NO_MEMORY;
	}

	pAuth->ppHeaderValues[pAuth->numHeaderValues]->pHeaderValue = NULL;

	/*
	* Invoke param parser
	*/

	if (_EcrioSigMgrParseSipParam(pSigMgr, &pAuth->ppHeaderValues[pAuth->numHeaderValues]->numParams, \
		&pAuth->ppHeaderValues[pAuth->numHeaderValues]->ppParams, pCurrentPos, pEnd, \
		Enum_FALSE, ','))
	{
		uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
		goto Error;
	}

	pAuth->numHeaderValues++;

	return ECRIO_SIGMGR_NO_ERROR;

Error:
	if (pAuth->ppHeaderValues[pAuth->numHeaderValues] != NULL)
	{
		pal_MemoryFree((void **)&pAuth->ppHeaderValues[pAuth->numHeaderValues]);
	}

	return uError;
}


u_int32	_EcrioSigMgrParseOptionalHeaders
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSigMgrHeaderStruct **ppHeader
)
{
	u_char *pCurrentPos = pStart;
	EcrioSigMgrHeaderStruct *pHeader = *ppHeader;
	u_char *pToken = NULL;
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;

	if (pHeader == NULL)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderStruct), (void **)&pHeader);
		if (NULL == pHeader)
		{
			return ECRIO_SIGMGR_NO_MEMORY;
		}

		*ppHeader = pHeader;
	}

	/* Skip leading LWS */
	pCurrentPos = _EcrioSigMgrStripLeadingLWS(pCurrentPos, pEnd);

	/* Skip trailing LWS */
	pEnd = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pEnd);

	/* Now pCurrentPos points to a non-LWS character */

	if (pHeader->ppHeaderValues == NULL)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderValueStruct *), (void **)&pHeader->ppHeaderValues);
		if (NULL == pHeader->ppHeaderValues)
		{
			return ECRIO_SIGMGR_NO_MEMORY;
		}
	}
	else
	{
		/* Check arithmetic overflow - do not use the ArithmeticOverflowDetected() API for u_int16 variable. */
		if (pHeader->numHeaderValues + 1 > USHRT_MAX)
		{
			return ECRIO_SIGMGR_NO_MEMORY;
		}

		/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
		if (pal_UtilityDataOverflowDetected((pHeader->numHeaderValues + 1), sizeof(EcrioSigMgrHeaderValueStruct *)) == Enum_TRUE)
		{
			return ECRIO_SIGMGR_NO_MEMORY;
		}

		pal_MemoryReallocate(sizeof(EcrioSigMgrHeaderValueStruct *) * (pHeader->numHeaderValues + 1), (void **)&pHeader->ppHeaderValues);
		if (NULL == pHeader->ppHeaderValues)
		{
			return ECRIO_SIGMGR_NO_MEMORY;
		}
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderValueStruct), (void **)&pHeader->ppHeaderValues[pHeader->numHeaderValues]);
	if (NULL == pHeader->ppHeaderValues[pHeader->numHeaderValues])
	{
		return ECRIO_SIGMGR_NO_MEMORY;
	}

	pHeader->ppHeaderValues[pHeader->numHeaderValues]->pHeaderValue = NULL;

	/* Tokenize for ; */
	pToken = _EcrioSigMgrTokenizeQuotedString(pCurrentPos, pEnd, ';');
	if (NULL == pToken)
	{
		/*
		* Header value is not present
		*/
		pHeader->ppHeaderValues[pHeader->numHeaderValues]->pHeaderValue = NULL;
	}
	/** Boundary checking; If ';' is last char in string, this is invalid SIP header/param scheme. */
	else if (pToken == pEnd)
	{
		uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
		goto Error;
	}
	else
	{
		/*
		* header value may or may not be present. If display name
		* is of non-zero length, store it.
		*/
		u_char *pTempLWS;
		u_int32 size = 0;

		pTempLWS = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pToken - 1);
		size = (u_int32)(pTempLWS - pCurrentPos + 1);
		if (size > 0)
		{
			/*
			* header value is present
			*/
			_EcrioSigMgrStringNCreate(pSigMgr, pCurrentPos, &pHeader->ppHeaderValues[pHeader->numHeaderValues]->pHeaderValue, size);
		}
		else
		{
			pHeader->ppHeaderValues[pHeader->numHeaderValues]->pHeaderValue = NULL;
		}
		/*
		* Advance pCurrentPos
		*/
		pCurrentPos = pToken + 1;
	}

	/*
	* Invoke param parser
	*/

	if (_EcrioSigMgrParseSipParam(pSigMgr, &pHeader->ppHeaderValues[pHeader->numHeaderValues]->numParams, \
		&pHeader->ppHeaderValues[pHeader->numHeaderValues]->ppParams, pCurrentPos, pEnd, \
		Enum_TRUE, ';'))
	{
		uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
		goto Error;
	}

	pHeader->numHeaderValues++;

	return ECRIO_SIGMGR_NO_ERROR;

Error:
	if (pHeader != NULL)
	{
		if (pHeader->ppHeaderValues != NULL)
		{
			for (u_int16 i = 0; i < (pHeader->numHeaderValues + 1); i++)
			{
				if (pHeader->ppHeaderValues[i] != NULL)
				{
					if (pHeader->ppHeaderValues[i]->pHeaderValue != NULL)
					{
						pal_MemoryFree((void **)&pHeader->ppHeaderValues[i]->pHeaderValue);
					}
					pal_MemoryFree((void **)&pHeader->ppHeaderValues[i]);
				}
			}
			pal_MemoryFree((void **)&pHeader->ppHeaderValues);
		}
		pal_MemoryFree((void **)&pHeader);
	}

	return uError;
}

/****************************************************************************
** FUNCTION: _EcrioSigMgrParseWarningHeader
** DESCRIPTION: This function parses the Warning Header
**
** PARAMETERS:
**				pStart(IN)		- The starting point in the input buffer
**								  from where this parser can start parsing.
**				pEnd(IN)		- The position in the input buffer till
**								  which this parser should parse.
**				hdr(IN)			- The particular header for which this
**								  parser is being invoked (required since
**								  multiple similar headers could be parsed
**								  by the same function).
**				pOutHdr(OUT)	- A void structure with the relevant
**								  headers filled.
**				pErr(OUT)		- Error value if any
**
** NOTE:
****************************************************************************/
u_int32	_EcrioSigMgrParseWarningHeader
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSigMgrHeaderStruct **ppHeader
)
{
	u_char *pCurrentPos = pStart;
	EcrioSigMgrHeaderStruct *pHeader = *ppHeader;
	u_char *pToken = NULL;
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;

	if (pHeader == NULL)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderStruct), (void **)&pHeader);
		if (NULL == pHeader)
		{
			return ECRIO_SIGMGR_NO_MEMORY;
		}

		*ppHeader = pHeader;
	}

	/* Skip leading LWS */
	pCurrentPos = _EcrioSigMgrStripLeadingLWS(pCurrentPos, pEnd);

	/* Skip trailing LWS */
	pEnd = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pEnd);

	pHeader->numHeaderValues = 0;

	/* Now pCurrentPos points to a non-LWS character */

	while (pCurrentPos <= pEnd)
	{
		if (pHeader->ppHeaderValues == NULL)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderValueStruct *), (void **)&pHeader->ppHeaderValues);
		}
		else
		{
			/* Check arithmetic overflow - do not use the ArithmeticOverflowDetected() API for u_int16 variable. */
			if (pHeader->numHeaderValues + 1 > USHRT_MAX)
			{
				uError = ECRIO_SIGMGR_NO_MEMORY;
				goto Error;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((pHeader->numHeaderValues + 1), sizeof(EcrioSigMgrHeaderValueStruct *)) == Enum_TRUE)
			{
				uError = ECRIO_SIGMGR_NO_MEMORY;
				goto Error;
			}

			pal_MemoryReallocate(sizeof(EcrioSigMgrHeaderValueStruct *) * (pHeader->numHeaderValues + 1),
				(void **)&pHeader->ppHeaderValues);
		}
		if (NULL == pHeader->ppHeaderValues)
		{
			uError = ECRIO_SIGMGR_NO_MEMORY;
			goto Error;
		}

		/* Tokenize for ' ' */
		pToken = _EcrioSigMgrTokenizeQuotedString(pCurrentPos, pEnd, ' ');
		if (NULL == pToken)
		{
			/** Last token */
			u_char *pTemp;
			u_int32 size = 0;
			if (*pCurrentPos == '"')
			{
				if (pCurrentPos == pEnd)
				{
					/** Boundary checking; If '"' is last char in string and the first '"' of the warn-text, this is invalid header value. */
					uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
					goto Error;
				}
				pTemp = pCurrentPos + 1;
				size++;
				while ((*pTemp != '"') && (pTemp < pEnd))
				{
					size++;
					pTemp++;
				}
				size++;//for '"' 
			}
			else
			{
				size = (u_int32)(pEnd - pCurrentPos + 1);
			}

			pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderValueStruct), (void **)&pHeader->ppHeaderValues[pHeader->numHeaderValues]);
			if (NULL == pHeader->ppHeaderValues[pHeader->numHeaderValues])
			{
				uError = ECRIO_SIGMGR_NO_MEMORY;
				goto Error;
			}

			if (size > 0)
			{
				/*
				* header value is present
				*/
				_EcrioSigMgrStringNCreate(pSigMgr, pCurrentPos, &pHeader->ppHeaderValues[pHeader->numHeaderValues]->pHeaderValue, size);
			}
			else
			{
				pHeader->ppHeaderValues[pHeader->numHeaderValues]->pHeaderValue = NULL;
			}

			pHeader->numHeaderValues++;

			break;
		}
		else
		{
			/*
			* header value may or may not be present. If display name
			* is of non-zero length, store it.
			*/
			u_char *pTempLWS;
			u_int32 size = 0;

			pTempLWS = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pToken - 1);
			size = (u_int32)(pTempLWS - pCurrentPos + 1);

			pal_MemoryAllocate(sizeof(EcrioSigMgrHeaderValueStruct), (void **)&pHeader->ppHeaderValues[pHeader->numHeaderValues]);
			if (NULL == pHeader->ppHeaderValues[pHeader->numHeaderValues])
			{
				uError = ECRIO_SIGMGR_NO_MEMORY;
				goto Error;
			}

			if (size > 0)
			{
				/*
				* header value is present
				*/
				_EcrioSigMgrStringNCreate(pSigMgr, pCurrentPos, &pHeader->ppHeaderValues[pHeader->numHeaderValues]->pHeaderValue, size);
			}
			else
			{
				pHeader->ppHeaderValues[pHeader->numHeaderValues]->pHeaderValue = NULL;
			}
			/*
			* Advance pCurrentPos
			*/
			pCurrentPos = pToken + 1;
		}
		pHeader->numHeaderValues++;
	}

	return ECRIO_SIGMGR_NO_ERROR;

Error:
	if (pHeader != NULL)
	{
		if (pHeader->ppHeaderValues != NULL)
		{
			if (pHeader->numHeaderValues > 0)
			{
				for (u_int16 i = 0; i < pHeader->numHeaderValues; i++)
				{
					if (pHeader->ppHeaderValues[i] != NULL)
					{
						if (pHeader->ppHeaderValues[i]->pHeaderValue != NULL)
						{
							pal_MemoryFree((void **)&pHeader->ppHeaderValues[i]->pHeaderValue);
						}
						pal_MemoryFree((void **)&pHeader->ppHeaderValues[i]);
					}
				}
			}
			pal_MemoryFree((void **)&pHeader->ppHeaderValues);
		}
		pal_MemoryFree((void **)&pHeader);
	}

	return uError;
}
