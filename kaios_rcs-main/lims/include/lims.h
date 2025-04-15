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
* @file lims.h
* @brief This is the header file for the public interface of the Light IMS (lims) module.
*
* The lims module acts as a layer between a Application and lims framework, providing the
* light IMS functionality such as registration, deregistration, NOTIFY, send/receive SMS 
* messages and light volte functionalities such as initiate and terminate 1-1 voice
* call for small foot print IOT devices.
*/

/*! \mainpage Introduction
 *
 * This is Light IMS API specification document. The lims acts as a layer between an Application and the lims Framework, 
 * providing the light IMS functionalities such as registration, deregistration, NOTIFY, send/receive SMS 
 * messages and light volte functionalities such as initiate and terminate 1-1 voice call, call hold/resume
 * and GSMA UP2.0 RCS functionalities such as Standalone Messaging, 1-1 Chat, Group Chat, FT over HTTP, 
 * Rich Cards and Suggestion for small foot print IOT devices.
 *
 * The purpose of this document is to outline the API interfaces and describe the features available for the application developer. The API and internal 
 * functions of lims are written in ANSI C. The platform specific functions that are required to be integrated with this lims are 
 * isolated with well-defined interfaces and documentation.
 *
 * The rest of the document explain all the APIs defined in lims with input parameters, output parameters and brief description of the functions 
 * themselves. Also it provides the details of data structures, enumerations and other definitions defined in lims.
 *
 */

#ifndef __LIMS_H__
#define __LIMS_H__

/* Include data types header file */
#include "EcrioPAL.h"
#include "EcrioSIPDataTypes.h"
#include "EcrioCPMDataTypes.h"
#include "EcrioUCEDataTypes.h"
#include "EcrioSUEDataTypes.h"
#include "EcrioOOM.h"
#include "EcrioIOTA.h"

/* Allow this module to be called directly from C++ compiled code. */
#ifdef __cplusplus
extern "C" {
#endif

/** @name Module Handle
 * \n Handle is void pointer which will point to structures known only internally to the Module.
 * The Calling Component should only reference the returned handle by these defined type. The handle
 * supported by this module include:
 */
/*@{*/

/**
 * The lims handle refers to the overall Light IMS Module instance that is created after calling lims_Init()
 * function.
 */
typedef void *LIMSHANDLE;

/*@}*/

/** @name Status Codes
 * \n Each lims module API will return a staus code indicating success or failure.
 * The lims implementation will indicate a more detailed code when a failure is
 * found.
 */
/*@{*/

#define LIMS_NO_ERROR					(0x00)		/**< Implies that the lims API function call was successful. */
#define LIMS_INVALID_OPERATION_ERROR	(0x01)		/**< Implies API invoked at wrong state or event. */
#define LIMS_SOCKET_ERROR				(0x02)		/**< Implies that API received socket related error while processing. */
#define LIMS_INVALID_DATA_ERROR			(0x03)		/**< Implies passed argument is NULL. */
/*@}*/

#ifdef ENABLE_QCMAPI
/** @enum lims_ACRTypeEnum
 * Enumeration holding the information of Autoconfiguration trigger reason types.
 */
typedef enum
{
	lims_ACR_UserRequest = 0,
	lims_ACR_RefreshToken,
	lims_ACR_InvalidToken,
	lims_ACR_InvalidCredential,
	lims_ACR_ClientChange,
	lims_ACR_DeviceUpgrde,
	lims_ACR_FactoryReset
} lims_ACRTypeEnum;
#endif
/** @enum lims_NetworkPDNTypeEnum
 * This enumerations defines the network connected PDN type.
 */
typedef enum
{
	lims_Network_PDN_Type_DEFAULT,					/**< Connected to default PDN. It may be Internet PDN or some other PDN which is configured as default.*/
	lims_Network_PDN_Type_IMS						/**< Connected PDN Type is IMS. */
} lims_NetworkPDNTypeEnum;

/** @enum lims_NetworkConnectionTypeEnum
 * This enumerations defines the current network connection type.
 */
typedef enum
{
	lims_Network_Connection_Type_NONE,						/**< Indicates there is no network connection with the device. */
	lims_Network_Connection_Type_LTE,						/**< Indicates the current connected bearer type is LTE. */
	lims_Network_Connection_Type_WIFI						/**< Indicates the current connection bearer type is WIFI.*/
} lims_NetworkConnectionTypeEnum;

/** @enum lims_NetworkConnectionStatus
* This enumeration defines the network connection status.
*/

typedef enum
{
	lims_Network_Status_Success,							/**< Represents Default bearer connected and VoLTE supported. */
	lims_Network_Status_VoLTE_Off,							/**< Represents Default bearer connected but network doesn't support VoLTE feature. */
	lims_Network_Status_IPsec_Established,					/**< Represents IPsec SA established. */
	lims_Network_Status_IPsec_Failed,						/**< Failed to establish IPsec SA. */
	lims_Netwotk_Status_IPsec_Disconneted,					/**< Represents IPsec SA successfully disconnected. */
	lims_Network_Status_IPsec_Lost,							/**< Represents IPsec SA connection lost*/
	lims_Network_Status_Dedicated_Bearer_Connected,			/**< Represents Dedicated Bearer setup complete by network*/
	lims_Network_Status_Dedicated_Bearer_Disconnected,		/**< Represents Dedicated Bearer disconnected by network*/
	lims_Network_Status_Dedicated_Bearer_Failed				/**< Represents Failure while setting up Dedicated Bearer by network*/
} lims_NetworkConnectionStatus;

/** @enum lims_IPTypeEnum
 * This enumeration defines the different IP protocol types, IPv4 and IPv6.
 */
typedef enum
{
	lims_Network_IP_Type_V4 = 0,	/**< IPV4 address type. Default value */
	lims_Network_IP_Type_V6			/**< IPV6 address type */
} lims_IPTypeEnum;

#ifndef ENABLE_QCMAPI
/** @enum lims_FeatureEnums
 * This enumeration defines the feature identifier. The lims adds feature tag for corresponding
 * feature in REGISTER request. The multiple features can be added using bit mask mechanism.
 */
typedef enum
{
	lims_Feature_IMS_Sms					= 0x0001,			/**< SMS over IMS feature supported and it adds the +g.3gpp.smsip feature tag in contact. */
	lims_Feature_IMS_VoLTE					= 0x0002,			/**< VoLTE feature supported and it adds the urn:urn-7:3gpp-service.ims.icsi.mmtel feature tag in contact. */
	lims_Feature_CPM_PagerMode				= 0x0004,			/**< CPM standalone pager mode message supported and it adds the +g.3gpp.icsi-ref="urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.msg" feature tag in contact. */
	lims_Feature_CPM_LargeMode				= 0x0008,			/**< CPM standalone larger message mode supported and it adds the +g.3gpp.icsi-ref="urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.largemsg" feature tag in contact. */
	lims_Feature_CPM_Chat         			= 0x0010,			/**< CPM Chat supported and it adds the +g.3gpp.iari-ref="urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.im" and 
																	 +g.3gpp.icsi-ref="urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session" feature tag in contact*/
	lims_Feature_CPM_FT_MSRP				= 0x0020,			/**< File transfer over MSRP supported and it adds the +g.3gpp.icsi-ref="urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.filetransfer"*/
	lims_Feature_CPM_FT_Http				= 0x0040,			/**< CPM File transfer over HTTP supported and it adds the +g.3gpp.iari-ref="urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fthttp*/
	lims_Feature_CPM_Chatbot				= 0x0080,			/**< CPM Chatbot Communication supported and it adds the +g.3gpp.iari-ref="urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot";+g.gsma.rcs.botversion="#1" and +g.gsma.rcs.isbot */
	lims_Feature_CPM_IMDN_Aggregation		= 0x0100,			/**< CPM it adds the "imdn-aggregation" feature tag in contact*/
	lims_Feature_CPM_GeoLocation     		= 0x0200			/**< CPM Push Geo-Location*/
} lims_FeatureEnums;
#else
typedef enum
{
	lims_Feature_IMS_Sms					= 0x0001,			/**< SMS over IMS feature supported and it adds the +g.3gpp.smsip feature tag in contact. */
	lims_Feature_IMS_VoLTE					= 0x0002,			/**< VoLTE feature supported and it adds the urn:urn-7:3gpp-service.ims.icsi.mmtel feature tag in contact. */
	lims_Feature_CPM_PagerMode				= 0x0004,			/**< CPM standalone pager mode message supported and it adds the +g.3gpp.icsi-ref="urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.msg" feature tag in contact. */
	lims_Feature_CPM_LargeMode				= 0x0008,			/**< CPM standalone larger message mode supported and it adds the +g.3gpp.icsi-ref="urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.largemsg" feature tag in contact. */
	lims_Feature_CPM_Chat         			= 0x0010,			/**< CPM Chat supported and it adds the +g.3gpp.icsi-ref="urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session" feature tag in contact*/
	lims_Feature_CPM_FT_MSRP				= 0x0020,			/**< File transfer over MSRP supported and it adds the +g.3gpp.icsi-ref="urn%3Aurn-7%3A3gppservice.ims.icsi.oma.cpm.filetransfer"*/
	lims_Feature_CPM_FT_Http				= 0x0040,			/**< CPM File transfer over HTTP supported and it adds the +g.3gpp.iari-ref="urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fthttp*/
	lims_Feature_CPM_Chatbot				= 0x0080,			/**< CPM Chatbot Communication supported and it adds the +g.3gpp.iari-ref="urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot";+g.gsma.rcs.botversion="#1" and +g.gsma.rcs.isbot */
	lims_Feature_CPM_Chat_Im				= 0x0100,			/**< CPM Chat supported and it adds the +g.3gpp.iari-ref="urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.im" feature tag in contact*/
	lims_Feature_CPM_FT_SF					= 0x0200,			/**< CPM adds the +g.3gpp.icsi-ref="urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftstandfw" feature tag */
	lims_Feature_CPM_DEFERRED				= 0x0400			/**< CPM adds the +g.3gpp.icsi-ref="urn%3Aurn-7%3A3gppservice.ims.icsi.oma.cpm.deferred"  feature tag */
} lims_FeatureEnums;	
#endif

/** @enum lims_NotifyTypeEnums
 * This enumeration defines the types of notification that the lims module implementation update
 * to the application layer.
 */
typedef enum
{
	lims_Notify_Type_NONE = 0,							/**< Ignore this type. */
	lims_Notify_Type_REGISTERED,						/**< Registration success notification. */
	lims_Notify_Type_REGISTER_FAILED,					/**< Registration failed notification. */
	lims_Notify_Type_DEREGISTERED,						/**< De-registration success notification. */
	lims_Notify_Type_NOTIFY_RECEIVED,					/**< Notify is received from the network. */
	lims_Notify_Type_SUBSCRIBED,						/**< Subscription to reg-events success notification. */
	lims_Notify_Type_SUBSCRIBE_FAILED,					/**< Subscription to reg-events failed notification. */
	lims_Notify_Type_UNSUBSCRIBED,						/**< Unsubscribe to reg-events success notification. */
	lims_Notify_Type_SMS_SENT,							/**< Send SMS message success notification. */
	lims_Notify_Type_SMS_FAILED,						/**< Send SMS message failed notification. pData in the callback function contains the details of failure. */
	lims_Notify_Type_SMS_RECEIVED,						/**< Incoming SMS message notification. pData will be typecast to lims_MessageStruct to access the details. */
	lims_Notify_Type_CALL_INCOMING,						/**< Incoming call notification. pData will be typecast to lims_CallStruct to access the details. */
	lims_Notify_Type_CALL_INCOMING_WITH_PRECONDITION,	/**< Incoming call notification with request for precondition. pData will be typecast to lims_CallStruct to access the details. */
	lims_Notify_Type_CALL_RINGING,						/**< Ringing notification update to initiator if 180 Ringing response received from the network. Caller plays the ringing tone after receiving this. */
	lims_Notify_Type_CALL_IS_BEING_FORWARDED,			/**< Forwarded notification update to initiator if 181 Call Is Being Forwarded response received from the network. */
	lims_Notify_Type_CALL_PROGRESS,						/**< Progressing notification update to initiator if 183 Session In Progress response received from the network. */
	lims_Notify_Type_CALL_ESTABLISHED,					/**< Call established notification. */
	lims_Notify_Type_CALL_FAILED,						/**< All kind of failure response from far end. */
	lims_Notify_Type_CALL_ENDED,						/**< Ended notification updated to the callee or caller if BYE request initiated/received to/from the network. */
	lims_Notify_Type_CALL_ONHOLD,						/**< Call put OnHold notification to the application. */
	lims_Notify_Type_CALL_HOLD_FAILED,					/**< Call Hold Failed notification to the application. */
	lims_Notify_Type_CALL_HELD,							/**< Call Held notification from the peer. */
	lims_Notify_Type_CALL_RESUMED,						/**< Call Resumed notification to the application. */
	lims_Notify_Type_CALL_RESUME_FAILED,				/**< Call Resume Failed notification to the application. */
	lims_Notify_Type_SESSION_INCOMING,					/**< Incoming CPM session notification. pData will be typecast to lims_RespondSessionStruct to access the details. */
	lims_Notify_Type_SESSION_RINGING,					/**< CPM session ringing notification to lims_RespondSessionStruct to access the details. */
	lims_Notify_Type_SESSION_PROGRESS,					/**< CPM session progress notification to lims_RespondSessionStruct to access the details. */
	lims_Notify_Type_SESSION_ESTABLISHED,				/**< CPM established notification. */
	lims_Notify_Type_SESSION_CONNECT_FAILED,			/**< All kind of failure response from far end. */
	lims_Notify_Type_SESSION_ENDED,						/**< Ended notification updated to the callee or caller if BYE request initiated/received to/from the network. */
	lims_Notify_Type_STAND_ALONE_MESSAGE_SENT,			/**< The stand alone message received notification. */
	lims_Notify_Type_STAND_ALONE_MESSAGE_FAILED,		/**< The stand alone message failed notification. */
	lims_Notify_Type_STAND_ALONE_MESSAGE_RECEIVED,		/**< The stand alone message received notification. */
	lims_Notify_Type_MESSAGE_SENT,						/**< The CPM session based message send success notification. */
	lims_Notify_Type_MESSAGE_FAILED,					/**< The CPM session based message send failed notification. */
	lims_Notify_Type_MESSAGE_RECEIVED,					/**< The CPM session based message received notification. */
	lims_Notify_Type_RICHCARD_SENT,						/**< The CPM rich card transfer success notification. */
	lims_Notify_Type_RICHCARD_FAILED,					/**< The CPM rich card transfer failed notification. */
	lims_Notify_Type_RICHCARD_RECEIVED,					/**< The CPM rich card transfer received notification. */
	lims_Notify_Type_IMDN,								/**< The IMDN disposition notification. */
	lims_Notify_Type_COMPOSING,							/**< The CPM Composing disposition notification. */
	lims_Notify_Type_CAPABILITIES_PUBLISHED,			/**< The UCE PUBLISH successful notification. pData is of type u_char* containing etag. */
	lims_Notify_Type_CAPABILITIES_PUBLISH_FAILED,		/**< The UCE PUBLISH failure notification. pData is NULL. */
	lims_Notify_Type_CAPABILITIES_REMOVED,				/**< The UCE removed publised capability success notification. pData is of type u_char* containing etag. */
	lims_Notify_Type_QUERY_CAPABILITIES_SUCCESS,		/**< The UCE subcription to capability was successful. pData is the UCE reason structure containing reason code and text. */
	lims_Notify_Type_QUERY_CAPABILITIES_FAILED,			/**< The UCE subcription to capability was failed. pData is the UCE reason structure containing reason code and text. */
	lims_Notify_Type_QUERY_CAPABILITIES_DISCOVERED,		/**< The UCE presence notify with PIDF document. pData is of type EcrioUCERecordsOfPresenceDocumentStruct* containing records of presence information. */
	lims_Notify_Type_SUBSCRIBE_PARTICIPANTINFO_SUCCESS,	/**< The CPM SUBSCRIBE successful notification. */
	lims_Notify_Type_SUBSCRIBE_PARTICIPANTINFO_FAILED,	/**< The CPM SUBSCRIBE failure notification. */
	lims_Notify_Type_UNSUBSCRIBE_PARTICIPANTINFO_SUCCESS,	/**< The CPM Un-SUBSCRIBE successful notification. */
	lims_Notify_Type_UNSUBSCRIBE_PARTICIPANTINFO_FAILED,	/**< The CPM Un-SUBSCRIBE failure notification. */
	lims_Notify_Type_UPDATE_PARTICIPANT_SUCCESS,		/**< The CPM REFER successful notification. */
	lims_Notify_Type_UPDATE_PARTICIPANT_FAILED,			/**< The CPM REFER failure notification. */
	lims_Notify_Type_MODIFY_GROUPCHAT_SUCCESS,			/**< Modify Group Chat Data successful notification. */
	lims_Notify_Type_MODIFY_GROUPCHAT_FAILED,			/**< Modify Group Chat Data failure notification. */
	lims_Notify_Type_GROUPCHAT_INFO,					/**< The group chat information received notification. */
	lims_Notify_Type_GROUPDATA_INFO,					/**< The group chat data information received in the EcrioCPMGroupDataRequestStruct */
	lims_Notify_Type_GROUPCHAT_ICON,					/**< The group chat icon information received in the EcrioCPMGroupChatIconStruct */
	lims_Notify_Type_PRIVACY_MANAGEMENT_RESPONSE,		/**< The result of privacy management commands notification. */
	lims_Notify_Type_SOCKET_ERROR,						/**< Received socket error notification from PAL */
	lims_Notify_Type_ESTABLISH_IPSEC_SA,				/**< IPsec establish SA notification. */
	lims_Notify_Type_DISCONNECT_IPSEC_SA,				/**< Tear down IPsec SA notification. */
#ifdef ENABLE_QCMAPI
	lims_Notify_Type_MONITOR_DEDICATED_BEARER,			/**< Request to start monitoring dedicated bearer from network. */
	lims_Notify_Type_CONNECTION_CREATED,				/**< Received connection creation notification from PAL. */
	lims_Notify_Type_GET_ACSCONFIGURATION,				/**< Received ACS Configuration from PAL. */
	lims_Notify_Type_AUTOCONFIG_REQUESTSTATUS,			/**< Received AutoConfig RequestStatus from PAL. */
	lims_Notify_Type_SERVICE_NOTALLOWED,				/**< Received IMS CM Service not allowed error. */
	lims_Notify_Type_SERVICE_FORCEFUL_CLOSE,			/**< IMS CM Service connection closed forcefully. */
	lims_Notify_Type_SERVICE_TERMINATE_CONNECTION		/**< Received IMS CM Service connection terminate error. */
#else
	lims_Notify_Type_MONITOR_DEDICATED_BEARER			/**< Request to start monitoring dedicated bearer from network. */
#endif
} lims_NotifyTypeEnums;

/*@}*/
/** @name Asynchronous Notification Handling
* \n Callback functions may be implemented by the caller and are invoked in response to various actions and events.
* This section defines the various callbacks that the lims module implementation may invoke as well as any related
* structures. Callbacks should always be invoked from an lims module implementation without blocking internally.
* The caller should likewise attempt to minimize the time spent in the callback handler.
*/
/*@{*/

/** @enum lims_CallbackTypeEnums
 * This enumeration defines the types of callbacks that the lims module implementation
 * may invoke.
 */
typedef enum
{
	lims_CallbackType_Error,				/**< A error callback, which is called when a error happend in any async operations.  */
	lims_CallbackType_Status				/**< A status callback, which is called when a any status needs to update to the calling layer.*/
} lims_CallbackTypeEnums;

/**
 * This function definition is a prototype of the get property callback handler that can be
 * called to obtain the runtime parameters such as (AKA response etc.) from the caller. It is
 * Synchronous call and when it returns, framework expects the value or error.
 *
 * @param[in] eName			Requested property name define in EcrioSipPropertyNameEnums.
 * @param[in] eType			Expected property value data type from lims implementation. Types are defined in
 *							EcrioSipPropertyTypeEnums.
 * @param[in,out] pData		Void Pointer, which will be typecast to the requested property values. Values are
 *							depends on eType argument.lims modules consumes the property value received in it. Application
 *							layer handles the memory allocation and deallocation.
 * @param[in] pContext	    The opaque data pointer provided by application layer in lims_CallbackStruct in the lims_init() function.
 *
 * @return Returns a success or failure error.
 */
typedef u_int32 (*lims_GetPropertyCallback)
(
	EcrioSipPropertyNameEnums eName,
	EcrioSipPropertyTypeEnums eType,
	void *pData,
	void *pContext
);

/**
* This function definition is a prototype of the get property callback handler that can be
* called to obtain the runtime parameters for UCE module
*
* @param[in] eName			Requested property name define in EcrioUCEPropertyNameEnums.
*							EcrioSipPropertyTypeEnums.
* @param[in,out] pData		Void Pointer, which will be typecast to the requested property values. Values are
*							depends on eType argument.lims modules consumes the property value received in it. Application
*							layer handles the memory allocation and deallocation.
* @param[in] pContext	    The opaque data pointer provided by application layer in lims_CallbackStruct in the lims_init() function.
*
* @return Returns a success or failure error.
*/
typedef u_int32(*lims_UCEPropertyCallback)
(
	EcrioUCEPropertyNameEnums eName,
	void *pData,
	void *pContext
);


/**
 * This function definition is a prototype of the callback handler that can be called for any asynchronous
 * notification from the lims implementation.
 *
 * @param[in] eCallbackType	type of the callback invoked from lims implementation.
 * @param[in] eNotifType		type of the notification invoked from lims module.
 * @param[in] pData			Pointer to a structure which will define the particular
 *							notification being sent to the application layer.
 * @param[in] eType
 * @param[in] pContext	    The opaque data pointer received in lims_CallbackStruct at the time of lims_init() function.
 *
 * @return Returns a void.
 */
typedef void (*lims_Callback)
(
	lims_CallbackTypeEnums eCallbackType,
	lims_NotifyTypeEnums eNotifType,
	void *pData,
	void *pContext
);
/*@}*/

/** @struct lims_NetworkConnectionStruct
 * This structure defines the required IPs for lims module which uses for communication. These Bearer connection details
 * provided to lims modules via lims_Register() API function. Calling layer get the Remote IP and DNS IP details either from LTE attach
 * procedure or read from configuration file. Calling layer obtaine the local ip once it establish the network connection with the bearer.
 * bearer may be either Wifi or LTE.
 */
typedef struct
{
	u_char uNoOfRemoteIps;				/**< number of remote Ips present in the array. */
	char **ppRemoteIPs;					/**< array of remote Ips.*/
	lims_IPTypeEnum eIPType;			/**< IP Version IPv4 or IPv6.*/
	char *pLocalIp;						/**< Indicates the local IP. */
	u_int32	uLocalPort;					/**< Indicates the local port used for communication.*/
	u_int32 uRemotePort;				/**< Indicates the remote port used for communication. */
	u_int32 uRemoteTLSPort;				/**< Indicates the remote tls port used for communication. */
	u_int32 uRemoteClientPort;          /**< Indicates the remote client port used for communication. */
	lims_NetworkConnectionStatus uStatus;/**< Indicates the connected network status. */
	BoolEnum isProxyRouteEnabled;		/**< Conditional variable to use proxy routing or not. */
	char* pProxyRouteAddress;			/**< Null terminated string holding Proxy server address. */
	u_int32 uProxyRoutePort;			/**< Proxy server port. */
} lims_NetworkConnectionStruct;

/** @struct lims_StartSessionStruct
 * This structure defines to hold parameters to start CPM session. pPath is
 * an output parameter, it will set a valid pointer to local "path" attribute
 * string used for MSRP session when function returns LIMS_NO_ERROR if successful.
 */
typedef struct
{
	u_int16 uMediaPort;						/**< Local port to be used for MSRP session. Same port is used in SDP. Set to 0 if dynamic port need to be used. */
	u_char *pTarget;						/**< Null terminated string holding the Mobile Device Number or MDN(which is basically a phone number). */
	EcrioCPMFileDescriptorStruct *pFile;	/**< The file descriptor, pointer to EcrioCPMFileDescriptorStruct. */
	EcrioCPMConversationsIdStruct *pConvId;	/**< The conversation Identification, pointer to EcrioCPMConversationsIdStruct. */
	u_int32 contentTypes;					/**< CPM Content types, this is the EcrioCPMContentTypeEnum with bitmask. */
	BoolEnum bIsLargeMode;					/**< If Enum_TRUE then start large mode session else CPM session. */
	BoolEnum bPrivacy;						/**< Enable this flag to provide privacy.*/
	BoolEnum bIsChatbot;					/**< If Enum_TRUE then start 1-to-1 chatbot session else CPM session.*/
	EcrioCPMChatbotPrivacyEnum eBotPrivacy;	/**< Privacy request for Chatbot session. */
} lims_StartSessionStruct;

/** @struct lims_RespondSessionStruct
* This structure defines to hold parameters to respond CPM session.
*/
typedef struct
{
	u_int16 uMediaPort;						/**< Local port to be used for MSRP session. Same port is used in SDP. Set to 0 if dynamic port need to be used. */
	char *pSessionId;						/**< The ID of the CPM session session that should be affected. */
	EcrioCPMConversationsIdStruct *pConvId;	/**< The conversation Identification, pointer to EcrioCPMConversationsIdStruct. */
	u_int32 contentTypes;					/**< CPM Content types, this is the EcrioCPMContentTypeEnum with bitmask. */
	EcrioCPMSessionStatusEnum eStatus;		/**< CPM Session Status Type. */
	EcrioCPMFileDescriptorStruct *pFile;	/**< The file descriptor, pointer to EcrioCPMFileDescriptorStruct. */
} lims_RespondSessionStruct;

/** @struct lims_StandAloneMessageStruct
* This structure defines to hold parameters to send stand alone mode message.
*/
typedef struct
{
	char* pDestUri;									/**< This is MDN of the recipient. */
	EcrioCPMConversationsIdStruct* pConvId;			/**< The CPM Conversation Identification headers, pointer to EcrioCPMConversationsIdStruct. */
	EcrioCPMMessageStruct* pMessage;				/**< Point to EcrioCPMMessageStruct structure. */
	BoolEnum bIsChatbot;							/**< A flag to indicate whether destination is a Chatbot or not. */
	BoolEnum bDeleteChatBotToken;					/**< A flag to delete the chatbot token. The Anonymization Function shall delete the token if it's Enum_TRUE. 
													 The bIsChatbot flag MUST be Enum_TRUE for chatbot features. */
} lims_StandAloneMessageStruct;

/** @struct lims_SendMessageStruct
* This structure defines to hold chat message details.
*/
typedef struct
{
	char *pSessionId;									/**< CPM session identifier. */
	EcrioCPMMessageStruct* pMessage;					/**< Point to EcrioCPMMessageStruct structure. */
} lims_SendMessageStruct;

/** @struct lims_DataStruct
* This structure defines to hold details of data to be sent out.
*/
typedef struct
{
	char *pSessionId;									/**< Session identifier. */
	u_char *pDataBuf;									/**< Pointer to data buffer to be sent out. */
	u_int32 uDataBufSize;								/**< Size of the data pointed by pDataBuf */
	u_int32 uByteRangeStart;							/**< Starting range of the data to be sent. This is mapped directly to MSRP 
														Byte-Range header. Value start with 0 and increased with uDataBufSize by the caller*/
	u_int32 uByteRangeEnd;								/**< Ending range of the data to be sent. This is mapped directly to MSRP 
														Byte-Range header. Value start with uDataBufSize and increased with uDataBufSize by the caller*/
	u_int32 uByteRangeTotal;							/**< Total file size of the file data. This is mapped directly to MSRP 
														Byte-Range header. Value is always the same during file transfer. */
	char *pContentType;									/**< Content type of the file to be sent data. This is mapped directly to 
														MSRP Content-Type header. */
	BoolEnum bIsEofReached;								/**< Enum_TRUE if this data reaches EOF, that means this is a final MSRP 
														chunk with "$" flags. Otherwise, there will set "+" flags. */
} lims_DataStruct;


/** @struct lims_ComposingStruct
* This structure defines to hold the parameters of Composing notification API.
*/
typedef struct
{
	char *pSessionId;									/**< CPM session identifier. */
	EcrioCPMComposingStruct *pComposing;				/**< The pointer to EcrioCPMComposingStruct. */
} lims_ComposingStruct;


/** @struct lims_ParticipantListStruct
 * This structure defines to hold the URI list of group chat participants.
 */
typedef struct
{
	u_int32 uNumOfUsers;					/**< Number of participants. */
	char **ppUri;							/**< Pointer to the URI lists. */
} lims_ParticipantListStruct;

/** @struct lims_UserCapabilityListStruct
* This structure defines to hold the URI list of users whos capabilities need to be queried.
*/
typedef struct
{
	u_int32 uNumOfUsers;					/**< Number of participants. */
	char **ppUri;							/**< Pointer to the URI lists. */
} lims_UserCapabilityListStruct;

/** @struct lims_StartGroupChatSessionStruct
 * This structure defines to hold parameters to start group chat session.
 */
typedef struct
{
	u_int16 uMediaPort;						/**< Local port to be used for MSRP session. Same port is used in SDP. Set to 0 if dynamic port need to be used. */
	u_char *pTarget;						/**< Null terminated string holding the Conference-Factory URI or Group Chat Session ID. */
	u_char *pSubject;						/**< Subject to be used for group chat. */
	EcrioCPMConversationsIdStruct *pConvId;	/**< The conversation Identification, pointer to EcrioCPMConversationsIdStruct. */
	EcrioCPMParticipantListStruct *pList;	/**< List of participants invited to group chat. */
	u_int32 contentTypes;					/**< CPM Content types, this is the EcrioCPMContentTypeEnum with bitmask. */
	BoolEnum bPrivacy;						/**< Enable this flag to provide privacy.*/
	BoolEnum bIsClosed;						/**< Enable if initiate the closed group chat session. */
} lims_StartGroupChatSessionStruct;

/** @struct lims_RespondGroupChatSessionStruct
 * This structure defines to hold parameters to respond group chat session.
 */
typedef struct
{
	u_int16 uMediaPort;						/**< Local port to be used for MSRP session. Same port is used in SDP. Set to 0 if dynamic port need to be used. */
	char *pSessionId;						/**< The ID of the CPM session session that should be affected. */
	EcrioCPMConversationsIdStruct *pConvId;	/**< The conversation Identification, pointer to EcrioCPMConversationsIdStruct. */
	u_int32 contentTypes;					/**< CPM Content types, this is the EcrioCPMContentTypeEnum with bitmask. */
	EcrioCPMSessionStatusEnum eStatus;		/**< CPM Session Status Type. */
	EcrioCPMParticipantListStruct *pList;	/**< List of participants for group chat. */
} lims_RespondGroupChatSessionStruct;

/** @struct lims_RegistrationModuleStruct
 * This structure defines to hold parameters for default or shared registartion model.
 */
typedef struct
{
    BoolEnum isRegistrationShared;
    void* pRegSharedData;
} lims_RegistrationModelStruct;

/** @struct lims_PrivacyManagementCommandsStruct
* This structure defines to hold parameters to send Privacy Management Commands.
*/
typedef struct
{
	char *pChatbot;									/**< Null terminated string holding the URI of Chatbot. */
	char *pCommandID;								/**< The Command ID. */
	EcrioCPMPrivMngCmdActionEnum eAction;			/**< CPM Privacy Management Commands action. */
} lims_PrivacyManagementCommandsStruct;

/** @struct lims_SpamReportMessage
* This structure defines to hold parameters to send Spam Report Message.
*/
typedef struct
{
	char *pChatbot;									/**< Null terminated string holding the URI of Chatbot. */
	char **ppMessageIDs;							/**< Pointer to the Message-ID lists. */
	u_int32 uNumOfMessageID;						/**< Number of Message-IDs. */
} lims_SpamReportMessage;


/** @struct lims_ConfigStruct
 * This structure defines the configuration parameters required for lims module to operate successfully. Calling layer obtained either from
 * UICC or configuration file.
 */
typedef struct
{
	PALINSTANCE pal;						/**< Platform Abstract Layer instance. */
	LOGHANDLE logHandle;					/**< Log module instance. */
	char *pPublicIdentity;					/**< public identity obtained from UICC or configuration file. */
	char *pPrivateIdentity;					/**< private identity obtained from UICC or configuration file. */
	char *pHomeDomain;						/**< home domain value obtained from UICC or configuration file. */
	EcrioSipAuthAlgorithmEnum eAlgorithm;	/**< algorithm type value obtain from configuration file or OMA DM. */
	char *pPassword;						/**< pPassword value will be used if the eAlgorithm is lims_Algorithm_Type_MD5.Otherwise it will be ignored.*/
	char *pUserAgent;						/**< User Agent String.*/
	u_int32 uRegExpireInterval;				/**< Registration expire timer interval. */
	u_int32 uSubExpireInterval;				/**< Subscription expire timer interval. */
	BoolEnum bSubscribeRegEvent;			/**< Subscribe to reg event. */
	BoolEnum bCapabilityInfo;				/**< Send Capability information automatically. */
	BoolEnum bUnSubscribeRegEvent;			/**< UnSubscribe to reg event. */
	BoolEnum bPublishRCSCapabilities;		/**< Publish RCS capability using PUBLISH method. */
	BoolEnum bSendRingingResponse;			/**< The lims will send 180 ringing response for an incoming INVITE if Enum_TRUE. */
	u_int32 uPublishRefreshInterval;		/**< Publish refresh interval in seconds for periodic publish. */
	BoolEnum bisChatbot;					/**< Supporting chatbot functionality. */
	char *pDeviceId;						/**< IMEI or MEID used to form the instance ID. */
	char *pDisplayName;						/**< Display-name string. */
	EcrioOOMStruct *pOOMObject;				/**< Operator Object Model instance. */
	BoolEnum bIsRelayEnabled;				/**< Route MSRP packets to relay server based on obtained use-path from relay server during AUTH process */
	char *pRelayServerIP;					/**< IP address of the relay server */
	u_int32 uRelayServerPort;				/**< Relay server port to which client needs to send MSRP AUTH */
	u_int32 uMtuSize;						/**< decission to use the UDP or TCP channel. 0 indicates the default size as per standard. */
	u_int32 uNoActivityTimerInterval;		/**< udp-ping packet sends to the network based on interval configured. 0 means featured disabled. */
	BoolEnum bEnableUdp;					/**< Enable this flag create the UDP channel. Neither bEnableUdp nor bEnableTcp flag to set to true. then it will return error.*/
	BoolEnum bEnableTcp;					/**< Enable this flag create the TCP channel. Neither bEnableUdp nor bEnableTcp flag to set to true. then it will return error.*/
	BoolEnum bEnablePAI;					/**< Check P-Associated-Identity header to validate restriction ID. */
	BoolEnum bSupportedPrecondition;		/**< Include precondition tag in Supported header and support any incoming call request for precondition. */
	BoolEnum bRequiredPrecondition;			/**< Include precondition tag in Required header and request for precondition in outing call. */
    lims_RegistrationModelStruct regModel;  /**< Registration specific data. */
	char *pPANI;							/**< Pointer to P-Access-Network-Info header value. */
	char *pSecurityVerify;					/**< Pointer to IPSec Security-Verify header value. */
	char *pAssociatedUri;					/**< Pointer to P-Associated-URI header value. */
	char *pUriUserPart;						/**< Pointer to user part in Contact-URI value. */
	BoolEnum bTLSEnabled;					/**< Enable this flag to support TLS. */
	BoolEnum bTLSPeerVerification;			/**< Check if TLS Peer verification is required. */
	char *pTLSCertificate;					/**< Pointer to the TLS certificate, along with the location. */
} lims_ConfigStruct;

/** @struct lims_CallbackStruct
 * The structure is passed to the lims_Init() function and contains function pointers to
 * status and get property callback functions.
 */
typedef struct
{
	lims_GetPropertyCallback pLimsGetPropertyCallback;				/**< GetProperty Callback function pointer. */
	lims_UCEPropertyCallback pUCEPropertyCallback;					/**< UCEProperty Callback function pointer.. */
	lims_Callback pLimsCallback;									/**< Status and error notification callback function pointer. */
	void *pContext;													/**< Opaque data pointer providing to lims implementation. This will be return to the application layer in the callback functions */
} lims_CallbackStruct;

/** @struct lims_RegisterStruct
* The structure is passed to the lims_Register() function and contains IPSec parameters
* required to establish IP security.
*/
typedef struct
{
	EcrioSipIPsecParamStruct *pIPsecParams;
}lims_RegisterStruct;

/** @struct lims_GroupChatIconInfoStruct
* The structure is used to the change the group chat icon using file-info.
*/
typedef struct
{
	u_char* pFileInfo;												/**< The content-id of the image file. */
	u_char* pData;													/**< The binary data of the icon. */
	u_int32 uDataLength;											/**< The length of the data. */
	u_char* pContentType;											/**< Content Type of the image file*/
} lims_GroupChatIconInfoStruct;

/**
 * This function can be used to initialize an overall lims implementation
 * instance. The lims definition assumes that only one lims instance can exist on
 * a device at a time. This function acts synchronously, and upon return the
 * lims instance should be prepared.
 *
 * It also initialize all the required underlying  modules..
 *
 * @param[in] pConfig			Pointer to a lims_ConfigStruct structure. Must
 *								be valid. The caller owns the structure and the
 *								lims implementation will consume or copy from it.
 * @param[in] pCallback			Pointer to a structure that holds all the callback
 *								pointers for asynchronous communications back to
 *								the caller.
 * @param[out] pError			Pointer to a u_int32 to place the result code in
 *								upon return. Must be valid.
 *
 * @return Returns a valid lims instance handle. If NULL, pError will provide
 * the error code. Callers only need to rely on the function return, it is not
 * necessary to check both the return and pError. They will be guaranteed to
 * be consistent.
 */
LIMSHANDLE lims_Init
(
	lims_ConfigStruct *pConfig,
	lims_CallbackStruct *pCallback,
	u_int32 *pError
);

/**
 * This function is used to deinitialize the lims implementation instance,
 * cleaning up all consumed resources.
 *
 * This function return error if device is in registration state. Invoke
 * lims_Deregister() function first before invoking this function even if there is no
 * active network connection.
 *
 * Invoking this function without initializing the lims module will also return
 * error.
 *
 * The caller should set their instance handle to NULL since upon return
 * from this function it will not point to a valid context any more.
 *
 * @param[in] handle			Pointer to lims instance handle.
 *
 * @return Returns LIMS_NO_ERROR if successful, otherwise an error
 * specific to the lims implementation.
 */
u_int32 lims_Deinit
(
	LIMSHANDLE *handle
);

/**
 * This function is used to update the current connected network bearer connection
 * state to lims module. Caller monitors the bearer connection and update the connection
 * change to lims module using this function. This function always assume that the first remote
 * IP to be used for the current remote address and corresponding channels will be created.
 *
 * If there is no bearer connection, caller invokes this function along
 * with the type lims_Network_Connection_Type_NONE and provide pConnectionDetails as NULL.
 *
 * @param[in] handle						Pointer to lims instance handle.
 * @param[in] ePdnType					Connected PDN type.
 * @param[in] eConnectionType			Network connection type enumeration.
 * @param[in] pConnectionDetails			pointer to lims_NetworkConnectionStruct.
 *
 * @return Returns LIMS_NO_ERROR if successful, otherwise an error
 * specific to the lims implementation.
 * If returned LIMS_SOCKET_ERROR, application need to re-initalize lims by invoking
 * lims_Deinit(), lims_Init() and lims_NetworkStateChange().
 */
u_int32 lims_NetworkStateChange
(
	LIMSHANDLE handle,
	lims_NetworkPDNTypeEnum	ePdnType,
	lims_NetworkConnectionTypeEnum eConnectionType,
	lims_NetworkConnectionStruct *pConnectionDetails
);

/**
 * This function can be used to initiate register request to the network.
 * Caller invoke this function once it has the valid communication bearer.
 *
 * Before invoking this function, caller need to invoke the
 * lims_NetworkConnectionStateChange with connected network details.
 *
 * This function return error if there is no network connection.
 *
 * @param[in] handle				lims instance handle
 * @param[in] uFeatures			Supported features. This is bitmask value of lims_FeatureEnums
 * @param[in] pRegisterStruct	IPSec parameters details required to establish IPSec connection but it can be
 *								set to NULL if IPSec is not required.
 *
 * @return Returns LIMS_NO_ERROR if successful, otherwise an error
 * specific to the lims implementation.
 * If returned LIMS_SOCKET_ERROR, application need to re-initalize lims by invoking
 * lims_Deinit(), lims_Init(), lims_NetworkStateChange() and lims_Register().
 */
u_int32 lims_Register
(
	LIMSHANDLE handle,
	u_int32 uFeatures,
	lims_RegisterStruct* pRegisterStruct
);

/**
 * This function can be used to initiate a deregister request to the network.
 * lims module internally check if the module is already registered with the network
 * it will perform the deregistration else it will return success but doesn't send
 * any request to the network.
 *
 * @param[in] handle				lims instance handle.
 *
 * Returns LIMS_NO_ERROR if successful, otherwise an error specific to the lims implementation.
 */
u_int32 lims_Deregister
(
	LIMSHANDLE handle
);

/**
* This API can be called to send stand alone mode instant message. The pTarget will 
* point to a target CPM User. Default URI type lims assumes internally TEL URI.
*
* @param[in] handle				The lims instance handle.
* @param[in] pStandAloneMsg		Pointer to lims_StandAloneMessageStruct.
* @param[in,out] ppCallId		Pointer to unique call Id.
*
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
* If returned LIMS_SOCKET_ERROR, application need to re-initalize lims by invoking
* lims_Deinit(), lims_Init(), lims_NetworkStateChange() and lims_Register().
*/
u_int32 lims_SendStandAloneMessage
(
	LIMSHANDLE handle,
	lims_StandAloneMessageStruct* pStandAloneMsg,
	char** ppCallId 
);

/**
* This API is used to get the size of the CPIM payload that need to be sent out.
*
* @param[in] handle				The lims instance handle.
* @param[in] pStandAloneMsg		Pointer to lims_StandAloneMessageStruct.
* @param[out] pMsgSize			Pointer to size of the message.
*
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
* If returned LIMS_SOCKET_ERROR, application need to re-initalize lims by invoking
* lims_Deinit(), lims_Init(), lims_NetworkStateChange() and lims_Register().
*/
u_int32 lims_GetStandAloneMessageSize
(
	LIMSHANDLE handle,
	lims_StandAloneMessageStruct* pPagerMsg,
	u_int32* pMsgSize
);

/**
 * This API can be called to start the CPM/File Transfer over MSRP session. The pTarget in 
 * lims_StartSessionStruct will point to a target User to start a session. Default URI 
 * type lims assumes internally TEL URI. For Filet Transfer over MSRP session the  contentTypes  
 * is set to EcrioCPMContentTypeEnum_FileTransfer and after establsihing session, content of the 
 * file is chunked and sent to other party via MSRP internally.
 *
 * @param[in] handle			The lims instance handle.
 * @param[in] pStart			Pointer to lims_StartSessionStruct.
 * @param[in,out] ppSessionId	The session handle.
 *
 * @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
 * lims implementation.
 * If returned LIMS_SOCKET_ERROR, application need to re-initalize lims by invoking
 * lims_Deinit(), lims_Init(), lims_NetworkStateChange() and lims_Register().
 */
u_int32 lims_StartSession
(
	LIMSHANDLE handle,
	lims_StartSessionStruct *pStart,
	char **ppSessionId
);

/**
 * This API can be used to respond incoming CPM/File Transfer over MSRP session.
 *
 * @param[in] handle			The lims instance handle.
 * @param[in] pRespond			Pointer to lims_RespondSessionStruct.
 *
 * @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
 * lims implementation.
 * If returned LIMS_SOCKET_ERROR, application need to re-initalize lims by invoking
 * lims_Deinit(), lims_Init(), lims_NetworkStateChange() and lims_Register().
 */
u_int32 lims_RespondSession
(
	LIMSHANDLE handle,
	lims_RespondSessionStruct *pRespond
);

/**
 * This API can be used to end ongoing CPM session.
 *
 * @param[in] handle		The lims instance handle.
 * @param[in] pSessionId	The ID of the CPM session that should be
 *							affected.
 * @param[in] termByUser	Boolean that decides reason code in BYE
 * @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
 * lims implementation.
 * If returned LIMS_SOCKET_ERROR, application need to re-initalize lims by invoking
 * lims_Deinit(), lims_Init(), lims_NetworkStateChange() and lims_Register().
 */
u_int32 lims_EndSession
(
	LIMSHANDLE handle,
	char *pSessionId,
	BoolEnum termByUser
);

/**
* This API can be called to send messages within a CPM session.
* Message within session includes, simple text, Rich card, Suggested Request, Suggested Response
* and JSON payload of File Transfer over HTTP.
*
* @param[in] handle			   The lims instance handle.
* @param[in] pSendMessage	   Pointer to lims_SendMessageStruct.
* @param[in,out] ppMessageId   Message Identifier.
*
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
*/
u_int32 lims_SendMessage
(
	LIMSHANDLE handle,
	lims_SendMessageStruct* pSendMessage,
	char **ppMessageId
);

/**
* This API can be called to send data within a File Transfer session. Maximum size
* of the data chunk that can be sent is 2048 Bytes.
*
* @param[in] handle					The lims instance handle.
* @param[in] pData					Pointer to lims_DataStruct.
* @param[in,out] ppMessageId		Message Identifier.
* @param[in,out] ppTransactionId	Transaction Identifier.
* @param[in] bIsCPIMWrapped			Condition variable to send file data wrapped with CPIM message.
*
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
*/
u_int32 lims_SendData
(
	LIMSHANDLE handle,
	lims_DataStruct* pData,
	char **ppMessageId,
	char **ppTransactionId,
	BoolEnum bIsCPIMWrapped
);

/**
 * This API can be used to abort ongoing file transfer.
 *
 * @param[in] handle		The lims instance handle.
 * @param[in] pSessionId	The ID of the CPM session that should be
 *							affected.
 *
 * @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
 * lims implementation.
 * If returned LIMS_SOCKET_ERROR, application need to re-initalize lims by invoking
 * lims_Deinit(), lims_Init(), lims_NetworkStateChange() and lims_Register().
 */
u_int32 lims_AbortFileTransfer
(
	LIMSHANDLE handle,
	char *pSessionId
);

/**
 * This API can be called to start the Group Chat session. The pTarget in lims_StartGroupChatSessionStruct
 * will point to a Conference-Factory URI or Group Chat Session ID which indicated by the Conference
 * Factory Application. By specifying the Group Chat Session ID that has already been created session,
 * this assume a role of Join the Group Chat Session.
 *
 * @param[in] handle			The lims instance handle.
 * @param[in] pStart			Pointer to lims_StartGroupChatSessionStruct.
 * @param[in,out] ppSessionId	The session handle.
 *
 * @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
 * lims implementation.
 * If returned LIMS_SOCKET_ERROR, application need to re-initalize lims by invoking
 * lims_Deinit(), lims_Init(), lims_NetworkStateChange() and lims_Register().
 */
u_int32 lims_StartGroupChatSession
(
	LIMSHANDLE handle,
	lims_StartGroupChatSessionStruct *pStart,
	char **ppSessionId
);

/**
 * This API can be used to respond incoming Group Chat session.
 *
 * @param[in] handle			The lims instance handle.
 * @param[in] pRespond			Pointer to lims_RespondGroupChatSessionStruct.
 *
 * @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
 * lims implementation.
 * If returned LIMS_SOCKET_ERROR, application need to re-initalize lims by invoking
 * lims_Deinit(), lims_Init(), lims_NetworkStateChange() and lims_Register().
 */
u_int32 lims_RespondGroupChatSession
(
	LIMSHANDLE handle,
	lims_RespondGroupChatSessionStruct *pRespond
);

/**
 * This API can be used to end ongoing Group Chat session.
 *
 * @param[in] handle		The lims instance handle.
 * @param[in] pSessionId	The ID of the CPM session that should be
 *							affected.
 * @param[in] leaveGroup	Boolean that decides reason code in BYE
 *
 * @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
 * lims implementation.
 * If returned LIMS_SOCKET_ERROR, application need to re-initalize lims by invoking
 * lims_Deinit(), lims_Init(), lims_NetworkStateChange() and lims_Register().
 */
u_int32 lims_EndGroupChatSession
(
	LIMSHANDLE handle,
	char *pSessionId,
	BoolEnum leaveGroup
);

/**
 * This API can be used to invite other principals to existing Group Chat session.
 * Participants are specified in a list format and multiple URIs can be pointed.
 *
 * @param[in] handle		The lims instance handle.
 * @param[in] pList			Pointer to lims_ParticipantListStruct.
 * @param[in] pSessionId	The Session ID of the group chat that currently in existing.
 * @param[in] ppReferId		ID to identify the REFER to add participant 
 *
 * @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
 * lims implementation.
 */
u_int32 lims_AddParticipantsToGroupChatSession
(
	LIMSHANDLE handle,
	lims_ParticipantListStruct *pList,
	char *pSessionId,
	char **ppReferId
);

/**
 * This API can be used to remove participants from existing Group Chat session.
 * Participants are specified in a list format and multiple URIs can be pointed.
 *
 * @param[in] handle		The lims instance handle.
 * @param[in] pList			Pointer to lims_ParticipantListStruct.
 * @param[in] pSessionId	The Session ID of the group chat that currently in existing.
 * @param[in] ppReferId		ID to identify the REFER to remove participant
 *
 * @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
 * lims implementation.
 */
u_int32 lims_RemoveParticipantsFromGroupChatSession
(
	LIMSHANDLE handle,
	lims_ParticipantListStruct *pList,
	char *pSessionId,
	char **ppReferId
);

/**
 * This API can be called to change subject for existing group chat session.
 *
 * @param[in] handle		The lims instance handle.
 * @param[in] pSubject		The string of subject.
 * @param[in] pSessionId	The Session ID of the group chat that currently in existing.
 *
 * @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
 * lims implementation.
 */
u_int32 lims_SetGroupChatSubject
(
	LIMSHANDLE handle,
	char *pSubject,
	char *pSessionId
);

/**
* This API can be called to change host of group chat session.
*
* @param[in] handle			The lims instance handle.
* @param[in] pHostUri		The string of HostUri.
* @param[in] pSessionId		The Session ID of the group chat that currently in existing.
*
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
*/
u_int32 lims_SetGroupChatHost
(
	LIMSHANDLE handle,
	char *pHostUri,
	char *pSessionId
);

/**
 * This API can be called to delete subject for existing group chat session.
 *
 * @param[in] handle		The lims instance handle.
 * @param[in] pSessionId	The Session ID of the group chat that currently in existing.
 *
 * @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
 * lims implementation.
 */
u_int32 lims_DeleteGroupChatSubject
(
	LIMSHANDLE handle,
	char *pSessionId
);

/**
* This API can be called to delete icon for existing group chat session.
*
* @param[in] handle		The lims instance handle.
* @param[in] pSessionId	The Session ID of the group chat that currently in existing.
*
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
*/
u_int32 lims_DeleteGroupChatIcon
(
	LIMSHANDLE handle,
	char *pSessionId
);

/**
* This API can be called to set the icon for existing group chat session.
* If icon URI is provided then the icon URI will be modified. If fileInfo
* is passed then the icon will be set using file info and file data. Only 
* one shall be used.
*
* @param[in] handle		The lims instance handle.
* @param[in] pIcon		The string of icon URI.
* @param[in] pFileInfo	The file-info of the icon.
* @param[in] pSessionId	The Session ID of the group chat that currently in existing.
*
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
*/
u_int32 lims_SetGroupChatIcon
(
	LIMSHANDLE handle,
	char *pIcon,
	lims_GroupChatIconInfoStruct *pFileInfo,
	char *pSessionId
);

/**
 * This API can be called to subscribe participant information connected to
 * indicated group chat session ID.
 *
 * @param[in] handle			The lims instance handle.
 * @param[in] pSessionId		Session identifier.
 * @param[in] pGroupSessionId	The ID of the group chat session that should be
 *								affected.
 *
 * @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
 * lims implementation.
 */
u_int32 lims_SubscribeToGroupParticipantInfo
(
	LIMSHANDLE handle,
	char *pSessionId,
	u_char *pGroupSessionId
);

/**
 * This API can be called to un-subscribe participant information connected to
 * indicated group chat session ID before ending Group Chat session.
 *
 * @param[in] handle			The lims instance handle.
 * @param[in] pSessionId		Session identifier.
 * @param[in] pGroupSessionId	The ID of the group chat session that should be
 *								affected.
 *
 * @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
 * lims implementation.
 */
u_int32 lims_UnSubscribeToGroupParticipantInfo
(
	LIMSHANDLE handle,
	char *pSessionId,
	u_char *pGroupSessionId
);

/**
* This function is called to to know if given session is group chat session or Not.
*
* @param[in] handle				The lims instance handle.
* @param[in] pSessionId			The ID of the session
* @return BoolEnum				Enum_TRUE if session is GroupChat Session,
* 								Enum_FALSE if session is 1-1 Chat Session
* 								Enum_INVALID if any error.
*/
BoolEnum lims_IsGroupChatSession
(
		LIMSHANDLE handle,
		char *pSessionId
);

/**
* This function is called to get the application data stored in the session.
*
* @param[in] handle				The lims instance handle.
* @param[in] pSessionId			The ID of the session
* @return void*					Pointer stored in the session structure
*/
void * lims_GetFileAppData
(
		LIMSHANDLE handle,
		char *pSessionId
);
#ifdef ENABLE_QCMAPI
/**
* This function is called to trigger ACSRequest.
*
* @param[in] handle				The lims instance handle.
* @param[in] eACSRequestType	lims_ACRTypeEnum.
 * @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
 * lims implementation.
*/
u_int32 lims_TriggerACSRequest
(
	LIMSHANDLE handle,
	lims_ACRTypeEnum eACSRequestType
);

/**
* This function is called to set the network id which will be use to bind the socket.
*
* @param[in] handle				The lims instance handle.
* @param[in] uNetID				network id to set.
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
*/
u_int32 lims_SetNetworkID
(
	LIMSHANDLE handle,
	u_int64 uNetID
);

/**
* This function is called to close the connection with IMS CM Service.
*
* @param[in] handle				The lims instance handle.
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
*/
u_int32 lims_QcmCloseConnection
(
	LIMSHANDLE handle
);

u_int32 lims_GetACSConfiguration
(
	LIMSHANDLE handle
);

#endif

/**
* This function is called to abort any ongoing session.
*
* @param[in] handle				The lims instance handle.
* @param[in] ppSessionId		List of IDs to all active sessions.
* @param[in] uCount				Size of ppSessionId list.
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
*/
u_int32 lims_AbortSession
(
	LIMSHANDLE handle,
	char **ppSessionId,
	u_int32 uCount
);

/**
* This function is called to publish initial or modify the existing capabilities of the user.
*
* @param[in] handle				The lims instance handle.
* @param[in] pETag				The tag associated with previous PUBLISH
* @param[in] userCapabilities	The list of capabilities that need to be published
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
*/
u_int32 lims_PublishCapabilities
(
	LIMSHANDLE handle,
	char *pETag,
	EcrioUCEUserCapabilityInfoStruct *pUserCapabilities
);

/**
* This function is called to delete the existing capabilities of the user.
*
* @param[in] handle				The lims instance handle.
* @param[in] pETag				The tag associated with previous PUBLISH
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
*/
u_int32 lims_RemoveCapabilities
(
	LIMSHANDLE handle,
	char *pETag
);

/**
* This function is called to query the capabilities of an user OR list of users.
*
* @param[in] handle				The lims instance handle.
* @param[in] pQueryCapabilties	Query capability data such as RLS URI, list of contacts, features required
*								to form SUBSCRIBE or OPTION to query the capabilities.
* @param[in] ppQueryId			Id associated with the query. Same Id will be passed in the callback
*								for matching.
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
*/
u_int32 lims_QueryCapabilities
(
	LIMSHANDLE handle,
	EcrioUCEQueryCapabilitiesStruct *pQueryCapabilties,
	u_char** ppQueryId
);

/**
* This function is called to send Privacy Management Commands.
*
* @param[in] handle				The lims instance handle.
* @param[in] pCmd				Pointer to lims_PrivacyManagementCommandsStruct.
* @param[in,out] ppCallId		Pointer to unique call Id.
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
*/
u_int32 lims_SendPrivacyManagementCommands
(
	LIMSHANDLE handle,
	lims_PrivacyManagementCommandsStruct *pCmd,
	char **ppCallId
);

/**
* This function is called to send Spam Report Message.
*
* @param[in] handle				The lims instance handle.
* @param[in] pCmd				Pointer to lims_SpamReportMessage.
* @param[in,out] ppCallId		Pointer to unique call Id.
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
*/
u_int32 lims_SendSpamReportMessage
(
	LIMSHANDLE handle,
	lims_SpamReportMessage *pMsg,
	char **ppCallId
);

/**
* This API can be called to change User-Agent header value for out going Requests/Responses
*
* @param[in] handle			The lims instance handle.
* @param[in] pUserAgent		The string of User-Agent.
*
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
*/
u_int32 lims_SetUserAgent
(
	LIMSHANDLE handle,
	u_char *pUserAgent
);



#ifdef __cplusplus
}
#endif

#endif	// __LIMS_H__
