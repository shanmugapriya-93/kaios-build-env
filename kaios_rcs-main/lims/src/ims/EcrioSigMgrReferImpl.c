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
#include "EcrioSigMgrRefer.h"
#include "EcrioSigMgrReferInternal.h"


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

  Function:		_EcrioSigMgrSendRefer()

  Purpose:		Construct and Send an Refer Message.

  Description:	Creates and sends a REFER request.

  Input:		EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                u_char* pCallId - Call Id.
                EcrioSigMgrReferRecipientInfoStruct* pStruct - Information for recipient.
                EcrioSigMgrSipMessageStruct* pReferReq - Message request Headers.
				u_char** ppReferId - ID to identify the REFER.

  Input/OutPut: none.

  Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrSendRefer
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pCallId,
	EcrioSigMgrReferRecipientInfoStruct *pStruct,
	EcrioSigMgrSipMessageStruct *pReferReq,
	u_char** ppReferId
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	_EcrioSigMgrDialogNodeStruct *pDialogNode = NULL;
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	ec_MapGetKeyData(pSigMgr->hHashMap, pCallId, (void **)&pDialogNode);
	pUsageData = (_EcrioSigMgrInviteUsageInfoStruct *)pDialogNode->pDialogContext;
	if (NULL == pUsageData)
	{
		error = ECRIO_SIGMGR_INVALID_DATA;
		goto Error_Level_01;
	}

	pUsageData->pData = pStruct;

	/* Check and Populate optional header. */
	if (pReferReq->pOptionalHeaderList == NULL)
	{
		error = EcrioSigMgrCreateOptionalHeaderList(pSigMgr, &pReferReq->pOptionalHeaderList);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\tEcrioSigMgrCreateOptionalHeaderList() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	/** Supporting gZip feature */
	{
		u_char *pGzip = (u_char *)"gzip";
		error = _EcrioSigMgrAddOptionalHeader(pSigMgr, pReferReq->pOptionalHeaderList, EcrioSipHeaderTypeAcceptEncoding, 1, &pGzip, 0, NULL, NULL);
		if (error != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
				"%s:%u\t_EcrioSigMgrAddOptionalHeader() error=%u",
				__FUNCTION__, __LINE__, error);
			goto Error_Level_01;
		}
	}

	/* Operation starts here */
	pReferReq->eMethodType = EcrioSipMessageTypeRefer;
	error = _EcrioSigMgrSendReferInDialog(pSigMgr, pReferReq, pDialogNode, ppReferId);
	if (error != ECRIO_SIGMGR_NO_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\t_EcrioSigMgrSendInDialogSipRequests() error=%u",
			__FUNCTION__, __LINE__, error);
		goto Error_Level_01;
	}

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}
/*****************************************************************************

  Function:		_EcrioSigMgrHandleReferResponse()

  Purpose:		Handles the Refer respnse

  Description:	Handles an incoming REFER response.

  Input:		EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
                EcrioSigMgrMessageResponseStruct* pMessageRespNtf - Message
                response Headers.
                EcrioSigMgrCommonInfoStruct* pCmnInfo - Common headers

  Returns:		error code.
*****************************************************************************/
u_int32 _EcrioSigMgrHandleReferResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pMessageRespNtf,
	EcrioSigMgrCommonInfoStruct *pCmnInfo
)
{
	u_int32	error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrMessageStruct message = { 0 };
	u_int32 statusCode = ECRIO_SIGMGR_INVALID_STATUS_CODE;
	BoolEnum bUpperNotif = Enum_FALSE;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);
	if ((pMessageRespNtf == NULL) || (pCmnInfo == NULL))
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
			"%s:%u\tInsufficient data", __FUNCTION__, __LINE__);
		goto Error_Level_01;
	}
	message.msgCmd = EcrioSigMgrReferResponseNotification;
	message.pData = (void *)pMessageRespNtf;
	statusCode = pMessageRespNtf->responseCode / 100;
	if (statusCode == ECRIO_SIGMGR_1XX_RESPONSE)
	{
		goto Error_Level_01;
	}
	if (statusCode == ECRIO_SIGMGR_2XX_RESPONSE)
	{
		bUpperNotif = Enum_TRUE;
	}
	else
	{
		bUpperNotif = Enum_TRUE;
	}

	/* Notify to the Upperlayer */
	if (bUpperNotif)
		_EcrioSigMgrUpdateStatusCPM(pSigMgr, &message);

Error_Level_01:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/*****************************************************************************
                    API Definition Section - End
*****************************************************************************/
