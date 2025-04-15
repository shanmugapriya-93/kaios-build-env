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
#include "EcrioSUEEngine.h"
#include "EcrioSUEInternalFunctions.h"

u_int32 EcrioSUERegister
(
	SUEENGINEHANDLE	pSUEHandle,
	u_int16	numFeatureTagParams,
	EcrioSUESipParamStruct **ppFeatureTagParams
)
{
	u_int32	uError = ECRIO_SUE_NO_ERROR;
	_EcrioSUEGlobalDataStruct *pSUEGlobalData = NULL;

	if (pSUEHandle == NULL)
	{
		uError = ECRIO_SUE_INSUFFICIENT_DATA_ERROR;
		return uError;
	}

	pSUEGlobalData = (_EcrioSUEGlobalDataStruct *)pSUEHandle;

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncEntry,
		"%s:%u", __FUNCTION__, __LINE__);

	uError = EcrioSigMgrSetFeatureTagParams(pSUEGlobalData->pSigMgrHandle, numFeatureTagParams, (EcrioSigMgrParamStruct **)ppFeatureTagParams);
	if (uError != ECRIO_SUE_NO_ERROR)
	{
		SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
			"%s:%u\tEcrioSigMgrSetFeatureTagParams() uError=%u", __FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	/*If deregistered then send initial register.*/
	if (pSUEGlobalData->eRegistrationState == _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_DeRegistered ||
		pSUEGlobalData->eRegistrationState == _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_DeRegistrationFailed)
	{
		/*Release subscription if NOTIFY is not received after deregistration*/
		if (pSUEGlobalData->pSubscribeSessionId != NULL)
		{
			EcrioSigMgrAbortSession(pSUEGlobalData->pSigMgrHandle, pSUEGlobalData->pSubscribeSessionId);
			//pal_MemoryFree((void **)&(pSUEGlobalData->pSubscribeSessionId));
			pSUEGlobalData->pSubscribeSessionId = NULL;
		}

		ECRIO_SUE_LOG_REGISTRATION_STATE_CHANGE(pSUEGlobalData, (u_char *)"_EcrioSUEHandleRegisterResponse", _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_None);
		pSUEGlobalData->eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_None;
	}

	uError = EcrioSUESigMgrSendRegister(pSUEGlobalData,
		EcrioSigMgrRegisterRequestType_Reg);
	if (uError != ECRIO_SUE_NO_ERROR)
	{
		SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
			"%s:%u\tEcrioSUESigMgrSendRegister() uError=%u", __FUNCTION__, __LINE__, uError);
		goto Error_Level_01;
	}

	if (pSUEGlobalData->eRegistrationState == _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_Registered)
	{
		ECRIO_SUE_LOG_REGISTRATION_STATE_CHANGE(pSUEGlobalData, (u_char *)"_EcrioSUEHandleRegisterResponse", _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_ReRegistering);
		pSUEGlobalData->eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_ReRegistering;
	}
	else
	{
		ECRIO_SUE_LOG_REGISTRATION_STATE_CHANGE(pSUEGlobalData, (u_char *)"_EcrioSUEHandleRegisterResponse", _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_Registering);
		pSUEGlobalData->eRegistrationState = _ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_Registering;
	}

Error_Level_01:

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uError);

	return uError;
}

/******************************************************************************

                API EcrioSUEDeregister

******************************************************************************/

/**
 * This function is used to initiate de-registration procedure to the IMS network before the termination of SUE Engine.
 * This is a function should be called before EcrioSUEDeinit()
 * is called. If the network is connected this function sends REGISTER request with expires 0 and returns the ECRIO_SUE_NO_IMMEDIATE_EXIT_ERROR.
 * The integration layer should not call EcrioSUEDeinit() immediately if it gets the ECRIO_SUE_NO_IMMEDIATE_EXIT_ERROR, instead the integration layer
 * should wait for the ECRIO_SUE_NOTIFICATION_ENUM_ExitSUE notification. Once the de-registration transaction completes SUE notifies
 * the integration layer with ECRIO_SUE_NOTIFICATION_ENUM_ExitSUE. At this time the integration layer should call the EcrioSUEDeinit() API. However, the integration
 * layer should not call EcrioSUEDeinit() from this notification thread.
 * If network is not connected then EcrioSUEDeregister() returns with ECRIO_SUE_NO_ERROR error code. The integration layer can call EcrioSUEDeinit() API.
 *
 * @param[in] pSUEHandle			The SUE Handle that was created by calling the
 *									EcrioSUEInit() function. If NULL, the function
 *									will return an error.
 * @return                          ECRIO_SUE_NO_IMMEDIATE_EXIT_ERROR if de-registration procedure is initiated. ECRIO_SUE_NO_ERROR if
 *									de-registration is not initiated but no error occured otherwise a
 *									non-zero value indicating the reason for failure.
 */
u_int32 EcrioSUEDeregister
(
	SUEENGINEHANDLE	pSUEHandle
)
{
	u_int32	uError = ECRIO_SUE_NO_ERROR;
	_EcrioSUEGlobalDataStruct *pSUEGlobalData = NULL;

	if (!pSUEHandle)
	{
		uError = ECRIO_SUE_INSUFFICIENT_DATA_ERROR;
		goto EndTag;
	}

	pSUEGlobalData = (_EcrioSUEGlobalDataStruct *)(pSUEHandle);

	SUELOGI( pSUEGlobalData->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__ );

	if (
		(pSUEGlobalData->eEngineState != _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Running) ||
		(pSUEGlobalData->eNetworkState == _ECRIO_SUE_INTERNAL_NETWORK_STATE_ENUM_None)
		)
	{
		uError = ECRIO_SUE_INVALID_OPERATION_ERROR;

		ECRIO_SUE_LOG_INVALID_OPERATION_ERROR(pSUEGlobalData, (u_char *)"EcrioSUEDeregister");

		ECRIO_SUE_LOG_ALL_STATES(pSUEGlobalData, (u_char *)"EcrioSUEDeregister");

		goto EndTag;
	}

	uError = _EcrioSUEStop(pSUEGlobalData);

	if (uError != ECRIO_SUE_NO_ERROR && uError != ECRIO_SUE_NO_IMMEDIATE_EXIT_ERROR)
	{
		SUELOGV( pSUEGlobalData->pLogHandle, KLogTypeGeneral, "%s:%u\t Internal error --> ", __FUNCTION__, __LINE__ );
		goto EndTag;
	}

EndTag:

	if (pSUEGlobalData)
	{
		SUELOGI( pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u\t Exit --> %u", __FUNCTION__, __LINE__, uError);
	}

	return uError;
}

#ifdef ENABLE_QCMAPI
u_int32 EcrioSUESetFeatureTagParams
(
	SUEENGINEHANDLE	pSUEHandle,
	u_int16	numFeatureTagParams,
	EcrioSUESipParamStruct **ppFeatureTagParams
)
{
	u_int32	uError = ECRIO_SUE_NO_ERROR;
	_EcrioSUEGlobalDataStruct *pSUEGlobalData = NULL;

	if (pSUEHandle == NULL)
	{
		uError = ECRIO_SUE_INSUFFICIENT_DATA_ERROR;
		return uError;
	}

	pSUEGlobalData = (_EcrioSUEGlobalDataStruct *)pSUEHandle;

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncEntry,
		"%s:%u", __FUNCTION__, __LINE__);

	uError = EcrioSigMgrSetFeatureTagParams(pSUEGlobalData->pSigMgrHandle, numFeatureTagParams, (EcrioSigMgrParamStruct **)ppFeatureTagParams);
	if (uError != ECRIO_SUE_NO_ERROR)
	{
		SUELOGE(pSUEGlobalData->pLogHandle, KLogTypeGeneral,
			"%s:%u\tEcrioSigMgrSetFeatureTagParams() uError=%u", __FUNCTION__, __LINE__, uError);
		return uError;
	}

	SUELOGI(pSUEGlobalData->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uError);

	return uError;
}
#endif
