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
* @file lims_Register.c
* @brief Implementation of lims module registration functionality.
*/

#include "lims.h"
#include "lims_internal.h"

/**
* This function can be used to initiate register request to the network.
* Caller invoke this function once it has the valid communication bearer.
*
* Before invoking this function, caller need to invoke the
* lims_NetworkConnectionStateChange with connected network details.
*
* This function return error if there is no network connection.
*
* @param[in] handle				lims instance handle
* @param[in] uFeatures			Supported features. This is bitmask value of lims_FeatureEnums
* @param[in] pRegisterStruct	IPSec parameters details required to establish IPSec connection but it can be
*								set to NULL if IPSec is not required.
*
* @return Returns LIMS_NO_ERROR if successful, otherwise an error
* specific to the lims implementation.
* If returned LIMS_SOCKET_ERROR, application need to re-initalize lims by invoking
* lims_Deinit(), lims_Init(), lims_NetworkStateChange() and lims_Register().
*/
u_int32 lims_Register
(
	LIMSHANDLE handle,
	u_int32 uFeatures,
	lims_RegisterStruct* pRegisterStruct
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
	u_int32 uSueError = ECRIO_SUE_NO_ERROR;
	lims_moduleStruct *m = NULL;
	u_int32 uSigMgrError = ECRIO_SIGMGR_NO_ERROR;
#ifdef ENABLE_QCMAPI
	u_int32 uPalError = KPALErrorNone;
#endif

	if ((handle == NULL) || (pRegisterStruct == NULL))
	{
		return LIMS_INVALID_PARAMETER1;
	}

	m = (lims_moduleStruct *)handle;

	pal_MutexGlobalLock(m->pal);

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

#ifdef ENABLE_QCMAPI
	if (m->moduleState == lims_Module_State_INITIALIZED || m->moduleState == lims_Module_State_CONNECTED || m->moduleState == lims_Module_State_REGISTERED)
	{
		u_char *pFeature = NULL;
//		EcrioSUESetRegistrationStateToNone(m->pSigMgrHandle);
	
		LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u", __FUNCTION__, __LINE__);
		
		LIMS_ALLOC(LIMS_COMMON_BUFFER_SIZE, (void **)&pFeature, uLimsError, ERR_None, m->logHandle);

		LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u", __FUNCTION__, __LINE__);

		if (uFeatures & lims_Feature_CPM_PagerMode)
		{
			pal_StringNCopy(pFeature, LIMS_3GPP_ICSI_FEATURE_TAG_STRING);
			pal_StringNConcatenate(pFeature, "=");
			pal_StringNConcatenate(pFeature, LIMS_3GPP_ICSI_PAGER_MODE_FEATURE_TAG_VALUE_STRING);
			pal_StringNConcatenate(pFeature, ";");		
		}
		LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u", __FUNCTION__, __LINE__);
		if (uFeatures & lims_Feature_CPM_LargeMode)
		{
			if (pFeature == NULL)
			{
				pal_StringNCopy(pFeature, LIMS_3GPP_ICSI_FEATURE_TAG_STRING);
			}
			else
			{
				pal_StringNConcatenate(pFeature, LIMS_3GPP_ICSI_FEATURE_TAG_STRING);
			}
			pal_StringNConcatenate(pFeature, "=");
			pal_StringNConcatenate(pFeature, LIMS_3GPP_ICSI_LARGE_MODE_FEATURE_TAG_VALUE_STRING);
			pal_StringNConcatenate(pFeature, ";");
		}
		LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u", __FUNCTION__, __LINE__);
		if (uFeatures & lims_Feature_CPM_Chat)
		{
			if (pFeature == NULL)
			{
				pal_StringNCopy(pFeature, LIMS_3GPP_ICSI_FEATURE_TAG_STRING);
			}
			else
			{
				pal_StringNConcatenate(pFeature, LIMS_3GPP_ICSI_FEATURE_TAG_STRING);
			}
			pal_StringNConcatenate(pFeature, "=");
			pal_StringNConcatenate(pFeature, LIMS_3GPP_ICSI_CPM_SESSION_FEATURE_TAG_VALUE_STRING);
			pal_StringNConcatenate(pFeature, ";");
		}
		LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u", __FUNCTION__, __LINE__);
		if (uFeatures & lims_Feature_CPM_FT_MSRP)
		{

		}
		if (uFeatures & lims_Feature_CPM_FT_Http)
		{
			if (pFeature == NULL)
			{
				pal_StringNCopy(pFeature, LIMS_3GPP_IARI_FEATURE_TAG_STRING);
			}
			else
			{
				pal_StringNConcatenate(pFeature, LIMS_3GPP_IARI_FEATURE_TAG_STRING);
			}
			pal_StringNConcatenate(pFeature, "=");
			pal_StringNConcatenate(pFeature, LIMS_3GPP_IARI_FT_FEATURE_TAG_VALUE_STRING);
			pal_StringNConcatenate(pFeature, ";");
		}
		LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u", __FUNCTION__, __LINE__);
		if (uFeatures & lims_Feature_CPM_Chatbot)
		{
			if (pFeature == NULL)
			{
				pal_StringNCopy(pFeature, LIMS_3GPP_IARI_FEATURE_TAG_STRING);
			}
			else
			{
				pal_StringNConcatenate(pFeature, LIMS_3GPP_IARI_FEATURE_TAG_STRING);
			}
			pal_StringNConcatenate(pFeature, "=");
			pal_StringNConcatenate(pFeature, LIMS_3GPP_IARI_CHATBOT_FEATURE_TAG_VALUE_STRING);
			pal_StringNConcatenate(pFeature, ";");
		}
		LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u", __FUNCTION__, __LINE__);
		if (uFeatures & lims_Feature_CPM_Chat_Im)
		{
			if (pFeature == NULL)
			{
				pal_StringNCopy(pFeature, LIMS_3GPP_IARI_FEATURE_TAG_STRING);
			}
			else
			{
				pal_StringNConcatenate(pFeature, LIMS_3GPP_IARI_FEATURE_TAG_STRING);
			}
			pal_StringNConcatenate(pFeature, "=");
			pal_StringNConcatenate(pFeature, LIMS_3GPP_IARI_FT_FEATURE_TAG_VALUE_STRING);
			pal_StringNConcatenate(pFeature, ";");
		}
		LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u", __FUNCTION__, __LINE__);
		if (uFeatures & lims_Feature_CPM_FT_SF)
		{
			if (pFeature == NULL)
			{
				pal_StringNCopy(pFeature, LIMS_3GPP_ICSI_FEATURE_TAG_STRING);
			}
			else
			{
				pal_StringNConcatenate(pFeature, LIMS_3GPP_ICSI_FEATURE_TAG_STRING);
			}
			pal_StringNConcatenate(pFeature, "=");
			pal_StringNConcatenate(pFeature, LIMS_3GPP_ICSI_FT_SF_FEATURE_TAG_VALUE_STRING);
			pal_StringNConcatenate(pFeature, ";");
		}
		LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u", __FUNCTION__, __LINE__);
		if (uFeatures & lims_Feature_CPM_DEFERRED)
		{
			if (pFeature == NULL)
			{
				pal_StringNCopy(pFeature, LIMS_3GPP_ICSI_FEATURE_TAG_STRING);
			}
			else
			{
				pal_StringNConcatenate(pFeature, LIMS_3GPP_ICSI_FEATURE_TAG_STRING);
			}
			pal_StringNConcatenate(pFeature, LIMS_3GPP_ICSI_FEATURE_TAG_STRING);
			pal_StringNConcatenate(pFeature, "=");
			pal_StringNConcatenate(pFeature, LIMS_3GPP_ICSI_DEFERRED_FEATURE_TAG_VALUE_STRING);
//			pal_StringNConcatenate(pFeature, ";");
		}
		LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u", __FUNCTION__, __LINE__);

		uPalError = pal_QcmRegister(m->pal, pFeature);
		if (uPalError != KPALErrorNone)
		{
			uLimsError = LIMS_SUE_REGISTER_ERROR;
			LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tEcrioSUERegister() failed with error code: uPalError=%d",
				__FUNCTION__, __LINE__, uPalError);
			if (pFeature)
			{
				LIMS_FREE((void **)&pFeature, m->logHandle);
			}
			goto ERR_None;
		}

//		EcrioSUESetRegistrationStateToRegistering(m->pSigMgrHandle);
		if (pFeature)
		{
			LIMS_FREE((void **)&pFeature, m->logHandle);
		}

		m->uFeatures = uFeatures;
	}
	else
	{
		uLimsError = LIMS_INVALID_OPERATION_ERROR;
	}
#else
	if ((m->moduleState == lims_Module_State_CONNECTED) && (pRegisterStruct->pIPsecParams != NULL))
	{
		uSigMgrError = EcrioSigMgrSetIPSecParam(m->pSigMgrHandle, pRegisterStruct->pIPsecParams);
		if (uSigMgrError != ECRIO_SIGMGR_NO_ERROR)
		{
			uLimsError = LIMS_SIGMGR_SET_PARAM_ERROR;
			goto ERR_None;
		}
	}
	if (m->moduleState == lims_Module_State_CONNECTED || m->moduleState == lims_Module_State_REGISTERED)
	{
		/* cleanUp the previously configured feature tags and configured the newly requested feature tags. */
		uLimsError = lims_cleanUpAndCopyFeatureTags(m, uFeatures);
		if (uLimsError != LIMS_NO_ERROR)
		{
			goto ERR_None;
		}

		/*Invoke register API */
		uSueError = EcrioSUERegister(m->pSueHandle, m->uFeatureTagCount, m->ppFeatureTagStruct);
		if (uSueError != ECRIO_SUE_NO_ERROR)
		{
			if (uSueError == ECRIO_SUE_PAL_SOCKET_ERROR)
				uLimsError = LIMS_SOCKET_ERROR;
			else
				uLimsError = LIMS_SUE_REGISTER_ERROR;

			LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tEcrioSUERegister() failed with error code: uSueError=%d",
				__FUNCTION__, __LINE__, uSueError);
			goto ERR_None;
		}
	}
	else
	{
		uLimsError = LIMS_INVALID_OPERATION_ERROR;
	}
#endif

ERR_None:
	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);

	pal_MutexGlobalUnlock(m->pal);

	return uLimsError;
}
