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

/**
* @file lims_Deregister.c
* @brief Implementation of lims deregistration functionality.
*/

#include "lims.h"
#include "lims_internal.h"

/**
* This function can be used to initiate a deregister request to the network.
* lims module internally check if the module is already registered with the network
* it will perform the deregistration else it will return success but doesn't send
* any request to the network.
*
* @param[in] handle				lims instance handle.
*
* Returns LIMS_NO_ERROR if successful, otherwise an error specific to the lims implementation.
*/
u_int32 lims_Deregister
(
	LIMSHANDLE handle
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
	u_int32 uSueError = ECRIO_SUE_NO_ERROR;
#ifdef ENABLE_QCMAPI
	u_int32 uPalError = KPALErrorNone;
#endif
	lims_moduleStruct *m = NULL;

	if (handle == NULL)
	{
		return LIMS_INVALID_PARAMETER1;
	}

	m = (lims_moduleStruct *)handle;

	pal_MutexGlobalLock(m->pal);

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

#ifdef ENABLE_QCMAPI
	if (m->moduleState == lims_Module_State_REGISTERED)
	{
		uPalError = pal_QcmDeregister(m->pal);
		if (uPalError != KPALErrorNone)
		{
			uLimsError = LIMS_SUE_REGISTER_ERROR;
			LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tpal_QcmDeregister() failed with error code: uPalError=%d",
				__FUNCTION__, __LINE__, uPalError);
		}

		uSueError = EcrioSUEDeregister(m->pSueHandle);
		if (uSueError != ECRIO_SUE_NO_ERROR && uSueError != ECRIO_SUE_NO_IMMEDIATE_EXIT_ERROR)
		{
			uLimsError = LIMS_SUE_REGISTER_ERROR;
			LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tEcrioSUEDeregister() failed with error code: uSueError=%d",
				__FUNCTION__, __LINE__, uSueError);
		}

		m->moduleState = lims_Module_State_CONNECTED;
		LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tChange State from REGISTERED to CONNECTED", __FUNCTION__, __LINE__);
	}
	else
	{
		uLimsError = LIMS_INVALID_STATE;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tCurrent State is not REGISTERED state",
			__FUNCTION__, __LINE__);
	}
#else
	if (m->moduleState == lims_Module_State_REGISTERED)
	{
		uSueError = EcrioSUEDeregister(m->pSueHandle);
		if (uSueError != ECRIO_SUE_NO_ERROR && uSueError != ECRIO_SUE_NO_IMMEDIATE_EXIT_ERROR)
		{
			uLimsError = LIMS_SUE_REGISTER_ERROR;
			LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tEcrioSUEDeregister() failed with error code: uSueError=%d",
				__FUNCTION__, __LINE__, uSueError);
		}

		m->moduleState = lims_Module_State_CONNECTED;
		LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tChange State from REGISTERED to CONNECTED", __FUNCTION__, __LINE__);
	}
	else
	{
		uLimsError = LIMS_INVALID_STATE;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tCurrent State is not REGISTERED state",
			__FUNCTION__, __LINE__);
	}
#endif

	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);

	pal_MutexGlobalUnlock(m->pal);

	return uLimsError;
}

#ifdef ENABLE_QCMAPI
u_int32 lims_QcmCloseConnection
(
	LIMSHANDLE handle
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
	u_int32 uPalError = KPALErrorNone;
	lims_moduleStruct *m = NULL;

	if (handle == NULL)
	{
		return LIMS_INVALID_PARAMETER1;
	}

	m = (lims_moduleStruct *)handle;

	pal_MutexGlobalLock(m->pal);

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	uPalError = pal_QcmCloseConnection(m->pal);
	if (uPalError != KPALErrorNone)
	{
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tpal_QcmCloseConnection() failed with error code: uPalError=%d",
			__FUNCTION__, __LINE__, uPalError);
		return LIMS_INVALID_OPERATION;
	}

	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);

	pal_MutexGlobalUnlock(m->pal);

	return uLimsError;
}
#endif