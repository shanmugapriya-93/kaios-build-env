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
#include "EcrioSigMgrCallControl.h"
#include "EcrioSigMgrCallControlInternal.h"
#include "EcrioSigMgrDialogHandler.h"
#include "EcrioSigMgrSubscribe.h"
#include "EcrioSigMgrSubscribeInternal.h"

/*****************************************************************************
                Internal API Declaration Section - Begin
*****************************************************************************/

/*****************************************************************************
                Internal API Declaration Section - End
*****************************************************************************/

/*****************************************************************************
                    API Definition Section - Begin
*****************************************************************************/
#if 0
/*****************************************************************************

  Function:		EcrioSigMgrCompareSessionID()

  Purpose:		compare Session ids.

  Description:	compare Session ids.

  Input:		SIGMGRHANDLE sigMgrHandle - Signaling Manager Instance
                SIGSESSIONHANDLE srcSessionId - Source registration id.
                SIGSESSIONHANDLE destSessionId - destination id.

  OutPut:		result - Enum_TRUE, if equal else Enum_FALSE.

  Returns:		error code.
*****************************************************************************/
u_int32 EcrioSigMgrCompareSessionID
(
	SIGMGRHANDLE sigMgrHandle,
	SIGSESSIONHANDLE srcSessionId,
	SIGSESSIONHANDLE destSessionId,
	BoolEnum *pResult
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrSessionHandleStruct *pSrcSessionHandle = NULL;
	EcrioSigMgrSessionHandleStruct *pDestSessionHandle = NULL;

	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pResult == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data passed", __FUNCTION__, __LINE__);

		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	*pResult = Enum_FALSE;

	if ((srcSessionId == NULL) || (destSessionId == NULL))
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data passed", __FUNCTION__, __LINE__);

		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	pSrcSessionHandle = (EcrioSigMgrSessionHandleStruct *)srcSessionId;
	pDestSessionHandle = (EcrioSigMgrSessionHandleStruct *)destSessionId;

	if (pSrcSessionHandle->pDialogHandle ==
		pDestSessionHandle->pDialogHandle)
	{
		*pResult = Enum_TRUE;
	}
	else
	{
		*pResult = Enum_FALSE;
	}

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************

  Function:		EcrioSigMgrReleaseSessionId()

  Purpose:		API used to release session Id.

  Description:	API used to release session Id.

  Input:		SIGMGRHANDLE sigMgrHandle - Signaling Manager Instance

  OutPut:		SIGSESSIONHANDLE* pSessionId - session id.

  Returns:		error code.
*****************************************************************************/
u_int32 EcrioSigMgrReleaseSessionId
(
	SIGMGRHANDLE sigMgrHandle,
	SIGSESSIONHANDLE *pSessionId
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;

	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tsigMgrHandle=%p, pSessionId=%p",
		__FUNCTION__, __LINE__, sigMgrHandle, pSessionId);

	if (pSessionId == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInvalid data passed", __FUNCTION__, __LINE__);

		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	_EcrioSigMgrInternalStructRelease(pSigMgr, EcrioSigMgrStructType_SessionHandle,
		(void **)pSessionId, Enum_TRUE);

	*pSessionId = NULL;

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}
#endif

/*****************************************************************************

Function:		EcrioSigMgrAbortSession()

Purpose:		Cleans the context of the session

Description:	Releases all data structures and deletes the dialog related
information.

Input:			SIGMGRHANDLE sigMgrHandle - Handle to signaling manager.

Input:			SIGSESSIONHANDLE* sessionHandle - Handle to session.

Returns:		error code.

*****************************************************************************/
u_int32 EcrioSigMgrAbortSession
(
	SIGMGRHANDLE sigMgrHandle,
	u_char *pCallId
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;
	_EcrioSigMgrDialogNodeStruct *pDialogHandle = NULL;
	_EcrioSigMgrSubscribeUsageInfoStruct *pSubscribeUsage = NULL;
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsage = NULL;

	if (sigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	ec_MapGetKeyData(pSigMgr->hHashMap, pCallId, (void **)&pDialogHandle);
	if (pDialogHandle == NULL)
	{
		error = ECRIO_SIGMGR_DIALOG_ERROR;
		goto End_Tag;
	}

	if (pDialogHandle->eDialogType == _EcrioSigMgrDialogType_INVITE)
	{
		pInviteUsage = pDialogHandle->pDialogContext;
	}
	else
	{
		pSubscribeUsage = pDialogHandle->pDialogContext;
	}

	_EcrioSigMgrDeleteDialog(pSigMgr, pDialogHandle);

	if (pInviteUsage != NULL)
	{
		_EcrioSigMgrInternalStructRelease(pSigMgr, EcrioSigMgrStructType_InviteUsageInfo,
			(void **)&pInviteUsage, Enum_TRUE);
	}
	else
	{
		_EcrioSigMgrInternalStructRelease(pSigMgr, EcrioSigMgrStructType_SubscribeUsageInfo,
			(void **)&pSubscribeUsage, Enum_TRUE);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral,
		"%s:%u\tDialog deleted", __FUNCTION__, __LINE__);

End_Tag:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************
                    API Definition Section - End
*****************************************************************************/
