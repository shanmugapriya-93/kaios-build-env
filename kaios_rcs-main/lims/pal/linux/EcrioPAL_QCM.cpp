/******************************************************************************

Copyright (c) 2019-2020 Ecrio, Inc. All Rights Reserved.

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
 * @file EcrioPAL_QCM.cpp
 * @brief This is the implementation of the Ecrio PAL's Qualcomm Connection Manager Module.
 */

#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <csignal>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <com/qualcomm/qti/imscmservice/2.2/IImsCmService.h>
#include <com/qualcomm/qti/imscmservice/2.1/IImsCmService.h>
#include <com/qualcomm/qti/imscmservice/2.0/IImsCmService.h>

using ::android::sp;
using ::com::qualcomm::qti::imscmservice::V2_0::IImsCMConnection;
using ::com::qualcomm::qti::imscmservice::V2_0::IImsCMConnectionListener;
using ::com::qualcomm::qti::imscmservice::V2_2::IImsCmService;
using ::com::qualcomm::qti::imscmservice::V2_2::IImsCmServiceListener;
using ::com::qualcomm::qti::imscmservice::V2_1::autoconfigRequestStatus;
using ::com::qualcomm::qti::imscmservice::V2_1::autoconfigTriggerReason;
using ::com::qualcomm::qti::imscmservice::V2_0::AutoConfigRequestType;
using ::com::qualcomm::qti::imscmservice::V2_0::ipTypeEnum;
using ::com::qualcomm::qti::imscmservice::V2_0::incomingMessage;
using ::com::qualcomm::qti::imscmservice::V2_0::incomingMessageKeys;
using ::com::qualcomm::qti::imscmservice::V2_0::outgoingMessage;
using ::com::qualcomm::qti::imscmservice::V2_0::outgoingMessageKeys;
using ::com::qualcomm::qti::imscmservice::V2_0::messageType;
using ::com::qualcomm::qti::imscmservice::V2_0::sipProtocolType;
using ::com::qualcomm::qti::imscmservice::V2_0::ConfigType;
using ::com::qualcomm::qti::imscmservice::V2_0::configData;
using ::com::qualcomm::qti::imscmservice::V2_0::userConfig;
using ::com::qualcomm::qti::imscmservice::V2_0::userConfigKeys;
using ::com::qualcomm::qti::imscmservice::V2_0::deviceConfig;
using ::com::qualcomm::qti::imscmservice::V2_2::deviceConfigKeys;
using ::com::qualcomm::qti::imscmservice::V2_0::ServiceStatus;
using ::com::qualcomm::qti::imscmservice::V2_1::StatusCode;
using ::com::qualcomm::qti::imscmservice::V2_0::connectionEvent;
using ::com::qualcomm::qti::imscmservice::V2_0::connectionEventData;
using ::com::qualcomm::qti::imscmservice::V2_2::AutoConfigResponse;
//using ::com::qualcomm::qti::imscmservice::V2_0;

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_death_recipient;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::wp;

using namespace std;

#include "EcrioPAL.h"
#include "EcrioPAL_Internal.h"

/** Logging defines and helper functions */
#ifdef ENABLE_LOG
#define QCMLOGD(a, b, c, ...)		pal_LogMessage((a), KLogLevelDebug, KLogComponentPAL, (b), (c),##__VA_ARGS__);
#define QCMLOGI(a, b, c, ...)		pal_LogMessage((a), KLogLevelInfo, KLogComponentPAL, (b), (c),##__VA_ARGS__);
#define QCMLOGV(a, b, c, ...)		pal_LogMessage((a), KLogLevelVerbose, KLogComponentPAL, (b), (c),##__VA_ARGS__);
#define QCMLOGW(a, b, c, ...)		pal_LogMessage((a), KLogLevelWarning, KLogComponentPAL, (b), (c),##__VA_ARGS__);
#define QCMLOGE(a, b, c, ...)		pal_LogMessage((a), KLogLevelError, KLogComponentPAL, (b), (c),##__VA_ARGS__);

#define QCMLOGDUMP(a, b, c, ...)		pal_LogMessageDump((a), KLogLevelInfo, KLogComponentPAL, (b), (c),##__VA_ARGS__);
#else /* ENABLE_LOG */
#define QCMLOGD(a, b, c, ...)
#define QCMLOGI(a, b, c, ...)
#define QCMLOGV(a, b, c, ...)
#define QCMLOGW(a, b, c, ...)
#define QCMLOGE(a, b, c, ...)

#define QCMLOGDUMP(a, b, c, ...)
#endif /* ENABLE_LOG */

#define QCMErrorEvent	0
#define QCMIncomingEvent	0

typedef void* QCM_SIGNAL_HANDLE;

#define QCM_SIGNAL_EXIT 100

#define QCMGetConfig_None 	0x00000000
#define QCMGetConfig_User 	0x00000001
#define QCMGetConfig_Device 	0x00000010

/* messageQ message Structure */
typedef struct
{
	unsigned int uCommand;
	unsigned int uParam;
	void* pParam;
} Qcm_messageStruct;

/* messageQ structure */
typedef struct
{
	mutex m;
	list <Qcm_messageStruct> messageQ;
} Qcm_messageQStruct;

typedef unsigned int(*Qcm_signalCallback)
(
	QCM_SIGNAL_HANDLE handle,
	unsigned int command,
	unsigned int uParam,
	void* pParam
);

typedef struct
{
	mutex m;
	condition_variable cond;
	thread hThread;
	Qcm_messageQStruct messageQStruct;
	Qcm_signalCallback callback;
	unsigned int isEnding;
	LOGHANDLE logHandle;
} Qcm_signalStruct;

/* MESSAGE QUEUE IMPLEMENTATION - START */
void Qcm_message_init
(
	Qcm_messageQStruct *messageQStruct
)
{
	(void)messageQStruct;
	//TBD
}

void Qcm_message_deinit
(
	Qcm_messageQStruct *messageQStruct
)
{
	(void)messageQStruct;
	//TBD
}

int Qcm_message_add
(
	Qcm_messageQStruct *messageQStruct,
	unsigned int uCommand,
	unsigned int uParam,
	void* pParam
)
{
	lock_guard<mutex> lock(messageQStruct->m);
	Qcm_messageStruct messageStruct = {};
	messageStruct.uCommand = uCommand;
	messageStruct.uParam = uParam;
	messageStruct.pParam = pParam;
	messageQStruct->messageQ.push_back(messageStruct);
	return 0;
}

int Qcm_message_get
(
	Qcm_messageQStruct *messageQStruct,
	Qcm_messageStruct *pStruct
)
{
	unique_lock<mutex> lock(messageQStruct->m);
	if (messageQStruct->messageQ.size() == 0)
	{
		lock.unlock();
		return -1;
	}
	Qcm_messageStruct messageStruct = {};
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

static void Qcm_signal_worker
(
	void *pStruct
)
{
	Qcm_signalStruct *h = (Qcm_signalStruct*)pStruct;
	while (1)
	{
		unique_lock<mutex> lock(h->m);
		h->cond.wait(lock);
		lock.unlock();
		Qcm_messageStruct messageStruct = {};
		while (Qcm_message_get(&(h->messageQStruct), &messageStruct) == 0)
		{
			switch (messageStruct.uCommand)
			{
			case QCM_SIGNAL_EXIT:
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

QCM_SIGNAL_HANDLE Qcm_signal_init
(
	Qcm_signalCallback pSignalFn,
	LOGHANDLE logHandle,
	u_int32 *pError
)
{
	Qcm_signalStruct *h = NULL;
	int returnValue = 0;

	if (pError == NULL)
	{
		return NULL;
	}

	if (pSignalFn == NULL)
	{
		*pError = 1;
		return NULL;
	}

	/* By default, the error value returned will indicate success. */
	*pError = returnValue;

	h = new (Qcm_signalStruct);

	h->logHandle = logHandle;
	h->isEnding = 0;

	h->callback = pSignalFn;

	Qcm_message_init(&(h->messageQStruct));

	h->hThread = thread(Qcm_signal_worker, (void *)h);
	return h;
}

int Qcm_signal_deinit
(
	QCM_SIGNAL_HANDLE *handle
)
{
	Qcm_signalStruct *h = NULL;
	if (handle == NULL)
	{
		return -1;
	}
	if (*handle == NULL)
	{
		return -1;
	}
	h = (Qcm_signalStruct*)*handle;
	h->isEnding = 1;
	Qcm_message_add(&(h->messageQStruct), QCM_SIGNAL_EXIT, 0, NULL);
	unique_lock<mutex> lock(h->m);
	h->cond.notify_one();
	lock.unlock();
	h->hThread.join();
	Qcm_message_deinit(&(h->messageQStruct));
	delete h;

	return 0;
}

int Qcm_signal_send
(
	QCM_SIGNAL_HANDLE handle,
	unsigned int command,
	unsigned int uParam,
	void* pParam
)
{
	Qcm_signalStruct *h = NULL;
	if (handle == NULL)
	{
		return -1;
	}
	h = (Qcm_signalStruct*)handle;
	Qcm_message_add(&(h->messageQStruct), command, uParam, pParam);
	unique_lock<mutex> lock(h->m);
	h->cond.notify_one();
	lock.unlock();
	return 0;
}

typedef struct
{
	string UEBehindNAT;
	string IPSecEnabled;
	string CompactFormEnabled;
	string KeepAliveStatusEnabled;
	string GruuEnabled;
	string SipOutBoundProxyName;
	string SipOutBoundProxyPort;
	string PCSCFClientPort;
	string PCSCFServerPort;
	string AuthChallenge;
	string ArrNC;
	string ServiceRoute;
	string SecurityVerify;
	string PCSCFOldSAClientPort;
	string TCPThresholdValue;
	string PANI;
	string PATH;
	string UriUserPart;
	string PLANI;
	string Ppa;
	string Pidentifier;
	string UEClientPort;
	string UEServerPort;
	string AssociatedUri;
	string UEPublicIPAddress;
	string UEPublicPort;
	string SipPublicUserId;
	string SipPrivateUserId;
	string SipHomeDomain;
	string UEPubGruu;
	string LocalHostIPAddress;
	string IPType;
	string IMEI;
	string UEOldSAClientPort;
} QCMInternalConfigStruct;

typedef struct {
	PALINSTANCE pal;
	LOGHANDLE logHandle;

	mutex m;
	condition_variable cond;

	mutex mConnection;
	condition_variable condConnection;

	APIVersions eVersion;

	sp<IImsCmService> service;
	sp<com::qualcomm::qti::imscmservice::V2_1::IImsCmService> service2_1;
	sp<com::qualcomm::qti::imscmservice::V2_0::IImsCmService> service2_0;

	sp<IImsCmServiceListener> serviceListener;
	sp<com::qualcomm::qti::imscmservice::V2_1::IImsCmServiceListener> serviceListener2_1;
	sp<com::qualcomm::qti::imscmservice::V2_0::IImsCmServiceListener> serviceListener2_0;

	sp<IImsCMConnection> connection;
	sp<IImsCMConnectionListener> connectionListener;

	sp<android::hardware::hidl_death_recipient> deathReceiver;
	uint64_t uCookie;

	uint64_t uConnectionManager;
	uint64_t uConnectionHandle;
	uint64_t uListenerToken;
	uint64_t uListenerId;

	string iccid;
	string featureTag;
	uint32_t uUserData;

	ServiceStatus eServiceStatus;
	StatusCode eStatusCode;
	connectionEvent eEvent;

	QcmEventCallback pEventCallbackFn;
	void *pEventCallbackFnData;

	QcmReceiveCallback pReceiveCallbackFn;
	QcmErrorCallback pErrorCallbackFn;
	void *pCallbackFnData;
	BoolEnum bEnableGlobalMutex;

	QCMInternalConfigStruct internalconfigStruct;

	QCM_SIGNAL_HANDLE eventHandle;
	QCM_SIGNAL_HANDLE incomingHandle;
	QCM_SIGNAL_HANDLE errorHandle;

	uint32_t uNOfInit;
	BoolEnum bIsServiceDeid;
	uint32_t uReadyCreateConnection;
	
	BoolEnum bIsConnectionAlived;
	BoolEnum bIsUpdatedNetworkParams;
} ImsCmServiceStruct;

ImsCmServiceStruct g_service;

static void QcmUserConfigStrutDelete
(
	void
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;

	if (!i->internalconfigStruct.UEClientPort.empty())
	{
		i->internalconfigStruct.UEClientPort.clear();
	}

	if (!i->internalconfigStruct.UEServerPort.empty())
	{
		i->internalconfigStruct.UEServerPort.clear();
	}

	if (!i->internalconfigStruct.AssociatedUri.empty())
	{
		i->internalconfigStruct.AssociatedUri.clear();
	}

	if (!i->internalconfigStruct.UEPublicIPAddress.empty())
	{
		i->internalconfigStruct.UEPublicIPAddress.clear();
	}

	if (!i->internalconfigStruct.UEPublicPort.empty())
	{
		i->internalconfigStruct.UEPublicPort.clear();
	}

	if (!i->internalconfigStruct.SipPublicUserId.empty())
	{
		i->internalconfigStruct.SipPublicUserId.clear();
	}

	if (!i->internalconfigStruct.SipPrivateUserId.empty())
	{
		i->internalconfigStruct.SipPrivateUserId.clear();
	}

	if (!i->internalconfigStruct.SipHomeDomain.empty())
	{
		i->internalconfigStruct.SipHomeDomain.clear();
	}

	if (!i->internalconfigStruct.UEPubGruu.empty())
	{
		i->internalconfigStruct.UEPubGruu.clear();
	}

	if (!i->internalconfigStruct.LocalHostIPAddress.empty())
	{
		i->internalconfigStruct.LocalHostIPAddress.clear();
	}

	if (!i->internalconfigStruct.IPType.empty())
	{
		i->internalconfigStruct.IPType.clear();
	}

	if (!i->internalconfigStruct.IMEI.empty())
	{
		i->internalconfigStruct.IMEI.clear();
	}

	if (!i->internalconfigStruct.UEOldSAClientPort.empty())
	{
		i->internalconfigStruct.UEOldSAClientPort.clear();
	}
}

static void QcmDeviceConfigStrutDelete
(
	void
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;

	if (!i->internalconfigStruct.UEBehindNAT.empty())
	{
		i->internalconfigStruct.UEBehindNAT.clear();
	}

	if (!i->internalconfigStruct.IPSecEnabled.empty())
	{
		i->internalconfigStruct.IPSecEnabled.clear();
	}

	if (!i->internalconfigStruct.CompactFormEnabled.empty())
	{
		i->internalconfigStruct.CompactFormEnabled.clear();
	}

	if (!i->internalconfigStruct.KeepAliveStatusEnabled.empty())
	{
		i->internalconfigStruct.KeepAliveStatusEnabled.clear();
	}

	if (!i->internalconfigStruct.GruuEnabled.empty())
	{
		i->internalconfigStruct.GruuEnabled.clear();
	}

	if (!i->internalconfigStruct.GruuEnabled.empty())
	{
		i->internalconfigStruct.GruuEnabled.clear();
	}

	if (!i->internalconfigStruct.SipOutBoundProxyName.empty())
	{
		i->internalconfigStruct.SipOutBoundProxyName.clear();
	}

	if (!i->internalconfigStruct.SipOutBoundProxyPort.empty())
	{
		i->internalconfigStruct.SipOutBoundProxyPort.clear();
	}

	if (!i->internalconfigStruct.PCSCFClientPort.empty())
	{
		i->internalconfigStruct.PCSCFClientPort.clear();
	}

	if (!i->internalconfigStruct.PCSCFServerPort.empty())
	{
		i->internalconfigStruct.PCSCFServerPort.clear();
	}

	if (!i->internalconfigStruct.AuthChallenge.empty())
	{
		i->internalconfigStruct.AuthChallenge.clear();
	}

	if (!i->internalconfigStruct.ArrNC.empty())
	{
		i->internalconfigStruct.ArrNC.clear();
	}

	if (!i->internalconfigStruct.ServiceRoute.empty())
	{
		i->internalconfigStruct.ServiceRoute.clear();
	}

	if (!i->internalconfigStruct.SecurityVerify.empty())
	{
		i->internalconfigStruct.SecurityVerify.clear();
	}

	if (!i->internalconfigStruct.PCSCFOldSAClientPort.empty())
	{
		i->internalconfigStruct.PCSCFOldSAClientPort.clear();
	}

	if (!i->internalconfigStruct.TCPThresholdValue.empty())
	{
		i->internalconfigStruct.TCPThresholdValue.clear();
	}

	if (!i->internalconfigStruct.PANI.empty())
	{
		i->internalconfigStruct.PANI.clear();
	}

	if (!i->internalconfigStruct.PATH.empty())
	{
		i->internalconfigStruct.PATH.clear();
	}

	if (!i->internalconfigStruct.UriUserPart.empty())
	{
		i->internalconfigStruct.UriUserPart.clear();
	}

	if (!i->internalconfigStruct.PLANI.empty())
	{
		i->internalconfigStruct.PLANI.clear();
	}

	if (!i->internalconfigStruct.Ppa.empty())
	{
		i->internalconfigStruct.Ppa.clear();
	}

	if (!i->internalconfigStruct.Pidentifier.empty())
	{
		i->internalconfigStruct.Pidentifier.clear();
	}
}

static void QcmOnConfigurationChange
(
	const configData& config
)
{
	if (config.userConfigData.data.size() > 0)
	{
		QcmUserConfigStrutDelete();
		for (auto iter = config.userConfigData.data.begin(); iter < config.userConfigData.data.end(); ++iter) {
			QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u key=%u, value=%s", __FUNCTION__, __LINE__, (uint32_t)iter->key, iter->value.c_str());

			if ((userConfigKeys)iter->key == userConfigKeys::UEClientPort && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.UEClientPort = iter->value;
			}
			else if ((userConfigKeys)iter->key == userConfigKeys::UEServerPort && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.UEServerPort = iter->value;
			}
			else if ((userConfigKeys)iter->key == userConfigKeys::AssociatedURI && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.AssociatedUri = iter->value;
			}
			else if ((userConfigKeys)iter->key == userConfigKeys::UEPublicIPAddress && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.UEPublicIPAddress = iter->value;
			}
			else if ((userConfigKeys)iter->key == userConfigKeys::UEPublicPort && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.UEPublicPort = iter->value;
			}
			else if ((userConfigKeys)iter->key == userConfigKeys::SipPublicUserId && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.SipPublicUserId = iter->value;
			}
			else if ((userConfigKeys)iter->key == userConfigKeys::SipPrivateUserId && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.SipPrivateUserId = iter->value;
			}
			else if ((userConfigKeys)iter->key == userConfigKeys::SipHomeDomain && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.SipHomeDomain = iter->value;
			}
			else if ((userConfigKeys)iter->key == userConfigKeys::UEPubGruu && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.UEPubGruu = iter->value;
			}
			else if ((userConfigKeys)iter->key == userConfigKeys::LocalHostIPAddress && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.LocalHostIPAddress = iter->value;
			}
			else if ((userConfigKeys)iter->key == userConfigKeys::IpType && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.IPType = iter->value;
			}
			else if ((userConfigKeys)iter->key == userConfigKeys::IMEIStr && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.IMEI = iter->value;
			}
			else if ((userConfigKeys)iter->key == userConfigKeys::UEOldSAClientPort && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.UEOldSAClientPort = iter->value;
			}
		}

		if (g_service.bIsServiceDeid == Enum_TRUE)
		{
			g_service.uReadyCreateConnection |= QCMGetConfig_User;
			if ((g_service.uReadyCreateConnection & (QCMGetConfig_User|QCMGetConfig_Device)) == (QCMGetConfig_User|QCMGetConfig_Device))
			{
				Qcm_signal_send(g_service.eventHandle, QCMConnectionEvent_ReadyCreateConnection, 0, NULL);
			}
		}
	}

	if (config.deviceConfigData.data.size() > 0)
	{
		QcmDeviceConfigStrutDelete();
		for (auto iter = config.deviceConfigData.data.begin(); iter < config.deviceConfigData.data.end(); ++iter) {
			QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u key=%u, value=%s", __FUNCTION__, __LINE__, (uint32_t)iter->key, iter->value.c_str());

			if ((deviceConfigKeys)iter->key == deviceConfigKeys::UEBehindNAT && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.UEBehindNAT = iter->value;
			}
			else if ((deviceConfigKeys)iter->key == deviceConfigKeys::IpSecEnabled && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.IPSecEnabled = iter->value;
			}
			else if ((deviceConfigKeys)iter->key == deviceConfigKeys::CompactFormEnabled && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.CompactFormEnabled = iter->value;
			}
			else if ((deviceConfigKeys)iter->key == deviceConfigKeys::KeepAliveEnableStatus && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.KeepAliveStatusEnabled = iter->value;
			}
			else if ((deviceConfigKeys)iter->key == deviceConfigKeys::GruuEnabled && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.GruuEnabled = iter->value;
			}
			else if ((deviceConfigKeys)iter->key == deviceConfigKeys::StrSipOutBoundProxyName && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.SipOutBoundProxyName = iter->value;
			}
			else if ((deviceConfigKeys)iter->key == deviceConfigKeys::SipOutBoundProxyPort && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.SipOutBoundProxyPort = iter->value;
			}
			else if ((deviceConfigKeys)iter->key == deviceConfigKeys::PCSCFClientPort && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.PCSCFClientPort = iter->value;
			}
			else if ((deviceConfigKeys)iter->key == deviceConfigKeys::PCSCFServerPort && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.PCSCFServerPort = iter->value;
			}
			else if ((deviceConfigKeys)iter->key == deviceConfigKeys::ArrAuthChallenge && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.AuthChallenge = iter->value;
			}
			else if ((deviceConfigKeys)iter->key == deviceConfigKeys::ArrNC && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.ArrNC = iter->value;
			}
			else if ((deviceConfigKeys)iter->key == deviceConfigKeys::ServiceRoute && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.ServiceRoute = iter->value;
			}
			else if ((deviceConfigKeys)iter->key == deviceConfigKeys::SecurityVerify && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.SecurityVerify = iter->value;
			}
			else if ((deviceConfigKeys)iter->key == deviceConfigKeys::PCSCFOldSAClientPort && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.PCSCFOldSAClientPort = iter->value;
			}
			else if ((deviceConfigKeys)iter->key == deviceConfigKeys::TCPThresholdValue && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.TCPThresholdValue = iter->value;
			}
			else if ((deviceConfigKeys)iter->key == deviceConfigKeys::PANI && iter->value.size() > 0)
			{
//				g_service.internalconfigStruct.PANI = iter->value;
				g_service.internalconfigStruct.PANI = "3GPP-E-UTRAN-FDD;utran-cell-id-3gpp=310410000b0038000";
			}
			else if ((deviceConfigKeys)iter->key == deviceConfigKeys::PATH && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.PATH = iter->value;
			}
			else if ((deviceConfigKeys)iter->key == deviceConfigKeys::UriUserPart && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.UriUserPart = iter->value;
			}
			else if ((deviceConfigKeys)iter->key == deviceConfigKeys::PLANI && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.PLANI = iter->value;
			}
			else if ((deviceConfigKeys)iter->key == deviceConfigKeys::PPA && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.Ppa = iter->value;
			}
			else if ((deviceConfigKeys)iter->key == deviceConfigKeys::PIDENTIFIER && iter->value.size() > 0)
			{
				g_service.internalconfigStruct.Pidentifier = iter->value;
			}
		}

		if (g_service.bIsServiceDeid == Enum_TRUE)
		{
			g_service.uReadyCreateConnection |= QCMGetConfig_Device;
			if ((g_service.uReadyCreateConnection & (QCMGetConfig_User|QCMGetConfig_Device)) == (QCMGetConfig_User|QCMGetConfig_Device))
			{
				Qcm_signal_send(g_service.eventHandle, QCMConnectionEvent_ReadyCreateConnection, 0, NULL);
			}
		}
	}

	if (config.autoConfigData.autoConfigXml.size() > 0)
	{
		QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u config.autoConfigData.autoConfigXml size=%u", __FUNCTION__, __LINE__, config.autoConfigData.autoConfigXml.size());
		QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u config.autoConfigData.autoConfigRequestType=%u", __FUNCTION__, __LINE__, (unsigned int)config.autoConfigData.autoConfigRequestType);

		QCMConnectionEvents e = QCMConnectionEvent_GetACSConfiguration;
		QcmACSConfigStruct *acsConfigStruct = NULL;
		pal_MemoryAllocate(sizeof(QcmACSConfigStruct), (void **)&acsConfigStruct);
		if (acsConfigStruct == NULL)
		{
			QCMLOGI(g_service.logHandle, KLogTypeFuncExit, "%s:%u\t memory allocation is failed!", __FUNCTION__, __LINE__);
			return;
		}
		pal_MemoryAllocate(config.autoConfigData.autoConfigXml.size()+1, (void **)&acsConfigStruct->pConfig);
		if (acsConfigStruct->pConfig == NULL)
		{
			QCMLOGI(g_service.logHandle, KLogTypeFuncExit, "%s:%u\t memory allocation is failed!", __FUNCTION__, __LINE__);
			pal_MemoryFree((void**) &acsConfigStruct);
			return;
		}
		pal_MemorySet((void *)acsConfigStruct->pConfig, 0x00, (u_int32)config.autoConfigData.autoConfigXml.size()+1);

		pal_MemoryCopy((void *)acsConfigStruct->pConfig, config.autoConfigData.autoConfigXml.size()+1, (const void *)config.autoConfigData.autoConfigXml.c_str(), (u_int32)config.autoConfigData.autoConfigXml.size());
		if ((AutoConfigRequestType)config.autoConfigData.autoConfigRequestType == AutoConfigRequestType::SERVER_UPDATE)
		{
			acsConfigStruct->uType = 0;
		}
		else if ((AutoConfigRequestType)config.autoConfigData.autoConfigRequestType == AutoConfigRequestType::CLIENT_REQUEST)
		{
			acsConfigStruct->uType = 1;
		}

		Qcm_signal_send(g_service.eventHandle, e, 0, (void *)acsConfigStruct);	
	}
}

static void QcmPrintStatusCode
(
	uint32_t uStatus
)
{
	if ((StatusCode)uStatus == StatusCode::UNSUPPORTED)
	{
		QCMLOGI(g_service.logHandle, KLogTypeGeneral, "%s:%u\t UNSUPPORTED", __FUNCTION__, __LINE__);
	}
	else if ((StatusCode)uStatus == StatusCode::SUCCESS)
	{
		QCMLOGI(g_service.logHandle, KLogTypeGeneral, "%s:%u\t SUCCESS", __FUNCTION__, __LINE__);
	}
	else if ((StatusCode)uStatus == StatusCode::FAILURE)
	{
		QCMLOGI(g_service.logHandle, KLogTypeGeneral, "%s:%u\t FAILURE", __FUNCTION__, __LINE__);
	}
	else if ((StatusCode)uStatus == StatusCode::MEMORY_ERROR)
	{
		QCMLOGI(g_service.logHandle, KLogTypeGeneral, "%s:%u\t MEMORY_ERROR", __FUNCTION__, __LINE__);
	}
	else if ((StatusCode)uStatus == StatusCode::INVALID_LISTENER)
	{
		QCMLOGI(g_service.logHandle, KLogTypeGeneral, "%s:%u\t INVALID_LISTENER", __FUNCTION__, __LINE__);
	}
	else if ((StatusCode)uStatus == StatusCode::INVALID_PARAM)
	{
		QCMLOGI(g_service.logHandle, KLogTypeGeneral, "%s:%u\t INVALID_PARAM", __FUNCTION__, __LINE__);
	}
	else if ((StatusCode)uStatus == StatusCode::SERVICE_NOTALLOWED)
	{
		QCMLOGI(g_service.logHandle, KLogTypeGeneral, "%s:%u\t SERVICE_NOTALLOWED", __FUNCTION__, __LINE__);
	}
	else if ((StatusCode)uStatus == StatusCode::SERVICE_UNAVAILABLE)
	{
		QCMLOGI(g_service.logHandle, KLogTypeGeneral, "%s:%u\t SERVICE_UNAVAILABLE", __FUNCTION__, __LINE__);
	}
	else if ((StatusCode)uStatus == StatusCode::INVALID_FEATURE_TAG)
	{
		QCMLOGI(g_service.logHandle, KLogTypeGeneral, "%s:%u\t INVALID_FEATURE_TAG", __FUNCTION__, __LINE__);
	}
	else if ((StatusCode)uStatus == StatusCode::DNSQUERY_PENDING)
	{
		QCMLOGI(g_service.logHandle, KLogTypeGeneral, "%s:%u\t DNSQUERY_PENDING", __FUNCTION__, __LINE__);
	}
	else if ((StatusCode)uStatus == StatusCode::DNSQUERY_FAILURE)
	{
		QCMLOGI(g_service.logHandle, KLogTypeGeneral, "%s:%u\t DNSQUERY_FAILURE", __FUNCTION__, __LINE__);
	}
	else if ((StatusCode)uStatus == StatusCode::SERVICE_DIED)
	{
		QCMLOGI(g_service.logHandle, KLogTypeGeneral, "%s:%u\t SERVICE_DIED", __FUNCTION__, __LINE__);
	}
	else if ((StatusCode)uStatus == StatusCode::MESSAGE_NOTALLOWED)
	{
		QCMLOGI(g_service.logHandle, KLogTypeGeneral, "%s:%u\t MESSAGE_NOTALLOWED", __FUNCTION__, __LINE__);
	}
	else if ((StatusCode)uStatus == StatusCode::DISPATCHER_SEND_SUCCESS)
	{
		QCMLOGI(g_service.logHandle, KLogTypeGeneral, "%s:%u\t DISPATCHER_SEND_SUCCESS", __FUNCTION__, __LINE__);
	}
	else if ((StatusCode)uStatus == StatusCode::INVALID_MAX)
	{
		QCMLOGI(g_service.logHandle, KLogTypeGeneral, "%s:%u\t INVALID_MAX", __FUNCTION__, __LINE__);
	}
}

class implhidl_death_recipient : public android::hardware::hidl_death_recipient
{
	void serviceDied(uint64_t cookie, const wp<::android::hidl::base::V1_0::IBase>& service) {
		QCMLOGI(g_service.logHandle, KLogTypeFuncEntry, "%s:%u\t", __FUNCTION__, __LINE__);

		Qcm_signal_send(g_service.eventHandle, QCMConnectionEvent_ServiceDied, 0, NULL);

		QCMLOGI(g_service.logHandle, KLogTypeFuncExit, "%s:%u\t", __FUNCTION__, __LINE__);
	}
};

class implIImsCMConnectionListener : public IImsCMConnectionListener {
public:
	implIImsCMConnectionListener() {};
	virtual ~implIImsCMConnectionListener() {};
 
	Return<void> onEventReceived(const connectionEventData& event) {
		QCMConnectionEvents e = QCMConnectionEvent_Unknown;

		g_service.eEvent = (connectionEvent)event.eEvent;

		if((connectionEvent)event.eEvent == connectionEvent::SERVICE_NOTREGISTERED)
		{
			QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u Service is NOT registered. featureTag=%s", __FUNCTION__, __LINE__, event.featureTag.c_str());
			e = QCMConnectionEvent_NotRegistered;
		}
		else if((connectionEvent)event.eEvent == connectionEvent::SERVICE_REGISTERED)
		{
			QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u Service is registered. featureTag=%s", __FUNCTION__, __LINE__, event.featureTag.c_str());
			e = QCMConnectionEvent_Registered;
		}
		else if((connectionEvent)event.eEvent == connectionEvent::SERVICE_ALLOWED)
		{
			QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u Service is allowed. featureTag=%s", __FUNCTION__, __LINE__, event.featureTag.c_str());
			e = QCMConnectionEvent_Allowed;
		}
		else if((connectionEvent)event.eEvent == connectionEvent::SERVICE_NOTALLOWED)
		{
			QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u Service is not allowed. featureTag=%s", __FUNCTION__, __LINE__, event.featureTag.c_str());
			e = QCMConnectionEvent_NotAllowed;
		}
		else if((connectionEvent)event.eEvent == connectionEvent::SERVICE_FORCEFUL_CLOSE)
		{
			QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u Service is closed forcefully. featureTag=%s", __FUNCTION__, __LINE__, event.featureTag.c_str());
			e = QCMConnectionEvent_ForcefulClose;
		}
		else if((connectionEvent)event.eEvent == connectionEvent::SERVICE_TERMINATE_CONNECTION)
		{
			QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u Service connection terminated. featureTag=%s", __FUNCTION__, __LINE__, event.featureTag.c_str());
			e = QCMConnectionEvent_TerminateConnection;
		}
		else if((connectionEvent)event.eEvent == connectionEvent::SERVICE_CREATED)
		{
			QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u Service is created. featureTag=%s", __FUNCTION__, __LINE__, event.featureTag.c_str());
			e = QCMConnectionEvent_Created;
		}
		else
		{
			QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u Unknow event received. featureTag=%s", __FUNCTION__, __LINE__, event.featureTag.c_str());
			return Void();
		}

		Qcm_signal_send(g_service.eventHandle, e, 0, NULL);	
		
		QCMLOGI(g_service.logHandle, KLogTypeFuncExit, "%s:%u\t", __FUNCTION__, __LINE__);

		return Void();
	}

	Return<void> handleIncomingMessage(const incomingMessage& data) {
		QCMLOGI(g_service.logHandle, KLogTypeFuncEntry, "%s:%u\t", __FUNCTION__, __LINE__);

		QcmReceiveStruct *receiveStruct = NULL;
		pal_MemoryAllocate(sizeof(QcmReceiveStruct), (void **)&receiveStruct);
		if (receiveStruct == NULL)
		{
			QCMLOGI(g_service.logHandle, KLogTypeFuncExit, "%s:%u\t memory allocation is failed!", __FUNCTION__, __LINE__);
			return Void();
		}

		for (auto iter = data.data.begin(); iter < data.data.end(); ++iter) {
//			QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u key=%u, value=%s", __FUNCTION__, __LINE__, (uint32_t)iter->key, iter->value.c_str());

			if ((incomingMessageKeys)iter->key == incomingMessageKeys::recdAddr)
			{
				if (iter->value.size() > 0)
				{
				pal_MemoryAllocate(iter->value.size()+1, (void **)&receiveStruct->pFromIP);
				if (receiveStruct->pFromIP == NULL)
				{
					QCMLOGI(g_service.logHandle, KLogTypeFuncExit, "%s:%u\t memory allocation is failed!", __FUNCTION__, __LINE__);
					pal_MemoryFree((void**) &receiveStruct);
					return Void();
				}
				pal_MemorySet((void *)receiveStruct->pFromIP, 0, (u_int32)(iter->value.size()+ 1));
				pal_StringNCopy((u_char *)receiveStruct->pFromIP, iter->value.size()+1, (const u_char *)iter->value.c_str(), (s_int32)iter->value.size());
			}
			else
			{
				receiveStruct->pFromIP = NULL;
			}
		}
		}

		for (auto iter = data.bufferData.begin(); iter < data.bufferData.end(); ++iter)
		{
			if ((incomingMessageKeys)iter->key == incomingMessageKeys::Message)
			{
				string value(iter->value.begin(), iter->value.end());
	//			QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u key=%u, value=%s", __FUNCTION__, __LINE__, (uint32_t)iter->key, value.c_str());

				if (value.size() > 0)
				{
					pal_MemoryAllocate(value.size()+1, (void **)&receiveStruct->pReceiveData);
					if (receiveStruct->pReceiveData == NULL)
					{
						QCMLOGI(g_service.logHandle, KLogTypeFuncExit, "%s:%u\t memory allocation is failed!", __FUNCTION__, __LINE__);
						pal_MemoryFree((void**) &receiveStruct);
						if (receiveStruct->pFromIP != NULL)
						{
							pal_MemoryFree((void**) &receiveStruct->pFromIP);
						}
						return Void();
					}
					pal_MemorySet((void *)receiveStruct->pReceiveData, 0, (u_int32)(value.size()+ 1));
					pal_MemoryCopy((void *)receiveStruct->pReceiveData, value.size()+1, (const void *)value.c_str(), (u_int32)value.size());
					receiveStruct->receiveLength = value.size();
				}
				else
				{
					receiveStruct->pReceiveData = NULL;
					receiveStruct->receiveLength = 0;
				}
			}
		}

		receiveStruct->handle = (QCMINSTANCE)&g_service;
		receiveStruct->result = KPALErrorNone;

		Qcm_signal_send(g_service.incomingHandle, QCMIncomingEvent, 0, (void *)receiveStruct);

		QCMLOGI(g_service.logHandle, KLogTypeFuncExit, "%s:%u\t", __FUNCTION__, __LINE__);
		return Void();
	}

	Return<void> onCommandStatus(uint32_t status, uint32_t userdata) {
		QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u status=%u, userdata=%u", __FUNCTION__, __LINE__, status, userdata);

		g_service.eStatusCode = (StatusCode)status;

#if 0
		unique_lock<mutex> lock(g_service.mConnection);
		g_service.condConnection.notify_one();
		lock.unlock();
#endif

		if ((StatusCode)status != StatusCode::SUCCESS)
		{
			u_int32 error = KPALEntityClosed;
//			if ((StatusCode)status == StatusCode::MESSAGE_NOTALLOWED)
//			{
//				error = KPALNotAllowed;
//			}

			Qcm_signal_send(g_service.errorHandle, QCMErrorEvent, error, NULL);
		}

		return Void();
	}
};

class implImsCmServiceListener : public IImsCmServiceListener {
public:
	implImsCmServiceListener() {};
	virtual ~implImsCmServiceListener() {};

	Return<void> onServiceReady(uint64_t connectionManager, uint32_t userdata, uint32_t eStatus) {
		QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u userdata=%u, eStatus=%u", __FUNCTION__, __LINE__, userdata, eStatus);

		g_service.eServiceStatus = (ServiceStatus)eStatus;
		g_service.uConnectionManager = connectionManager;

		if ((ServiceStatus)eStatus == ServiceStatus::STATUS_INIT_IN_PROGRESS)
		{
			return Void();
		}

		unique_lock<mutex> lock(g_service.m);
		g_service.cond.notify_one();
		lock.unlock();

		return Void();
	}

	Return<void> onStatusChange(uint32_t eStatus) {
		QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u eStatus=%u", __FUNCTION__, __LINE__, eStatus);
		return Void();
	}

	Return<void> onConfigurationChange(const configData& config, uint32_t userdata) {
		QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u userdata=%u", __FUNCTION__, __LINE__, userdata);

		if (g_service.uUserData != userdata)
		{
			return Void();
		}

		QcmOnConfigurationChange(config);

		return Void();
	}

	Return<void> onCommandStatus(uint32_t userdata, uint32_t status) {
		QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u userdata=%u, status=%u", __FUNCTION__, __LINE__, userdata, status);

		g_service.eStatusCode = (StatusCode)status;

		QcmPrintStatusCode(status);

		return Void();
	}

	Return<void> onConfigurationChange_2_1(const configData& config, uint32_t userdata) {
		QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u userdata=%u", __FUNCTION__, __LINE__, userdata);

		if (g_service.uUserData != userdata)
		{
			return Void();
		}

		QcmOnConfigurationChange(config);

		return Void();
	}

	Return<void> onAcsConnectionStatusChange(autoconfigRequestStatus autoConfigReqStatus, uint32_t userdata) {
		QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u userdata=%u", __FUNCTION__, __LINE__, userdata);
		
		QCMConnectionEvents e = QCMConnectionEvent_AutoconfigRequestStatus;
		u_int32 status = 0;
		
		if ((autoconfigRequestStatus)autoConfigReqStatus == autoconfigRequestStatus::AUTOCONFIG_STATUS_OFF)
		{
			status = 0;
		}
		else if ((autoconfigRequestStatus)autoConfigReqStatus == autoconfigRequestStatus::AUTOCONFIG_STATUS_ON)
		{
			status = 1;
		}

		Qcm_signal_send(g_service.eventHandle, e, status, NULL);	

		return Void();
	}

	Return<void> onConfigurationChange_2_2(const configData& config, uint32_t userdata) {
		QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u userdata=%u", __FUNCTION__, __LINE__, userdata);

		if (g_service.uUserData != userdata)
		{
			return Void();
		}

		QcmOnConfigurationChange(config);

		return Void();
	}

	Return<void> onAutoConfigResponse(const AutoConfigResponse& acsResponse) {
		QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u statusCode=%u, reasonPhrase=%s", __FUNCTION__, __LINE__, acsResponse.statusCode, acsResponse.reasonPhrase.c_str());
		return Void();
	}
};

class implImsCmServiceListener2_1 : public com::qualcomm::qti::imscmservice::V2_1::IImsCmServiceListener {
public:
	implImsCmServiceListener2_1() {};
	virtual ~implImsCmServiceListener2_1() {};

	Return<void> onServiceReady(uint64_t connectionManager, uint32_t userdata, uint32_t eStatus) {
		QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u userdata=%u, eStatus=%u", __FUNCTION__, __LINE__, userdata, eStatus);

		g_service.eServiceStatus = (ServiceStatus)eStatus;
		g_service.uConnectionManager = connectionManager;

		if ((ServiceStatus)eStatus == ServiceStatus::STATUS_INIT_IN_PROGRESS)
		{
			return Void();
		}

		unique_lock<mutex> lock(g_service.m);
		g_service.cond.notify_one();
		lock.unlock();

		return Void();
	}

	Return<void> onStatusChange(uint32_t eStatus) {
		QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u eStatus=%u", __FUNCTION__, __LINE__, eStatus);
		return Void();
	}

	Return<void> onConfigurationChange(const configData& config, uint32_t userdata) {
		QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u userdata=%u", __FUNCTION__, __LINE__, userdata);

		if (g_service.uUserData != userdata)
		{
			return Void();
		}

		QcmOnConfigurationChange(config);

		return Void();
	}

	Return<void> onCommandStatus(uint32_t userdata, uint32_t status) {
		QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u userdata=%u, status=%u", __FUNCTION__, __LINE__, userdata, status);

		g_service.eStatusCode = (StatusCode)status;

		QcmPrintStatusCode(status);

		return Void();
	}

	Return<void> onConfigurationChange_2_1(const configData& config, uint32_t userdata) {
		QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u userdata=%u", __FUNCTION__, __LINE__, userdata);

		if (g_service.uUserData != userdata)
		{
			return Void();
		}

		QcmOnConfigurationChange(config);

		return Void();
	}

	Return<void> onAcsConnectionStatusChange(autoconfigRequestStatus autoConfigReqStatus, uint32_t userdata) {
		QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u userdata=%u", __FUNCTION__, __LINE__, userdata);
		
		QCMConnectionEvents e = QCMConnectionEvent_AutoconfigRequestStatus;
		u_int32 status = 0;
		
		if ((autoconfigRequestStatus)autoConfigReqStatus == autoconfigRequestStatus::AUTOCONFIG_STATUS_OFF)
		{
			status = 0;
		}
		else if ((autoconfigRequestStatus)autoConfigReqStatus == autoconfigRequestStatus::AUTOCONFIG_STATUS_ON)
		{
			status = 1;
		}

		Qcm_signal_send(g_service.eventHandle, e, status, NULL);	

		return Void();
	}
};

class implImsCmServiceListener2_0 : public com::qualcomm::qti::imscmservice::V2_0::IImsCmServiceListener {
public:
	implImsCmServiceListener2_0() {};
	virtual ~implImsCmServiceListener2_0() {};

	Return<void> onServiceReady(uint64_t connectionManager, uint32_t userdata, uint32_t eStatus) {
		QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u userdata=%u, eStatus=%u", __FUNCTION__, __LINE__, userdata, eStatus);

		g_service.eServiceStatus = (ServiceStatus)eStatus;
		g_service.uConnectionManager = connectionManager;

		if ((ServiceStatus)eStatus == ServiceStatus::STATUS_INIT_IN_PROGRESS)
		{
			return Void();
		}

		unique_lock<mutex> lock(g_service.m);
		g_service.cond.notify_one();
		lock.unlock();

		return Void();
	}

	Return<void> onStatusChange(uint32_t eStatus) {
		QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u eStatus=%u", __FUNCTION__, __LINE__, eStatus);
		return Void();
	}

	Return<void> onConfigurationChange(const configData& config, uint32_t userdata) {
		QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u userdata=%u", __FUNCTION__, __LINE__, userdata);

		if (g_service.uUserData != userdata)
		{
			return Void();
		}

		QcmOnConfigurationChange(config);

		return Void();
	}

	Return<void> onCommandStatus(uint32_t userdata, uint32_t status) {
		QCMLOGD(g_service.logHandle, KLogTypeGeneral, "%s:%u userdata=%u, status=%u", __FUNCTION__, __LINE__, userdata, status);

		g_service.eStatusCode = (StatusCode)status;

		QcmPrintStatusCode(status);

		return Void();
	}
};

static void QcmConfigStrutDelete
(
	void
)
{
	QcmUserConfigStrutDelete();
	QcmDeviceConfigStrutDelete();
}

static u_int32 QcmLinkToDeath
(
	void
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;
	u_int32 error = KPALErrorNone;

	if (i->eVersion == APIVersion_20)
	{
		auto death_link = i->service2_0->linkToDeath(i->deathReceiver, i->uCookie);
		if (!death_link.isOk())
		{
			error = KPALInternalError;
		}
	}
	else if (i->eVersion == APIVersion_21)
	{
		auto death_link = i->service2_1->linkToDeath(i->deathReceiver, i->uCookie);
		if (!death_link.isOk())
		{
			error = KPALInternalError;
		}
	}
	else if (i->eVersion == APIVersion_22)
	{
		auto death_link = i->service->linkToDeath(i->deathReceiver, i->uCookie);
		if (!death_link.isOk())
		{
			error = KPALInternalError;
		}
	}
	
	return error;
}

static u_int32 QcmUnlinkToDeath
(
	void
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;
	u_int32 error = KPALErrorNone;

	if (i->eVersion == APIVersion_20)
	{
		auto death_unlink = i->service2_0->unlinkToDeath(i->deathReceiver);
		if (!death_unlink.isOk())
		{
			error = KPALInternalError;
		}
	}
	else if (i->eVersion == APIVersion_21)
	{
		auto death_unlink = i->service2_1->unlinkToDeath(i->deathReceiver);
		if (!death_unlink.isOk())
		{
			error = KPALInternalError;
		}
	}
	else if (i->eVersion == APIVersion_22)
	{
		auto death_unlink = i->service->unlinkToDeath(i->deathReceiver);
		if (!death_unlink.isOk())
		{
			error = KPALInternalError;
		}
	}
	
	return error;
}

static void QcmCloseService
(
	void
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;
	uint32_t status = 0;

//	QcmUnlinkToDeath();

#if 0
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tremoveListener() is calling", __FUNCTION__, __LINE__);
	if (i->eVersion == APIVersion_20)
	{
		status = i->service2_0->removeListener(i->uConnectionManager, i->uListenerId);
	}
	else if (i->eVersion == APIVersion_21)
	{
		status = i->service2_1->removeListener(i->uConnectionManager, i->uListenerId);
	}
	else if (i->eVersion == APIVersion_22)
	{
		status = i->service->removeListener(i->uConnectionManager, i->uListenerId);
	}
	if ((StatusCode)status != StatusCode::SUCCESS)
	{
		QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u removeListener() is failed: error=%u", __FUNCTION__, __LINE__, status);
	}
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\ttremoveListener() returns %u", __FUNCTION__, __LINE__, status);
#endif

	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tcloseService() is calling", __FUNCTION__, __LINE__);
	if (i->eVersion == APIVersion_20)
	{
		status = i->service2_0->closeService(i->uConnectionManager);
	}
	else if (i->eVersion == APIVersion_21)
	{
		status = i->service2_1->closeService(i->uConnectionManager);
	}
	else if (i->eVersion == APIVersion_22)
	{
		status = i->service->closeService(i->uConnectionManager);
	}
	if ((StatusCode)status != StatusCode::SUCCESS)
	{
		QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u closeService() is failed: error=%u", __FUNCTION__, __LINE__, status);
	}
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tcloseService() returns %u", __FUNCTION__, __LINE__, status);

//	Qcm_signal_deinit(&(i->errorHandle));
//	Qcm_signal_deinit(&(i->incomingHandle));
//	Qcm_signal_deinit(&(i->eventHandle));
}

static u_int32 QcmInternalInit
(
	void
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;
	u_int32 error = KPALErrorNone;
	uint32_t uStatus = 0;

	QCMLOGI(i->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	i->uConnectionManager = 0L;
	i->uConnectionHandle = 0L;
	i->uListenerToken = 0L;
	i->uListenerId = 0L;

	i->eServiceStatus = ServiceStatus::STATUS_SERVICE_UNKNOWN;;
	i->eStatusCode = StatusCode::FAILURE;
	i->eEvent = connectionEvent::SERVICE_NOTREGISTERED;

	i->uUserData = pal_UtilityRandomNumber();
	i->uCookie = pal_UtilityRandomNumber();

	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tgetService() 2_2 is calling", __FUNCTION__, __LINE__);
	i->service = IImsCmService::getService("qti.ims.connectionmanagerservice");
	if (i->service == nullptr)
	{
		QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tgetService() 2_1 is calling", __FUNCTION__, __LINE__);
		i->service2_1 = com::qualcomm::qti::imscmservice::V2_1::IImsCmService::getService("qti.ims.connectionmanagerservice");
		if (i->service2_1 == nullptr)
		{
			QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tgetService() 2_0 is calling", __FUNCTION__, __LINE__);
			i->service2_0 = com::qualcomm::qti::imscmservice::V2_0::IImsCmService::getService("qti.ims.connectionmanagerservice");
			if (i->service2_0 == nullptr)
			{
				QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u fail to get service", __FUNCTION__, __LINE__);
				return KPALInternalError;
			}
			else
			{
				i->eVersion = APIVersion_20;
				QCMLOGD(i->logHandle, KLogTypeGeneral, "%s:%u get qti.ims.connectionmanagerservice 2.0", __FUNCTION__, __LINE__);
			}
		}
		else
		{
			i->eVersion = APIVersion_21;
			QCMLOGD(i->logHandle, KLogTypeGeneral, "%s:%u get qti.ims.connectionmanagerservice 2.1", __FUNCTION__, __LINE__);
		}
	}
	else
	{
		i->eVersion = APIVersion_22;
		QCMLOGD(i->logHandle, KLogTypeGeneral, "%s:%u get qti.ims.connectionmanagerservice 2.2", __FUNCTION__, __LINE__);
	}

	i->deathReceiver = new implhidl_death_recipient();
	if (i->deathReceiver == nullptr)
	{
		QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u fail to allocate deathReceiver", __FUNCTION__, __LINE__);
		return KPALInternalError;
	}
	QcmLinkToDeath();

	StatusCode status = StatusCode::FAILURE;
	BoolEnum bDone = Enum_FALSE;
	while (!bDone)
	{
		switch (i->eVersion)
		{
			case APIVersion_20:
			{
				QCMLOGD(i->logHandle, KLogTypeGeneral, "%s:%u InitializeService() is calling.", __FUNCTION__, __LINE__);
				i->serviceListener2_0 = new implImsCmServiceListener2_0();
				i->service2_0->InitializeService(i->iccid, i->serviceListener2_0, i->uUserData,
				        				[&](uint32_t _status, uint64_t _listenerId) {
				        				QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u status=%u", __FUNCTION__, __LINE__, _status);
									i->uListenerId = _listenerId;
									status = (StatusCode)_status;
									} );
				if (status != StatusCode::SUCCESS || i->uListenerId == 0L)
				{
					i->eVersion = APIVersion_None;
				}
				else
				{
					bDone = Enum_TRUE;
					QCMLOGD(i->logHandle, KLogTypeGeneral, "%s:%u InitializeService() is done.", __FUNCTION__, __LINE__);
				}
			}
			break;

			case APIVersion_21:
			{
				QCMLOGD(i->logHandle, KLogTypeGeneral, "%s:%u InitializeService_2_1() is calling.", __FUNCTION__, __LINE__);
				i->serviceListener2_1 = new implImsCmServiceListener2_1();
				i->service2_1->InitializeService_2_1(i->iccid, i->serviceListener2_1, i->uUserData,
				        /* lamda func */ [&](uint32_t _status, uint64_t _listenerId) {
									QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u status=%u", __FUNCTION__, __LINE__, _status);
									i->uListenerId = _listenerId;
									status = (StatusCode)_status;
				        /* end Lamda Func*/ } );
				if (status == StatusCode::UNSUPPORTED)
				{
					QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u fail to InitializeService_2_1() : status=%u", __FUNCTION__, __LINE__, (uint32_t)status);
					i->service2_0 = com::qualcomm::qti::imscmservice::V2_0::IImsCmService::getService("qti.ims.connectionmanagerservice");
					if (i->service2_0 == nullptr)
					{
						i->eVersion = APIVersion_None;
					}
					else
					{
						i->eVersion = APIVersion_20;
						QCMLOGD(i->logHandle, KLogTypeGeneral, "%s:%u get qti.ims.connectionmanagerservice 2.0", __FUNCTION__, __LINE__);
					}
				}
				else if (i->uListenerId == 0L || status != StatusCode::SUCCESS)
				{
					QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u fail to InitializeService_2_1() : status=%u", __FUNCTION__, __LINE__, (uint32_t)status);
					i->eVersion = APIVersion_None;
				}
				else
				{
					bDone = Enum_TRUE;
					QCMLOGD(i->logHandle, KLogTypeGeneral, "%s:%u InitializeService_2_1() is done.", __FUNCTION__, __LINE__);
				}
			}
			break;

			case APIVersion_22:
			{
				QCMLOGD(i->logHandle, KLogTypeGeneral, "%s:%u InitializeService_2_2() is calling.", __FUNCTION__, __LINE__);
				i->serviceListener = new implImsCmServiceListener();
				i->service->InitializeService_2_2(i->iccid, i->serviceListener, i->uUserData,
				        /* lamda func */ [&](uint32_t _status, uint64_t _listenerId) {
									QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u status=%u", __FUNCTION__, __LINE__, _status);
									i->uListenerId = _listenerId;
									status = (StatusCode)_status;
				        /* end Lamda Func*/ } );
				if (status == StatusCode::UNSUPPORTED)
				{
					QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u fail to InitializeService_2_2() : status=%u", __FUNCTION__, __LINE__, (uint32_t)status);
					i->service2_1 = com::qualcomm::qti::imscmservice::V2_1::IImsCmService::getService("qti.ims.connectionmanagerservice");
					if (i->service2_1 == nullptr)
					{
						i->eVersion = APIVersion_None;
					}
					else
					{
						i->eVersion = APIVersion_21;
						QCMLOGD(i->logHandle, KLogTypeGeneral, "%s:%u get qti.ims.connectionmanagerservice 2.1", __FUNCTION__, __LINE__);
					}
				}
				else if (i->uListenerId == 0L || status != StatusCode::SUCCESS)
				{
					QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u fail to InitializeService_2_2() : status=%u", __FUNCTION__, __LINE__, (uint32_t)status);
					i->eVersion = APIVersion_None;
				}
				else
				{
					bDone = Enum_TRUE;
					QCMLOGD(i->logHandle, KLogTypeGeneral, "%s:%u InitializeService_2_2() is done.", __FUNCTION__, __LINE__);
				}
			}
			break;

			default:
			{
				QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u fail to InitializeService : Invalid version=%u", __FUNCTION__, __LINE__, i->eVersion);
				bDone = Enum_TRUE;
			}				
			break;
		}
	}

	if (i->eVersion == APIVersion_None)
	{
		error = KPALInternalError;
		goto ErrorInit;
	}

	if (i->eServiceStatus == ServiceStatus::STATUS_SERVICE_UNKNOWN)
	{
		unique_lock<mutex> serviceReadyState(i->m);
		i->cond.wait(serviceReadyState);
		serviceReadyState.unlock();
	}

	if (i->eServiceStatus != ServiceStatus::STATUS_SUCCESS)
	{
		i->eServiceStatus = ServiceStatus::STATUS_SERVICE_UNKNOWN;
		QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u fail to ServiceReady : status=%u", __FUNCTION__, __LINE__, (uint32_t)i->eServiceStatus);
		error = KPALInternalError;
		goto ErrorGetConfig;
	}

	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tgetConfiguration() is calling", __FUNCTION__, __LINE__);
	if (i->eVersion == APIVersion_20)
	{
		uStatus = i->service2_0->getConfiguration(i->uConnectionManager, ConfigType::DEVICE_CONFIG, i->uUserData);
	}
	else if (i->eVersion == APIVersion_21)
	{
		uStatus = i->service2_1->getConfiguration(i->uConnectionManager, ConfigType::DEVICE_CONFIG, i->uUserData);
	}
	else if (i->eVersion == APIVersion_22)
	{
		uStatus = i->service->getConfiguration(i->uConnectionManager, ConfigType::DEVICE_CONFIG, i->uUserData);
	}
	if ((StatusCode)uStatus != StatusCode::SUCCESS)
	{
		QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u getConfiguration() is failed: status=%u", __FUNCTION__, __LINE__, (uint32_t)uStatus);
		error = KPALInternalError;
		goto ErrorGetConfig;
	}
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tgetConfiguration() returns %u", __FUNCTION__, __LINE__, status);

	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tgetConfiguration() is calling", __FUNCTION__, __LINE__);
	if (i->eVersion == APIVersion_20)
	{
		uStatus = i->service2_0->getConfiguration(i->uConnectionManager, ConfigType::USER_CONFIG, i->uUserData);
	}
	else if (i->eVersion == APIVersion_21)
	{
		uStatus = i->service2_1->getConfiguration(i->uConnectionManager, ConfigType::USER_CONFIG, i->uUserData);
	}
	else if (i->eVersion == APIVersion_22)
	{
		uStatus = i->service->getConfiguration(i->uConnectionManager, ConfigType::USER_CONFIG, i->uUserData);
	}
	if ((StatusCode)uStatus != StatusCode::SUCCESS)
	{
		QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u getConfiguration() is failed: status=%u", __FUNCTION__, __LINE__, (uint32_t)uStatus);
		error = KPALInternalError;
		goto ErrorGetConfig;
	}
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tgetConfiguration() returns %u", __FUNCTION__, __LINE__, status);

	QCMLOGI(i->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return error;

ErrorGetConfig:
	QcmCloseService();
ErrorInit:
	QcmUnlinkToDeath();

	QCMLOGI(i->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return error;
}

static void QcmServiceDiedEnabled
(
	void
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;

	i->bIsServiceDeid = Enum_TRUE;
	i->uReadyCreateConnection = QCMGetConfig_None;
}

static void QcmServiceDiedDisabled
(
	void
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;

	i->bIsServiceDeid = Enum_FALSE;
	i->uReadyCreateConnection = QCMGetConfig_None;
}

static u_int32 QcmInternalRegister
(
	void
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;
	u_int32 error = KPALErrorNone;
	uint32_t status = 0;

	QCMLOGI(i->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (i->bIsConnectionAlived == Enum_TRUE)
	{
#if 0
		QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tremoveListener() is calling", __FUNCTION__, __LINE__);
		status = i->connection->removeListener(i->uListenerToken);
		if ((StatusCode)status != StatusCode::SUCCESS)
		{
			QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u removeListener() is failed: error=%u", __FUNCTION__, __LINE__, status);
		}
		QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tremoveListener() returns %u", __FUNCTION__, __LINE__, status);
#endif

		QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tcloseConnection() is calling", __FUNCTION__, __LINE__);
		if (i->eVersion == APIVersion_20)
		{
			status = i->service2_0->closeConnection(i->uConnectionManager, i->uConnectionHandle);
		}
		else if (i->eVersion == APIVersion_21)
		{
			status = i->service2_1->closeConnection(i->uConnectionManager, i->uConnectionHandle);
		}
		else if (i->eVersion == APIVersion_22)
		{
			status = i->service->closeConnection(i->uConnectionManager, i->uConnectionHandle);
		}
		if ((StatusCode)status != StatusCode::SUCCESS)
		{
			QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u closeConnection() is failed: error=%u", __FUNCTION__, __LINE__, status);
		}
		QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tcloseConnection() returns %u", __FUNCTION__, __LINE__, status);
	}

	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tcreateConnection() is calling", __FUNCTION__, __LINE__);
	i->connectionListener = new implIImsCMConnectionListener();
	if (i->eVersion == APIVersion_20)
	{
		i->service2_0->createConnection(i->uConnectionManager, i->connectionListener, i->featureTag,
			/* lamda func */ [&](const sp<IImsCMConnection> &_connection, uint64_t connectionHandle, uint64_t listenerToken) {
								QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u", __FUNCTION__, __LINE__);
								i->connection = _connection;
								i->uConnectionHandle = connectionHandle;
								i->uListenerToken = listenerToken;
			/* end Lamda Func*/ });
	}
	else if (i->eVersion == APIVersion_21)
	{
		i->service2_1->createConnection(i->uConnectionManager, i->connectionListener, i->featureTag,
			/* lamda func */ [&](const sp<IImsCMConnection> &_connection, uint64_t connectionHandle, uint64_t listenerToken) {
								QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u", __FUNCTION__, __LINE__);
								i->connection = _connection;
								i->uConnectionHandle = connectionHandle;
								i->uListenerToken = listenerToken;
			/* end Lamda Func*/ });
	}
	else if (i->eVersion == APIVersion_22)
	{
		i->service->createConnection(i->uConnectionManager, i->connectionListener, i->featureTag,
			/* lamda func */ [&](const sp<IImsCMConnection> &_connection, uint64_t connectionHandle, uint64_t listenerToken) {
								QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u", __FUNCTION__, __LINE__);
								i->connection = _connection;
								i->uConnectionHandle = connectionHandle;
								i->uListenerToken = listenerToken;
			/* end Lamda Func*/ });
	}
	if (i->connection.get() == nullptr)
	{
		QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u fail to createConnection()", __FUNCTION__, __LINE__);
		return KPALInternalError;
	}
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tcreateConnection() is done.", __FUNCTION__, __LINE__);

	QCMLOGI(i->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return error;
}

static unsigned int errorHandler
(
	QCM_SIGNAL_HANDLE handle,
	unsigned int command,
	unsigned int uParam,
	void *pParam
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;
	(void)handle;
	(void)pParam;

	if (command == QCMErrorEvent)
	{
		u_int32 error = uParam;
		if (g_service.pErrorCallbackFn != NULL)
		{
			if (g_service.bEnableGlobalMutex)
			{
				pal_MutexGlobalLock(g_service.pal);
			}

			QCMLOGI(g_service.logHandle, KLogTypeGeneral, "%s:%u\terror=%u", __FUNCTION__, __LINE__, error);
			g_service.pErrorCallbackFn(g_service.pal, g_service.pCallbackFnData, error);

			if (g_service.bEnableGlobalMutex)
			{
				pal_MutexGlobalUnlock(g_service.pal);
			}
		}
	}

	return 0;
}

static unsigned int incomingHandler
(
	QCM_SIGNAL_HANDLE handle,
	unsigned int command,
	unsigned int uParam,
	void *pParam
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;
	(void)handle;
	(void)uParam;

	if (command == QCMIncomingEvent)
	{
		if (g_service.pReceiveCallbackFn != NULL)
		{
			QcmReceiveStruct *receiveStruct = (QcmReceiveStruct *)pParam;
			if (g_service.bEnableGlobalMutex)
			{
				pal_MutexGlobalLock(g_service.pal);
			}

			QCMLOGI(g_service.logHandle, KLogTypeGeneral, "%s:%u\t", __FUNCTION__, __LINE__);
			g_service.pReceiveCallbackFn(g_service.pCallbackFnData, receiveStruct);

			if (g_service.bEnableGlobalMutex)
			{
				pal_MutexGlobalUnlock(g_service.pal);
			}

			if (receiveStruct->pFromIP != NULL)
			{
				pal_MemoryFree((void**) &receiveStruct->pFromIP);
				receiveStruct->pFromIP = NULL;
			}

			if (receiveStruct->pReceiveData != NULL)
			{
				pal_MemoryFree((void**) &receiveStruct->pReceiveData);
				receiveStruct->pReceiveData = NULL;
			}

			if (receiveStruct != NULL)
			{
				pal_MemoryFree((void**) &receiveStruct);
				receiveStruct = NULL;
			}
		}
	}

	return 0;
}

static unsigned int eventHandler
(
	QCM_SIGNAL_HANDLE handle,
	unsigned int command,
	unsigned int uParam,
	void *pParam
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;
	(void)handle;

	switch (command)
	{
		case QCMConnectionEvent_NotRegistered:
		case QCMConnectionEvent_Registered:
		case QCMConnectionEvent_Allowed:
		case QCMConnectionEvent_NotAllowed:
		case QCMConnectionEvent_ForcefulClose:
		case QCMConnectionEvent_TerminateConnection:
		{
			if (g_service.pEventCallbackFn != NULL)
			{
				if (g_service.bEnableGlobalMutex)
				{
					pal_MutexGlobalLock(g_service.pal);
				}

				ConnectionEvents e = ConnectionEvent_Unknown;
				if (command == QCMConnectionEvent_NotRegistered)
				{
					e = ConnectionEvent_NotRegistered;
				}
				else if (command == QCMConnectionEvent_Registered)
				{
					e = ConnectionEvent_Registered;
				}
				else if (command == QCMConnectionEvent_Allowed)
				{
					e = ConnectionEvent_Allowed;
				}
				else if (command == QCMConnectionEvent_NotAllowed)
				{
					e = ConnectionEvent_NotAllowed;
				}
				else if (command == QCMConnectionEvent_ForcefulClose)
				{
					e = ConnectionEvent_ForcefulClose;
				}
				else if (command == QCMConnectionEvent_TerminateConnection)
				{
					e = ConnectionEvent_TerminateConnection;
					g_service.bIsConnectionAlived = Enum_FALSE;
				}

				g_service.uNOfInit = 0;
				QcmServiceDiedDisabled();

				QCMLOGI(g_service.logHandle, KLogTypeGeneral, "%s:%u\teEvent=%u", __FUNCTION__, __LINE__, (uint32_t)e);
				g_service.pEventCallbackFn(g_service.pEventCallbackFnData, e, NULL);

				if (e == ConnectionEvent_Registered)
				{
					QcmServiceDiedEnabled();
					uint32_t uStatus = 0;
					QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tgetConfiguration() is calling", __FUNCTION__, __LINE__);
					if (i->eVersion == APIVersion_20)
					{
						uStatus = i->service2_0->getConfiguration(i->uConnectionManager, ConfigType::DEVICE_CONFIG, i->uUserData);
					}
					else if (i->eVersion == APIVersion_21)
					{
						uStatus = i->service2_1->getConfiguration(i->uConnectionManager, ConfigType::DEVICE_CONFIG, i->uUserData);
					}
					else if (i->eVersion == APIVersion_22)
					{
						uStatus = i->service->getConfiguration(i->uConnectionManager, ConfigType::DEVICE_CONFIG, i->uUserData);
					}
					if ((StatusCode)uStatus != StatusCode::SUCCESS)
					{
						QCMLOGE(g_service.logHandle, KLogTypeGeneral, "%s:%u getConfiguration() is failed: error=%u", __FUNCTION__, __LINE__, uStatus);
						e = ConnectionEvent_NotRegistered;
						g_service.pEventCallbackFn(g_service.pEventCallbackFnData, e, NULL);
						QcmServiceDiedDisabled();
					}
					QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tgetConfiguration() returns %u", __FUNCTION__, __LINE__, uStatus);

					QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tgetConfiguration() is calling", __FUNCTION__, __LINE__);
					if (i->eVersion == APIVersion_20)
					{
						uStatus = i->service2_0->getConfiguration(i->uConnectionManager, ConfigType::USER_CONFIG, i->uUserData);
					}
					else if (i->eVersion == APIVersion_21)
					{
						uStatus = i->service2_1->getConfiguration(i->uConnectionManager, ConfigType::USER_CONFIG, i->uUserData);
					}
					else if (i->eVersion == APIVersion_22)
					{
						uStatus = i->service->getConfiguration(i->uConnectionManager, ConfigType::USER_CONFIG, i->uUserData);
					}
					if ((StatusCode)uStatus != StatusCode::SUCCESS)
					{
						QCMLOGE(g_service.logHandle, KLogTypeGeneral, "%s:%u getConfiguration() is failed: error=%u", __FUNCTION__, __LINE__, uStatus);
						e = ConnectionEvent_NotRegistered;
						g_service.pEventCallbackFn(g_service.pEventCallbackFnData, e, NULL);
						QcmServiceDiedDisabled();
					}
					QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tgetConfiguration() returns %u", __FUNCTION__, __LINE__, uStatus);
				}

				if (g_service.bEnableGlobalMutex)
				{
					pal_MutexGlobalUnlock(g_service.pal);
				}
			}
		}
		break;

		case QCMConnectionEvent_Created:
		{
			ConnectionEvents e = ConnectionEvent_Created;
			uint32_t status = 0;
			QCMConfigStruct configStruct = {};
			pal_MemorySet(&configStruct, 0, sizeof(QCMConfigStruct));

			g_service.bIsConnectionAlived = Enum_TRUE;

			if (g_service.pEventCallbackFn != NULL)
			{
				if (g_service.bEnableGlobalMutex)
				{
					pal_MutexGlobalLock(g_service.pal);
				}

				if (!g_service.internalconfigStruct.SipPublicUserId.empty())
				{
					configStruct.pSipPublicUserId = (u_char *)g_service.internalconfigStruct.SipPublicUserId.c_str();
				}
				else
				{
					configStruct.pSipPublicUserId = NULL;
				}

				if (!g_service.internalconfigStruct.SipPrivateUserId.empty())
				{
					configStruct.pSipPrivateUserId = (u_char *)g_service.internalconfigStruct.SipPrivateUserId.c_str();
				}
				else
				{
					configStruct.pSipPrivateUserId = NULL;
				}

				if (!g_service.internalconfigStruct.SipHomeDomain.empty())
				{
					configStruct.pSipHomeDomain = (u_char *)g_service.internalconfigStruct.SipHomeDomain.c_str();
				}
				else
				{
					configStruct.pSipHomeDomain = NULL;
				}

				if (!g_service.internalconfigStruct.SipOutBoundProxyName.empty())
				{
					configStruct.pSipOutBoundProxyName = (u_char *)g_service.internalconfigStruct.SipOutBoundProxyName.c_str();
				}
				else
				{
					configStruct.pSipOutBoundProxyName = NULL;
				}

				if (!g_service.internalconfigStruct.LocalHostIPAddress.empty())
				{
					configStruct.pLocalHostIPAddress = (u_char *)g_service.internalconfigStruct.LocalHostIPAddress.c_str();
				}
				else
				{
					configStruct.pLocalHostIPAddress = NULL;
				}

				if (!g_service.internalconfigStruct.PCSCFClientPort.empty())
				{
					u_int32 value_u;
					stringstream value_s(g_service.internalconfigStruct.PCSCFClientPort);
					value_s >> value_u;	
					configStruct.uPCSCFClientPort = value_u;
				}
				else
				{
					configStruct.uPCSCFClientPort = 0;
				}

				if (!g_service.internalconfigStruct.PCSCFServerPort.empty())
				{
					u_int32 value_u;
					stringstream value_s(g_service.internalconfigStruct.PCSCFServerPort);
					value_s >> value_u;	
					configStruct.uPCSCFServerPort = value_u;
				}
				else
				{
					configStruct.uPCSCFServerPort = 0;
				}

				if (!g_service.internalconfigStruct.UEClientPort.empty())
				{
					u_int32 value_u;
					stringstream value_s(g_service.internalconfigStruct.UEClientPort);
					value_s >> value_u;	
					configStruct.uUEClientPort = value_u;
				}
				else
				{
					configStruct.uUEClientPort = 0;
				}

				if ((configStruct.uPCSCFServerPort == 0) && (configStruct.uUEClientPort == 0))
				{
					if (!g_service.internalconfigStruct.SipOutBoundProxyPort.empty())
					{
						u_int32 value_u;
						stringstream value_s(g_service.internalconfigStruct.SipOutBoundProxyPort);
						value_s >> value_u;	
						configStruct.uSipOutBoundProxyPort = value_u;
					}
					else
					{
						configStruct.uSipOutBoundProxyPort = 0;
					}					
				}

				if (!g_service.internalconfigStruct.IPType.empty())
				{
					u_int32 value_u;
					stringstream value_s(g_service.internalconfigStruct.IPType);
					value_s >> value_u;	
					configStruct.uIPType = value_u;
				}
				else
				{
					configStruct.uIPType = 0;
				}

				if (!g_service.internalconfigStruct.PANI.empty())
				{
					configStruct.pPANI = (u_char *)g_service.internalconfigStruct.PANI.c_str();
				}
				else
				{
					configStruct.pPANI = NULL;
				}

				if (!g_service.internalconfigStruct.SecurityVerify.empty())
				{
					configStruct.pSecurityVerify = (u_char *)g_service.internalconfigStruct.SecurityVerify.c_str();
				}
				else
				{
					configStruct.pSecurityVerify = NULL;
				}

				if (!g_service.internalconfigStruct.AssociatedUri.empty())
				{
					configStruct.pAssociatedUri = (u_char *)g_service.internalconfigStruct.AssociatedUri.c_str();
				}
				else
				{
					configStruct.pAssociatedUri = NULL;
				}

				if (!g_service.internalconfigStruct.UriUserPart.empty())
				{
					configStruct.pUriUserPart = (u_char *)g_service.internalconfigStruct.UriUserPart.c_str();
				}
				else
				{
					configStruct.pUriUserPart = NULL;
				}

				if (!g_service.internalconfigStruct.IMEI.empty())
				{
					configStruct.pIMEI = (u_char *)g_service.internalconfigStruct.IMEI.c_str();
				}
				else
				{
					configStruct.pIMEI = NULL;
				}
				if (g_service.bIsUpdatedNetworkParams != Enum_TRUE)
				{
					QCMLOGI(g_service.logHandle, KLogTypeGeneral, "%s:%u\teEvent=%u", __FUNCTION__, __LINE__, (uint32_t)e);
					g_service.pEventCallbackFn(g_service.pEventCallbackFnData, e, (void *)&configStruct);
					g_service.bIsUpdatedNetworkParams = Enum_TRUE;
				}

				QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\ttriggerRegistration() is calling", __FUNCTION__, __LINE__);
				if (g_service.eVersion == APIVersion_20)
				{
					status = g_service.service2_0->triggerRegistration(g_service.uConnectionManager, g_service.uUserData);
				}
				else if (g_service.eVersion == APIVersion_21)
				{
					status = g_service.service2_1->triggerRegistration(g_service.uConnectionManager, g_service.uUserData);
				}
				else if (g_service.eVersion == APIVersion_22)
				{
					status = g_service.service->triggerRegistration(g_service.uConnectionManager, g_service.uUserData);
				}
				if ((StatusCode)status != StatusCode::SUCCESS)
				{
					QCMLOGE(g_service.logHandle, KLogTypeGeneral, "%s:%u triggerRegistration() is failed: error=%u", __FUNCTION__, __LINE__, status);
					e = ConnectionEvent_NotRegistered;
					g_service.pEventCallbackFn(g_service.pEventCallbackFnData, e, NULL);
				}
				QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\ttriggerRegistration() returns %u", __FUNCTION__, __LINE__, status);

				QcmServiceDiedDisabled();

				if (g_service.bEnableGlobalMutex)
				{
					pal_MutexGlobalUnlock(g_service.pal);
				}
			}
		}
		break;

		case QCMConnectionEvent_GetACSConfiguration:
		{
			ConnectionEvents e = ConnectionEvent_GetACSConfiguration;
			QcmACSConfigStruct *pStruct = (QcmACSConfigStruct *)pParam;

			if (g_service.pEventCallbackFn != NULL)
			{
				if (g_service.bEnableGlobalMutex)
				{
					pal_MutexGlobalLock(g_service.pal);
				}

				QCMLOGI(g_service.logHandle, KLogTypeGeneral, "%s:%u\teEvent=%u", __FUNCTION__, __LINE__, (uint32_t)e);
				g_service.pEventCallbackFn(g_service.pEventCallbackFnData, e, (void *)pStruct);

				if (g_service.bEnableGlobalMutex)
				{
					pal_MutexGlobalUnlock(g_service.pal);
				}
			}

			if (pStruct->pConfig != NULL)
			{
				pal_MemoryFree((void**) &pStruct->pConfig);
				pStruct->pConfig = NULL;
			}

			if (pStruct != NULL)
			{
				pal_MemoryFree((void**) &pStruct);
				pStruct = NULL;
			}
		}
		break;

		case QCMConnectionEvent_AutoconfigRequestStatus:
		{
			ConnectionEvents e = ConnectionEvent_AutoconfigRequestStatus;
			u_int32 status = uParam;

			if (g_service.pEventCallbackFn != NULL)
			{
				if (g_service.bEnableGlobalMutex)
				{
					pal_MutexGlobalLock(g_service.pal);
				}

				QCMLOGI(g_service.logHandle, KLogTypeGeneral, "%s:%u\teEvent=%u", __FUNCTION__, __LINE__, (uint32_t)e);
				g_service.pEventCallbackFn(g_service.pEventCallbackFnData, e, (void *)&status);

				if (g_service.bEnableGlobalMutex)
				{
					pal_MutexGlobalUnlock(g_service.pal);
				}
			}
		}
		break;

		case QCMConnectionEvent_ServiceDied:
		{
			if (g_service.bEnableGlobalMutex)
			{
				pal_MutexGlobalLock(g_service.pal);
			}

			g_service.bIsConnectionAlived = Enum_FALSE;

			u_int32 error = KPALErrorNone;
			if (g_service.uNOfInit < 3)
			{
				g_service.uNOfInit++;
				QcmServiceDiedEnabled();

				error = QcmInternalInit();
				if (error != KPALErrorNone)
				{
					Qcm_signal_send(g_service.eventHandle, QCMConnectionEvent_ServiceDied, 0, NULL);
					QcmServiceDiedDisabled();
				}
			}
			else
			{
				QCMLOGE(g_service.logHandle, KLogTypeGeneral, "%s:%u QcmInternalInit() is failed", __FUNCTION__, __LINE__);
				g_service.pEventCallbackFn(g_service.pEventCallbackFnData, ConnectionEvent_NotRegistered, NULL);
				QcmServiceDiedDisabled();
			}

			if (g_service.bEnableGlobalMutex)
			{
				pal_MutexGlobalUnlock(g_service.pal);
			}
		}
		break;

		case QCMConnectionEvent_ReadyCreateConnection:
		{
			if (g_service.bEnableGlobalMutex)
			{
				pal_MutexGlobalLock(g_service.pal);
			}

			if (g_service.eEvent == connectionEvent::SERVICE_REGISTERED)
			{
				QcmServiceDiedDisabled();
				ConnectionEvents e = ConnectionEvent_GetConfiguration;
				QCMConfigStruct configStruct = {};
				pal_MemorySet(&configStruct, 0, sizeof(QCMConfigStruct));

				g_service.bIsConnectionAlived = Enum_TRUE;

				if (g_service.pEventCallbackFn != NULL)
				{
					if (!g_service.internalconfigStruct.SipPublicUserId.empty())
					{
						configStruct.pSipPublicUserId = (u_char *)g_service.internalconfigStruct.SipPublicUserId.c_str();
					}
					else
					{
						configStruct.pSipPublicUserId = NULL;
					}

					if (!g_service.internalconfigStruct.SipPrivateUserId.empty())
					{
						configStruct.pSipPrivateUserId = (u_char *)g_service.internalconfigStruct.SipPrivateUserId.c_str();
					}
					else
					{
						configStruct.pSipPrivateUserId = NULL;
					}

					if (!g_service.internalconfigStruct.SipHomeDomain.empty())
					{
						configStruct.pSipHomeDomain = (u_char *)g_service.internalconfigStruct.SipHomeDomain.c_str();
					}
					else
					{
						configStruct.pSipHomeDomain = NULL;
					}

					if (!g_service.internalconfigStruct.SipOutBoundProxyName.empty())
					{
						configStruct.pSipOutBoundProxyName = (u_char *)g_service.internalconfigStruct.SipOutBoundProxyName.c_str();
					}
					else
					{
						configStruct.pSipOutBoundProxyName = NULL;
					}

					if (!g_service.internalconfigStruct.LocalHostIPAddress.empty())
					{
						configStruct.pLocalHostIPAddress = (u_char *)g_service.internalconfigStruct.LocalHostIPAddress.c_str();
					}
					else
					{
						configStruct.pLocalHostIPAddress = NULL;
					}

					if (!g_service.internalconfigStruct.PCSCFClientPort.empty())
					{
						u_int32 value_u;
						stringstream value_s(g_service.internalconfigStruct.PCSCFClientPort);
						value_s >> value_u;	
						configStruct.uPCSCFClientPort = value_u;
					}
					else
					{
						configStruct.uPCSCFClientPort = 0;
					}

					if (!g_service.internalconfigStruct.PCSCFServerPort.empty())
					{
						u_int32 value_u;
						stringstream value_s(g_service.internalconfigStruct.PCSCFServerPort);
						value_s >> value_u;	
						configStruct.uPCSCFServerPort = value_u;
					}
					else
					{
						configStruct.uPCSCFServerPort = 0;
					}

					if (!g_service.internalconfigStruct.UEClientPort.empty())
					{
						u_int32 value_u;
						stringstream value_s(g_service.internalconfigStruct.UEClientPort);
						value_s >> value_u;	
						configStruct.uUEClientPort = value_u;
					}
					else
					{
						configStruct.uUEClientPort = 0;
					}

					if ((configStruct.uPCSCFServerPort == 0) && (configStruct.uUEClientPort == 0))
					{
						if (!g_service.internalconfigStruct.SipOutBoundProxyPort.empty())
						{
							u_int32 value_u;
							stringstream value_s(g_service.internalconfigStruct.SipOutBoundProxyPort);
							value_s >> value_u;	
							configStruct.uSipOutBoundProxyPort = value_u;
						}
						else
						{
							configStruct.uSipOutBoundProxyPort = 0;
						}					
					}

					if (!g_service.internalconfigStruct.IPType.empty())
					{
						u_int32 value_u;
						stringstream value_s(g_service.internalconfigStruct.IPType);
						value_s >> value_u;	
						configStruct.uIPType = value_u;
					}
					else
					{
						configStruct.uIPType = 0;
					}

					if (!g_service.internalconfigStruct.PANI.empty())
					{
						configStruct.pPANI = (u_char *)g_service.internalconfigStruct.PANI.c_str();
					}
					else
					{
						configStruct.pPANI = NULL;
					}

					if (!g_service.internalconfigStruct.SecurityVerify.empty())
					{
						configStruct.pSecurityVerify = (u_char *)g_service.internalconfigStruct.SecurityVerify.c_str();
					}
					else
					{
						configStruct.pSecurityVerify = NULL;
					}

					if (!g_service.internalconfigStruct.AssociatedUri.empty())
					{
						configStruct.pAssociatedUri = (u_char *)g_service.internalconfigStruct.AssociatedUri.c_str();
					}
					else
					{
						configStruct.pAssociatedUri = NULL;
					}

					if (!g_service.internalconfigStruct.UriUserPart.empty())
					{
						configStruct.pUriUserPart = (u_char *)g_service.internalconfigStruct.UriUserPart.c_str();
					}
					else
					{
						configStruct.pUriUserPart = NULL;
					}

					if (!g_service.internalconfigStruct.IMEI.empty())
					{
						configStruct.pIMEI = (u_char *)g_service.internalconfigStruct.IMEI.c_str();
					}
					else
					{
						configStruct.pIMEI = NULL;
					}

					g_service.pEventCallbackFn(g_service.pEventCallbackFnData, e, (void *)&configStruct);
				}
			}
			else
			{
				u_int32 error = KPALErrorNone;
				error = QcmInternalRegister();
				if (error != KPALErrorNone)
				{
					QCMLOGE(g_service.logHandle, KLogTypeGeneral, "%s:%u register is failed", __FUNCTION__, __LINE__);
					g_service.pEventCallbackFn(g_service.pEventCallbackFnData, ConnectionEvent_NotRegistered, NULL);
					QcmServiceDiedDisabled();
				}
			}

			if (g_service.bEnableGlobalMutex)
			{
				pal_MutexGlobalUnlock(g_service.pal);
			}
		}
		break;

		default:
		{
			QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u Invalid command : %u", __FUNCTION__, __LINE__, command);
		}
		break;
	}
	
	return 0;
}

/**
 * Implementation of the pal_QcmInit() function. See the
 * EcrioPAL_QCM.h file for interface definitions.
 */
u_int32 pal_QcmInit
(
	PALINSTANCE pal,
	LOGHANDLE logHandle,
	QCMINSTANCE *instance,
	u_char *pIccid,
	QcmEventCallback pEventCallbackFn,
	void *pCallbackFnData,
	BoolEnum bEnableGlobalMutex
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;
	u_int32 error = KPALErrorNone;

	if (pIccid == NULL)
	{
		return KPALInvalidParameters;
	}

	if (instance == NULL)
	{
		return KPALInvalidParameters;
	}
	*instance = NULL;

	QCMLOGI(logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	QCMLOGI(logHandle, KLogTypeGeneral, "%s:%u\tpIccid=%s", __FUNCTION__, __LINE__, pIccid);
	QCMLOGI(logHandle, KLogTypeGeneral, "%s:%u\tpEventCallbackFn=%p, pCallbackFnData=%p, bEnableGlobalMutex=%u", __FUNCTION__, __LINE__, pEventCallbackFn, pCallbackFnData, bEnableGlobalMutex);

	i->pal = pal;
	i->logHandle = logHandle;
	i->iccid.assign((const char *)pIccid);

	i->eVersion = APIVersion_None;

	i->pEventCallbackFn = pEventCallbackFn;
	i->pEventCallbackFnData = pCallbackFnData;
	i->bEnableGlobalMutex = bEnableGlobalMutex;

	i->uConnectionManager = 0L;
	i->uConnectionHandle = 0L;
	i->uListenerToken = 0L;
	i->uListenerId = 0L;

	i->eServiceStatus = ServiceStatus::STATUS_SERVICE_UNKNOWN;;
	i->eStatusCode = StatusCode::FAILURE;
	i->eEvent = connectionEvent::SERVICE_NOTREGISTERED;

	i->pReceiveCallbackFn = NULL;
	i->pErrorCallbackFn = NULL;
	i->pCallbackFnData = NULL;

	i->uNOfInit = 0;
	
	i->bIsConnectionAlived = Enum_FALSE;
	i->bIsUpdatedNetworkParams = Enum_FALSE;

	QcmServiceDiedDisabled();

	i->eventHandle = Qcm_signal_init(eventHandler, i->logHandle, &error);
	i->incomingHandle = Qcm_signal_init(incomingHandler, i->logHandle, &error);
	i->errorHandle = Qcm_signal_init(errorHandler, i->logHandle, &error);

	error = QcmInternalInit();
	if (error != KPALErrorNone)
	{
		Qcm_signal_deinit(&(i->errorHandle));
		Qcm_signal_deinit(&(i->incomingHandle));
		Qcm_signal_deinit(&(i->eventHandle));
	}

	*instance = (QCMINSTANCE)i;

	QCMLOGI(i->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return error;
}

/**
 * Implementation of the pal_QcmDeinit() function. See the
 * EcrioPAL_QCM.h file for interface definitions.
 */
void pal_QcmDeinit
(
	QCMINSTANCE instance
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)instance;

	if (i == NULL)
	{
		return;
	}

	QCMLOGI(i->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	i->bIsConnectionAlived = Enum_FALSE;
	i->bIsUpdatedNetworkParams = Enum_FALSE;

	uint32_t status = 0;

#if 0
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tremoveListener() is calling", __FUNCTION__, __LINE__);
	status = i->connection->removeListener(i->uListenerToken);
	if ((StatusCode)status != StatusCode::SUCCESS)
	{
		QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u removeListener() is failed: error=%u", __FUNCTION__, __LINE__, status);
	}
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tremoveListener() returns %u", __FUNCTION__, __LINE__, status);
#endif

	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tcloseConnection() is calling", __FUNCTION__, __LINE__);
	if (i->eVersion == APIVersion_20)
	{
		status = i->service2_0->closeConnection(i->uConnectionManager, i->uConnectionHandle);
	}
	else if (i->eVersion == APIVersion_21)
	{
		status = i->service2_1->closeConnection(i->uConnectionManager, i->uConnectionHandle);
	}
	else if (i->eVersion == APIVersion_22)
	{
		status = i->service->closeConnection(i->uConnectionManager, i->uConnectionHandle);
	}
	if ((StatusCode)status != StatusCode::SUCCESS)
	{
		QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u closeConnection() is failed: error=%u", __FUNCTION__, __LINE__, status);
	}
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tcloseConnection() returns %u", __FUNCTION__, __LINE__, status);

	QcmConfigStrutDelete();

	QcmUnlinkToDeath();

	QcmCloseService();

	Qcm_signal_deinit(&(i->errorHandle));
	Qcm_signal_deinit(&(i->incomingHandle));
	Qcm_signal_deinit(&(i->eventHandle));

	QCMLOGI(i->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
}

/**
 * Implementation of the pal_QcmSetCallback() function. See the
 * EcrioPAL_QCM.h file for interface definitions.
 */
u_int32 pal_QcmSetCallback
(
	PALINSTANCE pal,
	QcmCallbackFnStruct *pFnStruct,
	QCMINSTANCE *instance
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;
	u_int32 error = KPALErrorNone;

	(void)pal;

	QCMLOGI(i->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	i->pReceiveCallbackFn = pFnStruct->pReceiveCallbackFn;
	i->pErrorCallbackFn = pFnStruct->pErrorCallbackFn;
	i->pCallbackFnData = pFnStruct->pCallbackFnData;

	*instance = (QCMINSTANCE)i;

	QCMLOGI(i->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return error;
}

/**
 * Implementation of the pal_QcmSendData() function. See the
 * EcrioPAL_QCM.h file for interface definitions.
 */
u_int32 pal_QcmSendData
(
	PALINSTANCE pal,
	const u_char *pDataBuffer,
	u_int32 dataLength,
	IPProtocolTypes protocol,
	MessageTypes messageType,
	u_char *pOutboundProxy,
	u_int32 uRemotePort,
	u_char *pCallId
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;
	u_int32 error = KPALErrorNone;
	
	(void)pal;

	if (pDataBuffer == NULL || dataLength == 0 || pOutboundProxy == NULL || pCallId == NULL)
	{
		return KPALInvalidParameters;
	}

	if (i->eEvent != connectionEvent::SERVICE_REGISTERED)
	{
		return KPALInternalError;
	}

	QCMLOGI(i->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tpDataBuffer=%p, dataLength=%u", __FUNCTION__, __LINE__, pDataBuffer, dataLength);
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tprotocol=%u, messageType=%u", __FUNCTION__, __LINE__, protocol, messageType);
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tpOutboundProxy=%s, uRemotePort=%u, pCallId=%s", __FUNCTION__, __LINE__, pOutboundProxy, uRemotePort, pCallId);
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tuUserData=%u", __FUNCTION__, __LINE__, i->uUserData);

	u_int32 p = (u_int32)sipProtocolType::UDP;
	if (protocol == IPProtocolType_TCP)
	{
		p = (u_int32)sipProtocolType::TCP;
	}

	u_int32 m = (u_int32)messageType::TYPE_REQUEST;
	if (messageType == MessageType_Response)
	{
		m = (u_int32)messageType::TYPE_RESPONSE;
	}

	string c((const char *)pCallId);
	string proxy((const char *)pOutboundProxy);

	outgoingMessage connMessage;
	connMessage.data.resize(5);
	{
		connMessage.data[0].key = (uint32_t)outgoingMessageKeys::MessageType;
		connMessage.data[0].value = std::to_string(m);
	}
	{
		connMessage.data[1].key = (uint32_t)outgoingMessageKeys::OutboundProxy;
		connMessage.data[1].value = proxy;
	}
	{
		connMessage.data[2].key = (uint32_t)outgoingMessageKeys::RemotePort;
		connMessage.data[2].value = std::to_string(uRemotePort);
	}
	{
		connMessage.data[3].key = (uint32_t)outgoingMessageKeys::Protocol;
		connMessage.data[3].value = std::to_string(p);
	}
	{
		connMessage.data[4].key = (uint32_t)outgoingMessageKeys::CallId;
		connMessage.data[4].value = c;
	}

	char* sipPacket = new char[dataLength+1];
	pal_MemoryCopy(sipPacket, dataLength+1, pDataBuffer, dataLength);
	connMessage.bufferData.resize(1);
	connMessage.bufferData[0].key = (uint32_t)outgoingMessageKeys::Message;
	connMessage.bufferData[0].value.resize(dataLength);
	connMessage.bufferData[0].value.setToExternal((uint8_t*)sipPacket, dataLength, true);

	uint32_t status = 0;
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tsendMessage() is calling", __FUNCTION__, __LINE__);
	status = i->connection->sendMessage(connMessage, i->uUserData);
	if ((StatusCode)status != StatusCode::SUCCESS)
	{
		QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u sendMessage() is failed: error=%u", __FUNCTION__, __LINE__, status);
		return KPALInternalError;
	}
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tsendMessage() returns %u", __FUNCTION__, __LINE__, status);

#if 0
	unique_lock<mutex> sendState(i->mConnection);
	i->condConnection.wait(sendState);
	sendState.unlock();
#endif

	QCMLOGI(i->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return error;
}

/**
 * Implementation of the pal_QcmRegister() function. See the
 * EcrioPAL_QCM.h file for interface definitions.
 */
u_int32 pal_QcmRegister
(
	PALINSTANCE pal,
	u_char *pFeatures
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;
	u_int32 error = KPALErrorNone;
	uint32_t status = 0;
	
	(void)pal;

	QCMLOGI(i->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

//	if (i->eEvent == connectionEvent::SERVICE_NOTREGISTERED)
//	{
//		return KPALNotAvailable;
//	}
//	else if (i->eEvent != connectionEvent::SERVICE_CREATED || i->eEvent != connectionEvent::SERVICE_REGISTERED)
//	{
//		return KPALInternalError;
//	}

	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tpFeatures=%s", __FUNCTION__, __LINE__, pFeatures);

	if (pFeatures == NULL)
	{
		i->featureTag = "";
	}
	else
	{
		i->featureTag.assign((const char *)pFeatures);
	}

	error = QcmInternalRegister();

	QCMLOGI(i->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return error;
}

/**
 * Implementation of the pal_QcmDeregister() function. See the
 * EcrioPAL_QCM.h file for interface definitions.
 */
u_int32 pal_QcmDeregister
(
	PALINSTANCE pal
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;
	u_int32 error = KPALErrorNone;
	uint32_t status = 0;

	(void)pal;

	QCMLOGI(i->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tuUserData=%u", __FUNCTION__, __LINE__, i->uUserData);

	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tcloseAllTransactions() is calling", __FUNCTION__, __LINE__);
	status = i->connection->closeAllTransactions(i->uUserData);
	if ((StatusCode)status != StatusCode::SUCCESS)
	{
		QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u closeAllTransactions() is failed: error=%u", __FUNCTION__, __LINE__, status);
		return KPALInternalError;
	}
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tcloseAllTransactions() returns %u", __FUNCTION__, __LINE__, status);

#if 0
	unique_lock<mutex> closeTransactionState(i->mConnection);
	i->condConnection.wait(closeTransactionState);
	closeTransactionState.unlock();
#endif

	if (i->eStatusCode != StatusCode::SUCCESS)
	{
		QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u closeAllTransactions() is failed: error=%u", __FUNCTION__, __LINE__, (uint32_t)i->eStatusCode);
		i->eStatusCode = StatusCode::FAILURE;
		return KPALInternalError;
	}

	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\ttriggerDeRegistration() is calling", __FUNCTION__, __LINE__);
	if (i->eVersion == APIVersion_20)
	{
		status = i->service2_0->triggerDeRegistration(i->uConnectionManager, i->uUserData);
	}
	else if (i->eVersion == APIVersion_21)
	{
		status = i->service2_1->triggerDeRegistration(i->uConnectionManager, i->uUserData);
	}
	else if (i->eVersion == APIVersion_22)
	{
		status = i->service->triggerDeRegistration(i->uConnectionManager, i->uUserData);
	}
	if ((StatusCode)status != StatusCode::SUCCESS)
	{
		QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u triggerDeRegistration() is failed: status=%u", __FUNCTION__, __LINE__, (uint32_t)status);
		return KPALInternalError;
	}
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\ttriggerDeRegistration() returns %u", __FUNCTION__, __LINE__, status);

	QCMLOGI(i->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return error;
}

/**
 * Implementation of the pal_QcmGetConfig() function. See the
 * EcrioPAL_QCM.h file for interface definitions.
 */
u_int32 pal_QcmGetConfig
(
	PALINSTANCE pal,
	ConfigurationTypes type
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;
	u_int32 error = KPALErrorNone;

	(void)pal;

	QCMLOGI(i->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\ttype=%u", __FUNCTION__, __LINE__, type);

	uint32_t status = 0;
	ConfigType configType = ConfigType::AUTO_CONFIG;

	if (type == ConfigurationType_User)
	{
		configType = ConfigType::USER_CONFIG;
	}
	else if (type == ConfigurationType_Device)
	{
		configType = ConfigType::DEVICE_CONFIG;
	}

	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tconfigType=%u, uUserData=%u", __FUNCTION__, __LINE__, (u_int32)configType, i->uUserData);
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tgetConfiguration() is calling", __FUNCTION__, __LINE__);
	if (i->eVersion == APIVersion_20)
	{
		status = i->service2_0->getConfiguration(i->uConnectionManager, configType, i->uUserData);
	}
	else if (i->eVersion == APIVersion_21)
	{
		status = i->service2_1->getConfiguration(i->uConnectionManager, configType, i->uUserData);
	}
	else if (i->eVersion == APIVersion_22)
	{
		status = i->service->getConfiguration(i->uConnectionManager, configType, i->uUserData);
	}
	if ((StatusCode)status != StatusCode::SUCCESS)
	{
		QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u getConfiguration() is failed: status=%u", __FUNCTION__, __LINE__, (uint32_t)status);
		return KPALInternalError;
	}
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tgetConfiguration() returns %u", __FUNCTION__, __LINE__, status);

	QCMLOGI(i->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return error;
}

/**
 * Implementation of the pal_QcmCloseTransation() function. See the
 * EcrioPAL_QCM.h file for interface definitions.
 */
u_int32 pal_QcmCloseTransation
(
	PALINSTANCE pal,
	u_char *pCallId
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;
	u_int32 error = KPALErrorNone;

	(void)pal;

	if (pCallId == NULL)
	{
		return KPALInvalidParameters;
	}

	if (i->eEvent != connectionEvent::SERVICE_REGISTERED)
	{
		return KPALInternalError;
	}

	QCMLOGI(i->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tpCallId=%s, uUserData=%u", __FUNCTION__, __LINE__, pCallId, i->uUserData);

#if 0
	uint32_t status = 0;
	string callId((const char *)pCallId);
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tcloseTransaction() is calling callid=%s, uUserData=%u", __FUNCTION__, __LINE__, callId.c_str(), i->uUserData);
	status = i->connection->closeTransaction(callId, i->uUserData);
	if ((StatusCode)status != StatusCode::SUCCESS)
	{
		QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u closeTransaction() is failed: error=%u", __FUNCTION__, __LINE__, status);
		return KPALInternalError;
	}
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tcloseTransaction() returns %u", __FUNCTION__, __LINE__, status);
#endif

	QCMLOGI(i->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return error;
}

/**
 * Implementation of the pal_QcmCloseAllTransations() function. See the
 * EcrioPAL_QCM.h file for interface definitions.
 */
u_int32 pal_QcmCloseAllTransations
(
	PALINSTANCE pal
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;
	u_int32 error = KPALErrorNone;
	
	(void)pal;

	if (i->eEvent != connectionEvent::SERVICE_REGISTERED)
	{
		return KPALInternalError;
	}

	QCMLOGI(i->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tuUserData=%u", __FUNCTION__, __LINE__, i->uUserData);

#if 0
	uint32_t status = 0;
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tcloseAllTransactions() is calling", __FUNCTION__, __LINE__);
	status = i->connection->closeAllTransactions(i->uUserData);
	if ((StatusCode)status != StatusCode::SUCCESS)
	{
		QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u closeAllTransactions() is failed: error=%u", __FUNCTION__, __LINE__, status);
		return KPALInternalError;
	}
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tcloseAllTransactions() returns %u", __FUNCTION__, __LINE__, status);
#endif

	QCMLOGI(i->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return error;
}

/**
 * Implementation of the pal_QcmTriggerACSRequest() function. See the
 * EcrioPAL_QCM.h file for interface definitions.
 */
u_int32 pal_QcmTriggerACSRequest
(
	PALINSTANCE pal,
	ACRTypes type
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;
	u_int32 error = KPALErrorNone;
	
	(void)pal;

	QCMLOGI(i->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\ttype=%u", __FUNCTION__, __LINE__, type);

	uint32_t status = 0;
	autoconfigTriggerReason acrType = autoconfigTriggerReason::AUTOCONFIG_USER_REQUEST;

	if (type == ACRType_UserRequest)
	{
		acrType = autoconfigTriggerReason::AUTOCONFIG_USER_REQUEST;
	}
	else if (type == ACRType_RefreshToken)
	{
		acrType = autoconfigTriggerReason::AUTOCONFIG_REFRESH_TOKEN;
	}
	else if (type == ACRType_InvalidToken)
	{
		acrType = autoconfigTriggerReason::AUTOCONFIG_INVALID_TOKEN;
	}
	else if (type == ACRType_InvalidCredential)
	{
		acrType = autoconfigTriggerReason::AUTOCONFIG_INVALID_CREDENTIAL;
	}
	else if (type == ACRType_ClientChange)
	{
		acrType = autoconfigTriggerReason::AUTOCONFIG_CLIENT_CHANGE;
	}
	else if (type == ACRType_DeviceUpgrde)
	{
		acrType = autoconfigTriggerReason::AUTOCONFIG_DEVICE_UPGRADE;
	}
	else if (type == ACRType_FactoryReset)
	{
		acrType = autoconfigTriggerReason::AUTOCONFIG_FACTORY_RESET;
	}

	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tacrType=%u, uUserData=%u", __FUNCTION__, __LINE__, (u_int32)acrType, i->uUserData);
	if (i->eVersion == APIVersion_20)
	{
		QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u triggerACSRequest() is failed: status=%u", __FUNCTION__, __LINE__, (uint32_t)status);
		return KPALNotSupported;
	}
	else if (i->eVersion == APIVersion_21)
	{
		QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\ttriggerACSRequest() is calling", __FUNCTION__, __LINE__);
		status = i->service2_1->triggerACSRequest(i->uConnectionManager, acrType, i->uUserData);
	}
	else if (i->eVersion == APIVersion_22)
	{
		QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\ttriggerACSRequest() is calling", __FUNCTION__, __LINE__);
		status = i->service->triggerACSRequest(i->uConnectionManager, acrType, i->uUserData);
	}
	if ((StatusCode)status != StatusCode::SUCCESS)
	{
		QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u triggerACSRequest() is failed: status=%u", __FUNCTION__, __LINE__, (uint32_t)status);
		return KPALInternalError;
	}
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\ttriggerACSRequest() returns %u", __FUNCTION__, __LINE__, status);

	QCMLOGI(i->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return error;
}

/*
 * Implementation of the pal_QcmCloseConnection() function. See the
 * EcrioPAL_QCM.h file for interface definitions.
 */
u_int32 pal_QcmCloseConnection
(
	PALINSTANCE pal
)
{
	ImsCmServiceStruct *i = (ImsCmServiceStruct *)&g_service;
	u_int32 error = KPALErrorNone;

	QCMLOGI(i->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	i->bIsConnectionAlived = Enum_FALSE;

	uint32_t status = 0;

	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tcloseConnection() is calling", __FUNCTION__, __LINE__);
	if (i->eVersion == APIVersion_20)
	{
		status = i->service2_0->closeConnection(i->uConnectionManager, i->uConnectionHandle);
	}
	else if (i->eVersion == APIVersion_21)
	{
		status = i->service2_1->closeConnection(i->uConnectionManager, i->uConnectionHandle);
	}
	else if (i->eVersion == APIVersion_22)
	{
		status = i->service->closeConnection(i->uConnectionManager, i->uConnectionHandle);
	}
	if ((StatusCode)status != StatusCode::SUCCESS)
	{
		QCMLOGE(i->logHandle, KLogTypeGeneral, "%s:%u closeConnection() is failed: error=%u", __FUNCTION__, __LINE__, status);
		return KPALInternalError;
	}
	QCMLOGI(i->logHandle, KLogTypeGeneral, "%s:%u\tcloseConnection() returns %u", __FUNCTION__, __LINE__, status);

	i->eEvent = (connectionEvent)connectionEvent::SERVICE_NOTREGISTERED;
	Qcm_signal_send(i->eventHandle, QCMConnectionEvent_NotRegistered, 0, NULL);	

	QCMLOGI(i->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return error;
}