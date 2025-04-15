/******************************************************************************

Copyright (c) 2019-2020 Ecrio, Inc. All Rights Reserved.

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
* @file lims_SetGroupChatSubject.c
* @brief Implementation of lims module group chat functionality.
*/

#include "lims.h"
#include "lims_internal.h"

/**
* This API can be called to change subject for existing group chat session.
*
* @param[in] handle		The lims instance handle.
* @param[in] pSubject		The string of subject.
* @param[in] pSessionId	The Session ID of the group chat that currently in existing.
*
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
*/
u_int32 lims_SetGroupChatSubject
(
	LIMSHANDLE handle,
	char *pSubject,
	char *pSessionId
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;

#if defined(ENABLE_RCS)
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
	EcrioCPMDataManagementStruct dmStruct = { 0 };
#endif

	lims_moduleStruct *m = NULL;

	if (handle == NULL)
	{
		return LIMS_INVALID_PARAMETER1;
	}

	m = (lims_moduleStruct *)handle;

	pal_MutexGlobalLock(m->pal);

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

#if defined(ENABLE_RCS)
#if 0
	if (m->sessionState != lims_Session_State_IDLE)
	{
		uLimsError = LIMS_INVALID_OPERATION;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t Already in session!",
			__FUNCTION__, __LINE__);
		goto ERR_None;
	}
#endif
	if (pSubject == NULL)
	{
		uLimsError = LIMS_INVALID_PARAMETER2;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t pSubject is NULL",
			__FUNCTION__, __LINE__);
		goto ERR_None;
	}

	dmStruct.subject.action = EcrioCPMDataActionType_Set;
	dmStruct.subject.pSubjectLine = pSubject;
	dmStruct.icon.pIconUri = NULL;

	uCPMError = EcrioCPMModifyGroupChatData(m->pCpmHandle, &dmStruct, pSessionId);
	if (uCPMError != ECRIO_CPM_NO_ERROR)
	{
		if (uCPMError == ECRIO_CPM_SOCKET_ERROR)
			uLimsError = LIMS_SOCKET_ERROR;
		else
			uLimsError = LIMS_CPM_SET_GROUPCHAT_SUBJECT_ERROR;

		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t EcrioCPMModifyGroupChatData() failed with error code uCPMError:%d",
			__FUNCTION__, __LINE__, uCPMError);
		goto ERR_None;
	}

#if 0
		m->sessionState = lims_Session_State_INVITING;
#endif
#else
	pSubject = pSubject;
	pSessionId = pSessionId;
	LIMSLOGW(m->logHandle, KLogTypeGeneral, "%s:%u-  CPM feature disabled from this build", __FUNCTION__, __LINE__);
	uLimsError = LIMS_CPM_FEATURE_NOT_SUPPORTED;
	goto ERR_None;
#endif

ERR_None:
	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);

	pal_MutexGlobalUnlock(m->pal);

	return uLimsError;
}
