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

#include "EcrioCPM.h"
#include "EcrioCPMInternal.h"

u_int32 EcrioCPMSendData
(
	CPMHANDLE hCPMHandle,
	EcrioCPMSendDataStruct* pFile,
	u_char **ppMessageId,
	u_char **ppTransactionId,
	BoolEnum bIsCPIMWrapped
)
{
	u_int32 uError = ECRIO_CPM_NO_ERROR;
	u_int32 uCPIMError = ECRIO_CPIM_NO_ERROR;
	EcrioCPMContextStruct *pContext = NULL;
	LOGHANDLE hLogHandle = NULL;
	EcrioCPMSessionStruct* pSession = NULL;
	MSRPFileTransferStruct fileTransfer = { 0 };
	MSRPTextMessageStruct msrpMessage = { 0 };
	u_char *pCallId = NULL;
	CPIMMessageStruct cpimMessage = { 0 };
	EcrioDateAndTimeStruct dateAndTime = { 0 };
	u_char *pBuf = NULL;
	u_int32 uLen = 0;

	if ((NULL == hCPMHandle) || (NULL == pFile))
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	/** Get the CPM structure from the handle */
	pContext = (EcrioCPMContextStruct *)hCPMHandle;

	hLogHandle = pContext->hLogHandle;

	CPMLOGI(hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pContext->bIsRelayEnabled == Enum_TRUE)
		ec_CPM_MapGetData(pContext, (u_char *)pFile->pSessionId, (void**)&pCallId);
	else
		pCallId = (u_char *)pFile->pSessionId;

	EcrioSigMgrGetAppData(pContext->hEcrioSigMgrHandle, pCallId, (void**)&pSession);

	if (pSession == NULL)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
		CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tpSession not found for pSessionId=%s",
			__FUNCTION__, __LINE__, pCallId);
		goto Error_Level_01;
	}

	if (pSession->state != EcrioCPMSessionState_Active)
	{
		uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;

		CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tInvalid sessionId uError=%u",
			__FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	if (bIsCPIMWrapped == Enum_TRUE)
	{
		cpimMessage.trfType = CPIMTrafficType_None;
		cpimMessage.dnType = pSession->pFile->imdnConfig;
		cpimMessage.pToAddr = NULL;
		cpimMessage.pDateTime = pContext->pString;
		/*ISO8601 style (yyyyMMdd'T'HHmmss.SSSZ)*/
		pal_UtilityGetDateAndTime(&dateAndTime);
		if (0 >= pal_StringSNPrintf((char *)cpimMessage.pDateTime, ECRIO_CPM_MAX_COMMON_BUFFER_SIZE, (char *)"%04u-%02u-%02uT%02u:%02u:%02uZ", dateAndTime.year, dateAndTime.month, \
			dateAndTime.day, dateAndTime.hour, dateAndTime.minute, dateAndTime.second))
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
				__FUNCTION__, __LINE__);
			uError = ECRIO_CPM_MEMORY_ERROR;
			goto Error_Level_01;
		}

		cpimMessage.pFromAddr = NULL;
		cpimMessage.pMsgId = (u_char *)pSession->pFile->pIMDNMsgId;
		cpimMessage.buff[0].pMsgBody = pFile->pData;
		cpimMessage.buff[0].uMsgLen = pFile->uTotal;
		cpimMessage.buff[0].eContentType = CPIMContentType_Specified;
		cpimMessage.buff[0].pContentType = (u_char *)pFile->pContentType;
		cpimMessage.uNumOfBuffers++;

		if (pContext->pPANI != NULL)
		{
			cpimMessage.pPANI = pContext->pPANI;
		}
		pBuf = EcrioCPIMForm(pContext->hCPIMHandle, &cpimMessage, &uLen, &uCPIMError);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioCPIMForm failed, uError=%u",
				__FUNCTION__, __LINE__, uCPIMError);
			uError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
			goto Error_Level_01;
		}

		/*Send message using MSRP session*/
		msrpMessage.type = MSRPMessageType_CPIM;
		msrpMessage.u.text.pMsg = pBuf;
		msrpMessage.u.text.uMsgLen = uLen;

		/*fileTransfer.pData = pBuf;
		fileTransfer.uByteRangeStart = 1;
		fileTransfer.uByteRangeEnd = uLen;
		fileTransfer.uTotal = uLen;
		fileTransfer.pContentType = (u_char*)"message/cpim";// (u_char *)pFile->pContentType;
		fileTransfer.bIsEofReached = pFile->bIsEofReached;*/
		if (EcrioMSRPSendTextMessage(pSession->hMSRPSessionHandle, &msrpMessage) != ECRIO_MSRP_NO_ERROR)
		{
			CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioMSRPSendTextMessage failed", __FUNCTION__, __LINE__);
			uError = ECRIO_CPM_MSRP_ERROR;
			goto Error_Level_01;
		}
		if (ppMessageId)
		{
			*ppMessageId = pal_StringCreate((u_char*)pSession->pFile->pIMDNMsgId, pal_StringLength((u_char*)pSession->pFile->pIMDNMsgId));
		}
		ppTransactionId = NULL;
	}
	else
	{
		fileTransfer.pData = pFile->pData;
		fileTransfer.uByteRangeStart = pFile->uByteRangeStart;
		fileTransfer.uByteRangeEnd = pFile->uByteRangeEnd;
		fileTransfer.uTotal = pFile->uTotal;
		fileTransfer.pContentType = (u_char *)pFile->pContentType;
		fileTransfer.bIsEofReached = pFile->bIsEofReached;


		if (EcrioMSRPSendData(pSession->hMSRPSessionHandle, &fileTransfer) != ECRIO_MSRP_NO_ERROR)
		{
			CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioMSRPSendData failed", __FUNCTION__, __LINE__);
			uError = ECRIO_CPM_MSRP_ERROR;
			goto Error_Level_01;
		}

		/*store MSRP Message id and Transfer id */
		if (fileTransfer.pMessageId && ppMessageId)
		{
			*ppMessageId = pal_StringCreate((u_char*)pSession->pFile->pIMDNMsgId, pal_StringLength((u_char*)pSession->pFile->pIMDNMsgId));
		}
		if (fileTransfer.pTransactionId && ppTransactionId)
		{
			*ppTransactionId = pal_StringCreate((u_char*)fileTransfer.pTransactionId, pal_StringLength((u_char*)fileTransfer.pTransactionId));
		}
	}

Error_Level_01:

	CPMLOGI(hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uError;
}
