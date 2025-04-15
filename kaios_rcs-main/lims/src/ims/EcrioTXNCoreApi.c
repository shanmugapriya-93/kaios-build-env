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
Function:            _EcrioTXNFindMatchingTxn()

Purpose:             This API returns Transaction Node Pointer if input parameters match with stored
                     transactions. Other wise it will return null.

Description:.        This internal function does the matching with stored transaction information about
                     all ongoing transactions based on txnMatchingCriteria parameter.

Input:               _EcrioTXNDataStruct				*pTxnData
                     EcrioTXNTxnMatchingParamStruct		*pTxnMatchingParam
                     EcrioTXNTxnMatchingCriteriaEnum	txnMatchingCriteria
                     EcrioTXNReturnCodeEnum				*pTxnRetCode

OutPut:              Matched Transaction Node Pointer if found else NULL.

Returns:             _EcrioTXNTxnNodeStruct*.
**************************************************************************************************/
_EcrioTXNTxnNodeStruct *_EcrioTXNFindMatchingTxn
(
	_EcrioTXNDataStruct	*pTxnData,
	EcrioTXNTxnMatchingParamStruct *pTxnMatchingParam,
	EcrioTXNTxnMatchingCriteriaEnum	txnMatchingCriteria,
	EcrioTXNReturnCodeEnum *pTxnRetCode
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;
	EcrioTXNTxnMatchingCommonParamStruct *pCommonMatchingParam = NULL;
	BoolEnum bUseTxnMappingParamACK = Enum_FALSE;

	void *pLogHandle = NULL;

	if (!pTxnData)
	{
		*pTxnRetCode = EcrioTXNInsufficientDataError;
		return pTxnNode;
	}

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

// quiet	TXNMGRLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u\tpTxnData=%p\tpTxnMatchingParam=%p\ttxnMatchingCriteria=%u\tpTxnRetCode=%p", 
// quiet		__FUNCTION__, __LINE__, pTxnData, pTxnMatchingParam, txnMatchingCriteria, pTxnRetCode);

	if (!pTxnMatchingParam)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	pCommonMatchingParam = &(pTxnMatchingParam->commonMatchingParam);

	if (!(pCommonMatchingParam->txnIdentifier.pMethodName))
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tpMethodName is NULL", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	if (((pCommonMatchingParam->txnIdentifier.txnType == EcrioTXNTxnTypeClientInvite) ||
		(pCommonMatchingParam->txnIdentifier.txnType == EcrioTXNTxnTypeServerInvite)) &&
		!((pal_StringCompare(pCommonMatchingParam->txnIdentifier.pMethodName,
		ECRIO_TXN_METHOD_NAME_INVITE) == 0) ||
		(pal_StringCompare(pCommonMatchingParam->txnIdentifier.pMethodName,
		ECRIO_TXN_METHOD_NAME_ACK) == 0)))
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tWrong method name passed for INVITE txn", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	if (!((pCommonMatchingParam->txnIdentifier.txnType == EcrioTXNTxnTypeClientInvite) ||
		(pCommonMatchingParam->txnIdentifier.txnType == EcrioTXNTxnTypeServerInvite)) &&
		((pal_StringCompare(pCommonMatchingParam->txnIdentifier.pMethodName,
		ECRIO_TXN_METHOD_NAME_INVITE) == 0) ||
		(pal_StringCompare(pCommonMatchingParam->txnIdentifier.pMethodName,
		ECRIO_TXN_METHOD_NAME_ACK) == 0)))
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tWrong method name passed for Non-INVITE txn", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	if (pal_StringCompare(pCommonMatchingParam->txnIdentifier.pMethodName,
		ECRIO_TXN_METHOD_NAME_ACK))
	{
		if (!pCommonMatchingParam->pBranchParam)
		{
			TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tpBranchParam is NULL", __FUNCTION__, __LINE__);

			txnRetCode = EcrioTXNInsufficientDataError;
			goto EndTag;
		}
	}

	pTxnNode = pTxnData->pTxnListHead;

	while (pTxnNode)
	{
		if (pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier.txnType ==
			pCommonMatchingParam->txnIdentifier.txnType)
		{
			if (!pal_StringCompare(pCommonMatchingParam->txnIdentifier.pMethodName,
				ECRIO_TXN_METHOD_NAME_ACK))
			{
				if (pTxnNode->pTxnMatchingParamACK)
				{
					bUseTxnMappingParamACK = Enum_TRUE;
				}
				else
				{
					bUseTxnMappingParamACK = Enum_FALSE;
				}
			}
			else
			{
				if (pal_StringCompare(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier.pMethodName,
					pCommonMatchingParam->txnIdentifier.pMethodName))
				{
					pTxnNode = pTxnNode->pNext;
					continue;
				}
			}

			if (bUseTxnMappingParamACK == Enum_FALSE)
			{
				txnRetCode = _EcrioTXNMatchCommonMatchingParam(	pTxnData,
					pTxnNode->pTxnMatchingParam,
					pTxnMatchingParam,
					txnMatchingCriteria);
			}
			else
			{
				txnRetCode = _EcrioTXNMatchCommonMatchingParam(	pTxnData,
					pTxnNode->pTxnMatchingParamACK,
					pTxnMatchingParam,
					txnMatchingCriteria);
			}

			if (txnRetCode != EcrioTXNNoError)
			{
				pTxnNode = pTxnNode->pNext;
				continue;
			}

			break;
		}

		pTxnNode = pTxnNode->pNext;
	}

	if (!pTxnNode)
	{
		txnRetCode = EcrioTXNNoMatchingTxnFoundError;
	}

EndTag:

// quiet	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\tError=%u", __FUNCTION__, __LINE__, txnRetCode);

	*pTxnRetCode = txnRetCode;

	return pTxnNode;
}

/**************************************************************************************************
Function:            _EcrioTXNMatchCommonMatchingParam()

Purpose:             This API matches the common parameters between pTxnMatchingParamStored and pTxnMatchingParamInput
                     and returns 0 if matching found and EcrioTXNNoMatchingTxnFoundError if not found.

Description:.        This internal function does the matching between pTxnMatchingParamStored and pTxnMatchingParamInput
                     based on txnMatchingCriteria parameter.

Input:               _EcrioTXNDataStruct				*pTxnData
                     EcrioTXNTxnMatchingParamStruct		*pTxnMatchingParamStored
                     EcrioTXNTxnMatchingParamStruct		*pTxnMatchingParamInput
                     EcrioTXNTxnMatchingCriteriaEnum	txnMatchingCriteria

OutPut:              None.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum _EcrioTXNMatchCommonMatchingParam
(
	_EcrioTXNDataStruct	*pTxnData,
	EcrioTXNTxnMatchingParamStruct *pTxnMatchingParamStored,
	EcrioTXNTxnMatchingParamStruct *pTxnMatchingParamInput,
	EcrioTXNTxnMatchingCriteriaEnum	txnMatchingCriteria
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	BoolEnum bDefaultParamBranch = Enum_TRUE;
	BoolEnum bBranchMatched = Enum_TRUE;
	BoolEnum bCallIdMatched = Enum_TRUE;
	BoolEnum bToTagMatched = Enum_TRUE;
	BoolEnum bFromTagMatched = Enum_TRUE;
	void *pLogHandle = NULL;

	// @todo See if this function can be refactored if these aren't needed.
	(void)pTxnData;
	(void)txnMatchingCriteria;

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

//	TXNMGRLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u\tpTxnData=%p\tpTxnMatchingParamStored=%p\tpTxnMatchingParamInput=%p\ttxnMatchingCriteria=%u", 
//		__FUNCTION__, __LINE__, pTxnData, pTxnMatchingParamStored, pTxnMatchingParamInput, txnMatchingCriteria);

	if (!pal_StringCompare(pTxnMatchingParamInput->commonMatchingParam.txnIdentifier.pMethodName,
		ECRIO_TXN_METHOD_NAME_ACK) &&
		(pTxnMatchingParamStored->commonMatchingParam.responseCode >= 200 &&
		pTxnMatchingParamStored->commonMatchingParam.responseCode < 300))
	{
		if (!pTxnMatchingParamInput->commonMatchingParam.pCallId)
		{
			TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tInput basic common matching param's callId or toTag or fromTag is NULL, pCallId = %x ", \
				__FUNCTION__, __LINE__, \
				pTxnMatchingParamInput->commonMatchingParam.pCallId);

			txnRetCode = EcrioTXNInsufficientDataError;
			goto EndTag;
		}

		if (!pTxnMatchingParamStored->commonMatchingParam.pCallId)
		{
			TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tStored basic common matching param's callId or toTag or fromTag is NULL, pCallId = %x", \
				__FUNCTION__, __LINE__, \
				pTxnMatchingParamInput->commonMatchingParam.pCallId);

			txnRetCode = EcrioTXNInsufficientDataError;
			goto EndTag;
		}

		bDefaultParamBranch = Enum_FALSE;
	}
	else
	{
		if (!pTxnMatchingParamInput->commonMatchingParam.pBranchParam)
		{
			TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tInput basic common matching param's branch is NULL", __FUNCTION__, __LINE__);

			txnRetCode = EcrioTXNInsufficientDataError;
			goto EndTag;
		}

		if (!pTxnMatchingParamStored->commonMatchingParam.pBranchParam)
		{
			TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tStored basic common matching param's branch is NULL", __FUNCTION__, __LINE__);

			txnRetCode = EcrioTXNInsufficientDataError;
			goto EndTag;
		}

		bDefaultParamBranch = Enum_TRUE;
	}

	if (bDefaultParamBranch == Enum_FALSE)
	{
		if (pal_StringCompare(pTxnMatchingParamInput->commonMatchingParam.pCallId,
			pTxnMatchingParamStored->commonMatchingParam.pCallId))
		{
			bCallIdMatched = Enum_FALSE;
		}

		if (pal_StringCompare(pTxnMatchingParamInput->commonMatchingParam.pToTag,
			pTxnMatchingParamStored->commonMatchingParam.pToTag))
		{
			bToTagMatched = Enum_FALSE;
		}

		if (pal_StringCompare(pTxnMatchingParamInput->commonMatchingParam.pFromTag,
			pTxnMatchingParamStored->commonMatchingParam.pFromTag))
		{
			bFromTagMatched = Enum_FALSE;
		}
	}
	else
	{
		if (pal_StringICompare(pTxnMatchingParamInput->commonMatchingParam.pBranchParam,
			pTxnMatchingParamStored->commonMatchingParam.pBranchParam))
		{
			bBranchMatched = Enum_FALSE;
		}
	}

	if ((bBranchMatched == Enum_TRUE) && (bCallIdMatched == Enum_TRUE) &&
		(bToTagMatched == Enum_TRUE) && (bFromTagMatched == Enum_TRUE))
	{
//		TXNMGRLOGV(pLogHandle, KLogTypeGeneral, "%s:%u\tmatching found", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNNoError;
	}
	else
	{
//		TXNMGRLOGE(	pLogHandle,
//			KLogTypeGeneral,
//			"%s:%u\tmatching failed , matching result branch=%d callId=%d", 
//			__FUNCTION__, __LINE__,
//			bBranchMatched,
//			bCallIdMatched);

		txnRetCode = EcrioTXNNoMatchingTxnFoundError;
	}

EndTag:
//	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\tError=%u", __FUNCTION__, __LINE__, txnRetCode);

	return txnRetCode;
}

/**************************************************************************************************
Function:            EcrioTXNSendRequestToClientTxnHandler()

Purpose:             This API sends a SIP Request to SAC.

Description:.        This internal function (and TXN Layer API) for sending this request to network.
                     This function is called for client transactions only.

Input:				 void						*pTxn,
                     EcrioTXNInputParamStruct	*pTxnInputParam
                     BoolEnum					bDeleteFlag

OutPut:              None.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum EcrioTXNSendRequestToClientTxnHandler
(
	void *pTxn,
	EcrioTXNInputParamStruct *pTxnInputParam,
	BoolEnum bDeleteFlag,
	BoolEnum bIPSecFlag
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	u_int32	socketError = KPALErrorNone;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;
	void *pLogHandle = NULL;
	EcrioTXNInitParamStruct *pTxnInitParamL = NULL;
	EcrioTxnMgrTransportStruct *pTxnTransportStructL = NULL;
	u_int32	uChannelIndex = ECRIO_TXN_MAX_NO_OF_CHANNELS;

	if (pTxn == NULL || NULL == pTxnInputParam)
	{
		return EcrioTXNInsufficientDataError;
	}

	pTxnData = (_EcrioTXNDataStruct	*)pTxn;

	pTxnInitParamL = pTxnData->pTxnInitParam;

	if (NULL == pTxnInitParamL)
	{
		return EcrioTXNInsufficientDataError;
	}

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

	TXNMGRLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u\tpTxnData=%p\tpTxnInputParam=%p\tbDeleteFlag=%d", \
		__FUNCTION__, __LINE__, pTxnData, pTxnInputParam, bDeleteFlag);

	pTxnTransportStructL = pTxnInitParamL->pTxnTransportStruct;

	if (pTxnTransportStructL == NULL)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	if ((!pTxnInputParam->pMsg) || (!pTxnInputParam->msgLen) || (pTxnInputParam->messageType != EcrioTXNMessageTypeRequest))
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tpMsg is NULL or msgLen is 0 or not a request", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	pTxnNode = _EcrioTXNFindMatchingTxn(pTxnData,
		&(pTxnInputParam->txnMatchingParam),
		pTxnInputParam->txnMatchingCriteria,
		&txnRetCode);

	if ((txnRetCode != EcrioTXNNoError) && (txnRetCode != EcrioTXNNoMatchingTxnFoundError))
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tunexpected result from _EcrioTXNFindMatchingTxn()", __FUNCTION__, __LINE__);

		pTxnNode = NULL;
		goto EndTag;
	}

	if (!pTxnNode)
	{
		if (bIPSecFlag == Enum_FALSE)
		{
			if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeTCP)
			{
				uChannelIndex = ((EcrioTxnMgrTransportStruct *)(pTxnTransportStructL))->sendingTCPChannelIndex;
			}
			else if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeTLS)
			{
				uChannelIndex = ((EcrioTxnMgrTransportStruct *)(pTxnTransportStructL))->sendingTLSChannelIndex;
			}
			else
			{
				uChannelIndex = ((EcrioTxnMgrTransportStruct *)(pTxnTransportStructL))->sendingChannelIndex;
			}
		}
		else
		{
			if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeTCP)
			{
				uChannelIndex = ((EcrioTxnMgrTransportStruct *)(pTxnTransportStructL))->sendingIPsecTCPChannelIndex;
			}
			else
			{
				uChannelIndex = ((EcrioTxnMgrTransportStruct *)(pTxnTransportStructL))->sendingIPsecUdpChannelIndex;
			}
		}

		TXNMGRLOGI(pLogHandle, KLogTypeGeneral, "%s:%u\t methodName:%s and uChannelIndex:%d",
			__FUNCTION__,
			__LINE__,
			pTxnInputParam->txnMatchingParam.commonMatchingParam.txnIdentifier.pMethodName,
			uChannelIndex);

#ifdef ENABLE_QCMAPI
		socketError = _EcrioTxnMgrSendDataToCommunicationChannel(pTxnData, pTxnInputParam->pMsg, (u_int16)pTxnInputParam->msgLen, uChannelIndex, pTxnInputParam->messageType, pTxnInputParam->txnMatchingParam.commonMatchingParam.pCallId, Enum_FALSE);
#else
		socketError = _EcrioTxnMgrSendDataToCommunicationChannel(pTxnData, pTxnInputParam->pMsg, (u_int16)pTxnInputParam->msgLen, uChannelIndex);
#endif
		if (socketError != KPALErrorNone)
		{
			TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tsending to network failed", __FUNCTION__, __LINE__);

			txnRetCode = EcrioTXNTransportError;
			goto EndTag;
		}

		if (bDeleteFlag == Enum_TRUE)
		{
			TXNMGRLOGI(pLogHandle, KLogTypeGeneral, "%s:%u\tbDeleteFlag is TRUE, hence no txn node will be created", __FUNCTION__, __LINE__);

			txnRetCode = EcrioTXNNoError;
			goto EndTag;
		}

		pTxnNode = _EcrioTXNCreateTxn(	pTxnData,
			pTxnInputParam,
			uChannelIndex,
			&txnRetCode);

		if (!pTxnNode)
		{
			TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\ttxn creation failed", __FUNCTION__, __LINE__);

			goto EndTag;
		}

		TXNMGRLOGI(    pLogHandle,
			KLogTypeGeneral,
			"%s:%u\tnew %s created  and request sent to network", __FUNCTION__, __LINE__,
			_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));
	}
	else
	{
		uChannelIndex = pTxnNode->uChannelIndex;
		TXNMGRLOGI(pLogHandle, KLogTypeGeneral, "%s:%u\t methodName:%s", __FUNCTION__, __LINE__, pTxnInputParam->txnMatchingParam.commonMatchingParam.txnIdentifier.pMethodName);

		if (!pal_StringCompare(pTxnInputParam->txnMatchingParam.commonMatchingParam.txnIdentifier.pMethodName,
			ECRIO_TXN_METHOD_NAME_ACK))
		{
			/* This means that an ACK request against a negative final response
			    for an INVITE is being sent out

			    Please note that no action is being taken with respect to the
			    value of the bDeleteFlag.
			*/

			if (pTxnNode->txnState == EcrioTXNTxnStateProgressing)
			{
				/* Send the ACK Message out	*/
				if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeTCP)
				{
					if (uChannelIndex == ECRIO_TXN_MAX_NO_OF_CHANNELS)
					{
						uChannelIndex = ((EcrioTxnMgrTransportStruct *)(pTxnTransportStructL))->sendingTCPChannelIndex;
					}
				}
				else
				{
					if (uChannelIndex == ECRIO_TXN_MAX_NO_OF_CHANNELS)
					{
						uChannelIndex = ((EcrioTxnMgrTransportStruct *)(pTxnTransportStructL))->sendingChannelIndex;
					}
				}

#ifdef ENABLE_QCMAPI
				socketError = _EcrioTxnMgrSendDataToCommunicationChannel(pTxnData, pTxnInputParam->pMsg, pTxnInputParam->msgLen, uChannelIndex, pTxnInputParam->messageType, pTxnInputParam->txnMatchingParam.commonMatchingParam.pCallId, Enum_FALSE);
#else
				socketError = _EcrioTxnMgrSendDataToCommunicationChannel(pTxnData, pTxnInputParam->pMsg, pTxnInputParam->msgLen, uChannelIndex);
#endif
				if (socketError != KPALErrorNone)
				{
					TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tsending to network failed", __FUNCTION__, __LINE__);

					txnRetCode = EcrioTXNTransportError;
					goto EndTag;
				}

				if (pTxnNode->pMsg != NULL)
				{
					pal_MemoryFree((void **)&pTxnNode->pMsg);
				}

				pTxnNode->msgLen = 0;

				/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
				if (pal_UtilityDataOverflowDetected(pTxnInputParam->msgLen, sizeof(u_char)) == Enum_TRUE)
				{
					txnRetCode = EcrioTXNInsufficientMemoryError;
					goto EndTag;
				}

				pal_MemoryAllocate(1 + (pTxnInputParam->msgLen) * sizeof(u_char), (void **)&(pTxnNode->pMsg));

				if (!pTxnNode->pMsg)
				{
					txnRetCode = EcrioTXNInsufficientMemoryError;
					goto EndTag;
				}

				pal_MemoryCopy(
					pTxnNode->pMsg,
					pTxnInputParam->msgLen,
					pTxnInputParam->pMsg,
					pTxnInputParam->msgLen);

				pTxnNode->pMsg[pTxnInputParam->msgLen] = '\0';

				pTxnNode->msgLen = pTxnInputParam->msgLen;

				/* STOP: Copy the ACK buffer into a new buffer. Please reconsider shallow copying later */

				pTxnNode->retransmissionContext = (s_int32)(pTxnInputParam->retransmitContext);

				txnRetCode = _EcrioTXNCopyTxnMatchingParamACK(pTxnData,
					pTxnInputParam,
					pTxnNode);
				if (txnRetCode != EcrioTXNNoError)
				{
					TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tcopy to pTxnMatchingParamACK failed", __FUNCTION__, __LINE__);

					goto EndTag;
				}

				if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeUDP)
				{
					/*	Start Timer D	*/
					txnRetCode = _EcrioTXNStartTxnTimer(pTxnData,
						pTxnNode,
						EcrioTXNWaitTimer);

					if (txnRetCode != EcrioTXNNoError)
					{
						TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\ttimer start failed", __FUNCTION__, __LINE__);

						goto EndTag;
					}
				}

				TXNMGRLOGI(pLogHandle,
					KLogTypeGeneral,
					"%s:%u\t-> txnState is transiting from %s to %s of the transaction %s", __FUNCTION__, __LINE__,
					_EcrioTXNGetTxnStateName(pTxnNode->txnState),
					_EcrioTXNGetTxnStateName(EcrioTXNTxnStateCompleted),
					_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

				/*	txn state change	*/
				pTxnNode->txnState = EcrioTXNTxnStateCompleted;
			}
			else
			{
				/*  ACK should not come in @ this stage. Throw error. */
				TXNMGRLOGE(pLogHandle,
					KLogTypeGeneral,
					"%s:%u\t-> ACK is not expected in the txnState %s", __FUNCTION__, __LINE__,
					_EcrioTXNGetTxnStateName(pTxnNode->txnState));

				pTxnNode = NULL;
				txnRetCode = EcrioTXNMsgReceivedInWrongStateError;
				goto EndTag;
			}
		}
		else
		{
			TXNMGRLOGE(pLogHandle,
				KLogTypeGeneral,
				"%s:%u\t-> request is not expected in the txnState %s", __FUNCTION__, __LINE__,
				_EcrioTXNGetTxnStateName(pTxnNode->txnState));

			pTxnNode = NULL;
			txnRetCode = EcrioTXNMsgReceivedInWrongStateError;
			goto EndTag;
		}
	}

	pTxnNode = NULL;

EndTag:

	if (pTxnNode)
	{
		_EcrioTXNDeleteTxn(	pTxnData,
			pTxnNode);
	}

	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\tError=%u", __FUNCTION__, __LINE__, txnRetCode);

	return txnRetCode;
}

/**************************************************************************************************
Function:            EcrioTXNSendResponseToClientTxnHandler()

Purpose:             This API sends a SIP Response to SAC.

Description:.        This internal function send the response to SAC for sending this response to network.
                     This function is called for client transaction only.

Input:				 void						*pTxn,
                     EcrioTXNInputParamStruct	*pTxnInputParam
                     BoolEnum					bDeleteFlag

OutPut:              None.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum EcrioTXNSendResponseToClientTxnHandler
(
	void *pTxn,
	EcrioTXNInputParamStruct *pTxnInputParam,
	BoolEnum bDeleteFlag
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;
	EcrioTXNInitParamStruct *pTxnInitParamL = NULL;
	void *pLogHandle = NULL;
	EcrioTxnMgrTransportStruct *pTxnTransportStructL = NULL;
	u_int32	uChannelIndex = ECRIO_TXN_MAX_NO_OF_CHANNELS;
	u_int32	socketError = KPALErrorNone;

	if (!pTxn || !pTxnInputParam)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	pTxnData = (_EcrioTXNDataStruct	*)pTxn;

	pTxnInitParamL = pTxnData->pTxnInitParam;

	if (NULL == pTxnInitParamL || pTxnInitParamL->pTxnTransportStruct == NULL)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		return txnRetCode;
	}

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

	TXNMGRLOGI(	pLogHandle, \
		KLogTypeFuncEntry, \
		"%s:%u\tpTxnData = %x pTxnInputParam =%x bDeleteFlag =%d", \
		__FUNCTION__, __LINE__,	pTxnData, pTxnInputParam, bDeleteFlag);

	pTxnTransportStructL = pTxnInitParamL->pTxnTransportStruct;

	if (pTxnInputParam->messageType != EcrioTXNMessageTypeResponse ||
		!(pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode >= 100 &&
		pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode < 700))
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tnot a response or invalid response code", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	pTxnNode = _EcrioTXNFindMatchingTxn(pTxnData,
		&(pTxnInputParam->txnMatchingParam),
		pTxnInputParam->txnMatchingCriteria,
		&txnRetCode);

	if ((txnRetCode != EcrioTXNNoError) && (txnRetCode != EcrioTXNNoMatchingTxnFoundError))
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tunexpected result from _EcrioTXNFindMatchingTxn()", __FUNCTION__, __LINE__);

		pTxnNode = NULL;
		goto EndTag;
	}

	if (!pTxnNode)
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tmatching txn expected but not found", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNNoMatchingTxnFoundError;
		goto EndTag;
	}
	else
	{
		uChannelIndex = pTxnNode->uChannelIndex;

		if (bDeleteFlag == Enum_TRUE)
		{
			TXNMGRLOGI(pLogHandle,
				KLogTypeGeneral,
				"%s:%u\t-> bDeleteFlag is TRUE, hence txn node will be deleted", __FUNCTION__, __LINE__);

			txnRetCode = EcrioTXNNoError;
			goto EndTag;
		}

		if (pTxnInputParam->txnMatchingParam.commonMatchingParam.txnIdentifier.txnType == EcrioTXNTxnTypeClientInvite)
		{
			/*	INVITE client TXN	*/
			switch (pTxnNode->txnState)
			{
				case EcrioTXNTxnStateCalling:
				{
					if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeUDP)
					{
						/*	Stop Timer A	*/
						_EcrioTXNStopTxnTimer(pTxnData,
							pTxnNode,
							EcrioTXNRetransmitTimer);
					}

					if ((pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode >= 100) &&
						(pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode < 200))
					{
						/*	Stop Timer B	*/
						_EcrioTXNStopTxnTimer(pTxnData,
												pTxnNode,
												EcrioTXNTimeoutTimer);

						TXNMGRLOGI(pLogHandle,
							KLogTypeGeneral,
							"%s:%u\t-> txnState is transiting from %s to %s of the transaction %s", __FUNCTION__, __LINE__,
							_EcrioTXNGetTxnStateName(pTxnNode->txnState),
							_EcrioTXNGetTxnStateName(EcrioTXNTxnStateProceeding),
							_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

						pTxnNode->txnState = EcrioTXNTxnStateProceeding;
					}
					else if ((pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode >= 200) &&
						(pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode < 300))
					{
						/*	Stop Timer B	*/
						_EcrioTXNStopTxnTimer(pTxnData,
							pTxnNode,
							EcrioTXNTimeoutTimer);

						TXNMGRLOGI(pLogHandle,
							KLogTypeGeneral,
							"%s:%u\t-> txnState is transiting from %s to %s of the transaction %s", __FUNCTION__, __LINE__,
							_EcrioTXNGetTxnStateName(pTxnNode->txnState),
							_EcrioTXNGetTxnStateName(EcrioTXNTxnStateTerminated),
							_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

						pTxnNode->txnState = EcrioTXNTxnStateTerminated;

						txnRetCode = EcrioTXNNoError;
						goto EndTag;
					}
					else
					{
						/*	Stop Timer B	*/
						_EcrioTXNStopTxnTimer(pTxnData,
							pTxnNode,
							EcrioTXNTimeoutTimer);

						TXNMGRLOGI(    pLogHandle,
							KLogTypeGeneral,
							"%s:%u\t-> txnState is transiting from %s to %s of the transaction %s", __FUNCTION__, __LINE__,
							_EcrioTXNGetTxnStateName(pTxnNode->txnState),
							_EcrioTXNGetTxnStateName(EcrioTXNTxnStateProgressing),
							_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

						pTxnNode->txnState = EcrioTXNTxnStateProgressing;
					}

					txnRetCode = _EcrioTXNUpdateBasicTxnMatchingParam(pTxnData,
						pTxnNode,
						pTxnInputParam);
					if (txnRetCode != EcrioTXNNoError)
					{
						TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tupdate of basic txn matching param failed", __FUNCTION__, __LINE__);

						pTxnNode = NULL;
						goto EndTag;
					}

					break;
				}

				case EcrioTXNTxnStateProceeding:
				{
					if ((pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode >= 200) &&
						(pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode < 300))
					{
						/*	Stop Timer B	*/
						_EcrioTXNStopTxnTimer(pTxnData,
							pTxnNode,
							EcrioTXNTimeoutTimer);

						TXNMGRLOGI(    pLogHandle,
							KLogTypeGeneral,
							"%s:%u\t-> txnState is transiting from %s to %s of the transaction %s", __FUNCTION__, __LINE__,
							_EcrioTXNGetTxnStateName(pTxnNode->txnState),
							_EcrioTXNGetTxnStateName(EcrioTXNTxnStateTerminated),
							_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

						pTxnNode->txnState = EcrioTXNTxnStateTerminated;

						txnRetCode = EcrioTXNNoError;
						goto EndTag;
					}
					else if ((pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode >= 300) &&
						(pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode < 700))
					{
						/*	Stop Timer B	*/
						_EcrioTXNStopTxnTimer(pTxnData,
							pTxnNode,
							EcrioTXNTimeoutTimer);

						TXNMGRLOGI(    pLogHandle,
							KLogTypeGeneral,
							"%s:%u\t-> txnState is transiting from %s to %s of the transaction %s", __FUNCTION__, __LINE__,
							_EcrioTXNGetTxnStateName(pTxnNode->txnState),
							_EcrioTXNGetTxnStateName(EcrioTXNTxnStateProgressing),
							_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

						pTxnNode->txnState = EcrioTXNTxnStateProgressing;
					}

					txnRetCode = _EcrioTXNUpdateBasicTxnMatchingParam(pTxnData,
						pTxnNode,
						pTxnInputParam);
					if (txnRetCode != EcrioTXNNoError)
					{
						TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tupdate of basic txn matching param failed", __FUNCTION__, __LINE__);

						pTxnNode = NULL;
						goto EndTag;
					}

					break;
				}

				case EcrioTXNTxnStateProgressing:
				{
					if ((pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode >= 200) &&
						(pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode < 700))
					{
						TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tretransmitted response received", __FUNCTION__, __LINE__);

						txnRetCode = EcrioTXNRetransmittedMsgReceivedError;
					}
					else
					{
						TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tresponse received in wrong txn state", __FUNCTION__, __LINE__);

						txnRetCode = EcrioTXNMsgReceivedInWrongStateError;
					}

					break;
				}

				case EcrioTXNTxnStateCompleted:
				{
					if ((pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode >= 200) &&
						(pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode < 700))
					{
						if ((pTxnNode->msgLen > 0) && (pTxnNode->pMsg))
						{
							if (ECRIO_TXN_MAX_NO_OF_CHANNELS == uChannelIndex)
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

#ifdef ENABLE_QCMAPI
							socketError = _EcrioTxnMgrSendDataToCommunicationChannel(pTxnData, pTxnNode->pMsg, pTxnNode->msgLen, uChannelIndex, pTxnInputParam->messageType, pTxnInputParam->txnMatchingParam.commonMatchingParam.pCallId, Enum_TRUE);
#else
							socketError = _EcrioTxnMgrSendDataToCommunicationChannel(pTxnData, pTxnNode->pMsg, pTxnNode->msgLen, uChannelIndex);
#endif
							if (socketError != KPALErrorNone)
							{
								TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tsending to network failed", __FUNCTION__, __LINE__);

								txnRetCode = EcrioTXNTransportError;
								goto EndTag;
							}

							TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tretransmitted response received", __FUNCTION__, __LINE__);

							txnRetCode = EcrioTXNRetransmittedMsgReceivedError;
						}
						else
						{
							txnRetCode = EcrioTXNInsufficientDataError;
							goto EndTag;
						}

						TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tretransmitted response received", __FUNCTION__, __LINE__);

						txnRetCode = EcrioTXNRetransmittedMsgReceivedError;
					}
					else
					{
						TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tresponse received in wrong txn state", __FUNCTION__, __LINE__);

						txnRetCode = EcrioTXNMsgReceivedInWrongStateError;
					}

					break;
				}

				default:
				{
					TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tresponse received in wrong txn state", __FUNCTION__, __LINE__);

					txnRetCode = EcrioTXNMsgReceivedInWrongStateError;

					break;
				}
			}
		}
		else
		{
			/*	NON-INVITE client TXN	*/
			switch (pTxnNode->txnState)
			{
				case EcrioTXNTxnStateTrying:
				{
					if ((pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode >= 100) &&
						(pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode < 200))
					{
						TXNMGRLOGI(    pLogHandle,
							KLogTypeGeneral,
							"%s:%u\t-> txnState is transiting from %s to %s of the transaction %s", __FUNCTION__, __LINE__,
							_EcrioTXNGetTxnStateName(pTxnNode->txnState),
							_EcrioTXNGetTxnStateName(EcrioTXNTxnStateProceeding),
							_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

						pTxnNode->txnState = EcrioTXNTxnStateProceeding;
					}
					else
					{
						if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeUDP)
						{
							/*	Stop Timer E & F	*/
							_EcrioTXNStopTxnTimer(	pTxnData,
								pTxnNode,
								(EcrioTXNTimerTypeEnum)(((u_char)EcrioTXNRetransmitTimer) | ((u_char)EcrioTXNTimeoutTimer)));
						}
						else
						{
							/*	Stop Timer F	*/
							_EcrioTXNStopTxnTimer(	pTxnData,
								pTxnNode,
								EcrioTXNTimeoutTimer);
						}

						TXNMGRLOGD(    pLogHandle,
							KLogTypeGeneral,
							"%s:%u\t-> txnState is transiting from %s to %s of the transaction %s", __FUNCTION__, __LINE__,
							_EcrioTXNGetTxnStateName(pTxnNode->txnState),
							_EcrioTXNGetTxnStateName(EcrioTXNTxnStateCompleted),
							_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

						pTxnNode->txnState = EcrioTXNTxnStateCompleted;

						if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeUDP)
						{
							/*	Start Timer K	*/
							txnRetCode = _EcrioTXNStartTxnTimer(pTxnData,
								pTxnNode,
								EcrioTXNWaitTimer);
							if (txnRetCode != EcrioTXNNoError)
							{
								TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tstart timer failed", __FUNCTION__, __LINE__);

								goto EndTag;
							}
						}
					}

					txnRetCode = _EcrioTXNUpdateBasicTxnMatchingParam(pTxnData,
						pTxnNode,
						pTxnInputParam);
					if (txnRetCode != EcrioTXNNoError)
					{
						TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tupdate of basic txn matching param failed", __FUNCTION__, __LINE__);

						pTxnNode = NULL;
						goto EndTag;
					}

					break;
				}

				case EcrioTXNTxnStateProceeding:
				{
					if ((pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode >= 200) &&
						(pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode < 700))
					{
						// if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeUDP)
						{
							if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeUDP)
							{
								/*	Stop Timer E & F	*/
								_EcrioTXNStopTxnTimer(	pTxnData,
									pTxnNode,
									(EcrioTXNTimerTypeEnum)(((u_char)EcrioTXNRetransmitTimer) | ((u_char)EcrioTXNTimeoutTimer)));
							}
							else
							{
								/*	Stop Timer  F	*/
								_EcrioTXNStopTxnTimer(	pTxnData,
									pTxnNode,
									EcrioTXNTimeoutTimer);
							}

							if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeUDP)
							{
								txnRetCode = _EcrioTXNStartTxnTimer(pTxnData,
									pTxnNode,
									EcrioTXNWaitTimer);
								if (txnRetCode != EcrioTXNNoError)
								{
									TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tstart timer failed", __FUNCTION__, __LINE__);

									goto EndTag;
								}
							}

							TXNMGRLOGI(    pLogHandle,
								KLogTypeGeneral,
								"%s:%u\t-> txnState is transiting from %s to %s of the transaction %s", __FUNCTION__, __LINE__,
								_EcrioTXNGetTxnStateName(pTxnNode->txnState),
								_EcrioTXNGetTxnStateName(EcrioTXNTxnStateCompleted),
								_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

							pTxnNode->txnState = EcrioTXNTxnStateCompleted;
						}
					}

					txnRetCode = _EcrioTXNUpdateBasicTxnMatchingParam(pTxnData,
						pTxnNode,
						pTxnInputParam);
					if (txnRetCode != EcrioTXNNoError)
					{
						TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tupdate of basic txn matching param failed", __FUNCTION__, __LINE__);

						pTxnNode = NULL;
						goto EndTag;
					}

					break;
				}

				case EcrioTXNTxnStateCompleted:
				{
					if ((pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode >= 200) &&
						(pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode < 700))
					{
						TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tretransmitted response received", __FUNCTION__, __LINE__);

						txnRetCode = EcrioTXNRetransmittedMsgReceivedError;
					}
					else
					{
						TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tresponse received in wrong txn state", __FUNCTION__, __LINE__);

						txnRetCode = EcrioTXNMsgReceivedInWrongStateError;
					}

					break;
				}

				default:
				{
					TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tresponse received in wrong txn state", __FUNCTION__, __LINE__);

					txnRetCode = EcrioTXNMsgReceivedInWrongStateError;

					break;
				}
			}
		}
	}

	pTxnNode = NULL;

EndTag:

	if (pTxnNode)
	{
		_EcrioTXNDeleteTxn(	pTxnData,
			pTxnNode);
	}

	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\tError=%u", __FUNCTION__, __LINE__, txnRetCode);

	return txnRetCode;
}

/**************************************************************************************************
Function:            EcrioTXNSendRequestToServerTxnHandler()

Purpose:             This API sends a SIP Request to SAC.

Description:.        This internal function is invoked when
                        a. An incoming request has been received and an associated server txn needs to be created
                        b. To retransmit a response to an incoming request that got retransmitted from the remote end.

                     This function is called for server transaction only.

Input:				 void						*pTxn,
                     EcrioTXNInputParamStruct	*pTxnInputParam
                     BoolEnum					bDeleteFlag

OutPut:              None.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum EcrioTXNSendRequestToServerTxnHandler
(
	void *pTxn,
	EcrioTXNInputParamStruct *pTxnInputParam,
	BoolEnum bDeleteFlag
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	u_int32	socketError = KPALErrorNone;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;
	EcrioTXNInitParamStruct *pTxnInitParamL = NULL;
	void *pLogHandle = NULL;
	EcrioTxnMgrTransportStruct *pTxnTransportStructL = NULL;
	u_int32	uChannelIndex = ECRIO_TXN_MAX_NO_OF_CHANNELS;
	BoolEnum bReceivedOnProperIPsec = Enum_FALSE;

	if (pTxn == NULL || pTxnInputParam == NULL || pTxnInputParam->messageType != EcrioTXNMessageTypeRequest)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	pTxnData = (_EcrioTXNDataStruct	*)pTxn;

	pTxnInitParamL = pTxnData->pTxnInitParam;

	if (NULL == pTxnInitParamL || pTxnInitParamL->pTxnTransportStruct == NULL)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		return txnRetCode;
	}

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

	TXNMGRLOGI(	pLogHandle, \
		KLogTypeFuncEntry, \
		"%s:%u\tpTxnData = %x pTxnInputParam =%x bDeleteFlag =%d", __FUNCTION__, __LINE__,
		pTxnData,
		pTxnInputParam,
		bDeleteFlag);

	pTxnTransportStructL = pTxnInitParamL->pTxnTransportStruct;

	pTxnNode = _EcrioTXNFindMatchingTxn(pTxnData,
		&(pTxnInputParam->txnMatchingParam),
		pTxnInputParam->txnMatchingCriteria,
		&txnRetCode);

	if ((txnRetCode != EcrioTXNNoError) && (txnRetCode != EcrioTXNNoMatchingTxnFoundError))
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tunexpected result from _EcrioTXNFindMatchingTxn()", __FUNCTION__, __LINE__);

		pTxnNode = NULL;
		goto EndTag;
	}

	if (!pTxnNode)
	{
		if (bDeleteFlag == Enum_TRUE)
		{
			TXNMGRLOGI(pLogHandle, KLogTypeGeneral, "%s:%u\tbDeleteFlag is TRUE, hence no txn node will be created", __FUNCTION__, __LINE__);

			txnRetCode = EcrioTXNNoError;
			goto EndTag;
		}

		TXNMGRLOGI(pLogHandle, KLogTypeGeneral, "%s:%u\tmethodName:%s", __FUNCTION__, __LINE__,
			pTxnInputParam->txnMatchingParam.commonMatchingParam.txnIdentifier.pMethodName);

		if (!pal_StringCompare(pTxnInputParam->txnMatchingParam.commonMatchingParam.txnIdentifier.pMethodName,
			ECRIO_TXN_METHOD_NAME_ACK))
		{
			TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tmatched txn expected but not found", __FUNCTION__, __LINE__);

			txnRetCode = EcrioTXNNoMatchingTxnFoundError;
			goto EndTag;
		}

		if (pTxnInputParam->uReceivedChannelIndex != ECRIO_TXN_MAX_NO_OF_CHANNELS)
		{
			uChannelIndex = pTxnInputParam->uReceivedChannelIndex;
		}
		if (pTxnTransportStructL->bIPsecEnabled == Enum_TRUE)
		{
			/*	If IPsec is enabled, check that the incoming request is received on proper SA.	*/
			bReceivedOnProperIPsec = _EcrioTxnMgrCheckRequestReceivedOnProperIPsec(pTxnData,pTxnInputParam->uRemoteSendingPort,pTxnInputParam->uReceivedChannelIndex,Enum_TRUE);
			if(bReceivedOnProperIPsec != Enum_TRUE)
			{
				TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\trequest is not properly protected, hence not creating the transaction", __FUNCTION__, __LINE__);

				txnRetCode = EcrioTXNMessageNotProperlyProtected;
				goto EndTag;
			}

			/*	If IPsec is enabled, convert the channel index and store it so that data can be sent through it.	*/
			uChannelIndex = _EcrioTxnMgrModifyReceivedOnChannelIndex(pTxnData,pTxnInputParam->uRemoteSendingPort,pTxnInputParam->uReceivedChannelIndex);
			if(uChannelIndex == 0)
			{
				TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tno matching protected client channel found to send response, hence aborting", __FUNCTION__, __LINE__);

				txnRetCode = EcrioTXNMessageNotProperlyProtected;
				goto EndTag;
			}
		}

		pTxnNode = _EcrioTXNCreateTxn(	pTxnData,
			pTxnInputParam,
			uChannelIndex,
			&txnRetCode);

		if (!pTxnNode)
		{
			TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\ttxn creation failed", __FUNCTION__, __LINE__);

			goto EndTag;
		}

		TXNMGRLOGI(    pLogHandle,
			KLogTypeGeneral,
			"%s:%u\t-> new %s created", __FUNCTION__, __LINE__,
			_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));
	}
	else
	{
		u_int32	uTempChannelIndex = 0;

		if (bDeleteFlag == Enum_TRUE)
		{
			TXNMGRLOGI(    pLogHandle,
				KLogTypeGeneral,
				"%s:%u\t-> bDeleteFlag is TRUE, hence txn node will be deleted \r\n", __FUNCTION__, __LINE__);

			txnRetCode = EcrioTXNNoError;
			goto EndTag;
		}

		uChannelIndex = pTxnNode->uChannelIndex;

		TXNMGRLOGI(pLogHandle, KLogTypeGeneral, "%s:%u\tmethodName:%s", __FUNCTION__, __LINE__,
			pTxnInputParam->txnMatchingParam.commonMatchingParam.txnIdentifier.pMethodName);
		if (pTxnTransportStructL->bIPsecEnabled == Enum_TRUE)
		{
			/*	If IPsec is enabled, check that the incoming request is protected or not.	*/
			bReceivedOnProperIPsec = _EcrioTxnMgrCheckRequestReceivedOnProperIPsec(pTxnData,pTxnInputParam->uRemoteSendingPort,pTxnInputParam->uReceivedChannelIndex,Enum_FALSE);
			if(bReceivedOnProperIPsec != Enum_TRUE)
			{
				TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\trequest is not properly protected", __FUNCTION__, __LINE__);

				txnRetCode = EcrioTXNMessageNotProperlyProtected;
				goto EndTag;
			}

			/*	If IPsec is enabled, convert the channel index and match it with the previously stored sending channel index .	*/
			uTempChannelIndex = _EcrioTxnMgrModifyReceivedOnChannelIndex(pTxnData,pTxnInputParam->uRemoteSendingPort,pTxnInputParam->uReceivedChannelIndex);
			if(uTempChannelIndex == 0 || uTempChannelIndex != uChannelIndex)
			{
				TXNMGRLOGE (	pLogHandle,
								KLogTypeGeneral,
								"%s:%u\t-> no matching protected client channel found to send response, hence aborting.uTempChannelIndex = %u uChannelIndex = %u", __FUNCTION__, __LINE__,
								uTempChannelIndex,uChannelIndex);

				txnRetCode = EcrioTXNMessageNotProperlyProtected;
				goto EndTag;
			}
		}


		if (!pal_StringCompare(pTxnInputParam->txnMatchingParam.commonMatchingParam.txnIdentifier.pMethodName,
			ECRIO_TXN_METHOD_NAME_INVITE))
		{
			/*	INVITE Server TXN	*/
			switch (pTxnNode->txnState)
			{
				case EcrioTXNTxnStateProceeding:
				case EcrioTXNTxnStateCompleted:
				{
					if ((pTxnNode->msgLen > 0) && (pTxnNode->pMsg))
					{
						if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeTCP)
						{
							if (uChannelIndex == ECRIO_TXN_MAX_NO_OF_CHANNELS)
							{
								uChannelIndex = ((EcrioTxnMgrTransportStruct *)(pTxnTransportStructL))->sendingTCPChannelIndex;
							}
						}
						else
						{
							if (uChannelIndex == ECRIO_TXN_MAX_NO_OF_CHANNELS)
							{
								uChannelIndex = ((EcrioTxnMgrTransportStruct *)(pTxnTransportStructL))->sendingChannelIndex;
							}
						}

#ifdef ENABLE_QCMAPI
						socketError = _EcrioTxnMgrSendDataToCommunicationChannel(pTxnData, pTxnNode->pMsg, pTxnNode->msgLen, uChannelIndex, pTxnInputParam->messageType, pTxnInputParam->txnMatchingParam.commonMatchingParam.pCallId, Enum_FALSE);
#else
						socketError = _EcrioTxnMgrSendDataToCommunicationChannel(pTxnData, pTxnNode->pMsg, pTxnNode->msgLen, uChannelIndex);
#endif

						if (socketError != KPALErrorNone)
						{
							TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tsending to network failed", __FUNCTION__, __LINE__);

							txnRetCode = EcrioTXNTransportError;
							goto EndTag;
						}
					}

					TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tretransmitted request received", __FUNCTION__, __LINE__);

					txnRetCode = EcrioTXNRetransmittedMsgReceivedError;

					break;
				}

				default:
				{
					TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\trequest received in wrong txn state", __FUNCTION__, __LINE__);

					txnRetCode = EcrioTXNMsgReceivedInWrongStateError;

					break;
				}
			}
		}
		else if (!pal_StringCompare(pTxnInputParam->txnMatchingParam.commonMatchingParam.txnIdentifier.pMethodName,
			ECRIO_TXN_METHOD_NAME_ACK))
		{
			/*	INVITE Server TXN	*/
			switch (pTxnNode->txnState)
			{
				case EcrioTXNTxnStateCompleted:
				{
					// if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeUDP)
					{
						if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeUDP)
						{
							/*	Stop Timer G & H	*/
							_EcrioTXNStopTxnTimer(	pTxnData,
								pTxnNode,
								(EcrioTXNTimerTypeEnum)(((u_char)EcrioTXNRetransmitTimer) | ((u_char)EcrioTXNTimeoutTimer)));
						}
						else
						{
							/*	Stop Timer H	*/
							_EcrioTXNStopTxnTimer(	pTxnData,
								pTxnNode,
								EcrioTXNTimeoutTimer);
						}

						txnRetCode = _EcrioTXNCopyTxnMatchingParamACK(pTxnData,
							pTxnInputParam,
							pTxnNode);
						if (txnRetCode != EcrioTXNNoError)
						{
							TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tcopy to pTxnMatchingParamACK failed", __FUNCTION__, __LINE__);

							goto EndTag;
						}

						TXNMGRLOGI(    pLogHandle,
							KLogTypeGeneral,
							"%s:%u\t-> txnState is transiting from %s to %s of the transaction %s", __FUNCTION__, __LINE__,
							_EcrioTXNGetTxnStateName(pTxnNode->txnState),
							_EcrioTXNGetTxnStateName(EcrioTXNTxnStateConfirmed),
							_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

						pTxnNode->txnState = EcrioTXNTxnStateConfirmed;

						if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeUDP)
						{
							/*	Start Timer I	*/
							txnRetCode = _EcrioTXNStartTxnTimer(pTxnData,
								pTxnNode,
								EcrioTXNWaitTimer);
							if (txnRetCode != EcrioTXNNoError)
							{
								TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tstart timer failed", __FUNCTION__, __LINE__);

								goto EndTag;
							}
						}
					}
				}
				break;

				case EcrioTXNTxnStateConfirmed:
				{
					TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tretransmitted request received", __FUNCTION__, __LINE__);

					txnRetCode = EcrioTXNRetransmittedMsgReceivedError;

					break;
				}

				default:
				{
					TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\trequest received in wrong txn state", __FUNCTION__, __LINE__);

					txnRetCode = EcrioTXNMsgReceivedInWrongStateError;

					break;
				}
			}
		}
		else
		{
			/*	NON-INVITE Server TXN	*/
			switch (pTxnNode->txnState)
			{
				case EcrioTXNTxnStateTrying:
				{
					TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tretransmitted request received", __FUNCTION__, __LINE__);

					txnRetCode = EcrioTXNRetransmittedMsgReceivedError;

					break;
				}

				case EcrioTXNTxnStateProceeding:
				case EcrioTXNTxnStateCompleted:
				{
					if ((pTxnNode->msgLen > 0) && (pTxnNode->pMsg))
					{
						if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeTCP)
						{
							if (uChannelIndex == ECRIO_TXN_MAX_NO_OF_CHANNELS)
							{
								uChannelIndex = ((EcrioTxnMgrTransportStruct *)(pTxnTransportStructL))->sendingTCPChannelIndex;
							}
						}
						else
						{
							if (uChannelIndex == ECRIO_TXN_MAX_NO_OF_CHANNELS)
							{
								uChannelIndex = ((EcrioTxnMgrTransportStruct *)(pTxnTransportStructL))->sendingChannelIndex;
							}
						}

#ifdef ENABLE_QCMAPI
						socketError = _EcrioTxnMgrSendDataToCommunicationChannel(pTxnData, pTxnNode->pMsg, pTxnNode->msgLen, uChannelIndex, pTxnInputParam->messageType, pTxnInputParam->txnMatchingParam.commonMatchingParam.pCallId, Enum_FALSE);
#else
						socketError = _EcrioTxnMgrSendDataToCommunicationChannel(pTxnData, pTxnNode->pMsg, pTxnNode->msgLen, uChannelIndex);
#endif

						TXNMGRLOGE(pLogHandle,
							KLogTypeGeneral,
							"%s:%u\t-> Retransmitting %s response", __FUNCTION__, __LINE__,
							pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier.pMethodName);

						if (socketError != KPALErrorNone)
						{
							TXNMGRLOGE(    pLogHandle,
								KLogTypeGeneral,
								"%s:%u\t-> sending to network failed", __FUNCTION__, __LINE__);

							txnRetCode = EcrioTXNTransportError;
							goto EndTag;
						}
					}

					TXNMGRLOGE(    pLogHandle,
						KLogTypeGeneral,
						"%s:%u\tretransmitted request received", __FUNCTION__, __LINE__);

					txnRetCode = EcrioTXNRetransmittedMsgReceivedError;

					break;
				}

				default:
				{
					TXNMGRLOGE(    pLogHandle,
						KLogTypeGeneral,
						"%s:%u\t-> request received in wrong txn state", __FUNCTION__, __LINE__);

					txnRetCode = EcrioTXNMsgReceivedInWrongStateError;

					break;
				}
			}
		}
	}

	pTxnNode = NULL;

EndTag:

	if (pTxnNode)
	{
		_EcrioTXNDeleteTxn(	pTxnData,
			pTxnNode);
	}

	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\tError=%u", __FUNCTION__, __LINE__, txnRetCode);

	return txnRetCode;
}

/**************************************************************************************************
Function:            EcrioTXNSendResponseToServerTxnHandler()

Purpose:             This API sends a SIP Response to SAC.

Description:.        This internal function is invoked to send the response to an incoming request.

Input:				 void						*pTxn,
                     EcrioTXNInputParamStruct	*pTxnInputParam
                     BoolEnum					bDeleteFlag

OutPut:              None.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum EcrioTXNSendResponseToServerTxnHandler
(
	void *pTxn,
	EcrioTXNInputParamStruct *pTxnInputParam,
	BoolEnum bDeleteFlag
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	u_int32	socketError = KPALErrorNone;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;
	EcrioTXNInitParamStruct *pTxnInitParamL = NULL;
	void *pLogHandle = NULL;
	EcrioTxnMgrTransportStruct *pTxnTransportStructL = NULL;
	u_int32	uChannelIndex = ECRIO_TXN_MAX_NO_OF_CHANNELS;

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

	TXNMGRLOGI(	pLogHandle, \
		KLogTypeFuncEntry, \
		"%s:%u\tpTxnData = %x pTxnInputParam =%x bDeleteFlag =%d", \
		__FUNCTION__, __LINE__, pTxnData,
		pTxnInputParam,
		bDeleteFlag);

	pTxnTransportStructL = pTxnInitParamL->pTxnTransportStruct;

	if (NULL == pTxnTransportStructL)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	if (!pTxnInputParam)
	{
		TXNMGRLOGE(    pLogHandle,
			KLogTypeGeneral,
			"%s:%u\t-> pTxnInputParam is NULL", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	if ((!pTxnInputParam->pMsg) || (!pTxnInputParam->msgLen))
	{
		TXNMGRLOGE(    pLogHandle,
			KLogTypeGeneral,
			"%s:%u\t-> pMsg is NULL or msgLen is 0 ", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	if ((pTxnInputParam->messageType != EcrioTXNMessageTypeResponse) ||
		!((pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode >= 100) &&
		(pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode < 700)))
	{
		TXNMGRLOGE(    pLogHandle,
			KLogTypeGeneral,
			"%s:%u\t-> not a response or invalid response code", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	pTxnNode = _EcrioTXNFindMatchingTxn(pTxnData,
		&(pTxnInputParam->txnMatchingParam),
		pTxnInputParam->txnMatchingCriteria,
		&txnRetCode);

	if ((txnRetCode != EcrioTXNNoError) && (txnRetCode != EcrioTXNNoMatchingTxnFoundError))
	{
		TXNMGRLOGE(    pLogHandle,
			KLogTypeGeneral,
			"%s:%u\tunexpected result from _EcrioTXNFindMatchingTxn()", __FUNCTION__, __LINE__);

		pTxnNode = NULL;
		goto EndTag;
	}

	if (!pTxnNode)
	{
		TXNMGRLOGE(    pLogHandle,
			KLogTypeGeneral,
			"%s:%u\tmatching txn expected but not found", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNNoMatchingTxnFoundError;
		goto EndTag;
	}
	else
	{
		uChannelIndex = pTxnNode->uChannelIndex;
		if (pTxnInputParam->txnMatchingParam.commonMatchingParam.txnIdentifier.txnType == EcrioTXNTxnTypeServerInvite)
		{
			/*	INVITE Server TXN	*/
			switch (pTxnNode->txnState)
			{
				case EcrioTXNTxnStateProceeding:
				{
					if (ECRIO_TXN_MAX_NO_OF_CHANNELS == uChannelIndex)
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

#ifdef ENABLE_QCMAPI
					socketError = _EcrioTxnMgrSendDataToCommunicationChannel(pTxnData, pTxnInputParam->pMsg, (u_int16)pTxnInputParam->msgLen, uChannelIndex, pTxnInputParam->messageType, pTxnInputParam->txnMatchingParam.commonMatchingParam.pCallId, Enum_TRUE);
#else
					socketError = _EcrioTxnMgrSendDataToCommunicationChannel(pTxnData, pTxnInputParam->pMsg, (u_int16)pTxnInputParam->msgLen, uChannelIndex);
#endif
					if (socketError != KPALErrorNone)
					{
						TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tsending to network failed", __FUNCTION__, __LINE__);

						txnRetCode = EcrioTXNTransportError;
						goto EndTag;
					}

					if (pTxnNode->pMsg)
					{
						pal_MemoryFree((void **)&pTxnNode->pMsg);
					}

					pTxnNode->msgLen = 0;

					/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
					if (pal_UtilityDataOverflowDetected(pTxnInputParam->msgLen, sizeof(u_char)) == Enum_TRUE)
					{
						txnRetCode = EcrioTXNInsufficientMemoryError;
						goto EndTag;
					}

					pal_MemoryAllocate(1 + (pTxnInputParam->msgLen) * sizeof(u_char), (void **)&(pTxnNode->pMsg));
					if (!pTxnNode->pMsg)
					{
						txnRetCode = EcrioTXNInsufficientMemoryError;
						goto EndTag;
					}

					pal_MemoryCopy(
						pTxnNode->pMsg,
						pTxnInputParam->msgLen,
						pTxnInputParam->pMsg,
						pTxnInputParam->msgLen);

					pTxnNode->pMsg[pTxnInputParam->msgLen] = '\0';

					pTxnNode->msgLen = pTxnInputParam->msgLen;

					if (bDeleteFlag == Enum_TRUE)
					{
						TXNMGRLOGI(    pLogHandle,
							KLogTypeGeneral,
							"%s:%u\tbDeleteFlag is TRUE, hence txn node will be deleted", __FUNCTION__, __LINE__);

						txnRetCode = EcrioTXNNoError;
						goto EndTag;
					}

					if (pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode >= 200 &&
						pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode < 300)
					{
						TXNMGRLOGI(    pLogHandle,
							KLogTypeGeneral,
							"%s:%u\ttxnState is transiting from %s to %s of the transaction %s",
							__FUNCTION__, __LINE__,
							_EcrioTXNGetTxnStateName(pTxnNode->txnState),
							_EcrioTXNGetTxnStateName(EcrioTXNTxnStateTerminated),
							_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

						/*	txn state change	*/
						pTxnNode->txnState = EcrioTXNTxnStateTerminated;

						txnRetCode = EcrioTXNNoError;
						goto EndTag;
					}
					else if ((pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode >= 300) &&
						(pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode < 700))
					{
						if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeUDP)
						{
							/*	Start Timer G & H	*/
							EcrioTXNCustomTimersStruct *pCustomSIPTimers = &(pTxnData->pTxnInitParam->customSIPTimers);
							if (pCustomSIPTimers->TimerG >= pCustomSIPTimers->TimerH)
							{
								/* If Timer G is greater or equal to timer H for INVITE request then only Timer H will be fired. */
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
							/*	Start Timer H	*/
							txnRetCode = _EcrioTXNStartTxnTimer(pTxnData,
								pTxnNode,
								EcrioTXNTimeoutTimer);
						}

						if (txnRetCode != EcrioTXNNoError)
						{
							TXNMGRLOGE(    pLogHandle,
								KLogTypeGeneral,
								"%s:%u\ttimer start failed", __FUNCTION__, __LINE__);

							goto EndTag;
						}

						TXNMGRLOGI(    pLogHandle,
							KLogTypeGeneral,
							"%s:%u\ttxnState is transiting from %s to %s of the transaction %s",
							__FUNCTION__, __LINE__,
							_EcrioTXNGetTxnStateName(pTxnNode->txnState),
							_EcrioTXNGetTxnStateName(EcrioTXNTxnStateCompleted),
							_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));
						/*	txn state change	*/
						pTxnNode->txnState = EcrioTXNTxnStateCompleted;
					}

					txnRetCode = _EcrioTXNUpdateBasicTxnMatchingParam(pTxnData,
						pTxnNode,
						pTxnInputParam);
					if (txnRetCode != EcrioTXNNoError)
					{
						TXNMGRLOGE(    pLogHandle,
							KLogTypeGeneral,
							"%s:%u\tupdate of basic txn matching param failed", __FUNCTION__, __LINE__);

						pTxnNode = NULL;
						goto EndTag;
					}

					break;
				}

				default:
				{
					TXNMGRLOGE(    pLogHandle,
						KLogTypeGeneral,
						"%s:%u\tresponse received in wrong txn state", __FUNCTION__, __LINE__);

					txnRetCode = EcrioTXNMsgReceivedInWrongStateError;

					break;
				}
			}
		}
		else
		{
			/*	NON-INVITE Server TXN	*/
			switch (pTxnNode->txnState)
			{
				case EcrioTXNTxnStateTrying:
				{
#if 1
					if (ECRIO_TXN_MAX_NO_OF_CHANNELS == uChannelIndex)
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
#endif

#ifdef ENABLE_QCMAPI
					socketError = _EcrioTxnMgrSendDataToCommunicationChannel(pTxnData, pTxnInputParam->pMsg, pTxnInputParam->msgLen, uChannelIndex, pTxnInputParam->messageType, pTxnInputParam->txnMatchingParam.commonMatchingParam.pCallId, Enum_TRUE);
#else
					socketError = _EcrioTxnMgrSendDataToCommunicationChannel(pTxnData, pTxnInputParam->pMsg, pTxnInputParam->msgLen, uChannelIndex);
#endif
					if (socketError != KPALErrorNone)
					{
						TXNMGRLOGE(    pLogHandle,
							KLogTypeGeneral,
							"%s:%u\tsending to network failed", __FUNCTION__, __LINE__);

						txnRetCode = EcrioTXNTransportError;
						goto EndTag;
					}

					if (bDeleteFlag == Enum_TRUE)
					{
						TXNMGRLOGI(    pLogHandle,
							KLogTypeGeneral,
							"%s:%u\tbDeleteFlag is TRUE, hence txn node will be deleted", __FUNCTION__, __LINE__);

						txnRetCode = EcrioTXNNoError;
						goto EndTag;
					}

					if (pTxnNode->pMsg)
					{
						pal_MemoryFree(
							(void **)&(pTxnNode->pMsg));

						pTxnNode->pMsg = NULL;
						pTxnNode->msgLen = 0;
					}

					/* Check arithmetic overflow */
					if (pal_UtilityArithmeticOverflowDetected(pTxnInputParam->msgLen, 1) == Enum_TRUE)
					{
						txnRetCode = EcrioTXNInsufficientMemoryError;
						goto EndTag;
					}

					/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
					if (pal_UtilityDataOverflowDetected((pTxnInputParam->msgLen+1), sizeof(u_char)) == Enum_TRUE)
					{
						txnRetCode = EcrioTXNInsufficientMemoryError;
						goto EndTag;
					}

					pal_MemoryAllocate((pTxnInputParam->msgLen+1) * sizeof(u_char),
						(void **)&(pTxnNode->pMsg));

					if (!pTxnNode->pMsg)
					{
						TXNMGRLOGE(    pLogHandle,
							KLogTypeGeneral,
							"%s:%u\tbmemory allocation failed ", __FUNCTION__, __LINE__);

						txnRetCode = EcrioTXNInsufficientMemoryError;
						goto EndTag;
					}

					pal_MemoryCopy(
						pTxnNode->pMsg,
						pTxnInputParam->msgLen,
						pTxnInputParam->pMsg,
						pTxnInputParam->msgLen);
					pTxnNode->pMsg[pTxnInputParam->msgLen] = '\0';
					pTxnNode->msgLen = pTxnInputParam->msgLen;

					pTxnNode->retransmissionContext = (s_int32)(pTxnInputParam->retransmitContext);

					txnRetCode = _EcrioTXNUpdateBasicTxnMatchingParam(pTxnData,
						pTxnNode,
						pTxnInputParam);
					if (txnRetCode != EcrioTXNNoError)
					{
						TXNMGRLOGE(    pLogHandle,
							KLogTypeGeneral,
							"%s:%u\tupdate of basic txn matching param failed", __FUNCTION__, __LINE__);

						pTxnNode = NULL;
						goto EndTag;
					}

					txnRetCode = _EcrioTXNUpdateSessionTxnMappingParam(pTxnData,
						pTxnNode,
						pTxnInputParam);
					if (txnRetCode != EcrioTXNNoError)
					{
						TXNMGRLOGE(    pLogHandle,
							KLogTypeGeneral,
							"%s:%u\tupdate of session txn mapping param failed", __FUNCTION__, __LINE__);

						pTxnNode = NULL;
						goto EndTag;
					}

					if (pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode >= 100 &&
						pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode < 200)
					{
						TXNMGRLOGI(    pLogHandle,
							KLogTypeGeneral,
							"%s:%u\ttxnState is transiting from %s to %s of the transaction %s",
							__FUNCTION__, __LINE__,
							_EcrioTXNGetTxnStateName(pTxnNode->txnState),
							_EcrioTXNGetTxnStateName(EcrioTXNTxnStateProceeding),
							_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

						/*	txn state change	*/
						pTxnNode->txnState = EcrioTXNTxnStateProceeding;
					}
					else
					{
						// if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeUDP)
						{
							TXNMGRLOGI(    pLogHandle,
								KLogTypeGeneral,
								"%s:%u\ttxnState is transiting from %s to %s of the transaction %s",
								__FUNCTION__, __LINE__,
								_EcrioTXNGetTxnStateName(pTxnNode->txnState),
								_EcrioTXNGetTxnStateName(EcrioTXNTxnStateCompleted),
								_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

							/*	txn state change	*/
							pTxnNode->txnState = EcrioTXNTxnStateCompleted;
							if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeUDP)
							{
								/*	Start Timer J	*/
								txnRetCode = _EcrioTXNStartTxnTimer(pTxnData,
									pTxnNode,
									EcrioTXNWaitTimer);
								if (txnRetCode != EcrioTXNNoError)
								{
									TXNMGRLOGE(pLogHandle,
										KLogTypeGeneral,
										"%s:%u\ttimer start failed", __FUNCTION__, __LINE__);

									goto EndTag;
								}
							}
						}
					}
					break;
				}

				case EcrioTXNTxnStateProceeding:
				{
					if (ECRIO_TXN_MAX_NO_OF_CHANNELS == uChannelIndex)
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

#ifdef ENABLE_QCMAPI
					socketError = _EcrioTxnMgrSendDataToCommunicationChannel(pTxnData, pTxnInputParam->pMsg, pTxnInputParam->msgLen, uChannelIndex, pTxnInputParam->messageType, pTxnInputParam->txnMatchingParam.commonMatchingParam.pCallId, Enum_TRUE);
#else
					socketError = _EcrioTxnMgrSendDataToCommunicationChannel(pTxnData, pTxnInputParam->pMsg, pTxnInputParam->msgLen, uChannelIndex);
#endif
					if (socketError != KPALErrorNone)
					{
						TXNMGRLOGE(    pLogHandle,
							KLogTypeGeneral,
							"%s:%u\tsending to network failed ", __FUNCTION__, __LINE__);

						txnRetCode = EcrioTXNTransportError;
						goto EndTag;
					}

					if (bDeleteFlag == Enum_TRUE)
					{
						TXNMGRLOGI(    pLogHandle,
							KLogTypeGeneral,
							"%s:%u\tbDeleteFlag is TRUE, hence txn node will be deleted", __FUNCTION__, __LINE__);

						txnRetCode = EcrioTXNNoError;
						goto EndTag;
					}

					if (pTxnNode->pMsg)
					{
						pal_MemoryFree(
							(void **)&(pTxnNode->pMsg));

						pTxnNode->pMsg = NULL;
						pTxnNode->msgLen = 0;
					}

					/* Check arithmetic overflow */
					if (pal_UtilityArithmeticOverflowDetected(pTxnInputParam->msgLen, 1) == Enum_TRUE)
					{
						txnRetCode = EcrioTXNInsufficientMemoryError;
						goto EndTag;
					}

					/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
					if (pal_UtilityDataOverflowDetected((pTxnInputParam->msgLen + 1), sizeof(u_char)) == Enum_TRUE)
					{
						txnRetCode = EcrioTXNInsufficientMemoryError;
						goto EndTag;
					}

					pal_MemoryAllocate((pTxnInputParam->msgLen+1) * sizeof(u_char), (void **)&(pTxnNode->pMsg));

					if (!pTxnNode->pMsg)
					{
						TXNMGRLOGE(    pLogHandle,
							KLogTypeGeneral,
							"%s:%u\tmemory allocation failed", __FUNCTION__, __LINE__);

						txnRetCode = EcrioTXNInsufficientMemoryError;
						goto EndTag;
					}

					pal_MemoryCopy(
						pTxnNode->pMsg,
						pTxnInputParam->msgLen,
						pTxnInputParam->pMsg,
						pTxnInputParam->msgLen);

					pTxnNode->pMsg[pTxnInputParam->msgLen] = '\0';
					pTxnNode->msgLen = pTxnInputParam->msgLen;

					pTxnNode->retransmissionContext = (s_int32)(pTxnInputParam->retransmitContext);

					if ((pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode >= 200) &&
						(pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode < 700))
					{
						// if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeUDP)
						{
							if (pTxnInputParam->transportDetails.transportType == EcrioTXNTransportTypeUDP)
							{
								/*	Start Timer J	*/
								txnRetCode = _EcrioTXNStartTxnTimer(pTxnData,
									pTxnNode,
									EcrioTXNWaitTimer);
								if (txnRetCode != EcrioTXNNoError)
								{
									TXNMGRLOGE(pLogHandle,
										KLogTypeGeneral,
										"%s:%u\ttimer start failed ", __FUNCTION__, __LINE__);

									goto EndTag;
								}
							}
							TXNMGRLOGI(    pLogHandle,
								KLogTypeGeneral,
								"%s:%u\ttxnState is transiting from %s to %s of the transaction %s \r\n",
								__FUNCTION__, __LINE__,
								_EcrioTXNGetTxnStateName(pTxnNode->txnState),
								_EcrioTXNGetTxnStateName(EcrioTXNTxnStateCompleted),
								_EcrioTXNGetTxnName(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier));

							/*	txn state change	*/
							pTxnNode->txnState = EcrioTXNTxnStateCompleted;
						}
					}

					txnRetCode = _EcrioTXNUpdateBasicTxnMatchingParam(pTxnData,
						pTxnNode,
						pTxnInputParam);
					if (txnRetCode != EcrioTXNNoError)
					{
						TXNMGRLOGE(    pLogHandle,
							KLogTypeGeneral,
							"%s:%u\tupdate of basic txn matching param failed", __FUNCTION__, __LINE__);

						pTxnNode = NULL;
						goto EndTag;
					}

					txnRetCode = _EcrioTXNUpdateSessionTxnMappingParam(pTxnData,
						pTxnNode,
						pTxnInputParam);
					if (txnRetCode != EcrioTXNNoError)
					{
						TXNMGRLOGE(    pLogHandle,
							KLogTypeGeneral,
							"%s:%u\tupdate of session txn mapping param failed ", __FUNCTION__, __LINE__);

						pTxnNode = NULL;
						goto EndTag;
					}

					break;
				}

				default:
				{
					TXNMGRLOGE(    pLogHandle,
						KLogTypeGeneral,
						"%s:%u\tresponse received in wrong txn state", __FUNCTION__, __LINE__);

					txnRetCode = EcrioTXNMsgReceivedInWrongStateError;

					break;
				}
			}
		}
	}

	pTxnNode = NULL;

EndTag:

	if (pTxnNode)
	{
		_EcrioTXNDeleteTxn(	pTxnData,
			pTxnNode);
	}

	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\tError=%u", __FUNCTION__, __LINE__, txnRetCode);

	return txnRetCode;
}

/**************************************************************************************************
Function:            _EcrioTXNCreateTxn()

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
_EcrioTXNTxnNodeStruct *_EcrioTXNCreateTxn
(
	_EcrioTXNDataStruct	*pTxnData,
	EcrioTXNInputParamStruct *pTxnInputParam,
	u_int32	uChannelIndex,
	EcrioTXNReturnCodeEnum *pTxnRetCode
)
{
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;

	if (!pTxnData)
	{
		*pTxnRetCode = EcrioTXNInsufficientDataError;
		return NULL;
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, \
		KLogTypeFuncEntry, \
		"%s:%u\tpTxnData = %x pTxnInputParam =%x uChannelIndex = %u pTxnRetCode =%x", \
		__FUNCTION__, __LINE__,
		pTxnData,
		pTxnInputParam,
		uChannelIndex,
		pTxnRetCode);

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, \
		KLogTypeGeneral, \
		"%s:%u\tbefore new node craetion head = %x tail = %x", \
		__FUNCTION__, __LINE__,
		pTxnData->pTxnListHead,
		pTxnData->pTxnListTail);

	pTxnNode = _EcrioTXNCreateTxnNode(	pTxnData,
		pTxnInputParam,
		uChannelIndex,
		pTxnRetCode);

	if (!pTxnNode)
	{
		goto EndTag;
	}

	if (!pTxnData->pTxnListHead)
	{
		pTxnData->pTxnListHead = pTxnNode;
		pTxnData->pTxnListTail = pTxnNode;
	}
	else
	{
		pTxnNode->pPrev = pTxnData->pTxnListTail;
		pTxnData->pTxnListTail->pNext = pTxnNode;
		pTxnData->pTxnListTail = pTxnNode;
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, \
		KLogTypeGeneral, \
		"%s:%u\tafter new node = %x craetion head = %x tail = %x", \
		__FUNCTION__, __LINE__, pTxnNode,
		pTxnData->pTxnListHead,
		pTxnData->pTxnListTail);

	*pTxnRetCode = EcrioTXNNoError;

EndTag:

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncExit, "%s:%u\tError=%x", __FUNCTION__, __LINE__, pTxnNode);

	return pTxnNode;
}

/**************************************************************************************************
Function:            _EcrioTXNDeleteTxn()

Purpose:             This API deletes node for a tranction.

Description:.        This internal function deletes a node from transaction linklist.

Input:				 _EcrioTXNDataStruct		*pTxnData
                     _EcrioTXNTxnNodeStruct		*pTxnNode

OutPut:              None.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum _EcrioTXNDeleteTxn
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;

	void *pLogHandle = NULL;

	if (!pTxnData)
	{
		return EcrioTXNInsufficientDataError;
	}

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

	TXNMGRLOGI(	pLogHandle, \
		KLogTypeFuncEntry, \
		"%s:%u\tpTxnData = %x pTxnNode =%x", \
		__FUNCTION__, __LINE__,
		pTxnData,
		pTxnNode);

	if (!pTxnNode)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	TXNMGRLOGI(	pLogHandle, \
		KLogTypeGeneral, \
		"%s:%u\tbefore deletion of node = %x head = %x tail = %x", \
		__FUNCTION__, __LINE__, pTxnNode,
		pTxnData->pTxnListHead,
		pTxnData->pTxnListTail);

	if (pTxnNode->pPrev)
	{
		pTxnNode->pPrev->pNext = pTxnNode->pNext;
	}
	else
	{
		pTxnData->pTxnListHead = pTxnNode->pNext;
	}

	if (pTxnNode->pNext)
	{
		pTxnNode->pNext->pPrev = pTxnNode->pPrev;
	}
	else
	{
		pTxnData->pTxnListTail = pTxnNode->pPrev;
	}

	txnRetCode = _EcrioTXNDeleteTxnNode(pTxnData,
		pTxnNode);

	TXNMGRLOGI(	pLogHandle,	KLogTypeGeneral, "%s:%u\tafter deletion of node, head = %x tail = %x", \
		__FUNCTION__, __LINE__, pTxnData->pTxnListHead, pTxnData->pTxnListTail);

EndTag:

	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\tError=%x", __FUNCTION__, __LINE__, txnRetCode);

	return txnRetCode;
}

/**************************************************************************************************
Function:            _EcrioTXNStartTxnTimer()

Purpose:             This API starts timer for a tranction.

Description:.        This internal function starts timer for a transaction based on input parameter
                     timerType. Timer types are retrannsmit, timeout or wait.

Input:				 _EcrioTXNDataStruct		*pTxnData
                     _EcrioTXNTxnNodeStruct		*pTxnNode
                     EcrioTXNTimerTypeEnum		timerType

OutPut:              None.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum _EcrioTXNStartTxnTimer
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode,
	EcrioTXNTimerTypeEnum timerType
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	EcrioTXNSipTimerTypeEnum sipTimerType = EcrioTXNSipTimerTypeNone;
	EcrioTXNSipTimerTypeEnum sipTimerTypeIndex = EcrioTXNSipTimerTypeNone;
	EcrioTXNSipTimerTypeEnum sipTimerTypeMax = EcrioTXNSipTimerTypeNone;

	void *pLogHandle = NULL;

	if (!pTxnData)
	{
		return EcrioTXNInsufficientDataError;
	}

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

	TXNMGRLOGI(	pLogHandle, \
		KLogTypeFuncEntry, \
		"%s:%u\tpTxnData = %x pTxnNode =%x timerType =%u", \
		__FUNCTION__, __LINE__,
		pTxnData,
		pTxnNode,
		timerType);

	if (!pTxnNode)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	txnRetCode = _EcrioTXNGetSipTimerType(pTxnData, pTxnNode, timerType, Enum_TRUE, &sipTimerType);

	if (txnRetCode != EcrioTXNNoError)
	{
		goto EndTag;
	}

	if (sipTimerType == EcrioTXNSipTimerTypeNone)
	{
		TXNMGRLOGI(pLogHandle,
			KLogTypeGeneral,
			"%s:%u\tNo Timer to run, Skiping Timer Start", __FUNCTION__, __LINE__);
		goto EndTag;
	}

	sipTimerTypeMax = EcrioTXNSipTimerTypeK;

	/** Commented to start timeout timer without retransmit timer irrespective of transport ----SH*/

	for (sipTimerTypeIndex = sipTimerTypeMax; sipTimerTypeIndex > EcrioTXNSipTimerTypeNone;
		sipTimerTypeIndex = (EcrioTXNSipTimerTypeEnum)ECRIO_TXN_DecrementSipTimer(sipTimerTypeIndex))	/**Lint error resolved*/
	{
		if (sipTimerType & sipTimerTypeIndex)
		{
			txnRetCode = _EcrioTXNStartTxnTimerInternal(pTxnData,
				pTxnNode,
				sipTimerTypeIndex);

			if (txnRetCode != EcrioTXNNoError)
			{
				goto EndTag;
			}
		}
	}

EndTag:

	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\tError=%x", __FUNCTION__, __LINE__, txnRetCode);

	return txnRetCode;
}

/**************************************************************************************************
Function:            _EcrioTXNStopTxnTimer()

Purpose:             This API stops timer for a tranction.

Description:.        This internal function stops timer for a transaction based on input parameter
                     timerType. Timer types are retrannsmit, timeout or wait.

Input:				 _EcrioTXNDataStruct		*pTxnData
                     _EcrioTXNTxnNodeStruct		*pTxnNode
                     EcrioTXNTimerTypeEnum		timerType

OutPut:              None.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum _EcrioTXNStopTxnTimer
(
	_EcrioTXNDataStruct	*pTxnData,
	_EcrioTXNTxnNodeStruct *pTxnNode,
	EcrioTXNTimerTypeEnum timerType
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	EcrioTXNSipTimerTypeEnum sipTimerType = EcrioTXNSipTimerTypeNone;
	EcrioTXNSipTimerTypeEnum sipTimerTypeIndex = EcrioTXNSipTimerTypeNone;

	void *pLogHandle = NULL;

	if (!pTxnData)
	{
		return EcrioTXNInsufficientDataError;
	}

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

	TXNMGRLOGI(	pLogHandle, \
		KLogTypeFuncEntry, \
		"%s:%u\tpTxnData = %x pTxnNode =%x timerType =%u", \
		__FUNCTION__, __LINE__,
		pTxnData,
		pTxnNode,
		timerType);

	if (!pTxnNode)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	txnRetCode = _EcrioTXNGetSipTimerType(pTxnData, pTxnNode, timerType, Enum_TRUE, &sipTimerType);

	if (txnRetCode != EcrioTXNNoError)
	{
		goto EndTag;
	}

	if (sipTimerType == EcrioTXNSipTimerTypeNone)
	{
		goto EndTag;
	}

	TXNMGRLOGI(pLogHandle,
		KLogTypeGeneral,
		"%s:%u\tStopping %s", __FUNCTION__, __LINE__,
		_EcrioTXNGetSipTimerName(pTxnData, sipTimerType));

	for (sipTimerTypeIndex = EcrioTXNSipTimerTypeA; sipTimerTypeIndex <= EcrioTXNSipTimerTypeK;
		sipTimerTypeIndex = (EcrioTXNSipTimerTypeEnum)ECRIO_TXN_IncrementSipTimer(sipTimerTypeIndex))/*Lint error resolved*/
	{
		if (sipTimerType & sipTimerTypeIndex)
		{
			txnRetCode = _EcrioTXNStopTxnTimerInternal(pTxnData,
				pTxnNode,
				sipTimerTypeIndex);
		}
	}

EndTag:

	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\tError=%x", __FUNCTION__, __LINE__, txnRetCode);

	return txnRetCode;
}

/*	This function will set the received on channel index in trnsaction node.	*/
/**************************************************************************************************
Function:            EcrioTXNSetChannelIndexInTXNNode()

Purpose:             This API sets the channel index to a transaction node.

Description:.        This internal function finds the proper transaction node from link list
                     base on pTxnInputParam and set the channel index value as per uChannelIndex
                     to the node if match found.

Input:				 void						*pTxn
                     EcrioTXNInputParamStruct	*pTxnInputParam
                     u_int32					 uChannelIndex

OutPut:              None.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum EcrioTXNSetChannelIndexInTXNNode
(
	void *pTxn,
	EcrioTXNInputParamStruct *pTxnInputParam,
	u_int32 uChannelIndex
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;
	EcrioTXNInitParamStruct *pTxnInitParamL = NULL;
	EcrioTxnMgrTransportStruct *pTxnTransportStructL = NULL;
	BoolEnum bReceivedOnProperIPsec = Enum_FALSE;
	void *pLogHandle = NULL;

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

	TXNMGRLOGI(	pLogHandle, \
		KLogTypeFuncEntry, \
		"%s:%u\tpTxnData = %x pTxnInputParam =%x uChannelIndex =%u", \
		__FUNCTION__, __LINE__, pTxnData,
		pTxnInputParam,
		uChannelIndex);

	pTxnTransportStructL = pTxnInitParamL->pTxnTransportStruct;

	if (NULL == pTxnTransportStructL)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	if (!pTxnInputParam)
	{
		TXNMGRLOGE(    pLogHandle,
			KLogTypeGeneral,
			"%s:%u\tpTxnInputParam is NULL", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	pTxnNode = _EcrioTXNFindMatchingTxn(pTxnData,
		&(pTxnInputParam->txnMatchingParam),
		pTxnInputParam->txnMatchingCriteria,
		&txnRetCode);

	if ((txnRetCode != EcrioTXNNoError) && (txnRetCode != EcrioTXNNoMatchingTxnFoundError))
	{
		TXNMGRLOGE(    pLogHandle,
			KLogTypeGeneral,
			"%s:%u\tunexpected result from _EcrioTXNFindMatchingTxn", __FUNCTION__, __LINE__);

		pTxnNode = NULL;
		goto EndTag;
	}

	if (!pTxnNode)
	{
		TXNMGRLOGE(    pLogHandle,
			KLogTypeGeneral,
			"%s:%u\tmatching txn expected but not found", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNNoMatchingTxnFoundError;
		goto EndTag;
	}
	else
	{
		if ((pTxnTransportStructL->bIPsecEnabled == Enum_TRUE) && (pTxnTransportStructL->pCommunicationStruct != NULL))
		{
			/*	If IPsec is enabled, check that the incoming response is received on proper SA.	*/
			bReceivedOnProperIPsec = _EcrioTxnMgrCheckResponseReceivedOnProperIPsec(pTxnData, pTxnInputParam->uRemoteSendingPort,
				uChannelIndex, pTxnNode->uChannelIndex, pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier.pMethodName, 
				pTxnInputParam->txnMatchingParam.commonMatchingParam.responseCode);
			if(bReceivedOnProperIPsec != Enum_TRUE)
			{
				TXNMGRLOGE (pLogHandle, KLogTypeGeneral, "%s:%u\tThe response is not received on proper IPsec",__FUNCTION__, __LINE__);

				txnRetCode = EcrioTXNMessageNotProperlyProtected;
				goto EndTag;
			}

			/*	If IPsec is enabled, convert the channel index and store it in the transaction node .	*/
			pTxnNode->uChannelIndex = _EcrioTxnMgrModifyReceivedOnChannelIndex(pTxnData,pTxnInputParam->uRemoteSendingPort,uChannelIndex);
			if(pTxnNode->uChannelIndex == 0)
			{
				TXNMGRLOGE (pLogHandle, KLogTypeGeneral, "%s:%u\tNot able to find out the corresponding protected sending channel for \
					this received on channel",__FUNCTION__, __LINE__);

				txnRetCode = EcrioTXNMessageNotProperlyProtected;
				goto EndTag;
			}
		}
		else
		{
			pTxnNode->uChannelIndex = uChannelIndex;
		}
	}

	pTxnNode = NULL;

EndTag:

	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\tError=%x", __FUNCTION__, __LINE__, txnRetCode);

	return txnRetCode;
}


void  EcrioTXNFindMatchingTxnSMSRetryData
(
	void *pTxn,
	EcrioTXNTxnMatchingParamStruct *pTxnMatchingParam,
	EcrioTXNTxnMatchingCriteriaEnum	txnMatchingCriteria,
	EcrioTXNReturnCodeEnum *pTxnRetCode,
	u_char **			pSMSRetrydata,
	u_int32*				SmsRetryCount
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNTxnNodeStruct *pTxnNode = NULL;
	EcrioTXNTxnMatchingCommonParamStruct *pCommonMatchingParam = NULL;
	BoolEnum bUseTxnMappingParamACK = Enum_FALSE;
	_EcrioTXNDataStruct	*pTxnData = NULL;
	void *pLogHandle = NULL;

	if (!pTxn)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	pTxnData = (_EcrioTXNDataStruct	*)pTxn;


	if (!pTxnData)
	{
		*pTxnRetCode = EcrioTXNInsufficientDataError;
		return;
	}

	pLogHandle = pTxnData->pTxnInitParam->pLogHandle;

#if 0
 	TXNMGRLOGI(pLogHandle, KLogTypeFuncEntry, "%s:%u\tpTxnData=%p\tpTxnMatchingParam=%p\ttxnMatchingCriteria=%u\tpTxnRetCode=%p", 
		__FUNCTION__, __LINE__, pTxnData, pTxnMatchingParam, txnMatchingCriteria, pTxnRetCode);
#endif

	if (!pTxnMatchingParam)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}
	if(pSMSRetrydata == NULL)
	{
		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	*pSMSRetrydata = NULL;
	*SmsRetryCount = 0;

	pCommonMatchingParam = &(pTxnMatchingParam->commonMatchingParam);

	if (!(pCommonMatchingParam->txnIdentifier.pMethodName))
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tpMethodName is NULL", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	if (((pCommonMatchingParam->txnIdentifier.txnType == EcrioTXNTxnTypeClientInvite) ||
		(pCommonMatchingParam->txnIdentifier.txnType == EcrioTXNTxnTypeServerInvite)) &&
		!((pal_StringCompare(pCommonMatchingParam->txnIdentifier.pMethodName,
		ECRIO_TXN_METHOD_NAME_INVITE) == 0) ||
		(pal_StringCompare(pCommonMatchingParam->txnIdentifier.pMethodName,
		ECRIO_TXN_METHOD_NAME_ACK) == 0)))
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tWrong method name passed for INVITE txn", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	if (!((pCommonMatchingParam->txnIdentifier.txnType == EcrioTXNTxnTypeClientInvite) ||
		(pCommonMatchingParam->txnIdentifier.txnType == EcrioTXNTxnTypeServerInvite)) &&
		((pal_StringCompare(pCommonMatchingParam->txnIdentifier.pMethodName,
		ECRIO_TXN_METHOD_NAME_INVITE) == 0) ||
		(pal_StringCompare(pCommonMatchingParam->txnIdentifier.pMethodName,
		ECRIO_TXN_METHOD_NAME_ACK) == 0)))
	{
		TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tWrong method name passed for Non-INVITE txn", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNInsufficientDataError;
		goto EndTag;
	}

	if (pal_StringCompare(pCommonMatchingParam->txnIdentifier.pMethodName,
		ECRIO_TXN_METHOD_NAME_ACK))
	{
		if (!pCommonMatchingParam->pBranchParam)
		{
			TXNMGRLOGE(pLogHandle, KLogTypeGeneral, "%s:%u\tpBranchParam is NULL", __FUNCTION__, __LINE__);

			txnRetCode = EcrioTXNInsufficientDataError;
			goto EndTag;
		}
	}
	pTxnNode = pTxnData->pTxnListHead;

	while (pTxnNode)
	{
		if (pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier.txnType ==
			pCommonMatchingParam->txnIdentifier.txnType)
		{
			if (!pal_StringCompare(pCommonMatchingParam->txnIdentifier.pMethodName,
				ECRIO_TXN_METHOD_NAME_ACK))
			{
				if (pTxnNode->pTxnMatchingParamACK)
				{
					bUseTxnMappingParamACK = Enum_TRUE;
				}
				else
				{
					bUseTxnMappingParamACK = Enum_FALSE;
				}
			}
			else
			{
				if (pal_StringCompare(pTxnNode->pTxnMatchingParam->commonMatchingParam.txnIdentifier.pMethodName,
					pCommonMatchingParam->txnIdentifier.pMethodName))
				{
					pTxnNode = pTxnNode->pNext;
					continue;
				}
			}

			if (bUseTxnMappingParamACK == Enum_FALSE)
			{
				txnRetCode = _EcrioTXNMatchCommonMatchingParam(	pTxnData,
					pTxnNode->pTxnMatchingParam,
					pTxnMatchingParam,
					txnMatchingCriteria);
			}
			else
			{
				txnRetCode = _EcrioTXNMatchCommonMatchingParam(	pTxnData,
					pTxnNode->pTxnMatchingParamACK,
					pTxnMatchingParam,
					txnMatchingCriteria);
			}

			if (txnRetCode != EcrioTXNNoError)
			{
				pTxnNode = pTxnNode->pNext;
				continue;
			}

			break;
		}

		pTxnNode = pTxnNode->pNext;
	}

	if (!pTxnNode)
	{
		txnRetCode = EcrioTXNNoMatchingTxnFoundError;
	}

EndTag:

// quiet	TXNMGRLOGI(pLogHandle, KLogTypeFuncExit, "%s:%u\tError=%u", __FUNCTION__, __LINE__, txnRetCode);
	*pTxnRetCode = txnRetCode;
	if(pTxnNode != NULL)
	{
		*pSMSRetrydata = (u_char*)pTxnNode->pSMSRetryData;
		*SmsRetryCount = (u_int32)pTxnNode->SmsRetryCount;
	}
	

}
