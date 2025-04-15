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
* @file EcrioSDP.h
* @brief This is the header file for the public interface of the SDP module.
*/

#ifndef __ECRIOSDP_H__
#define __ECRIOSDP_H__

/* Include data types header file */
#include "EcrioDataTypes.h"
#include "EcrioMediaDataTypes.h"

/* Allow this module to be called directly from C++ compiled code. */
#ifdef __cplusplus
extern "C" {
#endif

/** @name Constant Number Definitions
 *
 */
#define MAX_MEDIA										1	/**< Maximum number of media types. */
#define MAX_PAYLOAD										16	/**< Maximum number of payloads. */
#define MAX_IPADDR_LENGTH								40	/**< Maximum length of IP address. */
#define MAX_USERNAME_LENGTH								64	/**< Maximum length of user name. */
#define MAX_STRINGS										1024/**< Maximum number of buffer length which allocated for each session. This is used in EcrioSDPSessionCreate() function, see the description. */

/** @name Status Codes
 *
 */
#define ECRIO_SDP_NO_ERROR								(0x0000)		/**< Indicates success when an API is called. */
#define ECRIO_SDP_INTERNAL_ERROR						(0x0001)		/**< Indicates internal error occured when an API is called. */
#define ECRIO_SDP_INSUFFICIENT_DATA_ERROR				(0x0002)		/**< Indicates parameters not passed proprly when an API is called. */
#define ECRIO_SDP_MEMORY_ERROR							(0x0003)		/**< Indicates memory handling error occured when an API is called. */
#define ECRIO_SDP_WRONG_STATE_ERROR						(0x0004)		/**< Indicates state error which conflict between a specified state and internal state occured when an API is called. */
#define ECRIO_SDP_CODEC_NOT_MATCHED_ERROR				(0x0005)		/**< Indicates codec not matched error occured when an API is called. */

#define ECRIO_SDP_PARSING_INSUFFICIENT_ERROR			(0x0101)		/**< Indicates parsing error caused by insufficient things in line. */
#define ECRIO_SDP_PARSING_A_LINE_ERROR					(0x0102)		/**< Indicates parsing error in SDP Attributes ("a=") line. */
#define ECRIO_SDP_PARSING_B_LINE_ERROR					(0x0103)		/**< Indicates parsing error in SDP bandwidth ("b=") line. */
#define ECRIO_SDP_PARSING_C_LINE_ERROR					(0x0104)		/**< Indicates parsing error in SDP Connection Data ("c=") line. */
#define ECRIO_SDP_PARSING_M_LINE_ERROR					(0x0105)		/**< Indicates parsing error in SDP Media Descriptions ("m=") line. */
#define ECRIO_SDP_PARSING_O_LINE_ERROR					(0x0106)		/**< Indicates parsing error in SDP Origin ("o=") line. */
#define ECRIO_SDP_PARSING_A_RTPMAP_LINE_ERROR			(0x0107)		/**< Indicates parsing error in RTP payload ("a=rtpmap") media attribute line. */
#define ECRIO_SDP_PARSING_A_FMTP_LINE_ERROR				(0x0108)		/**< Indicates parsing error in particular format ("a=fmtp") media attribute line. */
#define ECRIO_SDP_PARSING_AMR_PARAMETER_ERROR			(0x0109)		/**< Indicates parsing error in AMR parameters. */
#define ECRIO_SDP_PARSING_A_ACCEPT_TYPES_LINE_ERROR		(0x010A)		/**< Indicates parsing error in accept types ("a=accept-types") media attribute line. */
#define ECRIO_SDP_PARSING_A_FILE_SELECTOR_LINE_ERROR	(0x010B)		/**< Indicates parsing error in file-selector ("a=file-selector") media attribute line. */
#define ECRIO_SDP_PARSING_A_FILE_DATE_LINE_ERROR		(0x010C)		/**< Indicates parsing error in file-date ("a=file-date") media attribute line. */
#define ECRIO_SDP_PARSING_A_FILE_RANGE_LINE_ERROR		(0x010D)		/**< Indicates parsing error in file-range ("a=file-range") media attribute line. */
#define ECRIO_SDP_PARSING_A_PRECOND_CURRENT_LINE_ERROR	(0x010E)		/**< Indicates parsing error in current status ("a=curr") or confirm status ("a=conf") media attribute line. */
#define ECRIO_SDP_PARSING_A_PRECOND_DESIRED_LINE_ERROR	(0x010F)		/**< Indicates parsing error in desired status ("a=des") media attribute line. */
#define ECRIO_SDP_PARSING_A_FINGERPRINT_LINE_ERROR	    (0x0200)		/**< Indicates parsing error in desired status ("a=fingerprint") media attribute line. */

/**
 * The overall SDP instance handle. There should only be one.
 */
typedef void *SDPHANDLE;

/**
 * A specific session instance, of which there could be many.
 */
typedef void *SDPSESSIONHANDLE;

/** @enum EcrioSDPAddressTypeEnum
 * This enumeration defines the address types, which is IPv4 and IPv6.
 */
typedef enum
{
	ECRIO_SDP_ADDRESS_TYPE_NONE = 0,				/**< No address type defined. */
	ECRIO_SDP_ADDRESS_TYPE_IP4,						/**< Address type IP4. */
	ECRIO_SDP_ADDRESS_TYPE_IP6						/**< Address type IP6. */
} EcrioSDPAddressTypeEnum;

/** @enum EcrioSDPBandwidthModifierEnum
 * This enumeration defines the bandwidth types which can add to Bandwidth
 * ("b=") field. It is used as a bitmask in the definition for an adding
 * Bandwidth ("b=") line into the SDP with a provided corresponding values.
 */
typedef enum
{
	ECRIO_SDP_BANDWIDTH_MODIFIER_NONE	= 0x00,		/**< No bandwidth modifier defined. */
	ECRIO_SDP_BANDWIDTH_MODIFIER_AS		= 0x01,		/**< Application specific. */
	ECRIO_SDP_BANDWIDTH_MODIFIER_RS		= 0x02,		/**< RTCP sender. */
	ECRIO_SDP_BANDWIDTH_MODIFIER_RR		= 0x04		/**< RTCP receiver. */
} EcrioSDPBandwidthModifierEnum;

/** @enum EcrioSDPMediaTypeEnum
 * This enumeration defines the media types in the Media Descriptions.
 */
typedef enum
{
	ECRIO_SDP_MEDIA_TYPE_NONE = 0,					/**< No media type defined. */
	ECRIO_SDP_MEDIA_TYPE_AUDIO,						/**< Media type audio. */
	ECRIO_SDP_MEDIA_TYPE_MSRP,						/**< Media type MSRP. */
} EcrioSDPMediaTypeEnum;

/** @enum EcrioSDPMediaProtocolEnum
 * This enumeration defines the transport protocol in the Media Descriptions.
 */
typedef enum
{
	ECRIO_SDP_MEDIA_PROTOCOL_NONE = 0,				/**< No transport protocol defined. */
	ECRIO_SDP_MEDIA_PROTOCOL_RTP_AVP,				/**< RTP Profile for audio and video. */
	ECRIO_SDP_MEDIA_PROTOCOL_TCP_MSRP,				/**< MSRP over TCP. */
	ECRIO_SDP_MEDIA_PROTOCOL_TLS_MSRP,				/**< MSRP over TCP for TLS. */
} EcrioSDPMediaProtocolEnum;

/** @enum EcrioSDPMediaDirectionEnum
 * This enumeration defines the media direction what mode should be started,
 * which indicated in a Media Attribute.
 */
typedef enum
{
	ECRIO_SDP_MEDIA_DIRECTION_NONE = 0,				/**< No media direction defined. */
	ECRIO_SDP_MEDIA_DIRECTION_SENDONLY,				/**< Media is started in send-only mode. */
	ECRIO_SDP_MEDIA_DIRECTION_RECVONLY,				/**< Media is started in recv-only mode. */
	ECRIO_SDP_MEDIA_DIRECTION_INACTIVE,				/**< Media is started in inactive mode. */
	ECRIO_SDP_MEDIA_DIRECTION_SENDRECV				/**< Media is started in send and recv mode. */
} EcrioSDPMediaDirectionEnum;

/** @enum EcrioSDPShownAMRParameterEnum
 * This enumeration defines the bits which can be specified in the uOpaque field,
 * this means for showing AMR parameters in the fmtp attribute. If each bits are
 * on, it means their parameter exists in fmtp line even if parameter value is 0.
 */
typedef enum
{
	ECRIO_SDP_SHOWN_AMR_PARAMETER_NONE						= 0x00,		/**< No AMR parameter defined. */
	ECRIO_SDP_SHOWN_AMR_PARAMETER_MODESET					= 0x01,		/**< Showing mode-set parameter. */
	ECRIO_SDP_SHOWN_AMR_PARAMETER_OCTET_ALIGN				= 0x02,		/**< Showing octet-align parameter. */
	ECRIO_SDP_SHOWN_AMR_PARAMETER_MODE_CHANGE_PERIOD		= 0x04,		/**< Showing mode-change-period parameter. */
	ECRIO_SDP_SHOWN_AMR_PARAMETER_MODE_CHANGE_CAPABILITY	= 0x08,		/**< Showing mode-change-capability parameter. */
	ECRIO_SDP_SHOWN_AMR_PARAMETER_MODE_CHANGE_NEIGHBOR		= 0x10,		/**< Showing mode-change-neighbor parameter. */
	ECRIO_SDP_SHOWN_AMR_PARAMETER_CRC						= 0x20,		/**< Showing crc parameter. */
	ECRIO_SDP_SHOWN_AMR_PARAMETER_ROBUST_SORTING			= 0x40,		/**< Showing robust-sorting parameter. */
	ECRIO_SDP_SHOWN_AMR_PARAMETER_MAX_RED					= 0x80		/**< Showing max-red parameter. */
} EcrioSDPShownAMRParameterEnum;

/** @enum EcrioSDPEventTypeEnum
 * This enumeration defines the different types of events what the specified
 * SDP means.
 */
typedef enum
{
	ECRIO_SDP_EVENT_NONE = 0,						/**< No SDP event defined. */
	ECRIO_SDP_EVENT_INITIAL_OFFER,					/**< This SDP means an initial offer. */
	ECRIO_SDP_EVENT_INITIAL_ANSWER,					/**< This SDP means an initial answer. */
	ECRIO_SDP_EVENT_UPDATE_OFFER,					/**< This SDP means an update offer for precondition. */
	ECRIO_SDP_EVENT_UPDATE_ANSWER,					/**< This SDP means an update answer for precondition. */
	ECRIO_SDP_EVENT_IDENTICAL,						/**< This SDP means an identical to the previous SDP. */
	ECRIO_SDP_EVENT_HOLD,							/**< This SDP means session hold event. */
	ECRIO_SDP_EVENT_UNHOLD,							/**< This SDP means session unhold event. */
	ECRIO_SDP_EVENT_INACTIVE,						/**< This SDP means session held on hold event. */
	ECRIO_SDP_EVENT_CONSECUTIVE_FILE_TRANSER,		/**< This SDP means consecutive file transfer event. */
	ECRIO_SDP_EVENT_IP_ADDRESS_CHANGED,				/**< This SDP means to notify that IP address has changed. */
} EcrioSDPEventTypeEnum;

/** @enum EcrioSDPMSRPMediaTypesEnum
 * This enumeration defines the media types that are acceptable to
 * the endpoint in MSRP media session.
 */
typedef enum
{
	ECRIO_SDP_MSRP_MEDIA_TYPE_NONE					= 0x00000000,	/**< No media type defined. */
	ECRIO_SDP_MSRP_MEDIA_TYPE_TEXT_PLAIN			= 0x00000001,	/**< "text/plain" */
	ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_CPIM			= 0x00000002,	/**< "message/cpim" */
	ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_IMDN_XML		= 0x00000004,	/**< "message/imdn+xml" */
	ECRIO_SDP_MSRP_MEDIA_TYPE_IM_ISCOMPOSING_XML	= 0x00000008,	/**< "application/im-iscomposing+xml" */
	ECRIO_SDP_MSRP_MEDIA_TYPE_MULTIPART_MIXED		= 0x00000010,	/**< "multipart/mixed" */
	ECRIO_SDP_MSRP_MEDIA_TYPE_HTTP_FILE_TRANSFER	= 0x00000020,	/**< "application/vnd.gsma.rcs-ft-http+xml" */
	ECRIO_SDP_MSRP_MEDIA_TYPE_PUSH_LOCATION			= 0x00000040,	/**< "application/vnd.gsma.rcspushlocation+xml" */
	ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_REVOKE		= 0x00000080,	/**< "application/vnd.gsma.rcsrevoke+xml" */
	ECRIO_SDP_MSRP_MEDIA_TYPE_PRIVACY_MANAGEMENT	= 0x00000100,	/**< "application/vnd.gsma.rcsalias-mgmt+xml" */
	ECRIO_SDP_MSRP_MEDIA_TYPE_LINK_REPORT			= 0x00000200,	/**< "application/vnd.gsma.rcslink-report+xml" */
	ECRIO_SDP_MSRP_MEDIA_TYPE_SPAM_REPORT			= 0x00000400,	/**< "application/vnd.gsma.rcsspam-report+xml" */
	ECRIO_SDP_MSRP_MEDIA_TYPE_RICHCARD				= 0x00000800,	/**< "application/vnd.gsma.botmessage.v1.0+json" */
	ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTED_CHIPLIST	= 0x00001000,	/**< "application/vnd.gsma.botsuggestion.v1.0+json" */
	ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTION_RESPONSE	= 0x00002000,	/**< "application/vnd.gsma.botsuggestion.response.v1.0+json" */
	ECRIO_SDP_MSRP_MEDIA_TYPE_SHARED_CLIENT_DATA	= 0x00004000,	/**< "application/vnd.gsma.botsharedclientdata.v1.0+json" */
	ECRIO_SDP_MSRP_MEDIA_TYPE_GROUPDATA				= 0x00008000,	/**< "application/vnd.oma.cpm-groupdata+xml" */
	ECRIO_SDP_MSRP_MEDIA_TYPE_GENERAL				= 0x00010000,	/**< This is specified if none of the above applies. When specifying an arbitrary media type, the character string indicated in pType of the file descriptor is used. */
	ECRIO_SDP_MSRP_MEDIA_TYPE_CONFERENCE			= 0x00020000,	/**< "application/conference-info+xml" */
	ECRIO_SDP_MSRP_MEDIA_TYPE_WILDCARD				= 0xFFFFFFFF	/**< "*" which indicates that the sender may attempt to send content with media-types that have not been explicitly listed. */
} EcrioSDPMSRPMediaTypesEnum;

/** @enum EcrioSDPMSRPAttributeSetupEnum
 * This enumeration defines the media direction which endpoint will
 * create the MSRP transport connection towards the other UA in
 * "a=setup" attribute.
 */
typedef enum
{
	ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_NONE = 0,		/**< No media direction defined. */
	ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_ACTIVE,			/**< "active" attribute value. */
	ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_PASSIVE,			/**< "passive" attribute value. */
	ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_ACTPASS,			/**< "actpass" attribute value. */
	ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_HOLDCONN			/**< "holdconn" attribute value, but MSRP UA must not send with this attribute, and it must ignore. */
} EcrioSDPMSRPAttributeSetupEnum;

/** @enum EcrioSDPPreconditionStateEnum
 * This enumeration defines to indicate the current precondition state.
 */
typedef enum
{
	ECRIO_SDP_PRECONDITION_STATE_NONE = 0,					/**< No precondition state defined. */
	ECRIO_SDP_PRECONDITION_STATE_INITIATE,					/**< Initiate precondition. */
	ECRIO_SDP_PRECONDITION_STATE_NEGOTIATING,				/**< Precondition ongoing. */
	ECRIO_SDP_PRECONDITION_STATE_MEET						/**< Finished precondition exchange. */
} EcrioSDPPreconditionStateEnum;

/** @enum EcrioSDPPreconditionTypeEnum
 * This enumeration defines the precondition type.
 */
typedef enum
{
	ECRIO_SDP_PRECONDITION_TYPE_INVALID = 0,				/**< No precondition type defined. */
	ECRIO_SDP_PRECONDITION_TYPE_E2E,						/**< end-to-end status. */
	ECRIO_SDP_PRECONDITION_TYPE_SEGMENTED,					/**< segmented status. */
} EcrioSDPPreconditionTypeEnum;

/** @enum EcrioSDPPreconditionStrengthEnum
 * This enumeration defines the precondition strength whether or not the callee
 * can be alerted, in case the network fails to meet the preconditions.
 */
typedef enum
{
	ECRIO_SDP_PRECONDITION_STRENGTH_INVALID = 0,			/**< No strength defined. */
	ECRIO_SDP_PRECONDITION_STRENGTH_FAILURE,				/**< "failure" */
	ECRIO_SDP_PRECONDITION_STRENGTH_UNKNOWN,				/**< "unknown" */
	ECRIO_SDP_PRECONDITION_STRENGTH_NONE,					/**< "none" */
	ECRIO_SDP_PRECONDITION_STRENGTH_OPTIONAL,				/**< "optional" */
	ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY,				/**< "mandatory" */
} EcrioSDPPreconditionStrengthEnum;

/** @enum EcrioSDPPreconditionDirectionEnum
 * This enumeration defines a precondition direction in which a particular
 * attribute (current, desired or confirmation status) is applicable to.
 */
typedef enum
{
	ECRIO_SDP_PRECONDITION_DIRECTION_INVALID = 0,			/**< No direction defined. */
	ECRIO_SDP_PRECONDITION_DIRECTION_NONE,					/**< "none" */
	ECRIO_SDP_PRECONDITION_DIRECTION_SEND,					/**< "send" */
	ECRIO_SDP_PRECONDITION_DIRECTION_RECV,					/**< "recv" */
	ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV,				/**< "sendrecv" */
} EcrioSDPPreconditionDirectionEnum;

/** @struct EcrioSDPStringStruct
 * This structure is used to indicate a buffer to hold entire strings
 * in the EcrioSDPStreamStruct. For each character strings, pData will
 * allocate by the original SDP buffer size because entire string buffers
 * won't be any larger than it. This structure must allocate by function
 * caller and SDP library uses to hold the result of parsing any charactor
 * strings, then the function caller must care this structure.
 */
typedef struct
{
	u_char *pData;									/** Pointer to an allocated buffer. */
	u_int32	uContainerSize;							/** Size of the allocated memory. */
	u_int32	uSize;									/** Current size of the data in the buffer. */
} EcrioSDPStringStruct;

/** @struct EcrioSDPOriginStruct
 * This structure is used to compose the originator of the session,
 * identifier and version number ("o=").
 */
typedef struct
{
	u_char *pUsername;								/**< User name. */
	u_int64	uSessionid;								/**< Session ID. This number is used for numeric string. */
	u_int64	uVersion;								/**< Version. This number is used for numeric string. */
	EcrioSDPAddressTypeEnum	eAddressType;			/**< Enumeration constant that indicates the address type. */
	u_char *pAddress;								/**< The IP address of the host, accommodates both IPv4 and IPv6 addresses. */
} EcrioSDPOriginStruct;

/** @struct EcrioSDPConnectionInfomationStruct
 * This structure is used to compose the Connection Data ("c=").
 */
typedef struct
{
	EcrioSDPAddressTypeEnum	eAddressType;			/**< Enumeration constant that indicates the address type. */
	u_char *pAddress;								/**< The IP address of the host, accommodates both IPv4 and IPv6 addresses. */
} EcrioSDPConnectionInfomationStruct;

/** @struct EcrioSDPBandwidthStruct
 * This structure is used to compose the Bandwidth ("b="). This field will
 * appear when a specific bitmask indicated by uModifier is enabled, even if
 * its value set to 0.
 */
typedef struct
{
	u_int8 uModifier;								/**< Bitmask that indicates the bandwidth modifier. */
	u_int32	uAS;									/**< Number of Application specific bandwidth value. */
	u_int32	uRS;									/**< Number of RTCP sender bandwidth value. */
	u_int32	uRR;									/**< Number of RTCP receiver bandwidth value. */
} EcrioSDPBandwidthStruct;

/** @struct EcrioSDPAudioPayloadStruct
 * This structure is used to compose an audio payload information in the
 * Media Attribution ("a=").
 */
typedef struct
{
	u_int32	uClockrate;								/**< Clock rate. */
	u_int8 uChannels;								/**< Number of channels if any. */
	BoolEnum bFlag;									/**< General purpose flag. */
	u_int32	uOpaque;								/**< An opaque value for the given codec. For AMR codecs, a bitmask which parameters shown in fmtp attribute defined from EcrioSDPShownAMRParameterEnum. */
	u_int32	uSettings;								/**< Bitmask of settings for the given codec to configure locally. For AMR codecs, made up of one or more values from EcrioAudioCodecAMREnum. */
} EcrioSDPAudioPayloadStruct;

/** @struct EcrioSDPAudioAttributeStruct
 * This structure is used to compose an audio specific attribute information
 * in the Media Attribution ("a=").
 */
typedef struct
{
	u_int16	uPtime;									/**< The length of time in milliseconds represented by the media in a packet. If this value is 0, this attribute will ignore. */
	u_int16	uMaxPtime;								/**< The maximum amount of media that can be encapsulated in each packet, expressed as time in milliseconds. If this value is 0, this attribute will ignore. */
} EcrioSDPAudioAttributeStruct;

/** @struct EcrioSDPMSRPFileSelectorStruct
 * This structure is used to compose a "file-selector" attribute information
 * in the Media Attribution ("a=").
 */
typedef struct
{
	u_char *pName;									/**< "name" selector. */
	u_int32	uSize;									/**< "size" selector. */
	u_char *pType;									/**< "type" selector. */
	u_char *pHash;									/**< "hash" selector. */
} EcrioSDPMSRPFileSelectorStruct;

/** @struct EcrioSDPMSRPFileDateStruct
 * This structure is used to compose a "file-date" attribute information
 * in the Media Attribution ("a="). Each parameters should be specified
 * a plain-text style.
 */
typedef struct
{
	u_char *pCreation;								/**< "creation" parameter. */
	u_char *pModification;							/**< "modification" parameter. */
	u_char *pRead;									/**< "read" parameter. */
} EcrioSDPMSRPFileDateStruct;

/** @struct EcrioSDPMSRPFileRangeStruct
 * This structure is used to compose a "file-range" attribute information
 * in the Media Attribution ("a=").
 */
typedef struct
{
	u_int32	uStart;									/**< The octet position of start offset, will be greater than 0. If set to 0, then it will ignore a file-lange attribute. */
	u_int32	uStop;									/**< The octer position of stop offset, will be greater than 0. If set to 0, then it will set "*" which means if the total size of the file is not known in advance. */
} EcrioSDPMSRPFileRangeStruct;

/** @struct EcrioSDPMSRPAttributeStruct
 * This structure is used to compose a MSRP specific attribute information
 * in the Media Attribution ("a=").
 */
typedef struct
{
	EcrioSDPMSRPMediaTypesEnum eAcceptTypes;				/**< "accept-types" attribute. It is possible to specify multiple types by bitmask. */
	EcrioSDPMSRPMediaTypesEnum eAcceptWrappedTypes;			/**< "accept-wrapped-types" attribute. It is possible to specify multiple types by bitmask. */
	u_char *pPath;											/**< "path" attribute. */
	EcrioSDPMSRPAttributeSetupEnum eSetup;					/**< "setup" attribute. */
	EcrioSDPMSRPFileSelectorStruct fileSelector;			/**< "file-selector" attribute. */
	u_char *pFileDisposition;								/**< "file-disposition" attribute. */
	u_char *pFileTransferId;								/**< "file-transfer-id" attribute. */
	EcrioSDPMSRPFileDateStruct fileDate;					/**< "file-date" attribute. */
	u_char *pFileIcon;										/**< "file-icon" attribute. */
	EcrioSDPMSRPFileRangeStruct	fileRange;					/**< "file-range" attribute. */
	BoolEnum bCEMA;											/**< A flag to include "msrp-cema" attribute. */
	BoolEnum bIsClosed;										/**< A flag to indicate closed group chat session. */
	u_char* pFingerPrint;									 /**<"fingerprint" attribute. */
} EcrioSDPMSRPAttributeStruct;

/** @struct EcrioSDPPreconditionStatusDetailStruct
 * This structure is used to compose the detail of precondition status.
 */
typedef struct
{
	EcrioSDPPreconditionStrengthEnum strength;				/**< Strength tag. */
	EcrioSDPPreconditionDirectionEnum direction;			/**< Direction tag. */
} EcrioSDPPreconditionStatusDetailStruct;

/** @struct EcrioSDPPreconditionStatusStruct
 * This structure is used to compose the precondition status in the Media
 * Attribution ("a=").
 */
typedef struct
{
	EcrioSDPPreconditionStatusDetailStruct curr;			/**< "curr" attribute to indicate the current-status. */
	EcrioSDPPreconditionStatusDetailStruct des;				/**< "des" attribute to indicate the desired-status. */
	EcrioSDPPreconditionStatusDetailStruct conf;			/**< "conf" attribute to indicate the confirm-status. */
} EcrioSDPPreconditionStatusStruct;

/** @struct EcrioSDPPreconditionStruct
 * This structure is used to compose a precondition information in the Media
 * Attribution ("a=").
 */
typedef struct
{
	EcrioSDPPreconditionTypeEnum type;						/**< Precondition type which of E2E or segmented (local and remote) to use this session. */
	EcrioSDPPreconditionStatusStruct e2e;					/**< This member will be valid if precondition type is end-to-end. */
	EcrioSDPPreconditionStatusStruct local;					/**< This member will be valid if precondition type is local. */
	EcrioSDPPreconditionStatusStruct remote;				/**< This member will be valid if precondition type is remote. */
} EcrioSDPPreconditionStruct;

/** @struct EcrioSDPPayloadStruct
 * This structure is used to compose RTP payload type in the Media Attribution
 * ("a=").
 */
typedef struct
{
	u_int8 uType;									/**< Payload type in decimal value. */
	EcrioAudioCodecEnum	eCodec;						/**< The selected (negotiated) codec to use. */
	union
	{
		EcrioSDPAudioPayloadStruct audio;			/**< This member will be valid if media type is audio. */
	} u;
} EcrioSDPPayloadStruct;

/** @struct EcrioSDPStreamStruct
 * This structure is used to compose the Media Descriptions ("m="). This is
 * also containing a subset of stream information, especially media attribution
 * and bandwidth information.
 */
typedef struct
{
	EcrioSDPMediaTypeEnum eMediaType;							/**< Enumeration that indicates the media type. */
	u_int16	uMediaPort;											/**< Media port. */
	EcrioSDPMediaProtocolEnum eProtocol;						/**< Enumeration that indicates the transport protocol. */
	u_int8 uNumOfPayloads;										/**< Number of payload types. */
	EcrioSDPPayloadStruct payload[MAX_PAYLOAD];					/**< The structure containing the payload information. */

	EcrioSDPBandwidthStruct	bandwidth;							/**< The structure containing the bandwidth information. */
	EcrioSDPConnectionInfomationStruct conn;					/**< The structure containing the connection information. */
	EcrioSDPMediaDirectionEnum eDirection;						/**< Enumeration that indicates the media direction. */
	EcrioSDPPreconditionStruct precond;							/**< The structure containing the precondition information. */
	union
	{
		EcrioSDPAudioAttributeStruct audio;						/**< This member will be valid if media type is audio. */
		EcrioSDPMSRPAttributeStruct	msrp;						/**< This member will be valid if media type is MSRP. */
	} u;
} EcrioSDPStreamStruct;

/** @struct EcrioSDPSessionStruct
 * This structure defines an information of SDP contents.
 */
typedef struct
{
	EcrioSDPOriginStruct origin;							/**< The structure containing the originator information. */
	EcrioSDPConnectionInfomationStruct conn;				/**< The structure containing the connection information. */
	EcrioSDPBandwidthStruct	bandwidth;						/**< The structure containing the bandwidth information. */
	u_int8 uNumOfMedia;										/**< Number of media information. */
	EcrioSDPStreamStruct stream[MAX_MEDIA];					/**< The structure containing the stream information. */
	EcrioSDPStringStruct strings;							/**< The structure containing the memory information. */
} EcrioSDPSessionStruct;

/** @struct EcrioSDPInformationStruct
 * This structure defines the SDP information. This structure is used for
 * generating SDP strings and for storing stream information from remote end.
 *
 * When generate the SDP string, the API caller may be fill the event type
 * which means this SDP and data related to the event (if any). In case of
 * handling SDP offer-answer, this field will indicate what event is desired
 * by remote end.
 *
 * Regarding a stream structure, it will be used for notifying stream information
 * according to the situation. When SDP session status is "remote-offer" which
 * received an initial offer from remote end, it will be filled all of stream
 * information which supports by remote end. When SDP session status is
 * "negotiated" which in both cases of creating/handling SDP offer-answer, it
 * will be filled only negotiated stream information. Possibly a stream structure
 * have some undefined-length charactor strings, then a strings structure will
 * allocate as string buffer pool. The upper layer must be allocate properly
 * buffer and set pointer, contain size to a strings structure.
 */
typedef struct
{
	EcrioSDPEventTypeEnum eEvent;							/**< Enumeration of event type. */
	EcrioSDPPreconditionStateEnum ePrecondState;			/**< Enumeration of precondition state. */

	// Add definition in here when added new event parameters...
	// e.g.) precondition, upgrading call...

	EcrioSDPConnectionInfomationStruct localIp;				/**< Local IP address information. This is used for connection information which changed local IP address event. */
	EcrioSDPConnectionInfomationStruct remoteIp;			/**< Remote IP address information. This is used for notifying connection information if it is not described in each media stream. */

	/** If generating SDP string, the local stream struct should be set as default list.
	  * In that case, the remote stream does not need to set any information.
	  * Otherwise, the local/remote stream struct will use to notify the stream
	  * information of both local and remote ends. */
	u_int8 uNumOfLocalMedia;									/**< Number of local stream information. */
	EcrioSDPStreamStruct localStream[MAX_MEDIA];				/**< The structure containing the local stream information. */
	u_int8 uNumOfRemoteMedia;									/**< Number of local stream information. */
	EcrioSDPStreamStruct remoteStream[MAX_MEDIA];				/**< The structure containing the remote stream information. */

	EcrioSDPStringStruct strings;							/**< The structure containing the memory information. */
} EcrioSDPInformationStruct;

/** @struct EcrioSDPConfigStruct
 * This structure is provided to the EcrioSDPInit() function to supply
 * various configuration parameters.
 */
typedef struct
{
	LOGHANDLE logHandle;									/**< The log handle provided by the Ecrio PAL logging mechanism. */

	u_char *pUsername;										/**< User name used in "o=" line. */
	EcrioSDPConnectionInfomationStruct ip;					/**< Local IP address. */

	u_int32	uAllocationSize;								/**< The size of allocated buffer which the SDP module can consume at the initializing as instance. */
	void *pAllocated;										/**< Pointer to the allocated buffer. */
} EcrioSDPConfigStruct;

/**
 * This function is used to initialize an overall SDP implementation instance.
 * The SDP definition assumes that only one SDP instance can exist on one
 * device at a time. This function acts synchronously, and upon return the
 * SDP instance should be prepared to process sessions.
 *
 * The function will use the pAllocated pointer as the instance memory and
 * return it. The caller should not modify this memory or pointer until after
 * calling EcrioSDPDeinit.
 *
 * @param[in] pConfig			Pointer to a SDPConfigStruct structure. Must be
 *								valid. The caller owns the structure and the SDP
 *								implementation will consume or copy from it.
 * @param[out] pError			Pointer to a u_int32 to place the result code in
 *								upon return. Must be valid.
 * @return Returns a valid SDP instance handle. If NULL, pError will provide
 *	the error code. Callers only need to rely on the function return, it is not
 *	necessary to check both the return and pError. They will be guaranteed to
 *	be consistent.
 */
SDPHANDLE EcrioSDPInit
(
	const EcrioSDPConfigStruct *pConfig,
	u_int32 *pError
);

/**
 * This function is used to deinitialize the SDP implementation instance,
 * cleaning up all consumed resources. Any active sessions will NOT be
 * terminated (the caller must ensure that they are deleted prior to
 * invoking this function).
 *
 * The caller may release the instance memory after calling this function.
 *
 * @param[in] handle			The SDP instance handle.
 * @return Returns ECRIO_SDP_NO_ERROR if successful, otherwise an error.
 */
u_int32 EcrioSDPDeinit
(
	SDPHANDLE handle
);

/**
 * This function is used to create a session and acts like a constructor.
 *
 * The function will use the pAllocated pointer as an undefined length
 * charactor strings pool. The caller should not modify this memory or
 * pointer until after calling EcrioSDPSessionDelete.
 *
 * @param[in] handle			The SDP instance handle.
 * @param[in] uAllocationSize	The size of allocated buffer which the SDP
 *								module can consume within the session.
 *								The size is recommended about triple size of
 *								SDP length for local and remote end and working
 *								memory plus the size of EcrioSDPSessionStruct
 *								structure.
 * @param[in] pAllocated		Pointer to the allocated buffer.
 * @param[out] pError			Pointer to a u_int32 to place the result code
 *								in upon return. Must be valid.
 * @return Returns a valid session handle. If NULL, pError will provide the
 *	error code. Callers only need to rely on the function return, it is not
 *	necessary to check both the return and pError. They will be guaranteed to
 *	be consistent.
 */
SDPSESSIONHANDLE EcrioSDPSessionCreate
(
	SDPHANDLE handle,
	u_int32 uAllocationSize,
	void *pAllocated,
	u_int32 *pError
);

/**
 * This function is used to delete a session that was previously created
 * and acts like a destructor. All session resources must be cleaned up
 * before returning.
 *
 * The caller may release the session memory after calling this function.
 *
 * @param[in] session			The session handle.
 * @return Returns ECRIO_SDP_NO_ERROR if successful, otherwise an error.
 */
u_int32 EcrioSDPSessionDelete
(
	SDPSESSIONHANDLE session
);

/**
 * This function is used to generate initial/subsequent offer-answer SDP
 * strings. The function caller should handle the media information what
 * support media codec by list form called as "default list". It should
 * pass properly via EcrioSDPStreamStruct in EcrioSDPInformationStruct.
 * This list is used for adding all of the media information when creating
 * an initial offer, and used for verifying codec types which supported
 * media in remote offer when create an initial answer. In case of creating
 * subsequent offer-answer, it will prioritize the negotiated media codec
 * types.
 *
 * The caller can specify the event type and data. In particular, when
 * creating subsequent offer, the function caller should be conscious of
 * what information is to be updated and specified with SDPInformationStruct
 * structure in argument.
 *
 * @param[in] session			The session handle.
 * @param[in] pInfo				Pointer to an EcrioSDPInformationStruct structure.
 *								Must be valid.
 * @param[out] pSDP				Pointer to an EcrioSDPStringStruct structure.
 *								Must be valid. The caller owns the structure
 *								and the SDP library implementation set the
 *								internal pointers to directly reference the
 *								SDP data buffer.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 EcrioSDPSessionGenerateSDPString
(
	SDPSESSIONHANDLE session,
	EcrioSDPInformationStruct *pInfo,
	EcrioSDPStringStruct *pSDP
);

/**
 * This function is used to handle initial/subsequent offer-answer SDP strings.
 * This function will parse SDP string and analyze it for SDP session management,
 * and notify to the caller to fill media information with event type/parameters.
 *
 * See the description of the EcrioSDPInformationStruct structure for more information
 * about handling SDP string.
 *
 * @param[in] session			The session handle.
 * @param[in/out] pInfo			Pointer to an EcrioSDPInformationStruct structure.
 *								Must be valid.
 * @param[in] pSDP				Pointer to an EcrioSDPStringStruct structure.
 *								Must be valid.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 EcrioSDPSessionHandleSDPString
(
	SDPSESSIONHANDLE session,
	EcrioSDPInformationStruct *pInfo,
	EcrioSDPStringStruct *pSDP
);

/**
 * This function is used to find the best match codecs. The caller should be
 * set local media codecs information to verify the codec list which supported
 * by remote end.
 *
 * @param[in] session			The session handle.
 * @param[in/out] pInfo			Pointer to an EcrioSDPInformationStruct structure.
 *								Must be valid.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 EcrioSDPFindMatchPayloads
(
	SDPSESSIONHANDLE session,
	EcrioSDPInformationStruct *pInfo
);

/**
 * This is used to parse a SDP data and fill up the SDP structure.
 *
 * @param[out]	pSdpStruct		Pointer to the SDP structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the SDP. Must be non-NULL.
 * @param[in]	uSize			The size of SDP buffer.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 EcrioSDPParse
(
	EcrioSDPSessionStruct *pSdpStruct,
	u_char *pData,
	u_int32 uSize
);

/**
 * This is used to form a SDP data from SDP structure.
 *
 * This function does not create a buffer for SDP data, then function caller
 * must prepare an enough buffer and specified a maximum buffer size to pSize.
 *
 * After forming successfully, this function outputs entire SDP data to pBuffer
 * and length of SDP data to pSize.
 *
 * @param[in]		pSdpStruct		Pointer to the SDP structure. Must be non-NULL.
 * @param[in/out]	pBuffer			Pointer to the SDP buffer. Must be non-NULL.
 * @param[in]		uContainerSize	The maximum size of the SDP buffer to allocate.
 * @param[out]		pSize			Pointer to the size of SDP buffer. Must be non-NULL.
 * @param[in]		bIsOffer		If TRUE, there is an offer.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 EcrioSDPForm
(
	EcrioSDPSessionStruct *pSdpStruct,
	u_char *pBuffer,
	u_int32 uContainerSize,
	u_int32 *pSize,
	BoolEnum bIsOffer
);

u_int32 EcrioSDPUpdateUserName
(
	SDPHANDLE handle,
	unsigned char *pUserName
);
#ifdef __cplusplus
}
#endif

#endif /* #ifndef __ECRIOSDP_H__ */
