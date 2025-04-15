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
#include "EcrioBase64.h"
#include "EcrioPAL.h"
/**************************************************************************/

u_char *_EcrioIMSLibraryFindCharInString
(
	u_char *pString,
	u_char chr
);

/**************************************************************************
  Function:		_EcrioIMSLibraryBase64Encode

  Purpose:		To encode info

  Description:

  Input:		src	u_char buffer with input
            len	size of input in bytes

  Input/Output:		dst	output encoded data

  Returns:		0 success  , failure otherwise . //Number of bytes encoded info takes

**************************************************************************/
u_int32 _EcrioIMSLibraryBase64Encode
(
	u_char **ppBase64Data,
	u_char *pSrcData,
	u_int32 srcDataLen,
	u_int32 *puBase64DataLen
)
{
	/* basic initializations */
	u_int32	error = KPALErrorNone;
	u_int32	i = 0;
	u_int32	uCnt = 0;
	u_char *pTempBase64Data = NULL;
	u_char EncodeTable[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	u_int32 base64DataLen = 0;
	u_int32 tempDataLen = 0;

	if (!pSrcData || (srcDataLen == 0) || !ppBase64Data || !puBase64DataLen)
	{
		return KPALInvalidParameters;
	}

	/* Check for data over flow */
	tempDataLen = (srcDataLen / 3 + ((srcDataLen % 3) ? 1 : 0));
	if(tempDataLen >= (UINT_MAX/4))
		return KPALInvalidSize;

	base64DataLen = 4 * tempDataLen;
	error = pal_MemoryAllocate(base64DataLen + 1, (void **)&pTempBase64Data);
	if (error != KPALErrorNone)
	{
		return KPALMemoryError;
	}

	pTempBase64Data[base64DataLen] = 0;

	error = pal_MemorySet(pTempBase64Data, '=', base64DataLen);
	if (error != KPALErrorNone)
	{
		if (pTempBase64Data)
		{
			pal_MemoryFree((void **)&pTempBase64Data);
		}

		return KPALMemoryError;
	}

	*ppBase64Data = pTempBase64Data;

	i = 0;

	while (i < (srcDataLen - (srcDataLen % 3)))
	{
		/** Check for OOB reads */
		uCnt += 4;
		if (base64DataLen < uCnt)
		{
			pal_MemoryFree((void **)&pTempBase64Data);
			return KPALInternalError;
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
			return KPALInternalError;
		}

		*pTempBase64Data++ = EncodeTable[pSrcData[i] >> 2];

		if (srcDataLen - i == 1)
		{
			/** Check for OOB reads */
			uCnt += 1;
			if (base64DataLen < uCnt)
			{
				pal_MemoryFree((void **)&pTempBase64Data);
				return KPALInternalError;
			}

			*pTempBase64Data = EncodeTable[ (pSrcData[i] & 0x03) << 4];
		}
		else
		{
			/** Check for OOB reads */
			uCnt += 2;
			if (base64DataLen < uCnt)
			{
				pal_MemoryFree((void **)&pTempBase64Data);
				return KPALInternalError;
			}

			*pTempBase64Data++ = EncodeTable[((pSrcData[i] & 0x03) << 4) | (pSrcData[i + 1] >> 4)];
			*pTempBase64Data = EncodeTable[ (pSrcData[i + 1] & 0x0f) << 2];
		}
	}

	*puBase64DataLen = base64DataLen;
	return KPALErrorNone;
}

/**************************************************************************
  Function:		_EcrioIMSLibraryBase64Decode

  Purpose:		To decode info needed

  Description:

  Input:		in	u_char buffer with input
            len	size of input in bytes

  Input/Output:		ppDecodedData	output decoded data

  Returns:		0 success , failure otherwise . //Number of bytes decoded info takes

**************************************************************************/
u_int32 _EcrioIMSLibraryBase64Decode
(
	u_char **ppDecodedData,
	u_char *pBase64Data,
	u_int32 base64DataLen,
	u_int32 decodedDataLen
)
{
	/* basic initializations */
	u_int32	error = KPALErrorNone;
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

	pTemp = _EcrioIMSLibraryFindCharInString(pBase64Data, pPad[0]);
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
	error = pal_MemoryAllocate(decodedDataLen + 1, (void **)&pTempDecodedData);
	if (error != 0)
	{
		return 0;
	}

	pTempDecodedData[decodedDataLen] = '\0';

	pTemp = pBase64Data;
	i = 0;

	c = *pTemp++;

	while ((c != '\0') && i < base64DataLen && j < decodedDataLen)
	{
		if (c == pPad[0])
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

u_char *_EcrioIMSLibraryFindCharInString
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

/**************************************************************************
**************************************************************************/
u_int32 _EcrioIMSLibraryBase64DecodeRequireBufferLength
(
	u_int32 len
)
{
	return 3 * (len / 4);
}

/**************************************************************************
**************************************************************************/
