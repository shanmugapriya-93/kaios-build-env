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
* @file lims_internal.h
* @brief This file defines the primary internal definitions for the lims modules
*/

#ifndef __LIMSINTERNAL_H__
#define __LIMSINTERNAL_H__

#include "EcrioPAL.h"
#include "EcrioSigMgrCommon.h"
#include "EcrioSigMgrInit.h"
#include "EcrioSUEEngine.h"

#ifdef ENABLE_RCS
#include "EcrioCPM.h"
//#include "EcrioUCE.h"
#endif
#include "EcrioSIPDataTypes.h"

/* Implementation level errors. */
#define LIMS_INVALID_PARAMETER1									4001/**< */
#define LIMS_INVALID_PARAMETER2									4002/**< */
#define LIMS_INVALID_PARAMETER3									4003/**< */
#define LIMS_INVALID_PARAMETER4									4004/**< */
#define LIMS_INVALID_OPERATION									4005/**< */
#define LIMS_NO_MEMORY											4006/**< */
#define LIMS_NOT_SUPPORTED										4007/**< */
#define LIMS_SIGMGR_INIT_ERROR									4008/**< */
#define LIMS_SUE_INIT_ERROR										4009/**< */
#define LIMS_SMS_INIT_ERROR										4010/**< */
#define LIMS_SIGMGR_SET_PARAM_ERROR								4011/**< */
#define LIMS_SIGMGR_SET_CALLBACK_ERROR							4012/**< */
#define LIMS_SIGMGR_DEINIT_ERROR								4013/**< */
#define LIMS_SUE_DEINIT_ERROR									4014/**< */
#define LIMS_SMS_DEINIT_ERROR									4015/**< */
#define LIMS_DEREGISTER_FIRST									4016/**< */
#define LIMS_INVALID_STATE										4017/**< */
#define LIMS_SUE_REGISTER_ERROR									4018/**< */
#define LIMS_SUE_SET_NETWORK_PARAM_ERROR						4019/**< */
#define LIMS_SMS_SEND_MESSAGE_ERROR								4020/**< */
#define LIMS_MOIP_INIT_ERROR									4021/**< */
#define LIMS_MOIP_DEINIT_ERROR									4022/**< */
#define LIMS_MOIP_STARTCALL_ERROR								4023/**< */
#define LIMS_MOIP_ENDCALL_ERROR									4024/**< */
#define LIMS_MOIP_RESPONDCALL_ERROR								4025/**< */
#define LIMS_SUE_SUBSCRIBE_ERROR								4026/**< */
#define LIMS_MOIP_FEATURE_NOT_SUPPORTED							4027/**< */
#define LIMS_SMS_FEATURE_NOT_SUPPORTED							4028/**< */
#define LIMS_CPM_FEATURE_NOT_SUPPORTED							4029/**< */
#define LIMS_CPM_INIT_ERROR										4030/**< */
#define LIMS_CPM_DEINIT_ERROR									4031/**< */
#define LIMS_CPM_STARTSESSION_ERROR								4032/**< */
#define LIMS_CPM_ENDSESSION_ERROR								4033/**< */
#define LIMS_CPM_RESPONDSESSION_ERROR							4034/**< */
#define LIMS_CPM_SENDMESSAGE_ERROR								4035/**< */
#define LIMS_CPM_SENDFILE_ERROR									4036/**< */
#define LIMS_CPM_SENDRICHCARD_ERROR								4037/**< */
#define LIMS_CPM_FORMIMDN_ERROR									4038/**< */
#define LIMS_INVALID_CHANNEL_SETTING							4039/**< */
#define LIMS_CPM_ABORT_FILETRANSFER_ERROR						4040/**< */
#define LIMS_SUE_NO_IMMEDIATE_EXIT_ERROR						4048/**< */
#define LIMS_CPM_START_GROUPCHATSESSION_ERROR					4049/**< */
#define LIMS_CPM_RESPOND_GROUPCHATSESSION_ERROR					4050/**< */
#define LIMS_CPM_END_GROUPCHATSESSION_ERROR						4051/**< */
#define LIMS_CPM_ADD_PARTICIPANTS_ERROR							4052/**< */
#define LIMS_CPM_REMOVE_PARTICIPANTS_ERROR						4053/**< */
#define LIMS_CPM_SET_GROUPCHAT_SUBJECT_ERROR					4054/**< */
#define LIMS_CPM_DELETE_GROUPCHAT_SUBJECT_ERROR					4055/**< */
#define LIMS_CPM_SUBSCRIBE_PARTICIPANT_INFO_ERROR				4056/**< */
#define LIMS_CPM_ABORT_SESSIONID_ERROR							4057/**< */
#define LIMS_CPM_SEND_PRIVACYMANAGEMENTCOMMAND_ERROR			4058/**< */
#define LIMS_CPM_SEND_SPAMREPORTMESSAGE_ERROR					4059/**< */
#define LIMS_UCE_INIT_ERROR                     				4060/**< */
#define LIMS_UCE_DEINIT_ERROR                     				4061/**< */
#define LIMS_UCE_PUBLISH_INITIATE_ERROR							4062/**< */
#define LIMS_UCE_PUBLISH_MODIFY_ERROR							4063/**< */
#define LIMS_UCE_PUBLISH_REMOVE_ERROR							4064/**< */
#define LIMS_UCE_CAPABILITY_QUERY_ERROR						4065/**< */
#define LIMS_CPM_SET_NETWORK_PARAM_ERROR						4066/**< */


/* @Constant Strings
*/
/* The constant string for feature tags*/
#define LIMS_3GPP_ICSI_FEATURE_TAG_STRING						(const u_char *)"+g.3gpp.icsi-ref"
#define LIMS_3GPP_IARI_FEATURE_TAG_STRING						(const u_char *)"+g.3gpp.iari-ref"
#define LIMS_3GPP_CHATBOT_VERSION_FEATURE_TAG_STRING			(const u_char *)"+g.gsma.rcs.botversion"
#define LIMS_3GPP_ISCHATBOT_FEATURE_TAG_STRING					(const u_char *)"+g.gsma.rcs.isbot"
#define LIMS_3GPP_ICSI_MMTEL_FEATURE_TAG_VALUE_STRING			(const u_char *)"\"urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\""
#define LIMS_SIP_INSTANCE_FEATURE_TAG_STRING					(const u_char *)"+sip.instance"
#define LIMS_SIP_INSTANCE_FEATURE_TAG_VALUE_STRING				(const char *)"\"<urn:gsma:imei:%s>\""
#define LIMS_SMS_SMSIP_FEATURE_TAG_STRING						(const u_char *)"+g.3gpp.smsip"
#define LIMS_GSMA_RCS_TELEPHONY_FEATURE_TAG_STRING				(const u_char *)"+g.gsma.rcs.telephony"
#define LIMS_GSMA_RCS_TELEPHONY_FEATURE_TAG_VALUE_STRING		(const u_char *)"\"cs,volte\""
#define LIMS_3GPP_ICSI_PAGER_MODE_FEATURE_TAG_VALUE_STRING		(const u_char *)"\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.msg\""
#define LIMS_3GPP_ICSI_LARGE_MODE_FEATURE_TAG_VALUE_STRING		(const u_char *)"\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.largemsg\""
#define LIMS_3GPP_IARI_FT_FEATURE_TAG_VALUE_STRING				(const u_char *)"\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fthttp\""
#define LIMS_3GPP_IARI_CHATBOT_FEATURE_TAG_VALUE_STRING			(const u_char *)"\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot\""
#define LIMS_3GPP_ICSI_CPM_SESSION_FEATURE_TAG_VALUE_STRING		(const u_char *)"\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session\""
#define LIMS_3GPP_IARI_CPM_CHAT_FEATURE_TAG_VALUE_STRING		(const u_char *)"\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.im\""
#define LIMS_3GPP_ICSI_FT_SF_FEATURE_TAG_VALUE_STRING			(const u_char *)"\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftstandfw\""
#define LIMS_3GPP_ICSI_DEFERRED_FEATURE_TAG_VALUE_STRING		(const u_char *)"\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.deferred\""
#define LIMS_3GPP_IARI_GEOPUSH_FEATURE_TAG_VALUE_STRING		    (const u_char *)"\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geopush\""
#define LIMS_3GPP_HEADER_NAME_P_ACCESS_NETWORK_INFO				(const u_char *)"P-Access-Network-Info"
#define LIMS_3GPP_P_ACCESS_NETWORK_INFO_VALUE_STRING			(const u_char *)"3GPP2-1x-LTE"
#define LIMS_3GPP_ACCESS_TYPE_STRING							(const u_char *)"ci-3gpp2"
#define LIMS_3GPP_ACCESS_TYPE_VALUE_STRING						(const u_char *)"23415D0FCE11"
#define LIMS_IMDN_AGREEGATION_VALUE_STRING						(const u_char *)"imdn-aggregation"
#define LIMS_SIP_URI_LENGTH										256

/* @note Please note that macro definitions must be defined on single lines so
that line references remain intact during debugging and log output. */

/* Macros for logging. */

#ifdef ENABLE_LOG
#define LIMSLOGV(a, b, c, ...)		pal_LogMessage((a), KLogLevelVerbose, KLogComponentMedia, (b), (c),##__VA_ARGS__);
#define LIMSLOGI(a, b, c, ...)		pal_LogMessage((a), KLogLevelInfo, KLogComponentMedia, (b), (c),##__VA_ARGS__);
#define LIMSLOGD(a, b, c, ...)		pal_LogMessage((a), KLogLevelDebug, KLogComponentMedia, (b), (c),##__VA_ARGS__);
#define LIMSLOGW(a, b, c, ...)		pal_LogMessage((a), KLogLevelWarning, KLogComponentMedia, (b), (c),##__VA_ARGS__);
#define LIMSLOGE(a, b, c, ...)		pal_LogMessage((a), KLogLevelError, KLogComponentMedia, (b), (c),##__VA_ARGS__);
#else	// ENABLE_LOG
#define LIMSLOGV(a, b, c, ...)
#define LIMSLOGI(a, b, c, ...)
#define LIMSLOGD(a, b, c, ...)
#define LIMSLOGW(a, b, c, ...)
#define LIMSLOGE(a, b, c, ...)
#endif	// ENABLE_LOG

/* Macros for memory management. */

/* The standard macro for memory allocation. This assumes that upon any error, the memory is not allocated and the pointer is NULL,
   and vice versa. */
#define LIMS_ALLOC(size, ptr, err, label, log)						if (pal_MemoryAllocate((size), (ptr)) != KPALErrorNone) \
	{ \
		(err) = LIMS_NO_MEMORY; LIMSLOGE((log), KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() failed for %d", __FUNCTION__, __LINE__, (size)); goto label; \
	}

/* The standard macro for memory release. We check the return value only for logging purposes but do not act on it. */
#define LIMS_FREE(ptr, log)											if (pal_MemoryFree((ptr)) != KPALErrorNone) \
	{ \
		LIMSLOGE((log), KLogTypeGeneral, "%s:%u\tpal_MemoryFree() failed for %p", __FUNCTION__, __LINE__, (ptr)); \
	}

#define LIMS_ALLOC_AND_COPY_STRING(src, dst, err, label, log)		if ((dst = (char *)pal_StringCreate((const u_char *)(src), pal_StringLength(((const u_char *)src)))) == NULL) \
	{ \
		(err) = LIMS_NO_MEMORY; LIMSLOGE((log), KLogTypeGeneral, "%s:%u\t Failed to alloc and copy String:\t%s", __FUNCTION__, __LINE__, src); goto label; \
	}

/* Macros for fixed number. */

#define LIMS_COMMON_BUFFER_SIZE							1024


/**
*	module state enumerations.
*/
typedef enum
{
	lims_Module_State_UNINITIALIZED = 0,
	lims_Module_State_INITIALIZING,
	lims_Module_State_INITIALIZED,
	lims_Module_State_CONNECTED,
	lims_Module_State_REGISTERED
} lims_moduleStateEnum;

#ifdef ENABLE_QCMAPI
typedef enum
{
	_lims_REGISTRATION_STATE_ENUM_None = 0,
	_lims_REGISTRATION_STATE_ENUM_Registering,
	_lims_REGISTRATION_STATE_ENUM_Registered,
	_lims_REGISTRATION_STATE_ENUM_RegistrationFailed,
	_lims_REGISTRATION_STATE_ENUM_ReRegistering,
	_lims_REGISTRATION_STATE_ENUM_DeRegistering,
	_lims_REGISTRATION_STATE_ENUM_DeRegistered,
	_lims_REGISTRATION_STATE_ENUM_DeRegistrationFailed
}
_lims_registrationStateEnum;
#endif

#if 0
/**
* call state enumerations.
*/
typedef enum
{
	lims_Call_State_IDLE = 0,				/** Initial state. */
	lims_Call_State_CALLING,				/** Change to this state when user initiated a call. */
	lims_Call_State_RINGING,				/** Ringing state. */
	lims_Call_State_CONNECTED,				/** Call connected state. */
	lims_Call_State_PROGRESSING,			/** Ringing in progress state. ?*/
	lims_Call_State_INCOMING,				/** Incoming call state. */
	lims_Call_State_OnHOLD,
	lims_Call_State_MODIFYING,
	lims_Call_State_HELD
} lims_callStateEnum;


/**
* session state enumerations.
*/
typedef enum
{
	lims_Session_State_IDLE = 0,			/** Initial state. */
	lims_Session_State_INVITING,			/** Change to this state when user initiated a session. */
	lims_Session_State_ESTABLISHED,			/** Session established state. */
	lims_Session_State_INCOMING				/** Incoming session state. */
} lims_sessionStateEnum;
#endif //#if 0
#ifdef ENABLE_QCMAPI
typedef struct
{
	u_char *pSipPublicUserId;
	u_char *pSipHomeDomain;
	u_char *pSipPrivateUserId; 
	u_char *pPANI;
	u_char *pSecurityVerify;
	u_char *pAssociatedUri;
	u_char *pUriUserPart;
	u_char *pIMEI;
} lims_configInternalStruct;
#endif
/**
* module internal structure
*/
typedef struct
{
	PALINSTANCE pal;
	LOGHANDLE logHandle;
#ifdef ENABLE_QCMAPI
	QCMINSTANCE qcmInstance;
#endif
	lims_moduleStateEnum moduleState;
	lims_CallbackStruct pCallback;
	lims_ConfigStruct *pConfig;
	lims_NetworkConnectionStruct *pConnection;
	void *pSigMgrHandle;
	void *pSmsHandle;
	void *pMoipHandle;
	void *pCpmHandle;
	void *pUCEHandle;
	void *pSueHandle;
	EcrioSUESipParamStruct **ppFeatureTagStruct;
	u_int16 uFeatureTagCount;
	BoolEnum bSubscribed;
	u_int16 uSupportedMethodsCount;
	u_char **ppSupportedMethods;
	EcrioSipTimerStruct sipTimers;
	BoolEnum bMoIPFeature;
	BoolEnum bSmsFeature;
	BoolEnum bCpmFeature;
	u_int32 uFeatures;
	BoolEnum bEnablePAI;
} lims_moduleStruct;

/** Internal function prototype to initializes the Signaling Manager.*/
u_int32 lims_initSigMgrModule
(
	lims_moduleStruct *m
);

/** Internal function prototype to deinitialzes the Signaling Manager. */
u_int32 lims_deInitSigMgrModule
(
	lims_moduleStruct *m
);

/** Internal function prototype to initializes the SUE module. */
u_int32 lims_initSueModule
(
	lims_moduleStruct *m
);

/** Internal function prototype to deinitializes the SUE module.  */
u_int32 lims_deInitSueModule
(
	lims_moduleStruct *m
);

/** internal function prototype to initialize the Sms module. */
u_int32 lims_initSmsModule
(
	lims_moduleStruct *m
);

/** internal function prototype to deinitialize the Sms module. */
u_int32 lims_deInitSmsModule
(
	lims_moduleStruct *m
);

/** internal function prototype to initialize the Moip module. */
u_int32 lims_initMoipModule
(
	lims_moduleStruct *m
);

/** internal function prototype to deinitialize the Moip module. */
u_int32 lims_deInitMoipModule
(
	lims_moduleStruct *m
);

/** internal function prototype to initialize the Cpm module. */
u_int32 lims_initCpmModule
(
	lims_moduleStruct *m
);

/** internal function prototype to deinitialize the Cpm module. */
u_int32 lims_deInitCpmModule
(
	lims_moduleStruct *m
);


/** internal function prototype to initialize the UCE module. */
u_int32 lims_initUCEModule
(
lims_moduleStruct *m
);

/** internal function prototype to deinitialize the UCE module. */
u_int32 lims_deInitUCEModule
(
lims_moduleStruct *m
);



/** Internal function prototype to clean up the connection details from module structure and copy the new details */
u_int32 lims_cleanUpAndCopyConnectionDetails
(
	lims_moduleStruct *m,
	lims_NetworkConnectionStruct *pConnectionDetails
);

/** Internal function prototype to clean up the existing feature tags from the module structure. */
void lims_cleanUpFeatureTags
(
	lims_moduleStruct *m
);

/** Internal function prototype to clean up the existing feature tags from the module structure and copy the new feature tags details.*/
u_int32 lims_cleanUpAndCopyFeatureTags
(
	lims_moduleStruct *m,
	u_int32 uFeatures
);

/** Internal function prototype to get the allow methods from OOM module and update to IMS signaling module. */
u_int32 lims_getAndCopyAllowMethods
(
	lims_moduleStruct *m,
	u_int32 uMethods
);

/** Internal function prototype to clean up the lims module structure */
u_int32 lims_cleanUpModuleStructure
(
	lims_moduleStruct *m
);

/** Internal function prototype to update the network state change details to SUE */
void lims_getPropertyCallback
(
	EcrioSipPropertyNameEnums eName,
	EcrioSipPropertyTypeEnums eType,
	void *pData,
	void *pContext
);

/** Internal function prototype */
void lims_sueStatusCallbackHandler
(
	void *pCallbackData,
	void *pNotificationData
);

#ifdef ENABLE_RCS
/** */
void lims_cpmStatusCallbackHandler
(
	void *pCallbackData,
	EcrioCPMNotifStruct *pCPMNotifStruct
);

void lims_UCEGetPropertyCallback
(
EcrioUCEPropertyNameEnums eName,
void *pData,
void *pContext
);

#endif

#ifdef ENABLE_QCMAPI
u_int32 lims_InitInternal
(
	lims_moduleStruct *m,
	lims_configInternalStruct *pStruct
);

u_int32 lims_NetworkStateChangeInternal
(
	lims_moduleStruct *m,
	lims_NetworkPDNTypeEnum	ePdnType,
	lims_NetworkConnectionTypeEnum eConnectionType,
	lims_NetworkConnectionStruct *pConnectionDetails
);

u_int32 lims_updateSigMgrParameters
(
	lims_moduleStruct *m
);

#if defined(ENABLE_RCS)
u_int32 lims_updateCpmParamters
(
	lims_moduleStruct *m
);
#endif

u_int32 lims_updateParamters
(
	lims_moduleStruct *m,
	lims_configInternalStruct *pStruct
);
#endif
#endif	// __LIMSINTERNAL_H__
