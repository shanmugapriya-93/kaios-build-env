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
* @file lims_Deinit.c
* @brief Implementation of lims deinitialization functionality.
*/

#include "lims.h"
#include "lims_internal.h"

/**
* This function is used to deinitialize the lims implementation instance,
* cleaning up all consumed resources.
*
* This function return error if device is in registration state. Invoke
* lims_Deregister() function first before invoking this function even if there is no
* active network connection.
*
* Invoking this function without initializing the lims module will also return
* error.
*
* The caller should set their instance handle to NULL since upon return
* from this function it will not point to a valid context any more.
*
* @param[in] handle			Pointer to lims instance handle.
*
* @return Returns LIMS_NO_ERROR if successful, otherwise an error
* specific to the lims implementation.
*/
u_int32 lims_Deinit
(
	LIMSHANDLE *handle
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
	lims_moduleStruct *m = NULL;

#ifdef ENABLE_LOG
	LOGHANDLE logHandle = NULL;
#endif

	PALINSTANCE pal = NULL;

	if (handle == NULL || *handle == NULL)
	{
		return LIMS_INVALID_PARAMETER1;
	}

	m = (lims_moduleStruct *)*handle;
#ifdef ENABLE_LOG
	logHandle = m->logHandle;
#endif

	pal = m->pal;

	pal_MutexGlobalLock(pal);

#ifdef ENABLE_QCMAPI
	pal_QcmDeinit(m->qcmInstance);
#endif

	LIMSLOGI(logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	if (m->moduleState == lims_Module_State_UNINITIALIZED)
	{
		LIMSLOGW(logHandle, KLogTypeGeneral, "%s:%u\t current state is UNINITIALIZED",
			__FUNCTION__, __LINE__);
		goto ERR_None;
	}

	lims_cleanUpModuleStructure(m);

	if (m != NULL)
	{
		LIMS_FREE((void **)&m, m->logHandle);
		m = NULL;
	}

ERR_None:
	LIMSLOGI(logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);

	pal_MutexGlobalUnlock(pal);

	return uLimsError;
}
