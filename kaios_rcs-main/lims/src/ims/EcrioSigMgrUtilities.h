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

/*	Support for Authorization header in all request	start	*/

#define	_ECRIO_SUE_EXTN_METHOD_NAME_SET_AUTH_STRUCT						(u_char *)"EcrioSUEExtnSetAuthStructExtn"
#define	_ECRIO_SUE_EXTN_METHOD_NAME_GET_AUTH_STRUCT						(u_char *)"EcrioSUEExtnGetAuthStructExtn"
#define	_ECRIO_SUE_EXTN_METHOD_NAME_GET_PHONE_PROPERTY					(u_char *)"EcrioSUEExtnGetPhonePropertyExtn"
#define	_ECRIO_SUE_EXTN_METHOD_NAME_COUNT_TIMEDOUT_PROTECTED_MESSAGE	(u_char *)"EcrioSUEExtnCountTimedoutProtectedMessageExtn"
#define _ECRIO_SUE_EXTN_METHOD_NAME_SEND_INITIAL_REGISTER				(u_char *)"EcrioSUEExtnSendIntialRegisterRequestExtn"
#define _ECRIO_SUE_EXTN_METHOD_NAME_SEND_RE_REGISTER					(u_char *)"EcrioSUEExtnSendReRegisterRequestExtn"
#define	_ECRIO_SUE_EXTN_METHOD_NAME_ADD_IP_TO_IMS_ROUTE_TABLE			(u_char *)"EcrioSUEExtnAddIPToIMSRouteTableExtn"
#define	_ECRIO_SUE_EXTN_METHOD_NAME_REMOVE_IP_FROM_IMS_ROUTE_TABLE		(u_char *)"EcrioSUEExtnRemoveIPFromIMSRouteTableExtn"
#define	_ECRIO_SUE_EXTN_METHOD_NAME_DISCONNECTION_COMPLETE_NOTIF		(u_char *)"EcrioSUEExtnDisconnectionCompleteNotifExtn"

/*	Support for Authorization header in all request	end	*/

/* Resolving Build Error For Stand Alone UAE */
#include "EcrioSigMgrIMSLibHandler.h"
#include "khash.h"

typedef void* MAPHANDLE;

#ifndef _ECRIO_SIG_MGR_UTILITES_H_
#define  _ECRIO_SIG_MGR_UTILITES_H_

#define _ECRIO_SIGMGR_SIP_URI_ERROR						(0x02U)

#define _ECRIO_SIGMGR_SIP_AMPERSAND_SYMBOL				"&"
#define _ECRIO_SIGMGR_SIP_STAR_SYMBOL					"*"

#define	_ECRIO_SIGMGR_SIGNALING_SIP_CSEQ				"CSeq"
#define	_ECRIO_SIGMGR_SIGNALING_SIP_FROM				"From"
#define	_ECRIO_SIGMGR_SIGNALING_SIP_TO					"To"
#define	_ECRIO_SIGMGR_SIGNALING_SIP_CALL_ID				"Call-ID"
#define	_ECRIO_SIGMGR_SIGNALING_SIP_VIA					"Via"

#define	_ECRIO_SIGMGR_SIGNALING_SIP_CSEQ				"CSeq"
#define	_ECRIO_SIGMGR_SIGNALING_SIP_FROM				"From"
#define	_ECRIO_SIGMGR_SIGNALING_SIP_TO					"To"
#define	_ECRIO_SIGMGR_SIGNALING_SIP_CALL_ID				"Call-ID"
#define	_ECRIO_SIGMGR_SIGNALING_SIP_VIA					"Via"

/* Sip Uri Parameters String */
#define ECRIO_SIGMGR_SIP_URI_PARAM_NAME					"user"
#define ECRIO_SIGMGR_SIP_URI_PARAM_VALUE				"phone"

/* Reason value */
#define ECRIO_SIGMGR_REASON_EHRPD_CONNECTED				"SIP;text=\"Moved to eHRPD\""
#define ECRIO_SIGMGR_REASON_SESSION_EXPIRED				"SIP;text=\"Session Expired\""
#define ECRIO_SIGMGR_REASON_RTP_RTCP_TIMEOUT			"SIP;text=\"RTP-RTCP Timeout\""
#define ECRIO_SIGMGR_REASON_USER_TRIGGERED				"SIP;text=\"User Triggered\""
#define ECRIO_SIGMGR_REASON_AIRPLANEMODE_ON				"SIP;text=\"Airplane mode ON\""
#define ECRIO_SIGMGR_REASON_PRECONDITION_TIMEOUT		"SIP;text=\"Precondition Time Out\""
#define ECRIO_SIGMGR_REASON_NEW_DIALOG_ESTABLISHED		"SIP;text=\"New Dialog Established\""

typedef enum
{
	_ECRIO_SIGMGR_SIGNALING_MANDATORY_None = -1,
	_ECRIO_SIGMGR_SIGNALING_MANDATORY_CSeq,
	_ECRIO_SIGMGR_SIGNALING_MANDATORY_From,
	_ECRIO_SIGMGR_SIGNALING_MANDATORY_To,
	_ECRIO_SIGMGR_SIGNALING_MANDATORY_CallID,
	_ECRIO_SIGMGR_SIGNALING_MANDATORY_Via
} _EcrioSigMgrSignallingMandatoryHeaderEnum;

typedef enum
{
	_ECRIO_SIGMGR_SIP_ALPHA_NUM_None = -1,
	_ECRIO_SIGMGR_SIP_ALPHA_NUM_Digit,
	_ECRIO_SIGMGR_SIP_ALPHA_NUM_SmallAlpha,
	_ECRIO_SIGMGR_SIP_ALPHA_NUM_CapitalAlpha,
	_ECRIO_SIGMGR_SIP_ALPHA_NUM_HexDigit,
	_ECRIO_SIGMGR_SIP_ALPHA_NUM_AlphaNum,
} _EcrioSigMgrSipAlphaNumEnum;

typedef enum
{
	ECRIO_SIGMGR_SIP_URI_TOKEN_User = 0,
	ECRIO_SIGMGR_SIP_URI_TOKEN_Password,
	ECRIO_SIGMGR_SIP_URI_TOKEN_Parameters,
	ECRIO_SIGMGR_SIP_URI_TOKEN_Headers,
} _EcrioSigMgrSipUriTokenEnum;

u_int32 _EcrioSigMgrStringCreate
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pSrcString,
	u_char **ppDestString
);
u_int32 _EcrioSigMgrStringNCreate
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pSrcString,
	u_char **ppDestString,
	u_int32 len
);

/* Append Double Quotes */
u_char *_EcrioSigMgrStringQuote
(
	u_char *pIn_String
);

/* Remove Double Quotes */
u_char *_EcrioSigMgrStringUnquote
(
	u_char *pIn_String
);

/* Determine if string is Quoted */
/* Dont call this function when Input is NULL, That check should be done by the Caller */
BoolEnum _EcrioSigMgrStringIsQuoted
(
	u_char *In_String
);

u_int32 _EcrioSigMgrCopyRouteSet
(
	EcrioSigMgrStruct *pSigMgr,
	u_int16 routeSetCount,
	EcrioSigMgrNameAddrWithParamsStruct **ppSrcRouteSet,
	EcrioSigMgrNameAddrWithParamsStruct ***pppDestRouteSet
);

u_int32 _EcrioSigMgrReleaseRouteSet
(
	EcrioSigMgrStruct *pSigMgr,
	u_int16 routeSetCount,
	EcrioSigMgrNameAddrWithParamsStruct **ppRouteSet
);

u_int32 _EcrioSigMgrAddRouteHdr
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrIPAddrStruct *pIPAddr,
	EcrioSigMgrRouteStruct *pHdr
);

u_int32 _EcrioSigMgrCreateSignalingUDPSocket
(
	EcrioSigMgrStruct *pSigMgr
);

u_int32 _EcrioSigMgrComputeRemoteHost
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrIPAddrStruct *
	pRemoteHostAddr
);

u_int32 _EcrioSigMgrGenerateTag
(
	u_char **ppTag
);

u_int32 _EcrioSigMgrGenerateCallId
(
	u_char *pUserId,
	u_char **ppCallId
);

u_char *_EcrioSigMgrGenerateVia
(
	u_char *pClientAddr,
	u_int16 clientPort
);

u_int32 _EcrioSigMgrGenerateViaString
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrViaStruct *pVia,
	u_char **ppValue
);

u_int32 _EcrioSigMgrGenerateBranchParam
(
	EcrioSigMgrStruct *pSigMgr,
	u_char **ppBranch
);

u_int32 _EcrioSigMgrAddLocalContactUri
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrContactStruct **ppUri,
	BoolEnum bPrivacy
);

u_int32 _EcrioSigMgrAddLocalContactUriWithoutFeatureTags
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrContactStruct **ppUri
);

u_int32 _EcrioSigMgrUpdateRequestNonceCount
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrAuthorizationStruct *pAuth,
	u_int32 nonceCount
);

u_int32 _EcrioSigMgrPopulateAuthorizationDetails
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrAuthorizationStruct *pSrcAuth,
	EcrioSigMgrAuthorizationStruct *pDstAuth
);

u_int32 _EcrioSigMgrPopulateAuthenticationDetails
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrAuthenticationStruct *pSrcAuth,
	EcrioSigMgrAuthenticationStruct *pDstAuth
);

u_int32 _EcrioSigMgrPopulateViaHdr
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrViaStruct *pSrcVia,
	EcrioSigMgrViaStruct *pDstVia
);

u_int32 _EcrioSigMgrAddHdr
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrHeaderStruct *pSrcHdr,
	EcrioSigMgrHeaderStruct **ppDstHdr
);

u_int32 _EcrioSigMgrAddValuesToHdr
(
	EcrioSigMgrStruct *pSigMgr,
	u_int16 numValues,
	u_char **ppValues,
	EcrioSigMgrHeaderStruct *pHdr
);

u_int32 _EcrioSigMgrAddPublishEventHdr
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList
);

u_int32 _EcrioSigMgrAddPublishIfMatchHdr
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList,
	u_char *pETag
);

u_int32 _EcrioSigMgrAddUserAgentHeader
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList
);

u_int32 _EcrioSigMgrAddAllowHeader
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList
);

u_int32 _EcrioSigMgrAddReasonHeader
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList,
	u_char * pCallId
);

u_int32 _EcrioSigMgrAddP_AccessNWInfoHeader
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList
);

u_int32 _EcrioSigMgrAddP_LastAccessNWInfoHeader
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList
);

u_int32 _EcrioSigMgrAddSessionExpiresHeader
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList,
	BoolEnum isSessionRefresh,
	BoolEnum isRefresher,
	u_int32 uRefreshValue
);

u_int32 _EcrioSigMgrAddMinSEHeader
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList,
	u_int32 uMinSEValue
);

#if 0
u_int32 _EcrioSigMgrAddSupportedHeader
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList
);
#endif

u_int32 _EcrioSigMgrAddP_PreferredServiceHeader
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessageStruct
);

u_int32 _EcrioSigMgrAddP_EarlyMediaHeader
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessageStruct
);

u_int32 _EcrioSigMgrAddAccptContactHeader
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessageStruct
);

u_int32 _EcrioSigMgrAddRequestDispositionHeader
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList
);

EcrioSigMgrRouteStruct *_EcrioSigMgrAddRouteHeader
(
	EcrioSigMgrStruct *pSigMgr
);

u_int32 _EcrioSigMgrAddUserPhoneParam
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUriStruct *pUri
);

u_int32 _EcrioSigMgrAddP_PreferrdIdHeader
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessageStruct
);


u_int32 _EcrioSigMgrAddConversationsIDHeader
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessageStruct
);

EcrioSipHeaderTypeEnum _EcrioSigMgrGetHeaderTypeFromString
(
	u_char *pStart
);

u_int32 _EcrioSigMgrInsertOptionalHeader
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList,
	EcrioSigMgrHeaderStruct *pOptHeader
);
// u_int32 _EcrioSigMgrAddOptionalHeader
// (
// EcrioSigMgrStruct *pSigMgr,
// void *pOptionalHeaderList,
// EcrioSipHeaderTypeEnum eHeaderType,
// u_int16 numValues,
// u_char **ppValues
// );
//

u_char *_EcrioSigMgrGetFeatureTagValue
(
	EcrioSigMgrFeatureTagStruct *pFetaureTags,
	EcrioSipHeaderTypeEnum eHeaderType
);

u_int32 _EcrioSigMgrAddOptionalHeader
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList,
	EcrioSipHeaderTypeEnum eHeaderType,
	u_int16 numValues,
	u_char **ppValues,
	u_int16 numParams,
	u_char **ppParamN,
	u_char **ppParamV
);

u_int32 _EcrioSigMgrAddValueParamsToHdr
(
	EcrioSigMgrStruct *pSigMgr,
	u_int16 numValues,
	u_char **ppValues,
	u_int16 numParams,
	u_char **ppParamsName,
	u_char **ppParamsValue,
	EcrioSigMgrHeaderStruct *pHdr
);

u_int32	_EcrioSigMgrExtractRSeqFromRAck
(
	u_char *pRAckStr,
	u_int32	*pRseq
);

u_int32	_EcrioSigMgrExtractResponseCode
(
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData
);

u_int32	_EcrioSigMgrUpdateRSeqInProvResp
(
	_EcrioSigMgrInviteUsageInfoStruct *pUsageData
);

u_int32 _EcrioSigMgrCopyParam
(
	EcrioSigMgrStruct *pSigMgr,
	u_int16 paramCount,
	EcrioSigMgrParamStruct **ppSrcParamSet,
	EcrioSigMgrParamStruct ***pppDestParamSet
);

u_int32 _EcrioSigMgrAllocateAndPopulateUri
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUriStruct *pSrcUri,
	EcrioSigMgrUriStruct **ppDstUri
);

u_int32 _EcrioSigMgrPopulateUri
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUriStruct *pSrcUri,
	EcrioSigMgrUriStruct *pDstUri
);

u_int32 _EcrioSigMgrFillUriStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUriStruct *pUri,
	u_char *pUserID,
	EcrioSigMgrURISchemesEnum eUriType
);

u_int32 _EcrioSigMgrAddFeatureTag
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUriStruct *pUri
);

u_int32 _EcrioSigMgrPopulateSipUri
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipURIStruct *pSrcSipUri,
	EcrioSigMgrSipURIStruct *pDstSipUri
);

u_int32 _EcrioSigMgrPopulateTelUri
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrTelURIStruct *pSrcSipUri,
	EcrioSigMgrTelURIStruct *pDstSipUri
);

u_int32 _EcrioSigMgrPopulateTelParams
(
	EcrioSigMgrStruct *pSigMgr,
	u_int16	srcNumTelParams,
	EcrioSigMgrTelParStruct **ppSrcTelParams,
	u_int16	*pDstNumTelParams,
	EcrioSigMgrTelParStruct ***pppDstTelParams
);

u_int32 _EcrioSigMgrAllocateAndPopulateNameAddr
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrNameAddrStruct *pSrcNameAddr,
	EcrioSigMgrNameAddrStruct **ppDstNameAddr
);

u_int32 _EcrioSigMgrPopulateNameAddr
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrNameAddrStruct *pSrcNameAddr,
	EcrioSigMgrNameAddrStruct *pDstNameAddr
);

u_int32 _EcrioSigMgrAllocateAndPopulateNameAddrWithParams
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrNameAddrWithParamsStruct *pSrcNameAddrParams,
	EcrioSigMgrNameAddrWithParamsStruct **ppDstNameAddrParams
);

u_int32 _EcrioSigMgrPopulateNameAddrWithParams
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrNameAddrWithParamsStruct *pSrcNameAddrParams,
	EcrioSigMgrNameAddrWithParamsStruct *pDstNameAddrParams
);

u_int32 _EcrioSigMgrPopulateContact
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrContactStruct *pSrcUri,
	EcrioSigMgrContactStruct *pDstUri
);

u_int32 _EcrioSigMgrCompareUris
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUriStruct *pSrcUri,
	EcrioSigMgrUriStruct *pDestUri,
	BoolEnum *result
);

u_int32 _EcrioSigMgrValidateUri
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUriStruct *pUri,
	BoolEnum *bValid
);

u_int32 _EcrioSigMgrFillSipURIStruct
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pIdentifier,
	u_char *pDomain,
	EcrioSigMgrSipURIStruct **ppSigMgrSipURIStruct
);

u_int32 _EcrioSigMgrCreateSessionId
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pCallId,
	u_char **ppSessionId
);

u_int32 _EcrioSigMgrSIPGetUriParametersAndHeaders
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pUriIndex,
	EcrioSigMgrParamStruct **ppParamList,
	u_int16 *pNoOfParams,
	_EcrioSigMgrSipUriTokenEnum tokenEnum
);

u_int32 _EcrioSigMgrGetNameValuePair
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pSeparator,
	EcrioSigMgrParamStruct *pNameValuePair,
	_EcrioSigMgrSipUriTokenEnum enumTokenType,
	u_char *pMainString
);

BoolEnum _EcrioSigMgrIsTelUri
(
	u_char *pUri
);

/* This function is used to check wheather the Userinfo part of the SIP Uri is a number(global as well as local) */
BoolEnum _EcrioSigMgrSIPUriUserInfoIsNumber
(
	u_char *pSIPUriUserInfo
);

u_int32 _EcrioSigMgrAddEventHdr
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pEventName,
	u_char *pEventId,
	EcrioSigMgrHeaderStruct *pHdr
);

u_int32 _EcrioSigMgrGetHeaderParamValByName
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrHeaderStruct *pHdrStruct,
	u_char *pParameterName,
	u_char **ppParameterVal
);

u_int32 _EcrioSigMgrGetParamValByName
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrParamStruct **ppParams,
	u_int16 noOfParameters,
	u_char *pParameterName,
	u_char **ppParameterVal
);

u_int32 _EcrioSigMgrGetHeaderValue
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrHeaderStruct *pHdrStruct,
	u_char **ppHeaderVal
);

u_int32 _EcrioSigMgrGenerateHeaderURIValue
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUriStruct *pUriInfo,
	u_char **ppURI
);

u_int32 _EcrioSigMgrPopulateHdrValueFromString
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pHdrValue,
	EcrioSigMgrHeaderValueStruct **ppHdrValue
);

u_int32 _EcrioSigMgrPopulateParametersFromString
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pParameters,
	EcrioSigMgrParamStruct **ppParameter
);

u_int32 _EcrioSigMgrGenerateReqURIFromRouteString
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUriStruct *pReqURI,
	u_int16 routeCount,
	u_char **ppRouteSet,
	u_char **ppReqUri
);

u_int32 _EcrioSigMgrGenerateReqURIFromSIPURI
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipURIStruct *pUriInfo,
	BoolEnum bCopyParam,
	BoolEnum isContactUri,
	u_char **ppURI
);

u_int32 EcrioSigMgrCreateMatchingParam
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pBranch,
	EcrioSigMgrHeaderStruct *pEventHeader,
	u_char **ppMatchingParam
);

#ifdef SIG_PH_3
u_int32 _EcrioSigMgrAddUnknownMsgBody
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUnknownMessageBodyStruct *pSrcUnknownMsgBody,
	EcrioSigMgrUnknownMessageBodyStruct **ppDstUnknownMsgBody
);
#endif	// #ifdef SIG_PH_3
u_int32 _EcrioSigMgrAddServiceRoute
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrRouteStruct *pServiceRoute
);

u_int32 _EcrioSigMgrClearServiceRoute
(
	EcrioSigMgrStruct *pSigMgr
);

u_int32 _EcrioSigMgrAppendServiceRouteToRouteSet
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrRouteStruct **ppRouteSet
);

/*	Support for Authorization header in all request	start	*/
u_int32 _EcrioSigMgrCopyACKCredentials
(
	EcrioSigMgrStruct *pSigMgr,
	u_int16	*pDestNumProxyAuth,
	EcrioSigMgrAuthorizationStruct ***pppDestProxyAuth,
	EcrioSigMgrAuthorizationStruct **ppDestAuth,
	u_int16	srcNumProxyAuth,
	EcrioSigMgrAuthorizationStruct **ppSrcProxyAuth,
	EcrioSigMgrAuthorizationStruct *pSrcAuth
);

u_int32 _EcrioSigMgrCopyCredentials
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pMethod,
	u_char *pEntityBody
);

u_int32	_EcrioSigMgrUpdateSharedCredentialsFrom401407Resp
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrAuthenticationStruct	*pSrcAuth,
	u_char *pRequestUri,
	u_char *pEntityBody,
	EcrioSigMgrMethodTypeEnum eMethod,
	u_int32	responseCode
);

u_int32 _EcrioSigMgrGetAuthCredential
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrAuthenticationStruct *pAuthResponse,
	u_int32 nonceCount,
	const u_char *pRequestURI,
	const u_char *pEntityBody,
	EcrioSigMgrMethodTypeEnum eMethod,
	EcrioSigMgrAuthorizationStruct **ppAuthRequest
);

u_int32 _EcrioSigMgrSetReceivedOnChannelIndexAndRemoteSendingPort
(
	void *sigMgrHandle,
	u_int32 channelId,
	u_int32	remotePort
);

u_char *_EcrioSigMgrStripLeadingLWS
(
	u_char *pStart,
	u_char *pEnd
);
u_char *_EcrioSigMgrStripTrailingLWS
(
	u_char *pStart,
	u_char *pEnd
);
u_char *_EcrioSigMgrTokenize
(
	u_char *pStart,
	u_char *pEnd,
	u_char dLimiter
);
u_char *_EcrioSigMgrTokenizeQuotedString
(
	u_char *pStart,
	u_char *pEnd,
	u_char dLimiter
);

u_int32 _EcrioSigMgrSUEExtnStructRelease
(
	EcrioSigMgrStruct *pSigMgr,
	void **ppData,
	u_int32	eDataType,
	BoolEnum bReleaseParent
);

u_int32 _EcrioSigMgrModifyReceivedOnChannelIndex
(
	EcrioSigMgrStruct *pSigMgr,
	u_int32	remoteSendingPort,
	u_int32 receivedOnChannelId,
	EcrioTxnMgrTransportStruct *pSMSACStruct
);

u_int32	_EcrioSigMgrSendIntialRegister
(
	EcrioSigMgrStruct *pSigMgr
);

u_int32	_EcrioSigMgrSUEExtnCountTimedoutProtectedMessage
(
	EcrioSigMgrStruct *pSigMgr,
	BoolEnum bTimedout
);

u_int32 _EcrioSigMgrConstructHeaderWithValue
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrHeaderStruct **ppHeader,
	u_char *pValue
);

u_int32 _EcrioSigMgrExtractHostPort
(
	u_char *pStart,
	u_char *pEnd,
	u_char **ppHost,
	u_int16	*pPort
);

u_int32 _EcrioSigMgrPopulateManHdrs
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrMandatoryHeaderStruct *pSrcManHdrs,
	EcrioSigMgrMandatoryHeaderStruct *pDstManHdrs
);

u_int32 _EcrioSigMgrCopySipMessage
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSrcStruct,
	EcrioSigMgrSipMessageStruct *pDstStruct
);

u_int32 _EcrioSigMgrCopySipMessageBody
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrMessageBodyStruct *pSrcStruct,
	EcrioSigMgrMessageBodyStruct *pDstStruct
);

u_char *_EcrioSigMgrFindTagParam
(
	EcrioSigMgrStruct *pSigMgr,
	u_int32	numParams,
	EcrioSigMgrParamStruct **ppParams
);

u_int32	_EcrioSigMgrAppendParam
(
	EcrioSigMgrStruct *pSigMgr,
	u_int16	*pNumParams,
	EcrioSigMgrParamStruct ***pppParams,
	u_char *pName,
	u_char *pValue
);

u_int32 _EcrioSigMgrBase64Encode
(
	u_char **ppBase64Data,
	u_char *pSrcData,
	u_int32 srcDataLen,
	u_int32 base64DataLen
);

u_int32 _EcrioSigMgrBase64EncodeRequireBufferLength
(
	u_int32 len
);

u_int32 _EcrioSigMgrBase64Decode
(
	u_char **ppDecodedData,
	u_char *pBase64Data,
	u_int32 base64DataLen,
	u_int32 decodedDataLen
);

u_char *_EcrioSigMgrFindCharInString
(
	u_char *pString,
	u_char chr
);

u_int32 _EcrioSigMgrBase64DecodeRequireBufferLength
(
	u_int32 len
);

u_int32 _EcrioSigMgrGetAKARes
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pNonce,					/* Nonce value contained in the challenge */
	EcrioSipAuthAlgorithmEnum eAuthAlgo,/*	EcrioSipAuthAlgorithmAKAv1 or EcrioSipAuthAlgorithmAKAv2	*/
	EcrioSipAuthStruct **ppAuthResponse
);											/* Output: AKA RES, CK, IK, AUTS */

u_int32 _EcrioSigMgrDecomposeAKANonce
(
	u_char *pNonce,
	EcrioSipAuthStruct *pAuthResponse
);

EcrioSigMgrCallbackRegisteringModuleEnums _EcrioSigMgrGetModuleId
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pCallId
);

u_int32 _EcrioSigMgrGetConversationsIdHeader
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessage
);

u_int32 _EcrioSigMgrStoreModuleRoutingInfo
(
	EcrioSigMgrStruct *pSigMgr,
	u_char* pCallId,
	EcrioSigMgrCallbackRegisteringModuleEnums eModuleId
);

EcrioSigMgrCallbackRegisteringModuleEnums _EcrioSigMgrFindModuleId
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessage
);

u_int32 ec_MapInit
(
	MAPHANDLE* ppHandle
);

u_int32 ec_MapInsertData
(
	MAPHANDLE pHandle, 
	u_char* key, 
	void* pData
);

u_int32 ec_MapGetKeyData
(
	MAPHANDLE pHandle, 
	u_char* key, 
	void** ppData
);

u_int32 ec_MapDeleteKeyData
(
	MAPHANDLE pHandle, 
	u_char* key
);

u_int32 ec_MapGetNextData
(
	MAPHANDLE pHandle, 
	void** ppData
);

u_int32 ec_MapDeInit
(
	MAPHANDLE pHandle
);

// UDP Keep Alive Support

u_char* _EcrioSigMgrCreateUdpKeepAlivePacket
(
	LOGHANDLE pLogHandle,
	u_char* pPublicIdentity
);

u_int32 _EcrioSigMgrStartUdpKeepAlive
(
	EcrioSigMgrStruct *pSigMgr
);

u_int32 _EcrioSigMgrStopUdpKeepAlive
(
	EcrioSigMgrStruct *pSigMgr
);

void _EcrioSigMgrUdpKeepAliveCallback
(
	void *pData,
	TIMERHANDLE timerID
);

u_int32 _EcrioSigMgrCreateSecurityClientHdr
(
	EcrioSigMgrStruct *pSigMgr,
	void *pOptionalHeaderList
);

u_int32 _EcrioSigMgrCreateSecurityVerifyHdr
(
	EcrioSigMgrStruct *pSigMgr,
	void **ppOptionalHeaderList
);

u_int32 _EcrioSigMgrCreateTempIPSecCommunicationChannel
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioTxnMgrSignalingCommunicationChannelInfoStruct	*pChannelInfo
);

u_int32 _EcrioSigMgrExtractSecurityServerFromRegisterResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct	*pRegResp,
	BoolEnum *bValidSecurityServer
);

u_int32 _EcrioSigMgrFreeIPSecNegParams
(
	EcrioSigMgrStruct *pSigMgr
);

u_int32 _EcrioSigMgrTerminateChannel
(
	EcrioSigMgrStruct *pSigMgr,
	SOCKETHANDLE socketHandle
);

#endif /* _ECRIO_SIG_MGR_UTILITES_H_ */
