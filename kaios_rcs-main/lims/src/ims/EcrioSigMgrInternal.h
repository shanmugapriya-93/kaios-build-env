/******************************************************************************

Copyright (c) 2016 Ecrio, Inc. All Rights Reserved.

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
POSSIBILITY OF SUCH DAMAGES. IF THIS SOFTWARE IS PROVIDED ON A COMPACT DISK,
THE OTHER SOFTWARE AND DOCUMENTATION ON THE COMPACT DISK ARE SUBJECT TO THE
LICENSE AGREEMENT ACCOMPANYING THE COMPACT DISK.

******************************************************************************/

#ifndef _ECRIO_SIG_MGR_INTERNAL_H_
#define  _ECRIO_SIG_MGR_INTERNAL_H_

/**
*	Headers
*/
#include "EcrioTxnMgrSAC.h"
#include "EcrioOOM.h"

/**
* @def
*Constant Definition - START
*/

#ifdef ENABLE_LOG
#define SIGMGRLOGV(a, b, c, ...)		pal_LogMessage((a), KLogLevelVerbose, KLogComponentSignaling | KLogComponent_Engine, (b), (c),##__VA_ARGS__);
#define SIGMGRLOGI(a, b, c, ...)		pal_LogMessage((a), KLogLevelInfo, KLogComponentSignaling | KLogComponent_Engine, (b), (c),##__VA_ARGS__);
#define SIGMGRLOGD(a, b, c, ...)		pal_LogMessage((a), KLogLevelDebug, KLogComponentSignaling | KLogComponent_Engine, (b), (c),##__VA_ARGS__);
#define SIGMGRLOGW(a, b, c, ...)		pal_LogMessage((a), KLogLevelWarning, KLogComponentSignaling | KLogComponent_Engine, (b), (c),##__VA_ARGS__);
#define SIGMGRLOGE(a, b, c, ...)		pal_LogMessage((a), KLogLevelError, KLogComponentSignaling | KLogComponent_Engine, (b), (c),##__VA_ARGS__);

#define SIGMGRLOGDUMP(a, b, c, d)		pal_LogMessageDump((a), KLogLevelDebug, KLogComponentSignaling | KLogComponent_Engine, (b), (c), (d));
#else
#define SIGMGRLOGV(a, b, c, ...)		if (a != NULL) {;}
#define SIGMGRLOGI(a, b, c, ...)		if (a != NULL) {;}
#define SIGMGRLOGD(a, b, c, ...)		if (a != NULL) {;}
#define SIGMGRLOGW(a, b, c, ...)		if (a != NULL) {;}
#define SIGMGRLOGE(a, b, c, ...)		if (a != NULL) {;}

#define SIGMGRLOGDUMP(a, b, c, d)
#endif

#define	ECRIO_SIG_MGR_LAST_N_SIP_CHAR_COMPARE								10
#define	ECRIO_SIG_MGR_SIP_URI_PARAM_COMPARE_ENABLE							0

/* MD5 Hash Length */
#define HASHHEXLEN															32U

#define ECRIO_SIG_MGR_MAX_CONTACT_URIS										10U
#define ECRIO_SIG_MGR_RANGE_MAX_FORWARDS									255U
#define ECRIO_SIG_MGR_NAME													"Ecrio Signaling Manager"
#define ECRIO_SIG_MGR_MIN_TIMERX											1U
#define ECRIO_SIG_MGR_MAX_TIMERX											32U
#define ECRIO_SIG_MGR_INVALID_PORT											0U
#define ECRIO_SIG_MGR_DEFAULT_PORT											19000U
#define ECRIO_SIG_MGR_MAX_FORWARDS											70U
#define ECRIO_SIG_MGR_REG_TIMER_DELTA										1U		/* 1 Sec delta */
#define ECRIO_SIG_MGR_REG_EXPIRES											3600U
#define ECRIO_SIG_MGR_DEFAULT_SESSION_EXPIRE_VLUE							90U		// 300U - Minimum should be 90 as per VoLTE test plan
#define ECRIO_SIG_MGR_DEFAULT_MIN_SE_VLUE									90U
#define ECRIO_SIG_MGR_REG_PRIORITY											0U
#define ECRIO_SIG_MGR_SIP_OPEN_BRACKET_SYMBOL								"["
#define ECRIO_SIG_MGR_SIP_CLOSE_BRACKET_SYMBOL								"]"
#define ECRIO_SIG_MGR_SIP_COLON_SYMBOL										":"
#define ECRIO_SIG_MGR_SIP_COMMA_SYMBOL										","
#define ECRIO_SIG_MGR_SIP_CRLF_SYMBOL										"\r\n"
#define ECRIO_SIG_MGR_SIP_LWS_SYMBOL										" "
#define ECRIO_SIG_MGR_SIP_LR_SYMBOL											"lr"
#define ECRIO_SIG_MGR_SIP_AT_SYMBOL											"@"
#define ECRIO_SIG_MGR_SIP_SEMICOLON_SYMBOL									";"
#define ECRIO_SIG_MGR_SIP_TRANSPORT_PARAM									"transport"
#define ECRIO_SIG_MGR_SIP_UDP_PARAM											"UDP"
#define ECRIO_SIG_MGR_SIP_TCP_PARAM											"TCP"
#define ECRIO_SIG_MGR_SIP_TLS_PARAM											"TLS"
#define ECRIO_SIG_MGR_SIP_TLS_TRANSPORT_PARAM								"tls"
#define ECRIO_SIG_MGR_SIP_UDP_TRANSPORT_PARAM								"udp"
#define ECRIO_SIG_MGR_SIP_Q_PARAM											"q"
#define ECRIO_SIG_MGR_SIP_EQUALS_STRING										"="
#define ECRIO_SIG_MGR_SIP_ZERO_STRING										"0"
#define ECRIO_SIG_MGR_SIP_TAG_STRING										(u_char *)"tag"
#define ECRIO_SIG_MGR_AUTH_PARAMS											4U
#define ECRIO_SIG_MGR_CRED_VALUES											1U
#define ECRIO_SIG_MGR_INVALID_RESONSE_CODE									0U
// #define ECRIO_SIG_MGR_INVALID_TIMER_ID					0U
#define ECRIO_SIG_MGR_INITIAL_CSEQ											0U
#define ECRIO_SIG_MGR_SIP_URI_NAME											"sip"
#define ECRIO_SIG_MGR_SIP_VERSION											"SIP/2.0"
#define ECRIO_SIG_MGR_BRANCH_INITIAT_STR									"z9hG4bK"
#define ECRIO_SIG_MGR_DATE_HEADER											"Date"
#define ECRIO_SIG_MGR_SESSION_ID_HEADER										"Session-ID"
#define ECRIO_SIG_MGR_CALL_INFO_HEADER										"Call-Info"
#define ECRIO_SIG_MGR_ORGANIZATION_HEADER									"Organization"
#define ECRIO_SIG_MGR_SERVICE_ROUTE_HEADER									"Service-Route"
#define ECRIO_SIG_MGR_BRANCH_PARAM											"branch"
#define ECRIO_SIG_MGR_OPAQUE_PARAM											"opaque"
#define ECRIO_SIG_MGR_DOMAIN_PARAM											"domain"
#define ECRIO_SIG_MGR_STALE_PARAM											"stale"
#define ECRIO_SIG_MGR_1XX_UNREL_PERIODIC_INTERVAL							60000U
#define ECRIO_SIG_MGR_1XX_REL_PERIODIC_INTERVAL								180000U
#define ECRIO_SIG_MGR_MAX_NC_LENGTH											8U
#define ECRIO_SIG_MGR_AUTHORIZATION_CNONCE_VALUE							((u_char *)"12345")
#define ECRIO_SIG_MGR_AKA_RAND_LENGTH										16U			/* Rand Length = 128bits or 16 Bytes */
#define ECRIO_SIG_MGR_AKA_AUTN_LENGTH										16U			/* Autn Length = 128bits or 16 Bytes */
#define ECRIO_SIG_MGR_AKA_AUTS_LENGTH										14U			/* Auts Length = 112bits or 14 Bytes */
#define ECRIO_SIG_MGR_RSP_CODE_UNAUTHORIZED									(401U)
#define ECRIO_SIG_MGR_RSP_CODE_PROXY_AUTHENTICATION_REQUIRED				(407U)
#define ECRIO_SIG_MGR_MANDATORY_AUTHENTICATION_CREDENTIAL_COUNT				(3U)
#define ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_SCHEME						((u_char *)"Digest")
#define ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_DOMAIN						((u_char *)"domain")
#define ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_REALM							((u_char *)"realm")
#define ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_NONCE							((u_char *)"nonce")
#define	ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_QOP							((u_char *)"qop")
#define ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_ALGORITHM_NAME				((u_char *)"algorithm")
#define ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_ALGORITHM_VALUE_MD5			((u_char *)"MD5")
#define ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_ALGORITHM_VALUE_AKAv1_MD5		((u_char *)"AKAv1-MD5")
#define ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_ALGORITHM_VALUE_AKAv2_MD5		((u_char *)"AKAv2-MD5")

#define ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_USERNAME						((u_char *)"username")
#define ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_RESPONSE						((u_char *)"response")
#define ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_URI_NAME						((u_char *)"uri")
#define ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_CNONCE_NAME					((u_char *)"cnonce")
#define ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_NONCE_COUNT					((u_char *)"nc")
#define ECRIO_SIG_MGR_SIGNALING_AUTHORIZATION_AUTS							((u_char *)"auts")
#define ECRIO_SIG_MGR_SIGNALING_DOUBLE_QUOTE								"\""
#define ECRIO_SIG_MGR_MAXIMUM_WWWAUTHENTICATE_NONCECOUNT_LENGTH				8U
#define ECRIO_SIG_MGR_ZERO_STRING											"0"
#define ECRIO_SIG_MGR_SIGNALING_ANONYMOUS									"anonymous"
#define ECRIO_SIG_MGR_SIGNALING_ANONYMOUS_HOST								"anonymous.invalid"
#define ECRIO_SIG_MGR_SIGNALING_TRANSPORT_TCP								";transport=tcp"
#define ECRIO_SIG_MGR_SIGNALING_SUPPORTED_LANG_EN							"en"

#define ECRIO_SIG_MGR_SIP_TARGET_STRING										"target"
#define ECRIO_SIG_MGR_SIP_CAUSE_STRING										"cause"

#define ECRIO_SIG_MGR_SIP_QUESTION_SYMBOL									"?"
#define ECRIO_SIG_MGR_SIP_AMPERSAND_SYMBOL									"&"
#define ECRIO_SIG_MGR_HEADER_VALUE_SEC_AGREE								"sec-agree"
#define ECRIO_SIG_MGR_HEADER_VALUE_ACCEPT									"application/pidf+xml"
#define ECRIO_SIG_MGR_HEADER_NAME_SECURITY_SERVER							"Security-Server"
#define ECRIO_SIG_MGR_HEADER_NAME_SECURITY_CLIENT							"Security-Client"
#define ECRIO_SIG_MGR_HEADER_NAME_SECURITY_VERIFY							"Security-Verify"
#define ECRIO_SIG_MGR_HEADER_NAME_SECURITY_CLIENT_VALUE						"ipsec-3gpp"
#define ECRIO_SIG_MGR_PARAM_NAME_PORTC										"port-c"
#define ECRIO_SIG_MGR_PARAM_NAME_PORTS										"port-s"
#define ECRIO_SIG_MGR_PARAM_NAME_SPIC										"spi-c"
#define ECRIO_SIG_MGR_PARAM_NAME_SPIS										"spi-s"
#define ECRIO_SIG_MGR_PARAM_NAME_ALG										"alg"
#define ECRIO_SIG_MGR_PARAM_NAME_ENCRYPT_ALG								"ealg"

/* Method Name string*/
#define ECRIO_SIG_MGR_METHOD_REGISTER										"REGISTER"
#define ECRIO_SIG_MGR_METHOD_INVITE											"INVITE"
#define ECRIO_SIG_MGR_METHOD_ACK											"ACK"
#define ECRIO_SIG_MGR_METHOD_BYE											"BYE"
#define ECRIO_SIG_MGR_METHOD_CANCEL											"CANCEL"
#define ECRIO_SIG_MGR_METHOD_NOT_SUPPORTED									"NOTSUPPORTED"
#define ECRIO_SIG_MGR_METHOD_MESSAGE										"MESSAGE"
#define ECRIO_SIG_MGR_METHOD_PUBLISH										"PUBLISH"
#define ECRIO_SIG_MGR_METHOD_PRACK											"PRACK"
#define ECRIO_SIG_MGR_METHOD_SUBSCRIBE										"SUBSCRIBE"
#define ECRIO_SIG_MGR_METHOD_NOTIFY											"NOTIFY"
#define ECRIO_SIG_MGR_METHOD_UPDATE											"UPDATE"
#define ECRIO_SIG_MGR_METHOD_OPTIONS										"OPTIONS"
#define ECRIO_SIG_MGR_METHOD_REFER  										"REFER"
#define ECRIO_SIG_MGR_METHOD_INFO   										"INFO"

#define ECRIO_SIGMGR_1XX_RESPONSE											1U
#define ECRIO_SIGMGR_2XX_RESPONSE											2U
#define ECRIO_SIGMGR_3XX_RESPONSE											3U
#define ECRIO_SIGMGR_4XX_RESPONSE											4U
#define ECRIO_SIGMGR_5XX_RESPONSE											5U
#define ECRIO_SIGMGR_6XX_RESPONSE											6U

#define	ECRIO_SIGMGR_SIP_INSTANCE_PARAM_NAME								(u_char *)"+sip.instance"

/** Maximum length to send the data through UDP, otherwise TCP*/
#define ECRIO_SIG_MGR_MAX_SIP_LEN_OVER_UDP									1300U

#define ECRIO_SIGMGR_LIMS_3GPP_HEADER_NAME_P_ACCESS_NETWORK_INFO			(const u_char *)"P-Access-Network-Info"
#define ECRIO_SIGMGR_LIMS_3GPP_P_ACCESS_NETWORK_INFO_VALUE_STRING			(const u_char *)"3GPP2-1x-LTE"
#define ECRIO_SIGMGR_LIMS_3GPP_ACCESS_TYPE_STRING							(const u_char *)"utran-cell-id-3gpp"
#define ECRIO_SIGMGR_LIMS_3GPP_ACCESS_TYPE_VALUE_STRING						(const u_char *)"23415D0FCE11"

#define ECRIO_SIGMGR_LIMS_3GPP_P_PREFERRED_SERVICE_VALUE_STRING				(const u_char *)"urn:urn-7:3gpp-service.ims.icsi.oma.cpm.msg.group"
#define ECRIO_SIGMGR_LIMS_3GPP_ACCEPT_COONTACT_FEATURE_TAG_VALUE_STRING		(const u_char *)"urn:urn-7:3gpp-service.ims.icsi.oma.cpm.msg"

#define ECRIO_SIGMGR_LIMS_3GPP_P_EARLY_MEDIA_VALUE_STRING					(const u_char *)"supported"

#define ECRIO_SIGMGR_HASHMAP_INITIAL_SIZE									(1024)
#define ECRIO_SIGMGR_HASHMAP_GROWTH_FACTOR									(2)
#define ECRIO_SIGMGR_HASHMAP_MAX_LOAD_FACTOR								(1)

typedef enum
{
	EcrioSigMgrPresentParam_transport1	= 0x00000001,
	EcrioSigMgrPresentParam_transport2	= 0x00000002,
	EcrioSigMgrPresentParam_user1		= 0x00000010,
	EcrioSigMgrPresentParam_user2		= 0x00000020,
	EcrioSigMgrPresentParam_ttl1		= 0x00000100,
	EcrioSigMgrPresentParam_ttl2		= 0x00000200,
	EcrioSigMgrPresentParam_method1		= 0x00001000,
	EcrioSigMgrPresentParam_method2		= 0x00002000,
	EcrioSigMgrPresentParam_maddr1		= 0x00010000,
	EcrioSigMgrPresentParam_maddr2		= 0x00020000
} EcrioSigMgrPresentParamEnum;

/** Enumerated constants used to initialize and de initialize the engine structures */
/*TODO : To resolve enum types in struct init and deinit*/
typedef enum
{
	/** For representing structure - EcrioSigMgrStruct */
	EcrioSigMgrStructType_SigMgr = 0,
	EcrioSigMgrStructType_RegisterInfo,
	EcrioSigMgrStructType_RegContactInfo,
	EcrioSigMgrStructType_UserRegisterInfo,
	EcrioSigMgrStructType_SessionHandle,
	EcrioSigMgrStructType_InviteUsageInfo,
	EcrioSigMgrStructType_SubscribeUsageInfo,
	EcrioSigMgrStructType_DlgAuth,
	EcrioSigMgrStructType_CommonInfo,
	EcrioSigMgrStructType_TXNInfo,
	EcrioSigMgrStructType_DialogInfo
} EcrioSigMgrInternalStructTypeEnum;

/** request response types*/
typedef enum
{
	EcrioSigMgrMessageInviteRequest = 0,
	EcrioSigMgrMessageInviteResponse,
	EcrioSigMgrMessageNonInviteRequest,
	EcrioSigMgrMessageNonInviteResponse
} EcrioSigMgrMessageTypeEnum;

typedef enum
{
	EcrioSigMgrRoleUAC = 0,
	EcrioSigMgrRoleUAS
} EcrioSigMgrRoleEnum;

/** \enum EcrioSipIPsecStateEnum
* @brief This enum set indicates the current state of a particular IPsec..
*/
typedef enum
{
	ECRIO_SIP_IPSEC_STATE_ENUM_None = 0,			/**< Default value. */
	ECRIO_SIP_IPSEC_STATE_ENUM_Temporary,			/**< An IPsec will be in this state after creation from 401 REGISTER and before receiving 200 REGISTER. */
	ECRIO_SIP_IPSEC_STATE_ENUM_Established,			/**< An IPsec will be in this state after receiving 200 REGISTER and before any receiving any other request from server over this IPsec. */
	ECRIO_SIPC_IPSEC_STATE_ENUM_Active,				/**< An IPsec will be in this state after receiving any request from server over this IPsec. */
	ECRIO_SIP_IPSEC_STATE_ENUM_Expiring				/**< An IPsec will be in this state for 64*T1 after a new IPsec is established with the server. */
}EcrioSipIPsecStateEnum;

/**
*Constant Definition - END
*/

/**
*Structure Definition - START
*/

/** Structure to be used for Sig Session Handle 
typedef struct
{
	void *pDialogHandle;
} EcrioSigMgrSessionHandleStruct;*/

typedef struct
{
	u_char *pMethodName;
	u_char *pCallId;
	u_char *pToTag;
	u_char *pFromTag;
	u_char *pBranch;
	u_int32	cSeq;
	u_int32 responseCode;
	EcrioSigMgrRoleEnum role;
} EcrioSigMgrCommonInfoStruct;

typedef struct
{
	EcrioSigMgrSipMessageStruct *pSipMessage;		//input
	u_char reasonPhrase[ECRIO_SIGMGR_BUFFER_SIZE_64];						//output
	u_int32 responseCode;							//output
	EcrioSigMgrCommonInfoStruct cmnInfo;			//output
} EcrioSigMgrValidationStruct;

typedef struct
{
	u_char *pSessionMappingStr;
	u_char *pMsg;
	u_int32 msglen;
	EcrioSigMgrMessageTypeEnum messageType;
	s_int16	currentContext;
	s_int16	retransmitContext;
	EcrioSigMgrTransportEnum transport;
	BoolEnum deleteflag;
	u_char *pRemoteHost;
	u_int16 remotePort;
	void *pSMSRetryData;
	u_int16 retry_count;
	/*	The uReceivedChannelIndex and uRemoteSendingPort members will be passed to
	EcrioTXNSendMessageDirectToTransport() for retransmitted 200 OK INVITE, otherwise these value
	(stored in TxnMgr )for incoming INVITE ( on TCP) can be overwritten by PRACK ( on UDP).
	Though it is required for 200 OK INVITE yet these members must be populated for the situation when
	retransmitContext == ECRIO_SIGMGR_INVALID_CONTEXT. */
	u_int32	uReceivedChannelIndex;
	u_int32	uRemoteSendingPort;
	BoolEnum bSendDirect;	/*If Message sent direct to transport
							(through EcrioTXNSendMessageDirectToTransport)
							then value will be Enum_TRUE for this member.*/
} EcrioSigMgrTXNInfoStruct;

/*	Support for Authorization header in all request	start	*/
typedef	struct
{
	EcrioSigMgrAuthorizationStruct *pAuth;
	u_int32	authCount;
	BoolEnum bUpdatedFromChallengedResp;
} EcrioSigMgrSharedCredentialsStruct;
/*	Support for Authorization header in all request	end	*/

typedef struct
{
	EcrioSigMgrUriStruct *pFromSipURI;
	/** display name of the local party*/
	u_char *pDisplayName;
	/** user id - to be used  while constructing Address of record for the local party.*/
	u_char *pUserId;
	/** Password - to be used for Authentication.*/
	u_char *pPassword;
	/** Private user id - to be used  for Authentication.*/
	u_char *pPrivateId;
	u_char *pHomeDomain;
	/** Value to be used for Max Forwards. If 0 then default Max forward i.e. 70 value shall be used.*/
	u_int16 maxForwards;

	EcrioSigMgrHostTypeEnum eHostType;
	u_char *pUserAgent;
	u_int32 registrationExpires;
	u_int32 publishExpires;
	u_int32 subscriptionExpires;
	u_int32 registerationTimerDelta;
	EcrioSigMgrURISchemesEnum uriScheme;
	// u_char *pFeatureTag;
	u_int16 numFeatureTagParams;
	EcrioSigMgrParamStruct **ppFeatureTagParams;
	u_int32 sessionExpires;
	u_int32 minSe;
	EcrioSigMgrCheckMessageStandardEnum eStandardType;
	u_char *pRegistrarDomain;
	BoolEnum bEnabledRefreshSubscription;	/* This is used to decide wheather Refrsh subscription request will send or not */
	/* Flag to indicate IPsec Protection is supported or not */
	u_int32	uUdpMtu;		/* Maximum message length over UDP */
	BoolEnum bEnableNoActivityTimer; /* will send dummy UDP packet to P-CSCF */
	u_int32	uNoActivityTimerInterval;
	// u_char						*pContactURI;
	BoolEnum bWildCharInContact;
	BoolEnum bEnablePAUChecking;					/**< Validate the presence of P-Associated Uri in 200 OK/REGISTER */
	/** This Variable will maintain count of Associated URIs */
	u_int16 numPAssociatedURIs;
	/**	P-Associated-URI header field transports the set of Associated
	URIs to the registered AOR. */
	EcrioSigMgrNameAddrWithParamsStruct **ppPAssociatedURI;
	/** Route header */
	EcrioSigMgrRouteStruct *pRouteSet;
	EcrioSipAuthAlgorithmEnum eAuthAlgorithm;
	u_int16 numSupportedMethods;								/*Number method Supprted*/
	u_char **ppSupportedMethod;									/*Methods supported*/
	void *pPublishInfo;
	EcrioSigMgrHeaderStruct *pPAccessNWInfo;
	BoolEnum bEnableUdp;
	BoolEnum bEnableTcp;
	u_char *pPANI;
	u_char *pSecurityVerify;
	u_char *pAssociatedUri;
	u_char *pUriUserPart;
	BoolEnum bEnableTls;				/**< TLS enabled */
	BoolEnum bTLSPeerVerification;				/**< TLS Peer verification */
	char *pTLSCertificate;					/**< IP address of the relay server */
} _EcrioSigMgrInternalSignalingInfoStruct;

/*Hash table for routing the incoming/outgoing messages to registered modules*/

struct elt
{
	struct elt *pNext;
	u_char* pKey;
	void* pValue;
};

#if 0
typedef struct map
{
	u_int32 size;           /* size of the pointer table */
	u_int32 n;              /* number of elements stored */
	struct elt **ppTable;
}EcrioSigMgrHashMap;
#endif

typedef struct
{
	EcrioSipIPsecStateEnum eIPSecState;
	EcrioSipIPsecParamStruct *pIPsecParams;
	EcrioSipIPsecNegotiatedParamStruct *pIPsecNegParams;
	u_int32 ipsecRegId;
} EcrioSigMgrIPSecStruct;

typedef struct
{
	PALINSTANCE pal;
	/** Log engine handle*/
	LOGHANDLE pLogHandle;
	EcrioSigMgrEngineCallbackStruct	limsCallbackStruct;
	EcrioSigMgrEngineCallbackStruct	sueCallbackStruct;
	EcrioSigMgrEngineCallbackStruct	smsCallbackStruct;
	EcrioSigMgrEngineCallbackStruct	moipCallbackStruct;
	EcrioSigMgrEngineCallbackStruct	cpmCallbackStruct;
	EcrioSigMgrEngineCallbackStruct	uceCallbackStruct;
	/** timerX value*/
	u_int32 timerX;
	EcrioSigMgrCustomTimersStruct customSIPTimers;
	/** Transaction Mgr handle*/
	void *pTransactionMgrHndl;
	/** Signaling Information*/
	_EcrioSigMgrInternalSignalingInfoStruct *pSignalingInfo;
	EcrioSigMgrNetworkStateEnums eNetworkState;
	EcrioSigMgrNetworkInfoStruct *pNetworkInfo;
	/** Registration info */
	void *pRegisterInfo;
	EcrioTxnMgrTransportStruct *pSigMgrTransportStruct;
	/** Service-Route from Registration Response */
	EcrioSigMgrRouteStruct *pServiceRoute;
	/*	Support for Authorization header in all request	start	*/
	EcrioSigMgrSharedCredentialsStruct *pSharedCredentials;
	BoolEnum bIsDeInitializing;
	// Support for TO Uri comparison
	//void *pDialogList;
	EcrioOOMStruct *pOOMObject;

	/*Routing Support*/
	void* hHashMap;

	TIMERHANDLE udpKeepAliveTimerId;
	u_char* pUdpKeepAlivePkt;
	BoolEnum bUdpKeepAliveTimerStarted;

	BoolEnum bIPSecEnabled;
	EcrioSigMgrIPSecStruct *pIPSecDetails;
} EcrioSigMgrStruct;

typedef struct
{
	void *pSigMgrHandle;
	void *pCBData;
	EcrioSigMgrMethodTypeEnum methodType;
} EcrioSigMgrTimerCBStruct;

/**
*Structure Definition - END
*/

u_int32 _EcrioSigMgrInit
(
	EcrioSigMgrInitStruct *pSigMgrInitInfo,
	SIGMGRHANDLE *ppSigMgrHandle
);

u_int32 _EcrioSigMgrDeInit
(
	EcrioSigMgrStruct *pSigMgr
);

/**
*API for Setting Signaling Information.
*@Param IN SIGMGRHANDLE pSigMgrHandle - Handle to signaling manager.
*@Param IN EcrioSigMgrSignalingInfoStruct* pSignalingInfo - Signaling Parameters.
*@Return	u_int32 Error code.
*/
u_int32 _EcrioSigMgrSetSignalingParam
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSignalingInfoStruct *pSignalingInfo,
	EcrioSigMgrUriStruct *pMSISDN
);

u_int32	_EcrioSigMgrCreateCommunicationChannels
(
	EcrioSigMgrStruct *pSigMgr
);

u_int32	_EcrioSigMgrTerminateCommunicationChannels
(
	EcrioSigMgrStruct *pSigMgr
);

u_int32	_EcrioSigMgrCreateIPSecCommunicationChannels
(
	EcrioSigMgrStruct *pSigMgr
);

u_int32	_EcrioSigMgrTerminateIPSecCommunicationChannels
(
	EcrioSigMgrStruct *pSigMgr
);

/*****************************************************************************
*                         Structure Release APIs
*****************************************************************************/
u_int32 _EcrioSigMgrReleaseSigMgrStruct
(
	EcrioSigMgrStruct *pSigMgr
);

u_int32 _EcrioSigMgrReleaseSignalingInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSignalingInfoStruct *pSignalingInfo
);

u_int32 _EcrioSigMgrReleaseInternalSignalingInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrInternalSignalingInfoStruct *pSignalingInfo
);

u_int32 _EcrioSigMgrReleaseSignalingChangeParamInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSignalingChangeParamInfoStruct *pChangeParamInfo
);

u_int32 _EcrioSigMgrReleaseInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrInfoStruct *pInfo
);

u_int32 _EcrioSigMgrReleaseMessageStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrMessageStruct *pMessage
);

u_int32 _EcrioSigMgrReleaseIPAddrStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrIPAddrStruct *pIPAddrStruct
);

u_int32 _EcrioSigMgrReleaseParamsStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrParamStruct *pParamStruct
);

u_int32 _EcrioSigMgrReleaseSipUriStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipURIStruct *pSipUri
);

u_int32 _EcrioSigMgrReleaseTelUriStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrTelURIStruct *pTelUri
);

u_int32 _EcrioSigMgrReleaseUriStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUriStruct *pUri
);

u_int32 _EcrioSigMgrReleaseNameAddrStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrNameAddrStruct *pNameAddr
);

u_int32 _EcrioSigMgrReleaseNameAddrWithParamsStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrNameAddrWithParamsStruct *pNameAddrWithParams
);

u_int32 _EcrioSigMgrReleaseHdrValueStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrHeaderValueStruct *pHdrValue
);

u_int32 _EcrioSigMgrReleaseHdrStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrHeaderStruct *pHdr
);

u_int32 _EcrioSigMgrReleaseManHdrStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrMandatoryHeaderStruct *pMandatoryHdrs
);

u_int32 _EcrioSigMgrReleaseRouteStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrRouteStruct *pRoute
);

u_int32 _EcrioSigMgrReleaseContactStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrContactStruct *pContact
);

u_int32 _EcrioSigMgrReleaseViaStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrViaStruct *pVia
);

u_int32 _EcrioSigMgrReleaseMsgBodyStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrMessageBodyStruct *pMsgBody
);

#if 0
u_int32 _EcrioSigMgrReleaseSessionHandleStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSessionHandleStruct *pSessionHandle
);
#endif

u_int32 _EcrioSigMgrReleaseAuthorizationStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrAuthorizationStruct *pAuth
);

u_int32 _EcrioSigMgrReleaseAuthenticationStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrAuthenticationStruct *pAuth
);

u_int32 _EcrioSigMgrReleaseAKAStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSipAuthStruct *pAKA
);

u_int32 _EcrioSigMgrReleaseCommonInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrCommonInfoStruct *pCmnInfo
);

u_int32 _EcrioSigMgrReleaseTXNInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrTXNInfoStruct *pTXNInfo
);

/*	Support for Authorization header in all request	start	*/
u_int32 _EcrioSigMgrReleaseSharedCredentialsStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSharedCredentialsStruct *pSharedCredentials
);

u_int32 _EcrioSigMgrReleaseSipMessage
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessage
);

/* Start - Subscribe Refresh support*/
u_int32 _EcrioSigMgrReleaseSubscribeInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	void *pSubInfoStruct
);
/* End - Subscribe Refresh support*/

u_int32 _EcrioSigMgrUpdateAuthResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrAuthorizationStruct *pAuth,
	const u_char *pMethod,
	const u_char *pEntityBody,
	EcrioSipAuthStruct *pAuthResp
);

u_int32 _EcrioSigMgrInternalStructRelease
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrInternalStructTypeEnum structType,
	void **ppStruct,
	BoolEnum release
);

#if 0
/*Routing support*/
/* create a new empty hash map */
void ec_SigMgrHashMap_Create(u_int32 size, EcrioSigMgrHashMap **ppHashMap);

/* destroy a hash map */
void ec_SigMgrHashMap_Destroy(EcrioSigMgrHashMap **ppHashMap);

/* insert a new key-value pair into an existing hash map */
void ec_SigMgrHashMap_Insert(EcrioSigMgrHashMap *pHashMap, u_char *pKey, void* pValue);

/*search the most recently inserted value associated with a key*/
/*or NULL if no matching key is present*/
void ec_SigMgrHashMap_Search(EcrioSigMgrHashMap *pHashMap, u_char *pKey, void** ppValue);

/*delete the most recently inserted record with the given key*/
/*if there is no such record, has no effect*/
void ec_SigMgrHashMap_Delete(EcrioSigMgrHashMap *pHashMap, u_char *pKey);
#endif
#endif /* _ECRIO_SIG_MGR_INTERNAL_H_*/
