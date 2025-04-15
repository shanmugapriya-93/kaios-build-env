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
Function:            EcrioTXNInit()

Purpose:             This API initializes the transaction manager.

Description:.        It initializes the transaction manager.

Input:				 void **ppTxnData
                     EcrioTXNInitParamStruct		*pTxnInitParam

OutPut:              None.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum EcrioTXNInit
(
	void **ppTxnData,
	EcrioTXNInitParamStruct	*pTxnInitParam
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;

	if (!pTxnInitParam)
	{
		return EcrioTXNInsufficientDataError;
	}

	TXNMGRLOGI(	pTxnInitParam->pLogHandle, \
		KLogTypeFuncEntry, \
		"%s:%u\tppTxnData = %x pTxnInitParam =%x", __FUNCTION__, __LINE__, \
		ppTxnData,
		pTxnInitParam);

	*ppTxnData = NULL;

	pal_MemoryAllocate(sizeof(_EcrioTXNDataStruct), (void **)&(pTxnData));
	if (!pTxnData)
	{
		TXNMGRLOGE(    pTxnInitParam->pLogHandle,
			KLogTypeGeneral,
			"%s:%u\tCann't allocate memory for pTxnData", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNInsufficientMemoryError;
		goto ErrorTag;
	}

	pal_MemorySet(pTxnData, 0, sizeof(_EcrioTXNDataStruct));

	pal_MemoryAllocate(sizeof(EcrioTXNInitParamStruct), (void **)&(pTxnData->pTxnInitParam));
	if (!pTxnData->pTxnInitParam)
	{
		TXNMGRLOGE(    pTxnInitParam->pLogHandle,
			KLogTypeGeneral,
			"%s:%u\tCann't allocate memory for pTxnInitParam", __FUNCTION__, __LINE__);

		txnRetCode = EcrioTXNInsufficientMemoryError;
		goto ErrorTag;
	}

	*(pTxnData->pTxnInitParam) = *(pTxnInitParam);

	/* Validate and store custom SIP timer values */
	txnRetCode = EcrioTXNValidateAndStoreCustomTimerValues(&(pTxnData->pTxnInitParam->customSIPTimers));
	if (txnRetCode != EcrioTXNNoError)
	{
		TXNMGRLOGE(    pTxnInitParam->pLogHandle,
			KLogTypeGeneral,
			"%s:%u\tInvalid custom timer values", __FUNCTION__, __LINE__);
		goto ErrorTag;
	}

	*ppTxnData = (void *)pTxnData;
	goto ErrorNone;

ErrorTag:

	if (pTxnData)
	{
		if (pTxnData->pTxnInitParam)
		{
			pal_MemoryFree(
				(void **)(&pTxnData->pTxnInitParam));
		}

		pal_MemoryFree(
			(void **)(&pTxnData));
	}

ErrorNone:

	TXNMGRLOGI(pTxnInitParam->pLogHandle, KLogTypeFuncExit, "%s:%u\tError=%u", __FUNCTION__, __LINE__, txnRetCode);

	return txnRetCode;
}

/**************************************************************************************************
Function:            EcrioTXNChangeCustomTimerValues()

Purpose:             This API initialize change the SIP timer vaules which were set at the time of
                     initialization.

Description:.        Upper Layer can change the SIP timer values Like Timer-A, Timer-B etc using this API
                     after initialization.

Input:				 void *pTxn
                     EcrioTXNCustomTimersStruct *pCustomSIPTimers

OutPut:              None.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum EcrioTXNChangeCustomTimerValues
(
	void *pTxn,
	EcrioTXNCustomTimersStruct *pCustomSIPTimers
)
{
	EcrioTXNReturnCodeEnum txnRetCode = EcrioTXNNoError;
	_EcrioTXNDataStruct	*pTxnData = NULL;

	if (!pTxn)
	{
		return EcrioTXNInsufficientDataError;
	}

	pTxnData = (_EcrioTXNDataStruct	*)pTxn;

	if (pTxnData->pTxnInitParam == NULL)
	{
		return EcrioTXNInsufficientDataError;
	}

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle,
		KLogTypeFuncEntry,
		"%s:%u\tppTxnData = %x, pCustomSIPTimers = %x", __FUNCTION__, __LINE__,
		pTxnData, pCustomSIPTimers);

	if (pCustomSIPTimers == NULL)
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\tNull value passed", __FUNCTION__, __LINE__);
		return EcrioTXNInsufficientDataError;
	}

	pTxnData->pTxnInitParam->customSIPTimers.T1 = pCustomSIPTimers->T1;
	pTxnData->pTxnInitParam->customSIPTimers.T2 = pCustomSIPTimers->T2;
	pTxnData->pTxnInitParam->customSIPTimers.T4 = pCustomSIPTimers->T4;
	pTxnData->pTxnInitParam->customSIPTimers.TimerA = pCustomSIPTimers->TimerA;
	pTxnData->pTxnInitParam->customSIPTimers.TimerB = pCustomSIPTimers->TimerB;
	pTxnData->pTxnInitParam->customSIPTimers.TimerD = pCustomSIPTimers->TimerD;
	pTxnData->pTxnInitParam->customSIPTimers.TimerE = pCustomSIPTimers->TimerE;
	pTxnData->pTxnInitParam->customSIPTimers.TimerF = pCustomSIPTimers->TimerF;
	pTxnData->pTxnInitParam->customSIPTimers.TimerG = pCustomSIPTimers->TimerG;
	pTxnData->pTxnInitParam->customSIPTimers.TimerH = pCustomSIPTimers->TimerH;
	pTxnData->pTxnInitParam->customSIPTimers.TimerI = pCustomSIPTimers->TimerI;
	pTxnData->pTxnInitParam->customSIPTimers.TimerJ = pCustomSIPTimers->TimerJ;
	pTxnData->pTxnInitParam->customSIPTimers.TimerK = pCustomSIPTimers->TimerK;

	/* Validate and store custom SIP timer values */
	txnRetCode = EcrioTXNValidateAndStoreCustomTimerValues(&(pTxnData->pTxnInitParam->customSIPTimers));
	if (txnRetCode != EcrioTXNNoError)
	{
		TXNMGRLOGE(pTxnData->pTxnInitParam->pLogHandle, KLogTypeGeneral, "%s:%u\tInvalid custom timer values", __FUNCTION__, __LINE__);
	}

	// ErrorNone:

	TXNMGRLOGI(pTxnData->pTxnInitParam->pLogHandle, KLogTypeFuncExit, "%s:%u\tError=%u", __FUNCTION__, __LINE__, txnRetCode);

	return txnRetCode;
}

/**************************************************************************************************
Function:            EcrioTXNValidateAndStoreCustomTimerValues()

Purpose:             This API validate the input pCustomSIPTimers timer values and set the default values
                     if invalid.

Description:.        This API validate the input pCustomSIPTimers timer values and set the default values
                     if invalid.

Input:				 EcrioTXNCustomTimersStruct *pCustomSIPTimers

OutPut:              None.

Returns:             EcrioTXNReturnCodeEnum.
**************************************************************************************************/
EcrioTXNReturnCodeEnum EcrioTXNValidateAndStoreCustomTimerValues
(
	EcrioTXNCustomTimersStruct *pCustomSIPTimers
)
{
	EcrioTXNReturnCodeEnum retCode = EcrioTXNNoError;

	/* Timer Values Validation */
	if ((pCustomSIPTimers->T1 > pCustomSIPTimers->T2)
		|| (pCustomSIPTimers->TimerD < ECRIO_TXN_SIP_MIN_D_TIMER_INTERVAL)
		|| (pCustomSIPTimers->TimerA > pCustomSIPTimers->TimerB)
		|| (pCustomSIPTimers->TimerE > pCustomSIPTimers->TimerF)
		|| (pCustomSIPTimers->TimerG > pCustomSIPTimers->TimerH))
	{
		retCode = EcrioTXNTimerError;
	}

	return retCode;
}
