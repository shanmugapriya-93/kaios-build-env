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

/**
 * @file EcrioParse.c
 * @brief Implementation of the CPIM module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioCPIM.h"
#include "EcrioCPIMCommon.h"


/* #line 56 "EcrioCPIMParse.c" */
static const int ec_cpim_Parser_start = 1;
static const int ec_cpim_Parser_first_final = 565;
static const int ec_cpim_Parser_error = 0;

static const int ec_cpim_Parser_en_main = 1;


/* #line 673 "EcrioCPIMParse.rl" */



u_int32 ec_cpim_ParseMultiPartMessage
(
	EcrioCPIMStruct *c,
	CPIMMessageStruct *pStruct,
	u_char *pBoundary,
	u_char *pData,
	u_int32 uLen
)
{
	u_int32 uCPIMError = ECRIO_CPIM_NO_ERROR;
	u_int32 i;
	u_char *pCur = NULL, *pStart = NULL, *pEnd = NULL;

	/** Check parameter validity. */
	if (c == NULL)
	{
		return  ECRIO_CPIM_INVALID_HANDLE;
	}

	if (pStruct == NULL)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_2;
	}

	if (pBoundary == NULL)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_3;
	}

	if (pData == NULL)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_4;
	}

	if (uLen == 0)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_5;
	}

	CPIMLOGI(c->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pBoundary[0] == '\0')
	{
		CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tboundary string not specified.", __FUNCTION__, __LINE__);
		uCPIMError = ECRIO_CPIM_PARSING_INSUFFICIENT_ERROR;
		goto END;
	}

	pCur = pData;

	/** Initialize output buffer structure */
	pStruct->uNumOfBuffers = 0;
	pal_MemorySet(&pStruct->buff[0], 0, sizeof(CPIMMessageBufferStruct) * CPIM_MSG_NUM);

	for (i = 0; i < CPIM_MSG_NUM; i++)
	{
		/* 1. Find boundary string */
		pStart = pal_StringFindSubString(pCur, pBoundary);
		if (pStart == NULL)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tboundary not found.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_PARSING_INSUFFICIENT_ERROR;
			goto END;
		}

		/** 2. pStart pointer goes to boundary + crlf, there is head of MIME message body. */
		pStart = pStart + pal_StringLength(pBoundary) + 2;

		if (uLen <= (pStart - pData))
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tInvalid position of pStart.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_PARSING_INSUFFICIENT_ERROR;
			goto END;
		}

		/** 3. Find next boundary, 4 bytes ("--" string + crlf) before of pEnd is tail of MIME message body. */
		pEnd = pal_MemorySearch(pStart, uLen - (pStart - pData), pBoundary, pal_StringLength(pBoundary));

		if (pEnd == NULL)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tboundary not found.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_PARSING_INSUFFICIENT_ERROR;
			goto END;
		}
		pCur = pEnd;
		pEnd -= 4;

		/** 4. Copy MIME message to working buffer. */
		c->work.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->work, (u_int32)(pEnd - pStart));
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tThe working buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->work.pData, 0, c->work.uContainerSize);
		pal_MemoryCopy((void *)c->work.pData, c->work.uContainerSize, (const void *)pStart, (u_int32)(pEnd - pStart));
		c->work.uSize = (u_int32)(pEnd - pStart);

		/** 5. Parse MIME message */
		uCPIMError = ec_cpim_ParseMIMEMessage(c, pStruct, c->work.pData, (pEnd - pStart), i);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tParsing MIME message error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_PARSING_INSUFFICIENT_ERROR;
			goto END;
		}

		/** 6. Check whether the next boundary indicates a term boundary (exists "--" to bottom). */
		if (pal_StringNCompare(pCur + pal_StringLength(pBoundary), (u_char*)"--", 2) == 0)
		{
			CPIMLOGI(c->logHandle, KLogTypeGeneral, "%s:%u\tFound final boundary.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_NO_ERROR;
			break;
		}
	}

END:
	CPIMLOGI(c->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return uCPIMError;
}

u_int32 EcrioCPIMHandler
(
	CPIM_HANDLE handle,
	CPIMMessageStruct *pStruct,
	u_char *pData,
	u_int32 uLen
)
{
	EcrioCPIMStruct *c = NULL;
	u_int32 uCPIMError = ECRIO_CPIM_NO_ERROR;
	u_int32 uCntLen = 0;
	u_int32 uStrLen = 0;
	s_int32 cs = 1;
	const char *p;
	const char *pe;
	const char *eof;
	const char *tag_start = NULL;
	u_char cTemp[8];
	u_char cBoundary[CPIM_BOUNDARY_LENGTH] = { 0 };
	u_char *pBuff = NULL;
	u_char *pContentType = NULL;
	u_char *pContentDisposition = NULL;
	u_char *pContentId = NULL;
	u_char *pContentTransferEncoding = NULL;
	u_char *pAtSign = NULL;
	u_char *pSemiColon = NULL;
	BoolEnum bIsImdnNS = Enum_FALSE;
	BoolEnum bIsMaapNS = Enum_FALSE;
	BoolEnum bIsMyFeaturesNS = Enum_FALSE;
	BoolEnum bIsMultiPart = Enum_FALSE;
	CPIMContentTypeEnum eConType = CPIMContentType_None;

	/** Check parameter validity. */
	if (handle == NULL)
	{
		return  ECRIO_CPIM_INVALID_HANDLE;
	}

	if (pStruct == NULL)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_2;
	}

	if (pData == NULL)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_3;
	}

	if (uLen == 0)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_4;
	}

	/** Set the CPIM handle, c. */
	c = (EcrioCPIMStruct *)handle;

	CPIMLOGI(c->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	CPIM_MUTEX_LOCK(c->mutexAPI, c->logHandle);

	c->strings.uSize = 0;

	p = (char*)pData;
	pe = (char*)pData + uLen;
	eof = pe;

	/** Initialize output structure */
	pal_MemorySet(pStruct, 0, sizeof(CPIMMessageStruct));

	/** Parsing CPIM message by Ragel */
	
/* #line 263 "EcrioCPIMParse.c" */
	{
	cs = ec_cpim_Parser_start;
	}

/* #line 871 "EcrioCPIMParse.rl" */
	
/* #line 270 "EcrioCPIMParse.c" */
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr1;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr0:
/* #line 596 "EcrioCPIMParse.rl" */
	{
		/** This is a general error, but usually no CRLF at the end of line. */
		uCPIMError = ECRIO_CPIM_PARSING_INSUFFICIENT_ERROR;
	}
	goto st0;
/* #line 322 "EcrioCPIMParse.c" */
st0:
cs = 0;
	goto _out;
tr1:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
/* #line 336 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 58: goto st3;
	}
	goto tr0;
tr4:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st3;
tr217:
/* #line 359 "EcrioCPIMParse.rl" */
	{
		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pDateTime,
			(u_char*)tag_start,
			(p - tag_start));
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st3;
tr733:
/* #line 156 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pFromAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st3;
tr293:
/* #line 489 "EcrioCPIMParse.rl" */
	{
		CPIMLOGI(c->logHandle, KLogTypeGeneral, "%s:%u\tIMDN-Record-Route.", __FUNCTION__, __LINE__);
	}
	goto st3;
tr367:
/* #line 381 "EcrioCPIMParse.rl" */
	{
		bIsMyFeaturesNS = Enum_TRUE;
	}
	goto st3;
tr421:
/* #line 87 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pToAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st3;
tr512:
/* #line 402 "EcrioCPIMParse.rl" */
	{
		if (bIsMaapNS == Enum_TRUE)
		{
			if (pal_StringNCompare((u_char*)tag_start, CPIM_ADVERTISEMENT_STRING,
						pal_StringLength(CPIM_ADVERTISEMENT_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Advertisement;
			}
			else if (pal_StringNCompare((u_char*)tag_start, CPIM_PAYMENT_STRING,
						pal_StringLength(CPIM_PAYMENT_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Payment;
			}
			else if (pal_StringNCompare((u_char*)tag_start, CPIM_PREMIUM_STRING,
						pal_StringLength(CPIM_PREMIUM_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Premium;
			}
			else if (pal_StringNCompare((u_char*)tag_start, CPIM_SUBSCRIPTION_STRING,
						pal_StringLength(CPIM_SUBSCRIPTION_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Subscription;
			}
			else if (pal_StringNCompare((u_char*)tag_start, CPIM_PLUGIN_STRING,
						pal_StringLength(CPIM_PLUGIN_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Plugin;
			}
			else
			{
				pStruct->trfType = CPIMTrafficType_None;
			}
		}
	}
	goto st3;
tr547:
/* #line 386 "EcrioCPIMParse.rl" */
	{
		if (bIsImdnNS == Enum_TRUE)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pMsgId,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
			pStruct->uMsgIdLen = pal_StringLength(pStruct->pMsgId);
		}
	}
	goto st3;
tr571:
/* #line 504 "EcrioCPIMParse.rl" */
	{
		CPIMLOGI(c->logHandle, KLogTypeGeneral, "%s:%u\timdn.Original-To.", __FUNCTION__, __LINE__);
	}
	goto st3;
tr633:
/* #line 371 "EcrioCPIMParse.rl" */
	{
		bIsImdnNS = Enum_TRUE;
	}
	goto st3;
tr651:
/* #line 376 "EcrioCPIMParse.rl" */
	{
		bIsMaapNS = Enum_TRUE;
	}
	goto st3;
tr719:
/* #line 494 "EcrioCPIMParse.rl" */
	{
		CPIMLOGI(c->logHandle, KLogTypeGeneral, "%s:%u\tIMDN-Route.", __FUNCTION__, __LINE__);
	}
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
/* #line 596 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr16;
		case 37: goto st3;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st3;
		} else if ( (*p) >= 32 )
			goto st3;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st3;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st3;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
tr337:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st4;
tr16:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st4;
tr211:
/* #line 359 "EcrioCPIMParse.rl" */
	{
		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pDateTime,
			(u_char*)tag_start,
			(p - tag_start));
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st4;
tr216:
/* #line 359 "EcrioCPIMParse.rl" */
	{
		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pDateTime,
			(u_char*)tag_start,
			(p - tag_start));
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st4;
tr255:
/* #line 156 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pFromAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st4;
tr289:
/* #line 489 "EcrioCPIMParse.rl" */
	{
		CPIMLOGI(c->logHandle, KLogTypeGeneral, "%s:%u\tIMDN-Record-Route.", __FUNCTION__, __LINE__);
	}
	goto st4;
tr292:
/* #line 489 "EcrioCPIMParse.rl" */
	{
		CPIMLOGI(c->logHandle, KLogTypeGeneral, "%s:%u\tIMDN-Record-Route.", __FUNCTION__, __LINE__);
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st4;
tr363:
/* #line 381 "EcrioCPIMParse.rl" */
	{
		bIsMyFeaturesNS = Enum_TRUE;
	}
	goto st4;
tr366:
/* #line 381 "EcrioCPIMParse.rl" */
	{
		bIsMyFeaturesNS = Enum_TRUE;
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st4;
tr377:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
/* #line 499 "EcrioCPIMParse.rl" */
	{
		CPIMLOGI(c->logHandle, KLogTypeGeneral, "%s:%u\tSubject.", __FUNCTION__, __LINE__);
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st4;
tr379:
/* #line 499 "EcrioCPIMParse.rl" */
	{
		CPIMLOGI(c->logHandle, KLogTypeGeneral, "%s:%u\tSubject.", __FUNCTION__, __LINE__);
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st4;
tr410:
/* #line 87 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pToAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st4;
tr420:
/* #line 87 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pToAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st4;
tr508:
/* #line 402 "EcrioCPIMParse.rl" */
	{
		if (bIsMaapNS == Enum_TRUE)
		{
			if (pal_StringNCompare((u_char*)tag_start, CPIM_ADVERTISEMENT_STRING,
						pal_StringLength(CPIM_ADVERTISEMENT_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Advertisement;
			}
			else if (pal_StringNCompare((u_char*)tag_start, CPIM_PAYMENT_STRING,
						pal_StringLength(CPIM_PAYMENT_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Payment;
			}
			else if (pal_StringNCompare((u_char*)tag_start, CPIM_PREMIUM_STRING,
						pal_StringLength(CPIM_PREMIUM_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Premium;
			}
			else if (pal_StringNCompare((u_char*)tag_start, CPIM_SUBSCRIPTION_STRING,
						pal_StringLength(CPIM_SUBSCRIPTION_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Subscription;
			}
			else if (pal_StringNCompare((u_char*)tag_start, CPIM_PLUGIN_STRING,
						pal_StringLength(CPIM_PLUGIN_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Plugin;
			}
			else
			{
				pStruct->trfType = CPIMTrafficType_None;
			}
		}
	}
	goto st4;
tr511:
/* #line 402 "EcrioCPIMParse.rl" */
	{
		if (bIsMaapNS == Enum_TRUE)
		{
			if (pal_StringNCompare((u_char*)tag_start, CPIM_ADVERTISEMENT_STRING,
						pal_StringLength(CPIM_ADVERTISEMENT_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Advertisement;
			}
			else if (pal_StringNCompare((u_char*)tag_start, CPIM_PAYMENT_STRING,
						pal_StringLength(CPIM_PAYMENT_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Payment;
			}
			else if (pal_StringNCompare((u_char*)tag_start, CPIM_PREMIUM_STRING,
						pal_StringLength(CPIM_PREMIUM_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Premium;
			}
			else if (pal_StringNCompare((u_char*)tag_start, CPIM_SUBSCRIPTION_STRING,
						pal_StringLength(CPIM_SUBSCRIPTION_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Subscription;
			}
			else if (pal_StringNCompare((u_char*)tag_start, CPIM_PLUGIN_STRING,
						pal_StringLength(CPIM_PLUGIN_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Plugin;
			}
			else
			{
				pStruct->trfType = CPIMTrafficType_None;
			}
		}
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st4;
tr543:
/* #line 386 "EcrioCPIMParse.rl" */
	{
		if (bIsImdnNS == Enum_TRUE)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pMsgId,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
			pStruct->uMsgIdLen = pal_StringLength(pStruct->pMsgId);
		}
	}
	goto st4;
tr546:
/* #line 386 "EcrioCPIMParse.rl" */
	{
		if (bIsImdnNS == Enum_TRUE)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pMsgId,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
			pStruct->uMsgIdLen = pal_StringLength(pStruct->pMsgId);
		}
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st4;
tr567:
/* #line 504 "EcrioCPIMParse.rl" */
	{
		CPIMLOGI(c->logHandle, KLogTypeGeneral, "%s:%u\timdn.Original-To.", __FUNCTION__, __LINE__);
	}
	goto st4;
tr570:
/* #line 504 "EcrioCPIMParse.rl" */
	{
		CPIMLOGI(c->logHandle, KLogTypeGeneral, "%s:%u\timdn.Original-To.", __FUNCTION__, __LINE__);
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st4;
tr629:
/* #line 371 "EcrioCPIMParse.rl" */
	{
		bIsImdnNS = Enum_TRUE;
	}
	goto st4;
tr632:
/* #line 371 "EcrioCPIMParse.rl" */
	{
		bIsImdnNS = Enum_TRUE;
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st4;
tr647:
/* #line 376 "EcrioCPIMParse.rl" */
	{
		bIsMaapNS = Enum_TRUE;
	}
	goto st4;
tr650:
/* #line 376 "EcrioCPIMParse.rl" */
	{
		bIsMaapNS = Enum_TRUE;
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st4;
tr715:
/* #line 494 "EcrioCPIMParse.rl" */
	{
		CPIMLOGI(c->logHandle, KLogTypeGeneral, "%s:%u\tIMDN-Route.", __FUNCTION__, __LINE__);
	}
	goto st4;
tr718:
/* #line 494 "EcrioCPIMParse.rl" */
	{
		CPIMLOGI(c->logHandle, KLogTypeGeneral, "%s:%u\tIMDN-Route.", __FUNCTION__, __LINE__);
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st4;
tr732:
/* #line 156 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pFromAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
/* #line 1203 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st1;
	goto tr0;
tr50:
/* #line 477 "EcrioCPIMParse.rl" */
	{
		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pContentTransferEncoding,
			(u_char*)tag_start,
			(p - tag_start));
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st5;
tr99:
/* #line 465 "EcrioCPIMParse.rl" */
	{
		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pContentId,
			(u_char*)tag_start,
			(p - tag_start));
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st5;
tr112:
/* #line 509 "EcrioCPIMParse.rl" */
	{
		pal_MemorySet(cTemp, 0, 8);
		if (NULL == pal_StringNCopy(cTemp, 8, (u_char*)tag_start, (p - tag_start)))
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		uCntLen = pal_StringToNum(cTemp, NULL);
	}
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
/* #line 1250 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st565;
	goto tr0;
st565:
	if ( ++p == pe )
		goto _test_eof565;
case 565:
	switch( (*p) ) {
		case 13: goto st6;
		case 67: goto tr798;
	}
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	if ( (*p) == 10 )
		goto st566;
	goto tr0;
st566:
	if ( ++p == pe )
		goto _test_eof566;
case 566:
	goto tr799;
tr799:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st567;
st567:
	if ( ++p == pe )
		goto _test_eof567;
case 567:
/* #line 1285 "EcrioCPIMParse.c" */
	goto st567;
tr798:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
/* #line 1297 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 79: goto st8;
		case 111: goto st38;
	}
	goto tr0;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	if ( (*p) == 78 )
		goto st9;
	goto tr0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	if ( (*p) == 84 )
		goto st10;
	goto tr0;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
	if ( (*p) == 69 )
		goto st11;
	goto tr0;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
	if ( (*p) == 78 )
		goto st12;
	goto tr0;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
	if ( (*p) == 84 )
		goto st13;
	goto tr0;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	if ( (*p) == 45 )
		goto st14;
	goto tr0;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	if ( (*p) == 84 )
		goto st15;
	goto tr0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	if ( (*p) == 82 )
		goto st16;
	goto tr0;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	if ( (*p) == 65 )
		goto st17;
	goto tr0;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	if ( (*p) == 78 )
		goto st18;
	goto tr0;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
	if ( (*p) == 83 )
		goto st19;
	goto tr0;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
	if ( (*p) == 70 )
		goto st20;
	goto tr0;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
	if ( (*p) == 69 )
		goto st21;
	goto tr0;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
	if ( (*p) == 82 )
		goto st22;
	goto tr0;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
	if ( (*p) == 45 )
		goto st23;
	goto tr0;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	if ( (*p) == 69 )
		goto st24;
	goto tr0;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
	if ( (*p) == 78 )
		goto st25;
	goto tr0;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
	if ( (*p) == 67 )
		goto st26;
	goto tr0;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
	if ( (*p) == 79 )
		goto st27;
	goto tr0;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
	if ( (*p) == 68 )
		goto st28;
	goto tr0;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
	if ( (*p) == 73 )
		goto st29;
	goto tr0;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
	if ( (*p) == 78 )
		goto st30;
	goto tr0;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
	if ( (*p) == 71 )
		goto st31;
	goto tr0;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
	switch( (*p) ) {
		case 9: goto st31;
		case 32: goto st31;
		case 58: goto st32;
	}
	goto tr0;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
	switch( (*p) ) {
		case 13: goto st33;
		case 32: goto st32;
		case 37: goto tr47;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr47;
		} else if ( (*p) >= 33 )
			goto tr47;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr47;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr47;
		} else
			goto tr47;
	} else
		goto tr47;
	goto tr0;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
	if ( (*p) == 10 )
		goto st34;
	goto tr0;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
	if ( (*p) == 32 )
		goto st35;
	goto tr0;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
	switch( (*p) ) {
		case 32: goto st35;
		case 37: goto tr47;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr47;
		} else if ( (*p) >= 33 )
			goto tr47;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr47;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr47;
		} else
			goto tr47;
	} else
		goto tr47;
	goto tr0;
tr47:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st36;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
/* #line 1551 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr50;
		case 32: goto tr51;
		case 37: goto st36;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st36;
		} else if ( (*p) >= 33 )
			goto st36;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st36;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st36;
		} else
			goto st36;
	} else
		goto st36;
	goto tr0;
tr51:
/* #line 477 "EcrioCPIMParse.rl" */
	{
		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pContentTransferEncoding,
			(u_char*)tag_start,
			(p - tag_start));
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st37;
tr100:
/* #line 465 "EcrioCPIMParse.rl" */
	{
		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pContentId,
			(u_char*)tag_start,
			(p - tag_start));
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st37;
tr113:
/* #line 509 "EcrioCPIMParse.rl" */
	{
		pal_MemorySet(cTemp, 0, 8);
		if (NULL == pal_StringNCopy(cTemp, 8, (u_char*)tag_start, (p - tag_start)))
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		uCntLen = pal_StringToNum(cTemp, NULL);
	}
	goto st37;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
/* #line 1618 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st5;
		case 32: goto st37;
	}
	goto tr0;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
	if ( (*p) == 110 )
		goto st39;
	goto tr0;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
	if ( (*p) == 116 )
		goto st40;
	goto tr0;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
	if ( (*p) == 101 )
		goto st41;
	goto tr0;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
	if ( (*p) == 110 )
		goto st42;
	goto tr0;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
	if ( (*p) == 116 )
		goto st43;
	goto tr0;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
	if ( (*p) == 45 )
		goto st44;
	goto tr0;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
	switch( (*p) ) {
		case 68: goto st45;
		case 73: goto st69;
		case 76: goto st76;
		case 84: goto st87;
		case 108: goto st76;
		case 116: goto st148;
	}
	goto tr0;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
	if ( (*p) == 105 )
		goto st46;
	goto tr0;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
	if ( (*p) == 115 )
		goto st47;
	goto tr0;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
	if ( (*p) == 112 )
		goto st48;
	goto tr0;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
	if ( (*p) == 111 )
		goto st49;
	goto tr0;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
	if ( (*p) == 115 )
		goto st50;
	goto tr0;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
	if ( (*p) == 105 )
		goto st51;
	goto tr0;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
	if ( (*p) == 116 )
		goto st52;
	goto tr0;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
	if ( (*p) == 105 )
		goto st53;
	goto tr0;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
	if ( (*p) == 111 )
		goto st54;
	goto tr0;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
	if ( (*p) == 110 )
		goto st55;
	goto tr0;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
	switch( (*p) ) {
		case 9: goto st55;
		case 32: goto st55;
		case 58: goto st56;
	}
	goto tr0;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
	switch( (*p) ) {
		case 13: goto st57;
		case 32: goto st56;
		case 37: goto tr77;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr77;
		} else if ( (*p) >= 33 )
			goto tr77;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr77;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr77;
		} else
			goto tr77;
	} else
		goto tr77;
	goto tr0;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
	if ( (*p) == 10 )
		goto st58;
	goto tr0;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
	if ( (*p) == 32 )
		goto st59;
	goto tr0;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
	switch( (*p) ) {
		case 32: goto st59;
		case 37: goto tr77;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr77;
		} else if ( (*p) >= 33 )
			goto tr77;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr77;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr77;
		} else
			goto tr77;
	} else
		goto tr77;
	goto tr0;
tr77:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st60;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
/* #line 1836 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr80;
		case 32: goto tr81;
		case 37: goto st60;
		case 59: goto tr83;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st60;
		} else if ( (*p) >= 33 )
			goto st60;
	} else if ( (*p) > 93 ) {
		if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st60;
		} else if ( (*p) >= 95 )
			goto st60;
	} else
		goto st60;
	goto tr0;
tr80:
/* #line 453 "EcrioCPIMParse.rl" */
	{
		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pContentDisposition,
			(u_char*)tag_start,
			(p - tag_start));
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st61;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
/* #line 1875 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st568;
	goto tr0;
st568:
	if ( ++p == pe )
		goto _test_eof568;
case 568:
	switch( (*p) ) {
		case 13: goto st6;
		case 32: goto st62;
		case 67: goto tr798;
	}
	goto st0;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
	switch( (*p) ) {
		case 32: goto st62;
		case 59: goto st63;
	}
	goto tr0;
tr83:
/* #line 453 "EcrioCPIMParse.rl" */
	{
		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pContentDisposition,
			(u_char*)tag_start,
			(p - tag_start));
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st63;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
/* #line 1915 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st64;
		case 32: goto st63;
		case 33: goto st67;
		case 37: goto st67;
		case 39: goto st67;
		case 61: goto st67;
		case 126: goto st67;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st67;
		} else if ( (*p) >= 42 )
			goto st67;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st67;
		} else if ( (*p) >= 65 )
			goto st67;
	} else
		goto st67;
	goto tr0;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
	if ( (*p) == 10 )
		goto st65;
	goto tr0;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
	if ( (*p) == 32 )
		goto st66;
	goto tr0;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
	switch( (*p) ) {
		case 32: goto st66;
		case 33: goto st67;
		case 37: goto st67;
		case 39: goto st67;
		case 61: goto st67;
		case 126: goto st67;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st67;
		} else if ( (*p) >= 42 )
			goto st67;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st67;
		} else if ( (*p) >= 65 )
			goto st67;
	} else
		goto st67;
	goto tr0;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
	switch( (*p) ) {
		case 13: goto st61;
		case 32: goto st68;
		case 33: goto st67;
		case 37: goto st67;
		case 39: goto st67;
		case 59: goto st63;
		case 61: goto st67;
		case 126: goto st67;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st67;
		} else if ( (*p) >= 42 )
			goto st67;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st67;
		} else if ( (*p) >= 65 )
			goto st67;
	} else
		goto st67;
	goto tr0;
tr81:
/* #line 453 "EcrioCPIMParse.rl" */
	{
		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pContentDisposition,
			(u_char*)tag_start,
			(p - tag_start));
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st68;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
/* #line 2027 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st61;
		case 32: goto st68;
		case 59: goto st63;
	}
	goto tr0;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
	if ( (*p) == 68 )
		goto st70;
	goto tr0;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
	switch( (*p) ) {
		case 9: goto st70;
		case 32: goto st70;
		case 58: goto st71;
	}
	goto tr0;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
	switch( (*p) ) {
		case 13: goto st72;
		case 32: goto st71;
		case 37: goto tr96;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr96;
		} else if ( (*p) >= 33 )
			goto tr96;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr96;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr96;
		} else
			goto tr96;
	} else
		goto tr96;
	goto tr0;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
	if ( (*p) == 10 )
		goto st73;
	goto tr0;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
	if ( (*p) == 32 )
		goto st74;
	goto tr0;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
	switch( (*p) ) {
		case 32: goto st74;
		case 37: goto tr96;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr96;
		} else if ( (*p) >= 33 )
			goto tr96;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr96;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr96;
		} else
			goto tr96;
	} else
		goto tr96;
	goto tr0;
tr96:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st75;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
/* #line 2128 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr99;
		case 32: goto tr100;
		case 37: goto st75;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st75;
		} else if ( (*p) >= 33 )
			goto st75;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st75;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st75;
		} else
			goto st75;
	} else
		goto st75;
	goto tr0;
st76:
	if ( ++p == pe )
		goto _test_eof76;
case 76:
	if ( (*p) == 101 )
		goto st77;
	goto tr0;
st77:
	if ( ++p == pe )
		goto _test_eof77;
case 77:
	if ( (*p) == 110 )
		goto st78;
	goto tr0;
st78:
	if ( ++p == pe )
		goto _test_eof78;
case 78:
	if ( (*p) == 103 )
		goto st79;
	goto tr0;
st79:
	if ( ++p == pe )
		goto _test_eof79;
case 79:
	if ( (*p) == 116 )
		goto st80;
	goto tr0;
st80:
	if ( ++p == pe )
		goto _test_eof80;
case 80:
	if ( (*p) == 104 )
		goto st81;
	goto tr0;
st81:
	if ( ++p == pe )
		goto _test_eof81;
case 81:
	switch( (*p) ) {
		case 9: goto st81;
		case 32: goto st81;
		case 58: goto st82;
	}
	goto tr0;
st82:
	if ( ++p == pe )
		goto _test_eof82;
case 82:
	switch( (*p) ) {
		case 13: goto st83;
		case 32: goto st82;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr109;
	goto tr0;
st83:
	if ( ++p == pe )
		goto _test_eof83;
case 83:
	if ( (*p) == 10 )
		goto st84;
	goto tr0;
st84:
	if ( ++p == pe )
		goto _test_eof84;
case 84:
	if ( (*p) == 32 )
		goto st85;
	goto tr0;
st85:
	if ( ++p == pe )
		goto _test_eof85;
case 85:
	if ( (*p) == 32 )
		goto st85;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr109;
	goto tr0;
tr109:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st86;
st86:
	if ( ++p == pe )
		goto _test_eof86;
case 86:
/* #line 2241 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr112;
		case 32: goto tr113;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st86;
	goto tr0;
st87:
	if ( ++p == pe )
		goto _test_eof87;
case 87:
	switch( (*p) ) {
		case 114: goto st88;
		case 121: goto st103;
	}
	goto tr0;
st88:
	if ( ++p == pe )
		goto _test_eof88;
case 88:
	if ( (*p) == 97 )
		goto st89;
	goto tr0;
st89:
	if ( ++p == pe )
		goto _test_eof89;
case 89:
	if ( (*p) == 110 )
		goto st90;
	goto tr0;
st90:
	if ( ++p == pe )
		goto _test_eof90;
case 90:
	if ( (*p) == 115 )
		goto st91;
	goto tr0;
st91:
	if ( ++p == pe )
		goto _test_eof91;
case 91:
	if ( (*p) == 102 )
		goto st92;
	goto tr0;
st92:
	if ( ++p == pe )
		goto _test_eof92;
case 92:
	if ( (*p) == 101 )
		goto st93;
	goto tr0;
st93:
	if ( ++p == pe )
		goto _test_eof93;
case 93:
	if ( (*p) == 114 )
		goto st94;
	goto tr0;
st94:
	if ( ++p == pe )
		goto _test_eof94;
case 94:
	if ( (*p) == 45 )
		goto st95;
	goto tr0;
st95:
	if ( ++p == pe )
		goto _test_eof95;
case 95:
	if ( (*p) == 69 )
		goto st96;
	goto tr0;
st96:
	if ( ++p == pe )
		goto _test_eof96;
case 96:
	if ( (*p) == 110 )
		goto st97;
	goto tr0;
st97:
	if ( ++p == pe )
		goto _test_eof97;
case 97:
	if ( (*p) == 99 )
		goto st98;
	goto tr0;
st98:
	if ( ++p == pe )
		goto _test_eof98;
case 98:
	if ( (*p) == 111 )
		goto st99;
	goto tr0;
st99:
	if ( ++p == pe )
		goto _test_eof99;
case 99:
	if ( (*p) == 100 )
		goto st100;
	goto tr0;
st100:
	if ( ++p == pe )
		goto _test_eof100;
case 100:
	if ( (*p) == 105 )
		goto st101;
	goto tr0;
st101:
	if ( ++p == pe )
		goto _test_eof101;
case 101:
	if ( (*p) == 110 )
		goto st102;
	goto tr0;
st102:
	if ( ++p == pe )
		goto _test_eof102;
case 102:
	if ( (*p) == 103 )
		goto st31;
	goto tr0;
st103:
	if ( ++p == pe )
		goto _test_eof103;
case 103:
	if ( (*p) == 112 )
		goto st104;
	goto tr0;
st104:
	if ( ++p == pe )
		goto _test_eof104;
case 104:
	if ( (*p) == 101 )
		goto st105;
	goto tr0;
st105:
	if ( ++p == pe )
		goto _test_eof105;
case 105:
	switch( (*p) ) {
		case 9: goto st105;
		case 32: goto st105;
		case 58: goto st106;
	}
	goto tr0;
st106:
	if ( ++p == pe )
		goto _test_eof106;
case 106:
	switch( (*p) ) {
		case 13: goto st107;
		case 32: goto st106;
		case 37: goto tr135;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr135;
		} else if ( (*p) >= 33 )
			goto tr135;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr135;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr135;
		} else
			goto tr135;
	} else
		goto tr135;
	goto tr0;
st107:
	if ( ++p == pe )
		goto _test_eof107;
case 107:
	if ( (*p) == 10 )
		goto st108;
	goto tr0;
st108:
	if ( ++p == pe )
		goto _test_eof108;
case 108:
	if ( (*p) == 32 )
		goto st109;
	goto tr0;
st109:
	if ( ++p == pe )
		goto _test_eof109;
case 109:
	switch( (*p) ) {
		case 32: goto st109;
		case 37: goto tr135;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr135;
		} else if ( (*p) >= 33 )
			goto tr135;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr135;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr135;
		} else
			goto tr135;
	} else
		goto tr135;
	goto tr0;
tr135:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st110;
st110:
	if ( ++p == pe )
		goto _test_eof110;
case 110:
/* #line 2464 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr138;
		case 32: goto tr139;
		case 37: goto st110;
		case 59: goto tr141;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st110;
		} else if ( (*p) >= 33 )
			goto st110;
	} else if ( (*p) > 93 ) {
		if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st110;
		} else if ( (*p) >= 95 )
			goto st110;
	} else
		goto st110;
	goto tr0;
tr138:
/* #line 273 "EcrioCPIMParse.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_MULTIPART_MIXED_STRING,
				pal_StringLength(CPIM_CONTENT_TYPE_MULTIPART_MIXED_STRING)) == 0)
		{
			bIsMultiPart = Enum_TRUE;
		}
		else if (pal_StringNCompare((u_char*)tag_start, (u_char*)"text/plain",
					pal_StringLength((u_char*)"text/plain")) == 0)
		{
			eConType = CPIMContentType_Text;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_IMDN_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_IMDN_STRING)) == 0)
		{
			eConType = CPIMContentType_IMDN;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_FILE_TRANSFER_HTTP_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_FILE_TRANSFER_HTTP_STRING)) == 0)
		{
			eConType = CPIMContentType_FileTransferOverHTTP;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_PUSH_LOCATION_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_PUSH_LOCATION_STRING)) == 0)
		{
			eConType = CPIMContentType_PushLocation;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_MESSAGE_REVOKE_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_MESSAGE_REVOKE_STRING)) == 0)
		{
			eConType = CPIMContentType_MessageRevoke;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_PRIVACY_MANAGEMENT_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_PRIVACY_MANAGEMENT_STRING)) == 0)
		{
			eConType = CPIMContentType_PrivacyManagement;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_LINK_REPORT_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_LINK_REPORT_STRING)) == 0)
		{
			eConType = CPIMContentType_LinkReport;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SPAM_REPORT_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SPAM_REPORT_STRING)) == 0)
		{
			eConType = CPIMContentType_SpamReport;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_RICHCARD_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_RICHCARD_STRING)) == 0)
		{
			eConType = CPIMContentType_RichCard;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SUGGESTED_CHIPLIST_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SUGGESTED_CHIPLIST_STRING)) == 0)
		{
			eConType = CPIMContentType_SuggestedChipList;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SUGGESTED_RESPONSE_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SUGGESTED_RESPONSE_STRING)) == 0)
		{
			eConType = CPIMContentType_SuggestionResponse;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SHARED_CLIENT_DATA_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SHARED_CLIENT_DATA_STRING)) == 0)
		{
			eConType = CPIMContentType_SharedClientData;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_CPM_GROUPDATA_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_CPM_GROUPDATA_STRING)) == 0)
		{
			eConType = CPIMContentType_CpmGroupData;
		}
		else
		{
			eConType = CPIMContentType_Specified;
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pContentType,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st111;
tr162:
/* #line 259 "EcrioCPIMParse.rl" */
	{
		if (bIsMultiPart == Enum_TRUE)
		{
			pal_MemorySet(cBoundary, 0, CPIM_BOUNDARY_LENGTH);
			if (NULL == pal_StringNCopy(cBoundary, CPIM_BOUNDARY_LENGTH, (u_char*)tag_start + 1, (p - tag_start - 2)))
			{
				CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.", __FUNCTION__, __LINE__);
				uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
				goto END;
			}
		}
	}
	goto st111;
st111:
	if ( ++p == pe )
		goto _test_eof111;
case 111:
/* #line 2592 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st569;
	goto tr0;
st569:
	if ( ++p == pe )
		goto _test_eof569;
case 569:
	switch( (*p) ) {
		case 13: goto st6;
		case 32: goto st112;
		case 67: goto tr798;
	}
	goto st0;
st112:
	if ( ++p == pe )
		goto _test_eof112;
case 112:
	switch( (*p) ) {
		case 32: goto st112;
		case 59: goto st113;
	}
	goto tr0;
tr141:
/* #line 273 "EcrioCPIMParse.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_MULTIPART_MIXED_STRING,
				pal_StringLength(CPIM_CONTENT_TYPE_MULTIPART_MIXED_STRING)) == 0)
		{
			bIsMultiPart = Enum_TRUE;
		}
		else if (pal_StringNCompare((u_char*)tag_start, (u_char*)"text/plain",
					pal_StringLength((u_char*)"text/plain")) == 0)
		{
			eConType = CPIMContentType_Text;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_IMDN_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_IMDN_STRING)) == 0)
		{
			eConType = CPIMContentType_IMDN;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_FILE_TRANSFER_HTTP_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_FILE_TRANSFER_HTTP_STRING)) == 0)
		{
			eConType = CPIMContentType_FileTransferOverHTTP;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_PUSH_LOCATION_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_PUSH_LOCATION_STRING)) == 0)
		{
			eConType = CPIMContentType_PushLocation;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_MESSAGE_REVOKE_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_MESSAGE_REVOKE_STRING)) == 0)
		{
			eConType = CPIMContentType_MessageRevoke;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_PRIVACY_MANAGEMENT_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_PRIVACY_MANAGEMENT_STRING)) == 0)
		{
			eConType = CPIMContentType_PrivacyManagement;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_LINK_REPORT_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_LINK_REPORT_STRING)) == 0)
		{
			eConType = CPIMContentType_LinkReport;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SPAM_REPORT_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SPAM_REPORT_STRING)) == 0)
		{
			eConType = CPIMContentType_SpamReport;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_RICHCARD_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_RICHCARD_STRING)) == 0)
		{
			eConType = CPIMContentType_RichCard;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SUGGESTED_CHIPLIST_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SUGGESTED_CHIPLIST_STRING)) == 0)
		{
			eConType = CPIMContentType_SuggestedChipList;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SUGGESTED_RESPONSE_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SUGGESTED_RESPONSE_STRING)) == 0)
		{
			eConType = CPIMContentType_SuggestionResponse;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SHARED_CLIENT_DATA_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SHARED_CLIENT_DATA_STRING)) == 0)
		{
			eConType = CPIMContentType_SharedClientData;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_CPM_GROUPDATA_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_CPM_GROUPDATA_STRING)) == 0)
		{
			eConType = CPIMContentType_CpmGroupData;
		}
		else
		{
			eConType = CPIMContentType_Specified;
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pContentType,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st113;
tr165:
/* #line 259 "EcrioCPIMParse.rl" */
	{
		if (bIsMultiPart == Enum_TRUE)
		{
			pal_MemorySet(cBoundary, 0, CPIM_BOUNDARY_LENGTH);
			if (NULL == pal_StringNCopy(cBoundary, CPIM_BOUNDARY_LENGTH, (u_char*)tag_start + 1, (p - tag_start - 2)))
			{
				CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.", __FUNCTION__, __LINE__);
				uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
				goto END;
			}
		}
	}
	goto st113;
st113:
	if ( ++p == pe )
		goto _test_eof113;
case 113:
/* #line 2721 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st114;
		case 32: goto st113;
		case 37: goto st117;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st117;
		} else if ( (*p) >= 33 )
			goto st117;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st117;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st117;
		} else
			goto st117;
	} else
		goto st117;
	goto tr0;
st114:
	if ( ++p == pe )
		goto _test_eof114;
case 114:
	if ( (*p) == 10 )
		goto st115;
	goto tr0;
st115:
	if ( ++p == pe )
		goto _test_eof115;
case 115:
	if ( (*p) == 32 )
		goto st116;
	goto tr0;
st116:
	if ( ++p == pe )
		goto _test_eof116;
case 116:
	switch( (*p) ) {
		case 32: goto st116;
		case 37: goto st117;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st117;
		} else if ( (*p) >= 33 )
			goto st117;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st117;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st117;
		} else
			goto st117;
	} else
		goto st117;
	goto tr0;
st117:
	if ( ++p == pe )
		goto _test_eof117;
case 117:
	switch( (*p) ) {
		case 13: goto st118;
		case 32: goto st130;
		case 37: goto st117;
		case 61: goto st131;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st117;
		} else if ( (*p) >= 33 )
			goto st117;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st117;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st117;
		} else
			goto st117;
	} else
		goto st117;
	goto tr0;
st118:
	if ( ++p == pe )
		goto _test_eof118;
case 118:
	if ( (*p) == 10 )
		goto st119;
	goto tr0;
st119:
	if ( ++p == pe )
		goto _test_eof119;
case 119:
	if ( (*p) == 32 )
		goto st120;
	goto tr0;
st120:
	if ( ++p == pe )
		goto _test_eof120;
case 120:
	switch( (*p) ) {
		case 32: goto st120;
		case 61: goto st121;
	}
	goto tr0;
st121:
	if ( ++p == pe )
		goto _test_eof121;
case 121:
	switch( (*p) ) {
		case 13: goto st122;
		case 32: goto st121;
		case 37: goto tr156;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr156;
		} else if ( (*p) >= 33 )
			goto tr156;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr156;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr156;
		} else
			goto tr156;
	} else
		goto tr156;
	goto tr0;
st122:
	if ( ++p == pe )
		goto _test_eof122;
case 122:
	if ( (*p) == 10 )
		goto st123;
	goto tr0;
st123:
	if ( ++p == pe )
		goto _test_eof123;
case 123:
	if ( (*p) == 32 )
		goto st124;
	goto tr0;
st124:
	if ( ++p == pe )
		goto _test_eof124;
case 124:
	switch( (*p) ) {
		case 13: goto st125;
		case 32: goto st124;
		case 37: goto tr156;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr156;
		} else if ( (*p) >= 33 )
			goto tr156;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr156;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr156;
		} else
			goto tr156;
	} else
		goto tr156;
	goto tr0;
st125:
	if ( ++p == pe )
		goto _test_eof125;
case 125:
	if ( (*p) == 10 )
		goto st126;
	goto tr0;
st126:
	if ( ++p == pe )
		goto _test_eof126;
case 126:
	if ( (*p) == 32 )
		goto st127;
	goto tr0;
st127:
	if ( ++p == pe )
		goto _test_eof127;
case 127:
	switch( (*p) ) {
		case 32: goto st127;
		case 37: goto tr156;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr156;
		} else if ( (*p) >= 33 )
			goto tr156;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr156;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr156;
		} else
			goto tr156;
	} else
		goto tr156;
	goto tr0;
tr156:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st128;
st128:
	if ( ++p == pe )
		goto _test_eof128;
case 128:
/* #line 2954 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr162;
		case 32: goto tr163;
		case 37: goto st128;
		case 59: goto tr165;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st128;
		} else if ( (*p) >= 33 )
			goto st128;
	} else if ( (*p) > 93 ) {
		if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st128;
		} else if ( (*p) >= 95 )
			goto st128;
	} else
		goto st128;
	goto tr0;
tr139:
/* #line 273 "EcrioCPIMParse.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_MULTIPART_MIXED_STRING,
				pal_StringLength(CPIM_CONTENT_TYPE_MULTIPART_MIXED_STRING)) == 0)
		{
			bIsMultiPart = Enum_TRUE;
		}
		else if (pal_StringNCompare((u_char*)tag_start, (u_char*)"text/plain",
					pal_StringLength((u_char*)"text/plain")) == 0)
		{
			eConType = CPIMContentType_Text;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_IMDN_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_IMDN_STRING)) == 0)
		{
			eConType = CPIMContentType_IMDN;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_FILE_TRANSFER_HTTP_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_FILE_TRANSFER_HTTP_STRING)) == 0)
		{
			eConType = CPIMContentType_FileTransferOverHTTP;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_PUSH_LOCATION_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_PUSH_LOCATION_STRING)) == 0)
		{
			eConType = CPIMContentType_PushLocation;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_MESSAGE_REVOKE_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_MESSAGE_REVOKE_STRING)) == 0)
		{
			eConType = CPIMContentType_MessageRevoke;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_PRIVACY_MANAGEMENT_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_PRIVACY_MANAGEMENT_STRING)) == 0)
		{
			eConType = CPIMContentType_PrivacyManagement;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_LINK_REPORT_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_LINK_REPORT_STRING)) == 0)
		{
			eConType = CPIMContentType_LinkReport;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SPAM_REPORT_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SPAM_REPORT_STRING)) == 0)
		{
			eConType = CPIMContentType_SpamReport;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_RICHCARD_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_RICHCARD_STRING)) == 0)
		{
			eConType = CPIMContentType_RichCard;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SUGGESTED_CHIPLIST_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SUGGESTED_CHIPLIST_STRING)) == 0)
		{
			eConType = CPIMContentType_SuggestedChipList;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SUGGESTED_RESPONSE_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SUGGESTED_RESPONSE_STRING)) == 0)
		{
			eConType = CPIMContentType_SuggestionResponse;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SHARED_CLIENT_DATA_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SHARED_CLIENT_DATA_STRING)) == 0)
		{
			eConType = CPIMContentType_SharedClientData;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_CPM_GROUPDATA_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_CPM_GROUPDATA_STRING)) == 0)
		{
			eConType = CPIMContentType_CpmGroupData;
		}
		else
		{
			eConType = CPIMContentType_Specified;
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pContentType,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st129;
tr163:
/* #line 259 "EcrioCPIMParse.rl" */
	{
		if (bIsMultiPart == Enum_TRUE)
		{
			pal_MemorySet(cBoundary, 0, CPIM_BOUNDARY_LENGTH);
			if (NULL == pal_StringNCopy(cBoundary, CPIM_BOUNDARY_LENGTH, (u_char*)tag_start + 1, (p - tag_start - 2)))
			{
				CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.", __FUNCTION__, __LINE__);
				uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
				goto END;
			}
		}
	}
	goto st129;
st129:
	if ( ++p == pe )
		goto _test_eof129;
case 129:
/* #line 3082 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st111;
		case 32: goto st129;
		case 59: goto st113;
	}
	goto tr0;
st130:
	if ( ++p == pe )
		goto _test_eof130;
case 130:
	switch( (*p) ) {
		case 13: goto st118;
		case 32: goto st130;
		case 61: goto st121;
	}
	goto tr0;
st131:
	if ( ++p == pe )
		goto _test_eof131;
case 131:
	switch( (*p) ) {
		case 13: goto st132;
		case 32: goto st139;
		case 37: goto tr170;
		case 61: goto tr171;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr170;
		} else if ( (*p) >= 33 )
			goto tr170;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr170;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr170;
		} else
			goto tr170;
	} else
		goto tr170;
	goto tr0;
st132:
	if ( ++p == pe )
		goto _test_eof132;
case 132:
	if ( (*p) == 10 )
		goto st133;
	goto tr0;
st133:
	if ( ++p == pe )
		goto _test_eof133;
case 133:
	if ( (*p) == 32 )
		goto st134;
	goto tr0;
st134:
	if ( ++p == pe )
		goto _test_eof134;
case 134:
	switch( (*p) ) {
		case 13: goto st125;
		case 32: goto st134;
		case 37: goto tr156;
		case 61: goto tr174;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr156;
		} else if ( (*p) >= 33 )
			goto tr156;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr156;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr156;
		} else
			goto tr156;
	} else
		goto tr156;
	goto tr0;
tr174:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st135;
st135:
	if ( ++p == pe )
		goto _test_eof135;
case 135:
/* #line 3179 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr175;
		case 32: goto tr176;
		case 37: goto tr156;
		case 59: goto tr165;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr156;
		} else if ( (*p) >= 33 )
			goto tr156;
	} else if ( (*p) > 93 ) {
		if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr156;
		} else if ( (*p) >= 95 )
			goto tr156;
	} else
		goto tr156;
	goto tr0;
tr175:
/* #line 259 "EcrioCPIMParse.rl" */
	{
		if (bIsMultiPart == Enum_TRUE)
		{
			pal_MemorySet(cBoundary, 0, CPIM_BOUNDARY_LENGTH);
			if (NULL == pal_StringNCopy(cBoundary, CPIM_BOUNDARY_LENGTH, (u_char*)tag_start + 1, (p - tag_start - 2)))
			{
				CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.", __FUNCTION__, __LINE__);
				uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
				goto END;
			}
		}
	}
	goto st136;
st136:
	if ( ++p == pe )
		goto _test_eof136;
case 136:
/* #line 3220 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st570;
	goto tr0;
st570:
	if ( ++p == pe )
		goto _test_eof570;
case 570:
	switch( (*p) ) {
		case 13: goto st6;
		case 32: goto st137;
		case 67: goto tr798;
	}
	goto st0;
st137:
	if ( ++p == pe )
		goto _test_eof137;
case 137:
	switch( (*p) ) {
		case 13: goto st125;
		case 32: goto st137;
		case 37: goto tr156;
		case 59: goto st113;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr156;
		} else if ( (*p) >= 33 )
			goto tr156;
	} else if ( (*p) > 93 ) {
		if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr156;
		} else if ( (*p) >= 95 )
			goto tr156;
	} else
		goto tr156;
	goto tr0;
tr176:
/* #line 259 "EcrioCPIMParse.rl" */
	{
		if (bIsMultiPart == Enum_TRUE)
		{
			pal_MemorySet(cBoundary, 0, CPIM_BOUNDARY_LENGTH);
			if (NULL == pal_StringNCopy(cBoundary, CPIM_BOUNDARY_LENGTH, (u_char*)tag_start + 1, (p - tag_start - 2)))
			{
				CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.", __FUNCTION__, __LINE__);
				uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
				goto END;
			}
		}
	}
	goto st138;
st138:
	if ( ++p == pe )
		goto _test_eof138;
case 138:
/* #line 3278 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st136;
		case 32: goto st138;
		case 37: goto tr156;
		case 59: goto st113;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr156;
		} else if ( (*p) >= 33 )
			goto tr156;
	} else if ( (*p) > 93 ) {
		if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr156;
		} else if ( (*p) >= 95 )
			goto tr156;
	} else
		goto tr156;
	goto tr0;
st139:
	if ( ++p == pe )
		goto _test_eof139;
case 139:
	switch( (*p) ) {
		case 13: goto st132;
		case 32: goto st139;
		case 37: goto tr156;
		case 61: goto tr174;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr156;
		} else if ( (*p) >= 33 )
			goto tr156;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr156;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr156;
		} else
			goto tr156;
	} else
		goto tr156;
	goto tr0;
tr170:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st140;
st140:
	if ( ++p == pe )
		goto _test_eof140;
case 140:
/* #line 3338 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr181;
		case 32: goto tr182;
		case 37: goto st140;
		case 59: goto tr165;
		case 61: goto st144;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st140;
		} else if ( (*p) >= 33 )
			goto st140;
	} else if ( (*p) > 93 ) {
		if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st140;
		} else if ( (*p) >= 95 )
			goto st140;
	} else
		goto st140;
	goto tr0;
tr181:
/* #line 259 "EcrioCPIMParse.rl" */
	{
		if (bIsMultiPart == Enum_TRUE)
		{
			pal_MemorySet(cBoundary, 0, CPIM_BOUNDARY_LENGTH);
			if (NULL == pal_StringNCopy(cBoundary, CPIM_BOUNDARY_LENGTH, (u_char*)tag_start + 1, (p - tag_start - 2)))
			{
				CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.", __FUNCTION__, __LINE__);
				uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
				goto END;
			}
		}
	}
	goto st141;
st141:
	if ( ++p == pe )
		goto _test_eof141;
case 141:
/* #line 3380 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st571;
	goto tr0;
st571:
	if ( ++p == pe )
		goto _test_eof571;
case 571:
	switch( (*p) ) {
		case 13: goto st6;
		case 32: goto st142;
		case 67: goto tr798;
	}
	goto st0;
st142:
	if ( ++p == pe )
		goto _test_eof142;
case 142:
	switch( (*p) ) {
		case 32: goto st142;
		case 59: goto st113;
		case 61: goto st121;
	}
	goto tr0;
tr182:
/* #line 259 "EcrioCPIMParse.rl" */
	{
		if (bIsMultiPart == Enum_TRUE)
		{
			pal_MemorySet(cBoundary, 0, CPIM_BOUNDARY_LENGTH);
			if (NULL == pal_StringNCopy(cBoundary, CPIM_BOUNDARY_LENGTH, (u_char*)tag_start + 1, (p - tag_start - 2)))
			{
				CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.", __FUNCTION__, __LINE__);
				uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
				goto END;
			}
		}
	}
	goto st143;
st143:
	if ( ++p == pe )
		goto _test_eof143;
case 143:
/* #line 3423 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st141;
		case 32: goto st143;
		case 59: goto st113;
		case 61: goto st121;
	}
	goto tr0;
tr171:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st144;
st144:
	if ( ++p == pe )
		goto _test_eof144;
case 144:
/* #line 3441 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr189;
		case 32: goto tr190;
		case 37: goto tr170;
		case 59: goto tr165;
		case 61: goto tr171;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr170;
		} else if ( (*p) >= 33 )
			goto tr170;
	} else if ( (*p) > 93 ) {
		if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr170;
		} else if ( (*p) >= 95 )
			goto tr170;
	} else
		goto tr170;
	goto tr0;
tr189:
/* #line 259 "EcrioCPIMParse.rl" */
	{
		if (bIsMultiPart == Enum_TRUE)
		{
			pal_MemorySet(cBoundary, 0, CPIM_BOUNDARY_LENGTH);
			if (NULL == pal_StringNCopy(cBoundary, CPIM_BOUNDARY_LENGTH, (u_char*)tag_start + 1, (p - tag_start - 2)))
			{
				CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.", __FUNCTION__, __LINE__);
				uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
				goto END;
			}
		}
	}
	goto st145;
st145:
	if ( ++p == pe )
		goto _test_eof145;
case 145:
/* #line 3483 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st572;
	goto tr0;
st572:
	if ( ++p == pe )
		goto _test_eof572;
case 572:
	switch( (*p) ) {
		case 13: goto st6;
		case 32: goto st146;
		case 67: goto tr798;
	}
	goto st0;
st146:
	if ( ++p == pe )
		goto _test_eof146;
case 146:
	switch( (*p) ) {
		case 13: goto st125;
		case 32: goto st146;
		case 37: goto tr156;
		case 59: goto st113;
		case 61: goto tr174;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr156;
		} else if ( (*p) >= 33 )
			goto tr156;
	} else if ( (*p) > 93 ) {
		if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr156;
		} else if ( (*p) >= 95 )
			goto tr156;
	} else
		goto tr156;
	goto tr0;
tr190:
/* #line 259 "EcrioCPIMParse.rl" */
	{
		if (bIsMultiPart == Enum_TRUE)
		{
			pal_MemorySet(cBoundary, 0, CPIM_BOUNDARY_LENGTH);
			if (NULL == pal_StringNCopy(cBoundary, CPIM_BOUNDARY_LENGTH, (u_char*)tag_start + 1, (p - tag_start - 2)))
			{
				CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.", __FUNCTION__, __LINE__);
				uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
				goto END;
			}
		}
	}
	goto st147;
st147:
	if ( ++p == pe )
		goto _test_eof147;
case 147:
/* #line 3542 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st145;
		case 32: goto st147;
		case 37: goto tr156;
		case 59: goto st113;
		case 61: goto tr174;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr156;
		} else if ( (*p) >= 33 )
			goto tr156;
	} else if ( (*p) > 93 ) {
		if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr156;
		} else if ( (*p) >= 95 )
			goto tr156;
	} else
		goto tr156;
	goto tr0;
st148:
	if ( ++p == pe )
		goto _test_eof148;
case 148:
	if ( (*p) == 121 )
		goto st103;
	goto tr0;
tr3:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st149;
st149:
	if ( ++p == pe )
		goto _test_eof149;
case 149:
/* #line 3582 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
tr5:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st150;
st150:
	if ( ++p == pe )
		goto _test_eof150;
case 150:
/* #line 3618 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 97: goto st151;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st151:
	if ( ++p == pe )
		goto _test_eof151;
case 151:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 116: goto st152;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st152:
	if ( ++p == pe )
		goto _test_eof152;
case 152:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 101: goto st153;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st153:
	if ( ++p == pe )
		goto _test_eof153;
case 153:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 84: goto st154;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st154:
	if ( ++p == pe )
		goto _test_eof154;
case 154:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 105: goto st155;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st155:
	if ( ++p == pe )
		goto _test_eof155;
case 155:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 109: goto st156;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st156:
	if ( ++p == pe )
		goto _test_eof156;
case 156:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 101: goto st157;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st157:
	if ( ++p == pe )
		goto _test_eof157;
case 157:
	switch( (*p) ) {
		case 9: goto st158;
		case 32: goto st158;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st159;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st158:
	if ( ++p == pe )
		goto _test_eof158;
case 158:
	switch( (*p) ) {
		case 9: goto st158;
		case 32: goto st158;
		case 58: goto st159;
	}
	goto tr0;
st159:
	if ( ++p == pe )
		goto _test_eof159;
case 159:
	switch( (*p) ) {
		case 13: goto tr205;
		case 32: goto st159;
		case 37: goto tr206;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr206;
		} else if ( (*p) >= 33 )
			goto tr206;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr206;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr206;
		} else
			goto tr206;
	} else
		goto tr206;
	goto tr0;
tr205:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st160;
st160:
	if ( ++p == pe )
		goto _test_eof160;
case 160:
/* #line 3902 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st161;
	goto tr0;
st161:
	if ( ++p == pe )
		goto _test_eof161;
case 161:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr208;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr208:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st162;
st162:
	if ( ++p == pe )
		goto _test_eof162;
case 162:
/* #line 3955 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st162;
		case 37: goto tr210;
		case 58: goto tr206;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr210;
		} else if ( (*p) >= 33 )
			goto tr210;
	} else if ( (*p) > 57 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr210;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr210;
		} else
			goto tr210;
	} else
		goto tr210;
	goto tr0;
tr210:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st163;
st163:
	if ( ++p == pe )
		goto _test_eof163;
case 163:
/* #line 3990 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr211;
		case 32: goto tr212;
		case 37: goto st163;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st163;
		} else if ( (*p) >= 33 )
			goto st163;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st163;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st163;
		} else
			goto st163;
	} else
		goto st163;
	goto tr0;
tr212:
/* #line 359 "EcrioCPIMParse.rl" */
	{
		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pDateTime,
			(u_char*)tag_start,
			(p - tag_start));
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st164;
tr256:
/* #line 156 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pFromAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st164;
tr290:
/* #line 489 "EcrioCPIMParse.rl" */
	{
		CPIMLOGI(c->logHandle, KLogTypeGeneral, "%s:%u\tIMDN-Record-Route.", __FUNCTION__, __LINE__);
	}
	goto st164;
tr364:
/* #line 381 "EcrioCPIMParse.rl" */
	{
		bIsMyFeaturesNS = Enum_TRUE;
	}
	goto st164;
tr411:
/* #line 87 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pToAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st164;
tr509:
/* #line 402 "EcrioCPIMParse.rl" */
	{
		if (bIsMaapNS == Enum_TRUE)
		{
			if (pal_StringNCompare((u_char*)tag_start, CPIM_ADVERTISEMENT_STRING,
						pal_StringLength(CPIM_ADVERTISEMENT_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Advertisement;
			}
			else if (pal_StringNCompare((u_char*)tag_start, CPIM_PAYMENT_STRING,
						pal_StringLength(CPIM_PAYMENT_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Payment;
			}
			else if (pal_StringNCompare((u_char*)tag_start, CPIM_PREMIUM_STRING,
						pal_StringLength(CPIM_PREMIUM_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Premium;
			}
			else if (pal_StringNCompare((u_char*)tag_start, CPIM_SUBSCRIPTION_STRING,
						pal_StringLength(CPIM_SUBSCRIPTION_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Subscription;
			}
			else if (pal_StringNCompare((u_char*)tag_start, CPIM_PLUGIN_STRING,
						pal_StringLength(CPIM_PLUGIN_STRING) == 0))
			{
				pStruct->trfType = CPIMTrafficType_Plugin;
			}
			else
			{
				pStruct->trfType = CPIMTrafficType_None;
			}
		}
	}
	goto st164;
tr544:
/* #line 386 "EcrioCPIMParse.rl" */
	{
		if (bIsImdnNS == Enum_TRUE)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pMsgId,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
			pStruct->uMsgIdLen = pal_StringLength(pStruct->pMsgId);
		}
	}
	goto st164;
tr568:
/* #line 504 "EcrioCPIMParse.rl" */
	{
		CPIMLOGI(c->logHandle, KLogTypeGeneral, "%s:%u\timdn.Original-To.", __FUNCTION__, __LINE__);
	}
	goto st164;
tr630:
/* #line 371 "EcrioCPIMParse.rl" */
	{
		bIsImdnNS = Enum_TRUE;
	}
	goto st164;
tr648:
/* #line 376 "EcrioCPIMParse.rl" */
	{
		bIsMaapNS = Enum_TRUE;
	}
	goto st164;
tr716:
/* #line 494 "EcrioCPIMParse.rl" */
	{
		CPIMLOGI(c->logHandle, KLogTypeGeneral, "%s:%u\tIMDN-Route.", __FUNCTION__, __LINE__);
	}
	goto st164;
st164:
	if ( ++p == pe )
		goto _test_eof164;
case 164:
/* #line 4261 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st4;
		case 32: goto st164;
	}
	goto tr0;
tr206:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st165;
st165:
	if ( ++p == pe )
		goto _test_eof165;
case 165:
/* #line 4277 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr216;
		case 32: goto tr217;
		case 37: goto st165;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st165;
		} else if ( (*p) >= 33 )
			goto st165;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st165;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st165;
		} else
			goto st165;
	} else
		goto st165;
	goto tr0;
tr6:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st166;
st166:
	if ( ++p == pe )
		goto _test_eof166;
case 166:
/* #line 4311 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 114: goto st167;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st167:
	if ( ++p == pe )
		goto _test_eof167;
case 167:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 111: goto st168;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st168:
	if ( ++p == pe )
		goto _test_eof168;
case 168:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 109: goto st169;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st169:
	if ( ++p == pe )
		goto _test_eof169;
case 169:
	switch( (*p) ) {
		case 9: goto st170;
		case 32: goto st170;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st171;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st170:
	if ( ++p == pe )
		goto _test_eof170;
case 170:
	switch( (*p) ) {
		case 9: goto st170;
		case 32: goto st170;
		case 58: goto st171;
	}
	goto tr0;
tr225:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st171;
st171:
	if ( ++p == pe )
		goto _test_eof171;
case 171:
/* #line 4461 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr224;
		case 32: goto tr225;
		case 33: goto tr226;
		case 34: goto tr227;
		case 37: goto tr226;
		case 39: goto tr226;
		case 47: goto tr228;
		case 58: goto tr228;
		case 59: goto tr229;
		case 60: goto tr230;
		case 62: goto st3;
		case 92: goto st3;
		case 96: goto tr231;
		case 123: goto st3;
		case 125: goto st3;
		case 126: goto tr226;
	}
	if ( (*p) < 63 ) {
		if ( (*p) < 42 ) {
			if ( 40 <= (*p) && (*p) <= 41 )
				goto tr228;
		} else if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 61 )
				goto tr226;
		} else
			goto tr226;
	} else if ( (*p) > 64 ) {
		if ( (*p) < 91 ) {
			if ( 65 <= (*p) && (*p) <= 90 )
				goto tr226;
		} else if ( (*p) > 93 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr226;
		} else
			goto tr228;
	} else
		goto tr228;
	goto tr0;
tr224:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st172;
st172:
	if ( ++p == pe )
		goto _test_eof172;
case 172:
/* #line 4530 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st173;
	goto tr0;
st173:
	if ( ++p == pe )
		goto _test_eof173;
case 173:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr233;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr233:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st174;
tr235:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st174;
st174:
	if ( ++p == pe )
		goto _test_eof174;
case 174:
/* #line 4603 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 13: goto tr234;
		case 32: goto tr235;
		case 33: goto tr236;
		case 34: goto tr237;
		case 37: goto tr236;
		case 39: goto tr236;
		case 47: goto tr229;
		case 58: goto tr228;
		case 59: goto tr229;
		case 60: goto tr238;
		case 91: goto tr229;
		case 93: goto tr229;
		case 96: goto tr239;
		case 126: goto tr236;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 41 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr236;
		} else if ( (*p) >= 40 )
			goto tr229;
	} else if ( (*p) > 61 ) {
		if ( (*p) < 65 ) {
			if ( 63 <= (*p) && (*p) <= 64 )
				goto tr229;
		} else if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr236;
		} else
			goto tr236;
	} else
		goto tr236;
	goto tr0;
tr234:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st175;
st175:
	if ( ++p == pe )
		goto _test_eof175;
case 175:
/* #line 4663 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st176;
	goto tr0;
st176:
	if ( ++p == pe )
		goto _test_eof176;
case 176:
	if ( (*p) == 32 )
		goto st177;
	goto tr0;
st177:
	if ( ++p == pe )
		goto _test_eof177;
case 177:
	switch( (*p) ) {
		case 32: goto st177;
		case 34: goto st178;
		case 60: goto st185;
	}
	goto tr0;
st178:
	if ( ++p == pe )
		goto _test_eof178;
case 178:
	switch( (*p) ) {
		case 13: goto st179;
		case 34: goto st181;
		case 92: goto st528;
	}
	if ( 32 <= (*p) && (*p) <= 126 )
		goto st178;
	goto tr0;
st179:
	if ( ++p == pe )
		goto _test_eof179;
case 179:
	if ( (*p) == 10 )
		goto st180;
	goto tr0;
st180:
	if ( ++p == pe )
		goto _test_eof180;
case 180:
	if ( (*p) == 32 )
		goto st178;
	goto tr0;
st181:
	if ( ++p == pe )
		goto _test_eof181;
case 181:
	switch( (*p) ) {
		case 13: goto tr248;
		case 32: goto tr249;
		case 37: goto tr250;
		case 60: goto tr251;
		case 93: goto tr250;
		case 95: goto tr250;
		case 126: goto tr250;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr250;
		} else if ( (*p) >= 33 )
			goto tr250;
	} else if ( (*p) > 61 ) {
		if ( (*p) > 91 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr250;
		} else if ( (*p) >= 63 )
			goto tr250;
	} else
		goto tr250;
	goto tr0;
tr248:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st182;
st182:
	if ( ++p == pe )
		goto _test_eof182;
case 182:
/* #line 4758 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st183;
	goto tr0;
st183:
	if ( ++p == pe )
		goto _test_eof183;
case 183:
	if ( (*p) == 32 )
		goto st184;
	goto tr0;
st184:
	if ( ++p == pe )
		goto _test_eof184;
case 184:
	switch( (*p) ) {
		case 32: goto st184;
		case 60: goto st185;
	}
	goto tr0;
tr238:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st185;
tr251:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st185;
st185:
	if ( ++p == pe )
		goto _test_eof185;
case 185:
/* #line 4818 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 37: goto tr254;
		case 61: goto tr254;
		case 93: goto tr254;
		case 95: goto tr254;
		case 126: goto tr254;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr254;
		} else if ( (*p) >= 33 )
			goto tr254;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 91 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr254;
		} else if ( (*p) >= 63 )
			goto tr254;
	} else
		goto tr254;
	goto tr0;
tr254:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st186;
tr229:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st186;
tr250:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st186;
st186:
	if ( ++p == pe )
		goto _test_eof186;
case 186:
/* #line 4891 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr255;
		case 32: goto tr256;
		case 37: goto st186;
		case 62: goto tr258;
		case 93: goto st186;
		case 95: goto st186;
		case 126: goto st186;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st186;
		} else if ( (*p) >= 33 )
			goto st186;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 91 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st186;
		} else if ( (*p) >= 61 )
			goto st186;
	} else
		goto st186;
	goto tr0;
tr258:
/* #line 156 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pFromAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st187;
tr413:
/* #line 87 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pToAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st187;
st187:
	if ( ++p == pe )
		goto _test_eof187;
case 187:
/* #line 5060 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st188;
		case 32: goto st187;
	}
	goto tr0;
tr424:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st188;
st188:
	if ( ++p == pe )
		goto _test_eof188;
case 188:
/* #line 5077 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st189;
	goto tr0;
st189:
	if ( ++p == pe )
		goto _test_eof189;
case 189:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr262;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr262:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st190;
st190:
	if ( ++p == pe )
		goto _test_eof190;
case 190:
/* #line 5130 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 13: goto st4;
		case 32: goto st190;
		case 58: goto st3;
	}
	goto tr0;
tr7:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st191;
st191:
	if ( ++p == pe )
		goto _test_eof191;
case 191:
/* #line 5148 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 77: goto st192;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st192:
	if ( ++p == pe )
		goto _test_eof192;
case 192:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 68: goto st193;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st193:
	if ( ++p == pe )
		goto _test_eof193;
case 193:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 78: goto st194;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st194:
	if ( ++p == pe )
		goto _test_eof194;
case 194:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 45: goto st195;
		case 46: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( 42 <= (*p) && (*p) <= 43 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st195:
	if ( ++p == pe )
		goto _test_eof195;
case 195:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 82: goto st196;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st196:
	if ( ++p == pe )
		goto _test_eof196;
case 196:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 101: goto st197;
		case 111: goto st516;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st197:
	if ( ++p == pe )
		goto _test_eof197;
case 197:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 99: goto st198;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st198:
	if ( ++p == pe )
		goto _test_eof198;
case 198:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 111: goto st199;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st199:
	if ( ++p == pe )
		goto _test_eof199;
case 199:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 114: goto st200;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st200:
	if ( ++p == pe )
		goto _test_eof200;
case 200:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 100: goto st201;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st201:
	if ( ++p == pe )
		goto _test_eof201;
case 201:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 45: goto st202;
		case 46: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( 42 <= (*p) && (*p) <= 43 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st202:
	if ( ++p == pe )
		goto _test_eof202;
case 202:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 82: goto st203;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st203:
	if ( ++p == pe )
		goto _test_eof203;
case 203:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 111: goto st204;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st204:
	if ( ++p == pe )
		goto _test_eof204;
case 204:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 117: goto st205;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st205:
	if ( ++p == pe )
		goto _test_eof205;
case 205:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 116: goto st206;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st206:
	if ( ++p == pe )
		goto _test_eof206;
case 206:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 101: goto st207;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st207:
	if ( ++p == pe )
		goto _test_eof207;
case 207:
	switch( (*p) ) {
		case 9: goto st208;
		case 32: goto st208;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st209;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st208:
	if ( ++p == pe )
		goto _test_eof208;
case 208:
	switch( (*p) ) {
		case 9: goto st208;
		case 32: goto st208;
		case 58: goto st209;
	}
	goto tr0;
st209:
	if ( ++p == pe )
		goto _test_eof209;
case 209:
	switch( (*p) ) {
		case 13: goto tr283;
		case 32: goto st209;
		case 37: goto tr284;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr284;
		} else if ( (*p) >= 33 )
			goto tr284;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr284;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr284;
		} else
			goto tr284;
	} else
		goto tr284;
	goto tr0;
tr283:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st210;
st210:
	if ( ++p == pe )
		goto _test_eof210;
case 210:
/* #line 5699 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st211;
	goto tr0;
st211:
	if ( ++p == pe )
		goto _test_eof211;
case 211:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr286;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr286:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st212;
st212:
	if ( ++p == pe )
		goto _test_eof212;
case 212:
/* #line 5752 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st212;
		case 37: goto tr288;
		case 58: goto tr284;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr288;
		} else if ( (*p) >= 33 )
			goto tr288;
	} else if ( (*p) > 57 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr288;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr288;
		} else
			goto tr288;
	} else
		goto tr288;
	goto tr0;
tr288:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st213;
st213:
	if ( ++p == pe )
		goto _test_eof213;
case 213:
/* #line 5787 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr289;
		case 32: goto tr290;
		case 37: goto st213;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st213;
		} else if ( (*p) >= 33 )
			goto st213;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st213;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st213;
		} else
			goto st213;
	} else
		goto st213;
	goto tr0;
tr284:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st214;
st214:
	if ( ++p == pe )
		goto _test_eof214;
case 214:
/* #line 5821 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr292;
		case 32: goto tr293;
		case 37: goto st214;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st214;
		} else if ( (*p) >= 33 )
			goto st214;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st214;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st214;
		} else
			goto st214;
	} else
		goto st214;
	goto tr0;
tr8:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st215;
st215:
	if ( ++p == pe )
		goto _test_eof215;
case 215:
/* #line 5855 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 121: goto st216;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st216:
	if ( ++p == pe )
		goto _test_eof216;
case 216:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 70: goto st217;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st217:
	if ( ++p == pe )
		goto _test_eof217;
case 217:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 101: goto st218;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st218:
	if ( ++p == pe )
		goto _test_eof218;
case 218:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 97: goto st219;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st219:
	if ( ++p == pe )
		goto _test_eof219;
case 219:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 116: goto st220;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st220:
	if ( ++p == pe )
		goto _test_eof220;
case 220:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 117: goto st221;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st221:
	if ( ++p == pe )
		goto _test_eof221;
case 221:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 114: goto st222;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st222:
	if ( ++p == pe )
		goto _test_eof222;
case 222:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 101: goto st223;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st223:
	if ( ++p == pe )
		goto _test_eof223;
case 223:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 115: goto st224;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st224:
	if ( ++p == pe )
		goto _test_eof224;
case 224:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 45: goto st149;
		case 46: goto st225;
		case 58: goto st3;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( 42 <= (*p) && (*p) <= 43 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st225:
	if ( ++p == pe )
		goto _test_eof225;
case 225:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 80: goto st226;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st226:
	if ( ++p == pe )
		goto _test_eof226;
case 226:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 65: goto st227;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 66 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st227:
	if ( ++p == pe )
		goto _test_eof227;
case 227:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 78: goto st228;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st228:
	if ( ++p == pe )
		goto _test_eof228;
case 228:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 73: goto st229;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st229:
	if ( ++p == pe )
		goto _test_eof229;
case 229:
	switch( (*p) ) {
		case 9: goto st230;
		case 32: goto st230;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st231;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st230:
	if ( ++p == pe )
		goto _test_eof230;
case 230:
	switch( (*p) ) {
		case 9: goto st230;
		case 32: goto st230;
		case 58: goto st231;
	}
	goto tr0;
st231:
	if ( ++p == pe )
		goto _test_eof231;
case 231:
	switch( (*p) ) {
		case 13: goto tr311;
		case 32: goto st231;
		case 37: goto tr312;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr312;
		} else if ( (*p) >= 33 )
			goto tr312;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr312;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr312;
		} else
			goto tr312;
	} else
		goto tr312;
	goto tr0;
tr311:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st232;
st232:
	if ( ++p == pe )
		goto _test_eof232;
case 232:
/* #line 6347 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st233;
	goto tr0;
st233:
	if ( ++p == pe )
		goto _test_eof233;
case 233:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr314;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr314:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st234;
st234:
	if ( ++p == pe )
		goto _test_eof234;
case 234:
/* #line 6400 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st234;
		case 37: goto tr316;
		case 58: goto tr312;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr316;
		} else if ( (*p) >= 33 )
			goto tr316;
	} else if ( (*p) > 57 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr316;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr316;
		} else
			goto tr316;
	} else
		goto tr316;
	goto tr0;
tr316:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st235;
st235:
	if ( ++p == pe )
		goto _test_eof235;
case 235:
/* #line 6435 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr317;
		case 32: goto tr318;
		case 37: goto st235;
		case 59: goto st239;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st235;
		} else if ( (*p) >= 33 )
			goto st235;
	} else if ( (*p) > 93 ) {
		if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st235;
		} else if ( (*p) >= 95 )
			goto st235;
	} else
		goto st235;
	goto tr0;
tr702:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st236;
tr317:
/* #line 438 "EcrioCPIMParse.rl" */
	{
		if (bIsMyFeaturesNS == Enum_TRUE)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pPANI,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st236;
tr699:
/* #line 438 "EcrioCPIMParse.rl" */
	{
		if (bIsMyFeaturesNS == Enum_TRUE)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pPANI,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st236;
st236:
	if ( ++p == pe )
		goto _test_eof236;
case 236:
/* #line 6505 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st237;
	goto tr0;
st237:
	if ( ++p == pe )
		goto _test_eof237;
case 237:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr322;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr322:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st238;
st238:
	if ( ++p == pe )
		goto _test_eof238;
case 238:
/* #line 6558 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st238;
		case 58: goto st3;
		case 59: goto st239;
	}
	goto tr0;
st239:
	if ( ++p == pe )
		goto _test_eof239;
case 239:
	switch( (*p) ) {
		case 13: goto st240;
		case 32: goto st239;
		case 37: goto st235;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st235;
		} else if ( (*p) >= 33 )
			goto st235;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st235;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st235;
		} else
			goto st235;
	} else
		goto st235;
	goto tr0;
st240:
	if ( ++p == pe )
		goto _test_eof240;
case 240:
	if ( (*p) == 10 )
		goto st241;
	goto tr0;
st241:
	if ( ++p == pe )
		goto _test_eof241;
case 241:
	if ( (*p) == 32 )
		goto st242;
	goto tr0;
st242:
	if ( ++p == pe )
		goto _test_eof242;
case 242:
	switch( (*p) ) {
		case 32: goto st242;
		case 37: goto st235;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st235;
		} else if ( (*p) >= 33 )
			goto st235;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st235;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st235;
		} else
			goto st235;
	} else
		goto st235;
	goto tr0;
tr9:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st243;
st243:
	if ( ++p == pe )
		goto _test_eof243;
case 243:
/* #line 6643 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 83: goto st244;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st244:
	if ( ++p == pe )
		goto _test_eof244;
case 244:
	switch( (*p) ) {
		case 9: goto st245;
		case 32: goto st245;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st246;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st245:
	if ( ++p == pe )
		goto _test_eof245;
case 245:
	switch( (*p) ) {
		case 9: goto st245;
		case 32: goto st245;
		case 58: goto st246;
	}
	goto tr0;
tr331:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st246;
st246:
	if ( ++p == pe )
		goto _test_eof246;
case 246:
/* #line 6719 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr330;
		case 32: goto tr331;
		case 37: goto tr4;
		case 77: goto tr332;
		case 105: goto tr333;
		case 109: goto tr334;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr4;
		} else if ( (*p) >= 33 )
			goto tr4;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr4;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr4;
		} else
			goto tr4;
	} else
		goto tr4;
	goto tr0;
tr330:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st247;
st247:
	if ( ++p == pe )
		goto _test_eof247;
case 247:
/* #line 6761 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st248;
	goto tr0;
st248:
	if ( ++p == pe )
		goto _test_eof248;
case 248:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr336;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr336:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st249;
st249:
	if ( ++p == pe )
		goto _test_eof249;
case 249:
/* #line 6814 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 13: goto tr337;
		case 32: goto tr338;
		case 37: goto tr339;
		case 58: goto tr4;
		case 77: goto tr340;
		case 105: goto tr341;
		case 109: goto tr342;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr339;
		} else if ( (*p) >= 33 )
			goto tr339;
	} else if ( (*p) > 57 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr339;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr339;
		} else
			goto tr339;
	} else
		goto tr339;
	goto tr0;
tr338:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st250;
st250:
	if ( ++p == pe )
		goto _test_eof250;
case 250:
/* #line 6853 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 13: goto tr337;
		case 32: goto tr338;
		case 37: goto tr339;
		case 58: goto tr4;
		case 77: goto tr343;
		case 105: goto tr344;
		case 109: goto tr345;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr339;
		} else if ( (*p) >= 33 )
			goto tr339;
	} else if ( (*p) > 57 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr339;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr339;
		} else
			goto tr339;
	} else
		goto tr339;
	goto tr0;
tr339:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st251;
tr617:
/* #line 381 "EcrioCPIMParse.rl" */
	{
		bIsMyFeaturesNS = Enum_TRUE;
	}
	goto st251;
tr635:
/* #line 371 "EcrioCPIMParse.rl" */
	{
		bIsImdnNS = Enum_TRUE;
	}
	goto st251;
tr653:
/* #line 376 "EcrioCPIMParse.rl" */
	{
		bIsMaapNS = Enum_TRUE;
	}
	goto st251;
st251:
	if ( ++p == pe )
		goto _test_eof251;
case 251:
/* #line 6910 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st4;
		case 37: goto st251;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st251;
		} else if ( (*p) >= 32 )
			goto st251;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st251;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st251;
		} else
			goto st251;
	} else
		goto st251;
	goto tr0;
tr343:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st252;
st252:
	if ( ++p == pe )
		goto _test_eof252;
case 252:
/* #line 6943 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st4;
		case 37: goto st251;
		case 121: goto st253;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st251;
		} else if ( (*p) >= 32 )
			goto st251;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st251;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st251;
		} else
			goto st251;
	} else
		goto st251;
	goto tr0;
st253:
	if ( ++p == pe )
		goto _test_eof253;
case 253:
	switch( (*p) ) {
		case 13: goto st4;
		case 37: goto st251;
		case 70: goto st254;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st251;
		} else if ( (*p) >= 32 )
			goto st251;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st251;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st251;
		} else
			goto st251;
	} else
		goto st251;
	goto tr0;
st254:
	if ( ++p == pe )
		goto _test_eof254;
case 254:
	switch( (*p) ) {
		case 13: goto st4;
		case 37: goto st251;
		case 101: goto st255;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st251;
		} else if ( (*p) >= 32 )
			goto st251;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st251;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st251;
		} else
			goto st251;
	} else
		goto st251;
	goto tr0;
st255:
	if ( ++p == pe )
		goto _test_eof255;
case 255:
	switch( (*p) ) {
		case 13: goto st4;
		case 37: goto st251;
		case 97: goto st256;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st251;
		} else if ( (*p) >= 32 )
			goto st251;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st251;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st251;
		} else
			goto st251;
	} else
		goto st251;
	goto tr0;
st256:
	if ( ++p == pe )
		goto _test_eof256;
case 256:
	switch( (*p) ) {
		case 13: goto st4;
		case 37: goto st251;
		case 116: goto st257;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st251;
		} else if ( (*p) >= 32 )
			goto st251;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st251;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st251;
		} else
			goto st251;
	} else
		goto st251;
	goto tr0;
st257:
	if ( ++p == pe )
		goto _test_eof257;
case 257:
	switch( (*p) ) {
		case 13: goto st4;
		case 37: goto st251;
		case 117: goto st258;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st251;
		} else if ( (*p) >= 32 )
			goto st251;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st251;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st251;
		} else
			goto st251;
	} else
		goto st251;
	goto tr0;
st258:
	if ( ++p == pe )
		goto _test_eof258;
case 258:
	switch( (*p) ) {
		case 13: goto st4;
		case 37: goto st251;
		case 114: goto st259;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st251;
		} else if ( (*p) >= 32 )
			goto st251;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st251;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st251;
		} else
			goto st251;
	} else
		goto st251;
	goto tr0;
st259:
	if ( ++p == pe )
		goto _test_eof259;
case 259:
	switch( (*p) ) {
		case 13: goto st4;
		case 37: goto st251;
		case 101: goto st260;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st251;
		} else if ( (*p) >= 32 )
			goto st251;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st251;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st251;
		} else
			goto st251;
	} else
		goto st251;
	goto tr0;
st260:
	if ( ++p == pe )
		goto _test_eof260;
case 260:
	switch( (*p) ) {
		case 13: goto st4;
		case 37: goto st251;
		case 115: goto st261;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st251;
		} else if ( (*p) >= 32 )
			goto st251;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st251;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st251;
		} else
			goto st251;
	} else
		goto st251;
	goto tr0;
st261:
	if ( ++p == pe )
		goto _test_eof261;
case 261:
	switch( (*p) ) {
		case 13: goto st262;
		case 32: goto st261;
		case 37: goto tr357;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr357;
		} else if ( (*p) >= 33 )
			goto tr357;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr357;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr357;
		} else
			goto tr357;
	} else
		goto tr357;
	goto tr0;
tr688:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st262;
st262:
	if ( ++p == pe )
		goto _test_eof262;
case 262:
/* #line 7221 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st263;
	goto tr0;
st263:
	if ( ++p == pe )
		goto _test_eof263;
case 263:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr359;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr359:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st264;
st264:
	if ( ++p == pe )
		goto _test_eof264;
case 264:
/* #line 7274 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st264;
		case 37: goto tr361;
		case 58: goto tr362;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr361;
		} else if ( (*p) >= 33 )
			goto tr361;
	} else if ( (*p) > 57 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr361;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr361;
		} else
			goto tr361;
	} else
		goto tr361;
	goto tr0;
tr361:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st265;
st265:
	if ( ++p == pe )
		goto _test_eof265;
case 265:
/* #line 7309 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr363;
		case 32: goto tr364;
		case 37: goto st265;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st265;
		} else if ( (*p) >= 33 )
			goto st265;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st265;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st265;
		} else
			goto st265;
	} else
		goto st265;
	goto tr0;
tr362:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st266;
st266:
	if ( ++p == pe )
		goto _test_eof266;
case 266:
/* #line 7343 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr366;
		case 32: goto tr367;
		case 37: goto st266;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st266;
		} else if ( (*p) >= 33 )
			goto st266;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st266;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st266;
		} else
			goto st266;
	} else
		goto st266;
	goto tr0;
tr10:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st267;
st267:
	if ( ++p == pe )
		goto _test_eof267;
case 267:
/* #line 7377 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 117: goto st268;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st268:
	if ( ++p == pe )
		goto _test_eof268;
case 268:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 98: goto st269;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st269:
	if ( ++p == pe )
		goto _test_eof269;
case 269:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 106: goto st270;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st270:
	if ( ++p == pe )
		goto _test_eof270;
case 270:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 101: goto st271;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st271:
	if ( ++p == pe )
		goto _test_eof271;
case 271:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 99: goto st272;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st272:
	if ( ++p == pe )
		goto _test_eof272;
case 272:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 116: goto st273;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st273:
	if ( ++p == pe )
		goto _test_eof273;
case 273:
	switch( (*p) ) {
		case 9: goto st274;
		case 32: goto st274;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st275;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st274:
	if ( ++p == pe )
		goto _test_eof274;
case 274:
	switch( (*p) ) {
		case 9: goto st274;
		case 32: goto st274;
		case 58: goto st275;
	}
	goto tr0;
st275:
	if ( ++p == pe )
		goto _test_eof275;
case 275:
	switch( (*p) ) {
		case 13: goto tr377;
		case 37: goto tr378;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr378;
		} else if ( (*p) >= 32 )
			goto tr378;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr378;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr378;
		} else
			goto tr378;
	} else
		goto tr378;
	goto tr0;
tr378:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st276;
st276:
	if ( ++p == pe )
		goto _test_eof276;
case 276:
/* #line 7629 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr379;
		case 37: goto st276;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st276;
		} else if ( (*p) >= 32 )
			goto st276;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st276;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st276;
		} else
			goto st276;
	} else
		goto st276;
	goto tr0;
tr11:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st277;
st277:
	if ( ++p == pe )
		goto _test_eof277;
case 277:
/* #line 7662 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 111: goto st278;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st278:
	if ( ++p == pe )
		goto _test_eof278;
case 278:
	switch( (*p) ) {
		case 9: goto st279;
		case 32: goto st279;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st280;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st279:
	if ( ++p == pe )
		goto _test_eof279;
case 279:
	switch( (*p) ) {
		case 9: goto st279;
		case 32: goto st279;
		case 58: goto st280;
	}
	goto tr0;
st280:
	if ( ++p == pe )
		goto _test_eof280;
case 280:
	switch( (*p) ) {
		case 13: goto tr384;
		case 32: goto st280;
		case 33: goto tr385;
		case 34: goto tr386;
		case 37: goto tr385;
		case 39: goto tr385;
		case 47: goto tr387;
		case 58: goto tr387;
		case 59: goto tr388;
		case 60: goto st432;
		case 62: goto st3;
		case 92: goto st3;
		case 96: goto st443;
		case 123: goto st3;
		case 125: goto st3;
		case 126: goto tr385;
	}
	if ( (*p) < 63 ) {
		if ( (*p) < 42 ) {
			if ( 40 <= (*p) && (*p) <= 41 )
				goto tr387;
		} else if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 61 )
				goto tr385;
		} else
			goto tr385;
	} else if ( (*p) > 64 ) {
		if ( (*p) < 91 ) {
			if ( 65 <= (*p) && (*p) <= 90 )
				goto tr385;
		} else if ( (*p) > 93 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr385;
		} else
			goto tr387;
	} else
		goto tr387;
	goto tr0;
tr384:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st281;
st281:
	if ( ++p == pe )
		goto _test_eof281;
case 281:
/* #line 7782 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st282;
	goto tr0;
st282:
	if ( ++p == pe )
		goto _test_eof282;
case 282:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr392;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr392:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st283;
st283:
	if ( ++p == pe )
		goto _test_eof283;
case 283:
/* #line 7835 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 13: goto st284;
		case 32: goto st283;
		case 33: goto tr395;
		case 34: goto tr396;
		case 37: goto tr395;
		case 39: goto tr395;
		case 47: goto tr388;
		case 58: goto tr387;
		case 59: goto tr388;
		case 60: goto st294;
		case 91: goto tr388;
		case 93: goto tr388;
		case 96: goto st304;
		case 126: goto tr395;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 41 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr395;
		} else if ( (*p) >= 40 )
			goto tr388;
	} else if ( (*p) > 61 ) {
		if ( (*p) < 65 ) {
			if ( 63 <= (*p) && (*p) <= 64 )
				goto tr388;
		} else if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr395;
		} else
			goto tr395;
	} else
		goto tr395;
	goto tr0;
st284:
	if ( ++p == pe )
		goto _test_eof284;
case 284:
	if ( (*p) == 10 )
		goto st285;
	goto tr0;
st285:
	if ( ++p == pe )
		goto _test_eof285;
case 285:
	if ( (*p) == 32 )
		goto st286;
	goto tr0;
st286:
	if ( ++p == pe )
		goto _test_eof286;
case 286:
	switch( (*p) ) {
		case 32: goto st286;
		case 34: goto st287;
		case 60: goto st294;
	}
	goto tr0;
st287:
	if ( ++p == pe )
		goto _test_eof287;
case 287:
	switch( (*p) ) {
		case 13: goto st288;
		case 34: goto st290;
		case 92: goto st297;
	}
	if ( 32 <= (*p) && (*p) <= 126 )
		goto st287;
	goto tr0;
st288:
	if ( ++p == pe )
		goto _test_eof288;
case 288:
	if ( (*p) == 10 )
		goto st289;
	goto tr0;
st289:
	if ( ++p == pe )
		goto _test_eof289;
case 289:
	if ( (*p) == 32 )
		goto st287;
	goto tr0;
st290:
	if ( ++p == pe )
		goto _test_eof290;
case 290:
	switch( (*p) ) {
		case 13: goto st291;
		case 32: goto st296;
		case 37: goto tr388;
		case 60: goto st294;
		case 93: goto tr388;
		case 95: goto tr388;
		case 126: goto tr388;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr388;
		} else if ( (*p) >= 33 )
			goto tr388;
	} else if ( (*p) > 61 ) {
		if ( (*p) > 91 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr388;
		} else if ( (*p) >= 63 )
			goto tr388;
	} else
		goto tr388;
	goto tr0;
st291:
	if ( ++p == pe )
		goto _test_eof291;
case 291:
	if ( (*p) == 10 )
		goto st292;
	goto tr0;
st292:
	if ( ++p == pe )
		goto _test_eof292;
case 292:
	if ( (*p) == 32 )
		goto st293;
	goto tr0;
st293:
	if ( ++p == pe )
		goto _test_eof293;
case 293:
	switch( (*p) ) {
		case 32: goto st293;
		case 60: goto st294;
	}
	goto tr0;
st294:
	if ( ++p == pe )
		goto _test_eof294;
case 294:
	switch( (*p) ) {
		case 37: goto tr388;
		case 61: goto tr388;
		case 93: goto tr388;
		case 95: goto tr388;
		case 126: goto tr388;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr388;
		} else if ( (*p) >= 33 )
			goto tr388;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 91 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr388;
		} else if ( (*p) >= 63 )
			goto tr388;
	} else
		goto tr388;
	goto tr0;
tr388:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st295;
st295:
	if ( ++p == pe )
		goto _test_eof295;
case 295:
/* #line 8008 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr410;
		case 32: goto tr411;
		case 37: goto st295;
		case 62: goto tr413;
		case 93: goto st295;
		case 95: goto st295;
		case 126: goto st295;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st295;
		} else if ( (*p) >= 33 )
			goto st295;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 91 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st295;
		} else if ( (*p) >= 61 )
			goto st295;
	} else
		goto st295;
	goto tr0;
st296:
	if ( ++p == pe )
		goto _test_eof296;
case 296:
	switch( (*p) ) {
		case 13: goto st291;
		case 32: goto st296;
		case 60: goto st294;
	}
	goto tr0;
st297:
	if ( ++p == pe )
		goto _test_eof297;
case 297:
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 9 )
			goto st287;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) )
			goto st287;
	} else
		goto st287;
	goto tr0;
tr395:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st298;
st298:
	if ( ++p == pe )
		goto _test_eof298;
case 298:
/* #line 8066 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr414;
		case 32: goto tr415;
		case 33: goto st298;
		case 34: goto st295;
		case 37: goto st298;
		case 39: goto st298;
		case 47: goto st295;
		case 61: goto st298;
		case 62: goto tr413;
		case 91: goto st295;
		case 93: goto st295;
		case 96: goto st304;
		case 126: goto st298;
	}
	if ( (*p) < 58 ) {
		if ( (*p) < 42 ) {
			if ( 40 <= (*p) && (*p) <= 41 )
				goto st295;
		} else if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 57 )
				goto st298;
		} else
			goto st298;
	} else if ( (*p) > 59 ) {
		if ( (*p) < 65 ) {
			if ( 63 <= (*p) && (*p) <= 64 )
				goto st295;
		} else if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st298;
		} else
			goto st298;
	} else
		goto st295;
	goto tr0;
tr606:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st299;
tr414:
/* #line 87 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pToAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st299;
tr603:
/* #line 87 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pToAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st299;
st299:
	if ( ++p == pe )
		goto _test_eof299;
case 299:
/* #line 8259 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st300;
	goto tr0;
st300:
	if ( ++p == pe )
		goto _test_eof300;
case 300:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr418;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr418:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st301;
st301:
	if ( ++p == pe )
		goto _test_eof301;
case 301:
/* #line 8312 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 13: goto st291;
		case 32: goto st301;
		case 33: goto tr395;
		case 34: goto tr388;
		case 37: goto tr395;
		case 39: goto tr395;
		case 47: goto tr388;
		case 58: goto tr387;
		case 59: goto tr388;
		case 60: goto st294;
		case 91: goto tr388;
		case 93: goto tr388;
		case 96: goto st304;
		case 126: goto tr395;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 41 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr395;
		} else if ( (*p) >= 40 )
			goto tr388;
	} else if ( (*p) > 61 ) {
		if ( (*p) < 65 ) {
			if ( 63 <= (*p) && (*p) <= 64 )
				goto tr388;
		} else if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr395;
		} else
			goto tr395;
	} else
		goto tr395;
	goto tr0;
tr387:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st302;
st302:
	if ( ++p == pe )
		goto _test_eof302;
case 302:
/* #line 8358 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr420;
		case 32: goto tr421;
		case 37: goto st302;
		case 59: goto st295;
		case 60: goto st3;
		case 62: goto tr423;
		case 92: goto st3;
		case 96: goto st3;
		case 123: goto st3;
		case 125: goto st3;
		case 126: goto st302;
	}
	if ( (*p) < 39 ) {
		if ( 33 <= (*p) && (*p) <= 34 )
			goto st302;
	} else if ( (*p) > 43 ) {
		if ( (*p) > 93 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st302;
		} else if ( (*p) >= 45 )
			goto st302;
	} else
		goto st302;
	goto tr0;
tr735:
/* #line 156 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pFromAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st303;
tr423:
/* #line 87 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pToAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st303;
st303:
	if ( ++p == pe )
		goto _test_eof303;
case 303:
/* #line 8528 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr424;
		case 32: goto st303;
		case 37: goto st3;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st3;
		} else if ( (*p) >= 33 )
			goto st3;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st3;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st3;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
st304:
	if ( ++p == pe )
		goto _test_eof304;
case 304:
	switch( (*p) ) {
		case 13: goto st305;
		case 32: goto st308;
		case 33: goto st304;
		case 37: goto st304;
		case 39: goto st304;
		case 61: goto st304;
		case 126: goto st304;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st304;
		} else if ( (*p) >= 42 )
			goto st304;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st304;
		} else if ( (*p) >= 65 )
			goto st304;
	} else
		goto st304;
	goto tr0;
st305:
	if ( ++p == pe )
		goto _test_eof305;
case 305:
	if ( (*p) == 10 )
		goto st306;
	goto tr0;
st306:
	if ( ++p == pe )
		goto _test_eof306;
case 306:
	if ( (*p) == 32 )
		goto st307;
	goto tr0;
st307:
	if ( ++p == pe )
		goto _test_eof307;
case 307:
	switch( (*p) ) {
		case 13: goto st291;
		case 32: goto st307;
		case 33: goto tr395;
		case 34: goto tr388;
		case 37: goto tr395;
		case 39: goto tr395;
		case 47: goto tr388;
		case 60: goto st294;
		case 61: goto tr395;
		case 91: goto tr388;
		case 93: goto tr388;
		case 96: goto st304;
		case 126: goto tr395;
	}
	if ( (*p) < 58 ) {
		if ( (*p) < 42 ) {
			if ( 40 <= (*p) && (*p) <= 41 )
				goto tr388;
		} else if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 57 )
				goto tr395;
		} else
			goto tr395;
	} else if ( (*p) > 59 ) {
		if ( (*p) < 65 ) {
			if ( 63 <= (*p) && (*p) <= 64 )
				goto tr388;
		} else if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr395;
		} else
			goto tr395;
	} else
		goto tr388;
	goto tr0;
st308:
	if ( ++p == pe )
		goto _test_eof308;
case 308:
	switch( (*p) ) {
		case 13: goto st305;
		case 32: goto st308;
		case 33: goto tr395;
		case 34: goto tr388;
		case 37: goto tr395;
		case 39: goto tr395;
		case 47: goto tr388;
		case 60: goto st294;
		case 61: goto tr395;
		case 91: goto tr388;
		case 93: goto tr388;
		case 96: goto st304;
		case 126: goto tr395;
	}
	if ( (*p) < 58 ) {
		if ( (*p) < 42 ) {
			if ( 40 <= (*p) && (*p) <= 41 )
				goto tr388;
		} else if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 57 )
				goto tr395;
		} else
			goto tr395;
	} else if ( (*p) > 59 ) {
		if ( (*p) < 65 ) {
			if ( 63 <= (*p) && (*p) <= 64 )
				goto tr388;
		} else if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr395;
		} else
			goto tr395;
	} else
		goto tr388;
	goto tr0;
tr12:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st309;
st309:
	if ( ++p == pe )
		goto _test_eof309;
case 309:
/* #line 8684 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 109: goto st310;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st310:
	if ( ++p == pe )
		goto _test_eof310;
case 310:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 100: goto st311;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st311:
	if ( ++p == pe )
		goto _test_eof311;
case 311:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 110: goto st312;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st312:
	if ( ++p == pe )
		goto _test_eof312;
case 312:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 45: goto st149;
		case 46: goto st313;
		case 58: goto st3;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( 42 <= (*p) && (*p) <= 43 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st313:
	if ( ++p == pe )
		goto _test_eof313;
case 313:
	switch( (*p) ) {
		case 9: goto st2;
		case 13: goto tr337;
		case 32: goto tr434;
		case 33: goto tr435;
		case 34: goto tr339;
		case 37: goto tr435;
		case 39: goto tr435;
		case 47: goto tr339;
		case 58: goto tr4;
		case 61: goto tr435;
		case 68: goto tr436;
		case 73: goto tr437;
		case 77: goto tr438;
		case 79: goto tr439;
		case 123: goto tr339;
		case 125: goto tr339;
		case 126: goto tr435;
	}
	if ( (*p) < 60 ) {
		if ( (*p) < 42 ) {
			if ( 40 <= (*p) && (*p) <= 41 )
				goto tr339;
		} else if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 57 )
				goto tr435;
		} else
			goto tr435;
	} else if ( (*p) > 64 ) {
		if ( (*p) < 91 ) {
			if ( 65 <= (*p) && (*p) <= 90 )
				goto tr435;
		} else if ( (*p) > 93 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr435;
		} else
			goto tr339;
	} else
		goto tr339;
	goto tr0;
tr434:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st314;
st314:
	if ( ++p == pe )
		goto _test_eof314;
case 314:
/* #line 8853 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 13: goto st4;
		case 32: goto st314;
		case 37: goto st251;
		case 58: goto st3;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st251;
		} else if ( (*p) >= 33 )
			goto st251;
	} else if ( (*p) > 57 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st251;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st251;
		} else
			goto st251;
	} else
		goto st251;
	goto tr0;
tr435:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st315;
st315:
	if ( ++p == pe )
		goto _test_eof315;
case 315:
/* #line 8889 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 13: goto st4;
		case 32: goto st314;
		case 33: goto st315;
		case 34: goto st251;
		case 37: goto st315;
		case 39: goto st315;
		case 47: goto st251;
		case 58: goto st3;
		case 61: goto st315;
		case 123: goto st251;
		case 125: goto st251;
		case 126: goto st315;
	}
	if ( (*p) < 60 ) {
		if ( (*p) < 42 ) {
			if ( 40 <= (*p) && (*p) <= 41 )
				goto st251;
		} else if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 57 )
				goto st315;
		} else
			goto st315;
	} else if ( (*p) > 64 ) {
		if ( (*p) < 91 ) {
			if ( 65 <= (*p) && (*p) <= 90 )
				goto st315;
		} else if ( (*p) > 93 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st315;
		} else
			goto st251;
	} else
		goto st251;
	goto tr0;
tr436:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st316;
st316:
	if ( ++p == pe )
		goto _test_eof316;
case 316:
/* #line 8936 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 105: goto st317;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st317:
	if ( ++p == pe )
		goto _test_eof317;
case 317:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 115: goto st318;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st318:
	if ( ++p == pe )
		goto _test_eof318;
case 318:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 112: goto st319;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st319:
	if ( ++p == pe )
		goto _test_eof319;
case 319:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 111: goto st320;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st320:
	if ( ++p == pe )
		goto _test_eof320;
case 320:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 115: goto st321;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st321:
	if ( ++p == pe )
		goto _test_eof321;
case 321:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 105: goto st322;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st322:
	if ( ++p == pe )
		goto _test_eof322;
case 322:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 116: goto st323;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st323:
	if ( ++p == pe )
		goto _test_eof323;
case 323:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 105: goto st324;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st324:
	if ( ++p == pe )
		goto _test_eof324;
case 324:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 111: goto st325;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st325:
	if ( ++p == pe )
		goto _test_eof325;
case 325:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 110: goto st326;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st326:
	if ( ++p == pe )
		goto _test_eof326;
case 326:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 45: goto st327;
		case 46: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( 42 <= (*p) && (*p) <= 43 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st327:
	if ( ++p == pe )
		goto _test_eof327;
case 327:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 78: goto st328;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st328:
	if ( ++p == pe )
		goto _test_eof328;
case 328:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 111: goto st329;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st329:
	if ( ++p == pe )
		goto _test_eof329;
case 329:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 116: goto st330;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st330:
	if ( ++p == pe )
		goto _test_eof330;
case 330:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 105: goto st331;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st331:
	if ( ++p == pe )
		goto _test_eof331;
case 331:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 102: goto st332;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st332:
	if ( ++p == pe )
		goto _test_eof332;
case 332:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 105: goto st333;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st333:
	if ( ++p == pe )
		goto _test_eof333;
case 333:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 99: goto st334;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st334:
	if ( ++p == pe )
		goto _test_eof334;
case 334:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 97: goto st335;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st335:
	if ( ++p == pe )
		goto _test_eof335;
case 335:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 116: goto st336;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st336:
	if ( ++p == pe )
		goto _test_eof336;
case 336:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 105: goto st337;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st337:
	if ( ++p == pe )
		goto _test_eof337;
case 337:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 111: goto st338;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st338:
	if ( ++p == pe )
		goto _test_eof338;
case 338:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 110: goto st339;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st339:
	if ( ++p == pe )
		goto _test_eof339;
case 339:
	switch( (*p) ) {
		case 9: goto st340;
		case 32: goto st340;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st341;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st340:
	if ( ++p == pe )
		goto _test_eof340;
case 340:
	switch( (*p) ) {
		case 9: goto st340;
		case 32: goto st340;
		case 58: goto st341;
	}
	goto tr0;
st341:
	if ( ++p == pe )
		goto _test_eof341;
case 341:
	switch( (*p) ) {
		case 13: goto tr467;
		case 32: goto st341;
		case 37: goto tr468;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr468;
		} else if ( (*p) >= 33 )
			goto tr468;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr468;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr468;
		} else
			goto tr468;
	} else
		goto tr468;
	goto tr0;
tr467:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st342;
st342:
	if ( ++p == pe )
		goto _test_eof342;
case 342:
/* #line 9698 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st343;
	goto tr0;
st343:
	if ( ++p == pe )
		goto _test_eof343;
case 343:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr470;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr470:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st344;
st344:
	if ( ++p == pe )
		goto _test_eof344;
case 344:
/* #line 9751 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st344;
		case 37: goto tr472;
		case 58: goto tr468;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr472;
		} else if ( (*p) >= 33 )
			goto tr472;
	} else if ( (*p) > 57 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr472;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr472;
		} else
			goto tr472;
	} else
		goto tr472;
	goto tr0;
tr472:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st345;
st345:
	if ( ++p == pe )
		goto _test_eof345;
case 345:
/* #line 9786 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr473;
		case 32: goto tr474;
		case 37: goto st345;
		case 44: goto tr476;
	}
	if ( (*p) < 60 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 58 )
				goto st345;
		} else if ( (*p) >= 33 )
			goto st345;
	} else if ( (*p) > 93 ) {
		if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st345;
		} else if ( (*p) >= 95 )
			goto st345;
	} else
		goto st345;
	goto tr0;
tr519:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st346;
tr473:
/* #line 240 "EcrioCPIMParse.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, CPIM_NEGATIVE_DELIVERY_STRING,
				pal_StringLength(CPIM_NEGATIVE_DELIVERY_STRING)) == 0)
		{
			pStruct->dnType |= CPIMDispoNotifReq_Negative;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_POSITIVE_DELIVERY_STRING,
					pal_StringLength(CPIM_POSITIVE_DELIVERY_STRING)) == 0)
		{
			pStruct->dnType |= CPIMDispoNotifReq_Positive;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_DISPLAY_STRING,
					pal_StringLength(CPIM_DISPLAY_STRING)) == 0)
		{
			pStruct->dnType |= CPIMDispoNotifReq_Display;
		}
	}
	goto st346;
tr516:
/* #line 240 "EcrioCPIMParse.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, CPIM_NEGATIVE_DELIVERY_STRING,
				pal_StringLength(CPIM_NEGATIVE_DELIVERY_STRING)) == 0)
		{
			pStruct->dnType |= CPIMDispoNotifReq_Negative;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_POSITIVE_DELIVERY_STRING,
					pal_StringLength(CPIM_POSITIVE_DELIVERY_STRING)) == 0)
		{
			pStruct->dnType |= CPIMDispoNotifReq_Positive;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_DISPLAY_STRING,
					pal_StringLength(CPIM_DISPLAY_STRING)) == 0)
		{
			pStruct->dnType |= CPIMDispoNotifReq_Display;
		}
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st346;
st346:
	if ( ++p == pe )
		goto _test_eof346;
case 346:
/* #line 9864 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st347;
	goto tr0;
st347:
	if ( ++p == pe )
		goto _test_eof347;
case 347:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr478;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr478:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st348;
st348:
	if ( ++p == pe )
		goto _test_eof348;
case 348:
/* #line 9917 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st348;
		case 44: goto st349;
		case 58: goto st3;
	}
	goto tr0;
tr476:
/* #line 240 "EcrioCPIMParse.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, CPIM_NEGATIVE_DELIVERY_STRING,
				pal_StringLength(CPIM_NEGATIVE_DELIVERY_STRING)) == 0)
		{
			pStruct->dnType |= CPIMDispoNotifReq_Negative;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_POSITIVE_DELIVERY_STRING,
					pal_StringLength(CPIM_POSITIVE_DELIVERY_STRING)) == 0)
		{
			pStruct->dnType |= CPIMDispoNotifReq_Positive;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_DISPLAY_STRING,
					pal_StringLength(CPIM_DISPLAY_STRING)) == 0)
		{
			pStruct->dnType |= CPIMDispoNotifReq_Display;
		}
	}
	goto st349;
st349:
	if ( ++p == pe )
		goto _test_eof349;
case 349:
/* #line 9949 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st350;
		case 32: goto st349;
		case 37: goto tr472;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr472;
		} else if ( (*p) >= 33 )
			goto tr472;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr472;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr472;
		} else
			goto tr472;
	} else
		goto tr472;
	goto tr0;
st350:
	if ( ++p == pe )
		goto _test_eof350;
case 350:
	if ( (*p) == 10 )
		goto st351;
	goto tr0;
st351:
	if ( ++p == pe )
		goto _test_eof351;
case 351:
	if ( (*p) == 32 )
		goto st352;
	goto tr0;
st352:
	if ( ++p == pe )
		goto _test_eof352;
case 352:
	switch( (*p) ) {
		case 32: goto st352;
		case 37: goto tr472;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr472;
		} else if ( (*p) >= 33 )
			goto tr472;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr472;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr472;
		} else
			goto tr472;
	} else
		goto tr472;
	goto tr0;
tr13:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st353;
st353:
	if ( ++p == pe )
		goto _test_eof353;
case 353:
/* #line 10023 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 97: goto st354;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st354:
	if ( ++p == pe )
		goto _test_eof354;
case 354:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 97: goto st355;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st355:
	if ( ++p == pe )
		goto _test_eof355;
case 355:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 112: goto st356;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st356:
	if ( ++p == pe )
		goto _test_eof356;
case 356:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 45: goto st149;
		case 46: goto st357;
		case 58: goto st3;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( 42 <= (*p) && (*p) <= 43 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st357:
	if ( ++p == pe )
		goto _test_eof357;
case 357:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 84: goto st358;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st358:
	if ( ++p == pe )
		goto _test_eof358;
case 358:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 114: goto st359;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st359:
	if ( ++p == pe )
		goto _test_eof359;
case 359:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 97: goto st360;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st360:
	if ( ++p == pe )
		goto _test_eof360;
case 360:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 102: goto st361;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st361:
	if ( ++p == pe )
		goto _test_eof361;
case 361:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 102: goto st362;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st362:
	if ( ++p == pe )
		goto _test_eof362;
case 362:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 105: goto st363;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st363:
	if ( ++p == pe )
		goto _test_eof363;
case 363:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 99: goto st364;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st364:
	if ( ++p == pe )
		goto _test_eof364;
case 364:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 45: goto st365;
		case 46: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( 42 <= (*p) && (*p) <= 43 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st365:
	if ( ++p == pe )
		goto _test_eof365;
case 365:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 84: goto st366;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st366:
	if ( ++p == pe )
		goto _test_eof366;
case 366:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 121: goto st367;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st367:
	if ( ++p == pe )
		goto _test_eof367;
case 367:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 112: goto st368;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st368:
	if ( ++p == pe )
		goto _test_eof368;
case 368:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 101: goto st369;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st369:
	if ( ++p == pe )
		goto _test_eof369;
case 369:
	switch( (*p) ) {
		case 9: goto st370;
		case 32: goto st370;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st371;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st370:
	if ( ++p == pe )
		goto _test_eof370;
case 370:
	switch( (*p) ) {
		case 9: goto st370;
		case 32: goto st370;
		case 58: goto st371;
	}
	goto tr0;
st371:
	if ( ++p == pe )
		goto _test_eof371;
case 371:
	switch( (*p) ) {
		case 13: goto tr502;
		case 32: goto st371;
		case 37: goto tr503;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr503;
		} else if ( (*p) >= 33 )
			goto tr503;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr503;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr503;
		} else
			goto tr503;
	} else
		goto tr503;
	goto tr0;
tr502:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st372;
st372:
	if ( ++p == pe )
		goto _test_eof372;
case 372:
/* #line 10573 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st373;
	goto tr0;
st373:
	if ( ++p == pe )
		goto _test_eof373;
case 373:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr505;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr505:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st374;
st374:
	if ( ++p == pe )
		goto _test_eof374;
case 374:
/* #line 10626 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st374;
		case 37: goto tr507;
		case 58: goto tr503;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr507;
		} else if ( (*p) >= 33 )
			goto tr507;
	} else if ( (*p) > 57 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr507;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr507;
		} else
			goto tr507;
	} else
		goto tr507;
	goto tr0;
tr507:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st375;
st375:
	if ( ++p == pe )
		goto _test_eof375;
case 375:
/* #line 10661 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr508;
		case 32: goto tr509;
		case 37: goto st375;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st375;
		} else if ( (*p) >= 33 )
			goto st375;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st375;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st375;
		} else
			goto st375;
	} else
		goto st375;
	goto tr0;
tr503:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st376;
st376:
	if ( ++p == pe )
		goto _test_eof376;
case 376:
/* #line 10695 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr511;
		case 32: goto tr512;
		case 37: goto st376;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st376;
		} else if ( (*p) >= 33 )
			goto st376;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st376;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st376;
		} else
			goto st376;
	} else
		goto st376;
	goto tr0;
tr474:
/* #line 240 "EcrioCPIMParse.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, CPIM_NEGATIVE_DELIVERY_STRING,
				pal_StringLength(CPIM_NEGATIVE_DELIVERY_STRING)) == 0)
		{
			pStruct->dnType |= CPIMDispoNotifReq_Negative;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_POSITIVE_DELIVERY_STRING,
					pal_StringLength(CPIM_POSITIVE_DELIVERY_STRING)) == 0)
		{
			pStruct->dnType |= CPIMDispoNotifReq_Positive;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_DISPLAY_STRING,
					pal_StringLength(CPIM_DISPLAY_STRING)) == 0)
		{
			pStruct->dnType |= CPIMDispoNotifReq_Display;
		}
	}
	goto st377;
st377:
	if ( ++p == pe )
		goto _test_eof377;
case 377:
/* #line 10743 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st346;
		case 32: goto st377;
		case 44: goto st349;
	}
	goto tr0;
tr468:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st378;
st378:
	if ( ++p == pe )
		goto _test_eof378;
case 378:
/* #line 10760 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr516;
		case 32: goto tr517;
		case 37: goto st378;
		case 44: goto tr476;
	}
	if ( (*p) < 60 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 58 )
				goto st378;
		} else if ( (*p) >= 33 )
			goto st378;
	} else if ( (*p) > 93 ) {
		if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st378;
		} else if ( (*p) >= 95 )
			goto st378;
	} else
		goto st378;
	goto tr0;
tr517:
/* #line 240 "EcrioCPIMParse.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, CPIM_NEGATIVE_DELIVERY_STRING,
				pal_StringLength(CPIM_NEGATIVE_DELIVERY_STRING)) == 0)
		{
			pStruct->dnType |= CPIMDispoNotifReq_Negative;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_POSITIVE_DELIVERY_STRING,
					pal_StringLength(CPIM_POSITIVE_DELIVERY_STRING)) == 0)
		{
			pStruct->dnType |= CPIMDispoNotifReq_Positive;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_DISPLAY_STRING,
					pal_StringLength(CPIM_DISPLAY_STRING)) == 0)
		{
			pStruct->dnType |= CPIMDispoNotifReq_Display;
		}
	}
	goto st379;
st379:
	if ( ++p == pe )
		goto _test_eof379;
case 379:
/* #line 10806 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr519;
		case 32: goto st379;
		case 37: goto st3;
		case 44: goto st349;
	}
	if ( (*p) < 60 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 58 )
				goto st3;
		} else if ( (*p) >= 33 )
			goto st3;
	} else if ( (*p) > 93 ) {
		if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st3;
		} else if ( (*p) >= 95 )
			goto st3;
	} else
		goto st3;
	goto tr0;
tr437:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st380;
st380:
	if ( ++p == pe )
		goto _test_eof380;
case 380:
/* #line 10838 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 77: goto st381;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st381:
	if ( ++p == pe )
		goto _test_eof381;
case 381:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 68: goto st382;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st382:
	if ( ++p == pe )
		goto _test_eof382;
case 382:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 78: goto st383;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st383:
	if ( ++p == pe )
		goto _test_eof383;
case 383:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 45: goto st384;
		case 46: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( 42 <= (*p) && (*p) <= 43 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st384:
	if ( ++p == pe )
		goto _test_eof384;
case 384:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 82: goto st385;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st385:
	if ( ++p == pe )
		goto _test_eof385;
case 385:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 101: goto st197;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
tr438:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st386;
st386:
	if ( ++p == pe )
		goto _test_eof386;
case 386:
/* #line 11023 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 101: goto st387;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st387:
	if ( ++p == pe )
		goto _test_eof387;
case 387:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 115: goto st388;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st388:
	if ( ++p == pe )
		goto _test_eof388;
case 388:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 115: goto st389;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st389:
	if ( ++p == pe )
		goto _test_eof389;
case 389:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 97: goto st390;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st390:
	if ( ++p == pe )
		goto _test_eof390;
case 390:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 103: goto st391;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st391:
	if ( ++p == pe )
		goto _test_eof391;
case 391:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 101: goto st392;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st392:
	if ( ++p == pe )
		goto _test_eof392;
case 392:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 45: goto st393;
		case 46: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( 42 <= (*p) && (*p) <= 43 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st393:
	if ( ++p == pe )
		goto _test_eof393;
case 393:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 73: goto st394;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st394:
	if ( ++p == pe )
		goto _test_eof394;
case 394:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 68: goto st395;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st395:
	if ( ++p == pe )
		goto _test_eof395;
case 395:
	switch( (*p) ) {
		case 9: goto st396;
		case 32: goto st396;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st397;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st396:
	if ( ++p == pe )
		goto _test_eof396;
case 396:
	switch( (*p) ) {
		case 9: goto st396;
		case 32: goto st396;
		case 58: goto st397;
	}
	goto tr0;
st397:
	if ( ++p == pe )
		goto _test_eof397;
case 397:
	switch( (*p) ) {
		case 13: goto tr537;
		case 32: goto st397;
		case 37: goto tr538;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr538;
		} else if ( (*p) >= 33 )
			goto tr538;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr538;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr538;
		} else
			goto tr538;
	} else
		goto tr538;
	goto tr0;
tr537:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st398;
st398:
	if ( ++p == pe )
		goto _test_eof398;
case 398:
/* #line 11365 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st399;
	goto tr0;
st399:
	if ( ++p == pe )
		goto _test_eof399;
case 399:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr540;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr540:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st400;
st400:
	if ( ++p == pe )
		goto _test_eof400;
case 400:
/* #line 11418 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st400;
		case 37: goto tr542;
		case 58: goto tr538;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr542;
		} else if ( (*p) >= 33 )
			goto tr542;
	} else if ( (*p) > 57 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr542;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr542;
		} else
			goto tr542;
	} else
		goto tr542;
	goto tr0;
tr542:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st401;
st401:
	if ( ++p == pe )
		goto _test_eof401;
case 401:
/* #line 11453 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr543;
		case 32: goto tr544;
		case 37: goto st401;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st401;
		} else if ( (*p) >= 33 )
			goto st401;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st401;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st401;
		} else
			goto st401;
	} else
		goto st401;
	goto tr0;
tr538:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st402;
st402:
	if ( ++p == pe )
		goto _test_eof402;
case 402:
/* #line 11487 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr546;
		case 32: goto tr547;
		case 37: goto st402;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st402;
		} else if ( (*p) >= 33 )
			goto st402;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st402;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st402;
		} else
			goto st402;
	} else
		goto st402;
	goto tr0;
tr439:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st403;
st403:
	if ( ++p == pe )
		goto _test_eof403;
case 403:
/* #line 11521 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 114: goto st404;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st404:
	if ( ++p == pe )
		goto _test_eof404;
case 404:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 105: goto st405;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st405:
	if ( ++p == pe )
		goto _test_eof405;
case 405:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 103: goto st406;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st406:
	if ( ++p == pe )
		goto _test_eof406;
case 406:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 105: goto st407;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st407:
	if ( ++p == pe )
		goto _test_eof407;
case 407:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 110: goto st408;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st408:
	if ( ++p == pe )
		goto _test_eof408;
case 408:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 97: goto st409;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st409:
	if ( ++p == pe )
		goto _test_eof409;
case 409:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 108: goto st410;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st410:
	if ( ++p == pe )
		goto _test_eof410;
case 410:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 45: goto st411;
		case 46: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( 42 <= (*p) && (*p) <= 43 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st411:
	if ( ++p == pe )
		goto _test_eof411;
case 411:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 84: goto st412;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st412:
	if ( ++p == pe )
		goto _test_eof412;
case 412:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 111: goto st413;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st413:
	if ( ++p == pe )
		goto _test_eof413;
case 413:
	switch( (*p) ) {
		case 9: goto st414;
		case 32: goto st414;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st415;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st414:
	if ( ++p == pe )
		goto _test_eof414;
case 414:
	switch( (*p) ) {
		case 9: goto st414;
		case 32: goto st414;
		case 58: goto st415;
	}
	goto tr0;
st415:
	if ( ++p == pe )
		goto _test_eof415;
case 415:
	switch( (*p) ) {
		case 13: goto tr561;
		case 32: goto st415;
		case 37: goto tr562;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr562;
		} else if ( (*p) >= 33 )
			goto tr562;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr562;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr562;
		} else
			goto tr562;
	} else
		goto tr562;
	goto tr0;
tr561:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st416;
st416:
	if ( ++p == pe )
		goto _test_eof416;
case 416:
/* #line 11893 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st417;
	goto tr0;
st417:
	if ( ++p == pe )
		goto _test_eof417;
case 417:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr564;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr564:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st418;
st418:
	if ( ++p == pe )
		goto _test_eof418;
case 418:
/* #line 11946 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st418;
		case 37: goto tr566;
		case 58: goto tr562;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr566;
		} else if ( (*p) >= 33 )
			goto tr566;
	} else if ( (*p) > 57 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr566;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr566;
		} else
			goto tr566;
	} else
		goto tr566;
	goto tr0;
tr566:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st419;
st419:
	if ( ++p == pe )
		goto _test_eof419;
case 419:
/* #line 11981 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr567;
		case 32: goto tr568;
		case 37: goto st419;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st419;
		} else if ( (*p) >= 33 )
			goto st419;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st419;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st419;
		} else
			goto st419;
	} else
		goto st419;
	goto tr0;
tr562:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st420;
st420:
	if ( ++p == pe )
		goto _test_eof420;
case 420:
/* #line 12015 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr570;
		case 32: goto tr571;
		case 37: goto st420;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st420;
		} else if ( (*p) >= 33 )
			goto st420;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st420;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st420;
		} else
			goto st420;
	} else
		goto st420;
	goto tr0;
tr415:
/* #line 87 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pToAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st421;
st421:
	if ( ++p == pe )
		goto _test_eof421;
case 421:
/* #line 12113 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st299;
		case 32: goto st421;
		case 33: goto tr395;
		case 34: goto tr388;
		case 37: goto tr395;
		case 39: goto tr395;
		case 47: goto tr388;
		case 60: goto st294;
		case 61: goto tr395;
		case 91: goto tr388;
		case 93: goto tr388;
		case 96: goto st304;
		case 126: goto tr395;
	}
	if ( (*p) < 58 ) {
		if ( (*p) < 42 ) {
			if ( 40 <= (*p) && (*p) <= 41 )
				goto tr388;
		} else if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 57 )
				goto tr395;
		} else
			goto tr395;
	} else if ( (*p) > 59 ) {
		if ( (*p) < 65 ) {
			if ( 63 <= (*p) && (*p) <= 64 )
				goto tr388;
		} else if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr395;
		} else
			goto tr395;
	} else
		goto tr388;
	goto tr0;
tr396:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st422;
st422:
	if ( ++p == pe )
		goto _test_eof422;
case 422:
/* #line 12160 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr575;
		case 32: goto tr576;
		case 33: goto st422;
		case 34: goto st435;
		case 37: goto st422;
		case 44: goto st287;
		case 60: goto st287;
		case 62: goto tr579;
		case 92: goto st297;
		case 94: goto st287;
		case 96: goto st287;
		case 126: goto st422;
	}
	if ( (*p) < 39 ) {
		if ( 35 <= (*p) && (*p) <= 38 )
			goto st287;
	} else if ( (*p) > 122 ) {
		if ( 123 <= (*p) && (*p) <= 125 )
			goto st287;
	} else
		goto st422;
	goto tr0;
tr584:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st423;
tr575:
/* #line 87 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pToAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st423;
tr608:
/* #line 87 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pToAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st423;
st423:
	if ( ++p == pe )
		goto _test_eof423;
case 423:
/* #line 12340 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st424;
	goto tr0;
st424:
	if ( ++p == pe )
		goto _test_eof424;
case 424:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr581;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr581:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st425;
st425:
	if ( ++p == pe )
		goto _test_eof425;
case 425:
/* #line 12393 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 13: goto st288;
		case 32: goto st425;
		case 34: goto st290;
		case 58: goto st426;
		case 92: goto st297;
	}
	if ( 33 <= (*p) && (*p) <= 126 )
		goto st287;
	goto tr0;
tr609:
/* #line 87 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pToAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st426;
st426:
	if ( ++p == pe )
		goto _test_eof426;
case 426:
/* #line 12479 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr584;
		case 34: goto st427;
		case 37: goto st426;
		case 44: goto st287;
		case 59: goto st287;
		case 92: goto st433;
		case 94: goto st287;
		case 124: goto st287;
	}
	if ( (*p) < 35 ) {
		if ( 32 <= (*p) && (*p) <= 33 )
			goto st426;
	} else if ( (*p) > 38 ) {
		if ( 39 <= (*p) && (*p) <= 126 )
			goto st426;
	} else
		goto st287;
	goto tr0;
st427:
	if ( ++p == pe )
		goto _test_eof427;
case 427:
	switch( (*p) ) {
		case 13: goto tr587;
		case 32: goto st431;
		case 37: goto tr387;
		case 59: goto tr388;
		case 60: goto st432;
		case 62: goto st3;
		case 92: goto st3;
		case 96: goto st3;
		case 123: goto st3;
		case 125: goto st3;
		case 126: goto tr387;
	}
	if ( (*p) < 39 ) {
		if ( 33 <= (*p) && (*p) <= 34 )
			goto tr387;
	} else if ( (*p) > 43 ) {
		if ( (*p) > 93 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr387;
		} else if ( (*p) >= 45 )
			goto tr387;
	} else
		goto tr387;
	goto tr0;
tr587:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st428;
tr594:
/* #line 87 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pToAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st428;
tr613:
/* #line 87 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pToAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st428;
st428:
	if ( ++p == pe )
		goto _test_eof428;
case 428:
/* #line 12684 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st429;
	goto tr0;
st429:
	if ( ++p == pe )
		goto _test_eof429;
case 429:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr590;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr590:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st430;
st430:
	if ( ++p == pe )
		goto _test_eof430;
case 430:
/* #line 12737 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st430;
		case 58: goto st3;
		case 60: goto st294;
	}
	goto tr0;
tr614:
/* #line 87 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pToAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st431;
st431:
	if ( ++p == pe )
		goto _test_eof431;
case 431:
/* #line 12819 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr587;
		case 32: goto st431;
		case 37: goto st3;
		case 60: goto st432;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st3;
		} else if ( (*p) >= 33 )
			goto st3;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 61 <= (*p) && (*p) <= 93 )
				goto st3;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st3;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
st432:
	if ( ++p == pe )
		goto _test_eof432;
case 432:
	switch( (*p) ) {
		case 13: goto tr16;
		case 32: goto st3;
		case 37: goto tr387;
		case 59: goto tr388;
		case 60: goto st3;
		case 62: goto st3;
		case 92: goto st3;
		case 96: goto st3;
		case 123: goto st3;
		case 125: goto st3;
		case 126: goto tr387;
	}
	if ( (*p) < 39 ) {
		if ( 33 <= (*p) && (*p) <= 34 )
			goto tr387;
	} else if ( (*p) > 43 ) {
		if ( (*p) > 93 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr387;
		} else if ( (*p) >= 45 )
			goto tr387;
	} else
		goto tr387;
	goto tr0;
st433:
	if ( ++p == pe )
		goto _test_eof433;
case 433:
	switch( (*p) ) {
		case 13: goto tr16;
		case 37: goto st426;
		case 44: goto st287;
		case 59: goto st287;
		case 94: goto st287;
		case 124: goto st287;
		case 127: goto st287;
	}
	if ( (*p) < 32 ) {
		if ( (*p) > 9 ) {
			if ( 11 <= (*p) && (*p) <= 31 )
				goto st287;
		} else if ( (*p) >= 0 )
			goto st287;
	} else if ( (*p) > 34 ) {
		if ( (*p) > 38 ) {
			if ( 39 <= (*p) && (*p) <= 126 )
				goto st426;
		} else if ( (*p) >= 35 )
			goto st287;
	} else
		goto st426;
	goto tr0;
tr576:
/* #line 87 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pToAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st434;
st434:
	if ( ++p == pe )
		goto _test_eof434;
case 434:
/* #line 12975 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st423;
		case 32: goto st434;
		case 34: goto st290;
		case 92: goto st297;
	}
	if ( 33 <= (*p) && (*p) <= 126 )
		goto st287;
	goto tr0;
st435:
	if ( ++p == pe )
		goto _test_eof435;
case 435:
	switch( (*p) ) {
		case 13: goto tr594;
		case 32: goto tr595;
		case 37: goto tr388;
		case 60: goto st294;
		case 62: goto tr413;
		case 93: goto tr388;
		case 95: goto tr388;
		case 126: goto tr388;
	}
	if ( (*p) < 39 ) {
		if ( 33 <= (*p) && (*p) <= 34 )
			goto tr388;
	} else if ( (*p) > 43 ) {
		if ( (*p) > 91 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr388;
		} else if ( (*p) >= 45 )
			goto tr388;
	} else
		goto tr388;
	goto tr0;
tr595:
/* #line 87 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pToAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st436;
st436:
	if ( ++p == pe )
		goto _test_eof436;
case 436:
/* #line 13085 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st428;
		case 32: goto st436;
		case 60: goto st294;
	}
	goto tr0;
tr579:
/* #line 87 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pToAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st437;
st437:
	if ( ++p == pe )
		goto _test_eof437;
case 437:
/* #line 13166 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st438;
		case 32: goto st437;
		case 34: goto st290;
		case 92: goto st297;
	}
	if ( 33 <= (*p) && (*p) <= 126 )
		goto st287;
	goto tr0;
tr615:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st438;
st438:
	if ( ++p == pe )
		goto _test_eof438;
case 438:
/* #line 13187 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st439;
	goto tr0;
st439:
	if ( ++p == pe )
		goto _test_eof439;
case 439:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr601;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr601:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st440;
st440:
	if ( ++p == pe )
		goto _test_eof440;
case 440:
/* #line 13240 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 13: goto st423;
		case 32: goto st440;
		case 34: goto st290;
		case 58: goto st426;
		case 92: goto st297;
	}
	if ( 33 <= (*p) && (*p) <= 126 )
		goto st287;
	goto tr0;
tr385:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st441;
st441:
	if ( ++p == pe )
		goto _test_eof441;
case 441:
/* #line 13262 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr603;
		case 32: goto tr604;
		case 33: goto st441;
		case 34: goto st302;
		case 37: goto st441;
		case 39: goto st441;
		case 47: goto st302;
		case 58: goto st302;
		case 59: goto st295;
		case 60: goto st3;
		case 62: goto tr423;
		case 92: goto st3;
		case 96: goto st443;
		case 123: goto st3;
		case 125: goto st3;
		case 126: goto st441;
	}
	if ( (*p) < 63 ) {
		if ( (*p) < 42 ) {
			if ( 40 <= (*p) && (*p) <= 41 )
				goto st302;
		} else if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 61 )
				goto st441;
		} else
			goto st441;
	} else if ( (*p) > 64 ) {
		if ( (*p) < 91 ) {
			if ( 65 <= (*p) && (*p) <= 90 )
				goto st441;
		} else if ( (*p) > 93 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st441;
		} else
			goto st302;
	} else
		goto st302;
	goto tr0;
tr604:
/* #line 87 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pToAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st442;
st442:
	if ( ++p == pe )
		goto _test_eof442;
case 442:
/* #line 13376 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr606;
		case 32: goto st442;
		case 33: goto tr385;
		case 34: goto tr387;
		case 37: goto tr385;
		case 39: goto tr385;
		case 47: goto tr387;
		case 58: goto tr387;
		case 59: goto tr388;
		case 60: goto st432;
		case 62: goto st3;
		case 92: goto st3;
		case 96: goto st443;
		case 123: goto st3;
		case 125: goto st3;
		case 126: goto tr385;
	}
	if ( (*p) < 63 ) {
		if ( (*p) < 42 ) {
			if ( 40 <= (*p) && (*p) <= 41 )
				goto tr387;
		} else if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 61 )
				goto tr385;
		} else
			goto tr385;
	} else if ( (*p) > 64 ) {
		if ( (*p) < 91 ) {
			if ( 65 <= (*p) && (*p) <= 90 )
				goto tr385;
		} else if ( (*p) > 93 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr385;
		} else
			goto tr387;
	} else
		goto tr387;
	goto tr0;
st443:
	if ( ++p == pe )
		goto _test_eof443;
case 443:
	switch( (*p) ) {
		case 13: goto tr606;
		case 32: goto st442;
		case 33: goto st443;
		case 34: goto st3;
		case 37: goto st443;
		case 39: goto st443;
		case 47: goto st3;
		case 58: goto st3;
		case 61: goto st443;
		case 123: goto st3;
		case 125: goto st3;
		case 126: goto st443;
	}
	if ( (*p) < 60 ) {
		if ( (*p) < 42 ) {
			if ( 40 <= (*p) && (*p) <= 41 )
				goto st3;
		} else if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 57 )
				goto st443;
		} else
			goto st443;
	} else if ( (*p) > 64 ) {
		if ( (*p) < 91 ) {
			if ( 65 <= (*p) && (*p) <= 90 )
				goto st443;
		} else if ( (*p) > 93 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st443;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
tr386:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st444;
st444:
	if ( ++p == pe )
		goto _test_eof444;
case 444:
/* #line 13465 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr608;
		case 32: goto tr609;
		case 33: goto st444;
		case 34: goto st445;
		case 37: goto st444;
		case 44: goto st287;
		case 59: goto st422;
		case 60: goto st426;
		case 62: goto tr612;
		case 92: goto st433;
		case 94: goto st287;
		case 96: goto st426;
		case 123: goto st426;
		case 124: goto st287;
		case 125: goto st426;
	}
	if ( (*p) > 38 ) {
		if ( 39 <= (*p) && (*p) <= 126 )
			goto st444;
	} else if ( (*p) >= 35 )
		goto st287;
	goto tr0;
st445:
	if ( ++p == pe )
		goto _test_eof445;
case 445:
	switch( (*p) ) {
		case 13: goto tr613;
		case 32: goto tr614;
		case 37: goto tr387;
		case 59: goto tr388;
		case 60: goto st432;
		case 62: goto tr423;
		case 92: goto st3;
		case 96: goto st3;
		case 123: goto st3;
		case 125: goto st3;
		case 126: goto tr387;
	}
	if ( (*p) < 39 ) {
		if ( 33 <= (*p) && (*p) <= 34 )
			goto tr387;
	} else if ( (*p) > 43 ) {
		if ( (*p) > 93 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr387;
		} else if ( (*p) >= 45 )
			goto tr387;
	} else
		goto tr387;
	goto tr0;
tr612:
/* #line 87 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pToAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st446;
st446:
	if ( ++p == pe )
		goto _test_eof446;
case 446:
/* #line 13592 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr615;
		case 32: goto st446;
		case 33: goto st426;
		case 34: goto st427;
		case 37: goto st426;
		case 44: goto st287;
		case 59: goto st287;
		case 92: goto st433;
		case 94: goto st287;
		case 124: goto st287;
	}
	if ( (*p) > 38 ) {
		if ( 39 <= (*p) && (*p) <= 126 )
			goto st426;
	} else if ( (*p) >= 35 )
		goto st287;
	goto tr0;
tr357:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st447;
st447:
	if ( ++p == pe )
		goto _test_eof447;
case 447:
/* #line 13621 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr363;
		case 32: goto tr617;
		case 37: goto st447;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st447;
		} else if ( (*p) >= 33 )
			goto st447;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st447;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st447;
		} else
			goto st447;
	} else
		goto st447;
	goto tr0;
tr344:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st448;
st448:
	if ( ++p == pe )
		goto _test_eof448;
case 448:
/* #line 13655 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st4;
		case 37: goto st251;
		case 109: goto st449;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st251;
		} else if ( (*p) >= 32 )
			goto st251;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st251;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st251;
		} else
			goto st251;
	} else
		goto st251;
	goto tr0;
st449:
	if ( ++p == pe )
		goto _test_eof449;
case 449:
	switch( (*p) ) {
		case 13: goto st4;
		case 37: goto st251;
		case 100: goto st450;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st251;
		} else if ( (*p) >= 32 )
			goto st251;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st251;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st251;
		} else
			goto st251;
	} else
		goto st251;
	goto tr0;
st450:
	if ( ++p == pe )
		goto _test_eof450;
case 450:
	switch( (*p) ) {
		case 13: goto st4;
		case 37: goto st251;
		case 110: goto st451;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st251;
		} else if ( (*p) >= 32 )
			goto st251;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st251;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st251;
		} else
			goto st251;
	} else
		goto st251;
	goto tr0;
st451:
	if ( ++p == pe )
		goto _test_eof451;
case 451:
	switch( (*p) ) {
		case 13: goto st452;
		case 32: goto st451;
		case 37: goto tr623;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr623;
		} else if ( (*p) >= 33 )
			goto tr623;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr623;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr623;
		} else
			goto tr623;
	} else
		goto tr623;
	goto tr0;
tr692:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st452;
st452:
	if ( ++p == pe )
		goto _test_eof452;
case 452:
/* #line 13771 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st453;
	goto tr0;
st453:
	if ( ++p == pe )
		goto _test_eof453;
case 453:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr625;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr625:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st454;
st454:
	if ( ++p == pe )
		goto _test_eof454;
case 454:
/* #line 13824 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st454;
		case 37: goto tr627;
		case 58: goto tr628;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr627;
		} else if ( (*p) >= 33 )
			goto tr627;
	} else if ( (*p) > 57 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr627;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr627;
		} else
			goto tr627;
	} else
		goto tr627;
	goto tr0;
tr627:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st455;
st455:
	if ( ++p == pe )
		goto _test_eof455;
case 455:
/* #line 13859 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr629;
		case 32: goto tr630;
		case 37: goto st455;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st455;
		} else if ( (*p) >= 33 )
			goto st455;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st455;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st455;
		} else
			goto st455;
	} else
		goto st455;
	goto tr0;
tr628:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st456;
st456:
	if ( ++p == pe )
		goto _test_eof456;
case 456:
/* #line 13893 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr632;
		case 32: goto tr633;
		case 37: goto st456;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st456;
		} else if ( (*p) >= 33 )
			goto st456;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st456;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st456;
		} else
			goto st456;
	} else
		goto st456;
	goto tr0;
tr623:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st457;
st457:
	if ( ++p == pe )
		goto _test_eof457;
case 457:
/* #line 13927 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr629;
		case 32: goto tr635;
		case 37: goto st457;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st457;
		} else if ( (*p) >= 33 )
			goto st457;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st457;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st457;
		} else
			goto st457;
	} else
		goto st457;
	goto tr0;
tr345:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st458;
st458:
	if ( ++p == pe )
		goto _test_eof458;
case 458:
/* #line 13961 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st4;
		case 37: goto st251;
		case 97: goto st459;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st251;
		} else if ( (*p) >= 32 )
			goto st251;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st251;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st251;
		} else
			goto st251;
	} else
		goto st251;
	goto tr0;
st459:
	if ( ++p == pe )
		goto _test_eof459;
case 459:
	switch( (*p) ) {
		case 13: goto st4;
		case 37: goto st251;
		case 97: goto st460;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st251;
		} else if ( (*p) >= 32 )
			goto st251;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st251;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st251;
		} else
			goto st251;
	} else
		goto st251;
	goto tr0;
st460:
	if ( ++p == pe )
		goto _test_eof460;
case 460:
	switch( (*p) ) {
		case 13: goto st4;
		case 37: goto st251;
		case 112: goto st461;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st251;
		} else if ( (*p) >= 32 )
			goto st251;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st251;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st251;
		} else
			goto st251;
	} else
		goto st251;
	goto tr0;
st461:
	if ( ++p == pe )
		goto _test_eof461;
case 461:
	switch( (*p) ) {
		case 13: goto st462;
		case 32: goto st461;
		case 37: goto tr641;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr641;
		} else if ( (*p) >= 33 )
			goto tr641;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr641;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr641;
		} else
			goto tr641;
	} else
		goto tr641;
	goto tr0;
tr696:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st462;
st462:
	if ( ++p == pe )
		goto _test_eof462;
case 462:
/* #line 14077 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st463;
	goto tr0;
st463:
	if ( ++p == pe )
		goto _test_eof463;
case 463:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr643;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr643:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st464;
st464:
	if ( ++p == pe )
		goto _test_eof464;
case 464:
/* #line 14130 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st464;
		case 37: goto tr645;
		case 58: goto tr646;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr645;
		} else if ( (*p) >= 33 )
			goto tr645;
	} else if ( (*p) > 57 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr645;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr645;
		} else
			goto tr645;
	} else
		goto tr645;
	goto tr0;
tr645:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st465;
st465:
	if ( ++p == pe )
		goto _test_eof465;
case 465:
/* #line 14165 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr647;
		case 32: goto tr648;
		case 37: goto st465;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st465;
		} else if ( (*p) >= 33 )
			goto st465;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st465;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st465;
		} else
			goto st465;
	} else
		goto st465;
	goto tr0;
tr646:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st466;
st466:
	if ( ++p == pe )
		goto _test_eof466;
case 466:
/* #line 14199 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr650;
		case 32: goto tr651;
		case 37: goto st466;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st466;
		} else if ( (*p) >= 33 )
			goto st466;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st466;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st466;
		} else
			goto st466;
	} else
		goto st466;
	goto tr0;
tr641:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st467;
st467:
	if ( ++p == pe )
		goto _test_eof467;
case 467:
/* #line 14233 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr647;
		case 32: goto tr653;
		case 37: goto st467;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st467;
		} else if ( (*p) >= 33 )
			goto st467;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st467;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st467;
		} else
			goto st467;
	} else
		goto st467;
	goto tr0;
tr340:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st468;
st468:
	if ( ++p == pe )
		goto _test_eof468;
case 468:
/* #line 14267 "EcrioCPIMParse.c" */
	if ( (*p) == 121 )
		goto st469;
	goto tr0;
st469:
	if ( ++p == pe )
		goto _test_eof469;
case 469:
	if ( (*p) == 70 )
		goto st470;
	goto tr0;
st470:
	if ( ++p == pe )
		goto _test_eof470;
case 470:
	if ( (*p) == 101 )
		goto st471;
	goto tr0;
st471:
	if ( ++p == pe )
		goto _test_eof471;
case 471:
	if ( (*p) == 97 )
		goto st472;
	goto tr0;
st472:
	if ( ++p == pe )
		goto _test_eof472;
case 472:
	if ( (*p) == 116 )
		goto st473;
	goto tr0;
st473:
	if ( ++p == pe )
		goto _test_eof473;
case 473:
	if ( (*p) == 117 )
		goto st474;
	goto tr0;
st474:
	if ( ++p == pe )
		goto _test_eof474;
case 474:
	if ( (*p) == 114 )
		goto st475;
	goto tr0;
st475:
	if ( ++p == pe )
		goto _test_eof475;
case 475:
	if ( (*p) == 101 )
		goto st476;
	goto tr0;
st476:
	if ( ++p == pe )
		goto _test_eof476;
case 476:
	if ( (*p) == 115 )
		goto st477;
	goto tr0;
st477:
	if ( ++p == pe )
		goto _test_eof477;
case 477:
	switch( (*p) ) {
		case 13: goto st478;
		case 32: goto st477;
		case 37: goto tr361;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr361;
		} else if ( (*p) >= 33 )
			goto tr361;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr361;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr361;
		} else
			goto tr361;
	} else
		goto tr361;
	goto tr0;
st478:
	if ( ++p == pe )
		goto _test_eof478;
case 478:
	if ( (*p) == 10 )
		goto st479;
	goto tr0;
st479:
	if ( ++p == pe )
		goto _test_eof479;
case 479:
	if ( (*p) == 32 )
		goto st480;
	goto tr0;
st480:
	if ( ++p == pe )
		goto _test_eof480;
case 480:
	switch( (*p) ) {
		case 32: goto st480;
		case 37: goto tr361;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr361;
		} else if ( (*p) >= 33 )
			goto tr361;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr361;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr361;
		} else
			goto tr361;
	} else
		goto tr361;
	goto tr0;
tr341:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st481;
st481:
	if ( ++p == pe )
		goto _test_eof481;
case 481:
/* #line 14404 "EcrioCPIMParse.c" */
	if ( (*p) == 109 )
		goto st482;
	goto tr0;
st482:
	if ( ++p == pe )
		goto _test_eof482;
case 482:
	if ( (*p) == 100 )
		goto st483;
	goto tr0;
st483:
	if ( ++p == pe )
		goto _test_eof483;
case 483:
	if ( (*p) == 110 )
		goto st484;
	goto tr0;
st484:
	if ( ++p == pe )
		goto _test_eof484;
case 484:
	switch( (*p) ) {
		case 13: goto st485;
		case 32: goto st484;
		case 37: goto tr627;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr627;
		} else if ( (*p) >= 33 )
			goto tr627;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr627;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr627;
		} else
			goto tr627;
	} else
		goto tr627;
	goto tr0;
st485:
	if ( ++p == pe )
		goto _test_eof485;
case 485:
	if ( (*p) == 10 )
		goto st486;
	goto tr0;
st486:
	if ( ++p == pe )
		goto _test_eof486;
case 486:
	if ( (*p) == 32 )
		goto st487;
	goto tr0;
st487:
	if ( ++p == pe )
		goto _test_eof487;
case 487:
	switch( (*p) ) {
		case 32: goto st487;
		case 37: goto tr627;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr627;
		} else if ( (*p) >= 33 )
			goto tr627;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr627;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr627;
		} else
			goto tr627;
	} else
		goto tr627;
	goto tr0;
tr342:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st488;
st488:
	if ( ++p == pe )
		goto _test_eof488;
case 488:
/* #line 14499 "EcrioCPIMParse.c" */
	if ( (*p) == 97 )
		goto st489;
	goto tr0;
st489:
	if ( ++p == pe )
		goto _test_eof489;
case 489:
	if ( (*p) == 97 )
		goto st490;
	goto tr0;
st490:
	if ( ++p == pe )
		goto _test_eof490;
case 490:
	if ( (*p) == 112 )
		goto st491;
	goto tr0;
st491:
	if ( ++p == pe )
		goto _test_eof491;
case 491:
	switch( (*p) ) {
		case 13: goto st492;
		case 32: goto st491;
		case 37: goto tr645;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr645;
		} else if ( (*p) >= 33 )
			goto tr645;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr645;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr645;
		} else
			goto tr645;
	} else
		goto tr645;
	goto tr0;
st492:
	if ( ++p == pe )
		goto _test_eof492;
case 492:
	if ( (*p) == 10 )
		goto st493;
	goto tr0;
st493:
	if ( ++p == pe )
		goto _test_eof493;
case 493:
	if ( (*p) == 32 )
		goto st494;
	goto tr0;
st494:
	if ( ++p == pe )
		goto _test_eof494;
case 494:
	switch( (*p) ) {
		case 32: goto st494;
		case 37: goto tr645;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr645;
		} else if ( (*p) >= 33 )
			goto tr645;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr645;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr645;
		} else
			goto tr645;
	} else
		goto tr645;
	goto tr0;
tr332:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st495;
st495:
	if ( ++p == pe )
		goto _test_eof495;
case 495:
/* #line 14594 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr16;
		case 37: goto st3;
		case 121: goto st496;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st3;
		} else if ( (*p) >= 32 )
			goto st3;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st3;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st3;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
st496:
	if ( ++p == pe )
		goto _test_eof496;
case 496:
	switch( (*p) ) {
		case 13: goto tr16;
		case 37: goto st3;
		case 70: goto st497;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st3;
		} else if ( (*p) >= 32 )
			goto st3;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st3;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st3;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
st497:
	if ( ++p == pe )
		goto _test_eof497;
case 497:
	switch( (*p) ) {
		case 13: goto tr16;
		case 37: goto st3;
		case 101: goto st498;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st3;
		} else if ( (*p) >= 32 )
			goto st3;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st3;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st3;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
st498:
	if ( ++p == pe )
		goto _test_eof498;
case 498:
	switch( (*p) ) {
		case 13: goto tr16;
		case 37: goto st3;
		case 97: goto st499;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st3;
		} else if ( (*p) >= 32 )
			goto st3;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st3;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st3;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
st499:
	if ( ++p == pe )
		goto _test_eof499;
case 499:
	switch( (*p) ) {
		case 13: goto tr16;
		case 37: goto st3;
		case 116: goto st500;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st3;
		} else if ( (*p) >= 32 )
			goto st3;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st3;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st3;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
st500:
	if ( ++p == pe )
		goto _test_eof500;
case 500:
	switch( (*p) ) {
		case 13: goto tr16;
		case 37: goto st3;
		case 117: goto st501;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st3;
		} else if ( (*p) >= 32 )
			goto st3;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st3;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st3;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
st501:
	if ( ++p == pe )
		goto _test_eof501;
case 501:
	switch( (*p) ) {
		case 13: goto tr16;
		case 37: goto st3;
		case 114: goto st502;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st3;
		} else if ( (*p) >= 32 )
			goto st3;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st3;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st3;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
st502:
	if ( ++p == pe )
		goto _test_eof502;
case 502:
	switch( (*p) ) {
		case 13: goto tr16;
		case 37: goto st3;
		case 101: goto st503;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st3;
		} else if ( (*p) >= 32 )
			goto st3;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st3;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st3;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
st503:
	if ( ++p == pe )
		goto _test_eof503;
case 503:
	switch( (*p) ) {
		case 13: goto tr16;
		case 37: goto st3;
		case 115: goto st504;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st3;
		} else if ( (*p) >= 32 )
			goto st3;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st3;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st3;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
st504:
	if ( ++p == pe )
		goto _test_eof504;
case 504:
	switch( (*p) ) {
		case 13: goto tr688;
		case 32: goto st504;
		case 37: goto tr362;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr362;
		} else if ( (*p) >= 33 )
			goto tr362;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr362;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr362;
		} else
			goto tr362;
	} else
		goto tr362;
	goto tr0;
tr333:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st505;
st505:
	if ( ++p == pe )
		goto _test_eof505;
case 505:
/* #line 14871 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr16;
		case 37: goto st3;
		case 109: goto st506;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st3;
		} else if ( (*p) >= 32 )
			goto st3;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st3;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st3;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
st506:
	if ( ++p == pe )
		goto _test_eof506;
case 506:
	switch( (*p) ) {
		case 13: goto tr16;
		case 37: goto st3;
		case 100: goto st507;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st3;
		} else if ( (*p) >= 32 )
			goto st3;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st3;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st3;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
st507:
	if ( ++p == pe )
		goto _test_eof507;
case 507:
	switch( (*p) ) {
		case 13: goto tr16;
		case 37: goto st3;
		case 110: goto st508;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st3;
		} else if ( (*p) >= 32 )
			goto st3;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st3;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st3;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
st508:
	if ( ++p == pe )
		goto _test_eof508;
case 508:
	switch( (*p) ) {
		case 13: goto tr692;
		case 32: goto st508;
		case 37: goto tr628;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr628;
		} else if ( (*p) >= 33 )
			goto tr628;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr628;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr628;
		} else
			goto tr628;
	} else
		goto tr628;
	goto tr0;
tr334:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st509;
st509:
	if ( ++p == pe )
		goto _test_eof509;
case 509:
/* #line 14986 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr16;
		case 37: goto st3;
		case 97: goto st510;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st3;
		} else if ( (*p) >= 32 )
			goto st3;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st3;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st3;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
st510:
	if ( ++p == pe )
		goto _test_eof510;
case 510:
	switch( (*p) ) {
		case 13: goto tr16;
		case 37: goto st3;
		case 97: goto st511;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st3;
		} else if ( (*p) >= 32 )
			goto st3;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st3;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st3;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
st511:
	if ( ++p == pe )
		goto _test_eof511;
case 511:
	switch( (*p) ) {
		case 13: goto tr16;
		case 37: goto st3;
		case 112: goto st512;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st3;
		} else if ( (*p) >= 32 )
			goto st3;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st3;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st3;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
st512:
	if ( ++p == pe )
		goto _test_eof512;
case 512:
	switch( (*p) ) {
		case 13: goto tr696;
		case 32: goto st512;
		case 37: goto tr646;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr646;
		} else if ( (*p) >= 33 )
			goto tr646;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr646;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr646;
		} else
			goto tr646;
	} else
		goto tr646;
	goto tr0;
tr318:
/* #line 438 "EcrioCPIMParse.rl" */
	{
		if (bIsMyFeaturesNS == Enum_TRUE)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pPANI,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st513;
st513:
	if ( ++p == pe )
		goto _test_eof513;
case 513:
/* #line 15111 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st236;
		case 32: goto st513;
		case 59: goto st239;
	}
	goto tr0;
tr312:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st514;
st514:
	if ( ++p == pe )
		goto _test_eof514;
case 514:
/* #line 15128 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr699;
		case 32: goto tr700;
		case 37: goto st514;
		case 59: goto st239;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st514;
		} else if ( (*p) >= 33 )
			goto st514;
	} else if ( (*p) > 93 ) {
		if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st514;
		} else if ( (*p) >= 95 )
			goto st514;
	} else
		goto st514;
	goto tr0;
tr700:
/* #line 438 "EcrioCPIMParse.rl" */
	{
		if (bIsMyFeaturesNS == Enum_TRUE)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pPANI,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st515;
st515:
	if ( ++p == pe )
		goto _test_eof515;
case 515:
/* #line 15170 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr702;
		case 32: goto st515;
		case 37: goto st3;
		case 59: goto st239;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st3;
		} else if ( (*p) >= 33 )
			goto st3;
	} else if ( (*p) > 93 ) {
		if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st3;
		} else if ( (*p) >= 95 )
			goto st3;
	} else
		goto st3;
	goto tr0;
st516:
	if ( ++p == pe )
		goto _test_eof516;
case 516:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 117: goto st517;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st517:
	if ( ++p == pe )
		goto _test_eof517;
case 517:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 116: goto st518;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st518:
	if ( ++p == pe )
		goto _test_eof518;
case 518:
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st2;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st3;
		case 61: goto st149;
		case 101: goto st519;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st519:
	if ( ++p == pe )
		goto _test_eof519;
case 519:
	switch( (*p) ) {
		case 9: goto st520;
		case 32: goto st520;
		case 33: goto st149;
		case 37: goto st149;
		case 39: goto st149;
		case 58: goto st521;
		case 61: goto st149;
		case 126: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st149;
		} else if ( (*p) >= 42 )
			goto st149;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st149;
		} else if ( (*p) >= 65 )
			goto st149;
	} else
		goto st149;
	goto tr0;
st520:
	if ( ++p == pe )
		goto _test_eof520;
case 520:
	switch( (*p) ) {
		case 9: goto st520;
		case 32: goto st520;
		case 58: goto st521;
	}
	goto tr0;
st521:
	if ( ++p == pe )
		goto _test_eof521;
case 521:
	switch( (*p) ) {
		case 13: goto tr709;
		case 32: goto st521;
		case 37: goto tr710;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr710;
		} else if ( (*p) >= 33 )
			goto tr710;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr710;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr710;
		} else
			goto tr710;
	} else
		goto tr710;
	goto tr0;
tr709:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st522;
st522:
	if ( ++p == pe )
		goto _test_eof522;
case 522:
/* #line 15359 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st523;
	goto tr0;
st523:
	if ( ++p == pe )
		goto _test_eof523;
case 523:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr712;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr712:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st524;
st524:
	if ( ++p == pe )
		goto _test_eof524;
case 524:
/* #line 15412 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st524;
		case 37: goto tr714;
		case 58: goto tr710;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr714;
		} else if ( (*p) >= 33 )
			goto tr714;
	} else if ( (*p) > 57 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto tr714;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr714;
		} else
			goto tr714;
	} else
		goto tr714;
	goto tr0;
tr714:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st525;
st525:
	if ( ++p == pe )
		goto _test_eof525;
case 525:
/* #line 15447 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr715;
		case 32: goto tr716;
		case 37: goto st525;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st525;
		} else if ( (*p) >= 33 )
			goto st525;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st525;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st525;
		} else
			goto st525;
	} else
		goto st525;
	goto tr0;
tr710:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st526;
st526:
	if ( ++p == pe )
		goto _test_eof526;
case 526:
/* #line 15481 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr718;
		case 32: goto tr719;
		case 37: goto st526;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st526;
		} else if ( (*p) >= 33 )
			goto st526;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st526;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st526;
		} else
			goto st526;
	} else
		goto st526;
	goto tr0;
tr249:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st527;
st527:
	if ( ++p == pe )
		goto _test_eof527;
case 527:
/* #line 15525 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st182;
		case 32: goto st527;
		case 60: goto st185;
	}
	goto tr0;
st528:
	if ( ++p == pe )
		goto _test_eof528;
case 528:
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 9 )
			goto st178;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) )
			goto st178;
	} else
		goto st178;
	goto tr0;
tr236:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st529;
tr730:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st529;
st529:
	if ( ++p == pe )
		goto _test_eof529;
case 529:
/* #line 15589 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr723;
		case 32: goto tr724;
		case 33: goto st529;
		case 34: goto st186;
		case 37: goto st529;
		case 39: goto st529;
		case 47: goto st186;
		case 61: goto st529;
		case 62: goto tr258;
		case 91: goto st186;
		case 93: goto st186;
		case 96: goto st534;
		case 126: goto st529;
	}
	if ( (*p) < 58 ) {
		if ( (*p) < 42 ) {
			if ( 40 <= (*p) && (*p) <= 41 )
				goto st186;
		} else if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 57 )
				goto st529;
		} else
			goto st529;
	} else if ( (*p) > 59 ) {
		if ( (*p) < 65 ) {
			if ( 63 <= (*p) && (*p) <= 64 )
				goto st186;
		} else if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st529;
		} else
			goto st529;
	} else
		goto st186;
	goto tr0;
tr785:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st530;
tr743:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st530;
tr723:
/* #line 156 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pFromAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st530;
tr778:
/* #line 156 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pFromAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st530;
tr782:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st530;
st530:
	if ( ++p == pe )
		goto _test_eof530;
case 530:
/* #line 15819 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st531;
	goto tr0;
st531:
	if ( ++p == pe )
		goto _test_eof531;
case 531:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr728;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr728:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st532;
tr729:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st532;
st532:
	if ( ++p == pe )
		goto _test_eof532;
case 532:
/* #line 15888 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 13: goto tr248;
		case 32: goto tr729;
		case 33: goto tr730;
		case 34: goto tr250;
		case 37: goto tr730;
		case 39: goto tr730;
		case 47: goto tr250;
		case 58: goto tr731;
		case 59: goto tr250;
		case 60: goto tr251;
		case 91: goto tr250;
		case 93: goto tr250;
		case 96: goto st534;
		case 126: goto tr730;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 41 ) {
			if ( 42 <= (*p) && (*p) <= 43 )
				goto tr730;
		} else if ( (*p) >= 40 )
			goto tr250;
	} else if ( (*p) > 61 ) {
		if ( (*p) < 65 ) {
			if ( 63 <= (*p) && (*p) <= 64 )
				goto tr250;
		} else if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr730;
		} else
			goto tr730;
	} else
		goto tr730;
	goto tr0;
tr766:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st533;
tr228:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st533;
tr731:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st533;
st533:
	if ( ++p == pe )
		goto _test_eof533;
case 533:
/* #line 15974 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr732;
		case 32: goto tr733;
		case 37: goto st533;
		case 59: goto st186;
		case 60: goto st3;
		case 62: goto tr735;
		case 92: goto st3;
		case 96: goto st3;
		case 123: goto st3;
		case 125: goto st3;
		case 126: goto st533;
	}
	if ( (*p) < 39 ) {
		if ( 33 <= (*p) && (*p) <= 34 )
			goto st533;
	} else if ( (*p) > 43 ) {
		if ( (*p) > 93 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st533;
		} else if ( (*p) >= 45 )
			goto st533;
	} else
		goto st533;
	goto tr0;
tr239:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st534;
st534:
	if ( ++p == pe )
		goto _test_eof534;
case 534:
/* #line 16010 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st535;
		case 32: goto st538;
		case 33: goto st534;
		case 37: goto st534;
		case 39: goto st534;
		case 61: goto st534;
		case 126: goto st534;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st534;
		} else if ( (*p) >= 42 )
			goto st534;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st534;
		} else if ( (*p) >= 65 )
			goto st534;
	} else
		goto st534;
	goto tr0;
tr741:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st535;
st535:
	if ( ++p == pe )
		goto _test_eof535;
case 535:
/* #line 16055 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st536;
	goto tr0;
st536:
	if ( ++p == pe )
		goto _test_eof536;
case 536:
	if ( (*p) == 32 )
		goto st537;
	goto tr0;
tr740:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st537;
st537:
	if ( ++p == pe )
		goto _test_eof537;
case 537:
/* #line 16086 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr248;
		case 32: goto tr740;
		case 33: goto tr730;
		case 34: goto tr250;
		case 37: goto tr730;
		case 39: goto tr730;
		case 47: goto tr250;
		case 60: goto tr251;
		case 61: goto tr730;
		case 91: goto tr250;
		case 93: goto tr250;
		case 96: goto st534;
		case 126: goto tr730;
	}
	if ( (*p) < 58 ) {
		if ( (*p) < 42 ) {
			if ( 40 <= (*p) && (*p) <= 41 )
				goto tr250;
		} else if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 57 )
				goto tr730;
		} else
			goto tr730;
	} else if ( (*p) > 59 ) {
		if ( (*p) < 65 ) {
			if ( 63 <= (*p) && (*p) <= 64 )
				goto tr250;
		} else if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr730;
		} else
			goto tr730;
	} else
		goto tr250;
	goto tr0;
tr742:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st538;
st538:
	if ( ++p == pe )
		goto _test_eof538;
case 538:
/* #line 16143 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr741;
		case 32: goto tr742;
		case 33: goto tr730;
		case 34: goto tr250;
		case 37: goto tr730;
		case 39: goto tr730;
		case 47: goto tr250;
		case 60: goto tr251;
		case 61: goto tr730;
		case 91: goto tr250;
		case 93: goto tr250;
		case 96: goto st534;
		case 126: goto tr730;
	}
	if ( (*p) < 58 ) {
		if ( (*p) < 42 ) {
			if ( 40 <= (*p) && (*p) <= 41 )
				goto tr250;
		} else if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 57 )
				goto tr730;
		} else
			goto tr730;
	} else if ( (*p) > 59 ) {
		if ( (*p) < 65 ) {
			if ( 63 <= (*p) && (*p) <= 64 )
				goto tr250;
		} else if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr730;
		} else
			goto tr730;
	} else
		goto tr250;
	goto tr0;
tr744:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st539;
tr724:
/* #line 156 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pFromAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st539;
st539:
	if ( ++p == pe )
		goto _test_eof539;
case 539:
/* #line 16270 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr743;
		case 32: goto tr744;
		case 33: goto tr730;
		case 34: goto tr250;
		case 37: goto tr730;
		case 39: goto tr730;
		case 47: goto tr250;
		case 60: goto tr251;
		case 61: goto tr730;
		case 91: goto tr250;
		case 93: goto tr250;
		case 96: goto st534;
		case 126: goto tr730;
	}
	if ( (*p) < 58 ) {
		if ( (*p) < 42 ) {
			if ( 40 <= (*p) && (*p) <= 41 )
				goto tr250;
		} else if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 57 )
				goto tr730;
		} else
			goto tr730;
	} else if ( (*p) > 59 ) {
		if ( (*p) < 65 ) {
			if ( 63 <= (*p) && (*p) <= 64 )
				goto tr250;
		} else if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr730;
		} else
			goto tr730;
	} else
		goto tr250;
	goto tr0;
tr237:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st540;
st540:
	if ( ++p == pe )
		goto _test_eof540;
case 540:
/* #line 16331 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr745;
		case 32: goto tr746;
		case 33: goto st540;
		case 34: goto st553;
		case 37: goto st540;
		case 44: goto st178;
		case 60: goto st178;
		case 62: goto tr749;
		case 92: goto st528;
		case 94: goto st178;
		case 96: goto st178;
		case 126: goto st540;
	}
	if ( (*p) < 39 ) {
		if ( 35 <= (*p) && (*p) <= 38 )
			goto st178;
	} else if ( (*p) > 122 ) {
		if ( 123 <= (*p) && (*p) <= 125 )
			goto st178;
	} else
		goto st540;
	goto tr0;
tr754:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st541;
tr745:
/* #line 156 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pFromAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st541;
tr787:
/* #line 156 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pFromAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st541;
st541:
	if ( ++p == pe )
		goto _test_eof541;
case 541:
/* #line 16511 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st542;
	goto tr0;
st542:
	if ( ++p == pe )
		goto _test_eof542;
case 542:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr751;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr751:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st543;
st543:
	if ( ++p == pe )
		goto _test_eof543;
case 543:
/* #line 16564 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 13: goto st179;
		case 32: goto st543;
		case 34: goto st181;
		case 58: goto st544;
		case 92: goto st528;
	}
	if ( 33 <= (*p) && (*p) <= 126 )
		goto st178;
	goto tr0;
tr788:
/* #line 156 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pFromAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st544;
st544:
	if ( ++p == pe )
		goto _test_eof544;
case 544:
/* #line 16650 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr754;
		case 34: goto st545;
		case 37: goto st544;
		case 44: goto st178;
		case 59: goto st178;
		case 92: goto st551;
		case 94: goto st178;
		case 124: goto st178;
	}
	if ( (*p) < 35 ) {
		if ( 32 <= (*p) && (*p) <= 33 )
			goto st544;
	} else if ( (*p) > 38 ) {
		if ( 39 <= (*p) && (*p) <= 126 )
			goto st544;
	} else
		goto st178;
	goto tr0;
st545:
	if ( ++p == pe )
		goto _test_eof545;
case 545:
	switch( (*p) ) {
		case 13: goto tr757;
		case 32: goto tr758;
		case 37: goto tr731;
		case 59: goto tr250;
		case 60: goto tr759;
		case 62: goto st3;
		case 92: goto st3;
		case 96: goto st3;
		case 123: goto st3;
		case 125: goto st3;
		case 126: goto tr731;
	}
	if ( (*p) < 39 ) {
		if ( 33 <= (*p) && (*p) <= 34 )
			goto tr731;
	} else if ( (*p) > 43 ) {
		if ( (*p) > 93 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr731;
		} else if ( (*p) >= 45 )
			goto tr731;
	} else
		goto tr731;
	goto tr0;
tr763:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st546;
tr757:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st546;
tr769:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
/* #line 156 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pFromAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st546;
tr792:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
/* #line 156 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pFromAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st546;
st546:
	if ( ++p == pe )
		goto _test_eof546;
case 546:
/* #line 16904 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st547;
	goto tr0;
st547:
	if ( ++p == pe )
		goto _test_eof547;
case 547:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr761;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr761:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st548;
st548:
	if ( ++p == pe )
		goto _test_eof548;
case 548:
/* #line 16957 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 32: goto st548;
		case 58: goto st3;
		case 60: goto st185;
	}
	goto tr0;
tr758:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st549;
tr793:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
/* #line 156 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pFromAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st549;
st549:
	if ( ++p == pe )
		goto _test_eof549;
case 549:
/* #line 17069 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr763;
		case 32: goto st549;
		case 37: goto st3;
		case 60: goto st550;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st3;
		} else if ( (*p) >= 33 )
			goto st3;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 61 <= (*p) && (*p) <= 93 )
				goto st3;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st3;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
tr230:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st550;
tr759:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st550;
st550:
	if ( ++p == pe )
		goto _test_eof550;
case 550:
/* #line 17134 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr16;
		case 32: goto st3;
		case 37: goto tr766;
		case 59: goto tr254;
		case 60: goto st3;
		case 62: goto st3;
		case 92: goto st3;
		case 96: goto st3;
		case 123: goto st3;
		case 125: goto st3;
		case 126: goto tr766;
	}
	if ( (*p) < 39 ) {
		if ( 33 <= (*p) && (*p) <= 34 )
			goto tr766;
	} else if ( (*p) > 43 ) {
		if ( (*p) > 93 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr766;
		} else if ( (*p) >= 45 )
			goto tr766;
	} else
		goto tr766;
	goto tr0;
st551:
	if ( ++p == pe )
		goto _test_eof551;
case 551:
	switch( (*p) ) {
		case 13: goto tr16;
		case 37: goto st544;
		case 44: goto st178;
		case 59: goto st178;
		case 94: goto st178;
		case 124: goto st178;
		case 127: goto st178;
	}
	if ( (*p) < 32 ) {
		if ( (*p) > 9 ) {
			if ( 11 <= (*p) && (*p) <= 31 )
				goto st178;
		} else if ( (*p) >= 0 )
			goto st178;
	} else if ( (*p) > 34 ) {
		if ( (*p) > 38 ) {
			if ( 39 <= (*p) && (*p) <= 126 )
				goto st544;
		} else if ( (*p) >= 35 )
			goto st178;
	} else
		goto st544;
	goto tr0;
tr746:
/* #line 156 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pFromAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st552;
st552:
	if ( ++p == pe )
		goto _test_eof552;
case 552:
/* #line 17262 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st541;
		case 32: goto st552;
		case 34: goto st181;
		case 92: goto st528;
	}
	if ( 33 <= (*p) && (*p) <= 126 )
		goto st178;
	goto tr0;
st553:
	if ( ++p == pe )
		goto _test_eof553;
case 553:
	switch( (*p) ) {
		case 13: goto tr769;
		case 32: goto tr770;
		case 37: goto tr250;
		case 60: goto tr251;
		case 62: goto tr258;
		case 93: goto tr250;
		case 95: goto tr250;
		case 126: goto tr250;
	}
	if ( (*p) < 39 ) {
		if ( 33 <= (*p) && (*p) <= 34 )
			goto tr250;
	} else if ( (*p) > 43 ) {
		if ( (*p) > 91 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr250;
		} else if ( (*p) >= 45 )
			goto tr250;
	} else
		goto tr250;
	goto tr0;
tr770:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
/* #line 156 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pFromAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st554;
st554:
	if ( ++p == pe )
		goto _test_eof554;
case 554:
/* #line 17386 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st546;
		case 32: goto st554;
		case 60: goto st185;
	}
	goto tr0;
tr749:
/* #line 156 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pFromAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st555;
st555:
	if ( ++p == pe )
		goto _test_eof555;
case 555:
/* #line 17467 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto st556;
		case 32: goto st555;
		case 34: goto st181;
		case 92: goto st528;
	}
	if ( 33 <= (*p) && (*p) <= 126 )
		goto st178;
	goto tr0;
tr794:
/* #line 590 "EcrioCPIMParse.rl" */
	{
		/** No action when unsupported header was given */
		uCPIMError = ECRIO_CPIM_NO_ERROR;
	}
	goto st556;
st556:
	if ( ++p == pe )
		goto _test_eof556;
case 556:
/* #line 17488 "EcrioCPIMParse.c" */
	if ( (*p) == 10 )
		goto st557;
	goto tr0;
st557:
	if ( ++p == pe )
		goto _test_eof557;
case 557:
	switch( (*p) ) {
		case 9: goto tr1;
		case 13: goto st5;
		case 32: goto tr776;
		case 33: goto tr3;
		case 37: goto tr3;
		case 39: goto tr3;
		case 58: goto tr4;
		case 61: goto tr3;
		case 68: goto tr5;
		case 70: goto tr6;
		case 73: goto tr7;
		case 77: goto tr8;
		case 78: goto tr9;
		case 83: goto tr10;
		case 84: goto tr11;
		case 105: goto tr12;
		case 109: goto tr13;
		case 126: goto tr3;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr3;
		} else if ( (*p) >= 42 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto tr0;
tr776:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st558;
st558:
	if ( ++p == pe )
		goto _test_eof558;
case 558:
/* #line 17541 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 9: goto st2;
		case 13: goto st541;
		case 32: goto st558;
		case 34: goto st181;
		case 58: goto st544;
		case 92: goto st528;
	}
	if ( 33 <= (*p) && (*p) <= 126 )
		goto st178;
	goto tr0;
tr226:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st559;
tr784:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st559;
st559:
	if ( ++p == pe )
		goto _test_eof559;
case 559:
/* #line 17597 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr778;
		case 32: goto tr779;
		case 33: goto st559;
		case 34: goto st533;
		case 37: goto st559;
		case 39: goto st559;
		case 47: goto st533;
		case 58: goto st533;
		case 59: goto st186;
		case 60: goto st3;
		case 62: goto tr735;
		case 92: goto st3;
		case 96: goto st561;
		case 123: goto st3;
		case 125: goto st3;
		case 126: goto st559;
	}
	if ( (*p) < 63 ) {
		if ( (*p) < 42 ) {
			if ( 40 <= (*p) && (*p) <= 41 )
				goto st533;
		} else if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 61 )
				goto st559;
		} else
			goto st559;
	} else if ( (*p) > 64 ) {
		if ( (*p) < 91 ) {
			if ( 65 <= (*p) && (*p) <= 90 )
				goto st559;
		} else if ( (*p) > 93 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st559;
		} else
			goto st533;
	} else
		goto st533;
	goto tr0;
tr783:
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st560;
tr779:
/* #line 156 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pFromAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st560;
st560:
	if ( ++p == pe )
		goto _test_eof560;
case 560:
/* #line 17727 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr782;
		case 32: goto tr783;
		case 33: goto tr784;
		case 34: goto tr731;
		case 37: goto tr784;
		case 39: goto tr784;
		case 47: goto tr731;
		case 58: goto tr731;
		case 59: goto tr250;
		case 60: goto tr759;
		case 62: goto st3;
		case 92: goto st3;
		case 96: goto st561;
		case 123: goto st3;
		case 125: goto st3;
		case 126: goto tr784;
	}
	if ( (*p) < 63 ) {
		if ( (*p) < 42 ) {
			if ( 40 <= (*p) && (*p) <= 41 )
				goto tr731;
		} else if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 61 )
				goto tr784;
		} else
			goto tr784;
	} else if ( (*p) > 64 ) {
		if ( (*p) < 91 ) {
			if ( 65 <= (*p) && (*p) <= 90 )
				goto tr784;
		} else if ( (*p) > 93 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr784;
		} else
			goto tr731;
	} else
		goto tr731;
	goto tr0;
tr231:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
	goto st561;
st561:
	if ( ++p == pe )
		goto _test_eof561;
case 561:
/* #line 17777 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr785;
		case 32: goto st560;
		case 33: goto st561;
		case 34: goto st3;
		case 37: goto st561;
		case 39: goto st561;
		case 47: goto st3;
		case 58: goto st3;
		case 61: goto st561;
		case 123: goto st3;
		case 125: goto st3;
		case 126: goto st561;
	}
	if ( (*p) < 60 ) {
		if ( (*p) < 42 ) {
			if ( 40 <= (*p) && (*p) <= 41 )
				goto st3;
		} else if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 57 )
				goto st561;
		} else
			goto st561;
	} else if ( (*p) > 64 ) {
		if ( (*p) < 91 ) {
			if ( 65 <= (*p) && (*p) <= 90 )
				goto st561;
		} else if ( (*p) > 93 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st561;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
tr227:
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
/* #line 225 "EcrioCPIMParse.rl" */
	{
		if (p != tag_start)
		{
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pStruct->pDisplayName,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st562;
st562:
	if ( ++p == pe )
		goto _test_eof562;
case 562:
/* #line 17837 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr787;
		case 32: goto tr788;
		case 33: goto st562;
		case 34: goto st563;
		case 37: goto st562;
		case 44: goto st178;
		case 59: goto st540;
		case 60: goto st544;
		case 62: goto tr791;
		case 92: goto st551;
		case 94: goto st178;
		case 96: goto st544;
		case 123: goto st544;
		case 124: goto st178;
		case 125: goto st544;
	}
	if ( (*p) > 38 ) {
		if ( 39 <= (*p) && (*p) <= 126 )
			goto st562;
	} else if ( (*p) >= 35 )
		goto st178;
	goto tr0;
st563:
	if ( ++p == pe )
		goto _test_eof563;
case 563:
	switch( (*p) ) {
		case 13: goto tr792;
		case 32: goto tr793;
		case 37: goto tr731;
		case 59: goto tr250;
		case 60: goto tr759;
		case 62: goto tr735;
		case 92: goto st3;
		case 96: goto st3;
		case 123: goto st3;
		case 125: goto st3;
		case 126: goto tr731;
	}
	if ( (*p) < 39 ) {
		if ( 33 <= (*p) && (*p) <= 34 )
			goto tr731;
	} else if ( (*p) > 43 ) {
		if ( (*p) > 93 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr731;
		} else if ( (*p) >= 45 )
			goto tr731;
	} else
		goto tr731;
	goto tr0;
tr791:
/* #line 156 "EcrioCPIMParse.rl" */
	{
		c->cmnBuff.uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->cmnBuff, p - tag_start);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tCommon buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->cmnBuff.pData, 0, c->cmnBuff.uContainerSize);
		pal_MemoryCopy(c->cmnBuff.pData, c->cmnBuff.uContainerSize, tag_start, p - tag_start);

		pBuff = c->cmnBuff.pData;

		if (pal_StringNCompare(pBuff, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			pAtSign = pal_StringFindSubString(pBuff, (const u_char *)"@");
			/** Find '@' */
			if (pAtSign != NULL)
			{
				/** Find ';' after at sign */
				pSemiColon = pal_StringFindSubString(pAtSign, (const u_char *)";");
				if (pSemiColon != NULL)
				{
					/** Extract SIP URI up to the parameter part */
					uStrLen = pSemiColon - pBuff;
				}
				else
				{
					/** No parameters in SIP URI */
					uStrLen = p - tag_start;
				}
			}
			else
			{
				uStrLen = p - tag_start;
			}
		}
		else if (pal_StringNCompare(pBuff, (u_char*)"tel:", 4) == 0)
		{
			/** Find ';' */
			pSemiColon = pal_StringFindSubString(pBuff, (const u_char *)";");
			if (pSemiColon != NULL)
			{
				/** Extract TEL URI up to the parameter part */
				uStrLen = pSemiColon - pBuff;
			}
			else
			{
				/** No parameters in TEL URI */
				uStrLen = p - tag_start;
			}
		}
		else
		{
			uStrLen = p - tag_start;
		}

		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pStruct->pFromAddr,
			(u_char*)tag_start,
			uStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st564;
st564:
	if ( ++p == pe )
		goto _test_eof564;
case 564:
/* #line 17964 "EcrioCPIMParse.c" */
	switch( (*p) ) {
		case 13: goto tr794;
		case 32: goto st564;
		case 33: goto st544;
		case 34: goto st545;
		case 37: goto st544;
		case 44: goto st178;
		case 59: goto st178;
		case 92: goto st551;
		case 94: goto st178;
		case 124: goto st178;
	}
	if ( (*p) > 38 ) {
		if ( 39 <= (*p) && (*p) <= 126 )
			goto st544;
	} else if ( (*p) >= 35 )
		goto st178;
	goto tr0;
	}
	_test_eof1: cs = 1; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof565: cs = 565; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof566: cs = 566; goto _test_eof; 
	_test_eof567: cs = 567; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 
	_test_eof17: cs = 17; goto _test_eof; 
	_test_eof18: cs = 18; goto _test_eof; 
	_test_eof19: cs = 19; goto _test_eof; 
	_test_eof20: cs = 20; goto _test_eof; 
	_test_eof21: cs = 21; goto _test_eof; 
	_test_eof22: cs = 22; goto _test_eof; 
	_test_eof23: cs = 23; goto _test_eof; 
	_test_eof24: cs = 24; goto _test_eof; 
	_test_eof25: cs = 25; goto _test_eof; 
	_test_eof26: cs = 26; goto _test_eof; 
	_test_eof27: cs = 27; goto _test_eof; 
	_test_eof28: cs = 28; goto _test_eof; 
	_test_eof29: cs = 29; goto _test_eof; 
	_test_eof30: cs = 30; goto _test_eof; 
	_test_eof31: cs = 31; goto _test_eof; 
	_test_eof32: cs = 32; goto _test_eof; 
	_test_eof33: cs = 33; goto _test_eof; 
	_test_eof34: cs = 34; goto _test_eof; 
	_test_eof35: cs = 35; goto _test_eof; 
	_test_eof36: cs = 36; goto _test_eof; 
	_test_eof37: cs = 37; goto _test_eof; 
	_test_eof38: cs = 38; goto _test_eof; 
	_test_eof39: cs = 39; goto _test_eof; 
	_test_eof40: cs = 40; goto _test_eof; 
	_test_eof41: cs = 41; goto _test_eof; 
	_test_eof42: cs = 42; goto _test_eof; 
	_test_eof43: cs = 43; goto _test_eof; 
	_test_eof44: cs = 44; goto _test_eof; 
	_test_eof45: cs = 45; goto _test_eof; 
	_test_eof46: cs = 46; goto _test_eof; 
	_test_eof47: cs = 47; goto _test_eof; 
	_test_eof48: cs = 48; goto _test_eof; 
	_test_eof49: cs = 49; goto _test_eof; 
	_test_eof50: cs = 50; goto _test_eof; 
	_test_eof51: cs = 51; goto _test_eof; 
	_test_eof52: cs = 52; goto _test_eof; 
	_test_eof53: cs = 53; goto _test_eof; 
	_test_eof54: cs = 54; goto _test_eof; 
	_test_eof55: cs = 55; goto _test_eof; 
	_test_eof56: cs = 56; goto _test_eof; 
	_test_eof57: cs = 57; goto _test_eof; 
	_test_eof58: cs = 58; goto _test_eof; 
	_test_eof59: cs = 59; goto _test_eof; 
	_test_eof60: cs = 60; goto _test_eof; 
	_test_eof61: cs = 61; goto _test_eof; 
	_test_eof568: cs = 568; goto _test_eof; 
	_test_eof62: cs = 62; goto _test_eof; 
	_test_eof63: cs = 63; goto _test_eof; 
	_test_eof64: cs = 64; goto _test_eof; 
	_test_eof65: cs = 65; goto _test_eof; 
	_test_eof66: cs = 66; goto _test_eof; 
	_test_eof67: cs = 67; goto _test_eof; 
	_test_eof68: cs = 68; goto _test_eof; 
	_test_eof69: cs = 69; goto _test_eof; 
	_test_eof70: cs = 70; goto _test_eof; 
	_test_eof71: cs = 71; goto _test_eof; 
	_test_eof72: cs = 72; goto _test_eof; 
	_test_eof73: cs = 73; goto _test_eof; 
	_test_eof74: cs = 74; goto _test_eof; 
	_test_eof75: cs = 75; goto _test_eof; 
	_test_eof76: cs = 76; goto _test_eof; 
	_test_eof77: cs = 77; goto _test_eof; 
	_test_eof78: cs = 78; goto _test_eof; 
	_test_eof79: cs = 79; goto _test_eof; 
	_test_eof80: cs = 80; goto _test_eof; 
	_test_eof81: cs = 81; goto _test_eof; 
	_test_eof82: cs = 82; goto _test_eof; 
	_test_eof83: cs = 83; goto _test_eof; 
	_test_eof84: cs = 84; goto _test_eof; 
	_test_eof85: cs = 85; goto _test_eof; 
	_test_eof86: cs = 86; goto _test_eof; 
	_test_eof87: cs = 87; goto _test_eof; 
	_test_eof88: cs = 88; goto _test_eof; 
	_test_eof89: cs = 89; goto _test_eof; 
	_test_eof90: cs = 90; goto _test_eof; 
	_test_eof91: cs = 91; goto _test_eof; 
	_test_eof92: cs = 92; goto _test_eof; 
	_test_eof93: cs = 93; goto _test_eof; 
	_test_eof94: cs = 94; goto _test_eof; 
	_test_eof95: cs = 95; goto _test_eof; 
	_test_eof96: cs = 96; goto _test_eof; 
	_test_eof97: cs = 97; goto _test_eof; 
	_test_eof98: cs = 98; goto _test_eof; 
	_test_eof99: cs = 99; goto _test_eof; 
	_test_eof100: cs = 100; goto _test_eof; 
	_test_eof101: cs = 101; goto _test_eof; 
	_test_eof102: cs = 102; goto _test_eof; 
	_test_eof103: cs = 103; goto _test_eof; 
	_test_eof104: cs = 104; goto _test_eof; 
	_test_eof105: cs = 105; goto _test_eof; 
	_test_eof106: cs = 106; goto _test_eof; 
	_test_eof107: cs = 107; goto _test_eof; 
	_test_eof108: cs = 108; goto _test_eof; 
	_test_eof109: cs = 109; goto _test_eof; 
	_test_eof110: cs = 110; goto _test_eof; 
	_test_eof111: cs = 111; goto _test_eof; 
	_test_eof569: cs = 569; goto _test_eof; 
	_test_eof112: cs = 112; goto _test_eof; 
	_test_eof113: cs = 113; goto _test_eof; 
	_test_eof114: cs = 114; goto _test_eof; 
	_test_eof115: cs = 115; goto _test_eof; 
	_test_eof116: cs = 116; goto _test_eof; 
	_test_eof117: cs = 117; goto _test_eof; 
	_test_eof118: cs = 118; goto _test_eof; 
	_test_eof119: cs = 119; goto _test_eof; 
	_test_eof120: cs = 120; goto _test_eof; 
	_test_eof121: cs = 121; goto _test_eof; 
	_test_eof122: cs = 122; goto _test_eof; 
	_test_eof123: cs = 123; goto _test_eof; 
	_test_eof124: cs = 124; goto _test_eof; 
	_test_eof125: cs = 125; goto _test_eof; 
	_test_eof126: cs = 126; goto _test_eof; 
	_test_eof127: cs = 127; goto _test_eof; 
	_test_eof128: cs = 128; goto _test_eof; 
	_test_eof129: cs = 129; goto _test_eof; 
	_test_eof130: cs = 130; goto _test_eof; 
	_test_eof131: cs = 131; goto _test_eof; 
	_test_eof132: cs = 132; goto _test_eof; 
	_test_eof133: cs = 133; goto _test_eof; 
	_test_eof134: cs = 134; goto _test_eof; 
	_test_eof135: cs = 135; goto _test_eof; 
	_test_eof136: cs = 136; goto _test_eof; 
	_test_eof570: cs = 570; goto _test_eof; 
	_test_eof137: cs = 137; goto _test_eof; 
	_test_eof138: cs = 138; goto _test_eof; 
	_test_eof139: cs = 139; goto _test_eof; 
	_test_eof140: cs = 140; goto _test_eof; 
	_test_eof141: cs = 141; goto _test_eof; 
	_test_eof571: cs = 571; goto _test_eof; 
	_test_eof142: cs = 142; goto _test_eof; 
	_test_eof143: cs = 143; goto _test_eof; 
	_test_eof144: cs = 144; goto _test_eof; 
	_test_eof145: cs = 145; goto _test_eof; 
	_test_eof572: cs = 572; goto _test_eof; 
	_test_eof146: cs = 146; goto _test_eof; 
	_test_eof147: cs = 147; goto _test_eof; 
	_test_eof148: cs = 148; goto _test_eof; 
	_test_eof149: cs = 149; goto _test_eof; 
	_test_eof150: cs = 150; goto _test_eof; 
	_test_eof151: cs = 151; goto _test_eof; 
	_test_eof152: cs = 152; goto _test_eof; 
	_test_eof153: cs = 153; goto _test_eof; 
	_test_eof154: cs = 154; goto _test_eof; 
	_test_eof155: cs = 155; goto _test_eof; 
	_test_eof156: cs = 156; goto _test_eof; 
	_test_eof157: cs = 157; goto _test_eof; 
	_test_eof158: cs = 158; goto _test_eof; 
	_test_eof159: cs = 159; goto _test_eof; 
	_test_eof160: cs = 160; goto _test_eof; 
	_test_eof161: cs = 161; goto _test_eof; 
	_test_eof162: cs = 162; goto _test_eof; 
	_test_eof163: cs = 163; goto _test_eof; 
	_test_eof164: cs = 164; goto _test_eof; 
	_test_eof165: cs = 165; goto _test_eof; 
	_test_eof166: cs = 166; goto _test_eof; 
	_test_eof167: cs = 167; goto _test_eof; 
	_test_eof168: cs = 168; goto _test_eof; 
	_test_eof169: cs = 169; goto _test_eof; 
	_test_eof170: cs = 170; goto _test_eof; 
	_test_eof171: cs = 171; goto _test_eof; 
	_test_eof172: cs = 172; goto _test_eof; 
	_test_eof173: cs = 173; goto _test_eof; 
	_test_eof174: cs = 174; goto _test_eof; 
	_test_eof175: cs = 175; goto _test_eof; 
	_test_eof176: cs = 176; goto _test_eof; 
	_test_eof177: cs = 177; goto _test_eof; 
	_test_eof178: cs = 178; goto _test_eof; 
	_test_eof179: cs = 179; goto _test_eof; 
	_test_eof180: cs = 180; goto _test_eof; 
	_test_eof181: cs = 181; goto _test_eof; 
	_test_eof182: cs = 182; goto _test_eof; 
	_test_eof183: cs = 183; goto _test_eof; 
	_test_eof184: cs = 184; goto _test_eof; 
	_test_eof185: cs = 185; goto _test_eof; 
	_test_eof186: cs = 186; goto _test_eof; 
	_test_eof187: cs = 187; goto _test_eof; 
	_test_eof188: cs = 188; goto _test_eof; 
	_test_eof189: cs = 189; goto _test_eof; 
	_test_eof190: cs = 190; goto _test_eof; 
	_test_eof191: cs = 191; goto _test_eof; 
	_test_eof192: cs = 192; goto _test_eof; 
	_test_eof193: cs = 193; goto _test_eof; 
	_test_eof194: cs = 194; goto _test_eof; 
	_test_eof195: cs = 195; goto _test_eof; 
	_test_eof196: cs = 196; goto _test_eof; 
	_test_eof197: cs = 197; goto _test_eof; 
	_test_eof198: cs = 198; goto _test_eof; 
	_test_eof199: cs = 199; goto _test_eof; 
	_test_eof200: cs = 200; goto _test_eof; 
	_test_eof201: cs = 201; goto _test_eof; 
	_test_eof202: cs = 202; goto _test_eof; 
	_test_eof203: cs = 203; goto _test_eof; 
	_test_eof204: cs = 204; goto _test_eof; 
	_test_eof205: cs = 205; goto _test_eof; 
	_test_eof206: cs = 206; goto _test_eof; 
	_test_eof207: cs = 207; goto _test_eof; 
	_test_eof208: cs = 208; goto _test_eof; 
	_test_eof209: cs = 209; goto _test_eof; 
	_test_eof210: cs = 210; goto _test_eof; 
	_test_eof211: cs = 211; goto _test_eof; 
	_test_eof212: cs = 212; goto _test_eof; 
	_test_eof213: cs = 213; goto _test_eof; 
	_test_eof214: cs = 214; goto _test_eof; 
	_test_eof215: cs = 215; goto _test_eof; 
	_test_eof216: cs = 216; goto _test_eof; 
	_test_eof217: cs = 217; goto _test_eof; 
	_test_eof218: cs = 218; goto _test_eof; 
	_test_eof219: cs = 219; goto _test_eof; 
	_test_eof220: cs = 220; goto _test_eof; 
	_test_eof221: cs = 221; goto _test_eof; 
	_test_eof222: cs = 222; goto _test_eof; 
	_test_eof223: cs = 223; goto _test_eof; 
	_test_eof224: cs = 224; goto _test_eof; 
	_test_eof225: cs = 225; goto _test_eof; 
	_test_eof226: cs = 226; goto _test_eof; 
	_test_eof227: cs = 227; goto _test_eof; 
	_test_eof228: cs = 228; goto _test_eof; 
	_test_eof229: cs = 229; goto _test_eof; 
	_test_eof230: cs = 230; goto _test_eof; 
	_test_eof231: cs = 231; goto _test_eof; 
	_test_eof232: cs = 232; goto _test_eof; 
	_test_eof233: cs = 233; goto _test_eof; 
	_test_eof234: cs = 234; goto _test_eof; 
	_test_eof235: cs = 235; goto _test_eof; 
	_test_eof236: cs = 236; goto _test_eof; 
	_test_eof237: cs = 237; goto _test_eof; 
	_test_eof238: cs = 238; goto _test_eof; 
	_test_eof239: cs = 239; goto _test_eof; 
	_test_eof240: cs = 240; goto _test_eof; 
	_test_eof241: cs = 241; goto _test_eof; 
	_test_eof242: cs = 242; goto _test_eof; 
	_test_eof243: cs = 243; goto _test_eof; 
	_test_eof244: cs = 244; goto _test_eof; 
	_test_eof245: cs = 245; goto _test_eof; 
	_test_eof246: cs = 246; goto _test_eof; 
	_test_eof247: cs = 247; goto _test_eof; 
	_test_eof248: cs = 248; goto _test_eof; 
	_test_eof249: cs = 249; goto _test_eof; 
	_test_eof250: cs = 250; goto _test_eof; 
	_test_eof251: cs = 251; goto _test_eof; 
	_test_eof252: cs = 252; goto _test_eof; 
	_test_eof253: cs = 253; goto _test_eof; 
	_test_eof254: cs = 254; goto _test_eof; 
	_test_eof255: cs = 255; goto _test_eof; 
	_test_eof256: cs = 256; goto _test_eof; 
	_test_eof257: cs = 257; goto _test_eof; 
	_test_eof258: cs = 258; goto _test_eof; 
	_test_eof259: cs = 259; goto _test_eof; 
	_test_eof260: cs = 260; goto _test_eof; 
	_test_eof261: cs = 261; goto _test_eof; 
	_test_eof262: cs = 262; goto _test_eof; 
	_test_eof263: cs = 263; goto _test_eof; 
	_test_eof264: cs = 264; goto _test_eof; 
	_test_eof265: cs = 265; goto _test_eof; 
	_test_eof266: cs = 266; goto _test_eof; 
	_test_eof267: cs = 267; goto _test_eof; 
	_test_eof268: cs = 268; goto _test_eof; 
	_test_eof269: cs = 269; goto _test_eof; 
	_test_eof270: cs = 270; goto _test_eof; 
	_test_eof271: cs = 271; goto _test_eof; 
	_test_eof272: cs = 272; goto _test_eof; 
	_test_eof273: cs = 273; goto _test_eof; 
	_test_eof274: cs = 274; goto _test_eof; 
	_test_eof275: cs = 275; goto _test_eof; 
	_test_eof276: cs = 276; goto _test_eof; 
	_test_eof277: cs = 277; goto _test_eof; 
	_test_eof278: cs = 278; goto _test_eof; 
	_test_eof279: cs = 279; goto _test_eof; 
	_test_eof280: cs = 280; goto _test_eof; 
	_test_eof281: cs = 281; goto _test_eof; 
	_test_eof282: cs = 282; goto _test_eof; 
	_test_eof283: cs = 283; goto _test_eof; 
	_test_eof284: cs = 284; goto _test_eof; 
	_test_eof285: cs = 285; goto _test_eof; 
	_test_eof286: cs = 286; goto _test_eof; 
	_test_eof287: cs = 287; goto _test_eof; 
	_test_eof288: cs = 288; goto _test_eof; 
	_test_eof289: cs = 289; goto _test_eof; 
	_test_eof290: cs = 290; goto _test_eof; 
	_test_eof291: cs = 291; goto _test_eof; 
	_test_eof292: cs = 292; goto _test_eof; 
	_test_eof293: cs = 293; goto _test_eof; 
	_test_eof294: cs = 294; goto _test_eof; 
	_test_eof295: cs = 295; goto _test_eof; 
	_test_eof296: cs = 296; goto _test_eof; 
	_test_eof297: cs = 297; goto _test_eof; 
	_test_eof298: cs = 298; goto _test_eof; 
	_test_eof299: cs = 299; goto _test_eof; 
	_test_eof300: cs = 300; goto _test_eof; 
	_test_eof301: cs = 301; goto _test_eof; 
	_test_eof302: cs = 302; goto _test_eof; 
	_test_eof303: cs = 303; goto _test_eof; 
	_test_eof304: cs = 304; goto _test_eof; 
	_test_eof305: cs = 305; goto _test_eof; 
	_test_eof306: cs = 306; goto _test_eof; 
	_test_eof307: cs = 307; goto _test_eof; 
	_test_eof308: cs = 308; goto _test_eof; 
	_test_eof309: cs = 309; goto _test_eof; 
	_test_eof310: cs = 310; goto _test_eof; 
	_test_eof311: cs = 311; goto _test_eof; 
	_test_eof312: cs = 312; goto _test_eof; 
	_test_eof313: cs = 313; goto _test_eof; 
	_test_eof314: cs = 314; goto _test_eof; 
	_test_eof315: cs = 315; goto _test_eof; 
	_test_eof316: cs = 316; goto _test_eof; 
	_test_eof317: cs = 317; goto _test_eof; 
	_test_eof318: cs = 318; goto _test_eof; 
	_test_eof319: cs = 319; goto _test_eof; 
	_test_eof320: cs = 320; goto _test_eof; 
	_test_eof321: cs = 321; goto _test_eof; 
	_test_eof322: cs = 322; goto _test_eof; 
	_test_eof323: cs = 323; goto _test_eof; 
	_test_eof324: cs = 324; goto _test_eof; 
	_test_eof325: cs = 325; goto _test_eof; 
	_test_eof326: cs = 326; goto _test_eof; 
	_test_eof327: cs = 327; goto _test_eof; 
	_test_eof328: cs = 328; goto _test_eof; 
	_test_eof329: cs = 329; goto _test_eof; 
	_test_eof330: cs = 330; goto _test_eof; 
	_test_eof331: cs = 331; goto _test_eof; 
	_test_eof332: cs = 332; goto _test_eof; 
	_test_eof333: cs = 333; goto _test_eof; 
	_test_eof334: cs = 334; goto _test_eof; 
	_test_eof335: cs = 335; goto _test_eof; 
	_test_eof336: cs = 336; goto _test_eof; 
	_test_eof337: cs = 337; goto _test_eof; 
	_test_eof338: cs = 338; goto _test_eof; 
	_test_eof339: cs = 339; goto _test_eof; 
	_test_eof340: cs = 340; goto _test_eof; 
	_test_eof341: cs = 341; goto _test_eof; 
	_test_eof342: cs = 342; goto _test_eof; 
	_test_eof343: cs = 343; goto _test_eof; 
	_test_eof344: cs = 344; goto _test_eof; 
	_test_eof345: cs = 345; goto _test_eof; 
	_test_eof346: cs = 346; goto _test_eof; 
	_test_eof347: cs = 347; goto _test_eof; 
	_test_eof348: cs = 348; goto _test_eof; 
	_test_eof349: cs = 349; goto _test_eof; 
	_test_eof350: cs = 350; goto _test_eof; 
	_test_eof351: cs = 351; goto _test_eof; 
	_test_eof352: cs = 352; goto _test_eof; 
	_test_eof353: cs = 353; goto _test_eof; 
	_test_eof354: cs = 354; goto _test_eof; 
	_test_eof355: cs = 355; goto _test_eof; 
	_test_eof356: cs = 356; goto _test_eof; 
	_test_eof357: cs = 357; goto _test_eof; 
	_test_eof358: cs = 358; goto _test_eof; 
	_test_eof359: cs = 359; goto _test_eof; 
	_test_eof360: cs = 360; goto _test_eof; 
	_test_eof361: cs = 361; goto _test_eof; 
	_test_eof362: cs = 362; goto _test_eof; 
	_test_eof363: cs = 363; goto _test_eof; 
	_test_eof364: cs = 364; goto _test_eof; 
	_test_eof365: cs = 365; goto _test_eof; 
	_test_eof366: cs = 366; goto _test_eof; 
	_test_eof367: cs = 367; goto _test_eof; 
	_test_eof368: cs = 368; goto _test_eof; 
	_test_eof369: cs = 369; goto _test_eof; 
	_test_eof370: cs = 370; goto _test_eof; 
	_test_eof371: cs = 371; goto _test_eof; 
	_test_eof372: cs = 372; goto _test_eof; 
	_test_eof373: cs = 373; goto _test_eof; 
	_test_eof374: cs = 374; goto _test_eof; 
	_test_eof375: cs = 375; goto _test_eof; 
	_test_eof376: cs = 376; goto _test_eof; 
	_test_eof377: cs = 377; goto _test_eof; 
	_test_eof378: cs = 378; goto _test_eof; 
	_test_eof379: cs = 379; goto _test_eof; 
	_test_eof380: cs = 380; goto _test_eof; 
	_test_eof381: cs = 381; goto _test_eof; 
	_test_eof382: cs = 382; goto _test_eof; 
	_test_eof383: cs = 383; goto _test_eof; 
	_test_eof384: cs = 384; goto _test_eof; 
	_test_eof385: cs = 385; goto _test_eof; 
	_test_eof386: cs = 386; goto _test_eof; 
	_test_eof387: cs = 387; goto _test_eof; 
	_test_eof388: cs = 388; goto _test_eof; 
	_test_eof389: cs = 389; goto _test_eof; 
	_test_eof390: cs = 390; goto _test_eof; 
	_test_eof391: cs = 391; goto _test_eof; 
	_test_eof392: cs = 392; goto _test_eof; 
	_test_eof393: cs = 393; goto _test_eof; 
	_test_eof394: cs = 394; goto _test_eof; 
	_test_eof395: cs = 395; goto _test_eof; 
	_test_eof396: cs = 396; goto _test_eof; 
	_test_eof397: cs = 397; goto _test_eof; 
	_test_eof398: cs = 398; goto _test_eof; 
	_test_eof399: cs = 399; goto _test_eof; 
	_test_eof400: cs = 400; goto _test_eof; 
	_test_eof401: cs = 401; goto _test_eof; 
	_test_eof402: cs = 402; goto _test_eof; 
	_test_eof403: cs = 403; goto _test_eof; 
	_test_eof404: cs = 404; goto _test_eof; 
	_test_eof405: cs = 405; goto _test_eof; 
	_test_eof406: cs = 406; goto _test_eof; 
	_test_eof407: cs = 407; goto _test_eof; 
	_test_eof408: cs = 408; goto _test_eof; 
	_test_eof409: cs = 409; goto _test_eof; 
	_test_eof410: cs = 410; goto _test_eof; 
	_test_eof411: cs = 411; goto _test_eof; 
	_test_eof412: cs = 412; goto _test_eof; 
	_test_eof413: cs = 413; goto _test_eof; 
	_test_eof414: cs = 414; goto _test_eof; 
	_test_eof415: cs = 415; goto _test_eof; 
	_test_eof416: cs = 416; goto _test_eof; 
	_test_eof417: cs = 417; goto _test_eof; 
	_test_eof418: cs = 418; goto _test_eof; 
	_test_eof419: cs = 419; goto _test_eof; 
	_test_eof420: cs = 420; goto _test_eof; 
	_test_eof421: cs = 421; goto _test_eof; 
	_test_eof422: cs = 422; goto _test_eof; 
	_test_eof423: cs = 423; goto _test_eof; 
	_test_eof424: cs = 424; goto _test_eof; 
	_test_eof425: cs = 425; goto _test_eof; 
	_test_eof426: cs = 426; goto _test_eof; 
	_test_eof427: cs = 427; goto _test_eof; 
	_test_eof428: cs = 428; goto _test_eof; 
	_test_eof429: cs = 429; goto _test_eof; 
	_test_eof430: cs = 430; goto _test_eof; 
	_test_eof431: cs = 431; goto _test_eof; 
	_test_eof432: cs = 432; goto _test_eof; 
	_test_eof433: cs = 433; goto _test_eof; 
	_test_eof434: cs = 434; goto _test_eof; 
	_test_eof435: cs = 435; goto _test_eof; 
	_test_eof436: cs = 436; goto _test_eof; 
	_test_eof437: cs = 437; goto _test_eof; 
	_test_eof438: cs = 438; goto _test_eof; 
	_test_eof439: cs = 439; goto _test_eof; 
	_test_eof440: cs = 440; goto _test_eof; 
	_test_eof441: cs = 441; goto _test_eof; 
	_test_eof442: cs = 442; goto _test_eof; 
	_test_eof443: cs = 443; goto _test_eof; 
	_test_eof444: cs = 444; goto _test_eof; 
	_test_eof445: cs = 445; goto _test_eof; 
	_test_eof446: cs = 446; goto _test_eof; 
	_test_eof447: cs = 447; goto _test_eof; 
	_test_eof448: cs = 448; goto _test_eof; 
	_test_eof449: cs = 449; goto _test_eof; 
	_test_eof450: cs = 450; goto _test_eof; 
	_test_eof451: cs = 451; goto _test_eof; 
	_test_eof452: cs = 452; goto _test_eof; 
	_test_eof453: cs = 453; goto _test_eof; 
	_test_eof454: cs = 454; goto _test_eof; 
	_test_eof455: cs = 455; goto _test_eof; 
	_test_eof456: cs = 456; goto _test_eof; 
	_test_eof457: cs = 457; goto _test_eof; 
	_test_eof458: cs = 458; goto _test_eof; 
	_test_eof459: cs = 459; goto _test_eof; 
	_test_eof460: cs = 460; goto _test_eof; 
	_test_eof461: cs = 461; goto _test_eof; 
	_test_eof462: cs = 462; goto _test_eof; 
	_test_eof463: cs = 463; goto _test_eof; 
	_test_eof464: cs = 464; goto _test_eof; 
	_test_eof465: cs = 465; goto _test_eof; 
	_test_eof466: cs = 466; goto _test_eof; 
	_test_eof467: cs = 467; goto _test_eof; 
	_test_eof468: cs = 468; goto _test_eof; 
	_test_eof469: cs = 469; goto _test_eof; 
	_test_eof470: cs = 470; goto _test_eof; 
	_test_eof471: cs = 471; goto _test_eof; 
	_test_eof472: cs = 472; goto _test_eof; 
	_test_eof473: cs = 473; goto _test_eof; 
	_test_eof474: cs = 474; goto _test_eof; 
	_test_eof475: cs = 475; goto _test_eof; 
	_test_eof476: cs = 476; goto _test_eof; 
	_test_eof477: cs = 477; goto _test_eof; 
	_test_eof478: cs = 478; goto _test_eof; 
	_test_eof479: cs = 479; goto _test_eof; 
	_test_eof480: cs = 480; goto _test_eof; 
	_test_eof481: cs = 481; goto _test_eof; 
	_test_eof482: cs = 482; goto _test_eof; 
	_test_eof483: cs = 483; goto _test_eof; 
	_test_eof484: cs = 484; goto _test_eof; 
	_test_eof485: cs = 485; goto _test_eof; 
	_test_eof486: cs = 486; goto _test_eof; 
	_test_eof487: cs = 487; goto _test_eof; 
	_test_eof488: cs = 488; goto _test_eof; 
	_test_eof489: cs = 489; goto _test_eof; 
	_test_eof490: cs = 490; goto _test_eof; 
	_test_eof491: cs = 491; goto _test_eof; 
	_test_eof492: cs = 492; goto _test_eof; 
	_test_eof493: cs = 493; goto _test_eof; 
	_test_eof494: cs = 494; goto _test_eof; 
	_test_eof495: cs = 495; goto _test_eof; 
	_test_eof496: cs = 496; goto _test_eof; 
	_test_eof497: cs = 497; goto _test_eof; 
	_test_eof498: cs = 498; goto _test_eof; 
	_test_eof499: cs = 499; goto _test_eof; 
	_test_eof500: cs = 500; goto _test_eof; 
	_test_eof501: cs = 501; goto _test_eof; 
	_test_eof502: cs = 502; goto _test_eof; 
	_test_eof503: cs = 503; goto _test_eof; 
	_test_eof504: cs = 504; goto _test_eof; 
	_test_eof505: cs = 505; goto _test_eof; 
	_test_eof506: cs = 506; goto _test_eof; 
	_test_eof507: cs = 507; goto _test_eof; 
	_test_eof508: cs = 508; goto _test_eof; 
	_test_eof509: cs = 509; goto _test_eof; 
	_test_eof510: cs = 510; goto _test_eof; 
	_test_eof511: cs = 511; goto _test_eof; 
	_test_eof512: cs = 512; goto _test_eof; 
	_test_eof513: cs = 513; goto _test_eof; 
	_test_eof514: cs = 514; goto _test_eof; 
	_test_eof515: cs = 515; goto _test_eof; 
	_test_eof516: cs = 516; goto _test_eof; 
	_test_eof517: cs = 517; goto _test_eof; 
	_test_eof518: cs = 518; goto _test_eof; 
	_test_eof519: cs = 519; goto _test_eof; 
	_test_eof520: cs = 520; goto _test_eof; 
	_test_eof521: cs = 521; goto _test_eof; 
	_test_eof522: cs = 522; goto _test_eof; 
	_test_eof523: cs = 523; goto _test_eof; 
	_test_eof524: cs = 524; goto _test_eof; 
	_test_eof525: cs = 525; goto _test_eof; 
	_test_eof526: cs = 526; goto _test_eof; 
	_test_eof527: cs = 527; goto _test_eof; 
	_test_eof528: cs = 528; goto _test_eof; 
	_test_eof529: cs = 529; goto _test_eof; 
	_test_eof530: cs = 530; goto _test_eof; 
	_test_eof531: cs = 531; goto _test_eof; 
	_test_eof532: cs = 532; goto _test_eof; 
	_test_eof533: cs = 533; goto _test_eof; 
	_test_eof534: cs = 534; goto _test_eof; 
	_test_eof535: cs = 535; goto _test_eof; 
	_test_eof536: cs = 536; goto _test_eof; 
	_test_eof537: cs = 537; goto _test_eof; 
	_test_eof538: cs = 538; goto _test_eof; 
	_test_eof539: cs = 539; goto _test_eof; 
	_test_eof540: cs = 540; goto _test_eof; 
	_test_eof541: cs = 541; goto _test_eof; 
	_test_eof542: cs = 542; goto _test_eof; 
	_test_eof543: cs = 543; goto _test_eof; 
	_test_eof544: cs = 544; goto _test_eof; 
	_test_eof545: cs = 545; goto _test_eof; 
	_test_eof546: cs = 546; goto _test_eof; 
	_test_eof547: cs = 547; goto _test_eof; 
	_test_eof548: cs = 548; goto _test_eof; 
	_test_eof549: cs = 549; goto _test_eof; 
	_test_eof550: cs = 550; goto _test_eof; 
	_test_eof551: cs = 551; goto _test_eof; 
	_test_eof552: cs = 552; goto _test_eof; 
	_test_eof553: cs = 553; goto _test_eof; 
	_test_eof554: cs = 554; goto _test_eof; 
	_test_eof555: cs = 555; goto _test_eof; 
	_test_eof556: cs = 556; goto _test_eof; 
	_test_eof557: cs = 557; goto _test_eof; 
	_test_eof558: cs = 558; goto _test_eof; 
	_test_eof559: cs = 559; goto _test_eof; 
	_test_eof560: cs = 560; goto _test_eof; 
	_test_eof561: cs = 561; goto _test_eof; 
	_test_eof562: cs = 562; goto _test_eof; 
	_test_eof563: cs = 563; goto _test_eof; 
	_test_eof564: cs = 564; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 567: 
/* #line 521 "EcrioCPIMParse.rl" */
	{
		if (bIsMultiPart == Enum_FALSE)
		{
			if (uCntLen == 0 || uCntLen > (p - tag_start))
			{
				uCntLen = p - tag_start;
			}

			c->buff[0].uSize = 0;
			uCPIMError = ec_cpim_MaintenanceBuffer(&c->buff[0], uCntLen);
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tThe working buffer maintenance error.", __FUNCTION__, __LINE__);
				uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
				goto END;
			}
			pal_MemorySet(c->buff[0].pData, 0, c->buff[0].uContainerSize);
			pal_MemoryCopy(c->buff[0].pData, c->buff[0].uContainerSize, (u_char*)tag_start, uCntLen);
			c->buff[0].uSize = uCntLen;

			pStruct->uNumOfBuffers = 1;
			pStruct->buff[0].eContentType = eConType;
			if (eConType == CPIMContentType_Specified)
			{
				pStruct->buff[0].pContentType = pContentType;
			}
			else
			{
				pStruct->buff[0].pContentType = NULL;
			}
			pStruct->buff[0].pMsgBody = c->buff[0].pData;
			pStruct->buff[0].uMsgLen = c->buff[0].uSize;
			if (pContentId != NULL)
			{
				pStruct->buff[0].pContentId = pContentId;
			}
			else
			{
				pStruct->buff[0].pContentId = NULL;
			}
			if (pContentDisposition != NULL)
			{
				pStruct->buff[0].pContentDisposition = pContentDisposition;
			}
			else
			{
				pStruct->buff[0].pContentDisposition = NULL;
			}
			if (pContentTransferEncoding != NULL)
			{
				pStruct->buff[0].pContentTransferEncoding = pContentTransferEncoding;
			}
			else
			{
				pStruct->buff[0].pContentTransferEncoding = NULL;
			}
		}
		else
		{
			uCPIMError = ec_cpim_ParseMultiPartMessage(c, pStruct, cBoundary, (u_char*)tag_start, uCntLen);
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				uCPIMError = ECRIO_CPIM_PARSING_INSUFFICIENT_ERROR;
				goto END;
			}
		}
	}
	break;
	case 1: 
	case 2: 
	case 3: 
	case 4: 
	case 5: 
	case 6: 
	case 7: 
	case 8: 
	case 9: 
	case 10: 
	case 11: 
	case 12: 
	case 13: 
	case 14: 
	case 15: 
	case 16: 
	case 17: 
	case 18: 
	case 19: 
	case 20: 
	case 21: 
	case 22: 
	case 23: 
	case 24: 
	case 25: 
	case 26: 
	case 27: 
	case 28: 
	case 29: 
	case 30: 
	case 31: 
	case 32: 
	case 33: 
	case 34: 
	case 35: 
	case 36: 
	case 37: 
	case 38: 
	case 39: 
	case 40: 
	case 41: 
	case 42: 
	case 43: 
	case 44: 
	case 45: 
	case 46: 
	case 47: 
	case 48: 
	case 49: 
	case 50: 
	case 51: 
	case 52: 
	case 53: 
	case 54: 
	case 55: 
	case 56: 
	case 57: 
	case 58: 
	case 59: 
	case 60: 
	case 61: 
	case 62: 
	case 63: 
	case 64: 
	case 65: 
	case 66: 
	case 67: 
	case 68: 
	case 69: 
	case 70: 
	case 71: 
	case 72: 
	case 73: 
	case 74: 
	case 75: 
	case 76: 
	case 77: 
	case 78: 
	case 79: 
	case 80: 
	case 81: 
	case 82: 
	case 83: 
	case 84: 
	case 85: 
	case 86: 
	case 87: 
	case 88: 
	case 89: 
	case 90: 
	case 91: 
	case 92: 
	case 93: 
	case 94: 
	case 95: 
	case 96: 
	case 97: 
	case 98: 
	case 99: 
	case 100: 
	case 101: 
	case 102: 
	case 103: 
	case 104: 
	case 105: 
	case 106: 
	case 107: 
	case 108: 
	case 109: 
	case 110: 
	case 111: 
	case 112: 
	case 113: 
	case 114: 
	case 115: 
	case 116: 
	case 117: 
	case 118: 
	case 119: 
	case 120: 
	case 121: 
	case 122: 
	case 123: 
	case 124: 
	case 125: 
	case 126: 
	case 127: 
	case 128: 
	case 129: 
	case 130: 
	case 131: 
	case 132: 
	case 133: 
	case 134: 
	case 135: 
	case 136: 
	case 137: 
	case 138: 
	case 139: 
	case 140: 
	case 141: 
	case 142: 
	case 143: 
	case 144: 
	case 145: 
	case 146: 
	case 147: 
	case 148: 
	case 149: 
	case 150: 
	case 151: 
	case 152: 
	case 153: 
	case 154: 
	case 155: 
	case 156: 
	case 157: 
	case 158: 
	case 159: 
	case 160: 
	case 161: 
	case 162: 
	case 163: 
	case 164: 
	case 165: 
	case 166: 
	case 167: 
	case 168: 
	case 169: 
	case 170: 
	case 171: 
	case 172: 
	case 173: 
	case 174: 
	case 175: 
	case 176: 
	case 177: 
	case 178: 
	case 179: 
	case 180: 
	case 181: 
	case 182: 
	case 183: 
	case 184: 
	case 185: 
	case 186: 
	case 187: 
	case 188: 
	case 189: 
	case 190: 
	case 191: 
	case 192: 
	case 193: 
	case 194: 
	case 195: 
	case 196: 
	case 197: 
	case 198: 
	case 199: 
	case 200: 
	case 201: 
	case 202: 
	case 203: 
	case 204: 
	case 205: 
	case 206: 
	case 207: 
	case 208: 
	case 209: 
	case 210: 
	case 211: 
	case 212: 
	case 213: 
	case 214: 
	case 215: 
	case 216: 
	case 217: 
	case 218: 
	case 219: 
	case 220: 
	case 221: 
	case 222: 
	case 223: 
	case 224: 
	case 225: 
	case 226: 
	case 227: 
	case 228: 
	case 229: 
	case 230: 
	case 231: 
	case 232: 
	case 233: 
	case 234: 
	case 235: 
	case 236: 
	case 237: 
	case 238: 
	case 239: 
	case 240: 
	case 241: 
	case 242: 
	case 243: 
	case 244: 
	case 245: 
	case 246: 
	case 247: 
	case 248: 
	case 249: 
	case 250: 
	case 251: 
	case 252: 
	case 253: 
	case 254: 
	case 255: 
	case 256: 
	case 257: 
	case 258: 
	case 259: 
	case 260: 
	case 261: 
	case 262: 
	case 263: 
	case 264: 
	case 265: 
	case 266: 
	case 267: 
	case 268: 
	case 269: 
	case 270: 
	case 271: 
	case 272: 
	case 273: 
	case 274: 
	case 275: 
	case 276: 
	case 277: 
	case 278: 
	case 279: 
	case 280: 
	case 281: 
	case 282: 
	case 283: 
	case 284: 
	case 285: 
	case 286: 
	case 287: 
	case 288: 
	case 289: 
	case 290: 
	case 291: 
	case 292: 
	case 293: 
	case 294: 
	case 295: 
	case 296: 
	case 297: 
	case 298: 
	case 299: 
	case 300: 
	case 301: 
	case 302: 
	case 303: 
	case 304: 
	case 305: 
	case 306: 
	case 307: 
	case 308: 
	case 309: 
	case 310: 
	case 311: 
	case 312: 
	case 313: 
	case 314: 
	case 315: 
	case 316: 
	case 317: 
	case 318: 
	case 319: 
	case 320: 
	case 321: 
	case 322: 
	case 323: 
	case 324: 
	case 325: 
	case 326: 
	case 327: 
	case 328: 
	case 329: 
	case 330: 
	case 331: 
	case 332: 
	case 333: 
	case 334: 
	case 335: 
	case 336: 
	case 337: 
	case 338: 
	case 339: 
	case 340: 
	case 341: 
	case 342: 
	case 343: 
	case 344: 
	case 345: 
	case 346: 
	case 347: 
	case 348: 
	case 349: 
	case 350: 
	case 351: 
	case 352: 
	case 353: 
	case 354: 
	case 355: 
	case 356: 
	case 357: 
	case 358: 
	case 359: 
	case 360: 
	case 361: 
	case 362: 
	case 363: 
	case 364: 
	case 365: 
	case 366: 
	case 367: 
	case 368: 
	case 369: 
	case 370: 
	case 371: 
	case 372: 
	case 373: 
	case 374: 
	case 375: 
	case 376: 
	case 377: 
	case 378: 
	case 379: 
	case 380: 
	case 381: 
	case 382: 
	case 383: 
	case 384: 
	case 385: 
	case 386: 
	case 387: 
	case 388: 
	case 389: 
	case 390: 
	case 391: 
	case 392: 
	case 393: 
	case 394: 
	case 395: 
	case 396: 
	case 397: 
	case 398: 
	case 399: 
	case 400: 
	case 401: 
	case 402: 
	case 403: 
	case 404: 
	case 405: 
	case 406: 
	case 407: 
	case 408: 
	case 409: 
	case 410: 
	case 411: 
	case 412: 
	case 413: 
	case 414: 
	case 415: 
	case 416: 
	case 417: 
	case 418: 
	case 419: 
	case 420: 
	case 421: 
	case 422: 
	case 423: 
	case 424: 
	case 425: 
	case 426: 
	case 427: 
	case 428: 
	case 429: 
	case 430: 
	case 431: 
	case 432: 
	case 433: 
	case 434: 
	case 435: 
	case 436: 
	case 437: 
	case 438: 
	case 439: 
	case 440: 
	case 441: 
	case 442: 
	case 443: 
	case 444: 
	case 445: 
	case 446: 
	case 447: 
	case 448: 
	case 449: 
	case 450: 
	case 451: 
	case 452: 
	case 453: 
	case 454: 
	case 455: 
	case 456: 
	case 457: 
	case 458: 
	case 459: 
	case 460: 
	case 461: 
	case 462: 
	case 463: 
	case 464: 
	case 465: 
	case 466: 
	case 467: 
	case 468: 
	case 469: 
	case 470: 
	case 471: 
	case 472: 
	case 473: 
	case 474: 
	case 475: 
	case 476: 
	case 477: 
	case 478: 
	case 479: 
	case 480: 
	case 481: 
	case 482: 
	case 483: 
	case 484: 
	case 485: 
	case 486: 
	case 487: 
	case 488: 
	case 489: 
	case 490: 
	case 491: 
	case 492: 
	case 493: 
	case 494: 
	case 495: 
	case 496: 
	case 497: 
	case 498: 
	case 499: 
	case 500: 
	case 501: 
	case 502: 
	case 503: 
	case 504: 
	case 505: 
	case 506: 
	case 507: 
	case 508: 
	case 509: 
	case 510: 
	case 511: 
	case 512: 
	case 513: 
	case 514: 
	case 515: 
	case 516: 
	case 517: 
	case 518: 
	case 519: 
	case 520: 
	case 521: 
	case 522: 
	case 523: 
	case 524: 
	case 525: 
	case 526: 
	case 527: 
	case 528: 
	case 529: 
	case 530: 
	case 531: 
	case 532: 
	case 533: 
	case 534: 
	case 535: 
	case 536: 
	case 537: 
	case 538: 
	case 539: 
	case 540: 
	case 541: 
	case 542: 
	case 543: 
	case 544: 
	case 545: 
	case 546: 
	case 547: 
	case 548: 
	case 549: 
	case 550: 
	case 551: 
	case 552: 
	case 553: 
	case 554: 
	case 555: 
	case 556: 
	case 557: 
	case 558: 
	case 559: 
	case 560: 
	case 561: 
	case 562: 
	case 563: 
	case 564: 
/* #line 596 "EcrioCPIMParse.rl" */
	{
		/** This is a general error, but usually no CRLF at the end of line. */
		uCPIMError = ECRIO_CPIM_PARSING_INSUFFICIENT_ERROR;
	}
	break;
	case 566: 
/* #line 82 "EcrioCPIMParse.rl" */
	{
		tag_start = p;
	}
/* #line 521 "EcrioCPIMParse.rl" */
	{
		if (bIsMultiPart == Enum_FALSE)
		{
			if (uCntLen == 0 || uCntLen > (p - tag_start))
			{
				uCntLen = p - tag_start;
			}

			c->buff[0].uSize = 0;
			uCPIMError = ec_cpim_MaintenanceBuffer(&c->buff[0], uCntLen);
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tThe working buffer maintenance error.", __FUNCTION__, __LINE__);
				uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
				goto END;
			}
			pal_MemorySet(c->buff[0].pData, 0, c->buff[0].uContainerSize);
			pal_MemoryCopy(c->buff[0].pData, c->buff[0].uContainerSize, (u_char*)tag_start, uCntLen);
			c->buff[0].uSize = uCntLen;

			pStruct->uNumOfBuffers = 1;
			pStruct->buff[0].eContentType = eConType;
			if (eConType == CPIMContentType_Specified)
			{
				pStruct->buff[0].pContentType = pContentType;
			}
			else
			{
				pStruct->buff[0].pContentType = NULL;
			}
			pStruct->buff[0].pMsgBody = c->buff[0].pData;
			pStruct->buff[0].uMsgLen = c->buff[0].uSize;
			if (pContentId != NULL)
			{
				pStruct->buff[0].pContentId = pContentId;
			}
			else
			{
				pStruct->buff[0].pContentId = NULL;
			}
			if (pContentDisposition != NULL)
			{
				pStruct->buff[0].pContentDisposition = pContentDisposition;
			}
			else
			{
				pStruct->buff[0].pContentDisposition = NULL;
			}
			if (pContentTransferEncoding != NULL)
			{
				pStruct->buff[0].pContentTransferEncoding = pContentTransferEncoding;
			}
			else
			{
				pStruct->buff[0].pContentTransferEncoding = NULL;
			}
		}
		else
		{
			uCPIMError = ec_cpim_ParseMultiPartMessage(c, pStruct, cBoundary, (u_char*)tag_start, uCntLen);
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				uCPIMError = ECRIO_CPIM_PARSING_INSUFFICIENT_ERROR;
				goto END;
			}
		}
	}
	break;
/* #line 19275 "EcrioCPIMParse.c" */
	}
	}

	_out: {}
	}

/* #line 872 "EcrioCPIMParse.rl" */

END:

	CPIM_MUTEX_UNLOCK(c->mutexAPI, c->logHandle);

	CPIMLOGI(c->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return uCPIMError;
}
