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
Function:            EcrioTXNDeleteTxnAll()

Purpose:             This function deletes all transaction nodes and associated timers.

Description:.        This internal deletes all transaction nodes and associated timers.

Input:				 void						*pTxn

OutPut:              None.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum EcrioTXNDeleteTxnAll
(
	void *pTxn
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	void *pLogHandle = NULL;

	if (!pTxn)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	pTxnData = (_EcrioTXNDataStruct	*)pTxn;

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

	TXNMGRLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	TXNMGRLOGI(pLogHandle, KLogTypeFuncParams, "%s:%u\tpTxn=%p",
		__FUNCTION__, __LINE__, pTxn);

	if (pTxnData->pTxnListHead)
	{
		_EcrioTXNDeleteAllTxnNode(pTxnData);
	}

EndTag:

	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\ttxnRetCode=%u", __FUNCTION__, __LINE__, txnRetCode);

	return txnRetCode;
}

/**************************************************************************************************
Function:            EcrioTXNDeleteTxnMatchedSessionMatchedTxnIdentifier()

Purpose:             This function deletes all transaction related to a session.

Description:.        This internal deletes all transaction as per input parameter pSessionTxnMappingParamString
                     and pTxnIdentifier.

Input:				 void						*pTxn
                     u_char					*pSessionTxnMappingParamString
                     EcrioTXNTxnIdentificationStruct		*pTxnIdentifier

OutPut:              None.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum EcrioTXNDeleteTxnMatchedSessionMatchedTxnIdentifier
(
	void *pTxn,
	u_char *pSessionTxnMappingParamString,
	u_char *pViaBranch,
	EcrioTXNTxnIdentificationStruct	*pTxnIdentifier
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL, *pTxnNodeNext = NULL;
	void *pLogHandle = NULL;

	if (!pTxn)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	pTxnData = (_EcrioTXNDataStruct	*)pTxn;

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

	TXNMGRLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	TXNMGRLOGI(pLogHandle, KLogTypeFuncParams, "%s:%u\tpTxn=%p, pSessionTxnMappingParamString=%p, pTxnIdentifier=%p",
		__FUNCTION__, __LINE__, pTxn, pSessionTxnMappingParamString, pTxnIdentifier);

	if ((!pSessionTxnMappingParamString) || (!pTxnIdentifier))
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral,
			"%s:%u\tpTxnIdentifier or pSessionTxnMappingParamString is NULL", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	pTxnNode = pTxnData->pTxnListHead;

	while (pTxnNode)
	{
		pTxnNodeNext = pTxnNode->pNext;

		if (!pal_StringICompare(pTxnNode->pSessionTxnMappingParam->pSessionTxnMappingParamString,
			pSessionTxnMappingParamString))
		{
			if (!pal_StringCompare(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier.pMethodName,
				pTxnIdentifier->pMethodName) &&
				pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier.txnType ==
				pTxnIdentifier->txnType)
			{
				if (pViaBranch)
				{
					if (pal_StringICompare(pTxnNode->pTxnMatchingParam->commonMatchingParam.pBranchParam, pViaBranch) == 0)
					{
						_EcrioTXNDeleteTxn(pTxnData,
							pTxnNode);
						break;
					}
				}
				else
				{
					_EcrioTXNDeleteTxn(pTxnData,
						pTxnNode);
				}
			}
		}

		pTxnNode = pTxnNodeNext;
	}

EndTag:

	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\ttxnRetCode=%u", __FUNCTION__, __LINE__, txnRetCode);

	return txnRetCode;
}


/**************************************************************************************************
Function:            EcrioTXNDeleteMatchedTransactions()

Purpose:             This function deletes all transaction matching given callID.

Description:.        This internal deletes all transaction as per input parameter pCallID.

Input:				 void						*pTxn
u_char					*pCallID


OutPut:              None.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum EcrioTXNDeleteMatchedTransactions
(
	void *pTxn,
	u_char *pCallID
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL, *pTxnNodeNext = NULL;
	void *pLogHandle = NULL;

	if (!pTxn)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	pTxnData = (_EcrioTXNDataStruct	*)pTxn;

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

	TXNMGRLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	TXNMGRLOGI(pLogHandle, KLogTypeFuncParams, "%s:%u\tpTxn=%p, pCallID=%s",
		__FUNCTION__, __LINE__, pTxn, pCallID);

	if (!pCallID)
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral,
			"%s:%u\tpCallID is NULL", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	pTxnNode = pTxnData->pTxnListHead;

	while (pTxnNode)
	{
		pTxnNodeNext = pTxnNode->pNext;

		if (!pal_StringICompare(pTxnNode->pTxnMatchingParam->commonMatchingParam.pCallId,
			pCallID))
		{
			_EcrioTXNDeleteTxn(pTxnData,
				pTxnNode);
		}

		pTxnNode = pTxnNodeNext;
	}

EndTag:

	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\ttxnRetCode=%u", __FUNCTION__, __LINE__, txnRetCode);

	return txnRetCode;
}

// #if MAIN_IMS

/**************************************************************************************************
Function:            EcrioTXNIsAnyTxnExistMatchedSessionMatchedTxnIdentifier()

Purpose:             This function is for checking if there is any transaction exist or not.

Description:.        is for checking if there is any transaction exist or not based on input parameter
                     pSessionTxnMappingParamString and pTxnIdentifier. If exist then this
                     function returns true else false.

Input:				 void						*pTxn
                     u_char								*pSessionTxnMappingParamString
                     EcrioTXNTxnIdentificationStruct		*pTxnIdentifier

OutPut:              None.

Returns:             BoolEnum.
**************************************************************************************************/
BoolEnum EcrioTXNIsAnyTxnExistMatchedSessionMatchedTxnIdentifier
(
	void *pTxn,
	u_char *pSessionTxnMappingParamString,
	EcrioTXNTxnIdentificationStruct	*pTxnIdentifier
)
{
	_EcrioTXNDataStruct	*pTxnData = NULL;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;
	BoolEnum bExist = Enum_INVALID;
	void *pLogHandle = NULL;

	if (!pTxn)
	{
		goto EndTag;
	}

	pTxnData = (_EcrioTXNDataStruct	*)pTxn;

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

	TXNMGRLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	TXNMGRLOGI(pLogHandle, KLogTypeFuncParams, "%s:%u\tpTxn=%p, pSessionTxnMappingParamString=%p, pTxnIdentifier=%p",
		__FUNCTION__, __LINE__, pTxn, pSessionTxnMappingParamString, pTxnIdentifier);

	if ((!pSessionTxnMappingParamString) || (!pTxnIdentifier))
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral,
			"%s:%u\tpSessionTxnMappingParamString or pTxnIdentifier is NULL", __FUNCTION__, __LINE__);

		goto EndTag;
	}

	bExist = (pTxnData->pTxnListHead) ? Enum_FALSE : Enum_INVALID;

	pTxnNode = pTxnData->pTxnListHead;

	while (pTxnNode)
	{
		if (!pal_StringICompare(pTxnNode->pSessionTxnMappingParam->pSessionTxnMappingParamString,
			pSessionTxnMappingParamString))
		{
			if (!pal_StringCompare(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier.pMethodName,
				pTxnIdentifier->pMethodName) &&
				(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier.txnType ==
				pTxnIdentifier->txnType))
			{
				bExist = Enum_TRUE;
				break;
			}
		}

		pTxnNode = pTxnNode->pNext;
	}

EndTag:

	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\tbExist=%d", __FUNCTION__, __LINE__, bExist);

	return bExist;
}

// #endif // MAIN_IMS

/**************************************************************************************************
Function:            EcrioTXNGetDetailsOfAllTxnMatchedTxnIdentifier()

Purpose:             This function is for getting details of all ongoing transactions.

Description:.        Is for getting getting details of all ongoing transactions based on inout parameter
                     pTxnIdentifier.

Input:				 void						*pTxn
                     EcrioTXNTxnIdentificationStruct		*pTxnIdentifier

OutPut:              EcrioTXNOutputParamStruct			*pOutputParamStruct.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum EcrioTXNGetDetailsOfAllTxnMatchedTxnIdentifier
(
	void *pTxn,
	EcrioTXNTxnIdentificationStruct	*pTxnIdentifier,
	EcrioTXNOutputParamStruct *pOutputParamStruct
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;
	u_int16	i = 0, noOfOutputParamDetails = 0;
	EcrioTXNOutputParamDetailsStruct **ppOutputParamaDetails = NULL;
	EcrioTXNOutputParamDetailsStruct *pOutputParamaDetails = NULL;
	void *pLogHandle = NULL;

	if (!pTxn)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	pTxnData = (_EcrioTXNDataStruct	*)pTxn;

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

// quiet	TXNMGRLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

// quiet	TXNMGRLOGI(pLogHandle, KLogTypeFuncParams, "%s:%u\tpTxn=%p, pTxnIdentifier=%p, pOutputParamStruct=%p",
// quiet		__FUNCTION__, __LINE__, pTxn, pTxnIdentifier, pOutputParamStruct);

	if ((!pTxnIdentifier) || (!pOutputParamStruct))
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral,
			"%s:%u\tpTxnIdentifier or pOutputParamStruct is NULL", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	pTxnNode = pTxnData->pTxnListHead;

	while (pTxnNode)
	{
		if (!pal_StringCompare(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier.pMethodName,
			pTxnIdentifier->pMethodName) &&
			(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier.txnType ==
			pTxnIdentifier->txnType))
		{
			pal_MemoryAllocate(sizeof(EcrioTXNOutputParamDetailsStruct), (void **)&(pOutputParamaDetails));
			if (!pOutputParamaDetails)
			{
				TXNMGRLOGE(pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryAllocate() failed", __FUNCTION__, __LINE__);

				txnRetCode = EcrioTXNInsufficientMemoryError;
				goto EndTag;
			}

			txnRetCode = _EcrioTXNAllocateCopyTxnMatchingParam(pTxnData,
				pTxnNode->pTxnMatchingParam,
				&(pOutputParamaDetails->pTxnMatchingParam));
			if (txnRetCode != EcrioTXNNoError)
			{
				TXNMGRLOGE(pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioTXNAllocateCopyTxnMatchingParam() txnRetCode=%u",
					__FUNCTION__, __LINE__, txnRetCode);

				goto EndTag;
			}

			txnRetCode = _EcrioTXNAllocateCopySessionTxnMappingParam(pTxnData,
				pTxnNode->pSessionTxnMappingParam,
				&(pOutputParamaDetails->pSessionTxnMappingParam));

			if (txnRetCode != EcrioTXNNoError)
			{
				TXNMGRLOGE(pLogHandle, KLogTypeGeneral,
					"%s:%u\t_EcrioTXNAllocateCopySessionTxnMappingParam() txnRetCode=%u",
					__FUNCTION__, __LINE__, txnRetCode);
				goto EndTag;
			}

			if (ppOutputParamaDetails == NULL)
			{
				pal_MemoryAllocate(sizeof(EcrioTXNOutputParamDetailsStruct *), (void **)&ppOutputParamaDetails);
			}
			else
			{
				/* Check arithmetic overflow - do not use the ArithmeticOverflowDetected() API for u_int16 variable. */
				if (noOfOutputParamDetails + 1 > USHRT_MAX)
				{
					txnRetCode = EcrioTXNInsufficientMemoryError;
					goto EndTag;
				}

				/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
				if (pal_UtilityDataOverflowDetected((noOfOutputParamDetails + 1), sizeof(EcrioTXNOutputParamDetailsStruct *)) == Enum_TRUE)
				{
					txnRetCode = EcrioTXNInsufficientMemoryError;
					goto EndTag;
				}

				pal_MemoryReallocate((noOfOutputParamDetails + 1) * sizeof(EcrioTXNOutputParamDetailsStruct *),
					(void **)&ppOutputParamaDetails);
			}

			if (!ppOutputParamaDetails)
			{
				TXNMGRLOGE(pLogHandle, KLogTypeGeneral,
					"%s:%u\tpal_MemoryReallocate() failed", __FUNCTION__, __LINE__);

				txnRetCode = EcrioTXNInsufficientMemoryError;
				goto EndTag;
			}

			ppOutputParamaDetails[noOfOutputParamDetails++] = pOutputParamaDetails;
			pOutputParamaDetails = NULL;
		}

		pTxnNode = pTxnNode->pNext;
	}

	pOutputParamStruct->noOfOutputParamDetails = noOfOutputParamDetails;
	pOutputParamStruct->ppOutputParamaDetails = ppOutputParamaDetails;
	ppOutputParamaDetails = NULL;
	noOfOutputParamDetails = 0;

EndTag:

	if (pOutputParamaDetails)
	{
		_EcrioTXNStructRelease(pTxnData,
			(void **)&pOutputParamaDetails,
			_EcrioTXNStructType_EcrioTXNOutputParamDetailsStruct,
			Enum_TRUE);
	}

	if (ppOutputParamaDetails)
	{
		for (i = 0; i < noOfOutputParamDetails; i++)
		{
			_EcrioTXNStructRelease(pTxnData,
				(void **)&ppOutputParamaDetails[i],
				_EcrioTXNStructType_EcrioTXNOutputParamDetailsStruct,
				Enum_TRUE);
		}

		pal_MemoryFree((void **)&ppOutputParamaDetails);
	}

// quiet	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\ttxnRetCode=%u", __FUNCTION__, __LINE__, txnRetCode);
	return txnRetCode;
}

// #if MAIN_IMS

/**************************************************************************************************
Function:            EcrioTXNDisableRetransmissionMatchedSessionMatchedTxnIdentifier()

Purpose:             This function disables all transactions for retransmition.

Description:.        Disables all transactions for retransmition based on input parameter
                     pSessionTxnMappingParamString and pTxnIdentifier.

Input:				 void						*pTxn
                     u_char					*pSessionTxnMappingParamString
                     EcrioTXNTxnIdentificationStruct		*pTxnIdentifier

OutPut:              None.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum EcrioTXNDisableRetransmissionMatchedSessionMatchedTxnIdentifier
(
	void *pTxn,
	u_char *pSessionTxnMappingParamString,
	EcrioTXNTxnIdentificationStruct	*pTxnIdentifier
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;
	void *pLogHandle = NULL;

	if (!pTxn)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	pTxnData = (_EcrioTXNDataStruct	*)pTxn;

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

	TXNMGRLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	TXNMGRLOGI(pLogHandle, KLogTypeFuncParams, "%s:%u\tpTxn=%p, pSessionTxnMappingParamString=%p, pTxnIdentifier=%p",
		__FUNCTION__, __LINE__, pTxn, pSessionTxnMappingParamString, pTxnIdentifier);

	if ((!pSessionTxnMappingParamString) || (!pTxnIdentifier))
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral,
			"%s:%u\tpSessionTxnMappingParamString or pTxnIdentifier is NULL", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	pTxnNode = pTxnData->pTxnListHead;

	while (pTxnNode)
	{
		if (!pal_StringICompare(pTxnNode->pSessionTxnMappingParam->pSessionTxnMappingParamString,
			pSessionTxnMappingParamString))
		{
			if (!pal_StringCompare(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier.pMethodName,
				pTxnIdentifier->pMethodName) &&
				(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier.txnType ==
				pTxnIdentifier->txnType))
			{
				pTxnNode->bRetryEnabled = Enum_FALSE;
			}
		}

		pTxnNode = pTxnNode->pNext;
	}

EndTag:

	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\ttxnRetCode=%u", __FUNCTION__, __LINE__, txnRetCode);

	return txnRetCode;
}

// #endif // MAIN_IMS

/**************************************************************************************************
Function:            EcrioTXNSendMessageDirectToTransport()

Purpose:             This function sent the message to network.

Description:.        sent the message to network through SAC.

Input:				 void						*pTxn
                     EcrioTXNInputParamStruct	*pTxnInputParam

OutPut:              None.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum EcrioTXNSendMessageDirectToTransport
(
	void *pTxn,
	EcrioTXNInputParamStruct *pTxnInputParam
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	u_int32	socketError = KPALErrorNone;
	EcrioTXNInitParamStruct *pTxnInitParamL = NULL;
	void *pLogHandle = NULL;

	/* Adding SAC support */
	EcrioTxnMgrTransportStruct *pTxnTransportStructL = NULL;
	u_int32 uChannelIndex = 0;

	/*Add ends */
	if (!pTxn)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	pTxnData = (_EcrioTXNDataStruct	*)pTxn;

	pTxnInitParamL = pTxnData->pTxnInitParam;

	if (NULL == pTxnInitParamL)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		return txnRetCode;
	}

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

	TXNMGRLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	TXNMGRLOGI(pLogHandle, KLogTypeFuncParams, "%s:%u\tpTxnData=%p, pTxnInputParam=%p",
		__FUNCTION__, __LINE__, pTxnData, pTxnInputParam);

	pTxnTransportStructL = pTxnInitParamL->pTxnTransportStruct;

	if (NULL == pTxnTransportStructL)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	if (!pTxnInputParam)
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral,
			"%s:%u\tpTxnInputParam is NULL", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	if ((!pTxnInputParam->pMsg) || (!pTxnInputParam->msgLen))
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral,
			"%s:%u\tpMsg is NULL or msgLen is 0", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	if (pTxnInputParam->txnMatchingParam.commonMatchingParam.txnIdentifier.pMethodName &&
		pal_StringCompare(pTxnInputParam->txnMatchingParam.commonMatchingParam.txnIdentifier.pMethodName, ECRIO_TXN_METHOD_NAME_ACK) == 0)
	{
		if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeUDP)
		{
			uChannelIndex = ((EcrioTxnMgrTransportStruct *)(pTxnTransportStructL))->sendingChannelIndex;
		}
		else if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeTCP)
		{
			uChannelIndex = ((EcrioTxnMgrTransportStruct *)(pTxnTransportStructL))->sendingTCPChannelIndex;
		}
		else if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeTLS)
		{
			uChannelIndex = ((EcrioTxnMgrTransportStruct *)(pTxnTransportStructL))->sendingTLSChannelIndex;
		}
		else
		{
			txnRetCode = EcrioTXNTransportError;
			goto EndTag;
		}
	}
	else
	{
		uChannelIndex = ((EcrioTxnMgrTransportStruct *)(pTxnTransportStructL))->receivedOnChannelIndex;
	}

#ifdef ENABLE_QCMAPI
	socketError = _EcrioTxnMgrSendDataToCommunicationChannel(pTxnData, pTxnInputParam->pMsg, (u_int16)pTxnInputParam->msgLen, uChannelIndex, pTxnInputParam->messageType, pTxnInputParam->txnMatchingParam.commonMatchingParam.pCallId, Enum_FALSE);
#else
	socketError = _EcrioTxnMgrSendDataToCommunicationChannel(pTxnData, pTxnInputParam->pMsg, (u_int16)pTxnInputParam->msgLen, uChannelIndex);
#endif
	if (socketError != KPALErrorNone)
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral,
			"%s:%u\tSending to network failed, socketError=%u", __FUNCTION__, __LINE__, socketError);

		txnRetCode = EcrioTXNTransportError;
		goto EndTag;
	}

EndTag:

	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\ttxnRetCode=%u", __FUNCTION__, __LINE__, txnRetCode);

	return txnRetCode;
}

/**************************************************************************************************
Function:            _EcrioTxnMgrModifyReceivedOnChannelIndex()

Purpose:             This function will return the protected client channel

Description:.        This function will return the protected client channel corresponding to the given
protected server channel over UDP. For TCP it returns the same channel as the received
channel will be used to send data.

Input:				 _EcrioTXNDataStruct	*pTxnData
u_int32	remoteSendingPort
u_int32 receivedOnChannelId

OutPut:              None.

Returns:             u_int32.
**************************************************************************************************/
u_int32	_EcrioTxnMgrModifyReceivedOnChannelIndex(_EcrioTXNDataStruct *pTxnData, u_int32	remoteSendingPort, u_int32 receivedOnChannelId)
{
	u_int32	i = 0;
	u_int32	sendingChannelId = 0;
	BoolEnum	bCommStructMatched = Enum_FALSE;

	EcrioTxnMgrSignalingCommunicationStruct* pCommunicationStruct = NULL;
	EcrioTxnMgrTransportStruct *pTxnStruct = NULL;

	if (pTxnData && pTxnData->pTxnInitParam)
	{
		pTxnStruct = (EcrioTxnMgrTransportStruct*)pTxnData->pTxnInitParam->pTxnTransportStruct;
	}

	if (pTxnStruct == NULL ||
		((pTxnStruct->pCommunicationStruct == NULL || pTxnStruct->pCommunicationStruct->pChannels == NULL)))
	{
		goto EndTag;
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	sendingChannelId = receivedOnChannelId;

	if (pTxnStruct->pCommunicationStruct != NULL)
	{
		pCommunicationStruct = pTxnStruct->pCommunicationStruct;

		for (i = 0; i<pCommunicationStruct->noOfChannels; i++)
		{
			if ((pCommunicationStruct->pChannels)[i].channelIndex == receivedOnChannelId)
			{
				bCommStructMatched = Enum_TRUE;
				break;
			}
		}
	}

	if (bCommStructMatched == Enum_TRUE)
	{
		if ((pCommunicationStruct->pChannels)[i].pChannelInfo)
		{
			if (((pCommunicationStruct->pChannels)[i].pChannelInfo)->bIPsecProtected == Enum_TRUE)
			{
				if (((pCommunicationStruct->pChannels)[i].pChannelInfo)->remotePort == remoteSendingPort)
				{
					sendingChannelId = 	(pCommunicationStruct->pChannels)[i].channelIndex;
				}
			}
		}
	}
	else
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\tinput receivedOnChannelId is not found in channel array", __FUNCTION__, __LINE__);
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
EndTag:
	return sendingChannelId;
}

/**************************************************************************************************
Function:            _EcrioTxnMgrCheckRequestReceivedOnProperIPsec()

Purpose:             This function will check whether the incoming request is received

Description:.        This function will check whether the incoming request is received on
the expected/valid protected channel or not.

Input:				 _EcrioTXNDataStruct	*pTxnData
u_int32	remoteSendingPort
u_int32 receivedOnChannelId
BoolEnum	bCheckState

OutPut:              None.

Returns:             BoolEnum.
**************************************************************************************************/
BoolEnum _EcrioTxnMgrCheckRequestReceivedOnProperIPsec(_EcrioTXNDataStruct	*pTxnData, u_int32	remoteSendingPort, u_int32 receivedOnChannelId, BoolEnum	bCheckState)
{
	u_int32	i = 0;
	BoolEnum bReceivedProperly = Enum_FALSE;
	BoolEnum bCommStructMatched = Enum_FALSE;
	EcrioTxnMgrSignalingCommunicationStruct* pCommunicationStruct = NULL;
	EcrioTxnMgrTransportStruct *pTxnStruct = NULL;
	(void)bCheckState;

	if (pTxnData && pTxnData->pTxnInitParam)
	{
		pTxnStruct = (EcrioTxnMgrTransportStruct*)pTxnData->pTxnInitParam->pTxnTransportStruct;
	}

	if (pTxnStruct == NULL ||
		((pTxnStruct->pCommunicationStruct == NULL || pTxnStruct->pCommunicationStruct->pChannels == NULL)))
	{
		goto EndTag;
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pTxnStruct->pCommunicationStruct)
	{
		pCommunicationStruct = pTxnStruct->pCommunicationStruct;

		for (i = 0; i<pCommunicationStruct->noOfChannels; i++)
		{
			if ((pCommunicationStruct->pChannels)[i].channelIndex == receivedOnChannelId)
			{
				bCommStructMatched = Enum_TRUE;
				break;
			}
		}
	}

	if (bCommStructMatched == Enum_TRUE)
	{
		if ((pCommunicationStruct->pChannels)[i].pChannelInfo)
		{
			if (((pCommunicationStruct->pChannels)[i].pChannelInfo)->bIPsecProtected == Enum_TRUE)
			{
				if (pCommunicationStruct->pChannels[i].pChannelInfo->eSocketType == ProtocolType_UDP)
				{
					if (pCommunicationStruct->pChannels[i].pChannelInfo->remotePort == remoteSendingPort)
					{
						bReceivedProperly = Enum_TRUE;
					}
				}
			}
			else
			{
				TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
					"%s:%u\tInput receivedOnChannelId is not protected", __FUNCTION__, __LINE__);
			}
		}
	}
	else
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInput receivedOnChannelId is not found in channel array", __FUNCTION__, __LINE__);
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
EndTag:
	return bReceivedProperly;
}

/**************************************************************************************************
Function:            _EcrioTxnMgrCheckResponseReceivedOnProperIPsec()

Purpose:             This function will check whether the incoming response is received

Description:.        This function will check whether the incoming response is received on the expected/valid
protected channel ( corresponding to the protected channel through which
the request was sent) or not.

Input:				_EcrioTXNDataStruct	*pTxnData
u_int32	remoteSendingPort
u_int32 receivedOnChannelId
u_int32	sendingChannelId
uchar	*pMethodName
u_int32	responseCode

OutPut:              None.

Returns:             BoolEnum.
**************************************************************************************************/
BoolEnum _EcrioTxnMgrCheckResponseReceivedOnProperIPsec
(
	_EcrioTXNDataStruct *pTxnData,
	u_int32	remoteSendingPort, 
	u_int32 receivedOnChannelId, 
	u_int32	sendingChannelId, 
	u_char	*pMethodName, 
	u_int32	responseCode
)
{
	u_int32	i = 0;
	BoolEnum bReceivedProperly = Enum_FALSE;
	BoolEnum bCommStructMatched = Enum_FALSE;
	EcrioTxnMgrSignalingCommunicationStruct *pCommunicationStruct = NULL;
	EcrioTxnMgrTransportStruct *pTxnStruct = NULL;

	if (pTxnData && pTxnData->pTxnInitParam)
	{
		pTxnStruct = (EcrioTxnMgrTransportStruct*)pTxnData->pTxnInitParam->pTxnTransportStruct;
	}

	if (pTxnStruct == NULL ||
		((pTxnStruct->pCommunicationStruct == NULL || pTxnStruct->pCommunicationStruct->pChannels == NULL)))
	{
		goto EndTag;
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
		"%s:%u\tremoteSendingPort=%u, receivedOnChannelId=%u, sendingChannelId=%u",
		__FUNCTION__, __LINE__, remoteSendingPort, receivedOnChannelId, sendingChannelId);

	if (pTxnStruct->pCommunicationStruct != NULL)
	{
		pCommunicationStruct = pTxnStruct->pCommunicationStruct;

		for (i = 0; i<pCommunicationStruct->noOfChannels; i++)
		{
			TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\tchannel_id=%u, bProtected=%d",
				__FUNCTION__, __LINE__, (pCommunicationStruct->pChannels)[i].channelIndex,
				(pCommunicationStruct->pChannels)[i].pChannelInfo->bIPsecProtected);

			if ((pCommunicationStruct->pChannels)[i].channelIndex == receivedOnChannelId)
			{
				bCommStructMatched = Enum_TRUE;
				break;
			}
		}
	}

	if (bCommStructMatched == Enum_TRUE)
	{
		if ((pCommunicationStruct->pChannels)[i].pChannelInfo)
		{
			if (((pCommunicationStruct->pChannels)[i].pChannelInfo)->bIPsecProtected == Enum_TRUE)
			{
				if (pCommunicationStruct->pChannels[i].pChannelInfo->eSocketType == ProtocolType_UDP)
				{
					if (pCommunicationStruct->pChannels[i].pChannelInfo->remotePort == remoteSendingPort)
					{
						bReceivedProperly = Enum_TRUE;
					}
				}
				else if(pCommunicationStruct->pChannels[i].pChannelInfo->eSocketType == ProtocolType_TCP_Client)
				{
					if (pCommunicationStruct->pChannels[i].pChannelInfo->remotePort == remoteSendingPort)
					{
						bReceivedProperly = Enum_TRUE;
					}
				}
			}
			else
			{
				TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
					"%s:%u\tInput receivedOnChannelId is not protected", __FUNCTION__, __LINE__);
			}
		}
	}
	else
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInput receivedOnChannelId is not found in channel array", __FUNCTION__, __LINE__);
	}
	if ((pal_StringCompare(pMethodName, (u_char*)"REGISTER") == 0) && (responseCode != 200))
	{
		bReceivedProperly = Enum_TRUE;
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
EndTag:
	return bReceivedProperly;
}
