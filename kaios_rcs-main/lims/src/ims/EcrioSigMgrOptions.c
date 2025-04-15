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
#include "EcrioSigMgrOptions.h"
#include "EcrioSigMgrOptionsInternal.h"


/*****************************************************************************

  Function:		  EcrioSigMgrSendOptionsRequest()

  Purpose:		  Construct and Send an Options Message.

  Description:	  Creates and sends a OPTIONS request.

  Input:		  SIGMGRHANDLE sigMgrHandle - Signaling Manager Instance.
                  u_char* pCallId - Call Id.
                  EcrioSigMgrOptionsRecipientInfoStruct* pStruct - information for recipient.
                  EcrioSigMgrSipMessageStruct* pOptionsReq - OPTIONS request Headers.

  OutPut:		  none

  Returns:		  error code.
*****************************************************************************/
u_int32 EcrioSigMgrSendOptionsRequest
(
	SIGMGRHANDLE sigMgrHandle,
	u_char *pCallId,
	EcrioSigMgrSipMessageStruct *pOptionsReq,
	u_char *pDest
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;

	if (sigMgrHandle == NULL) return ECRIO_SIGMGR_BAD_HANDLE;

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u\t", __FUNCTION__, __LINE__);

	if (pOptionsReq == NULL )
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tpParameter is NULL", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_NOT_INITIALIZED;
		goto Error_Level_01;
	}
	error = _EcrioSigMgrSendOptionsRequest(pSigMgr, pCallId, pOptionsReq, pDest);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendOptionsRequest() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_IMS_LIB_ERROR;
		goto Error_Level_01;
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

	return error;
}



u_int32 EcrioSigMgrSendOptionsResponse
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSipMessageStruct *pOptionsResp
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;

	if (sigMgrHandle == NULL) return ECRIO_SIGMGR_BAD_HANDLE;

	pSigMgr = (EcrioSigMgrStruct *)(sigMgrHandle);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u\t", __FUNCTION__, __LINE__);

	if (pOptionsResp == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tpParameter is NULL", __FUNCTION__, __LINE__);
		error = ECRIO_SIGMGR_NOT_INITIALIZED;
		goto Error_Level_01;
	}
	error = _EcrioSigMgrSendOptionsResponse(pSigMgr, pOptionsResp);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendOptionsResponse() error=%u",
			__FUNCTION__, __LINE__, error);

		error = ECRIO_SIGMGR_IMS_LIB_ERROR;
		goto Error_Level_01;
	}

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************
                    API Definition Section - End
*****************************************************************************/
