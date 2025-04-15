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

/**************************************************************************************************
Function:            _EcrioTXNTimerACallback()

Purpose:             This is a callback function.

Description:.        This callback function fires when Timer-A fires for a transaction.

Input:				 u_int32 timerID
                     void *pCallbackData

OutPut:              None.

Returns:             void.
**************************************************************************************************/
void _EcrioTXNTimerACallback
(
	void *pCallbackData,
	TIMERHANDLE timerID
)
{

	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;
	EcrioTXNTUNotificationDataStruct *pTxnTuNotificationData = NULL;
	EcrioTXNAppCBStruct	*pTxnAppCBData = NULL;

	/* Stop the timer before proceeding further. */
	pal_TimerStop(timerID);

	if (!pCallbackData)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		return;
	}

	pTxnNode = (_EcrioTXNTxnNodeStruct *)pCallbackData;
	pTxnData = (_EcrioTXNDataStruct *)pTxnNode->pTxnData;

	if (pTxnData == NULL || pTxnData->pTxnInitParam == NULL)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		return;
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncParams, "%s:%u\tpCallbackData=%p, timerID=%u",
		__FUNCTION__, __LINE__, pCallbackData, timerID);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeMutex, "%s:%u\tLocking pTxnMutex", __FUNCTION__, __LINE__);

	TXNMGRLOGV(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\tStart processing %s (%p)", __FUNCTION__, __LINE__,
		_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier), pTxnNode);

	txnRetCode = _EcrioTXNGenerateTUNotificationData(pTxnData, pTxnNode, &pTxnTuNotificationData, Enum_FALSE);

	if (txnRetCode != EcrioTXNNoError)
	{
		goto EndTag;
	}

	if (pTxnNode->txnState != EcrioTXNTxnStateCalling)
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioTXNGenerateTUNotificationData() Current Transaction state %s of the Transaction Node %p is not valid for this callback, txnRetCode=%u",
			__FUNCTION__, __LINE__, _EcrioTXNGetTxnStateName(pTxnNode->txnState), pTxnNode, txnRetCode);

		goto EndTag;
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\tTimer A expires - Retransmitting Request", __FUNCTION__, __LINE__);
	txnRetCode = _EcrioTXNHandleTimerExpiryRetrans(pTxnData, pTxnNode);

	if (txnRetCode != EcrioTXNNoError)
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioTXNHandleTimerExpiryRetrans() Retransmitting Request, txnRetCode=%u",
			__FUNCTION__, __LINE__, txnRetCode);
		goto EndTag;
	}

	pal_MemoryAllocate(sizeof(EcrioTXNAppCBStruct), (void **)&(pTxnAppCBData));
	if (!pTxnAppCBData)
	{
		txnRetCode = EcrioTXNInsufficientMemoryError;
		goto EndTag;
	}

	pTxnAppCBData->pTxnCBData = (void *)pTxnTuNotificationData;
	pTxnAppCBData->pAppCBData = pTxnData->pTxnInitParam->pTimerNotificationData;

	(*pTxnData->pTxnInitParam->timerExpiryNotificationFunctions.pTimerANotificationFn)((void *)pTxnAppCBData);

EndTag:

	TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\tEnd processing txn node %p", __FUNCTION__, __LINE__, pTxnNode);

	if (txnRetCode != EcrioTXNNoError)
	{
		EcrioTUNotifyError(txnRetCode, (void *)pTxnTuNotificationData);
	}

	if (pTxnTuNotificationData)
	{
		_EcrioTXNStructRelease(pTxnData,
			(void **)&(pTxnTuNotificationData),
			_EcrioTXNStructType_EcrioTXNTUNotificationDataStruct,
			Enum_TRUE);
	}

	if (pTxnAppCBData)
	{
		pal_MemoryFree((void **)&(pTxnAppCBData));
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
}

/**************************************************************************************************
Function:            _EcrioTXNTimerBCallback()

Purpose:             This is a callback function.

Description:.        This callback function fires when Timer-B fires for a transaction.

Input:				 u_int32 timerID
                     void *pCallbackData

OutPut:              None.

Returns:             void.
**************************************************************************************************/
void _EcrioTXNTimerBCallback
(
	void *pCallbackData,
	TIMERHANDLE timerID
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;
	EcrioTXNTUNotificationDataStruct *pTxnTuNotificationData = NULL;
	EcrioTXNAppCBStruct	*pTxnAppCBData = NULL;
	u_int32 index = 0;

	/* Stop the timer before proceeding further. */
	pal_TimerStop(timerID);

	if (!pCallbackData)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		return;
	}

	pTxnNode = (_EcrioTXNTxnNodeStruct *)pCallbackData;
	pTxnData = (_EcrioTXNDataStruct *)pTxnNode->pTxnData;

	if (pTxnData == NULL || pTxnData->pTxnInitParam == NULL)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		return;
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncParams, "%s:%u\tpCallbackData=%p, timerID=%u",
		__FUNCTION__, __LINE__, pCallbackData, timerID);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\tStart processing %s (%p)",
		__FUNCTION__, __LINE__, _EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier), pTxnNode);

	/* Check and update timer details. */
	index = _EcrioTXNGetRunningTimerIndex(pTxnNode, EcrioTXNSipTimerTypeB);

	if (index >= TXN_NODE_TIMER_ARRAYSIZE_MAX)
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioTXNGetRunningTimerIndex() no timer of type %u running on node %p",
			__FUNCTION__, __LINE__, EcrioTXNSipTimerTypeB, pTxnNode);

		txnRetCode = EcrioTXNTimerError;
		goto EndTag;
	}

	pTxnNode->timerDetails[index].isRunning = Enum_FALSE;
	pTxnNode->timerDetails[index].absoluteDuration = 0;
	pTxnNode->timerDetails[index].timerID = NULL;
	pTxnNode->timerDetails[index].sipTimerType = EcrioTXNSipTimerTypeNone;

	txnRetCode = _EcrioTXNGenerateTUNotificationData(pTxnData, pTxnNode, &pTxnTuNotificationData, Enum_FALSE);

	if (txnRetCode != EcrioTXNNoError)
	{
		goto EndTag;
	}

	if (pTxnNode->txnState != EcrioTXNTxnStateCalling)
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioTXNGenerateTUNotificationData() Current Transaction state %s of the Transaction Node %p is not valid for this callback",
			__FUNCTION__, __LINE__, _EcrioTXNGetTxnStateName(pTxnNode->txnState), pTxnNode);

		goto EndTag;
	}

	TXNMGRLOGV(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\ttxnState is transiting from %s to %s of the transaction %s",
		__FUNCTION__, __LINE__, _EcrioTXNGetTxnStateName(pTxnNode->txnState), _EcrioTXNGetTxnStateName(EcrioTXNTxnStateTerminated),
		_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

	/*	txn state change	*/
	pTxnNode->txnState = EcrioTXNTxnStateTerminated;

	TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\tTimer B expires. Request Failed", __FUNCTION__, __LINE__);
	/*	txn node deletion	*/
	_EcrioTXNDeleteTxn(pTxnData, pTxnNode);
	pTxnNode = NULL;

	pal_MemoryAllocate(sizeof(EcrioTXNAppCBStruct), (void **)&(pTxnAppCBData));
	if (!pTxnAppCBData)
	{
		txnRetCode = EcrioTXNInsufficientMemoryError;
		goto EndTag;
	}

	pTxnAppCBData->pTxnCBData = (void *)pTxnTuNotificationData;
	pTxnAppCBData->pAppCBData = pTxnData->pTxnInitParam->pTimerNotificationData;

	(*pTxnData->pTxnInitParam->timerExpiryNotificationFunctions.pTimerBNotificationFn)((void *)pTxnAppCBData);

EndTag:

	TXNMGRLOGV(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\tEnd processing txn node %p", __FUNCTION__, __LINE__, pTxnNode);

	if (txnRetCode != EcrioTXNNoError)
	{
		EcrioTUNotifyError(txnRetCode, (void *)pTxnTuNotificationData);
	}

	if (pTxnTuNotificationData)
	{
		_EcrioTXNStructRelease(pTxnData,
			(void **)&(pTxnTuNotificationData),
			_EcrioTXNStructType_EcrioTXNTUNotificationDataStruct,
			Enum_TRUE);
	}

	if (pTxnAppCBData)
	{
		pal_MemoryFree((void **)&(pTxnAppCBData));
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
}

/**************************************************************************************************
Function:            _EcrioTXNTimerDCallback()

Purpose:             This is a callback function.

Description:.        This callback function fires when Timer-D fires for a transaction.

Input:				 u_int32 timerID
                     void *pCallbackData

OutPut:              None.

Returns:             void.
**************************************************************************************************/
void _EcrioTXNTimerDCallback
(
	void *pCallbackData,
	TIMERHANDLE timerID
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;
	EcrioTXNTUNotificationDataStruct *pTxnTuNotificationData = NULL;
	EcrioTXNAppCBStruct	*pTxnAppCBData = NULL;
	u_int32 index = 0;

	/* Stop the timer before proceeding further. */
	pal_TimerStop(timerID);

	if (!pCallbackData)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		return;
	}

	pTxnNode = (_EcrioTXNTxnNodeStruct *)pCallbackData;
	pTxnData = (_EcrioTXNDataStruct *)pTxnNode->pTxnData;

	if (pTxnData == NULL || pTxnData->pTxnInitParam == NULL)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		return;
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncParams, "%s:%u\tpCallbackData=%p, timerID=%u",
		__FUNCTION__, __LINE__, pCallbackData, timerID);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\tStart processing %s (%p)",
		__FUNCTION__, __LINE__, _EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier), pTxnNode);

	/* Check and update timer details. */
	index = _EcrioTXNGetRunningTimerIndex(pTxnNode, EcrioTXNSipTimerTypeD);

	if (index >= TXN_NODE_TIMER_ARRAYSIZE_MAX)
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioTXNGetRunningTimerIndex() no timer of type %u running on node %p",
			__FUNCTION__, __LINE__, EcrioTXNSipTimerTypeD, pTxnNode);

		txnRetCode = EcrioTXNTimerError;
		goto EndTag;
	}

	pTxnNode->timerDetails[index].isRunning = Enum_FALSE;
	pTxnNode->timerDetails[index].absoluteDuration = 0;
	pTxnNode->timerDetails[index].timerID = NULL;
	pTxnNode->timerDetails[index].sipTimerType = EcrioTXNSipTimerTypeNone;

	txnRetCode = _EcrioTXNGenerateTUNotificationData(pTxnData, pTxnNode, &pTxnTuNotificationData, Enum_TRUE);

	if (txnRetCode != EcrioTXNNoError)
	{
		goto EndTag;
	}

	if (pTxnNode->txnState != EcrioTXNTxnStateCompleted)
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioTXNGenerateTUNotificationData() Current Transaction state %s of the Transaction Node %p is not valid for this callback, txnRetCode=%u",
			__FUNCTION__, __LINE__, _EcrioTXNGetTxnStateName(pTxnNode->txnState), pTxnNode, txnRetCode);

		goto EndTag;
	}

	TXNMGRLOGV(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\ttxnState is transiting from %s to %s of the transaction %s",
		__FUNCTION__, __LINE__, _EcrioTXNGetTxnStateName(pTxnNode->txnState), _EcrioTXNGetTxnStateName(EcrioTXNTxnStateTerminated),
		_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));
	/*	txn state change	*/
	pTxnNode->txnState = EcrioTXNTxnStateTerminated;

	TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\tTimer D expires", __FUNCTION__, __LINE__);
	/*	txn node deletion	*/
	_EcrioTXNDeleteTxn(pTxnData, pTxnNode);
	pTxnNode = NULL;

	pal_MemoryAllocate(sizeof(EcrioTXNAppCBStruct), (void **)&(pTxnAppCBData));
	if (!pTxnAppCBData)
	{
		txnRetCode = EcrioTXNInsufficientMemoryError;
		goto EndTag;
	}

	pTxnAppCBData->pTxnCBData = (void *)pTxnTuNotificationData;
	pTxnAppCBData->pAppCBData = pTxnData->pTxnInitParam->pTimerNotificationData;

	(*pTxnData->pTxnInitParam->timerExpiryNotificationFunctions.pTimerDNotificationFn)((void *)pTxnAppCBData);

EndTag:

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\tEnd processing txn node %p", __FUNCTION__, __LINE__, pTxnNode);

	if (pTxnTuNotificationData)
	{
		_EcrioTXNStructRelease(pTxnData,
			(void **)&(pTxnTuNotificationData),
			_EcrioTXNStructType_EcrioTXNTUNotificationDataStruct,
			Enum_TRUE);
	}

	if (txnRetCode != EcrioTXNNoError)
	{
		EcrioTUNotifyError(txnRetCode, (void *)pTxnTuNotificationData);
	}

	if (pTxnTuNotificationData)
	{
		_EcrioTXNStructRelease(pTxnData,
			(void **)&(pTxnTuNotificationData),
			_EcrioTXNStructType_EcrioTXNTUNotificationDataStruct,
			Enum_TRUE);
	}

	if (pTxnAppCBData)
	{
		pal_MemoryFree((void **)&(pTxnAppCBData));
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
}

/**************************************************************************************************
Function:            _EcrioTXNTimerECallback()

Purpose:             This is a callback function.

Description:.        This callback function fires when Timer-E fires for a transaction.

Input:				 u_int32 timerID
                     void *pCallbackData

OutPut:              None.

Returns:             void.
**************************************************************************************************/
void _EcrioTXNTimerECallback
(
	void *pCallbackData,
	TIMERHANDLE timerID
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;
	EcrioTXNTUNotificationDataStruct *pTxnTuNotificationData = NULL;
	EcrioTXNAppCBStruct	*pTxnAppCBData = NULL;

	/* Stop the timer before proceeding further. */
	pal_TimerStop(timerID);

	if (pCallbackData == NULL)
	{
		return;
	}

	pTxnNode = (_EcrioTXNTxnNodeStruct *)pCallbackData;

	pTxnData = (_EcrioTXNDataStruct *)pTxnNode->pTxnData;

	if (pTxnData == NULL || pTxnData->pTxnInitParam == NULL)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		return;
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncParams, "%s:%u\tpCallbackData=%p, timerID=%u",
		__FUNCTION__, __LINE__, pCallbackData, timerID);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\tStart processing %s (%p)",
		__FUNCTION__, __LINE__, _EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier), pTxnNode);

	txnRetCode = _EcrioTXNGenerateTUNotificationData(pTxnData, pTxnNode, &pTxnTuNotificationData, Enum_FALSE);
	if (txnRetCode != EcrioTXNNoError)
	{
		goto EndTag;
	}

	if ((pTxnNode->txnState != EcrioTXNTxnStateTrying) && (pTxnNode->txnState != EcrioTXNTxnStateProceeding))
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioTXNGenerateTUNotificationData() Current Transaction state %s of the Transaction Node %p is not valid for this callback, txnRetCode=%u",
			__FUNCTION__, __LINE__, _EcrioTXNGetTxnStateName(pTxnNode->txnState), pTxnNode, txnRetCode);
		goto EndTag;
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\tTimer E expires - Retransmitting Request",
		__FUNCTION__, __LINE__);

	txnRetCode = _EcrioTXNHandleTimerExpiryRetrans(pTxnData, pTxnNode);
	if (txnRetCode != EcrioTXNNoError)
	{
		TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioTXNHandleTimerExpiryRetrans() Retransmitting Request, txnRetCode=%u",
			__FUNCTION__, __LINE__, txnRetCode);
		goto EndTag;
	}

	pal_MemoryAllocate(sizeof(EcrioTXNAppCBStruct), (void **)&(pTxnAppCBData));
	if (!pTxnAppCBData)
	{
		txnRetCode = EcrioTXNInsufficientMemoryError;
		goto EndTag;
	}

	pTxnAppCBData->pTxnCBData = (void *)pTxnTuNotificationData;
	pTxnAppCBData->pAppCBData = pTxnData->pTxnInitParam->pTimerNotificationData;

	(*pTxnData->pTxnInitParam->timerExpiryNotificationFunctions.pTimerENotificationFn)((void *)pTxnAppCBData);

EndTag:

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\tEnd processing txn node %p",
		__FUNCTION__, __LINE__, pTxnNode);

	if (pTxnTuNotificationData)
	{
		_EcrioTXNStructRelease(pTxnData,
			(void **)&(pTxnTuNotificationData),
			_EcrioTXNStructType_EcrioTXNTUNotificationDataStruct,
			Enum_TRUE);
	}

	if (txnRetCode != EcrioTXNNoError)
	{
		EcrioTUNotifyError(txnRetCode, (void *)pTxnTuNotificationData);
	}

	if (pTxnTuNotificationData)
	{
		_EcrioTXNStructRelease(pTxnData,
			(void **)&(pTxnTuNotificationData),
			_EcrioTXNStructType_EcrioTXNTUNotificationDataStruct,
			Enum_TRUE);
	}

	if (pTxnAppCBData)
	{
		pal_MemoryFree((void **)&(pTxnAppCBData));
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}

/**************************************************************************************************
Function:            _EcrioTXNTimerFCallback()

Purpose:             This is a callback function.

Description:.        This callback function fires when Timer-F fires for a transaction.

Input:				 u_int32 timerID
                     void *pCallbackData

OutPut:              None.

Returns:             void.
**************************************************************************************************/
void _EcrioTXNTimerFCallback
(
	void *pCallbackData,
	TIMERHANDLE timerID
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;
	EcrioTXNTUNotificationDataStruct *pTxnTuNotificationData = NULL;
	EcrioTXNAppCBStruct	*pTxnAppCBData = NULL;
	u_int32 index = 0;

	/* Stop the timer before proceeding further. */
	pal_TimerStop(timerID);

	if (pCallbackData == NULL)
	{
		return;
	}

	pTxnNode = (_EcrioTXNTxnNodeStruct *)pCallbackData;

	pTxnData = (_EcrioTXNDataStruct *)pTxnNode->pTxnData;

	if (pTxnData == NULL || pTxnData->pTxnInitParam == NULL)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		return;
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncParams, "%s:%u\tpCallbackData=%p, timerID=%u",
		__FUNCTION__, __LINE__, pCallbackData, timerID);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\tStart processing %s (%p)",
		__FUNCTION__, __LINE__, _EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier), pTxnNode);

	/* Check and update timer details. */
	index = _EcrioTXNGetRunningTimerIndex(pTxnNode, EcrioTXNSipTimerTypeF);

	if (index >= TXN_NODE_TIMER_ARRAYSIZE_MAX)
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioTXNGetRunningTimerIndex() no timer of type %u running on node %p",
			__FUNCTION__, __LINE__, EcrioTXNSipTimerTypeF, pTxnNode);

		txnRetCode = EcrioTXNTimerError;
		goto EndTag;
	}

	pTxnNode->timerDetails[index].isRunning = Enum_FALSE;
	pTxnNode->timerDetails[index].absoluteDuration = 0;
	pTxnNode->timerDetails[index].timerID = NULL;
	pTxnNode->timerDetails[index].sipTimerType = EcrioTXNSipTimerTypeNone;

	txnRetCode = _EcrioTXNGenerateTUNotificationData(pTxnData, pTxnNode, &pTxnTuNotificationData, Enum_FALSE);
	if (txnRetCode != EcrioTXNNoError)
	{
		goto EndTag;
	}

	if ((pTxnNode->txnState != EcrioTXNTxnStateTrying) && (pTxnNode->txnState != EcrioTXNTxnStateProceeding))
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioTXNGenerateTUNotificationData() Current Transaction state (%s) of the Transaction Node (%p) is not valid for this callback, txnRetCode=%u",
			__FUNCTION__, __LINE__, _EcrioTXNGetTxnStateName(pTxnNode->txnState), pTxnNode, txnRetCode);
		goto EndTag;
	}

	TXNMGRLOGV(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\ttxnState is transiting from %s to %s of the transaction %s",
		__FUNCTION__, __LINE__, _EcrioTXNGetTxnStateName(pTxnNode->txnState), _EcrioTXNGetTxnStateName(EcrioTXNTxnStateTerminated),
		_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

	/*	txn state change */
	pTxnNode->txnState = EcrioTXNTxnStateTerminated;

	TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\tTimer F expires. Request Failed",
		__FUNCTION__, __LINE__);

	/*	txn node deletion	*/
	_EcrioTXNDeleteTxn(pTxnData, pTxnNode);
	pTxnNode = NULL;

	pal_MemoryAllocate(sizeof(EcrioTXNAppCBStruct), (void **)&(pTxnAppCBData));
	if (!pTxnAppCBData)
	{
		txnRetCode = EcrioTXNInsufficientMemoryError;
		goto EndTag;
	}

	pTxnAppCBData->pTxnCBData = (void *)pTxnTuNotificationData;
	pTxnAppCBData->pAppCBData = pTxnData->pTxnInitParam->pTimerNotificationData;

	(*pTxnData->pTxnInitParam->timerExpiryNotificationFunctions.pTimerFNotificationFn)((void *)pTxnAppCBData);

EndTag:

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\tEnd processing txn node %p",
		__FUNCTION__, __LINE__, pTxnNode);

	if (txnRetCode != EcrioTXNNoError)
	{
		EcrioTUNotifyError(txnRetCode, (void *)pTxnTuNotificationData);
	}

	if (pTxnTuNotificationData)
	{
		_EcrioTXNStructRelease(pTxnData,
			(void **)&(pTxnTuNotificationData),
			_EcrioTXNStructType_EcrioTXNTUNotificationDataStruct,
			Enum_TRUE);
	}

	if (pTxnAppCBData)
	{
		pal_MemoryFree((void **)&(pTxnAppCBData));
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}

/**************************************************************************************************
Function:            _EcrioTXNTimerGCallback()

Purpose:             This is a callback function.

Description:.        This callback function fires when Timer-G fires for a transaction.

Input:				 u_int32 timerID
                     void *pCallbackData

OutPut:              None.

Returns:             void.
**************************************************************************************************/
void _EcrioTXNTimerGCallback
(
	void *pCallbackData,
	TIMERHANDLE timerID
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;
	EcrioTXNTUNotificationDataStruct *pTxnTuNotificationData = NULL;
	EcrioTXNAppCBStruct	*pTxnAppCBData = NULL;

	/* Stop the timer before proceeding further. */
	pal_TimerStop(timerID);

	if (!pCallbackData)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		return;
	}

	pTxnNode = (_EcrioTXNTxnNodeStruct *)pCallbackData;
	pTxnData = (_EcrioTXNDataStruct *)pTxnNode->pTxnData;

	if (pTxnData == NULL || pTxnData->pTxnInitParam == NULL)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		return;
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncParams, "%s:%u\tpCallbackData=%p, timerID=%u",
		__FUNCTION__, __LINE__, pCallbackData, timerID);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\tStart processing %s (%p)",
		__FUNCTION__, __LINE__, _EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier), pTxnNode);

	txnRetCode = _EcrioTXNGenerateTUNotificationData(pTxnData, pTxnNode, &pTxnTuNotificationData, Enum_FALSE);

	if (txnRetCode != EcrioTXNNoError)
	{
		goto EndTag;
	}

	if (pTxnNode->txnState != EcrioTXNTxnStateCompleted)
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioTXNGenerateTUNotificationData() Current Transaction state %s of the Transaction Node %p is not valid for this callback, txnRetCode=%u",
			__FUNCTION__, __LINE__, _EcrioTXNGetTxnStateName(pTxnNode->txnState), pTxnNode, txnRetCode);

		goto EndTag;
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\tTimer G expires - Retransmitting Response", __FUNCTION__, __LINE__);

	txnRetCode = _EcrioTXNHandleTimerExpiryRetrans(pTxnData, pTxnNode);

	if (txnRetCode != EcrioTXNNoError)
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioTXNHandleTimerExpiryRetrans() Retransmitting Response, txnRetCode=%u", __FUNCTION__, __LINE__, txnRetCode);
		goto EndTag;
	}

	pal_MemoryAllocate(sizeof(EcrioTXNAppCBStruct), (void **)&(pTxnAppCBData));
	if (!pTxnAppCBData)
	{
		txnRetCode = EcrioTXNInsufficientMemoryError;
		goto EndTag;
	}

	pTxnAppCBData->pTxnCBData = (void *)pTxnTuNotificationData;
	pTxnAppCBData->pAppCBData = pTxnData->pTxnInitParam->pTimerNotificationData;

	(*pTxnData->pTxnInitParam->timerExpiryNotificationFunctions.pTimerGNotificationFn)((void *)pTxnAppCBData);

EndTag:

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\tEnd processing txn node %p", __FUNCTION__, __LINE__, pTxnNode);

	if (txnRetCode != EcrioTXNNoError)
	{
		EcrioTUNotifyError(txnRetCode, (void *)pTxnTuNotificationData);
	}

	if (pTxnTuNotificationData)
	{
		_EcrioTXNStructRelease(pTxnData,
			(void **)&(pTxnTuNotificationData),
			_EcrioTXNStructType_EcrioTXNTUNotificationDataStruct,
			Enum_TRUE);
	}

	if (pTxnAppCBData)
	{
		pal_MemoryFree((void **)&(pTxnAppCBData));
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
}

/**************************************************************************************************
Function:            _EcrioTXNTimerHCallback()

Purpose:             This is a callback function.

Description:.        This callback function fires when Timer-H fires for a transaction.

Input:				 u_int32 timerID
                     void *pCallbackData

OutPut:              None.

Returns:             void.
**************************************************************************************************/
void _EcrioTXNTimerHCallback
(
	void *pCallbackData,
	TIMERHANDLE timerID
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;
	EcrioTXNTUNotificationDataStruct *pTxnTuNotificationData = NULL;
	EcrioTXNAppCBStruct	*pTxnAppCBData = NULL;
	u_int32 index = 0;

	/* Stop the timer before proceeding further. */
	pal_TimerStop(timerID);

	if (!pCallbackData)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		return;
	}

	pTxnNode = (_EcrioTXNTxnNodeStruct *)pCallbackData;
	pTxnData = (_EcrioTXNDataStruct *)pTxnNode->pTxnData;

	if (pTxnData == NULL || pTxnData->pTxnInitParam == NULL)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		return;
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncParams, "%s:%u\tpCallbackData=%p, timerID=%u",
		__FUNCTION__, __LINE__, pCallbackData, timerID);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\tStart processing %s (%p)",
		__FUNCTION__, __LINE__, _EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier), pTxnNode);

	/* Check and update timer details. */
	index = _EcrioTXNGetRunningTimerIndex(pTxnNode, EcrioTXNSipTimerTypeH);

	if (index >= TXN_NODE_TIMER_ARRAYSIZE_MAX)
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioTXNGetRunningTimerIndex() no timer of type %u running on node %p",
			__FUNCTION__, __LINE__, EcrioTXNSipTimerTypeH, pTxnNode);

		txnRetCode = EcrioTXNTimerError;
		goto EndTag;
	}

	pTxnNode->timerDetails[index].isRunning = Enum_FALSE;
	pTxnNode->timerDetails[index].absoluteDuration = 0;
	pTxnNode->timerDetails[index].timerID = NULL;
	pTxnNode->timerDetails[index].sipTimerType = EcrioTXNSipTimerTypeNone;

	txnRetCode = _EcrioTXNGenerateTUNotificationData(pTxnData, pTxnNode, &pTxnTuNotificationData, Enum_FALSE);

	if (txnRetCode != EcrioTXNNoError)
	{
		goto EndTag;
	}

	if (pTxnNode->txnState != EcrioTXNTxnStateCompleted)
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioTXNGenerateTUNotificationData() Current Transaction state %s of the Transaction Node %p is not valid for this callback, txnRetCode=%u",
			__FUNCTION__, __LINE__, _EcrioTXNGetTxnStateName(pTxnNode->txnState), pTxnNode, txnRetCode);
		goto EndTag;
	}

	TXNMGRLOGV(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\ttxnState is transiting from %s to %s of the transaction %s",
		__FUNCTION__, __LINE__, _EcrioTXNGetTxnStateName(pTxnNode->txnState), _EcrioTXNGetTxnStateName(EcrioTXNTxnStateTerminated),
		_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

	/*	txn state change	*/
	pTxnNode->txnState = EcrioTXNTxnStateTerminated;

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\tTimer H expires", __FUNCTION__, __LINE__);
	/*	txn node deletion	*/
	_EcrioTXNDeleteTxn(pTxnData, pTxnNode);
	pTxnNode = NULL;

	pal_MemoryAllocate(sizeof(EcrioTXNAppCBStruct), (void **)&(pTxnAppCBData));
	if (!pTxnAppCBData)
	{
		txnRetCode = EcrioTXNInsufficientMemoryError;
		goto EndTag;
	}

	pTxnAppCBData->pTxnCBData = (void *)pTxnTuNotificationData;
	pTxnAppCBData->pAppCBData = pTxnData->pTxnInitParam->pTimerNotificationData;

	(*pTxnData->pTxnInitParam->timerExpiryNotificationFunctions.pTimerHNotificationFn)((void *)pTxnAppCBData);

EndTag:

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\tEnd processing txn node %p", __FUNCTION__, __LINE__, pTxnNode);

	if (txnRetCode != EcrioTXNNoError)
	{
		EcrioTUNotifyError(txnRetCode, (void *)pTxnTuNotificationData);
	}

	if (pTxnTuNotificationData)
	{
		_EcrioTXNStructRelease(pTxnData,
			(void **)&(pTxnTuNotificationData),
			_EcrioTXNStructType_EcrioTXNTUNotificationDataStruct,
			Enum_TRUE);
	}

	if (pTxnAppCBData)
	{
		pal_MemoryFree((void **)&(pTxnAppCBData));
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
}

/**************************************************************************************************
Function:            _EcrioTXNTimerICallback()

Purpose:             This is a callback function.

Description:.        This callback function fires when Timer-I fires for a transaction.

Input:				 u_int32 timerID
                     void *pCallbackData

OutPut:              None.

Returns:             void.
**************************************************************************************************/
void _EcrioTXNTimerICallback
(
	void *pCallbackData,
	TIMERHANDLE timerID
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;
	EcrioTXNTUNotificationDataStruct *pTxnTuNotificationData = NULL;
	EcrioTXNAppCBStruct	*pTxnAppCBData = NULL;
	u_int32 index = 0;

	/* Stop the timer before proceeding further. */
	pal_TimerStop(timerID);

	if (!pCallbackData)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		return;
	}

	pTxnNode = (_EcrioTXNTxnNodeStruct *)pCallbackData;
	pTxnData = (_EcrioTXNDataStruct *)pTxnNode->pTxnData;

	if (pTxnData == NULL || pTxnData->pTxnInitParam == NULL)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		return;
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncParams, "%s:%u\tpCallbackData=%p, timerID=%u",
		__FUNCTION__, __LINE__, pCallbackData, timerID);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\tstart processing %s (%p)",
		__FUNCTION__, __LINE__, _EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier), pTxnNode);

	/* Check and update timer details. */
	index = _EcrioTXNGetRunningTimerIndex(pTxnNode, EcrioTXNSipTimerTypeI);

	if (index >= TXN_NODE_TIMER_ARRAYSIZE_MAX)
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioTXNGetRunningTimerIndex() no timer of type %u running on node %p",
			__FUNCTION__, __LINE__, EcrioTXNSipTimerTypeI, pTxnNode);

		txnRetCode = EcrioTXNTimerError;
		goto EndTag;
	}

	pTxnNode->timerDetails[index].isRunning = Enum_FALSE;
	pTxnNode->timerDetails[index].absoluteDuration = 0;
	pTxnNode->timerDetails[index].timerID = NULL;
	pTxnNode->timerDetails[index].sipTimerType = EcrioTXNSipTimerTypeNone;

	txnRetCode = _EcrioTXNGenerateTUNotificationData(pTxnData, pTxnNode, &pTxnTuNotificationData, Enum_TRUE);

	if (txnRetCode != EcrioTXNNoError)
	{
		goto EndTag;
	}

	if (pTxnNode->txnState != EcrioTXNTxnStateConfirmed)
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioTXNGenerateTUNotificationData() Current Transaction state %s of the Transaction Node %p is not valid for this callback, txnRetCode=%u",
			__FUNCTION__, __LINE__, _EcrioTXNGetTxnStateName(pTxnNode->txnState), pTxnNode, txnRetCode);
		goto EndTag;
	}

	TXNMGRLOGV(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\ttxnState is transiting from %s to %s of the transaction %s",
		__FUNCTION__, __LINE__, _EcrioTXNGetTxnStateName(pTxnNode->txnState), _EcrioTXNGetTxnStateName(EcrioTXNTxnStateTerminated),
		_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

	/*	txn state change	*/
	pTxnNode->txnState = EcrioTXNTxnStateTerminated;

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\tTimer I expires", __FUNCTION__, __LINE__);
	/*	txn node deletion	*/
	_EcrioTXNDeleteTxn(pTxnData, pTxnNode);
	pTxnNode = NULL;

	pal_MemoryAllocate(sizeof(EcrioTXNAppCBStruct), (void **)&(pTxnAppCBData));
	if (!pTxnAppCBData)
	{
		txnRetCode = EcrioTXNInsufficientMemoryError;
		goto EndTag;
	}

	pTxnAppCBData->pTxnCBData = (void *)pTxnTuNotificationData;
	pTxnAppCBData->pAppCBData = pTxnData->pTxnInitParam->pTimerNotificationData;

	(*pTxnData->pTxnInitParam->timerExpiryNotificationFunctions.pTimerINotificationFn)((void *)pTxnAppCBData);

EndTag:

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\tEnd processing txn node %p", __FUNCTION__, __LINE__, pTxnNode);

	if (txnRetCode != EcrioTXNNoError)
	{
		EcrioTUNotifyError(txnRetCode, (void *)pTxnTuNotificationData);
	}

	if (pTxnTuNotificationData)
	{
		_EcrioTXNStructRelease(pTxnData,
			(void **)&(pTxnTuNotificationData),
			_EcrioTXNStructType_EcrioTXNTUNotificationDataStruct,
			Enum_TRUE);
	}

	if (pTxnAppCBData)
	{
		pal_MemoryFree((void **)&(pTxnAppCBData));
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
}

/**************************************************************************************************
Function:            _EcrioTXNTimerJCallback()

Purpose:             This is a callback function.

Description:.        This callback function fires when Timer-J fires for a transaction.

Input:				 u_int32 timerID
                     void *pCallbackData

OutPut:              None.

Returns:             void.
**************************************************************************************************/
void _EcrioTXNTimerJCallback
(
	void *pCallbackData,
	TIMERHANDLE timerID
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;
	EcrioTXNTUNotificationDataStruct *pTxnTuNotificationData = NULL;
	EcrioTXNAppCBStruct	*pTxnAppCBData = NULL;
	u_int32 index = 0;

	/* Stop the timer before proceeding further. */
	pal_TimerStop(timerID);

	if (pCallbackData == NULL)
	{
		return;
	}

	pTxnNode = (_EcrioTXNTxnNodeStruct *)pCallbackData;

	pTxnData = (_EcrioTXNDataStruct *)pTxnNode->pTxnData;

	if (pTxnData == NULL || pTxnData->pTxnInitParam == NULL)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		return;
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncParams, "%s:%u\tpCallbackData=%p, timerID=%u",
		__FUNCTION__, __LINE__, pCallbackData, timerID);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\tStart processing %s (%p)",
		__FUNCTION__, __LINE__, _EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier), pTxnNode);

	/* Check and update timer details. */
	index = _EcrioTXNGetRunningTimerIndex(pTxnNode, EcrioTXNSipTimerTypeJ);

	if (index >= TXN_NODE_TIMER_ARRAYSIZE_MAX)
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioTXNGetRunningTimerIndex() no timer of type %u running on node %p",
			__FUNCTION__, __LINE__, EcrioTXNSipTimerTypeJ, pTxnNode);

		txnRetCode = EcrioTXNTimerError;
		goto EndTag;
	}

	pTxnNode->timerDetails[index].isRunning = Enum_FALSE;
	pTxnNode->timerDetails[index].absoluteDuration = 0;
	pTxnNode->timerDetails[index].timerID = NULL;
	pTxnNode->timerDetails[index].sipTimerType = EcrioTXNSipTimerTypeNone;

	txnRetCode = _EcrioTXNGenerateTUNotificationData(pTxnData, pTxnNode, &pTxnTuNotificationData, Enum_FALSE);
	if (txnRetCode != EcrioTXNNoError)
	{
		goto EndTag;
	}

	if (pTxnNode->txnState != EcrioTXNTxnStateCompleted)
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioTXNGenerateTUNotificationData() Current Transaction state %s of the Transaction Node %p is not valid for this callback, txnRetCode=%u",
			__FUNCTION__, __LINE__, _EcrioTXNGetTxnStateName(pTxnNode->txnState), pTxnNode, txnRetCode);

		goto EndTag;
	}

	TXNMGRLOGV(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\ttxnState is transiting from %s to %s of the transaction %s",
		__FUNCTION__, __LINE__, _EcrioTXNGetTxnStateName(pTxnNode->txnState), _EcrioTXNGetTxnStateName(EcrioTXNTxnStateTerminated),
		_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

	/*	txn state change */
	pTxnNode->txnState = EcrioTXNTxnStateTerminated;

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\tTimer J expires",
		__FUNCTION__, __LINE__);

	/*	txn node deletion	*/
	_EcrioTXNDeleteTxn(pTxnData, pTxnNode);
	pTxnNode = NULL;

	pal_MemoryAllocate(sizeof(EcrioTXNAppCBStruct), (void **)&(pTxnAppCBData));
	if (!pTxnAppCBData)
	{
		txnRetCode = EcrioTXNInsufficientMemoryError;
		goto EndTag;
	}

	pTxnAppCBData->pTxnCBData = (void *)pTxnTuNotificationData;
	pTxnAppCBData->pAppCBData = pTxnData->pTxnInitParam->pTimerNotificationData;

	(*pTxnData->pTxnInitParam->timerExpiryNotificationFunctions.pTimerJNotificationFn)((void *)pTxnAppCBData);

EndTag:

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\tEnd processing txn node %p",
		__FUNCTION__, __LINE__, pTxnNode);

	if (pTxnTuNotificationData)
	{
		_EcrioTXNStructRelease(pTxnData,
			(void **)&(pTxnTuNotificationData),
			_EcrioTXNStructType_EcrioTXNTUNotificationDataStruct,
			Enum_TRUE);
	}

	if (txnRetCode != EcrioTXNNoError)
	{
		EcrioTUNotifyError(txnRetCode, (void *)pTxnTuNotificationData);
	}

	if (pTxnTuNotificationData)
	{
		_EcrioTXNStructRelease(pTxnData,
			(void **)&(pTxnTuNotificationData),
			_EcrioTXNStructType_EcrioTXNTUNotificationDataStruct,
			Enum_TRUE);
	}

	if (pTxnAppCBData)
	{
		pal_MemoryFree((void **)&(pTxnAppCBData));
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}

/**************************************************************************************************
Function:            _EcrioTXNTimerKCallback()

Purpose:             This is a callback function.

Description:.        This callback function fires when Timer-K fires for a transaction.

Input:				 u_int32 timerID
                     void *pCallbackData

OutPut:              None.

Returns:             void.
**************************************************************************************************/
void _EcrioTXNTimerKCallback
(
	void *pCallbackData,
	TIMERHANDLE timerID
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;
	EcrioTXNTUNotificationDataStruct *pTxnTuNotificationData = NULL;
	BoolEnum bDeleteTxn = Enum_TRUE;
	EcrioTXNAppCBStruct	*pTxnAppCBData = NULL;
	u_int32 index = 0;

	/* Stop the timer before proceeding further. */
	pal_TimerStop(timerID);

	if (pCallbackData == NULL)
	{
		return;
	}

	pTxnNode = (_EcrioTXNTxnNodeStruct *)pCallbackData;

	pTxnData = (_EcrioTXNDataStruct *)pTxnNode->pTxnData;

	if (pTxnData == NULL || pTxnData->pTxnInitParam == NULL)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		return;
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncParams, "%s:%u\tpCallbackData=%p, timerID=%u",
		__FUNCTION__, __LINE__, pCallbackData, timerID);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\tStart processing %s (%p)",
		__FUNCTION__, __LINE__, _EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier), pTxnNode);

	/* Check and update timer details. */
	index = _EcrioTXNGetRunningTimerIndex(pTxnNode, EcrioTXNSipTimerTypeK);

	if (index >= TXN_NODE_TIMER_ARRAYSIZE_MAX)
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioTXNGetRunningTimerIndex() no timer of type %u running on node %p",
			__FUNCTION__, __LINE__, EcrioTXNSipTimerTypeK, pTxnNode);

		txnRetCode = EcrioTXNTimerError;
		goto EndTag;
	}

	pTxnNode->timerDetails[index].isRunning = Enum_FALSE;
	pTxnNode->timerDetails[index].absoluteDuration = 0;
	pTxnNode->timerDetails[index].timerID = NULL;
	pTxnNode->timerDetails[index].sipTimerType = EcrioTXNSipTimerTypeNone;

	txnRetCode = _EcrioTXNGenerateTUNotificationData(pTxnData, pTxnNode, &pTxnTuNotificationData, Enum_FALSE);
	if (txnRetCode != EcrioTXNNoError)
	{
		goto EndTag;
	}

	if (pTxnNode->txnState != EcrioTXNTxnStateCompleted)
	{
		TXNMGRLOGD(pTxnData->pTxnInitParam->pLogHandle, KLogTypeState,
			"%s:%u\t_EcrioTXNGenerateTUNotificationData() Current Transaction state %s of the Transaction Node %p is not valid for this callback, txnRetCode=%u",
			__FUNCTION__, __LINE__, _EcrioTXNGetTxnStateName(pTxnNode->txnState), pTxnNode, txnRetCode);

		goto EndTag;
	}

	if (bDeleteTxn == Enum_TRUE)
	{
		TXNMGRLOGD(pTxnData->pTxnInitParam->pLogHandle, KLogTypeState, "%s:%u\ttxnState is transiting from %s to %s of the transaction %s",
			__FUNCTION__, __LINE__, _EcrioTXNGetTxnStateName(pTxnNode->txnState), _EcrioTXNGetTxnStateName(EcrioTXNTxnStateTerminated),
			_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

		/* txn state change */
		pTxnNode->txnState = EcrioTXNTxnStateTerminated;

		TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\tTimer K expires",
			__FUNCTION__, __LINE__);

		/* txn node deletion */
		_EcrioTXNDeleteTxn(pTxnData, pTxnNode);
		pTxnNode = NULL;
	}
	else
	{
		TXNMGRLOGD(pTxnData->pTxnInitParam->pLogHandle, KLogTypeState, "%s:%u\ttxnState is transiting from %s to %s of the transaction %s",
			__FUNCTION__, __LINE__, _EcrioTXNGetTxnStateName(pTxnNode->txnState), _EcrioTXNGetTxnStateName(EcrioTXNTxnStateTerminated),
			_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

		/* txn state change */
		pTxnNode->txnState = EcrioTXNTxnStateTerminated;

		pTxnNode = NULL;

		TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\tTimer K expires",
			__FUNCTION__, __LINE__);
	}

	pal_MemoryAllocate(sizeof(EcrioTXNAppCBStruct), (void **)&(pTxnAppCBData));
	if (!pTxnAppCBData)
	{
		txnRetCode = EcrioTXNInsufficientMemoryError;
		goto EndTag;
	}

	pTxnAppCBData->pTxnCBData = (void *)pTxnTuNotificationData;
	pTxnAppCBData->pAppCBData = pTxnData->pTxnInitParam->pTimerNotificationData;

	(*pTxnData->pTxnInitParam->timerExpiryNotificationFunctions.pTimerKNotificationFn)((void *)pTxnAppCBData);

EndTag:

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\tEnd processing txn node %p",
		__FUNCTION__, __LINE__, pTxnNode);

	if (pTxnTuNotificationData)
	{
		_EcrioTXNStructRelease(pTxnData,
			(void **)&(pTxnTuNotificationData),
			_EcrioTXNStructType_EcrioTXNTUNotificationDataStruct,
			Enum_TRUE);
	}

	if (txnRetCode != EcrioTXNNoError)
	{
		EcrioTUNotifyError(txnRetCode, (void *)pTxnTuNotificationData);
	}

	if (pTxnTuNotificationData)
	{
		_EcrioTXNStructRelease(pTxnData,
			(void **)&(pTxnTuNotificationData),
			_EcrioTXNStructType_EcrioTXNTUNotificationDataStruct,
			Enum_TRUE);
	}

	if (pTxnAppCBData)
	{
		pal_MemoryFree((void **)&(pTxnAppCBData));
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}
