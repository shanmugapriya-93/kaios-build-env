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

#ifndef _ECRIO_SIG_MGR_INIT_H_
#define  _ECRIO_SIG_MGR_INIT_H_

typedef enum
{
	EcrioSigMgrCallbackRegisteringModule_LIMS,
	EcrioSigMgrCallbackRegisteringModule_SUE,
	EcrioSigMgrCallbackRegisteringModule_SMS,
	EcrioSigMgrCallbackRegisteringModule_MoIP,
	EcrioSigMgrCallbackRegisteringModule_CPM,
	EcrioSigMgrCallbackRegisteringModule_UCE
} EcrioSigMgrCallbackRegisteringModuleEnums;

typedef enum
{
	EcrioSigMgrNetworkState_NotConnected,
	EcrioSigMgrNetworkState_LTEConnected,
	EcrioSigMgrNetworkState_WiFiConnected,
} EcrioSigMgrNetworkStateEnums;

typedef enum
{
	EcrioSigMgrNetworkStatus_Success,							/**< Represents Default bearer connected and VoLTE supported. */
	EcrioSigMgrNetworkStatus_VoLTE_Off,							/**< Represents Default bearer connected but network doesn't support VoLTE feature. */
	EcrioSigMgrNetworkStatus_IPsec_Established,					/**< Represents IPsec SA established. */
	EcrioSigMgrNetworkStatus_IPsec_Failed,						/**< Failed to establish IPsec SA. */
	EcrioSigMgrNetwotkStatus_IPsec_Disconneted,					/**< Represents IPsec SA successfully disconnected. */
	EcrioSigMgrNetworkStatus_IPsec_Lost,						/**< Represents IPsec SA connection lost*/
	EcrioSigMgrNetworkStatus_Dedicated_Bearer_Connected,		/**< Represents Dedicated Bearer setup complete by network*/
	EcrioSigMgrNetworkStatus_Dedicated_Bearer_Disconnected,		/**< Represents Dedicated Bearer disconnected by network*/
	EcrioSigMgrNetworkStatus_Dedicated_Bearer_Failed			/**< Represents Failure while setting up Dedicated Bearer by network*/
} EcrioSigMgrNetworkConnectionStatus;

/**
*This structure is used for setting the network parameters.
*@brief used for setting the network parameters.
*/
typedef struct
{
	u_char *pRemoteIP;					/**< Indicate current remote Ips.*/
	u_char *pLocalIp;					/**< Indicates the local IP. */
	u_int32	uLocalPort;					/**< Indicates the local port used for communication.*/
	u_int32 uRemotePort;				/**< Indicates the remote port used for communication. */
	u_int32 uRemoteTLSPort;				/**< Indicates the remote TSL port used for communication. */
	u_int32 uRemoteClientPort;
	BoolEnum bIsIPv6;
	EcrioSigMgrNetworkConnectionStatus uStatus;
	BoolEnum bIsProxyRouteEnabled;	/**< Conditional variable to use proxy routing or not. */
	u_char* pProxyRouteAddress; /**< Null terminated string holding Proxy server address. */
	u_int32 uProxyRoutePort;	/**< Proxy server port. */
} EcrioSigMgrNetworkInfoStruct;

/**
*This structure is used for setting the signaling parameters.
*@brief used for setting the signaling parameters.
*/
typedef struct
{
	u_char *pDisplayName;						/* will use lims passed pointer. Don't release in SigMgr */
	u_char *pPublicIdentity_msisdn;
	u_char *pPublicIdentity_imsi;
	/** Password - to be used for Authentication.*/
	u_char *pPassword;	// will use lims passed pointer. Don't release in SigMgr
	/** Private user id - to be used  for Authentication.*/
	u_char *pPrivateId;	// will use lims passed pointer. Don't release in SigMgr
	/** local domain - to be used  while constructing Address of record for the local party.*/
	/* Home network domain name */
	u_char *pHomeDomain;	// will use lims passed pointer. Don't release in SigMgr
	/** user agent name - to be used for specifying software version*/
	u_char *pUserAgent;	// will use lims passed pointer. Don't release in SigMgr
	/** expires parameter value for a contact.*/
	u_int32 registrationExpires;
	u_int32 subscriptionExpires;
	u_int32 publishExpires;
	u_int32 sessionExpires;
	u_int32 minSe;
	/** SigMgr uses this domain for constructing Request URI for register request.*/
	u_char *pRegistrarDomain;// will use lims passed pointer. Don't release in SigMgr
	u_int32	uUdpMtu;		/* Maximum message length over UDP */
	BoolEnum bWildCharInContact;
	EcrioSigMgrCustomTimersStruct *pCustomSIPTimers;/**<SIP Timers which can be configured from the application. If not configured default values as mentioned in RFC3261 would be used.*/
	BoolEnum bEnableNoActivityTimer;				/* will send dummy UDP packet to P-CSCF */
	u_int32	uNoActivityTimerInterval;
	EcrioSipAuthAlgorithmEnum eAuthAlgorithm;
	u_int16 numSupportedMethods;								/*Number method Supprted*/
	u_char **ppSupportedMethods;								/*Methods supported*/
	EcrioSigMgrHeaderStruct *pAccessNWInfo;
	BoolEnum bEnableUdp;		/* Flag to enable UDP channel. */
	BoolEnum bEnableTcp;        /* Flag to enable TCP channel. */
	u_char *pPANI;
	u_char *pSecurityVerify;
	u_char *pAssociatedUri;
	u_char *pUriUserPart;
	BoolEnum bTLSEnabled;				/**< TLS enabled */
	BoolEnum bTLSPeerVerification;				/**< TLS Peer verification */
	char *pTLSCertificate;					/**< IP address of the relay server */
} EcrioSigMgrSignalingInfoStruct;

/**
*This structure is used by the upper layer for providing different callback functions.
*@brief used by the upper layer for providing different callback functions.*/
typedef struct
{
	/** status callback function*/
	EcrioSigMgrStatusCallBack statusCallbackFn;
	/** Info callback function*/
	EcrioSigMgrInfoCallback infoCallbackFn;
	EcrioSigMgrGetPropertyCallback getPropertyCallbackFn;
	/** Store the upper layer data and returned back*/
	void *pCallbackData;
} EcrioSigMgrEngineCallbackStruct;

/**
*This structure is used for initializing the signaling manager.
*@brief used  for initializing Signaling Manager.
*/
typedef struct
{
	PALINSTANCE pal;										/** Platform Instance */
	LOGHANDLE pLogEngineHandle;								/** Log engine handle */
	void *pOOMObject;										/* Operator Object Model object */
} EcrioSigMgrInitStruct;

/**
*This structure is used to specify change signaling information at runtime.
*@brief used  for changing run time signaling parameter.
*/
typedef struct
{
	u_int32	uSessionExpires;
	u_int32	uMinSe;
	EcrioSigMgrCustomTimersStruct *pCustomSIPTimers;/**<Pointer to Custom SIP timer structure.*/
} EcrioSigMgrSignalingChangeParamInfoStruct;

/** \brief This function is used for initializing the signaling manager.
 *
 *
 *
 * @pre										None.
 *
 * <b>Memory allocation:</b>				<br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;Memory will be allocated to pSigMgrHandle by signaling manager.
 *
 * @param[in] pSigMgrInitInfo				- Information required for initializing signaling manager.
 * @param[in] pIMContextData				- The context data of IM server to be passed to IM client. If IM client use IP to communicate with server this will be NULL.
 * @param[out] pSigMgrHandle				- Handle to the signaling manager.
 *
 * <b>Transaction:</b>                  <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This function is used to initialize the signaling manager.
 * @return						        One of the pre-defined Error Codes.
 * @note                                Mandatory Structure Member
 * - SigMgrInitInfo
 * - SigMgrHandle
 */
u_int32 EcrioSigMgrInit
(
	EcrioSigMgrInitStruct *pSigMgrInitInfo,
	SIGMGRHANDLE *pSigMgrHandle
);

/** \brief This function is used to de-initialize the signaling manager.
 *
 *
 *
 * @pre										EcrioSigMgrInit() must be call.
 *
 * <b>Memory allocation:</b>				<br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;De-allocates all the memory used by pSigMgrHandle.
 *
 * @param[in] pSigMgrHandle					- Handle to the signaling manager.
 *
 *
 * <b>Transaction:</b>                  <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This function is used to de-initialize the signaling manager.
 * @return						        One of the pre-defined Error Codes.
 * @note                                Mandatory Structure Member
 * - pSigMgrHandler
 */

u_int32 EcrioSigMgrDeInit
(
	SIGMGRHANDLE *pSigMgrHandle
);

/** \brief This function is used to de-initialize different types of signaling manager structures.
 *
 *
 *
 * @pre										EcrioSigMgrInit() must be call.
 *
 * <b>Memory allocation:</b>				<br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;De-allocated memory associated with pStruct.
 *
 * @param[in] sigMgrHandle					- Handle to the signaling manager.
 * @param[in] structType					- Type of structures.
 * @param[in] ppStruct						- Pointer to the structure.
 * @param[in] release						- Flag to indicate whether the base pointer will be released or not.
 *
 * <b>Transaction:</b>						<br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This function is used to de-initialize different types of signaling manager structures.
 *
 * @return									One of the pre-defined Error Codes.
 * @note									Mandatory Structure Member
 * - ppStruct
 * - *ppStruct
 */

u_int32 EcrioSigMgrStructRelease
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrStructTypeEnum structType,
	void **ppStruct,
	BoolEnum release
);

/** \brief This function is used to set the callback functions and data.
*
*
*
* @pre										EcrioSigMgrInit() must be call.
*
* <b>Memory allocation:</b>				<br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;None.
*
* @param[in] sigMgrHandle					- Handle to the signaling manager.
* @param[in] pCallbackStruct				- Callback functions and data.
*
* <b>Transaction:</b>						<br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This function is used to set the signaling parameters.
*
* @return									One of the pre-defined Error Codes.
* @note									Mandatory Structure Member
* - pCallbackStruct
*
*/

u_int32 EcrioSigMgrSetCallbacks
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrEngineCallbackStruct *pCallbackStruct,
	EcrioSigMgrCallbackRegisteringModuleEnums eCallbackRegisteringModule
);

/** \brief This function is used to set the signaling parameters.
*
*
*
* @pre										EcrioSigMgrInit() must be call.
*
* <b>Memory allocation:</b>				<br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;None.
*
* @param[in] sigMgrHandle					- Handle to the signaling manager.
* @param[in] pNetworkInfo					- Network parameters.
*
* <b>Transaction:</b>						<br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This function is used to set the signaling parameters.
*
* @return									One of the pre-defined Error Codes.
* @note									Mandatory Structure Member
* - pNetworkInfo
* - pNetworkInfo->eNetworkState
*
*/

u_int32 EcrioSigMgrSetNetworkParam
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrNetworkStateEnums eNetworkState,
	EcrioSigMgrNetworkInfoStruct *pNetworkInfo
);

/** \brief This function is used to set the signaling parameters.
 *
 *
 *
 * @pre										EcrioSigMgrInit() must be call.
 *
 * <b>Memory allocation:</b>				<br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;None.
 *
 * @param[in] sigMgrHandle					- Handle to the signaling manager.
 * @param[in] pSignalingInfo				- Signaling parameters.
 *
 * <b>Transaction:</b>						<br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This function is used to set the signaling parameters.
 *
 * @return									One of the pre-defined Error Codes.
 * @note									Mandatory Structure Member
 * - pSignalingInfo
 * - pSignalingInfo->pUserId
 * - pSignalingInfo->pHomeDomain
 * - pSignalingInfo->pPrivateId
 *
 */

u_int32 EcrioSigMgrSetSignalingParam
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSignalingInfoStruct *pSignalingInfo
);

u_int32 EcrioSigMgrGetSignalingParam
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSignalingInfoStruct **ppSignalingInfo
);

/** \brief This function is used to set the IP Security parameters.
*
*
*
* @pre										EcrioSigMgrInit() must be call.
*
* <b>Memory allocation:</b>				<br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;None.
*
* @param[in] sigMgrHandle				- Handle to the signaling manager.
* @param[in] pIPSecParams				- IP Security parameters.
*
* <b>Transaction:</b>						<br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This function is used to set the signaling parameters.
*
* @return									One of the pre-defined Error Codes.
*
*/
u_int32 EcrioSigMgrSetIPSecParam
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSipIPsecParamStruct *pIPSecParams
);

/** \brief This function is used to change signaling parameters values at run time.
 *
 *
 *
 * @pre										EcrioSigMgrInit() must be call.
 *
 * <b>Memory allocation:</b>				<br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;None.
 *
 * @param[in] sigMgrHandle					- Handle to the signaling manager.
 * @param[in] pChangeParamInfo				- Change Parameters informations.
 *
 * <b>Transaction:</b>						<br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This function is used to change signaling information.
 *											If local IP is null and local port is 0 then it will remove the
 *                                           previous local IP and close the socket.
 *
 *
 * @return									One of the pre-defined Error Codes.
 * @note									Mandatory Structure Member
 * - sigMgrHandle
 * - pChangeParamInfo
 *
 */

u_int32 EcrioSigMgrChangeSignalingParam
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSignalingChangeParamInfoStruct *pChangeParamInfo
);

u_int32 EcrioSigMgrUpdateSignalingParam
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSignalingInfoStruct *pSignalingInfo
);
#endif /* _ECRIO_SIG_MGR_INIT_H_ */
