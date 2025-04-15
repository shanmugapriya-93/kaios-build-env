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

#include "EcrioCPM.h"
#include "EcrioCPMInternal.h"

u_int32 EcrioCPMFormIMDN
(
	CPMHANDLE hCPMHandle,
	EcrioCPMIMDispoNotifStruct *pIMDN,
	u_int32 *pLength,
	u_char **ppBuffer,
	u_int32 uNotifBodyElement
)
{
	u_int32 uError = ECRIO_CPM_NO_ERROR;
	EcrioCPMContextStruct *pContext = NULL;
	LOGHANDLE hLogHandle = NULL;

	if ((NULL == hCPMHandle) || (NULL == pIMDN) || (NULL == pLength) || (NULL == ppBuffer))
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	/** Get the CPM structure from the handle */
	pContext = (EcrioCPMContextStruct *)hCPMHandle;

	hLogHandle = pContext->hLogHandle;

	CPMLOGI(hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);	

	/* Code to add the IMDN details */
	if (NULL != pIMDN)
	{
		EcrioDateAndTimeStruct dateAndTime = { 0 };
		IMDNDispoNotifStruct imdnNotif = { 0 };
		char dateTime[30] = { 0 };

		/*ISO8601 style (yyyyMMdd'T'HHmmss.SSSZ)*/
		pal_UtilityGetDateAndTime(&dateAndTime);
		if (0 >= pal_StringSNPrintf((char*)&dateTime, 30, (char *)"%04u-%02u-%02uT%02u:%02u:%02uZ", dateAndTime.year, dateAndTime.month, \
			dateAndTime.day, dateAndTime.hour, dateAndTime.minute, dateAndTime.second))
		{
			CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
				__FUNCTION__, __LINE__);
			uError = ECRIO_CPM_MEMORY_ERROR;
			goto END;
		}

		imdnNotif.pDateTime = (u_char*)&dateTime;

		/* map the delivery enum */
		switch (pIMDN->pCPMIMDispoNotifBody[uNotifBodyElement].eDelivery)
		{
		case EcrioCPMIMDNDeliveryNotif_Delivered:
		{
			imdnNotif.eDelivery = IMDNDeliveryNotif_Delivered;
		}
		break;
		case EcrioCPMIMDNDeliveryNotif_Failed:
		{
			imdnNotif.eDelivery = IMDNDeliveryNotif_Failed;
		}
		break;
		case EcrioCPMIMDNDeliveryNotif_Forbidden:
		{
			imdnNotif.eDelivery = IMDNDeliveryNotif_Forbidden;
		}
		break;
		case EcrioCPMIMDNDeliveryNotif_Error:
		{
			imdnNotif.eDelivery = IMDNDeliveryNotif_Error;
		}
		break;
		default:
		{
			imdnNotif.eDelivery = IMDNDeliveryNotif_None;
		}
		break;
		}
		/* map the Disposition enum */
		switch (pIMDN->pCPMIMDispoNotifBody[uNotifBodyElement].eDisNtf)
		{
		case EcrioCPMIMDispositionNtfTypeReq_Delivery:
		{
			imdnNotif.eDisNtf = IMDNDisNtfTypeReq_Delivery;
		}
		break;
		case EcrioCPMIMDispositionNtfTypeReq_Display:
		{
			imdnNotif.eDisNtf = IMDNDisNtfTypeReq_Display;
		}
		break;
		default:
		{
			imdnNotif.eDisNtf = IMDNDisNtfTypeReq_None;
		}
		break;
		}
		/* map the display enum */
		switch (pIMDN->pCPMIMDispoNotifBody[uNotifBodyElement].eDisplay)
		{
		case EcrioCPMIMDNDisplayNotif_Displayed:
		{
			imdnNotif.eDisplay = IMDNDisplayNotif_Displayed;
		}
		break;
		case EcrioCPMIMDNDisplayNotif_Forbidden:
		{
			imdnNotif.eDisplay = IMDNDisplayNotif_Forbidden;
		}
		break;
		case EcrioCPMIMDNDisplayNotif_Error:
		{
			imdnNotif.eDisplay = IMDNDisplayNotif_Error;
		}
		break;
		default:
		{
			imdnNotif.eDisplay = IMDNDisplayNotif_None;
		}
		break;
		}

		imdnNotif.pMsgId = pIMDN->pCPMIMDispoNotifBody[uNotifBodyElement].pIMDNMsgId;
		imdnNotif.pRecipientUri = pIMDN->pCPMIMDispoNotifBody[uNotifBodyElement].pRecipientUri;
		imdnNotif.pOriginalRecipientUri = pIMDN->pCPMIMDispoNotifBody[uNotifBodyElement].pOriginalRecipientUri;

		*ppBuffer = EcrioIMDNForm(pContext->hIMDNHandle, &imdnNotif, pLength, &uError);

		if ((NULL == *ppBuffer) || (uError != ECRIO_IMDN_NO_ERROR))
		{
			CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioIMDNForm returned with uError=%u",
				__FUNCTION__, __LINE__, uError);
			uError = ECRIO_CPM_IMDN_ERROR;
		}
	}
	else
	{
		CPMLOGE(hLogHandle, KLogTypeGeneral, "%s:%u\tEcrioIMDNForm returned with uError=%u",
			__FUNCTION__, __LINE__, uError);
	}

END:
	CPMLOGI(hLogHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	
	return uError;
}
