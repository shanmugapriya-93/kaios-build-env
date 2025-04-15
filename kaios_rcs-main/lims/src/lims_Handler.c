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
* @file lims_Handler.c
* @brief Implementation of all the callbacks received from the lower components.
*/

#include "lims.h"
#include "lims_internal.h"

/**
* This function handles the registration, subscriptions and notify from registration  module.
*/
void lims_sueStatusCallbackHandler
(
	void *pCallbackData,
	void *pNotificationData
)
{
	lims_moduleStruct *m = (lims_moduleStruct *)pCallbackData;
	EcrioSUENotifyStruct *pSueNotifyStruct = NULL;
	lims_NotifyTypeEnums notifyType = lims_Notify_Type_NONE;

	if (m == NULL)
	{
		return;
	}

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pNotificationData == NULL)
	{
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tpNotificationData is NULL",
			__FUNCTION__, __LINE__);
		goto ERR_None;
	}

	pSueNotifyStruct = (EcrioSUENotifyStruct *)pNotificationData;

	switch (pSueNotifyStruct->eNotificationType)
	{
		case ECRIO_SUE_NOTIFICATION_ENUM_RegisterResponse:
		{
			EcrioSUENotifyRegisterResponseStruct *pRegRspStruct = (EcrioSUENotifyRegisterResponseStruct *)pSueNotifyStruct->u.pRegisterResponse;
			if (pRegRspStruct->uRspCode == 200)
			{
				if (m->moduleState == lims_Module_State_CONNECTED)
				{
					LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tREGISTRATION SUCCESSFUL", __FUNCTION__, __LINE__);
					notifyType = lims_Notify_Type_REGISTERED;
					m->moduleState = lims_Module_State_REGISTERED;
					LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tChange State from CONNECTED to REGISTERED", __FUNCTION__, __LINE__);

					m->pCallback.pLimsCallback(lims_CallbackType_Status, notifyType, NULL, m->pCallback.pContext);
				}
				else if (m->moduleState == lims_Module_State_REGISTERED)
				{
					notifyType = lims_Notify_Type_REGISTERED;
					LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tChange State from REGISTERED to REGISTERED", __FUNCTION__, __LINE__);

					m->pCallback.pLimsCallback(lims_CallbackType_Status, notifyType, NULL, m->pCallback.pContext);
				}
#if defined(ENABLE_RCS)
#if 0
				// Sending Auto initiating PUBLISH
				if (m->pConfig->bCapabilityInfo && m->uFeatures > 0)
				{
					u_int32 uSmsError = ECRIO_CPM_NO_ERROR;
					uSmsError = EcrioCPMPublishCapabilityInfo(m->pCpmHandle, m->uFeatures, EcrioCPM_Publish_Initiate);
					if (uSmsError != ECRIO_CPM_NO_ERROR)
					{
						LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t EcrioSmsSendMessage() failed with error:%d",
							__FUNCTION__, __LINE__, uSmsError);
						goto ERR_None;
					}
				}
#endif
#endif //ENABLE_RCS
			}
			else if (pRegRspStruct->uRspCode == 401)
			{
				LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tREGISTRATION UNAUTHORIZED",
					__FUNCTION__, __LINE__);
				goto ERR_None;
			}
			else
			{
				LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tREGISTRATION UNSUCCESSFUL with rspCode:%d",
					__FUNCTION__, __LINE__, pRegRspStruct->uRspCode);

				m->moduleState = lims_Module_State_CONNECTED;
				LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tChange State from REGISTERING to CONNECTED", __FUNCTION__, __LINE__);
				notifyType = lims_Notify_Type_REGISTER_FAILED;
				m->pCallback.pLimsCallback(lims_CallbackType_Status, notifyType, NULL, m->pCallback.pContext);
			}
		}
		break;

		case ECRIO_SUE_NOTIFICATION_ENUM_DeregisterResponse:
		{
			EcrioSUENotifyRegisterResponseStruct *pDeRegRspStruct = (EcrioSUENotifyRegisterResponseStruct *)pSueNotifyStruct->u.pDeregisterResponse;

			m->moduleState = lims_Module_State_CONNECTED;
			notifyType = lims_Notify_Type_DEREGISTERED;

			if (pDeRegRspStruct->uRspCode == 200)
			{
				LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tDEREGISTRATION SUCCESSFUL",
					__FUNCTION__, __LINE__);
#if defined(ENABLE_RCS)
#if 0
				if (m->pConfig->bCapabilityInfo)
				{
					u_int32 uSmsError = ECRIO_CPM_NO_ERROR;
					// Sending Auto removing PUBLISH
					uSmsError = EcrioCPMPublishCapabilityInfo(m->pCpmHandle, m->uFeatures, EcrioCPM_Publish_Remove);
					if (uSmsError != ECRIO_CPM_NO_ERROR)
					{
						LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t EcrioSmsSendMessage() failed with error:%d",
							__FUNCTION__, __LINE__, uSmsError);
						goto ERR_None;
					}
				}
#endif
#endif //ENABLE_RCS

			}
			else
			{
				LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tDEREGISTRATION UNSUCCESSFUL with rspCode:%d",
					__FUNCTION__, __LINE__, pDeRegRspStruct->uRspCode);
			}

			m->pCallback.pLimsCallback(lims_CallbackType_Status, notifyType, pDeRegRspStruct, m->pCallback.pContext);
		}
		break;

		case ECRIO_SUE_NOTIFICATION_ENUM_SubscribeResponse:
		{
			EcrioSUENotifySubscribeResponseStruct *pSubRspStruct = (EcrioSUENotifySubscribeResponseStruct *)pSueNotifyStruct->u.pSubscribeResponse;
			if (pSubRspStruct->uRspCode == 200)
			{
				LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tSUBSCRIPTION SUCCESSFUL",
					__FUNCTION__, __LINE__);
				if (m->bSubscribed == Enum_FALSE && pSubRspStruct->uExpire > 0)
				{
					notifyType = lims_Notify_Type_SUBSCRIBED;
					m->bSubscribed = Enum_TRUE;
				}
				else if (pSubRspStruct->uExpire == 0)
				{
					notifyType = lims_Notify_Type_UNSUBSCRIBED;
					m->bSubscribed = Enum_FALSE;
				}

				m->pCallback.pLimsCallback(lims_CallbackType_Status, notifyType, NULL, m->pCallback.pContext);
			}
		}
		break;

		case ECRIO_SUE_NOTIFICATION_ENUM_NotifyRequest:
		{
			EcrioSUENotifyNotifyRequestStruct *pNotifReqStruct = (EcrioSUENotifyNotifyRequestStruct *)pSueNotifyStruct->u.pNotifyRequest;

			if (pNotifReqStruct == NULL)
			{
				LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\t INVALID REG-INFO",
					__FUNCTION__, __LINE__);
				break;
			}

			notifyType = lims_Notify_Type_NOTIFY_RECEIVED;
			if (pNotifReqStruct->eRegInfoContactState == ECRIO_SUE_REGINFO_CONTACT_STATE_ENUM_Active)
			{
				LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tCONTACT REGISTRATION IS ACTIVE",
					__FUNCTION__, __LINE__);
			}
			else if (pNotifReqStruct->eRegInfoContactState == ECRIO_SUE_REGINFO_CONTACT_STATE_ENUM_Terminated)
			{
				LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tCONTACT REGISTRATION IS TERMINATED",
					__FUNCTION__, __LINE__);
			}

			m->pCallback.pLimsCallback(lims_CallbackType_Status, notifyType, pNotifReqStruct, m->pCallback.pContext);
		}
		break;

		case ECRIO_SUE_NOTIFICATION_ENUM_ExitSUE:
		{
			EcrioSUEExitReasonEnum reason = (EcrioSUEExitReasonEnum)pSueNotifyStruct->u.eExitReason;

			m->moduleState = lims_Module_State_CONNECTED;
			if (reason == ECRIO_SUE_EXIT_REASON_ENUM_RegistrationFailed)
			{
				m->moduleState = lims_Module_State_INITIALIZED;
				LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tExit SUE! Registration Failed",
					__FUNCTION__, __LINE__);
				notifyType = lims_Notify_Type_SOCKET_ERROR;
			}
			else if (reason == ECRIO_SUE_EXIT_REASON_ENUM_Deregistered)
			{
				LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tExit SUE! Registration Failed",
					__FUNCTION__, __LINE__);
				notifyType = lims_Notify_Type_DEREGISTERED;
			}
			else if (reason == ECRIO_SUE_EXIT_REASON_ENUM_SubscriptionFailed)
			{
				LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\t Subscription Failed",
					__FUNCTION__, __LINE__);
				notifyType = lims_Notify_Type_SUBSCRIBE_FAILED;
			}

			m->pCallback.pLimsCallback(lims_CallbackType_Status, notifyType, NULL, m->pCallback.pContext);
		}
		break;

		case ECRIO_SUE_NOTIFICATION_ENUM_SocketError:
		{
			m->moduleState = lims_Module_State_INITIALIZED;
			notifyType = lims_Notify_Type_SOCKET_ERROR;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, notifyType, NULL, m->pCallback.pContext);
		}
		break;

		case ECRIO_SUE_NOTIFICATION_ENUM_EstablishIPSECSA:
		{
			m->moduleState = lims_Module_State_CONNECTED;
			notifyType = lims_Notify_Type_ESTABLISH_IPSEC_SA;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, notifyType, pSueNotifyStruct->u.pIPSecNotification, m->pCallback.pContext);
		}
		break;

		default:
		{
			goto ERR_None;
		}
	}

ERR_None:
	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}

/**
*
*/
#ifdef ENABLE_RCS
void lims_cpmStatusCallbackHandler
(
	void *pCallbackData,
	EcrioCPMNotifStruct *pCPMNotifStruct
)
{
	lims_moduleStruct *m = NULL;

	if (pCallbackData == NULL)
	{
		return;
	}

	m = (lims_moduleStruct *)pCallbackData;
	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

#if defined(ENABLE_RCS)
	if (pCPMNotifStruct == NULL)
	{
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tpCPMNotifStruct is NULL",
			__FUNCTION__, __LINE__);
		return;
	}

	LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u CPM Notification Command: %u.", __FUNCTION__, __LINE__, pCPMNotifStruct->eNotifCmd)

	switch (pCPMNotifStruct->eNotifCmd)
	{
		case EcrioCPM_Notif_PublishSendSuccess:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_CAPABILITIES_PUBLISHED, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;
		case EcrioCPM_Notif_PublishSendFailure:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_CAPABILITIES_PUBLISH_FAILED, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;
		case EcrioCPM_Notif_PagerMessageSendSuccess:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_STAND_ALONE_MESSAGE_SENT, pCPMNotifStruct->pNotifData, m->pCallback.pContext);

		}
		break;

		case EcrioCPM_Notif_PagerMessageSendFailure:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_STAND_ALONE_MESSAGE_FAILED, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;
		
		case EcrioCPM_Notif_PagerMessageReceived:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_STAND_ALONE_MESSAGE_RECEIVED, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;

		case EcrioCPM_Notif_IncomingSession:
		{
			//m->sessionState = lims_Session_State_INCOMING;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_SESSION_INCOMING, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;

		case EcrioCPM_Notif_SessionRinging:
		{
			//m->sessionState = lims_Session_State_ESTABLISHED;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_SESSION_RINGING, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;

		case EcrioCPM_Notif_SessionProgress:
		{
			//m->sessionState = lims_Session_State_ESTABLISHED;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_SESSION_PROGRESS, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;

		case EcrioCPM_Notif_SessionEstablished:
		{
			//m->sessionState = lims_Session_State_ESTABLISHED;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_SESSION_ESTABLISHED, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;

		case EcrioCPM_Notif_SessionConnectFailure:
		{
			//m->sessionState = lims_Session_State_IDLE;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_SESSION_CONNECT_FAILED, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;

		case EcrioCPM_Notif_SessionEnded:
		{
			//m->sessionState = lims_Session_State_IDLE;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_SESSION_ENDED, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;
		case EcrioCPM_Notif_SessionMessageSendSuccess:
		{
			//m->sessionState = lims_Session_State_IDLE;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_MESSAGE_SENT, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;
		case EcrioCPM_Notif_SessionMessageSendFailure:
		{
			//m->sessionState = lims_Session_State_IDLE;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_MESSAGE_FAILED, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;
		case EcrioCPM_Notif_IncomingSessionMessage:
		{
			//m->sessionState = lims_Session_State_IDLE;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_MESSAGE_RECEIVED, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;
		case EcrioCPM_Notif_IMDNDisposition:
		{
			//m->sessionState = lims_Session_State_IDLE;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_IMDN, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;
		case EcrioCPM_Notif_IMComposing:
		{
			//m->sessionState = lims_Session_State_IDLE;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_COMPOSING, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;
		case EcrioCPM_Notif_RichCardTransferSuccess:
		{
			//m->sessionState = lims_Session_State_IDLE;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_RICHCARD_SENT, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;
		case EcrioCPM_Notif_RichCardTransferFailure:
		{
			//m->sessionState = lims_Session_State_IDLE;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_RICHCARD_FAILED, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;
		case EcrioCPM_Notif_RichCardReceived:
		{
			//m->sessionState = lims_Session_State_IDLE;
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_RICHCARD_RECEIVED, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;

		case EcrioCPM_Notif_SubscribeSendSuccess:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_SUBSCRIBE_PARTICIPANTINFO_SUCCESS, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;

		case EcrioCPM_Notif_SubscribeSendFailure:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_SUBSCRIBE_PARTICIPANTINFO_FAILED, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;

		case EcrioCPM_Notif_UnSubscribeSendSuccess:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_UNSUBSCRIBE_PARTICIPANTINFO_SUCCESS, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;

		case EcrioCPM_Notif_UnSubscribeSendFailure:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_UNSUBSCRIBE_PARTICIPANTINFO_FAILED, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;

		case EcrioCPM_Notif_ReferSendSuccess:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_UPDATE_PARTICIPANT_SUCCESS, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;

		case EcrioCPM_Notif_ReferSendFailure:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_UPDATE_PARTICIPANT_FAILED, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;

		case EcrioCPM_Notif_ModifyGroupChatSuccess:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_MODIFY_GROUPCHAT_SUCCESS, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;

		case EcrioCPM_Notif_ModifyGroupChatFailure:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_MODIFY_GROUPCHAT_FAILED, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;

		case EcrioCPM_Notif_GroupChatInfo:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_GROUPCHAT_INFO, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;

		case EcrioCPM_Notif_GroupDataStatus:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_GROUPDATA_INFO, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;

		case EcrioCPM_Notif_PrivacyManagementCommands:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_PRIVACY_MANAGEMENT_RESPONSE, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;

		case EcrioCPM_Notif_GroupChatIcon:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_GROUPCHAT_ICON, pCPMNotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;

		default:
		break;
	}
#else
	LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u CPM feature disabled.", __FUNCTION__, __LINE__)
	pCPMNotifStruct = pCPMNotifStruct;
#endif

	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}

#endif


#if 0
void lims_UCEStatusCallbackHandler
(
void *pCallbackData,
EcrioUCENotifStruct *pUCENotifStruct
)
{
	lims_moduleStruct *m = NULL;

	if (pCallbackData == NULL)
	{
		return;
	}

	m = (lims_moduleStruct *)pCallbackData;
	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

#if defined(ENABLE_RCS)
	if (pUCENotifStruct == NULL)
	{
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tpUCENotifStruct is NULL",
			__FUNCTION__, __LINE__);
		return;
	}

	LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u UCE Notification Command: %u.", __FUNCTION__, __LINE__, pUCENotifStruct->eNotifCmd)

		switch (pUCENotifStruct->eNotifCmd)
	{
		case EcrioUCE_Notif_PublishSendSuccess:
		{
			if (pUCENotifStruct->pNotifData)
				m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_CAPABILITIES_PUBLISHED, pUCENotifStruct->pNotifData, m->pCallback.pContext);
			else
				m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_CAPABILITIES_REMOVED, pUCENotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;
		case EcrioUCE_Notif_PublishSendFailure:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_CAPABILITIES_PUBLISH_FAILED, pUCENotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;
		case EcrioUCE_Notif_SubscribeSendSuccess:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_QUERY_CAPABILITIES_SUCCESS, pUCENotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;
		case EcrioUCE_Notif_SubscribeSendFailure:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_QUERY_CAPABILITIES_FAILED, pUCENotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;
		case EcrioUCE_Notif_QueryCapabilities:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_QUERY_CAPABILITIES_DISCOVERED, pUCENotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;
		case EcrioUCE_Notif_OptionsSendSuccess:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_QUERY_CAPABILITIES_SUCCESS, pUCENotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;
		case EcrioUCE_Notif_OptionsSendFailure:
		{
			m->pCallback.pLimsCallback(lims_CallbackType_Status, lims_Notify_Type_QUERY_CAPABILITIES_FAILED, pUCENotifStruct->pNotifData, m->pCallback.pContext);
		}
		break;

		default:
			break;
	}
#else
	LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u UCE feature disabled.", __FUNCTION__, __LINE__)
		pUCENotifStruct = pUCENotifStruct;
#endif

	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}


void lims_UCEGetPropertyCallback
(
	EcrioUCEPropertyNameEnums eName,
	void *pData,
	void *pContext
)
{
	lims_moduleStruct *m = NULL;

	if (pContext == NULL)
	{
		return;
	}
	m = (lims_moduleStruct *)pContext;
	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	LIMSLOGI(m->logHandle, KLogTypeGeneral, "%s:%u\tUCE Get Property Command: %d", __FUNCTION__, __LINE__, (u_int32)eName)
	if (m->pCallback.pUCEPropertyCallback != NULL)
		m->pCallback.pUCEPropertyCallback(eName, pData, m->pCallback.pContext);
	else
	{
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tpUCEPropertyCallback() is NULL", __FUNCTION__, __LINE__, (u_int32)eName)
	}

	
	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
	return;
}
#endif

