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
 * @file EcrioSUEEngine.h
 * @brief This is the header file for the public interface of SUE Engine.
 *
 */

#ifndef __ECRIOSUEENGINE_H__
#define __ECRIOSUEENGINE_H__

/* Allow this module to be called directly from C++ compiled code. */
#ifdef __cplusplus
extern "C" {
#endif

/* Always reference and use the Ecrio data type definitions. */
#include "EcrioDataTypes.h"
#include "EcrioSUEDataTypes.h"

/** \brief SUE Engine handle.
 *
 */
typedef void *SUEENGINEHANDLE;			/**< SUE Engine handle */

/** @name Error Codes
 * @brief List of Error codes that can be returned by the SUE Engine API.
 */
/*@{*/

#define ECRIO_SUE_NO_ERROR						(0x00)	/**< No error, API success. */
#define ECRIO_SUE_INSUFFICIENT_DATA_ERROR		(0x01)	/**< Data is not provided in the input parameter. */
#define ECRIO_SUE_INVALID_INPUT_ERROR			(0x02)	/**< Provided input parameter is not the valid one in the current state. */
#define ECRIO_SUE_INCONSISTENT_DATA_ERROR		(0x03)	/**< The value of an internal data is different from the expected one in the current state. */
#define ECRIO_SUE_INSUFFICIENT_MEMORY_ERROR		(0x04)	/**< Memory allocation failed. */
#define ECRIO_SUE_PAL_ERROR						(0x05)	/**< Some PAL operation is failed. */
#define ECRIO_SUE_INVALID_XML_SCHEMA_ERROR		(0x06)	/**< The xml violates the schema. */
#define ECRIO_SUE_XML_PARSER_ERROR				(0x07)	/**< The xml is syntactically incorrect. */
#define ECRIO_SUE_UA_ENGINE_ERROR				(0x08)	/**< Error occured in core sip signaling module */
#define ECRIO_SUE_INVALID_OPERATION_ERROR		(0x09)	/**< The operation is not permissible in the current state. */
#define ECRIO_SUE_NO_IMMEDIATE_EXIT_ERROR		(0x0a)	/**< Deregistration process is initiated by EcrioSUEDeregister() and EcrioSUEDeinit() should be called after the complition of deregistration for gracefull termination. */
#define ECRIO_SUE_REGISTRATION_FAILED_ERROR		(0x0b)	/**< The registration to IMS network is failed. */
#define ECRIO_SUE_PLATMAN_ERROR					(0x0c)	/**< Some PlatMan related operation failed. */
#define ECRIO_SUE_PAL_SOCKET_ERROR				(0x0d)  /**< Error from Socket PAL APIs. */
#define ECRIO_SUE_TLS_SOCKET_ERROR				(0x0e)  /**< Error from Socket PAL APIs. */
/*@}*/

typedef enum
{
	EcrioSUENetworkState_NotConnected,
	EcrioSUENetworkState_LTEConnected,
} EcrioSUENetworkStateEnums;

/** \enum EcrioSUEExitReasonEnum
* @brief This enum set indicates the reason to terminate SUE.
*/
typedef enum
{
	ECRIO_SUE_EXIT_REASON_ENUM_None = 0,			/**< Default value during initialization state. */
	ECRIO_SUE_EXIT_REASON_ENUM_Deregistered,		/**< SUE is de-registered in effect to EcrioSUEDeregister() API call. */
	ECRIO_SUE_EXIT_REASON_ENUM_RegistrationFailed,
	ECRIO_SUE_EXIT_REASON_ENUM_SubscriptionFailed	/**< SUE is failed to register and further retry is not possible. */
} EcrioSUEExitReasonEnum;

/** \enum EcrioSUENotificationEnum
* @brief This enum set specifies the exact type of information being passed in a particular notification to the integration layer.
*/
typedef enum
{
	ECRIO_SUE_NOTIFICATION_ENUM_None = 0,						/**< Default value during initialization state. */
	ECRIO_SUE_NOTIFICATION_ENUM_RegisterResponse,				/**< The notification contains the information regarding the received register response. The notification data should be typecasted to EcrioSUENotifyRegisterResponseStruct. */
	ECRIO_SUE_NOTIFICATION_ENUM_DeregisterResponse,				/**< The notification contains the information regarding the received de-register response. The notification data should be typecasted to EcrioSUENotifyRegisterResponseStruct. */
	ECRIO_SUE_NOTIFICATION_ENUM_SubscribeResponse,				/**< The notification contains the information regarding the received subscribe response. The notification data should be typecasted to EcrioSUENotifySubscribeResponseStruct. */
	ECRIO_SUE_NOTIFICATION_ENUM_NotifyRequest,					/**< The notification contains the information regarding the received notify request. The notification data should be typecasted to EcrioSUENotifyNotifyRequestStruct. */
	ECRIO_SUE_NOTIFICATION_ENUM_ExitSUE,						/**< The notification indicates that the integration layer should call EcrioSUEDeinit() to terminate SUE. The notification data should be typecasted to EcrioSUENotifySUEExitReasonStruct. It provides the reason for the termination, e.g. SUE is deregistered or SUE  is failed to register. */
	ECRIO_SUE_NOTIFICATION_ENUM_EstablishIPSECSA,				/**< The notification indicates that the integration layer should notify the application to establish the secure IPSec SA channel. */
	ECRIO_SUE_NOTIFICATION_ENUM_SocketError
} EcrioSUENotificationEnum;

/** \enum EcrioSUEStructEnum
 * @brief This enum set indicate the particular structure which is used in notification to the integration layer. These enums are also passed to EcrioSUEStructInit() or EcrioSUEStructRelease() to identify the structure.
 */
typedef enum
{
	ECRIO_SUE_STRUCT_ENUM_None = 0,											/**< default value during initialization state. */
	ECRIO_SUE_STRUCT_ENUM_EcrioSUECallbackStruct,							/**< To represent the structure EcrioSUECallbackFnStruct. */
	ECRIO_SUE_STRUCT_ENUM_EcrioSUEParamStruct,								/**< To represent the structure EcrioSUEParamStruct. */
	ECRIO_SUE_STRUCT_ENUM_EcrioSUENetworkInfoStruct,						/**< To represent the structure EcrioSUENetworkInfoStruct. */
	ECRIO_SUE_STRUCT_ENUM_EcrioSUENotifyNotifyRequestStruct,				/**< To represent the structure EcrioSUENotifyNotifyRequestStruct. */
	ECRIO_SUE_STRUCT_ENUM_EcrioSUENotifyRegisterResponseStruct,				/**< To represent the structure EcrioSUENotifyRegisterResponseStruct. */
	ECRIO_SUE_STRUCT_ENUM_EcrioSUENotifySubscribeResponseStruct,			/**< To represent the structure EcrioSUENotifySubscribeResponseStruct. */
	ECRIO_SUE_STRUCT_ENUM_EcrioSUENotifyStruct								/**< To represent the structure EcrioSUENotifyStruct. */
} EcrioSUEStructEnum;

/** \enum EcrioSUENetworkConnectionStatus
* @brief This enum indicate the network connection status set from the integration layer.
*/
typedef enum
{
	ECRIO_SUE_Network_Status_Success,							/**< Represents Default bearer connected and VoLTE supported. */
	ECRIO_SUE_Network_Status_VoLTE_Off,							/**< Represents Default bearer connected but network doesn't support VoLTE feature. */
	ECRIO_SUE_Network_Status_IPsec_Established,					/**< Represents IPsec SA established. */
	ECRIO_SUE_Network_Status_IPsec_Failed,						/**< Failed to establish IPsec SA. */
	ECRIO_SUE_Netwotk_Status_IPsec_Disconneted,					/**< Represents IPsec SA successfully disconnected. */
	ECRIO_SUE_Network_Status_IPsec_Lost,						/**< Represents IPsec SA connection lost*/
	ECRIO_SUE_Network_Status_Dedicated_Bearer_Connected,		/**< Represents Dedicated Bearer setup complete by network*/
	ECRIO_SUE_Network_Status_Dedicated_Bearer_Disconnected,		/**< Represents Dedicated Bearer disconnected by network*/
	ECRIO_SUE_Network_Status_Dedicated_Bearer_Failed			/**< Represents Failure while setting up Dedicated Bearer by network*/
} EcrioSUENetworkConnectionStatus;

/** \struct EcrioSUEParamStruct
 * @brief This structure is used to populate the initialization time parameters and passed to EcrioSUEInit() API.
 */
typedef struct
{
	PALINSTANCE pal;
	void *pLogHandle;
	void *pOOMObject;
	BoolEnum bSubscribeRegEvent;
	BoolEnum bUnSubscribeRegEvent;
} EcrioSUEParamStruct;

typedef struct
{
	u_char **ppRemoteIPs;					/**< array of remote Ips.*/
	u_char uNoOfRemoteIps;					/**< number of remote Ips present in the array.*/
	BoolEnum bIsIPv6;						/**< Enum_TRUE for IPv6 and Enum_FALSE for IPv4.*/
	u_char *pLocalIp;						/**< Indicates the local IP. */
	u_int32	uLocalPort;						/**< Indicates the local port used for communication.*/
	u_int32 uRemotePort;					/**< Indicates the remote port used for communication. */
	u_int32 uRemoteTLSPort;					/**< Indicates the remote TSL port used for communication. */
	u_int32 uRemoteClientPort;
	EcrioSUENetworkConnectionStatus uStatus;/**< Indicates the network connection status. */
	BoolEnum bIsProxyRouteEnabled;	/**< Conditional variable to use proxy routing or not. */
	u_char* pProxyRouteAddress; /**< Null terminated string holding Proxy server address. */
	u_int32 uProxyRoutePort;	/**< Proxy server port. */
} EcrioSUENetworkInfoStruct;

/** \struct EcrioSUEParamStruct
* @brief This structure contains the details of the parameter name-value pair.
*/
typedef struct
{
	/** Parameter Name*/
	u_char *pParamName;
	/** Parameter Value*/
	u_char *pParamValue;
} EcrioSUESipParamStruct;

/** \struct EcrioSUENotifySubscribeResponseStruct
* @brief This structure contains the details of the received SUBSCRIBE response.
*/
typedef struct
{
	u_int32 uRspCode;					/**< The sip response code of the SUBSCRIBE response. */
	u_int32 uExpire;					/**< Expire value. */
} EcrioSUENotifySubscribeResponseStruct;

/** \struct EcrioSUENotifyStruct
* @brief This structure is passed in the notification callback to update the integration layer about certain asynchronous event.
*/
typedef struct
{
	EcrioSUENotificationEnum eNotificationType;						/**< This identifies the notification type. Based on this notification type one of the following members will be valid. */
	union
	{
		EcrioSUENotifyRegisterResponseStruct *pRegisterResponse;	/**< This member will be valid if the eNotificationType is ECRIO_SUE_NOTIFICATION_ENUM_NotifyIMSSignallingInformation. */
		EcrioSUENotifyRegisterResponseStruct *pDeregisterResponse;	/**< This member will be valid if the eNotificationType is ECRIO_SUE_NOTIFICATION_ENUM_NotifyIMSSignallingInformation. */
		EcrioSUENotifySubscribeResponseStruct *pSubscribeResponse;	/**< This member will be valid if the eNotificationType is ECRIO_SUE_NOTIFICATION_ENUM_NotifyIMSSignallingInformation. */
		EcrioSUENotifyNotifyRequestStruct *pNotifyRequest;			/**< This member will be valid if the eNotificationType is ECRIO_SUE_NOTIFICATION_ENUM_NotifyIMSSignallingInformation. */
		void *pIPSecNotification;									/**< This member will be valid if the eNotificationType is ECRIO_SUE_NOTIFICATION_ENUM_EstablishIPSECSA */
		EcrioSUEExitReasonEnum eExitReason;							/**< The reason to terminate SUE. It could be SUE is deregistered or SUE failed to register and further retry is not possible. */
	} u;
} EcrioSUENotifyStruct;

/** @name Callback Functions
 * \n Callback functions must be implemented by the Integration Layer
 * and are invoked in response to various actions and events. The only callback
 * functions supported by this module is: NotifySUECallbackType.
 */
/*@{*/

/**
 * This CallBack API is invoked by the SUE to notify the integration layer of any asynchronous events. The events could be related to connections with
 * the Bearer Network and with the SAC or could be related to SIP messages received from the network, expiry of Timers or any error during
 * operations.
 * The integration layer must implement this callback API and provided to the EcrioSUEInit() function by function pointer.
 *
 * @param[in] pCallbackData			Pointer to any optional data provided by the
 *									calling layer, opaque to the SUE Engine. This pointer
 *									would have been provided to the SUE Engine during
 *									initialization.
 * @param[in] pNotificationData		A void pointer to the associated data structure
 *									containing the details of the notification.
 * @return None.
 */
typedef void (*NotifySUECallbackType)
(
	void *pCallbackData,
	void *pNotificationData
);

/*@}*/

/** \struct EcrioSUECallbackStruct
 * @brief This structure contains the details of the callback function and the callback data those the integration layer provides to SUE during initialization. SUE will call the function pointed by pNotifySUECallbackFn and will provide the pCallbackData.
 */
typedef struct
{
	NotifySUECallbackType pNotifySUECallbackFn;		/**< Callback for notifying the asynchronous event. */
	void *pCallbackData;							/**< Void pointer to some opaque data that should always be communicated back to the calling layer. */
} EcrioSUECallbackStruct;

/**
 * This function is used to initialize the SUE Engine. This is a synchronous
 * function which should be called first before any other SUE API
 * function is called. This function will perform basic checks of input
 * parameters and perform basic component initialization. This function
 * returns the SUE Handle as an output parameter which points to the
 * complete internal state of the SUE.
 *
 * @param[in] pSUEParam				Pointer to a structure that holds the initial
 *									configuration for the SUE. This structure
 *									should be populated by the calling
 *									function. If the calling function allocates this pointer can free the
 *									memory associated with this pointer after
 *									this function returns.
 * @param[in] pCallbackStruct		Pointer to a structure holding the callback
 *									information used to communicate notification information
 *									to the calling layer. This structure should be
 *									populated by the calling function. If the calling function
 *									allocates this pointer can free the memory associated
 *									with this pointer after this function returns.
 * @param[out] ppSUEHandle			The SUE Handle an output parameter. This is
 *									a pointer to which the SUE will allocate
 *									memory. If the referenced pointer is not NULL,
 *									an error will be returned. The calling layer should
 *									never free the memory associated with the handle
 *									since it is needed when invoking any other SUE
 *									API. The EcrioSUEDeinit() should be called to free
 *									the memory.
 * @return                          ECRIO_SUE_NO_ERROR if successful otherwise a
 *									non-zero value indicating the reason for failure.
 */
u_int32 EcrioSUEInit
(
	EcrioSUEParamStruct *pSUEParam,
	EcrioSUECallbackStruct *pCallbackStruct,
	void *hSigMgrHandle,
	SUEENGINEHANDLE *ppSUEHandle
);

u_int32 EcrioSUESetNetworkParam
(
	SUEENGINEHANDLE pSUEHandle,
	EcrioSUENetworkStateEnums eNetworkState,
	EcrioSUENetworkInfoStruct *pNetworkInfo
);

/**
* This function is used to initiate registration procedure to the IMS network.
* This is a function should be called after EcrioSUESetNetworkParam()
* is called if network is connected. If the network is connected this function sends REGISTER request with non-0 expires.
*
* @param[in] pSUEHandle			The SUE Handle that was created by calling the
*									EcrioSUEInit() function. If NULL, the function
*									will return an error.
* @return                           ECRIO_SUE_NO_IMMEDIATE_EXIT_ERROR if de-registration procedure is initiated. ECRIO_SUE_NO_ERROR if
*									de-registration is not initiated but no error occured otherwise a
*									non-zero value indicating the reason for failure.
*/
u_int32 EcrioSUERegister
(
	SUEENGINEHANDLE	pSUEHandle,
	u_int16	numFeatureTagParams,
	EcrioSUESipParamStruct **ppFeatureTagParams
);

/**
* This function is used to initiate subscription to reg-event procedure to the IMS network.
* This is a function should be called after EcrioSUERegister().
* If the network is connected this function sends SUBSCRIBE request with non-0 expires.
*
* @param[in] pSUEHandle			The SUE Handle that was created by calling the
*									EcrioSUEInit() function. If NULL, the function
*									will return an error.
* @return                           ECRIO_SUE_NO_ERROR if no error occured otherwise a
*									non-zero value indicating the reason for failure.
*/
u_int32 EcrioSUESubscribe
(
	SUEENGINEHANDLE pSUEHandle
);

/**
* This function is used to initiate un-subscribe to reg-event procedure to the IMS network before the deregistration and termination of
* SUE Engine. This is a function should be called before EcrioSUEDeregister()
* is called. If the network is connected, UE is registered and has active subscription to reg-event this function sends SUBSCRIBE request
* with expires 0. The integration layer can call EcrioSUEDeregister() before reciving the response of sent SUBSCRIBE request.
*
* @param[in] pSUEHandle			The SUE Handle that was created by calling the
*									EcrioSUEInit() function. If NULL, the function
*									will return an error.
* @return                          ECRIO_SUE_NO_IMMEDIATE_EXIT_ERROR if de-registration procedure is initiated. ECRIO_SUE_NO_ERROR if
*									de-registration is not initiated but no error occured otherwise a
*									non-zero value indicating the reason for failure.
*/
u_int32 EcrioSUEUnsubscribe
(
	SUEENGINEHANDLE pSUEHandle
);

/**
 * This function is used to initiate de-registration procedure to the IMS network before the termination of SUE Engine.
 * This is a function should be called before EcrioSUEDeinit()
 * is called. If the network is connected this function sends REGISTER request with expires 0 and returns the ECRIO_SUE_NO_IMMEDIATE_EXIT_ERROR.
 * The integration layer should not call EcrioSUEDeinit() immediately if it gets the ECRIO_SUE_NO_IMMEDIATE_EXIT_ERROR, instead the integration layer
 * should wait for the ECRIO_SUE_NOTIFICATION_ENUM_ExitSUE notification. Once the de-registration transaction completes SUE notifies
 * the integration layer with ECRIO_SUE_NOTIFICATION_ENUM_ExitSUE. At this time the integration layer should call the EcrioSUEDeinit() API. However, the integration
 * layer should not call EcrioSUEDeinit() from this notification thread.
 * If network is not connected then EcrioSUEDeregister() returns with ECRIO_SUE_NO_ERROR error code. The integration layer can call EcrioSUEDeinit() API.
 *
 * @param[in] pSUEHandle			The SUE Handle that was created by calling the
 *									EcrioSUEInit() function. If NULL, the function
 *									will return an error.
 * @return                          ECRIO_SUE_NO_IMMEDIATE_EXIT_ERROR if de-registration procedure is initiated. ECRIO_SUE_NO_ERROR if
 *									de-registration is not initiated but no error occured otherwise a
 *									non-zero value indicating the reason for failure.
 */
u_int32 EcrioSUEDeregister
(
	SUEENGINEHANDLE pSUEHandle
);

/**
 * This function is used to de-initialize the SUE. This function will clean
 * up all resources and free all memory used by the SUE. This is a synchronous
 * function and can be called at any time. Calling this function without initializing
 * the SUE Handle using the EcrioSUEInit() API could result in unexpected
 * behavior.
 *
 * @param[in,out] ppSUEHandle		Pointer to the SUE Handle. If the referenced
 *									pointer is NULL, then the function will return
 *									successfully. If not NULL, an attempt will be
 *									made to clean up resources and memory.
 * @return                          ECRIO_SUE_NO_ERROR if successful otherwise a
 *									non-zero value indicating the reason of failure.
 */
u_int32 EcrioSUEDeinit
(
	SUEENGINEHANDLE *ppSUEHandle
);

/**
 * A common API that can be used to initialize the members of the various
 * data structures defined by the SUE. It is best advised to intialize an
 * SUE data structure prior to its usage to avoid unwanted behavior.
 *
 * @param[in,out]	pData			Pointer to the structure to be initailized.
 *								    This pointer will be de-referenced as per
 *								    the structure type above to obtain the
 *								    actual structure pointer before initializing.
 * @param[in]       eDataType		Identify the structure to be initailized by
 *                                  providing the appropriate enum.
 * @return                          ECRIO_SUE_NO_ERROR
 */
u_int32 EcrioSUEStructInit
(
	void *pData,
	EcrioSUEStructEnum eDataType
);

/**
 * A common API that can be used to de-intialize and free the members of the
 * different data structures defined by the SUE. It can also be used to free
 * the data structure itself.
 *
 * @param[in,out]	ppData			Double Pointer to the structure to be freed.
 *								    This pointer will be de-referenced as per
 *								    the structure type above to obtain the
 *								    actual structure pointer before releasing.
 * @param[in]       eDataType		Identify the structure to be released by
 *                                  providing the appropriate enum.
 * @param[in]       bReleaseParent	Release only the structure members if set to
 *								    Enum_FALSE. Release even the base structure
 *								    pointer if set to Enum_TRUE.
 * @return                          ECRIO_SUE_NO_ERROR
 */
u_int32 EcrioSUEStructRelease
(
	void **ppData,
	EcrioSUEStructEnum eDataType,
	BoolEnum bReleaseParent
);

#ifdef ENABLE_QCMAPI
void EcrioSUESetRegistrationState
(
	SUEENGINEHANDLE	pSUEHandle,
	unsigned int eRegistrationState
);

void EcrioSUESetRegistrationStateToNone
(
	SUEENGINEHANDLE	pSUEHandle
);

void EcrioSUESetRegistrationStateToRegistering
(
	SUEENGINEHANDLE	pSUEHandle
);

u_int32 EcrioSUESetFeatureTagParams
(
	SUEENGINEHANDLE	pSUEHandle,
	u_int16	numFeatureTagParams,
	EcrioSUESipParamStruct **ppFeatureTagParams
);
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __ECRIOSUEENGINE_H__ */
