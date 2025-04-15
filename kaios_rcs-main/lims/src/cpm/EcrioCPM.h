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
* @file EcrioCPM.h
* @brief This is the header file for the public interface of the CPM ALC.
*
* The CPM module acts as a layer between a LIMS module and the IMS Client Framework,
* providing CPM messaging capability specific to a particular network or operator.
*/

#ifndef __ECRIOCPM_H__
#define __ECRIOCPM_H__

/** Import Data Types */
#include "EcrioPAL.h"
//#include "EcrioMSRP.h"
#include "EcrioCPMDataTypes.h"
#include "EcrioOOM.h"

/** Allow this module to be called directly from C++ compiled code. */
#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
Section 1: Compile Time Switches
******************************************************************************/

/** \brief CPM  Handle.
*
* Type definition of the CPM  handle to be used by application or upper
* layer to access CPM  APIs.
*
*/
typedef void *CPMHANDLE;					/**<  CPM ALC handle */

/*-----------------------------------------------------------------------------
Section 2b: Callback Definitions
-----------------------------------------------------------------------------*/
/** \struct EcrioCPMNotifStruct
* \brief CPM will populate this structure while notifying upper layer of
* asynchronous events through EcrioCPMNotificationCallback.
*/
typedef struct EcrioCPMNotifStruct EcrioCPMNotifStruct;

/** \brief Callback to NOTIFY the upper layer of asynchronous events.
*
* This function, implemented by an upper layer, will be called whenever a
* notification must be communicated based on an event occuring.
*
* @param[in] pCallbackData		  -	The opaque data provided by upper layer in EcrioCPMCallbackStruct.
* @param[in] pCPMNotifStruct -	Pointer to a notification structure which
*									will define the particular notification
*									being sent to the upper layer.
*/
typedef void (*EcrioCPMNotificationCallback)
(
	void *pCallbackData,
	EcrioCPMNotifStruct *pCPMNotifStruct
);

/******************************************************************************
Section 3: Preprocessor Definitions
******************************************************************************/
/*-----------------------------------------------------------------------------
Section 3a: Error Codes
-----------------------------------------------------------------------------*/
/** @name Error Codes
* List of Error codes that will be returned by CPM API and also in
* "uErrorCode" of EcrioCPMErrorCallback.
*/
/*@{*/

#define ECRIO_CPM_NO_ERROR										(0x00)	/**<  no error, API success. */
#define ECRIO_CPM_MEMORY_ERROR									(0x01)	/**< Indicates memory allocation failed when an API is called. */
#define ECRIO_CPM_INTERNAL_ERROR								(0x02)	/**< Indicates internal error occured when an API is called. */
#define ECRIO_CPM_NOT_INITIALIZED_ERROR							(0x03)	/**< Indicates CPM not initialized when an API is called. */
#define ECRIO_CPM_BUSY_ERROR									(0x04)	/**< Indicates another CPM is in progress when a CPM is initiated. */
#define ECRIO_CPM_INSUFFICIENT_DATA_ERROR						(0x05)	/**< Indicates parameters not passed proprly when an API is called. */
#define ECRIO_CPM_MSRP_ERROR									(0x06)	/**< Indicates parameters not passed proprly when an API is called. */
#define ECRIO_CPM_IMDN_ERROR									(0x07)	/**< Indicates an error while forming IMDN message. */
#define ECRIO_CPM_SOCKET_ERROR									(0x08)	/**< Indicates an error related to socket. */
#define ECRIO_CPM_MESSAGE_PARSING_ERROR							(0x09)	/**< Indicates message parsing error occured when an API is called. */

#define ECRIO_CPM_RESPONSE_SUCCESS								(200)
#define ECRIO_CPM_RESPONSE_BUSY									(486)
#define ECRIO_CPM_RESPONSE_NOT_ACCEPTABLE_HERE					(488)
#define ECRIO_CPM_RESPONSE_TEMPORARILY_UNAVAILABLE				(480)
#define ECRIO_CPM_RESPONSE_DECLINED								(603)
#define ECRIO_CPM_RESPONSE_NOT_ACCEPTABLE						(606)

#define ECRIO_CPM_RESPONSE_SUCCESS_PHRASE						(u_char *)"OK"
#define ECRIO_CPM_RESPONSE_BUSY_PHRASE							(u_char *)"Busy"
#define ECRIO_CPM_RESPONSE_NOT_ACCEPTABLE_HERE_PHRASE			(u_char *)"Not Acceptable Here"
#define ECRIO_CPM_RESPONSE_TEMPORARILY_UNAVAILABLE_PHRASE		(u_char *)"Temporarily Unavailable"
#define ECRIO_CPM_RESPONSE_DECLINED_PHRASE						(u_char *)"Declined"
#define ECRIO_CPM_RESPONSE_NOT_ACCEPTABLE_PHRASE				(u_char *)"Not Acceptable"

#define ECRIO_CPM_CALLER_ID_RESTRICTED							(u_char *)"RESTRICTED"

/*Service ID*/
//#define ECRIO_SERVICEID_3GPP											(u_char *)"org.3gpp.urn:urn-7:3gpp-" //Base String
#define ECRIO_SERVICEID_3GPP_FT_VIA_SMS									(u_char *)"org.3gpp.urn:urn-7:3gpp-application.ims.iari.rcs.ftsms" ////application.ims.iari.rcs.ftsms //File Transfer via SMS
#define ECRIO_SERVICEID_3GPP_IP_VIDEO_CALL								(u_char *)"org.3gpp.urn:urn-7:3gpp-service.ims.icsi.mmtel" ////service.ims.icsi.mmtel //IP video call
#define ECRIO_SERVICEID_3GPP_GEO_PUSH									(u_char *)"org.3gpp.urn:urn-7:3gpp-application.ims.iari.rcs.geopush" ////application.ims.iari.rcs.geopush //Geolocation PUSH
#define ECRIO_SERVICEID_3GPP_GEO_PUSH_VIA_SMS							(u_char *)"org.3gpp.urn:urn-7:3gpp-service.ims.iari.rcs.geosms" ////service.ims.iari.rcs.geosms //Geolocation PUSH via SMS
#define ECRIO_SERVICEID_3GPP_CALL_COMPOSER								(u_char *)"org.3gpp.urn:urn-7:3gpp-service.ims.icsi.gsma.callcomposer" //service.ims.icsi.gsma.callcomposer //Call composer
#define ECRIO_SERVICEID_3GPP_POST_CALL									(u_char *)"org.3gpp.urn:urn-7:3gpp-service.ims.icsi.gsma.callunanswered" //service.ims.icsi.gsma.callunanswered //Post-Call
#define ECRIO_SERVICEID_3GPP_SHARED_MAP									(u_char *)"org.3gpp.urn:urn-7:3gpp-service.ims.icsi.gsma.sharedmap" //service.ims.icsi.gsma.sharedmap //Shared Map
#define ECRIO_SERVICEID_3GPP_SHARED_SKETCH								(u_char *)"org.3gpp.urn:urn-7:3gpp-service.ims.icsi.gsma.sharedsketch" //service.ims.icsi.gsma.sharedsketch //Shared Sketch
//#define ECRIO_SERVICEID_IMS												(u_char *)"service.ims.iari.rcs." //Base String
#define ECRIO_SERVICEID_IMS_CHATBOT										(u_char *)"service.ims.iari.rcs.chatbot" //service.ims.iari.rcs.chatbot //Chatbot Communication
#define ECRIO_SERVICEID_IMS_PLUGIN										(u_char *)"service.ims.iari.rcs.plugin" //service.ims.iari.rcs.plugin //Plug-ins
//#define ECRIO_SERVICEID_GSMA											(u_char *)"org.gsma." //Base String
#define ECRIO_SERVICEID_GSMA_RCS_ISBOT									(u_char *)"org.gsma.rcs.isbot" ////org.gsma.rcs.isbot //Chatbot role
#define ECRIO_SERVICEID_GSMA_RCS_VIDEO_SHARE							(u_char *)"org.gsma.videoshare" ////org.gsma.videoshare //Video Share
//#define ECRIO_SERVICEID_OMA												(u_char *)"org.openmobilealliance:" //Base String
#define ECRIO_SERVICEID_OMA_IM											(u_char *)"org.openmobilealliance:IM-session" //org.openmobilealliance:IM-session //Chat IM version 1.0 OR ChatSession
#define ECRIO_SERVICEID_OMA_CHAT										(u_char *)"org.openmobilealliance:ChatSession" //org.openmobilealliance:ChatSession //Chat Session
#define ECRIO_SERVICEID_OMA_STANDALONE_MESSAGING						(u_char *)"org.openmobilealliance:StandaloneMsg" //org.openmobilealliance:StandaloneMsg //Standalone Messaging
#define ECRIO_SERVICEID_OMA_FTHTTP										(u_char *)"org.openmobilealliance:File-Transfer-HTTP" //org.openmobilealliance:File-Transfer-HTTP //File Transfer
#define ECRIO_SERVICEID_OMA_FTMSRP										(u_char *)"org.openmobilealliance:File-Transfer" //org.openmobilealliance:File-Transfer //File Transfer via MSRP
#define ECRIO_SERVICEID_OMA_FT_THUMB									(u_char *)"org.openmobilealliance:File-Transfer-thumb" //org.openmobilealliance:File-Transfer-thumb //File Transfer Thumbnail
#define ECRIO_SERVICEID_VERSION_1										(u_char *)"1.0" //Service id version
#define ECRIO_SERVICEID_VERSION_2										(u_char *)"2.0" //Service id version

/*Service ID*/
typedef enum
{
	EcrioCPM_NONE = 0x0000,
	//EcrioCPM_FT_VIA_SMS = 0x0001,
	//EcrioCPM_IP_VIDEO_CALL = 0x0002,
	EcrioCPM_PAGER_MODE = 0x0004,
	EcrioCPM_LARGE_MODE = 0x0008,
	EcrioCPM_CHAT = 0x0010,
	EcrioCPM_FTMSRP = 0x0020,
	EcrioCPM_FTHTTP = 0x0040,
	EcrioCPM_CHATBOT = 0x0080
}EcrioCPMPublishServiceEnum;

typedef enum
{
	EcrioCPM_Publish_None,
	EcrioCPM_Publish_Initiate,
	EcrioCPM_Publish_Modify,
	EcrioCPM_Publish_Remove
}EcrioCPMPublishEvent;

/** @enum EcrioCPMDataActionTypeEnum
* This enumeration describes the action type to modify the group chat data management.
*/
typedef enum
{
	EcrioCPMDataActionType_None = 0,				/**< No action type defined. */
	EcrioCPMDataActionType_Set,						/**< "set" action. */
	EcrioCPMDataActionType_Delete,					/**< "delete" action. */
	EcrioCPMDataActionType_Move						/**< "move" action. */
} EcrioCPMDataActionTypeEnum;

/** \enum EcrioCPMNotifCmdEnum
* Defines different notifications that the Ecrio CPM will send to the upper layer.
*/
typedef enum
{
	EcrioCPM_Notif_None = 0,
	EcrioCPM_Notif_IncomingSession,						/**< Incoming CPM Session Notification. For this event, notification structure should be EcrioCPMIncomingSessionStruct */
	EcrioCPM_Notif_SessionEstablished,					/**< Session Established Notification. For this event, notification structure should be EcrioCPMAcceptedSessionStruct */
	EcrioCPM_Notif_SessionRinging,						/**< Session Ringing Notification. For this event, notification structure should be EcrioCPMAcceptedSessionStruct */
	EcrioCPM_Notif_SessionProgress,						/**< Session In Progress Notification. For this event, notification structure should be EcrioCPMAcceptedSessionStruct */
	EcrioCPM_Notif_SessionConnectFailure,				/**< Session Connect Failure Notification. For this event, notification structure should be EcrioCPMFailedSessionStruct */
	EcrioCPM_Notif_SessionEnded,						/**< Session Ended Notification. For this event, notification structure should be EcrioCPMReasonStruct */
	EcrioCPM_Notif_PagerMessageReceived,				/**< Incoming Pagere Mode Message Notification. For this event, notification structure should be EcrioCPMIncomingPagerMessageStruct  */
	EcrioCPM_Notif_PagerMessageSendSuccess,				/**< Outgoing Pager Mode Message Send Succsess Notification. For this event, notification structure should be EcrioCPMPagerMessageIDStruct  */
	EcrioCPM_Notif_PagerMessageSendFailure,				/**< Outgoing Pager Mode Message Send Failed Notification. For this event, notification structure should be EcrioCPMPagerMessageIDStruct  */
	EcrioCPM_Notif_SessionMessageSendSuccess,			/**< Outgoing Session Message Send Succsess Notification. For this event, notification structure should be EcrioCPMSessionIDStruct  */
	EcrioCPM_Notif_SessionMessageSendFailure,			/**< Outgoing Session Message Send Failed Notification. For this event, notification structure should be EcrioCPMSessionIDStruct  */
	EcrioCPM_Notif_IncomingSessionMessage,				/**< Incoming Session Message Notification. For this event, notification structure should be EcrioCPMSessionMessageStruct  */
	EcrioCPM_Notif_IMDNDisposition,						/**< IMDN Disposition Notification. For this event, notification structure should be EcrioCPMIMDispoNotifStruct  */
	EcrioCPM_Notif_IMComposing,							/**< IMDN Disposition Notification. For this event, notification structure should be EcrioCPMSessionIMComposingStruct  */
	EcrioCPM_Notif_RichCardTransferSuccess,				/**< Rich Card Transfer Success Notification. For this event, notification structure should be EcrioCPMSessionIDStruct  */
	EcrioCPM_Notif_RichCardTransferFailure,				/**< Rich Card Transfer Failed Notification. For this event, notification structure should be EcrioCPMSessionIDStruct  */
	EcrioCPM_Notif_RichCardReceived,					/**< Rich Card Transfer Failed Notification. For this event, notification structure should be EcrioCPMSessionRichCardStruct  */
	EcrioCPM_Notif_PublishSendSuccess,					/**< Outgoing Publish Send Succsess Notification.*/
	EcrioCPM_Notif_PublishSendFailure,					/**< Outgoing Publish Send Failed Notification.*/
	EcrioCPM_Notif_SubscribeSendSuccess,				/**< Outgoing Subscribe Send Succsess Notification. */
	EcrioCPM_Notif_SubscribeSendFailure,				/**< Outgoing Subscribe Send Failure Notification. */
	EcrioCPM_Notif_UnSubscribeSendSuccess,				/**< Outgoing Unsubscribe Send Succsess Notification. */
	EcrioCPM_Notif_UnSubscribeSendFailure,				/**< Outgoing Unsubscribe Send Failure Notification. */
	EcrioCPM_Notif_ReferSendSuccess,					/**< Outgoing Refer Send Success Notification. */
	EcrioCPM_Notif_ReferSendFailure,					/**< Outgoing Refer Send Failure Notification. */
	EcrioCPM_Notif_ModifyGroupChatSuccess,				/**< Outgoing Group Session Data Management Send Succsess Notification. */
	EcrioCPM_Notif_ModifyGroupChatFailure,				/**< Outgoing Group Session Data Management Send Failure Notification. */
	EcrioCPM_Notif_GroupChatInfo,						/**< Group Chat Information Notification. For this event, notification structure should be EcrioCPMGroupChatInfoStruct */
	EcrioCPM_Notif_GroupDataStatus,						/**< Group Icon information notification. For this event, notification structure should be EcrioCPMDataManagementStruct */
	EcrioCPM_Notif_PrivacyManagementCommands,			/**< Receiving result of Privacy Management Commands Notification. For this event, notification structure should be EcrioCPMPrivMngCmdRespStruct. */
	EcrioCPM_Notif_SendMessage,
	EcrioCPM_Notif_GroupChatIcon,						/**< Receiving Group Chat Icon Notification. For this event, notification structure should be EcrioCPMGroupChatIconStruct. */
} EcrioCPMNotifCmdEnum;

/** \struct EcrioCPMNotifStruct
* \brief This structure stores the various notification details that are sent from the
* CPM to the applications.
*/
struct EcrioCPMNotifStruct
{
	EcrioCPMNotifCmdEnum eNotifCmd;								/**< Notification command enumerated value */
	void *pNotifData;											/**< Corresponding notification structure */
};

/** \struct EcrioCPMCallbackStruct
* \brief The structure is passed to the EcrioCPMInit function and contains pointers to
* notification and error callback functions.
*/
typedef struct
{
	EcrioCPMNotificationCallback pCPMNotificationCallbackFn;	/**< Event Notification callback */
//	EcrioMSRPGetPropertyCallback pMSRPGetPropertyCallback;			/**< GetProperty Callback function pointer. */
	void *pCallbackData;										/**< The opaque data which will be returned back in the callbacks */
} EcrioCPMCallbackStruct;

typedef struct
{
	void* pal;
	LOGHANDLE hLogHandle;										/**< Pointer to the log handle. */
	u_char *pPublicIdentity;
	u_char *pPrivateIdentity;
	u_char *pPassword;
	u_char *pHomeDomain;
	u_char *pDeviceId;
	u_char *pDisplayName;
	char *pRelayServerIP;							/**< IP address of the relay server */
	u_int32 uRelayServerPort;						/**< Relay server port to which client needs to send MSRP AUTH */
	BoolEnum bIsRelayEnabled;					/**< Route MSRP packets to relay server based on obtained use-path from relay server during AUTH process */
	BoolEnum bSendRingingResponse;
	BoolEnum bEnablePAI;
	EcrioOOMStruct *pOOMObject;				/**< Operator Object Model instance. */
	u_char *pPANI;
	BoolEnum bMsrpOverTLS;					/**< The flag to support MSRP Over TLS. */
} EcrioCPMParamStruct;

/** @struct EcrioCPMConnectionInfoStruct
* This structure defines local address
*/
typedef struct
{
	BoolEnum isIPv6;									/**< The IP Type i.e. IPv4 or IPv6. */
	u_char *pLocalIp;									/**< The local IP. */
	u_int16	uLocalPort;									/**< The local port. */
} EcrioCPMConnectionInfoStruct;

/** @struct EcrioCPMGroupIconFileInfoStruct
* The structure is used to the change the group chat icon using file-info.
*/
typedef struct
{
	u_char* pFileInfo;												/**< The content-id of the image file. */
	u_char* pData;													/**< The binary data of the icon. */
	u_int32 uDataLength;											/**< The length of the data. */
	u_char* pContentType;											/**< Content Type of the image file*/
} EcrioCPMGroupIconFileInfoStruct;

/** @struct EcrioCPMDataSubjectStruct
* This structure holds the Subject change/update details.
*/
typedef struct
{
	EcrioCPMDataActionTypeEnum action;			/**< Action type. */
	char *pSubjectLine;								/**< Subject to be used for group chat. */
} EcrioCPMDataSubjectStruct;

/** @struct EcrioCPMDataIconStruct
* This structure holds the Icon change/update details.
*/
typedef struct
{
	EcrioCPMDataActionTypeEnum action;			/**< Action type. */
	char *pIconUri;								/**< Indicate the icon URI. */
	EcrioCPMGroupIconFileInfoStruct* pFileInfo;	/**< Indicate the file info. */
} EcrioCPMDataIconStruct;

/** @struct EcrioCPMDataIconStruct
* This structure holds the Icon change/update details.
*/
typedef struct
{
	EcrioCPMDataActionTypeEnum action;			/**< Action type. */
	char *pHostUri;								/**< Indicate the Host URI. */
	char *pUserRole;							/**< Indicates role of the user. */
} EcrioCPMDataHostStruct;

/** @struct EcrioCPMDataManagementStruct
* This structure holds the parameters of modify group chat data management.
*/
typedef struct
{
	EcrioCPMDataSubjectStruct subject;		/**< Subject to be used for group chat. */
	EcrioCPMDataIconStruct icon;			/**< Indicate the icon URI. */
	EcrioCPMDataHostStruct host;			/**< Indicate the icon URI. */
} EcrioCPMDataManagementStruct;

/** @struct EcrioCPMStartSessionStruct
* This structure holds the parameters of start session 
*/
typedef struct
{
	u_char *pDest;
	EcrioCPMConnectionInfoStruct connStruct;
	EcrioCPMConversationsIdStruct *pConvId;
	u_int32 contentTypes;
	BoolEnum bIsLargeMode;
	BoolEnum bPrivacy;
	BoolEnum bIsChatbot;						/**< Enable if this is for chatbot session. */
	EcrioCPMFileDescriptorStruct *pFile;
	BoolEnum bIsGroupChat;						/**< Enable if this is for group chat. */
	BoolEnum bIsClosed;							/**< Enable if initiate the closed group chat session. */
	char *pSubject;								/**< Subject to be used for group chat. */
	EcrioCPMParticipantListStruct *pList;		/**< The URI list of group chat participants. */
	EcrioCPMChatbotPrivacyEnum eBotPrivacy;		/**< Privacy request for Chatbot session. */
} EcrioCPMStartSessionStruct;



/** @struct EcrioCPMPagerMessageStruct
* This structure holds the parameters of send pager message API.
*/
typedef struct
{
	u_char* pDestUri;
	EcrioCPMConversationsIdStruct* pConvId;
	EcrioCPMMessageStruct baseMsg;
	BoolEnum bIsChatbot;						/**< A flag to indicate whether destination is a Chatbot or not. */
	BoolEnum bDeleteChatBotToken;				/**< A flag to delete the chatbot token. The Anonymization Function shall delete the token if it's Enum_TRUE. */
} EcrioCPMPagerMessageStruct;

/** @struct EcrioCPMSendTextMessageStruct
* This structure holds the parameters of send message API.
*/
typedef struct
{
	char *pSessionId;
	EcrioCPMMessageStruct baseMsg;
} EcrioCPMSendMessageStruct;

//typedef EcrioCPMSendMessageStruct EcrioCPMSendRichCardStruct;


/** @struct EcrioCPMSendDataStruct
* This structure holds the parameters of send data API.
*/
typedef struct
{
	char *pSessionId;					/**< Session identifier. */
	u_char *pData;						/**< Pointer to data buffer to be sent out. */
	u_int32 uByteRangeStart;			/**< Starting range of the data to be sent. This is mapped directly to MSRP Byte-Range header. */
	u_int32 uByteRangeEnd;				/**< Starting range of the data to be sent. This is mapped directly to MSRP Byte-Range header. */
	u_int32 uTotal;						/**< Total file size to be sent. This is mapped directly to MSRP Byte-Range header. */
	char *pContentType;					/**< Content type of the file to be sent data. This is mapped directly to MSRP Content-Type header. */
	BoolEnum bIsEofReached;				/**< Enum_TRUE if this data reaches EOF, that means this is a final MSRP chunk with "$" flags. Otherwise, there will set "+" flags. */
} EcrioCPMSendDataStruct;

/** @struct EcrioCPMRespondSessionStruct
* This structure holds the parameters of respond session API.
*/
typedef struct
{
	char *pSessionId;
	EcrioCPMConnectionInfoStruct conn;
	EcrioCPMContentTypeEnum eContentType;
	EcrioCPMSessionStatusEnum eStatus;
	EcrioCPMConversationsIdStruct *pConvId;
	EcrioCPMFileDescriptorStruct *pFile;
	EcrioCPMParticipantListStruct *pList;		/**< The URI list of group chat participants. */
} EcrioCPMRespondSessionStruct;

/** @struct EcrioCPMPrivacyManagementCommandsStruct
* This structure holds the parameters of send Privacy Management Commands API.
*/
typedef struct
{
	char *pChatbot;									/**< Null terminated string holding the URI of Chatbot. */
	char *pCommandID;								/**< The Command ID. */
	EcrioCPMPrivMngCmdActionEnum eAction;			/**< CPM Privacy Management Commands action. */
} EcrioCPMPrivacyManagementCommandsStruct;

/** @struct EcrioCPMSpamReportMessageStruct
* This structure holds the parameters of send Spam Report Message API.
*/
typedef struct
{
	char *pChatbot;									/**< Null terminated string holding the URI of Chatbot. */
	char **ppMessageIDs;							/**< Pointer to the Message-ID lists. */
	u_int32 uNumOfMessageID;						/**< Number of Message-IDs. */
} EcrioCPMSpamReportMessageStruct;

/** @struct EcrioCPMNetworkParamStruct
* This structure holds network parameters specific to proxy server.
*/
typedef struct
{
	u_char *pProxyRouteAddress;						/**< Null terminated string holding Proxy server address. */
	u_int32 uProxyRoutePort;						/**< Proxy server port. */
	BoolEnum bIsProxyRouteEnabled;					/**< Conditional variable to use proxy routing or not. */
} EcrioCPMNetworkParamStruct;

/******************************************************************************
Section 6: Function Declarations
******************************************************************************/

/**
* This function, when called, will initialize resources for the ALC. In
* general, an internal structure will be allocated to hold all run-time
* ALC variables. A pointer to this structure will be returned as the
* hCPMALCHandle. All settings specified by pCPMALCParamStruct will be
* copied or acted upon.
*
* The CPM initialization.
*
* @param[in] pCPMParamStruct	Pointer to a structure that holds all
*								the basic parameters the ALC needs to
*								properly operate.
* @param[in] pCPMCallback		Pointer to a structure that holds all
*								the callback pointers for asynchronous
*								communications back to the caller.
* @param[in] hSigMgrHandle		pointer to SIGMGRHANDLE. EcrioSigMgrInit() function
*								output this handle. Valid handle should be pass and it should
*								not be NULL.
* @param[out] hCPMHandle		Pointer to the CPM  handle for this
*								instance of the ALC's memory. Note that
*								CPMHANDLE is a pointer, so a double
*								pointer is passed. The ALC will set it
*								to a valid memory address if this function
*								is successful.
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMInit
(
	EcrioCPMParamStruct *pCPMParam,
	EcrioCPMCallbackStruct *pCPMCallback,
	void *hSigMgrHandle,
	CPMHANDLE *hCPMHandle
);

/**
* This function will deinitialize the CPM . The hCPMHandle will be set
* to NULL when the function returns.
*
* @param[in] hCPMHandle	The CPM handle.
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMDeinit
(
	CPMHANDLE *hCPMHandle
);

/**
* This API can be called to publish their services. 
*
* pSessionId will be returned which contains the session ID of the CPM session,
* even before the CPM session is actually established.
*
* @param[in] hCPMHandle			The CPM ALC handle.
* @param[in] EcrioCPMPublishServiceEnum what are those services.
* @param[in] EcrioCPMPublishEvent Is he initiate/ modify/ removing to see his capabilities.
*
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/

u_int32 EcrioCPMPublishCapabilityInfo
(
	CPMHANDLE hCPMHandle,
	EcrioCPMPublishServiceEnum eServices,
	EcrioCPMPublishEvent eEvents
);
/**
* This API can be called to start CPM session. The pDest will point to
* a target CPM User to start CPM session. Default URI type lims assumes
* internally TEL URI. A SIP INVITE request will be sent to the IMS
* network as a result of this session.
*
* pSessionId will be returned which contains the session ID of the CPM session,
* even before the CPM session is actually established.
*
* @param[in] hCPMHandle			The CPM ALC handle.
* @param[in] pDestUri			Pointer to a EcrioCPMPagerMessageStruct.
* @param[in,out] ppCallId		Pointer to Call Id of the sent pager message.
*
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMSendPagerModeMessage
(
	CPMHANDLE hCPMHandle,
	EcrioCPMPagerMessageStruct* pPagerMessage,
	u_char **ppCallId
);

/**
* This API is used to get the size of the CPIM payload that need to be sent out.
*
* @param[in] hCPMHandle			The CPM ALC handle.
* @param[in] pDestUri			Pointer to a EcrioCPMPagerMessageStruct.
* @param[out] pMsgSize			Pointer to size of the message.
*
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMGetPagerModeMessageSize
(
	CPMHANDLE hCPMHandle,
	EcrioCPMPagerMessageStruct* pPagerMessage,
	u_int32* pMsgSize
);

/**
* This API can be called to start CPM session. The pDest will point to
* a target CPM User to start CPM session. Default URI type lims assumes
* internally TEL URI. A SIP INVITE request will be sent to the IMS
* network as a result of this session.
*
* pSessionId will be returned which contains the session ID of the CPM session,
* even before the CPM session is actually established.
*
* @param[in] hCPMHandle			The CPM ALC handle.
* @param[in] pStartSession		Pointer to EcrioCPMStartSessionStruct.
*
* @param[in/out] pSessionId		The ID of the CPM session that should be
*								affected.
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMStartSession
(
	CPMHANDLE hCPMHandle,
	EcrioCPMStartSessionStruct *pStartSession,
	char **ppSessionId
);

/**
* This function can be called after a notification is received that an
* incoming CPM session is detected. The sessionId would be provided with
* the notification and is needed to match the response to the specific
* CPM session to respond to.
*
* eSessionStatusCode is an enumeration and will include either 'Accept',
* 'Busy', 'Temporarily Unavailable' and 'Declined'.
*
* @param[in] hCPMHandle			The CPM handle.
* @param[in] pRespond			The pointer to EcrioCPMRespondSessionStruct.
*
*/
u_int32 EcrioCPMRespondSession
(
	CPMHANDLE hCPMHandle,
	EcrioCPMRespondSessionStruct *pRespond
);

/**
* This function is called to end an active CPM session.
*
* This API can also be called to abort a CPM session. All information regarding
* the session will be erased locally. No Request is sent out.
*
* @param[in] hCPMHandle			The CPM ALC handle.
* @param[in] sessionId			The ID of the CPM session that should be
*								affected.
* @param[in] termByUser 		Boolean that decides reason code in BYE.
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMEndSession
(
	CPMHANDLE hCPMHandle,
	char *pSessionId,
	BoolEnum termByUser
);

/**
* This function can be called to send a session mode instant message to the
 * specific session.
*
*
* @param[in] hCPMHandle			The CPM handle.
* @param[in] pTextMessage		The pointer to EcrioCPMSendMessageStruct.
* @param[in,out] ppMessageId	Pointer to Message Id .
*
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMSendMessage
(
	CPMHANDLE hCPMHandle,
	EcrioCPMSendMessageStruct* pTextMessage,
	u_char **ppMessageId
);

/**
* This function can be called to send a file data to the specific session.
*
*
* @param[in] hCPMHandle				The CPM handle.
* @param[in] pFile					The pointer to EcrioCPMSendDataStruct.
* @param[in,out] ppMessageId		Pointer to Message Id.
* @param[in,out] ppTransactionId	Pointer to Transaction Id.
*
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMSendData
(
	CPMHANDLE hCPMHandle,
	EcrioCPMSendDataStruct* pFile,
	u_char **ppMessageId,
	u_char **ppTransactionId,
	BoolEnum bIsCPIMWrapped
);

/**
* This function is called to abort file transfer an active CPM session.
*
* @param[in] hCPMHandle			The CPM handle.
* @param[in] sessionId			The ID of the CPM session that should be
*								affected.
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMAbortFileTransfer
(
	CPMHANDLE hCPMHandle,
	char *pSessionId
);

/**
* This function is called to Form IMDN XML buffer.
*
* @param[in] hCPMPHandle			The CPM handle.
* @param[in] pIMDN					The pointer to hold EcrioCPMIMDispoNotifStruct.
* @param[in/out] pLength			The pointer to return the length of the buffer.
* @param[in/out] ppBuffer			The pointer to return the formed IMDN XML buffer.
* @return							0 (zero) value upon success, else non-zero
*									if failure.
*/
u_int32 EcrioCPMFormIMDN
(
	CPMHANDLE hCPMHandle,
	EcrioCPMIMDispoNotifStruct *pIMDN,
	u_int32 *pLength,
	u_char **ppBuffer,
	u_int32 uNotifBodyElement
);

/**
* This API can be called to start group chat session. The pDest will point to
* Conference-Factory URI or Group Chat Session ID which indicated by the Conference
* Factory Application. By specifying the Group Chat Session ID that has already
* been created session, this assume a role of Join the Group Chat Session.
*
* ppSessionId will be returned which contains the CPM session ID of the internal
* group chat session, even before the CPM / group chat session is actually established.
* This is NOT a Group Chat Session ID which is notified from Conference-Factory
* Application.
*
* @param[in] hCPMHandle			The CPM handle.
* @param[in] pGroupChat			The pointer to EcrioCPMStartSessionStruct.
* @param[in/out] ppSessionId	The ID of the group chat session that should be
*								affected.
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMStartGroupChatSession
(
	CPMHANDLE hCPMHandle,
	EcrioCPMStartSessionStruct *pGroupChat,
	char **ppSessionId
);

/**
* This function can be called after a notification is received that an
* incoming a group chat session is detected. The sessionId would be provided
* with the notification and is needed to match the response to the specific
* group chat session to respond to. The pList will point to the participant
* list that indicates who is attended to the session.
*
* eSessionStatusCode is an enumeration and will include either 'Accept',
* 'Busy', 'Temporarily Unavailable' and 'Declined'.
*
* @param[in] hCPMHandle			The CPM handle.
* @param[in] pRespond			The pointer to EcrioCPMRespondSessionStruct.
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMRespondGroupChatSession
(
	CPMHANDLE hCPMHandle,
	EcrioCPMRespondSessionStruct *pRespond
);

/**
* This function is called to end an active group chat session.
*
* This API can also be called to abort a group chat session. All information
* regarding the session will be erased locally. No Request is sent out.
*
* @param[in] hCPMHandle			The CPM handle.
* @param[in] pSessionId			The ID of the group chat session that should be
*								affected.
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMEndGroupChatSession
(
	CPMHANDLE hCPMHandle,
	char *pSessionId,
	BoolEnum leaveGroup
);

/**
* This API can be used to invite other principals to existing Group Chat session.
* Participants are specified in a list format and multiple URIs can be pointed.
*
* @param[in] hCPMHandle			The CPM handle.
* @param[in] pList				Pointer to a EcrioCPMParticipantListStruct.
* @param[in] pSessionId			The ID of the CPM session that should be
*								affected.
* @param[in] ppReferId			ID to idenitfy the REFER request associated
*								with request to add participant
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMAddParticipantsToGroupChatSession
(
	CPMHANDLE hCPMHandle,
	EcrioCPMParticipantListStruct *pList,
	char *pSessionId,
	char **ppReferId
);

/**
* This API can be used to remove participants from existing Group Chat session.
* Participants are specified in a list format and multiple URIs can be pointed.
*
* @param[in] hCPMHandle			The CPM handle.
* @param[in] pList				Pointer to a EcrioCPMParticipantListStruct.
* @param[in] pSessionId			The ID of the CPM session that should be
*								affected.
* @param[in] ppReferId			ID to idenitfy the REFER request associated
*								with request to remove participant
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMRemoveParticipantsFromGroupChatSession
(
	CPMHANDLE hCPMHandle,
	EcrioCPMParticipantListStruct *pList,
	char *pSessionId,
	char **ppReferId
);

/**
* This function can be called to modify the group session data management
* related to the subject or icon. Change them according to the contents
* set in action.
*
* @param[in] hCPMHandle			The CPM handle.
* @param[in] pDataMng			Pointer to a EcrioCPMDataManagementStruct.
* @param[in] pSessionId			The ID of the CPM session that should be
*								affected.
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMModifyGroupChatData
(
	CPMHANDLE hCPMHandle,
	EcrioCPMDataManagementStruct *pDataMng,
	char *pSessionId
);

/**
* This function can be called to subscribe participant information connected
* to indicated group chat session ID.
*
* @param[in] hCPMHandle			The CPM handle.
* @param[in] pSessionId			Session identifier.
* @param[in] pGroupSessionId	The ID of the group chat session that
*								should be affected.
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMSubscribeToGroupParticipantInfo
(
	CPMHANDLE hCPMHandle,
	char *pSessionId,
	u_char *pGroupSessionId
);

/**
* This function can be called to un-subscribe participant information connected
* to indicated group chat session ID before ending Group Chat session.
*
* @param[in] hCPMHandle			The CPM handle.
* @param[in] pSessionId			Session identifier.
* @param[in] pGroupSessionId	The ID of the group chat session that
*								should be affected.
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMUnSubscribeToGroupParticipantInfo
(
	CPMHANDLE hCPMHandle,
	char *pSessionId,
	u_char *pGroupSessionId
);

/**
* This function is called to to know if given session is group chat session or Not.
*
* @param[in] hCPMHandle			The CPM handle.
* @param[in] pSessionId			The ID of the session
* @return BoolEnum				Enum_TRUE if session is GroupChat Session,
* 								Enum_FALSE if session is 1-1 Chat Session
* 								Enum_INVALID if any error.
*/
BoolEnum EcrioCPMIsGroupChatSession
(
		CPMHANDLE hCPMHandle,
		char *pSessionId
);

/**
* This function is called to get the application data.
* @param[in] hCPMHandle			The CPM handle.
* @param[in] pSessionId			The ID of the session
* @param[in/out] ppAppData		Pointer to application data
*/
u_int32 EcrioCPMGetFileAppData
(
		CPMHANDLE hCPMHandle,
		char *pSessionId,
		void **ppAppData
);

u_int32 EcrioCPMUpdate
(
	EcrioCPMParamStruct *pCPMParam,
	void *hSigMgrHandle,
	CPMHANDLE hCPMHandle
);

/**
* This function is called to Abort an active CPM session.
*
* This API can also be called to abort a CPM session. All information regarding
* the session will be erased locally. No Request is sent out.
*
* @param[in] hCPMHandle			The CPM ALC handle.
* @param[in] sessionId			The ID of the CPM session that should be
*								affected.
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMAbortSession
(
	CPMHANDLE hCPMHandle,
	char *pSessionId
);

/**
* This function is called to send Privacy Management Command.
*
*
* @param[in] hCPMHandle			The CPM handle.
* @param[in] pCmd				The pointer to EcrioCPMPrivacyManagementCommandsStruct.
* @param[in,out] ppCallId		Pointer to Call-ID of the sent pager message.
*
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMSendPrivacyManagementCommand
(
	CPMHANDLE hCPMHandle,
	EcrioCPMPrivacyManagementCommandsStruct *pCmd,
	char **ppCallId
);

/**
* This function is called to send Spam Report Message.
*
*
* @param[in] hCPMHandle			The CPM handle.
* @param[in] pCmd				The pointer to EcrioCPMSpamReportMessageStruct.
* @param[in,out] ppCallId		Pointer to Call-ID of the sent pager message.
*
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMSendSpamReportMessage
(
	CPMHANDLE hCPMHandle,
	EcrioCPMSpamReportMessageStruct *pMsg,
	char **ppCallId
);

/**
* This function is called to set network settings specific to proxy routing .
*
*
* @param[in] hCPMHandle			The CPM handle.
* @param[in] pCPMParam			The pointer to EcrioCPMNetworkParamStruct to pass network info.
*
* @return						0 (zero) value upon success, else non-zero
*								if failure.
*/
u_int32 EcrioCPMSetNetworkParam
(
	CPMHANDLE hCPMHandle,
	EcrioCPMNetworkParamStruct *pCPMParam
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// __ECRIOCPM_H__
