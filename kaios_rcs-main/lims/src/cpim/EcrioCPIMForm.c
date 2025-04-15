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
 * @file EcrioForm.c
 * @brief Implementation of the CPIM module forming functionality.
*/

#include "EcrioPAL.h"
#include "EcrioCPIM.h"
#include "EcrioCPIMCommon.h"


u_int32 ec_cpim_GenerateRandomAsciiCharString
(
	u_char *pStr,
	u_int32 numOfChar
)
{
	u_int32 i;
	u_int32 randNo;
	static const u_char *charBase = (u_char*)"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	static const u_int32 cardinal = 62;

	if (pStr == NULL)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_1;
	}

	pal_MemorySet(pStr, 0, numOfChar + 1);

	for (i = 0; i < numOfChar; i++)
	{
		randNo = pal_UtilityRandomNumber() % cardinal;
		pStr[i] = charBase[randNo];
	}

	return ECRIO_CPIM_NO_ERROR;
}

u_int32 ec_cpim_FormCPIMMessage
(
	EcrioCPIMStruct *c,
	CPIMMessageStruct *pStruct,
	EcrioCPIMBufferStruct *pBuffStruct,
	u_int32 *pLen
)
{
	u_int32 uCPIMError = ECRIO_CPIM_NO_ERROR;
	u_int32 uSubStrLen = 0, uTotalLen = 0;
	u_int32 uCnt = 0;
	u_int32 uContainerSize = 0;
	u_int32 i;
	u_char cTemp[8];
	u_char cBoundary[32];
	u_char *pCur = NULL, *pBuff = NULL;

	/** Check parameter validity. */
	if (c == NULL)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_1;
	}

	if (pStruct == NULL)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_2;
	}

	if (pBuffStruct == NULL)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_3;
	}

	if (pLen == NULL)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_4;
	}

	CPIMLOGI(c->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Initialize buffers */
	c->uNumOfBuffers = 0;

	pBuffStruct->uSize = 0;
	uCPIMError = ec_cpim_MaintenanceBuffer(pBuffStruct, CPIM_MESSAGE_BUFFER_LENGTH);
	if (uCPIMError != ECRIO_CPIM_NO_ERROR)
	{
		CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tSending message buffer maintenance error.", __FUNCTION__, __LINE__);
		uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
		goto END;
	}
	pal_MemorySet(pBuffStruct->pData, 0, pBuffStruct->uContainerSize);

	c->work.uSize = 0;
	uCPIMError = ec_cpim_MaintenanceBuffer(&c->work, CPIM_WORKING_BUFFER_LENGTH);
	if (uCPIMError != ECRIO_CPIM_NO_ERROR)
	{
		CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tThe working buffer maintenance error.", __FUNCTION__, __LINE__);
		uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
		goto END;
	}
	pal_MemorySet(c->work.pData, 0, c->work.uContainerSize);

	pBuff = c->work.pData;
	uContainerSize = c->work.uContainerSize;

	/** From header */
	{
		pal_MemorySet(pBuff, 0, uContainerSize);
		uSubStrLen = 0;

		ec_cpim_StringConcatenate(pBuff, uContainerSize, CPIM_FROM_STRING, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_COLON_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);
		if (pStruct->pDisplayName != NULL)
		{
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_DOUBLEQUOTES_SYMBOL, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, pStruct->pDisplayName, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_DOUBLEQUOTES_SYMBOL, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);
		}
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_OPENING_ANGLE_BRACE, &pCur);
		if (pStruct->pFromAddr != NULL)
		{
			ec_cpim_StringConcatenate(pCur, uContainerSize, pStruct->pFromAddr, &pCur);
		}
		else
		{
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_ANONYMOUS_URI_STRING, &pCur);
		}
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CLOSING_ANGLE_BRACE, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);

		uSubStrLen = pal_StringLength(pBuff);
		uCPIMError = ec_cpim_MaintenanceBuffer(pBuffStruct, uSubStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tSending message buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemoryCopy(pBuffStruct->pData + uCnt, pBuffStruct->uContainerSize, pBuff, uSubStrLen);
		pBuffStruct->uSize += uSubStrLen;
		uCnt += uSubStrLen;
	}

	/** To header */
	{
		pal_MemorySet(pBuff, 0, uContainerSize);
		uSubStrLen = 0;

		ec_cpim_StringConcatenate(pBuff, uContainerSize, CPIM_TO_STRING, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_COLON_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_OPENING_ANGLE_BRACE, &pCur);
		if (pStruct->pToAddr != NULL)
		{
			ec_cpim_StringConcatenate(pCur, uContainerSize, pStruct->pToAddr, &pCur);
		}
		else
		{
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_ANONYMOUS_URI_STRING, &pCur);
		}
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CLOSING_ANGLE_BRACE, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);

		uSubStrLen = pal_StringLength(pBuff);
		uCPIMError = ec_cpim_MaintenanceBuffer(pBuffStruct, uSubStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tSending message buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemoryCopy(pBuffStruct->pData + uCnt, pBuffStruct->uContainerSize, pBuff, uSubStrLen);
		pBuffStruct->uSize += uSubStrLen;
		uCnt += uSubStrLen;
	}

	/** DateTime header */
	{
		if (pStruct->pDateTime == NULL)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tDateTime is NULL.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_INSUFFICIENT_PARAMETER;
			goto END;
		}

		pal_MemorySet(pBuff, 0, uContainerSize);
		uSubStrLen = 0;

		ec_cpim_StringConcatenate(pBuff, uContainerSize, CPIM_DATETIME_STRING, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_COLON_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, pStruct->pDateTime, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);

		/** Namespace header: imdn param */
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_NAMESPACE_STRING, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_COLON_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);

		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_NAMESPACE_IMDN_STRING, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_OPENING_ANGLE_BRACE, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_IMDN_URN_STRING, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CLOSING_ANGLE_BRACE, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);

		uSubStrLen = pal_StringLength(pBuff);
		uCPIMError = ec_cpim_MaintenanceBuffer(pBuffStruct, uSubStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tSending message buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemoryCopy(pBuffStruct->pData + uCnt, pBuffStruct->uContainerSize, pBuff, uSubStrLen);
		pBuffStruct->uSize += uSubStrLen;
		uCnt += uSubStrLen;
	}

	/** Namespace header: maap param, if any */
	{
		if (pStruct->trfType != CPIMTrafficType_None)
		{
			pal_MemorySet(pBuff, 0, uContainerSize);
			uSubStrLen = 0;

			ec_cpim_StringConcatenate(pBuff, uContainerSize, CPIM_NAMESPACE_STRING, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_COLON_SYMBOL, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_NAMESPACE_MAAP_STRING, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_OPENING_ANGLE_BRACE, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_MAAP_URN_STRING, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CLOSING_ANGLE_BRACE, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);

			uSubStrLen = pal_StringLength(pBuff);
			uCPIMError = ec_cpim_MaintenanceBuffer(pBuffStruct, uSubStrLen);
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tSending message buffer maintenance error.", __FUNCTION__, __LINE__);
				uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
				goto END;
			}
			pal_MemoryCopy(pBuffStruct->pData + uCnt, pBuffStruct->uContainerSize, pBuff, uSubStrLen);
			pBuffStruct->uSize += uSubStrLen;
			uCnt += uSubStrLen;
		}
	}

	/** imdn.Message-ID header */
	{
		if (pStruct->pMsgId == NULL)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tMessage-ID is NULL.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_INSUFFICIENT_PARAMETER;
			goto END;
		}

		pal_MemorySet(pBuff, 0, uContainerSize);
		uSubStrLen = 0;

		ec_cpim_StringConcatenate(pBuff, uContainerSize, CPIM_NAMESPACE_IMDN_STRING, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_DOT_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_MESSAGE_ID_STRING, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_COLON_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, pStruct->pMsgId, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);

		uSubStrLen = pal_StringLength(pBuff);
		uCPIMError = ec_cpim_MaintenanceBuffer(pBuffStruct, uSubStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tSending message buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemoryCopy(pBuffStruct->pData + uCnt, pBuffStruct->uContainerSize, pBuff, uSubStrLen);
		pBuffStruct->uSize += uSubStrLen;
		uCnt += uSubStrLen;
	}

	/** imdn.Disposition-Notification header */
	{
		if (pStruct->dnType != CPIMDispoNotifReq_None)
		{
			pal_MemorySet(pBuff, 0, uContainerSize);
			uSubStrLen = 0;

			ec_cpim_StringConcatenate(pBuff, uContainerSize, CPIM_NAMESPACE_IMDN_STRING, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_DOT_SYMBOL, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_DISPOSITION_NOTIFICATION_STRING, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_COLON_SYMBOL, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);

			/** "negative-delivery" notification */
			if ((pStruct->dnType & CPIMDispoNotifReq_Negative) != 0)
			{
				ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_NEGATIVE_DELIVERY_STRING, &pCur);
				ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_COMMA_SYMBOL, &pCur);
			}
			/** "positive-delivery" notification */
			if ((pStruct->dnType & CPIMDispoNotifReq_Positive) != 0)
			{
				ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_POSITIVE_DELIVERY_STRING, &pCur);
				ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_COMMA_SYMBOL, &pCur);
			}
			/** "display" notification */
			if ((pStruct->dnType & CPIMDispoNotifReq_Display) != 0)
			{
				ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_DISPLAY_STRING, &pCur);
				ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_COMMA_SYMBOL, &pCur);
			}

			uSubStrLen = pal_StringLength(pBuff);

			/** Remove last comma */
			pBuff[--uSubStrLen] = '\0';
			pCur--;

			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);

			uSubStrLen = pal_StringLength(pBuff);
			uCPIMError = ec_cpim_MaintenanceBuffer(pBuffStruct, uSubStrLen);
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tSending message buffer maintenance error.", __FUNCTION__, __LINE__);
				uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
				goto END;
			}
			pal_MemoryCopy(pBuffStruct->pData + uCnt, pBuffStruct->uContainerSize, pBuff, uSubStrLen);
			pBuffStruct->uSize += uSubStrLen;
			uCnt += uSubStrLen;
		}
	}

	/** maap.Traffic-Type header, if any */
	{
		if (pStruct->trfType != CPIMTrafficType_None)
		{
			pal_MemorySet(pBuff, 0, uContainerSize);
			uSubStrLen = 0;

			ec_cpim_StringConcatenate(pBuff, uContainerSize, CPIM_NAMESPACE_MAAP_STRING, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_DOT_SYMBOL, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_TRAFFIC_TYPE_STRING, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_COLON_SYMBOL, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);

			switch (pStruct->trfType)
			{
				/** "advertisement" token */
				case CPIMTrafficType_Advertisement:
				{
					ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_ADVERTISEMENT_STRING, &pCur);
				}
				break;

				/** "payment" token */
				case CPIMTrafficType_Payment:
				{
					ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_PAYMENT_STRING, &pCur);
				}
				break;

				/** "premium" token */
				case CPIMTrafficType_Premium:
				{
					ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_PREMIUM_STRING, &pCur);
				}
				break;

				/** "subscription" token */
				case CPIMTrafficType_Subscription:
				{
					ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SUBSCRIPTION_STRING, &pCur);
				}
				break;

				/** "plugin" token */
				case CPIMTrafficType_Plugin:
				{
					ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_PLUGIN_STRING, &pCur);
				}
				break;

				default:
					break;
			}

			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);

			uSubStrLen = pal_StringLength(pBuff);
			uCPIMError = ec_cpim_MaintenanceBuffer(pBuffStruct, uSubStrLen);
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tSending message buffer maintenance error.", __FUNCTION__, __LINE__);
				uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
				goto END;
			}
			pal_MemoryCopy(pBuffStruct->pData + uCnt, pBuffStruct->uContainerSize, pBuff, uSubStrLen);
			pBuffStruct->uSize += uSubStrLen;
			uCnt += uSubStrLen;
		}
	}

	if (pStruct->pPANI != NULL)
	{
		pal_MemorySet(pBuff, 0, uContainerSize);
		uSubStrLen = 0;

		/** Namespace header: MyFeatures<mailto:RCSFeatures@att.com>  param */
		ec_cpim_StringConcatenate(pBuff, uContainerSize, CPIM_NAMESPACE_STRING, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_COLON_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);

		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_NAMESPACE_MYFEATURE_STRING, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_OPENING_ANGLE_BRACE, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_MYFEATURE_MAILTO_STRING, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CLOSING_ANGLE_BRACE, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);

		uSubStrLen = pal_StringLength(pBuff);
		uCPIMError = ec_cpim_MaintenanceBuffer(pBuffStruct, uSubStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tSending message buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemoryCopy(pBuffStruct->pData + uCnt, pBuffStruct->uContainerSize, pBuff, uSubStrLen);
		pBuffStruct->uSize += uSubStrLen;
		uCnt += uSubStrLen;
	}

	/** MyFeatures.PANI header */
	if (pStruct->pPANI != NULL)
	{
		pal_MemorySet(pBuff, 0, uContainerSize);
		uSubStrLen = 0;

		ec_cpim_StringConcatenate(pBuff, uContainerSize, CPIM_NAMESPACE_MYFEATURE_STRING, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_DOT_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_PANI_ID_STRING, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_COLON_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, pStruct->pPANI, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);

		uSubStrLen = pal_StringLength(pBuff);
		uCPIMError = ec_cpim_MaintenanceBuffer(pBuffStruct, uSubStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tSending message buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemoryCopy(pBuffStruct->pData + uCnt, pBuffStruct->uContainerSize, pBuff, uSubStrLen);
		pBuffStruct->uSize += uSubStrLen;
		uCnt += uSubStrLen;
	}

	/** Blank separator line */
	pal_MemorySet(pBuff, 0, uContainerSize);
	ec_cpim_StringConcatenate(pBuff, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);

	uSubStrLen = pal_StringLength(pBuff);
	uCPIMError = ec_cpim_MaintenanceBuffer(pBuffStruct, uSubStrLen);
	if (uCPIMError != ECRIO_CPIM_NO_ERROR)
	{
		CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tSending message buffer maintenance error.", __FUNCTION__, __LINE__);
		uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
		goto END;
	}
	pal_MemoryCopy(pBuffStruct->pData + uCnt, pBuffStruct->uContainerSize, pBuff, uSubStrLen);
	pBuffStruct->uSize += uSubStrLen;
	uCnt += uSubStrLen;

	/** Form encapsulated MIME object containing the message content as per a number of message bodies */
	for (i = 0; i < pStruct->uNumOfBuffers; i++)
	{
		/** Clean up local buffer */
		pal_MemorySet(pBuff, 0, uContainerSize);
		uSubStrLen = 0;

		/** Content-Type header */
		ec_cpim_StringConcatenate(pBuff, uContainerSize, CPIM_CONTENT_TYPE_STRING, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_COLON_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);

		switch (pStruct->buff[i].eContentType)
		{
			case CPIMContentType_Text:
			{
				ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CONTENT_TYPE_TEXT_STRING, &pCur);
			}
			break;

			case CPIMContentType_IMDN:
			{
				ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CONTENT_TYPE_IMDN_STRING, &pCur);
			}
			break;

			case CPIMContentType_FileTransferOverHTTP:
			{
				ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CONTENT_TYPE_FILE_TRANSFER_HTTP_STRING, &pCur);
			}
			break;

			case CPIMContentType_PushLocation:
			{
				ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CONTENT_TYPE_PUSH_LOCATION_STRING, &pCur);
			}
			break;

			case CPIMContentType_MessageRevoke:
			{
				ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CONTENT_TYPE_MESSAGE_REVOKE_STRING, &pCur);
			}
			break;

			case CPIMContentType_PrivacyManagement:
			{
				ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CONTENT_TYPE_PRIVACY_MANAGEMENT_STRING, &pCur);
			}
			break;

			case CPIMContentType_LinkReport:
			{
				ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CONTENT_TYPE_LINK_REPORT_STRING, &pCur);
			}
			break;

			case CPIMContentType_SpamReport:
			{
				ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CONTENT_TYPE_SPAM_REPORT_STRING, &pCur);
			}
			break;

			case CPIMContentType_RichCard:
			{
				ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CONTENT_TYPE_RICHCARD_STRING, &pCur);
			}
			break;

			case CPIMContentType_SuggestedChipList:
			{
				ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CONTENT_TYPE_SUGGESTED_CHIPLIST_STRING, &pCur);
			}
			break;

			case CPIMContentType_SuggestionResponse:
			{
				ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CONTENT_TYPE_SUGGESTED_RESPONSE_STRING, &pCur);
			}
			break;

			case CPIMContentType_SharedClientData:
			{
				ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CONTENT_TYPE_SHARED_CLIENT_DATA_STRING, &pCur);
			}
			break;

			case CPIMContentType_CpmGroupData:
			{
				ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CONTENT_TYPE_CPM_GROUPDATA_STRING, &pCur);
			}
			break;

			case CPIMContentType_Specified:
			{
				if (pStruct->buff[i].pContentType == NULL)
				{
					CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tInsufficient content type.", __FUNCTION__, __LINE__);
					uCPIMError = ECRIO_CPIM_INSUFFICIENT_PARAMETER;
					goto END;
				}
				ec_cpim_StringConcatenate(pCur, uContainerSize, pStruct->buff[i].pContentType, &pCur);
			}
			break;

			case CPIMContentType_None:
			default:
			{
				CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tInsufficient content type.", __FUNCTION__, __LINE__);
				uCPIMError = ECRIO_CPIM_INSUFFICIENT_PARAMETER;
				goto END;
			}
			break;
		}
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);

		/** Content-Disposition header, if Content-Type is IMDN. */
		if (pStruct->buff[i].eContentType == CPIMContentType_IMDN)
		{
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CONTENT_DISPOSITION_STRING, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_COLON_SYMBOL, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CONTENT_DISPOSITION_NOTIFICATION_STRING, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);
		}

		/** Content-Length header */
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CONTENT_LENGTH_STRING, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_COLON_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);

		pal_MemorySet(cTemp, 0, 8);
		if (0 >= pal_NumToString(pStruct->buff[i].uMsgLen, cTemp, 8))
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tString Copy error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_INSUFFICIENT_MEMORY_ERROR;
			goto END;
		}
		ec_cpim_StringConcatenate(pCur, uContainerSize, cTemp, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);

		if (pStruct->buff[i].pContentId != NULL)
		{
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CONTENT_ID_STRING, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_COLON_SYMBOL, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, pStruct->buff[i].pContentId, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);
		}

		if (pStruct->buff[i].pContentDisposition != NULL)
		{
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CONTENT_DISPOSITION_STRING, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_COLON_SYMBOL, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, pStruct->buff[i].pContentDisposition, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);
		}

		if (pStruct->buff[i].pContentTransferEncoding != NULL)
		{
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CONTENT_TRANSFER_ENCODING_STRING, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_COLON_SYMBOL, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, pStruct->buff[i].pContentTransferEncoding, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);
		}

		/** Blank separator line */
//		if (pStruct->buff[i].uMsgLen > 0)
//		{
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);
//		}

		/** Message body */
		uSubStrLen = pal_StringLength(pBuff);
		c->buff[i].uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->buff[i], uSubStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tSending message buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemoryCopy(c->buff[i].pData, c->buff[i].uContainerSize, pBuff, uSubStrLen);
		c->buff[i].uSize += uSubStrLen;

		uCPIMError = ec_cpim_MaintenanceBuffer(&c->buff[i], pStruct->buff[i].uMsgLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tSending message buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemoryCopy(c->buff[i].pData + c->buff[i].uSize, c->buff[i].uContainerSize, pStruct->buff[i].pMsgBody, pStruct->buff[i].uMsgLen);
		c->buff[i].uSize += pStruct->buff[i].uMsgLen;

		c->uNumOfBuffers++;
	}

	/** Add MIME body */
	if (pStruct->uNumOfBuffers == 0)
	{
		CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tNot exists message.", __FUNCTION__, __LINE__);
		uCPIMError = ECRIO_CPIM_INSUFFICIENT_PARAMETER;
		goto END;
	}
	else if (pStruct->uNumOfBuffers == 1)
	{
		uCPIMError = ec_cpim_MaintenanceBuffer(pBuffStruct, c->buff[0].uSize);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tSending message buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemoryCopy(pBuffStruct->pData + uCnt, pBuffStruct->uContainerSize ,c->buff[0].pData, c->buff[0].uSize);
		pBuffStruct->uSize += c->buff[0].uSize;
		uCnt += c->buff[0].uSize;
	}
	else
	{
		/** Create boundary string */
		pal_MemorySet(cBoundary, 0, 32);
		ec_cpim_StringConcatenate(&cBoundary[0], 32, CPIM_DASH_SYMBOL, NULL);
		ec_cpim_StringConcatenate(&cBoundary[1], 32, CPIM_DASH_SYMBOL, NULL);
		ec_cpim_GenerateRandomAsciiCharString(&cBoundary[2], 16);

		/** Content-Type header */
		pal_MemorySet(pBuff, 0, uContainerSize);
		uSubStrLen = 0;

		ec_cpim_StringConcatenate(pBuff, uContainerSize, CPIM_CONTENT_TYPE_STRING, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_COLON_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CONTENT_TYPE_MULTIPART_MIXED_STRING, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SEMICOLON_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_BOUNDARY_STRING, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_EQUAL_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_DOUBLEQUOTES_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, cBoundary + 2, &pCur);// no preamble "--" in boundary at Content-Type header
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_DOUBLEQUOTES_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);

		/** Content-Length header */
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CONTENT_LENGTH_STRING, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_COLON_SYMBOL, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_SPACE_SYMBOL, &pCur);

		/** Calculate total length */
		uTotalLen = pal_StringLength(cBoundary);
		uTotalLen += 2;		// CRLF
		for (i = 0; i < c->uNumOfBuffers; i++)
		{
			uTotalLen += c->buff[i].uSize;
			uTotalLen += 2;		// CRLF
			uTotalLen += pal_StringLength(cBoundary);
			uTotalLen += 2;		// CRLF, and it will replace to "--" indicated the last boundary
		}

		pal_MemorySet(cTemp, 0, 8);
		if (0 >= pal_NumToString(uTotalLen, cTemp, 8))
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tString Copy error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_INSUFFICIENT_MEMORY_ERROR;
			goto END;
		}
		ec_cpim_StringConcatenate(pCur, uContainerSize, cTemp, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);

		/** Blank separator line */
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);

		/** Boundary line */
		ec_cpim_StringConcatenate(pCur, uContainerSize, cBoundary, &pCur);
		ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);

		uSubStrLen = pal_StringLength(pBuff);
		uCPIMError = ec_cpim_MaintenanceBuffer(pBuffStruct, uSubStrLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tSending message buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemoryCopy(pBuffStruct->pData + uCnt, pBuffStruct->uContainerSize, pBuff, uSubStrLen);
		pBuffStruct->uSize += uSubStrLen;
		uCnt += uSubStrLen;

		/** Contents body */
		for (i = 0; i < c->uNumOfBuffers; i++)
		{
			uCPIMError = ec_cpim_MaintenanceBuffer(pBuffStruct, c->buff[i].uSize);
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tSending message buffer maintenance error.", __FUNCTION__, __LINE__);
				uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
				goto END;
			}
			pal_MemoryCopy(pBuffStruct->pData + uCnt, pBuffStruct->uContainerSize, c->buff[i].pData, c->buff[i].uSize);
			pBuffStruct->uSize += c->buff[i].uSize;
			uCnt += c->buff[i].uSize;

			pal_MemorySet(pBuff, 0, uContainerSize);
			uSubStrLen = 0;

			ec_cpim_StringConcatenate(pBuff, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, cBoundary, &pCur);
			ec_cpim_StringConcatenate(pCur, uContainerSize, CPIM_CRLF_SYMBOL, &pCur);

			uSubStrLen = pal_StringLength(pBuff);
			uCPIMError = ec_cpim_MaintenanceBuffer(pBuffStruct, uSubStrLen);
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tSending message buffer maintenance error.", __FUNCTION__, __LINE__);
				uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
				goto END;
			}
			pal_MemoryCopy(pBuffStruct->pData + uCnt, pBuffStruct->uContainerSize, pBuff, uSubStrLen);
			pBuffStruct->uSize += uSubStrLen;
			uCnt += uSubStrLen;
		}
		/** Replace the last CRLF to "--" */
		pBuffStruct->pData[uCnt-2] = '-';
		pBuffStruct->pData[uCnt-1] = '-';
	}

	*pLen = uCnt;

END:

	CPIMLOGI(c->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return uCPIMError;
}

u_char *EcrioCPIMForm
(
	CPIM_HANDLE handle,
	CPIMMessageStruct *pStruct,
	u_int32 *pLen,
	u_int32 *pError
)
{
	EcrioCPIMStruct *c = NULL;
	u_int32 uCPIMError = ECRIO_CPIM_NO_ERROR;
	u_int32 uLen = 0;
	u_char *pBuff = NULL;

	/** Check parameter validity. */
	if (pError == NULL)
	{
		return NULL;
	}

	if (handle == NULL)
	{
		*pError = ECRIO_CPIM_INVALID_HANDLE;
		return NULL;
	}

	if (pStruct == NULL)
	{
		*pError = ECRIO_CPIM_INVALID_PARAMETER_2;
		return NULL;
	}

	if (pLen == NULL)
	{
		*pError = ECRIO_CPIM_INVALID_PARAMETER_3;
		return NULL;
	}

	/** Set the CPIM handle, c. */
	c = (EcrioCPIMStruct *)handle;

	CPIMLOGI(c->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	CPIM_MUTEX_LOCK(c->mutexAPI, c->logHandle);

	/** Form CPIM Message */
	uCPIMError = ec_cpim_FormCPIMMessage(c, pStruct, &c->msgBuff, &uLen);
	if (uCPIMError != ECRIO_CPIM_NO_ERROR)
	{
		CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tForm CPIM message error.", __FUNCTION__, __LINE__);
		pBuff = NULL;
		uLen = 0;
		goto Err_FormCPIMHeader;
	}

	pBuff = c->msgBuff.pData;

// quiet	CPIMLOGDUMPI(c->logHandle, KLogTypeSIP, pBuff, uLen);

	goto Err_None;

Err_FormCPIMHeader:
Err_None:

	*pLen = uLen;
	*pError = uCPIMError;

	CPIM_MUTEX_UNLOCK(c->mutexAPI, c->logHandle);

	CPIMLOGI(c->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return pBuff;
}
