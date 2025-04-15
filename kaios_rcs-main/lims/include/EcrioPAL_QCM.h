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
 * @file EcrioPAL_QCM.h
 * @brief This is the interface of the Ecrio PAL's Qualcomm Connection Manager.
 *
 */

#ifndef __ECRIOPAL_QCM_H__
#define __ECRIOPAL_QCM_H__

/* Include common PAL definitions for building. */
#include "EcrioPAL_Common.h"

/* Include the Ecrio PAL Logging Module definition as a direct dependency. */
#include "EcrioPAL_Log.h"

/* Allow this module to be called directly from C++ compiled code. */
#ifdef __cplusplus
extern "C" {
#endif

/** @name Module Handles
 * \n Handles are void pointers which will point to structures known
 * only internally to the Module. The Calling Component should only
 * reference the returned handles by these defined types. The handles
 * supported by this module include:
 */
/*@{*/

/**
 * A definition for the QCM instance, for use internally by the PAL.
 */
typedef void *QCMINSTANCE;

/** @enum ConnectionEvents
 * Enumeration holding the information of QCM connection event.
 */
typedef enum
{
	QCMConnectionEvent_NotRegistered = 0,
	QCMConnectionEvent_Registered,
	QCMConnectionEvent_Allowed,
	QCMConnectionEvent_NotAllowed,
	QCMConnectionEvent_ForcefulClose,
	QCMConnectionEvent_TerminateConnection,
	QCMConnectionEvent_Created,
	QCMConnectionEvent_GetACSConfiguration,
	QCMConnectionEvent_AutoconfigRequestStatus,
	QCMConnectionEvent_ServiceDied,
	QCMConnectionEvent_ReadyCreateConnection,
	QCMConnectionEvent_Unknown
} QCMConnectionEvents;

/** @enum ConnectionEvents
 * Enumeration holding the information of connection event.
 */
typedef enum
{
	ConnectionEvent_NotRegistered = 0,
	ConnectionEvent_Registered,
	ConnectionEvent_Allowed,
	ConnectionEvent_NotAllowed,
	ConnectionEvent_ForcefulClose,
	ConnectionEvent_TerminateConnection,
	ConnectionEvent_Created,
	ConnectionEvent_GetACSConfiguration,
	ConnectionEvent_AutoconfigRequestStatus,
	ConnectionEvent_GetConfiguration,
	ConnectionEvent_Unknown
} ConnectionEvents;

/** @enum ConfigurationTypes
 * Enumeration holding the information of configuration type.
 */
typedef enum
{
	ConfigurationType_User = 0,
	ConfigurationType_Device,
	ConfigurationType_Auto
} ConfigurationTypes;

/** @enum ACRTypes
 * Enumeration holding the information of Autoconfiguration trigger reason types.
 */
typedef enum
{
	ACRType_UserRequest = 0,
	ACRType_RefreshToken,
	ACRType_InvalidToken,
	ACRType_InvalidCredential,
	ACRType_ClientChange,
	ACRType_DeviceUpgrde,
	ACRType_FactoryReset
} ACRTypes;

/** @enum ProtocolTypes
 * Enumeration holding the information of protocol type.
 */
typedef enum
{
	IPProtocolType_UDP = 0,
	IPProtocolType_TCP
} IPProtocolTypes;

/** @enum MessgeTypes
 * Enumeration holding the information of message type.
 */
typedef enum
{
	MessageType_Request = 0,
	MessageType_Response
} MessageTypes;

/** @enum IPTypes
 * Enumeration holding the information of message type.
 */
typedef enum
{
	IPType_IPv4 = 1,
	IPType_IPv6
} IPTypes;

/** @enum APIVersions
 * Enumeration holding the information of QCM Version.
 */
typedef enum
{
	APIVersion_20 = 0,
	APIVersion_21,
	APIVersion_22,
	APIVersion_None
} APIVersions;

/** @struct QCMConfigStruct
 * This is the structure for the ims configuration from QCM module.
 */
typedef struct
{
	BoolEnum bUEBehindNAT;				/**< Indicates whether the UE is behind NAT. */
	BoolEnum bIPSecEnabled;				/**< Indicates whether IPSec is enabled. */
	BoolEnum bCompactFormEnabled;		/**< Indicates whether Compact Form is enabled. */
	BoolEnum bKeepAliveStatusEnabled;	/**< Indicates whether Keep Alive is enabled. */
	BoolEnum bGruuEnabled;				/**< Indicates whether GRUU is enabled. */
	u_char *pSipOutBoundProxyName;		/**< Outbound SIP proxy name/IP. */
	u_int32 uSipOutBoundProxyPort;		/**< Outbound SIP proxy port. */
	u_int32 uPCSCFClientPort;			/**< P-CSCF client port. */
	u_int32 uPCSCFServerPort;			/**< P-CSCF server port. */
	u_char *pAuthChallenge;				/**< Authentication header. */
	u_char *pArrNC;				/**< Nonce count. */
	u_char *pServiceRoute;				/**< Service route value. */
	u_char *pSecurityVerify;			/**< Security verify value. */
	u_int32 uPCSCFOldSAClientPort;		/**< IPSec old SA P-CSCF client port. */
	u_char *pTCPThresholdValue;			/**< Configured TCP threshold value for SIP. */
	u_char *pPANI;			/**<  PANI header value. */
	u_char *pPATH;			/**< Path header value from IMS registration. */
	u_char *pUriUserPart;			/**< URI user part in the contact header. */
	u_char *pPLANI;			/**< PLANI header value. */
	u_char *pPpa;			/** < P-Preferred-Association header value. */
	u_char *pPidentifier;		/** < PIDENTIFIER value. */
	u_int32 uUEClientPort;				/**< UE client port. */
	u_int32 uUEServerPort;				/**< UE server port. */
	u_char *pAssociatedUri;				/**< Asociated URI value. */
	u_char *pUEPublicIPAddress;			/**< Received UE public IP address. */
	u_int32 uUEPublicPort;				/**< UE public IP port. */
	u_char *pSipPublicUserId;			/**< User public ID. */
	u_char *pSipPrivateUserId;			/**< User private ID. */
	u_char *pSipHomeDomain;				/**< Home domain address. */
	u_char *pUEPubGruu;					/**< UE public GRUU. */
	u_char *pLocalHostIPAddress;		/**< UE public IP address. */
	u_int32 uIPType;					/**< UE IP type. */
	u_char *pIMEI;						/**< UE IMEI value. */
	u_int32 uUEOldSAClientPort;			/**< IPSec old SA UE client port. */
} QCMConfigStruct;

/** @struct QcmACSConfigStruct
 * This is the structure for the AutoConfig request type.
 */
typedef struct
{
	u_int32 uType;					/**< If it is 0, SERVER_UPDATE. If it is 1, CLIENT_REQUEST. */
	u_char *pConfig;
} QcmACSConfigStruct;

/** @struct QcmReceiveStruct
 * This is the structure for the received data and associated fields.
 */
typedef struct
{
	QCMINSTANCE handle;			/**< Pointer to the Qcm instance. */
	u_char *pReceiveData;		/**< Pointer to a byte buffer containing the received data. */
	u_int32 receiveLength;		/**< The length of the data received that the pCallbackFnData buffer points to. */
	const u_char *pFromIP;			/**< Points to an received address. */
	u_int32 result;				/**< A result code indicating the validity of the received data or some other error. */
} QcmReceiveStruct;

/** @name Callback Functions
 * \n Callback functions must be implemented by the Calling Component and are
 * invoked in response to various actions and events. The callback functions
 * supported by this module include:
 */
/*@{*/

/**
 * This callback function will be triggered whenever a conection event is arrived.
 *
 * @param[in] pCallbackFnData	Pointer to opaque data that the Calling
 *								Component will use.
 * @return None.
 */
typedef void (*QcmEventCallback)
(
	void *pCallbackFnData,
	ConnectionEvents uEvent,
	void *pData
);

/**
 * This callback function will be triggered whenever data is received for
 * a incoming channel.
 *
 * @param[in] pCallbackFnData	Pointer to opaque data that the Calling
 *								Component will use. This pointer was provided
 *								by the Calling Component in the call to
 *								pal_QcmSetCallback(). This is typically
 *								used to point to the instance of the Calling
 *								Component that is utilizing the QCM
 *								Module. This pointer should not be freed by
 *								the Calling Component, because it will still
 *								be used for other expiration events.
 * @param[in] pReceive			Pointer to a structure for the received data
 *								and associated fields.
 * @return None.
 */
typedef void (*QcmReceiveCallback)
(
	void *pCallbackFnData,
	QcmReceiveStruct *pReceive
);

/**
 * This callback function will be triggered mainly for significant errors
 * detected by a outgoing channel.
 *
 * @param[in] pal				Pointer to the PAL instance.
 *								This pointer should not be freed by the
 *								Calling Component.
 * @param[in] pCallbackFnData	Pointer to opaque data that the Calling
 *								Component will use. This pointer was provided
 *								by the Calling Component in the call to
 *								pal_QcmSetCallback(). This is typically
 *								used to point to the instance of the Calling
 *								Component that is utilizing the QCM
 *								Module. This pointer should not be freed by
 *								the Calling Component, because it will still
 *								be used for other expiration events.
 * @param[in] error				An error code indicating the type of error
 *								experienced. It will correspond to any of the
 *								error codes defined by EcrioPALErrors
 *								enumeration.
 * @return None.
 */
typedef void (*QcmErrorCallback)
(
	PALINSTANCE pal,
	void *pCallbackFnData,
	u_int32 error
);

/*@}*/

/** @struct QcmCallbackFnStruct
 * This structure is used by the Calling Component to provide all the
 * required callback function pointers to the QCM Module via
 * the pal_QcmSetCallback() function for a QCM instance.
 */
typedef struct
{
	QcmReceiveCallback pReceiveCallbackFn;	/**< Pointer to a callback function to be called when packets are received. */
	QcmErrorCallback pErrorCallbackFn;		/**< Pointer to a callback function to be called when a outgoing error is detected. */
	void *pCallbackFnData;						/**< Pointer to opaque data that the Calling Component will use. This is sent with each callback notification. */
} QcmCallbackFnStruct;

/**
 * This function initializes the QCM instance.
 *
 * @param[in] pal				An optional PAL handle which can be used
 *								to get access to a shared context.
 * @param[in] logHandle			An optional Ecrio Log handle.
 * @param[out] instance			The instance pointer for the QCM or NULL.
 * @return KPALErrorNone is successful, otherwise a specific error code.
 */
u_int32 pal_QcmInit
(
	PALINSTANCE pal,
	LOGHANDLE logHandle,
	QCMINSTANCE *instance,
	u_char *pIccid,
//	u_char *pUrl,
	QcmEventCallback pEventCallbackFn,
	void *pCallbackFnData,
	BoolEnum bEnableGlobalMutex
);

/**
 * This function deinitializes the QCM instance.
 *
 * @param[in] instance			The QCM instance handle to deinitialize.
 * @return None.
 */
void pal_QcmDeinit
(
	QCMINSTANCE instance
);

/**
 * This function is used to set callback function.
 *
 * @param[in] pal
 * @param[in] pFnStruct			Pointer to a QcmCallbackFnStruct structure
 *								holding callback function pointers that the
 *								Calling Component wishes to use. This pointer
 *								must not be NULL, otherwise an error is
 *								returned.
 * @return KPALErrorNone is successful, otherwise a specific error code.
 */
u_int32 pal_QcmSetCallback
(
	PALINSTANCE pal,
	QcmCallbackFnStruct *pFnStruct,
	QCMINSTANCE *instance
);

/**
 * This function sends a data packet using the QCM module.
 *
 * @param[in] pal
 * @param[in] pDataBuffer		Pointer to a buffer of byte data to send over
 *								the currently open socket. If the pointer is
 *								NULL, the dataLength parameter must be 0, or
 *								else an error is returned. The function will
 *								not free the memory pDataBuffer points to. The
 *								Calling Component is responsible for doing so
 *								upon return.
 * @param[in] dataLength		The length of data to be sent. This represents
 *								the length of the data pointed to by the
 *								pDataBuffer parameter and can be 0, whether or
 *								not the pDataBuffer is NULL.
 * @param[in] pProxy			Message destination. For SIP requests, an 
 * 								outbound proxy address can be specified; for 
 * 								SIP responses, a header address can be used.
 * @param[in] pCallId			Refers to the SIP request/response call ID 
 * 								value as per SIP RFC	3261.
 * @return KPALErrorNone is successful, otherwise a specific error code.
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
);

/**
 * This function is used to trigger registration.
 *
 * @param[in] pal
 * @param[in] pUrl			Pointer to service tags, example for 
 *							"+g.oma.sip-im";"+g.3gpp.cs-voice";"+g.3gpp.iari-ref=
 *							"urn%3Aurn-7%3A3gpp-application.ims.iari.gsma-is"";
 *							"+g.3gpp.iari-ref="urn%3Aurn-7%3A3gpp-application.ims.
 *							iari.rcse.ft"""
 * @return KPALErrorNone is successful, otherwise a specific error code.
 */
u_int32 pal_QcmRegister
(
	PALINSTANCE pal,
	u_char *pFeatures
);

/**
 * This function is used to trigger deregistration.
 *
 * @param[in] pal
 * @return KPALErrorNone is successful, otherwise a specific error code.
 */
u_int32 pal_QcmDeregister
(
	PALINSTANCE pal
);

/**
 * This function is used to query the configuration 
 *
 * @param[in] pal
 * @param[in] pStruct			Pointer to a QCMConfigStruct structure.
 * @return KPALErrorNone is successful, otherwise a specific error code.
 */
u_int32 pal_QcmGetConfig
(
	PALINSTANCE pal,
	ConfigurationTypes type
);

/**
 * This function is used to close a transactions.
 *
 * @param[in] pal
 * @param[in] pCallId			Refers to the SIP request/response call ID 
 * 								value as per SIP RFC	3261.
 * @return KPALErrorNone is successful, otherwise a specific error code.
 */
u_int32 pal_QcmCloseTransation
(
	PALINSTANCE pal,
	u_char *pCallId
);

/**
 * This function is used to close all transactions.
 *
 * @param[in] pal
 * @return KPALErrorNone is successful, otherwise a specific error code.
 */
u_int32 pal_QcmCloseAllTransations
(
	PALINSTANCE pal
);

/**
 * This function is used to traggier ACS Request
 *
 * @param[in] pal
 * @return KPALErrorNone is successful, otherwise a specific error code.
 */
u_int32 pal_QcmTriggerACSRequest
(
	PALINSTANCE pal,
	ACRTypes type
);

/**
 * This function is used to close Connection
 *
 * @param[in] pal
 * @return KPALErrorNone is successful, otherwise a specific error code.
 */
u_int32 pal_QcmCloseConnection
(
	PALINSTANCE pal
);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __ECRIOPAL_QCM_H__ */
