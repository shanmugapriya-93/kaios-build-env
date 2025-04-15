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
 * @file iota_test.cpp
 * @brief The iota Main Test App primary test code implementation.
 */

#include <cstdlib>
#include <cstdio>
#include <cstring>
//#include <fstream>
#include "../src/external/yxml/yxml.h"

#include "EcrioPAL.h"
#include "lims.h"

#include "iota_test.h"
#include "iota_test_menu.h" 
extern iotaTestStateStruct iotaState;

#ifdef ENABLE_RCS
extern class FileTransfer ft;
#endif // ENABLE_RCS

static iotaTestPushLocationStruct gPushLocation =
{
	(u_char *)"tel:+1234578901",
	(u_char *)"a1233",
	(u_char *)"meeting location",
	-300,
	(u_char *)"2012-03-15T21:00:00-05:00",
	(u_char *)"urn:ogc:def:crs:EPSG::4326",
	(u_char *)"26.1181289 -80.1283921",
	(u_char *)"urn:ogc:def:uom:EPSG::9001",
	10,
	(u_char *)"2012-03-15T21:00:00-05:00",
	(u_char *)"2012-03-15T16:09:44-05:00"
};


#ifdef WIN32
/* Definitions. */
#define SIGNAL_VERSION_REVISION					1	/**< The revision of this implementation, ever incrementing. */
#define SIGNAL_SEND_EVENT						(WM_USER+1)	/**< Message used to indicate that a signal should be sent. */
#define SIGNAL_EXIT_EVENT						(WM_USER+2)	/**< Message used to indicate that the Signal Thread should exit. */

static unsigned int WINAPI IPSECThread
(
	LPVOID lpParam
);

#else

#include <iostream>
#include <list>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <csignal>
#include <cstdlib>
#include <cstdio>
#include <cstring>
using namespace std;

#define IOTA_TEST_SIGNAL_EXIT 0
#define IOTA_TEST_IPSEC_ESTABLISHED 1

/* messageQ message Structure */
typedef struct
{
	unsigned int uCommand;
	unsigned int uParam;
	void* pParam;
} iota_test_messageStruct;

/* messageQ structure */
typedef struct
{
	mutex m;
	list <iota_test_messageStruct> messageQ;
} iota_test_messageQStruct;

static unsigned int iota_test_IPSECThread
(
IOTA_TEST_SIGNAL_HANDLE handle,
unsigned int command,
unsigned int uParam,
void* pParam
);

typedef unsigned int(*iota_test_signalCallback)
(
	IOTA_TEST_SIGNAL_HANDLE handle,
	unsigned int command,
	unsigned int uParam,
	void* pParam
);

typedef struct
{
	mutex m;
	condition_variable cond;
	thread hThread;
	iota_test_messageQStruct messageQStruct;
	iota_test_signalCallback callback;
	unsigned int isEnding;
	LOGHANDLE logHandle;
} iota_test_signalStruct;

/* MESSAGE QUEUE IMPLEMENTATION - START */
void iota_test_message_init
(
	iota_test_messageQStruct *messageQStruct
)
{
	(void)messageQStruct;
	//TBD
}

void iota_test_message_deinit
(
	iota_test_messageQStruct *messageQStruct
)
{
	(void)messageQStruct;
	//TBD
}

int iota_test_message_add
(
	iota_test_messageQStruct *messageQStruct,
	unsigned int uCommand,
	unsigned int uParam,
	void* pParam
)
{
	lock_guard<mutex> lock(messageQStruct->m);
	iota_test_messageStruct messageStruct = {};
	messageStruct.uCommand = uCommand;
	messageStruct.uParam = uParam;
	messageStruct.pParam = pParam;
	messageQStruct->messageQ.push_back(messageStruct);
	return 0;
}

int iota_test_message_get
(
	iota_test_messageQStruct *messageQStruct,
	iota_test_messageStruct *pStruct
)
{
	unique_lock<mutex> lock(messageQStruct->m);
	if (messageQStruct->messageQ.size() == 0)
	{
		lock.unlock();
		return -1;
	}
	iota_test_messageStruct messageStruct = {};
	messageStruct = messageQStruct->messageQ.front();
	pStruct->uCommand = messageStruct.uCommand;
	pStruct->uParam = messageStruct.uParam;
	pStruct->pParam = messageStruct.pParam;
	messageQStruct->messageQ.pop_front();
	lock.unlock();
	return 0;
}
/* MESSAGE QUEUE IMPLEMENTATION - END */

/* SIGNAL IMPLEMENTATION - START */

static void iota_test_signal_worker
(
	void *pStruct
)
{
	iota_test_signalStruct *h = (iota_test_signalStruct*)pStruct;
	while (1)
	{
		unique_lock<mutex> lock(h->m);
		h->cond.wait(lock);
		lock.unlock();
		iota_test_messageStruct messageStruct = {};
		while (iota_test_message_get(&(h->messageQStruct), &messageStruct) == 0)
		{
			switch (messageStruct.uCommand)
			{
			case IOTA_TEST_SIGNAL_EXIT:
			{
				return;
			}
			default:
			{
				if ((h->callback != NULL) && (!h->isEnding))
				{
					h->callback(h, messageStruct.uCommand, messageStruct.uParam, messageStruct.pParam);
				}
			}
			break;
			}
		}
	}
}

IOTA_TEST_SIGNAL_HANDLE iota_test_signal_init
(
	iota_test_signalCallback pSignalFn,
	LOGHANDLE logHandle,
	int *pError
)
{
	iota_test_signalStruct *h = NULL;
	int returnValue = 0;

	if (pError == NULL)
	{
		return NULL;
	}

	if (pSignalFn == NULL)
	{
		*pError = -1;
		return NULL;
	}

	/* By default, the error value returned will indicate success. */
	*pError = returnValue;

	h = new (iota_test_signalStruct);

	h->logHandle = logHandle;
	h->isEnding = 0;

	h->callback = pSignalFn;

	iota_test_message_init(&(h->messageQStruct));

	h->hThread = thread(iota_test_signal_worker, (void *)h);
	return h;
}

int iota_test_signal_deinit
(
IOTA_TEST_SIGNAL_HANDLE *handle
)
{
	iota_test_signalStruct *h = NULL;
	if (handle == NULL)
	{
		return -1;
	}
	if (*handle == NULL)
	{
		return -1;
	}
	h = (iota_test_signalStruct*)*handle;
	h->isEnding = 1;
	iota_test_message_add(&(h->messageQStruct), IOTA_TEST_SIGNAL_EXIT, 0, NULL);
	unique_lock<mutex> lock(h->m);
	h->cond.notify_one();
	lock.unlock();
	h->hThread.join();
	iota_test_message_deinit(&(h->messageQStruct));
	delete h;

	return 0;
}

int iota_test_signal_send
(
IOTA_TEST_SIGNAL_HANDLE handle,
unsigned int command,
unsigned int uParam,
void* pParam
)
{
	iota_test_signalStruct *h = NULL;
	if (handle == NULL)
	{
		return -1;
	}
	h = (iota_test_signalStruct*)handle;
	iota_test_message_add(&(h->messageQStruct), command, uParam, pParam);
	unique_lock<mutex> lock(h->m);
	h->cond.notify_one();
	lock.unlock();
	return 0;
}
#endif
/* Define your callback handlers here. */

u_int32 limsGetPropertyCallback
(
	EcrioSipPropertyNameEnums eName,
	EcrioSipPropertyTypeEnums eType,
	void *pData,
	void *pContext
)
{
	// All the values needs to retrieve from
	u_int32 error = 0;
	char tempBuf[13];
	u_int len = 0;

	memset(tempBuf, 0, 13);

	(void)pContext;

	switch (eName)
	{
		case EcrioSipPropertyName_ACCESSTYPE:
		{
			/* Value will be provided to the framework depends on the current connected network. If the
			  current connected network is LTE. It will provide either 3GPP-E-UTRAN-FDD or 3GPP-E-UTRAN-TDD.
			  Refer 3GPP 24.229 section 7.2A.4 for more details. */

			if (eType == EcrioSipPropertyType_STRING)
			{
				u_char *pAccessType = (u_char *)pData;
				if (pAccessType != NULL)
				{
					len = pal_StringLength((const u_char *)iotaState.paniType);
					if (len != 0)
					{
						if (NULL == pal_StringNCopy(pAccessType, len + 1, (const u_char *)iotaState.paniType, len))
						{
							return 1;
						}
					}
					else
					{
						len = pal_StringLength((const u_char *)IOTA_TEST_ACCESSTYPE);
						if (NULL == pal_StringNCopy(pAccessType, len + 1, (const u_char *)IOTA_TEST_ACCESSTYPE, len))
						{
							return 1;
						}
					}
				}
			}
		}
		break;

		case EcrioSipPropertyName_ACCESSINFO_VALUE:
		{
			/* This value will be generated based on the access-type. If the access type field is equal to "3GPP-UTRAN-FDD", or
			   "3GPP-UTRAN-TDD", a "utran-cell-id-3gpp" parameter set to a concatenation of the MCC, MNC, LAC
			   (as described in 3GPP TS 23.003 [3]) and the UMTS Cell Identity (as described in 3GPP TS 25.331 [9A]),
			   obtained from lower layers of the UE, and is coded as a text string as follows:

			   Starting with the most significant bit, MCC (3 digits), MNC (2 or 3 digits depending on MCC value),
			   LAC (fixed length code of 16 bits using full hexadecimal representation) and UMTS Cell Identity
			   (fixed length code of 28 bits using a full hexadecimal representation); */

			if (eType == EcrioSipPropertyType_STRING)
			{
				u_char *pAccessInfo = (u_char *)pData;
				if (pAccessInfo != NULL)
				{
					len = pal_StringLength((const u_char *)iotaState.paniInfo);
					if (len != 0)
					{
						if (NULL == pal_StringNCopy(pAccessInfo, len + 1, (const u_char *)iotaState.paniInfo, len))
						{
							return 1;
						}
					}
					else
					{
						len = pal_StringLength((const u_char *)IOTA_TEST_ACCESSINFO_VALUE);
						if (NULL == pal_StringNCopy(pAccessInfo, len + 1, (const u_char *)IOTA_TEST_ACCESSINFO_VALUE, len))
						{
							return 1;
						}
					}
				}
			}
		}
		break;
		case EcrioSipPropertyName_LAST_ACCESSTYPE:
		{
			/* Value will be provided to the framework depends on the current connected network. If the
			current connected network is LTE. It will provide either 3GPP-E-UTRAN-FDD or 3GPP-E-UTRAN-TDD.
			Refer 3GPP 24.229 section 7.2A.4 for more details. */

			if (eType == EcrioSipPropertyType_STRING)
			{
				u_char *pLastAccessType = (u_char *)pData;
				if (pLastAccessType != NULL)
				{
					len = pal_StringLength((const u_char *)iotaState.planiType);
					if (len != 0)
					{						
						if (NULL == pal_StringNCopy(pLastAccessType, len + 1, (const u_char *)iotaState.planiType, len))
						{
							return 1;
						}
					}
					else
					{
						len = pal_StringLength((const u_char *)IOTA_TEST_LAST_ACCESSTYPE);					
						if (NULL == pal_StringNCopy(pLastAccessType, len + 1, (const u_char *)IOTA_TEST_LAST_ACCESSTYPE, len))
						{
							return 1;
						}
					}
				}
				// *pLastAccessType = NULL;
			}
		}
		break;
		case EcrioSipPropertyName_LAST_ACCESSINFO_VALUE:
		{
			/* This value will be generated based on the access-type. If the access type field is equal to "3GPP-UTRAN-FDD", or
			"3GPP-UTRAN-TDD", a "utran-cell-id-3gpp" parameter set to a concatenation of the MCC, MNC, LAC
			(as described in 3GPP TS 23.003 [3]) and the UMTS Cell Identity (as described in 3GPP TS 25.331 [9A]),
			obtained from lower layers of the UE, and is coded as a text string as follows:

			Starting with the most significant bit, MCC (3 digits), MNC (2 or 3 digits depending on MCC value),
			LAC (fixed length code of 16 bits using full hexadecimal representation) and UMTS Cell Identity
			(fixed length code of 28 bits using a full hexadecimal representation); */

			if (eType == EcrioSipPropertyType_STRING)
			{
				u_char *pLastAccessInfo = (u_char *)pData;
				if (pLastAccessInfo != NULL)
				{
					len = pal_StringLength((const u_char *)iotaState.planiInfo);
					if (len != 0)
					{
						if (NULL == pal_StringNCopy(pLastAccessInfo, len + 1, (const u_char *)iotaState.planiInfo, len))
						{
							return 1;
						}
					}
					else
					{
						len = pal_StringLength((const u_char *)IOTA_TEST_LAST_ACCESSINFO_VALUE);
						if (NULL == pal_StringNCopy(pLastAccessInfo, len + 1, (const u_char *)IOTA_TEST_LAST_ACCESSINFO_VALUE, len))
						{
							return 1;
						}
					}
				}
				 // *pLastAccessInfo = NULL;
			}
		}
		break;

		case EcrioSipPropertyName_AKAV1:
		{
			/* eType should be lims_PropertyType_AUTH else it will return error. */
			if (eType == EcrioSipPropertyType_AUTH)
			{
				EcrioSipAuthStruct *pAuthStruct = (EcrioSipAuthStruct *)pData;
				if (pAuthStruct != NULL)
				{
					if ((pAuthStruct->pAutn != NULL) && (pAuthStruct->pRand != NULL) && (pAuthStruct->uAutnSize > 0) && (pAuthStruct->uRandSize > 0))
					{
						pAuthStruct->uIKSize = pal_StringLength((const u_char *)"6543210987654321");
						pAuthStruct->pIK = pal_StringCreate((const u_char *)"6543210987654321", pAuthStruct->uIKSize);
						pAuthStruct->uCKSize = pal_StringLength((const u_char *)"1234567890123456");
						pAuthStruct->pCK = pal_StringCreate((const u_char *)"1234567890123456", pAuthStruct->uCKSize);
						pAuthStruct->uResSize = pal_StringLength((const u_char *)"1234567890");
						pAuthStruct->pRes = pal_StringCreate((const u_char *)"1234567890", pAuthStruct->uResSize);
					}
					else
					{
						// TODO error handling here
					}
				}
			}
			else
			{
				// TODO error handling here
			}
		}
		break;

		case EcrioSipPropertyName_AKAV2:
		{
			/* eType should be lims_PropertyType_AUTH else it will return error. */
			if (eType == EcrioSipPropertyType_AUTH)
			{
				EcrioSipAuthStruct *pAuthStruct = (EcrioSipAuthStruct *)pData;
				if (pAuthStruct != NULL)
				{
					if ((pAuthStruct->pAutn != NULL) && (pAuthStruct->pRand != NULL) && (pAuthStruct->uAutnSize > 0) && (pAuthStruct->uRandSize > 0))
					{
						pAuthStruct->pIK = NULL;
						pAuthStruct->uIKSize = 0;
						pAuthStruct->pCK = NULL;
						pAuthStruct->uCKSize = 0;
						pAuthStruct->uResSize = pal_StringLength((const u_char *)"ab57e2a47ab9aa617737cd7229f27eb5");
						pAuthStruct->pRes = pal_StringCreate((const u_char *)"ab57e2a47ab9aa617737cd7229f27eb5", pAuthStruct->uResSize);
					}
					else
					{
						// TODO error handling here
					}
				}
			}
			else
			{
				// TODO error handling here
			}
		}
		break;
		case EcrioSipPropertyName_VOLTE_STATUS:
		{
		}
		break;
	}

	return error;
}

void handleStatusCallback
(
	lims_NotifyTypeEnums eNotifType,
	void *pData,
	void *pContext
)
{
	unsigned int error = LIMS_NO_ERROR;

	(void)pData;
	(void)pContext;

	pal_MutexLock(iotaState.mutexHandle);

	switch (eNotifType)
	{
		case lims_Notify_Type_REGISTERED:
		{
			iota_test_printf("REGISTERED\n");
			iotaState.bRegistered = true;
		}
		break; 
/*		case lims_Notify_Type_CAPABILITIES_PUBLISH_FAILED:
		{
			iota_test_printf("CAPABILITIES PUBLISH FAILED\n");
			EcrioUCEReasonStruct *pReason = (EcrioUCEReasonStruct *)pData;
			if (pReason){
				iota_test_printf("Response Code = %u\n", pReason->uReasonCause);
			}
		}
		break;*/
		case lims_Notify_Type_CAPABILITIES_PUBLISHED:
		{
			iota_test_printf("CAPABILITIES PUBLISHED!\n");
			iotaState.pETag = (char *)pData;
			iota_test_printf("E-Tag = %s\n", iotaState.pETag);
		}
		break; 
		case lims_Notify_Type_CAPABILITIES_REMOVED:
		{
			iota_test_printf("CAPABILITIES REMOVED\n");
			iotaState.pETag = NULL;
		}
		break; 
		case lims_Notify_Type_REGISTER_FAILED:
		{
			iota_test_printf("REGISTRATION FAILED\n");
			iotaState.bRegistered = false;
		}
		break;

		case lims_Notify_Type_DEREGISTERED:
		{
			EcrioSUENotifyRegisterResponseStruct *RegisterResponseStruct = (EcrioSUENotifyRegisterResponseStruct *)pData;
			if (RegisterResponseStruct != NULL)
				iota_test_printf("DEREGISTERED %u\n", RegisterResponseStruct->uRspCode);

			iotaState.bRegistered = false;
//			stop_timer();

			if (iotaState.bAirplaneModeOn)
			{
				iota_test_dettach_network();
				iotaState.bAirplaneModeOn = false;
			}
		}
		break;

		case lims_Notify_Type_SUBSCRIBED:
		{
			iota_test_printf("SUBSCRIBED to reg-events\n");
		}
		break;

		case lims_Notify_Type_SUBSCRIBE_FAILED:
		{
			iota_test_printf("SUBSCRIBED to reg-events failed\n");
		}
		break;

		case lims_Notify_Type_UNSUBSCRIBED:
		{
			iota_test_printf("UNSUBSCRIBED\n");
		}
		break;

		case lims_Notify_Type_NOTIFY_RECEIVED:
		{
			EcrioSUENotifyNotifyRequestStruct *pNotifReqStruct = (EcrioSUENotifyNotifyRequestStruct *)pData;
			
			if (pNotifReqStruct != NULL)
			{
				
				if (pNotifReqStruct->eRegInfoContactState == ECRIO_SUE_REGINFO_CONTACT_STATE_ENUM_Active)
				{
					iota_test_printf("ECRIO_SUE_REGINFO_CONTACT_STATE_ENUM_Active\n");
				}
				else if (pNotifReqStruct->eRegInfoContactState == ECRIO_SUE_REGINFO_CONTACT_STATE_ENUM_Terminated)
				{
					iota_test_printf("ECRIO_SUE_REGINFO_CONTACT_STATE_ENUM_Terminated\n");
				}
				
				iota_test_printf("contact event=%u\n", pNotifReqStruct->eRegInfoContactEvent);
				iota_test_printf("contact Expires=%u\n", pNotifReqStruct->uExpires);
				iota_test_printf("contact Retry After=%u\n", pNotifReqStruct->uRetryAfter);
			}
		}
		break;
		case lims_Notify_Type_SOCKET_ERROR:
		{
			iota_test_printf("Received Socket Error - Re-Initalize lims\n");
		}
		break;
#ifdef ENABLE_RCS
		case lims_Notify_Type_STAND_ALONE_MESSAGE_SENT:
		{
			EcrioCPMStandAloneMessageIDStruct *pStandAloneMessageID = (EcrioCPMStandAloneMessageIDStruct*)pData;

			iota_test_printf("StandAlone message sent!\n");
			iota_test_printf("Call Id- %s\n", pStandAloneMessageID->pCallId);
		}
		break;

		case lims_Notify_Type_STAND_ALONE_MESSAGE_FAILED:
		{
			EcrioCPMStandAloneMessageIDStruct *pStandAloneMessageID = (EcrioCPMStandAloneMessageIDStruct*)pData;

			iota_test_printf("StandAlone message sending failed!\n");
			iota_test_printf("statusCode: %d\n", pStandAloneMessageID->statusCode);
			iota_test_printf("Reason: %s\n", pStandAloneMessageID->pszReason);
			iota_test_printf("Call Id- %s\n", pStandAloneMessageID->pCallId);
			if (pStandAloneMessageID->uWarningCode != 0)
			{
				iota_test_printf("Warning Code is %d\n", pStandAloneMessageID->uWarningCode);
			}
			if (pStandAloneMessageID->pWarningHostName != NULL)
			{
				iota_test_printf("Warning Host Name is %s\n", pStandAloneMessageID->pWarningHostName);
			}
			if (pStandAloneMessageID->pWarningText != NULL)
			{
				iota_test_printf("Warning Text is %s\n", pStandAloneMessageID->pWarningText);
			}

			if (pStandAloneMessageID->ppPAssertedIdentity)
			{
				for (int i = 0; i < pStandAloneMessageID->uNumOfPAssertedIdentity; i++)
				{
					iota_test_printf("P-Asserted-Identity: %s\n", pStandAloneMessageID->ppPAssertedIdentity[i]);
				}
			}

		}
		break;

		case lims_Notify_Type_STAND_ALONE_MESSAGE_RECEIVED:
		{
			EcrioCPMIncomingStandAloneMessageStruct* pIncomingStandAloneMsg = NULL;

			pIncomingStandAloneMsg = (EcrioCPMIncomingStandAloneMessageStruct*)pData;
			
			iota_test_printf("StandAlone message received from %s with DisplayName as %s!\n", pIncomingStandAloneMsg->pReqFrom, pIncomingStandAloneMsg->pDisplayName);
			iota_test_printf("StandAlone message: %s\n", pIncomingStandAloneMsg->pMessage->message.pBuffer->pMessage);

			iota_test_printConversationHeaders(pIncomingStandAloneMsg->pConvId);
			
			iota_test_printf("imdn.Message-ID: %s\n", pIncomingStandAloneMsg->pMessage->pIMDNMsgId);

			iota_test_printIMDNDispositionNotification(pIncomingStandAloneMsg->pMessage->imdnConfig);

			iota_test_printf("bIsChatbot: %s\n", pIncomingStandAloneMsg->bIsChatbot == Enum_TRUE ? "TRUE" : "FALSE");
		}
		break;

		case lims_Notify_Type_SESSION_INCOMING:
		{
			EcrioCPMIncomingSessionStruct *pIncomingSession = (EcrioCPMIncomingSessionStruct *)pData;

			iotaState.pSessionId = (char*)pal_StringCreate((u_char*)pIncomingSession->pSessionId, pal_StringLength((u_char*)pIncomingSession->pSessionId));



			if (pIncomingSession->pConvId != NULL)
			{
				if (pIncomingSession->pConvId->pConversationId != NULL)
				{
					if (NULL == pal_StringNCopy(iotaState.convID, 64, pIncomingSession->pConvId->pConversationId, pal_StringLength(pIncomingSession->pConvId->pConversationId)))
					{
						iota_test_printf("Memory copy error.\n");
						break;
					}

					iotaState.convIDStruct.pConversationId = (u_char *)iotaState.convID;
				}

				if (pIncomingSession->pConvId->pContributionId != NULL)
				{
					if (NULL == pal_StringNCopy(iotaState.contID, 64, pIncomingSession->pConvId->pContributionId, pal_StringLength(pIncomingSession->pConvId->pContributionId)))
					{
						iota_test_printf("Memory copy error.\n");
						break;
					}
					iotaState.convIDStruct.pContributionId = (u_char *)iotaState.contID;
				}

				if (pIncomingSession->pConvId->pInReplyToContId != NULL)
				{
					if (NULL == pal_StringNCopy(iotaState.inReplyTo, 64, pIncomingSession->pConvId->pInReplyToContId, pal_StringLength(pIncomingSession->pConvId->pInReplyToContId)))
					{
						iota_test_printf("Memory copy error.\n");
						break;
					}
					iotaState.convIDStruct.pInReplyToContId = (u_char *)iotaState.inReplyTo;
				}
			}
			iota_test_printf("Incoming session request from: %s with DisplayName as %s\n", pIncomingSession->pReqFrom, pIncomingSession->pDisplayName);
			iota_test_printf("uSessionId %s\n", pIncomingSession->pSessionId);

			iota_test_printConversationHeaders(pIncomingSession->pConvId);
			if (pIncomingSession->pFile != NULL)
			{
				iota_test_printf("Sesion type: File Transfer - File Name:%s, File Size:%d, MIME Type:%s\n",
					pIncomingSession->pFile->pFileName, pIncomingSession->pFile->uFileSize, pIncomingSession->pFile->pFileType);
				iota_test_printf("File Transfer-ID: %s\n", pIncomingSession->pFile->pFileTransferId);

				iotaState.fileSize = pIncomingSession->pFile->uFileSize;

				if (NULL == pal_StringNCopy((unsigned char *)iotaState.fileName, 64, (unsigned char *)pIncomingSession->pFile->pFileName, pal_StringLength((unsigned char *)pIncomingSession->pFile->pFileName)))
				{
					iota_test_printf("Memory copy error.\n");
					break;
				}
				if (NULL == pal_StringNCopy((unsigned char *)iotaState.mimeType, 64, (unsigned char *)pIncomingSession->pFile->pFileType, pal_StringLength((unsigned char *)pIncomingSession->pFile->pFileType)))
				{
					iota_test_printf("Memory copy error.\n");
					break;
				}
				
				if (NULL == pal_StringNCopy((unsigned char *)iotaState.fileTransferId, 32, (unsigned char *)pIncomingSession->pFile->pFileTransferId, pal_StringLength((unsigned char *)pIncomingSession->pFile->pFileTransferId)))
				{
					iota_test_printf("Memory copy error.\n");
					break;
				}
				iota_test_printf("imdn.Message-ID: %s\n", pIncomingSession->pFile->pIMDNMsgId);
				iota_test_printf("Disposition Notification: ");
				if (pIncomingSession->pFile->imdnConfig == 0)
				{
					iota_test_printf("(None)\n");
				}
				else
				{
					if ((pIncomingSession->pFile->imdnConfig & EcrioCPMIMDispositionConfigPositiveDelivery) != 0)
					{
						iota_test_printf("positive-delivery ");
					}
					if ((pIncomingSession->pFile->imdnConfig & EcrioCPMIMDispositionConfigNegativeDelivery) != 0)
					{
						iota_test_printf("negative-delivery ");
					}
					if ((pIncomingSession->pFile->imdnConfig & EcrioCPMIMDispositionConfigDisplay) != 0)
					{
						iota_test_printf("display ");
					}
					iota_test_printf("\n");
				}
			}
			else
			{
				iota_test_printContentTypes(pIncomingSession->contentTypes);
			}
			
			iotaState.eContentTypes = pIncomingSession->contentTypes;

			iotaState.bIsGroupChat = pIncomingSession->bIsGroupChat;
			iotaState.bIsClosed = pIncomingSession->bIsClosed;
			iotaState.numofParticipants = 0;
			if (pIncomingSession->pList != NULL)
			{
				unsigned int i;
				for (i = 0; i < 4; i++)
				{
					pal_MemorySet(&iotaState.participants[i], 0, 128);
				}
				for (i = 0; i < pIncomingSession->pList->uNumOfUsers; i++)
				{
					if (NULL == pal_StringNCopy((unsigned char *)iotaState.participants[i], 128, (unsigned char *)pIncomingSession->pList->ppUri[i],
						pal_StringLength((unsigned char *)pIncomingSession->pList->ppUri[i])))
					{
						iota_test_printf("Memory copy error.\n");
						break;
					}
					iotaState.numofParticipants++;
					if (i == 3)
						break;
				}
			}

			if (pIncomingSession->bIsGroupChat == Enum_TRUE)
			{
				unsigned int i;






				if (pIncomingSession->bIsClosed == Enum_TRUE)
				{
					iota_test_printf("Group Chat Session - this is the closed group chat session \n");
				}
				else
				{
					iota_test_printf("Group Chat Session\n");
				}
				iota_test_printf("Number of participants: %d\n", iotaState.numofParticipants);
				if (iotaState.numofParticipants > 0 && pIncomingSession->pList)
				{
					for (i = 0; i < pIncomingSession->pList->uNumOfUsers; i++)
					{
						iota_test_printf("    uri: %s\n", iotaState.participants[i]);
					}
				}

				if (pIncomingSession->pGroupSessionId)
				{
					if (iotaState.pGroupSessionId)
					{
						pal_MemoryFree((void**)&iotaState.pGroupSessionId);
					}
					iotaState.pGroupSessionId = (char*)pal_StringCreate((u_char*)pIncomingSession->pGroupSessionId, pal_StringLength((u_char*)pIncomingSession->pGroupSessionId));
					iota_test_printf("Group Session ID: %s\n", iotaState.pGroupSessionId);
				}
			}

			if (pIncomingSession->bIsChatbot == Enum_TRUE)
			{
				iota_test_printf("Chatbot 1-to-1 Session\n");
				if (pIncomingSession->pAlias != NULL)
				{
					iota_test_printf("  tk param: ");
					switch (pIncomingSession->pAlias->eTkParam)
					{
						case EcrioCPMTkParam_On:
							iota_test_printf("on\n");
							break;
						case EcrioCPMTkParam_Off:
							iota_test_printf("off\n");
							break;
						case EcrioCPMTkParam_None:
						default:
							iota_test_printf("none\n");
							break;
					}
					iota_test_printf("  Aliason: %s\n", pIncomingSession->pAlias->bIsAliason == Enum_TRUE ? "TRUE" : "FALSE");
				}
			}
		}
		break;

		case lims_Notify_Type_SESSION_RINGING:
		{
			EcrioCPMAcceptedSessionStruct *pAcceptedSession = (EcrioCPMAcceptedSessionStruct *)pData;

			iota_test_printf("180 Ringing response received with DisplayName %s\n", pAcceptedSession->pDisplayName);
			iota_test_printf("uSessionId %s\n", pAcceptedSession->pSessionId);
		}
		break;

		case lims_Notify_Type_SESSION_PROGRESS:
		{
			EcrioCPMAcceptedSessionStruct *pAcceptedSession = (EcrioCPMAcceptedSessionStruct *)pData;

			iota_test_printf("183 Session in progress\n");
			iota_test_printf("uSessionId %s\n", pAcceptedSession->pSessionId);
		}
		break;

		case lims_Notify_Type_SESSION_ESTABLISHED:
		{
			EcrioCPMAcceptedSessionStruct *pAcceptedSession = (EcrioCPMAcceptedSessionStruct *)pData;

			iota_test_printf("Session Established with DisplayName %s\n", pAcceptedSession->pDisplayName);

			iota_test_printf("uSessionId %s\n", pAcceptedSession->pSessionId);


			if (iotaState.bGrAuto == Enum_TRUE){

				if (NULL == pal_StringNCopy((unsigned char *)iotaState.GrSessionId[iotaState.GrSessionCount], 128, 
					(unsigned char *)pAcceptedSession->pSessionId, pal_StringLength((unsigned char *)pAcceptedSession->pSessionId)))
				{
					iota_test_printf("Memory copy error.\n");
					break;
				}
				if (NULL == pal_StringNCopy((unsigned char *)iotaState.GrSessionIdGroup[iotaState.GrSessionCount], 128, 
					(unsigned char *)pAcceptedSession->pGroupSessionId, pal_StringLength((unsigned char *)pAcceptedSession->pGroupSessionId)))
				{
					iota_test_printf("Memory copy error.\n");
					break;
				}
				iotaState.GrSessionCount++;
			}


			iota_test_printConversationHeaders(pAcceptedSession->pConvId);

			iota_test_printContentTypes(pAcceptedSession->contentTypes);

			if (pAcceptedSession->pGroupSessionId)
			{
				if (iotaState.pGroupSessionId)
				{
					pal_MemoryFree((void**)&iotaState.pGroupSessionId);
				}
				iotaState.pGroupSessionId = (char*)pal_StringCreate((u_char*)pAcceptedSession->pGroupSessionId, pal_StringLength((u_char*)pAcceptedSession->pGroupSessionId));
				iota_test_printf("Group Session ID: %s\n", iotaState.pGroupSessionId);
			}

			if (pAcceptedSession->pAlias != NULL)
			{
				iota_test_printf("\n");
				iota_test_printf("Aliasing parameters:\n");
				iota_test_printf("  tk param: ");
				switch (pAcceptedSession->pAlias->eTkParam)
				{
					case EcrioCPMTkParam_On:
						iota_test_printf("on\n");
						break;
					case EcrioCPMTkParam_Off:
						iota_test_printf("off\n");
						break;
					case EcrioCPMTkParam_None:
					default:
						iota_test_printf("none\n");
						break;
				}
				iota_test_printf("  Aliason: %s\n", pAcceptedSession->pAlias->bIsAliason == Enum_TRUE ? "TRUE" : "FALSE");
			}

			if (pAcceptedSession->bIsChatbotRole == Enum_TRUE)
			{
				iota_test_printf("  Chatbot Role: TRUE\n");
			}

		}
		break;

		case lims_Notify_Type_SESSION_CONNECT_FAILED:
		{
			// @todo: end MSRP session
			iota_test_printf("Session Connect Failed\n");

			//here check if response code is 403 or 504, check for the pData action event and notify accordingly.
			EcrioCPMFailedSessionStruct *pFailData = (EcrioCPMFailedSessionStruct *)pData;
			if (pFailData != NULL)
			{
				if (pFailData->uResponseCode == 408)
				{
					iota_test_printf("Request Timeout!\n");
				}
				if (pFailData->uResponseCode == 403)
				{
					if (pFailData->eAction == EcrioCPMActionSendReRegister)
					{
						iota_test_printf("403 received with out warning header\n");
					}
					else
					{
						iota_test_printf("403 received with Warning header\n");
					}
				}

				if (pFailData->uResponseCode == 504)
				{
					if (pFailData->eAction == EcrioCPMActionSendInitialRegister)
					{
						iota_test_printf("504 received with xml, send initial register.\n");
					}
					else
					{
						iota_test_printf("504 received\n");
					}
				}

				if (pFailData->uResponseCode == 480)
				{
					iota_test_printf("Temporarily Unavailable\n");
				}

				if (pFailData->uWarningCode != 0)
				{
					iota_test_printf("Warning Code is %d\n", pFailData->uWarningCode);
				}
				if (pFailData->pWarningHostName != NULL)
				{
					iota_test_printf("Warning Host Name is %s\n", pFailData->pWarningHostName);
				}
				if (pFailData->pWarningText != NULL)
				{
					iota_test_printf("Warning Text is %s\n", pFailData->pWarningText);
				}

				if (pFailData->ppPAssertedIdentity)
				{
					for (int i = 0; i < pFailData->uNumOfPAssertedIdentity; i++)
					{
						iota_test_printf("P-Asserted-Identity: %s\n", pFailData->ppPAssertedIdentity[i]);
					}
				}
			}
			iotaState.convID[0] = '\0';
			iotaState.contID[0] = '\0';
			iotaState.inReplyTo[0] = '\0';
			iotaState.convIDStruct.pConversationId = NULL;
			iotaState.convIDStruct.pContributionId = NULL;
			iotaState.convIDStruct.pInReplyToContId = NULL;
			pal_MemoryFree((void**)&iotaState.pSessionId);
			pal_MemoryFree((void**)&iotaState.pGroupSessionId);
			pal_MemoryFree((void**)&iotaState.pReferId);
			iotaState.pSessionId = NULL;
			iotaState.pReferId = NULL;
			iotaState.fileName[0] = '\0';
			iotaState.fileSize = 0;
			iotaState.mimeType[0] = '\0';
			iotaState.fileTransferId[0] = '\0';
			iotaState.eContentTypes = 0;
			if (ft.inputFile)
			{
				ft.inputFile.close();
				ft.inputFile.clear();
			}
			if (ft.outputFile)
			{
				ft.outputFile.close();
				ft.outputFile.clear();
			}
		}
		break;

		case lims_Notify_Type_SESSION_ENDED:
		{
			EcrioCPMReasonStruct *pReason = (EcrioCPMReasonStruct *)pData;

			// @todo: end MSRP session
			iota_test_printf("Session Ended - session id:%s, cause code:%d, reason text:%s\n", pReason->pSessionId, pReason->uReasonCause, pReason->pReasonText);

			iotaState.convID[0] = '\0';
			iotaState.contID[0] = '\0';
			iotaState.inReplyTo[0] = '\0';
			iotaState.convIDStruct.pConversationId = NULL;
			iotaState.convIDStruct.pContributionId = NULL;
			iotaState.convIDStruct.pInReplyToContId = NULL;
			pal_MemoryFree((void**)&iotaState.pSessionId);
			pal_MemoryFree((void**)&iotaState.pGroupSessionId);
			pal_MemoryFree((void**)&iotaState.pReferId);
			iotaState.pSessionId = NULL;
			iotaState.pReferId = NULL;
			iotaState.fileName[0] = '\0';
			iotaState.fileSize = 0;
			iotaState.mimeType[0] = '\0';
			iotaState.fileTransferId[0] = '\0';
			iotaState.eContentTypes = 0;
			if (ft.inputFile)
			{
				ft.inputFile.close();
				ft.inputFile.clear();
			}
			if (ft.outputFile)
			{
				ft.outputFile.close();
				ft.outputFile.clear();
			}
			iotaState.sentSize = 0;
			iotaState.receivedSize = 0;
			iotaState.bIsEofReached = Enum_FALSE;
			iotaState.bIsFileSender = false;
			iotaState.bIsCPIMWrapped = Enum_FALSE;

			/*
			if (iotaState.bGrAuto == Enum_TRUE){

				for (int i = 0; i < iotaState.GrSessionCount;i++)
					pal_MemoryFree((void**)&iotaState.GrSessionId[iotaState.GrSessionCount]);
				
				
			}
			*/


		}
		break;

		case lims_Notify_Type_MESSAGE_SENT:
		{
			EcrioCPMSessionIDStruct* pSessionID = (EcrioCPMSessionIDStruct *)pData;
			iota_test_printf("Message sent, session id: %s, MSRP message id: %s, MSRP transaction id: %s \n", pSessionID->pSessionId, pSessionID->pMessageId, pSessionID->pTransactionId);

			if (iotaState.eContentTypes == EcrioCPMContentTypeEnum_FileTransferOverMSRP &&
				iotaState.bIsFileSender == true)
			{
				// @note: originally we don't need to use send_success notification as trigger for chunk sending, we should send chunk a continuous stream
				//		  but we don't have a thread feature in iota so we are obliged to implement this mechanism here.
				if (iotaState.bIsEofReached == Enum_FALSE)
				{
					char *pMessageId = NULL;
					char *pTransactionId = NULL;
					unsigned int readSize = 0;
					lims_DataStruct sendData;
					BoolEnum isCPIMWrapped = Enum_FALSE;

					memset(&sendData, 0, sizeof(lims_DataStruct));

					// send next chunk
					memset(iotaState.readBuf, 0, MSRP_CHUNK_SIZE);

					if (iotaState.fileSize < (iotaState.sentSize + MSRP_CHUNK_SIZE))
					{
						readSize = iotaState.fileSize - iotaState.sentSize;
						ft.inputFile.read((char *)iotaState.readBuf, readSize);
						sendData.bIsEofReached = Enum_TRUE;
						iotaState.bIsEofReached = Enum_TRUE;
					}
					else
					{
						readSize = MSRP_CHUNK_SIZE;
						ft.inputFile.read((char *)iotaState.readBuf, readSize);
						std::cout << ft.inputFile.gcount() << " characters read: " << '\n';
						if (iotaState.fileSize == (iotaState.sentSize + MSRP_CHUNK_SIZE))
						{
							sendData.bIsEofReached = Enum_TRUE;
							iotaState.bIsEofReached = Enum_TRUE;
						}
						else
						{
							sendData.bIsEofReached = Enum_FALSE;
						}
					}

					sendData.pSessionId = iotaState.pSessionId;
					sendData.pDataBuf = iotaState.readBuf;
					sendData.uByteRangeStart = iotaState.sentSize + 1;
					sendData.uByteRangeTotal = iotaState.fileSize;
					sendData.uByteRangeEnd = iotaState.sentSize + readSize;
					sendData.pContentType = iotaState.mimeType;

					iota_test_printf("Send file data - %d bytes, ByteRange: %d-%d/%d\n",
						readSize, sendData.uByteRangeStart, sendData.uByteRangeEnd, sendData.uByteRangeTotal);

					iota_test_printf("Calling lims_SendData()\n");

//					error = lims_SendData(iotaState.limsHandle, &sendData, &pMessageId, &pTransactionId, iotaState.bIsCPIMWrapped);
					if (error != LIMS_NO_ERROR)
					{
						iota_test_printf("lims_SendData() failed.\n");
						ft.inputFile.seekg(iotaState.sentSize);
					}
					else
					{
						iota_test_printf("lims_SendData() success.\n");
						iotaState.sentSize += readSize;
					}
					if (pMessageId)
					{
						iota_test_printf("Message Id: %s\n", pMessageId);
						pal_MemoryFree((void**)&pMessageId);
					}
					if (pTransactionId)
					{
						iota_test_printf("Transaction Id: %s\n", pTransactionId);
						pal_MemoryFree((void**)&pTransactionId);
					}
				}
				else
				{
					if (iotaState.fileSize == iotaState.sentSize)
					{
						iota_test_printf("Sent file successfully.\n");
					}
					else
					{
						iota_test_printf("Finished to send file.\n");
					}
					iotaState.bIsFileSender = false;
				}
			}
		}
		break;

		case lims_Notify_Type_MESSAGE_FAILED:
		{
			EcrioCPMSessionIDStruct* pSessionId = (EcrioCPMSessionIDStruct *)pData;
			iota_test_printf("Message sending failed, session id: %s\n", pSessionId->pSessionId);
			iota_test_printf("MSRP response, status code: %u\n", pSessionId->uStatusCode);
			iota_test_printf("MSRP response, description: %s\n", pSessionId->pDescription);

			if (iotaState.eContentTypes == EcrioCPMContentTypeEnum_FileTransferOverMSRP)
			{
				iota_test_printf("MSRP chunk sent failure, we need to close file transfer session...\n");
				iotaState.bIsEofReached = Enum_TRUE;
				iotaState.bIsFileSender = false;
				iotaState.bIsCPIMWrapped = Enum_FALSE;
			}
		}
		break;

		case lims_Notify_Type_MESSAGE_RECEIVED:
		{
			EcrioCPMSessionMessageStruct* pIncomingSessnMsg = (EcrioCPMSessionMessageStruct *)pData;

			if (pIncomingSessnMsg->pMessage)
			{
				if (pIncomingSessnMsg->pMessage->eContentType == EcrioCPMContentTypeEnum_FileTransferOverMSRP)
				{
					EcrioCPMBufferStruct *pBuffer = pIncomingSessnMsg->pMessage->message.pBuffer;

					if (pBuffer != NULL)
					{
						iota_test_printf("Received file data - %d bytes, ByteRange: %d-%d/%d\n",
							pBuffer->uMessageLen, pBuffer->uByteRangeStart, pBuffer->uByteRangeEnd, iotaState.fileSize);
						iota_test_printf("uSessionId %s, MSRP Message-ID %s, MSRP Transaction-ID %s\n",
							pIncomingSessnMsg->pSessionId, pIncomingSessnMsg->pMessageId, pIncomingSessnMsg->pTransactionId);

						// received file data
						if (ft.outputFile)
						{
							// write data to file
							if (!ft.outputFile.write((char *)pBuffer->pMessage, pBuffer->uMessageLen))
							{
								iota_test_printf("Error in writing data to file!\n");
							}

							iotaState.receivedSize += pBuffer->uMessageLen;
							if (iotaState.fileSize == iotaState.receivedSize)
							{
								iota_test_printf("Completed to receive file!\n");
							}
							else if (iotaState.fileSize < iotaState.receivedSize)
							{
								iota_test_printf("Received data size exceeded file size...\n");
							}
						}
					}
				}
				else
				{
					iota_test_printf("Message received from %s with DisplayName as %s!\n", pIncomingSessnMsg->pSenderURI, pIncomingSessnMsg->pDisplayName);
					iota_test_printf("uSessionId %s\n", pIncomingSessnMsg->pSessionId);
					iota_test_printf("MSRP messageId %s\n", pIncomingSessnMsg->pMessageId);

					if (pIncomingSessnMsg->pMessage->eContentType == EcrioCPMContentTypeEnum_FileTransferOverHTTP)
					{
						iota_test_printf("Received File Transfer over HTTP XML payload\n");
					}
					else if (pIncomingSessnMsg->pMessage->eContentType == EcrioCPMContentTypeEnum_PushLocation)
					{
						iotaTestPushLocationStruct pushLocation;
//						iota_test_ParsePushLocation(&pushLocation, pIncomingSessnMsg->pMessage->message.pBuffer->pMessage, pIncomingSessnMsg->pMessage->message.pBuffer->uMessageLen);
						iota_test_printf("pushLocation.label: %s\n", pushLocation.pushLocationLabel);
						iota_test_printf("pushLocation.pos: %s\n", pushLocation.pos);
						iota_test_printf("pushLocation.timestamp: %s\n", pushLocation.timestamp);
					}
					else
					{
						iota_test_printf("Text message: %s\n", pIncomingSessnMsg->pMessage->message.pBuffer->pMessage);
					}
					

					iota_test_printf("imdn.Message-ID: %s\n", pIncomingSessnMsg->pMessage->pIMDNMsgId);
					
					iota_test_printIMDNDispositionNotification(pIncomingSessnMsg->pMessage->imdnConfig);

					//here if SCL is present, it must be copied to a text file.
					if (pIncomingSessnMsg->pMessage->pBotSuggestion)
					{
						if (pIncomingSessnMsg->pMessage->pBotSuggestion->pJson)
						{
							fstream outfile;
							outfile.open("SCL_o.txt", fstream::app);
							if (outfile.write((char *)(pIncomingSessnMsg->pMessage->pBotSuggestion->pJson), pIncomingSessnMsg->pMessage->pBotSuggestion->uJsonLen))
							{
								iota_test_printf("Suggested chip list written to SCL_o.txt!\n");
							}
							else
							{
								iota_test_printf("Error in writing Suggested chip List to file!\n");
							}
							outfile.close();
						}
					}
				}
			}
		}
		break;
		case lims_Notify_Type_RICHCARD_RECEIVED:
		{
			EcrioCPMSessionMessageStruct* pIncomingSessnMsg = (EcrioCPMSessionMessageStruct *)pData;
			iota_test_printf("Richcard received from %s!\n", pIncomingSessnMsg->pSenderURI);
			iota_test_printf("uSessionId %s\n", pIncomingSessnMsg->pSessionId);
			iota_test_printf("MSRP messageId %s\n", pIncomingSessnMsg->pMessageId);
			//writing the RichCard to richcard.txt file
			fstream outfile;
			outfile.open("RC_o.txt", fstream::app);
			if (outfile.write((char *)(pIncomingSessnMsg->pMessage->message.pBuffer->pMessage), pIncomingSessnMsg->pMessage->message.pBuffer->uMessageLen))
			{
				iota_test_printf("Richcard written to RC_o.txt!\n");
			}
			else
			{
				iota_test_printf("Error in writing Rich Card to file!\n");
			}
			outfile.close();
			iota_test_printf("imdn.Message-ID: %s\n", pIncomingSessnMsg->pMessage->pIMDNMsgId);
			iota_test_printIMDNDispositionNotification(pIncomingSessnMsg->pMessage->imdnConfig);
			//here if SCL is present, it must be copied to a text file.
			if (pIncomingSessnMsg->pMessage->pBotSuggestion)
			{
				if (pIncomingSessnMsg->pMessage->pBotSuggestion->pJson)
				{
					fstream outfile;
					outfile.open("SCL_o.txt", fstream::app);
					if (outfile.write((char *)(pIncomingSessnMsg->pMessage->pBotSuggestion->pJson), pIncomingSessnMsg->pMessage->pBotSuggestion->uJsonLen))
					{
						iota_test_printf("Suggested chip list written to SCL_o.txt!\n");
					}
					else
					{
						iota_test_printf("Error in writing Suggested chip List to file!\n");
					}
					outfile.close();
				}
			}
		}
		break;
		case lims_Notify_Type_COMPOSING:
		{
			EcrioCPMComposingStruct *pComposing = NULL;
			EcrioCPMSessionMessageStruct *pSessionMsgStruct = (EcrioCPMSessionMessageStruct*)pData;
			if (pSessionMsgStruct != NULL && pSessionMsgStruct->pMessage != NULL &&
				pSessionMsgStruct->pMessage->eContentType == EcrioCPMContentTypeEnum_Composing)
				pComposing = pSessionMsgStruct->pMessage->message.pComposing;

			if (pComposing != NULL)
			{
				iota_test_printf("Is Composing message received for session %s\n", pSessionMsgStruct->pSessionId);

				if (pSessionMsgStruct->pSenderURI)
				{
					iota_test_printf("isComposing sender is %s\n", pSessionMsgStruct->pSenderURI);
				}
				if (pComposing->pDisplayName)
				{
					iota_test_printf("pComposing->pDisplayName is %s\n", pComposing->pDisplayName);
				}
				if (pComposing->pContent)
				{
					iota_test_printf("pIsComposing->pContent is %s\n", pComposing->pContent);
				}
				if (pComposing->pLastActive)
				{
					iota_test_printf("pIsComposing->pLastActive is %s\n", pComposing->pLastActive);
				}
				iota_test_printf("pIsComposing->uRefresh is %d\n", pComposing->uRefresh);
				if (EcrioCPMComposingType_Active == pComposing->eComposingState)
				{
					iota_test_printf("pIsComposing->state is Active\n");
				}
				else
				{
					iota_test_printf("pIsComposing->state is Idle\n");
				}
			}
		}
		break;
		case lims_Notify_Type_IMDN:
		{
			EcrioCPMIMDispoNotifStruct *pImdispNotif = (EcrioCPMIMDispoNotifStruct *)pData;
			if (pImdispNotif != NULL)
			{
				u_int32 index = 0;
				for (index = 0; index < pImdispNotif->uNumOfNotifBody; index++)
				{	
					iota_test_printf("IMDN Disposition - %d\n", index);
					/* map the Disposition enum */
					switch (pImdispNotif->pCPMIMDispoNotifBody[index].eDisNtf)
					{
					case EcrioCPMIMDispositionNtfTypeReq_Delivery:
					{
						iota_test_printf("IMDN Disposition msg type - Delivery\n");
						switch (pImdispNotif->pCPMIMDispoNotifBody[index].eDelivery)
						{
							case EcrioCPMIMDNDeliveryNotif_Delivered:
							{
								iota_test_printf("Delivered\n");
							}
							break;
							case EcrioCPMIMDNDeliveryNotif_Failed:
							{
								iota_test_printf("Failed\n");
							}
							break;
							case EcrioCPMIMDNDeliveryNotif_Forbidden:
							{
								iota_test_printf("Forbidden\n");
							}
							break;
							case EcrioCPMIMDNDeliveryNotif_Error:
							{
								iota_test_printf("Error\n");
							}
							break;
							default:
							{
								iota_test_printf("None\n");
							}
							break;
						}
					}
					break;
					case EcrioCPMIMDispositionNtfTypeReq_Display:
					{
						iota_test_printf("IMDN Disposition msg type - Display\n");
						/* map the display enum */
						switch (pImdispNotif->pCPMIMDispoNotifBody[index].eDisplay)
						{
							case EcrioCPMIMDNDisplayNotif_Displayed:
							{
								iota_test_printf("Displayed\n");
							}
							break;
							case EcrioCPMIMDNDisplayNotif_Forbidden:
							{
								iota_test_printf("Forbidden\n");
							}
							break;
							case EcrioCPMIMDNDisplayNotif_Error:
							{
								iota_test_printf("Error\n");
							}
							break;
							default:
							{
								iota_test_printf("None\n");
							}
							break;
						}
					}
					break;
					default:
					{
						iota_test_printf("IMDN Disposition msg type - None\n");
					}
					break;
					}
					
					if (pImdispNotif->pSenderURI)
					{
						iota_test_printf("IMDN Sender is %s\n", pImdispNotif->pSenderURI);
					}
					if (pImdispNotif->pDisplayName)
					{
						iota_test_printf("IMDN Sender DisplayName is %s\n", pImdispNotif->pDisplayName);
					}
					if (pImdispNotif->pDestURI)
					{
						iota_test_printf("IMDN Destination is %s\n", pImdispNotif->pDestURI);
					}

					if (pImdispNotif->pCPMIMDispoNotifBody[index].pRecipientUri)
					{
						iota_test_printf("IMDN Recipient is %s\n", pImdispNotif->pCPMIMDispoNotifBody[index].pRecipientUri);
					}
					if (pImdispNotif->pCPMIMDispoNotifBody[index].pOriginalRecipientUri)
					{
						iota_test_printf("IMDN Original Recipient is %s\n", pImdispNotif->pCPMIMDispoNotifBody[index].pOriginalRecipientUri);
					}
					if (pImdispNotif->pCPMIMDispoNotifBody[index].pIMDNMsgId)
					{
						iota_test_printf("IMDN msgId is %s\n", pImdispNotif->pCPMIMDispoNotifBody[index].pIMDNMsgId);
					}
				}
			}
		}
		break;

		case lims_Notify_Type_SUBSCRIBE_PARTICIPANTINFO_SUCCESS:
		{
			EcrioCPMReasonStruct *pReasonStruct = (EcrioCPMReasonStruct*)pData;

			iota_test_printf("Subscribe participant information success\n");

			if (pReasonStruct != NULL)
			{
				iota_test_printf("Session Id: %s\n", pReasonStruct->pSessionId);
				iota_test_printf("reason cause: %d\n", pReasonStruct->uReasonCause);
				iota_test_printf("reason text: %s\n", pReasonStruct->pReasonText != NULL ? pReasonStruct->pReasonText : (u_char*)"(NULL)");
			}
		}
		break;

		case lims_Notify_Type_SUBSCRIBE_PARTICIPANTINFO_FAILED:
		{
			EcrioCPMReasonStruct *pReasonStruct = (EcrioCPMReasonStruct*)pData;

			iota_test_printf("Subscribe participant information failed\n");

			if (pReasonStruct != NULL)
			{
				iota_test_printf("Session Id: %s\n", pReasonStruct->pSessionId);
				iota_test_printf("reason cause: %d\n", pReasonStruct->uReasonCause);
				iota_test_printf("reason text: %s\n", pReasonStruct->pReasonText != NULL ? pReasonStruct->pReasonText : (u_char*)"(NULL)");
			}
		}
		break;

		case lims_Notify_Type_UNSUBSCRIBE_PARTICIPANTINFO_SUCCESS:
		{
			EcrioCPMReasonStruct *pReasonStruct = (EcrioCPMReasonStruct*)pData;

			iota_test_printf("Un-Subscribe participant information success\n");

			if (pReasonStruct != NULL)
			{
				iota_test_printf("Session Id: %s\n", pReasonStruct->pSessionId);
				iota_test_printf("reason cause: %d\n", pReasonStruct->uReasonCause);
				iota_test_printf("reason text: %s\n", pReasonStruct->pReasonText != NULL ? pReasonStruct->pReasonText : (u_char*)"(NULL)");
			}
		}
		break;

		case lims_Notify_Type_UNSUBSCRIBE_PARTICIPANTINFO_FAILED:
		{
			EcrioCPMReasonStruct *pReasonStruct = (EcrioCPMReasonStruct*)pData;

			iota_test_printf("Un-Subscribe participant information failed\n");

			if (pReasonStruct != NULL)
			{
				iota_test_printf("Session Id: %s\n", pReasonStruct->pSessionId);
				iota_test_printf("reason cause: %d\n", pReasonStruct->uReasonCause);
				iota_test_printf("reason text: %s\n", pReasonStruct->pReasonText != NULL ? pReasonStruct->pReasonText : (u_char*)"(NULL)");
			}
		}
		break;

		case lims_Notify_Type_UPDATE_PARTICIPANT_SUCCESS:
		{
			EcrioCPMAddRemoveParticipantResponseStruct *pReasonStruct = (EcrioCPMAddRemoveParticipantResponseStruct*)pData;

			iota_test_printf("Update participant success\n");

			if (pReasonStruct != NULL)
			{
				iota_test_printf("Session Id: %s\n", pReasonStruct->pSessionId);
				iota_test_printf("Refer Id: %s\n", pReasonStruct->pReferId);
				iota_test_printf("reason cause: %d\n", pReasonStruct->uReasonCause);
				iota_test_printf("reason text: %s\n", pReasonStruct->pReasonText != NULL ? pReasonStruct->pReasonText : (u_char*)"(NULL)");
			}
		}
		break;

		case lims_Notify_Type_UPDATE_PARTICIPANT_FAILED:
		{
			EcrioCPMAddRemoveParticipantResponseStruct *pReasonStruct = (EcrioCPMAddRemoveParticipantResponseStruct*)pData;

			iota_test_printf("Update participant failed\n");

			if (pReasonStruct != NULL)
			{
				iota_test_printf("Session Id: %s\n", pReasonStruct->pSessionId);
				iota_test_printf("Refer Id: %s\n", pReasonStruct->pReferId);
				iota_test_printf("reason cause: %d\n", pReasonStruct->uReasonCause);
				iota_test_printf("reason text: %s\n", pReasonStruct->pReasonText != NULL ? pReasonStruct->pReasonText : (u_char*)"(NULL)");
			}
		}
		break;

		case lims_Notify_Type_MODIFY_GROUPCHAT_SUCCESS:
		{
			u_char *pSessionId = (u_char *)pData;

			iota_test_printf("Modify Group Chat Data success\n");

			if (pSessionId != NULL)
			{
				iota_test_printf("Session Id: %s\n", pSessionId);
			}
		}
		break;

		case lims_Notify_Type_MODIFY_GROUPCHAT_FAILED:
		{
			u_char *pSessionId = (u_char *)pData;

			iota_test_printf("Modify Group Chat Data failed\n");

			if (pSessionId != NULL)
			{
				iota_test_printf("Session Id: %s\n", pSessionId);
			}
		}
		break;

		case lims_Notify_Type_GROUPCHAT_INFO:
		{
			unsigned int i;
			EcrioCPMGroupChatInfoStruct *pInfo = (EcrioCPMGroupChatInfoStruct*)pData;

			iota_test_printf("Received Group Chat Information\n");
			if (pInfo != NULL)
			{
				if (pInfo->pSessionId != NULL)
					iota_test_printf("  Session Id: %s\n", pInfo->pSessionId);

				if (pInfo->eState == EcrioCPMGroupChatConferenceState_Full)
				{
					iota_test_printf("  Conference document state: full\n");
				}
				else if (pInfo->eState == EcrioCPMGroupChatConferenceState_Partial)
				{
					iota_test_printf("  Conference document state: partial\n");
				}
				else
				{
					iota_test_printf("  Conference document state: none\n");
				}

				if (pInfo->desc.pSubject != NULL)
					iota_test_printf("  Subject: %s\n", pInfo->desc.pSubject);
				if (pInfo->desc.pSubCngUser != NULL)
					iota_test_printf("  Who is changed subject: %s\n", pInfo->desc.pSubCngUser);
				if (pInfo->desc.pSubCngUser != NULL)
					iota_test_printf("  When is changed subject: %s\n", pInfo->desc.pSubCngDate);
				if (pInfo->desc.pIconUri != NULL)
					iota_test_printf("  Icon URI: %s\n", pInfo->desc.pIconUri);
				if (pInfo->desc.pIconInfo != NULL)
					iota_test_printf("  Icon info: %s\n", pInfo->desc.pIconInfo);
				if (pInfo->desc.pIconCngUser != NULL)
					iota_test_printf("  Who is changed icon: %s\n", pInfo->desc.pIconCngUser);
				if (pInfo->desc.pIconCngDate != NULL)
					iota_test_printf("  When is changed icon: %s\n", pInfo->desc.pIconCngDate);

				iota_test_printf("  Number of participants: %d\n", pInfo->uNumOfUsers);
				if (pInfo->ppUsers != NULL)
				{
					for (i = 0; i < pInfo->uNumOfUsers; i++)
					{
						if (pInfo->ppUsers[i] != NULL)
						{
							iota_test_printf("    User [%d]\n", i);
							if (pInfo->ppUsers[i]->pUri != NULL)
								iota_test_printf("      URI: %s\n", pInfo->ppUsers[i]->pUri);
							if (pInfo->ppUsers[i]->pDisplayText != NULL)
								iota_test_printf("      Display text: %s\n", pInfo->ppUsers[i]->pDisplayText);

							if (pInfo->ppUsers[i]->isHost == Enum_TRUE)
							{
								iota_test_printf("Group admin \n");
							}
							else
							{
								iota_test_printf("Group participant \n");
							}

							switch (pInfo->ppUsers[i]->status)
							{
								case EcrioCPMGroupChatUserStatus_Connected:
									iota_test_printf("      Status : Connected\n");
								break;
								case EcrioCPMGroupChatUserStatus_Disconnected:
									iota_test_printf("      Status : Disconnected\n");
								break;
								case EcrioCPMGroupChatUserStatus_Pending:
									iota_test_printf("      Status : Pending\n");
								break;
								case EcrioCPMGroupChatUserStatus_None:
								default:
									iota_test_printf("      Status : (none)\n");
								break;
							}
							switch (pInfo->ppUsers[i]->disMethod)
							{
								case EcrioCPMGroupChatDisconnMethod_Departed:
									iota_test_printf("      Disconnected method : Departed\n");
								break;
								case EcrioCPMGroupChatDisconnMethod_Booted:
									iota_test_printf("      Disconnected method : Booted\n");
								break;
								case EcrioCPMGroupChatDisconnMethod_Failed:
									iota_test_printf("      Disconnected method : Failed\n");
								break;
								case EcrioCPMGroupChatDisconnMethod_None:
								default:
									iota_test_printf("      Disconnected method : (none)\n");
								break;
							}
						}
						else
						{
							iota_test_printf("  ppUsers[%d] is NULL\n", i);
						}
					}
				}
				else
				{
					iota_test_printf("  ppUsers is NULL\n");
				}

				if (pInfo->pIconInfo)
				{
					iota_test_printf("		Icon Info\n");
					iota_test_printf("      Content Type: %s\n", pInfo->pIconInfo->pContentType);
					iota_test_printf("      Content Disposition: %s\n", pInfo->pIconInfo->pContentDisposition);
					iota_test_printf("      Content Id: %s\n", pInfo->pIconInfo->pContentId);
					iota_test_printf("      Content Transfer Encoding: %s\n", pInfo->pIconInfo->pContentTransferEncoding);
					iota_test_printf("      Content Length: %d\n", pInfo->pIconInfo->uDataLength);
				}
			}
		}
		break;
#if 0
		case lims_Notify_Type_QUERY_CAPABILITIES_SUCCESS:
		{
			EcrioUCEReasonStruct *pReason = (EcrioUCEReasonStruct *)pData;

			iota_test_printf("Query capabilities SUCCESS - cause code:%d, reason text:%s, queryId: %s\n", 
				pReason->uReasonCause, pReason->pReasonText, pReason->pCallId);
		}
		break;

		case lims_Notify_Type_QUERY_CAPABILITIES_FAILED:
		{
			EcrioUCEReasonStruct *pReason = (EcrioUCEReasonStruct *)pData;

			iota_test_printf("Query capabilities FAILED - cause code:%d, reason text:%s, queryId: %s\n", 
				pReason->uReasonCause, pReason->pReasonText, pReason->pCallId);
		}
		break;

		case lims_Notify_Type_QUERY_CAPABILITIES_DISCOVERED:
		{

			EcrioUCECapabilityDiscoveryStruct *pCapability = (EcrioUCECapabilityDiscoveryStruct *)pData;
			
			u_int32 uOptonsFeatures = 0;

			if (pCapability == NULL) {
				iota_test_printf("Capabilities NOT discovered \n");
				break;
			}

			iota_test_printf("Query capabilities DISCOVERED\n");

			if (pCapability->eCapabilityType == ECRIO_UCE_Capabilty_Type_OPTIONS){

				EcrioUCEOptionsCallBackStruct *pFeatures = (EcrioUCEOptionsCallBackStruct *)(pCapability->pCapabilities);

				if (pFeatures)
				{
					iota_test_printf("Features received --- \n");

					uOptonsFeatures = pFeatures->uOptionsFeatures;

					/* Features items print */
					if (uOptonsFeatures & ECRIO_UCE_ServiceID_StandaloneMessaging)
						cout << "Support Messaging\n";

					if (uOptonsFeatures & ECRIO_UCE_ServiceID_Chat)
						cout << "Support Chat \n";

					if (uOptonsFeatures & ECRIO_UCE_ServiceID_FileTransferHTTP)
						cout << "Support FileTransferHTTP \n";

					if (uOptonsFeatures & ECRIO_UCE_ServiceID_FileTransferMSRP)
						cout << "Support MSRP  \n";


					if (uOptonsFeatures & ECRIO_UCE_ServiceID_FileTransferThumbnail)
						cout << "Support Thumbnail  \n";

					if (uOptonsFeatures & ECRIO_UCE_ServiceID_QueryCapsUsingPresence)
						cout << "Support Query  \n";

					if (uOptonsFeatures & ECRIO_UCE_ServiceID_VoLTE)
						cout << "Support VoLTE  \n";

					if (uOptonsFeatures & ECRIO_UCE_ServiceID_Video)
						cout << "Support Video  \n";

					if (uOptonsFeatures & ECRIO_UCE_ServiceID_PostCall)
						cout << "PostCall  \n";

					if (uOptonsFeatures & ECRIO_UCE_ServiceID_FileTransfer)
						cout << "File Transfer  \n";

					if (uOptonsFeatures & ECRIO_UCE_ServiceID_FileTransferSMS)
						cout << "File Transfer via SMS \n";

					if (uOptonsFeatures & ECRIO_UCE_ServiceID_Chatbot)
						cout << "Chatbot role \n";

					if (uOptonsFeatures & ECRIO_UCE_ServiceID_ChatbotUsingSession)
						cout << "Chatbot Communication using sessions \n";
 
					if (uOptonsFeatures & ECRIO_UCE_ServiceID_ChatbotUsingStandaloneMessage)
						cout << "Chatbot Communication using Standalone Messaging \n";


					if (uOptonsFeatures & ECRIO_UCE_ServiceID_GeolocationPush)
						cout << "Geolocation PUSH  \n";

					if (uOptonsFeatures & ECRIO_UCE_ServiceID_GeolocationPushSMS)
						cout << "Geolocation PUSH via SMS  \n";
 
					if (uOptonsFeatures & ECRIO_UCE_ServiceID_CallComposerEnrichedCalling)
						cout << "Call composer via Enriched Calling Session  \n";

					
					if (uOptonsFeatures & ECRIO_UCE_ServiceID_CallComposerMultimediaTelephony)
						cout << "Call composer via Multimedia Telephony session \n";
					

					if (uOptonsFeatures & ECRIO_UCE_ServiceID_SharedMap)
						cout << "Shared Map \n";

					if (uOptonsFeatures & ECRIO_UCE_ServiceID_SharedSketch)
						cout << "Shared Sketch  \n";
				}

				if (pFeatures->uNumberOfPresentity > 0) {
					for (int i = 0; i < pFeatures->uNumberOfPresentity; i++)
					{
						iota_test_printf("  Presentity URI: %s\n", pFeatures->pPresentityURI[i].pURI);
					}		
				}

			}
			/* SUBSCRIBE/NOTIFY*/
			else if (pCapability->eCapabilityType == ECRIO_UCE_Capabilty_Type_SUBSCRIBENOTIFY){

				EcrioUCENotifyCallBackStruct *pDocument = (EcrioUCENotifyCallBackStruct *)(pCapability->pCapabilities);
				unsigned int i, j;

				
				if (pDocument != NULL)
				{
					iota_test_printf("  Record count: %d\n", pDocument->uRecordCount);
					if (pDocument->ppRecords != NULL)
					{
						for (i = 0; i < pDocument->uRecordCount; i++)
						{
							iota_test_printf("   Record number: %d\n", i);
							if (pDocument->ppRecords[i] == NULL)
							{
								iota_test_printf("    ppRecords is NULL!!\n");
								continue;
							}
							iota_test_printf("    Service count: %d\n", pDocument->ppRecords[i]->uServiceCount);
							if (pDocument->ppRecords[i]->ppServices != NULL)
							{
								for (j = 0; j < pDocument->ppRecords[i]->uServiceCount; j++)
								{
									iota_test_printf("    Service number: %d\n", j);
									if (pDocument->ppRecords[i]->ppServices[j] == NULL)
									{
										iota_test_printf("    ppServices is NULL!!\n");
										continue;
									}
									iota_test_printf("     Service ID: %d\n", pDocument->ppRecords[i]->ppServices[j]->eID);
									if (pDocument->ppRecords[i]->ppServices[j]->eID == ECRIO_UCE_ServiceID_VoLTE ||
										pDocument->ppRecords[i]->ppServices[j]->eID == ECRIO_UCE_ServiceID_Video)
									{
										iota_test_printf("       Service: %s\n", pDocument->ppRecords[i]->ppServices[j]->volte.bServc == Enum_TRUE ? "true" : "false");
										iota_test_printf("       Audio: %s\n", pDocument->ppRecords[i]->ppServices[j]->volte.bAudio == Enum_TRUE ? "true" : "false");
										iota_test_printf("       Video: %s\n", pDocument->ppRecords[i]->ppServices[j]->volte.bVideo == Enum_TRUE ? "true" : "false");
										iota_test_printf("       Duplex type: %d\n", pDocument->ppRecords[i]->ppServices[j]->volte.duplex.eDuplexType);
										iota_test_printf("       Supported: %s\n", pDocument->ppRecords[i]->ppServices[j]->volte.duplex.bSupported == Enum_TRUE ? "true" : "false");
									}
									iota_test_printf("     Contact URI: %s, URI Type: %d\n", pDocument->ppRecords[i]->ppServices[j]->pContact->pURI, pDocument->ppRecords[i]->ppServices[j]->pContact->eUriType);
									iota_test_printf("     Timestamp: %s\n", pDocument->ppRecords[i]->ppServices[j]->pTimestamp);
								}
							}
							else
							{
								iota_test_printf("  ppServices is NULL!!\n");
							}
						}
					}
					else
					{
						iota_test_printf("  ppRecords is NULL!!\n");
					}
				}
				else
				{
					iota_test_printf("  No data !!\n");
				}

			}

		}
		break;
#endif

		case lims_Notify_Type_GROUPDATA_INFO:
		{
			EcrioCPMGroupDataRequestStruct* pGrpData = (EcrioCPMGroupDataRequestStruct*)pData;
			iota_test_printf("      Group Data Type : %d\n", pGrpData->type);
			iota_test_printf("      Group Data Subject : %s\n", pGrpData->subject.pText);
			iota_test_printf("      Group Data icon-uri : %s\n", pGrpData->icon.pUri);
		}
		break;

		case lims_Notify_Type_GROUPCHAT_ICON:
		{
			fstream iconFile; 
			EcrioCPMGroupChatIconStruct* pIcon = (EcrioCPMGroupChatIconStruct*)pData;
			iota_test_printf("      Group Icon Data Received!!!\n");
			iota_test_printf("      Content Type : %s\n", pIcon->pContentType);
			iota_test_printf("      Sender URI : %s\n", pIcon->pSenderURI);

			string contentType = string(pIcon->pContentType);
			int pos = contentType.find("/");
			string fileExtension = contentType.substr(pos + 1);
			string fileName = to_string((unsigned long)pal_UtilityRandomNumber) + "." + fileExtension;

			iconFile.open(fileName, ios::binary | ios::out);
			if (iconFile)
			{
				iconFile.write((char*)pIcon->pData, pIcon->uDataLength);
				iconFile.close();

				iota_test_printf("      Icon File : %s\n", fileName.c_str());
			}
		}
		break;

		case lims_Notify_Type_PRIVACY_MANAGEMENT_RESPONSE:
		{
			EcrioCPMPrivMngCmdRespStruct *pStruct = (EcrioCPMPrivMngCmdRespStruct *)pData;

			iota_test_printf("Received Privacy Management Response - Command-ID:%s, result:", pStruct->pCommandID);
			switch (pStruct->eResp)
			{
				case EcrioCPMPrivMngCmdResp_On:
					iota_test_printf("ON\n");
					break;
				case EcrioCPMPrivMngCmdResp_Off:
					iota_test_printf("OFF\n");
					break;
				case EcrioCPMPrivMngCmdResp_Failure:
					iota_test_printf("Failure\n");
					break;
				case EcrioCPMPrivMngCmdResp_None:
				default:
					iota_test_printf("(None)\n");
					break;
			}
		}
		break;
#endif //ENABLE_RCS

		case lims_Notify_Type_ESTABLISH_IPSEC_SA:
		{
			EcrioSipIPsecNegotiatedParamStruct *pIPSecNeg = (EcrioSipIPsecNegotiatedParamStruct*)pData;

			if (pIPSecNeg->pAlgo != NULL)
			{
				iota_test_printf("auth algorithm is %s\n", pIPSecNeg->pAlgo);
			}
#ifdef WIN32
			/* Send the signal the Signal thread for processing. */
			BOOL bReturn = PostThreadMessage(iotaState.ipsecthreadId, SIGNAL_SEND_EVENT, (WPARAM)lims_Network_Status_IPsec_Established, 
				(LPARAM)iotaState.limsHandle);
			if (bReturn == FALSE)
			{
				iota_test_printf("PostThreadMessage failed with error\n");
			}
#else
			//
			iota_test_signal_send(iotaState.hSignal, IOTA_TEST_IPSEC_ESTABLISHED, 0, NULL);
#endif
		}
		break;

#ifdef ENABLE_QCMAPI
		case lims_Notify_Type_CONNECTION_CREATED:
		{
			iota_test_printf("Connection Manager Service is Created!\n");
		}
		break;

		case lims_Notify_Type_GET_ACSCONFIGURATION:
		{
			iota_test_printf("get ACSConfiguration!\n");
			QcmACSConfigStruct *configStruct = (QcmACSConfigStruct *)pData;
			
			iota_test_printf("type=%u\n", configStruct->uType);
			iota_test_printf("config=%s\n", configStruct->pConfig);
		}
		break;
		
		case lims_Notify_Type_AUTOCONFIG_REQUESTSTATUS:
		{
			iota_test_printf("AutoconfigRequestStatus is arrived!\n");
			u_int32 *status = (u_int32 *)pData;
			
			iota_test_printf("status=%u\n", *status);
		}
		break;
#endif
		
		default:
		{
		}
		break;
	}

	pal_MutexUnlock(iotaState.mutexHandle);
}

void limsCallback
(
	lims_CallbackTypeEnums eCallbackType,
	lims_NotifyTypeEnums eNotifType,
	void *pData,
	void *pContext
)
{
	switch (eCallbackType)
	{
		case lims_CallbackType_Status:
		{
			handleStatusCallback(eNotifType, pData, pContext);
		}
		break;

		default:
		{
		}
		break;
	}
}

// Helper and wrapper functions.

void iota_test_Setup
(
	void
)
{
	unsigned int error = KPALErrorNone;

	iotaState.limsHandle = NULL;

	// Initialize logging to use for the Light IMS test application.
#ifdef ANDROID
	iotaState.logHandle = pal_LogInit((void *)"/sdcard/iota_test.log", KLogOutputFile, 0x000003B3, KLogLevel_All, KLogComponent_All, KLogType_All);
#else
	iotaState.logHandle = pal_LogInit((void *)"iota_test.log", KLogOutputFile, 0x000003B3, KLogLevel_All, KLogComponent_All, KLogType_All);
#endif
	// @todo Add check here.

	// Initialize the PAL, get the PAL instance.
	error = pal_Init(iotaState.logHandle, &iotaState.palLimsInstance);
	if (error != KPALErrorNone)
	{
		iota_test_printf("Could not initialize the PAL instance!\n");

		if (iotaState.logHandle != NULL)
		{
			// Deinitialize the logging instance.
			pal_LogDeinit(&iotaState.logHandle);
		}

		return;
	}

	error = pal_MutexCreate(iotaState.palLimsInstance, &iotaState.mutexHandle);
	if (error != KPALErrorNone)
	{
		iota_test_printf("Could not initialize the PAL instance!\n");

		pal_Deinit(iotaState.palLimsInstance);
		if (iotaState.logHandle != NULL)
		{
			// Deinitialize the logging instance.
			pal_LogDeinit(&iotaState.logHandle);
		}

		return;
	}

#ifndef WIN32
	error = pal_SocketSetDeviceName(iotaState.palLimsInstance, iotaState.localInterface);
	if (error != KPALErrorNone)
	{
		pal_Deinit(iotaState.palLimsInstance);
		if (iotaState.logHandle != NULL)
		{
			// Deinitialize the logging instance.
			pal_LogDeinit(&iotaState.logHandle);
		}

		return;
	}
#endif

#ifdef WIN32
#ifdef _DEBUG
	pal_memory_init();
	pal_memory_reset(iotaState.logHandle);
#endif
#endif
}

void iota_test_Teardown
(
	void
)
{
#ifdef WIN32
#ifdef _DEBUG
	pal_memory_log_dump();
	pal_memory_deinit();
#endif
#endif

	pal_MutexDelete(&iotaState.mutexHandle);

	if (iotaState.palLimsInstance != NULL)
	{
		// Deinitialize the PAL instance.
		pal_Deinit(iotaState.palLimsInstance);
		iotaState.palLimsInstance = NULL;

		if (iotaState.logHandle != NULL)
		{
			// Deinitialize the logging instance.
			pal_LogDeinit(&iotaState.logHandle);
		}
	}
}

// API Support Functions

#ifdef ENABLE_QCMAPI
unsigned int iota_test_init
(
	string iccid
)
#else
unsigned int iota_test_init
(
	void
)
#endif
{
	pal_MutexLock(iotaState.mutexHandle);
	unsigned int error = LIMS_NO_ERROR;
	lims_ConfigStruct config;
	lims_CallbackStruct callback;
	char *pLocalIp = NULL;
	int signalerror = 0;

	memset(&config, 0, sizeof(lims_ConfigStruct));
	memset(&callback, 0, sizeof(lims_CallbackStruct));

	iota_test_printf("Calling lims_Init()\n");

	config.pal = iotaState.palLimsInstance;
	config.logHandle = iotaState.logHandle;

	// setting default channel is TCP only. actual configuration options read from config ini file.
	config.bEnableTcp = Enum_TRUE;
	config.bEnableUdp = Enum_FALSE;

	if (iota_test_config_getConfig(iotaState.configFile, &config) != 0)
	{
		iota_test_printf("Can't load %s for general configuration\n", iotaState.configFile);
		pal_MutexUnlock(iotaState.mutexHandle);
		return 1;
	}

	if (iota_test_config_getRcsService(iotaState.configFile, &config) != 0)
	{
		iota_test_printf("Can't load %s for rcs service configuration\n", iotaState.configFile);
		pal_MutexUnlock(iotaState.mutexHandle);
		return 1;
	}

	config.bIsRelayEnabled = Enum_FALSE;
	config.pRelayServerIP = NULL;
	config.uRelayServerPort = 2855;

	iotaState.bIsFileSender = false;

	if (iota_test_config_getTLSParameters(iotaState.configFile, &config) != 0)
	{
		iota_test_printf("Can't load %s for TLS configuration\n", iotaState.configFile);
		pal_MutexUnlock(iotaState.mutexHandle);
		return 1;
	}

	if (iota_test_config_getMsrpRelay(iotaState.configFile, &config) != 0)
	{
		iota_test_printf("Can't load %s for msrp relay configuration\n", iotaState.configFile);
		pal_MutexUnlock(iotaState.mutexHandle);
		return 1;
	}

	if (iota_test_config_getPrecondition(iotaState.configFile, &config) != 0)
	{
		iota_test_printf("Can't load %s for precondition configuration\n", iotaState.configFile);
		pal_MutexUnlock(iotaState.mutexHandle);
		return 1;
	}

	strcpy(iotaState.calleeNumber, "sip:+14087770002@ecrio.com");
	strcpy(iotaState.message, "initial string");

	if (iota_test_config_getDefaults(iotaState.configFile, &iotaState) != 0)
	{
		iota_test_printf("Can't load %s for defaults configuration\n", iotaState.configFile);
		pal_MutexUnlock(iotaState.mutexHandle);
		return 1;
	}

#ifdef ENABLE_QCMAPI
	config.regModel.isRegistrationShared = Enum_TRUE;
	config.regModel.pRegSharedData = (void *)iccid.c_str();
#endif

	//config.cpm.pUsername = config.pPublicIdentity;
	callback.pLimsCallback = limsCallback;
	callback.pLimsGetPropertyCallback = limsGetPropertyCallback;
	callback.pContext = NULL;

	/* UCE Property CallBack*/
	callback.pUCEPropertyCallback = limsUCEPropertyCallback;

	pal_MutexUnlock(iotaState.mutexHandle);
	iotaState.limsHandle = lims_Init(&config, &callback, &error);
	pal_MutexLock(iotaState.mutexHandle);
	if (iotaState.limsHandle != NULL)
	{
#ifndef ENABLE_QCMAPI
		lims_NetworkConnectionStruct network;
		memset(&network, 0, sizeof(lims_NetworkConnectionStruct));

		
//		if (iota_test_fqdn_getNetwork(&config,&network) != 0)
		{
			if (iota_test_config_getNetwork(iotaState.configFile, &network) != 0)
			{
				iota_test_printf("Can't load %s for network configuration\n", iotaState.configFile);
				pal_MutexUnlock(iotaState.mutexHandle);
				return 1;
			}
		}

		if (network.uNoOfRemoteIps >= 1)
		{
			if (strstr(network.ppRemoteIPs[0], ":") != NULL)
			{
				pLocalIp = iota_test_getLocalIpAddress(6, iotaState.localInterface);
				if (pLocalIp != NULL)
				{
					strncpy((char *)iotaState.localAddress, pLocalIp, strlen(pLocalIp));
					free(pLocalIp);
				}
				else
				{
					iota_test_printf("Can't find a local IP!\n");
					goto Cleanup;
				}

				network.pLocalIp = (char *)iotaState.localAddress;
			}
			else
			{
				pLocalIp = iota_test_getLocalIpAddress(4, iotaState.localInterface);
				if (pLocalIp != NULL)
				{
					strncpy((char *)iotaState.localAddress, pLocalIp, strlen(pLocalIp));
					free(pLocalIp);
				}
				else
				{
					iota_test_printf("Can't find a local IP!\n");
					goto Cleanup;
				}

				network.pLocalIp = (char *)iotaState.localAddress;
			}
		}

		iota_test_printf("RemoteIP[0]: %s\n", network.ppRemoteIPs[0]);
		iota_test_printf("LocalIp: %s\n", network.pLocalIp);

		if (strstr(network.pLocalIp, ":") != NULL)
		{
			network.eIPType = lims_Network_IP_Type_V6;
		}
		else
		{
			network.eIPType = lims_Network_IP_Type_V4;
		}

		iota_test_printf("Calling lims_NetworkStateChange()\n");
		network.uStatus = lims_Network_Status_Success;

		pal_MutexUnlock(iotaState.mutexHandle);
		error = lims_NetworkStateChange(iotaState.limsHandle, lims_Network_PDN_Type_IMS, lims_Network_Connection_Type_LTE, &network);
		pal_MutexLock(iotaState.mutexHandle);
		if (error != LIMS_NO_ERROR)
		{
			iota_test_printf("lims_NetworkStateChange failed \n");
		}		
#endif
#ifdef WIN32
		iotaState.hThreadReady = NULL;

		/** Create an event that will be used to wait for the thread to complete. */
		iotaState.hThreadReady = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (iotaState.hThreadReady == NULL)
		{
			/* CreateEvent() function failed. Use GetLastError() to get reason. */
			/* @todo Log this situation. */
			iota_test_printf("CreateEvent failed \n");
		}

		/** Create the Signal thread. */

		iotaState.hIPSecHandle = (HANDLE)_beginthreadex(NULL, 0, IPSECThread, (void *)&iotaState, FALSE, (unsigned int *)&(iotaState.ipsecthreadId));
		if (iotaState.hIPSecHandle == NULL)
		{
			/* CreateThread() function failed. Use GetLastError() to get reason. */
			/* @todo Log this situation. */

			iota_test_printf("Thread creation failed \n");
		}
		WaitForSingleObject(iotaState.hThreadReady, INFINITE);

#else
		iotaState.hSignal = iota_test_signal_init(iota_test_IPSECThread, iotaState.logHandle, &signalerror);
		if (signalerror != 0)
		{
			iota_test_printf("iota_test_signal_init() failed with error:%d",signalerror);
		}
#endif

#ifndef ENABLE_QCMAPI
Cleanup:

		// Release memory consumed by ini handler for networking.
		if (network.ppRemoteIPs != NULL)
		{
			unsigned int i;

			for (i = 0; i < network.uNoOfRemoteIps; ++i)
			{
				if (network.ppRemoteIPs[i])
				{
					free(network.ppRemoteIPs[i]);
				}
			}

			free(network.ppRemoteIPs);
		}
#endif
	}

	// Release memory consumed by ini handler for configuration.
	if (config.pHomeDomain)
	{
		free(config.pHomeDomain);
	}

	if (config.pPassword)
	{
		free(config.pPassword);
	}

	if (config.pPrivateIdentity)
	{
		free(config.pPrivateIdentity);
	}

	if (config.pPublicIdentity)
	{
		free(config.pPublicIdentity);
	}

	if (config.pUserAgent)
	{
		free(config.pUserAgent);
	}

	if (config.pDeviceId)
	{
		free(config.pDeviceId);
	}

	if (config.pDisplayName)
	{
		free(config.pDisplayName);
	}

	if (config.pPANI)
	{
		free(config.pPANI);
	}


	if (config.pRelayServerIP)
	{
		free(config.pRelayServerIP);
	}

	if (config.pTLSCertificate)
	{
		free(config.pTLSCertificate);
	}

	pal_MutexUnlock(iotaState.mutexHandle);
	return error;
}

unsigned int iota_test_deinit
(
	void
)
{
	unsigned int error = LIMS_NO_ERROR;

	if (iotaState.limsHandle != NULL)
	{
		iota_test_printf("Calling lims_Deinit()\n");

		error = lims_Deinit(&iotaState.limsHandle);
		if (error != LIMS_NO_ERROR)
		{
			iota_test_printf("lims_Deinit()- failed with error:%d\n", error);
		}
		else
		{
			iotaState.limsHandle = NULL;
		}
	}
	else
	{
		iota_test_printf("Calling already deinitialized\n");
	}

#ifndef WIN32


	iota_test_signal_deinit(&(iotaState.hSignal));
#endif

	if (iotaState.pSessionId != NULL)
	{
		pal_MemoryFree((void**)&iotaState.pSessionId);
	}
	if (iotaState.pGroupSessionId != NULL)
	{
		pal_MemoryFree((void**)&iotaState.pGroupSessionId);
	}
	if (iotaState.pReferId != NULL)
	{
		pal_MemoryFree((void**)&iotaState.pReferId);
	}

	return error;
}

unsigned int iota_test_register
(
	void
)
{
	pal_MutexLock(iotaState.mutexHandle);
	unsigned int error = LIMS_NO_ERROR;
#ifdef ENABLE_RCS
	unsigned int uFeatures = lims_Feature_CPM_PagerMode | lims_Feature_CPM_LargeMode | lims_Feature_CPM_Chat | lims_Feature_CPM_GeoLocation | lims_Feature_CPM_IMDN_Aggregation;
//	unsigned int uFeatures = lims_Feature_CPM_PagerMode;
#else
	unsigned int uFeatures = lims_Feature_IMS_Sms | lims_Feature_IMS_VoLTE;
#endif

	if (iotaState.limsHandle != NULL)
	{
		lims_RegisterStruct registerStruct;

		registerStruct.pIPsecParams = NULL;
		iota_test_printf("Calling lims_Register()\n");

		pal_MutexUnlock(iotaState.mutexHandle);
		error = lims_Register(iotaState.limsHandle, uFeatures, &registerStruct);
		pal_MutexLock(iotaState.mutexHandle);
		if (error != LIMS_NO_ERROR)
		{
			iota_test_printf("lims_Register() failed\n");
		}
	}

	pal_MutexUnlock(iotaState.mutexHandle);
	return error;
}

unsigned int iota_test_register_IPsec
(
	void
)
{
	pal_MutexLock(iotaState.mutexHandle);
	unsigned int error = LIMS_NO_ERROR;
#ifdef ENABLE_RCS
	unsigned int uFeatures = lims_Feature_CPM_PagerMode | lims_Feature_CPM_LargeMode | lims_Feature_CPM_Chat | lims_Feature_CPM_GeoLocation;
#else
	unsigned int uFeatures = lims_Feature_IMS_Sms | lims_Feature_IMS_VoLTE;
#endif

	if (iotaState.limsHandle != NULL)
	{
		lims_RegisterStruct registerStruct;
		EcrioSipIPsecParamStruct iPsecParams;
		EcrioSipIPsecValueStruct *pAlgo = NULL, algo;
		EcrioSipIPsecValueStruct *pEnc = NULL, *pEnc1 = NULL, *ppEnc = NULL, enc = { 0 }, enc1 = { 0 };

		memset(&iPsecParams, 0, sizeof(EcrioSipIPsecParamStruct));
		memset(&algo, 0, sizeof(EcrioSipIPsecValueStruct));

		iPsecParams.uPort_uc = 5002;
		iPsecParams.uPort_us = 65555;
		iPsecParams.uSpi_uc = 3602756719;
		iPsecParams.uSpi_us = 3123742313;
		iPsecParams.uEncryptCount = 1;
		iPsecParams.ppEncrypt = &ppEnc;
		enc.pValue = (char*)pal_StringCreate((u_char*)"des-ede3-cbc", pal_StringLength((u_char*)"des-ede3-cbc"));
		pEnc = &enc;
		iPsecParams.ppEncrypt[0] = pEnc;
//		enc1.pValue = (char*)pal_StringCreate((u_char*)"aec-cbc", pal_StringLength((u_char*)"aec-cbc"));
//		pEnc1 = &enc1;
//		iPsecParams.ppEncrypt[1] = pEnc1;

		iPsecParams.uAlgoCount = 1;
		iPsecParams.ppAlgo = &pAlgo;
		algo.pValue = (char*)pal_StringCreate((u_char*)"hmac-md5-96", pal_StringLength((u_char*)"hmac-md5-96"));
		pAlgo = &algo;

		registerStruct.pIPsecParams = &iPsecParams;
		iota_test_printf("Calling lims_Register()\n");

		pal_MutexUnlock(iotaState.mutexHandle);
		error = lims_Register(iotaState.limsHandle, uFeatures, &registerStruct);
		pal_MutexLock(iotaState.mutexHandle);
		if (error != LIMS_NO_ERROR)
		{
			iota_test_printf("lims_Register() failed\n");
		}
	}

	pal_MutexUnlock(iotaState.mutexHandle);
	return error;
}

unsigned int iota_test_deregister
(
	void
)
{
	pal_MutexLock(iotaState.mutexHandle);
	unsigned int error = LIMS_NO_ERROR;

	if (iotaState.limsHandle != NULL)
	{
		iota_test_printf("Calling lims_Deregister()\n");

		pal_MutexUnlock(iotaState.mutexHandle);
		error = lims_Deregister(iotaState.limsHandle);
		pal_MutexLock(iotaState.mutexHandle);
		if (error != LIMS_NO_ERROR)
		{
			iota_test_printf("lims_Deregister() failed\n");
		}
	}

	pal_MutexUnlock(iotaState.mutexHandle);
	return error;
}

unsigned int iota_test_dettach_network
	()
{
	pal_MutexLock(iotaState.mutexHandle);
	unsigned int error = LIMS_NO_ERROR;

	iotaState.bRegistered = false;
	iota_test_printf("ims pdn detached.\n");
	pal_MutexUnlock(iotaState.mutexHandle);
	error = lims_NetworkStateChange(iotaState.limsHandle, lims_Network_PDN_Type_IMS, lims_Network_Connection_Type_NONE, NULL);
	pal_MutexLock(iotaState.mutexHandle);
	if (error != LIMS_NO_ERROR)
	{
		iota_test_printf("lims_NetworkStateChange() Failed.\n");
	}
	else
	{
		iota_test_printf("network disconnected\n");
	}

	pal_MutexUnlock(iotaState.mutexHandle);
	return error;
}


unsigned int iota_test_SendStandAloneMessage
(
	char* pText,
	BoolEnum bIsChatbot,
	BoolEnum bDeleteToken
)
{
	pal_MutexLock(iotaState.mutexHandle);
	unsigned int error = LIMS_NO_ERROR;
	lims_StandAloneMessageStruct sendMessg;
	EcrioCPMBufferStruct text;
	EcrioCPMConversationsIdStruct ids;
	u_char convId[64];
	u_char contId[64];
	char *pCallId = NULL;
	char imdnId[21] = {'\0'};

	EcrioCPMMessageStruct message;

	memset(&sendMessg, 0, sizeof(lims_StandAloneMessageStruct));
	memset(&text, 0, sizeof(EcrioCPMBufferStruct));
	memset(&ids, 0, sizeof(EcrioCPMConversationsIdStruct));
	memset(&convId, 0, 64);
	memset(&contId, 0, 64);
	memset(&message, 0, sizeof(EcrioCPMMessageStruct));

	iota_test_printf("Calling lims_SendStandAloneMessage()\n");
	
	sendMessg.pDestUri = (char *)iotaState.calleeNumber;
	
	sendMessg.bIsChatbot = bIsChatbot;
	sendMessg.bDeleteChatBotToken = bDeleteToken;

	if (bDeleteToken != Enum_TRUE)
	{
		sendMessg.pMessage = &message;
		sendMessg.pMessage->imdnConfig = EcrioCPMIMDispositionConfigPositiveDelivery | EcrioCPMIMDispositionConfigDisplay;
		iota_test_getRandomString((unsigned char *)imdnId, 20);
		sendMessg.pMessage->pIMDNMsgId = (char *)imdnId;
		//sendMessg.pMessage->pIMDNMsgId = (char*)"DUpjRCHsZKFA5lrDmDNGi5MIeEgdP";

		sendMessg.pMessage->message.pBuffer = &text;
		sendMessg.pMessage->eContentType = EcrioCPMContentTypeEnum_Text;
		text.pMessage = (u_char*)pText;
		text.uMessageLen = pal_StringLength(text.pMessage);
		sendMessg.pConvId = &ids;

		iota_test_getContributionID(contId);
		iota_test_getContributionID(convId);
		ids.pContributionId = contId;
		ids.pConversationId = convId;

		if (NULL == pal_StringNCopy(iotaState.contID, 64, ids.pContributionId, pal_StringLength(ids.pContributionId)))
		{
			iota_test_printf("Memory copy error.\n");
			return 1;
		}
		if (NULL == pal_StringNCopy(iotaState.convID, 64, ids.pConversationId, pal_StringLength(ids.pContributionId)))
		{
			iota_test_printf("Memory copy error.\n");
			return 1;
		}
	}

	pal_MutexUnlock(iotaState.mutexHandle);
	error = lims_SendStandAloneMessage(iotaState.limsHandle, &sendMessg, &pCallId);
	pal_MutexLock(iotaState.mutexHandle);
	if (error != LIMS_NO_ERROR)
	{
		iota_test_printf("lims_SendStandAloneMessage() failed.\n");
	}
	else
	{
		iota_test_printf("lims_SendStandAloneMessage() success.\n");
	}
	if (pCallId)
	{ 
		iota_test_printf("Call id: %s \n", pCallId);
		pal_MemoryFree((void**)&pCallId);
	}

	pal_MutexUnlock(iotaState.mutexHandle);
	return error;
}

unsigned int iota_test_SendPagerIMDNMessage
(
char* pText
)
{
	pal_MutexLock(iotaState.mutexHandle);
	unsigned int error = LIMS_NO_ERROR;
	lims_StandAloneMessageStruct sendMessg;
	EcrioCPMBufferStruct text;
	EcrioCPMConversationsIdStruct ids;
	u_char convId[64];
	u_char contId[64];
	char *pCallId = NULL;

	EcrioCPMMessageStruct message;
	EcrioCPMIMDispoNotifStruct imdn;
	EcrioCPMIMDispoNotifBodyStruct* pDispoNotif = NULL;

	memset(&sendMessg, 0, sizeof(lims_StandAloneMessageStruct));
	memset(&text, 0, sizeof(EcrioCPMBufferStruct));
	memset(&ids, 0, sizeof(EcrioCPMConversationsIdStruct));
	memset(&convId, 0, 64);
	memset(&contId, 0, 64);
	memset(&message, 0, sizeof(EcrioCPMMessageStruct));
	memset(&imdn, 0, sizeof(EcrioCPMIMDispoNotifStruct));

	iota_test_printf("Calling lims_SendStandAloneMessage()\n");

	sendMessg.pDestUri = (char *)iotaState.calleeNumber;
	sendMessg.pMessage = &message;
	sendMessg.pMessage->imdnConfig = EcrioCPMIMDispositionConfigPositiveDelivery | EcrioCPMIMDispositionConfigDisplay;
	sendMessg.pMessage->pIMDNMsgId = (char*) "DUpjRCHsZKFA5lrDmDNGi5MIeEgdP";

	sendMessg.pMessage->message.pBuffer = &text;
	sendMessg.pMessage->eContentType = EcrioCPMContentTypeEnum_Text;
	text.pMessage = (u_char*)pText;
	text.uMessageLen = pal_StringLength(text.pMessage);
	sendMessg.pConvId = &ids;

	iota_test_getContributionID(contId);
	iota_test_getContributionID(convId);

	ids.pContributionId = contId;
	ids.pConversationId = convId;

	if (NULL == pal_StringNCopy(iotaState.contID, 64, ids.pContributionId, pal_StringLength(ids.pContributionId)))
	{
		iota_test_printf("Memory copy error.\n");
		pal_MutexUnlock(iotaState.mutexHandle);
		return 1;
	}
	if (NULL == pal_StringNCopy(iotaState.convID, 64, ids.pConversationId, pal_StringLength(ids.pContributionId)))
	{
		iota_test_printf("Memory copy error.\n");
		pal_MutexUnlock(iotaState.mutexHandle);
		return 1;
	}

	sendMessg.pMessage->eContentType = EcrioCPMContentTypeEnum_IMDN;
	message.message.pIMDN = &imdn;
	imdn.uNumOfNotifBody = 1;
	pal_MemoryAllocate(sizeof(EcrioCPMIMDispoNotifBodyStruct)*imdn.uNumOfNotifBody, (void**)&imdn.pCPMIMDispoNotifBody);
	if (imdn.pCPMIMDispoNotifBody == NULL)
	{
		iota_test_printf("Memory allocation error.\n");
		pal_MutexUnlock(iotaState.mutexHandle);
		return 1;
	}
	pDispoNotif = &imdn.pCPMIMDispoNotifBody[0];
	pDispoNotif->eDelivery = EcrioCPMIMDNDeliveryNotif_Delivered;
	pDispoNotif->eDisNtf = EcrioCPMIMDispositionNtfTypeReq_Display;//		Delivery;
	pDispoNotif->eDisplay = EcrioCPMIMDNDisplayNotif_Displayed;
	pDispoNotif->pIMDNMsgId = (char*) "DUpjRCHsZKFA5lrDmDNGi5MIeEgdP";

	pal_MutexUnlock(iotaState.mutexHandle);
	error = lims_SendStandAloneMessage(iotaState.limsHandle, &sendMessg, &pCallId);
	pal_MutexLock(iotaState.mutexHandle);
	if (error != LIMS_NO_ERROR)
	{
		iota_test_printf("lims_SendStandAloneMessage() failed.\n");
	}
	else
	{
		iota_test_printf("lims_SendStandAloneMessage() success.\n");
	}
	if (pCallId)
	{
		iota_test_printf("Call id: %s \n", pCallId);
		pal_MemoryFree((void**)&pCallId);
	}

	if (pDispoNotif)
		pal_MemoryFree((void**)&pDispoNotif);

	pal_MutexUnlock(iotaState.mutexHandle);
	return error;
}

unsigned int iota_test_SendMessage
(
iotaMessageStruct message
)
{
	pal_MutexLock(iotaState.mutexHandle);
	unsigned int error = LIMS_NO_ERROR;
	lims_SendMessageStruct sendMessg;
	EcrioCPMBufferStruct text;
	EcrioCPMBotSuggestionStruct suggestion;
	char *pMessageId = NULL;
	EcrioCPMMessageStruct cpmmessage;
	EcrioCPMIMDispoNotifStruct imdn;

	memset(&sendMessg, 0, sizeof(lims_SendMessageStruct));
	memset(&text, 0, sizeof(EcrioCPMBufferStruct));
	memset(&suggestion, 0, sizeof(EcrioCPMBotSuggestionStruct));
	memset(&cpmmessage, 0, sizeof(EcrioCPMMessageStruct));
	memset(&imdn, 0, sizeof(EcrioCPMIMDispoNotifStruct));

	sendMessg.pSessionId = iotaState.pSessionId;
	sendMessg.pMessage = &cpmmessage;
	sendMessg.pMessage->imdnConfig = EcrioCPMIMDispositionConfigPositiveDelivery | EcrioCPMIMDispositionConfigDisplay;
	sendMessg.pMessage->pIMDNMsgId = (char*) "DUpjRCHsZKFA5lrDmDNGi5MIeEgdP";

	sendMessg.pMessage->message.pBuffer = &text;
	sendMessg.pMessage->eContentType = EcrioCPMContentTypeEnum_Text;

	text.pMessage = (u_char *)message.stData.Message.c_str();
	text.uMessageLen = message.stData.uMsgLen;
	if (message.stBot.Json.compare(""))
	{
		sendMessg.pMessage->pBotSuggestion = &suggestion;
		suggestion.bIsBotSuggestionResponse = (BoolEnum)message.stBot.bisSCLResponse;
		suggestion.pJson = (u_char*)message.stBot.Json.c_str();
		suggestion.uJsonLen = message.stBot.uJsonLen;
	}

	iota_test_printf("Calling lims_SendMessage()\n");

	pal_MutexUnlock(iotaState.mutexHandle);
//	error = lims_SendMessage(iotaState.limsHandle, &sendMessg, &pMessageId);
	pal_MutexLock(iotaState.mutexHandle);
	if (error != LIMS_NO_ERROR)
	{
		iota_test_printf("lims_SendMessage() failed.\n");
	}
	else
	{
		iota_test_printf("lims_SendMessage() success.\n");
	}
	if (pMessageId)
	{
		iota_test_printf("MSRP Id: %s\n", pMessageId);
		pal_MemoryFree((void**)&pMessageId);
	}

	pal_MutexUnlock(iotaState.mutexHandle);
	return error;
}

unsigned int iota_test_SendIMDNMessage
(
iotaMessageStruct message
)
{
	pal_MutexLock(iotaState.mutexHandle);
	unsigned int error = LIMS_NO_ERROR;
	lims_SendMessageStruct sendMessg;
	EcrioCPMBufferStruct text;
	EcrioCPMBotSuggestionStruct suggestion;
	char *pMessageId = NULL;
	EcrioCPMMessageStruct cpmmessage;
	EcrioCPMIMDispoNotifStruct imdn;
	EcrioCPMIMDispoNotifBodyStruct* pDispoNotif = NULL;

	memset(&sendMessg, 0, sizeof(lims_SendMessageStruct));
	memset(&text, 0, sizeof(EcrioCPMBufferStruct));
	memset(&suggestion, 0, sizeof(EcrioCPMBotSuggestionStruct));
	memset(&cpmmessage, 0, sizeof(EcrioCPMMessageStruct));
	memset(&imdn, 0, sizeof(EcrioCPMIMDispoNotifStruct));

	sendMessg.pSessionId = iotaState.pSessionId;
	sendMessg.pMessage = &cpmmessage;
	sendMessg.pMessage->imdnConfig = EcrioCPMIMDispositionConfigPositiveDelivery | EcrioCPMIMDispositionConfigDisplay;
	if (iotaState.bIsGroupChat)
	{
		sendMessg.pMessage->pDestURI = (char*)"imdn-to-uri@example.com";
	}
	sendMessg.pMessage->pIMDNMsgId = (char*) "DUpjRCHsZKFA5lrDmDNGi5MIeEgdP";

	sendMessg.pMessage->message.pBuffer = &text;
	sendMessg.pMessage->eContentType = EcrioCPMContentTypeEnum_Text;

	text.pMessage = (u_char *)message.stData.Message.c_str();
	text.uMessageLen = message.stData.uMsgLen;
	if (message.stBot.Json.compare(""))
	{
		sendMessg.pMessage->pBotSuggestion = &suggestion;
		suggestion.bIsBotSuggestionResponse = (BoolEnum)message.stBot.bisSCLResponse;
		suggestion.pJson = (u_char*)message.stBot.Json.c_str();
		suggestion.uJsonLen = message.stBot.uJsonLen;
	}

	iota_test_printf("Calling lims_SendMessage()\n");

	sendMessg.pMessage->eContentType = EcrioCPMContentTypeEnum_IMDN;
	sendMessg.pMessage->message.pIMDN = &imdn;
	imdn.uNumOfNotifBody = 1;
	pal_MemoryAllocate(sizeof(EcrioCPMIMDispoNotifBodyStruct)*imdn.uNumOfNotifBody, (void**)&imdn.pCPMIMDispoNotifBody);
	if (imdn.pCPMIMDispoNotifBody == NULL)
	{
		iota_test_printf("Memory allocation error.\n");
		pal_MutexUnlock(iotaState.mutexHandle);
		return 1;
	}
	pDispoNotif = &imdn.pCPMIMDispoNotifBody[0];
	pDispoNotif->eDelivery = EcrioCPMIMDNDeliveryNotif_Delivered;
	pDispoNotif->eDisNtf = EcrioCPMIMDispositionNtfTypeReq_Delivery;
	pDispoNotif->eDisplay = EcrioCPMIMDNDisplayNotif_Displayed;
	pDispoNotif->pIMDNMsgId = (char*) "DUpjRCHsZKFA5lrDmDNGi5MIeEgdP";

	pal_MutexUnlock(iotaState.mutexHandle);
//	error = lims_SendMessage(iotaState.limsHandle, &sendMessg, &pMessageId);
	pal_MutexLock(iotaState.mutexHandle);
	if (error != LIMS_NO_ERROR)
	{
		iota_test_printf("lims_SendMessage() failed.\n");
	}
	else
	{
		iota_test_printf("lims_SendMessage() success.\n");
	}
	if (pMessageId)
	{
		iota_test_printf("MSRP Id: %s\n", pMessageId);
		pal_MemoryFree((void**)&pMessageId);
	}

	if (pDispoNotif)
		pal_MemoryFree((void**)&pDispoNotif);

	pal_MutexUnlock(iotaState.mutexHandle);
	return error;
}

unsigned int iota_test_SendIsComposing
(
iotaMessageStruct message
)
{
	pal_MutexLock(iotaState.mutexHandle);
	unsigned int error = LIMS_NO_ERROR;
	lims_SendMessageStruct sendMessg;
	char *pMessageId = NULL;
	EcrioCPMMessageStruct cpmmessage;
	EcrioCPMComposingStruct isComp;// = { 0 };
	(void)message;

	memset(&sendMessg, 0, sizeof(lims_SendMessageStruct));
	memset(&cpmmessage, 0, sizeof(EcrioCPMMessageStruct));

	sendMessg.pSessionId = iotaState.pSessionId;
	sendMessg.pMessage = &cpmmessage;
	sendMessg.pMessage->imdnConfig = EcrioCPMIMDispositionConfigPositiveDelivery | EcrioCPMIMDispositionConfigDisplay;
	sendMessg.pMessage->pIMDNMsgId = (char*) "DUpjRCHsZKFA5lrDmDNGi5MIeEgdP";

	iota_test_printf("Calling lims_SendMessage()\n");

	sendMessg.pMessage->eContentType = EcrioCPMContentTypeEnum_Composing;
	sendMessg.pMessage->message.pComposing = &isComp;

	isComp.pContent = pal_StringCreate((u_char*)"text/plain", pal_StringLength((u_char*)"text/plain"));
	isComp.pLastActive = NULL;
	isComp.uRefresh = 60;
	isComp.eComposingState = EcrioCPMComposingType_Active;

	pal_MutexUnlock(iotaState.mutexHandle);
	//error = lims_SendMessage(iotaState.limsHandle, &sendMessg, &pMessageId);
	pal_MutexLock(iotaState.mutexHandle);
	if (error != LIMS_NO_ERROR)
	{
		iota_test_printf("lims_SendMessage() failed.\n");
	}
	else
	{
		iota_test_printf("lims_SendMessage() success.\n");
	}
	if (pMessageId)
	{
		iota_test_printf("MSRP Id: %s\n", pMessageId);
		pal_MemoryFree((void**)&pMessageId);
	}
	pal_MemoryFree((void**)&isComp.pContent);
	pal_MutexUnlock(iotaState.mutexHandle);
	return error;
}

unsigned int iota_test_SendRichCard
(
iotaMessageStruct message
)
{
	pal_MutexLock(iotaState.mutexHandle);
	unsigned int error = LIMS_NO_ERROR;
	lims_SendMessageStruct sendRich;
	EcrioCPMBufferStruct RichCard;;
	EcrioCPMBotSuggestionStruct suggestion;
	EcrioCPMMessageStruct cpmmessage;
	char *pMessageId = NULL;

	memset(&sendRich, 0, sizeof(lims_SendMessageStruct));
	memset(&RichCard, 0, sizeof(EcrioCPMBufferStruct));
	memset(&suggestion, 0, sizeof(EcrioCPMBotSuggestionStruct));
	memset(&cpmmessage, 0, sizeof(EcrioCPMMessageStruct));

	sendRich.pSessionId = iotaState.pSessionId;
	sendRich.pMessage = &cpmmessage;
	sendRich.pMessage->imdnConfig = EcrioCPMIMDispositionConfigPositiveDelivery | EcrioCPMIMDispositionConfigDisplay;
	
	cpmmessage.pIMDNMsgId = (char*) "DUpjRCHsZKFA5lrDmDNGi5MIeEgdP";
	cpmmessage.message.pBuffer = &RichCard;
	cpmmessage.eContentType = EcrioCPMContentTypeEnum_RichCard;
	RichCard.pMessage = (u_char *)message.stData.Message.c_str();
	RichCard.uMessageLen = message.stData.uMsgLen;

	if (message.stBot.Json.compare(""))
	{
		cpmmessage.pBotSuggestion = &suggestion;
		suggestion.bIsBotSuggestionResponse = (BoolEnum)message.stBot.bisSCLResponse;
		suggestion.pJson = (u_char *)message.stBot.Json.c_str();
		suggestion.uJsonLen = message.stBot.uJsonLen;
	}
	iota_test_printf("Calling lims_SendMessage()\n");

	pal_MutexUnlock(iotaState.mutexHandle);
//	error = lims_SendMessage(iotaState.limsHandle, &sendRich, &pMessageId);
	pal_MutexLock(iotaState.mutexHandle);
	if (error != LIMS_NO_ERROR)
	{
		iota_test_printf("lims_SendRichCard() failed.\n");
	}
	else
	{
		iota_test_printf("lims_SendRichCard() success.\n");
	}
	if (pMessageId)
	{
		iota_test_printf("MSRP Id: %s\n", pMessageId);
		pal_MemoryFree((void**)&pMessageId);
	}
	pal_MutexUnlock(iotaState.mutexHandle);
	return error;
}


void iota_test_getContributionID(u_char* pContributionID)
{
	u_char tempBuf[13];

	memset(tempBuf, 0, 13);
	iota_test_getRandomStringHex(tempBuf, 8);
	if (NULL == pal_StringNCopy(pContributionID, 64, (const u_char *)tempBuf, pal_StringLength((const u_char *)tempBuf)))
	{
		iota_test_printf("Memory copy error.\n");
		return;
	}
	if (NULL == pal_StringNConcatenate(pContributionID, 64 - pal_StringLength((const u_char *)pContributionID), (const u_char *)"-", pal_StringLength((const u_char *)"-")))
	{
		iota_test_printf("Memory copy error.\n");
		return;
	}
	memset(tempBuf, 0, 13);
	iota_test_getRandomStringHex(tempBuf, 4);
	if (NULL == pal_StringNConcatenate(pContributionID, 64 - pal_StringLength((const u_char *)pContributionID), tempBuf, pal_StringLength((const u_char *)tempBuf)))
	{
		iota_test_printf("Memory copy error.\n");
		return;
	}
	if (NULL == pal_StringNConcatenate(pContributionID, 64 - pal_StringLength((const u_char *)pContributionID), (const u_char *)"-", pal_StringLength((const u_char *)"-")))
	{
		iota_test_printf("Memory copy error.\n");
		return;
	}
	memset(tempBuf, 0, 13);
	iota_test_getRandomStringHex(tempBuf, 4);
	if (NULL == pal_StringNConcatenate(pContributionID, 64 - pal_StringLength((const u_char *)pContributionID), tempBuf, pal_StringLength(tempBuf)))
	{
		iota_test_printf("Memory copy error.\n");
		return;
	}
	if (NULL == pal_StringNConcatenate(pContributionID, 64 - pal_StringLength((const u_char *)pContributionID), (const u_char *)"-", pal_StringLength((const u_char *)"-")))
	{
		iota_test_printf("Memory copy error.\n");
		return;
	}
	memset(tempBuf, 0, 13);
	iota_test_getRandomStringHex(tempBuf, 4);
	if (NULL == pal_StringNConcatenate(pContributionID, 64 - pal_StringLength((const u_char *)pContributionID), tempBuf, pal_StringLength((const u_char *)tempBuf)))
	{
		iota_test_printf("Memory copy error.\n");
		return;
	}
	if (NULL == pal_StringNConcatenate(pContributionID, 64 - pal_StringLength((const u_char *)pContributionID), (const u_char *)"-", pal_StringLength((const u_char *)"-")))
	{
		iota_test_printf("Memory copy error.\n");
		return;
	}
	memset(tempBuf, 0, 13);
	iota_test_getRandomStringHex(tempBuf, 12);
	if (NULL == pal_StringNConcatenate(pContributionID, 64 - pal_StringLength((const u_char *)pContributionID), tempBuf, pal_StringLength(tempBuf)))
	{
		iota_test_printf("Memory copy error.\n");
		return;
	}
}

void iota_test_printContentTypes(u_int32 contentTypes)
{
	iota_test_printf("\nSupported content types\n");
	if (contentTypes == EcrioCPMContentTypeEnum_FileTransferOverMSRP)
	{
		iota_test_printf("%s\n", "File Transfer");
	}
	else
	{
		if ((contentTypes & EcrioCPMContentTypeEnum_Text) == EcrioCPMContentTypeEnum_Text)
			iota_test_printf("%s\n", "Plain Text");
		if ((contentTypes & EcrioCPMContentTypeEnum_IMDN) == EcrioCPMContentTypeEnum_IMDN)
			iota_test_printf("%s\n", "IMDN");
		if ((contentTypes & EcrioCPMContentTypeEnum_FileTransferOverHTTP) == EcrioCPMContentTypeEnum_FileTransferOverHTTP)
			iota_test_printf("%s\n", "File Transfer HTTP");
		if ((contentTypes & EcrioCPMContentTypeEnum_RichCard) == EcrioCPMContentTypeEnum_RichCard)
			iota_test_printf("%s\n", "botmessage");
		if ((contentTypes & EcrioCPMContentTypeEnum_SuggestedChipList) == EcrioCPMContentTypeEnum_SuggestedChipList)
			iota_test_printf("%s\n", "botsuggestion");
		if ((contentTypes & EcrioCPMContentTypeEnum_SuggestionResponse) == EcrioCPMContentTypeEnum_SuggestionResponse)
			iota_test_printf("%s\n", "botsuggestion.response");
		if ((contentTypes & EcrioCPMContentTypeEnum_PushLocation) == EcrioCPMContentTypeEnum_PushLocation)
			iota_test_printf("%s\n", "Push Location");
	}
} 

void iota_test_printConversationHeaders(EcrioCPMConversationsIdStruct* pID)
{
	if (pID)
	{
		iota_test_printf("Conversation Id: %s\n", pID->pConversationId);
		iota_test_printf("Contribution Id: %s\n", pID->pContributionId);
		iota_test_printf("InReplyTo-Contribution-Id: %s\n", pID->pInReplyToContId);
	}
}

void iota_test_printIMDNDispositionNotification(u_int32 imdnNtf)
{
	
	iota_test_printf("imdn.Disposition-Notification:");
	if ((imdnNtf & EcrioCPMIMDispositionConfigPositiveDelivery) == EcrioCPMIMDispositionConfigPositiveDelivery)
	{
		iota_test_printf("positive-delivery");
	}

	if ((imdnNtf & EcrioCPMIMDispositionConfigNegativeDelivery) == EcrioCPMIMDispositionConfigNegativeDelivery)
	{
		iota_test_printf("negative-delivery");
	}

	if ((imdnNtf & EcrioCPMIMDispositionConfigDisplay) == EcrioCPMIMDispositionConfigDisplay)
	{
		iota_test_printf(",display\n");
	}	
}



//#endif


#ifdef WIN32
static unsigned int WINAPI IPSECThread
(
	LPVOID lpParam
)
{
	unsigned int error = 0;
	BOOL bReturn;
	iotaTestStateStruct *pIotaState;
	lims_NetworkConnectionStatus status;
	lims_NetworkConnectionStruct network = { 0 };
	MSG msg;

	/* Check the parameters for validity. */
	if (lpParam == NULL)
	{
		/* We need the parameter to operate! Not likely to happen. */
		return 1;
	}

	pIotaState = (iotaTestStateStruct*)lpParam;

	/* Force the system to create the message queue. */
	bReturn = PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	/* Starting the message loop for this thread, fire the event to let the
	initialization code know the thread is ready. If the event handle is
	NULL for some reason, ignore it. */
	if (pIotaState->hThreadReady != NULL)
	{
		bReturn = SetEvent(pIotaState->hThreadReady);
		/* Ignore return value. */
	}

	/** This function implements a message loop and uses the GetMessage()
	function to block and wait for messages sent to the thread. */
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		/* Check the msg structure for type of message. */
		switch (msg.message)
		{
			/** The SIGNAL_SEND_EVENT message is handled which indicates that
			a signal should be processed. */
		case SIGNAL_SEND_EVENT:
		{
			/* The wParam should be the signalId. */
			status = (lims_NetworkConnectionStatus)msg.wParam;
			lims_NetworkConnectionStruct network = { 0 };
			char *pLocalIp = NULL;

			if (iota_test_config_getNetwork(pIotaState->configFile, &network) != 0)
			{
				return 0;
			}

			if (network.uNoOfRemoteIps >= 1)
			{
				if (strstr(network.ppRemoteIPs[0], ":") != NULL)
				{
					pLocalIp = iota_test_getLocalIpAddress(6, pIotaState->localInterface);
					if (pLocalIp != NULL)
					{
						strncpy((char *)pIotaState->localAddress, pLocalIp, strlen(pLocalIp));
						free(pLocalIp);
					}
					else
					{
						return 0;
					}

					network.pLocalIp = (char *)pIotaState->localAddress;
				}
				else
				{
					pLocalIp = iota_test_getLocalIpAddress(4, pIotaState->localInterface);
					if (pLocalIp != NULL)
					{
						strncpy((char *)pIotaState->localAddress, pLocalIp, strlen(pLocalIp));
						free(pLocalIp);
					}
					else
					{
						return 0;
					}

					network.pLocalIp = (char *)pIotaState->localAddress;
				}
			}

			if (strstr(network.pLocalIp, ":") != NULL)
			{
				network.eIPType = lims_Network_IP_Type_V6;
			}
			else
			{
				network.eIPType = lims_Network_IP_Type_V4;
			}
			network.uStatus = status;// lims_Network_Status_IPsec_Established;

			error = lims_NetworkStateChange(pIotaState->limsHandle, lims_Network_PDN_Type_IMS, lims_Network_Connection_Type_LTE, &network);
			if (error != LIMS_NO_ERROR)
			{
				iota_test_printf("lims_NetworkStateChange failed \n");
			}

		}
		break;

		/** The SIGNAL_EXIT_EVENT event instructs the thread to exit. */
		case SIGNAL_EXIT_EVENT:
		{
			_endthreadex(0);

		}
		break;

		default:
			break;
		}
	}

	_endthreadex(0);

	return 0;
}
#else
static unsigned int iota_test_IPSECThread
(
IOTA_TEST_SIGNAL_HANDLE handle,
unsigned int command,
unsigned int uParam,
void* pParam
)
{
	unsigned int error = 0;
	(void)handle;
	(void)uParam;
	(void)pParam;

	switch (command)
	{
	case IOTA_TEST_IPSEC_ESTABLISHED:
	default:
	{
		/* The wParam should be the signalId. */
		lims_NetworkConnectionStruct network;
		char *pLocalIp = NULL;

		memset(&network, 0, sizeof(lims_NetworkConnectionStruct));
		if (iota_test_config_getNetwork(iotaState.configFile, &network) != 0)
		{
			return 0;
		}

		if (network.uNoOfRemoteIps >= 1)
		{
			if (strstr(network.ppRemoteIPs[0], ":") != NULL)
			{
				pLocalIp = iota_test_getLocalIpAddress(6, iotaState.localInterface);
				if (pLocalIp != NULL)
				{
					strncpy((char *)iotaState.localAddress, pLocalIp, strlen(pLocalIp));
					free(pLocalIp);
				}
				else
				{
					return 0;
				}

				network.pLocalIp = (char *)iotaState.localAddress;
			}
			else
			{
				pLocalIp = iota_test_getLocalIpAddress(4, iotaState.localInterface);
				if (pLocalIp != NULL)
				{
					strncpy((char *)iotaState.localAddress, pLocalIp, strlen(pLocalIp));
					free(pLocalIp);
				}
				else
				{
					return 0;
				}

				network.pLocalIp = (char *)iotaState.localAddress;
			}
		}

		if (strstr(network.pLocalIp, ":") != NULL)
		{
			network.eIPType = lims_Network_IP_Type_V6;
		}
		else
		{
			network.eIPType = lims_Network_IP_Type_V4;
		}
		network.uStatus = lims_Network_Status_IPsec_Established;

		error = lims_NetworkStateChange(iotaState.limsHandle, lims_Network_PDN_Type_IMS, lims_Network_Connection_Type_LTE, &network);
		if (error != LIMS_NO_ERROR)
		{
			iota_test_printf("lims_NetworkStateChange failed \n");
		}
	}
	break;
	}

	return 0;
}
#endif




#ifdef ENABLE_RCS
#if 0
unsigned int iota_test_lims_PublishCapabilities
(
)
{
	unsigned int error = LIMS_NO_ERROR;
	EcrioUCEUserCapabilityInfoStruct userCapabilities = {};
	EcrioUCEServiceDetailsStruct *pServices = NULL;
	int i = 0;
	EcrioUCEVoLTEServiceDuplexStruct stDuplex = {};

	iota_test_printf("Calling lims_PublishCapabilities()\n");

	/* generate Capability structure*/
	userCapabilities.uServiceCount = 7;

	pal_MemoryAllocate(sizeof(EcrioUCEServiceDetailsStruct)*(userCapabilities.uServiceCount), (void **)&pServices);
	if (pServices == NULL)
	{
		iota_test_printf("Memory allocation error.\n");
		return 1;
	}

	userCapabilities.pServices = pServices;

	string message;


	for (i = 0; i < userCapabilities.uServiceCount; i++){
		switch (i){
		case 0:

			cout << "Support Messaging ?(y/n) \n";
			getline(cin, message);
			if (message == "y")
				pServices[i].eID = ECRIO_UCE_ServiceID_StandaloneMessaging;
			break;
		case 1:
			cout << "Support Chat ?(y/n) \n";
			getline(cin, message);
			if (message == "y")
				pServices[i].eID = ECRIO_UCE_ServiceID_Chat;
			break;
		case 2:
			cout << "Support FileTransferHTTP ?(y/n) \n";
			getline(cin, message);
			if (message == "y")
				pServices[i].eID = ECRIO_UCE_ServiceID_FileTransferHTTP;
			break;
		case 3:
			cout << "Support MSRP ?(y/n) \n";
			getline(cin, message);
			if (message == "y")
			{
				pServices[i].eID = ECRIO_UCE_ServiceID_FileTransferMSRP;
				pal_MemoryAllocate(sizeof(EcrioUCEURIStruct), (void**)&pServices[i].pContact);
				if (pServices[i].pContact == NULL)
				{
					iota_test_printf("Memory allocation error.\n");
					return 1;
				}
				pServices[i].numOfContacts = 1;
				pServices[i].pContact->eUriType = ECRIO_UCE_UriType_SIP;
			}
			break;
		case 4:
			cout << "Support Thumbnail ?(y/n) \n";
			getline(cin, message);
			if (message == "y"){
				pServices[i].eID = ECRIO_UCE_ServiceID_FileTransferThumbnail;
			}
			break;
		case 5:
			cout << "Support Query ?(y/n) \n";
			getline(cin, message);
			if (message == "y")
				pServices[i].eID = ECRIO_UCE_ServiceID_QueryCapsUsingPresence;
			break;
		case 6:
			cout << "Support : VoLTE - l , Video - v , None  - n";
			getline(cin, message);

			if (message == "l"){
				pServices[i].eID = ECRIO_UCE_ServiceID_VoLTE;


				stDuplex.bSupported = Enum_TRUE;

				cout << "Duples Support - Full - f, Half - h, Send Only - s , Receive Only - r , None -n \n";
				getline(cin, message);
				if (message == "f"){
					stDuplex.eDuplexType = EcrioUCE_VoLTE_SERVC_Full;
				}
				else if (message == "h"){
					stDuplex.eDuplexType = EcrioUCE_VoLTE_SERVC_Half;
				}
				else if (message == "r"){
					stDuplex.eDuplexType = EcrioUCE_VoLTE_SERVC_ReceiveOnly;
				}
				else if (message == "s"){
					stDuplex.eDuplexType = EcrioUCE_VoLTE_SERVC_SendOnly;
				}
				else{
					stDuplex.eDuplexType = EcrioUCE_VoLTE_SERVC_None;
				}


				pServices[i].volte.duplex = stDuplex;
				pServices[i].volte.bAudio = Enum_TRUE;

				pServices[i].volte.bServc = Enum_TRUE;
			}
			if (message == "v"){
				pServices[i].eID = ECRIO_UCE_ServiceID_Video;


				stDuplex.bSupported = Enum_TRUE;

				cout << "Duples Support - Full - f, Half - h, Send Only - s , Receive Only - r , None -n \n";
				getline(cin, message);
				if (message == "f"){
					stDuplex.eDuplexType = EcrioUCE_VoLTE_SERVC_Full;
				}
				else if (message == "h"){
					stDuplex.eDuplexType = EcrioUCE_VoLTE_SERVC_Half;
				}
				else if (message == "r"){
					stDuplex.eDuplexType = EcrioUCE_VoLTE_SERVC_ReceiveOnly;
				}
				else if (message == "s"){
					stDuplex.eDuplexType = EcrioUCE_VoLTE_SERVC_SendOnly;
				}
				else{
					stDuplex.eDuplexType = EcrioUCE_VoLTE_SERVC_None;
				}


				pServices[i].volte.duplex = stDuplex;
				pServices[i].volte.bAudio = Enum_TRUE;
				pServices[i].volte.bVideo = Enum_TRUE;

				pServices[i].volte.bServc = Enum_TRUE;
			}



			break;
		case 7:
			pServices[i].eID = ECRIO_UCE_ServiceID_Video;
			break;
		default:
			break;
		}
	}


	/* invokde the API */
	error = lims_PublishCapabilities(iotaState.limsHandle, NULL, &userCapabilities);
	if (error != LIMS_NO_ERROR)
	{
		iota_test_printf("lims_PublishCapabilities() failed.\n");
	}
	else
	{
		iota_test_printf("lims_PublishCapabilities() success.\n");
	}


	for (i = 0; i < userCapabilities.uServiceCount; i++){
		userCapabilities.pServices[i].eID = ECRIO_UCE_ServiceID_None;
		if (userCapabilities.pServices[i].numOfContacts > 0)
		{
			for (int j = 0; j < userCapabilities.pServices[i].numOfContacts; j++)
			{
				userCapabilities.pServices[i].pContact[j].eUriType = ECRIO_UCE_UriType_Unknown;
				pal_MemoryFree((void**)&userCapabilities.pServices[i].pContact[j].pURI);
			}

			pal_MemoryFree((void**)&userCapabilities.pServices[i].pContact);
		}
		
		userCapabilities.pServices[i].pContact = NULL;
	}

	pal_MemoryFree((void**)&userCapabilities.pServices);


	userCapabilities.pServices = NULL;
	userCapabilities.uServiceCount = 0;


	return error;
}
#endif

unsigned int iota_test_lims_ModifyCapabilities
(
)
{
	pal_MutexLock(iotaState.mutexHandle);
	unsigned int error = LIMS_NO_ERROR;
	EcrioUCEUserCapabilityInfoStruct userCapabilities = {};
	EcrioUCEServiceDetailsStruct *pServices = NULL;
	int i = 0;


	iota_test_printf("Calling lims_PublishCapabilities()\n");

	/* generate Capability structure*/
	userCapabilities.uServiceCount = 1;

	pal_MemoryAllocate(sizeof(EcrioUCEServiceDetailsStruct)*(userCapabilities.uServiceCount), (void **)&pServices);
	if (pServices == NULL)
	{
		iota_test_printf("Memory allocation error.\n");
		pal_MutexUnlock(iotaState.mutexHandle);
		return 1;
	}

	userCapabilities.pServices = pServices;

	for (i = 0; i < userCapabilities.uServiceCount; i++){
		switch (i){
		case 0:
			pServices[i].eID = ECRIO_UCE_ServiceID_StandaloneMessaging;
			pal_MemoryAllocate(sizeof(EcrioUCEURIStruct), (void**)&pServices[i].pContact);
			pServices[i].numOfContacts = 1;
			pServices[i].pContact->eUriType = ECRIO_UCE_UriType_SIP;
			break;
		case 1:
			pServices[i].eID = ECRIO_UCE_ServiceID_Chat;
			pal_MemoryAllocate(sizeof(EcrioUCEURIStruct), (void**)&pServices[i].pContact);
			pServices[i].numOfContacts = 1;
			pServices[i].pContact->eUriType = ECRIO_UCE_UriType_SIP;
			break;
		case 2:
			pServices[i].eID = ECRIO_UCE_ServiceID_FileTransferHTTP;
			break;
		case 3:
			pServices[i].eID = ECRIO_UCE_ServiceID_FileTransferMSRP;
			pal_MemoryAllocate(sizeof(EcrioUCEURIStruct), (void**)&pServices[i].pContact);
			pServices[i].numOfContacts = 1;
			pServices[i].pContact->eUriType = ECRIO_UCE_UriType_TEL;
			break;
		case 4:
			pServices[i].eID = ECRIO_UCE_ServiceID_FileTransferThumbnail;
			pal_MemoryAllocate(sizeof(EcrioUCEURIStruct), (void**)&pServices[i].pContact);
			pServices[i].numOfContacts = 1;
			pServices[i].pContact->eUriType = ECRIO_UCE_UriType_TEL;
			break;
		case 5:
			pServices[i].eID = ECRIO_UCE_ServiceID_QueryCapsUsingPresence;
			break;
		case 6:
			break;
		default:
			break;
		}
	}



	string inputstr;
	char strETag[32] = {};
	string message;

	cout << "Get Previous ETag?(y/n) \n";
	getline(cin, message);
	if (message == "y")
	{
		if (iotaState.pETag){
			cout << "Previous E-Tag:" << iotaState.pETag << "\n";;
			strcpy(strETag, iotaState.pETag);
		}
		else{
			cout << "Previous E-Tag Does Not Exixts" << "\n";
		}
	}
	else if (message == "n")
	{
		cout << "Provide E-Tag: ";
		getline(cin, inputstr);
		strcpy(strETag, inputstr.c_str());
	}
	else {
		cout << "Press y/n";
		pal_MutexUnlock(iotaState.mutexHandle);
		return error;
	}


	/* invokde the API */
	//error = lims_PublishCapabilities(iotaState.limsHandle, iotaState.pETag, &userCapabilities);

	pal_MutexUnlock(iotaState.mutexHandle);
//	error = lims_PublishCapabilities(iotaState.limsHandle, strETag, &userCapabilities);
	pal_MutexLock(iotaState.mutexHandle);
	if (error != LIMS_NO_ERROR)
	{
		iota_test_printf("lims_PublishCapabilities() failed.\n");
	}
	else
	{
		iota_test_printf("lims_PublishCapabilities() success.\n");
	}


	for (i = 0; i < userCapabilities.uServiceCount; i++){
		userCapabilities.pServices[i].eID = ECRIO_UCE_ServiceID_None;
		if (userCapabilities.pServices[i].numOfContacts > 0)
		{
			for (int j = 0; j < userCapabilities.pServices[i].numOfContacts; j++)
			{
				userCapabilities.pServices[i].pContact[j].eUriType = ECRIO_UCE_UriType_Unknown;
				pal_MemoryFree((void**)&userCapabilities.pServices[i].pContact[j].pURI);
			}

			pal_MemoryFree((void**)&userCapabilities.pServices[i].pContact);
		}
		userCapabilities.pServices[i].pContact = NULL;
	}

	pal_MemoryFree((void**)&userCapabilities.pServices);


	userCapabilities.pServices = NULL;
	userCapabilities.uServiceCount = 0;

	pal_MutexUnlock(iotaState.mutexHandle);
	return error;
}



unsigned int iota_test_lims_SetUserAgent
(
	u_char* pUserAgent
)
{
	unsigned int error = LIMS_NO_ERROR;

	iota_test_printf("Calling lims_SetUserAgent()\n");
	error = lims_SetUserAgent(iotaState.limsHandle, pUserAgent);
	if (error != LIMS_NO_ERROR)
	{
		iota_test_printf("lims_SetUserAgent() failed.\n");
	}
	else
	{
		iota_test_printf("lims_SetUserAgent() success.\n");
	}

	return error;
}

#if 0
unsigned int iota_test_QueryCapabilitiesOptions
(
void
)
{
	u_int32 error = LIMS_NO_ERROR;
	EcrioUCEOptionsRequestStruct	stOptons = {};
	u_int32 uOptonsFeatures=0;
	EcrioUCEQueryCapabilitiesStruct stQuaryCapability = {};
	char *toURI = NULL;
	char tmp[128] = {0};
 	
	u_int32 i = 0;
	string message;


	iota_test_printf("Calling u_int32 lims_QueryCapabilities()\n");



	cout << "Provide destination number (e.g. 6505550056): ";
	getline(cin, message);
	pal_MemorySet(tmp, 0, 128);
	strcpy(tmp, message.c_str());
	toURI = (char*)malloc(strlen(tmp) + 1);
	strcpy(toURI, tmp);
	


	if (toURI && strlen(toURI)){

		cout << "Support Messaging ?(y/n) \n";
		getline(cin, message);
		if (message == "y")
			uOptonsFeatures = uOptonsFeatures | ECRIO_UCE_ServiceID_StandaloneMessaging;


		cout << "Support VoLTE ?(y/n) \n";
		getline(cin, message);
		if (message == "y")
			uOptonsFeatures = uOptonsFeatures | ECRIO_UCE_ServiceID_VoLTE;


		cout << "Support Video ?(y/n) \n";
		getline(cin, message);
		if (message == "y")
			uOptonsFeatures = uOptonsFeatures | ECRIO_UCE_ServiceID_Video;
		cout << "Support Chat ?(y/n) \n";
		getline(cin, message);
		if (message == "y")
			uOptonsFeatures = uOptonsFeatures | ECRIO_UCE_ServiceID_Chat;

		cout << "Support FileTransferHTTP ?(y/n) \n";
		getline(cin, message);
		if (message == "y")
			uOptonsFeatures = uOptonsFeatures | ECRIO_UCE_ServiceID_FileTransferHTTP;

		
		cout << "Support FileTransfer SMS ?(y/n) \n";
		getline(cin, message);
		if (message == "y")
			uOptonsFeatures = uOptonsFeatures | ECRIO_UCE_ServiceID_FileTransferSMS;


		cout << "Support Geo Location Push ?(y/n) \n";
		getline(cin, message);
		if (message == "y")
			uOptonsFeatures = uOptonsFeatures | ECRIO_UCE_ServiceID_GeolocationPush;


		cout << "Support Geo Location Push Via SMS ?(y/n) \n";
		getline(cin, message);
		if (message == "y")
			uOptonsFeatures = uOptonsFeatures | ECRIO_UCE_ServiceID_GeolocationPushSMS;


		cout << "Support Chatbot Role ?(y/n) \n";
		getline(cin, message);
		if (message == "y")
			uOptonsFeatures = uOptonsFeatures | ECRIO_UCE_ServiceID_Chatbot;

		cout << "Support Chatbot using Session ?(y/n) \n";
		getline(cin, message);
		if (message == "y")
			uOptonsFeatures = uOptonsFeatures | ECRIO_UCE_ServiceID_ChatbotUsingSession;



		cout << "Support Chatbot using sntadalone messgae ?(y/n) \n";
		getline(cin, message);
		if (message == "y")
			uOptonsFeatures = uOptonsFeatures | ECRIO_UCE_ServiceID_ChatbotUsingStandaloneMessage;

		cout << "Support Shared MAP ?(y/n) \n";
		getline(cin, message);
		if (message == "y")
			uOptonsFeatures = uOptonsFeatures | ECRIO_UCE_ServiceID_SharedMap;

		cout << "Support Shared SKETCHE ?(y/n) \n";
		getline(cin, message);
		if (message == "y")
			uOptonsFeatures = uOptonsFeatures | ECRIO_UCE_ServiceID_SharedSketch;



		cout << "Support call composer ?(y/n) \n";
		getline(cin, message);
		if (message == "y")
			uOptonsFeatures = uOptonsFeatures | ECRIO_UCE_ServiceID_CallComposerEnrichedCalling;


		cout << "Support post call ?(y/n) \n";
		getline(cin, message);
		if (message == "y")
			uOptonsFeatures = uOptonsFeatures | ECRIO_UCE_ServiceID_PostCall;

		/* generate Capability structure*/
		stOptons.pCallId = NULL;
		stOptons.eURIType = ECRIO_UCE_UriType_SIP;
		stOptons.pDest = (u_char*)toURI;
		stOptons.uOptionsFeatures = uOptonsFeatures;

		stQuaryCapability.eQueryType = ECRIO_UCE_Capabilty_Type_OPTIONS;
		stQuaryCapability.u.pOptions = &stOptons;


		/* invokde the API */
		error = lims_QueryCapabilities(iotaState.limsHandle, &stQuaryCapability);

		free(toURI);

		if (error != LIMS_NO_ERROR)
			iota_test_printf("lims_QueryCapabilities() failed.\n")
		else
			iota_test_printf("lims_QueryCapabilities() success.\n")
	}

	return error;
}
#endif

u_int32 limsUCEPropertyCallback
(
EcrioUCEPropertyNameEnums eName,
void *pData,
void *pContext
)
{
	// All the values needs to retrieve from
	u_int32 error = 0;

	(void)pContext;

	switch (eName)
	{
		case EcrioUCEPropertyName_OPTIONSFEATURETAG:
		{

			u_int32 *pFeatureList = (u_int32 *)pData;
			if (pFeatureList != NULL)
			{
				

				*pFeatureList = ECRIO_UCE_ServiceID_FileTransferHTTP |
					ECRIO_UCE_ServiceID_FileTransferSMS |
					ECRIO_UCE_ServiceID_GeolocationPush |
					ECRIO_UCE_ServiceID_GeolocationPushSMS |
					ECRIO_UCE_ServiceID_ChatbotUsingSession |
					ECRIO_UCE_ServiceID_ChatbotUsingStandaloneMessage |
					ECRIO_UCE_ServiceID_PostCall |
					ECRIO_UCE_ServiceID_CallComposerEnrichedCalling |
					ECRIO_UCE_ServiceID_SharedMap |
					ECRIO_UCE_ServiceID_SharedSketch;
			}
		}
		break;
		default:
		{
		}
		break;
	}

	return error;
}

#endif
