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

#include "EcrioSms.h"
#include "EcrioSmsInternal.h"

#include "EcrioIOTA.h"
/** API for Initialising the SMS ALC.
*@ Param IN  EcrioSMSParamStruct		pSMSParamStruct - Structure holding the initial
                                               configuration for the SMS ALC.
*@ Param IN  EcrioSMSCallbackStruct	pSMSCallback - structure holding the Callback
                                               Functions to be invoked when a message/
                                               acknowledgement/ delivery report / error
                                               is to be notified to the application.
*@ Param OUT ECRIOSMSHANDLE			hEcrioSMS - SMS Handle. This must be used by the
                                               application while invoking any subsequent
                                               API's provided by SMS ALC.
*@ return								u_int32 - Indicating the initialization status
                                               ECRIO_SMS_SUCCESS or ECRIO_SMS_FAILURE.
**/
u_int32 EcrioSmsInit
(
	EcrioSmsParamStruct *pSmsParam,
	EcrioSmsCallbackStruct *pSMSCallback,
	void *hSigMgrHandle,
	ECRIOSMSHANDLE *hEcrioSMS
)
{
	u_int32 dwSMSStatus = ECRIO_SMS_NO_ERROR;
	EcrioSMSContext	*pContextStruct = NULL;
	u_int32	sigMgrError = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrEngineCallbackStruct sigMgrCB = { 0 };
	LOGHANDLE hLogHandle = NULL;

	if (pSmsParam == NULL || pSMSCallback == NULL)
	{
		return ECRIO_SMS_INSUFFICIENT_DATA_ERROR;
	}

	hLogHandle = pSmsParam->hLogHandle;
	SMSLOGI(hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pal_MemoryAllocate(sizeof(EcrioSMSContext), (void **)hEcrioSMS);
	if (NULL == *hEcrioSMS)
	{
		dwSMSStatus = ECRIO_SMS_MEMORY_ERROR;

		SMSLOGE(hLogHandle, KLogTypeGeneral,
			"%s:%u\tpal_MemoryAllocate() dwSMSStatus=%u",
			__FUNCTION__, __LINE__, dwSMSStatus);

		*hEcrioSMS = NULL;
		goto Error_Level_01;
	}

	pal_MemorySet(*hEcrioSMS, 0, sizeof(EcrioSMSContext));
	pContextStruct = (EcrioSMSContext *)*hEcrioSMS;

	/* Remember the Log Handle and Module Id */
	pContextStruct->hLogHandle = hLogHandle;

	pContextStruct->callbackStruct = *pSMSCallback;

	pContextStruct->hEcrioSigMgrHandle = hSigMgrHandle;

	sigMgrCB.infoCallbackFn = SMSSigMgrInfoCallback;
	sigMgrCB.statusCallbackFn = SMSSigMgrStatusCallback;
	sigMgrCB.pCallbackData = pContextStruct;

	sigMgrError = EcrioSigMgrSetCallbacks(pContextStruct->hEcrioSigMgrHandle, &sigMgrCB, EcrioSigMgrCallbackRegisteringModule_SMS);
	if (sigMgrError != ECRIO_SIGMGR_NO_ERROR)
	{
		dwSMSStatus = ECRIO_SMS_INTERNAL_ERROR;
		goto Error_Level_01;
	}

	goto Error_Level_01;

Error_Level_01:
	SMSLOGI(hLogHandle, KLogTypeFuncExit, "%s:%u dwSMSStatus=%u",
		__FUNCTION__, __LINE__, dwSMSStatus);

	if (dwSMSStatus != ECRIO_SMS_NO_ERROR)
	{
		if (*hEcrioSMS)
		{
			pal_MemoryFree((void **)hEcrioSMS);
		}

		*hEcrioSMS = NULL;
	}

	return dwSMSStatus;
}
