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

#ifndef  _ECRIO_SDP_INTERNAL_H_
#define  _ECRIO_SDP_INTERNAL_H_

#include "EcrioPAL.h"
#include "EcrioSDP.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENABLE_LOG
/* The build project should define ENABLE_LOG to enable non-error logging
   using the Ecrio PAL logging mechanism. */
#define SDPLOGV(a, b, c, ...)		pal_LogMessage((a), KLogLevelVerbose, KLogComponentGeneral, (b), (c),##__VA_ARGS__);
#define SDPLOGI(a, b, c, ...)		pal_LogMessage((a), KLogLevelInfo, KLogComponentGeneral, (b), (c),##__VA_ARGS__);
#define SDPLOGD(a, b, c, ...)		pal_LogMessage((a), KLogLevelDebug, KLogComponentGeneral, (b), (c),##__VA_ARGS__);
#define SDPLOGW(a, b, c, ...)		pal_LogMessage((a), KLogLevelWarning, KLogComponentGeneral, (b), (c),##__VA_ARGS__);
#define SDPLOGE(a, b, c, ...)		pal_LogMessage((a), KLogLevelError, KLogComponentGeneral, (b), (c),##__VA_ARGS__);

#define SDPLOGDUMPI(a, b, c, d)		pal_LogMessageDump((a), KLogLevelInfo, KLogComponentGeneral, (b), (c), (d));
#else
#define SDPLOGV(a, b, c, ...)		if (a != NULL) {;}
#define SDPLOGI(a, b, c, ...)		if (a != NULL) {;}
#define SDPLOGD(a, b, c, ...)		if (a != NULL) {;}
#define SDPLOGW(a, b, c, ...)		if (a != NULL) {;}
#define SDPLOGE(a, b, c, ...)		if (a != NULL) {;}

#define SDPLOGDUMPI(a, b, c, d)
#endif

#define CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize)	if ((uCnt + uLength) > uContainerSize) \
	{ uError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR; goto END; }

#define SDP_STRING_CONCATENATE(pDst, uDstLen, pSrc, uSrcLen, uError)	if (NULL == pal_StringNConcatenate(pDst, uDstLen, pSrc, uSrcLen)) \
	{ uError = ECRIO_SDP_MEMORY_ERROR; goto END; }

#define ECRIO_SDP_CRLF_SYMBOL											(u_char *)"\r\n"
#define ECRIO_SDP_SPACE_SYMBOL											(u_char *)" "
#define ECRIO_SDP_SLASH_SYMBOL											(u_char *)"/"
#define ECRIO_SDP_COLON_SYMBOL											(u_char *)":"
#define ECRIO_SDP_SEMICOLON_SYMBOL										(u_char *)";"
#define ECRIO_SDP_COMMA_SYMBOL											(u_char *)","
#define ECRIO_SDP_DASH_SYMBOL											(u_char *)"-"
#define ECRIO_SDP_ASTERISK_SYMBOL										(u_char *)"*"

#define ECRIO_SDP_ADDR_NETTYPE_INTERNET_STRING							(u_char *)"IN"
#define ECRIO_SDP_ADDR_TYPE_IP4_STRING									(u_char *)"IP4"
#define ECRIO_SDP_ADDR_TYPE_IP6_STRING									(u_char *)"IP6"

#define ECRIO_SDP_BANDWIDTH_AS_STRING									(u_char *)"AS"
#define ECRIO_SDP_BANDWIDTH_RS_STRING									(u_char *)"RS"
#define ECRIO_SDP_BANDWIDTH_RR_STRING									(u_char *)"RR"

#define ECRIO_SDP_MEDIA_TYPE_AUDIO_STRING								(u_char *)"audio"
#define ECRIO_SDP_MEDIA_TYPE_MSRP_STRING								(u_char *)"message"
#define ECRIO_SDP_MEDIA_PROTOCOL_RTP_AVP_STRING							(u_char *)"RTP/AVP"
#define ECRIO_SDP_MEDIA_PROTOCOL_TCP_MSRP_STRING						(u_char *)"TCP/MSRP"
#define ECRIO_SDP_MEDIA_PROTOCOL_TLS_MSRP_STRING						(u_char *)"TCP/TLS/MSRP"

#define ECRIO_SDP_MEDIA_DIRECTION_SENDONLY_STRING						(u_char *)"sendonly"
#define ECRIO_SDP_MEDIA_DIRECTION_RECVONLY_STRING						(u_char *)"recvonly"
#define ECRIO_SDP_MEDIA_DIRECTION_INACTIVE_STRING						(u_char *)"inactive"
#define ECRIO_SDP_MEDIA_DIRECTION_SENDRECV_STRING						(u_char *)"sendrecv"

#define ECRIO_SDP_MEDIA_MSRP_CEMA_STRING								(u_char *)"msrp-cema"
#define ECRIO_SDP_MEDIA_CHATROOM_STRING									(u_char *)"chatroom"

#define ECRIO_SDP_MEDIA_ATTRIBUTE_SETUP_ACTIVE_STRING					(u_char *)"active"
#define ECRIO_SDP_MEDIA_ATTRIBUTE_SETUP_PASSIVE_STRING					(u_char *)"passive"
#define ECRIO_SDP_MEDIA_ATTRIBUTE_SETUP_ACTPASS_STRING					(u_char *)"actpass"
#define ECRIO_SDP_MEDIA_ATTRIBUTE_SETUP_HOLDCONN_STRING					(u_char *)"holdconn"

#define ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_MAXPTIME_STRING					(u_char *)"maxptime"
#define ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_PTIME_STRING						(u_char *)"ptime"
#define ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_RTPMAP_STRING					(u_char *)"rtpmap"
#define ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FMTP_STRING						(u_char *)"fmtp"
#define ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_ACCEPT_TYPES_STRING				(u_char *)"accept-types"
#define ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_ACCEPT_WRAPPED_TYPES_STRING		(u_char *)"accept-wrapped-types"
#define ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_PATH_STRING						(u_char *)"path"
#define ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_SETUP_STRING						(u_char *)"setup"
#define ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FILE_SELECTOR_STRING				(u_char *)"file-selector"
#define ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FILE_DISPOSITION_STRING			(u_char *)"file-disposition"
#define ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FILE_TRANSFER_ID_STRING			(u_char *)"file-transfer-id"
#define ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FILE_DATE_STRING					(u_char *)"file-date"
#define ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FILE_ICON_STRING					(u_char *)"file-icon"
#define ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FILE_RANGE_STRING				(u_char *)"file-range"

#define ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_CURRENT_STATUS_STRING			(u_char *)"curr"
#define ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_DESIRED_STATUS_STRING			(u_char *)"des"
#define ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_CONFIRM_STATUS_STRING			(u_char *)"conf"

#define ECRIO_SDP_AUDIO_CODEC_PCMU_STRING								(u_char *)"PCMU"
#define ECRIO_SDP_AUDIO_CODEC_L16_STRING								(u_char *)"L16"
#define ECRIO_SDP_AUDIO_CODEC_AMR_WB_STRING								(u_char *)"AMR-WB"
#define ECRIO_SDP_AUDIO_CODEC_AMR_STRING								(u_char *)"AMR"
#define ECRIO_SDP_AUDIO_CODEC_DTMF_STRING								(u_char *)"telephone-event"

#define ECRIO_SDP_CODEC_AMR_MODE_SET_STRING								(u_char *)"mode-set"
#define ECRIO_SDP_CODEC_AMR_OCTET_ALIGN_STRING							(u_char *)"octet-align"
#define ECRIO_SDP_CODEC_AMR_MODE_CHANGE_PERIOD_STRING					(u_char *)"mode-change-period"
#define ECRIO_SDP_CODEC_AMR_MODE_CHANGE_CAPABILITY_STRING				(u_char *)"mode-change-capability"
#define ECRIO_SDP_CODEC_AMR_MODE_CHANGE_NEIGHBOR_STRING					(u_char *)"mode-change-neighbor"
#define ECRIO_SDP_CODEC_AMR_CRC_STRING									(u_char *)"crc"
#define ECRIO_SDP_CODEC_AMR_ROBUST_SORTING_STRING						(u_char *)"robust-sorting"
#define ECRIO_SDP_CODEC_AMR_MAX_RED_STRING								(u_char *)"max-red"

#define ECRIO_SDP_MSRP_MEDIA_TYPE_CONFERENCE_STRING						(u_char *)"application/conference-info+xml"
#define ECRIO_SDP_MSRP_MEDIA_TYPE_TEXT_PLAIN_STRING						(u_char *)"text/plain"
#define ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_CPIM_STRING					(u_char *)"message/cpim"
#define ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_IMDN_XML_STRING				(u_char *)"message/imdn+xml"
#define ECRIO_SDP_MSRP_MEDIA_TYPE_IM_ISCOMPOSING_XML_STRING				(u_char *)"application/im-iscomposing+xml"
#define ECRIO_SDP_MSRP_MEDIA_TYPE_MULTIPART_MIXED_STRING				(u_char *)"multipart/mixed"
#define ECRIO_SDP_MSRP_MEDIA_TYPE_HTTP_FILE_TRANSFER_STRING				(u_char *)"application/vnd.gsma.rcs-ft-http+xml"
#define ECRIO_SDP_MSRP_MEDIA_TYPE_PUSH_LOCATION_STRING					(u_char *)"application/vnd.gsma.rcspushlocation+xml"
#define ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_REVOKE_STRING					(u_char *)"application/vnd.gsma.rcsrevoke+xml"
#define ECRIO_SDP_MSRP_MEDIA_TYPE_PRIVACY_MANAGEMENT_STRING				(u_char *)"application/vnd.gsma.rcsalias-mgmt+xml"
#define ECRIO_SDP_MSRP_MEDIA_TYPE_LINK_REPORT_STRING					(u_char *)"application/vnd.gsma.rcslink-report+xml"
#define ECRIO_SDP_MSRP_MEDIA_TYPE_SPAM_REPORT_STRING					(u_char *)"application/vnd.gsma.rcsspam-report+xml"
#define ECRIO_SDP_MSRP_MEDIA_TYPE_RICHCARD_STRING						(u_char *)"application/vnd.gsma.botmessage.v1.0+json"
#define ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTED_CHIPLIST_STRING				(u_char *)"application/vnd.gsma.botsuggestion.v1.0+json"
#define ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTION_RESPONSE_STRING			(u_char *)"application/vnd.gsma.botsuggestion.response.v1.0+json"
#define ECRIO_SDP_MSRP_MEDIA_TYPE_SHARED_CLIENT_DATA_STRING				(u_char *)"application/vnd.gsma.botsharedclientdata.v1.0+json"
#define ECRIO_SDP_MSRP_MEDIA_TYPE_GROUPDATA_STRING						(u_char *)"application/vnd.oma.cpm-groupdata+xml"

#define ECRIO_SDP_CLOSED_GROUPCHAT_STRING								(u_char *)"org.3gpp.urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.closedgroupchat"

#define ECRIO_SDP_FILE_SELECTOR_NAME_STRING								(u_char *)"name"
#define ECRIO_SDP_FILE_SELECTOR_SIZE_STRING								(u_char *)"size"
#define ECRIO_SDP_FILE_SELECTOR_TYPE_STRING								(u_char *)"type"
#define ECRIO_SDP_FILE_SELECTOR_HASH_STRING								(u_char *)"hash"

#define ECRIO_SDP_MSRP_FILE_DATE_CREATION_STRING						(u_char *)"creation"
#define ECRIO_SDP_MSRP_FILE_DATE_MODIFICATION_STRING					(u_char *)"modification"
#define ECRIO_SDP_MSRP_FILE_DATE_READ_STRING							(u_char *)"read"

#define ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING							(u_char *)"qos"
#define ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY_STRING				(u_char *)"mandatory"
#define ECRIO_SDP_PRECONDITION_STRENGTH_OPTIONAL_STRING					(u_char *)"optional"
#define ECRIO_SDP_PRECONDITION_STRENGTH_NONE_STRING						(u_char *)"none"
#define ECRIO_SDP_PRECONDITION_STRENGTH_FAILURE_STRING					(u_char *)"failure"
#define ECRIO_SDP_PRECONDITION_STRENGTH_UNKNOWN_STRING					(u_char *)"unknown"
#define ECRIO_SDP_PRECONDITION_STATUS_E2E_STRING						(u_char *)"e2e"
#define ECRIO_SDP_PRECONDITION_STATUS_LOCAL_STRING						(u_char *)"local"
#define ECRIO_SDP_PRECONDITION_STATUS_REMOTE_STRING						(u_char *)"remote"
#define ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING					(u_char *)"none"
#define ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING					(u_char *)"send"
#define ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING					(u_char *)"recv"
#define ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING				(u_char *)"sendrecv"
#define ECRIO_SDP_MEDIA_ATTRIBUTE_TYPE_FINGERPRINT						(u_char *)"fingerprint"

#define ECRIO_SDP_CODEC_MIN_DYNAMIC_VALUE								96
#define ECRIO_SDP_CODEC_MAX_DYNAMIC_VALUE								127

/**
 * SDP state enumeration.
 */
typedef enum
{
	ECRIO_SDP_State_None = 0x00,				/**< No current SDP state. */
	ECRIO_SDP_State_InitialLocalOffer,			/**< Generated local offer, waiting to receive remote answer from remote end. */
	ECRIO_SDP_State_InitialRemoteOffer,			/**< Handled remote offer, waiting to generate local answer from the upper layer. */
	ECRIO_SDP_State_Negotiated,					/**< SDP session negotiated. */
	ECRIO_SDP_State_SubsequentLocalOffer,		/**< Generated subsequent offer, waiting to receive remote subsequent answer from remote end. */
	ECRIO_SDP_State_SubsequentRemoteOffer,		/**< Handled subsequent remote offer, waiting to generate subsequent local answer from the upper layer. */
} EcrioSDPStateEnum;

/**
 * This structure defines the basic SDP information for operating structure or
 * strings to generate/handle SDP.
 */
typedef struct
{
	LOGHANDLE logHandle;

	u_char *pUsername;
	EcrioSDPConnectionInfomationStruct ip;

	EcrioSDPStringStruct strings;
} EcrioSDPStruct;

/**
 * This structure defines the entire SDP information for each SDP session.
 */
typedef struct
{
	EcrioSDPStruct *s;

	EcrioSDPStateEnum eState;
	EcrioSDPPreconditionStateEnum ePrecondState;

	/** Local SDP structure */
	EcrioSDPSessionStruct localSession;

	/** Remote SDP structure */
	EcrioSDPSessionStruct remoteSession;

	/** Working memory for verifying Local/Remote SDP structure */
	EcrioSDPSessionStruct work;
} EcrioSDPInternalSessionStruct;

/**
 * This function is used to parse an RTP payload ("a=rtpmap") media attribute
 * line and store information to the SDP information structure.
 *
 * The parsing string may have a non-NULL terminator, and pData should be
 * starting payload type. It should be specified proper size to uSize.
 * For example, in case of parsing the following
 * "a=rtpmap" line:
 *   a=rtpmap:104 AMR-WB/16000<CR><LF>...
 * Then pData should be indicated pointer of a head of payload type "104"
 * that after "rtpmap:" string. uSize should be set 16 which do not include
 * a CRLF part.
 *
 * @param[out]	pStream			Pointer to the stream structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the string of payload type. Must be non-NULL.
 * @param[in]	uSize			The size of parsing string.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseRtpmap
(
	EcrioSDPStreamStruct *pStream,
	u_char *pData,
	u_int32 uSize
);

/**
 * This function is used to parse a particular format ("a=fmtp") media attribute
 * line and store information to the SDP information structure.
 *
 * The parsing string may have a non-NULL terminator, but should be specified
 * proper size to uSize. For example, in case of parsing the following
 * "a=fmtp" line:
 *   a=fmtp:104 mode-set=2; octet-align=0<CR><LF>...
 * Then pData should be indicated pointer of a head of format "104"
 * that after "fmtp:" string. uSize should be set 29 which do not include
 * a CRLF part.
 *
 * @param[out]	pStream			Pointer to the stream structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the string of format. Must be non-NULL.
 * @param[in]	uSize			The size of parsing string.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseFmtp
(
	EcrioSDPStreamStruct *pStream,
	u_char *pData,
	u_int32 uSize
);

/**
 * This function is used to parse AMR specific parameters in a particular
 * format ("a=fmtp") media attribute line and store AMR parameters to the
 * SDP information structure.
 *
 * The parsing string may have a non-NULL terminator, and pData should be starting
 * AMR specific paramter, coming after format string. It should be specified
 * proper size to uSize. For example, in case of parsing the following part of
 * "a=fmtp" line:
 *   a=fmtp:104 mode-set=2; octet-align=0<CR><LF>...
 * Then pData should be indicated pointer of a head of "mode-set" parameter
 * that after the format (fmtp:104) string.
 * string and uSize should be set 25 which do not include a CRLF part.
 *
 * @param[out]	pPayload		Pointer to the RTP payload structure. Must be non-NULL.
 * @param[in]	pData			Pointer to an AMR Parameter in "a=fmtp" line. Must
 *                              be non-NULL.
 * @param[in]	uSize			The size of parsing string.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseAMRParameters
(
	EcrioSDPPayloadStruct *pPayload,
	u_char *pData,
	u_int32 uSize
);

/**
 * This function is used to parse DTMF specific parameters in a particular
 * format ("a=fmtp") media attribute line and store DTMF parameters to the
 * SDP information structure.
 *
 * The parsing string may have a non-NULL terminator, and pData should be starting
 * AMR specific paramter, coming after format string. It should be specified
 * proper size to uSize. For example, in case of parsing the following part of
 * "a=fmtp" line:
 *   a=fmtp:108 0-15,66,77<CR><LF>...
 * Then pData should be indicated pointer of a head of DTMF parameter that
 * after the format (fmtp:110) string.
 * string and uSize should be set 10 which do not include a CRLF part.
 *
 * @param[out]	pPayload		Pointer to the RTP payload structure. Must be non-NULL.
 * @param[in]	pData			Pointer to an DTMF Parameter in "a=fmtp" line. Must
 *                              be non-NULL.
 * @param[in]	uSize			The size of parsing string.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseDTMFParameters
(
	EcrioSDPPayloadStruct *pPayload,
	u_char *pData,
	u_int32 uSize
);

/**
 * This function is used to parse the supported media types ("a=accept-types") in
 * media attribute line and store information to the SDP information structure.
 *
 * The parsing string may have a non-NULL terminator, but should be specified
 * proper size to uSize. If string has multiple types, then out parameters will
 * indicate multiple types by bitmask.
 *
 * @param[out]	pAcceptTypes	Pointer to the u_int32 variable which indicates to
 *								the accept-types, it will be EcrioSDPMSRPMediaTypesEnum
 *								enumeration. Must be non-NULL.
 * @param[in]	pData			Pointer to the string of format. Must be non-NULL.
 * @param[in]	uSize			The size of parsing string.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseAcceptTypes
(
	u_int32 *pAcceptTypes,
	u_char *pData,
	u_int32 uSize
);

/**
 * This function is used to parse the file selector attribute ("a=file-selector")
 * in media attribute line and store information to the SDP information structure.
 *
 * The parsing string may have a non-NULL terminator, but should be specified
 * proper size to uSize.
 *
 * @param[in]	pStrings		Pointer to the string structure. Must be non-NULL.
 * @param[out]	pStream			Pointer to the stream structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the string of format. Must be non-NULL.
 * @param[in]	uSize			The size of parsing string.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseFileSelector
(
	EcrioSDPStringStruct *pStrings,
	EcrioSDPStreamStruct *pStream,
	u_char *pData,
	u_int32 uSize
);

/**
 * This function is used to parse the file date attribute ("a=file-date") in
 * media attribute line and store information to the SDP information structure.
 *
 * The parsing string may have a non-NULL terminator, but should be specified
 * proper size to uSize.
 *
 * @param[in]	pStrings		Pointer to the string structure. Must be non-NULL.
 * @param[out]	pStream			Pointer to the stream structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the string of format. Must be non-NULL.
 * @param[in]	uSize			The size of parsing string.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseFileDate
(
	EcrioSDPStringStruct *pStrings,
	EcrioSDPStreamStruct *pStream,
	u_char *pData,
	u_int32 uSize
);

/**
 * This function is used to parse the file-range attribute ("a=file-range") in
 * media attribute line and store information to the SDP information structure.
 *
 * The parsing string may have a non-NULL terminator, but should be specified
 * proper size to uSize.
 *
 * @param[out]	pStream			Pointer to the stream structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the string of format. Must be non-NULL.
 * @param[in]	uSize			The size of parsing string.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseFileRange
(
	EcrioSDPStreamStruct *pStream,
	u_char *pData,
	u_int32 uSize
);

/**
 * This function is used to parse the current status attribute ("a=curr") or
 * confirm status attribute ("a=conf") in media attribute line and store
 * information to the SDP information structure.
 * 
 * The parsing string may have a non-NULL terminator, but should be specified
 * proper size to uSize.
 * 
 * @param[out]	pStream			Pointer to the stream structure. Must be non-NULL.
 * @param[in]	bIsCurrent		If TRUE, it works "curr", else "conf".
 * @param[in]	pData			Pointer to the string of format. Must be non-NULL.
 * @param[in]	uSize			The size of parsing string.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseCurrentStatus
(
	EcrioSDPStreamStruct *pStream,
	BoolEnum bIsCurrent,
	u_char *pData,
	u_int32 uSize
);

/**
 * This function is used to parse the desire status attribute ("a=des")in media
 * attribute line and store information to the SDP information structure.
 * 
 * The parsing string may have a non-NULL terminator, but should be specified
 * proper size to uSize.
 * 
 * @param[out]	pStream			Pointer to the stream structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the string of format. Must be non-NULL.
 * @param[in]	uSize			The size of parsing string.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseDesiredStatus
(
	EcrioSDPStreamStruct *pStream,
	u_char *pData,
	u_int32 uSize
);

/**
 * This is used to parse SDP Attributes ("a=") line and fill up the SDP structure.
 *
 * @param[in]	pStrings		Pointer to the string structure. Must be non-NULL.
 * @param[out]	pStream			Pointer to the stream structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the SDP Attributes ("a=") line.
 *                              Must be non-NULL.
 * @param[in]	uSize			The size of SDP buffer.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseHeaderA
(
	EcrioSDPStringStruct *pStrings,
	EcrioSDPStreamStruct *pStream,
	u_char *pData,
	u_int32 uSize
);

/**
 * This is used to parse SDP bandwidth ("b=") line and fill up the SDP structure.
 *
 * @param[out]	pBandwidth		Pointer to the bandwidth structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the SDP bandwidth ("b=") line.
 *                              Must be non-NULL.
 * @param[in]	uSize			The size of SDP buffer.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseHeaderB
(
	EcrioSDPBandwidthStruct *pBandwidth,
	u_char *pData,
	u_int32 uSize
);

/**
 * This is used to parse SDP Connection Data ("c=") line and fill up the SDP structure.
 *
 * @param[in]	pStrings		Pointer to the string structure. Must be non-NULL.
 * @param[out]	pConnection		Pointer to the connection information structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the SDP Connection Data ("c=") line.
 *                              Must be non-NULL.
 * @param[in]	uSize			The size of SDP buffer.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseHeaderC
(
	EcrioSDPStringStruct *pStrings,
	EcrioSDPConnectionInfomationStruct *pConnection,
	u_char *pData,
	u_int32 uSize
);

/**
 * This is used to parse SDP Media Descriptions ("m=") line and fill up the SDP structure.
 *
 * @param[out]	pStream			Pointer to the stream structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the SDP Media Descriptions ("m=") line.
 *                              Must be non-NULL.
 * @param[in]	uSize			The size of SDP buffer.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseHeaderM
(
	EcrioSDPStreamStruct *pStream,
	u_char *pData,
	u_int32 uSize
);

/**
 * This is used to parse SDP Origin ("o=") line and fill up the SDP structure.
 *
 * @param[in]	pStrings		Pointer to the string structure. Must be non-NULL.
 * @param[out]	pOrigin			Pointer to the origin structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the SDP Origin ("o=") line. Must be non-NULL.
 * @param[in]	uSize			The size of SDP buffer.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseHeaderO
(
	EcrioSDPStringStruct *pStrings,
	EcrioSDPOriginStruct *pOrigin,
	u_char *pData,
	u_int32 uSize
);

/**
 * This is used to convert a string to an unsigned, 64-bit numeric value.
 *
 * @param[in]	pString			Pointer to the string holding the string version
 *								of the number to convert. Must be non-NULL.
 * @return The unsigned 64-bit numeric value after the conversion or 0 if there
 * was an error.
 */
u_int64 ec_sdp_ConvertToUI64Num
(
	const u_char *pString
);

/**
 * This function will copy string from source string to one memory block and
 * indicate the pointer to the destination. The destination memory block will
 * manage using pData in EcrioSDPStringStruct which is a fixed-length buffer
 * by uContainerSize, and set specified pointer in buffer to the destination
 * memory block as pDest.
 *
 * @param[in]	pStrings		Pointer to the string structure. Must be non-NULL.
 * @param[out]  ppDest			Pointer to the destination memory block, to
 *								copy byte data to.
 * @param[in]   pSource			Pointer to the source memory block, to copy
 *								byte data from.
 * @param[in]   memLength		Length of the memory block to be copied, in
 *								bytes.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_StringCopy
(
	EcrioSDPStringStruct *pStrings,
	void **ppDest,
	const void *pSource,
	u_int32 strLength
);

/**
 * This function is used to get NTP timestamp indicated by 64-bit length.
 *
 * @param		void
 * @return NTP time
 */
u_int64 ec_sdp_GetNTPtime
(
	void
);

u_int32 ec_SDP_CheckSDPEvent
(
	EcrioSDPInternalSessionStruct *S,
	EcrioSDPInformationStruct *pInfo
);

u_int32 ec_SDP_CopyStreamStructure
(
	EcrioSDPInternalSessionStruct *S,
	EcrioSDPStringStruct *pStrings,
	EcrioSDPStreamStruct *pDest,
	EcrioSDPStreamStruct *pSource
);

u_int32 ec_SDP_CopySessionStructure
(
	EcrioSDPInternalSessionStruct *S,
	EcrioSDPSessionStruct *pDest,
	EcrioSDPSessionStruct *pSource
);

u_int32 ec_SDP_UpdateLocalSDPStructure
(
	EcrioSDPInternalSessionStruct *S,
	EcrioSDPInformationStruct *pInfo
);

u_int32 ec_SDP_FillSDPMessageBody
(
	EcrioSDPInternalSessionStruct *S,
	EcrioSDPSessionStruct *pSession,
	u_int8 uNumOfList,
	EcrioSDPStreamStruct *pStream,
	EcrioSDPEventTypeEnum eEvent
);

u_int32 ec_SDP_CheckMediaStatus
(
	EcrioSDPInternalSessionStruct *S,
	EcrioSDPInformationStruct *pInfo
);

void ec_SDP_FindMatchAudioPayloads
(
	EcrioSDPInternalSessionStruct *S,
	EcrioSDPStreamStruct *pLocal,
	EcrioSDPStreamStruct *pRemote,
	s_char *pLocalAudioIndex,
	s_char *pRemoteAudioIndex,
	s_char *pLocalDTMFIndex,
	s_char *pRemoteDTMFIndex
);

u_int32 ec_SDP_FindMatchPayloads
(
	EcrioSDPInternalSessionStruct *S,
	EcrioSDPInformationStruct *pInfo,
	BoolEnum bAddRemote
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*  _ECRIO_SMS_INTERNAL_H_ */
