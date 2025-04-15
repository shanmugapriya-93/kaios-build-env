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

#include "EcrioTXN.h"
#include "EcrioTXNInternal.h"
#include "EcrioPAL.h"

#ifdef ENABLE_QCMAPI
u_int32	_EcrioTxnMgrSendDataToCommunicationChannel
(
	_EcrioTXNDataStruct *pTxnData,
	u_char *pData,
	u_int32 uDataLength,
	u_int32	uChannelIndex,
	EcrioTXNMessageTypeEnum	messageType,
	u_char *pCallId,
	BoolEnum bIsResponse
)
#else
u_int32	_EcrioTxnMgrSendDataToCommunicationChannel
(
	_EcrioTXNDataStruct *pTxnData,
	u_char *pData,
	u_int32 uDataLength,
	u_int32	uChannelIndex
)
#endif
{
	u_int32	uError = EcrioTXNNoError;
#ifdef ENABLE_QCMAPI
	IPProtocolTypes p = IPProtocolType_UDP;
	MessageTypes m = MessageType_Request;
	u_char *pProtocol = NULL;
	u_int32 uPort;
#endif

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (uChannelIndex == 0 || ((EcrioTxnMgrTransportStruct *)(pTxnData->pTxnInitParam->pTxnTransportStruct))->pCommunicationStruct == NULL)
	{
		uError = EcrioTXNInsufficientDataError;
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\t- Insufficient Data Error = %d \r\n\r\n", __FUNCTION__, __LINE__, uError);
		goto EndTag;
	}

	TXNMGRLOGD(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u - Incoming uChannelIndex : %d noOfChannels : %d stored channelIndex : %d", __FUNCTION__, __LINE__, uChannelIndex,
		((EcrioTxnMgrTransportStruct *)(pTxnData->pTxnInitParam->pTxnTransportStruct))->pCommunicationStruct->noOfChannels,
		((EcrioTxnMgrTransportStruct *)(pTxnData->pTxnInitParam->pTxnTransportStruct))->pCommunicationStruct->pChannels[uChannelIndex - 1].channelIndex);

	if (uChannelIndex > ((EcrioTxnMgrTransportStruct *)(pTxnData->pTxnInitParam->pTxnTransportStruct))->pCommunicationStruct->noOfChannels ||
		((EcrioTxnMgrTransportStruct *)(pTxnData->pTxnInitParam->pTxnTransportStruct))->pCommunicationStruct->pChannels[uChannelIndex - 1].channelIndex != uChannelIndex)
	{
		uError = EcrioTXNInsufficientDataError;
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\t- Invalid channel index Error = %d \r\n\r\n", __FUNCTION__, __LINE__, uError);
		goto EndTag;
	}

	if (((EcrioTxnMgrTransportStruct *)(pTxnData->pTxnInitParam->pTxnTransportStruct))->pCommunicationStruct->pChannels[uChannelIndex - 1].hChannelHandle == NULL)
	{
		uError = EcrioTXNInsufficientDataError;
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\t- Channel not established Error = %d \r\n\r\n", __FUNCTION__, __LINE__, uError);
		goto EndTag;
	}

#ifdef ENABLE_QCMAPI
	if (messageType == EcrioTXNMessageTypeRequest)
	{
		m = MessageType_Request;
	}
	else if (messageType == EcrioTXNMessageTypeResponse)
	{
		m = MessageType_Response;
	}
	pProtocol = pal_StringFindSubString(pData, (const u_char *)"TCP");
	if (pProtocol != NULL)
	{
		p = IPProtocolType_TCP;
	}
	else
	{
		p = IPProtocolType_UDP;
	}
	if ((bIsResponse == Enum_TRUE) && (p == IPProtocolType_TCP) && 
		(((EcrioTxnMgrTransportStruct *)(pTxnData->pTxnInitParam->pTxnTransportStruct))->pCommunicationStruct->pChannels[uChannelIndex - 1].pChannelInfo->remoteClientPort != 0))
	{
		uPort = ((EcrioTxnMgrTransportStruct *)(pTxnData->pTxnInitParam->pTxnTransportStruct))->pCommunicationStruct->pChannels[uChannelIndex - 1].pChannelInfo->remoteClientPort;
	}
	else
	{
		uPort = ((EcrioTxnMgrTransportStruct *)(pTxnData->pTxnInitParam->pTxnTransportStruct))->pCommunicationStruct->pChannels[uChannelIndex - 1].pChannelInfo->remotePort;
	}
	uError = pal_QcmSendData(pTxnData->pTxnInitParam->pal, pData, uDataLength, p, m, ((EcrioTxnMgrTransportStruct *)(pTxnData->pTxnInitParam->pTxnTransportStruct))->pCommunicationStruct->pChannels[uChannelIndex - 1].pChannelInfo->pRemoteIp, 
		uPort, pCallId);
	if (uError != KPALErrorNone)
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\t- pal_QcmSendData() failed ... Error = %d \r\n\r\n", __FUNCTION__, __LINE__, uError);
		uError = EcrioTXNTransportError;
		goto EndTag;
	}
#else
	uError = pal_SocketSendData(((EcrioTxnMgrTransportStruct *)(pTxnData->pTxnInitParam->pTxnTransportStruct))->pCommunicationStruct->pChannels[uChannelIndex - 1].hChannelHandle, pData, uDataLength);
	if (uError != KPALErrorNone)
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\t- pal_SocketSendData() failed ... Error = %d \r\n\r\n", __FUNCTION__, __LINE__, uError);
		uError = EcrioTXNTransportError;
		goto EndTag;
	}
#endif

EndTag:

	return uError;
}

/**************************************************************************************************
Function:            _EcrioTXNDeleteAllTxnNode()

Purpose:             This function deletes all transactions nodes.

Description:.        This internal function deletes all transactions nodes.

Input:				 _EcrioTXNDataStruct			*pTxnData

OutPut:              none.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum _EcrioTXNDeleteAllTxnNode
(
	_EcrioTXNDataStruct	*pTxnData
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL, *pTxnNodeNext = NULL;
	void *pLogHandle = NULL;
	u_int32	index = 0, timerError = 0;

	if (!pTxnData)
	{
		return EcrioTXNInsufficientDataError;
	}

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

	pTxnNode = pTxnData->pTxnListHead;
	pTxnData->pTxnListHead = NULL;
	pTxnData->pTxnListTail = NULL;

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	while (pTxnNode)
	{
		TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncEntry, "%s:%u and pTxnNode:%x",
			__FUNCTION__, __LINE__, pTxnNode);

		pTxnNodeNext = pTxnNode->pNext;

		if (pTxnNode->pTxnMatchingParam != NULL)
		{
			TXNMGRLOGI(pLogHandle, KLogTypeGeneral,
				"%s:%u\tDeleting %s (%p) current state is %s, pBranch=%s, pCallId=%s , pSessionTxnMappingParam=%s",
				__FUNCTION__, __LINE__,
				_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier),
				pTxnNode, _EcrioTXNGetTxnStateName(pTxnNode->txnState),
				pTxnNode->pTxnMatchingParam->commonMatchingParam.pBranchParam,
				pTxnNode->pTxnMatchingParam->commonMatchingParam.pCallId,
				pTxnNode->pSessionTxnMappingParam->pSessionTxnMappingParamString);
		}

		for (index = 0; index < TXN_NODE_TIMER_ARRAYSIZE_MAX; index++)
		{
			if (pTxnNode->timerDetails[index].isRunning == Enum_TRUE)
			{
				timerError = pal_TimerStop(pTxnNode->timerDetails[index].timerID);
				if (timerError == KPALErrorNone)
				{
					pTxnNode->timerDetails[index].isRunning = Enum_FALSE;
					pTxnNode->timerDetails[index].absoluteDuration = 0;
					pTxnNode->timerDetails[index].timerID = NULL;
					pTxnNode->timerDetails[index].sipTimerType = EcrioTXNSipTimerTypeNone;
				}
				else
				{
					txnRetCode = EcrioTXNTimerError;
				}
			}
		}

		_EcrioTXNStructRelease(	pTxnData,
			(void **)&pTxnNode,
			_EcrioTXNStructType_EcrioTXNTxnNodeStruct,
			Enum_TRUE);

		pTxnNode = pTxnNodeNext;
	}

	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\ttxnRetCode=%d",
		__FUNCTION__, __LINE__, txnRetCode);

	return txnRetCode;
}

/**************************************************************************************************
Function:            _EcrioTXNUpdateSessionTxnMappingParam()

Purpose:             This function updates the transaction mapping parameters.

Description:         This internal updates the transaction mapping parameters based on
                     pTxnInputParam for a perticular session.

Input:				 _EcrioTXNDataStruct			*pTxnData
                     _EcrioTXNTxnNodeStruct		*pTxnNode
                     EcrioTXNInputParamStruct	*pTxnInputParam

OutPut:              none.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum _EcrioTXNUpdateSessionTxnMappingParam
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode,
	EcrioTXNInputParamStruct *pTxnInputParam
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	u_char *pSessionTxnMappingParamString = NULL;

	if (!pTxnData)
	{
		return EcrioTXNInsufficientDataError;
	}

	if ((!pTxnNode) || (!pTxnInputParam))
	{
		return EcrioTXNInsufficientDataError;
	}

	if ((pTxnInputParam->sessionTxnMappingParam.pSessionTxnMappingParamString) &&
		((!pTxnNode->pSessionTxnMappingParam->pSessionTxnMappingParamString) ||
		pal_StringICompare(pTxnInputParam->sessionTxnMappingParam.pSessionTxnMappingParamString,
		pTxnNode->pSessionTxnMappingParam->pSessionTxnMappingParamString)))
	{
		pSessionTxnMappingParamString = pal_StringCreate(
			pTxnInputParam->sessionTxnMappingParam.pSessionTxnMappingParamString,
			pal_StringLength(pTxnInputParam->sessionTxnMappingParam.pSessionTxnMappingParamString));

		if (!pSessionTxnMappingParamString)
		{
			return EcrioTXNInsufficientMemoryError;
		}

		pal_MemoryFree((void **)&(pTxnNode->pSessionTxnMappingParam->pSessionTxnMappingParamString));

		pTxnNode->pSessionTxnMappingParam->pSessionTxnMappingParamString = pSessionTxnMappingParamString;
		pSessionTxnMappingParamString = NULL;
	}

	return txnRetCode;
}

/**************************************************************************************************
Function:            _EcrioTXNUpdateBasicTxnMatchingParam()

Purpose:             This function updates the basic transaction mapping parameters.

Description:         This internal function updates the basic transaction mapping parameters based on
                     pTxnInputParam for a perticular transaction.

Input:				 _EcrioTXNDataStruct			*pTxnData
                     _EcrioTXNTxnNodeStruct		*pTxnNode
                     EcrioTXNInputParamStruct	*pTxnInputParam

OutPut:              none.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum _EcrioTXNUpdateBasicTxnMatchingParam
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode,
	EcrioTXNInputParamStruct *pTxnInputParam
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;

	if (!pTxnData)
	{
		return EcrioTXNInsufficientDataError;
	}

	if ((!pTxnNode) || (!pTxnInputParam))
	{
		return EcrioTXNInsufficientDataError;
	}

	pTxnNode->pTxnMatchingParam->commonMatchingParam.responseCode =
		pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode;

	if (!pTxnNode->pTxnMatchingParam->commonMatchingParam.pBranchParam)
	{
		if (pTxnInputParam->txnMatchingParam.commonMatchingParam.pBranchParam)
		{
			pTxnNode->pTxnMatchingParam->commonMatchingParam.pBranchParam = pal_StringCreate(
				pTxnInputParam->txnMatchingParam.commonMatchingParam.pBranchParam,
				pal_StringLength(pTxnInputParam->txnMatchingParam.commonMatchingParam.pBranchParam));
		}
	}

	if (!pTxnNode->pTxnMatchingParam->commonMatchingParam.pCallId)
	{
		if (pTxnInputParam->txnMatchingParam.commonMatchingParam.pCallId)
		{
			pTxnNode->pTxnMatchingParam->commonMatchingParam.pCallId = pal_StringCreate(
				pTxnInputParam->txnMatchingParam.commonMatchingParam.pCallId,
				pal_StringLength(pTxnInputParam->txnMatchingParam.commonMatchingParam.pCallId));
		}
	}

	if (!pTxnNode->pTxnMatchingParam->commonMatchingParam.cSeq)
	{
		pTxnNode->pTxnMatchingParam->commonMatchingParam.cSeq =
			pTxnInputParam->txnMatchingParam.commonMatchingParam.cSeq;
	}

	if (!pTxnNode->pTxnMatchingParam->commonMatchingParam.pFromTag)
	{
		if (pTxnInputParam->txnMatchingParam.commonMatchingParam.pFromTag)
		{
			pTxnNode->pTxnMatchingParam->commonMatchingParam.pFromTag = pal_StringCreate(
				pTxnInputParam->txnMatchingParam.commonMatchingParam.pFromTag,
				pal_StringLength(pTxnInputParam->txnMatchingParam.commonMatchingParam.pFromTag));
		}
	}

	if (pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode != 100)
	{
		if (!pTxnNode->pTxnMatchingParam->commonMatchingParam.pToTag)
		{
			if (pTxnInputParam->txnMatchingParam.commonMatchingParam.pToTag)
			{
				pTxnNode->pTxnMatchingParam->commonMatchingParam.pToTag = pal_StringCreate(
					pTxnInputParam->txnMatchingParam.commonMatchingParam.pToTag,
					pal_StringLength(pTxnInputParam->txnMatchingParam.commonMatchingParam.pToTag));
			}
		}
	}

	return txnRetCode;
}

/**************************************************************************************************
Function:            _EcrioTXNAllocateCopySessionTxnMappingParam()

Purpose:             This function copy EcrioTXNSessionTxnMappingParamStruct.

Description:         This internal function allocate memory for ppDestSessionTxnMappingParam and
                     copy pSrcSessionTxnMappingParam member values to ppDestSessionTxnMappingParam.

Input:				 _EcrioTXNDataStruct			*pTxnData
                     EcrioTXNSessionTxnMappingParamStruct	*pSrcSessionTxnMappingParam

OutPut:              EcrioTXNSessionTxnMappingParamStruct	**ppDestSessionTxnMappingParam.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum _EcrioTXNAllocateCopySessionTxnMappingParam
(
	_EcrioTXNDataStruct	*pTxnData,
	EcrioTXNSessionTxnMappingParamStruct *pSrcSessionTxnMappingParam,
	EcrioTXNSessionTxnMappingParamStruct **ppDestSessionTxnMappingParam
)
{
	EcrioTXNSessionTxnMappingParamStruct *pSessionTxnMappingParam = NULL;

	(void)pTxnData;

	*ppDestSessionTxnMappingParam = NULL;

	pal_MemoryAllocate(sizeof(EcrioTXNSessionTxnMappingParamStruct), (void **)&(pSessionTxnMappingParam));
	if (!pSessionTxnMappingParam)
	{
		return EcrioTXNInsufficientMemoryError;
	}

	pal_MemorySet(pSessionTxnMappingParam, 0, sizeof(EcrioTXNSessionTxnMappingParamStruct));

	if (pSrcSessionTxnMappingParam->pSessionTxnMappingParamString)
	{
		pSessionTxnMappingParam->pSessionTxnMappingParamString = pal_StringCreate(
			pSrcSessionTxnMappingParam->pSessionTxnMappingParamString,
			pal_StringLength(pSrcSessionTxnMappingParam->pSessionTxnMappingParamString));
	}

	*ppDestSessionTxnMappingParam = pSessionTxnMappingParam;

	return EcrioTXNNoError;
}

/**************************************************************************************************
Function:            _EcrioTXNAllocateCopyTxnMatchingParam()

Purpose:             This function copy EcrioTXNTxnMatchingParamStruct.

Description:         This internal function allocate memory for ppDestTxnMatchingParam and
                     copy pSrcSessionTxnMappingParam member values to ppDestTxnMatchingParam.

Input:				 _EcrioTXNDataStruct			*pTxnData
                     EcrioTXNTxnMatchingParamStruct	*pSrcTxnMatchingParam

OutPut:              EcrioTXNTxnMatchingParamStruct	**ppDestTxnMatchingParam.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum _EcrioTXNAllocateCopyTxnMatchingParam
(
	_EcrioTXNDataStruct	*pTxnData,
	EcrioTXNTxnMatchingParamStruct *pSrcTxnMatchingParam,
	EcrioTXNTxnMatchingParamStruct **ppDestTxnMatchingParam
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	EcrioTXNTxnMatchingParamStruct *pTxnMatchingParam = NULL;

	*ppDestTxnMatchingParam = NULL;

	pal_MemoryAllocate(sizeof(EcrioTXNTxnMatchingParamStruct), (void **)&(pTxnMatchingParam));
	if (!pTxnMatchingParam)
	{
		txnRetCode = EcrioTXNInsufficientMemoryError;
		goto ErrorTag;
	}

	pal_MemorySet(pTxnMatchingParam, 0, sizeof(EcrioTXNTxnMatchingParamStruct));
	pTxnMatchingParam->commonMatchingParam.responseCode = pSrcTxnMatchingParam->commonMatchingParam.responseCode;
	pTxnMatchingParam->commonMatchingParam.txnIdentifier.txnType = pSrcTxnMatchingParam->commonMatchingParam.txnIdentifier.txnType;
	pTxnMatchingParam->commonMatchingParam.cSeq = pSrcTxnMatchingParam->commonMatchingParam.cSeq;

	if (pSrcTxnMatchingParam->commonMatchingParam.pBranchParam)
	{
		pTxnMatchingParam->commonMatchingParam.pBranchParam = pal_StringCreate(
			pSrcTxnMatchingParam->commonMatchingParam.pBranchParam,
			pal_StringLength(pSrcTxnMatchingParam->commonMatchingParam.pBranchParam));
		if (pTxnMatchingParam->commonMatchingParam.pBranchParam == NULL)
		{
			txnRetCode = EcrioTXNInsufficientMemoryError;
			goto ErrorTag;
		}
	}

	if (pSrcTxnMatchingParam->commonMatchingParam.pCallId)
	{
		pTxnMatchingParam->commonMatchingParam.pCallId = pal_StringCreate(
			pSrcTxnMatchingParam->commonMatchingParam.pCallId,
			pal_StringLength(pSrcTxnMatchingParam->commonMatchingParam.pCallId));
		if (pTxnMatchingParam->commonMatchingParam.pCallId == NULL)
		{
			txnRetCode = EcrioTXNInsufficientMemoryError;
			goto ErrorTag;
		}
	}

	if (pSrcTxnMatchingParam->commonMatchingParam.pFromTag)
	{
		pTxnMatchingParam->commonMatchingParam.pFromTag = pal_StringCreate(
			pSrcTxnMatchingParam->commonMatchingParam.pFromTag,
			pal_StringLength(pSrcTxnMatchingParam->commonMatchingParam.pFromTag));
		if (pTxnMatchingParam->commonMatchingParam.pFromTag == NULL)
		{
			txnRetCode = EcrioTXNInsufficientMemoryError;
			goto ErrorTag;
		}
	}

	if (pSrcTxnMatchingParam->commonMatchingParam.pToTag)
	{
		pTxnMatchingParam->commonMatchingParam.pToTag = pal_StringCreate(
			pSrcTxnMatchingParam->commonMatchingParam.pToTag,
			pal_StringLength(pSrcTxnMatchingParam->commonMatchingParam.pToTag));
		if (pTxnMatchingParam->commonMatchingParam.pToTag == NULL)
		{
			txnRetCode = EcrioTXNInsufficientMemoryError;
			goto ErrorTag;
		}
	}

	if (pSrcTxnMatchingParam->commonMatchingParam.txnIdentifier.pMethodName)
	{
		pTxnMatchingParam->commonMatchingParam.txnIdentifier.pMethodName = pal_StringCreate(
			pSrcTxnMatchingParam->commonMatchingParam.txnIdentifier.pMethodName,
			pal_StringLength(pSrcTxnMatchingParam->commonMatchingParam.txnIdentifier.pMethodName));
		if (pTxnMatchingParam->commonMatchingParam.txnIdentifier.pMethodName == NULL)
		{
			txnRetCode = EcrioTXNInsufficientMemoryError;
			goto ErrorTag;
		}
	}

	*ppDestTxnMatchingParam = pTxnMatchingParam;

	return txnRetCode;

ErrorTag:

	if (pTxnMatchingParam)
	{
		_EcrioTXNStructRelease(pTxnData,
			(void **)&pTxnMatchingParam,
			_EcrioTXNStructType_EcrioTXNTxnMatchingParamStruct,
			Enum_TRUE);
	}

	return txnRetCode;
}

/**************************************************************************************************
Function:            _EcrioTXNCopyTxnMatchingParamACK()

Purpose:             This function copy EcrioTXNTxnMatchingParamStruct for ACK.

Description:         This internal function allocate memory for EcrioTXNTxnMatchingParamStruct which
                     is member of _EcrioTXNTxnNodeStruct. This function also copy the
                     EcrioTXNTxnMatchingParamStruct of pTxnInputParam to pTxnMatchingParamACK.

Input:				 _EcrioTXNDataStruct			*pTxnData
                     EcrioTXNInputParamStruct	*pTxnInputParam
                     _EcrioTXNTxnNodeStruct		*pTxnNode

OutPut:              None.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum _EcrioTXNCopyTxnMatchingParamACK
(
	_EcrioTXNDataStruct	*pTxnData,
	EcrioTXNInputParamStruct *pTxnInputParam,
	_EcrioTXNTxnNodeStruct *pTxnNode
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;

	txnRetCode = _EcrioTXNAllocateCopyTxnMatchingParam(pTxnData,
		&(pTxnInputParam->txnMatchingParam),
		&(pTxnNode->pTxnMatchingParamACK));

	if (txnRetCode != EcrioTXNNoError)
	{
		return txnRetCode;
	}

	pTxnNode->pTxnMatchingParamACK->commonMatchingParam.responseCode = pTxnNode->pTxnMatchingParam->commonMatchingParam.responseCode;

	return txnRetCode;
}

/**************************************************************************************************
Function:            _EcrioTXNCreateTxnNode()

Purpose:             This API creates a new node for a tranction.

Description:.        This internal function creates a new node to transaction linklist for a
                     transaction with the value it receives in input parameters.

Input:				 _EcrioTXNDataStruct				*pTxnData
                     EcrioTXNInputParamStruct	*pTxnInputParam
                     u_int32						uChannelIndex
                     EcrioTXNReturnCodeEnum		*pTxnRetCode

OutPut:              None.

Returns:             _EcrioTXNTxnNodeStruct*.
**************************************************************************************************/
_EcrioTXNTxnNodeStruct *_EcrioTXNCreateTxnNode
(
	_EcrioTXNDataStruct	*pTxnData,
	EcrioTXNInputParamStruct *pTxnInputParam,
	u_int32	uChannelIndex,
	EcrioTXNReturnCodeEnum *pTxnRetCode
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;
	void *pLogHandle = NULL;

	if (!pTxnData)
	{
		*pTxnRetCode = EcrioTXNInsufficientDataError;
		return NULL;
	}

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

	TXNMGRLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pal_MemoryAllocate(sizeof(_EcrioTXNTxnNodeStruct), (void **)&(pTxnNode));
	if (!pTxnNode)
	{
		txnRetCode = EcrioTXNInsufficientMemoryError;
		goto ErrorTag;
	}

	pal_MemorySet(pTxnNode, 0, sizeof(_EcrioTXNTxnNodeStruct));

	pTxnNode->bRetryEnabled = Enum_TRUE;
	pTxnNode->pTxnData = (void *)pTxnData;

	txnRetCode = _EcrioTXNAllocateCopyTxnMatchingParam(pTxnData,
		&(pTxnInputParam->txnMatchingParam),
		&(pTxnNode->pTxnMatchingParam));
	if (txnRetCode != EcrioTXNNoError)
	{
		goto ErrorTag;
	}

	txnRetCode = _EcrioTXNAllocateCopySessionTxnMappingParam(pTxnData,
		&(pTxnInputParam->sessionTxnMappingParam),
		&(pTxnNode->pSessionTxnMappingParam));
	if (txnRetCode != EcrioTXNNoError)
	{
		goto ErrorTag;
	}

	pTxnNode->transportDetails.transportType = pTxnInputParam->transportDetails.transportType;

	pTxnNode->retransmissionContext = (s_int32)(pTxnInputParam->retransmitContext);

	pTxnNode->uChannelIndex = uChannelIndex;

	TXNMGRLOGI(pLogHandle, KLogTypeGeneral,
		"%s:%u\tNew node=%p created for the transaction %s",
		__FUNCTION__, __LINE__, pTxnNode,
		_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

	if ((pTxnInputParam->txnMatchingParam.commonMatchingParam.txnIdentifier.txnType == EcrioTXNTxnTypeClientInvite) ||
		(pTxnInputParam->txnMatchingParam.commonMatchingParam.txnIdentifier.txnType == EcrioTXNTxnTypeClientNonInvite))
	{
		/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
		if (pal_UtilityDataOverflowDetected(pTxnInputParam->msgLen, sizeof(u_char)) == Enum_TRUE)
		{
			txnRetCode = EcrioTXNInsufficientMemoryError;
			goto ErrorTag;
		}

		pal_MemoryAllocate((pTxnInputParam->msgLen) * sizeof(u_char), (void **)&(pTxnNode->pMsg));
		if (!pTxnNode->pMsg)
		{
			txnRetCode = EcrioTXNInsufficientMemoryError;
			goto ErrorTag;
		}

		pal_MemoryCopy(
			pTxnNode->pMsg,
			pTxnInputParam->msgLen,
			pTxnInputParam->pMsg,
			pTxnInputParam->msgLen);

		pTxnNode->msgLen = pTxnInputParam->msgLen;
		pTxnNode->pSMSRetryData = pTxnInputParam->pSMSRetryData;
		pTxnNode->SmsRetryCount = pTxnInputParam->SmsRetryCount;


		if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeUDP)
		{
			EcrioTXNCustomTimersStruct *pCustomSIPTimers = &(pTxnData->pTxnInitParam->customSIPTimers);

			/*	Start Retransmit (A/E) and Timeout (B/F) Timer	*/
			if ((pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier.txnType == EcrioTXNTxnTypeClientInvite) &&
				(pCustomSIPTimers->TimerA >= pCustomSIPTimers->TimerB))
			{
				/* If Timer A is greater or equal to timer B for INVITE request then only Timer B will be fired. */
				txnRetCode = _EcrioTXNStartTxnTimer(pTxnData,
					pTxnNode,
					EcrioTXNTimeoutTimer);
			}
			else if ((pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier.txnType == EcrioTXNTxnTypeClientNonInvite) &&
				(pCustomSIPTimers->TimerE >= pCustomSIPTimers->TimerF))
			{
				/* If Timer E is greater or equal to timer F for non-INVITE request then only Timer F will be fired. */
				txnRetCode = _EcrioTXNStartTxnTimer(pTxnData,
					pTxnNode,
					EcrioTXNTimeoutTimer);
			}
			else
			{
				txnRetCode = _EcrioTXNStartTxnTimer(pTxnData,
					pTxnNode,
					(EcrioTXNTimerTypeEnum)(((u_char)EcrioTXNRetransmitTimer) | ((u_char)EcrioTXNTimeoutTimer)));
			}
		}
		else
		{
			/*	Start Timeout (B/F) Timer	*/
			txnRetCode = _EcrioTXNStartTxnTimer(pTxnData,
				pTxnNode,
				EcrioTXNTimeoutTimer);
		}

		if (txnRetCode != EcrioTXNNoError)
		{
			goto ErrorTag;
		}

		if (pTxnInputParam->txnMatchingParam.commonMatchingParam.txnIdentifier.txnType == EcrioTXNTxnTypeClientInvite)
		{
			TXNMGRLOGD(pLogHandle, KLogTypeState,
				"%s:%u\ttxnState is transiting from %s to %s of the transaction %s",
				__FUNCTION__, __LINE__, _EcrioTXNGetTxnStateName(pTxnNode->txnState),
				_EcrioTXNGetTxnStateName(EcrioTXNTxnStateCalling),
				_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

			pTxnNode->txnState = EcrioTXNTxnStateCalling;
		}
		else
		{
			TXNMGRLOGD(pLogHandle, KLogTypeState,
				"%s:%u\ttxnState is transiting from %s to %s of the transaction %s",
				__FUNCTION__, __LINE__, _EcrioTXNGetTxnStateName(pTxnNode->txnState),
				_EcrioTXNGetTxnStateName(EcrioTXNTxnStateTrying),
				_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

			pTxnNode->txnState = EcrioTXNTxnStateTrying;
		}
	}
	else if ((pTxnInputParam->txnMatchingParam.commonMatchingParam.txnIdentifier.txnType == EcrioTXNTxnTypeServerInvite) ||
		(pTxnInputParam->txnMatchingParam.commonMatchingParam.txnIdentifier.txnType == EcrioTXNTxnTypeServerNonInvite))
	{
		if (pTxnInputParam->txnMatchingParam.commonMatchingParam.txnIdentifier.txnType == EcrioTXNTxnTypeServerInvite)
		{
			TXNMGRLOGD(pLogHandle, KLogTypeState,
				"%s:%u\ttxnState is transiting from %s to %s of the transaction %s",
				__FUNCTION__, __LINE__, _EcrioTXNGetTxnStateName(pTxnNode->txnState),
				_EcrioTXNGetTxnStateName(EcrioTXNTxnStateProceeding),
				_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

			pTxnNode->txnState = EcrioTXNTxnStateProceeding;
		}
		else
		{
			TXNMGRLOGD(pLogHandle, KLogTypeState,
				"%s:%u\ttxnState is transiting from %s to %s of the transaction %s",
				__FUNCTION__, __LINE__,	_EcrioTXNGetTxnStateName(pTxnNode->txnState),
				_EcrioTXNGetTxnStateName(EcrioTXNTxnStateTrying),
				_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

			pTxnNode->txnState = EcrioTXNTxnStateTrying;
		}
	}

	*pTxnRetCode = txnRetCode;
	return pTxnNode;

ErrorTag:

	if (pTxnNode)
	{
		_EcrioTXNStructRelease(pTxnData,
			(void **)&pTxnNode,
			_EcrioTXNStructType_EcrioTXNTxnNodeStruct,
			Enum_TRUE);
	}

	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	*pTxnRetCode = txnRetCode;

	return NULL;
}

/**************************************************************************************************
Function:            _EcrioTXNDeleteTxnNode()

Purpose:             This API deletes node for a tranction.

Description:.        This internal function deletes a node from transaction linklist.

Input:				 _EcrioTXNDataStruct		*pTxnData
                     _EcrioTXNTxnNodeStruct		*pTxnNode

OutPut:              None.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum _EcrioTXNDeleteTxnNode
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	u_int32 index = 0;
	void *pLogHandle = NULL;

	if (!pTxnData)
	{
		return EcrioTXNInsufficientDataError;
	}

	if (!pTxnNode)
	{
		return EcrioTXNInsufficientDataError;
	}

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

	TXNMGRLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	TXNMGRLOGD(pLogHandle, KLogTypeState,
		"%s:%u\tDeleting %s transaction, current state is %s", __FUNCTION__, __LINE__,
		_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier),
		_EcrioTXNGetTxnStateName(pTxnNode->txnState));

	for (index = 0; index < TXN_NODE_TIMER_ARRAYSIZE_MAX; index++)
	{
		if (pTxnNode->timerDetails[index].isRunning == Enum_TRUE)
		{
			pal_TimerStop(pTxnNode->timerDetails[index].timerID);

			pTxnNode->timerDetails[index].isRunning	= Enum_FALSE;
			pTxnNode->timerDetails[index].absoluteDuration = 0;
			pTxnNode->timerDetails[index].timerID = NULL;
			pTxnNode->timerDetails[index].sipTimerType = EcrioTXNSipTimerTypeNone;
		}
	}

	_EcrioTXNStructRelease(pTxnData,
		(void **)&pTxnNode,
		_EcrioTXNStructType_EcrioTXNTxnNodeStruct,
		Enum_TRUE);

	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return txnRetCode;
}

/**************************************************************************************************
Function:            _EcrioTXNStartTxnTimerInternal()

Purpose:             This API starts timer for a tranction.

Description:.        This internal function starts timer for a transaction based on input parameter
                     timerType. Timer types are A,B,E,F etc.

Input:				 _EcrioTXNDataStruct		*pTxnData
                     _EcrioTXNTxnNodeStruct		*pTxnNode
                     EcrioTXNSipTimerTypeEnum		sipTimerType

OutPut:              None.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum _EcrioTXNStartTxnTimerInternal
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode,
	EcrioTXNSipTimerTypeEnum sipTimerType
)
{
	TIMERHANDLE	newTimerID = 0;
	u_int32	timerError = KPALErrorNone;
	u_int32	index = 0;
	// u_int32							newTimerInterval = 0;
	u_int32	revisedTimerInterval = 0;
	u_int32	maxTimeInterval = 0;
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	// EcrioPlatManTimerCallback		timerCallbackFn = NULL;
	// EcrioPlatManTimerInitStruct		timerInit;
	TimerStartConfigStruct timerConfig;
	u_int32	timerX;
	EcrioTXNCustomTimersStruct *pCustomSIPTimers = NULL;

	void *pLogHandle = NULL;

	if (!pTxnData)
	{
		return EcrioTXNInsufficientDataError;
	}

	pCustomSIPTimers = &(pTxnData->pTxnInitParam->customSIPTimers);

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

// quiet	TXNMGRLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

// quiet	TXNMGRLOGI(pLogHandle, KLogTypeFuncParams, "%s:%u\tpTxnData=%p, pTxnNode=%p, sipTimerType=%s",
// quiet		__FUNCTION__, __LINE__, pTxnData, pTxnNode, _EcrioTXNGetSipTimerName(pTxnData, sipTimerType));

	if (!pTxnNode)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	/* to avoid Symbian warnings */
	timerError = timerError;

	timerConfig.uInitialInterval = 0;

	//EcrioTXNValidateAndStoreCustomTimerValues(pCustomSIPTimers);

	index = _EcrioTXNGetRunningTimerIndex(pTxnNode, sipTimerType);

	if (index >= TXN_NODE_TIMER_ARRAYSIZE_MAX)
	{
		index = _EcrioTXNGetFreeTimerIndex(pTxnNode);

		if (index >= TXN_NODE_TIMER_ARRAYSIZE_MAX)
		{
			TXNMGRLOGE(pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioTXNGetFreeTimerIndex() no timer %s running and no free space to accomodate it for node %p",
				__FUNCTION__, __LINE__, _EcrioTXNGetSipTimerName(pTxnData, sipTimerType), pTxnNode);

			txnRetCode = EcrioTXNTimerError;
			goto EndTag;
		}
	}

	timerX = pTxnData->pTxnInitParam->timerX;

	if (sipTimerType == EcrioTXNSipTimerTypeA)
	{
		if (pTxnNode->timerDetails[index].absoluteDuration == 0)
		{
			timerConfig.uInitialInterval = pCustomSIPTimers->TimerA * timerX;
		}
		else
		{
			timerConfig.uInitialInterval = 2 * pTxnNode->timerDetails[index].absoluteDuration;
		}

		timerConfig.timerCallback = _EcrioTXNTimerACallback;
	}
	else if (sipTimerType == EcrioTXNSipTimerTypeB)
	{
		timerConfig.uInitialInterval = pCustomSIPTimers->TimerB * timerX;
		timerConfig.timerCallback = _EcrioTXNTimerBCallback;
	}
	else if (sipTimerType == EcrioTXNSipTimerTypeD)
	{
		if (pTxnNode->transportDetails.transportType == EcrioTXNTransportTypeTCP)
		{
			timerConfig.uInitialInterval = ECRIO_TXN_SIP_TCP_WAIT_TIMER_INTERVAL;
		}
		else if (pTxnNode->transportDetails.transportType == EcrioTXNTransportTypeTLS)
		{
			timerConfig.uInitialInterval = ECRIO_TXN_SIP_TCP_WAIT_TIMER_INTERVAL;
		}
		else
		{
			timerConfig.uInitialInterval = pCustomSIPTimers->TimerD * timerX;
		}

		timerConfig.timerCallback = _EcrioTXNTimerDCallback;
	}
	else if (sipTimerType == EcrioTXNSipTimerTypeE)
	{
		if (pTxnNode->timerDetails[index].absoluteDuration == 0)
		{
			timerConfig.uInitialInterval = pCustomSIPTimers->TimerE * timerX;
		}
		else
		{
			revisedTimerInterval = 2 * pTxnNode->timerDetails[index].absoluteDuration;
			maxTimeInterval	= pCustomSIPTimers->T2 * timerX;
		}

		timerConfig.timerCallback = _EcrioTXNTimerECallback;
	}
	else if (sipTimerType == EcrioTXNSipTimerTypeF)
	{
		timerConfig.uInitialInterval = pCustomSIPTimers->TimerF * timerX;
		timerConfig.timerCallback = _EcrioTXNTimerFCallback;
	}
	else if (sipTimerType == EcrioTXNSipTimerTypeG)
	{
		if (pTxnNode->timerDetails[index].absoluteDuration == 0)
		{
			timerConfig.uInitialInterval = pCustomSIPTimers->TimerG * timerX;
		}
		else
		{
			revisedTimerInterval = 2 * pTxnNode->timerDetails[index].absoluteDuration;
			maxTimeInterval	= pCustomSIPTimers->T2 * timerX;
		}

		timerConfig.timerCallback = _EcrioTXNTimerGCallback;
	}
	else if (sipTimerType == EcrioTXNSipTimerTypeH)
	{
		timerConfig.uInitialInterval = pCustomSIPTimers->TimerH * timerX;
		timerConfig.timerCallback = _EcrioTXNTimerHCallback;
	}
	else if (sipTimerType == EcrioTXNSipTimerTypeI)
	{
		if (pTxnNode->transportDetails.transportType == EcrioTXNTransportTypeTCP)
		{
			timerConfig.uInitialInterval = ECRIO_TXN_SIP_TCP_WAIT_TIMER_INTERVAL;
		}
		else if (pTxnNode->transportDetails.transportType == EcrioTXNTransportTypeTLS)
		{
			timerConfig.uInitialInterval = ECRIO_TXN_SIP_TCP_WAIT_TIMER_INTERVAL;
		}
		else
		{
			timerConfig.uInitialInterval = pCustomSIPTimers->TimerI * timerX;
		}

		timerConfig.timerCallback = _EcrioTXNTimerICallback;
	}
	else if (sipTimerType == EcrioTXNSipTimerTypeJ)
	{
		if (pTxnNode->transportDetails.transportType == EcrioTXNTransportTypeTCP)
		{
			timerConfig.uInitialInterval = ECRIO_TXN_SIP_TCP_WAIT_TIMER_INTERVAL;
		}
		else if (pTxnNode->transportDetails.transportType == EcrioTXNTransportTypeTLS)
		{
			timerConfig.uInitialInterval = ECRIO_TXN_SIP_TCP_WAIT_TIMER_INTERVAL;
		}
		else
		{
			timerConfig.uInitialInterval = pCustomSIPTimers->TimerJ * timerX;
		}

		timerConfig.timerCallback = _EcrioTXNTimerJCallback;
	}
	else if (sipTimerType == EcrioTXNSipTimerTypeK)
	{
		if (pTxnNode->transportDetails.transportType == EcrioTXNTransportTypeTCP)
		{
			timerConfig.uInitialInterval = ECRIO_TXN_SIP_TCP_WAIT_TIMER_INTERVAL;
		}
		else if (pTxnNode->transportDetails.transportType == EcrioTXNTransportTypeTLS)
		{
			timerConfig.uInitialInterval = ECRIO_TXN_SIP_TCP_WAIT_TIMER_INTERVAL;
		}
		else
		{
			timerConfig.uInitialInterval = pCustomSIPTimers->TimerK * timerX;
		}

		timerConfig.timerCallback = _EcrioTXNTimerKCallback;
	}
	else
	{
		txnRetCode = EcrioTXNTimerError;
		goto EndTag;
	}

	if (timerConfig.uInitialInterval == 0)
	{
		timerConfig.uInitialInterval = (revisedTimerInterval < maxTimeInterval)
			? (revisedTimerInterval)
			: (maxTimeInterval);
	}

	TXNMGRLOGD(pLogHandle, KLogTypeGeneral,
		"%s:%u\tStarting timer %s for time=%u for node=%p",
		__FUNCTION__, __LINE__, _EcrioTXNGetSipTimerName(pTxnData, sipTimerType), timerConfig.uInitialInterval, pTxnNode);

	timerConfig.uPeriodicInterval = 0;
	timerConfig.pCallbackData = pTxnNode;
	timerConfig.bEnableGlobalMutex = Enum_TRUE;

	timerError = pal_TimerStart(pTxnData->pTxnInitParam->pal, &timerConfig, &newTimerID);
	if (timerError == KPALErrorNone)
	{
		pTxnNode->timerDetails[index].isRunning	= Enum_TRUE;
		pTxnNode->timerDetails[index].absoluteDuration = timerConfig.uInitialInterval;
		pTxnNode->timerDetails[index].timerID = newTimerID;
		pTxnNode->timerDetails[index].sipTimerType = sipTimerType;
	}
	else
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_TimerStart(), platManError=%u",
			__FUNCTION__, __LINE__, timerError);
	}

EndTag:
// quiet	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\ttxnRetCode=%u",
// quiet		__FUNCTION__, __LINE__, txnRetCode);

	return txnRetCode;
}

/**************************************************************************************************
Function:            _EcrioTXNStopTxnTimerInternal()

Purpose:             This API stops timer for a tranction.

Description:.        This internal function stops timer for a transaction based on input parameter
                     timerType. Timer types are Timer types are A,B,E,F etc.

Input:				 _EcrioTXNDataStruct		*pTxnData
                     _EcrioTXNTxnNodeStruct		*pTxnNode
                     EcrioTXNSipTimerTypeEnum		sipTimerType

OutPut:              None.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum _EcrioTXNStopTxnTimerInternal
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode,
	EcrioTXNSipTimerTypeEnum sipTimerType
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	// u_int32	timerError = KPALErrorNone;
	u_int32	index = 0;
	TIMERHANDLE	timerID = NULL;
	void *pLogHandle = NULL;

	if (!pTxnData)
	{
		return txnRetCode;
	}

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

	TXNMGRLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	TXNMGRLOGI(pLogHandle, KLogTypeFuncParams,
		"%s:%u\tpTxnData=%p, pTxnNode=%p, sipTimerType=%s",
		__FUNCTION__, __LINE__, pTxnData, pTxnNode, _EcrioTXNGetSipTimerName(pTxnData, sipTimerType));

	if (!pTxnNode)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	index = _EcrioTXNGetRunningTimerIndex(pTxnNode, sipTimerType);

	if (index >= TXN_NODE_TIMER_ARRAYSIZE_MAX)
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioTXNGetRunningTimerIndex() no timer of type %u running on node %p",
			__FUNCTION__, __LINE__, sipTimerType, pTxnNode);

		txnRetCode = EcrioTXNTimerError;
		goto EndTag;
	}

	timerID = pTxnNode->timerDetails[index].timerID;

	TXNMGRLOGI(pLogHandle, KLogTypeGeneral,
		"%s:%u\tStopping timer %s for node=%p",
		__FUNCTION__, __LINE__, _EcrioTXNGetSipTimerName(pTxnData, sipTimerType), pTxnNode);

#ifdef ECRIO_AUTOTEST
	TXNMGRLOGI(pLogHandle, KLogTypeGeneral,
		"ECRIO_AUTOTEST - %s:%u\tStopping timer %s", __FUNCTION__, __LINE__, _EcrioTXNGetSipTimerName(pTxnData, sipTimerType));
#endif	// ECRIO_AUTOTEST

	pal_TimerStop(timerID);

	pTxnNode->timerDetails[index].isRunning	= Enum_FALSE;
	pTxnNode->timerDetails[index].absoluteDuration = 0;
	pTxnNode->timerDetails[index].timerID = NULL;
	pTxnNode->timerDetails[index].sipTimerType = EcrioTXNSipTimerTypeNone;

EndTag:

	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\ttxnRetCode=%u", __FUNCTION__, __LINE__, txnRetCode);

	return txnRetCode;
}

/**************************************************************************************************
Function:            _EcrioTXNGetRunningTimerIndex()

Purpose:             This API returns running timer index.

Description:.        This internal function returns running timer index for input paramer sipTimerType.

Input:				 _EcrioTXNTxnNodeStruct		*pTxnNode
                     EcrioTXNSipTimerTypeEnum	sipTimerType

OutPut:              None.

Returns:             u_int32.
**************************************************************************************************/
u_int32 _EcrioTXNGetRunningTimerIndex
(
	_EcrioTXNTxnNodeStruct *pTxnNode,
	EcrioTXNSipTimerTypeEnum sipTimerType
)
{
	u_int32	index = 0;

	for (index = 0; index < TXN_NODE_TIMER_ARRAYSIZE_MAX; index++)
	{
		if (pTxnNode->timerDetails[index].isRunning == Enum_TRUE)
		{
			if (pTxnNode->timerDetails[index].sipTimerType == sipTimerType)
			{
				break;
			}
		}
	}

	return index;
}

/**************************************************************************************************
Function:            _EcrioTXNGetRunningTimerIndex()

Purpose:             This API returns free timer index.

Description:.        This internal function returns free timer index.

Input:				 _EcrioTXNTxnNodeStruct		*pTxnNode

OutPut:              None.

Returns:             u_int32.
**************************************************************************************************/
u_int32 _EcrioTXNGetFreeTimerIndex
(
	_EcrioTXNTxnNodeStruct *pTxnNode
)
{
	u_int32	index = 0;

	for (index = 0; index < TXN_NODE_TIMER_ARRAYSIZE_MAX; index++)
	{
		if (pTxnNode->timerDetails[index].isRunning == Enum_FALSE)
		{
			break;
		}
	}

	return index;
}

/**************************************************************************************************
Function:            _EcrioTXNGetRunningTimerIds()

Purpose:             This API returns running timer IDs.

Description:.        This internal function returns running timer IDs.

Input:				 _EcrioTXNDataStruct			*pTxnData
                     _EcrioTXNTxnNodeStruct		*pTxnNode

OutPut:              u_int16					*pRunningTimerCount.

Returns:             u_int32*.
**************************************************************************************************/
u_int32 *_EcrioTXNGetRunningTimerIds
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode,
	u_int16	*pRunningTimerCount
)
{
	u_int16	index = 0;
	u_int16	timerCount = 0;
	u_int32	*pTimerIds = NULL;
	u_int32	timerIDArray[TXN_NODE_TIMER_ARRAYSIZE_MAX] = {0};

	*pRunningTimerCount = 0;

	for (index = 0; index < TXN_NODE_TIMER_ARRAYSIZE_MAX; index++)
	{
		if (pTxnNode->timerDetails[index].isRunning == Enum_TRUE)
		{
			timerIDArray[timerCount++] = _EcrioTXNGetDummyTimerID(pTxnData, pTxnNode->timerDetails[index].sipTimerType);
		}
	}

	pal_MemoryAllocate(timerCount * sizeof(u_int32), (void **)&(pTimerIds));
	if (!pTimerIds)
	{
		return NULL;
	}

	for (index = 0; index < timerCount; index++)
	{
		pTimerIds[index] = timerIDArray[index];
	}

	*pRunningTimerCount = timerCount;

	return pTimerIds;
}

/**************************************************************************************************
Function:            _EcrioTXNGetTxnStateName()

Purpose:             This API returns state in text.

Description:.        This internal function returns transaction state in String format ageinst
                     input parameter txnState.

Input:				 EcrioTXNTxnStateEnum	txnState

OutPut:              None

Returns:             u_char*.
**************************************************************************************************/
u_char *_EcrioTXNGetTxnStateName
(
	EcrioTXNTxnStateEnum txnState
)
{
	u_char *pTxnStateName = NULL;

	switch (txnState)
	{
		case EcrioTXNTxnStateNone:
		{
			pTxnStateName = ECRIO_TXN_TXN_STATE_NAME_NONE;
			break;
		}

		case EcrioTXNTxnStateCalling:
		{
			pTxnStateName = ECRIO_TXN_TXN_STATE_NAME_CALLING;
			break;
		}

		case EcrioTXNTxnStateTrying:
		{
			pTxnStateName = ECRIO_TXN_TXN_STATE_NAME_TRYING;
			break;
		}

		case EcrioTXNTxnStateProceeding:
		{
			pTxnStateName = ECRIO_TXN_TXN_STATE_NAME_PROCEEDING;
			break;
		}

		case EcrioTXNTxnStateProgressing:
		{
			pTxnStateName = ECRIO_TXN_TXN_STATE_NAME_PROGRESSING;
			break;
		}

		case EcrioTXNTxnStateCompleted:
		{
			pTxnStateName = ECRIO_TXN_TXN_STATE_NAME_COMPLETED;
			break;
		}

		case EcrioTXNTxnStateConfirmed:
		{
			pTxnStateName = ECRIO_TXN_TXN_STATE_NAME_CONFIRMED;
			break;
		}

		case EcrioTXNTxnStateTerminated:
		{
			pTxnStateName = ECRIO_TXN_TXN_STATE_NAME_TERMINATED;
			break;
		}
	}

	return pTxnStateName;
}

/**************************************************************************************************
Function:            _EcrioTXNGetTxnName()

Purpose:             This API returns transaction name in text.

Description:.        This internal function returns transaction name in String format ageinst
                     input parameter txnIdentifier.

Input:				 EcrioTXNTxnIdentificationStruct		txnIdentifier,
                     u_char									*pTxnName

OutPut:              None

Returns:             u_char*.
**************************************************************************************************/
u_char *_EcrioTXNGetTxnName
(
	EcrioTXNTxnIdentificationStruct	txnIdentifier
)
{
	u_char *pTxnNameStr = NULL;

	switch (txnIdentifier.pMethodName[0])
	{
		case 'R':
		{
			pTxnNameStr = ECRIO_TXN_TXN_TYPE_NAME_REGISTER_CLIENT;
		}
		break;

		case 'M':
		{
			pTxnNameStr = (txnIdentifier.txnType == EcrioTXNTxnTypeClientNonInvite) ? ECRIO_TXN_TXN_TYPE_NAME_MESSAGE_CLIENT : ECRIO_TXN_TXN_TYPE_NAME_MESSAGE_SERVER;
		}
		break;

		case 'B':
		{
			pTxnNameStr = (txnIdentifier.txnType == EcrioTXNTxnTypeClientNonInvite) ? ECRIO_TXN_TXN_TYPE_NAME_BYE_CLIENT : ECRIO_TXN_TXN_TYPE_NAME_BYE_SERVER;
		}
		break;

		case 'C':
		{
			pTxnNameStr = (txnIdentifier.txnType == EcrioTXNTxnTypeClientNonInvite) ? ECRIO_TXN_TXN_TYPE_NAME_CANCEL_CLIENT : ECRIO_TXN_TXN_TYPE_NAME_CANCEL_SERVER;
		}
		break;

		case 'I':
		{
			pTxnNameStr = (txnIdentifier.txnType == EcrioTXNTxnTypeClientInvite) ? ECRIO_TXN_TXN_TYPE_NAME_INVITE_CLIENT : ECRIO_TXN_TXN_TYPE_NAME_INVITE_SERVER;
		}
		break;
	}

	return pTxnNameStr;
}

/**************************************************************************************************
Function:            _EcrioTXNGetSipTimerName()

Purpose:             This API returns SIP timer name in text.

Description:.        This internal function returns SIP timer name in String format ageinst
                     input parameter sipTimerType.

Input:				 _EcrioTXNDataStruct			*pTxnData
                     EcrioTXNSipTimerTypeEnum	sipTimerType

OutPut:              None

Returns:             u_char*.
**************************************************************************************************/
u_char *_EcrioTXNGetSipTimerName
(
	_EcrioTXNDataStruct	*pTxnData,
	EcrioTXNSipTimerTypeEnum sipTimerType
)
{
	u_char *pTimerName = NULL;

	// @todo Do we really need pTxnData in this function?
	(void)pTxnData;

	if (sipTimerType == EcrioTXNSipTimerTypeA)
	{
		pTimerName = ECRIO_TXN_SIP_TIMER_A_NAME;
	}
	else if (sipTimerType == EcrioTXNSipTimerTypeB)
	{
		pTimerName = ECRIO_TXN_SIP_TIMER_B_NAME;
	}
	else if (sipTimerType == EcrioTXNSipTimerTypeD)
	{
		pTimerName = ECRIO_TXN_SIP_TIMER_D_NAME;
	}
	else if (sipTimerType == EcrioTXNSipTimerTypeE)
	{
		pTimerName = ECRIO_TXN_SIP_TIMER_E_NAME;
	}
	else if (sipTimerType == EcrioTXNSipTimerTypeF)
	{
		pTimerName = ECRIO_TXN_SIP_TIMER_F_NAME;
	}
	else if (sipTimerType == EcrioTXNSipTimerTypeG)
	{
		pTimerName = ECRIO_TXN_SIP_TIMER_G_NAME;
	}
	else if (sipTimerType == EcrioTXNSipTimerTypeH)
	{
		pTimerName = ECRIO_TXN_SIP_TIMER_H_NAME;
	}
	else if (sipTimerType == EcrioTXNSipTimerTypeI)
	{
		pTimerName = ECRIO_TXN_SIP_TIMER_I_NAME;
	}
	else if (sipTimerType == EcrioTXNSipTimerTypeJ)
	{
		pTimerName = ECRIO_TXN_SIP_TIMER_J_NAME;
	}
	else if (sipTimerType == EcrioTXNSipTimerTypeK)
	{
		pTimerName = ECRIO_TXN_SIP_TIMER_K_NAME;
	}

	return pTimerName;
}

/**************************************************************************************************
Function:            _EcrioTXNGetSipTimerType()

Purpose:             This API returns SIP timer type.

Description:.        This internal function returns SIP timer Type.

Input:				 _EcrioTXNDataStruct			*pTxnData
                     _EcrioTXNTxnNodeStruct		*pTxnNode,
                     EcrioTXNTimerTypeEnum		timerType,
                     BoolEnum					bCheckTransState

OutPut:              EcrioTXNSipTimerTypeEnum	*pSipTimerType

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum _EcrioTXNGetSipTimerType
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode,
	EcrioTXNTimerTypeEnum timerType,
	BoolEnum bCheckTransState,
	EcrioTXNSipTimerTypeEnum *pSipTimerType
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;

	void *pLogHandle = NULL;

	if (!pTxnData)
	{
		return EcrioTXNInsufficientDataError;
	}

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

	if ((!pTxnNode) || (!pSipTimerType))
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tInsufficient Data", __FUNCTION__, __LINE__);
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

// quiet	TXNMGRLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

// quiet	TXNMGRLOGI(pLogHandle, KLogTypeFuncParams,
// quiet		"%s:%u\tpTxnData=%p, pTxnNode=%p, TimerType=%u",
// quiet		__FUNCTION__, __LINE__, pTxnData, pTxnNode, timerType);

	*pSipTimerType = EcrioTXNSipTimerTypeNone;

	switch (pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier.txnType)
	{
		case EcrioTXNTxnTypeClientInvite:
		{
			if (timerType & EcrioTXNRetransmitTimer)
			{
				if ((bCheckTransState == Enum_FALSE)
					|| (pTxnNode->txnState == EcrioTXNTxnStateNone)
					|| (pTxnNode->txnState == EcrioTXNTxnStateCalling))
				{
					*pSipTimerType |= EcrioTXNSipTimerTypeA;
				}
			}

			if (timerType & EcrioTXNTimeoutTimer)
			{
				if ((bCheckTransState == Enum_FALSE)
					|| (pTxnNode->txnState == EcrioTXNTxnStateNone)
					|| (pTxnNode->txnState == EcrioTXNTxnStateCalling))
				{
					*pSipTimerType |= EcrioTXNSipTimerTypeB;
				}
			}

			if (timerType & EcrioTXNWaitTimer)
			{
				if ((bCheckTransState == Enum_FALSE)
					|| (pTxnNode->txnState == EcrioTXNTxnStateProgressing)
					|| (pTxnNode->txnState == EcrioTXNTxnStateCompleted))
				{
					*pSipTimerType |= EcrioTXNSipTimerTypeD;
				}
			}

			break;
		}	/* case EcrioTXNTxnTypeClientInvite */

		case EcrioTXNTxnTypeServerInvite:
		{
			if (timerType & EcrioTXNRetransmitTimer)
			{
				if ((bCheckTransState == Enum_FALSE)
					|| (pTxnNode->txnState == EcrioTXNTxnStateProceeding)
					|| (pTxnNode->txnState == EcrioTXNTxnStateCompleted))
				{
					*pSipTimerType |= EcrioTXNSipTimerTypeG;
				}
			}

			if (timerType & EcrioTXNTimeoutTimer)
			{
				if ((bCheckTransState == Enum_FALSE)
					|| (pTxnNode->txnState == EcrioTXNTxnStateProceeding)
					|| (pTxnNode->txnState == EcrioTXNTxnStateCompleted))
				{
					*pSipTimerType |= EcrioTXNSipTimerTypeH;
				}
			}

			if (timerType & EcrioTXNWaitTimer)
			{
				if ((bCheckTransState == Enum_FALSE)
					|| (pTxnNode->txnState == EcrioTXNTxnStateConfirmed))
				{
					*pSipTimerType |= EcrioTXNSipTimerTypeI;
				}
			}

			break;
		}	/* case EcrioTXNTxnTypeServerInvite */

		case EcrioTXNTxnTypeClientNonInvite:
		{
			if (timerType & EcrioTXNRetransmitTimer)
			{
				if ((bCheckTransState == Enum_FALSE)
					|| (pTxnNode->txnState == EcrioTXNTxnStateNone) ||
					(pTxnNode->txnState == EcrioTXNTxnStateTrying)
					|| (pTxnNode->txnState == EcrioTXNTxnStateProceeding))
				{
					*pSipTimerType |= EcrioTXNSipTimerTypeE;
				}
			}

			if (timerType & EcrioTXNTimeoutTimer)
			{
				if ((bCheckTransState == Enum_FALSE)
					|| (pTxnNode->txnState == EcrioTXNTxnStateNone)
					|| (pTxnNode->txnState == EcrioTXNTxnStateTrying)
					|| (pTxnNode->txnState == EcrioTXNTxnStateProceeding))
				{
					*pSipTimerType |= EcrioTXNSipTimerTypeF;
				}
			}

			if (timerType & EcrioTXNWaitTimer)
			{
				if ((bCheckTransState == Enum_FALSE)
					|| (pTxnNode->txnState == EcrioTXNTxnStateCompleted))
				{
					*pSipTimerType |= EcrioTXNSipTimerTypeK;
				}
			}

			break;
		}	/* case EcrioTXNTxnTypeClientNonInvite */

		case EcrioTXNTxnTypeServerNonInvite:
		{
			if (timerType & EcrioTXNWaitTimer)
			{
				if ((bCheckTransState == Enum_FALSE)
					|| (pTxnNode->txnState == EcrioTXNTxnStateCompleted))
				{
					*pSipTimerType |= EcrioTXNSipTimerTypeJ;
				}
			}

			break;
		}	/* case EcrioTXNTxnTypeServerNonInvite */

		default:
		{
		}
		break;
	}

EndTag:
// quiet	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\ttxnRetCode=%u", __FUNCTION__, __LINE__, txnRetCode);

	return txnRetCode;
}

/**************************************************************************************************
Function:            _EcrioTXNGetTimerType()

Purpose:             This API returns Transaction Timer type.

Description:.        This internal function returns Transaction timer Type.

Input:				 _EcrioTXNDataStruct			*pTxnData
                     EcrioTXNSipTimerTypeEnum	sipTimerType

OutPut:              EcrioTXNTimerTypeEnum		*pTimerType

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum _EcrioTXNGetTimerType
(
	_EcrioTXNDataStruct	*pTxnData,
	EcrioTXNSipTimerTypeEnum sipTimerType,
	EcrioTXNTimerTypeEnum *pTimerType
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;

	void *pLogHandle = NULL;

	if (!pTxnData)
	{
		return EcrioTXNInsufficientDataError;
	}

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

	if (!pTimerType)
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tInsufficient Data", __FUNCTION__, __LINE__);
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	TXNMGRLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	TXNMGRLOGI(pLogHandle, KLogTypeFuncParams,
		"%s:%u\tpTxnData=%p, sipTimerType=%s",
		__FUNCTION__, __LINE__, pTxnData, _EcrioTXNGetSipTimerName(pTxnData, sipTimerType));

	*pTimerType = EcrioTXNNoneTimer;

	if ((sipTimerType & EcrioTXNSipTimerTypeA)
		|| (sipTimerType & EcrioTXNSipTimerTypeE)
		|| (sipTimerType & EcrioTXNSipTimerTypeG))
	{
		*pTimerType |= EcrioTXNRetransmitTimer;
	}

	if ((sipTimerType & EcrioTXNSipTimerTypeB)
		|| (sipTimerType & EcrioTXNSipTimerTypeF)
		|| (sipTimerType & EcrioTXNSipTimerTypeH))
	{
		*pTimerType |= EcrioTXNTimeoutTimer;
	}

	if ((sipTimerType & EcrioTXNSipTimerTypeD)
		|| (sipTimerType & EcrioTXNSipTimerTypeI)
		|| (sipTimerType & EcrioTXNSipTimerTypeJ)
		|| (sipTimerType & EcrioTXNSipTimerTypeK))
	{
		*pTimerType |= EcrioTXNWaitTimer;
	}

EndTag:
	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\ttxnRetCode=%u", __FUNCTION__, __LINE__, txnRetCode);

	return txnRetCode;
}

/**************************************************************************************************
Function:            _EcrioTXNGenerateTUNotificationData()

Purpose:             This API creates transaction user notification data.

Description:.        This internal function creates transaction user notification data which will be passed
                     transaction user layer.

Input:				 _EcrioTXNDataStruct			*pTxnData
                     _EcrioTXNTxnNodeStruct				*pTxnNode
                     BoolEnum							bUseAckParam

OutPut:              EcrioTXNTUNotificationDataStruct	**ppTxnTuNotificationData

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum _EcrioTXNGenerateTUNotificationData
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode,
	EcrioTXNTUNotificationDataStruct **ppTxnTuNotificationData,
	BoolEnum bUseAckParam
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	EcrioTXNTUNotificationDataStruct *pTxnTuNotificationData = NULL;
	void *pLogHandle = NULL;

	bUseAckParam = bUseAckParam;

	if (!pTxnData)
	{
		return EcrioTXNInsufficientDataError;
	}

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

// quiet	TXNMGRLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

// quiet	TXNMGRLOGI(pLogHandle, KLogTypeFuncParams,
// quiet		"%s:%u\tpTxnData=%p, pTxnNode=%p",
// quiet		__FUNCTION__, __LINE__, pTxnData, pTxnNode);

	if ((!pTxnNode) || (!ppTxnTuNotificationData))
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tInsufficient Data", __FUNCTION__, __LINE__);
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	pal_MemoryAllocate(sizeof(EcrioTXNTUNotificationDataStruct), (void **)ppTxnTuNotificationData);

	pTxnTuNotificationData = *ppTxnTuNotificationData;

	if (!pTxnTuNotificationData)
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() failed", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNInsufficientMemoryError;
		goto EndTag;
	}

	pTxnTuNotificationData->pRunningTimerIds = _EcrioTXNGetRunningTimerIds(pTxnData,
		pTxnNode,
		&(pTxnTuNotificationData->noOfRunningTimers));
	if (bUseAckParam == Enum_TRUE)
	{
		txnRetCode = _EcrioTXNAllocateCopyTxnMatchingParam(pTxnData,
			pTxnNode->pTxnMatchingParamACK,
			&(pTxnTuNotificationData->pTxnMatchingParam));
	}
	else
	{
		txnRetCode = _EcrioTXNAllocateCopyTxnMatchingParam(pTxnData,
			pTxnNode->pTxnMatchingParam,
			&(pTxnTuNotificationData->pTxnMatchingParam));
	}

	if (txnRetCode != EcrioTXNNoError)
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tUnable to copy matching param", __FUNCTION__, __LINE__);
		goto EndTag;
	}

	txnRetCode = _EcrioTXNAllocateCopySessionTxnMappingParam(pTxnData,
		pTxnNode->pSessionTxnMappingParam,
		&(pTxnTuNotificationData->pSessionTxnMappingParam));

	if (txnRetCode != EcrioTXNNoError)
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tUnable to copy session matching param", __FUNCTION__, __LINE__);
		goto EndTag;
	}

EndTag:

	if ((txnRetCode != EcrioTXNNoError) && (ppTxnTuNotificationData) && (*ppTxnTuNotificationData))
	{
		_EcrioTXNStructRelease(pTxnData,
			(void **)ppTxnTuNotificationData,
			_EcrioTXNStructType_EcrioTXNTUNotificationDataStruct,
			Enum_TRUE);
	}

// quiet	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\ttxnRetCode=%u", __FUNCTION__, __LINE__, txnRetCode);
	return txnRetCode;
}

/**************************************************************************************************
Function:            _EcrioTXNHandleTimerExpiryRetrans()

Purpose:             This API handles retransmit timer expiry.

Description:.        This internal function handles retransmit timer expiry and sends the request
                     to network through SAC.

Input:				 _EcrioTXNDataStruct			*pTxnData
                     _EcrioTXNTxnNodeStruct				*pTxnNode

OutPut:              None

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum _EcrioTXNHandleTimerExpiryRetrans
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	u_int32	socketError = KPALErrorNone;
	u_int32	timerIndex = 0;

	// s_int32	context = 0;
	/*	Modification for SUE	BEGIN	*/
	u_int32	uChannelIndex = 0;
	/*	Modification for SUE	END		*/
	void *pLogHandle = NULL;

	if (pTxnData == NULL || (pTxnData->pTxnInitParam) == NULL)
	{
		return EcrioTXNInsufficientDataError;
	}

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;
	TXNMGRLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	TXNMGRLOGI(pLogHandle, KLogTypeFuncParams,
		"%s:%u\tpTxnData=%p, pTxnNode=%p",
		__FUNCTION__, __LINE__, pTxnData, pTxnNode);

	if (!pTxnNode)
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tInsufficient Data", __FUNCTION__, __LINE__);
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	/*	Modification for SUE	BEGIN	*/
	if ((pTxnData->pTxnInitParam->pTxnTransportStruct) == NULL)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}
	else
	{
		if (ECRIO_TXN_MAX_NO_OF_CHANNELS == pTxnNode->uChannelIndex)
		{
			uChannelIndex = ((EcrioTxnMgrTransportStruct *)(pTxnData->pTxnInitParam->pTxnTransportStruct))->sendingChannelIndex;
		}
		else
		{
			uChannelIndex = pTxnNode->uChannelIndex;
		}
	}

	/*	Modification for SUE	END		*/

	if (pTxnNode->bRetryEnabled == Enum_TRUE)
	{
		if ((pTxnNode->msgLen > 0) && (pTxnNode->pMsg))
		{
			// context = pTxnNode->retransmissionContext;

			u_int32 resCode = 0;
#ifdef ENABLE_QCMAPI
			EcrioTXNMessageTypeEnum	messageType = EcrioTXNMessageTypeNone;
#endif
			resCode = pTxnNode->pTxnMatchingParam->commonMatchingParam.responseCode;
			if ((resCode < 700) && ((700 - resCode) <= 600))
			{
#ifdef ENABLE_QCMAPI
				messageType = EcrioTXNMessageTypeResponse;
#endif
				TXNMGRLOGI(pLogHandle, KLogTypeGeneral,
					"%s:%u\tRetransmitting %s Response",
					__FUNCTION__, __LINE__, pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier.pMethodName);
			}
			else
			{
#ifdef ENABLE_QCMAPI
				messageType = EcrioTXNMessageTypeRequest;
#endif
				TXNMGRLOGI(pLogHandle, KLogTypeGeneral,
					"%s:%u\tRetransmitting %s Request",
					__FUNCTION__, __LINE__, pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier.pMethodName);
			}

#ifdef ENABLE_QCMAPI
			socketError = _EcrioTxnMgrSendDataToCommunicationChannel(pTxnData, pTxnNode->pMsg, pTxnNode->msgLen, uChannelIndex, messageType, pTxnNode->pTxnMatchingParam->commonMatchingParam.pCallId, Enum_FALSE);
#else
			socketError = _EcrioTxnMgrSendDataToCommunicationChannel(pTxnData, pTxnNode->pMsg, pTxnNode->msgLen, uChannelIndex);
#endif
			if (socketError != EcrioTXNNoError)
			{
				TXNMGRLOGE(pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioTxnMgrSendDataToCommunicationChannel() socketError=%u",
					__FUNCTION__, __LINE__, socketError);
			}

			txnRetCode = _EcrioTXNStartTxnTimer(pTxnData,
				pTxnNode,
				EcrioTXNRetransmitTimer);

			if (txnRetCode != EcrioTXNNoError)
			{
				TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\t_EcrioTXNStartTxnTimer() txnRetCode=%u",
					__FUNCTION__, __LINE__, txnRetCode);

				goto EndTag;
			}
		}
	}
	else
	{
		EcrioTXNSipTimerTypeEnum sipTimerType = EcrioTXNSipTimerTypeNone;
		txnRetCode = _EcrioTXNGetSipTimerType(pTxnData,
			pTxnNode,
			EcrioTXNRetransmitTimer,
			Enum_FALSE,
			&sipTimerType);

		if (txnRetCode != EcrioTXNNoError)
		{
			goto EndTag;
		}

		timerIndex = _EcrioTXNGetRunningTimerIndex(pTxnNode, sipTimerType);

		if (timerIndex >= TXN_NODE_TIMER_ARRAYSIZE_MAX)
		{
			TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tNo data for running timer available", __FUNCTION__, __LINE__);

			txnRetCode = EcrioTXNTimerError;
			goto EndTag;
		}

		pTxnNode->timerDetails[timerIndex].isRunning = Enum_FALSE;
		pTxnNode->timerDetails[timerIndex].absoluteDuration	= 0;
		pTxnNode->timerDetails[timerIndex].timerID = 0;
		pTxnNode->timerDetails[timerIndex].sipTimerType	= EcrioTXNSipTimerTypeNone;
	}

EndTag:

	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\ttxnRetCode=%u", __FUNCTION__, __LINE__, txnRetCode);
	return txnRetCode;
}

/**************************************************************************************************
Function:            _EcrioTXNGetDummyTimerID()

Purpose:             This API returns dummy timer ID.

Description:.        This internal function returns dummy timer ID based on input parameter sipTimer.

Input:				 _EcrioTXNDataStruct			*pTxnData
                     EcrioTXNSipTimerTypeEnum	sipTimer

OutPut:              None

Returns:             u_int32.
**************************************************************************************************/
u_int32 _EcrioTXNGetDummyTimerID
(
	_EcrioTXNDataStruct	*pTxnData,
	EcrioTXNSipTimerTypeEnum sipTimer
)
{
	u_int32 dummyTimerID = ECRIO_TXN_INVALID_DUMMY_ID;

	switch (sipTimer)
	{
		case EcrioTXNSipTimerTypeA:
		{
			dummyTimerID = pTxnData->pTxnInitParam->timerIds.timerAId;
			break;
		}

		case EcrioTXNSipTimerTypeB:
		{
			dummyTimerID = pTxnData->pTxnInitParam->timerIds.timerBId;
			break;
		}

		case EcrioTXNSipTimerTypeD:
		{
			dummyTimerID = pTxnData->pTxnInitParam->timerIds.timerDId;
			break;
		}

		case EcrioTXNSipTimerTypeE:
		{
			dummyTimerID = pTxnData->pTxnInitParam->timerIds.timerEId;
			break;
		}

		case EcrioTXNSipTimerTypeF:
		{
			dummyTimerID = pTxnData->pTxnInitParam->timerIds.timerFId;
			break;
		}

		case EcrioTXNSipTimerTypeG:
		{
			dummyTimerID = pTxnData->pTxnInitParam->timerIds.timerGId;
			break;
		}

		case EcrioTXNSipTimerTypeH:
		{
			dummyTimerID = pTxnData->pTxnInitParam->timerIds.timerHId;
			break;
		}

		case EcrioTXNSipTimerTypeI:
		{
			dummyTimerID = pTxnData->pTxnInitParam->timerIds.timerIId;
			break;
		}

		case EcrioTXNSipTimerTypeJ:
		{
			dummyTimerID = pTxnData->pTxnInitParam->timerIds.timerJId;
			break;
		}

		case EcrioTXNSipTimerTypeK:
		{
			dummyTimerID = pTxnData->pTxnInitParam->timerIds.timerKId;
			break;
		}

		default:
		{
			break;
		}
	}

	return dummyTimerID;
}
