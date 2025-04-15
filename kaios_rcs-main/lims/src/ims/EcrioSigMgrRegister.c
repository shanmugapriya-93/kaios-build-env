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
#include "EcrioSigMgrRegister.h"
#include "EcrioSigMgrRegisterInternal.h"
#include "EcrioTXN.h"

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

  Function:		EcrioSigMgrSendRegister()

  Purpose:		Starts the registration / deregistration Process for a User.

  Description:	Creates and sends a register request.

  Input:		SIGMGRHANDLE sigMgrHandle - Signaling Manager Instance.
                EcrioSigMgrRegisterReqStruct* pRegReq - Registration request
                    Headers.

  Input/OutPut:	void** ppRegisterIdentifier - Registration Identifier
                    (call-id) of the register request.

  Returns:		error code.
*****************************************************************************/
u_int32 EcrioSigMgrSendRegister
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrRegisterRequestEnum eRegReq,
	void *pRegisterIdentifier
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;

	/* check for incoming parameters*/
	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pSigMgr->pSignalingInfo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tSignaling Manager not initialized", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_NOT_INITIALIZED;
		goto Error_Level_01;
	}

	if (pRegisterIdentifier == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data, pRegisterIdentifier=%p",
			__FUNCTION__, __LINE__, pRegisterIdentifier);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	/* Pass Enum_True to the function so that header values passed from */
	/* the upper layer should be stored/updated in user reg info object */
	error = _EcrioSigMgrSendRegister(pSigMgr, eRegReq, pRegisterIdentifier);

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************

  Function:		EcrioSigMgrReleaseRegObject()

  Purpose:		API used to release registration object.

  Description:	API used to release registration object.

  Input:		SIGMGRHANDLE sigMgrHandle - Signaling Manager Instance

  OutPut:		void** pRegisterIdentifier - reg id.

  Returns:		error code.
*****************************************************************************/
u_int32 EcrioSigMgrReleaseRegObject
(
	SIGMGRHANDLE sigMgrHandle,
	void **ppRegisterIdentifier
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;

	/* check for incoming parameters*/
	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pSigMgr->pSignalingInfo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tSignaling Manager not initialized", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_NOT_INITIALIZED;
		goto Error_Level_01;
	}

	if ((ppRegisterIdentifier == NULL) || (*ppRegisterIdentifier == NULL))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	/* Clear service-route */
	if (pSigMgr->pServiceRoute)
	{
		_EcrioSigMgrReleaseRouteStruct(pSigMgr, pSigMgr->pServiceRoute);
		pal_MemoryFree((void **)&pSigMgr->pServiceRoute);
	}

	if (pSigMgr->pSharedCredentials)
	{
		_EcrioSigMgrReleaseSharedCredentialsStruct(pSigMgr, (void*)pSigMgr->pSharedCredentials);
		pal_MemoryFree((void **)&pSigMgr->pSharedCredentials);
	}

	_EcrioSigMgrRemoveUserRegInfo(pSigMgr, (u_int32 *)(*ppRegisterIdentifier));
	pal_MemoryFree((void **)ppRegisterIdentifier);

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************

  Function:		EcrioSigMgrInitRegObject()

  Purpose:		API used to create user registration object.

  Description:	API used to create user registration object.

  Input:		SIGMGRHANDLE sigMgrHandle - Signaling Manager Instance

  OutPut:		void** pRegisterIdentifier - reg id.

  Returns:		error code.
*****************************************************************************/
u_int32 EcrioSigMgrInitRegObject
(
	SIGMGRHANDLE sigMgrHandle,
	void **ppRegisterIdentifier
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;

	/* check for incoming parameters*/
	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if ((ppRegisterIdentifier == NULL) || (*ppRegisterIdentifier != NULL))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	error = _EcrioSigMgrInitRegObject(pSigMgr, ppRegisterIdentifier);

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************

Function:		EcrioSigMgrStopRegistrationTimer()

Purpose:		API used to stop the Registration related timer.

Description:	API used to stop the Registration related timer to handle network disconnected state.

Input:		SIGMGRHANDLE sigMgrHandle - Signaling Manager Instance
            EcrioSigMgrRegisterTimerType - registration timer type

Returns:		error code.
*****************************************************************************/
u_int32 EcrioSigMgrStopRegistrationTimer
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrRegisterTimerType eRegTimerType,
	void *pRegIdentifier
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	u_int32 pal_error = KPALErrorNone;
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrUserRegisterInfoStruct *pUserInfo = NULL;

	/* check for incoming parameters*/
	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pRegIdentifier == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tpRegIdentifier == NULL, error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	error = _EcrioSigMgrGetUserRegInfoRegId(pSigMgr, pRegIdentifier, &pUserInfo);
	if (error != ECRIO_SIGMGR_NO_ERROR || pUserInfo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendRegister() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

	switch (eRegTimerType)
	{
		case EcrioSigMgrReRegisterTimer:
		{
			/* Stopping reregister timer */
			if (pUserInfo->reregistrationTimerId)
			{
				pal_error = pal_TimerStop(pUserInfo->reregistrationTimerId);
				if (pal_error != KPALErrorNone)
				{
					error = ECRIO_SIGMGR_TIMER_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tFailed to stop reregister timer, error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}

				pUserInfo->reregistrationTimerId = NULL;
			}
		}
		break;

		case EcrioSigMgrRegisterExpiryTimer:
		{
			/* Stopping registration expire timer */
			if (pUserInfo->bIsRegExpireTimerRunning == Enum_TRUE)
			{
				pal_error = pal_TimerStop(pUserInfo->uRegExpireTimerID);
				if (pal_error != KPALErrorNone)
				{
					error = ECRIO_SIGMGR_TIMER_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tFailed to stop register expire timer, error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}

				pUserInfo->uRegExpireTimerID = NULL;
				pUserInfo->bIsRegExpireTimerRunning = Enum_FALSE;
			}
		}
		break;

		case EcrioSigMgrRegisterTimerAll:
		{
			/* Stopping reregister timer */
			if (pUserInfo->reregistrationTimerId)
			{
				pal_error = pal_TimerStop(pUserInfo->reregistrationTimerId);
				if (pal_error != KPALErrorNone)
				{
					error = ECRIO_SIGMGR_TIMER_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tFailed to stop reregister timer, error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}

				pUserInfo->reregistrationTimerId = NULL;
			}

			/* Stopping registration expire timer */
			if (pUserInfo->bIsRegExpireTimerRunning == Enum_TRUE)
			{
				pal_error = pal_TimerStop(pUserInfo->uRegExpireTimerID);
				if (pal_error != KPALErrorNone)
				{
					error = ECRIO_SIGMGR_TIMER_ERROR;
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
						"%s:%u\tFailed to stop register expire timer, error=%u",
						__FUNCTION__, __LINE__, error);
					goto Error_Level_01;
				}

				pUserInfo->uRegExpireTimerID = NULL;
				pUserInfo->bIsRegExpireTimerRunning = Enum_FALSE;
			}
		}
		break;

		default:
		case EcrioSigMgrRegisterTimerNone:
		{
			error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		}
		break;
	}

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************
                    API Definition Section - End
*****************************************************************************/
