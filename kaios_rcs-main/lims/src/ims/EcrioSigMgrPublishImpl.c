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

#include "EcrioSigMgrCommon.h"
#include "EcrioSigMgrInit.h"
#include "EcrioSigMgrInternal.h"
#include "EcrioSigMgrUtilities.h"
#include "EcrioSigMgrCallbacks.h"
#include "EcrioSigMgrIMSLibHandler.h"
#include "EcrioSigMgrTransactionHandler.h"
#include "EcrioDSListInterface.h"

/*****************************************************************************
                Internal API Declaration Section - Begin
*****************************************************************************/

/*****************************************************************************
                Internal API Declaration Section - End
*****************************************************************************/

/*****************************************************************************
                    API Definition Section - Begin
*****************************************************************************/


/*****************************************************************************

Function:		_EcrioSigMgrStartRePublishTimer()

Purpose:		starts a reregistration timer for list of contact uris.

Description:	starts a reregistration timer for list of contact uris.

Input:			
                
                
                
                
                

OutPut:			none.

Returns:		error code.
*****************************************************************************/

u_int32 _EcrioSigMgrStartRePublishTimer
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pPubRsp,
	EcrioSigMgrPublishInfo *pPubInfo
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 expires	= 0;
	TimerStartConfigStruct timerConfig;
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	/* Stopping publish expire callback */
	if (pPubInfo->bIsPubExpireTimerRunning == Enum_TRUE)
	{
		pal_TimerStop(pPubInfo->uPubExpireTimerID);
		pPubInfo->uPubExpireTimerID = NULL;
		pPubInfo->bIsPubExpireTimerRunning = Enum_FALSE;
	}

	if (pPubRsp->pExpires != NULL)
	{
		expires = *pPubRsp->pExpires;
		SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tValue for Expires header = %d",
			__FUNCTION__, __LINE__, expires);
	}
	if (expires != 0)
	{
		timerConfig.uPeriodicInterval = 0;
		timerConfig.timerCallback = _EcrioSigMgrRePublishCallBack;
		timerConfig.pCallbackData = (void *)pPubInfo;
		timerConfig.bEnableGlobalMutex = Enum_TRUE;
		/* 80% of the given expire value */
		timerConfig.uInitialInterval = (expires * 80)/100;
		timerConfig.uInitialInterval *= 1000;
		error = pal_TimerStart(pSigMgr->pal, &timerConfig, &pPubInfo->uPubExpireTimerID);
		if (error != KPALErrorNone)
		{
			error = ECRIO_SIGMGR_TIMER_ERROR;
			goto Error_Level_01;
		}
		pPubInfo->bIsPubExpireTimerRunning = Enum_TRUE;
	}
Error_Level_01:
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		if (pPubInfo->uPubExpireTimerID != NULL)
		{
			pal_TimerStop(pPubInfo->uPubExpireTimerID);
			pPubInfo->uPubExpireTimerID = NULL;
			pPubInfo->bIsPubExpireTimerRunning = Enum_FALSE;
		}
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return error;
}



/*****************************************************************************

Function:		_EcrioSigMgrRePublishCallBack()

Purpose:		Publish timer callback function

Description:	Sends Publish request

Input:			
                

Returns:		error code.
*****************************************************************************/
void _EcrioSigMgrRePublishCallBack
(
	void *pData,
	TIMERHANDLE timerID
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;

	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrPublishInfo *pPubInfo = NULL;
	EcrioSigMgrSipMessageStruct messageReq = { .eMethodType = EcrioSipMessageTypeNone };
	EcrioSigMgrSipMessageStruct *pMessageReq = NULL;
	messageReq.eModuleId = EcrioSigMgrCallbackRegisteringModule_CPM;

	/* Stop the timer before proceeding further. */
	pal_TimerStop(timerID);

	if (pData == NULL)
	{
		return;
	}
	pPubInfo = (EcrioSigMgrPublishInfo *)pData;
	pSigMgr = pPubInfo->pSigMgr;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\ttimerID=%u, pData=%s",
		__FUNCTION__, __LINE__, timerID, pPubInfo->SipETag);

	if (pPubInfo->uPubExpireTimerID == timerID)
	{
		pPubInfo->uPubExpireTimerID = NULL;
	}
	pPubInfo->eState = ECRIO_SIG_MGR_PUBLISH_STATE_Refresh;
	error = _EcrioSigMgrSendPublish(pSigMgr, &messageReq);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
	    SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t_EcrioSigMgrSendRegister) failed with error = %u",
	        __FUNCTION__, __LINE__, error);

	    goto Error_Level_01;
	}

Error_Level_01:
	pMessageReq = &messageReq;
	EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_SipMessage, (void**)&pMessageReq, Enum_FALSE);
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}

/*****************************************************************************
                    API Definition Section - End
*****************************************************************************/
