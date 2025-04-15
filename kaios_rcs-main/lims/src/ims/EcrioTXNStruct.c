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

#include "EcrioPAL.h"
#include "EcrioTXN.h"
#include "EcrioTXNInternal.h"

/**************************************************************************************************
Function:            EcrioTXNStructRelease()

Purpose:             This API releases the memory associated with the exposed structures.

Description:.        This API releases the memory associated with the exposed structures.

Input:				 void								*pTxn
                     void							**ppInputStruct
                     EcrioTXNStructTypeEnum			structType
                     BoolEnum						bRelease

OutPut:              None.

Returns:             void.
**************************************************************************************************/
void EcrioTXNStructRelease
(
	void *pTxn,
	void **ppInputStruct,
	EcrioTXNStructTypeEnum structType,
	BoolEnum bRelease
)
{
	_EcrioTXNDataStruct	*pTxnData = NULL;
	_EcrioTXNStructTypeEnum	internalStructType = _EcrioTXNStructType_None;

	if ((!pTxn) || (!ppInputStruct) || (!(*ppInputStruct)))
	{
		return;
	}

	pTxnData = (_EcrioTXNDataStruct	*)pTxn;

	switch (structType)
	{
		case EcrioTXNStructType_EcrioTXNInitParamStruct:
		{
			internalStructType = _EcrioTXNStructType_EcrioTXNInitParamStruct;
			break;
		}

		case EcrioTXNStructType_EcrioTXNInputParamStruct:
		{
			internalStructType = _EcrioTXNStructType_EcrioTXNInputParamStruct;
			break;
		}

		case EcrioTXNStructType_EcrioTXNOutputParamStruct:
		{
			internalStructType = _EcrioTXNStructType_EcrioTXNOutputParamStruct;
			break;
		}

		case EcrioTXNStructType_EcrioTXNTUNotificationDataStruct:
		{
			internalStructType = _EcrioTXNStructType_EcrioTXNTUNotificationDataStruct;
			break;
		}

		case EcrioTXNStructType_EcrioTXNSentMsgDetailsStruct:
		{
			internalStructType = _EcrioTXNStructType_EcrioTXNSentMsgDetailsStruct;
			break;
		}

		case EcrioTXNStructType_EcrioTXNTxnIdentificationStruct:
		{
			internalStructType = _EcrioTXNStructType_EcrioTXNTxnIdentificationStruct;
			break;
		}

		case EcrioTXNStructType_EcrioTXNTxnMatchingCommonParamStruct:
		{
			internalStructType = _EcrioTXNStructType_EcrioTXNTxnMatchingCommonParamStruct;
			break;
		}

		case EcrioTXNStructType_EcrioTXNTxnMatchingExtraParamStruct:
		{
			internalStructType = _EcrioTXNStructType_EcrioTXNTxnMatchingExtraParamStruct;
			break;
		}

		case EcrioTXNStructType_EcrioTXNTxnMatchingParamStruct:
		{
			internalStructType = _EcrioTXNStructType_EcrioTXNTxnMatchingParamStruct;
			break;
		}

		case EcrioTXNStructType_EcrioTXNSessionTxnMappingParamStruct:
		{
			internalStructType = _EcrioTXNStructType_EcrioTXNSessionTxnMappingParamStruct;
			break;
		}

		case EcrioTXNStructType_EcrioTXNTransportDetailsStruct:
		{
			internalStructType = _EcrioTXNStructType_EcrioTXNTransportDetailsStruct;
			break;
		}

		case EcrioTXNStructType_EcrioTXNOutputParamDetailsStruct:
		{
			internalStructType = _EcrioTXNStructType_EcrioTXNOutputParamDetailsStruct;
			break;
		}

		case EcrioTXNStructType_EcrioTXNTransportStruct:
		{
			internalStructType = _EcrioTXNStructType_EcrioTxnMgrTransportStruct;
			break;
		}

		case EcrioTXNStructType_EcrioTxnMgrSignalingCommunicationStruct:
		{
			internalStructType = _EcrioTXNStructType_EcrioTxnMgrSignalingCommunicationStruct;
			break;
		}

		case EcrioTXNStructType_EcrioTxnMgrSignalingCommunicationChannelStruct:
		{
			internalStructType = _EcrioTXNStructType_EcrioTxnMgrSignalingCommunicationChannelStruct;
			break;
		}

		case EcrioTXNStructType_EcrioTxnMgrSignalingCommunicationChannelInfoStruct:
		{
			internalStructType = _EcrioTXNStructType_EcrioTxnMgrSignalingCommunicationChannelInfoStruct;
			break;
		}

		default:
		{
		}
		break;
	}

	_EcrioTXNStructRelease(pTxnData,
		ppInputStruct,
		internalStructType,
		bRelease);
}

/**************************************************************************************************
Function:            _EcrioTXNStructRelease()

Purpose:             Internal function to release the memory associated with exposed structures.

Description:.        Internal function to release the memory associated with exposed structures.

Input:				 _EcrioTXNDataStruct			*pTxnData
                     void							**ppInputStruct
                     EcrioTXNStructTypeEnum			structType
                     BoolEnum						bRelease

OutPut:              None.

Returns:             void.
**************************************************************************************************/
void _EcrioTXNStructRelease
(
	_EcrioTXNDataStruct	*pTxnData,
	void **ppInputStruct,
	_EcrioTXNStructTypeEnum	structType,
	BoolEnum bRelease
)
{
	s_int32	i = 0;

	if ((!pTxnData) || (!ppInputStruct) || (!(*ppInputStruct)))
	{
		return;
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncEntry, "%s:%u\t", __FUNCTION__, __LINE__);

	switch (structType)
	{
		/* This case is not used and it is NOT logically correct also */
		case _EcrioTXNStructType_EcrioTXNDataStruct:
		{
			_EcrioTXNDataStruct	*pStruct = NULL;

			pStruct = (_EcrioTXNDataStruct *)(*ppInputStruct);

			if (pStruct->pTxnListHead)
			{
				pal_MemoryFree(
					(void **)&(pStruct->pTxnListHead));

				pStruct->pTxnListHead = NULL;
			}

			if (pStruct->pTxnListTail)
			{
				pal_MemoryFree(
					(void **)&(pStruct->pTxnListTail));

				pStruct->pTxnListTail = NULL;
			}

			break;
		}

		case _EcrioTXNStructType_EcrioTXNInitParamStruct:
		{
			EcrioTXNInitParamStruct	*pStruct = NULL;

			pStruct = (EcrioTXNInitParamStruct *)(*ppInputStruct);

			pStruct->pLogHandle = NULL;
			pStruct->timerX = 0;
			pal_MemorySet(
				&(pStruct->customSIPTimers),
				0,
				sizeof(EcrioTXNCustomTimersStruct));

			pal_MemorySet(
				&(pStruct->timerExpiryNotificationFunctions),
				0,
				sizeof(EcrioTXNTimerExpiryNotificationFunctionStruct));

			break;
		}

		case _EcrioTXNStructType_EcrioTXNInputParamStruct:
		{
			EcrioTXNInputParamStruct *pStruct = NULL;

			pStruct = (EcrioTXNInputParamStruct	*)(*ppInputStruct);

			pStruct->currentContext = 0;
			pStruct->messageType = EcrioTXNMessageTypeNone;

			if (pStruct->pMsg)
			{
				pal_MemoryFree(
					(void **)&pStruct->pMsg);
			}

			pStruct->msgLen = 0;
			pStruct->pMsg = NULL;
			pStruct->retransmitContext = 0;
			pStruct->txnMatchingCriteria = EcrioTXNTxnMatchingCriteriaDefault;

			_EcrioTXNStructRelease(pTxnData,
				(void *)&(pStruct->sessionTxnMappingParam),
				_EcrioTXNStructType_EcrioTXNSessionTxnMappingParamStruct,
				Enum_TRUE);

			_EcrioTXNStructRelease(pTxnData,
				(void *)&(pStruct->transportDetails),
				_EcrioTXNStructType_EcrioTXNTransportDetailsStruct,
				Enum_FALSE);

			_EcrioTXNStructRelease(pTxnData,
				(void *)&(pStruct->txnMatchingParam),
				_EcrioTXNStructType_EcrioTXNTxnMatchingParamStruct,
				Enum_TRUE);
			pStruct->uReceivedChannelIndex = ECRIO_TXN_MAX_NO_OF_CHANNELS;

			break;
		}

		case _EcrioTXNStructType_EcrioTXNOutputParamStruct:
		{
			EcrioTXNOutputParamStruct *pStruct = NULL;

			pStruct = (EcrioTXNOutputParamStruct *)(*ppInputStruct);

			if (pStruct->ppOutputParamaDetails)
			{
				for (i = 0; i < pStruct->noOfOutputParamDetails; i++)
				{
					_EcrioTXNStructRelease(pTxnData,
						(void **)&(pStruct->ppOutputParamaDetails[i]),
						_EcrioTXNStructType_EcrioTXNOutputParamDetailsStruct,
						Enum_TRUE);
				}

				pal_MemoryFree(
					(void **)&(pStruct->ppOutputParamaDetails));

				pStruct->ppOutputParamaDetails = NULL;
			}

			pStruct->noOfOutputParamDetails = 0;
			break;
		}

		case _EcrioTXNStructType_EcrioTXNTxnIdentificationStruct:
		{
			EcrioTXNTxnIdentificationStruct	*pStruct = NULL;

			pStruct = (EcrioTXNTxnIdentificationStruct *)(*ppInputStruct);

			pStruct->txnType = EcrioTXNTxnTypeNone;

			if (pStruct->pMethodName)
			{
				pal_MemoryFree((void **)&(pStruct->pMethodName));
			}

			break;
		}

		case _EcrioTXNStructType_EcrioTXNTxnNodeStruct:
		{
			_EcrioTXNTxnNodeStruct *pStruct = NULL;

			pStruct = (_EcrioTXNTxnNodeStruct *)(*ppInputStruct);

			TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncEntry, "%s:%u\t_EcrioTXNStructType_EcrioTXNTxnNodeStruct", __FUNCTION__, __LINE__);
			if (pStruct->pTxnMatchingParam)
			{
				_EcrioTXNStructRelease(	pTxnData,
					(void **)&(pStruct->pTxnMatchingParam),
					_EcrioTXNStructType_EcrioTXNTxnMatchingParamStruct,
					Enum_TRUE);
				pStruct->pTxnMatchingParam = NULL;
			}
			TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\t", __FUNCTION__, __LINE__);

			if (pStruct->pTxnMatchingParamACK)
			{
				_EcrioTXNStructRelease(	pTxnData,
					(void **)&(pStruct->pTxnMatchingParamACK),
					_EcrioTXNStructType_EcrioTXNTxnMatchingParamStruct,
					Enum_TRUE);
				pStruct->pTxnMatchingParamACK = NULL;
			}
			TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\t", __FUNCTION__, __LINE__);

			if (pStruct->pSessionTxnMappingParam)
			{
				_EcrioTXNStructRelease(	pTxnData,
					(void **)&(pStruct->pSessionTxnMappingParam),
					_EcrioTXNStructType_EcrioTXNSessionTxnMappingParamStruct,
					Enum_TRUE);
				pStruct->pSessionTxnMappingParam = NULL;
			}
			TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\t", __FUNCTION__, __LINE__);

			if (pStruct->pMsg)
			{
				pal_MemoryFree(
					(void **)&(pStruct->pMsg));

				pStruct->pMsg = NULL;
			}
			TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\t", __FUNCTION__, __LINE__);

			pStruct->bRetryEnabled = Enum_FALSE;
			pStruct->msgLen = 0;
			pStruct->pNext = NULL;
			pStruct->pPrev = NULL;
			pStruct->retransmissionContext = 0;

			if (pStruct->pSMSRetryData)
			{
				pal_MemoryFree(
					(void **)&(pStruct->pSMSRetryData));

				pStruct->pSMSRetryData = NULL;
			}
			pStruct->SmsRetryCount = 0;
			TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\t", __FUNCTION__, __LINE__);
			
			pStruct->transportDetails.transportType = EcrioTXNTransportTypeNone;

			for (i = 0; i < TXN_NODE_TIMER_ARRAYSIZE_MAX; i++)
			{
				pStruct->timerDetails[i].absoluteDuration = 0;
				pStruct->timerDetails[i].isRunning = Enum_FALSE;
				pStruct->timerDetails[i].timerID = 0;
				pStruct->timerDetails[i].sipTimerType = EcrioTXNSipTimerTypeNone;
			}

			pStruct->uChannelIndex = 0;

			TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncExit, "%s:%u\t_EcrioTXNStructType_EcrioTXNTxnNodeStruct", __FUNCTION__, __LINE__);
			break;
		}

		case _EcrioTXNStructType_EcrioTXNTxnMatchingParamStruct:
		{
			EcrioTXNTxnMatchingParamStruct *pStruct = NULL;

			pStruct = (EcrioTXNTxnMatchingParamStruct *)(*ppInputStruct);

			TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncEntry, "%s:%u\t_EcrioTXNStructType_EcrioTXNTxnMatchingParamStruct", __FUNCTION__, __LINE__);
			if (pStruct->commonMatchingParam.pBranchParam)
			{
				pal_MemoryFree((void **)&(pStruct->commonMatchingParam.pBranchParam));
			}
			TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\t", __FUNCTION__, __LINE__);

			if (pStruct->commonMatchingParam.pCallId)
			{
				pal_MemoryFree((void **)&(pStruct->commonMatchingParam.pCallId));
			}
			TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\t", __FUNCTION__, __LINE__);

			if (pStruct->commonMatchingParam.pFromTag)
			{
				pal_MemoryFree((void **)&(pStruct->commonMatchingParam.pFromTag));
			}
			TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\t", __FUNCTION__, __LINE__);

			if (pStruct->commonMatchingParam.pToTag)
			{
				pal_MemoryFree((void **)&(pStruct->commonMatchingParam.pToTag));
			}
			TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\t", __FUNCTION__, __LINE__);

			if (pStruct->commonMatchingParam.txnIdentifier.pMethodName)
			{
				pal_MemoryFree((void **)&(pStruct->commonMatchingParam.txnIdentifier.pMethodName));
			}

			TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncExit, "%s:%u\t_EcrioTXNStructType_EcrioTXNTxnMatchingParamStruct", __FUNCTION__, __LINE__);
			break;
		}

		case _EcrioTXNStructType_EcrioTXNSessionTxnMappingParamStruct:
		{
			EcrioTXNSessionTxnMappingParamStruct *pStruct = NULL;

			pStruct = (EcrioTXNSessionTxnMappingParamStruct	*)(*ppInputStruct);

			TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncEntry, "%s:%u\t_EcrioTXNStructType_EcrioTXNSessionTxnMappingParamStruct", __FUNCTION__, __LINE__);
			if (pStruct->pSessionTxnMappingParamString)
			{
				pal_MemoryFree((void **)&(pStruct->pSessionTxnMappingParamString));
			}

			TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncExit, "%s:%u\t_EcrioTXNStructType_EcrioTXNSessionTxnMappingParamStruct", __FUNCTION__, __LINE__);
			break;
		}

		case _EcrioTXNStructType_EcrioTXNTUNotificationDataStruct:
		{
			EcrioTXNTUNotificationDataStruct *pStruct = NULL;

			pStruct = (EcrioTXNTUNotificationDataStruct	*)(*ppInputStruct);

			if (pStruct->pRunningTimerIds)
			{
				pal_MemoryFree((void **)&(pStruct->pRunningTimerIds));
				pStruct->pRunningTimerIds = NULL;
			}

			pStruct->noOfRunningTimers = 0;

			if (pStruct->pTxnMatchingParam)
			{
				_EcrioTXNStructRelease(	pTxnData,
					(void **)&(pStruct->pTxnMatchingParam),
					_EcrioTXNStructType_EcrioTXNTxnMatchingParamStruct,
					Enum_TRUE);
				pStruct->pTxnMatchingParam = NULL;
			}

			if (pStruct->pSessionTxnMappingParam)
			{
				_EcrioTXNStructRelease(	pTxnData,
					(void **)&(pStruct->pSessionTxnMappingParam),
					_EcrioTXNStructType_EcrioTXNSessionTxnMappingParamStruct,
					Enum_TRUE);
				pStruct->pSessionTxnMappingParam = NULL;
			}

			break;
		}

		case _EcrioTXNStructType_EcrioTXNTxnMatchingCommonParamStruct:
		{
			EcrioTXNTxnMatchingCommonParamStruct *pStruct = NULL;

			pStruct = (EcrioTXNTxnMatchingCommonParamStruct	*)(*ppInputStruct);

			pStruct->responseCode = 0;

			if (pStruct->pBranchParam)
			{
				pal_MemoryFree((void **)&(pStruct->pBranchParam));
			}

			if (pStruct->pCallId)
			{
				pal_MemoryFree((void **)&(pStruct->pCallId));
			}

			pStruct->cSeq = 0;
			if (pStruct->pFromTag)
			{
				pal_MemoryFree((void **)&(pStruct->pFromTag));
			}

			if (pStruct->pToTag)
			{
				pal_MemoryFree((void **)&(pStruct->pToTag));
			}

			if (pStruct->txnIdentifier.pMethodName)
			{
				pal_MemoryFree((void **)&(pStruct->txnIdentifier.pMethodName));
			}

			pStruct->txnIdentifier.txnType = EcrioTXNTxnTypeNone;

			break;
		}

		case _EcrioTXNStructType_EcrioTXNTransportDetailsStruct:
		{
			EcrioTXNTransportDetailsStruct *pStruct = NULL;

			pStruct = (EcrioTXNTransportDetailsStruct *)(*ppInputStruct);

			pStruct->transportType = EcrioTXNTransportTypeNone;
			/* ECRIO_VCTK phase 3A changes*/
			/* MemoryLeakFix*/
			break;
		}

		case _EcrioTXNStructType_EcrioTXNOutputParamDetailsStruct:
		{
			EcrioTXNOutputParamDetailsStruct *pStruct = NULL;

			pStruct = (EcrioTXNOutputParamDetailsStruct	*)(*ppInputStruct);

			if (pStruct->pTxnMatchingParam)
			{
				_EcrioTXNStructRelease(	pTxnData,
					(void **)&(pStruct->pTxnMatchingParam),
					_EcrioTXNStructType_EcrioTXNTxnMatchingParamStruct,
					Enum_TRUE);
				pStruct->pTxnMatchingParam = NULL;
			}

			if (pStruct->pSessionTxnMappingParam)
			{
				_EcrioTXNStructRelease(	pTxnData,
					(void **)&(pStruct->pSessionTxnMappingParam),
					_EcrioTXNStructType_EcrioTXNSessionTxnMappingParamStruct,
					Enum_TRUE);
				pStruct->pSessionTxnMappingParam = NULL;
			}

			break;
		}

		case _EcrioTXNStructType_EcrioTxnMgrSignalingCommunicationChannelInfoStruct:
		{
			EcrioTxnMgrSignalingCommunicationChannelInfoStruct *pTxnMgrChannelInfoStruct = NULL;

			pTxnMgrChannelInfoStruct = (EcrioTxnMgrSignalingCommunicationChannelInfoStruct *)(*ppInputStruct);
			if (pTxnMgrChannelInfoStruct->pLocalIp)
			{
				pal_MemoryFree((void **)&pTxnMgrChannelInfoStruct->pLocalIp);
			}

			if (pTxnMgrChannelInfoStruct->pRemoteIp)
			{
				pal_MemoryFree((void **)&pTxnMgrChannelInfoStruct->pRemoteIp);
			}

			pTxnMgrChannelInfoStruct->localPort = 0;
			pTxnMgrChannelInfoStruct->remotePort = 0;

			if (pTxnMgrChannelInfoStruct->pTcpBuffer)
			{
				pal_MemoryFree((void **)&pTxnMgrChannelInfoStruct->pTcpBuffer);
			}

			if (pTxnMgrChannelInfoStruct->pHttpConnectMsg)
			{
				pal_MemoryFree((void **)&pTxnMgrChannelInfoStruct->pHttpConnectMsg);
			}

			break;
		}

		case _EcrioTXNStructType_EcrioTxnMgrSignalingCommunicationChannelStruct:
		{
			EcrioTxnMgrSignalingCommunicationChannelStruct *pTxnMgrChannelStruct = NULL;

			pTxnMgrChannelStruct = (EcrioTxnMgrSignalingCommunicationChannelStruct *)(*ppInputStruct);

			if (pTxnMgrChannelStruct->pChannelInfo)
			{
				_EcrioTXNStructRelease(pTxnData,
					(void **)&(pTxnMgrChannelStruct->pChannelInfo),
					_EcrioTXNStructType_EcrioTxnMgrSignalingCommunicationChannelInfoStruct,
					Enum_TRUE);
			}

			pTxnMgrChannelStruct->channelIndex = 0;

			break;
		}

		case _EcrioTXNStructType_EcrioTxnMgrSignalingCommunicationStruct:
		{
			EcrioTxnMgrSignalingCommunicationStruct *pTxnMgrCommStruct = NULL;

			pTxnMgrCommStruct = (EcrioTxnMgrSignalingCommunicationStruct *)(*ppInputStruct);

			for (i = 0; i < (s_int32)pTxnMgrCommStruct->noOfChannels; i++)
			{
				EcrioTxnMgrSignalingCommunicationChannelStruct *pChannel = NULL;
				pChannel = &pTxnMgrCommStruct->pChannels[i];
				_EcrioTXNStructRelease(pTxnData,
					(void **)&(pChannel),
					_EcrioTXNStructType_EcrioTxnMgrSignalingCommunicationChannelStruct,
					Enum_FALSE);
			}

			pal_MemoryFree((void **)&pTxnMgrCommStruct->pChannels);
			pTxnMgrCommStruct->pChannels = NULL;
			pTxnMgrCommStruct->noOfChannels = 0;

			break;
		}

		case _EcrioTXNStructType_EcrioTxnMgrTransportStruct:
		{
			EcrioTxnMgrTransportStruct *pTxnMgrSACStruct = NULL;

			pTxnMgrSACStruct = (EcrioTxnMgrTransportStruct *)(*ppInputStruct);

			if (pTxnMgrSACStruct->pCommunicationStruct)
			{
				_EcrioTXNStructRelease(pTxnData,
					(void **)&(pTxnMgrSACStruct->pCommunicationStruct),
					_EcrioTXNStructType_EcrioTxnMgrSignalingCommunicationStruct,
					Enum_TRUE);
			}

			pTxnMgrSACStruct->sendingChannelIndex = 0;
			pTxnMgrSACStruct->sendingTCPChannelIndex = 0;
			pTxnMgrSACStruct->receivedOnChannelIndex = 0;
			break;
		}

		default:
		{
			break;
		}
	}

	if (bRelease == Enum_TRUE)
	{
		pal_MemoryFree(
			ppInputStruct);
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncExit, "%s:%u\t", __FUNCTION__, __LINE__);
}
