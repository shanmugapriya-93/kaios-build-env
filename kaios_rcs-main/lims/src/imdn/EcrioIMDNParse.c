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
 * @file EcrioIMDNParse.c
 * @brief Implementation of the IMDN module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioIMDN.h"
#include "EcrioIMDNCommon.h"
#include "yxml.h"


u_int32 EcrioIMDNHandler
(
	IMDN_HANDLE handle,
	IMDNDispoNotifStruct *pStruct,
	u_char *pData,
	u_int32 uLen
)
{
	enum
	{
		element_none			= 0,
		element_messageid		= 1,
		element_datetime		= 2,
		element_deliv_notif		= 4,
		element_dispo_notif		= 8,
		element_status					= 16,
		element_recipient_uri			= 32,
		element_original_recipient_uri	= 64,
	} prev_elements = element_none, curr_element = element_none;

	EcrioIMDNStruct *s = NULL;
	u_int32 uIMDNError = ECRIO_IMDN_NO_ERROR;
	u_int32 index = 0;
	yxml_ret_t yxmlret;
	yxml_t x[1];
	u_char *pContent = NULL, *pTemp = NULL;
	u_char size_buf[64];
	pal_MemorySet((void*)&size_buf, '\0', sizeof(size_buf));

	/** Check parameter validity. */
	if (handle == NULL)
	{
		return ECRIO_IMDN_INVALID_PARAMETER_1;
	}
	if (pStruct == NULL)
	{
		return ECRIO_IMDN_INVALID_PARAMETER_2;
	}
	if (pData == NULL)
	{
		return ECRIO_IMDN_INVALID_PARAMETER_3;
	}
	if (uLen == 0)
	{
		return ECRIO_IMDN_INVALID_PARAMETER_4;
	}

	/** Set the IMDN handle, s. */
	s = (EcrioIMDNStruct *)handle;

	IMDNLOGI(s->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	IMDN_MUTEX_LOCK(s->mutexAPI, s->logHandle);

	/** Check the working buffer */
	s->strings.uSize = 0;
	pal_MemorySet(s->strings.pData, 0, s->strings.uContainerSize);

	s->work.uSize = 0;
	uIMDNError = ec_imdn_MaintenanceBuffer(&s->work, uLen);
	if (uIMDNError != ECRIO_IMDN_NO_ERROR)
	{
		IMDNLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tThe working buffer maintenance error.", __FUNCTION__, __LINE__);
		goto END;
	}

	pal_MemorySet(s->work.pData, 0, s->work.uContainerSize);

	yxml_init(x, s->work.pData, s->work.uContainerSize);

	for (index = 0; index < uLen; index++)
	{
		yxmlret = yxml_parse(x, (int)pData[index]);

		switch (yxmlret)
		{
			case YXML_OK:
			{
			}
			break;

			case YXML_ELEMSTART:
			{
				if (pal_StringCompare((u_char *)x->elem, IMDN_MESSAGE_ID_STRING) == 0)
				{
					prev_elements = prev_elements | element_messageid;
					curr_element = element_messageid;
					pContent = size_buf;
				}
				else if (pal_StringCompare((u_char *)x->elem, IMDN_DATETIME_STRING) == 0)
				{
					prev_elements = prev_elements | element_datetime;
					curr_element = element_datetime;
					pContent = size_buf;
				}
				else if (pal_StringCompare((u_char *)x->elem, IMDN_RECIPIENTURI_STRING) == 0)
				{
					prev_elements = prev_elements | element_recipient_uri;
					curr_element = element_recipient_uri;
					pContent = size_buf;
				}
				else if (pal_StringCompare((u_char *)x->elem, IMDN_ORIGINALRECIPIENTURI_STRING) == 0)
				{
					prev_elements = prev_elements | element_original_recipient_uri;
					curr_element = element_original_recipient_uri;
					pContent = size_buf;
				}
				else if (pal_StringCompare((u_char *)x->elem, IMDN_DELIVERY_NOTIFICATION_STRING) == 0)
				{
					pStruct->eDisNtf = IMDNDisNtfTypeReq_Delivery;
					prev_elements = prev_elements | element_deliv_notif;
					curr_element = element_deliv_notif;
				}
				else if (pal_StringCompare((u_char *)x->elem, IMDN_DISPLAY_NOTIFICATION_STRING) == 0)
				{
					pStruct->eDisNtf = IMDNDisNtfTypeReq_Display;
					prev_elements = prev_elements | element_dispo_notif;
					curr_element = element_dispo_notif;
				}
				else if (pal_StringCompare((u_char *)x->elem, IMDN_STATUS_STRING) == 0)
				{
					prev_elements = prev_elements | element_status;
				}
				else
				{
					if ((prev_elements & element_status) == element_status &&
						curr_element == element_deliv_notif)
					{
						if (pal_StringCompare((u_char *)x->elem, IMDN_DELIVERED_STRING) == 0)
						{
							pStruct->eDelivery = IMDNDeliveryNotif_Delivered;
						}
						else if (pal_StringCompare((u_char *)x->elem, IMDN_FAILED_STRING) == 0)
						{
							pStruct->eDelivery = IMDNDeliveryNotif_Failed;
						}
						else if (pal_StringCompare((u_char *)x->elem, IMDN_FORBIDDEN_STRING) == 0)
						{
							pStruct->eDelivery = IMDNDeliveryNotif_Forbidden;
						}
						else if (pal_StringCompare((u_char *)x->elem, IMDN_ERROR_STRING) == 0)
						{
							pStruct->eDelivery = IMDNDeliveryNotif_Error;
						}
					}
					else if ((prev_elements & element_status) == element_status &&
							  curr_element == element_dispo_notif)
					{
						if (pal_StringCompare((u_char *)x->elem, IMDN_DISPLAYED_STRING) == 0)
						{
							pStruct->eDisplay = IMDNDisplayNotif_Displayed;
						}
						else if (pal_StringCompare((u_char *)x->elem, IMDN_FORBIDDEN_STRING) == 0)
						{
							pStruct->eDisplay = IMDNDisplayNotif_Forbidden;
						}
						else if (pal_StringCompare((u_char *)x->elem, IMDN_ERROR_STRING) == 0)
						{
							pStruct->eDisplay = IMDNDisplayNotif_Error;
						}
					}
				}
			}
			break;

			case YXML_ELEMEND:
			{
				if (pContent != NULL)
				{
					/** As pContent pointer is incremented on YXML_CONTENT,
					  * so get the actual content from size_buf.
					  */
					pContent = size_buf;

					switch (curr_element)
					{
						case element_messageid:
						{
							uIMDNError = ec_imdn_StringCopy(&s->strings,
											(void **)&pStruct->pMsgId,
											s->strings.uContainerSize,
											(u_char *)pContent,
											pal_StringLength((u_char *)pContent));
							if (uIMDNError != ECRIO_IMDN_NO_ERROR)
							{
								IMDNLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tCouldn't allocate memory for string.", __FUNCTION__, __LINE__);
								uIMDNError = ECRIO_IMDN_MEMORY_ALLOCATION_ERROR;
								goto END;
							}
						}
						break;

						case element_datetime:
						{
							uIMDNError = ec_imdn_StringCopy(&s->strings,
											(void **)&pStruct->pDateTime,
											s->strings.uContainerSize, 
											(u_char *)pContent,
											pal_StringLength((u_char *)pContent));
							if (uIMDNError != ECRIO_IMDN_NO_ERROR)
							{
								IMDNLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tCouldn't allocate memory for string.", __FUNCTION__, __LINE__);
								uIMDNError = ECRIO_IMDN_MEMORY_ALLOCATION_ERROR;
								goto END;
							}
						}
						break;

						case element_recipient_uri:
						{
							uIMDNError = ec_imdn_StringCopy(&s->strings,
								(void **)&pStruct->pRecipientUri,
								s->strings.uContainerSize, 
								(u_char *)pContent,
								pal_StringLength((u_char *)pContent));
							if (uIMDNError != ECRIO_IMDN_NO_ERROR)
							{
								IMDNLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tCouldn't allocate memory for string.", __FUNCTION__, __LINE__);
								uIMDNError = ECRIO_IMDN_MEMORY_ALLOCATION_ERROR;
								goto END;
							}
						}
						break;

						case element_original_recipient_uri:
						{
							uIMDNError = ec_imdn_StringCopy(&s->strings,
								(void **)&pStruct->pOriginalRecipientUri,
								s->strings.uContainerSize, 
								(u_char *)pContent,
								pal_StringLength((u_char *)pContent));
							if (uIMDNError != ECRIO_IMDN_NO_ERROR)
							{
								IMDNLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tCouldn't allocate memory for string.", __FUNCTION__, __LINE__);
								uIMDNError = ECRIO_IMDN_MEMORY_ALLOCATION_ERROR;
								goto END;
							}
						}
						break;

						default:
							break;
					}

					pContent = NULL;
				}

				prev_elements = prev_elements ^ curr_element;
			}
			break;

			case YXML_CONTENT:
			{
				pTemp = (u_char*)x->data;
				if (pContent != NULL)
				{
					while (*pTemp && pContent < size_buf + sizeof(size_buf))
					{
						*(pContent++) = *(pTemp++);
					}

					if (pContent == size_buf + sizeof(size_buf))
					{
						/** Terminate the loop */
						IMDNLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tXML Parsing failed, too long content value, memeory error."
							, __FUNCTION__, __LINE__);
						index = uLen;
					}
					else
					{
						*pContent = '\0';
					}
				}
			}
			break;

			case YXML_ATTRSTART:
			{
			}
			break;

			case YXML_ATTREND:
			{
			}
			break;

			case YXML_ATTRVAL:
			{
			}
			break;

			case YXML_PISTART:
			{
			}
			break;

			case YXML_PIEND:
			{
			}
			break;

			case YXML_PICONTENT:
			{
			}
			break;

			default:
				/** Terminate the loop */
				index = uLen;
		}
	}

	if (yxml_eof(x) < 0)
	{
		IMDNLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tXML Parsing failed.", __FUNCTION__, __LINE__);
		uIMDNError = ECRIO_IMDN_PARSING_INSUFFICIENT_ERROR;
		goto END;
	}

END:

	IMDN_MUTEX_UNLOCK(s->mutexAPI, s->logHandle);

	IMDNLOGI(s->logHandle, KLogTypeFuncExit, "%s:%u\t%u", __FUNCTION__, __LINE__, uIMDNError);

	return uIMDNError;
}


