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

#ifndef  _ECRIO_CPM_INTERNAL_H_
#define  _ECRIO_CPM_INTERNAL_H_

/** Import data types */
#include "EcrioSDP.h"
#include "EcrioCPIM.h"
//#include "EcrioMSRP.h"
#include "EcrioIMDN.h"
#include "EcrioCPM.h"
#include "EcrioPAL.h"
#include "EcrioSigMgrCommon.h"
#include "EcrioSigMgrInit.h"
#include "EcrioSigMgrInstantMessage.h"
#include "EcrioSigMgrPublish.h"
#include "EcrioSigMgrCallControl.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**********************************************************************************************/
/*                  CPM Internal Macros Definition - START                    */
/**********************************************************************************************/

#ifdef ENABLE_LOG
//#define BITSWANTED 8 // its use to travarse bits to get information of publish service id's 
#define CPMLOGV(a, b, c, ...)		pal_LogMessage((a), KLogLevelVerbose, KLogComponentCPM | KLogComponent_ALC, (b), (c),##__VA_ARGS__);
#define CPMLOGI(a, b, c, ...)		pal_LogMessage((a), KLogLevelInfo, KLogComponentCPM | KLogComponent_ALC, (b), (c),##__VA_ARGS__);
#define CPMLOGD(a, b, c, ...)		pal_LogMessage((a), KLogLevelDebug, KLogComponentCPM | KLogComponent_ALC, (b), (c),##__VA_ARGS__);
#define CPMLOGW(a, b, c, ...)		pal_LogMessage((a), KLogLevelWarning, KLogComponentCPM | KLogComponent_ALC, (b), (c),##__VA_ARGS__);
#define CPMLOGE(a, b, c, ...)		pal_LogMessage((a), KLogLevelError, KLogComponentCPM | KLogComponent_ALC, (b), (c),##__VA_ARGS__);
#else
#define CPMLOGV(a, b, c, ...)		if (a != NULL) {;}
#define CPMLOGI(a, b, c, ...)		if (a != NULL) {;}
#define CPMLOGD(a, b, c, ...)		if (a != NULL) {;}
#define CPMLOGW(a, b, c, ...)		if (a != NULL) {;}
#define CPMLOGE(a, b, c, ...)		if (a != NULL) {;}
#endif
#define BITSWANTED 8 // its use to travarse bits to get information of publish service id's
#define ECRIO_CPM_MAX_SESSIONS							1
#define ECRIO_CPM_MAX_COMMON_BUFFER_SIZE				512
#define ECRIO_CPM_MAX_IMDN_MESSAGE						100

#define ECRIO_CPM_STRING_SIZE_16							16
#define ECRIO_CPM_SDP_INSTANCE_SIZE						512
#define ECRIO_CPM_CPIM_INSTANCE_SIZE					512
#define ECRIO_CPM_IMDN_ALLOCATION_SIZE					256
#define ECRIO_CPM_PIDF_ALLOCATION_SIZE					256
#define ECRIO_CPM_MSRP_ALLOCATION_SIZE					3072
#define ECRIO_CPM_SDP_SESSION_INSTANCE_SIZE				16384
#define ECRIO_CPM_SDP_INFO_BUFFER_SIZE					512
#define ECRIO_CPM_SDP_STRING_SIZE						1024
#define ECRIO_CPM_XML_BUFFER_SIZE						1024
#define ECRIO_CPM_XML_ENTRY_URI_BUFFER_SIZE				294
//#define ECRIO_CPM_NOTIFICATION_BUFFER_SIZE				80
#define ECRIO_CPM_MRSP_RECV_BUFFER_SIZE					8192
#define ECRIO_CPM_MSRP_BASE_PORT						40000

#define ECRIO_CPM_CRLF_SYMBOL							(u_char *)"\r\n"
#define ECRIO_CPM_BOUNDARY								(u_char *)"--"
#define ECRIO_CPM_SPACE_SYMBOL							(u_char *)" "
#define ECRIO_CPM_SLASH_SYMBOL							(u_char *)"/"
#define ECRIO_CPM_COLON_SYMBOL							(u_char *)":"
#define ECRIO_CPM_CONTENT_TYPE_HEADER					(u_char *)"Content-Type"
#define ECRIO_CPM_CONTENT_LENGTH_HEADER					(u_char *)"Content-Length"

#define ECRIO_CPM_3GPP_DOUBLE_QUOTE						(u_char *)"\""
#define ECRIO_CPM_3GPP_SEMICOLON						(u_char *)";"
#define ECRIO_CPM_3GPP_COMMA							(u_char *)","
#define ECRIO_CPM_3GPP_EQUAL							(u_char *)"="
#define ECRIO_CPM_3GPP_SIP_INSTANCE						(u_char *)"+sip.instance"
#define ECRIO_CPM_3GPP_INSTANCE_VALUE					(u_char *)"\"<urn:gsma:imei:%s>\""
#define ECRIO_CPM_3GPP_ICSI_PREFIX_REF					(u_char *)"+g.3gpp.icsi-ref="
#define ECRIO_CPM_3GPP_IARI_PREFIX_REF					(u_char *)"+g.3gpp.iari-ref="
#define ECRIO_CPM_3GPP_FEATURE_FILE_TRANSFER			(u_char *)"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fthttp"
#define ECRIO_CPM_3GPP_FEATURE_CHATBOT					(u_char *)"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot"
#define ECRIO_CPM_3GPP_FEATURE_CHATBOT_STANDALONE		(u_char *)"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot.sa"
#define ECRIO_CPM_3GPP_FEATURE_ALIAS					(u_char *)"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.alias"
#define ECRIO_CPM_3GPP_FEATURE_DEL_TK					(u_char *)"urn%3Aurn-7%3A3gpp-application.ims.iari.rcsmaap.tkdel"
#define ECRIO_CPM_3GPP_FEATURE_SESSION					(u_char *)"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session"
#define ECRIO_CPM_3GPP_FEATURE_GROUP_SESSION			(u_char *)"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session.group"
#define ECRIO_CPM_3GPP_FEATURE_LARGEMSG					(u_char *)"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.largemsg"
#define ECRIO_CPM_3GPP_FEATURE_PAGERMSG					(u_char *)"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.msg"
#define ECRIO_CPM_3GPP_FEATURE_FT_OVER_MSRP				(u_char *)"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.filetransfer"
#define ECRIO_CPM_3GPP_FEATURE_SYSTEMMSG				(u_char *)"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.systemmsg"
#define ECRIO_CPM_3GPP_FEATURE_BOTVERSION				(u_char *)"+g.gsma.rcs.botversion=\"#=1\""
#define ECRIO_CPM_3GPP_FEATURE_ISBOT					(u_char *)"+g.gsma.rcs.isbot"
#define ECRIO_CPM_CONTACT_HEADER_STRING					(u_char *)"Contact"
#define ECRIO_CPM_ACCEPT_CONTACT_HEADER_STRING			(u_char *)"Accept-Contact"
#define ECRIO_CPM_P_PREFERRED_SERVICE_HEADER_STRING		(u_char *)"P-Preferred-Service"
#define ECRIO_CPM_P_PREFERRED_SERVICE_3GPP_FEATURE_PAGERMSG					(u_char *)"urn:urn-7:3gpp-service.ims.icsi.oma.cpm.msg"
#define ECRIO_CPM_P_PREFERRED_SERVICE_3GPP_FEATURE_LARGEMSG					(u_char *)"urn:urn-7:3gpp-service.ims.icsi.oma.cpm.largemsg"
#define ECRIO_CPM_P_PREFERRED_SERVICE_3GPP_FEATURE_SESSION					(u_char *)"urn:urn-7:3gpp-service.ims.icsi.oma.cpm.session"
#define ECRIO_CPM_P_PREFERRED_SERVICE_3GPP_FEATURE_GROUP_SESSION			(u_char *)"urn:urn-7:3gpp-service.ims.icsi.oma.cpm.session.group"
#define ECRIO_CPM_P_PREFERRED_SERVICE_3GPP_FEATURE_SYSTEMMSG				(u_char *)"urn:urn-7:3gpp-service.ims.icsi.oma.cpm.systemmsg"
#define ECRIO_CPM_3GPP_FEATURE_PUSH_LOCATION			(u_char *)"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geopush"

#define ECRIO_CPM_XML_VERSION							(u_char *)"1.0"
#define ECRIO_CPM_XML_ENCODING							(u_char *)"UTF-8"
#define ECRIO_CPM_XML_OPENING_ANGLE_BRACE				(u_char *)"<"
#define ECRIO_CPM_XML_CLOSING_ANGLE_BRACE				(u_char *)">"
#define ECRIO_CPM_XML_QUESTIONMARK						(u_char *)"?"
#define ECRIO_CPM_XML_VERSION1							(u_char *)"xml version"
#define ECRIO_CPM_XML_ENCODING1							(u_char *)"encoding"
#define ECRIO_CPM_XML_EQUALTO							(u_char *)"="
#define ECRIO_CPM_XML_SPACE								(u_char *)" "
#define ECRIO_CPM_XML_NEWLINE							(u_char *)"\n"
#define ECRIO_CPM_XML_DOUBLEQUOTES						(u_char *)"\""
#define ECRIO_CPM_XML_SLASH								(u_char *)"/"

#define ECRIO_CPM_XML_RESOURCE_LISTS					(u_char *)"resource-lists"
#define ECRIO_CPM_XML_CPM_GROUP_MANAGEMENT				(u_char *)"cpm-group-management"
#define ECRIO_CPM_XML_ATTRIBUTE_DEFAULTNS				(u_char *)"xmlns"
#define ECRIO_CPM_XML_VALUE_RESOURCE_LISTNS				(u_char *)"urn:ietf:params:xml:ns:resource-lists"
#define ECRIO_CPM_XML_VALUE_GROUP_DATANS				(u_char *)"urn:oma:xml:cpm:groupdata:1.0"
#define ECRIO_CPM_XML_ATTRIBUTE_CPNS					(u_char *)"xmlns:cp"
#define ECRIO_CPM_XML_VALUE_CPNS						(u_char *)"urn:ietf:params:xml:ns:copyControl"
#define ECRIO_CPM_XML_REQUEST							(u_char *)"request"
#define ECRIO_CPM_XML_TARGET							(u_char *)"target"
#define ECRIO_CPM_XML_SUBJECT							(u_char *)"subject"
#define ECRIO_CPM_XML_USER_ROLE							(u_char *)"user-role"
#define ECRIO_CPM_XML_PARTICIPANT						(u_char *)"participant"
#define ECRIO_CPM_XML_ICON								(u_char *)"icon"
#define ECRIO_CPM_XML_ROLE								(u_char *)"role"
#define ECRIO_CPM_XML_ICON_URI							(u_char *)"icon-uri"
#define ECRIO_CPM_XML_FILE_INFO							(u_char *)"file-info"
#define ECRIO_CPM_XML_ACTION							(u_char *)"action"
#define ECRIO_CPM_XML_SET								(u_char *)"set"
#define ECRIO_CPM_XML_DELETE							(u_char *)"delete"
#define ECRIO_CPM_XML_MOVE								(u_char *)"move"
#define ECRIO_CPM_XML_DATA								(u_char *)"data"
#define ECRIO_CPM_XML_LIST								(u_char *)"list"
#define ECRIO_CPM_XML_GROUP_DATA						(u_char *)"group-data"
#define ECRIO_CPM_XML_ID								(u_char *)"id"
#define ECRIO_CPM_XML_ENTRY_URI							(u_char *)"entry uri"
#define ECRIO_CPM_XML_CP_COPYCONTROL					(u_char *)"cp:copyControl"
#define ECRIO_CPM_XML_SMALL_TO							(u_char *)"to"
#define ECRIO_CPM_XML_SMALL_CC							(u_char *)"cc"
#define ECRIO_CPM_XML_SMALL_BCC							(u_char *)"bcc"
#define ECRIO_CPM_XML_METHOD							(u_char *)"method"
#define ECRIO_CPM_XML_INVITE							(u_char *)"INVITE"
#define ECRIO_CPM_XML_BYE								(u_char *)"BYE"

#define ECRIO_CPM_SESSION_TERMINATION_REASON_1001		1001
#define ECRIO_CPM_SESSION_TERMINATION_REASON_TEXT_1001		(u_char *)"MSRP Socket Failure"

#define CPM_STRING_CONCATENATE(pDst, uDstLen, pSrc, uSrcLen, err, label)	if (NULL == pal_StringNConcatenate(pDst, uDstLen, pSrc, uSrcLen)) \
	{ err = ECRIO_SDP_MEMORY_ERROR; goto label; }

/**********************************************************************************************/
/*                  CPM Internal Macros Definition - END                      */
/**********************************************************************************************/

/**********************************************************************************************/
/*                  CPM Internal Enumerated Constants Definition - START                    */
/**********************************************************************************************/
typedef void* CPMMAPHANDLE;

/**********************************************************************************************/
/*                  CPM Internal Enumerated Constants Definition - END                      */
/**********************************************************************************************/

/** \enum EcrioCPMSessionStateEnum
 * Defines different CPM state enumeration.
 */
typedef enum
{
	EcrioCPMSessionState_Unused = 0,				/**< Session slot is not yet used, but initialized. */
	EcrioCPMSessionState_Starting,					/**< Session has been started or incoming session presented to UI. */
	EcrioCPMSessionState_Active,					/**< Session is active and in-session. */
	EcrioCPMSessionState_Ending						/**< Session is ending. */
} EcrioCPMSessionStateEnum;

typedef enum
{
	EcrioCPMParticipantType_None = 0,
	EcrioCPMParticipantType_Add,
	EcrioCPMParticipantType_Remove
} EcrioCPMParticipantTypeEnum;

typedef enum
{
	EcrioSessionTerminationReason_None = 0,
	EcrioSessionTerminationReason_MSRPSocketFailure
} EcrioSessionTerminationReasonEnum;

/**********************************************************************************************/
/*                  CPM Internal Data Structure Definition - START                          */
/**********************************************************************************************/

/** @struct EcrioCPMRemoteSDPInfoStruct
* Contains information about the remote MSRP information.
*/
typedef struct
{
	u_char* pRemoteIp;
	u_int16 uRemotePort;
	BoolEnum isIPv6;
	u_char* pRemotePath;
} EcrioCPMRemoteSDPInfoStruct;

/** @struct EcrioCPMRelayDataStruct
* Contains information that is required to book keep to support MSRP Relay.
*/
typedef struct
{
	BoolEnum bPrivacy;
	EcrioCPMSessionStatusEnum eSessionStatus;
	u_char *pDest;
	u_char *pLocalPath;
	u_char *pUsePath;
	u_char *pAppCallId;
	EcrioCPMConnectionInfoStruct localConnInfo;
} EcrioCPMRelayDataStruct;

/** @struct EcrioCPMParsedXMLAttrStruct
* Contains information about attribute name and value in XML element.
*/
typedef struct
{
	u_char *pAttrName;
	u_char *pAttrValue;
} EcrioCPMParsedXMLAttrStruct;

/** @struct EcrioCPMParsedXMLStruct
* Contains information that is parsed XML document.
*/
typedef struct
{
	u_char *pElementName;
	u_char *pElementValue;

	u_int32 uNumOfAttr;
	EcrioCPMParsedXMLAttrStruct **ppAttr;

	u_int32 uNumOfChild;
	void **ppChild;

	void *parent;
} EcrioCPMParsedXMLStruct;

/** @struct EcrioCPMMultipartBodyStruct
* Contains information about contents of message body part in multipart message.
*/
typedef struct
{
	u_char *pContentType;
	u_char *pContentId;
	u_char *pContentDisposition;
	u_char *pContentTransferEncoding;
	u_int32 uContentLength;
	u_char *pMessageBody;
} EcrioCPMMultipartBodyStruct;

/** @struct EcrioCPMParsedXMLStruct
* Contains information that is parsed multipart messages.
*/
typedef struct
{
	u_int32 uNumOfBody;
	EcrioCPMMultipartBodyStruct **ppBody;
} EcrioCPMMultipartMessageStruct;

/** @struct EcrioCPMSessionStruct
 * Contains information about the session.
 */
typedef struct
{
	u_int32 contentTypes;
	EcrioCPMSessionStateEnum state;
	u_char *pCallId;
	SDPSESSIONHANDLE hSDPSessionHandle;
	u_char *pSDPSessionInstance;
	u_char *pSDPInformationBuffer;
	u_char *pSDPStringBuffer;
	u_char *pSDPWorkingBuffer;
	EcrioSDPInformationStruct localSDP;
	EcrioSDPInformationStruct remoteSDP;
	//u_char *pNotificationBuffer;
	u_char *pMSRPSessionInstance;
//	MSRP_SESSION_HANDLE hMSRPSessionHandle;
	EcrioSessionTerminationReasonEnum eTerminationReason;
	u_char *pMSRPRecvBuffer;
	EcrioCPMRemoteSDPInfoStruct remoteSDPInfo;
//	MSRPConnectionTypeEnum eLocalConnectionType;
	BoolEnum bIsLargeMessage;
	EcrioCPMSessionStatusEnum eSessionStatus;
	EcrioCPMRelayDataStruct relayData;
	EcrioCPMConversationsIdStruct *pConvId;
	EcrioCPMFileDescriptorStruct *pFile;
	BoolEnum bIsGroupChat;
	BoolEnum bIsClosedGroupChat;
	BoolEnum bIsChatbot;
	u_char *pGroupSessionId;
	char *pSubject;
	EcrioCPMParticipantListStruct *pList;
	u_char *pGrpChatSubSessId;
	BoolEnum bIsStartMSRPSession;
	BoolEnum bPrivacy;
	u_char *pPeerDisplayName;
	EcrioCPMChatbotPrivacyEnum eBotPrivacy;
} EcrioCPMSessionStruct;

/** @struct EcrioCPMContextStruct
 * Context maintained for CPM. This is given to the application as EcrioCPMHANDLE
 */

typedef  struct
{
	EcrioCPMCallbackStruct callbackStruct;			/**< contains information about callbacks to upper layer */
	void *hEcrioSigMgrHandle;						/**< handle of IMS module */
	SDPHANDLE hSDPHandle;							/**< handle of SDP module */
	u_char *pSDPInstance;							/**< Instance buffer of SDP module */
	CPIM_HANDLE hCPIMHandle;						/**< handle of CPIM module */
	u_char *pCPIMInstance;							/**< Instance buffer of CPIM module */
	IMDN_HANDLE hIMDNHandle;						/**< handle of IMDN module */
	u_char *pIMDNInstance;							/**< Instance buffer of IMDN module */
//	MSRP_HANDLE hMSRPHandle;						/**< handle of MSRP module */
	u_char *pMSRPInstance;							/**< Instance buffer of MSRP module */
	LOGHANDLE hLogHandle;							/**< handle of Log module */
	u_char *pPublicIdentity;						/**< public identifier */
	u_char *pLocalIpAddress;						/**< local-IP used in sdp */
	u_char *pLocalDomain;							/**< local-domain used in sdp */
	u_char *pDeviceId;								/**< device id used to build feature-tag */
	u_char *pDisplayName;							/**< display-name used in Contact header */
	BoolEnum bSendRingingResponse;					/**< send 180 ringing response for an INVITE if Enum_TRUE. */
	u_char *pString;								/**< common buffer for strings */
	EcrioOOMStruct *pOOMObject;						/**< Operator Object Model instance. */
	BoolEnum bIsRelayEnabled;						/**< Route MSRP packets to relay server based on obtained use-path from relay server during AUTH process */
	char *pRelayServerIP;							/**< IP address of the relay server */
	u_int32 uRelayServerPort;						/**< Relay server port to which client needs to send MSRP AUTH */
	BoolEnum bEnablePAI;							/**< Check P-Associated-Identity header to validate restriction ID. */
	u_char *pPANI;
	CPMMAPHANDLE hHashMap;							/**< Handle to a hashmap that hold sessionID and CallId */
	BoolEnum bMsrpOverTLS;							/**< The flag to support MSRP Over TLS. */
	EcrioCPMNetworkParamStruct proxyParam;			/**< Hold network info specific to proxy routing info. */
} EcrioCPMContextStruct;

/**********************************************************************************************/
/*                  CPM Internal Data Structure Definition - END                            */
/**********************************************************************************************/

/**********************************************************************************************/
/*                        Internal CPM FUNCTIONS - START                                 */
/**********************************************************************************************/

/** Function that will be called when the SigMgr calls the Status Callback of the CPM **/
void CPMSigMgrStatusCallback
(
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct,
	void *pCallbackData
);

/** Function that will be called when the SigMgr calls the InfoCallback of the CPM **/
void CPMSigMgrInfoCallback
(
	EcrioSigMgrInfoTypeEnum infoType,
	s_int32 infoCode,
	EcrioSigMgrInfoStruct *pData,
	void *pCallbackData
);


/** Internal Function that populates sdp body **/
u_int32 ec_CPM_FillInviteSdp
(
	EcrioCPMContextStruct *pContextStruct,
	EcrioCPMSessionStruct *pCPMSession,
	EcrioSDPEventTypeEnum eEvent,
	u_char *pLocalIp,
	u_int32 uLocalPort,
	BoolEnum isIPv6,
	EcrioCPMContentTypeEnum eContentType,
	u_char *pPath,
	EcrioSDPMSRPAttributeSetupEnum eSetup,
	EcrioSDPInformationStruct *pInfo,
	EcrioSigMgrUnknownMessageBodyStruct *pUnknownMsgBody
);

/** Internal function called when an incoming BYE Request Notification is received	**/
void ec_CPM_HandleSessionEndRequest
(
	EcrioCPMContextStruct *pContextStruct,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
);

/** Internal function called when an incoming BYE Response Notification is received	**/
void ec_CPM_HandleSessionEndResponse
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
);

void ec_CPM_HandlePrackRequest
(
	EcrioCPMContextStruct *pContextStruct,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
);
/** Internal function called when an incoming INVITE Request Notification is received	**/
void ec_CPM_HandleInviteMessageRequest
(
	EcrioCPMContextStruct *pContextStruct,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
);
/** Internal function called when an incoming INVITE Response Notification is received	**/
void ec_CPM_HandleInviteMessageResponse
(
	EcrioCPMContextStruct *pContextStruct,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
);

/** Internal function called when an incoming Re-INVITE Request Notification is received	**/
void ec_CPM_HandleReInviteMessageRequest
(
	EcrioCPMContextStruct *pContextStruct,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
);
/** Internal function called when an incoming Re-INVITE Response Notification is received	**/
void ec_CPM_HandleReInviteMessageResponse
(
	EcrioCPMContextStruct *pContextStruct,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
);

u_int32 ec_CPM_HandleSessionRefreshRequest
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
);

void ec_CPM_HandleInstantMessageRequest
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
);

void ec_CPM_HandleSubscribeResponse
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
);

void ec_CPM_HandleNotifyRequest
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
);

void ec_CPM_HandleReferResponse
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
);

u_int32 ec_CPM_EndSession
(
	EcrioCPMContextStruct *pContextStruct,
	EcrioCPMSessionStruct *pCurrentCPMSessionStruct,
	BoolEnum isTerminatedByUser
);

void ec_CPM_HandleUpdateRequest
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
);

u_int32 ec_CPM_ReleaseCPMSession
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMSessionStruct *pSession
);

u_int32 ec_CPM_PopulateFeatureTags
(
	EcrioCPMContextStruct *pContext,
	u_int32 contentTypes,
	BoolEnum bIsLargeMode,
	BoolEnum bIsGroupChat,
	BoolEnum bIsPagerMode,
	BoolEnum bIsChatbotSA,
	BoolEnum bDeleteToken,
	EcrioSigMgrFeatureTagStruct **ppFeatureTags
);

void ec_CPM_getRandomString
(
	u_char *pStr,
	u_int32 uLength
);

u_int32 ec_CPM_PopulateCPIMHeaders
(
	EcrioCPMContextStruct *pContext,
	u_char* pFromAddr,
	u_char* pFromDispName,
	u_char* pToAddr,
	EcrioCPMMessageStruct *pMsgStruct,
	CPIMMessageStruct *pCPIMMessage
);

u_int32 ec_CPM_GetSessionId
(
	u_char **ppStr
);


u_char* ec_FormCPIMFromHeaderValue
(
	EcrioCPMContextStruct *pContext,
	u_char* pFromAddr
);

/*Internal function which checks the response xml body elements present. */
BoolEnum ec_CPM_Handle3gppIMSXML
(
	CPIM_HANDLE hLogHandle,
	u_char *message
);

BoolEnum ec_CPMIsLargeMessage
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrSipMessageStruct *pSipMessage
);

BoolEnum ec_CPMIsPrivacyManagementResponse
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrSipMessageStruct *pSipMessage
);

u_int32 ec_CPM_GetAliasParamFromPAI
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrNameAddrStruct *pPAI,
	EcrioCPMAliasingParamStruct **ppAlias
);

u_int32 ec_CPMSendInviteResponse
(
	EcrioCPMContextStruct *pContext,
	u_char *pCallId,
	u_char *pReasonPhrase,
	u_int32 eResponseCode
);

u_int32 ec_CPM_ReleaseCPMNotifStruct
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMNotifStruct* pCPMNotifStruct
);

/** Internal function called when an ACK request is received */
void ec_CPM_HandleAckMessageRequest
(
	EcrioCPMContextStruct *pContext,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
);

u_int32 ec_CPM_MapInit
(
	CPMMAPHANDLE* ppHandle
);

u_int32 ec_CPM_MapDeInit
(
	CPMMAPHANDLE pHandle
);

u_int32 ec_CPM_MapSetData
(
	EcrioCPMContextStruct *pContext,
	u_char* pSessionID,
	void* pAppData
);

u_int32 ec_CPM_MapDeleteKeyData
(
	CPMMAPHANDLE pHandle, 
	u_char* key
);

u_int32 ec_CPM_MapGetData
(
	EcrioCPMContextStruct *pContext,
	u_char* pSessionID,
	void** ppAppData
);

u_int32 ec_CPM_StartSession
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMStartSessionStruct *pStartSession,
	EcrioCPMSessionStruct *pCPMSession,
	char **ppSessionId
);

u_int32 ec_CPM_StartMSRPAuthProcess
(
	EcrioCPMContextStruct *pCPMContext,
	EcrioCPMStartSessionStruct *pStartSession,
	EcrioCPMSessionStruct *pCPMSession,
	char **ppSessionId
);

u_int16 ec_CPM_GenerateRandomPort();

BoolEnum ec_CPM_FindXmlElementNameAndValue
(
	EcrioCPMParsedXMLStruct *pParsedXml,
	u_char *pElemName,
	u_char *pElemValue
);

BoolEnum ec_CPM_FindXmlAttributeNameAndValue
(
	EcrioCPMParsedXMLStruct *pParsedXml,
	u_char *pAttrName,
	u_char *ppAttrValue
);

u_char* ec_CPM_GetXmlAttributeValue
(
	EcrioCPMParsedXMLStruct *pParsedXml,
	u_char *pElemName
);

u_int32 ec_CPM_FillGroupChatInfo
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMGroupChatInfoStruct *pInfo,
	EcrioCPMParsedXMLStruct *pParsedXml
);

u_int32 ec_CPM_FillResourceLists
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMParticipantListStruct *pList,
	EcrioCPMParsedXMLStruct *pParsedXml
);

BoolEnum ec_CPM_StringIsQuoted
(
	u_char *pIn_String
);

u_char *ec_CPM_StringUnquote
(
	u_char *pIn_String
);

u_int32 ec_CPM_HandleParticipantsToGroupChatSession
(
	EcrioCPMContextStruct *pContext,
	BoolEnum bIsAddParticipants,
	EcrioCPMParticipantListStruct *pList,
	char *pSessionId,
	char **ppReferId
);

u_int32 ec_CPM_ParseXmlDocument
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMParsedXMLStruct **ppParsedXml,
	char *pData,
	u_int32 uLen
);

void ec_CPM_ReleaseParsedXmlStruct
(
	EcrioCPMParsedXMLStruct **ppParsedXml,
	BoolEnum release
);

u_int32 ec_CPM_ParseMIMEMessageBody
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMMultipartBodyStruct *pStruct,
	u_char *pData,
	u_int32 uLen
);

u_int32 ec_CPM_ParseMultipartMessage
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMMultipartMessageStruct **ppStruct,
	char *pBoundary,
	char *pData,
	u_int32 uLen
);

void ec_CPM_ReleaseMultiPartStruct
(
	EcrioCPMMultipartMessageStruct **ppStruct,
	BoolEnum release
);

u_int32 ec_CPM_FormMultipartMessage
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMMultipartMessageStruct *pStruct,
	char **ppData,
	u_int32 *pLen,
	char **ppBoundary
);

u_int32 ec_CPM_FormGroupSessionDataManagementXML
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMGroupDataRequestStruct *pStruct,
	char **ppData,
	u_int32 *pLen
);

u_int32 ec_CPM_FormResourceListXML
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMParticipantListStruct *pStruct,
	EcrioCPMParticipantTypeEnum eType,
	char **ppData,
	u_int32 *pLen
);

u_int32 ec_CPM_FormIsComposingXML
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMComposingStruct *pComposing,
	char **ppData,
	u_int32 *pLen
);

u_int32 ec_CPM_FormPrivacyManagementCommandXML
(
	EcrioCPMContextStruct *pContext,
	char *pCommandID,
	EcrioCPMPrivMngCmdActionEnum eAction,
	char **ppData,
	u_int32 *pLen
);

u_int32 ec_CPM_FormSpamReportMessageXML
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMSpamReportMessageStruct *pMsg,
	char **ppData,
	u_int32 *pLen
);

/**********************************************************************************************/
/*                        Internal CPM FUNCTIONS - END                                   */
/**********************************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*  _ECRIO_CPM_INTERNAL_H_ */
