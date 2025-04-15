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

#include "EcrioPAL.h"

#include "EcrioSUEInternalFunctions.h"

#include "EcrioSigMgrInit.h"/* for EcrioSigMgrStructRelease API */

/**************************************************************************

  Function:		_EcrioSUEStructRelease()

  Purpose:		to release SUE Structure

  Description:	This internal function will be used from EcrioSUEStructRelease API at the time of
                release , and will release the structure members.

  Input:		ppData			- void double pointer.
                eDataType		- data type of type unsigned int.
                bReleaseParent  - data type of type BoolEnum.

  Output:		ppData			- void double pointer.

  Returns:		Error codes --
                ECRIO_SUE_NO_ERROR
                ECRIO_SUE_INSUFFICIENT_DATA_ERROR
                ECRIO_SUE_INVALID_INPUT_ERROR
                ECRIO_SUE_INCONSISTENT_DATA_ERROR
                ECRIO_SUE_INSUFFICIENT_MEMORY_ERROR
                ECRIO_SUE_PAL_ERROR
                ECRIO_SUE_INVALID_XML_SCHEMA_ERROR
                ECRIO_SUE_XML_PARSER_ERROR
                ECRIO_SUE_DEPLOYMENT_ERROR


**************************************************************************/
u_int32 _EcrioSUEStructRelease
(
	void **ppData,
	u_int32 eDataType,
	BoolEnum bReleaseParent
)
{
	u_int32 uError = ECRIO_SUE_NO_ERROR;

	if ((ppData == NULL) || (*ppData == NULL))
	{
		uError = ECRIO_SUE_INSUFFICIENT_DATA_ERROR;
		return uError;
	}

	switch (eDataType)
	{
		/*********************************************************************/

		// Interface Struct Release Start

		/*********************************************************************/

		case ECRIO_SUE_STRUCT_ENUM_EcrioSUENotifyNotifyRequestStruct:
		{
			/*EcrioSUENotifyNotifyRequestStruct *pStruct = (EcrioSUENotifyNotifyRequestStruct *)*ppData;

			if (pStruct->pSubscriptionState)
			{
			    pal_MemoryFree((void **)&pStruct->pSubscriptionState);
			    pStruct->pSubscriptionState = NULL;
			}*/

			break;
		};

		case ECRIO_SUE_STRUCT_ENUM_EcrioSUECallbackStruct:
		{
			EcrioSUECallbackStruct *pStruct = (EcrioSUECallbackStruct *)*ppData;

			pStruct->pNotifySUECallbackFn = NULL;
			pStruct->pCallbackData = NULL;

			break;
		}

		case ECRIO_SUE_STRUCT_ENUM_EcrioSUENotifyRegisterResponseStruct:
		{
			EcrioSUENotifyRegisterResponseStruct *pStruct = (EcrioSUENotifyRegisterResponseStruct *)*ppData;

			pStruct->uRspCode = 0;

			break;
		}

		case ECRIO_SUE_STRUCT_ENUM_EcrioSUENotifySubscribeResponseStruct:
		{
			EcrioSUENotifySubscribeResponseStruct *pStruct = (EcrioSUENotifySubscribeResponseStruct *)*ppData;

			pStruct->uRspCode = 0;

			break;
		}

		case ECRIO_SUE_STRUCT_ENUM_EcrioSUENotifyStruct:
		{
			EcrioSUENotifyStruct *pStruct = (EcrioSUENotifyStruct *)*ppData;

			switch (pStruct->eNotificationType)
			{
				case ECRIO_SUE_NOTIFICATION_ENUM_RegisterResponse:
				{
					if (pStruct->u.pRegisterResponse != NULL)
					{
						_EcrioSUEStructRelease((void **)&(pStruct->u.pRegisterResponse), ECRIO_SUE_STRUCT_ENUM_EcrioSUENotifyRegisterResponseStruct, Enum_TRUE);
					}

					break;
				}

				case ECRIO_SUE_NOTIFICATION_ENUM_DeregisterResponse:
				{
					if (pStruct->u.pDeregisterResponse != NULL)
					{
						_EcrioSUEStructRelease((void **)&(pStruct->u.pDeregisterResponse), ECRIO_SUE_STRUCT_ENUM_EcrioSUENotifyRegisterResponseStruct, Enum_TRUE);
					}

					break;
				}

				case ECRIO_SUE_NOTIFICATION_ENUM_SubscribeResponse:
				{
					if ((pStruct->u.pSubscribeResponse) != NULL)
					{
						_EcrioSUEStructRelease((void **)&(pStruct->u.pSubscribeResponse), ECRIO_SUE_STRUCT_ENUM_EcrioSUENotifySubscribeResponseStruct, Enum_TRUE);
					}

					break;
				}

				case ECRIO_SUE_NOTIFICATION_ENUM_NotifyRequest:
				{
					if ((pStruct->u.pNotifyRequest) != NULL)
					{
						_EcrioSUEStructRelease((void **)&(pStruct->u.pNotifyRequest), ECRIO_SUE_STRUCT_ENUM_EcrioSUENotifyNotifyRequestStruct, Enum_TRUE);
					}

					break;
				}

				default:
				{
				}
			}

			pStruct->eNotificationType = ECRIO_SUE_NOTIFICATION_ENUM_None;

			break;
		}

		/*  End -- For Notification */

		/*********************************************************************/

		// Interface Struct Release End

		// Internal Struct Release Start

		/*********************************************************************/

		case _ECRIO_SUE_STRUCT_ENUM_EcrioSUEGlobalDataStruct:
		{
			_EcrioSUEGlobalDataStruct *pStruct = (_EcrioSUEGlobalDataStruct *)*ppData;
			u_char c;
			pStruct->eEngineState = _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_None;
			if (pStruct->pTransportAddressStruct != NULL)
			{
				if (pStruct->pTransportAddressStruct->pLocalIP)
				{
					pal_MemoryFree((void **)&pStruct->pTransportAddressStruct->pLocalIP);
					pStruct->pTransportAddressStruct->pLocalIP = NULL;
				}

				for (c = 0; c < pStruct->pTransportAddressStruct->uNoPCSCF; c++)
				{
					if (pStruct->pTransportAddressStruct->ppPCSCFList[c] != NULL)
					{
						pal_MemoryFree((void **)&pStruct->pTransportAddressStruct->ppPCSCFList[c]);
					}
				}

				if (pStruct->pTransportAddressStruct->ppPCSCFList != NULL)
				{
					pal_MemoryFree((void **)&pStruct->pTransportAddressStruct->ppPCSCFList);
				}

				pStruct->pTransportAddressStruct->uLocalPort = 0;
				pStruct->pTransportAddressStruct->uPCSCFPort = 0;
				pStruct->pTransportAddressStruct->uPCSCFTLSPort = 0;
				pal_MemoryFree((void **)&pStruct->pTransportAddressStruct);
				pStruct->pTransportAddressStruct = NULL;
			}

			pStruct->uRegGeneralFailureCount = 0;
			pStruct->uSubGeneralFailureCount = 0;

			pStruct->bExitSUE = Enum_FALSE;
			pStruct->eExitReason = ECRIO_SUE_EXIT_REASON_ENUM_None;
			pStruct->bNotifyApp = Enum_FALSE;

			pStruct->uRegReAttemptTimerId = 0;

			pStruct->pLogHandle = NULL;

			break;
		}

		/*  Internal Struct Release End   */

		default:
		{
			uError = ECRIO_SUE_INVALID_INPUT_ERROR;
			break;
		}
	}

	// EndTag:

	if (bReleaseParent == Enum_TRUE)
	{
		pal_MemoryFree(ppData);
		*ppData = NULL;
	}

	return uError;
}
