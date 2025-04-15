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

#include "EcrioSigMgrCommon.h"
#include "EcrioSigMgrInit.h"

#include "EcrioSUEInternalFunctions.h"
#include "EcrioSUESigMgrCallBacks.h"
#include "EcrioSUESigMgr.h"

/******************************************************************************

                Internal Function _EcrioSUEStop

******************************************************************************/
u_int32 _EcrioSUEStop
(
	_EcrioSUEGlobalDataStruct *pSUEGlobalData
)
{
	u_int32	uError = ECRIO_SUE_NO_ERROR;
	u_int32	uSigMgrError = ECRIO_SIGMGR_NO_ERROR;

	// BoolEnum bUseStoredSIPParam = Enum_FALSE;
	_EcrioSUEInternalNetworkStateEnum eNetworkState = _ECRIO_SUE_INTERNAL_NETWORK_STATE_ENUM_None;
	_EcrioSUEInternalRegistrationStateEnum eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_None;

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	eNetworkState = pSUEGlobalData->eNetworkState;
	eRegistrationState = pSUEGlobalData->eRegistrationState;

	if (eRegistrationState == _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_Registered)
	{
		if (eNetworkState == _ECRIO_SUE_INTERNAL_NETWORK_STATE_ENUM_Connected)
		{
			/*stop subscription retry timer*/
			if (pSUEGlobalData->bSubReAttemptTimerStarted == Enum_TRUE)
			{
				pal_TimerStop(pSUEGlobalData->uSubReAttemptTimerId);
				pSUEGlobalData->uSubReAttemptTimerId = NULL;
				pSUEGlobalData->bSubReAttemptTimerStarted = Enum_FALSE;

				EcrioSigMgrAbortSession(pSUEGlobalData->pSigMgrHandle, pSUEGlobalData->pSubscribeSessionId);
				//pal_MemoryFree((void **)&(pSUEGlobalData->pSubscribeSessionId));
				pSUEGlobalData->pSubscribeSessionId = NULL;
			}

#ifndef ENABLE_QCMAPI
			if (pSUEGlobalData->bUnSubscribeRegEvent == Enum_TRUE &&
				pSUEGlobalData->pSubscribeSessionId != NULL)
			{
				SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\tSending UnSubscribe request", __FUNCTION__, __LINE__);
				uError = _EcrioSUESigMgrSendSubscribe(pSUEGlobalData,
					0);
				if (uError != ECRIO_SUE_NO_ERROR)
				{
					SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
						"%s:%u\tEcrioSUESigMgrSendSubscribe() uError=%u", __FUNCTION__, __LINE__, uError);
				}
			}

			SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\tSending DE_REGSITER", __FUNCTION__, __LINE__);
			pSUEGlobalData->bNotifyApp = Enum_TRUE;	/* Notify the deployed application about graceful de-registration*/

			uSigMgrError = EcrioSUESigMgrSendRegister(pSUEGlobalData, EcrioSigMgrDeRegisterRequestType_DeReg);
			if (uSigMgrError != ECRIO_SIGMGR_NO_ERROR)
			{
				uError = ECRIO_SUE_UA_ENGINE_ERROR;

				SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\tEcrioSUESendRegister() uError=%u",
					__FUNCTION__, __LINE__, uError);
				goto EndTag;
			}
#endif

			ECRIO_SUE_LOG_REGISTRATION_STATE_CHANGE(pSUEGlobalData, (u_char *)"_EcrioSUEStop", _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_DeRegistering);

			pSUEGlobalData->eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_DeRegistering;
			pSUEGlobalData->bNotifyApp = Enum_FALSE;/* Reset its value to Enum_FALSE after sending de-register request */

			ECRIO_SUE_LOG_ALL_STATES(pSUEGlobalData, (u_char *)"_EcrioSUEStop");

			uError = ECRIO_SUE_NO_IMMEDIATE_EXIT_ERROR;
		}
	}
	else
	{
		SUELOGV(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
			"%s:%u\tDeRegistration is not sent as the current state is not compatible to send immediate De-Register",
			__FUNCTION__, __LINE__);
	}

	ECRIO_SUE_LOG_ENGINE_STATE_CHANGE(pSUEGlobalData, (u_char *)"_EcrioSUEStop", _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Deinitializing);

EndTag:
	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u\tuError=%u", __FUNCTION__, __LINE__, uError);

	return uError;
}
