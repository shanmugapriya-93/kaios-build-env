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

#ifndef __ECRIOCPMDATATYPES_H__
#define __ECRIOCPMDATATYPES_H__

/** \enum EcrioCPMSessionStatusEnum
 * Defines different ways to respond to an incoming CPM session.
 */
typedef enum
{
	EcrioCPM_SessionStatus_ACCEPT = 0,					/**< Status used to accept the incoming CPM session. */
	EcrioCPM_SessionStatus_BUSY,						/**< Status used to busy the incoming CPM session. */
	EcrioCPM_SessionStatus_NOT_ACCEPTABLE_HERE,			/**< Status used to indicate that does not acceptable here. */
	EcrioCPM_SessionStatus_TEMPORARILY_UNAVAILABLE,		/**< Status used to temporarily unavailable the incoming CPM session. */
	EcrioCPM_SessionStatus_DECLINED,					/**< Status used to declined the incoming CPM session. */
	EcrioCPM_SessionStatus_NOT_ACCEPTABLE				/**< Status used to not accept the incoming CPM session. */
} EcrioCPMSessionStatusEnum;

/** \enum EcrioCPMContentTypeEnum
 * This enumeration defines the types of requested CPM session.
 */
typedef enum
{
	EcrioCPMContentTypeEnum_None = 0x0000,					/**< No content-type defined. */
	EcrioCPMContentTypeEnum_Text = 0x0001,					/**< Plain text message, indicate the "text/plain". */
	EcrioCPMContentTypeEnum_IMDN = 0x0002,					/**< Disposition Notification, indicate the "message/imdn+xml". */
	EcrioCPMContentTypeEnum_FileTransferOverMSRP = 0x0004,	/**< MSRP Based File Transfer. Used as part of session setup to include File Transfer over MSRP related headers in INVITE, 200 OK and SDP. */
	EcrioCPMContentTypeEnum_FileTransferOverHTTP = 0x0008,	/**< HTTP based File Transfer, indicate the "application/vnd.gsma.rcs-ft-http+xml". */
	EcrioCPMContentTypeEnum_Composing = 0x0010,				/**< Composing notification, indicate the "application/im-iscomposing+xml" */
	EcrioCPMContentTypeEnum_RichCard = 0x0020,				/**< Rich Cards, indicate the "application/vnd.gsma.botmessage.v1.0+json". */
	EcrioCPMContentTypeEnum_SuggestedChipList = 0x0040,		/**< Suggested Chip List, indicate the "application/vnd.gsma.botsuggestion.v1.0+json". */
	EcrioCPMContentTypeEnum_SuggestionResponse = 0x0080,	/**< Client Response to Suggestion, indicate the "application/vnd.gsma.botsuggestion.response.v1.0+json". */
	EcrioCPMContentTypeEnum_PrivacyManagementCommand = 0x0100,	/**< Privacy Management Commands, indicate the "application/vnd.gsma.rcsalias-mgmt+xml". */
	EcrioCPMContentTypeEnum_SpamReportMessage = 0x0200,		/**< Spam Report Message, indicate the "application/vnd.gsma.rcsspam-report+xml". */
	EcrioCPMContentTypeEnum_GroupData = 0x0400,				/**< CPM Group Data, indicate the "application/vnd.oma.cpm-groupdata+xml". */
	EcrioCPMContentTypeEnum_WildCard = 0x0800,				/**< Indicates wild card. */
	EcrioCPMContentTypeEnum_PushLocation = 0x1000,			/**< Push Location, indicate the "application/vnd.gsma.rcspushlocation+xml". */
	EcrioCPMContentTypeEnum_SharedClientData = 0x2000		/**< Shared Client Data, indicate the "application/vnd.gsma.botsharedclientdata.v1.0+json". */
} EcrioCPMContentTypeEnum;

/** @enum EcrioCPMTrafficTypeEnum
* This enumeration describes the  CPIM header Traffic-Type.
* An example CPIM header is maap.Traffic-Type: advertisement.
*/
typedef enum
{
	EcrioCPMTrafficTypeNone,				/**< No Traffic-Type type defined. */
	EcrioCPMTrafficTypeAdvertisement,		/**< "advertisement" Traffic-Type. */
	EcrioCPMTrafficTypePayment,				/**< "payment" Traffic-Type. */
	EcrioCPMTrafficTypePremium,				/**< "premium" Traffic-Type. */
	EcrioCPMTrafficTypeSubscription,		/**< "subscription" Traffic-Type. */
	EcrioCPMTrafficTypePlugin				/**< "plugin" Traffic-Type. */
} EcrioCPMTrafficTypeEnum;

/** @enum EcrioCPMIMDispositionConfigEnum
* This enumeration describes the value of imdn.Disposition-Notification header that 
* is configured during outgoing message. The bitmask shall be used for multiple settings.
*/
typedef enum
{
	EcrioCPMIMDispositionConfigNone = 0x0000,				/**< No disposition-Notification type defined. */
    EcrioCPMIMDispositionConfigNegativeDelivery= 0x0001,	/**< "positive-delivery" disposition notification. */
    EcrioCPMIMDispositionConfigPositiveDelivery = 0x0002,	/**< "negative-delivery" disposition notification. */
	EcrioCPMIMDispositionConfigDisplay = 0x0004			/**< "display" disposition notification. */
} EcrioCPMIMDispositionConfigEnum;

/** @enum EcrioCPMIMDispositionNtfTypeReqEnum
* This enumeration describes the type of Disposition Notification requested
* by the message sender.
*/
typedef enum
{
	EcrioCPMIMDispositionNtfTypeReq_None,			/**< No disposition-Notification type defined. */
	EcrioCPMIMDispositionNtfTypeReq_Delivery,		/**< <delivery-notification> element. */
	EcrioCPMIMDispositionNtfTypeReq_Display 		/**< <display-notification> element. */
} EcrioCPMIMDispositionNtfTypeReqEnum;

/** @enum EcrioCPMIMDNDeliveryNotifEnum
* This enumeration describes the result of the disposition request for
* delivery-notification element.
*/
typedef enum
{
	EcrioCPMIMDNDeliveryNotif_None,					/**< No Delivery-Notification type defined. */
	EcrioCPMIMDNDeliveryNotif_Delivered,			/**< Delivery notification indicating "delivered". */
	EcrioCPMIMDNDeliveryNotif_Failed,				/**< Delivery notification indicating "failed". */
	EcrioCPMIMDNDeliveryNotif_Forbidden,			/**< Delivery notification indicating "forbidden". */
	EcrioCPMIMDNDeliveryNotif_Error					/**< Delivery notification indicating "error". */
} EcrioCPMIMDNDeliveryNotifEnum;

/** @enum EcrioCPMIMDNDisplayNotifEnum
* This enumeration describes the display notification for delivery-notification
* element.
*/
typedef enum
{
	EcrioCPMIMDNDisplayNotif_None,					/**< No Display-Notification type defined. */
	EcrioCPMIMDNDisplayNotif_Displayed,				/**< Display notification indicating "displayed". */
	EcrioCPMIMDNDisplayNotif_Forbidden,				/**< Display notification indicating "forbidden". */
	EcrioCPMIMDNDisplayNotif_Error					/**< Display notification indicating "error". */
} EcrioCPMIMDNDisplayNotifEnum;


/** \enum EcrioCPMComposingTypeEnum
* Defines various types of message composition.
*/
typedef enum
{
	EcrioCPMComposingType_None = -1,				/**< Invalid IM Typing type */
	EcrioCPMComposingType_Idle = 0,					/**< Typing type - Idle */
	EcrioCPMComposingType_Active = 1				/**< Typing type - Active */
}EcrioCPMComposingTypeEnum;

/** \enum EcrioCPMActionEnum
* The application will get this action enum in non-register response.
* The application will send register or reregister depends on this action.
*/
typedef enum
{
	EcrioCPMActionNone,								/**< No action taken */
	EcrioCPMActionSendInitialRegister,				/**< Initial Register is to be send */
	EcrioCPMActionSendReRegister					/**< ReRegister is to be send */
} EcrioCPMActionEnum;

/** @enum EcrioCPMGroupDataTypeEnum
 * This enumeration describes data type for the CPM Group Session Data Management.
 */
typedef enum
{
	EcrioCPMGroupDataType_None = 0,					/**< No data type defined. */
	EcrioCPMGroupDataType_Request,					/**< "request" action. */
	EcrioCPMGroupDataType_Response					/**< "response" action. */
} EcrioCPMGroupDataTypeEnum;

/** @enum EcrioCPMGroupDataMngTypeEnum
 * This enumeration describes data management type for the CPM Group Session
 * Data Management.
 */
typedef enum
{
	EcrioCPMGroupDataMngType_None = 0,				/**< No management type defined. */
	EcrioCPMGroupDataMngType_Subject,				/**< Change subject. */
	EcrioCPMGroupDataMngType_Icon,					/**< Change icon. */
	EcrioCPMGroupDataMngType_Role,					/**< Change User Role. */
	EcrioCPMGroupDataMngType_Multi					/**< Multi-data passed. */
} EcrioCPMGroupDataMngTypeEnum;	

/** @enum EcrioCPMGroupDataActionTypeEnum
 * This enumeration describes action type to management the CPM Group Session
 * Data Management.
 */
typedef enum
{
	EcrioCPMGroupDataActionType_None = 0,			/**< No action type defined. */
	EcrioCPMGroupDataActionType_Set,				/**< "set" action. */
	EcrioCPMGroupDataActionType_Delete,				/**< "delete" action. */
	EcrioCPMGroupDataActionType_Move				/**< "move" action. */
} EcrioCPMGroupDataActionTypeEnum;

/** @enum EcrioCPMGroupChatStatusEnum
* This enumeration describes the connection status to the group chat.
*/
typedef enum
{
	EcrioCPMGroupChatUserStatus_None = 0,			/**< No status defined. */
	EcrioCPMGroupChatUserStatus_Connected,			/**< User connected. */
	EcrioCPMGroupChatUserStatus_Disconnected,		/**< User disconnected. */
	EcrioCPMGroupChatUserStatus_Pending				/**< User pending. */
} EcrioCPMGroupChatStatusEnum;

/** @enum EcrioCPMGroupChatDisconnMethodEnum
* This enumeration describes the disconnected method.
*/
typedef enum
{
	EcrioCPMGroupChatDisconnMethod_None = 0,		/**< No disconnected method defined. */
	EcrioCPMGroupChatDisconnMethod_Departed,		/**< Endpoint sent BYE, thus leaving the chat. */
	EcrioCPMGroupChatDisconnMethod_Booted,			/**< The focus ejected participant out of the chat. */
	EcrioCPMGroupChatDisconnMethod_Failed			/**< Endpoint could not attend the chat. */
} EcrioCPMGroupChatDisconnMethodEnum;

/** @enum EcrioCPMGroupChatConferenceStateEnum
* This enumeration describes the conference document state to the group chat.
*/
typedef enum
{
	EcrioCPMGroupChatConferenceState_None = 0,		/**< No state defined. */
	EcrioCPMGroupChatConferenceState_Full,			/**< "full" state. */
	EcrioCPMGroupChatConferenceState_Partial		/**< "partial" state. */
} EcrioCPMGroupChatConferenceStateEnum;

/** @enum EcrioCPMPrivMngCmdActionEnum
* This enumeration describes the action type of Privacy Management Command.
*/
typedef enum
{
	EcrioCPMPrivMngCmdAction_None = 0,				/**< No privacy management command action */
	EcrioCPMPrivMngCmdAction_Fetch,					/**< Action - fetchsettings */
	EcrioCPMPrivMngCmdAction_On,					/**< Action - aliason */
	EcrioCPMPrivMngCmdAction_Off,					/**< Action - aliasoff */
	EcrioCPMPrivMngCmdAction_Link					/**< Action - aliaslink */
} EcrioCPMPrivMngCmdActionEnum;

/** @enum EcrioCPMPrivMngCmdRespEnum
* This enumeration describes the result of Privacy Management Command response.
*/
typedef enum
{
	EcrioCPMPrivMngCmdResp_None = 0,				/**< No privacy management command response */
	EcrioCPMPrivMngCmdResp_On,						/**< Result - on */
	EcrioCPMPrivMngCmdResp_Off,						/**< Result - off */
	EcrioCPMPrivMngCmdResp_Failure,					/**< Result - failure */
} EcrioCPMPrivMngCmdRespEnum;

/** @enum EcrioCPMTkParamEnum
* This enumeration describes the tk parameter that indicates whether or not
* aliasing is used in the conversation.
*/
typedef enum
{
	EcrioCPMTkParam_None = 0,						/**< No tk param is indicated. */
	EcrioCPMTkParam_On,								/**< tk param on */
	EcrioCPMTkParam_Off								/**< tk param off */
} EcrioCPMTkParamEnum;

/** @enum EcrioCPMTkParamEnum
* This enumeration describes the tk and tklink parameter that indicates to be
* added to Privacy header when requesting privacy in session with Chatbot.
*/
typedef enum
{
	EcrioCPMChatbotPrivacy_None = 0,				/**< No privacy request for Chatbot session. */
	EcrioCPMChatbotPrivacy_Request,					/**< Request privacy. */
	EcrioCPMChatbotPrivacy_Link						/**< To link the user's token to their actual identity when initiating a session. */
} EcrioCPMChatbotPrivacyEnum;

/** @struct EcrioCPMBufferStruct
* This structure defines to hold a buffer
*/
typedef struct
{
	u_char *pMessage;									/**< The binary message. */
	u_int32 uMessageLen;								/**< The length of the message. */
	u_int32 uByteRangeStart;							/**< Starting range of the data. */
	u_int32 uByteRangeEnd;								/**< Ending range of the data. */
	u_char *pDate;										/**< Date from CPIM header. */
	u_char *pContentType;								/**< Content Type */
} EcrioCPMBufferStruct;

/** @struct EcrioCPMConversationsIdStruct
* This structure defines to hold CPM Conversation Identification headers.
*/
typedef struct
{
	u_char *pConversationId;					/**< Conversation-ID. */
	u_char *pContributionId;					/**< Contribution-ID. */
	u_char *pInReplyToContId;					/**< InReplyTo-Contribution-ID. */
} EcrioCPMConversationsIdStruct;

/** @struct EcrioCPMFileDescriptorStruct
* This structure holds the parameters need to start file transfer session.
* This is valid only if contentTypes is EcrioCPMContentTypeEnum_FileTransferOverMSRP
* i.e., File Trasfer over MSRP.
*/
typedef struct
{
    char *pFilePath;                                    /**< Null terminated string holding path to the file. */
	char *pFileName;									/**< Null terminated string holding name of incoming file. */
	char *pFileType;									/**< Null terminated string holding the standardized MIME type of the file. */
	u_int32 uFileSize;									/**< Total size of the file that be shared with pTarget. */
	char *pFileTransferId;								/**< Null terminated string holding file transfer id. */
	char *pIMDNMsgId;									/**< The imdn.Message-ID value. */
	u_int32 imdnConfig;									/**< The IMDN request configuration, The EcrioCPMIMDispositionConfigEnum with bitmask. */
	BoolEnum bIsCPIMWrapped;							/**< Wrapped file data with CPIM message if Enum_TRUE else send file data in chunks. */
	void *pAppData;										/**< Application data. */
} EcrioCPMFileDescriptorStruct;

/** \struct EcrioCPMBotSuggestionStruct
* \brief This structure holds the contains of suggested chip list or suggestion response.
* It holds the json body only.
*/
typedef struct
{
	BoolEnum bIsBotSuggestionResponse;				/**< Is it gsma.botsuggestion.response.v1.0+json or vnd.gsma.botsuggestion.v1.0+json. */
	u_char *pJson;									/**< The JSON data. */
	u_int32 uJsonLen;								/**< The length of the JSON data. */
} EcrioCPMBotSuggestionStruct;

/** @struct EcrioCPMGroupChatDescStruct
 * This structure holds the description of existing group chat.
 */
typedef struct
{
	char *pSubject;									/**< Subject to be used for group chat. */
	char *pSubCngUser;								/**< Who is changed the subject. */
	char *pSubCngDate;								/**< When is changed the subject. */

	char *pIconUri;									/**< Indicated to the icon URI. */
	char *pIconInfo;								/**< Icon information. */
	char *pIconCngUser;								/**< Who is changed the icon. */
	char *pIconCngDate;								/**< When is changed the icon. */
} EcrioCPMGroupChatDescStruct;

/** @struct EcrioCPMGroupChatUsersStruct
 * This structure holds the participant information of existing group chat.
 */
typedef struct
{
	char *pUri;										/**< Participant's URI. */
	char *pDisplayText;								/**< Display text. */
	BoolEnum isHost;								/**< Is this host. */
	EcrioCPMGroupChatStatusEnum status;				/**< User status for group chat. */
	EcrioCPMGroupChatDisconnMethodEnum disMethod;	/**< Disconnection method when group chat status is disconnected. */
} EcrioCPMGroupChatUsersStruct;

/** @struct EcrioCPMHostInfoStruct
* This structure holds the URI list of group chat host.
*/
typedef struct
{
	char *pDisplayText;								/**< Pointer to the Display Text. */
	char *pWebPage;									/**< Pointer to the Web Page Info. */
	u_int32 uNumOfUsers;							/**< Number of participants. */
	char **ppUri;									/**< Pointer to the URI lists. */
} EcrioCPMHostInfoStruct;

/** @struct EcrioCPMIconInfoStruct
* This structure holds the icon information that receives with Group Chat Info.
*/
typedef struct
{
	u_char *pContentType;							/**< Pointer to the Content Type. */
	u_char *pContentId;								/**< Pointer to the Content Id. */
	u_char *pContentDisposition;					/**< Pointer to the Content Disposition. */
	u_char *pContentTransferEncoding;				/**< Pointer to the Content Transfer Encoding. */
	u_int32 uDataLength;							/**< Pointer to the binary data length. */
	u_char *pData;									/**< Pointer to the binary data. */
} EcrioCPMIconInfoStruct;

/** @struct EcrioCPMGroupChatInfoStruct
 * This structure holds the group chat information.
 */
typedef struct
{
	char *pSessionId;								/**< CPM session identifier. */
	EcrioCPMGroupChatConferenceStateEnum eState;	/**< Enumeration that indicates conference document state notifying this information. */
	EcrioCPMGroupChatDescStruct desc;				/**< Pointer to the descripion of the group chat. */
	u_int32 uNumOfUsers;							/**< Number of participants. */
	EcrioCPMGroupChatUsersStruct **ppUsers;			/**< Pointer to the participant users list. */
	EcrioCPMHostInfoStruct hostInfo;				/**< Pointer to the Host info Struct. */
	EcrioCPMIconInfoStruct *pIconInfo;			    /**< The icon information that receives with Group Chat Info. */
} EcrioCPMGroupChatInfoStruct;

/** @struct EcrioCPMParticipantListStruct
 * This structure holds the URI list of group chat participants.
 */
typedef struct
{
	u_int32 uNumOfUsers;							/**< Number of participants. */
	char **ppUri;									/**< Pointer to the URI lists. */
} EcrioCPMParticipantListStruct;

/** @struct EcrioCPMAliasingParamStruct
 * This structure holds the aliasing parameters used by a Chatbot to communicate.
 */
typedef struct
{
	EcrioCPMTkParamEnum eTkParam;					/**< Enumeration of tk param. */
	BoolEnum bIsAliason;							/**< Enable this flag to exist aliason param. */
} EcrioCPMAliasingParamStruct;

/** \struct EcrioCPMIncomingSessionStruct
 * \brief This structure provides the incoming CPM session details. When the
 * eNotifCmd is lims_Notify_Type_SESSION_INCOMING then pNotifData contains
 * EcrioCPMIncomingSessionStruct.
 */
typedef struct
{
	char *pReqFrom;										/**< User Identity of the remote end. */
	char *pDisplayName;									/**< Display Name received which can also be an alias name. */
	char *pSessionId;									/**< Incoming CPM session identifier. */
	u_int32 contentTypes;								/**< CPM Content types, this is the EcrioCPMContentTypeEnum with bitmask. */
	EcrioCPMConversationsIdStruct *pConvId;				/**< The conversation Id, pointer to EcrioCPMConversationsIdStruct. */
	EcrioCPMFileDescriptorStruct *pFile;				/**< The file descriptor, pointer to EcrioCPMFileDescriptorStruct. */
	BoolEnum bIsGroupChat;								/**< Enable this flag to indicate group chat session. */
	BoolEnum bIsClosed;									/**< Enable this flag to indicate closed group chat session. */
	BoolEnum bIsChatbot;								/**< Enable this flag to indicate 1-to-1 chatbot session. */
	EcrioCPMAliasingParamStruct *pAlias;				/**< Aliasing parameters, this pointer exists in the case of 1-to-1 chatbot session. */
	char *pGroupSessionId;								/**< CPM Group Session identifier if this session is group chat. */
	char *pGroupSubject;								/**< CPM Group Session's subject received in "Subject" header of INVITE. */
	EcrioCPMParticipantListStruct *pList;				/**< List of participants for group chat. */
} EcrioCPMIncomingSessionStruct;

/** \struct EcrioCPMAcceptedSessionStruct
 * \brief This structure provides the accepted CPM session details. When the
 * eNotifCmd is lims_Notify_Type_SESSION_ESTABLISHED, lims_Notify_Type_SESSION_RINGING 
 * or lims_Notify_Type_SESSION_PROGRESS then pNotifData contains EcrioCPMAcceptedSessionStruct.
 */
typedef struct
{
	char *pDisplayName;									/**< Display Name received which can also be an alias name. */
	u_int32 contentTypes;								/**< CPM Content types, this is the EcrioCPMContentTypeEnum with bitmask. */
	char *pSessionId;									/**< CPM session identifier. */
	EcrioCPMConversationsIdStruct *pConvId;				/**< The conversation Id, pointer to EcrioCPMConversationsIdStruct. */
	char *pGroupSessionId;								/**< CPM Group Session identifier if this session is group chat. */
	BoolEnum bIsClosedGroupChat;						/**< A flag to check closed group chat. */
	EcrioCPMAliasingParamStruct *pAlias;				/**< Aliasing parameters, this pointer exists in case of 1-to-1 chatbot session. */
	BoolEnum bIsChatbotRole;							/**< A flag to check the Chatbot role. */
} EcrioCPMAcceptedSessionStruct;

/** \struct EcrioCPMFailedSessionStruct
* \brief This structure provides the CPM session failure details. When the
* eNotifCmd is lims_Notify_Type_SESSION_CONNECT_FAILED then pNotifData contains 
* EcrioCPMFailedSessionStruct.
*/
typedef struct
{
	char *pSessionId;									/**< CPM session identifier. */
	u_int32 uResponseCode;								/**< Response code. */
	EcrioCPMActionEnum eAction;							/**< What action is to be performed for that response code. */
	u_int32 uWarningCode;								/**< Warning code. */
	char *pWarningHostName;								/**< Warning host name. */
	char *pWarningText;									/**< Warning text. */
	char** ppPAssertedIdentity;							/**< The P-Asserted-Identity URIs. */
	u_int8 uNumOfPAssertedIdentity;					    /**< The number of P-Asserted-Identity URIs. */
} EcrioCPMFailedSessionStruct; 

/** \struct EcrioCPMReasonStruct
* \brief This structure provides the cause of the session termination indicated
* by SIP Reason header. When the eNotifCmd is lims_Notify_Type_SESSION_ENDED then 
* pNotifData contains EcrioCPMReasonStruct.
*/
typedef struct
{
	char *pSessionId;									/**< CPM session identifier. */
	u_int32 uResponseCode;								/**< Response code. */
	u_int32 uReasonCause;								/**< Reason cause. */
	u_char *pReasonText;								/**< Reason text. */
} EcrioCPMReasonStruct; 

/** \struct EcrioCPMAddRemoveParticipantResponseStruct
* \brief This structure provides the status of the REFER request that was triggered
* to add or remove participant to group chat. When the eNotifCmd is 
* lims_Notify_Type_UPDATE_PARTICIPANT_SUCCESS and lims_Notify_Type_UPDATE_PARTICIPANT_FAILED then
* pNotifData contains EcrioCPMAddRemoveParticipantResponseStruct.
*/
typedef struct
{
	char *pSessionId;									/**< CPM Group session identifier. */
	char *pReferId;										/**< ID to idenify and map refer request. */
	u_int32 uReasonCause;								/**< Reason cause. */
	u_char *pReasonText;								/**< Reason text. */
} EcrioCPMAddRemoveParticipantResponseStruct;

/** \struct EcrioCPMSessionIDStruct
* \brief This structure provides the status code and reason for the particular message response.
* When the eNotifCmd is message/file send success/failure then pNotifData
* contains EcrioCPMSessionIDStruct.
*/
typedef struct
{
	char *pSessionId;									/**< Session Identifier. */
	char *pMessageId;									/**< Message Identifier. */
	char *pTransactionId;								/**< Transaction Identifier. */
	u_int32 uStatusCode;								/**< Status code of MSRP reponse. */
	u_char *pDescription;								/**< The description string of MSRP reponse. */
} EcrioCPMSessionIDStruct;

/** \struct EcrioCPMStandAloneMessageIDStruct
* \brief This structure provides the status code and reason for the particular stand alone message response.
*/
typedef struct
{
	u_int32	statusCode;									/**< Response Code - to identify type of failure response */
	u_char *pszReason;									/**< Reason Phrase - description of failure */
	char   *pCallId;                                    /**< Call Id - pointer to unique call Id */
	u_int32 uWarningCode;								/**< Warning code. */
	char *pWarningHostName;								/**< Warning host name. */
	char *pWarningText;									/**< Warning text. */
	char** ppPAssertedIdentity;							/**< The P-Asserted-Identity URIs. */
	u_int8 uNumOfPAssertedIdentity;					    /**< The number of P-Asserted-Identity URIs. */
} EcrioCPMStandAloneMessageIDStruct;

/** @struct EcrioCPMIMDispoNotifBodyStruct
* This structure is used to compose the Disposition Notification Multiple Body. 
*
* The Disposition-Notification-Body can describe multiple body in one message,
*/
typedef struct
{		
	EcrioCPMIMDispositionNtfTypeReqEnum eDisNtf;	/**< Enumeration that indicates the "Disposition-Notification" element. */
	EcrioCPMIMDNDeliveryNotifEnum eDelivery;		/**< Enumeration that indicates the result of delivery request. */
	EcrioCPMIMDNDisplayNotifEnum eDisplay;			/**< Enumeration that indicates the display notification. */
	u_char *pDate;									/**< Date from CPIM header. */
	char *pIMDNMsgId;								/**< The imdn.Message-ID value. */
	char *pRecipientUri;							/**  <recipient-uri value. */
	char *pOriginalRecipientUri;					/**  < original-recipient-uri value */
} EcrioCPMIMDispoNotifBodyStruct;

/** @struct EcrioCPMIMDispoNotifStruct
* This structure is used to compose the Disposition Notification. This will be
* used to build disposition-notification for sending and receiving disposition-
* notification when received it.
*
* The Disposition-Notification can describe multiple elements in one message,
* to set multiple notifications, specify them to eDispoNotif with bitmask. When
* any values are contained, each value will set by valid number.
*/
typedef struct
{
	char *pDisplayName;								/**< Sender's DisplayName*/
	char *pSenderURI;								/**< Sender URI. */
	char *pDestURI;									/**< Destination URI. */
	u_int32 uNumOfNotifBody;
	EcrioCPMIMDispoNotifBodyStruct *pCPMIMDispoNotifBody;
} EcrioCPMIMDispoNotifStruct;

/** \struct EcrioCPMSessionComposingStruct
*  \brief This structure provides the information of incoming composing details along with the URI of
* the sender. When the eNotifCmd is lims_Notify_Type_COMPOSING then pNotifData
* contains EcrioCPMSessionComposingStruct.
*/
typedef struct
{
	u_char *pSenderURI;								/**< Sender URI. */
	char *pSessionID;								/**< Session Identifier. */
	u_int32	uRefresh;								/**< active refresh value */
	EcrioCPMComposingTypeEnum	eComposingState; 	/**< Composing type if this message is IM composition */
} EcrioCPMSessionComposingStruct;

/** \struct EcrioCPMServicesStruct
* \brief This structure provides the information of RCS capability of
* the remote user. When the eNotifCmd is lims_Notify_Type_CAPABILTY_DISCOVERY_SUCCESS or
* lims_Notify_Type_CAPABILTY_QUERY_RECEIVED then pNotifData contains EcrioCPMServicesStruct.
*/
typedef struct
{
	u_int32 uServices;					/**< RCS services, lims features with bitmasks. */
} EcrioCPMServicesStruct;

/** @struct EcrioCPMComposingStruct
* This structure is used to compose the "isComposing" notification.
*/
typedef struct
{
	char *pDisplayName;							/**< Sender's DisplayName*/
	EcrioCPMComposingTypeEnum eComposingState;	/**< Enumeration that indicates the state of Composing notification. */
	u_int32 uRefresh;							/**< Refresh interval for using a refresh element. Valid when a value is not 0. */
	u_char *pContent;							/**< Pointer to the Content-Type string. Valid when this pointer is set. */
	u_char *pLastActive;						/**< Pointer to the time of last activity string. The syntax of date-time should be a profile of ISO8601 style (yyyyMMdd'T'HHmmss.SSSZ). Valid when this pointer is set. */
	char *pDestURI;								/**< Destination URI indicates who is for disposition notification within group chat. */
} EcrioCPMComposingStruct;

/** \struct EcrioCPMMessageStruct
* \brief This structure is a common structure to hold all the details of the message sent and received. 
* It hold the IMDN message ID, a common buffer structure to hold text/filetransfer/richcard details, structure for Composing
* structure for IM Disposition notification, structure for BotSuggestion and chiplist and maap Traffic Type.
*/
typedef struct
{
	char *pDestURI;									/**< Destination URI indicates who is for disposition notification within group chat. */
	char *pIMDNMsgId;								/**< The imdn.Message-ID value. */
	u_int32 imdnConfig;								/**< The IMDN request configuration, The EcrioCPMIMDNDispositionConfigEnum with bitmask. */
	EcrioCPMContentTypeEnum eContentType;			/**< This enum defines the type for message; if it is 
													EcrioCPMContentTypeEnum_Composing then EcrioCPMIsComposingStruct need to be used; if it is
													EcrioCPMContentTypeEnum_IMDN then EcrioCPMIMDispoNotifStruct need to be used and for the
													rest of the enums EcrioCPMBufferStruct need to be used. */
	union
	{
		EcrioCPMBufferStruct *pBuffer;				/**< EcrioCPMBufferStruct pointer to send text/richcard/filetransfer message. */
		EcrioCPMComposingStruct *pComposing;		/**< Pointer to EcrioCPMComposingStruct structure. */
		EcrioCPMIMDispoNotifStruct *pIMDN;			/**< Pointer to EcrioCPMIMDispoNotifStruct structure. */
	} message;
	EcrioCPMBotSuggestionStruct *pBotSuggestion;	/**< Pointer to EcrioCPMBotSuggestionStruct structure. */
	EcrioCPMTrafficTypeEnum eTrafficType;			/**< Enum to hold the Maap Traffic type. */
} EcrioCPMMessageStruct;

/** \struct EcrioCPMIncomingStandAloneMessageStruct
* \brief This structure provides the incoming StandAlone mode message details along with the URI of
* the sender. When the eNotifCmd is lims_Notify_Type_STAND_ALONE_MESSAGE_RECEIVED then pNotifData
* contains EcrioCPMIncomingStandAloneMessageStruct.
*/
typedef struct
{
	char *pDisplayName;								/**< Display Name received which can also be an alias name. */
	char *pReqFrom;									/**< User Identity of the remote end. */
	EcrioCPMConversationsIdStruct* pConvId;			/**< The CPM Conversation Identification headers, pointer to EcrioCPMConversationsIdStruct. */
	EcrioCPMMessageStruct* pMessage;				/**< Point to EcrioCPMMessageStruct structure. */
	BoolEnum bIsChatbot;							/**< A flag to indicate whether this message from Chatbot or not. */
} EcrioCPMIncomingStandAloneMessageStruct;

/** \struct EcrioCPMSessionMessageStruct
* \brief This structure provides the incoming session mode message details along with the URI of
* the sender. When the eNotifCmd is lims_Notify_Type_MESSAGE_RECEIVED then pNotifData
* contains EcrioCPMSessionMessageStruct.
*/
typedef struct
{
	char *pDisplayName;									/**< Display Name received which can also be an alias name. */
	char *pSenderURI;									/**< Sender URI. */
	char *pSessionId;									/**< Session Identifier. */
	char *pMessageId;									/**< Message Identifier. */
	char *pTransactionId;								/**< Transaction Identifier. */
	EcrioCPMMessageStruct* pMessage;					/**< Point to EcrioCPMMessageStruct structure. */
} EcrioCPMSessionMessageStruct;

/** \struct EcrioCPMGroupChatIconStruct
* \brief This structure provides the group chat icon details along with the URI of
* the sender. When the eNotifCmd is EcrioCPM_Notif_GroupChatIcon then pNotifData
* contains EcrioCPMGroupChatIconStruct.
*/
typedef struct
{
	char* pSenderURI;									/**< Sender URI. */
	char* pSessionId;									/**< Session Identifier. */
	char* pFileInfo;									/**< The content-id of the image file. */
	u_char* pData;										/**< The binary data of the icon. */
	u_int32 uDataLength;								/**< The length of the data. */
	char* pContentType;									/**< Content Type of the image file*/
	char* pContentTypeTransferEncoding;					/**< Content Transfer Encoding*/
} EcrioCPMGroupChatIconStruct;


/** @struct EcrioCPMGroupDataSubjectStruct
 * This structure is used to compose the subject data type for management the
 * CPM Group Session Data Management.
 */
typedef struct
{
	EcrioCPMGroupDataActionTypeEnum type;				/**< Action type. */
	u_char *pText;										/**< Subject text. */
} EcrioCPMGroupDataSubjectStruct;

/** @struct EcrioCPMGroupDataIconStruct
 * This structure is used to compose the icon data type for management the CPM
 * Group Session Data Management.
 */
typedef struct
{
	EcrioCPMGroupDataActionTypeEnum type;				/**< Action type. */
	u_char *pUri;										/**< Icon URI. */
	u_char *pFileInfo;									/**< File-info. */
} EcrioCPMGroupDataIconStruct;

/** @struct EcrioCPMGroupDataRoleStruct
 * This structure is used to compose the role data type for management the CPM
 * Group Session Data Management.
 */
typedef struct
{
	EcrioCPMGroupDataActionTypeEnum type;				/**< Action type. */
	u_char *pUserRole;									/**< User Role. */
	u_char *pHostUri;										/**< Host URI. */
} EcrioCPMGroupDataRoleStruct;

/** @struct EcrioCPMGroupDataRequestStruct
 * This structure is used to compose the request data structure of the CPM Group
 * Session Data Management.
 */
typedef struct
{
	char *pSessionId;									/**< CPM session identifier. */
	EcrioCPMGroupDataMngTypeEnum type;					/**< Data management type. */
	EcrioCPMGroupDataSubjectStruct subject;				/**< Subject. */
	EcrioCPMGroupDataIconStruct icon;					/**< Icon. */
	EcrioCPMGroupDataRoleStruct role;					/**< Role. */
} EcrioCPMGroupDataRequestStruct;

/** @struct EcrioCPMGroupDataResponseStruct
 * This structure is used to compose the response data structure of the CPM Group
 * Session Data Management.
 */
typedef struct
{
	u_int32 uCode;										/**< Response code. */
	u_char *pText;										/**< Response text. */
} EcrioCPMGroupDataResponseStruct;

/** @struct EcrioCPMGroupDataStruct
 * This structure is used to compose the data structure of the CPM Group Session
 * Data Management. In this structure, only one of request and response is stored.
 */
typedef struct
{
	EcrioCPMGroupDataTypeEnum type;						/**< Group session data type. */
	EcrioCPMGroupDataRequestStruct req;					/**< Request for group data. */
	EcrioCPMGroupDataResponseStruct rsp;				/**< Response for group data. */
} EcrioCPMGroupDataStruct;

/** @struct EcrioCPMPrivMngCmdRespStruct
 * This structure is used to compose the response data structure of the result of
 * Private Management Command response.
 */
typedef struct
{
	char *pCommandID;									/**< Command-ID. */
	EcrioCPMPrivMngCmdRespEnum eResp;					/**< Enumeration for result of privacy management commands. */
} EcrioCPMPrivMngCmdRespStruct;

#endif /* #ifndef __ECRIOCPMDATATYPES_H__ */
