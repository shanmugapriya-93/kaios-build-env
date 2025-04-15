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
* @file lims_SendData
* @brief Implementation of lims module to send data chunk within a session.
*/

#include "lims.h"
#include "lims_internal.h"

/**
* This API can be called to send data within a File Transfer session. Maximum size
* of the data chunk that can be sent is 2048 Bytes.
*
* @param[in] handle					The lims instance handle.
* @param[in] pData					Pointer to lims_DataStruct.
* @param[in,out] ppMessageId		Message Identifier.
* @param[in,out] ppTransactionId	Transaction Identifier.
* @param[in] bIsCPIMWrapped			Condition variable to send file data wrapped with CPIM message.
*
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
*/
u_int32 lims_SendData
(
	LIMSHANDLE handle,
	lims_DataStruct* pData,
	char **ppMessageId,
	char **ppTransactionId,
	BoolEnum bIsCPIMWrapped
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
#if defined(ENABLE_RCS)
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
	EcrioCPMSendDataStruct cpmSendData = { 0 };
#endif //ENABLE_RCS

	lims_moduleStruct *m = NULL;

	if (handle == NULL)
	{
		return LIMS_INVALID_PARAMETER1;
	}

	m = (lims_moduleStruct *)handle;

//	pal_MutexGlobalLock(m->pal);

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
#if defined(ENABLE_RCS)
	if (pData == NULL)
	{
		uLimsError = LIMS_INVALID_PARAMETER2;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t pSendMessage is NULL",
			__FUNCTION__, __LINE__);
		goto ERR_None;
	}

	/* Populate file data */
	cpmSendData.pSessionId = pData->pSessionId;
	cpmSendData.pData = pData->pDataBuf;
	cpmSendData.uByteRangeStart = pData->uByteRangeStart;
	cpmSendData.uByteRangeEnd = pData->uByteRangeEnd;
	cpmSendData.uTotal = pData->uByteRangeTotal;
	cpmSendData.pContentType = pData->pContentType;
	cpmSendData.bIsEofReached = pData->bIsEofReached;

	uCPMError = EcrioCPMSendData(m->pCpmHandle, &cpmSendData, (u_char **)ppMessageId, (u_char **)ppTransactionId, bIsCPIMWrapped);
	if (uCPMError != ECRIO_CPM_NO_ERROR)
	{
		uLimsError = LIMS_CPM_SENDFILE_ERROR;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t EcrioCPMSendData is failed",
			__FUNCTION__, __LINE__);
		goto ERR_None;
	}
#else
	LIMSLOGW(m->logHandle, KLogTypeGeneral, "%s:%u-  CPM feature disabled from this build", __FUNCTION__, __LINE__);
	uLimsError = LIMS_CPM_FEATURE_NOT_SUPPORTED;
	goto ERR_None;
#endif //ENABLE_RCS

ERR_None:

	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);

//	pal_MutexGlobalUnlock(m->pal);

	return uLimsError;
}
