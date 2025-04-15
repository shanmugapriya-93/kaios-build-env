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

typedef enum
{
	en_headerNameExpected = 0,
	en_headerBodyExpected
} HeaderParsingStateEnum;

EcrioSipMessageTypeEnum _EcrioSigMgrGetMethodTypeFromString
(
	u_char *pMethod
)
{
	if (pal_StringICompare(pMethod, (u_char *)"REGISTER") == 0)
	{
		return EcrioSipMessageTypeRegister;
	}
	else if (pal_StringICompare(pMethod, (u_char *)"INVITE") == 0)
	{
		return EcrioSipMessageTypeInvite;
	}
	else if (pal_StringICompare(pMethod, (u_char *)"ACK") == 0)
	{
		return EcrioSipMessageTypeAck;
	}
	else if (pal_StringICompare(pMethod, (u_char *)"BYE") == 0)
	{
		return EcrioSipMessageTypeBye;
	}
	else if (pal_StringICompare(pMethod, (u_char *)"CANCEL") == 0)
	{
		return EcrioSipMessageTypeCancel;
	}
	else if (pal_StringICompare(pMethod, (u_char *)"MESSAGE") == 0)
	{
		return EcrioSipMessageTypeMessage;
	}
	else if (pal_StringICompare(pMethod, (u_char *)"PRACK") == 0)
	{
		return EcrioSipMessageTypePrack;
	}
	else if (pal_StringICompare(pMethod, (u_char *)"SUBSCRIBE") == 0)
	{
		return EcrioSipMessageTypeSubscribe;
	}
	else if (pal_StringICompare(pMethod, (u_char *)"NOTIFY") == 0)
	{
		return EcrioSipMessageTypeNotify;
	}
	else if (pal_StringICompare(pMethod, (u_char *)"UPDATE") == 0)
	{
		return EcrioSipMessageTypeUpdate;
	}
	else if (pal_StringICompare(pMethod, (u_char *)"PUBLISH") == 0)
	{
		return EcrioSipMessageTypePublish;
	}
	else if (pal_StringICompare(pMethod, (u_char *)"REFER") == 0)
	{
		return EcrioSipMessageTypeRefer;
	}
	else if (pal_StringICompare(pMethod, (u_char *)"INFO") == 0)
	{
		return EcrioSipMessageTypeInfo;
	}
	else if (pal_StringICompare(pMethod, (u_char *)"OPTIONS") == 0)
	{
		return EcrioSipMessageTypeOptions;
	}
	return EcrioSipMessageTypeNone;
}

EcrioSipHeaderTypeEnum _EcrioSigMgrGetHeaderTypeFromString
(
	u_char *pStart
)
{
	switch (pStart[0])
	{
		case 'A':
		case 'a':
		{
			if (pal_StringICompare((u_char *)"Allow", pStart) == 0)
			{
				return EcrioSipHeaderTypeAllow;
			}
			else if (pal_StringICompare((u_char *)"Accept-Contact", pStart) == 0)
			{
				return EcrioSipHeaderTypeAcceptContact;
			}
			else if (pal_StringICompare((u_char *)"a", pStart) == 0)
			{
				return EcrioSipHeaderTypeAcceptContact;
			}
			else if (pal_StringICompare((u_char *)"Allow-Events", \
				pStart) == 0)
			{
				return EcrioSipHeaderTypeAllowEvents;
			}
			else if (pal_StringICompare((u_char *)"Authentication-Info", \
				pStart) == 0)
			{
				return EcrioSipHeaderTypeAuthenticationInfo;
			}
			else
			{
				return EcrioSipHeaderTypeHeader_Unknown;
			}
		}

		case 'c':
		case 'C':
		{
			if (pal_StringICompare((u_char *)"Call-id", pStart) == 0)
			{
				return EcrioSipHeaderTypeCallId;
			}
			else if (pal_StringICompare((u_char *)"Contact", pStart) == 0)
			{
				return EcrioSipHeaderTypeContact;
			}
			else if (pal_StringICompare((u_char *)"Content-Length", \
				pStart) == 0)
			{
				return EcrioSipHeaderTypeContentLength;
			}
			else if ((pal_StringICompare((u_char *)"Content-Type", \
				pStart) == 0))
			{
				return EcrioSipHeaderTypeContentType;
			}
			else if ((pal_StringICompare((u_char *)"Content-ID", \
				pStart) == 0))
			{
				return EcrioSipHeaderTypeContentID;
			}
			else if ((pal_StringICompare((u_char *)"Content-Disposition", \
				pStart) == 0))
			{
				return EcrioSipHeaderTypeContentDisposition;
			}
			else if ((pal_StringICompare((u_char *)"c", pStart) == 0))
			{
				return EcrioSipHeaderTypeContentType;
			}
			else if (pal_StringICompare((u_char *)"CSeq", pStart) == 0)
			{
				return EcrioSipHeaderTypeCSeq;
			}
			else if (pal_StringICompare((u_char *)"Content-Encoding", pStart) == 0)
			{
				return EcrioSipHeaderTypeContentEncoding;
			}
			else if (pal_StringICompare((u_char *)"Content-Transfer-Encoding", pStart) == 0)
			{
				return EcrioSipHeaderTypeContentTransferEncoding;
			}
			else if (pal_StringICompare((u_char *)"Contribution-ID", pStart) == 0)
			{
				return EcrioSipHeaderTypeContributionID;
			}
			else if (pal_StringICompare((u_char *)"Conversation-ID", pStart) == 0)
			{
				return EcrioSipHeaderTypeConversationID;
			}
			else
			{
				return EcrioSipHeaderTypeHeader_Unknown;
			}
		}

		case 'D':
		case 'd':
		{
			if (pal_StringICompare((u_char *)"Date", pStart) == 0)
			{
				return EcrioSipHeaderTypeDate;
			}
			else if (pal_StringICompare((u_char *)"Request-Disposition", pStart) == 0)
			{
				return EcrioSipHeaderTypeRequestDisposition;
			}
			else
			{
				return EcrioSipHeaderTypeHeader_Unknown;
			}
		}

		case 'E':
		case 'e':
		{
			if (pal_StringICompare((u_char *)"Expires", pStart) == 0)
			{
				return EcrioSipHeaderTypeExpires;
			}
			else if (pal_StringICompare((u_char *)"Event", pStart) == 0)
			{
				return EcrioSipHeaderTypeEvent;
			}
			else
			{
				return EcrioSipHeaderTypeHeader_Unknown;
			}
		}

		case 'F':
		case 'f':
		{
			if (pal_StringICompare((u_char *)"From", pStart) == 0)
			{
				return EcrioSipHeaderTypeFrom;
			}
			else if (pal_StringICompare((u_char *)"f", pStart) == 0)
			{
				return EcrioSipHeaderTypeFrom;
			}
			else
			{
				return EcrioSipHeaderTypeHeader_Unknown;
			}
		}

		case 'I':
		case 'i':
		{
			if (pal_StringICompare((u_char *)"i", pStart) == 0)
			{
				return EcrioSipHeaderTypeCallId;
			}
			else if (pal_StringICompare((u_char *)"InReplyTo-Contribution-ID", pStart) == 0)
			{
				return EcrioSipHeaderTypeInReplyToContributionID;
			}
			else
			{
				return EcrioSipHeaderTypeHeader_Unknown;
			}
		}

		case 'K':
		case 'k':
		{
			if (pal_StringICompare((u_char *)"k", pStart) == 0)
			{
				return EcrioSipHeaderTypeSupported;
			}
			else
			{
				return EcrioSipHeaderTypeHeader_Unknown;
			}
		}

		case 'l':
		case 'L':
		{
			if (pal_StringICompare((u_char *)"l", pStart) == 0)
			{
				return EcrioSipHeaderTypeContentLength;
			}
			else
			{
				return EcrioSipHeaderTypeHeader_Unknown;
			}
		}

		case 'M':
		case 'm':
		{
			if (pal_StringICompare((u_char *)"Mime-Version", pStart) == 0)
			{
				return EcrioSipHeaderTypeMimeVersion;
			}

			if (pal_StringICompare((u_char *)"m", pStart) == 0)
			{
				return EcrioSipHeaderTypeContact;
			}
			else if (pal_StringICompare((u_char *)"Min-Expires", pStart) == 0)
			{
				return EcrioSipHeaderTypeMinExpires;
			}
			else if (pal_StringICompare((u_char *)"Max-Forwards", pStart) == 0)
			{
				return EcrioSipHeaderTypeMaxForward;
			}
			else if (pal_StringICompare((u_char *)"Min-SE", pStart) == 0)
			{
				return EcrioSipHeaderTypeMinSE;
			}
			else
			{
				return EcrioSipHeaderTypeHeader_Unknown;
			}
		}

		case 'O':
		case 'o':
		{
			if (pal_StringICompare((u_char *)"o", pStart) == 0)
			{
				return EcrioSipHeaderTypeEvent;
			}
			else
			{
				return EcrioSipHeaderTypeHeader_Unknown;
			}
		}

		case 'P':
		case 'p':
		{
			if (pal_StringICompare((u_char *)"P-Associated-URI", \
				pStart) == 0)
			{
				return EcrioSipHeaderTypeP_AssociatedUri;
			}
			else if (pal_StringICompare((u_char *)"Privacy", \
				pStart) == 0)
			{
				return EcrioSipHeaderTypePrivacy;
			}
			else if (pal_StringICompare((u_char *)"Path", \
				pStart) == 0)
			{
				return EcrioSipHeaderTypePath;
			}
			else if (pal_StringICompare((u_char *)"P-Asserted-Identity", \
				pStart) == 0)
			{
				return EcrioSipHeaderTypeP_AssertedIdentity;
			}
			else if (pal_StringICompare((u_char *)"P-Preferred-Service", \
				pStart) == 0)
			{
				return EcrioSipHeaderTypeP_PreferredService;
			}
			else if (pal_StringICompare((u_char *)"P-Preferred-Identity", \
				pStart) == 0)
			{
				return EcrioSipHeaderTypeP_PreferredIdentity;
			}
			else if (pal_StringICompare((u_char *)"P-Early-Media", \
				pStart) == 0)
			{
				return EcrioSipHeaderTypeP_EarlyMedia;
			}
			else
			{
				return EcrioSipHeaderTypeHeader_Unknown;
			}
		}

		case 'R':
		case 'r':
		{
			if (pal_StringICompare((u_char *)"RAck", \
				pStart) == 0)
			{
				return EcrioSipHeaderTypeRAck;
			}
			else if (pal_StringICompare((u_char *)"Record-Route", \
				pStart) == 0)
			{
				return EcrioSipHeaderTypeRecordRoute;
			}
			else if (pal_StringICompare((u_char *)"Request-Disposition", \
				pStart) == 0)
			{
				return EcrioSipHeaderTypeRequestDisposition;
			}
			else if (pal_StringICompare((u_char *)"RSeq", \
				pStart) == 0)
			{
				return EcrioSipHeaderTypeRSeq;
			}
			else if (pal_StringICompare((u_char *)"Require", \
				pStart) == 0)
			{
				return EcrioSipHeaderTypeRequire;
			}
			else if (pal_StringICompare((u_char *)"Route", pStart) == 0)
			{
				return EcrioSipHeaderTypeRoute;
			}
			else if (pal_StringICompare((u_char *)"Retry-After", \
				pStart) == 0)
			{
				return EcrioSipHeaderTypeRetryAfter;
			}
			else if (pal_StringICompare((u_char *)"Reason", pStart) == 0)
			{
				return EcrioSipHeaderTypeReason;
			}
			else if (pal_StringICompare((u_char *)"Referred-By", pStart) == 0)
			{
				return EcrioSipHeaderTypeReferredBy;
			}
			else if (pal_StringICompare((u_char *)"Refer-To", pStart) == 0)
			{
				return EcrioSipHeaderTypeReferTo;
			}
			else if (pal_StringICompare((u_char *)"Refer-Sub", pStart) == 0)
			{
				return EcrioSipHeaderTypeReferSub;
			}
			else
			{
				return EcrioSipHeaderTypeHeader_Unknown;
			}
		}

		case 'S':
		case 's':
		{
			if (pal_StringICompare((u_char *)"Server", pStart) == 0)
			{
				return EcrioSipHeaderTypeServer;
			}
			else if (pal_StringICompare((u_char *)"SIP-ETag", pStart) == 0)
			{
				return EcrioSipHeaderTypeSipEtag;
			}
			else if (pal_StringICompare((u_char *)"Service-Route", pStart) == 0)
			{
				return EcrioSipHeaderTypeServiceRoute;
			}
			else if (pal_StringICompare((u_char *)"Session-Expires", pStart) == 0)
			{
				return EcrioSipHeaderTypeSessionExpires;
			}
			else if (pal_StringICompare((u_char *)"Subscription-State", pStart) == 0)
			{
				return EcrioSipHeaderTypeSubscriptionState;
			}
			if (pal_StringICompare((u_char *)"Supported", pStart) == 0)
			{
				return EcrioSipHeaderTypeSupported;
			}
			if (pal_StringICompare((u_char *)"Subject", pStart) == 0)
			{
				return EcrioSipHeaderTypeSubject;
			}
			else if (pal_StringICompare((u_char *)"Security-Server", pStart) == 0)
			{
				return EcrioSipHeaderTypeSecurityServer;
			}
			else if (pal_StringICompare((u_char *)"Security-Client", pStart) == 0)
			{
				return EcrioSipHeaderTypeSecurityClient;
			}
			else if (pal_StringICompare((u_char *)"Security-Verify", pStart) == 0)
			{
				return EcrioSipHeaderTypeSecurityVerify;
			}
			else
			{
				return EcrioSipHeaderTypeHeader_Unknown;
			}
		}

		case 'T':
		case 't':
		{
			if (pal_StringICompare((u_char *)"Timestamp", pStart) == 0)
			{
				return EcrioSipHeaderTypeTimestamp;
			}
			else if ((pal_StringICompare((u_char *)"To", pStart) == 0))
			{
				return EcrioSipHeaderTypeTo;
			}
			else if ((pal_StringICompare((u_char *)"t", pStart) == 0))
			{
				return EcrioSipHeaderTypeTo;
			}
			else
			{
				return EcrioSipHeaderTypeHeader_Unknown;
			}
		}

		case 'U':
		case 'u':
		{
			if (pal_StringICompare((u_char *)"Unsupported", pStart) == 0)
			{
				return EcrioSipHeaderTypeUnsupported;
			}
			else if (pal_StringICompare((u_char *)"User-Agent", pStart) == 0)
			{
				return EcrioSipHeaderTypeUserAgent;
			}
			else if (pal_StringICompare((u_char *)"u", pStart) == 0)
			{
				return EcrioSipHeaderTypeAllowEvents;
			}
			else
			{
				return EcrioSipHeaderTypeHeader_Unknown;
			}
		}

		case 'V':
		case 'v':
		{
			if (pal_StringICompare((u_char *)"Via", pStart) == 0)
			{
				return EcrioSipHeaderTypeVia;
			}
			else if (pal_StringICompare((u_char *)"v", pStart) == 0)
			{
				return EcrioSipHeaderTypeVia;
			}
			else
			{
				return EcrioSipHeaderTypeHeader_Unknown;
			}
		}

		case 'W':
		case 'w':
		{
			if (pal_StringICompare((u_char *)"Warning", pStart) == 0)
			{
				return EcrioSipHeaderTypeWarning;
			}
			else if (pal_StringICompare((u_char *)"WWW-Authenticate", \
				pStart) == 0)
			{
				return EcrioSipHeaderTypeWWWAuthenticate;
			}
			else
			{
				return EcrioSipHeaderTypeHeader_Unknown;
			}
		}

		case 'x':
		{
			if (pal_StringICompare((u_char *)"x", pStart) == 0)
			{
				return EcrioSipHeaderTypeSessionExpires;
			}
			else
			{
				return EcrioSipHeaderTypeHeader_Unknown;
			}
		}

		default:
			return EcrioSipHeaderTypeHeader_Unknown;
	}
}

/****************************************************************************
** FUNCTION: _EcrioSigMgrParseEachParam
** DESCRIPTION: This function parses and fills a SipParam structure
**
** PARAMETERS:
**				ppStart(IN)		- The starting point in the input buffer
**								  from where this parser can start parsing.
**				pEnd(IN)		- The position in the input buffer till
**								  which this parser should parse.
**				Flag			- Flag telling whether name=value is a must
**								  headers filled.
**				pErr(OUT		- Error value if any
**
** RETURN VALUE: Pointer to a SipParam structure
****************************************************************************/
EcrioSigMgrParamStruct *_EcrioSigMgrParseEachParam
(
	EcrioSigMgrStruct *pSigMgr, \
	u_char *pStart,
	u_char *pEnd,
	BoolEnum valueReqd
)
{
	EcrioSigMgrParamStruct *tmpSipParam = NULL;
	u_char *tmp = NULL;
	u_char *pPtr = NULL;

	/* Strip leading and trailing LWS */
	pStart = _EcrioSigMgrStripLeadingLWS(pStart, pEnd);
	pEnd = _EcrioSigMgrStripTrailingLWS(pStart, pEnd);

	pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&tmpSipParam);
	if (tmpSipParam == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() for tmpSipParam",
			__FUNCTION__, __LINE__);

		return NULL;
	}

	/* Look for param name */
	tmp = _EcrioSigMgrTokenize(pStart, pEnd, '=');
	if (tmp != NULL)
	{
		pPtr = _EcrioSigMgrStripLeadingLWS(tmp + 1, pEnd);
		if ((*pPtr == '\n') || (*pPtr == '\r') || (pPtr > pEnd))
		{
			goto error;
		}
	}

	if (tmp == NULL)
	{
		/*
		*params of type abc i.e. name only
		* In this case check whether such params are allowed.
		*if allowed then store the name
		*/

		if (valueReqd == Enum_FALSE)
		{
			/*If name alone is allowed store  name
			*/
			_EcrioSigMgrStringNCreate(pSigMgr, pStart, &tmpSipParam->pParamName, (pEnd - pStart + 1));
			if (NULL == tmpSipParam->pParamName)
			{
				pal_MemoryFree((void **)&tmpSipParam);
				return NULL;
			}

			pStart = pEnd;
		}
		else
		{
			/*
			if name alone is not allowed return error.
			*/
			pal_MemoryFree((void **)&tmpSipParam);
			return NULL;
		}
	}
	/** Boundary checking; If '=' is last char in string such as "abc=", this is invalid SIP param scheme. */
	else if (tmp == pEnd)
	{
		goto error;
	}
	else
	/*Params of type  abc=def
	If got an equal sign    store name and value
	*/
	{
		u_char *valueString = NULL;
		u_char *valueBegin = NULL;
		u_char *valueEnd = NULL;
		u_char *nameEnd = NULL;
		nameEnd = _EcrioSigMgrStripTrailingLWS(pStart, tmp - 1);
		_EcrioSigMgrStringNCreate(pSigMgr, pStart, &tmpSipParam->pParamName, (nameEnd - pStart + 1));
		if (tmpSipParam->pParamName == NULL)
		{
			pal_MemoryFree((void **)&tmpSipParam);
			return NULL;
		}

		valueBegin = _EcrioSigMgrStripLeadingLWS(tmp + 1, pEnd);
		valueEnd = _EcrioSigMgrStripTrailingLWS(tmp + 1, pEnd);
		_EcrioSigMgrStringNCreate(pSigMgr, valueBegin, &valueString, (valueEnd - valueBegin + 1));
		if (valueString == NULL)
		{
			if (NULL != tmpSipParam->pParamName)
			{
				pal_MemoryFree((void **)&tmpSipParam->pParamName);
				tmpSipParam->pParamName = NULL;
			}
			pal_MemoryFree((void **)&tmpSipParam);
			return NULL;
		}

		if ((NULL == tmpSipParam->pParamName) || (NULL == valueString))
		{
			if (NULL != valueString)
			{
				pal_MemoryFree((void **)&valueString);
			}

			pal_MemoryFree((void **)&tmpSipParam);
			return NULL;
		}

		tmpSipParam->pParamValue = valueString;
	}

	return tmpSipParam;

error:
	if (tmpSipParam != NULL)
	{
		_EcrioSigMgrReleaseParamsStruct(pSigMgr, tmpSipParam);
		pal_MemoryFree((void **)&tmpSipParam);
	}
	return NULL;
}

/****************************************************************************
** FUNCTION: _EcrioSigMgrParseSipParam
** DESCRIPTION: This function parses a SipParam and Returns a list of slParam
**
** PARAMETERS:
**				pStart(IN)		- The starting point in the input buffer
**								  from where this parser can start parsing.
**				pEnd(IN)		- The position in the input buffer till
**								  which this parser should parse.
**				slParam(OUT)	- The sipList which is filled and returned
**				Flag			- Flag telling whether name=value is a must
**								  headers filled.
**				dLimiter		- The dLimiter by which params are separated
**				pErr(OUT		- Error value if any
**
** NOTE:
****************************************************************************/
u_int32 _EcrioSigMgrParseSipParam
(
	EcrioSigMgrStruct *pSigMgr, \
	u_int16 *pNumParams,
	EcrioSigMgrParamStruct ***pppParams, \
	u_char *pStart,
	u_char *pEnd,
	BoolEnum valueReqd,
	u_char dLimiter
)
{
	EcrioSigMgrParamStruct *pTmpParam = NULL;
	EcrioSigMgrParamStruct **ppParams = NULL;
	u_int16 numParams = 0;
	u_char *pTmp = NULL;
	u_char *pPtr = NULL;

	if (pStart > pEnd)
	{
		return ECRIO_SIGMGR_INVALID_URI_SCHEME;
	}

	if (*pEnd == dLimiter)
	{
		return ECRIO_SIGMGR_INVALID_URI_SCHEME;
	}

	while (pStart <= pEnd)	/*Continue until the start < End*/
	{
		/*		while ((*pStart == ' ')|| (*pStart == '\t'))
		pStart++;remove the leading white spaces*/
		pStart = _EcrioSigMgrStripLeadingLWS(pStart, pEnd);
		if (pStart > pEnd)
		{
			goto error;
		}

		/* First extract each parameter by looking for the delimiter
		* and then parse each param individually.
		*/

		pTmp = _EcrioSigMgrTokenize(pStart, pEnd, dLimiter);

		/** Boundary checking; If delimiter is last char in string, this is invalid sip param. */
		if (pTmp == pEnd)
		{
			goto error;
		}

		if (pTmp != NULL)
		{
			pPtr = _EcrioSigMgrStripTrailingLWS(pTmp + 1, pEnd);
			if (*pPtr == dLimiter)
			{
				goto error;
			}
		}

		if (pTmp == NULL)
		{
			pTmp = pEnd + 1;
		}

		pTmpParam = _EcrioSigMgrParseEachParam(pSigMgr, pStart, pTmp - 1, valueReqd);
		if (pTmpParam != NULL)
		{
			if (ppParams == NULL)
			{
				pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct *), (void **)&ppParams);
				if (ppParams == NULL)
				{
					if (NULL != pTmpParam)
					{
						pal_MemoryFree((void **)&pTmpParam);
					}
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tpal_MemoryAllocate() for ppParams",
						__FUNCTION__, __LINE__);

					return ECRIO_SIGMGR_NO_MEMORY;
				}
			}
			else
			{
				/* Check arithmetic overflow - do not use the ArithmeticOverflowDetected() API for u_int16 variable. */
				if (numParams + 1 > USHRT_MAX)
				{
					return ECRIO_SIGMGR_NO_MEMORY;
				}

				/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
				if (pal_UtilityDataOverflowDetected((numParams + 1), sizeof(EcrioSigMgrParamStruct *)) == Enum_TRUE)
				{
					return ECRIO_SIGMGR_IMS_LIB_ERROR;
				}

				pal_MemoryReallocate(sizeof(EcrioSigMgrParamStruct *) * (numParams + 1), (void **)&ppParams);
				if (ppParams == NULL)
				{
					if (NULL != pTmpParam)
					{
						pal_MemoryFree((void **)&pTmpParam);
					}
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tpal_MemoryAllocate() for ppParams",
						__FUNCTION__, __LINE__);

					return ECRIO_SIGMGR_NO_MEMORY;
				}
			}

			ppParams[numParams] = pTmpParam;
			numParams++;
		}
		else
		{
			goto error;
		}

		pStart = pTmp + 1;
	}

	*pNumParams = numParams;
	*pppParams = ppParams;
	return ECRIO_SIGMGR_NO_ERROR;

error:
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

	if (pTmpParam != NULL)
	{
		_EcrioSigMgrReleaseParamsStruct(pSigMgr, pTmpParam);
		pal_MemoryFree((void **)&pTmpParam);
	}

	return ECRIO_SIGMGR_INVALID_URI_SCHEME;
}

/****************************************************************************
** FUNCTION: _EcrioSigMgrParseHostPort
** DESCRIPTION: This function parses the Host and Port fields
**
** PARAMETERS:
**				pStart(IN)		- The starting point in the input buffer
**								  from where this parser can start parsing.
**				pEnd(IN)		- The position in the input buffer till
**								  which this parser should parse.
**				ppHost(OUT)		- The parsed Host address
**				ppPort(OUT)		- The parsed port no
**				pErr(OUT		- Error value if any
**
** NOTE:
****************************************************************************/
u_int32 _EcrioSigMgrParseHostPort
(
	EcrioSigMgrStruct *pSigMgr, \
	u_char *pStart,
	u_char *pEnd, \
	u_char **ppHost,
	u_int16 **ppPort
)
{
	u_char *Host = NULL;
	u_int16 *pPort = NULL;
	u_int32	tmpPort = 0;
	u_char *pCurrentPos = NULL;
	u_char *tmp = NULL;

	pal_MemoryAllocate(sizeof(u_int32), (void **)&pPort);
	if (NULL == pPort)
	{
		return ECRIO_SIGMGR_NO_MEMORY;
	}

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
			/*If Current Position is NULL tis means an invalid IPV6Address
			*is present
			*/
			goto error;
		}
		else
		{
			/* Commented out for dual checking, done in IMS Library*/
#if 0
			/*Call validate IPV6 address
			*/

			if (SipFail == sip_validateIPv6Addr(pStart, pCurrentPos, pErr))
			{
				sip_error(SIP_Minor, "Error in Validating IPV6\n");
				goto error;
			}/*End of Validation*/
			else
#endif
			{
				/*If it is a valid IPV6 Now Store IPV6Address and Port
				*/
				tmp = _EcrioSigMgrTokenize(pCurrentPos + 1, pEnd, ':');
				if (tmp == NULL)
				{
					/*This means only Host part is present
					*/
					pStart = pStart + 1;
					pEnd = pEnd - 1;

					/** Boundary checking; if input string has "[]" which pStart indicates '[' and pEnd indicates ']'
					 *  and no port number is present, then going through this path, but this is an invalid string so
					 *  we need to return error.
					 */
					if (pStart > pEnd)
					{
						goto error;
					}

					_EcrioSigMgrStringNCreate(pSigMgr, pStart, &Host, (pEnd - pStart + 1));
					pal_MemoryFree((void **)&pPort);
					pPort = NULL;
					if (NULL == Host)
					{
						return ECRIO_SIGMGR_INVALID_URI_SCHEME;
					}
				}
				/** Boundary checking; If ':' is last char in string, this is invalid URI scheme. */
				else if (tmp == pEnd)
				{
					goto error;
				}
				else
				{
					/* Both host and port present. */
					u_char *pHostEnd = NULL;
					u_char *pPortBegin = NULL;

					pHostEnd = _EcrioSigMgrStripTrailingLWS(pStart, tmp - 1);
					pStart = pStart + 1;
					pHostEnd = pHostEnd - 1;

					_EcrioSigMgrStringNCreate(pSigMgr, pStart, &Host, (pHostEnd - pStart + 1));
					if (NULL == Host)
					{
						if (NULL != pPort)
						{
							pal_MemoryFree((void **)&pPort);
							pPort = NULL;
						}
						return ECRIO_SIGMGR_INVALID_URI_SCHEME;
					}

					pPortBegin = _EcrioSigMgrStripLeadingLWS(tmp + 1, pEnd);
					tmpPort = pal_StringConvertToUNum(pPortBegin, NULL, 10);
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

			pal_MemoryFree((void **)&pPort);
			pPort = NULL;
			_EcrioSigMgrStringNCreate(pSigMgr, pStart, &Host, (pEnd - pStart + 1));
			if (Host == NULL)
			{
				goto error;
			}
		}
		/** Boundary checking; If ':' is last char in string, this is invalid URI scheme. */
		else if (tmp == pEnd)
		{
			goto error;
		}
		else
		{
			u_char *pHostEnd = NULL;
			u_char *pPortBegin = NULL;

			pHostEnd = _EcrioSigMgrStripTrailingLWS(pStart, tmp - 1);
			_EcrioSigMgrStringNCreate(pSigMgr, pStart, &Host, (pHostEnd - pStart + 1));
			if (NULL == Host)
			{
				goto error;
			}

			pPortBegin = _EcrioSigMgrStripLeadingLWS(tmp + 1, pEnd);
			tmpPort = pal_StringConvertToUNum(pPortBegin, NULL, 10);
			*pPort = (u_int16)tmpPort;
		}
	}

	if (Host)
	{
		*ppHost = Host;
	}

	*ppPort = pPort;
	return ECRIO_SIGMGR_NO_ERROR;

error:
	pal_MemoryFree((void **)&pPort);
	pal_MemoryFree((void **)&Host);

	return ECRIO_SIGMGR_INVALID_URI_SCHEME;
}

/****************************************************************************
** FUNCTION: _EcrioSigMgrParseUserAndHostPort
** DESCRIPTION: This function parses the username, password, host and port
**
** PARAMETERS:
**				pStart(IN)		- The starting point in the input buffer
**								  from where this parser can start parsing.
**				pEnd(IN)		- The position in the input buffer till
**				ppHost(OUT)		- The Host address
**				ppPort(OUT)		- The port no
**				ppUser(OUT)		- The user name
**				ppPassword(OUT)	- The password
**				pErr(OUT		- Error value if any
**
** NOTE:
****************************************************************************/
u_int32 _EcrioSigMgrParseUserAndHostPort
(
	EcrioSigMgrStruct *pSigMgr, \
	u_char *pStart, \
	u_char *pEnd,
	u_char **ppUser,
	u_char **ppPassword, \
	u_char **ppHost,
	u_int16 **ppPort
)
{
	u_char *pCurrentPos = NULL;
	u_char *tmp = NULL;
	u_char *User = NULL;
	u_char *Password = NULL;
	u_char *Host = NULL;
	u_int16 *Port = NULL;
	u_char *pPasswordEnd = NULL;

	/*Remove LWS*/
	pStart = _EcrioSigMgrStripLeadingLWS(pStart, pEnd);
	pEnd = _EcrioSigMgrStripTrailingLWS(pStart, pEnd);
	if (*pStart == '@')
	{
		/*username:password@host:port cannot begin	with @*/
		return ECRIO_SIGMGR_INVALID_URI_SCHEME;
	}

	/*Now tokenize for @ to get user and Password*/

	pCurrentPos = _EcrioSigMgrTokenize(pStart, pEnd, '@');
	if (pCurrentPos != NULL)
	{
		pPasswordEnd = _EcrioSigMgrStripTrailingLWS(pStart, pCurrentPos - 1);
	}

	if (pCurrentPos != NULL)
	{
		/*This means user:password@hss.hns.com:5060 is  there*/
		tmp = _EcrioSigMgrTokenize(pStart, pCurrentPos, ':');
		if (tmp != NULL)
		{
			u_char *pUserEnd = NULL;
			u_char *pPasswordBegin = NULL;
			/*This means user:password is  there
			*Now tokenize to get password and User name
			*/
			pUserEnd = _EcrioSigMgrStripTrailingLWS(pStart, tmp - 1);
			_EcrioSigMgrStringNCreate(pSigMgr, pStart, &User, (pUserEnd - pStart + 1));
			if (User == NULL)
			{
				return ECRIO_SIGMGR_INVALID_URI_SCHEME;
			}

			pPasswordBegin = _EcrioSigMgrStripLeadingLWS(tmp + 1, pPasswordEnd);
			_EcrioSigMgrStringNCreate(pSigMgr, pPasswordBegin, &Password, (pPasswordEnd - pPasswordBegin + 1));
			if (Password == NULL)
			{
				goto error;
			}

			if (_EcrioSigMgrParseHostPort(pSigMgr, pCurrentPos + 1, pEnd, \
				&Host, &Port))
			{
				goto error;
			}
		}
		else
		{
			u_char *pUserEnd = NULL;
			/*if Colon is absent
			* Password is not there
			*/
			pUserEnd = _EcrioSigMgrStripTrailingLWS(pStart, pCurrentPos - 1);
			_EcrioSigMgrStringNCreate(pSigMgr, pStart, &User, (pUserEnd - pStart + 1));
			/* if for Storing  User and  password*/
			if (NULL == User)
			{
				return ECRIO_SIGMGR_INVALID_URI_SCHEME;
			}

			/** Boundary checking; If '@' is last char in string, this is invalid URI scheme. */
			else if (pCurrentPos == pEnd)
			{
				goto error;
			}

			if (_EcrioSigMgrParseHostPort(pSigMgr, pCurrentPos + 1, pEnd, \
				&Host, &Port))
			{
				goto error;
			}
		}
	}
	else
	{
		/*This Case User:password is not there
		* This is of the form sip:hss.hns.com:5060
		* So get the host and port
		*/

		if (_EcrioSigMgrParseHostPort(pSigMgr, pStart, pEnd, &Host, &Port))
		{
			return ECRIO_SIGMGR_INVALID_URI_SCHEME;
		}
	}

	*ppUser = User;
	*ppPassword = Password;
	*ppHost = Host;
	*ppPort = Port;
	return ECRIO_SIGMGR_NO_ERROR;

error:
	if (NULL != User)
		pal_MemoryFree((void **)&User);
		
	if (NULL != Password)
		pal_MemoryFree((void **)&Password);

	return ECRIO_SIGMGR_INVALID_URI_SCHEME;
}

u_int32 _EcrioSigMgrParseTelParams
(
	EcrioSigMgrStruct *pSigMgr,
	u_int16	*pTelParamCount,
	EcrioSigMgrTelParStruct ***pppOutTelParams,
	u_char *pStart,
	u_char *pEnd
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_char *pCurrentPos = NULL;
	u_char *pNextParamStart = NULL;
	u_char *pParamNameEnd = NULL;
	u_char *pParamValueStart = NULL;
	u_char *pParamValueEnd = NULL;
	u_char *pParamVal = NULL;
	u_int16	telParamCount = 0;
	EcrioSigMgrTelParStruct **ppOutTelParams = NULL;
	BoolEnum bContinue = Enum_TRUE;

	pCurrentPos = _EcrioSigMgrStripLeadingLWS(pStart, pEnd);
	pEnd = _EcrioSigMgrStripTrailingLWS(pStart, pEnd);

	do
	{
		pNextParamStart = _EcrioSigMgrTokenize(pCurrentPos, pEnd, (u_char)';');
		if (pNextParamStart == NULL)// it is the last parameter, hence no more ;
		{
			bContinue = Enum_FALSE;
			pNextParamStart = pEnd + 1;	// set at the end
		}

		pParamValueEnd = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pNextParamStart - 1);// going to end of param value
		if (bContinue == Enum_TRUE)
		{
			/** Boundary checking; If ';' is last char in string, then this is considered the last parameter. */
			if (pNextParamStart == pEnd)
			{
				bContinue = Enum_FALSE;
				pNextParamStart = pEnd + 1;	// set at the end
			}
			else
			{
				pNextParamStart = _EcrioSigMgrStripLeadingLWS(pNextParamStart + 1, pEnd);	// if there is more parameters go to the begining of the next param name
			}
		}

		pParamNameEnd = _EcrioSigMgrTokenize(pCurrentPos, pParamValueEnd, (u_char)'=');	// tentative param name end
		if (pParamNameEnd == NULL)	// there is no param value such as aliason part of tel-uri "tel:1234;phone-context=example.com;tk=on;aliason"
		{
			pParamValueStart = NULL;
			pParamNameEnd = _EcrioSigMgrTokenize(pCurrentPos, pEnd, (u_char)';');	// get pointer to end of param name
			if (pParamNameEnd == NULL)
			{
				pParamNameEnd = pNextParamStart - 1;
			}
			else
			{
				pParamNameEnd--;
			}
		}
		else
		{
			pParamValueStart = pParamNameEnd + 1;
			pParamNameEnd = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pParamNameEnd - 1);
		}

		if (ppOutTelParams == NULL)
		{
			pal_MemoryAllocate(sizeof(EcrioSigMgrTelParStruct *), (void **)&ppOutTelParams);
			if (ppOutTelParams == NULL)
			{
				goto Error;
			}
		}
		else
		{
			/* Check arithmetic overflow - do not use the ArithmeticOverflowDetected() API for u_int16 variable. */
			if (telParamCount + 1 > USHRT_MAX)
				goto Error;

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((telParamCount + 1), sizeof(EcrioSigMgrTelParStruct *)) == Enum_TRUE)
				goto Error;

			pal_MemoryReallocate((telParamCount + 1) * sizeof(EcrioSigMgrTelParStruct *), (void **)&ppOutTelParams);
			if (ppOutTelParams == NULL)
			{
				goto Error;
			}
		}

		pal_MemoryAllocate(sizeof(EcrioSigMgrTelParStruct), (void **)&ppOutTelParams[telParamCount]);
		if (ppOutTelParams[telParamCount] == NULL)
		{
			goto Error;
		}

		if (pParamValueStart != NULL)
		{
			_EcrioSigMgrStringNCreate(pSigMgr, pParamValueStart, &pParamVal, (u_int32)(pParamValueEnd - pParamValueStart + 1));
		}

		if (pal_StringNCompare(pCurrentPos, (u_char *)"ext", 3) == 0)
		{
			ppOutTelParams[telParamCount]->telParType = EcrioSigMgrTelParExtension;
			ppOutTelParams[telParamCount]->u.pExtension = pParamVal;
		}
		else if (pal_StringNCompare(pCurrentPos, (u_char *)"isub", 3) == 0)
		{
			ppOutTelParams[telParamCount]->telParType = EcrioSigMgrTelParIsdnSubAddress;
			ppOutTelParams[telParamCount]->u.pIsdnSubaddress = pParamVal;
		}
		else
		{
			ppOutTelParams[telParamCount]->telParType = EcrioSigMgrTelParParameter;

			pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&ppOutTelParams[telParamCount]->u.pParameter);
			if (ppOutTelParams[telParamCount]->u.pParameter == NULL)
			{
				pal_MemoryFree((void **)&ppOutTelParams);
				pal_MemoryFree((void **)&pParamVal);
				goto Error;
			}

			_EcrioSigMgrStringNCreate(pSigMgr, pCurrentPos, &ppOutTelParams[telParamCount]->u.pParameter->pParamName, (u_int32)(pParamNameEnd - pCurrentPos + 1));
			ppOutTelParams[telParamCount]->u.pParameter->pParamValue = pParamVal;
		}

		pParamVal = NULL;
		pCurrentPos = pNextParamStart;
		telParamCount++;
	} while (bContinue == Enum_TRUE);

	*pTelParamCount = telParamCount;
	*pppOutTelParams = ppOutTelParams;

Error:
	ppOutTelParams = NULL;
	return error;
}

/****************************************************************************
** FUNCTION: _EcrioSigMgrParseTelUrl
** DESCRIPTION: This function parses the TelURL
**
** PARAMETERS:
**				pStart(IN)		- The starting point in the input buffer
**								  from where this parser can start parsing.
**				pEnd(IN)		- The position in the input buffer till
**								  which this parser should parse.
**				ppOutUrl(OUT)	- The TelUrl structure with relevan values
**								  filled.
**				pErr(OUT		- Error value if any
**
** NOTE:
****************************************************************************/
u_int32 _EcrioSigMgrParseTelUrl
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrTelURIStruct **ppOutUrl,
	u_char *pStart,
	u_char *pEnd
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrTelURIStruct	*pTelUrl = NULL;
	EcrioSigMgrTelLocalNumberStruct	*pLocalNum = NULL;
	EcrioSigMgrTelGlobalNumberStruct *pGlobalNum = NULL;
	u_char *pCurrentPos = NULL;
	u_char *pNumEnd = NULL;
	u_char *pNum = NULL;
	BoolEnum numberOnly = Enum_FALSE;

	/*
	* tel:+12345
	* The higher level function getTelUrlFrom AddrSpec will always check
	* whether the given URI is a tel URL.So here we dont need to  check the
	* same again.The higer level function getTelUrlFroAddrSpec will give the
	* input pStart as +12345.This will reduce the amount of processing.
	* So we need to check only whether the given number is a TelLocalNum or a
	* TelGlobalNumber.
	*		 telephone-uri       =  "tel:" subscriber *param
	*       subscriber          =  global-number / local-number
	*       global-number       =  global-number-part [isdn-subaddress]
	*       global-number-part  =  "+" 1*phonedigit
	*       local-number        =  local-number-part [isdn-subaddress]
	*                              [context]
	*/

	pal_MemoryAllocate(sizeof(EcrioSigMgrTelURIStruct), (void **)&pTelUrl);
	if (pTelUrl == NULL)
	{
		error = ECRIO_SIGMGR_NO_MEMORY;
		goto Error;
	}

	pCurrentPos = _EcrioSigMgrStripLeadingLWS(pStart, pEnd);
	pEnd = _EcrioSigMgrStripTrailingLWS(pStart, pEnd);

	/*
	* Now we have stripped the leading white space.We have the telephone number.
	* if the number starts with "+" it is a telephone global number else
	* it is a telelphone local number.
	*/
	if (*pCurrentPos == '+')
	{
		/** Boundary checking; If pCurrentPos is last char in string, this is invalid URI scheme. */
		if (pCurrentPos == pEnd)
		{
			error = ECRIO_SIGMGR_INVALID_URI_SCHEME;
			goto Error;
		}
		/*
		* Bypassing +
		*/
		pCurrentPos = _EcrioSigMgrStripLeadingLWS(pCurrentPos + 1, pEnd);

		/*
		* telGlobal number and do parsing
		*/
		/*
		* Find the end of the  number
		*/

		pNumEnd = _EcrioSigMgrTokenize(pCurrentPos, pEnd, ';');
		/*
		* If there is no ; this means only the number is there
		* no other params.
		*/
		if (pNumEnd == NULL)
		{
			pNumEnd = pEnd;
			numberOnly = Enum_TRUE;
		}
		else
		{
			pNumEnd--;
		}

		pNum = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pNumEnd);

		pal_MemoryAllocate(sizeof(EcrioSigMgrTelGlobalNumberStruct), (void **)&pGlobalNum);
		if (pGlobalNum == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error;
		}

		pTelUrl->subscriberType = EcrioSigMgrTelSubscriberGlobal;
		pTelUrl->u.pGlobalNumber = pGlobalNum;

		_EcrioSigMgrStringNCreate(pSigMgr, pCurrentPos, &pGlobalNum->pGlobalNumberDigits, (u_int32)((pNum - pCurrentPos) + 1));
		/*
		* If we have only numbers and no parameters we can stop
		* parsing and at this pont we can return a success
		*/
		if (numberOnly == Enum_TRUE)
		{
			error = ECRIO_SIGMGR_NO_ERROR;
			*ppOutUrl = pTelUrl;
			goto End;
		}

		/*
		* Try to get the parameters
		* First we are checking for isub=
		* If we come here this means there is params after number.
		*/
		error = _EcrioSigMgrParseTelParams(pSigMgr, &pGlobalNum->numTelPar, &pGlobalNum->ppTelPar, pNumEnd + 2, pEnd);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			goto Error;
		}
	}
	else
	{
		/**
		** initialize telLocal number
		**/
		/*
		* Find the end of the  number
		*/
		pNumEnd = _EcrioSigMgrTokenize(pCurrentPos, pEnd, ';');
		/*
		* If there is no ; this means only the number is there
		* no other params.
		*/
		if (pNumEnd == NULL)
		{
			pNumEnd = pEnd;
			numberOnly = Enum_TRUE;
		}
		else
		{
			pNumEnd--;
		}

		/*
		* Validate the phone number to see whether any illegal characters
		* are there
		*/

		pNum = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pNumEnd);

		/*
		* We need to initialize the local number only if
		* the initial validation of telephone number
		* succeeds
		*/
		pal_MemoryAllocate(sizeof(EcrioSigMgrTelLocalNumberStruct), (void **)&pLocalNum);
		if (pLocalNum == NULL)
		{
			error = ECRIO_SIGMGR_NO_MEMORY;
			goto Error;
		}

		pTelUrl->subscriberType = EcrioSigMgrTelSubscriberLocal;
		pTelUrl->u.pLocalNumber = pLocalNum;/* -mustfreeonly flag warning */

		_EcrioSigMgrStringNCreate(pSigMgr, pCurrentPos, &pLocalNum->pLocalNumberDigits, (u_int32)((pNum - pCurrentPos) + 1));

		/*
		* If we have only numbers and no parameters we can stop
		* parsing and at this pont we can return a success
		*/
		if (numberOnly == Enum_TRUE)
		{
			error = ECRIO_SIGMGR_NO_ERROR;
			*ppOutUrl = pTelUrl;
			goto End;
		}

		/*
		* Try to get the parameters
		* First we are checking for isub=
		* If we come here this means there is params after number.
		*/
		pNumEnd = _EcrioSigMgrStripLeadingLWS(pNumEnd + 2, pEnd);
		pCurrentPos = pal_SubString(pNumEnd, (u_char *)"phone-context");
		if (pCurrentPos == NULL)
		{
			error = ECRIO_SIGMGR_INVALID_URI_SCHEME;
			goto Error;
		}

		if (pCurrentPos && pCurrentPos <= pEnd)	// phone-context found
		{
			// pCurrentPos--;

			while (*pCurrentPos != ';')
			{
				pCurrentPos--;
			}

			if (!pal_SubString(pCurrentPos + 1, (u_char *)"phone-context"))
			{
				error = _EcrioSigMgrParseTelParams(pSigMgr, &pLocalNum->numTelPar1, &pLocalNum->ppTelPar1, pCurrentPos + 1, pEnd);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					goto Error;
				}
			}
			pCurrentPos = pal_SubString(pCurrentPos + 1, (u_char *)"phone-context");	// start of phone-context
			if (pCurrentPos == NULL)
			{
				error = ECRIO_SIGMGR_INVALID_URI_SCHEME;
				goto Error;
			}
			pCurrentPos += pal_StringLength((u_char *)"phone-context");
			pCurrentPos = _EcrioSigMgrTokenize(pCurrentPos, pEnd, '=');
			/** Boundary checking; If '=' is last char in string, this is invalid URI scheme. */
			if (pCurrentPos == pEnd || pCurrentPos == NULL)
			{
				error = ECRIO_SIGMGR_INVALID_URI_SCHEME;
				goto Error;
			}
			pCurrentPos = _EcrioSigMgrStripLeadingLWS(pCurrentPos + 1, pEnd);// start of phone-context value
			pNumEnd = _EcrioSigMgrTokenize(pCurrentPos, pEnd, ';');
			if (pNumEnd == NULL)// no more param after phone-context, ie, pTelPar2 not exists
			{
				pNumEnd = pEnd;
				numberOnly = Enum_TRUE;
			}
			else// there are params after phone-context, ie, pTelPar2  exists
			{
				numberOnly = Enum_FALSE;
				pNum = _EcrioSigMgrStripLeadingLWS(pNumEnd + 1, pEnd);	// pNum points to the start of telPar2
				pNumEnd = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pNumEnd - 1);
			}

			pal_MemoryAllocate(sizeof(EcrioSigMgrTelContextStruct), (void **)&pLocalNum->pContext);
			if (pLocalNum->pContext == NULL)
			{
				goto Error;
			}

			if (*pCurrentPos >= '0' && *pCurrentPos <= '9')
			{
				pLocalNum->pContext->contextType = EcrioSigMgrTelContextGlobalNumDigits;
			}
			else
			{
				pLocalNum->pContext->contextType = EcrioSigMgrTelContextDomainName;
			}

			_EcrioSigMgrStringNCreate(pSigMgr, pCurrentPos, &pLocalNum->pContext->u.pDomainName, (u_int32)(pNumEnd - pCurrentPos + 1));	// same allocation will work for both enum

			if (numberOnly == Enum_FALSE)	// telPar2 exists
			{
				error = _EcrioSigMgrParseTelParams(pSigMgr, &pLocalNum->numTelPar2, &pLocalNum->ppTelPar2, pNum, pEnd);
				if (error != ECRIO_SIGMGR_NO_ERROR)
				{
					goto Error;
				}
			}
		}
		else// phone-context not found, only pTelPar1 present
		{
			error = _EcrioSigMgrParseTelParams(pSigMgr, &pLocalNum->numTelPar1, &pLocalNum->ppTelPar1, pNumEnd, pEnd);
			if (error != ECRIO_SIGMGR_NO_ERROR)
			{
				goto Error;
			}
		}
	}
	*ppOutUrl = pTelUrl;
	goto End;
Error:
	if (NULL != pTelUrl)
	{
		if (pTelUrl->subscriberType == EcrioSigMgrTelSubscriberGlobal && pTelUrl->u.pGlobalNumber != NULL)
		{
			if (pTelUrl->u.pGlobalNumber->pGlobalNumberDigits != NULL)
			{
				pal_MemoryFree((void**)&pTelUrl->u.pGlobalNumber->pGlobalNumberDigits);
			}
			pal_MemoryFree((void**)&pTelUrl->u.pGlobalNumber);
		}

		if (pTelUrl->subscriberType == EcrioSigMgrTelSubscriberLocal && pTelUrl->u.pLocalNumber != NULL)
		{
			if (pTelUrl->u.pLocalNumber->pLocalNumberDigits != NULL)
			{
				pal_MemoryFree((void**)&pTelUrl->u.pLocalNumber->pLocalNumberDigits);
			}

			if (pTelUrl->u.pLocalNumber->pContext != NULL)
			{
				if (pTelUrl->u.pLocalNumber->pContext->u.pDomainName != NULL)
				{
					pal_MemoryFree((void**)&pTelUrl->u.pLocalNumber->pContext->u.pDomainName);
				}
				pal_MemoryFree((void**)&pTelUrl->u.pLocalNumber->pContext);
			}

			if (pTelUrl->u.pLocalNumber->ppTelPar1 != NULL)
			{
				for (int i = 0; i < pTelUrl->u.pLocalNumber->numTelPar1; i++)
				{
					if (pTelUrl->u.pLocalNumber->ppTelPar1[i]->telParType == EcrioSigMgrTelParExtension && 
						pTelUrl->u.pLocalNumber->ppTelPar1[i]->u.pExtension != NULL)
					{
						pal_MemoryFree((void**)&pTelUrl->u.pLocalNumber->ppTelPar1[i]->u.pExtension);
					}
					else if (pTelUrl->u.pLocalNumber->ppTelPar1[i]->telParType == EcrioSigMgrTelParIsdnSubAddress && 
						pTelUrl->u.pLocalNumber->ppTelPar1[i]->u.pIsdnSubaddress != NULL)
					{
						pal_MemoryFree((void**)&pTelUrl->u.pLocalNumber->ppTelPar1[i]->u.pIsdnSubaddress);
					}
					else if (pTelUrl->u.pLocalNumber->ppTelPar1[i]->telParType == EcrioSigMgrTelParParameter &&
						pTelUrl->u.pLocalNumber->ppTelPar1[i]->u.pParameter != NULL)
					{
						if (pTelUrl->u.pLocalNumber->ppTelPar1[i]->u.pParameter->pParamName != NULL)
						{
							pal_MemoryFree((void**)&pTelUrl->u.pLocalNumber->ppTelPar1[i]->u.pParameter->pParamName);
						}

						if (pTelUrl->u.pLocalNumber->ppTelPar1[i]->u.pParameter->pParamValue != NULL)
						{
							pal_MemoryFree((void**)&pTelUrl->u.pLocalNumber->ppTelPar1[i]->u.pParameter->pParamValue);
						}
						pal_MemoryFree((void**)&pTelUrl->u.pLocalNumber->ppTelPar1[i]->u.pParameter);
					}
					pal_MemoryFree((void**)&pTelUrl->u.pLocalNumber->ppTelPar1[i]);
				}
				pal_MemoryFree((void**)&pTelUrl->u.pLocalNumber->ppTelPar1);
		}
			pal_MemoryFree((void**)&pTelUrl->u.pLocalNumber);
		}
		pal_MemoryFree((void **)&pTelUrl);
	}
End:
	return error;
}

/****************************************************************************
** FUNCTION: _EcrioSigMgrParseSipUrl
** DESCRIPTION: This function parses the SipURL
**
** PARAMETERS:
**				pStart(IN)		- The starting point in the input buffer
**								  from where this parser can start parsing.
**				pEnd(IN)		- The position in the input buffer till
**								  which this parser should parse.
**				ppOutUrl(OUT)	- The SipUrl structure with relevan values
**								  filled.
**				pErr(OUT		- Error value if any
**
** NOTE:
****************************************************************************/
u_int32 _EcrioSigMgrParseSipUrl
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipURIStruct **ppOutUrl,
	u_char *pStart,
	u_char *pEnd
)
{
	EcrioSigMgrSipURIStruct	*tmpUrl = NULL;
	u_char *tmp;
	u_char *pCurrentPos = NULL;
	BoolEnum Flag = Enum_FALSE;
	BoolEnum userinfoPresent = Enum_FALSE;
	u_char *pEndUserinfo = NULL;
	u_char *pHost = NULL;
	u_int16 *pPort = NULL;

	pStart = _EcrioSigMgrStripLeadingLWS(pStart, pEnd);
	pEnd = _EcrioSigMgrStripTrailingLWS(pStart, pEnd);

	pal_MemoryAllocate(sizeof(EcrioSigMgrSipURIStruct), (void **)&tmpUrl);
	if (tmpUrl == NULL)
	{
		goto error;
	}

	/*
	* First Check for  headers
	*/
	pEndUserinfo = _EcrioSigMgrTokenize(pStart, pEnd, '@');

	/*
	* First Check for  headers
	*/
	if (pEndUserinfo == NULL)
	{
		tmp = _EcrioSigMgrTokenize(pStart, pEnd, '?');
	}
	else
	{
		tmp = _EcrioSigMgrTokenize(pEndUserinfo, pEnd, '?');
		userinfoPresent = Enum_TRUE;
	}

	/** Boundary checking; If '@' or '?' is last char in string, this is invalid header value. */
	if (pEndUserinfo == pEnd || tmp == pEnd)
	{
		goto error;
	}

	if (tmp == NULL)
	{
		/*	This Case Headers are absent
		*This means headers are absent
		*/
		tmpUrl->numURIHeaders = 0;
		tmpUrl->ppURIHeaders = NULL;

		/*Now look for parameters and fill the host and port part
		*/
		if (userinfoPresent == Enum_TRUE)
		{
			pCurrentPos = _EcrioSigMgrTokenize(pEndUserinfo, pEnd, ';');
		}
		else
		{
			pCurrentPos = _EcrioSigMgrTokenize(pStart, pEnd, ';');
		}

		if (pCurrentPos == NULL)
		{
			/*Call Host port Parserc with pStart and pEnd
			*/
			if (_EcrioSigMgrParseUserAndHostPort(pSigMgr, pStart, \
				pEnd, &(tmpUrl->pUserId), &(tmpUrl->pPassword), \
				&pHost, (u_int16 **)&pPort))
			{
				goto error;
			}

			if (pPort != NULL)
			{
				if ((*pPort > 0) && (tmpUrl->pIPAddr == NULL))
				{
					pal_MemoryAllocate(sizeof(EcrioSigMgrIPAddrStruct), (void **)&tmpUrl->pIPAddr);
					if (tmpUrl->pIPAddr == NULL)
					{
						goto error;
					}

					tmpUrl->pIPAddr->pIPAddr = pHost;
					pHost = NULL;
					tmpUrl->pIPAddr->port = *pPort;
					pal_MemoryFree((void **)&pPort);
				}
				else
				{
					if (NULL != pHost)
					{
						pal_MemoryFree((void **)&pHost);
					}

					pal_MemoryFree((void **)&pPort);
				}
			}
			else
			{
				tmpUrl->pDomain = pHost;
				pHost = NULL;
			}
		}
		/** Boundary checking; If ';' is last char in string, this is invalid header value. */
		else if (pCurrentPos == pEnd)
		{
			goto error;
		}
		else
		{
			/*Call Host Port Parser with pStart and pCurrentPos
			*Call sipparam parser with pCurrentPos(START) and pEnd(END)
			*/

			if (_EcrioSigMgrParseUserAndHostPort(pSigMgr, pStart, pCurrentPos - 1, \
				&(tmpUrl->pUserId), &(tmpUrl->pPassword), \
				&pHost, (u_int16 **)&pPort))
			{
				goto error;
			}

			if (pPort != NULL)
			{
				if ((*pPort > 0) && (tmpUrl->pIPAddr == NULL))
				{
					pal_MemoryAllocate(sizeof(EcrioSigMgrIPAddrStruct), (void **)&tmpUrl->pIPAddr);
					if (tmpUrl->pIPAddr == NULL)
					{
						goto error;
					}

					tmpUrl->pIPAddr->pIPAddr = pHost;
					pHost = NULL;
					tmpUrl->pIPAddr->port = *pPort;
					pal_MemoryFree((void **)&pPort);
				}
				else
				{
					if (NULL != pHost)
					{
						pal_MemoryFree((void **)&pHost);
					}

					pal_MemoryFree((void **)&pPort);
				}
			}
			else
			{
				tmpUrl->pDomain = pHost;
				pHost = NULL;
			}

			if (_EcrioSigMgrParseSipParam(pSigMgr, &(tmpUrl->numURIParams), &(tmpUrl->ppURIParams), \
				pCurrentPos + 1, pEnd, Flag, ';'))
			{
				goto  error;
			}
		}
	}
	else
	{
		/*This Case Headers are present if headers are present fill headers
		*/
		if (_EcrioSigMgrParseSipParam(pSigMgr, &(tmpUrl->numURIHeaders), &(tmpUrl->ppURIHeaders), \
			tmp + 1, pEnd, Flag, '?'))
		{
			goto  error;
		}

		/*Now look for parameters and fill the host and port part
		*/
		if (userinfoPresent == Enum_TRUE)
		{
			pCurrentPos = _EcrioSigMgrTokenize(pEndUserinfo, tmp - 1, ';');
		}
		else
		{
			pCurrentPos = _EcrioSigMgrTokenize(pStart, tmp - 1, ';');
		}

		if (pCurrentPos == NULL)
		{
			/*Call Host port Parser with pStart and pEnd
			*/
			if (_EcrioSigMgrParseUserAndHostPort(pSigMgr, pStart, tmp - 1, \
				&(tmpUrl->pUserId), &(tmpUrl->pPassword), \
				&pHost, (u_int16 **)&pPort))
			{
				goto error;
			}

			if (pPort != NULL && *pPort > 0)
			{
				if (tmpUrl->pIPAddr == NULL)
				{
					pal_MemoryAllocate(sizeof(EcrioSigMgrIPAddrStruct), (void **)&tmpUrl->pIPAddr);
					if (tmpUrl->pIPAddr == NULL)
					{
						goto error;
					}

					tmpUrl->pIPAddr->pIPAddr = pHost;
					pHost = NULL;
					tmpUrl->pIPAddr->port = *pPort;
					pal_MemoryFree((void **)&pPort);
					pPort = NULL;
				}
			}
			else
			{
				tmpUrl->pDomain = pHost;
				pHost = NULL;
			}
		}
		/** Boundary checking; If ';' is last char in string, this is invalid header value. */
		else if (pCurrentPos == pEnd)
		{
			goto error;
		}
		else
		{
			/*Call Host Port Parser with pStart and pCurrentPos Call sipparam parser with pCurrentPos(START) and pEnd(END)
			*/

			if (_EcrioSigMgrParseUserAndHostPort(pSigMgr, pStart, pCurrentPos - 1, \
				&(tmpUrl->pUserId), &(tmpUrl->pPassword), \
				&pHost, (u_int16 **)&pPort))
			{
				goto error;
			}

			if (pPort != NULL && *pPort > 0)
			{
				if (tmpUrl->pIPAddr == NULL)
				{
					pal_MemoryAllocate(sizeof(EcrioSigMgrIPAddrStruct), (void **)&tmpUrl->pIPAddr);
					if (tmpUrl->pIPAddr == NULL)
					{
						goto error;
					}

					tmpUrl->pIPAddr->pIPAddr = pHost;
					pHost = NULL;
					tmpUrl->pIPAddr->port = *pPort;
					pal_MemoryFree((void **)&pPort);
					pPort = NULL;
				}
			}
			else
			{
				tmpUrl->pDomain = pHost;
				pHost = NULL;
			}

			/*
			* parse the Url parameters
			*/
			if (_EcrioSigMgrParseSipParam(pSigMgr, &(tmpUrl->numURIParams), &(tmpUrl->ppURIParams), \
				pCurrentPos + 1, pEnd - pal_StringLength((const u_char*)tmp), Flag, ';'))
			{
				goto  error;
			}
		}
	}

	if (NULL != pPort)
	{
		pal_MemoryFree((void **)&pPort);
	}
	*ppOutUrl = tmpUrl;
	return ECRIO_SIGMGR_NO_ERROR;

error:
	if (NULL != tmpUrl)
	{
		if (NULL != tmpUrl->pDomain)
		{
			pal_MemoryFree((void **)&tmpUrl->pDomain);
		}
		if (NULL != tmpUrl->pIPAddr)
		{
			if (tmpUrl->pIPAddr->pIPAddr != NULL)
			{
				pal_MemoryFree((void **)&tmpUrl->pIPAddr->pIPAddr);
			}
			pal_MemoryFree((void **)&tmpUrl->pIPAddr);
		}
		if (NULL != tmpUrl->pUserId)
		{
			pal_MemoryFree((void **)&tmpUrl->pUserId);
		}
		if (NULL != tmpUrl->pPassword)
		{
			pal_MemoryFree((void **)&tmpUrl->pPassword);
		}

		if (NULL != tmpUrl->ppURIHeaders)
		{
			u_int16 i = 0;
			for (i = 0; i < tmpUrl->numURIHeaders; ++i)
			{
				_EcrioSigMgrReleaseParamsStruct(pSigMgr, tmpUrl->ppURIHeaders[i]);
				pal_MemoryFree((void **)&(tmpUrl->ppURIHeaders[i]));
			}
			pal_MemoryFree((void **)&tmpUrl->ppURIHeaders);
		}
		pal_MemoryFree((void **)&tmpUrl);
	}
	if (NULL != pHost)
	{
		pal_MemoryFree((void **)&pHost);
	}
	if (NULL != pPort)
	{
		pal_MemoryFree((void **)&pPort);
	}

	return ECRIO_SIGMGR_INVALID_URI_SCHEME;
}

/****************************************************************************
** FUNCTION: _EcrioSigMgrParseAddrSpec
** DESCRIPTION: This function parses the Addrspec
**
** PARAMETERS:
**				pStart(IN)		- The starting point in the input buffer
**								  from where this parser can start parsing.
**				pEnd(IN)		- The position in the input buffer till
**								  which this parser should parse.
**				ppOutAddr(OUT)	- The SipAddrSpec structure with the relevant
**								  headers filled.
**				pErr(OUT		- Error value if any
**
** NOTE:
****************************************************************************/
u_int32 _EcrioSigMgrParseAddrSpec
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSigMgrUriStruct *pOutAddr
)
{
	pStart = _EcrioSigMgrStripLeadingLWS(pStart, pEnd);
	pEnd = _EcrioSigMgrStripTrailingLWS(pStart, pEnd);

	/*
	* Checking to find the input is a SipUrl or not
	*/
	if (pal_StringNCompare(pStart, (u_char *)"sip:", 4) == 0)
	{
		pStart = pStart + 4;
		if (pStart > pEnd)
		{
			goto error;
		}

		/*
		* Now Call SipURL Parser
		*/
		if (_EcrioSigMgrParseSipUrl(pSigMgr, &(pOutAddr->u.pSipUri), pStart, pEnd))
		{
			goto error;
		}
		else
		{
			pOutAddr->uriScheme = EcrioSigMgrURISchemeSIP;
		}
	}
	else if (pal_StringNCompare(pStart, (u_char *)"tel:", 4) == 0)
	{
		pStart = pStart + 4;
		if (pStart > pEnd)
		{
			goto error;
		}

		/*
		* Now Call TelUri Parser
		*/
		if (_EcrioSigMgrParseTelUrl(pSigMgr, &(pOutAddr->u.pTelUri), pStart, pEnd))
		{
			goto error;
		}
		else
		{
			pOutAddr->uriScheme = EcrioSigMgrURISchemeTEL;
		}
	}
	else
	{
		/*
		* Comming here means input is a uri and storethe uri
		*/
		pOutAddr->uriScheme = EcrioSigMgrURISchemeCustom;
		_EcrioSigMgrStringNCreate(pSigMgr, pStart, &pOutAddr->u.pAbsUri, (pEnd - pStart + 1));
	}

	return ECRIO_SIGMGR_NO_ERROR;

error:

	return ECRIO_SIGMGR_INVALID_URI_SCHEME;
}

/****************************************************************************
** FUNCTION: _EcrioSigMgrParseSipVersion
** DESCRIPTION: This function parses the SIP Version field.
**
** PARAMETERS:
**				pStart(IN)		- The starting point in the input buffer
**								  from where this parser can start parsing.
**				pEnd(IN)		- The position in the input buffer till
**								  which this parser should parse.
**				ppVersion(OUT)	- The parsed SIP Version field.
**				pErr(OUT)		- Error value if any
**
** NOTE:
****************************************************************************/
u_int32	_EcrioSigMgrParseSipVersion
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	u_char **ppVersion
)
{
	/* Remove leading and trailing LWS before validating
	* the version.
	*/

	pStart = _EcrioSigMgrStripLeadingLWS(pStart, pEnd);
	pEnd = _EcrioSigMgrStripTrailingLWS(pStart, pEnd);

	if ((*pEnd == '/') || \
		(0 != pal_StringNICompare(pStart, (u_char *)"SIP/", \
		pal_StringLength((u_char *)"SIP/"))))
	{
		/*
		* Unsupported/Unknown version
		*/

		return ECRIO_SIGMGR_IMS_LIB_ERROR;
	}

	/*
	* Store Version field
	*/

	_EcrioSigMgrStringNCreate(pSigMgr, pStart, ppVersion, (pEnd - pStart + 1));

	return ECRIO_SIGMGR_NO_ERROR;
}

/****************************************************************************
** FUNCTION: _EcrioSigMgrParseStatusLine
** DESCRIPTION: This function parses the Status Line.
**
** PARAMETERS:
**				pStart(IN)			- The starting point in the input buffer
**								      from where this parser can start
**									  parsing.
**				pEnd(IN)			- The position in the input buffer till
**								      which this parser should parse.
**				ppStatusLine(OUT)	- A SipStatusLine structure with the
**									  status line filled.
**				pErr(OUT)			- Error value if any
**
** NOTE:
****************************************************************************/
u_int32	_EcrioSigMgrParseStatusLine
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSigMgrSipMessageStruct *pSipMessage
)
{
	u_char *pCurrentPos = pStart;
	u_char *pToken = NULL;
	u_char *pTempToken = NULL;

	/*
	* Skip leading LWS
	*/

	/*pCurrentPos = _EcrioSigMgrStripLeadingLWS(pCurrentPos, pEnd);*/

	/*
	* Now pCurrentPos points to a non-LWS character
	*/

	/*
	* Look for Version
	*/

	pToken = _EcrioSigMgrTokenize(pCurrentPos, pEnd, ' ');
	/** Boundary checking */
	if (NULL == pToken || pToken == pEnd)
	{
		/*
		* Incomplete status line
		*/

		return ECRIO_SIGMGR_IMS_LIB_ERROR;
	}

	/*
	* Validate version
	*/

	if (_EcrioSigMgrParseSipVersion(pSigMgr, pCurrentPos, pToken - 1, &pSipMessage->pSipVersion))
	{
		return ECRIO_SIGMGR_IMS_LIB_ERROR;
	}

	pCurrentPos = pToken + 1;

	/*
	* Look for status-code
	*/

	pToken = _EcrioSigMgrTokenize(pCurrentPos, pEnd, ' ');
	if (NULL == pToken)
	{
		pTempToken = pEnd;
	}
	/** Boundary checking */
	else if (pToken == pEnd)
	{
		/*
		* Incomplete status line
		*/

		return ECRIO_SIGMGR_IMS_LIB_ERROR;
	}
	else
	{
		pTempToken = pToken - 1;
	}

	/*
	* Validate status-code
	*/
	pSipMessage->responseCode = pal_StringConvertToUNum(pCurrentPos, &pTempToken, 10);

	if (pToken == NULL)
	{
		/* reason phrase is missing*/
		return ECRIO_SIGMGR_NO_ERROR;
	}

	pCurrentPos = pToken + 1;	/* -mustfreefresh flag warning */
	/*
	* Store reason phrase, if present
	*/

	pCurrentPos = _EcrioSigMgrStripLeadingLWS(pCurrentPos, pEnd);	/* -mustfreefresh flag warning */
	pEnd = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pEnd);			/* -mustfreefresh flag warning */

	if (pCurrentPos <= pEnd)
	{
		/*
		* Reason-phrase present
		*/
		_EcrioSigMgrStringNCreate(pSigMgr, pCurrentPos, &pSipMessage->pReasonPhrase, (pEnd - pCurrentPos + 1));
	}

	pSipMessage->eReqRspType = EcrioSigMgrSIPResponse;

	return ECRIO_SIGMGR_NO_ERROR;
}

/****************************************************************************
** FUNCTION: _EcrioSigMgrParseRequestLine
** DESCRIPTION: This function parses the Request Line.
**
** PARAMETERS:
**				pStart(IN)		- The starting point in the input buffer
**								  from where this parser can start parsing.
**				pEnd(IN)		- The position in the input buffer till
**								  which this parser should parse.
**				ppReqLine(OUT)	- A SipReqLine structure with the request
**								  line filled.
**				pErr(OUT)		- Error value if any
**
** NOTE:
****************************************************************************/
u_int32	_EcrioSigMgrParseRequestLine
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd, \
	EcrioSigMgrSipMessageStruct *pSipMessage
)
{
	u_char *pCurrentPos = pStart;
	u_char *pToken;
	u_char *pTemp;
	u_char *pTempMethod;
	u_char *pMethod = NULL;
	u_int32	temperror = 0;

	/*
	* Skip leading LWS
	*/

	/*pCurrentPos = _EcrioSigMgrStripLeadingLWS(pCurrentPos, pEnd);*/

	/*
	* Now pCurrentPos points to a non-LWS character
	*/

	/*
	* Look for Method
	*/

	pToken = _EcrioSigMgrTokenize(pCurrentPos, pEnd, ' ');
	if (NULL == pToken)
	{
		/*
		* Incomplete request line
		*/
		/** To prevent OOB reads, the following are not considered. */
#if 0
		/* To take care of only methodname without end-gap e.g INVITE\r\n*/
		pToken = _EcrioSigMgrTokenize(pCurrentPos, pEnd + 1, '\r');
		if (NULL == pToken)
		{
			return ECRIO_SIGMGR_IMS_LIB_ERROR;
		}
#endif
		return ECRIO_SIGMGR_IMS_LIB_ERROR;
	}
	/** Boundary checking */
	else if (pToken == pEnd)
	{
		/*
		* Incomplete request line
		*/
		return ECRIO_SIGMGR_IMS_LIB_ERROR;
	}

	/* To validate Method name, to check it is not header
	* when requestline missing */
	/* eg, 'From:' will be failed here,as it cannnot be MethodName */
	pTempMethod = pToken;
	if (*(pTempMethod - 1) == ':')
	{
		return ECRIO_SIGMGR_IMS_LIB_ERROR;
	}

	/* eg, 'From   :' will be failed here,as it cannnot be MethodName */

	while (pTempMethod <= pEnd)
	{
		if (*pTempMethod == ':')
		{
			temperror = ECRIO_SIGMGR_IMS_LIB_ERROR;
			/*if request line problem then 400 bad has to be formed*/
		}
		else if (*pTempMethod != ' ')
		{
			break;
		}

		pTempMethod++;
	}

	/*
	* Skip trailing LWS and store Method
	*/

	pTemp = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pToken - 1);
	_EcrioSigMgrStringNCreate(pSigMgr, pCurrentPos, &pMethod, (pTemp - pCurrentPos + 1));
	if (NULL == pMethod)
	{
		return ECRIO_SIGMGR_IMS_LIB_ERROR;
	}

	pSipMessage->eMethodType = _EcrioSigMgrGetMethodTypeFromString(pMethod);

	pal_MemoryFree((void **)&pMethod);

	pCurrentPos = pToken + 1;
	pCurrentPos = _EcrioSigMgrStripLeadingLWS(pCurrentPos, pEnd);

	/*
	* Look for Request-URI
	*/
	/*  Remove trailing LWS */
	pTemp = _EcrioSigMgrStripTrailingLWS(pCurrentPos, pEnd);
	pEnd = pTemp;

	pTemp = _EcrioSigMgrTokenize(pCurrentPos, pEnd, ' ');

	/** Boundary checking */
	if (NULL == pTemp || pTemp == pEnd)
	{
		/*
		* Incomplete request line
		*/
		return ECRIO_SIGMGR_IMS_LIB_ERROR;
	}

	/*
	* Request-URI found; invoke addr-spec parser
	*/
	if (*pCurrentPos == '<')
	{
		pCurrentPos++;
		/** Boundary checking */
		if (pCurrentPos > pEnd)
		{
			/*
			* Incomplete request line
			*/
			return ECRIO_SIGMGR_IMS_LIB_ERROR;
		}

		pTemp = _EcrioSigMgrTokenize(pCurrentPos, pEnd, '>');
		/** Boundary checking */
		if (!pTemp || pTemp == pEnd)
		{
			return ECRIO_SIGMGR_IMS_LIB_ERROR;
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

	if (pSipMessage->pMandatoryHdrs->pRequestUri == NULL)
	{
		pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&pSipMessage->pMandatoryHdrs->pRequestUri);
		if (pSipMessage->pMandatoryHdrs->pRequestUri == NULL)
		{
			return ECRIO_SIGMGR_NO_MEMORY;
		}
	}

	if (_EcrioSigMgrParseAddrSpec(pSigMgr, pCurrentPos, pTemp - 1,
		pSipMessage->pMandatoryHdrs->pRequestUri))
	{
		return ECRIO_SIGMGR_IMS_LIB_ERROR;
	}

	pCurrentPos = pToken + 1;

	/*
	* Process SIP Version field
	*/

	if (_EcrioSigMgrParseSipVersion(pSigMgr, pTemp + 1, pEnd,
		&(pSipMessage->pSipVersion)))
	{
		/*
		* Validation of SIP Version failed;
		* raise parser error.
		*/

		return ECRIO_SIGMGR_IMS_LIB_ERROR;
	}

	if (temperror != ECRIO_SIGMGR_NO_ERROR)
	{
		return ECRIO_SIGMGR_IMS_LIB_ERROR;
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

/****************************************************************************
** FUNCTION: _EcrioSigMgrHandleIncParser
** DESCRIPTION: This function calls the function sip_parseHeaderBody and
**              also handles incremental parsing
**
** PARAMETERS:
**				ppSipMessage(OUT)- The SipMessage Structure
**				ppOutHdr(OUT)	- The SipHeader Structure
**				pStart(In)		- The  begin Index
**				pEnd(IN)		- The  end index
**				pContext(OUT)	- The Event Context Structure ,Which contains
**                                 the header List
**				en_HeaderType	- SipHeaderType
**				mimeHdrParsing	- Flag indicating whether this function is being
**								  for parsing MIME Headers.
**				pMimeHdrs		- The poiner to the MimeHeader structure.
**				pErr(OUT)		- Error value if any
**
** NOTE:
****************************************************************************/
u_int32 _EcrioSigMgrHandleIncParser
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSigMgrSipMessageStruct *pSipMessage,
	EcrioSipHeaderTypeEnum dHdrType,
	BoolEnum mimeHdrParsing,
	u_char *pHNameBegin,
	u_char *pHNameEnd
)
{
	// @todo This function needs to be refactored if these are not needed.
	(void)mimeHdrParsing;
	(void)pHNameBegin;
	(void)pHNameEnd;

//	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
//		__FUNCTION__, __LINE__);

	switch (dHdrType)
	{
		/*General Headers*/
		case EcrioSipHeaderTypeCallId:
		case EcrioSipHeaderTypeAccept:
		case EcrioSipHeaderTypeTimestamp:
		case EcrioSipHeaderTypeRetryAfter:
		case EcrioSipHeaderTypeProxyRequire:
		case EcrioSipHeaderTypeRAck:
		case EcrioSipHeaderTypeRequire:
		case EcrioSipHeaderTypeRSeq:
		case EcrioSipHeaderTypeSubscriptionState:
		case EcrioSipHeaderTypeSupported:
		case EcrioSipHeaderTypeUnsupported:
		case EcrioSipHeaderTypeUserAgent:
		case EcrioSipHeaderTypeAllow:
		case EcrioSipHeaderTypeErrorInfo:
		case EcrioSipHeaderTypeServer:
		case EcrioSipHeaderTypeDate:
		case EcrioSipHeaderTypePrivacy:
		case EcrioSipHeaderTypeContentType:
		case EcrioSipHeaderTypeContentID:
		case EcrioSipHeaderTypeContentDisposition:
		case EcrioSipHeaderTypeContentEncoding:
		case EcrioSipHeaderTypeContentTransferEncoding:
		case EcrioSipHeaderTypeAcceptContact:
		case EcrioSipHeaderTypeContributionID:
		case EcrioSipHeaderTypeConversationID:
		case EcrioSipHeaderTypeInReplyToContributionID:
		case EcrioSipHeaderTypeP_PreferredService:
		case EcrioSipHeaderTypeSessionExpires:
		case EcrioSipHeaderTypePath:
		case EcrioSipHeaderTypeSipEtag:
		case EcrioSipHeaderTypeP_EarlyMedia:
		case EcrioSipHeaderTypeReferTo:
		case EcrioSipHeaderTypeReferSub:
		case EcrioSipHeaderTypeSubject:
		case EcrioSipHeaderTypeEvent:
		case EcrioSipHeaderTypeAllowEvents:
		case EcrioSipHeaderTypeMinSE:
		{
			if (_EcrioSigMgrParseStringHeader(pSigMgr, pStart, pEnd, dHdrType, pSipMessage))
			{
				goto error;
			}

			break;
		}

		case EcrioSipHeaderTypeReason:
		{
			if (_EcrioSigMgrParseReasonHeader(pSigMgr, pStart, pEnd, pSipMessage))
			{
				goto error;
			}

			break;
		}

		case EcrioSipHeaderTypeContact:
		{
			if (_EcrioSigMgrParseContactHeader(pSigMgr, pStart, pEnd, &pSipMessage->pContact))
			{
				goto error;
			}

			break;
		}

		case EcrioSipHeaderTypeCSeq:
		{
			if (_EcrioSigMgrParseCSeqHeader(pSigMgr, pStart, pEnd, pSipMessage))
			{
				goto error;
			}

			break;
		}

		case EcrioSipHeaderTypeFrom:
		{
			if (pSipMessage->pMandatoryHdrs->pFrom == NULL)
			{
				pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct), (void **)&pSipMessage->pMandatoryHdrs->pFrom);
				if (pSipMessage->pMandatoryHdrs->pFrom == NULL)
				{
					goto error;
				}
			}
			else
			{
				goto error;
			}

			if (_EcrioSigMgrParseNameAddrWithParams(pSigMgr, pStart, pEnd, pSipMessage->pMandatoryHdrs->pFrom))
			{
				goto error;
			}

			break;
		}

		case EcrioSipHeaderTypeTo:
		{
			if (pSipMessage->pMandatoryHdrs->pTo == NULL)
			{
				pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct), (void **)&pSipMessage->pMandatoryHdrs->pTo);
				if (pSipMessage->pMandatoryHdrs->pTo == NULL)
				{
					goto error;
				}
			}
			else
			{
				goto error;
			}

			if (_EcrioSigMgrParseNameAddrWithParams(pSigMgr, pStart, pEnd, pSipMessage->pMandatoryHdrs->pTo))
			{
				goto error;
			}

			break;
		}

		case EcrioSipHeaderTypeRecordRoute:
		{
			_EcrioSigMgrParseRouteHeader(pSigMgr, pStart, pEnd, &pSipMessage->pRecordRouteSet);
			break;
		}

		case EcrioSipHeaderTypeVia:
		{
			if (pSipMessage->pMandatoryHdrs->ppVia == NULL)
			{
				pal_MemoryAllocate(sizeof(EcrioSigMgrViaStruct *), (void **)&pSipMessage->pMandatoryHdrs->ppVia);
				if (pSipMessage->pMandatoryHdrs->ppVia == NULL)
				{
					goto error;
				}
			}
			else
			{
				/* Check arithmetic overflow - do not use the ArithmeticOverflowDetected() API for u_int16 variable. */
				if (pSipMessage->pMandatoryHdrs->numVia + 1 > USHRT_MAX)
				{
					goto error;
				}

				/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
				if (pal_UtilityDataOverflowDetected((pSipMessage->pMandatoryHdrs->numVia + 1), sizeof(EcrioSigMgrViaStruct *)) == Enum_TRUE)
				{
					goto error;
				}

				pal_MemoryReallocate(sizeof(EcrioSigMgrViaStruct *) * (pSipMessage->pMandatoryHdrs->numVia + 1), \
					(void **)&pSipMessage->pMandatoryHdrs->ppVia);
				if (pSipMessage->pMandatoryHdrs->ppVia == NULL)
				{
					goto error;
				}
			}

			pal_MemoryAllocate(sizeof(EcrioSigMgrViaStruct), (void **)&pSipMessage->pMandatoryHdrs->ppVia[pSipMessage->pMandatoryHdrs->numVia]);
			if (pSipMessage->pMandatoryHdrs->ppVia[pSipMessage->pMandatoryHdrs->numVia] == NULL)
			{
				goto error;
			}

			if (_EcrioSigMgrParseViaHeader(pSigMgr, pStart, pEnd, pSipMessage->pMandatoryHdrs->ppVia[pSipMessage->pMandatoryHdrs->numVia]))
			{
				/* in case error this would be leaked otherwise */
				pSipMessage->pMandatoryHdrs->numVia++;
				goto error;
			}

			pSipMessage->pMandatoryHdrs->numVia++;
			break;
		}

		case EcrioSipHeaderTypeP_AssociatedUri:
		{
			if (pSipMessage->ppPAssociatedURI == NULL)
			{
				pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct *), (void **)&pSipMessage->ppPAssociatedURI);
				if (pSipMessage->ppPAssociatedURI == NULL)
				{
					goto error;
				}
			}
			else
			{
				/* Check arithmetic overflow - do not use the ArithmeticOverflowDetected() API for u_int16 variable. */
				if (pSipMessage->numPAssociatedURIs + 1 > USHRT_MAX)
				{
					goto error;
				}

				/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
				if (pal_UtilityDataOverflowDetected((pSipMessage->numPAssociatedURIs + 1), sizeof(EcrioSigMgrNameAddrWithParamsStruct *)) == Enum_TRUE)
				{
					goto error;
				}

				pal_MemoryReallocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct *) * (pSipMessage->numPAssociatedURIs + 1), \
					(void **)&pSipMessage->ppPAssociatedURI);
				if (pSipMessage->ppPAssociatedURI == NULL)
				{
					goto error;
				}
			}

			pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct), (void **)&pSipMessage->ppPAssociatedURI[pSipMessage->numPAssociatedURIs]);
			if (pSipMessage->ppPAssociatedURI[pSipMessage->numPAssociatedURIs] == NULL)
			{
				goto error;
			}

			if (_EcrioSigMgrParseNameAddrWithParams(pSigMgr, pStart, pEnd, pSipMessage->ppPAssociatedURI[pSipMessage->numPAssociatedURIs]))
			{
				pal_MemoryFree((void **)&pSipMessage->ppPAssociatedURI[pSipMessage->numPAssociatedURIs]);
				goto error;
			}

			pSipMessage->numPAssociatedURIs++;
			break;
		}

		case EcrioSipHeaderTypeContentLength:
		case EcrioSipHeaderTypeMaxForward:
		case EcrioSipHeaderTypeMinExpires:
		case EcrioSipHeaderTypeExpires:
		{
			if (_EcrioSigMgrParseContentLengthHeader(pSigMgr, pStart, pEnd, pSipMessage, dHdrType))
			{
				goto error;
			}

			break;
		}

		case EcrioSipHeaderTypeWWWAuthenticate:
		{
			if (_EcrioSigMgrParseAuthHeaders(pSigMgr, pStart, pEnd, &pSipMessage->pAuthentication))
			{
				goto error;
			}

			break;
		}

		case EcrioSipHeaderTypeRoute:
		{
			_EcrioSigMgrParseRouteHeader(pSigMgr, pStart, pEnd, &pSipMessage->pRouteSet);

			break;
		}

		case EcrioSipHeaderTypeServiceRoute:
		{
			_EcrioSigMgrParseRouteHeader(pSigMgr, pStart, pEnd, &pSipMessage->pServiceRoute);

			break;
		}

		case EcrioSipHeaderTypeAuthenticationInfo:
		{
			_EcrioSigMgrParseAuthenticationInfoHeaders(pSigMgr, pStart, pEnd, &pSipMessage->pAuthenticationInfo);
			break;
		}

		case EcrioSipHeaderTypeP_AssertedIdentity:
		{
			if (pSipMessage->ppPAssertedIdentity == NULL)
			{
				pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrStruct *), (void **)&pSipMessage->ppPAssertedIdentity);
				if (pSipMessage->ppPAssertedIdentity == NULL)
				{
					goto error;
				}
			}
			else
			{
				/* Check arithmetic overflow - do not use the ArithmeticOverflowDetected() API for u_int16 variable. */
				if (pSipMessage->numPAssertedIdentities + 1 > USHRT_MAX)
				{
					goto error;
				}

				/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
				if (pal_UtilityDataOverflowDetected((pSipMessage->numPAssertedIdentities + 1), sizeof(EcrioSigMgrNameAddrStruct *)) == Enum_TRUE)
				{
					goto error;
				}

				pal_MemoryReallocate(sizeof(EcrioSigMgrNameAddrStruct *) * (pSipMessage->numPAssertedIdentities + 1), \
					(void **)&pSipMessage->ppPAssertedIdentity);
				if (pSipMessage->ppPAssertedIdentity == NULL)
				{
					goto error;
				}
			}

			pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrStruct), (void **)&pSipMessage->ppPAssertedIdentity[pSipMessage->numPAssertedIdentities]);
			if (pSipMessage->ppPAssertedIdentity[pSipMessage->numPAssertedIdentities] == NULL)
			{
				goto error;
			}

			if (_EcrioSigMgrParseNameAddr(pSigMgr, pStart, pEnd, pSipMessage->ppPAssertedIdentity[pSipMessage->numPAssertedIdentities]))
			{
				pal_MemoryFree((void **)&pSipMessage->ppPAssertedIdentity[pSipMessage->numPAssertedIdentities]);
				goto error;
			}

			pSipMessage->numPAssertedIdentities++;
			break;
		}

		case EcrioSipHeaderTypeReferredBy:
		{
			if (pSipMessage->ppReferredBy == NULL)
			{
				pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct *), (void **)&pSipMessage->ppReferredBy);
				if (pSipMessage->ppReferredBy == NULL)
				{
					goto error;
				}
			}
			else
			{
				/* Check arithmetic overflow - do not use the ArithmeticOverflowDetected() API for u_int16 variable. */
				if (pSipMessage->numReferredBy + 1 > USHRT_MAX)
				{
					goto error;
				}

				/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
				if (pal_UtilityDataOverflowDetected((pSipMessage->numReferredBy + 1), sizeof(EcrioSigMgrNameAddrWithParamsStruct *)) == Enum_TRUE)
				{
					goto error;
				}

				pal_MemoryReallocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct *) * (pSipMessage->numReferredBy + 1), \
					(void **)&pSipMessage->ppReferredBy);
				if (pSipMessage->ppReferredBy == NULL)
				{
					goto error;
				}
			}

			pal_MemoryAllocate(sizeof(EcrioSigMgrNameAddrWithParamsStruct), (void **)&pSipMessage->ppReferredBy[pSipMessage->numReferredBy]);
			if (pSipMessage->ppReferredBy[pSipMessage->numReferredBy] == NULL)
			{
				goto error;
			}

			if (_EcrioSigMgrParseNameAddrWithParams(pSigMgr, pStart, pEnd, pSipMessage->ppReferredBy[pSipMessage->numReferredBy]))
			{
				pal_MemoryFree((void **)&pSipMessage->ppReferredBy[pSipMessage->numReferredBy]);
				goto error;
			}

			pSipMessage->numReferredBy++;
			break;
		}

		case EcrioSipHeaderTypeSecurityClient:
		case EcrioSipHeaderTypeSecurityServer:
		case EcrioSipHeaderTypeSecurityVerify:
		{
			EcrioSigMgrHeaderStruct *pHeader = NULL;
			u_char *pTempLWS;
			u_int32 size = 0;
			u_char *pToken = NULL;

			if (pSipMessage->pOptionalHeaderList == NULL)
			{
				EcrioSigMgrCreateOptionalHeaderList(pSigMgr, &pSipMessage->pOptionalHeaderList);
				if (pSipMessage->pOptionalHeaderList == NULL)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrCreateOptionalHeaderList() returned NULL",
						__FUNCTION__, __LINE__);
					goto error;
				}
			}
			if (_EcrioSigMgrParseOptionalHeaders(pSigMgr, pStart, pEnd, &pHeader))
			{
				goto error;
			}
			pToken = _EcrioSigMgrTokenizeQuotedString(pHNameBegin, pEnd, ':');
			pTempLWS = _EcrioSigMgrStripTrailingLWS(pHNameBegin, pToken - 1);
			size = (u_int32)(pTempLWS - pHNameBegin + 1);
			if (size > 0)
			{
				/*
				* header value is present
				*/
				_EcrioSigMgrStringNCreate(pSigMgr, pHNameBegin, &pHeader->pHeaderName, size);
			}
			else
			{
				pHeader->pHeaderName = NULL;
			}
			pHeader->eHdrType = dHdrType;
			if (_EcrioSigMgrInsertOptionalHeader(pSigMgr, pSipMessage->pOptionalHeaderList, pHeader))
			{
				goto error;
			}
			break;
		}
		case EcrioSipHeaderTypeWarning:
		{
			EcrioSigMgrHeaderStruct *pHeader = NULL;
			u_char *pTempLWS;
			u_int32 size = 0;
			u_char *pToken = NULL;

			if (pSipMessage->pOptionalHeaderList == NULL)
			{
				EcrioSigMgrCreateOptionalHeaderList(pSigMgr, &pSipMessage->pOptionalHeaderList);
				if (pSipMessage->pOptionalHeaderList == NULL)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSigMgrCreateOptionalHeaderList() returned NULL",
						__FUNCTION__, __LINE__);
					goto error;
				}
			}
			if (_EcrioSigMgrParseWarningHeader(pSigMgr, pStart, pEnd, &pHeader))
			{
				goto error;
			}
			pToken = _EcrioSigMgrTokenizeQuotedString(pHNameBegin, pEnd, ':');
			pTempLWS = _EcrioSigMgrStripTrailingLWS(pHNameBegin, pToken - 1);
			size = (u_int32)(pTempLWS - pHNameBegin + 1);
			if (size > 0)
			{
				/*
				* header value is present
				*/
				_EcrioSigMgrStringNCreate(pSigMgr, pHNameBegin, &pHeader->pHeaderName, size);
			}
			else
			{
				pHeader->pHeaderName = NULL;
			}
			pHeader->eHdrType = dHdrType;
			if (_EcrioSigMgrInsertOptionalHeader(pSigMgr, pSipMessage->pOptionalHeaderList, pHeader))
			{
				goto error;
			}
			break;
		}

		default:;
	}/* End of Switch Case .*/

	/*
	* For Failure cases.pHeader would have already been freed in side parse
	* header body
	*/

//	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
//		__FUNCTION__, __LINE__, ECRIO_SIGMGR_NO_ERROR);

	return ECRIO_SIGMGR_NO_ERROR;
error:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, ECRIO_SIGMGR_IMS_LIB_ERROR);

	return ECRIO_SIGMGR_IMS_LIB_ERROR;
}

/****************************************************************************
** FUNCTION :		_EcrioSigMgrParseFirstLine
**
** DESCRIPTION:	This function parses the first line of SIP message. If this
**					API succeeds, returns the pointer indicates to the next
**					to the end of first line. Otherwise, return NULL and set
**					appropriate error to pError.
**
** PARAMETERS:
**		pStart(IN)			- The starting point in the input buffer
**							  from where this parser can start
**							  parsing. This pointer should point to
**							  the beginning of the SIP message.
**		pEnd(IN)			- The position in the input buffer till
**							  which this parser should parse.
**		pSipMessage(IN)		- The pointer to the EcrioSigMgrSipMessageStruct.
**		pError(OUT)			- Error value if any
**
****************************************************************************/
u_char*	_EcrioSigMgrParseFirstLine
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSigMgrSipMessageStruct *pSipMessage,
	u_int32 *pError
)
{
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;
	u_char *pCurrentPos = NULL;
	u_char *pBeginIndex = NULL;
	u_char *pEndIndex = NULL;

	/** Check for input parameters */
	if (NULL == pSigMgr || NULL == pStart || NULL == pEnd ||
		NULL == pSipMessage || NULL == pError)
	{
		return NULL;
	}

	pCurrentPos = pStart;

	/* pBeginIndex is the begining of Request/StatusLine. */
	pBeginIndex = pCurrentPos;

	while (*pCurrentPos != '\r' && *pCurrentPos != '\n')
	{
		pCurrentPos++;
		if (pCurrentPos >= pEnd)
		{
			uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
			pCurrentPos = NULL;
			goto End;
		}

		/* Removal of line folding in request/StatusLine starts from here. */
		if (*pCurrentPos == '\r' &&
			(_EcrioSigMgrIsAccessWithinRange(pCurrentPos, pEnd, 1) == Enum_TRUE))
		{
			if (*(pCurrentPos + 1) == '\n' &&
				(_EcrioSigMgrIsAccessWithinRange(pCurrentPos, pEnd, 2) == Enum_TRUE) &&
				(*(pCurrentPos + 2) == ' ' || *(pCurrentPos + 2) == '\t'))
			{
				*pCurrentPos = ' ';
				*(pCurrentPos + 1) = ' ';
			}
			else if (*(pCurrentPos + 1) == ' ' || *(pCurrentPos + 1) == '\t')
			{
				*pCurrentPos = ' ';
			}
		}

		if (*pCurrentPos == '\n' && 
			(_EcrioSigMgrIsAccessWithinRange(pCurrentPos, pEnd, 1) == Enum_TRUE))
		{
			if (*(pCurrentPos + 1) == ' ' || *(pCurrentPos + 1) == '\t')
			{
				*pCurrentPos = ' ';
			}
		}
	}

	/* pEndIndex is the end of Request/StatusLine. */
	pEndIndex = pCurrentPos;

	/* Here finding the exact beginning of next header: The end Index/pCurrentPos
	 * has to be incremented to the beginning of the next header. Make both same
	 *  INVITE sip:xyz@hss.hns.com SIP/2.0\r\n
	 *    It may be ended by a CR only
	 *  INVITE sip:xyz@hss.hns.com SIP/2.0\r
	 *    It may be ended by a LF only
	 *  INVITE sip:xyz@hss.hns.com SIP/2.0\n
	 *    So increase the pointer accordingly to bypass the CRLF\CR\LF */
	if (_EcrioSigMgrIsAccessWithinRange(pCurrentPos, pEnd, 1) != Enum_TRUE)
	{
		uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
		pCurrentPos = NULL;
		goto End;
	}

	if (*pCurrentPos == '\r' && *(pCurrentPos + 1) == '\n')
	{
		/* SipFrag message body may contains extra CRLF at the end of body. */
		while (*pCurrentPos == '\r' && *(pCurrentPos + 1) == '\n')
		{
			pCurrentPos = pCurrentPos + 2;
			if (_EcrioSigMgrIsAccessWithinRange(pCurrentPos, pEnd, 0) != Enum_TRUE)
			{
				uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
				pCurrentPos = NULL;
				goto End;
			}
		}
	}
	else
	{
		/* If the status line or the request line contains an improper line
		 * fold loop until
		 * Example:
		 *   1) SIP/2.0 401 Unauthorised\r
		 *   2) SIP/2.0 401 Unauthorised\r\r
		 *   3) SIP/2.0 401 Unauthorised\r\n\n etc
		 *   4) Request Lines also.
		 */
		pCurrentPos = pCurrentPos + 1;

		while (*pCurrentPos == '\r' || *pCurrentPos == '\n')
		{
			pCurrentPos = pCurrentPos + 1;
			if (_EcrioSigMgrIsAccessWithinRange(pCurrentPos, pEnd, 0) != Enum_TRUE)
			{
				uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
				pCurrentPos = NULL;
				goto End;
			}
		}
	}

	/* Now pCurrentPos is pointing to the beginning of first header.
	 * Now we have bypassed \r\n (OR \r OR \n) of SIP/2.0\r\n in the
	 * above example. */

	/* Check for line folding. If line folding is there, raise error. */
	if (pCurrentPos + 1 >= pEnd)
	{
		uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
		pCurrentPos = NULL;
		goto End;
	}

	if (*(pCurrentPos + 1) == '\t' || *(pCurrentPos + 1) == ' ')
	{
		uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
		pCurrentPos = NULL;
		goto End;
	}

	/* Now initialize response type SipMessage structure. */
	if (pal_StringNICompare(pBeginIndex, (u_char *)"sip", 3) == 0)
	{
		/* Now call the StatusLine Line parser. */
		if (_EcrioSigMgrParseStatusLine(pSigMgr, pBeginIndex, pEndIndex - 1, pSipMessage))
		{
			uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
			pCurrentPos = NULL;
			goto End;
		}

		/* Handle possible sipfrag issues. */
		if (pCurrentPos >= pEnd)
		{
			uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
			pCurrentPos = NULL;
			goto End;
		}
	}
	/* Else for Request Line */
	else
	{
		/* Now call Request Line Parser. */
		if (_EcrioSigMgrParseRequestLine(pSigMgr, pBeginIndex, pEndIndex - 1, pSipMessage))
		{
			uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
			pCurrentPos = NULL;
			goto End;
		}
	}

End:
	*pError = uError;

	return pCurrentPos;
}

/****************************************************************************
** FUNCTION :		_EcrioSigMgrParseSipHeaderPart
**
** DESCRIPTION:	This function parses SIP header part of SIP message. If this
**				API succeeds, returns the pointer indicates to the next pointer
**				at the end of SIP header part. Note that if SIP message does
**				not contain a body and not NULL-terminated, return value will
**				indicate the position that is out of bounds of the input buffer.
**				Otherwise, return NULL and set the appropriate error to pError.
**
** PARAMETERS:
**		pStart(IN)			- The starting point in the input buffer
**							  from where this parser can start
**							  parsing. This pointer should point to
**							  the beginning of the SIP header part of
**							  SIP message.
**		pEnd(IN)			- The position in the input buffer till
**							  which this parser should parse.
**		pSipMessage(IN)		- The pointer to the EcrioSigMgrSipMessageStruct.
**		pError(OUT)			- Error value if any
**
****************************************************************************/
u_char*	_EcrioSigMgrParseSipHeaderPart
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSigMgrSipMessageStruct *pSipMessage,
	u_int32 *pError
)
{
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;
	u_int32 lengthOfHeader = 0;
	u_int32 quotesFound = 0;
	u_int32 angleFound = 0;
	u_int32 bracketFound = 0;
	u_int32 bracketCount = 0;
	u_int32 angleCount = 0;
	u_char *pCurrentPos = NULL;
	u_char *pBodyBeginIndex = NULL;
	u_char *pBodyEndIndex = NULL;
	u_char *pNextLineBeginIndex = NULL;
	u_char *pHeaderNameBeginIndex = NULL;
	u_char *pHeaderNameEndIndex = NULL;
	u_char *pName = NULL;
	HeaderParsingStateEnum headerParsingState;
	EcrioSipHeaderTypeEnum dType = EcrioSipHeaderTypeNone;
	BoolEnum isAuth = Enum_FALSE;
	BoolEnum exitCondition = Enum_FALSE;
	BoolEnum skipCommaSaparator = Enum_FALSE;
	BoolEnum bContinueMainLoop = Enum_FALSE;

	/** Check for input parameters */
	if (NULL == pSigMgr || NULL == pStart || NULL == pEnd ||
		NULL == pSipMessage || NULL == pError)
	{
		return NULL;
	}

	/* Now start parsing headers. This is in a do-while loop. */
	headerParsingState = en_headerNameExpected;

	pCurrentPos = pStart;

	do
	{
		bContinueMainLoop = Enum_FALSE;

		/* Check if current buffer position still within range */
		if (_EcrioSigMgrIsAccessWithinRange(pCurrentPos, pEnd, 0) != Enum_TRUE)
		{
			uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
			pCurrentPos = NULL;
			goto End;
		}

		if (headerParsingState == en_headerNameExpected)
		{
			/* Resetting the variables. */
			pNextLineBeginIndex = NULL;
			pHeaderNameBeginIndex = pCurrentPos;

			/* Here trying to find the end of the header name. */
			while (pCurrentPos <= pEnd)
			{
				/* If find a ':', this means the end of the header name. so break */
				if (*pCurrentPos == ':')
				{
					break;
				}

				/* If find a line fold handle properly. */
				if (*pCurrentPos == '\r' && 
					(_EcrioSigMgrIsAccessWithinRange(pCurrentPos, pEnd, 1) == Enum_TRUE))
				{
					if (*(pCurrentPos + 1) == '\n' &&
						(_EcrioSigMgrIsAccessWithinRange(pCurrentPos, pEnd, 2) == Enum_TRUE) &&
						(*(pCurrentPos + 2) == ' ' || *(pCurrentPos + 2) == '\t'))
					{
						/* Replace line folds with white spaces. */
						*pCurrentPos = ' ';
						*(pCurrentPos + 1) = ' ';
					}
					else if (*(pCurrentPos + 1) == ' ' || *(pCurrentPos + 1) == '\t')
					{
						/* Replace line folds with white spaces. */
						*pCurrentPos = ' ';
					}
					else if (*(pCurrentPos + 1) == '\n')
					{
						pHeaderNameBeginIndex = pCurrentPos + 2;
					}
				}

				if (*pCurrentPos == '\n' && 
					(_EcrioSigMgrIsAccessWithinRange(pCurrentPos, pEnd, 1) == Enum_TRUE))
				{
					if (*(pCurrentPos + 1) == ' ' || *(pCurrentPos + 1) == '\t')
					{
						/* Replace line folds with white spaces. */
						*pCurrentPos = ' ';
					}
				}

				pCurrentPos++;
			}

			pHeaderNameEndIndex = pCurrentPos - 1;
			pHeaderNameBeginIndex = _EcrioSigMgrStripLeadingLWS(pHeaderNameBeginIndex, pHeaderNameEndIndex);
			pHeaderNameEndIndex = _EcrioSigMgrStripTrailingLWS(pHeaderNameBeginIndex, pHeaderNameEndIndex);
			lengthOfHeader = (u_int32)(pHeaderNameEndIndex - pHeaderNameBeginIndex + 1);
			_EcrioSigMgrStringNCreate(pSigMgr, pHeaderNameBeginIndex, &pName, lengthOfHeader);
			if (NULL == pName)
			{
				uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
				pCurrentPos = NULL;
				goto End;
			}

			if (pName != NULL)
			{
				dType = _EcrioSigMgrGetHeaderTypeFromString(pName);
			}

			quotesFound = 0;
			bracketFound = 0;
			bracketCount = 0;
			angleCount = 0;
			angleFound = 0;
			skipCommaSaparator = Enum_FALSE;
			if (pal_StringICompare(pName, (u_char *)"Privacy") == 0 ||
				pal_StringICompare(pName, (u_char *)"To") == 0 || pal_StringICompare(pName, (u_char *)"t") == 0 ||
				pal_StringICompare(pName, (u_char *)"From") == 0 || pal_StringICompare(pName, (u_char *)"f") == 0 ||
				pal_StringICompare(pName, (u_char *)"Refer-To") == 0 || pal_StringICompare(pName, (u_char *)"r") == 0 ||
				pal_StringICompare(pName, (u_char *)"Subject") == 0 || pal_StringICompare(pName, (u_char *)"s") == 0 ||
				pal_StringICompare(pName, (u_char *)"Date") == 0)
			{
				skipCommaSaparator = Enum_TRUE;
			}

			pal_MemoryFree((void **)&pName);

			if (/*dType == SipHdrTypeProxyAuthenticate ||
				dType == SipHdrTypeProxyauthorization ||*/
				dType == EcrioSipHeaderTypeAuthenticationInfo ||
				dType == EcrioSipHeaderTypeWWWAuthenticate ||
				dType == EcrioSipHeaderTypeAuthorization)
			{
				isAuth = Enum_TRUE;
			}
			else
			{
				isAuth = Enum_FALSE;
			}

			/* Now changing the state to parsing the header body. */
			headerParsingState = en_headerBodyExpected;

			/* if Header Type is SipUnknown then we need to pass the name and
			 * value to the parser. */

			/* Bypassing COLON */
			pCurrentPos++;
			pBodyBeginIndex = pCurrentPos;
			pBodyEndIndex = pCurrentPos;
		}

		/* Check if current buffer position still within range */
		if (_EcrioSigMgrIsAccessWithinRange(pBodyEndIndex, pEnd, 0) != Enum_TRUE)
		{
			uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
			pCurrentPos = NULL;
			goto End;
		}

		if (headerParsingState == en_headerBodyExpected)
		{
			if (_EcrioSigMgrIsAccessWithinRange(pBodyEndIndex, pEnd, 0) != Enum_TRUE)
			{
				uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
				pCurrentPos = NULL;
				goto End;
			}

			while (*pBodyEndIndex != ',' &&
				   *pBodyEndIndex != '\r' &&
				   *pBodyEndIndex != '\n' &&
				   *pBodyEndIndex != '"' &&
				   *pBodyEndIndex != '<' &&
				   *pBodyEndIndex != '>' &&
				   *pBodyEndIndex != '(' &&
				   *pBodyEndIndex != ')')
			{
				pBodyEndIndex++;
				if (_EcrioSigMgrIsAccessWithinRange(pBodyEndIndex, pEnd, 0) != Enum_TRUE)
				{
					uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
					pCurrentPos = NULL;
					goto End;
				}
			}

			/* Given below is the checking for buffer overflow. This single checking
			 * is done to reduce code size. This is because if *pBodyEndIndex is '"'
			 * or ')' or '(' we need to increment pBodyEndIndex* by one to continue
			 * the parsing. So we can do this checking at one palce instead of doing
			 * at each place where we increment pBodyEndIndex for checking for comments
			 * and quoted string. */
			if (*pBodyEndIndex == '"' ||
				*pBodyEndIndex == '<' ||
				*pBodyEndIndex == '>' ||
				*pBodyEndIndex == ')' ||
				*pBodyEndIndex == '(')
			{
				/* Given below is the checking. If buffer over flow is there return
				 * fail by freeing the resources. */
				if (_EcrioSigMgrIsAccessWithinRange(pBodyEndIndex, pEnd, 1) != Enum_TRUE)
				{
					uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
					pCurrentPos = NULL;
					goto End;
				}
			}

			if (*pBodyEndIndex == ',')
			{
				/* If already found a quote or bracket, this is not a comma for
				 * separating headers, but a comma inside a comment or a quoted
				 * string. So continue. */
				if ((quotesFound == 1) || (bracketFound == 1) || angleFound == 1 || skipCommaSaparator == Enum_TRUE)
				{
					pBodyEndIndex++;
					continue;
				}
			}

			if (*pBodyEndIndex == '"')
			{
				if (bracketFound == 0)
				{
					/* If the state of bracket found = 1, this means that trying
					 * to find the closing paranthesis. So if bracketFound is one
					 * and it comes here it may be a case like above. So we need
					 * to continue with out any processinf. */

					/* If already found a quote and if again finding one again
					 * toggle the state. */
					if (quotesFound == Enum_TRUE)
					{
						quotesFound = 0;
						pBodyEndIndex++;
						continue;
					}

					/* If find a quote then continue until next quote. */
					quotesFound = 1;
					pBodyEndIndex++;
					continue;
				}

				/* If in the state of bracket found, continue */
				pBodyEndIndex++;
				continue;
			}

			if (*pBodyEndIndex == '(')
			{
				if (quotesFound == Enum_FALSE)
				{
					/* Now in the state of bracketFound. Now we need to find the
					 * end of the comment. We will be in the mission to find the
					 * end of the comment. So we will be checking for a ')'. this
					 * is done by makeing bracket count = 0. At the moment when we
					 * find that bracket count is zero this means the comment has
					 * ended. Then we will give the buffer to the Header Parser. */

					/* Now we set the state to bracket found */
					bracketFound = 1;

					/* Increment the bracket count */
					bracketCount++;

					/* Now continue to find the end of the comment */
					pBodyEndIndex++;
					continue;
				}

				/* If in quotes found state and getting a ')' don't consider it.
				 * Need to ignore it can continue until the quotesFound becomes zero. */
				pBodyEndIndex++;
				continue;
			}

			if (*pBodyEndIndex == ')')
			{
				if (quotesFound == 0)
				{
					/* If find a ')' decrement the bracket count. If bracket count is zero,
					 * reset the state bracketFound to 0. This means we have reached the
					 * end of the comment. */
					bracketCount--;
					if (bracketCount == Enum_FALSE)
					{
						/* If baracket count is zero reset the state. */
						bracketFound = 0;
					}

					/* Any case, continue */
					pBodyEndIndex++;
					continue;
				}

				/* If in quotes found state, and getting a '(' don't consider it.
				 * Need to ignore it can continue until the quotesFound becomes zero. */
				pBodyEndIndex++;
				continue;
			}

			/* Here adding support for checking comma between '<' and '>'. */
			if (*pBodyEndIndex == '<')
			{
				if (angleFound == 0)
				{
					/* Now in the state of angleFound. Now we need to find the
					 * end of the uri. We will be in the mission to find the
					 * end of URI. So we will be checking for a '>'. this is done
					 * by makeing angle count = 0. At the moment when we find
					 * that angle count is zero this means the comment has ended.
					 * Then we will give the buffer to the Header Parser. */

					/* Now we set the state to bracket found */
					angleFound = 1;

					/* Increment the bracket count */
					angleCount++;

					/* Now continue to find the end of the URI */
					pBodyEndIndex++;
					continue;
				}

				/* If in quotes found state, and getting a '>' don't consider it.
				 * Need to ignore it can continue until the quotesFound becomes zero. */
				pBodyEndIndex++;
				continue;
			}

			if (*pBodyEndIndex == '>')
			{
				if (quotesFound == 0)
				{
					/* If find a '>' decrement the angle count. If angle count is zero,
					 * reset the state angleFound to 0. This means we have reached the
					 * end of the comment. */
					angleCount--;
					if (angleCount == Enum_FALSE)
					{
						/* If angle count is zero, reset the state */
						angleFound = 0;
					}

					/* Any case, continue */
					pBodyEndIndex++;
					continue;
				}

				/* If in quotes found state, and getting a '>' don't consider it.
				 * Need to ignore it can continue until the quotesFound becomes zero. */
				pBodyEndIndex++;
				continue;
			}

			/* Support for '<' and '>' ends here. */

			/* Here pCurrentPos is the position after the first occurance of CR\LF.
			 * To start the next\header line we need to increment pCurrentPos. */
			pCurrentPos = pBodyEndIndex;
			if (pNextLineBeginIndex != NULL)
			{
				pBodyBeginIndex = pNextLineBeginIndex;
			}

			/* The following means the header is not an auth header but a comma
			 * separated one. */
			if ((isAuth == Enum_FALSE) && (*pBodyEndIndex) == ',')
			{
				/* Here first comma separated header is parsed. We must not forgot
				 * that a header will always end a CR or LF. So the last Header in
				 * the comma separated series will be handled at the end. */

				/*  header,\r\n */
				while (pBodyEndIndex < pEnd)
				{
					/* Check if current buffer position still within range */
					if (_EcrioSigMgrIsAccessWithinRange(pBodyEndIndex, pEnd, 0) != Enum_TRUE)
					{
						uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
						pCurrentPos = NULL;
						goto End;
					}

					if (*pBodyEndIndex == ' ' || *pBodyEndIndex == '\t')
					{
						while (pBodyEndIndex < pEnd &&
							(*pBodyEndIndex == ' ' || *pBodyEndIndex == '\t'))
						{
							pBodyEndIndex++;
						}
						continue;
					}
					else if (*pBodyEndIndex == ',')
					{
						*pBodyEndIndex = ' ';
						pBodyEndIndex++;
						continue;
					}
					else if (*pBodyEndIndex == '\r' &&
						(_EcrioSigMgrIsAccessWithinRange(pBodyEndIndex, pEnd, 1) == Enum_TRUE))
					{
						if (pBodyEndIndex + 1 < pEnd)
						{
							if (*(pBodyEndIndex + 1) == '\n' &&
								(_EcrioSigMgrIsAccessWithinRange(pBodyEndIndex, pEnd, 2) == Enum_TRUE))
							{
								if ((pBodyEndIndex + 2) < pEnd)
								{
									if (*(pBodyEndIndex + 2) == ' ' || *(pBodyEndIndex + 2) == '\t')
									{
										*pBodyEndIndex = ' ';
										*(pBodyEndIndex + 1) = ' ';
										pBodyEndIndex += 2;
										continue;
									}
									else
									{
										bContinueMainLoop = Enum_TRUE;
										break;
									}
								}
								else
								{
									bContinueMainLoop = Enum_TRUE;
									break;
								}
							}
							else
							{
								if (*(pBodyEndIndex + 1) == ' ' || *(pBodyEndIndex + 1) == '\t')
								{
									*pBodyEndIndex = ' ';
									pBodyEndIndex += 1;
									continue;
								}
								else
								{
									bContinueMainLoop = Enum_TRUE;
									break;
								}
							}
						}
						else
						{
							bContinueMainLoop = Enum_TRUE;
							break;
						}
					}
					else if (*pBodyEndIndex == '\n' &&
						(_EcrioSigMgrIsAccessWithinRange(pBodyEndIndex, pEnd, 1) == Enum_TRUE))
					{
						if ((pBodyEndIndex + 1) < pEnd)
						{
							if (*(pBodyEndIndex + 1) == ' ' || *(pBodyEndIndex + 1) == '\t')
							{
								*pBodyEndIndex = ' ';
								pBodyEndIndex += 1;
								continue;
							}
							else
							{
								bContinueMainLoop = Enum_TRUE;
								break;
							}
						}
						else
						{
							bContinueMainLoop = Enum_TRUE;
							break;
						}
					}
					else
					{
						pBodyEndIndex -= 1;
						break;
					}
				}

				if (pBodyEndIndex == pEnd)
				{
					bContinueMainLoop = Enum_TRUE;
				}

				if (bContinueMainLoop == Enum_TRUE)
				{
					continue;
				}

				if (_EcrioSigMgrHandleIncParser(
					pSigMgr,
					pBodyBeginIndex,
					pBodyEndIndex - 1,
					pSipMessage,
					dType,
					Enum_FALSE,
					pHeaderNameBeginIndex,
					pHeaderNameEndIndex))
				{
					uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
					pCurrentPos = NULL;
					goto End;
				}
				else
				{
					/* Need to bypass the comma */
					pBodyEndIndex++;

					/* Assign pBodyBeginIndex to pBodyEndIndex to the beginning
					 * of next header. */
					pBodyBeginIndex = pBodyEndIndex;
					continue;
				}
			}

			if (isAuth == Enum_TRUE && (*pBodyEndIndex) == ',')
			{
				pBodyEndIndex++;
				continue;
			}

			/* From here checking of line fold starts. Check of CR with/without
			 * line folding. */
			if (*pBodyEndIndex == '\r' &&
				(_EcrioSigMgrIsAccessWithinRange(pBodyEndIndex, pEnd, 2) == Enum_TRUE))
			{
				/* Check for CRLF followed by SP/TAB. */
				if (*(pBodyEndIndex + 1) == '\n' &&
					((*(pBodyEndIndex + 2) == ' ' || *(pBodyEndIndex + 2) == '\t')))
				{
					/* Replace line folding with white space */
					*pBodyEndIndex = ' ';
					*(pBodyEndIndex + 1) = ' ';

					/* Incrementing pBodyEndIndex to cross CR+LF. */
					pBodyEndIndex = pBodyEndIndex + 2;
					continue;
				}

				/* Check whether CR followed by LF followed by SPACE or tab */
				if (*(pBodyEndIndex + 1) == ' ' || *(pBodyEndIndex + 1) == '\t')
				{
					/* Replace line folding with white space */
					*pBodyEndIndex = ' ';

					/* Incrementing pBodyEndIndex to cross CR */
					pBodyEndIndex = pBodyEndIndex + 1;
					continue;
				}

				/* Check whether CR followed by CR. */
				if (*(pBodyEndIndex + 1) == '\r')
				{
					/* Replace line folding with white space */
					*pBodyEndIndex = ' ';

					/* Incrementing pBodyEndIndex to cross CR */
					pBodyEndIndex = pBodyEndIndex + 1;
					continue;
				}

				/* If comming here means need to parse the header here.
				 * No line folding is here. Call handle Incremental parsing here. */
				if (_EcrioSigMgrHandleIncParser(
					pSigMgr,
					pBodyBeginIndex,
					pBodyEndIndex - 1,
					pSipMessage,
					dType,
					Enum_FALSE,
					pHeaderNameBeginIndex,
					pHeaderNameEndIndex))
				{
					uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
					pCurrentPos = NULL;
					goto End;
				}
				else
				{
					if (*pBodyEndIndex == '\r' && *(pBodyEndIndex + 1) == '\n')
					{
						/* If CR followed by LF increment pBodyEndIndex by 2 */
						pBodyEndIndex = pBodyEndIndex + 2;

						/* Now header parsed we need to assign the begining of
						 * next header. */
						pCurrentPos = pBodyEndIndex;
					}
					else
					{
						/* Comming here means ended only with CR, increment by one. */
						pBodyEndIndex++;

						/* Now header parsed we need to assign the begining of
						 * next header. */
						pCurrentPos = pBodyEndIndex;
					}
				}
			}
			else if (*pBodyEndIndex == '\n' &&
				(_EcrioSigMgrIsAccessWithinRange(pBodyEndIndex, pEnd, 1) == Enum_TRUE))
			{
				if (*(pBodyEndIndex + 1) == ' ' || *(pBodyEndIndex + 1) == '\t')
				{
					/* Replace line folding with white space */
					*pBodyEndIndex = ' ';

					/* Incrementing pBodyEndIndex to cross LF */
					pBodyEndIndex = pBodyEndIndex + 1;
					continue;
				}

				/* Check whether LF followed by LF. */
				if (*(pBodyEndIndex + 1) == '\n')
				{
					/* Replace line folding with white space */
					*pBodyEndIndex = ' ';

					/* Incrementing pBodyEndIndex to cross CR */
					pBodyEndIndex = pBodyEndIndex + 1;
					continue;
				}

				/* If comming here means need to parse the header here.
				 * No line folding is here. Call handle Incremental parsing here. */
				if (_EcrioSigMgrHandleIncParser(
					pSigMgr,
					pBodyBeginIndex,
					pBodyEndIndex - 1,
					pSipMessage,
					dType,
					Enum_FALSE,
					pHeaderNameBeginIndex,
					pHeaderNameEndIndex))
				{
					uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
					pCurrentPos = NULL;
					goto End;
				}
				else
				{
					/* Comming here means we need to increment pBodyEndIndex by one. */
					pBodyEndIndex++;

					/* Starting point of next header is assigned to pCurrentPos */
					pCurrentPos = pBodyEndIndex;
				}
			}
			else if (*pBodyEndIndex == '\0')
			{
				/* If coming here means need to parse the header here.
				 * No line folding is here. Call handle Incremental parsing here. */
				if (_EcrioSigMgrHandleIncParser(
					pSigMgr,
					pBodyBeginIndex,
					pBodyEndIndex - 1,
					pSipMessage,
					dType,
					Enum_FALSE,
					pHeaderNameBeginIndex,
					pHeaderNameEndIndex))
				{
					uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
					pCurrentPos = NULL;
					goto End;
				}
				else
				{
					/* Starting point of next header is assigned to pCurrentPos */
					pCurrentPos = pBodyEndIndex;
				}
			}

			/* Here check for exit condition */
			if ((*pBodyEndIndex == '\r' && *(pBodyEndIndex + 1) == '\n') || 
				*pBodyEndIndex == '\n' ||
				*pBodyEndIndex == '\0' ||
				*pBodyEndIndex == '\r')
			{
				exitCondition = Enum_TRUE;
			}

			/* Now change the state to header name expected */
			headerParsingState = en_headerNameExpected;
		}
	} while (exitCondition == Enum_FALSE);

	/* Check if current buffer position still within range */
	if (_EcrioSigMgrIsAccessWithinRange(pCurrentPos, pEnd, 1) != Enum_TRUE)
	{
		uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
		pCurrentPos = NULL;
		goto End;
	}

	if (*pCurrentPos == '\r' && *(pCurrentPos + 1) == '\n')
	{
		pCurrentPos = pCurrentPos + 2;
		/* Making the code to support any number of CRLF's at the end & return
		 * NO ERROR as per requirement. */
	}
	else if (*pCurrentPos == '\n')
	{
		pCurrentPos = pCurrentPos + 1;
	}
	else if (*pCurrentPos == '\r')
	{
		pCurrentPos = pCurrentPos + 1;
	}
	else if (*pCurrentPos == '\0')
	{
		/* No move pCurrentpos */
	}

End:
	*pError = uError;

	return pCurrentPos;
}

/****************************************************************************
** FUNCTION :		_EcrioSigMgrParseMessageBodyPart
**
** DESCRIPTION:	This function parses the message body part of SIP message.
**					If this API succeeds, returns no error. Otherwise, return
**					the error code.
**
** PARAMETERS:
**		pStart(IN)			- The starting point in the input buffer
**							  from where this parser can start
**							  parsing. This pointer should point to
**							  the beginning of the message body part of
**							  SIP message.
**		pEnd(IN)			- The position in the input buffer till
**							  which this parser should parse.
**		pSipMessage(IN)		- The pointer to the EcrioSigMgrSipMessageStruct.
**
****************************************************************************/
u_int32	_EcrioSigMgrParseMessageBodyPart
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSigMgrSipMessageStruct *pSipMessage
)
{
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;
	u_int32 index = 0;
	u_int32 dContentLength = 0;
	u_char *pMessageBodyBegin = NULL;
	u_char *pMessageBodyEnd = NULL;
	u_char *pMessageEnd = NULL;
	u_char *pMediaType = NULL;
	BoolEnum ParseBody = Enum_TRUE;
	EcrioSigMgrUnknownMessageBodyStruct *pUnkwnBody = NULL;
	EcrioSigMgrHeaderStruct *pContentType = NULL;
	EcrioSigMgrMessageBodyStruct *pMessageBody = NULL;

	/** Check for input parameters */
	if (NULL == pSigMgr || NULL == pStart || NULL == pEnd || NULL == pSipMessage)
	{
		return ECRIO_SIGMGR_IMS_LIB_ERROR;
	}

	pMessageBodyBegin = pStart;
	pMessageEnd = pEnd - 1;
	pContentType = pSipMessage->pContentType;

	if (pSipMessage->contentLength == 0)
	{
		/* If the message has no Content-Length header the message body is assumed
		 * to end at the end of the transport packet. */
		if (pMessageEnd < pMessageBodyBegin)
		{
			if (NULL == pSipMessage->pContentType)
			{
				ParseBody = Enum_FALSE;
			}
			else
			{
				uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
				goto End;
			}
		}
		else
		{
			pMessageBodyEnd = pMessageEnd;
		}
	}
	else
	{
		dContentLength = pSipMessage->contentLength;

		/* If content length > 0 but no message body in sip message, do not parse
		 * message body. */
		if (_EcrioSigMgrIsAccessWithinRange(pMessageBodyBegin, pMessageEnd, 0) != Enum_TRUE)
		{
			ParseBody = Enum_FALSE;
		}
		else
		{
			/* Please refer Bis-05 section 19.3 Framing if the transport packet ends
			 * before end of the message body, this is considered an error. */
			if ((pMessageBodyBegin + dContentLength - 1) > pMessageEnd)
			{
				pMessageBodyEnd = pMessageEnd;
			}
			else
			{
				/* If there are additional bytes in the transport packet below 
				 * end of the body, they MUST be discarded. */
				if ((pMessageBodyBegin + dContentLength - 1) <= pMessageEnd)
				{
					pMessageBodyEnd = pMessageBodyBegin + dContentLength - 1;
				}
			}
		}
	}

	if (ParseBody == Enum_TRUE)
	{
		if (pContentType == NULL)
		{
			uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
			goto End;
		}

		pal_MemoryAllocate(sizeof(EcrioSigMgrMessageBodyStruct), (void **)&pSipMessage->pMessageBody);
		if (NULL == pSipMessage->pMessageBody)
		{
			uError = ECRIO_SIGMGR_NO_MEMORY;
			goto End;
		}

		pMessageBody = pSipMessage->pMessageBody;

		if (pContentType->ppHeaderValues &&
			pContentType->ppHeaderValues[0]->pHeaderValue != NULL)
		{
			pMediaType = pContentType->ppHeaderValues[0]->pHeaderValue;

			if (pal_StringNICompare(pMediaType, (u_char *)"application/sdp",
				pal_StringLength((u_char *)"application/sdp")) == 0)
			{
				pMessageBody->messageBodyType = EcrioSigMgrMessageBodySDP;
			}
			else
			{
				pMessageBody->messageBodyType = EcrioSigMgrMessageBodyUnknown;
			}

			pal_MemoryAllocate(sizeof(EcrioSigMgrUnknownMessageBodyStruct), (void **)&pUnkwnBody);
			if (pUnkwnBody == NULL)
			{
				uError = ECRIO_SIGMGR_NO_MEMORY;
				goto Err_Release_MessageBody;
			}

			pMessageBody->pMessageBody = pUnkwnBody;
			pUnkwnBody->bufferLength = (u_int32)(pMessageBodyEnd - pMessageBodyBegin + 1);

			pal_MemoryAllocate(pUnkwnBody->bufferLength + 1, (void **)&pUnkwnBody->pBuffer);
			if (pUnkwnBody->pBuffer == NULL)
			{
				uError = ECRIO_SIGMGR_NO_MEMORY;
				goto Err_Release_UnknownBody;
			}

			pal_MemoryCopy(pUnkwnBody->pBuffer, pUnkwnBody->bufferLength, pMessageBodyBegin, pUnkwnBody->bufferLength);
			pUnkwnBody->contentType.pHeaderValue = pal_StringCreate(pContentType->ppHeaderValues[0]->pHeaderValue,
				pal_StringLength(pContentType->ppHeaderValues[0]->pHeaderValue));
			pUnkwnBody->contentType.numParams = pContentType->ppHeaderValues[0]->numParams;
			if (pUnkwnBody->contentType.numParams > 0)
			{
				u_int32 error = ECRIO_SIGMGR_NO_ERROR;
				/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
				if (pal_UtilityDataOverflowDetected(pContentType->ppHeaderValues[0]->numParams, sizeof(EcrioSigMgrParamStruct*)) == Enum_TRUE)
				{
					_EcrioSigMgrReleaseSipMessage(pSigMgr, pSipMessage);
					pal_MemoryFree((void **)&pSipMessage);
					pSipMessage = NULL;
					pal_MemoryFree((void **)&pUnkwnBody);
					return ECRIO_SIGMGR_NO_MEMORY;
				}

				pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct*) * pContentType->ppHeaderValues[0]->numParams,
					(void **)&pUnkwnBody->contentType.ppParams);
				if (pUnkwnBody->contentType.ppParams == NULL)
				{
					uError = ECRIO_SIGMGR_NO_MEMORY;
					goto Err_Release_UnknownBody;
				}
				for (index = 0; index < pUnkwnBody->contentType.numParams; index++)
				{
					pal_MemoryAllocate(sizeof(EcrioSigMgrParamStruct), (void **)&pUnkwnBody->contentType.ppParams[index]);
					if (pUnkwnBody->contentType.ppParams[index] == NULL)
					{
						uError = ECRIO_SIGMGR_NO_MEMORY;
						goto Err_Release_ContentParams;
					}
					error = _EcrioSigMgrStringCreate(pSigMgr,
						pContentType->ppHeaderValues[0]->ppParams[index]->pParamName,
						&pUnkwnBody->contentType.ppParams[index]->pParamName);
					if (error != ECRIO_SIGMGR_NO_ERROR)
					{
						uError = ECRIO_SIGMGR_NO_MEMORY;
						goto Err_Release_ContentParams;
					}
					error = _EcrioSigMgrStringCreate(pSigMgr,
						pContentType->ppHeaderValues[0]->ppParams[index]->pParamValue,
						&pUnkwnBody->contentType.ppParams[index]->pParamValue);
					if (error != ECRIO_SIGMGR_NO_ERROR)
					{
						uError = ECRIO_SIGMGR_NO_MEMORY;
						goto Err_Release_ContentParams;
					}
				}
			}
			else
			{
				pUnkwnBody->contentType.ppParams = NULL;
			}
		}
	}

	goto End;

Err_Release_ContentParams:
	if (NULL != pUnkwnBody)
	{
		for (index = 0; index < pUnkwnBody->contentType.numParams; index++)
		{
			if (NULL != pUnkwnBody->contentType.ppParams[index])
			{
				if (NULL != pUnkwnBody->contentType.ppParams[index]->pParamName)
				{
					pal_MemoryFree((void**)&pUnkwnBody->contentType.ppParams[index]->pParamName);
				}
				if (NULL != pUnkwnBody->contentType.ppParams[index]->pParamValue)
				{
					pal_MemoryFree((void**)&pUnkwnBody->contentType.ppParams[index]->pParamValue);
				}
				pal_MemoryFree((void**)&pUnkwnBody->contentType.ppParams[index]);
			}
		}
		pal_MemoryFree((void**)&pUnkwnBody->contentType.ppParams);

		if (NULL != pUnkwnBody->contentType.pHeaderValue)
		{
			pal_MemoryFree((void**)&pUnkwnBody->contentType.pHeaderValue);
		}

		if (NULL != pUnkwnBody->pBuffer)
		{
			pal_MemoryFree((void**)&pUnkwnBody->pBuffer);
		}
	}

Err_Release_UnknownBody:
	if (NULL != pUnkwnBody)
	{
		pal_MemoryFree((void **)&pUnkwnBody);
	}

Err_Release_MessageBody:
	if (NULL != pSipMessage->pMessageBody)
	{
		pal_MemoryFree((void **)&pSipMessage->pMessageBody);
	}

End:
	return uError;
}

/****************************************************************************
** FUNCTION :		_EcrioSigMgrSipMessageParse
**
** DESCRIPTION:	This function parses the textual SIP buffer passed to
**					it till a failure is reached or till the next message
**					is encountered. The SipMessage data structure filling
**					is done as a part of this API. sip_decodeMessage()
**					internally invokes this API to perform parsing.
**
**   PARAMETERS:
**			pMessage        (IN)	:The buffer to be parsed
**			ppDecodedMsg	(OUT)	:The filled sipmessage structure
**          pOpt			(IN)	:The runtime options for decoding
**			dMessageLength	(IN)	:The length of message buffer
**          ppNextMsg		(OUT)	:The next message in the buffer
**          pContext		(IN)	:The event Context
**          SipMimeHeaders	(IN)	:to indicate whether this function is
**									  called from message body parser
**          pErr			(IN/OUT): SipError

****************************************************************************/
u_int32 _EcrioSigMgrSipMessageParse
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pMessage,
	EcrioSigMgrSipMessageStruct **ppDecodedMsg,
	u_int32 dMessageLength
)
{
	u_int32 uError = ECRIO_SIGMGR_NO_ERROR;
	u_char *pCurrentPos = NULL;
	u_char *pMessageEnd = NULL;
	EcrioSigMgrSipMessageStruct *pSipMessage = NULL;

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
// quiet		__FUNCTION__, __LINE__);

	/* Check for *ppDecodedMsg being NULL before parsing sip message. At this 
	 * time this function will only parse sip message, so it returns an error
	 * if already allocated *ppDecodedMsg. */
	if (NULL != *ppDecodedMsg)
	{
		uError = ECRIO_SIGMGR_IMS_LIB_ERROR;
		goto End;
	}

	/* Store pointer of begining of the sip message. */
	pCurrentPos = pMessage;
	pMessageEnd = pMessage + dMessageLength;

	/* Remove whitespace at the begining of lines */
	if (pCurrentPos != NULL)
	{
		while (*pCurrentPos == '\r' ||
			   *pCurrentPos == '\n' ||
			   *pCurrentPos == ' '  ||
			   *pCurrentPos == '\t')
		{
			pCurrentPos++;
			if (pCurrentPos >= pMessageEnd)
			{
				return ECRIO_SIGMGR_IMS_LIB_ERROR;
			}
		}
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrSipMessageStruct), (void **)&pSipMessage);
	if (pSipMessage == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tFailed to allocation memory.",
			__FUNCTION__, __LINE__);
		return ECRIO_SIGMGR_NO_MEMORY;
	}

	pal_MemoryAllocate(sizeof(EcrioSigMgrMandatoryHeaderStruct), (void **)&pSipMessage->pMandatoryHdrs);
	if (pSipMessage->pMandatoryHdrs == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tFailed to allocation memory.",
			__FUNCTION__, __LINE__);
		pal_MemoryFree((void **)&pSipMessage);
		return ECRIO_SIGMGR_NO_MEMORY;
	}

	/* Parse the first line */
	pCurrentPos = _EcrioSigMgrParseFirstLine(pSigMgr, pCurrentPos, pMessageEnd, pSipMessage, &uError);
	if (pCurrentPos == NULL || uError != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tFailed to parse first line.",
			__FUNCTION__, __LINE__);
		goto Err_Release_SipMessage;
	}

	/* Parse SIP header part */
	pCurrentPos = _EcrioSigMgrParseSipHeaderPart(pSigMgr, pCurrentPos, pMessageEnd, pSipMessage, &uError);
	if (pCurrentPos == NULL || uError != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tFailed to parse SIP headers.",
			__FUNCTION__, __LINE__);
		goto Err_Release_SipMessage;
	}

	/* Now pCurrentPos points to the beginning of message body part; if message body
	 * does not exist, it points to the next to end of pMessage (i.e. out-of-boundary)
	 * otherwise, it points to the beginning of message body part. */

	/* Parse message body part */
	uError = _EcrioSigMgrParseMessageBodyPart(pSigMgr, pCurrentPos, pMessageEnd, pSipMessage);
	if (uError != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tFailed to parse message body.",
			__FUNCTION__, __LINE__);
		goto Err_Release_SipMessage;
	}

	*ppDecodedMsg = pSipMessage;

	goto End;

Err_Release_SipMessage:
	if (NULL != pSipMessage)
	{
		_EcrioSigMgrReleaseSipMessage(pSigMgr, pSipMessage);
		pal_MemoryFree((void **)&pSipMessage);
	}

End:

// quiet	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
// quiet		__FUNCTION__, __LINE__);

	return uError;
}

/****************************************************************************
** FUNCTION: _EcrioSigMgrIsAccessWithinRange
** DESCRIPTION: This function is used to check the boundary of the buffer pointers
**
** PARAMETERS:
**      	      pStart(IN)		:Beginnning of Buffer
**				  pEnd(IN)			:End of buffer
**				  uJump(IN)			:Number of units to jump
****************************************************************************/
BoolEnum _EcrioSigMgrIsAccessWithinRange
(
	u_char *pStart,
	u_char *pEnd,
	u_int32 uJump
)
{
	if (pStart == NULL || pEnd == NULL)
	{
		return Enum_INVALID;
	}
	else
	{
		if ((pStart + uJump) < pEnd)
		{
			return Enum_TRUE;
		}
		else
		{
			return Enum_FALSE;
		}
	}
}

