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

#ifndef __ECRIOSIPDATATYPES_H__
#define __ECRIOSIPDATATYPES_H__

/* Enumerations */

/** @enum EcrioSipPropertyTypeEnums
* This enumeration defines the various property types that a property obtained via the lims_GetPropertyCallback()
* function can be provided as.
*
* The ‘String’ type implies that the property is in a null-terminated C string format. The ‘Numeric’ type implies
* that the property is a numeric (unsigned integer) value. The ‘Auth’ types implies that the property points to a
* lims_AuthStruct structure.
*/
typedef enum
{
	EcrioSipPropertyType_STRING = 0,					/**< String property value requested. */
	EcrioSipPropertyType_NUMERIC,						/**< Numeric property value requested. */
	EcrioSipPropertyType_AUTH							/**< Authentication property type requested. */
} EcrioSipPropertyTypeEnums;

/** @enum EcrioSipPropertyNameEnums
* This enumeration defines the various property names that a property obtained via the lims_GetPropertyCallback()
* function can be provided as. All these properties are place holder for now. Phase 1 we don't use these properties.
*/
typedef enum
{
	EcrioSipPropertyName_VOLTE_STATUS,					/**< VoLTE setting is a configuration option to enable or disable VoLTE feature. 0 is OFF and 1 is ON. */
	EcrioSipPropertyName_LAST_ACCESSTYPE,				/**< Indicates the last access type which is obtained from modem. */
	EcrioSipPropertyName_LAST_ACCESSINFO_VALUE,			/**< Indicates the last LTE access info value required for P-Access-Network-Info header. It is concatenation of PLMN+TAI+ECI.*/
	EcrioSipPropertyName_ACCESSTYPE,					/**< Indicates the access type which is obtained from modem. */
	EcrioSipPropertyName_ACCESSINFO_VALUE,				/**< Indicates the LTE access info value required for P-Access-Network-Info header. It is concatenation of PLMN+TAI+ECI.*/
	EcrioSipPropertyName_AKAV1,							/**< Indicates the requested authentication response should be AKAv1.*/
	EcrioSipPropertyName_AKAV2							/**< Indicates the requested authentication response should be AKAv2.*/
} EcrioSipPropertyNameEnums;

/** @enum EcrioSipMessageTypeEnum
*
*/
typedef enum
{
	EcrioSipMessageTypeNone	= 0x00000000,

	EcrioSipMessageTypeRegister		= 0x00000002,
	EcrioSipMessageTypeInvite		= 0x00000004,
	EcrioSipMessageTypeAck			= 0x00000008,
	EcrioSipMessageTypeCancel		= 0x00000010,
	EcrioSipMessageTypeBye			= 0x00000020,
	EcrioSipMessageTypeMessage		= 0x00000040,
	EcrioSipMessageTypePrack		= 0x00000080,
	EcrioSipMessageTypeSubscribe	= 0x00000100,
	EcrioSipMessageTypeNotify		= 0x00000200,
	EcrioSipMessageTypeUpdate		= 0x00000400,
	EcrioSipMessageTypePublish		= 0x00000800,

	EcrioSipMessageTypeNotify_2xx	= 0x00001000,
	EcrioSipMessageTypeInvite_1xx	= 0x00002000,
	EcrioSipMessageTypeInvite_2xx	= 0x00004000,
	EcrioSipMessageTypeInvite_3xx	= 0x00008000,
	EcrioSipMessageTypeInvite_4xx	= 0x00010000,
	EcrioSipMessageTypeInvite_6xx	= 0x00020000,
	EcrioSipMessageTypeMessage_2xx	= 0x00040000,
	EcrioSipMessageTypeBye_2xx		= 0x00080000,
	EcrioSipMessageTypePrack_2xx	= 0x00100000,
	EcrioSipMessageTypeUpdate_2xx	= 0x00200000,
	EcrioSipMessageTypeUpdate_4xx   = 0x00400000,

	EcrioSipMessageTypeOptions      = 0x00800000,
	EcrioSipMessageTypeInfo         = 0x01000000,
	EcrioSipMessageTypeRefer        = 0x02000000
} EcrioSipMessageTypeEnum;

/** @enum EcrioSipHeaderTypeEnum
*
*/
typedef enum
{
	EcrioSipHeaderTypeNone = 0,							/**<Specifies no header.*/
	EcrioSipHeaderTypeAllow,							/**<Specifies that the type of the header is "allow".*/
	EcrioSipHeaderTypeAccept,							/**<Specifies that the type of the header is "accept".*/
	EcrioSipHeaderTypeAcceptContact,					/**<Specifies that the type of the header is "Accept-Contact".*/
	EcrioSipHeaderTypeAcceptEncoding,					/**<Specifies that the type of the header is "Accept-Encoding".*/
	EcrioSipHeaderTypeAcceptLanguage,					/**<Specifies that the type of the header is "Accept-Language".*/
	EcrioSipHeaderTypeAllowEvents,						/**<Specifies that the type of the header is "allow events".*/
	EcrioSipHeaderTypeAuthenticationInfo,				/**<Specifies that the type of the header is "authentication info".*/
	EcrioSipHeaderTypeAuthorization,					/**<Specifies that the type of the header is "authorization".*/
	EcrioSipHeaderTypeCallId,							/**<Specifies that the type of the header is "Call Id".*/
	EcrioSipHeaderTypeCSeq,								/**<Specifies that the type of the header is "Cseq".*/
	EcrioSipHeaderTypeContact,							/**<Specifies that the type of the header is "Contact".*/
	EcrioSipHeaderTypeContentEncoding,					/**<Specifies that the type of the header is "Content-Encoding".*/
	EcrioSipHeaderTypeContentLength,					/**<Specifies that the type of the header is "Content Length".*/
	EcrioSipHeaderTypeContentType,						/**<Specifies that the type of the header is "Content Type".*/
	EcrioSipHeaderTypeContentTransferEncoding,			/**<Specifies that the type of the header is "Content Type".*/
	EcrioSipHeaderTypeContentID,						/**<Specifies that the type of the header is "Content-ID".*/
	EcrioSipHeaderTypeContentDisposition,				/**<Specifies that the type of the header is "Content-Disposition".*/
	EcrioSipHeaderTypeContributionID,					/**<Specifies that the type of the header is "Contribution - ID".*/
	EcrioSipHeaderTypeConversationID,					/**<Specifies that the type of the header is "Conversation - ID".*/
	EcrioSipHeaderTypeDate,								/**<Specifies that the type of the header is "Date".*/
	EcrioSipHeaderTypeEvent,							/**<Specifies that the type of the header is "Event".*/
	EcrioSipHeaderTypeExpires,							/**<Specifies that the type of the header is "Expires".*/
	EcrioSipHeaderTypeErrorInfo,						/**<Specifies that the type of the header is "Error Info".*/
	EcrioSipHeaderTypeFrom,								/**<Specifies that the type of the header is "From".*/
	EcrioSipHeaderTypeInReplyToContributionID,			/**<Specifies that the type of the header is "InReplyTo - Contribution - ID".*/
	EcrioSipHeaderTypeMaxForward,						/**<Specifies that the type of the header is "MaxForward".*/
	EcrioSipHeaderTypeMimeVersion,						/**<Specifies that the type of the header is "MimeVersion".*/
	EcrioSipHeaderTypeMinExpires,						/**<Specifies that the type of the header is "Min-Expires".*/
	EcrioSipHeaderTypeMinSE,							/**<Specifies that the type of the header is "Min-SE".*/
	EcrioSipHeaderTypeOrganization,						/**<Specifies that the type of the header is "Organization".*/
	EcrioSipHeaderTypeP_AccessNetworkInfo,				/**<Specifies that the type of the header is "P_accessNetworkInfo".*/
	EcrioSipHeaderTypeP_LastAccessNetworkInfo,			/**<Specifies that the type of the header is "P-Last-Access-Network-Info".*/
	EcrioSipHeaderTypeP_AssertedIdentity,				/**<Specifies that the type of the header is "P_asserted_identity".*/
	EcrioSipHeaderTypeP_PreferredIdentity,				/**<Specifies that the type of the header is "P_preferred_identity".*/
	EcrioSipHeaderTypeP_AssociatedUri,					/**<Specifies that the type of the header is "P_assiciatedUri".*/
	EcrioSipHeaderTypeP_ChargingFunctionAddr,			/**<Specifies that the type of the header is "P_ChargingFunctionAddr".*/
	EcrioSipHeaderTypeP_PreferredService,				/**<Specifies that the type of the header is "P-Preferred-Service".*/
	EcrioSipHeaderTypeP_EarlyMedia,						/**<Specifies that the type of the header is "P-Early-Media".*/
	EcrioSipHeaderTypePath,								/**<Specifies that the type of the header is "Path".*/
	EcrioSipHeaderTypePrivacy,							/**<Specifies that the type of the header is "Privacy".*/
	EcrioSipHeaderTypeProxyRequire,						/**<Specifies that the type of the header is "ProxyRequire".*/
	EcrioSipHeaderTypeRAck,								/**<Specifies that the type of the header is "RAck".*/
	EcrioSipHeaderTypeReason,							/**<Specifies that the type of the header is "Reason".*/
	EcrioSipHeaderTypeReferTo,							/**<Specifies that the type of the header is "Refer-To".*/
	EcrioSipHeaderTypeReferSub,							/**<Specifies that the type of the header is "Refer-Sub".*/
	EcrioSipHeaderTypeRoute,							/**<Specifies that the type of the header is "Route".*/
	EcrioSipHeaderTypeRecordRoute,						/**<Specifies that the type of the header is "Record Route".*/
	EcrioSipHeaderTypeRequire,							/**<Specifies that the type of the header is "Require".*/
	EcrioSipHeaderTypeRetryAfter,						/**<Specifies that the type of the header is "Retry After".*/
	EcrioSipHeaderTypeRequestDisposition,				/**<Specifies that the type of the header is "Retry After".*/
	EcrioSipHeaderTypeRSeq,								/**<Specifies that the type of the header is "RSeq".*/
	EcrioSipHeaderTypeServiceRoute,						/**<Specifies that the type of the header is "Service Route".*/
	EcrioSipHeaderTypeServer,							/**<Specifies that the type of the header is "Server".*/
	EcrioSipHeaderTypeSessionExpires,					/**<Specifies that the type of the header is "Session-Expires".*/
	EcrioSipHeaderTypeSubscriptionState,				/**<Specifies that the type of the header is "Subscription-State".*/
	EcrioSipHeaderTypeTimestamp,						/**<Specifies that the type of the header is "Timestamp".*/
	EcrioSipHeaderTypeTo,								/**<Specifies that the type of the header is "To".*/
	EcrioSipHeaderTypeSupported,						/**<Specifies that the type of the header is "Supported".*/
	EcrioSipHeaderTypeSubject,							/**<Specifies that the type of the header is "Subject".*/
	EcrioSipHeaderTypeSecurityClient,					/**<Specifies that the type of the header is "Security-Client".*/
	EcrioSipHeaderTypeSecurityServer,					/**<Specifies that the type of the header is "Security-Server".*/
	EcrioSipHeaderTypeSecurityVerify,					/**<Specifies that the type of the header is "Security-Verify".*/
	EcrioSipHeaderTypeUnsupported,						/**<Specifies that the type of the header is "Unsupported".*/
	EcrioSipHeaderTypeUserAgent,						/**<Specifies that the type of the header is "User Agent".*/
	EcrioSipHeaderTypeVia,								/**<Specifies that the type of the header is "Via".*/
	EcrioSipHeaderTypeWWWAuthenticate,					/**<Specifies that the type of the header is "WWW-Authenticate".*/
	EcrioSipHeaderTypeWarning,							/**<Specifies that the type of the header is "Warning".*/
	EcrioSipHeaderTypeReferredBy,						/**<Specifies that the type of the header is "Referred-By".*/
	EcrioSipHeaderTypeSipEtag,							/**<Specifies that the type of the header is "SIP-ETag".*/
	EcrioSipHeaderTypeSipIfMatch,						/**<Specifies that the type of the header is "SIP-If-Match".*/
	EcrioSipHeaderTypePcomServiceType,					/**<Specifies that the type of the header is "P-Com.ServiceType".*/	
	EcrioSipHeaderTypePcomE911ServiceType,				/**<Specifies that the type of the header is "P-com.E911ServiceType".*/	
	EcrioSipHeaderTypePriority,				/**<Specifies that the type of the header is "Priority".*/ 
	EcrioSipHeaderTypeHeader_Unknown					/**<Specifies that the type of the header is not known.*/
} EcrioSipHeaderTypeEnum;

/** @enum EcrioSipAuthAlgorithmEnum
*	Enumeration of constants that is used to specify the algorithm used for calculating response.
*/
typedef enum
{
	EcrioSipAuthAlgorithmDefault,							/**< No algorithm is specified. Use Default (MD5) for calculating response.*/
	EcrioSipAuthAlgorithmMD5,								/**< Enumerated constant that represents the MD5 algorithm to be used for calculating response.*/
	EcrioSipAuthAlgorithmAKAv1,							/**< Enumerated constant that represents the AKA algorithm to be used for calculating response.*/
	EcrioSipAuthAlgorithmAKAv2,							/**< Unknown/Unsupported Algorithm is specified. */
	EcrioSipAuthAlgorithmUnknown
} EcrioSipAuthAlgorithmEnum;
/* Structures */

/** @struct EcrioSipAuthStruct
* This structure is provided for properties that return authentication
* credentials. The structure is two-way, it accepts data as input and will
* update the structure with output data. This structure is typically used to
* obtain an AKA response string based on authentication property name.
*/
typedef struct
{
	u_char *pAutn;						/**< Pointer to an Authentication Token. This is input to callee from caller. */
	u_int32 uAutnSize;					/**< The length of Authentication Token. This is input to callee from caller. */
	u_char *pRand;						/**< Pointer to Random Challenge. This is input to callee from caller. */
	u_int32 uRandSize;					/**< The length of Random Challenge. This is input to callee. */
	u_char *pAuts;						/**< Pointer to an re-synchronization Token. This is output to caller from callee. */
	u_int32 uAutsSize;					/**< The length of re-synchronization Token. This is output to caller from callee. */
	u_char *pRes;						/**< Pointer to a Response Code string. This is output to caller from callee. */
	u_int32 uResSize;					/**< The length of the Response Code string. This is output to caller from callee.  */
	u_char *pIK;						/**< Pointer to a session key for integrity check IK string. This is output to caller from callee. */
	u_int32 uIKSize;					/**< The length of the pIK string. This is output to caller from callee. */
	u_char *pCK;						/**< Pointer to a session key for encryption CK string. This is output to caller from callee. */
	u_int32 uCKSize;					/**< The length of the pCK string. This is output to caller from callee. */
} EcrioSipAuthStruct;

/** @struct EcrioSipTimerStruct
*
*/
typedef struct
{
	u_int32 uTimerT1;					/**< retranmit timer value for non_INVITE request and INVITE responses. */
	u_int32 uTimerT2;					/**< The maximum retransmit interval for non-INVITE requests and INVITE responses. */
	u_int32 uTimerF;					/**< non-INVITE transaction timer timer. */
} EcrioSipTimerStruct;

/** @struct EcrioSipSessionTimerStruct
*
*/
typedef struct
{
	u_int32 uTimerSE;						/**< session expirty timer interval.  */
	u_int32 uTimerMinSE;					/**< minimum session expity timer interval. */
} EcrioSipSessionTimerStruct;

/** @enum EcrioSipURITypeEnums
* This enumeration defines the various URI Types
*
*/
typedef enum
{
	EcrioSipURIType_None = 0,					/**< Represents URI type none. */
	EcrioSipURIType_SIP,						/**< Represents SIP URI. */
	EcrioSipURIType_TEL,						/**< Represents TEL URI.. */
	EcrioSipURITypee_ABSOLUTE					/**< Represents ABSOLUTE URI.. */
} EcrioSipURITypeEnums;

/**Enumeration of constants that is used to specify the P-Early-Media states.*/
typedef enum
{
	/**Call object is at Uninitialized state*/
	EcrioSigMgrEarlyMediaStateNone = 0,
	EcrioSigMgrEarlyMediaStateSendrecv,
	EcrioSigMgrEarlyMediaStateSendOnly,
	EcrioSigMgrEarlyMediaStateRecvOnly,
	EcrioSigMgrEarlyMediaStateInactive

}EcrioSigMgrEarlyMediaStateEnum;

/** @enum EcrioSipIPTypeEnum
* This enumeration defines the different IP protocol types, IPv4 and IPv6.
*/
typedef enum
{
	EcrioSipNetwork_IP_Type_V4 = 0,	/**< IPV4 address type. Default value */
	EcrioSipNetwork_IP_Type_V6			/**< IPV6 address type */
} EcrioSipIPTypeEnum;

/** @struct EcrioSipIPsecValueStruct
*
*/
typedef struct
{
	char *pValue; 			/**< Pointer to hold the security paramters. */
} EcrioSipIPsecValueStruct;

/** @struct EcrioSipIPsecParamStruct
*
*/
typedef struct
{
	u_int32						uAlgoCount;		/**< The number of supported auth algorithm.*/
	EcrioSipIPsecValueStruct	**ppAlgo;		/**< The array of supported auth algorithm.*/
	u_int32						uEncryptCount;	/**< The number of supported encrypt algorithm.This member could be 0*/
	EcrioSipIPsecValueStruct	**ppEncrypt;	/**< The array of supported encrypt algorithm.This member could be NULL*/
	u_int32						uSpi_uc;		/**< The local unique security parameter index (spi) for the protected channel created over port_uc.*/
	u_int32						uSpi_us;		/**< The local unique security parameter index (spi) for the protected channel created over port_us.*/
	u_int32						uPort_uc;		/**< The local protected client port.Over UDP transport through this port the device will only send the requests and responses. Over TCP transport through this port the device will send the requests and receive the corresponding responses.*/
	u_int32						uPort_us;		/**< The local protected server port.Over UDP transport on this port the device will only receive the requests and responses. Over TCP transport on this port the device will receive the requests and send the corresponding responses.*/
}EcrioSipIPsecParamStruct;

/** @struct EcrioSipIPsecNegotiatedParamStruct
*
*/
typedef struct
{
	u_char					*pAlgo;			/**< The auth algorithm negotiated from Security-Client and Security-Server sip headers.*/
	u_char					*pEncrypt;		/**< The encrypt algorithm negotiated from Security-Client and Security-Server sip headers.*/
	u_int32					uSpi_uc;		/**< The spi of the UE for the protected channel created over port_c.All incoming IP packet on the port_uc protected by this SA will contain this spi value in ESP header.*/
	u_int32					uSpi_us;		/**< The spi of the UE for the protected channel created over port_s.All incoming IP packet on the port_us protected by this SA will contain this spi value in ESP header.*/
	u_int32					uSpi_pc;		/**< The spi of the P-CSCF for the protected channel created over port_c.All outgoing IP packets towards the port_pc protected by this SA will contain this spi value in ESP header.*/
	u_int32					uSpi_ps;		/**< The spi of the P-CSCF for the protected channel created over port_s.All outgoing IP packets towards the port_ps protected by this SA will contain this spi value in ESP header.*/
	u_int32					uPort_uc;		/**< The client port of the UE. The UE will send requests and responses from this port over UDP transport. The UE will send requests and receive corresponding responses on this port over TCP transport.*/
	u_int32					uPort_us;		/**< The server port of the UE.The UE will receive requests and responses on this port over UDP transport. The UE will receive requests and send corresponding responses on this port over TCP transport.*/
	u_int32					uPort_pc;		/**< The client port of the P-CSCF.The P-CSCF will send requests and responses from this port over UDP transport. The P-CSCF will send requests and receive corresponding responses on this port over TCP transport.*/
	u_int32					uPort_ps;		/**< The server port of the P-CSCF.The P-CSCF will receive requests and responses on this port over UDP transport. The P-CSCF will receive requests and send corresponding responses on this port over TCP transport.*/
	EcrioSipIPTypeEnum		eLocalIPType;	/**< IP Version IPv4 or IPv6.*/
	u_char			  		*pLocalIp;		/**< Indicates the local IP. */
	EcrioSipIPTypeEnum		eRemoteIPType;	/**< IP Version IPv4 or IPv6.*/
	u_char			  		*pRemoteIp;		/**< Indicates the local IP. */
	u_char			  		*pIK;			/**< Pointer to a session key for integrity check IK string. This is output to caller from callee. */
	u_int32			  		uIKSize;		/**< The length of the pIK string. This is output to caller from callee. */
	u_char			  		*pCK;			/**< Pointer to a session key for encryption CK string. This is output to caller from callee. */
	u_int32			  		uCKSize;		/**< The length of the pCK string. This is output to caller from callee. */
}EcrioSipIPsecNegotiatedParamStruct;

#endif /* #ifndef __ECRIOSIPDATATYPES_H__ */
