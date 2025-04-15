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

#ifndef	_ECRIO_SUE_INTERNAL_DATA_TYPES_H_
#define	_ECRIO_SUE_INTERNAL_DATA_TYPES_H_

#include "EcrioSigMgrCommon.h"
#include "EcrioSigMgrRegister.h"
#include "EcrioSUEEngine.h"
#include "EcrioOOM.h"

/******************************************************************************

                Constant defination start

******************************************************************************/
#define	_ECRIO_SUE_SUBSCRIBE_EXPIRY											3600
#define _ECRIO_SUE_ATTRIBUTE_MAX_LEN										3600
#define ECRIO_SUE_ENG_NAME													(u_char *)"SUE Engine"
#define _ECRIO_SUE_ALLOW													(u_char *)"NOTIFY,MESSAGE"
#define _ECRIO_SUE_SUPPORTED												(u_char *)"path"
#define _ECRIO_SUE_SUBSCRIBE_EVENT											(u_char *)"reg"
#define _ECRIO_SUE_SUBSCRIBE_MESSAGE_SUMMERY_EVENT							(u_char *)"message-summary"

#define _ECRIO_SUE_FEATURE_TAG												(u_char *)"+g.3gpp.cs-voice"
#define _ECRIO_SUE_CONTACT_PARAMETER_EXPIRES								(u_char *)"expires=0"

#define _ECRIO_SUE_MAX_CHANNEL												10
#define _ECRIO_SUE_MAX_BEARAR_NW											10

#define _ECRIO_SUE_MAX_STRING_LENGTH										256
#define _ECRIO_SUE_MAX_UINT_LENGTH											8
/* Default SIP Port as defined in RFC 3261 */
#define _ECRIO_SUE_DEFAULT_SIP_PORT											5060

/* Constant Definition for Runtime ConfigHeaderListStruct */
/* ------------------------------------------------------ */
#define _ECRIO_SUE_UTRAN_CELL_ID_3GPP_VALUE									(u_char *)"EHGFT1I7895ASD"
#define _ECRIO_SUE_HEADER_NAME_P_ACCESS_NETWORK_INFO						(u_char *)"P-Access-Network-Info"
#define _ECRIO_SUE_HEADER_NAME_ACCEPT										(u_char *)"Accept"
#define _ECRIO_SUE_HEADER_NAME_ACCEPT_VALUE									(u_char *)"application/reginfo+xml"
#define _ECRIO_SUE_HEADER_NAME_P_ASSERTED_IDENTITY							(u_char *)"P-Asserted-Identity"
#define _ECRIO_SUE_HEADER_NAME_P_PREFERRED_IDENTITY							(u_char *)"P-Preferred-Identity"
#define _ECRIO_SUE_PARAM_NAME_UTRAN_CELL_ID_3GPP							(u_char *)"utran-cell-id-3gpp"
#define _ECRIO_SUE_PARAM_NAME_3GPP_UTRAN_TDD								(u_char *)"3GPP-UTRAN-TDD"
#define _ECRIO_SUE_HEADER_NAME_MAX_FORWARDS									(u_char *)"Max-Forwards"
#define _ECRIO_SUE_HEADER_NAME_MAX_FORWARDS_VALUE							(u_char *)"70"
#define _ECRIO_SUE_HEADER_NAME_CONTACT										(u_char *)"Contact"
#define _ECRIO_SUE_HEADER_NAME_CONTACT_VALUE								(u_char *)"sip:[5555::aaa:bbb:ccc:ddd]"
#define _ECRIO_SUE_PARAM_NAME_COMP											(u_char *)"comp"
#define _ECRIO_SUE_PARAM_NAME_COMP_VALUE									(u_char *)"sigcomp"
#define _ECRIO_SUE_PARAM_NAME_EXPIRES										(u_char *)"expires"
#define _ECRIO_SUE_PARAM_NAME_EXPIRES_VALUE									(u_char *)"600000"
#define _ECRIO_SUE_HEADER_NAME_SUPPORTED									(u_char *)"Supported"
#define _ECRIO_SUE_PARAM_NAME_LR											(u_char *)"lr"
#define _ECRIO_SUE_HEADER_NAME_SUPPORTED_VALUE_PATH							(u_char *)"path"
#define _ECRIO_SUE_HEADER_NAME_SUPPORTED_VALUE_SEC_AGREE					(u_char *)"sec-agree"
#define _ECRIO_SUE_HEADER_NAME_SUPPORTED_VALUE_GRUU							(u_char *)"gruu"
#define _ECRIO_SUE_HEADER_NAME_EVENT_VALUE									(u_char *)"reg"
#define _ECRIO_SUE_HEADER_NAME_PATH											(u_char *)"path"
#define _ECRIO_SUE_AUTH_SCHEME												(u_char *)"Digest"
#define _ECRIO_SUE_HEADER_NAME_ALLOW										(u_char *)"Allow"
#define _ECRIO_SUE_HEADER_NAME_ALLOW_VALUE									(u_char *)"NOTIFY,MESSAGE"
#define _ECRIO_SUE_HEADER_NAME_ALLOW_EVENTS									(u_char *)"Allow-Events"
#define _ECRIO_SUE_HEADER_NAME_ALLOW_EVENTS_VALUE							(u_char *)"reg"
#define _ECRIO_SUE_HEADER_NAME_PROXY_REQUIRE								(u_char *)"Proxy-Require"
#define _ECRIO_SUE_HEADER_NAME_REQUIRE										(u_char *)"Require"
#define _ECRIO_SUE_HEADER_NAME_EXPIRES										(u_char *)"Expires"
#define _ECRIO_SUE_HEADER_NAME_PRIVACY										(u_char *)"Privacy"
#define _ECRIO_SUE_HEADER_NAME_PRIVACY_VALUE								(u_char *)"none"
/* End - Added By A.D. to Fill Header Default Value */
/*Changes for MWI Task -Start*/
#define _ECRIO_SUE_HEADER_NAME_EVENT_VALUE_MWI								(u_char *)"message-summary"
#define _ECRIO_SUE_HEADER_NAME_ACCEPT_VALUE_MWI								(u_char *)"application/simple-message-summary"
/*Changes for MWI Task -End*/

/* Method Name string - Start */
#define _ECRIO_SUE_METHOD_INVITE											(u_char *)"INVITE"
#define _ECRIO_SUE_METHOD_ACK												(u_char *)"ACK"
#define _ECRIO_SUE_METHOD_BYE												(u_char *)"BYE"
#define _ECRIO_SUE_METHOD_CANCEL											(u_char *)"CANCEL"
#define _ECRIO_SUE_METHOD_PRACK												(u_char *)"PRACK"
#define _ECRIO_SUE_METHOD_UPDATE											(u_char *)"UPDATE"
#define _ECRIO_SUE_METHOD_REFER												(u_char *)"REFER"
#define _ECRIO_SUE_METHOD_NOTIFY											(u_char *)"NOTIFY"
#define _ECRIO_SUE_METHOD_OPTIONS											(u_char *)"OPTIONS"
#define _ECRIO_SUE_METHOD_MESSAGE											(u_char *)"MESSAGE"
#define _ECRIO_SUE_METHOD_INFO												(u_char *)"INFO"
/* Method Name string - End */

/* Constant for Reginfo Data Storing - Added by A.D */
#define	_ECRIO_SUE_NETWORK_INITIATED_REGISTRATION_STATE_FULL				(u_char *)"full"
#define	_ECRIO_SUE_NETWORK_INITIATED_REGISTRATION_STATE_PARTIAL				(u_char *)"partial"
#define _ECRIO_SUE_NETWORK_INITIATED_REGISTRATION_STATE_INIT				(u_char *)"init"
#define _ECRIO_SUE_NETWORK_INITIATED_REGISTRATION_STATE_ACTIVE				(u_char *)"active"
#define _ECRIO_SUE_NETWORK_INITIATED_REGISTRATION_STATE_TERMINATED			(u_char *)"terminated"
#define _ECRIO_SUE_NETWORK_INITIATED_REGISTRATION_STATE_REGISTERED			(u_char *)"registered"
#define _ECRIO_SUE_NETWORK_INITIATED_REGISTRATION_STATE_CREATED				(u_char *)"created"
#define _ECRIO_SUE_NETWORK_INITIATED_REGISTRATION_STATE_REFRESHED			(u_char *)"refreshed"
#define _ECRIO_SUE_NETWORK_INITIATED_REGISTRATION_STATE_SHORTEND			(u_char *)"shortened"
#define _ECRIO_SUE_NETWORK_INITIATED_REGISTRATION_STATE_EXPIRED				(u_char *)"expired"
#define _ECRIO_SUE_NETWORK_INITIATED_REGISTRATION_STATE_PROBATION			(u_char *)"probation"
#define _ECRIO_SUE_NETWORK_INITIATED_REGISTRATION_STATE_UNREGISTERED		(u_char *)"unregistered"
#define _ECRIO_SUE_NETWORK_INITIATED_REGISTRATION_STATE_REJECTED			(u_char *)"rejected"
#define _ECRIO_SUE_NETWORK_INITIATED_REGISTRATION_STATE_DEACTIVATED			(u_char *)"deactivated"

#define	_ECRIO_SIP_RESPONSE_CODE_200										200
#define	_ECRIO_SIP_RESPONSE_STRING_200										(u_char *)"OK"

#define	_ECRIO_SIP_RESPONSE_CODE_300										300
#define	_ECRIO_SIP_RESPONSE_CODE_401										401
#define	_ECRIO_SIP_RESPONSE_CODE_407										407
#define	_ECRIO_SIP_RESPONSE_CODE_408										408
#define	_ECRIO_SIP_RESPONSE_CODE_481										481
#define	_ECRIO_SIP_RESPONSE_CODE_482										482
#define	_ECRIO_SIP_RESPONSE_CODE_487										487

#define	_ECRIO_SIP_RESPONSE_CODE_305										305
#define	_ECRIO_SIP_RESPONSE_CODE_420										420
#define	_ECRIO_SIP_RESPONSE_CODE_423										423
#define	_ECRIO_SIP_RESPONSE_CODE_500										500
#define	_ECRIO_SIP_RESPONSE_CODE_504										504
/* Support LTE Requirement */
#define	_ECRIO_SIP_RESPONSE_CODE_404										404
#define	_ECRIO_SIP_RESPONSE_CODE_503										503
/* Support LTE Requirement */

#define	_ECRIO_SIP_RESPONSE_CODE_600										600
#define	_ECRIO_SIP_RESPONSE_CODE_403										403
/* added for handling 501 de-register response*/
#define	_ECRIO_SIP_RESPONSE_CODE_501										501
#define	_ECRIO_SIP_RESPONSE_CODE_400										400

#define	_ECRIO_SUE_INTERNAL_STATE_NONE										(u_char *)"NONE"
#define	_ECRIO_SUE_INTERNAL_STATE_TYPE_ENGINE								(u_char *)"ENGINE"
#define	_ECRIO_SUE_INTERNAL_STATE_TYPE_NETWORK								(u_char *)"NETWORK"
#define	_ECRIO_SUE_INTERNAL_STATE_TYPE_REGISTRATION							(u_char *)"REGISTRATION"

#define	_ECRIO_SUE_INTERNAL_ENGINE_STATE_INITIALIZED						(u_char *)"INITIALIZED"
#define	_ECRIO_SUE_INTERNAL_ENGINE_STATE_RUNNING							(u_char *)"RUNNING"
#define	_ECRIO_SUE_INTERNAL_ENGINE_STATE_DEINITIALIZING						(u_char *)"DE_INITIALIZING"
#define	_ECRIO_SUE_INTERNAL_ENGINE_STATE_DEINITILIZED						(u_char *)"DE_INITILIZED"

#define	_ECRIO_SUE_INTERNAL_NETWORK_STATE_CONNECTED							(u_char *)"CONNECTED"
#define	_ECRIO_SUE_INTERNAL_NETWORK_STATE_DISCONNECTED						(u_char *)"DIS_CONNECTED"

#define	_ECRIO_SUE_INTERNAL_REGISTRATION_STATE_REGISTERING					(u_char *)"REGISTERING"
#define	_ECRIO_SUE_INTERNAL_REGISTRATION_STATE_REGISTERED					(u_char *)"REGISTERED"
#define	_ECRIO_SUE_INTERNAL_REGISTRATION_STATE_REGISTRATION_FAILED			(u_char *)"REGISTRATION_FAILED"
#define	_ECRIO_SUE_INTERNAL_REGISTRATION_STATE_REREGISTERING				(u_char *)"REREGISTERING"
#define	_ECRIO_SUE_INTERNAL_REGISTRATION_STATE_DE_REGISTERING				(u_char *)"DE_REGISTERING"
#define	_ECRIO_SUE_INTERNAL_REGISTRATION_STATE_DE_REGISTERED				(u_char *)"DE_REGISTERED"
#define	_ECRIO_SUE_INTERNAL_REGISTRATION_STATE_DE_REGISTRATION_FAILED		(u_char *)"DE_REGISTRATION_FAILED"

#define	_ECRIO_SUE_ACTIVATE_NETWORK_TIMER_ID								65000
#define	_ECRIO_SUE_ACTIVATE_REG_RETRY_ON_AUTH_FAILURE_TIMER_ID				66000	/* Timer ID for REG-RETRY-INTERVAL-ON-AUTH-FAILURE */

#define _ECRIO_SUE_SIP_T1_TIMER_INTERVAL									(3000U)
#define _ECRIO_SUE_SIP_T2_TIMER_INTERVAL									(16000U)
#define _ECRIO_SUE_SIP_F_TIMER_INTERVAL										(30000U)
#define _ECRIO_SUE_SIP_REGBLOCK_TIMER_INTERVAL								(300000U)		/* 5 minutes */
#define _ECRIO_SUE_SIP_REREGBLOCK_TIMER_INTERVAL							(1800000U)		/* 30 minutes */
#define _ECRIO_SUE_SIP_REGBLOCK_TIMER_ID									(65001U)
#define _ECRIO_SUE_RETRY_REGISTER_ON_ALL_PCCF_FAILURE_TIMER_ID				(65002U)
#define _ECRIO_SUE_RETRY_REGISTER_ON_ALL_PCCF_FAILURE_TIMER_INTERVAL		(900000U)		/* 15 minutes */
#define _ECRIO_SUE_MAX_IMMEDIATE_REGISTER_ATTEMPT							(4U)
/* Start  - To Support LTE Requirement */
#define _ECRIO_SUE_LTE_NETWORK_REATTACH_TIMER_ID							(65009U)
#define _ECRIO_SUE_LTE_NETWORK_REATTACH_TIMER_INTERVAL						(720000U)		/* 12 minutes*/
#define _ECRIO_SUE_LTE_NETWORK_REG_REATTEMPT_TIMER_ID						(65010U)
#define _ECRIO_SUE_LTE_NETWORK_REG_REATTEMPT_TIMER_INTERVAL					(30000U)		/* 30 sec */
#define _ECRIO_SUE_MAX_REG_RETRY_ATTEMPT									(2U)
// #define _ECRIO_SUE_MAX_REG_404_FAILURE_COUNT								(3U)
/* Start - 503 response handling for SUBSCRIBE requests */
#define _ECRIO_SUE_SIP_SUBSCRIPTION_BLOCK_TIMER_INTERVAL				(300000U)			/* 5 minutes */
#define _ECRIO_SUE_SIP_SUBSCRIPTION_BLOCK_TIMER_ID						(65003U)
#define _ECRIO_SUE_MAX_IMMEDIATE_SUBSCRIPTION_ATTEMPT					(4U)
/* End - 503 response handling for SUBSCRIBE requests */

#define _ECRIO_SUE_LTE_REG_ATTEMPT_ON_NETWORK_DEREG_TIMER_ID			(65020U)
#define _ECRIO_SUE_LTE_REG_ATTEMPT_ON_NETWORK_DEREG_TIMER_INTERVAL		(60000U)			/* 60 sec */

/* MrWho - Check for reason and disable resubscription  - Start */
#define _ECRIO_SUE_SIP_SUBSCRIPTION_RETRY_AFTER_TIMER_ID				(65004U)
/* MrWho - Check for reason and disable resubscription  - End */

/* Default value for Reg and Sub Expires(in millisecond) - Start  */
#define _ECRIO_SUE_DEFAULT_REG_EXPIRES									(7200000U)
#define _ECRIO_SUE_DEFAULT_SUB_EXPIRES									(7200000U)
/*Changes for MWI Handling Task -Start*/
#define _ECRIO_SUE_DEFAULT_SUB_TO_MWI_EXPIRES							(7200000U)	/*120 Minutes*/
/*Changes for MWI Handling Task -End*/
/* Default value for Reg and Sub Expires(in millisecond) - End  */
/* Support for NAT - Start */
#define ECRIO_SUE_ENGINE_NO_ACTIVITY_TIMER_INTERVAL						(25000U)				/* 25 sec */
#define ECRIO_SUE_ENGINE_NOOP_PREFIX									(const u_char *)"UDP-PING "	/* SBC Support */
/* Support for NAT - End */

/* IARI support - start*/
#define ECRIO_SUE_ENGINE_IARI_PREFIX									(u_char *)"featuretag"
/* IARI support - end*/

#define _ECRIO_SUE_EMERGENCY_REG_TIMER_INTERVAL							(720000U)	/* 12 minutes*/

#define	_ECRIO_SUE_REGISTRATION_ELEMENT									(u_char *)"registration"
#define	_ECRIO_SUE_CONTACT_ELEMENT										(u_char *)"contact"
#define	_ECRIO_SUE_UNKNOWN_ELEMENT										(u_char *)"unknown-param"
#define	_ECRIO_SUE_URI_ELEMENT											(u_char *)"uri"
#define	_ECRIO_SUE_AOR_ATTRIBUTE										(u_char *)"aor"
#define	_ECRIO_SUE_STATE_ATTRIBUTE										(u_char *)"state"
#define	_ECRIO_SUE_EVENT_ATTRIBUTE										(u_char *)"event"
#define	_ECRIO_SUE_EXPIRES_ATTRIBUTE									(u_char *)"expires"
#define	_ECRIO_SUE_RETRY_AFTER_ATTRIBUTE								(u_char *)"retry-after"
#define	_ECRIO_SUE_URI_ATTRIBUTE										(u_char *)"uri"
#define	_ECRIO_SUE_ATTRIBUTE_VALUE_ACTIVE								(u_char *)"active"
#define	_ECRIO_SUE_ATTRIBUTE_VALUE_TERMINATED							(u_char *)"terminated"
#define	_ECRIO_SUE_ATTRIBUTE_VALUE_REGISTERED							(u_char *)"registered"
#define	_ECRIO_SUE_ATTRIBUTE_VALUE_CREATED								(u_char *)"created"
#define	_ECRIO_SUE_ATTRIBUTE_VALUE_REFRESHED							(u_char *)"refreshed"
#define	_ECRIO_SUE_ATTRIBUTE_VALUE_SHORTENED							(u_char *)"shortened"
#define	_ECRIO_SUE_ATTRIBUTE_VALUE_EXPIRED								(u_char *)"expired"
#define	_ECRIO_SUE_ATTRIBUTE_VALUE_DEACTIVATED							(u_char *)"deactivated"
#define	_ECRIO_SUE_ATTRIBUTE_VALUE_PROBATION							(u_char *)"probation"
#define	_ECRIO_SUE_ATTRIBUTE_VALUE_UNREGISTERED							(u_char *)"unregistered"
#define	_ECRIO_SUE_ATTRIBUTE_VALUE_REJECTED								(u_char *)"rejected"

/******************************************************************************

                Constant defination end

******************************************************************************/

/******************************************************************************

                Enumeration constant defination start

******************************************************************************/

typedef	enum
{
	_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_None = 0,
	_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Engine,
	_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Network,
	_ECRIO_SUE_INTERNAL_STATE_TYPE_ENUM_Registration,
}
_EcrioSUEInternalStateTypeEnum;

typedef enum
{
	_ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_None = 1,
	_ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Initialized,
	_ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Running,
	_ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Deinitializing,
	_ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Deinitialized,
}
_EcrioSUEInternalEngineStateEnum;

typedef enum
{
	_ECRIO_SUE_INTERNAL_NETWORK_STATE_ENUM_None = 0,
	_ECRIO_SUE_INTERNAL_NETWORK_STATE_ENUM_Connected,
	_ECRIO_SUE_INTERNAL_NETWORK_STATE_ENUM_Disconnected,
	_ECRIO_SUE_INTERNAL_NETWORK_STATE_ENUM_LLF
}
_EcrioSUEInternalNetworkStateEnum;

typedef enum
{
	_ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_None = 0,
	_ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_Registering,
	_ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_Registered,
	_ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_RegistrationFailed,
	_ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_ReRegistering,
	_ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_DeRegistering,
	_ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_DeRegistered,
	_ECRIO_SUE_INTERNAL_REGISTRATION_STATE_ENUM_DeRegistrationFailed
}
_EcrioSUEInternalRegistrationStateEnum;

typedef enum
{
	_ECRIO_SUE_STRUCT_ENUM_None						= 0,
	_ECRIO_SUE_STRUCT_ENUM_EcrioSUEGlobalDataStruct = 100
}
_EcrioSUEStructEnum;

typedef enum
{
	_ECRIO_SUE_INTERNAL_REATTEMPT_None,				// exit SUE
	_ECRIO_SUE_INTERNAL_REATTEMPT_SAME_PCSCF,		// retry initial request to same PCSCF
	_ECRIO_SUE_INTERNAL_REATTEMPT_NEXT_PCSCF,		// retry initial request to next PCSCF
	_ECRIO_SUE_INTERNAL_REATTEMPT_REREQUEST			// retry rerequest i.e. reregister or resubscribe to same PCSCF
}
_EcrioSUEReAttemptEnum;

/******************************************************************************

                Enumeration constant defination end

******************************************************************************/

/******************************************************************************

                Structure declarations start

******************************************************************************/

typedef	struct
{
	u_char curPCSCFIndex;
	u_char **ppPCSCFList;
	u_char uNoPCSCF;
	u_char *pLocalIP;
	u_int32	uLocalPort;
	u_int32	uPCSCFPort;
	u_int32	uPCSCFTLSPort;
	u_int32	uPCSCFClientPort;
	BoolEnum bIsIPv6;
	BoolEnum bIsProxyRouteEnabled;	/**< Conditional variable to use proxy routing or not. */
	u_char* pProxyRouteAddress; /**< Null terminated string holding Proxy server address. */
	u_int32 uProxyRoutePort;	/**< Proxy server port. */
} _EcrioSUETransportAddressStruct;

typedef struct
{
	_EcrioSUEInternalNetworkStateEnum eNetworkState;
	_EcrioSUEInternalRegistrationStateEnum eRegistrationState;
	_EcrioSUETransportAddressStruct *pTransportAddressStruct;
	void *pRegisterIdentifier;																/* UA returned registraton identifier to get registration states later on*/
	_EcrioSUEInternalEngineStateEnum eEngineState;
	PALINSTANCE pal;
	void *pLogHandle;
	void *pSigMgrHandle;
	BoolEnum bExitSUE;
	EcrioSUEExitReasonEnum eExitReason;
	BoolEnum bNotifyApp;												/* used to specify weather deployed application should get a notification */
	u_int8 uRegGeneralFailureCount;
	u_int8 uSubGeneralFailureCount;								/* Count for number of Subscription failure*/
	BoolEnum bSubscribeRegEvent;
	BoolEnum bUnSubscribeRegEvent;
	TIMERHANDLE	uRegReAttemptTimerId;
	BoolEnum bRegReAttemptTimerStarted;
	TIMERHANDLE	uSubReAttemptTimerId;
	BoolEnum bSubReAttemptTimerStarted;
	u_char *pSubscribeSessionId;
	EcrioSUECallbackStruct callbackStruct;
	EcrioOOMStruct *pOOMObject;
	_EcrioSUEReAttemptEnum eReAttempt;
} _EcrioSUEGlobalDataStruct;

/* Start Utility Data Structure */
/* Start - For Runtime Cinfig Struct (generateRuntimeConfiguarableHeader) */

/******************************************************************************

                Structure declarations end

******************************************************************************/

#endif /*  _ECRIO_SUE_INTERNAL_DATA_TYPES_H_    */
