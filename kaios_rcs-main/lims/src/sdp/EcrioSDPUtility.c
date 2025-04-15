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
* @file EcrioSDPUtility.c
* @brief Implementation of utility functionality for SDP module.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"

/**
 * This is used to convert a string to an unsigned, 64-bit numeric value.
 *
 * @param[in]	pString			Pointer to the string holding the string version
 *								of the number to convert. Must be non-NULL.
 * @return The unsigned 64-bit numeric value after the conversion or 0 if there
 * was an error.
 */
u_int64 ec_sdp_ConvertToUI64Num
(
	const u_char *pString
)
{
	u_int64	ret = 0;
	u_char c = 0;

	if (pString == NULL)
	{
		return 0;
	}

	while (0 != (c = *pString++))
	{
		c -= '0';
		if (c <= 9)
		{
			ret = (ret * 10) + c;
		}
		else
		{
			break;
		}
	}

	return ret;
}

/**
 * This function will copy string from source string to one memory block and
 * indicate the pointer to the destination. The destination memory block will
 * manage using pData in EcrioSDPStringStruct which is a fixed-length buffer
 * by uContainerSize, and set specified pointer in buffer to the destination
 * memory block as pDest.
 *
 * @param[in]	pStrings		Pointer to the string structure. Must be non-NULL.
 * @param[out]  ppDest			Pointer to the destination memory block, to
 *								copy byte data to.
 * @param[in]   pSource			Pointer to the source memory block, to copy
 *								byte data from.
 * @param[in]   memLength		Length of the memory block to be copied, in
 *								bytes.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_StringCopy
(
	EcrioSDPStringStruct *pStrings,
	void **ppDest,
	const void *pSource,
	u_int32 strLength
)
{
	if (pStrings == NULL || ppDest == NULL || pSource == NULL || strLength == 0)
	{
		return ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
	}

	if ((pStrings->uSize + strLength + 1) > pStrings->uContainerSize)
	{
		return ECRIO_SDP_MEMORY_ERROR;
	}

	*ppDest = &pStrings->pData[pStrings->uSize];
	if (KPALErrorNone != pal_MemoryCopy(&pStrings->pData[pStrings->uSize], pStrings->uContainerSize - pStrings->uSize, pSource, strLength))
	{
		return ECRIO_SDP_MEMORY_ERROR;
	}
	pStrings->pData[pStrings->uSize + strLength] = 0;
	pStrings->uSize += strLength + 1;

	return ECRIO_SDP_NO_ERROR;
}

/**
 * This function is used to get NTP timestamp indicated by 64-bit length.
 *
 * @param		void
 * @return NTP time
 */
u_int64 ec_sdp_GetNTPtime
(
	void
)
{
	u_int32 uMilliSeconds;
	u_int32 uHigh32bits;
	u_int32 uLow32bits;
	u_int32 uTime;
	u_int64 uRet;

	uTime = pal_UtilityGetMillisecondCount();

	/** Exchange millisecond to second */
	uHigh32bits = uTime / 1000;

	/** Extract the lower 3 figure of uTime */
	uMilliSeconds = uTime - uHigh32bits * 1000;

	/** Adjust to the NTP timestamp lower 32-bit style */
	uLow32bits = (u_int32)(((float64)0x100000000LL / (float64)1000.) * (float64)uMilliSeconds);

	uRet = (u_int64)(uHigh32bits * 0x100000000LL) + (u_int64)uLow32bits;
	return uRet;
}
