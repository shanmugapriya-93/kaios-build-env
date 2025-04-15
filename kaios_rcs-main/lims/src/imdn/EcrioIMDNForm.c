/******************************************************************************

Copyright (c) 2018-2020 Ecrio, Inc. All Rights Reserved.

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
 * @file EcrioIMDNForm.c
 * @brief Implementation of the IMDN module forming functionality.
*/

#include "EcrioPAL.h"
#include "EcrioIMDN.h"
#include "EcrioIMDNCommon.h"


u_int32 ec_imdn_FormIMDNXmlBody
(
	EcrioIMDNStruct *s,
	IMDNDispoNotifStruct *pStruct,
	u_char *pBuff,
	u_int32 *pLen
)
{
	u_int32 uIMDNError = ECRIO_IMDN_NO_ERROR;
	u_char *pCur = NULL;

	/** Check parameter validity. */
	if (s == NULL)
	{
		return ECRIO_IMDN_INVALID_PARAMETER_1;
	}

	if (pStruct == NULL)
	{
		return ECRIO_IMDN_INVALID_PARAMETER_2;
	}

	if (pBuff == NULL)
	{
		return ECRIO_IMDN_INVALID_PARAMETER_3;
	}

	if (pLen == NULL)
	{
		return ECRIO_IMDN_INVALID_PARAMETER_4;
	}

	IMDNLOGI(s->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Note: In this function, the XML body length to be created does not exceed
	  * IMDN_XML_LENGTH (1024 bytes). Therefore, no length check is performed for
	  * each phrase to concatenate to the buffer. However, this does not apply if
	  * change definition the value of IMDN_XML_LENGTH.
	  */

	pal_MemorySet(pBuff, 0, IMDN_XML_LENGTH);
	pCur = pBuff;

	/** <?xml version="1.0" encoding="UTF-8"?> */
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_OPENING_ANGLE_BRACE, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_QUESTIONMARK, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_VERSION1, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_EQUALTO, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_DOUBLEQUOTES, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_VERSION, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_DOUBLEQUOTES, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_SPACE, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_ENCODING1, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_EQUALTO, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_DOUBLEQUOTES, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_ENCODING, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_DOUBLEQUOTES, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_QUESTIONMARK, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_CLOSING_ANGLE_BRACE, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_NEWLINE, &pCur);

	/** <imdn xmlns="urn:ietf:params:xml:ns:imdn"> */
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_OPENING_ANGLE_BRACE, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_COMPOSING_STRING, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_SPACE, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_COMPOSING_ATTRIBUTE_DEFAULTNS, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_EQUALTO, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_DOUBLEQUOTES, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_COMPOSING_VALUE_DEFAULTNS, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_DOUBLEQUOTES, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_CLOSING_ANGLE_BRACE, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_NEWLINE, &pCur);

	/** <message-id> */
	if (pStruct->pMsgId == NULL)
	{
		IMDNLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tMessage-ID is NULL", __FUNCTION__, __LINE__);
		uIMDNError = ECRIO_IMDN_INSUFFICIENT_PARAMETER;
		goto END;
	}
	else if (IMDN_MAX_MESSAGE_ID_LENGTH < pal_StringLength((const u_char *)pStruct->pMsgId))
	{
		IMDNLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tInvalid Message-ID", __FUNCTION__, __LINE__);
		uIMDNError = ECRIO_IMDN_MESSAGE_ID_TOO_LONG;
		goto END;
	}
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_OPENING_ANGLE_BRACE, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_MESSAGE_ID_STRING, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_CLOSING_ANGLE_BRACE, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, (u_char *)pStruct->pMsgId, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_OPENING_ANGLE_BRACE, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_SLASH, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_MESSAGE_ID_STRING, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_CLOSING_ANGLE_BRACE, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_NEWLINE, &pCur);

	//Rcpnt URI
	if (pStruct->pRecipientUri != NULL)
	{
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_OPENING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_RECIPIENTURI_STRING, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_CLOSING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, (u_char *)pStruct->pRecipientUri, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_OPENING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_SLASH, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_RECIPIENTURI_STRING, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_CLOSING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_NEWLINE, &pCur);
	}

	//Original Rcpnt URI
	if (pStruct->pOriginalRecipientUri != NULL)
	{
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_OPENING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_ORIGINALRECIPIENTURI_STRING, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_CLOSING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, (u_char *)pStruct->pOriginalRecipientUri, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_OPENING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_SLASH, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_ORIGINALRECIPIENTURI_STRING, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_CLOSING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_NEWLINE, &pCur);
	}

	/** <datetime> */
	if (pStruct->pDateTime == NULL)
	{
		IMDNLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tDateTime is NULL", __FUNCTION__, __LINE__);
		uIMDNError = ECRIO_IMDN_INSUFFICIENT_PARAMETER;
		goto END;
	}
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_OPENING_ANGLE_BRACE, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_DATETIME_STRING, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_CLOSING_ANGLE_BRACE, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, pStruct->pDateTime, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_OPENING_ANGLE_BRACE, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_SLASH, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_DATETIME_STRING, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_CLOSING_ANGLE_BRACE, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_NEWLINE, &pCur);

	/** <delivery-notification> */
	if (pStruct->eDisNtf == IMDNDisNtfTypeReq_Delivery)
	{
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_OPENING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_DELIVERY_NOTIFICATION_STRING, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_CLOSING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_OPENING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_STATUS_STRING, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_CLOSING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_OPENING_ANGLE_BRACE, &pCur);
		if (pStruct->eDelivery == IMDNDeliveryNotif_Delivered)
		{
			ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_DELIVERED_STRING, &pCur);
		}
		else if (pStruct->eDelivery == IMDNDeliveryNotif_Failed)
		{
			ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_FAILED_STRING, &pCur);
		}
		else if (pStruct->eDelivery == IMDNDeliveryNotif_Forbidden)
		{
			ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_FORBIDDEN_STRING, &pCur);
		}
		else if (pStruct->eDelivery == IMDNDeliveryNotif_Error)
		{
			ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_ERROR_STRING, &pCur);
		}
		else
		{
			IMDNLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tInvalid parameter: Delivery-Notification=%d",
				__FUNCTION__, __LINE__, pStruct->eDelivery);
			uIMDNError = ECRIO_IMDN_INSUFFICIENT_PARAMETER;
			goto END;
		}
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_SLASH, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_CLOSING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_OPENING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_SLASH, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_STATUS_STRING, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_CLOSING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_OPENING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_SLASH, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_DELIVERY_NOTIFICATION_STRING, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_CLOSING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_NEWLINE, &pCur);
	}

	/** <display-notification> */
	else if (pStruct->eDisNtf == IMDNDisNtfTypeReq_Display)
	{
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_OPENING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_DISPLAY_NOTIFICATION_STRING, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_CLOSING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_OPENING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_STATUS_STRING, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_CLOSING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_OPENING_ANGLE_BRACE, &pCur);
		if (pStruct->eDisplay == IMDNDisplayNotif_Displayed)
		{
			ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_DISPLAYED_STRING, &pCur);
		}
		else if (pStruct->eDisplay == IMDNDisplayNotif_Forbidden)
		{
			ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_FORBIDDEN_STRING, &pCur);
		}
		else if (pStruct->eDisplay == IMDNDisplayNotif_Error)
		{
			ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_ERROR_STRING, &pCur);
		}
		else
		{
			IMDNLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tInvalid parameter: Display-Notification=%d",
				__FUNCTION__, __LINE__, pStruct->eDisplay);
			uIMDNError = ECRIO_IMDN_INSUFFICIENT_PARAMETER;
			goto END;
		}
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_SLASH, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_CLOSING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_OPENING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_SLASH, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_STATUS_STRING, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_CLOSING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_OPENING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_SLASH, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_DISPLAY_NOTIFICATION_STRING, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_CLOSING_ANGLE_BRACE, &pCur);
		ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_NEWLINE, &pCur);
	}

	/** </imdn> */
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_OPENING_ANGLE_BRACE, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_SLASH, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_COMPOSING_STRING, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_CLOSING_ANGLE_BRACE, &pCur);
	ec_imdn_StringConcatenate(pCur, IMDN_XML_LENGTH, IMDN_XML_NEWLINE, &pCur);

	*pLen = pal_StringLength(pBuff);

END:

	IMDNLOGI(s->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return uIMDNError;
}

u_char *EcrioIMDNForm
(
	IMDN_HANDLE handle,
	IMDNDispoNotifStruct *pStruct,
	u_int32 *pLen,
	u_int32 *pError
)
{
	EcrioIMDNStruct *s = NULL;
	u_int32 uIMDNError = ECRIO_IMDN_NO_ERROR;
	u_int32 uLen = 0;
	u_char *pBuff = NULL;

	/** Check parameter validity. */
	if (pError == NULL)
	{
		return NULL;
	}

	if (handle == NULL)
	{
		*pError = ECRIO_IMDN_INVALID_HANDLE;
		return NULL;
	}

	if (pStruct == NULL)
	{
		*pError = ECRIO_IMDN_INVALID_PARAMETER_2;
		return NULL;
	}

	if (pLen == NULL)
	{
		*pError = ECRIO_IMDN_INVALID_PARAMETER_3;
		return NULL;
	}

	/** Set the IMDN handle, s. */
	s = (EcrioIMDNStruct *)handle;

	IMDNLOGI(s->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	IMDN_MUTEX_LOCK(s->mutexAPI, s->logHandle);

	s->work.uSize = 0;
	uIMDNError = ec_imdn_MaintenanceBuffer(&s->work, IMDN_XML_LENGTH);
	if (uIMDNError != ECRIO_IMDN_NO_ERROR)
	{
		IMDNLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tThe working buffer maintenance error.", __FUNCTION__, __LINE__);
		pBuff = NULL;
		uLen = 0;
		goto Err_BufferMaintenance;
	}

	pBuff = s->work.pData;

	/** Form IMDN XML body */
	uIMDNError = ec_imdn_FormIMDNXmlBody(s, pStruct, pBuff, &uLen);
	if (uIMDNError != ECRIO_IMDN_NO_ERROR)
	{
		IMDNLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tForm IMDN XML body error.", __FUNCTION__, __LINE__);
		pBuff = NULL;
		uLen = 0;
		goto Err_FormIMDNXmlBody;
	}

	IMDNLOGDUMPI(s->logHandle, KLogTypeSIP, pBuff, uLen);

	goto Err_None;

Err_BufferMaintenance:
Err_FormIMDNXmlBody:
Err_None:

	*pLen = uLen;
	*pError = uIMDNError;

	IMDN_MUTEX_UNLOCK(s->mutexAPI, s->logHandle);

	IMDNLOGI(s->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return pBuff;
}
