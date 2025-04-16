/******************************************************************************

Copyright (c) 2015-2025 Ecrio, Inc. All Rights Reserved.

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <emscripten.h>

#include "EcrioPAL.h"
#include "lims.h"

#include "lims_wrapper.h"
#ifndef LIMS_ERROR_MEMORY
#define LIMS_ERROR_MEMORY 1001  // Or any unused error code
#endif


#if 0
typedef struct
{
	LIMSHANDLE limsHandle;
	LOGHANDLE logHandle;
	MUTEXHANDLE mutexHandle;

	char *pSessionId;
	char *pGroupSessionId;
	char *pReferId;
	u_int32 eContentTypes; //bitmask with EcrioCPMContentTypeEnum

	PALINSTANCE palLimsInstance;

	char localInterface[64];

	char localAddress[64];

	char calleeNumber[32];

	char confFactoryUri[128];

	char configFile[128];

	char message[160];

	int codecListType;

	unsigned char seed[63];
	unsigned char seedHex[17];
	unsigned char localPath[64];
	unsigned char remotePath[64];

	bool bRegistered;
	bool bAirplaneModeOn;
	bool bIsLargeMode;
	unsigned char convID[64];
	unsigned char contID[64];
	unsigned char inReplyTo[64];

	char paniType[64];
	char paniInfo[64];
	char planiType[64];
	char planiInfo[64];

} iotaTestStateStruct;
#endif

iotaTestStateStruct iotaState;


EMSCRIPTEN_KEEPALIVE void iota_test_printConversationHeaders(EcrioCPMConversationsIdStruct* pID)
{
	if (pID)
	{
		iota_test_printf("Conversation Id: %s\n", pID->pConversationId);
		iota_test_printf("Contribution Id: %s\n", pID->pContributionId);
		iota_test_printf("InReplyTo-Contribution-Id: %s\n", pID->pInReplyToContId);
	}
}

EMSCRIPTEN_KEEPALIVE void iota_test_printIMDNDispositionNotification(u_int32 imdnNtf)
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


EMSCRIPTEN_KEEPALIVE void iota_test_getRandomString
(
	unsigned char *pStr,
	unsigned int uLength
)
{
	unsigned int i = 0;

	for (i = 0; i < uLength; i++)
	{
		*(pStr + i) = iotaState.seed[pal_UtilityRandomNumber() % 62];
	}
}

EMSCRIPTEN_KEEPALIVE unsigned int iota_test_dettach_network
(
	void
)
{
	pal_MutexLock(iotaState.mutexHandle);
	unsigned int error = LIMS_NO_ERROR;

	iotaState.bRegistered = false;
	printf("ims pdn detached.\n");
	pal_MutexUnlock(iotaState.mutexHandle);
	error = lims_NetworkStateChange(iotaState.limsHandle, lims_Network_PDN_Type_IMS, lims_Network_Connection_Type_NONE, NULL);
	pal_MutexLock(iotaState.mutexHandle);
	if (error != LIMS_NO_ERROR)
	{
		printf("lims_NetworkStateChange() Failed.\n");
	}
	else
	{
		printf("network disconnected\n");
	}

	pal_MutexUnlock(iotaState.mutexHandle);
	return error;
}

EMSCRIPTEN_KEEPALIVE void handleStatusCallback
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
		printf("REGISTERED\n");
		iotaState.bRegistered = true;
	}
	break;

	case lims_Notify_Type_REGISTER_FAILED:
	{
		printf("REGISTRATION FAILED\n");
		iotaState.bRegistered = false;
	}
	break;

	case lims_Notify_Type_DEREGISTERED:
	{
		EcrioSUENotifyRegisterResponseStruct *RegisterResponseStruct = (EcrioSUENotifyRegisterResponseStruct *)pData;
		if (RegisterResponseStruct != NULL)
			printf("DEREGISTERED %u\n", RegisterResponseStruct->uRspCode);

		iotaState.bRegistered = false;
		//			stop_timer();

		if (iotaState.bAirplaneModeOn)
		{
			iota_test_dettach_network();
			iotaState.bAirplaneModeOn = false;
		}
	}
	break;

	case lims_Notify_Type_NOTIFY_RECEIVED:
	{
		EcrioSUENotifyNotifyRequestStruct *pNotifReqStruct = (EcrioSUENotifyNotifyRequestStruct *)pData;

		if (pNotifReqStruct != NULL)
		{

			if (pNotifReqStruct->eRegInfoContactState == ECRIO_SUE_REGINFO_CONTACT_STATE_ENUM_Active)
			{
				printf("ECRIO_SUE_REGINFO_CONTACT_STATE_ENUM_Active\n");
			}
			else if (pNotifReqStruct->eRegInfoContactState == ECRIO_SUE_REGINFO_CONTACT_STATE_ENUM_Terminated)
			{
				printf("ECRIO_SUE_REGINFO_CONTACT_STATE_ENUM_Terminated\n");
			}

			printf("contact event=%u\n", pNotifReqStruct->eRegInfoContactEvent);
			printf("contact Expires=%u\n", pNotifReqStruct->uExpires);
			printf("contact Retry After=%u\n", pNotifReqStruct->uRetryAfter);
		}
	}
	break;

	case lims_Notify_Type_SOCKET_ERROR:
	{
		printf("Received Socket Error - Re-Initalize lims\n");
	}
	break;

	case lims_Notify_Type_STAND_ALONE_MESSAGE_SENT:
	{
		EcrioCPMStandAloneMessageIDStruct *pStandAloneMessageID = (EcrioCPMStandAloneMessageIDStruct*)pData;

		printf("StandAlone message sent!\n");
		printf("Call Id- %s\n", pStandAloneMessageID->pCallId);
	}
	break;

	case lims_Notify_Type_STAND_ALONE_MESSAGE_FAILED:
	{
		EcrioCPMStandAloneMessageIDStruct *pStandAloneMessageID = (EcrioCPMStandAloneMessageIDStruct*)pData;

		printf("StandAlone message sending failed!\n");
		printf("statusCode: %d\n", pStandAloneMessageID->statusCode);
		printf("Reason: %s\n", pStandAloneMessageID->pszReason);
		printf("Call Id- %s\n", pStandAloneMessageID->pCallId);
		if (pStandAloneMessageID->uWarningCode != 0)
		{
			printf("Warning Code is %d\n", pStandAloneMessageID->uWarningCode);
		}
		if (pStandAloneMessageID->pWarningHostName != NULL)
		{
			printf("Warning Host Name is %s\n", pStandAloneMessageID->pWarningHostName);
		}
		if (pStandAloneMessageID->pWarningText != NULL)
		{
			printf("Warning Text is %s\n", pStandAloneMessageID->pWarningText);
		}

		if (pStandAloneMessageID->ppPAssertedIdentity)
		{
			for (int i = 0; i < pStandAloneMessageID->uNumOfPAssertedIdentity; i++)
			{
				printf("P-Asserted-Identity: %s\n", pStandAloneMessageID->ppPAssertedIdentity[i]);
			}
		}

	}
	break;

	case lims_Notify_Type_STAND_ALONE_MESSAGE_RECEIVED:
	{
		EcrioCPMIncomingStandAloneMessageStruct* pIncomingStandAloneMsg = NULL;

		pIncomingStandAloneMsg = (EcrioCPMIncomingStandAloneMessageStruct*)pData;

		printf("StandAlone message received from %s with DisplayName as %s!\n", pIncomingStandAloneMsg->pReqFrom, pIncomingStandAloneMsg->pDisplayName);
		printf("StandAlone message: %s\n", pIncomingStandAloneMsg->pMessage->message.pBuffer->pMessage);

		iota_test_printConversationHeaders(pIncomingStandAloneMsg->pConvId);

		printf("imdn.Message-ID: %s\n", pIncomingStandAloneMsg->pMessage->pIMDNMsgId);

		iota_test_printIMDNDispositionNotification(pIncomingStandAloneMsg->pMessage->imdnConfig);

		printf("bIsChatbot: %s\n", pIncomingStandAloneMsg->bIsChatbot == Enum_TRUE ? "TRUE" : "FALSE");
	}
	break;




	default:
	{
	}
	break;
	}

	pal_MutexUnlock(iotaState.mutexHandle);
}

EMSCRIPTEN_KEEPALIVE void limsCallback
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

void iota_test_Setup(void) {
    unsigned int error = KPALErrorNone;
    iotaState.limsHandle = NULL;

    iotaState.logHandle = pal_LogInit(
        (void *)"iota_test.log",
        KLogOutputStdOut,
        0x000003B3,
        KLogLevel_All,
        KLogComponent_All,
        KLogType_All
    );

    error = pal_Init(iotaState.logHandle, &iotaState.palLimsInstance);
    if (error != KPALErrorNone) {
        printf("❌ Could not initialize PAL instance! Error code: %u\n", error);
        if (iotaState.logHandle != NULL) {
            pal_LogDeinit(&iotaState.logHandle);
        }
        return;
    }

    printf("✅ PAL instance initialized successfully: %p\n", iotaState.palLimsInstance);
    EM_ASM({ console.log("✅ PAL instance initialized in WASM:", $0); }, iotaState.palLimsInstance);

    error = pal_MutexCreate(iotaState.palLimsInstance, &iotaState.mutexHandle);
    if (error != KPALErrorNone) {
        printf("❌ Could not create mutex!\n");
        pal_Deinit(iotaState.palLimsInstance);
        if (iotaState.logHandle != NULL) {
            pal_LogDeinit(&iotaState.logHandle);
        }
        return;
    }

    error = pal_SocketSetDeviceName(iotaState.palLimsInstance, iotaState.localInterface);
    if (error != KPALErrorNone) {
        printf("❌ Failed to set device name!\n");
        pal_Deinit(iotaState.palLimsInstance);
        if (iotaState.logHandle != NULL) {
            pal_LogDeinit(&iotaState.logHandle);
        }
        return;
    }
}

EMSCRIPTEN_KEEPALIVE
unsigned int iota_test_init(void) {
    EM_ASM({ console.log("STEP 1: Locking mutex"); });
    pal_MutexLock(iotaState.mutexHandle);

    unsigned int error = LIMS_NO_ERROR;
    lims_ConfigStruct config;
    lims_CallbackStruct callback;
    memset(&config, 0, sizeof(config));
    memset(&callback, 0, sizeof(callback));

    EM_ASM({ console.log("STEP 3: Calling iota_test_Setup"); });
    iota_test_Setup();

    EM_ASM({ console.log("STEP 4: Setting basic config fields"); });
    if (iotaState.palLimsInstance == NULL) {
        EM_ASM({ console.error("❌ PAL instance is NULL"); });
        pal_MutexUnlock(iotaState.mutexHandle);
        return 9999;
    }

    config.pal = iotaState.palLimsInstance;
    config.logHandle = iotaState.logHandle;
    config.bEnableTcp = Enum_FALSE;
    config.bEnableUdp = Enum_TRUE;

    EM_ASM({ console.log("STEP 5: Allocating config strings"); });
    if (!(config.pHomeDomain = strdup("ecrio.com")) ||
        !(config.pPassword = strdup("ecrio@123")) ||
        !(config.pPrivateIdentity = strdup("1111@ecrio.com")) ||
        !(config.pPublicIdentity = strdup("sip:1111@ecrio.com")) ||
        !(config.pUserAgent = strdup("Ecrio-iota-Client/V1.0")) ||
        !(config.pDeviceId = strdup("01437600-003868-4")) ||
        !(config.pPANI = strdup("3GPP-E-UTRAN-FDD;utran-cell-id-3gpp=310410000b0038000"))) {
        EM_ASM({ console.error("❌ Memory allocation failed"); });
        pal_MutexUnlock(iotaState.mutexHandle);
        return LIMS_ERROR_MEMORY;
    }

    config.uRegExpireInterval = 36000;
    config.bSubscribeRegEvent = false;
    config.bUnSubscribeRegEvent = false;
    config.eAlgorithm = EcrioSipAuthAlgorithmMD5;
    config.pOOMObject = default_oom_GetObject();
    config.uMtuSize = 1300;
    config.bIsRelayEnabled = Enum_FALSE;

    iotaState.bIsFileSender = false;
    strcpy(iotaState.calleeNumber, "sip:+14087770002@ecrio.com");
    strcpy(iotaState.message, "initial string");

    EM_ASM({ console.log("STEP 7: Setting callbacks"); });
    callback.pLimsCallback = limsCallback;
    callback.pContext = NULL;

    pal_MutexUnlock(iotaState.mutexHandle);

    EM_ASM({ console.log("STEP 9: Calling lims_Init"); });
    iotaState.limsHandle = lims_Init(&config, &callback, &error);
    EM_ASM({ console.log("STEP 10: lims_Init returned"); });

    if (iotaState.limsHandle == NULL) {
        EM_ASM({ console.error("❌ lims_Init returned NULL"); });
        goto cleanup;
    }

    pal_MutexLock(iotaState.mutexHandle);
    lims_NetworkConnectionStruct network;
    memset(&network, 0, sizeof(network));
    network.uNoOfRemoteIps = 1;
    network.ppRemoteIPs = (char **)calloc(1, sizeof(char *));
    if (network.ppRemoteIPs && (network.ppRemoteIPs[0] = strdup("192.168.29.197"))) {
        network.uRemotePort = 5060;
        network.eIPType = lims_Network_IP_Type_V4;
        network.uStatus = lims_Network_Status_Success;

        pal_MutexUnlock(iotaState.mutexHandle);
        error = lims_NetworkStateChange(iotaState.limsHandle, lims_Network_PDN_Type_IMS, lims_Network_Connection_Type_LTE, &network);
        pal_MutexLock(iotaState.mutexHandle);
        if (error != LIMS_NO_ERROR) {
            EM_ASM({ console.error("❌ lims_NetworkStateChange failed"); });
        }
    } else {
        EM_ASM({ console.error("❌ Memory allocation for IP failed"); });
        error = LIMS_ERROR_MEMORY;
    }

    if (network.ppRemoteIPs) {
        if (network.ppRemoteIPs[0]) free(network.ppRemoteIPs[0]);
        free(network.ppRemoteIPs);
    }

    pal_MutexUnlock(iotaState.mutexHandle);

cleanup:
    if (config.pHomeDomain) free(config.pHomeDomain);
    if (config.pPassword) free(config.pPassword);
    if (config.pPrivateIdentity) free(config.pPrivateIdentity);
    if (config.pPublicIdentity) free(config.pPublicIdentity);
    if (config.pUserAgent) free(config.pUserAgent);
    if (config.pDeviceId) free(config.pDeviceId);
    if (config.pPANI) free(config.pPANI);
    if (config.pRelayServerIP) free(config.pRelayServerIP);
    if (config.pDisplayName) free(config.pDisplayName);
    if (config.pTLSCertificate) free(config.pTLSCertificate);

    EM_ASM({ console.log("✅ iota_test_init completed with code:", $0); }, error);
    return error;
}

EMSCRIPTEN_KEEPALIVE
unsigned int iota_test_deinit(void) {
    unsigned int error = LIMS_NO_ERROR;

    if (iotaState.limsHandle != NULL) {
        printf("Calling lims_Deinit()\n");
        error = lims_Deinit(&iotaState.limsHandle);
        if (error != LIMS_NO_ERROR) {
            printf("lims_Deinit() failed: %d\n", error);
        } else {
            iotaState.limsHandle = NULL;
        }
    }

    if (iotaState.pSessionId) pal_MemoryFree((void **)&iotaState.pSessionId);
    if (iotaState.pGroupSessionId) pal_MemoryFree((void **)&iotaState.pGroupSessionId);
    if (iotaState.pReferId) pal_MemoryFree((void **)&iotaState.pReferId);

    return error;
}

EMSCRIPTEN_KEEPALIVE
unsigned int iota_test_register(void) {
    pal_MutexLock(iotaState.mutexHandle);
    unsigned int error = LIMS_NO_ERROR;
    unsigned int uFeatures = lims_Feature_CPM_PagerMode | lims_Feature_CPM_LargeMode | lims_Feature_CPM_Chat | lims_Feature_CPM_GeoLocation | lims_Feature_CPM_IMDN_Aggregation;

    if (iotaState.limsHandle != NULL) {
        lims_RegisterStruct registerStruct;
        registerStruct.pIPsecParams = NULL;

        printf("Calling lims_Register()\n");
        pal_MutexUnlock(iotaState.mutexHandle);
        error = lims_Register(iotaState.limsHandle, uFeatures, &registerStruct);
        pal_MutexLock(iotaState.mutexHandle);

        if (error != LIMS_NO_ERROR) {
            printf("lims_Register() failed with code %u\n", error);
        }
    }

    pal_MutexUnlock(iotaState.mutexHandle);
    return error;
}

EMSCRIPTEN_KEEPALIVE
unsigned int iota_test_deregister(void) {
    pal_MutexLock(iotaState.mutexHandle);
    unsigned int error = LIMS_NO_ERROR;

    if (iotaState.limsHandle != NULL) {
        printf("Calling lims_Deregister()\n");
        error = lims_Deregister(iotaState.limsHandle);
        if (error != LIMS_NO_ERROR) {
            printf("lims_Deregister() failed: %u\n", error);
        }
    }

    pal_MutexUnlock(iotaState.mutexHandle);
    error = lims_Deregister(iotaState.limsHandle);
		pal_MutexLock(iotaState.mutexHandle);
		if (error != LIMS_NO_ERROR)
		{
			printf("lims_Deregister() failed\n");
		}
	}

	pal_MutexUnlock(iotaState.mutexHandle);
	return error;
}

EMSCRIPTEN_KEEPALIVE void iota_test_getRandomStringHex
(
	unsigned char *pStr,
	unsigned int uLength
)
{
	unsigned int i = 0;

	for (i = 0; i < uLength; i++)
	{
		*(pStr + i) = iotaState.seedHex[pal_UtilityRandomNumber() % 16];
	}
}


EMSCRIPTEN_KEEPALIVE void iota_test_getContributionID(u_char* pContributionID)
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


EMSCRIPTEN_KEEPALIVE unsigned int iota_test_SendStandAloneMessage
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
	char imdnId[21] = { '\0' };

	EcrioCPMMessageStruct message;

	memset(&sendMessg, 0, sizeof(lims_StandAloneMessageStruct));
	memset(&text, 0, sizeof(EcrioCPMBufferStruct));
	memset(&ids, 0, sizeof(EcrioCPMConversationsIdStruct));
	memset(&convId, 0, 64);
	memset(&contId, 0, 64);
	memset(&message, 0, sizeof(EcrioCPMMessageStruct));

	printf("Calling lims_SendStandAloneMessage()\n");

	sendMessg.pDestUri = (char *)iotaState.calleeNumber;

	sendMessg.bIsChatbot = false;
	sendMessg.bDeleteChatBotToken = false;

	{
		sendMessg.pMessage = &message;
		sendMessg.pMessage->imdnConfig = EcrioCPMIMDispositionConfigPositiveDelivery | EcrioCPMIMDispositionConfigDisplay;
		iota_test_getRandomString((unsigned char *)imdnId, 20);
		sendMessg.pMessage->pIMDNMsgId = (char *)imdnId;

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
			printf("Memory copy error.\n");
			return 1;
		}
		if (NULL == pal_StringNCopy(iotaState.convID, 64, ids.pConversationId, pal_StringLength(ids.pContributionId)))
		{
			printf("Memory copy error.\n");
			return 1;
		}
	}

	pal_MutexUnlock(iotaState.mutexHandle);
	error = lims_SendStandAloneMessage(iotaState.limsHandle, &sendMessg, &pCallId);
	pal_MutexLock(iotaState.mutexHandle);
	if (error != LIMS_NO_ERROR)
	{
		printf("lims_SendStandAloneMessage() failed.\n");
	}
	else
	{
		printf("lims_SendStandAloneMessage() success.\n");
	}
	if (pCallId)
	{
		printf("Call id: %s \n", pCallId);
		pal_MemoryFree((void**)&pCallId);
	}

	pal_MutexUnlock(iotaState.mutexHandle);
	return error;
}


