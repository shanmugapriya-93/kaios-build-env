/******************************************************************************

Copyright (c) 2019-2020 Ecrio, Inc. All Rights Reserved.

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
 * @file EcrioCPMParseXML.c
 * @brief Implementation of the CPM module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioCPM.h"
#include "EcrioCPMInternal.h"


/* #line 56 "EcrioCPMParseXML.c" */
static const int ec_cpm_XMLParser_start = 1;
static const int ec_cpm_XMLParser_first_final = 458;
static const int ec_cpm_XMLParser_error = 0;

static const int ec_cpm_XMLParser_en_main = 1;


/* #line 279 "EcrioCPMParseXML.rl" */


u_int32 ec_CPM_ParseXmlDocument
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMParsedXMLStruct **ppParsedXml,
	char *pData,
	u_int32 uLen
)
{
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
	EcrioCPMParsedXMLStruct *pCurr = NULL;
	EcrioCPMParsedXMLStruct *pStruct = NULL;
	EcrioCPMParsedXMLAttrStruct *pAttr = NULL;
	s_int32 cs = 1;
	const char *p;
	const char *pe;
	const char *eof;
	const char *tag_start = NULL;
	const char *value_start = NULL;
	BoolEnum bIsValue = Enum_FALSE;

	if (pContext == NULL || ppParsedXml == NULL || pData == NULL || uLen == 0)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	CPMLOGI(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpData = %x, uLen = %d", __FUNCTION__, __LINE__, pData, uLen);

	pCurr = (void*)ppParsedXml;

	p = pData;
	pe = pData + uLen;
	eof = pe;

	
/* #line 102 "EcrioCPMParseXML.c" */
	{
	cs = ec_cpm_XMLParser_start;
	}

/* #line 316 "EcrioCPMParseXML.rl" */
	
/* #line 109 "EcrioCPMParseXML.c" */
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	switch( (*p) ) {
		case 13: goto st2;
		case 32: goto st2;
		case 60: goto st401;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st2;
	goto tr0;
tr0:
/* #line 195 "EcrioCPMParseXML.rl" */
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tError: p = 0x%x, number = %d, pe = 0x%x", __FUNCTION__, __LINE__, p, p - pData, pe);
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tValue: %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x |%2.2x| %2.2x %2.2x %2.2x", __FUNCTION__, __LINE__,
			*(p-8), *(p-7), *(p-6), *(p-5), *(p-4), *(p-3), *(p-2), *(p-1), *(p), *(p+1), *(p+2), *(p+3));
		uCPMError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}
	goto st0;
/* #line 133 "EcrioCPMParseXML.c" */
st0:
cs = 0;
	goto _out;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	switch( (*p) ) {
		case 13: goto st2;
		case 32: goto st2;
		case 60: goto st3;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st2;
	goto tr0;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	switch( (*p) ) {
		case 33: goto st112;
		case 58: goto tr4;
		case 63: goto st395;
		case 95: goto tr4;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto tr4;
		} else if ( (*p) >= -64 )
			goto tr4;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr4;
		} else if ( (*p) >= 65 )
			goto tr4;
	} else
		goto tr4;
	goto tr0;
tr4:
/* #line 81 "EcrioCPMParseXML.rl" */
	{
		tag_start = p;
	}
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
/* #line 184 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto tr8;
		case 32: goto tr8;
		case 47: goto tr9;
		case 62: goto tr10;
		case 96: goto tr0;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto tr8;
	goto st4;
tr8:
/* #line 92 "EcrioCPMParseXML.rl" */
	{
		if (bIsValue == Enum_TRUE)
		{
			bIsValue = Enum_FALSE;
		}

		pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLStruct), (void **)&pStruct);
		if (NULL == pStruct)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;

			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto END;
		}
		pal_MemorySet((void*)pStruct, 0, sizeof(EcrioCPMParsedXMLStruct));
		pStruct->parent = (void*)pCurr;
		pCurr = pStruct;

		pStruct->pElementName = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
/* #line 246 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 13: goto st5;
		case 32: goto st5;
		case 47: goto st12;
		case 58: goto tr11;
		case 62: goto st25;
		case 95: goto tr11;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto tr11;
		} else if ( (*p) >= -64 )
			goto tr11;
	} else if ( (*p) > -1 ) {
		if ( (*p) < 65 ) {
			if ( 9 <= (*p) && (*p) <= 10 )
				goto st5;
		} else if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr11;
		} else
			goto tr11;
	} else
		goto tr11;
	goto tr0;
tr11:
/* #line 81 "EcrioCPMParseXML.rl" */
	{
		tag_start = p;
	}
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
/* #line 283 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto tr16;
		case 32: goto tr16;
		case 47: goto tr0;
		case 61: goto tr17;
		case 96: goto tr0;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto tr16;
	goto st6;
tr16:
/* #line 124 "EcrioCPMParseXML.rl" */
	{
		pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLAttrStruct), (void **)&pAttr);
		if (NULL == pAttr)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;

			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto END;
		}

		if (pStruct->uNumOfAttr == 0)
		{
			pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLAttrStruct*), (void **)&pStruct->ppAttr);
		}
		else
		{
			/* Check arithmetic overflow */
			if (pal_UtilityArithmeticOverflowDetected(pStruct->uNumOfAttr, 1) == Enum_TRUE)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;
				goto END;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((pStruct->uNumOfAttr + 1), sizeof(EcrioCPMParsedXMLAttrStruct*)) == Enum_TRUE)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;
				goto END;
			}
			pal_MemoryReallocate((u_int32)((pStruct->uNumOfAttr + 1) * sizeof(EcrioCPMParsedXMLAttrStruct*)), (void **)&pStruct->ppAttr);
		}
		if (NULL == pStruct->ppAttr)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;

			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto END;
		}
		pStruct->ppAttr[pStruct->uNumOfAttr] = pAttr;
		pStruct->uNumOfAttr++;

		pAttr->pAttrName = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
/* #line 356 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 13: goto st7;
		case 32: goto st7;
		case 61: goto st8;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st7;
	goto tr0;
tr17:
/* #line 124 "EcrioCPMParseXML.rl" */
	{
		pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLAttrStruct), (void **)&pAttr);
		if (NULL == pAttr)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;

			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto END;
		}

		if (pStruct->uNumOfAttr == 0)
		{
			pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLAttrStruct*), (void **)&pStruct->ppAttr);
		}
		else
		{
			/* Check arithmetic overflow */
			if (pal_UtilityArithmeticOverflowDetected(pStruct->uNumOfAttr, 1) == Enum_TRUE)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;
				goto END;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((pStruct->uNumOfAttr + 1), sizeof(EcrioCPMParsedXMLAttrStruct*)) == Enum_TRUE)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;
				goto END;
			}
			pal_MemoryReallocate((u_int32)((pStruct->uNumOfAttr + 1) * sizeof(EcrioCPMParsedXMLAttrStruct*)), (void **)&pStruct->ppAttr);
		}
		if (NULL == pStruct->ppAttr)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;

			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto END;
		}
		pStruct->ppAttr[pStruct->uNumOfAttr] = pAttr;
		pStruct->uNumOfAttr++;

		pAttr->pAttrName = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
/* #line 404 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 13: goto st8;
		case 32: goto st8;
		case 34: goto st9;
		case 39: goto st103;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st8;
	goto tr0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	switch( (*p) ) {
		case 34: goto tr23;
		case 38: goto tr24;
		case 60: goto tr0;
	}
	goto tr22;
tr22:
/* #line 81 "EcrioCPMParseXML.rl" */
	{
		tag_start = p;
	}
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
/* #line 434 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 34: goto tr26;
		case 38: goto st96;
		case 60: goto tr0;
	}
	goto st10;
tr23:
/* #line 81 "EcrioCPMParseXML.rl" */
	{
		tag_start = p;
	}
/* #line 158 "EcrioCPMParseXML.rl" */
	{
		pAttr->pAttrValue = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st11;
tr26:
/* #line 158 "EcrioCPMParseXML.rl" */
	{
		pAttr->pAttrValue = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
/* #line 461 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 13: goto st5;
		case 32: goto st5;
		case 47: goto st12;
		case 62: goto st25;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st5;
	goto tr0;
tr9:
/* #line 92 "EcrioCPMParseXML.rl" */
	{
		if (bIsValue == Enum_TRUE)
		{
			bIsValue = Enum_FALSE;
		}

		pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLStruct), (void **)&pStruct);
		if (NULL == pStruct)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;

			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto END;
		}
		pal_MemorySet((void*)pStruct, 0, sizeof(EcrioCPMParsedXMLStruct));
		pStruct->parent = (void*)pCurr;
		pCurr = pStruct;

		pStruct->pElementName = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
/* #line 499 "EcrioCPMParseXML.c" */
	if ( (*p) == 62 )
		goto st458;
	goto tr0;
st458:
	if ( ++p == pe )
		goto _test_eof458;
case 458:
	switch( (*p) ) {
		case 13: goto tr495;
		case 32: goto tr495;
		case 60: goto tr497;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto tr495;
	goto st0;
tr495:
/* #line 163 "EcrioCPMParseXML.rl" */
	{
		if (pCurr->parent == ppParsedXml)
		{
			*ppParsedXml = pCurr;
		}
		else
		{
			pStruct = pCurr;
			pCurr = (EcrioCPMParsedXMLStruct*)pCurr->parent;

			if (pCurr->uNumOfChild == 0)
			{
				pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLStruct*), (void **)&pCurr->ppChild);
			}
			else
			{
				/* Check arithmetic overflow */
				if (pal_UtilityArithmeticOverflowDetected(pCurr->uNumOfChild, 1) == Enum_TRUE)
				{
					uCPMError = ECRIO_CPM_MEMORY_ERROR;
					goto END;
				}

				/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
				if (pal_UtilityDataOverflowDetected((pCurr->uNumOfChild + 1), sizeof(EcrioCPMParsedXMLStruct*)) == Enum_TRUE)
				{
					uCPMError = ECRIO_CPM_MEMORY_ERROR;
					goto END;
				}
				pal_MemoryReallocate((u_int32)((pCurr->uNumOfChild + 1) * sizeof(EcrioCPMParsedXMLStruct*)), (void **)&pCurr->ppChild);
			}
			if (NULL == pCurr->ppChild)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;

				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
					__FUNCTION__, __LINE__, uCPMError);
				goto END;
			}
			pCurr->ppChild[pCurr->uNumOfChild] = pStruct;
			pCurr->uNumOfChild++;
		}
	}
	goto st459;
st459:
	if ( ++p == pe )
		goto _test_eof459;
case 459:
/* #line 552 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 13: goto st459;
		case 32: goto st459;
		case 60: goto st13;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st459;
	goto st0;
tr497:
/* #line 163 "EcrioCPMParseXML.rl" */
	{
		if (pCurr->parent == ppParsedXml)
		{
			*ppParsedXml = pCurr;
		}
		else
		{
			pStruct = pCurr;
			pCurr = (EcrioCPMParsedXMLStruct*)pCurr->parent;

			if (pCurr->uNumOfChild == 0)
			{
				pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLStruct*), (void **)&pCurr->ppChild);
			}
			else
			{
				/* Check arithmetic overflow */
				if (pal_UtilityArithmeticOverflowDetected(pCurr->uNumOfChild, 1) == Enum_TRUE)
				{
					uCPMError = ECRIO_CPM_MEMORY_ERROR;
					goto END;
				}

				/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
				if (pal_UtilityDataOverflowDetected((pCurr->uNumOfChild + 1), sizeof(EcrioCPMParsedXMLStruct*)) == Enum_TRUE)
				{
					uCPMError = ECRIO_CPM_MEMORY_ERROR;
					goto END;
				}
				pal_MemoryReallocate((u_int32)((pCurr->uNumOfChild + 1) * sizeof(EcrioCPMParsedXMLStruct*)), (void **)&pCurr->ppChild);
			}
			if (NULL == pCurr->ppChild)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;

				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
					__FUNCTION__, __LINE__, uCPMError);
				goto END;
			}
			pCurr->ppChild[pCurr->uNumOfChild] = pStruct;
			pCurr->uNumOfChild++;
		}
	}
	goto st13;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
/* #line 598 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 33: goto st14;
		case 63: goto st19;
	}
	goto tr0;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	if ( (*p) == 45 )
		goto st15;
	goto tr0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	if ( (*p) == 45 )
		goto st16;
	goto tr0;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	if ( (*p) == 45 )
		goto st17;
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st16;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	if ( (*p) == 45 )
		goto st18;
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st16;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
	if ( (*p) == 62 )
		goto st459;
	goto tr0;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
	switch( (*p) ) {
		case 58: goto st20;
		case 88: goto st23;
		case 95: goto st20;
		case 120: goto st23;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st20;
		} else if ( (*p) >= -64 )
			goto st20;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st20;
		} else if ( (*p) >= 65 )
			goto st20;
	} else
		goto st20;
	goto tr0;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st21;
		case 32: goto st21;
		case 47: goto tr0;
		case 63: goto st18;
		case 88: goto st23;
		case 96: goto tr0;
		case 120: goto st23;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st21;
	goto st20;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
	if ( (*p) == 63 )
		goto st22;
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st21;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
	switch( (*p) ) {
		case 62: goto st459;
		case 63: goto st22;
	}
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st21;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st21;
		case 32: goto st21;
		case 47: goto tr0;
		case 63: goto st18;
		case 77: goto st24;
		case 88: goto st23;
		case 96: goto tr0;
		case 109: goto st24;
		case 120: goto st23;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st21;
	goto st20;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st21;
		case 32: goto st21;
		case 47: goto tr0;
		case 63: goto st18;
		case 76: goto tr0;
		case 88: goto st23;
		case 96: goto tr0;
		case 108: goto tr0;
		case 120: goto st23;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st21;
	goto st20;
tr10:
/* #line 92 "EcrioCPMParseXML.rl" */
	{
		if (bIsValue == Enum_TRUE)
		{
			bIsValue = Enum_FALSE;
		}

		pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLStruct), (void **)&pStruct);
		if (NULL == pStruct)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;

			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto END;
		}
		pal_MemorySet((void*)pStruct, 0, sizeof(EcrioCPMParsedXMLStruct));
		pStruct->parent = (void*)pCurr;
		pCurr = pStruct;

		pStruct->pElementName = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st25;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
/* #line 861 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 38: goto tr42;
		case 60: goto tr43;
	}
	goto tr41;
tr41:
/* #line 86 "EcrioCPMParseXML.rl" */
	{
		value_start = p;
		bIsValue = Enum_TRUE;
	}
	goto st26;
tr77:
/* #line 163 "EcrioCPMParseXML.rl" */
	{
		if (pCurr->parent == ppParsedXml)
		{
			*ppParsedXml = pCurr;
		}
		else
		{
			pStruct = pCurr;
			pCurr = (EcrioCPMParsedXMLStruct*)pCurr->parent;

			if (pCurr->uNumOfChild == 0)
			{
				pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLStruct*), (void **)&pCurr->ppChild);
			}
			else
			{
				/* Check arithmetic overflow */
				if (pal_UtilityArithmeticOverflowDetected(pCurr->uNumOfChild, 1) == Enum_TRUE)
				{
					uCPMError = ECRIO_CPM_MEMORY_ERROR;
					goto END;
				}

				/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
				if (pal_UtilityDataOverflowDetected((pCurr->uNumOfChild + 1), sizeof(EcrioCPMParsedXMLStruct*)) == Enum_TRUE)
				{
					uCPMError = ECRIO_CPM_MEMORY_ERROR;
					goto END;
				}
				pal_MemoryReallocate((u_int32)((pCurr->uNumOfChild + 1) * sizeof(EcrioCPMParsedXMLStruct*)), (void **)&pCurr->ppChild);
			}
			if (NULL == pCurr->ppChild)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;

				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
					__FUNCTION__, __LINE__, uCPMError);
				goto END;
			}
			pCurr->ppChild[pCurr->uNumOfChild] = pStruct;
			pCurr->uNumOfChild++;
		}
	}
	goto st26;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
/* #line 911 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 38: goto st27;
		case 60: goto st34;
	}
	goto st26;
tr42:
/* #line 86 "EcrioCPMParseXML.rl" */
	{
		value_start = p;
		bIsValue = Enum_TRUE;
	}
	goto st27;
tr78:
/* #line 163 "EcrioCPMParseXML.rl" */
	{
		if (pCurr->parent == ppParsedXml)
		{
			*ppParsedXml = pCurr;
		}
		else
		{
			pStruct = pCurr;
			pCurr = (EcrioCPMParsedXMLStruct*)pCurr->parent;

			if (pCurr->uNumOfChild == 0)
			{
				pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLStruct*), (void **)&pCurr->ppChild);
			}
			else
			{
				/* Check arithmetic overflow */
				if (pal_UtilityArithmeticOverflowDetected(pCurr->uNumOfChild, 1) == Enum_TRUE)
				{
					uCPMError = ECRIO_CPM_MEMORY_ERROR;
					goto END;
				}

				/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
				if (pal_UtilityDataOverflowDetected((pCurr->uNumOfChild + 1), sizeof(EcrioCPMParsedXMLStruct*)) == Enum_TRUE)
				{
					uCPMError = ECRIO_CPM_MEMORY_ERROR;
					goto END;
				}
				pal_MemoryReallocate((u_int32)((pCurr->uNumOfChild + 1) * sizeof(EcrioCPMParsedXMLStruct*)), (void **)&pCurr->ppChild);
			}
			if (NULL == pCurr->ppChild)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;

				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
					__FUNCTION__, __LINE__, uCPMError);
				goto END;
			}
			pCurr->ppChild[pCurr->uNumOfChild] = pStruct;
			pCurr->uNumOfChild++;
		}
	}
	goto st27;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
/* #line 961 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 35: goto st29;
		case 48: goto st31;
		case 58: goto st28;
		case 95: goto st28;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st28;
		} else if ( (*p) >= -64 )
			goto st28;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st28;
		} else if ( (*p) >= 65 )
			goto st28;
	} else
		goto st28;
	goto tr0;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 47: goto tr0;
		case 59: goto st26;
		case 96: goto tr0;
	}
	if ( (*p) < 0 ) {
		if ( (*p) > -74 ) {
			if ( -72 <= (*p) && (*p) <= -65 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 44 ) {
		if ( (*p) < 91 ) {
			if ( 60 <= (*p) && (*p) <= 64 )
				goto tr0;
		} else if ( (*p) > 94 ) {
			if ( 123 <= (*p) )
				goto tr0;
		} else
			goto tr0;
	} else
		goto tr0;
	goto st28;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st30;
	goto tr0;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
	if ( (*p) == 59 )
		goto st26;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st30;
	goto tr0;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
	if ( (*p) == 120 )
		goto st32;
	goto tr0;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st33;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st33;
	} else
		goto st33;
	goto tr0;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
	if ( (*p) == 59 )
		goto st26;
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st33;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st33;
	} else
		goto st33;
	goto tr0;
tr43:
/* #line 86 "EcrioCPMParseXML.rl" */
	{
		value_start = p;
		bIsValue = Enum_TRUE;
	}
	goto st34;
tr79:
/* #line 163 "EcrioCPMParseXML.rl" */
	{
		if (pCurr->parent == ppParsedXml)
		{
			*ppParsedXml = pCurr;
		}
		else
		{
			pStruct = pCurr;
			pCurr = (EcrioCPMParsedXMLStruct*)pCurr->parent;

			if (pCurr->uNumOfChild == 0)
			{
				pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLStruct*), (void **)&pCurr->ppChild);
			}
			else
			{
				/* Check arithmetic overflow */
				if (pal_UtilityArithmeticOverflowDetected(pCurr->uNumOfChild, 1) == Enum_TRUE)
				{
					uCPMError = ECRIO_CPM_MEMORY_ERROR;
					goto END;
				}

				/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
				if (pal_UtilityDataOverflowDetected((pCurr->uNumOfChild + 1), sizeof(EcrioCPMParsedXMLStruct*)) == Enum_TRUE)
				{
					uCPMError = ECRIO_CPM_MEMORY_ERROR;
					goto END;
				}
				pal_MemoryReallocate((u_int32)((pCurr->uNumOfChild + 1) * sizeof(EcrioCPMParsedXMLStruct*)), (void **)&pCurr->ppChild);
			}
			if (NULL == pCurr->ppChild)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;

				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
					__FUNCTION__, __LINE__, uCPMError);
				goto END;
			}
			pCurr->ppChild[pCurr->uNumOfChild] = pStruct;
			pCurr->uNumOfChild++;
		}
	}
	goto st34;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
/* #line 1107 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 33: goto st61;
		case 47: goto st75;
		case 58: goto tr53;
		case 63: goto st90;
		case 95: goto tr53;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto tr53;
		} else if ( (*p) >= -64 )
			goto tr53;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr53;
		} else if ( (*p) >= 65 )
			goto tr53;
	} else
		goto tr53;
	goto tr0;
tr53:
/* #line 81 "EcrioCPMParseXML.rl" */
	{
		tag_start = p;
	}
	goto st35;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
/* #line 1140 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto tr58;
		case 32: goto tr58;
		case 47: goto tr59;
		case 62: goto tr10;
		case 96: goto tr0;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto tr58;
	goto st35;
tr58:
/* #line 92 "EcrioCPMParseXML.rl" */
	{
		if (bIsValue == Enum_TRUE)
		{
			bIsValue = Enum_FALSE;
		}

		pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLStruct), (void **)&pStruct);
		if (NULL == pStruct)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;

			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto END;
		}
		pal_MemorySet((void*)pStruct, 0, sizeof(EcrioCPMParsedXMLStruct));
		pStruct->parent = (void*)pCurr;
		pCurr = pStruct;

		pStruct->pElementName = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st36;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
/* #line 1202 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 13: goto st36;
		case 32: goto st36;
		case 47: goto st43;
		case 58: goto tr60;
		case 62: goto st25;
		case 95: goto tr60;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto tr60;
		} else if ( (*p) >= -64 )
			goto tr60;
	} else if ( (*p) > -1 ) {
		if ( (*p) < 65 ) {
			if ( 9 <= (*p) && (*p) <= 10 )
				goto st36;
		} else if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr60;
		} else
			goto tr60;
	} else
		goto tr60;
	goto tr0;
tr60:
/* #line 81 "EcrioCPMParseXML.rl" */
	{
		tag_start = p;
	}
	goto st37;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
/* #line 1239 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto tr64;
		case 32: goto tr64;
		case 47: goto tr0;
		case 61: goto tr65;
		case 96: goto tr0;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto tr64;
	goto st37;
tr64:
/* #line 124 "EcrioCPMParseXML.rl" */
	{
		pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLAttrStruct), (void **)&pAttr);
		if (NULL == pAttr)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;

			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto END;
		}

		if (pStruct->uNumOfAttr == 0)
		{
			pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLAttrStruct*), (void **)&pStruct->ppAttr);
		}
		else
		{
			/* Check arithmetic overflow */
			if (pal_UtilityArithmeticOverflowDetected(pStruct->uNumOfAttr, 1) == Enum_TRUE)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;
				goto END;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((pStruct->uNumOfAttr + 1), sizeof(EcrioCPMParsedXMLAttrStruct*)) == Enum_TRUE)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;
				goto END;
			}
			pal_MemoryReallocate((u_int32)((pStruct->uNumOfAttr + 1) * sizeof(EcrioCPMParsedXMLAttrStruct*)), (void **)&pStruct->ppAttr);
		}
		if (NULL == pStruct->ppAttr)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;

			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto END;
		}
		pStruct->ppAttr[pStruct->uNumOfAttr] = pAttr;
		pStruct->uNumOfAttr++;

		pAttr->pAttrName = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st38;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
/* #line 1312 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 13: goto st38;
		case 32: goto st38;
		case 61: goto st39;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st38;
	goto tr0;
tr65:
/* #line 124 "EcrioCPMParseXML.rl" */
	{
		pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLAttrStruct), (void **)&pAttr);
		if (NULL == pAttr)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;

			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto END;
		}

		if (pStruct->uNumOfAttr == 0)
		{
			pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLAttrStruct*), (void **)&pStruct->ppAttr);
		}
		else
		{
			/* Check arithmetic overflow */
			if (pal_UtilityArithmeticOverflowDetected(pStruct->uNumOfAttr, 1) == Enum_TRUE)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;
				goto END;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((pStruct->uNumOfAttr + 1), sizeof(EcrioCPMParsedXMLAttrStruct*)) == Enum_TRUE)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;
				goto END;
			}
			pal_MemoryReallocate((u_int32)((pStruct->uNumOfAttr + 1) * sizeof(EcrioCPMParsedXMLAttrStruct*)), (void **)&pStruct->ppAttr);
		}
		if (NULL == pStruct->ppAttr)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;

			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto END;
		}
		pStruct->ppAttr[pStruct->uNumOfAttr] = pAttr;
		pStruct->uNumOfAttr++;

		pAttr->pAttrName = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st39;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
/* #line 1360 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 13: goto st39;
		case 32: goto st39;
		case 34: goto st40;
		case 39: goto st52;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st39;
	goto tr0;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
	switch( (*p) ) {
		case 34: goto tr71;
		case 38: goto tr72;
		case 60: goto tr0;
	}
	goto tr70;
tr70:
/* #line 81 "EcrioCPMParseXML.rl" */
	{
		tag_start = p;
	}
	goto st41;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
/* #line 1390 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 34: goto tr74;
		case 38: goto st45;
		case 60: goto tr0;
	}
	goto st41;
tr71:
/* #line 81 "EcrioCPMParseXML.rl" */
	{
		tag_start = p;
	}
/* #line 158 "EcrioCPMParseXML.rl" */
	{
		pAttr->pAttrValue = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st42;
tr74:
/* #line 158 "EcrioCPMParseXML.rl" */
	{
		pAttr->pAttrValue = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st42;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
/* #line 1417 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 13: goto st36;
		case 32: goto st36;
		case 47: goto st43;
		case 62: goto st25;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st36;
	goto tr0;
tr59:
/* #line 92 "EcrioCPMParseXML.rl" */
	{
		if (bIsValue == Enum_TRUE)
		{
			bIsValue = Enum_FALSE;
		}

		pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLStruct), (void **)&pStruct);
		if (NULL == pStruct)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;

			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto END;
		}
		pal_MemorySet((void*)pStruct, 0, sizeof(EcrioCPMParsedXMLStruct));
		pStruct->parent = (void*)pCurr;
		pCurr = pStruct;

		pStruct->pElementName = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st43;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
/* #line 1455 "EcrioCPMParseXML.c" */
	if ( (*p) == 62 )
		goto st44;
	goto tr0;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
	switch( (*p) ) {
		case 38: goto tr78;
		case 60: goto tr79;
	}
	goto tr77;
tr72:
/* #line 81 "EcrioCPMParseXML.rl" */
	{
		tag_start = p;
	}
	goto st45;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
/* #line 1478 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 35: goto st47;
		case 48: goto st49;
		case 58: goto st46;
		case 95: goto st46;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st46;
		} else if ( (*p) >= -64 )
			goto st46;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st46;
		} else if ( (*p) >= 65 )
			goto st46;
	} else
		goto st46;
	goto tr0;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 47: goto tr0;
		case 59: goto st41;
		case 96: goto tr0;
	}
	if ( (*p) < 0 ) {
		if ( (*p) > -74 ) {
			if ( -72 <= (*p) && (*p) <= -65 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 44 ) {
		if ( (*p) < 91 ) {
			if ( 60 <= (*p) && (*p) <= 64 )
				goto tr0;
		} else if ( (*p) > 94 ) {
			if ( 123 <= (*p) )
				goto tr0;
		} else
			goto tr0;
	} else
		goto tr0;
	goto st46;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st48;
	goto tr0;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
	if ( (*p) == 59 )
		goto st41;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st48;
	goto tr0;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
	if ( (*p) == 120 )
		goto st50;
	goto tr0;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st51;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st51;
	} else
		goto st51;
	goto tr0;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
	if ( (*p) == 59 )
		goto st41;
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st51;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st51;
	} else
		goto st51;
	goto tr0;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
	switch( (*p) ) {
		case 38: goto tr87;
		case 39: goto tr71;
		case 60: goto tr0;
	}
	goto tr86;
tr86:
/* #line 81 "EcrioCPMParseXML.rl" */
	{
		tag_start = p;
	}
	goto st53;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
/* #line 1600 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 38: goto st54;
		case 39: goto tr74;
		case 60: goto tr0;
	}
	goto st53;
tr87:
/* #line 81 "EcrioCPMParseXML.rl" */
	{
		tag_start = p;
	}
	goto st54;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
/* #line 1617 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 35: goto st56;
		case 48: goto st58;
		case 58: goto st55;
		case 95: goto st55;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st55;
		} else if ( (*p) >= -64 )
			goto st55;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st55;
		} else if ( (*p) >= 65 )
			goto st55;
	} else
		goto st55;
	goto tr0;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 47: goto tr0;
		case 59: goto st53;
		case 96: goto tr0;
	}
	if ( (*p) < 0 ) {
		if ( (*p) > -74 ) {
			if ( -72 <= (*p) && (*p) <= -65 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 44 ) {
		if ( (*p) < 91 ) {
			if ( 60 <= (*p) && (*p) <= 64 )
				goto tr0;
		} else if ( (*p) > 94 ) {
			if ( 123 <= (*p) )
				goto tr0;
		} else
			goto tr0;
	} else
		goto tr0;
	goto st55;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st57;
	goto tr0;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
	if ( (*p) == 59 )
		goto st53;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st57;
	goto tr0;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
	if ( (*p) == 120 )
		goto st59;
	goto tr0;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st60;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st60;
	} else
		goto st60;
	goto tr0;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
	if ( (*p) == 59 )
		goto st53;
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st60;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st60;
	} else
		goto st60;
	goto tr0;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
	switch( (*p) ) {
		case 45: goto st62;
		case 91: goto st66;
	}
	goto tr0;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
	if ( (*p) == 45 )
		goto st63;
	goto tr0;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
	if ( (*p) == 45 )
		goto st64;
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st63;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
	if ( (*p) == 45 )
		goto st65;
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st63;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
	if ( (*p) == 62 )
		goto st26;
	goto tr0;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
	if ( (*p) == 67 )
		goto st67;
	goto tr0;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
	if ( (*p) == 68 )
		goto st68;
	goto tr0;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
	if ( (*p) == 65 )
		goto st69;
	goto tr0;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
	if ( (*p) == 84 )
		goto st70;
	goto tr0;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
	if ( (*p) == 65 )
		goto st71;
	goto tr0;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
	if ( (*p) == 91 )
		goto st72;
	goto tr0;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
	if ( (*p) == 93 )
		goto st73;
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st72;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
	if ( (*p) == 93 )
		goto st74;
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st72;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
	switch( (*p) ) {
		case 62: goto st26;
		case 93: goto st74;
	}
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st72;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
	switch( (*p) ) {
		case 58: goto tr109;
		case 95: goto tr109;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto tr109;
		} else if ( (*p) >= -64 )
			goto tr109;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr109;
		} else if ( (*p) >= 65 )
			goto tr109;
	} else
		goto tr109;
	goto tr0;
tr109:
/* #line 115 "EcrioCPMParseXML.rl" */
	{
		if (bIsValue == Enum_TRUE)
		{
			pStruct->pElementValue = pal_StringCreate((u_char*)value_start, p - value_start - 2);
		}
		bIsValue = Enum_FALSE;
	}
	goto st76;
st76:
	if ( ++p == pe )
		goto _test_eof76;
case 76:
/* #line 1898 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st77;
		case 32: goto st77;
		case 47: goto tr0;
		case 62: goto st460;
		case 96: goto tr0;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st77;
	goto st76;
st77:
	if ( ++p == pe )
		goto _test_eof77;
case 77:
	switch( (*p) ) {
		case 13: goto st77;
		case 32: goto st77;
		case 62: goto st460;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st77;
	goto tr0;
st460:
	if ( ++p == pe )
		goto _test_eof460;
case 460:
	switch( (*p) ) {
		case 13: goto tr499;
		case 32: goto tr499;
		case 38: goto tr78;
		case 60: goto tr500;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto tr499;
	goto tr77;
tr499:
/* #line 163 "EcrioCPMParseXML.rl" */
	{
		if (pCurr->parent == ppParsedXml)
		{
			*ppParsedXml = pCurr;
		}
		else
		{
			pStruct = pCurr;
			pCurr = (EcrioCPMParsedXMLStruct*)pCurr->parent;

			if (pCurr->uNumOfChild == 0)
			{
				pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLStruct*), (void **)&pCurr->ppChild);
			}
			else
			{
				/* Check arithmetic overflow */
				if (pal_UtilityArithmeticOverflowDetected(pCurr->uNumOfChild, 1) == Enum_TRUE)
				{
					uCPMError = ECRIO_CPM_MEMORY_ERROR;
					goto END;
				}

				/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
				if (pal_UtilityDataOverflowDetected((pCurr->uNumOfChild + 1), sizeof(EcrioCPMParsedXMLStruct*)) == Enum_TRUE)
				{
					uCPMError = ECRIO_CPM_MEMORY_ERROR;
					goto END;
				}
				pal_MemoryReallocate((u_int32)((pCurr->uNumOfChild + 1) * sizeof(EcrioCPMParsedXMLStruct*)), (void **)&pCurr->ppChild);
			}
			if (NULL == pCurr->ppChild)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;

				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
					__FUNCTION__, __LINE__, uCPMError);
				goto END;
			}
			pCurr->ppChild[pCurr->uNumOfChild] = pStruct;
			pCurr->uNumOfChild++;
		}
	}
	goto st461;
st461:
	if ( ++p == pe )
		goto _test_eof461;
case 461:
/* #line 1994 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 13: goto st461;
		case 32: goto st461;
		case 38: goto st27;
		case 60: goto st78;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st461;
	goto st26;
tr500:
/* #line 163 "EcrioCPMParseXML.rl" */
	{
		if (pCurr->parent == ppParsedXml)
		{
			*ppParsedXml = pCurr;
		}
		else
		{
			pStruct = pCurr;
			pCurr = (EcrioCPMParsedXMLStruct*)pCurr->parent;

			if (pCurr->uNumOfChild == 0)
			{
				pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLStruct*), (void **)&pCurr->ppChild);
			}
			else
			{
				/* Check arithmetic overflow */
				if (pal_UtilityArithmeticOverflowDetected(pCurr->uNumOfChild, 1) == Enum_TRUE)
				{
					uCPMError = ECRIO_CPM_MEMORY_ERROR;
					goto END;
				}

				/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
				if (pal_UtilityDataOverflowDetected((pCurr->uNumOfChild + 1), sizeof(EcrioCPMParsedXMLStruct*)) == Enum_TRUE)
				{
					uCPMError = ECRIO_CPM_MEMORY_ERROR;
					goto END;
				}
				pal_MemoryReallocate((u_int32)((pCurr->uNumOfChild + 1) * sizeof(EcrioCPMParsedXMLStruct*)), (void **)&pCurr->ppChild);
			}
			if (NULL == pCurr->ppChild)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;

				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
					__FUNCTION__, __LINE__, uCPMError);
				goto END;
			}
			pCurr->ppChild[pCurr->uNumOfChild] = pStruct;
			pCurr->uNumOfChild++;
		}
	}
	goto st78;
st78:
	if ( ++p == pe )
		goto _test_eof78;
case 78:
/* #line 2041 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 33: goto st79;
		case 47: goto st75;
		case 58: goto tr53;
		case 63: goto st84;
		case 95: goto tr53;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto tr53;
		} else if ( (*p) >= -64 )
			goto tr53;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr53;
		} else if ( (*p) >= 65 )
			goto tr53;
	} else
		goto tr53;
	goto tr0;
st79:
	if ( ++p == pe )
		goto _test_eof79;
case 79:
	switch( (*p) ) {
		case 45: goto st80;
		case 91: goto st66;
	}
	goto tr0;
st80:
	if ( ++p == pe )
		goto _test_eof80;
case 80:
	if ( (*p) == 45 )
		goto st81;
	goto tr0;
st81:
	if ( ++p == pe )
		goto _test_eof81;
case 81:
	if ( (*p) == 45 )
		goto st82;
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st81;
st82:
	if ( ++p == pe )
		goto _test_eof82;
case 82:
	if ( (*p) == 45 )
		goto st83;
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st81;
st83:
	if ( ++p == pe )
		goto _test_eof83;
case 83:
	if ( (*p) == 62 )
		goto st461;
	goto tr0;
st84:
	if ( ++p == pe )
		goto _test_eof84;
case 84:
	switch( (*p) ) {
		case 58: goto st85;
		case 88: goto st88;
		case 95: goto st85;
		case 120: goto st88;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st85;
		} else if ( (*p) >= -64 )
			goto st85;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st85;
		} else if ( (*p) >= 65 )
			goto st85;
	} else
		goto st85;
	goto tr0;
st85:
	if ( ++p == pe )
		goto _test_eof85;
case 85:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st86;
		case 32: goto st86;
		case 47: goto tr0;
		case 63: goto st83;
		case 88: goto st88;
		case 96: goto tr0;
		case 120: goto st88;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st86;
	goto st85;
st86:
	if ( ++p == pe )
		goto _test_eof86;
case 86:
	if ( (*p) == 63 )
		goto st87;
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st86;
st87:
	if ( ++p == pe )
		goto _test_eof87;
case 87:
	switch( (*p) ) {
		case 62: goto st461;
		case 63: goto st87;
	}
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st86;
st88:
	if ( ++p == pe )
		goto _test_eof88;
case 88:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st86;
		case 32: goto st86;
		case 47: goto tr0;
		case 63: goto st83;
		case 77: goto st89;
		case 88: goto st88;
		case 96: goto tr0;
		case 109: goto st89;
		case 120: goto st88;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st86;
	goto st85;
st89:
	if ( ++p == pe )
		goto _test_eof89;
case 89:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st86;
		case 32: goto st86;
		case 47: goto tr0;
		case 63: goto st83;
		case 76: goto tr0;
		case 88: goto st88;
		case 96: goto tr0;
		case 108: goto tr0;
		case 120: goto st88;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st86;
	goto st85;
st90:
	if ( ++p == pe )
		goto _test_eof90;
case 90:
	switch( (*p) ) {
		case 58: goto st91;
		case 88: goto st94;
		case 95: goto st91;
		case 120: goto st94;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st91;
		} else if ( (*p) >= -64 )
			goto st91;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st91;
		} else if ( (*p) >= 65 )
			goto st91;
	} else
		goto st91;
	goto tr0;
st91:
	if ( ++p == pe )
		goto _test_eof91;
case 91:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st92;
		case 32: goto st92;
		case 47: goto tr0;
		case 63: goto st65;
		case 88: goto st94;
		case 96: goto tr0;
		case 120: goto st94;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st92;
	goto st91;
st92:
	if ( ++p == pe )
		goto _test_eof92;
case 92:
	if ( (*p) == 63 )
		goto st93;
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st92;
st93:
	if ( ++p == pe )
		goto _test_eof93;
case 93:
	switch( (*p) ) {
		case 62: goto st26;
		case 63: goto st93;
	}
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st92;
st94:
	if ( ++p == pe )
		goto _test_eof94;
case 94:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st92;
		case 32: goto st92;
		case 47: goto tr0;
		case 63: goto st65;
		case 77: goto st95;
		case 88: goto st94;
		case 96: goto tr0;
		case 109: goto st95;
		case 120: goto st94;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st92;
	goto st91;
st95:
	if ( ++p == pe )
		goto _test_eof95;
case 95:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st92;
		case 32: goto st92;
		case 47: goto tr0;
		case 63: goto st65;
		case 76: goto tr0;
		case 88: goto st94;
		case 96: goto tr0;
		case 108: goto tr0;
		case 120: goto st94;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st92;
	goto st91;
tr24:
/* #line 81 "EcrioCPMParseXML.rl" */
	{
		tag_start = p;
	}
	goto st96;
st96:
	if ( ++p == pe )
		goto _test_eof96;
case 96:
/* #line 2483 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 35: goto st98;
		case 48: goto st100;
		case 58: goto st97;
		case 95: goto st97;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st97;
		} else if ( (*p) >= -64 )
			goto st97;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st97;
		} else if ( (*p) >= 65 )
			goto st97;
	} else
		goto st97;
	goto tr0;
st97:
	if ( ++p == pe )
		goto _test_eof97;
case 97:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 47: goto tr0;
		case 59: goto st10;
		case 96: goto tr0;
	}
	if ( (*p) < 0 ) {
		if ( (*p) > -74 ) {
			if ( -72 <= (*p) && (*p) <= -65 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 44 ) {
		if ( (*p) < 91 ) {
			if ( 60 <= (*p) && (*p) <= 64 )
				goto tr0;
		} else if ( (*p) > 94 ) {
			if ( 123 <= (*p) )
				goto tr0;
		} else
			goto tr0;
	} else
		goto tr0;
	goto st97;
st98:
	if ( ++p == pe )
		goto _test_eof98;
case 98:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st99;
	goto tr0;
st99:
	if ( ++p == pe )
		goto _test_eof99;
case 99:
	if ( (*p) == 59 )
		goto st10;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st99;
	goto tr0;
st100:
	if ( ++p == pe )
		goto _test_eof100;
case 100:
	if ( (*p) == 120 )
		goto st101;
	goto tr0;
st101:
	if ( ++p == pe )
		goto _test_eof101;
case 101:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st102;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st102;
	} else
		goto st102;
	goto tr0;
st102:
	if ( ++p == pe )
		goto _test_eof102;
case 102:
	if ( (*p) == 59 )
		goto st10;
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st102;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st102;
	} else
		goto st102;
	goto tr0;
st103:
	if ( ++p == pe )
		goto _test_eof103;
case 103:
	switch( (*p) ) {
		case 38: goto tr137;
		case 39: goto tr23;
		case 60: goto tr0;
	}
	goto tr136;
tr136:
/* #line 81 "EcrioCPMParseXML.rl" */
	{
		tag_start = p;
	}
	goto st104;
st104:
	if ( ++p == pe )
		goto _test_eof104;
case 104:
/* #line 2605 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 38: goto st105;
		case 39: goto tr26;
		case 60: goto tr0;
	}
	goto st104;
tr137:
/* #line 81 "EcrioCPMParseXML.rl" */
	{
		tag_start = p;
	}
	goto st105;
st105:
	if ( ++p == pe )
		goto _test_eof105;
case 105:
/* #line 2622 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 35: goto st107;
		case 48: goto st109;
		case 58: goto st106;
		case 95: goto st106;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st106;
		} else if ( (*p) >= -64 )
			goto st106;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st106;
		} else if ( (*p) >= 65 )
			goto st106;
	} else
		goto st106;
	goto tr0;
st106:
	if ( ++p == pe )
		goto _test_eof106;
case 106:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 47: goto tr0;
		case 59: goto st104;
		case 96: goto tr0;
	}
	if ( (*p) < 0 ) {
		if ( (*p) > -74 ) {
			if ( -72 <= (*p) && (*p) <= -65 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 44 ) {
		if ( (*p) < 91 ) {
			if ( 60 <= (*p) && (*p) <= 64 )
				goto tr0;
		} else if ( (*p) > 94 ) {
			if ( 123 <= (*p) )
				goto tr0;
		} else
			goto tr0;
	} else
		goto tr0;
	goto st106;
st107:
	if ( ++p == pe )
		goto _test_eof107;
case 107:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st108;
	goto tr0;
st108:
	if ( ++p == pe )
		goto _test_eof108;
case 108:
	if ( (*p) == 59 )
		goto st104;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st108;
	goto tr0;
st109:
	if ( ++p == pe )
		goto _test_eof109;
case 109:
	if ( (*p) == 120 )
		goto st110;
	goto tr0;
st110:
	if ( ++p == pe )
		goto _test_eof110;
case 110:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st111;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st111;
	} else
		goto st111;
	goto tr0;
st111:
	if ( ++p == pe )
		goto _test_eof111;
case 111:
	if ( (*p) == 59 )
		goto st104;
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st111;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st111;
	} else
		goto st111;
	goto tr0;
st112:
	if ( ++p == pe )
		goto _test_eof112;
case 112:
	switch( (*p) ) {
		case 45: goto st113;
		case 68: goto st117;
	}
	goto tr0;
st113:
	if ( ++p == pe )
		goto _test_eof113;
case 113:
	if ( (*p) == 45 )
		goto st114;
	goto tr0;
st114:
	if ( ++p == pe )
		goto _test_eof114;
case 114:
	if ( (*p) == 45 )
		goto st115;
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st114;
st115:
	if ( ++p == pe )
		goto _test_eof115;
case 115:
	if ( (*p) == 45 )
		goto st116;
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st114;
st116:
	if ( ++p == pe )
		goto _test_eof116;
case 116:
	if ( (*p) == 62 )
		goto st2;
	goto tr0;
st117:
	if ( ++p == pe )
		goto _test_eof117;
case 117:
	if ( (*p) == 79 )
		goto st118;
	goto tr0;
st118:
	if ( ++p == pe )
		goto _test_eof118;
case 118:
	if ( (*p) == 67 )
		goto st119;
	goto tr0;
st119:
	if ( ++p == pe )
		goto _test_eof119;
case 119:
	if ( (*p) == 84 )
		goto st120;
	goto tr0;
st120:
	if ( ++p == pe )
		goto _test_eof120;
case 120:
	if ( (*p) == 89 )
		goto st121;
	goto tr0;
st121:
	if ( ++p == pe )
		goto _test_eof121;
case 121:
	if ( (*p) == 80 )
		goto st122;
	goto tr0;
st122:
	if ( ++p == pe )
		goto _test_eof122;
case 122:
	if ( (*p) == 69 )
		goto st123;
	goto tr0;
st123:
	if ( ++p == pe )
		goto _test_eof123;
case 123:
	switch( (*p) ) {
		case 13: goto st124;
		case 32: goto st124;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st124;
	goto tr0;
st124:
	if ( ++p == pe )
		goto _test_eof124;
case 124:
	switch( (*p) ) {
		case 13: goto st124;
		case 32: goto st124;
		case 58: goto st125;
		case 95: goto st125;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st125;
		} else if ( (*p) >= -64 )
			goto st125;
	} else if ( (*p) > -1 ) {
		if ( (*p) < 65 ) {
			if ( 9 <= (*p) && (*p) <= 10 )
				goto st124;
		} else if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st125;
		} else
			goto st125;
	} else
		goto st125;
	goto tr0;
st125:
	if ( ++p == pe )
		goto _test_eof125;
case 125:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st126;
		case 32: goto st126;
		case 47: goto tr0;
		case 62: goto st127;
		case 91: goto st152;
		case 96: goto tr0;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 92 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st126;
	goto st125;
st126:
	if ( ++p == pe )
		goto _test_eof126;
case 126:
	switch( (*p) ) {
		case 13: goto st126;
		case 32: goto st126;
		case 62: goto st127;
		case 80: goto st140;
		case 83: goto st390;
		case 91: goto st152;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st126;
	goto tr0;
st127:
	if ( ++p == pe )
		goto _test_eof127;
case 127:
	switch( (*p) ) {
		case 13: goto st127;
		case 32: goto st127;
		case 60: goto st128;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st127;
	goto tr0;
st128:
	if ( ++p == pe )
		goto _test_eof128;
case 128:
	switch( (*p) ) {
		case 33: goto st129;
		case 58: goto tr4;
		case 63: goto st134;
		case 95: goto tr4;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto tr4;
		} else if ( (*p) >= -64 )
			goto tr4;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr4;
		} else if ( (*p) >= 65 )
			goto tr4;
	} else
		goto tr4;
	goto tr0;
st129:
	if ( ++p == pe )
		goto _test_eof129;
case 129:
	if ( (*p) == 45 )
		goto st130;
	goto tr0;
st130:
	if ( ++p == pe )
		goto _test_eof130;
case 130:
	if ( (*p) == 45 )
		goto st131;
	goto tr0;
st131:
	if ( ++p == pe )
		goto _test_eof131;
case 131:
	if ( (*p) == 45 )
		goto st132;
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st131;
st132:
	if ( ++p == pe )
		goto _test_eof132;
case 132:
	if ( (*p) == 45 )
		goto st133;
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st131;
st133:
	if ( ++p == pe )
		goto _test_eof133;
case 133:
	if ( (*p) == 62 )
		goto st127;
	goto tr0;
st134:
	if ( ++p == pe )
		goto _test_eof134;
case 134:
	switch( (*p) ) {
		case 58: goto st135;
		case 88: goto st138;
		case 95: goto st135;
		case 120: goto st138;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st135;
		} else if ( (*p) >= -64 )
			goto st135;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st135;
		} else if ( (*p) >= 65 )
			goto st135;
	} else
		goto st135;
	goto tr0;
st135:
	if ( ++p == pe )
		goto _test_eof135;
case 135:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st136;
		case 32: goto st136;
		case 47: goto tr0;
		case 63: goto st133;
		case 88: goto st138;
		case 96: goto tr0;
		case 120: goto st138;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st136;
	goto st135;
st136:
	if ( ++p == pe )
		goto _test_eof136;
case 136:
	if ( (*p) == 63 )
		goto st137;
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st136;
st137:
	if ( ++p == pe )
		goto _test_eof137;
case 137:
	switch( (*p) ) {
		case 62: goto st127;
		case 63: goto st137;
	}
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st136;
st138:
	if ( ++p == pe )
		goto _test_eof138;
case 138:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st136;
		case 32: goto st136;
		case 47: goto tr0;
		case 63: goto st133;
		case 77: goto st139;
		case 88: goto st138;
		case 96: goto tr0;
		case 109: goto st139;
		case 120: goto st138;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st136;
	goto st135;
st139:
	if ( ++p == pe )
		goto _test_eof139;
case 139:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st136;
		case 32: goto st136;
		case 47: goto tr0;
		case 63: goto st133;
		case 76: goto tr0;
		case 88: goto st138;
		case 96: goto tr0;
		case 108: goto tr0;
		case 120: goto st138;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st136;
	goto st135;
st140:
	if ( ++p == pe )
		goto _test_eof140;
case 140:
	if ( (*p) == 85 )
		goto st141;
	goto tr0;
st141:
	if ( ++p == pe )
		goto _test_eof141;
case 141:
	if ( (*p) == 66 )
		goto st142;
	goto tr0;
st142:
	if ( ++p == pe )
		goto _test_eof142;
case 142:
	if ( (*p) == 76 )
		goto st143;
	goto tr0;
st143:
	if ( ++p == pe )
		goto _test_eof143;
case 143:
	if ( (*p) == 73 )
		goto st144;
	goto tr0;
st144:
	if ( ++p == pe )
		goto _test_eof144;
case 144:
	if ( (*p) == 67 )
		goto st145;
	goto tr0;
st145:
	if ( ++p == pe )
		goto _test_eof145;
case 145:
	switch( (*p) ) {
		case 13: goto st146;
		case 32: goto st146;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st146;
	goto tr0;
st146:
	if ( ++p == pe )
		goto _test_eof146;
case 146:
	switch( (*p) ) {
		case 13: goto st146;
		case 32: goto st146;
		case 34: goto st147;
		case 39: goto st389;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st146;
	goto tr0;
st147:
	if ( ++p == pe )
		goto _test_eof147;
case 147:
	switch( (*p) ) {
		case 10: goto st147;
		case 13: goto st147;
		case 34: goto st148;
		case 61: goto st147;
		case 95: goto st147;
	}
	if ( (*p) < 39 ) {
		if ( 32 <= (*p) && (*p) <= 37 )
			goto st147;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st147;
		} else if ( (*p) >= 63 )
			goto st147;
	} else
		goto st147;
	goto tr0;
st148:
	if ( ++p == pe )
		goto _test_eof148;
case 148:
	switch( (*p) ) {
		case 13: goto st149;
		case 32: goto st149;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st149;
	goto tr0;
st149:
	if ( ++p == pe )
		goto _test_eof149;
case 149:
	switch( (*p) ) {
		case 13: goto st149;
		case 32: goto st149;
		case 34: goto st150;
		case 39: goto st388;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st149;
	goto tr0;
st150:
	if ( ++p == pe )
		goto _test_eof150;
case 150:
	if ( (*p) == 34 )
		goto st151;
	goto st150;
st151:
	if ( ++p == pe )
		goto _test_eof151;
case 151:
	switch( (*p) ) {
		case 13: goto st151;
		case 32: goto st151;
		case 62: goto st127;
		case 91: goto st152;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st151;
	goto tr0;
st152:
	if ( ++p == pe )
		goto _test_eof152;
case 152:
	switch( (*p) ) {
		case 13: goto st152;
		case 32: goto st152;
		case 37: goto st153;
		case 60: goto st155;
		case 93: goto st387;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st152;
	goto tr0;
st153:
	if ( ++p == pe )
		goto _test_eof153;
case 153:
	switch( (*p) ) {
		case 58: goto st154;
		case 95: goto st154;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st154;
		} else if ( (*p) >= -64 )
			goto st154;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st154;
		} else if ( (*p) >= 65 )
			goto st154;
	} else
		goto st154;
	goto tr0;
st154:
	if ( ++p == pe )
		goto _test_eof154;
case 154:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 47: goto tr0;
		case 59: goto st152;
		case 96: goto tr0;
	}
	if ( (*p) < 0 ) {
		if ( (*p) > -74 ) {
			if ( -72 <= (*p) && (*p) <= -65 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 44 ) {
		if ( (*p) < 91 ) {
			if ( 60 <= (*p) && (*p) <= 64 )
				goto tr0;
		} else if ( (*p) > 94 ) {
			if ( 123 <= (*p) )
				goto tr0;
		} else
			goto tr0;
	} else
		goto tr0;
	goto st154;
st155:
	if ( ++p == pe )
		goto _test_eof155;
case 155:
	switch( (*p) ) {
		case 33: goto st156;
		case 63: goto st381;
	}
	goto tr0;
st156:
	if ( ++p == pe )
		goto _test_eof156;
case 156:
	switch( (*p) ) {
		case 45: goto st157;
		case 65: goto st161;
		case 69: goto st248;
		case 78: goto st359;
	}
	goto tr0;
st157:
	if ( ++p == pe )
		goto _test_eof157;
case 157:
	if ( (*p) == 45 )
		goto st158;
	goto tr0;
st158:
	if ( ++p == pe )
		goto _test_eof158;
case 158:
	if ( (*p) == 45 )
		goto st159;
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st158;
st159:
	if ( ++p == pe )
		goto _test_eof159;
case 159:
	if ( (*p) == 45 )
		goto st160;
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st158;
st160:
	if ( ++p == pe )
		goto _test_eof160;
case 160:
	if ( (*p) == 62 )
		goto st152;
	goto tr0;
st161:
	if ( ++p == pe )
		goto _test_eof161;
case 161:
	if ( (*p) == 84 )
		goto st162;
	goto tr0;
st162:
	if ( ++p == pe )
		goto _test_eof162;
case 162:
	if ( (*p) == 84 )
		goto st163;
	goto tr0;
st163:
	if ( ++p == pe )
		goto _test_eof163;
case 163:
	if ( (*p) == 76 )
		goto st164;
	goto tr0;
st164:
	if ( ++p == pe )
		goto _test_eof164;
case 164:
	if ( (*p) == 73 )
		goto st165;
	goto tr0;
st165:
	if ( ++p == pe )
		goto _test_eof165;
case 165:
	if ( (*p) == 83 )
		goto st166;
	goto tr0;
st166:
	if ( ++p == pe )
		goto _test_eof166;
case 166:
	if ( (*p) == 84 )
		goto st167;
	goto tr0;
st167:
	if ( ++p == pe )
		goto _test_eof167;
case 167:
	switch( (*p) ) {
		case 13: goto st168;
		case 32: goto st168;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st168;
	goto tr0;
st168:
	if ( ++p == pe )
		goto _test_eof168;
case 168:
	switch( (*p) ) {
		case 13: goto st168;
		case 32: goto st168;
		case 58: goto st169;
		case 95: goto st169;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st169;
		} else if ( (*p) >= -64 )
			goto st169;
	} else if ( (*p) > -1 ) {
		if ( (*p) < 65 ) {
			if ( 9 <= (*p) && (*p) <= 10 )
				goto st168;
		} else if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st169;
		} else
			goto st169;
	} else
		goto st169;
	goto tr0;
st169:
	if ( ++p == pe )
		goto _test_eof169;
case 169:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st170;
		case 32: goto st170;
		case 47: goto tr0;
		case 62: goto st152;
		case 96: goto tr0;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st170;
	goto st169;
st170:
	if ( ++p == pe )
		goto _test_eof170;
case 170:
	switch( (*p) ) {
		case 13: goto st170;
		case 32: goto st170;
		case 58: goto st171;
		case 62: goto st152;
		case 95: goto st171;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st171;
		} else if ( (*p) >= -64 )
			goto st171;
	} else if ( (*p) > -1 ) {
		if ( (*p) < 65 ) {
			if ( 9 <= (*p) && (*p) <= 10 )
				goto st170;
		} else if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st171;
		} else
			goto st171;
	} else
		goto st171;
	goto tr0;
st171:
	if ( ++p == pe )
		goto _test_eof171;
case 171:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st172;
		case 32: goto st172;
		case 47: goto tr0;
		case 96: goto tr0;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st172;
	goto st171;
st172:
	if ( ++p == pe )
		goto _test_eof172;
case 172:
	switch( (*p) ) {
		case 13: goto st172;
		case 32: goto st172;
		case 40: goto st173;
		case 67: goto st215;
		case 69: goto st219;
		case 73: goto st226;
		case 78: goto st231;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st172;
	goto tr0;
st173:
	if ( ++p == pe )
		goto _test_eof173;
case 173:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st173;
		case 32: goto st173;
		case 47: goto tr0;
		case 96: goto tr0;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st173;
	goto st174;
st174:
	if ( ++p == pe )
		goto _test_eof174;
case 174:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st175;
		case 32: goto st175;
		case 41: goto st176;
		case 47: goto tr0;
		case 96: goto tr0;
		case 124: goto st173;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st175;
	goto st174;
st175:
	if ( ++p == pe )
		goto _test_eof175;
case 175:
	switch( (*p) ) {
		case 13: goto st175;
		case 32: goto st175;
		case 41: goto st176;
		case 124: goto st173;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st175;
	goto tr0;
st176:
	if ( ++p == pe )
		goto _test_eof176;
case 176:
	switch( (*p) ) {
		case 13: goto st177;
		case 32: goto st177;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st177;
	goto tr0;
st177:
	if ( ++p == pe )
		goto _test_eof177;
case 177:
	switch( (*p) ) {
		case 13: goto st177;
		case 32: goto st177;
		case 34: goto st178;
		case 35: goto st188;
		case 39: goto st195;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st177;
	goto tr0;
st178:
	if ( ++p == pe )
		goto _test_eof178;
case 178:
	switch( (*p) ) {
		case 34: goto tr226;
		case 38: goto tr227;
		case 60: goto tr0;
	}
	goto tr225;
tr225:
/* #line 81 "EcrioCPMParseXML.rl" */
	{
		tag_start = p;
	}
	goto st179;
st179:
	if ( ++p == pe )
		goto _test_eof179;
case 179:
/* #line 3764 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 34: goto tr229;
		case 38: goto st181;
		case 60: goto tr0;
	}
	goto st179;
tr226:
/* #line 81 "EcrioCPMParseXML.rl" */
	{
		tag_start = p;
	}
/* #line 158 "EcrioCPMParseXML.rl" */
	{
		pAttr->pAttrValue = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st180;
tr229:
/* #line 158 "EcrioCPMParseXML.rl" */
	{
		pAttr->pAttrValue = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st180;
st180:
	if ( ++p == pe )
		goto _test_eof180;
case 180:
/* #line 3791 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 13: goto st170;
		case 32: goto st170;
		case 62: goto st152;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st170;
	goto tr0;
tr227:
/* #line 81 "EcrioCPMParseXML.rl" */
	{
		tag_start = p;
	}
	goto st181;
st181:
	if ( ++p == pe )
		goto _test_eof181;
case 181:
/* #line 3810 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 35: goto st183;
		case 48: goto st185;
		case 58: goto st182;
		case 95: goto st182;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st182;
		} else if ( (*p) >= -64 )
			goto st182;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st182;
		} else if ( (*p) >= 65 )
			goto st182;
	} else
		goto st182;
	goto tr0;
st182:
	if ( ++p == pe )
		goto _test_eof182;
case 182:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 47: goto tr0;
		case 59: goto st179;
		case 96: goto tr0;
	}
	if ( (*p) < 0 ) {
		if ( (*p) > -74 ) {
			if ( -72 <= (*p) && (*p) <= -65 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 44 ) {
		if ( (*p) < 91 ) {
			if ( 60 <= (*p) && (*p) <= 64 )
				goto tr0;
		} else if ( (*p) > 94 ) {
			if ( 123 <= (*p) )
				goto tr0;
		} else
			goto tr0;
	} else
		goto tr0;
	goto st182;
st183:
	if ( ++p == pe )
		goto _test_eof183;
case 183:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st184;
	goto tr0;
st184:
	if ( ++p == pe )
		goto _test_eof184;
case 184:
	if ( (*p) == 59 )
		goto st179;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st184;
	goto tr0;
st185:
	if ( ++p == pe )
		goto _test_eof185;
case 185:
	if ( (*p) == 120 )
		goto st186;
	goto tr0;
st186:
	if ( ++p == pe )
		goto _test_eof186;
case 186:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st187;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st187;
	} else
		goto st187;
	goto tr0;
st187:
	if ( ++p == pe )
		goto _test_eof187;
case 187:
	if ( (*p) == 59 )
		goto st179;
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st187;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st187;
	} else
		goto st187;
	goto tr0;
st188:
	if ( ++p == pe )
		goto _test_eof188;
case 188:
	switch( (*p) ) {
		case 70: goto st189;
		case 73: goto st204;
		case 82: goto st210;
	}
	goto tr0;
st189:
	if ( ++p == pe )
		goto _test_eof189;
case 189:
	if ( (*p) == 73 )
		goto st190;
	goto tr0;
st190:
	if ( ++p == pe )
		goto _test_eof190;
case 190:
	if ( (*p) == 88 )
		goto st191;
	goto tr0;
st191:
	if ( ++p == pe )
		goto _test_eof191;
case 191:
	if ( (*p) == 69 )
		goto st192;
	goto tr0;
st192:
	if ( ++p == pe )
		goto _test_eof192;
case 192:
	if ( (*p) == 68 )
		goto st193;
	goto tr0;
st193:
	if ( ++p == pe )
		goto _test_eof193;
case 193:
	switch( (*p) ) {
		case 13: goto st194;
		case 32: goto st194;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st194;
	goto tr0;
st194:
	if ( ++p == pe )
		goto _test_eof194;
case 194:
	switch( (*p) ) {
		case 13: goto st194;
		case 32: goto st194;
		case 34: goto st178;
		case 39: goto st195;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st194;
	goto tr0;
st195:
	if ( ++p == pe )
		goto _test_eof195;
case 195:
	switch( (*p) ) {
		case 38: goto tr246;
		case 39: goto tr226;
		case 60: goto tr0;
	}
	goto tr245;
tr245:
/* #line 81 "EcrioCPMParseXML.rl" */
	{
		tag_start = p;
	}
	goto st196;
st196:
	if ( ++p == pe )
		goto _test_eof196;
case 196:
/* #line 3994 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 38: goto st197;
		case 39: goto tr229;
		case 60: goto tr0;
	}
	goto st196;
tr246:
/* #line 81 "EcrioCPMParseXML.rl" */
	{
		tag_start = p;
	}
	goto st197;
st197:
	if ( ++p == pe )
		goto _test_eof197;
case 197:
/* #line 4011 "EcrioCPMParseXML.c" */
	switch( (*p) ) {
		case 35: goto st199;
		case 48: goto st201;
		case 58: goto st198;
		case 95: goto st198;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st198;
		} else if ( (*p) >= -64 )
			goto st198;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st198;
		} else if ( (*p) >= 65 )
			goto st198;
	} else
		goto st198;
	goto tr0;
st198:
	if ( ++p == pe )
		goto _test_eof198;
case 198:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 47: goto tr0;
		case 59: goto st196;
		case 96: goto tr0;
	}
	if ( (*p) < 0 ) {
		if ( (*p) > -74 ) {
			if ( -72 <= (*p) && (*p) <= -65 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 44 ) {
		if ( (*p) < 91 ) {
			if ( 60 <= (*p) && (*p) <= 64 )
				goto tr0;
		} else if ( (*p) > 94 ) {
			if ( 123 <= (*p) )
				goto tr0;
		} else
			goto tr0;
	} else
		goto tr0;
	goto st198;
st199:
	if ( ++p == pe )
		goto _test_eof199;
case 199:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st200;
	goto tr0;
st200:
	if ( ++p == pe )
		goto _test_eof200;
case 200:
	if ( (*p) == 59 )
		goto st196;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st200;
	goto tr0;
st201:
	if ( ++p == pe )
		goto _test_eof201;
case 201:
	if ( (*p) == 120 )
		goto st202;
	goto tr0;
st202:
	if ( ++p == pe )
		goto _test_eof202;
case 202:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st203;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st203;
	} else
		goto st203;
	goto tr0;
st203:
	if ( ++p == pe )
		goto _test_eof203;
case 203:
	if ( (*p) == 59 )
		goto st196;
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st203;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st203;
	} else
		goto st203;
	goto tr0;
st204:
	if ( ++p == pe )
		goto _test_eof204;
case 204:
	if ( (*p) == 77 )
		goto st205;
	goto tr0;
st205:
	if ( ++p == pe )
		goto _test_eof205;
case 205:
	if ( (*p) == 80 )
		goto st206;
	goto tr0;
st206:
	if ( ++p == pe )
		goto _test_eof206;
case 206:
	if ( (*p) == 76 )
		goto st207;
	goto tr0;
st207:
	if ( ++p == pe )
		goto _test_eof207;
case 207:
	if ( (*p) == 73 )
		goto st208;
	goto tr0;
st208:
	if ( ++p == pe )
		goto _test_eof208;
case 208:
	if ( (*p) == 69 )
		goto st209;
	goto tr0;
st209:
	if ( ++p == pe )
		goto _test_eof209;
case 209:
	if ( (*p) == 68 )
		goto st180;
	goto tr0;
st210:
	if ( ++p == pe )
		goto _test_eof210;
case 210:
	if ( (*p) == 69 )
		goto st211;
	goto tr0;
st211:
	if ( ++p == pe )
		goto _test_eof211;
case 211:
	if ( (*p) == 81 )
		goto st212;
	goto tr0;
st212:
	if ( ++p == pe )
		goto _test_eof212;
case 212:
	if ( (*p) == 85 )
		goto st213;
	goto tr0;
st213:
	if ( ++p == pe )
		goto _test_eof213;
case 213:
	if ( (*p) == 73 )
		goto st214;
	goto tr0;
st214:
	if ( ++p == pe )
		goto _test_eof214;
case 214:
	if ( (*p) == 82 )
		goto st208;
	goto tr0;
st215:
	if ( ++p == pe )
		goto _test_eof215;
case 215:
	if ( (*p) == 68 )
		goto st216;
	goto tr0;
st216:
	if ( ++p == pe )
		goto _test_eof216;
case 216:
	if ( (*p) == 65 )
		goto st217;
	goto tr0;
st217:
	if ( ++p == pe )
		goto _test_eof217;
case 217:
	if ( (*p) == 84 )
		goto st218;
	goto tr0;
st218:
	if ( ++p == pe )
		goto _test_eof218;
case 218:
	if ( (*p) == 65 )
		goto st176;
	goto tr0;
st219:
	if ( ++p == pe )
		goto _test_eof219;
case 219:
	if ( (*p) == 78 )
		goto st220;
	goto tr0;
st220:
	if ( ++p == pe )
		goto _test_eof220;
case 220:
	if ( (*p) == 84 )
		goto st221;
	goto tr0;
st221:
	if ( ++p == pe )
		goto _test_eof221;
case 221:
	if ( (*p) == 73 )
		goto st222;
	goto tr0;
st222:
	if ( ++p == pe )
		goto _test_eof222;
case 222:
	if ( (*p) == 84 )
		goto st223;
	goto tr0;
st223:
	if ( ++p == pe )
		goto _test_eof223;
case 223:
	switch( (*p) ) {
		case 73: goto st224;
		case 89: goto st176;
	}
	goto tr0;
st224:
	if ( ++p == pe )
		goto _test_eof224;
case 224:
	if ( (*p) == 69 )
		goto st225;
	goto tr0;
st225:
	if ( ++p == pe )
		goto _test_eof225;
case 225:
	if ( (*p) == 83 )
		goto st176;
	goto tr0;
st226:
	if ( ++p == pe )
		goto _test_eof226;
case 226:
	if ( (*p) == 68 )
		goto st227;
	goto tr0;
st227:
	if ( ++p == pe )
		goto _test_eof227;
case 227:
	switch( (*p) ) {
		case 13: goto st177;
		case 32: goto st177;
		case 82: goto st228;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st177;
	goto tr0;
st228:
	if ( ++p == pe )
		goto _test_eof228;
case 228:
	if ( (*p) == 69 )
		goto st229;
	goto tr0;
st229:
	if ( ++p == pe )
		goto _test_eof229;
case 229:
	if ( (*p) == 70 )
		goto st230;
	goto tr0;
st230:
	if ( ++p == pe )
		goto _test_eof230;
case 230:
	switch( (*p) ) {
		case 13: goto st177;
		case 32: goto st177;
		case 83: goto st176;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st177;
	goto tr0;
st231:
	if ( ++p == pe )
		goto _test_eof231;
case 231:
	switch( (*p) ) {
		case 77: goto st232;
		case 79: goto st237;
	}
	goto tr0;
st232:
	if ( ++p == pe )
		goto _test_eof232;
case 232:
	if ( (*p) == 84 )
		goto st233;
	goto tr0;
st233:
	if ( ++p == pe )
		goto _test_eof233;
case 233:
	if ( (*p) == 79 )
		goto st234;
	goto tr0;
st234:
	if ( ++p == pe )
		goto _test_eof234;
case 234:
	if ( (*p) == 75 )
		goto st235;
	goto tr0;
st235:
	if ( ++p == pe )
		goto _test_eof235;
case 235:
	if ( (*p) == 69 )
		goto st236;
	goto tr0;
st236:
	if ( ++p == pe )
		goto _test_eof236;
case 236:
	if ( (*p) == 78 )
		goto st230;
	goto tr0;
st237:
	if ( ++p == pe )
		goto _test_eof237;
case 237:
	if ( (*p) == 84 )
		goto st238;
	goto tr0;
st238:
	if ( ++p == pe )
		goto _test_eof238;
case 238:
	if ( (*p) == 65 )
		goto st239;
	goto tr0;
st239:
	if ( ++p == pe )
		goto _test_eof239;
case 239:
	if ( (*p) == 84 )
		goto st240;
	goto tr0;
st240:
	if ( ++p == pe )
		goto _test_eof240;
case 240:
	if ( (*p) == 73 )
		goto st241;
	goto tr0;
st241:
	if ( ++p == pe )
		goto _test_eof241;
case 241:
	if ( (*p) == 79 )
		goto st242;
	goto tr0;
st242:
	if ( ++p == pe )
		goto _test_eof242;
case 242:
	if ( (*p) == 78 )
		goto st243;
	goto tr0;
st243:
	if ( ++p == pe )
		goto _test_eof243;
case 243:
	switch( (*p) ) {
		case 13: goto st244;
		case 32: goto st244;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st244;
	goto tr0;
st244:
	if ( ++p == pe )
		goto _test_eof244;
case 244:
	switch( (*p) ) {
		case 13: goto st244;
		case 32: goto st244;
		case 40: goto st245;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st244;
	goto tr0;
st245:
	if ( ++p == pe )
		goto _test_eof245;
case 245:
	switch( (*p) ) {
		case 13: goto st245;
		case 32: goto st245;
		case 58: goto st246;
		case 95: goto st246;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st246;
		} else if ( (*p) >= -64 )
			goto st246;
	} else if ( (*p) > -1 ) {
		if ( (*p) < 65 ) {
			if ( 9 <= (*p) && (*p) <= 10 )
				goto st245;
		} else if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st246;
		} else
			goto st246;
	} else
		goto st246;
	goto tr0;
st246:
	if ( ++p == pe )
		goto _test_eof246;
case 246:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st247;
		case 32: goto st247;
		case 41: goto st176;
		case 47: goto tr0;
		case 96: goto tr0;
		case 124: goto st245;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st247;
	goto st246;
st247:
	if ( ++p == pe )
		goto _test_eof247;
case 247:
	switch( (*p) ) {
		case 13: goto st247;
		case 32: goto st247;
		case 41: goto st176;
		case 124: goto st245;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st247;
	goto tr0;
st248:
	if ( ++p == pe )
		goto _test_eof248;
case 248:
	switch( (*p) ) {
		case 76: goto st249;
		case 78: goto st284;
	}
	goto tr0;
st249:
	if ( ++p == pe )
		goto _test_eof249;
case 249:
	if ( (*p) == 69 )
		goto st250;
	goto tr0;
st250:
	if ( ++p == pe )
		goto _test_eof250;
case 250:
	if ( (*p) == 77 )
		goto st251;
	goto tr0;
st251:
	if ( ++p == pe )
		goto _test_eof251;
case 251:
	if ( (*p) == 69 )
		goto st252;
	goto tr0;
st252:
	if ( ++p == pe )
		goto _test_eof252;
case 252:
	if ( (*p) == 78 )
		goto st253;
	goto tr0;
st253:
	if ( ++p == pe )
		goto _test_eof253;
case 253:
	if ( (*p) == 84 )
		goto st254;
	goto tr0;
st254:
	if ( ++p == pe )
		goto _test_eof254;
case 254:
	switch( (*p) ) {
		case 13: goto st255;
		case 32: goto st255;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st255;
	goto tr0;
st255:
	if ( ++p == pe )
		goto _test_eof255;
case 255:
	switch( (*p) ) {
		case 13: goto st255;
		case 32: goto st255;
		case 58: goto st256;
		case 95: goto st256;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st256;
		} else if ( (*p) >= -64 )
			goto st256;
	} else if ( (*p) > -1 ) {
		if ( (*p) < 65 ) {
			if ( 9 <= (*p) && (*p) <= 10 )
				goto st255;
		} else if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st256;
		} else
			goto st256;
	} else
		goto st256;
	goto tr0;
st256:
	if ( ++p == pe )
		goto _test_eof256;
case 256:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st257;
		case 32: goto st257;
		case 47: goto tr0;
		case 96: goto tr0;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st257;
	goto st256;
st257:
	if ( ++p == pe )
		goto _test_eof257;
case 257:
	switch( (*p) ) {
		case 13: goto st257;
		case 32: goto st257;
		case 40: goto st258;
		case 65: goto st279;
		case 69: goto st281;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st257;
	goto tr0;
st258:
	if ( ++p == pe )
		goto _test_eof258;
case 258:
	switch( (*p) ) {
		case 13: goto st263;
		case 32: goto st263;
		case 35: goto st264;
		case 40: goto st262;
		case 41: goto st278;
		case 44: goto st262;
		case 58: goto st259;
		case 95: goto st259;
		case 124: goto st262;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st259;
		} else if ( (*p) >= -64 )
			goto st259;
	} else if ( (*p) > -1 ) {
		if ( (*p) < 65 ) {
			if ( 9 <= (*p) && (*p) <= 10 )
				goto st263;
		} else if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st259;
		} else
			goto st259;
	} else
		goto st259;
	goto tr0;
st259:
	if ( ++p == pe )
		goto _test_eof259;
case 259:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 41: goto st260;
		case 44: goto tr0;
		case 47: goto tr0;
		case 63: goto st262;
		case 96: goto tr0;
	}
	if ( (*p) < 42 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 40 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 43 ) {
		if ( (*p) < 91 ) {
			if ( 59 <= (*p) && (*p) <= 64 )
				goto tr0;
		} else if ( (*p) > 94 ) {
			if ( 123 <= (*p) )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st262;
	goto st259;
st260:
	if ( ++p == pe )
		goto _test_eof260;
case 260:
	switch( (*p) ) {
		case 13: goto st261;
		case 32: goto st261;
		case 62: goto st152;
		case 63: goto st261;
	}
	if ( (*p) > 10 ) {
		if ( 42 <= (*p) && (*p) <= 43 )
			goto st261;
	} else if ( (*p) >= 9 )
		goto st261;
	goto tr0;
st261:
	if ( ++p == pe )
		goto _test_eof261;
case 261:
	switch( (*p) ) {
		case 13: goto st261;
		case 32: goto st261;
		case 62: goto st152;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st261;
	goto tr0;
st262:
	if ( ++p == pe )
		goto _test_eof262;
case 262:
	if ( (*p) == 41 )
		goto st260;
	goto tr0;
st263:
	if ( ++p == pe )
		goto _test_eof263;
case 263:
	switch( (*p) ) {
		case 13: goto st263;
		case 32: goto st263;
		case 35: goto st264;
		case 40: goto st262;
		case 41: goto st276;
		case 44: goto st262;
		case 58: goto st259;
		case 95: goto st259;
		case 124: goto st262;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st259;
		} else if ( (*p) >= -64 )
			goto st259;
	} else if ( (*p) > -1 ) {
		if ( (*p) < 65 ) {
			if ( 9 <= (*p) && (*p) <= 10 )
				goto st263;
		} else if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st259;
		} else
			goto st259;
	} else
		goto st259;
	goto tr0;
st264:
	if ( ++p == pe )
		goto _test_eof264;
case 264:
	if ( (*p) == 80 )
		goto st265;
	goto tr0;
st265:
	if ( ++p == pe )
		goto _test_eof265;
case 265:
	if ( (*p) == 67 )
		goto st266;
	goto tr0;
st266:
	if ( ++p == pe )
		goto _test_eof266;
case 266:
	if ( (*p) == 68 )
		goto st267;
	goto tr0;
st267:
	if ( ++p == pe )
		goto _test_eof267;
case 267:
	if ( (*p) == 65 )
		goto st268;
	goto tr0;
st268:
	if ( ++p == pe )
		goto _test_eof268;
case 268:
	if ( (*p) == 84 )
		goto st269;
	goto tr0;
st269:
	if ( ++p == pe )
		goto _test_eof269;
case 269:
	if ( (*p) == 65 )
		goto st270;
	goto tr0;
st270:
	if ( ++p == pe )
		goto _test_eof270;
case 270:
	switch( (*p) ) {
		case 13: goto st270;
		case 32: goto st270;
		case 41: goto st271;
		case 124: goto st272;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st270;
	goto tr0;
st271:
	if ( ++p == pe )
		goto _test_eof271;
case 271:
	switch( (*p) ) {
		case 13: goto st261;
		case 32: goto st261;
		case 42: goto st261;
		case 62: goto st152;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st261;
	goto tr0;
st272:
	if ( ++p == pe )
		goto _test_eof272;
case 272:
	switch( (*p) ) {
		case 13: goto st272;
		case 32: goto st272;
		case 58: goto st273;
		case 95: goto st273;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st273;
		} else if ( (*p) >= -64 )
			goto st273;
	} else if ( (*p) > -1 ) {
		if ( (*p) < 65 ) {
			if ( 9 <= (*p) && (*p) <= 10 )
				goto st272;
		} else if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st273;
		} else
			goto st273;
	} else
		goto st273;
	goto tr0;
st273:
	if ( ++p == pe )
		goto _test_eof273;
case 273:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st274;
		case 32: goto st274;
		case 41: goto st275;
		case 47: goto tr0;
		case 96: goto tr0;
		case 124: goto st272;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st274;
	goto st273;
st274:
	if ( ++p == pe )
		goto _test_eof274;
case 274:
	switch( (*p) ) {
		case 13: goto st274;
		case 32: goto st274;
		case 41: goto st275;
		case 124: goto st272;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st274;
	goto tr0;
st275:
	if ( ++p == pe )
		goto _test_eof275;
case 275:
	if ( (*p) == 42 )
		goto st261;
	goto tr0;
st276:
	if ( ++p == pe )
		goto _test_eof276;
case 276:
	switch( (*p) ) {
		case 13: goto st261;
		case 32: goto st261;
		case 41: goto st260;
		case 62: goto st152;
		case 63: goto st277;
	}
	if ( (*p) > 10 ) {
		if ( 42 <= (*p) && (*p) <= 43 )
			goto st277;
	} else if ( (*p) >= 9 )
		goto st261;
	goto tr0;
st277:
	if ( ++p == pe )
		goto _test_eof277;
case 277:
	switch( (*p) ) {
		case 13: goto st261;
		case 32: goto st261;
		case 41: goto st260;
		case 62: goto st152;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st261;
	goto tr0;
st278:
	if ( ++p == pe )
		goto _test_eof278;
case 278:
	switch( (*p) ) {
		case 41: goto st260;
		case 63: goto st262;
	}
	if ( 42 <= (*p) && (*p) <= 43 )
		goto st262;
	goto tr0;
st279:
	if ( ++p == pe )
		goto _test_eof279;
case 279:
	if ( (*p) == 78 )
		goto st280;
	goto tr0;
st280:
	if ( ++p == pe )
		goto _test_eof280;
case 280:
	if ( (*p) == 89 )
		goto st261;
	goto tr0;
st281:
	if ( ++p == pe )
		goto _test_eof281;
case 281:
	if ( (*p) == 77 )
		goto st282;
	goto tr0;
st282:
	if ( ++p == pe )
		goto _test_eof282;
case 282:
	if ( (*p) == 80 )
		goto st283;
	goto tr0;
st283:
	if ( ++p == pe )
		goto _test_eof283;
case 283:
	if ( (*p) == 84 )
		goto st280;
	goto tr0;
st284:
	if ( ++p == pe )
		goto _test_eof284;
case 284:
	if ( (*p) == 84 )
		goto st285;
	goto tr0;
st285:
	if ( ++p == pe )
		goto _test_eof285;
case 285:
	if ( (*p) == 73 )
		goto st286;
	goto tr0;
st286:
	if ( ++p == pe )
		goto _test_eof286;
case 286:
	if ( (*p) == 84 )
		goto st287;
	goto tr0;
st287:
	if ( ++p == pe )
		goto _test_eof287;
case 287:
	if ( (*p) == 89 )
		goto st288;
	goto tr0;
st288:
	if ( ++p == pe )
		goto _test_eof288;
case 288:
	switch( (*p) ) {
		case 13: goto st289;
		case 32: goto st289;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st289;
	goto tr0;
st289:
	if ( ++p == pe )
		goto _test_eof289;
case 289:
	switch( (*p) ) {
		case 13: goto st289;
		case 32: goto st289;
		case 37: goto st337;
		case 58: goto st290;
		case 95: goto st290;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st290;
		} else if ( (*p) >= -64 )
			goto st290;
	} else if ( (*p) > -1 ) {
		if ( (*p) < 65 ) {
			if ( 9 <= (*p) && (*p) <= 10 )
				goto st289;
		} else if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st290;
		} else
			goto st290;
	} else
		goto st290;
	goto tr0;
st290:
	if ( ++p == pe )
		goto _test_eof290;
case 290:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st291;
		case 32: goto st291;
		case 47: goto tr0;
		case 96: goto tr0;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st291;
	goto st290;
st291:
	if ( ++p == pe )
		goto _test_eof291;
case 291:
	switch( (*p) ) {
		case 13: goto st291;
		case 32: goto st291;
		case 34: goto st292;
		case 39: goto st301;
		case 80: goto st310;
		case 83: goto st332;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st291;
	goto tr0;
st292:
	if ( ++p == pe )
		goto _test_eof292;
case 292:
	switch( (*p) ) {
		case 34: goto st261;
		case 37: goto st293;
		case 38: goto st295;
	}
	goto st292;
st293:
	if ( ++p == pe )
		goto _test_eof293;
case 293:
	switch( (*p) ) {
		case 58: goto st294;
		case 95: goto st294;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st294;
		} else if ( (*p) >= -64 )
			goto st294;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st294;
		} else if ( (*p) >= 65 )
			goto st294;
	} else
		goto st294;
	goto tr0;
st294:
	if ( ++p == pe )
		goto _test_eof294;
case 294:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 47: goto tr0;
		case 59: goto st292;
		case 96: goto tr0;
	}
	if ( (*p) < 0 ) {
		if ( (*p) > -74 ) {
			if ( -72 <= (*p) && (*p) <= -65 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 44 ) {
		if ( (*p) < 91 ) {
			if ( 60 <= (*p) && (*p) <= 64 )
				goto tr0;
		} else if ( (*p) > 94 ) {
			if ( 123 <= (*p) )
				goto tr0;
		} else
			goto tr0;
	} else
		goto tr0;
	goto st294;
st295:
	if ( ++p == pe )
		goto _test_eof295;
case 295:
	switch( (*p) ) {
		case 35: goto st296;
		case 48: goto st298;
		case 58: goto st294;
		case 95: goto st294;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st294;
		} else if ( (*p) >= -64 )
			goto st294;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st294;
		} else if ( (*p) >= 65 )
			goto st294;
	} else
		goto st294;
	goto tr0;
st296:
	if ( ++p == pe )
		goto _test_eof296;
case 296:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st297;
	goto tr0;
st297:
	if ( ++p == pe )
		goto _test_eof297;
case 297:
	if ( (*p) == 59 )
		goto st292;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st297;
	goto tr0;
st298:
	if ( ++p == pe )
		goto _test_eof298;
case 298:
	if ( (*p) == 120 )
		goto st299;
	goto tr0;
st299:
	if ( ++p == pe )
		goto _test_eof299;
case 299:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st300;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st300;
	} else
		goto st300;
	goto tr0;
st300:
	if ( ++p == pe )
		goto _test_eof300;
case 300:
	if ( (*p) == 59 )
		goto st292;
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st300;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st300;
	} else
		goto st300;
	goto tr0;
st301:
	if ( ++p == pe )
		goto _test_eof301;
case 301:
	switch( (*p) ) {
		case 37: goto st302;
		case 38: goto st304;
		case 39: goto st261;
	}
	goto st301;
st302:
	if ( ++p == pe )
		goto _test_eof302;
case 302:
	switch( (*p) ) {
		case 58: goto st303;
		case 95: goto st303;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st303;
		} else if ( (*p) >= -64 )
			goto st303;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st303;
		} else if ( (*p) >= 65 )
			goto st303;
	} else
		goto st303;
	goto tr0;
st303:
	if ( ++p == pe )
		goto _test_eof303;
case 303:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 47: goto tr0;
		case 59: goto st301;
		case 96: goto tr0;
	}
	if ( (*p) < 0 ) {
		if ( (*p) > -74 ) {
			if ( -72 <= (*p) && (*p) <= -65 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 44 ) {
		if ( (*p) < 91 ) {
			if ( 60 <= (*p) && (*p) <= 64 )
				goto tr0;
		} else if ( (*p) > 94 ) {
			if ( 123 <= (*p) )
				goto tr0;
		} else
			goto tr0;
	} else
		goto tr0;
	goto st303;
st304:
	if ( ++p == pe )
		goto _test_eof304;
case 304:
	switch( (*p) ) {
		case 35: goto st305;
		case 48: goto st307;
		case 58: goto st303;
		case 95: goto st303;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st303;
		} else if ( (*p) >= -64 )
			goto st303;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st303;
		} else if ( (*p) >= 65 )
			goto st303;
	} else
		goto st303;
	goto tr0;
st305:
	if ( ++p == pe )
		goto _test_eof305;
case 305:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st306;
	goto tr0;
st306:
	if ( ++p == pe )
		goto _test_eof306;
case 306:
	if ( (*p) == 59 )
		goto st301;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st306;
	goto tr0;
st307:
	if ( ++p == pe )
		goto _test_eof307;
case 307:
	if ( (*p) == 120 )
		goto st308;
	goto tr0;
st308:
	if ( ++p == pe )
		goto _test_eof308;
case 308:
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st309;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st309;
	} else
		goto st309;
	goto tr0;
st309:
	if ( ++p == pe )
		goto _test_eof309;
case 309:
	if ( (*p) == 59 )
		goto st301;
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st309;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st309;
	} else
		goto st309;
	goto tr0;
st310:
	if ( ++p == pe )
		goto _test_eof310;
case 310:
	if ( (*p) == 85 )
		goto st311;
	goto tr0;
st311:
	if ( ++p == pe )
		goto _test_eof311;
case 311:
	if ( (*p) == 66 )
		goto st312;
	goto tr0;
st312:
	if ( ++p == pe )
		goto _test_eof312;
case 312:
	if ( (*p) == 76 )
		goto st313;
	goto tr0;
st313:
	if ( ++p == pe )
		goto _test_eof313;
case 313:
	if ( (*p) == 73 )
		goto st314;
	goto tr0;
st314:
	if ( ++p == pe )
		goto _test_eof314;
case 314:
	if ( (*p) == 67 )
		goto st315;
	goto tr0;
st315:
	if ( ++p == pe )
		goto _test_eof315;
case 315:
	switch( (*p) ) {
		case 13: goto st316;
		case 32: goto st316;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st316;
	goto tr0;
st316:
	if ( ++p == pe )
		goto _test_eof316;
case 316:
	switch( (*p) ) {
		case 13: goto st316;
		case 32: goto st316;
		case 34: goto st317;
		case 39: goto st331;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st316;
	goto tr0;
st317:
	if ( ++p == pe )
		goto _test_eof317;
case 317:
	switch( (*p) ) {
		case 10: goto st317;
		case 13: goto st317;
		case 34: goto st318;
		case 61: goto st317;
		case 95: goto st317;
	}
	if ( (*p) < 39 ) {
		if ( 32 <= (*p) && (*p) <= 37 )
			goto st317;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st317;
		} else if ( (*p) >= 63 )
			goto st317;
	} else
		goto st317;
	goto tr0;
st318:
	if ( ++p == pe )
		goto _test_eof318;
case 318:
	switch( (*p) ) {
		case 13: goto st319;
		case 32: goto st319;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st319;
	goto tr0;
st319:
	if ( ++p == pe )
		goto _test_eof319;
case 319:
	switch( (*p) ) {
		case 13: goto st319;
		case 32: goto st319;
		case 34: goto st320;
		case 39: goto st330;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st319;
	goto tr0;
st320:
	if ( ++p == pe )
		goto _test_eof320;
case 320:
	if ( (*p) == 34 )
		goto st321;
	goto st320;
st321:
	if ( ++p == pe )
		goto _test_eof321;
case 321:
	switch( (*p) ) {
		case 13: goto st322;
		case 32: goto st322;
		case 62: goto st152;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st322;
	goto tr0;
st322:
	if ( ++p == pe )
		goto _test_eof322;
case 322:
	switch( (*p) ) {
		case 13: goto st322;
		case 32: goto st322;
		case 62: goto st152;
		case 78: goto st323;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st322;
	goto tr0;
st323:
	if ( ++p == pe )
		goto _test_eof323;
case 323:
	if ( (*p) == 68 )
		goto st324;
	goto tr0;
st324:
	if ( ++p == pe )
		goto _test_eof324;
case 324:
	if ( (*p) == 65 )
		goto st325;
	goto tr0;
st325:
	if ( ++p == pe )
		goto _test_eof325;
case 325:
	if ( (*p) == 84 )
		goto st326;
	goto tr0;
st326:
	if ( ++p == pe )
		goto _test_eof326;
case 326:
	if ( (*p) == 65 )
		goto st327;
	goto tr0;
st327:
	if ( ++p == pe )
		goto _test_eof327;
case 327:
	switch( (*p) ) {
		case 13: goto st328;
		case 32: goto st328;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st328;
	goto tr0;
st328:
	if ( ++p == pe )
		goto _test_eof328;
case 328:
	switch( (*p) ) {
		case 13: goto st328;
		case 32: goto st328;
		case 58: goto st329;
		case 95: goto st329;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st329;
		} else if ( (*p) >= -64 )
			goto st329;
	} else if ( (*p) > -1 ) {
		if ( (*p) < 65 ) {
			if ( 9 <= (*p) && (*p) <= 10 )
				goto st328;
		} else if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st329;
		} else
			goto st329;
	} else
		goto st329;
	goto tr0;
st329:
	if ( ++p == pe )
		goto _test_eof329;
case 329:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st261;
		case 32: goto st261;
		case 47: goto tr0;
		case 62: goto st152;
		case 96: goto tr0;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st261;
	goto st329;
st330:
	if ( ++p == pe )
		goto _test_eof330;
case 330:
	if ( (*p) == 39 )
		goto st321;
	goto st330;
st331:
	if ( ++p == pe )
		goto _test_eof331;
case 331:
	switch( (*p) ) {
		case 10: goto st331;
		case 13: goto st331;
		case 39: goto st318;
		case 61: goto st331;
		case 95: goto st331;
	}
	if ( (*p) < 40 ) {
		if ( (*p) > 33 ) {
			if ( 35 <= (*p) && (*p) <= 37 )
				goto st331;
		} else if ( (*p) >= 32 )
			goto st331;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st331;
		} else if ( (*p) >= 63 )
			goto st331;
	} else
		goto st331;
	goto tr0;
st332:
	if ( ++p == pe )
		goto _test_eof332;
case 332:
	if ( (*p) == 89 )
		goto st333;
	goto tr0;
st333:
	if ( ++p == pe )
		goto _test_eof333;
case 333:
	if ( (*p) == 83 )
		goto st334;
	goto tr0;
st334:
	if ( ++p == pe )
		goto _test_eof334;
case 334:
	if ( (*p) == 84 )
		goto st335;
	goto tr0;
st335:
	if ( ++p == pe )
		goto _test_eof335;
case 335:
	if ( (*p) == 69 )
		goto st336;
	goto tr0;
st336:
	if ( ++p == pe )
		goto _test_eof336;
case 336:
	if ( (*p) == 77 )
		goto st318;
	goto tr0;
st337:
	if ( ++p == pe )
		goto _test_eof337;
case 337:
	switch( (*p) ) {
		case 13: goto st338;
		case 32: goto st338;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st338;
	goto tr0;
st338:
	if ( ++p == pe )
		goto _test_eof338;
case 338:
	switch( (*p) ) {
		case 13: goto st338;
		case 32: goto st338;
		case 58: goto st339;
		case 95: goto st339;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st339;
		} else if ( (*p) >= -64 )
			goto st339;
	} else if ( (*p) > -1 ) {
		if ( (*p) < 65 ) {
			if ( 9 <= (*p) && (*p) <= 10 )
				goto st338;
		} else if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st339;
		} else
			goto st339;
	} else
		goto st339;
	goto tr0;
st339:
	if ( ++p == pe )
		goto _test_eof339;
case 339:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st340;
		case 32: goto st340;
		case 47: goto tr0;
		case 96: goto tr0;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st340;
	goto st339;
st340:
	if ( ++p == pe )
		goto _test_eof340;
case 340:
	switch( (*p) ) {
		case 13: goto st340;
		case 32: goto st340;
		case 34: goto st292;
		case 39: goto st301;
		case 80: goto st341;
		case 83: goto st354;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st340;
	goto tr0;
st341:
	if ( ++p == pe )
		goto _test_eof341;
case 341:
	if ( (*p) == 85 )
		goto st342;
	goto tr0;
st342:
	if ( ++p == pe )
		goto _test_eof342;
case 342:
	if ( (*p) == 66 )
		goto st343;
	goto tr0;
st343:
	if ( ++p == pe )
		goto _test_eof343;
case 343:
	if ( (*p) == 76 )
		goto st344;
	goto tr0;
st344:
	if ( ++p == pe )
		goto _test_eof344;
case 344:
	if ( (*p) == 73 )
		goto st345;
	goto tr0;
st345:
	if ( ++p == pe )
		goto _test_eof345;
case 345:
	if ( (*p) == 67 )
		goto st346;
	goto tr0;
st346:
	if ( ++p == pe )
		goto _test_eof346;
case 346:
	switch( (*p) ) {
		case 13: goto st347;
		case 32: goto st347;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st347;
	goto tr0;
st347:
	if ( ++p == pe )
		goto _test_eof347;
case 347:
	switch( (*p) ) {
		case 13: goto st347;
		case 32: goto st347;
		case 34: goto st348;
		case 39: goto st353;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st347;
	goto tr0;
st348:
	if ( ++p == pe )
		goto _test_eof348;
case 348:
	switch( (*p) ) {
		case 10: goto st348;
		case 13: goto st348;
		case 34: goto st349;
		case 61: goto st348;
		case 95: goto st348;
	}
	if ( (*p) < 39 ) {
		if ( 32 <= (*p) && (*p) <= 37 )
			goto st348;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st348;
		} else if ( (*p) >= 63 )
			goto st348;
	} else
		goto st348;
	goto tr0;
st349:
	if ( ++p == pe )
		goto _test_eof349;
case 349:
	switch( (*p) ) {
		case 13: goto st350;
		case 32: goto st350;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st350;
	goto tr0;
st350:
	if ( ++p == pe )
		goto _test_eof350;
case 350:
	switch( (*p) ) {
		case 13: goto st350;
		case 32: goto st350;
		case 34: goto st351;
		case 39: goto st352;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st350;
	goto tr0;
st351:
	if ( ++p == pe )
		goto _test_eof351;
case 351:
	if ( (*p) == 34 )
		goto st261;
	goto st351;
st352:
	if ( ++p == pe )
		goto _test_eof352;
case 352:
	if ( (*p) == 39 )
		goto st261;
	goto st352;
st353:
	if ( ++p == pe )
		goto _test_eof353;
case 353:
	switch( (*p) ) {
		case 10: goto st353;
		case 13: goto st353;
		case 39: goto st349;
		case 61: goto st353;
		case 95: goto st353;
	}
	if ( (*p) < 40 ) {
		if ( (*p) > 33 ) {
			if ( 35 <= (*p) && (*p) <= 37 )
				goto st353;
		} else if ( (*p) >= 32 )
			goto st353;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st353;
		} else if ( (*p) >= 63 )
			goto st353;
	} else
		goto st353;
	goto tr0;
st354:
	if ( ++p == pe )
		goto _test_eof354;
case 354:
	if ( (*p) == 89 )
		goto st355;
	goto tr0;
st355:
	if ( ++p == pe )
		goto _test_eof355;
case 355:
	if ( (*p) == 83 )
		goto st356;
	goto tr0;
st356:
	if ( ++p == pe )
		goto _test_eof356;
case 356:
	if ( (*p) == 84 )
		goto st357;
	goto tr0;
st357:
	if ( ++p == pe )
		goto _test_eof357;
case 357:
	if ( (*p) == 69 )
		goto st358;
	goto tr0;
st358:
	if ( ++p == pe )
		goto _test_eof358;
case 358:
	if ( (*p) == 77 )
		goto st349;
	goto tr0;
st359:
	if ( ++p == pe )
		goto _test_eof359;
case 359:
	if ( (*p) == 79 )
		goto st360;
	goto tr0;
st360:
	if ( ++p == pe )
		goto _test_eof360;
case 360:
	if ( (*p) == 84 )
		goto st361;
	goto tr0;
st361:
	if ( ++p == pe )
		goto _test_eof361;
case 361:
	if ( (*p) == 65 )
		goto st362;
	goto tr0;
st362:
	if ( ++p == pe )
		goto _test_eof362;
case 362:
	if ( (*p) == 84 )
		goto st363;
	goto tr0;
st363:
	if ( ++p == pe )
		goto _test_eof363;
case 363:
	if ( (*p) == 73 )
		goto st364;
	goto tr0;
st364:
	if ( ++p == pe )
		goto _test_eof364;
case 364:
	if ( (*p) == 79 )
		goto st365;
	goto tr0;
st365:
	if ( ++p == pe )
		goto _test_eof365;
case 365:
	if ( (*p) == 78 )
		goto st366;
	goto tr0;
st366:
	if ( ++p == pe )
		goto _test_eof366;
case 366:
	switch( (*p) ) {
		case 13: goto st367;
		case 32: goto st367;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st367;
	goto tr0;
st367:
	if ( ++p == pe )
		goto _test_eof367;
case 367:
	switch( (*p) ) {
		case 13: goto st367;
		case 32: goto st367;
		case 58: goto st368;
		case 95: goto st368;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st368;
		} else if ( (*p) >= -64 )
			goto st368;
	} else if ( (*p) > -1 ) {
		if ( (*p) < 65 ) {
			if ( 9 <= (*p) && (*p) <= 10 )
				goto st367;
		} else if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st368;
		} else
			goto st368;
	} else
		goto st368;
	goto tr0;
st368:
	if ( ++p == pe )
		goto _test_eof368;
case 368:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st369;
		case 32: goto st369;
		case 47: goto tr0;
		case 96: goto tr0;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st369;
	goto st368;
st369:
	if ( ++p == pe )
		goto _test_eof369;
case 369:
	switch( (*p) ) {
		case 13: goto st369;
		case 32: goto st369;
		case 80: goto st370;
		case 83: goto st354;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st369;
	goto tr0;
st370:
	if ( ++p == pe )
		goto _test_eof370;
case 370:
	if ( (*p) == 85 )
		goto st371;
	goto tr0;
st371:
	if ( ++p == pe )
		goto _test_eof371;
case 371:
	if ( (*p) == 66 )
		goto st372;
	goto tr0;
st372:
	if ( ++p == pe )
		goto _test_eof372;
case 372:
	if ( (*p) == 76 )
		goto st373;
	goto tr0;
st373:
	if ( ++p == pe )
		goto _test_eof373;
case 373:
	if ( (*p) == 73 )
		goto st374;
	goto tr0;
st374:
	if ( ++p == pe )
		goto _test_eof374;
case 374:
	if ( (*p) == 67 )
		goto st375;
	goto tr0;
st375:
	if ( ++p == pe )
		goto _test_eof375;
case 375:
	switch( (*p) ) {
		case 13: goto st376;
		case 32: goto st376;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st376;
	goto tr0;
st376:
	if ( ++p == pe )
		goto _test_eof376;
case 376:
	switch( (*p) ) {
		case 13: goto st376;
		case 32: goto st376;
		case 34: goto st377;
		case 39: goto st380;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st376;
	goto tr0;
st377:
	if ( ++p == pe )
		goto _test_eof377;
case 377:
	switch( (*p) ) {
		case 10: goto st377;
		case 13: goto st377;
		case 34: goto st378;
		case 61: goto st377;
		case 95: goto st377;
	}
	if ( (*p) < 39 ) {
		if ( 32 <= (*p) && (*p) <= 37 )
			goto st377;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st377;
		} else if ( (*p) >= 63 )
			goto st377;
	} else
		goto st377;
	goto tr0;
st378:
	if ( ++p == pe )
		goto _test_eof378;
case 378:
	switch( (*p) ) {
		case 13: goto st379;
		case 32: goto st379;
		case 62: goto st152;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st379;
	goto tr0;
st379:
	if ( ++p == pe )
		goto _test_eof379;
case 379:
	switch( (*p) ) {
		case 13: goto st379;
		case 32: goto st379;
		case 34: goto st351;
		case 39: goto st352;
		case 62: goto st152;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st379;
	goto tr0;
st380:
	if ( ++p == pe )
		goto _test_eof380;
case 380:
	switch( (*p) ) {
		case 10: goto st380;
		case 13: goto st380;
		case 39: goto st378;
		case 61: goto st380;
		case 95: goto st380;
	}
	if ( (*p) < 40 ) {
		if ( (*p) > 33 ) {
			if ( 35 <= (*p) && (*p) <= 37 )
				goto st380;
		} else if ( (*p) >= 32 )
			goto st380;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st380;
		} else if ( (*p) >= 63 )
			goto st380;
	} else
		goto st380;
	goto tr0;
st381:
	if ( ++p == pe )
		goto _test_eof381;
case 381:
	switch( (*p) ) {
		case 58: goto st382;
		case 88: goto st385;
		case 95: goto st382;
		case 120: goto st385;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st382;
		} else if ( (*p) >= -64 )
			goto st382;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st382;
		} else if ( (*p) >= 65 )
			goto st382;
	} else
		goto st382;
	goto tr0;
st382:
	if ( ++p == pe )
		goto _test_eof382;
case 382:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st383;
		case 32: goto st383;
		case 47: goto tr0;
		case 63: goto st160;
		case 88: goto st385;
		case 96: goto tr0;
		case 120: goto st385;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st383;
	goto st382;
st383:
	if ( ++p == pe )
		goto _test_eof383;
case 383:
	if ( (*p) == 63 )
		goto st384;
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st383;
st384:
	if ( ++p == pe )
		goto _test_eof384;
case 384:
	switch( (*p) ) {
		case 62: goto st152;
		case 63: goto st384;
	}
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st383;
st385:
	if ( ++p == pe )
		goto _test_eof385;
case 385:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st383;
		case 32: goto st383;
		case 47: goto tr0;
		case 63: goto st160;
		case 77: goto st386;
		case 88: goto st385;
		case 96: goto tr0;
		case 109: goto st386;
		case 120: goto st385;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st383;
	goto st382;
st386:
	if ( ++p == pe )
		goto _test_eof386;
case 386:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st383;
		case 32: goto st383;
		case 47: goto tr0;
		case 63: goto st160;
		case 76: goto tr0;
		case 88: goto st385;
		case 96: goto tr0;
		case 108: goto tr0;
		case 120: goto st385;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st383;
	goto st382;
st387:
	if ( ++p == pe )
		goto _test_eof387;
case 387:
	switch( (*p) ) {
		case 13: goto st387;
		case 32: goto st387;
		case 62: goto st127;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st387;
	goto tr0;
st388:
	if ( ++p == pe )
		goto _test_eof388;
case 388:
	if ( (*p) == 39 )
		goto st151;
	goto st388;
st389:
	if ( ++p == pe )
		goto _test_eof389;
case 389:
	switch( (*p) ) {
		case 10: goto st389;
		case 13: goto st389;
		case 39: goto st148;
		case 61: goto st389;
		case 95: goto st389;
	}
	if ( (*p) < 40 ) {
		if ( (*p) > 33 ) {
			if ( 35 <= (*p) && (*p) <= 37 )
				goto st389;
		} else if ( (*p) >= 32 )
			goto st389;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st389;
		} else if ( (*p) >= 63 )
			goto st389;
	} else
		goto st389;
	goto tr0;
st390:
	if ( ++p == pe )
		goto _test_eof390;
case 390:
	if ( (*p) == 89 )
		goto st391;
	goto tr0;
st391:
	if ( ++p == pe )
		goto _test_eof391;
case 391:
	if ( (*p) == 83 )
		goto st392;
	goto tr0;
st392:
	if ( ++p == pe )
		goto _test_eof392;
case 392:
	if ( (*p) == 84 )
		goto st393;
	goto tr0;
st393:
	if ( ++p == pe )
		goto _test_eof393;
case 393:
	if ( (*p) == 69 )
		goto st394;
	goto tr0;
st394:
	if ( ++p == pe )
		goto _test_eof394;
case 394:
	if ( (*p) == 77 )
		goto st148;
	goto tr0;
st395:
	if ( ++p == pe )
		goto _test_eof395;
case 395:
	switch( (*p) ) {
		case 58: goto st396;
		case 88: goto st399;
		case 95: goto st396;
		case 120: goto st399;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st396;
		} else if ( (*p) >= -64 )
			goto st396;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st396;
		} else if ( (*p) >= 65 )
			goto st396;
	} else
		goto st396;
	goto tr0;
st396:
	if ( ++p == pe )
		goto _test_eof396;
case 396:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st397;
		case 32: goto st397;
		case 47: goto tr0;
		case 63: goto st116;
		case 88: goto st399;
		case 96: goto tr0;
		case 120: goto st399;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st397;
	goto st396;
st397:
	if ( ++p == pe )
		goto _test_eof397;
case 397:
	if ( (*p) == 63 )
		goto st398;
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st397;
st398:
	if ( ++p == pe )
		goto _test_eof398;
case 398:
	switch( (*p) ) {
		case 62: goto st2;
		case 63: goto st398;
	}
	if ( (*p) < 11 ) {
		if ( 0 <= (*p) && (*p) <= 8 )
			goto tr0;
	} else if ( (*p) > 12 ) {
		if ( 14 <= (*p) && (*p) <= 31 )
			goto tr0;
	} else
		goto tr0;
	goto st397;
st399:
	if ( ++p == pe )
		goto _test_eof399;
case 399:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st397;
		case 32: goto st397;
		case 47: goto tr0;
		case 63: goto st116;
		case 77: goto st400;
		case 88: goto st399;
		case 96: goto tr0;
		case 109: goto st400;
		case 120: goto st399;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st397;
	goto st396;
st400:
	if ( ++p == pe )
		goto _test_eof400;
case 400:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st397;
		case 32: goto st397;
		case 47: goto tr0;
		case 63: goto st116;
		case 76: goto tr0;
		case 88: goto st399;
		case 96: goto tr0;
		case 108: goto tr0;
		case 120: goto st399;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st397;
	goto st396;
st401:
	if ( ++p == pe )
		goto _test_eof401;
case 401:
	switch( (*p) ) {
		case 33: goto st112;
		case 58: goto tr4;
		case 63: goto st402;
		case 95: goto tr4;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto tr4;
		} else if ( (*p) >= -64 )
			goto tr4;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr4;
		} else if ( (*p) >= 65 )
			goto tr4;
	} else
		goto tr4;
	goto tr0;
st402:
	if ( ++p == pe )
		goto _test_eof402;
case 402:
	switch( (*p) ) {
		case 58: goto st396;
		case 88: goto st399;
		case 95: goto st396;
		case 120: goto st403;
	}
	if ( (*p) < -8 ) {
		if ( (*p) > -42 ) {
			if ( -40 <= (*p) && (*p) <= -10 )
				goto st396;
		} else if ( (*p) >= -64 )
			goto st396;
	} else if ( (*p) > -1 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st396;
		} else if ( (*p) >= 65 )
			goto st396;
	} else
		goto st396;
	goto tr0;
st403:
	if ( ++p == pe )
		goto _test_eof403;
case 403:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st397;
		case 32: goto st397;
		case 47: goto tr0;
		case 63: goto st116;
		case 77: goto st400;
		case 88: goto st399;
		case 96: goto tr0;
		case 109: goto st404;
		case 120: goto st399;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st397;
	goto st396;
st404:
	if ( ++p == pe )
		goto _test_eof404;
case 404:
	switch( (*p) ) {
		case -41: goto tr0;
		case -9: goto tr0;
		case 13: goto st397;
		case 32: goto st397;
		case 47: goto tr0;
		case 63: goto st116;
		case 76: goto tr0;
		case 88: goto st399;
		case 96: goto tr0;
		case 108: goto st405;
		case 120: goto st399;
	}
	if ( (*p) < 9 ) {
		if ( (*p) < -72 ) {
			if ( (*p) <= -74 )
				goto tr0;
		} else if ( (*p) > -65 ) {
			if ( 0 <= (*p) && (*p) <= 8 )
				goto tr0;
		} else
			goto tr0;
	} else if ( (*p) > 10 ) {
		if ( (*p) < 59 ) {
			if ( 11 <= (*p) && (*p) <= 44 )
				goto tr0;
		} else if ( (*p) > 64 ) {
			if ( (*p) > 94 ) {
				if ( 123 <= (*p) )
					goto tr0;
			} else if ( (*p) >= 91 )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st397;
	goto st396;
st405:
	if ( ++p == pe )
		goto _test_eof405;
case 405:
	switch( (*p) ) {
		case 13: goto st406;
		case 32: goto st406;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st406;
	goto tr0;
st406:
	if ( ++p == pe )
		goto _test_eof406;
case 406:
	switch( (*p) ) {
		case 13: goto st406;
		case 32: goto st406;
		case 118: goto st407;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st406;
	goto tr0;
st407:
	if ( ++p == pe )
		goto _test_eof407;
case 407:
	if ( (*p) == 101 )
		goto st408;
	goto tr0;
st408:
	if ( ++p == pe )
		goto _test_eof408;
case 408:
	if ( (*p) == 114 )
		goto st409;
	goto tr0;
st409:
	if ( ++p == pe )
		goto _test_eof409;
case 409:
	if ( (*p) == 115 )
		goto st410;
	goto tr0;
st410:
	if ( ++p == pe )
		goto _test_eof410;
case 410:
	if ( (*p) == 105 )
		goto st411;
	goto tr0;
st411:
	if ( ++p == pe )
		goto _test_eof411;
case 411:
	if ( (*p) == 111 )
		goto st412;
	goto tr0;
st412:
	if ( ++p == pe )
		goto _test_eof412;
case 412:
	if ( (*p) == 110 )
		goto st413;
	goto tr0;
st413:
	if ( ++p == pe )
		goto _test_eof413;
case 413:
	switch( (*p) ) {
		case 13: goto st413;
		case 32: goto st413;
		case 61: goto st414;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st413;
	goto tr0;
st414:
	if ( ++p == pe )
		goto _test_eof414;
case 414:
	switch( (*p) ) {
		case 13: goto st414;
		case 32: goto st414;
		case 34: goto st415;
		case 39: goto st456;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st414;
	goto tr0;
st415:
	if ( ++p == pe )
		goto _test_eof415;
case 415:
	if ( (*p) == 95 )
		goto st416;
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st416;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st416;
		} else if ( (*p) >= 65 )
			goto st416;
	} else
		goto st416;
	goto tr0;
st416:
	if ( ++p == pe )
		goto _test_eof416;
case 416:
	switch( (*p) ) {
		case 34: goto st417;
		case 95: goto st416;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st416;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st416;
		} else if ( (*p) >= 65 )
			goto st416;
	} else
		goto st416;
	goto tr0;
st417:
	if ( ++p == pe )
		goto _test_eof417;
case 417:
	switch( (*p) ) {
		case 13: goto st418;
		case 32: goto st418;
		case 63: goto st116;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st418;
	goto tr0;
st418:
	if ( ++p == pe )
		goto _test_eof418;
case 418:
	switch( (*p) ) {
		case 13: goto st418;
		case 32: goto st418;
		case 63: goto st116;
		case 101: goto st419;
		case 115: goto st432;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st418;
	goto tr0;
st419:
	if ( ++p == pe )
		goto _test_eof419;
case 419:
	if ( (*p) == 110 )
		goto st420;
	goto tr0;
st420:
	if ( ++p == pe )
		goto _test_eof420;
case 420:
	if ( (*p) == 99 )
		goto st421;
	goto tr0;
st421:
	if ( ++p == pe )
		goto _test_eof421;
case 421:
	if ( (*p) == 111 )
		goto st422;
	goto tr0;
st422:
	if ( ++p == pe )
		goto _test_eof422;
case 422:
	if ( (*p) == 100 )
		goto st423;
	goto tr0;
st423:
	if ( ++p == pe )
		goto _test_eof423;
case 423:
	if ( (*p) == 105 )
		goto st424;
	goto tr0;
st424:
	if ( ++p == pe )
		goto _test_eof424;
case 424:
	if ( (*p) == 110 )
		goto st425;
	goto tr0;
st425:
	if ( ++p == pe )
		goto _test_eof425;
case 425:
	if ( (*p) == 103 )
		goto st426;
	goto tr0;
st426:
	if ( ++p == pe )
		goto _test_eof426;
case 426:
	switch( (*p) ) {
		case 13: goto st426;
		case 32: goto st426;
		case 61: goto st427;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st426;
	goto tr0;
st427:
	if ( ++p == pe )
		goto _test_eof427;
case 427:
	switch( (*p) ) {
		case 13: goto st427;
		case 32: goto st427;
		case 34: goto st428;
		case 39: goto st454;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st427;
	goto tr0;
st428:
	if ( ++p == pe )
		goto _test_eof428;
case 428:
	if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st429;
	} else if ( (*p) >= 65 )
		goto st429;
	goto tr0;
st429:
	if ( ++p == pe )
		goto _test_eof429;
case 429:
	switch( (*p) ) {
		case 34: goto st430;
		case 95: goto st429;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st429;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st429;
		} else if ( (*p) >= 65 )
			goto st429;
	} else
		goto st429;
	goto tr0;
st430:
	if ( ++p == pe )
		goto _test_eof430;
case 430:
	switch( (*p) ) {
		case 13: goto st431;
		case 32: goto st431;
		case 63: goto st116;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st431;
	goto tr0;
st431:
	if ( ++p == pe )
		goto _test_eof431;
case 431:
	switch( (*p) ) {
		case 13: goto st431;
		case 32: goto st431;
		case 63: goto st116;
		case 115: goto st432;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st431;
	goto tr0;
st432:
	if ( ++p == pe )
		goto _test_eof432;
case 432:
	if ( (*p) == 116 )
		goto st433;
	goto tr0;
st433:
	if ( ++p == pe )
		goto _test_eof433;
case 433:
	if ( (*p) == 97 )
		goto st434;
	goto tr0;
st434:
	if ( ++p == pe )
		goto _test_eof434;
case 434:
	if ( (*p) == 110 )
		goto st435;
	goto tr0;
st435:
	if ( ++p == pe )
		goto _test_eof435;
case 435:
	if ( (*p) == 100 )
		goto st436;
	goto tr0;
st436:
	if ( ++p == pe )
		goto _test_eof436;
case 436:
	if ( (*p) == 97 )
		goto st437;
	goto tr0;
st437:
	if ( ++p == pe )
		goto _test_eof437;
case 437:
	if ( (*p) == 108 )
		goto st438;
	goto tr0;
st438:
	if ( ++p == pe )
		goto _test_eof438;
case 438:
	if ( (*p) == 111 )
		goto st439;
	goto tr0;
st439:
	if ( ++p == pe )
		goto _test_eof439;
case 439:
	if ( (*p) == 110 )
		goto st440;
	goto tr0;
st440:
	if ( ++p == pe )
		goto _test_eof440;
case 440:
	if ( (*p) == 101 )
		goto st441;
	goto tr0;
st441:
	if ( ++p == pe )
		goto _test_eof441;
case 441:
	switch( (*p) ) {
		case 13: goto st441;
		case 32: goto st441;
		case 61: goto st442;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st441;
	goto tr0;
st442:
	if ( ++p == pe )
		goto _test_eof442;
case 442:
	switch( (*p) ) {
		case 13: goto st442;
		case 32: goto st442;
		case 34: goto st443;
		case 39: goto st449;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st442;
	goto tr0;
st443:
	if ( ++p == pe )
		goto _test_eof443;
case 443:
	switch( (*p) ) {
		case 110: goto st444;
		case 121: goto st447;
	}
	goto tr0;
st444:
	if ( ++p == pe )
		goto _test_eof444;
case 444:
	if ( (*p) == 111 )
		goto st445;
	goto tr0;
st445:
	if ( ++p == pe )
		goto _test_eof445;
case 445:
	if ( (*p) == 34 )
		goto st446;
	goto tr0;
st446:
	if ( ++p == pe )
		goto _test_eof446;
case 446:
	switch( (*p) ) {
		case 13: goto st446;
		case 32: goto st446;
		case 63: goto st116;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st446;
	goto tr0;
st447:
	if ( ++p == pe )
		goto _test_eof447;
case 447:
	if ( (*p) == 101 )
		goto st448;
	goto tr0;
st448:
	if ( ++p == pe )
		goto _test_eof448;
case 448:
	if ( (*p) == 115 )
		goto st445;
	goto tr0;
st449:
	if ( ++p == pe )
		goto _test_eof449;
case 449:
	switch( (*p) ) {
		case 110: goto st450;
		case 121: goto st452;
	}
	goto tr0;
st450:
	if ( ++p == pe )
		goto _test_eof450;
case 450:
	if ( (*p) == 111 )
		goto st451;
	goto tr0;
st451:
	if ( ++p == pe )
		goto _test_eof451;
case 451:
	if ( (*p) == 39 )
		goto st446;
	goto tr0;
st452:
	if ( ++p == pe )
		goto _test_eof452;
case 452:
	if ( (*p) == 101 )
		goto st453;
	goto tr0;
st453:
	if ( ++p == pe )
		goto _test_eof453;
case 453:
	if ( (*p) == 115 )
		goto st451;
	goto tr0;
st454:
	if ( ++p == pe )
		goto _test_eof454;
case 454:
	if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st455;
	} else if ( (*p) >= 65 )
		goto st455;
	goto tr0;
st455:
	if ( ++p == pe )
		goto _test_eof455;
case 455:
	switch( (*p) ) {
		case 39: goto st430;
		case 95: goto st455;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st455;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st455;
		} else if ( (*p) >= 65 )
			goto st455;
	} else
		goto st455;
	goto tr0;
st456:
	if ( ++p == pe )
		goto _test_eof456;
case 456:
	if ( (*p) == 95 )
		goto st457;
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st457;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st457;
		} else if ( (*p) >= 65 )
			goto st457;
	} else
		goto st457;
	goto tr0;
st457:
	if ( ++p == pe )
		goto _test_eof457;
case 457:
	switch( (*p) ) {
		case 39: goto st417;
		case 95: goto st457;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st457;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st457;
		} else if ( (*p) >= 65 )
			goto st457;
	} else
		goto st457;
	goto tr0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof458: cs = 458; goto _test_eof; 
	_test_eof459: cs = 459; goto _test_eof; 
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
	_test_eof460: cs = 460; goto _test_eof; 
	_test_eof461: cs = 461; goto _test_eof; 
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
	_test_eof137: cs = 137; goto _test_eof; 
	_test_eof138: cs = 138; goto _test_eof; 
	_test_eof139: cs = 139; goto _test_eof; 
	_test_eof140: cs = 140; goto _test_eof; 
	_test_eof141: cs = 141; goto _test_eof; 
	_test_eof142: cs = 142; goto _test_eof; 
	_test_eof143: cs = 143; goto _test_eof; 
	_test_eof144: cs = 144; goto _test_eof; 
	_test_eof145: cs = 145; goto _test_eof; 
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

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 458: 
	case 460: 
/* #line 163 "EcrioCPMParseXML.rl" */
	{
		if (pCurr->parent == ppParsedXml)
		{
			*ppParsedXml = pCurr;
		}
		else
		{
			pStruct = pCurr;
			pCurr = (EcrioCPMParsedXMLStruct*)pCurr->parent;

			if (pCurr->uNumOfChild == 0)
			{
				pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLStruct*), (void **)&pCurr->ppChild);
			}
			else
			{
				/* Check arithmetic overflow */
				if (pal_UtilityArithmeticOverflowDetected(pCurr->uNumOfChild, 1) == Enum_TRUE)
				{
					uCPMError = ECRIO_CPM_MEMORY_ERROR;
					goto END;
				}

				/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
				if (pal_UtilityDataOverflowDetected((pCurr->uNumOfChild + 1), sizeof(EcrioCPMParsedXMLStruct*)) == Enum_TRUE)
				{
					uCPMError = ECRIO_CPM_MEMORY_ERROR;
					goto END;
				}
				pal_MemoryReallocate((u_int32)((pCurr->uNumOfChild + 1) * sizeof(EcrioCPMParsedXMLStruct*)), (void **)&pCurr->ppChild);
			}
			if (NULL == pCurr->ppChild)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;

				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
					__FUNCTION__, __LINE__, uCPMError);
				goto END;
			}
			pCurr->ppChild[pCurr->uNumOfChild] = pStruct;
			pCurr->uNumOfChild++;
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
/* #line 195 "EcrioCPMParseXML.rl" */
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tError: p = 0x%x, number = %d, pe = 0x%x", __FUNCTION__, __LINE__, p, p - pData, pe);
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tValue: %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x |%2.2x| %2.2x %2.2x %2.2x", __FUNCTION__, __LINE__,
			*(p-8), *(p-7), *(p-6), *(p-5), *(p-4), *(p-3), *(p-2), *(p-1), *(p), *(p+1), *(p+2), *(p+3));
		uCPMError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}
	break;
/* #line 8305 "EcrioCPMParseXML.c" */
	}
	}

	_out: {}
	}

/* #line 317 "EcrioCPMParseXML.rl" */

END:
	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u\tuCPMError=%u", __FUNCTION__, __LINE__, uCPMError);

	return uCPMError;
}

void ec_CPM_ReleaseParsedXmlStruct
(
	EcrioCPMParsedXMLStruct **ppParsedXml,
	BoolEnum release
)
{
	u_int32 i;
	EcrioCPMParsedXMLStruct *pParsedXml;

	if (ppParsedXml == NULL || *ppParsedXml == NULL)
	{
		return;
	}

	pParsedXml = *ppParsedXml;

	pal_MemoryFree((void**)&pParsedXml->pElementName);
	pal_MemoryFree((void**)&pParsedXml->pElementValue);

	for (i = 0; i < pParsedXml->uNumOfAttr; i++)
	{
		pal_MemoryFree((void**)&pParsedXml->ppAttr[i]->pAttrName);
		pal_MemoryFree((void**)&pParsedXml->ppAttr[i]->pAttrValue);
		pal_MemoryFree((void**)&pParsedXml->ppAttr[i]);
	}
	pal_MemoryFree((void**)&pParsedXml->ppAttr);

	for (i = 0; i < pParsedXml->uNumOfChild; i++)
	{
		ec_CPM_ReleaseParsedXmlStruct((EcrioCPMParsedXMLStruct**)&pParsedXml->ppChild[i], Enum_TRUE);
	}
	pal_MemoryFree((void**)&pParsedXml->ppChild);

	if (release == Enum_TRUE)
	{
		pal_MemoryFree((void**)ppParsedXml);
	}
}
