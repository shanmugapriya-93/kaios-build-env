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
* @file EcrioOOM.h
* @brief This is the header file for the Operator Object Model.
*/

#ifndef __ECRIOOOM_H__
#define __ECRIOOOM_H__

/* Import Data Types */
#include "EcrioDataTypes.h"
#include "EcrioSIPDataTypes.h"

/* Allow this module to be called directly from C++ compiled code. */
#ifdef __cplusplus
extern "C" {
#endif

/** @enum EcrioOOMErrors
* Enumerated values of OOM error codes
*/
typedef enum
{
	KOOMErrorNone = 0,						/**< return if there is no error. */
	KOOMInvalidParameters,					/**< one or more input parameters are not valid. */
	KOOMInvalidSize,						/**< a size or length parameter is not acceptable input. */
	KOOMInsufficientParameters				/**< not enough information was provided or data is missing. */
} EcrioOOMErrors;

/** @enum EcrioOOMRetryEnum
*  Enumration for Retry mechanism, the enumeration represents
*  the P-CSCF type where the framework needs to rerty
*  The framework needs to retry either on same P-CSCF or the next
*  available P-CSCF
*/

typedef enum
{
	EcrioOOMRetry_None						= 0,		/**< Indicates no retry or retry in next power cycle.*/
	EcrioOOMRetry_InitialRequest_SamePCSCF	= 1,		/**< Retry initial request to same P-PCSF as that of previous one */
	EcrioOOMRetry_InitialRequest_NextPCSCF	= 2,		/**< Retry initial request to the next available P-PCSF  */
	EcrioOOMRetry_ReRequest_SamePCSCF		= 3			/**< Retry rerequest i.e. reregister or resubscribe to active P-PCSF  */
} EcrioOOMRetryEnum;

/** @enum EcrioOOMCodecEnum
*  Enumration for supported codec, which is used to get RTCP bandwidth values
*  as per codecs. If we support more codecs, it need to add codec types into
*  this enumeration.
*/

typedef enum
{
	EcrioOOMCodec_None = 0,		/**< Indicates no specified codec. */
	EcrioOOMCodec_AMR_NB,		/**< Indicates AMR-NB codec. */
	EcrioOOMCodec_AMR_WB,		/**< Indicates AMR-WB codec. */
	EcrioOOMCodec_General		/**< Indicates general codec, mostly at a sampling rate of 8000 Hz */
} EcrioOOMCodecEnum;

/** @struct EcrioOOMRetryQueryStruct
*  The structure is the input structre to the API ec_oom_GetRetryActionValues()
*  The framework will provide these required information through this structure
*/
typedef struct
{
	BoolEnum bIsInitialRequest;	/**< Indicates initial or re Request */
	u_int16 uResponseCode;		/**< Indicates the SIP Response codes */
	u_char *pReasonPhrase;		/**< Indicates the Reason Phrase associated with Response Code */
	u_int8 uRetryCount;			/**< Indicates retry count */
} EcrioOOMRetryQueryStruct;

/** @struct EcrioOOMRetryValuesStruct
*  The structre is the output of the API() ec_oom_GetRegRetryValues
*  The OOM will prvoide the P-CSCF type along with the retry interval
*/
typedef struct
{
	EcrioOOMRetryEnum eRetryType;	/**< Indicates retrying PCSCF type*/
	u_int16	uRetryInterval;			/**< Indicates retry interval in seconds  */
} EcrioOOMRetryValuesStruct;

/** @struct EcrioOOMRTCPBandwidthStruct
*  The structure is the input structre to the API ec_oom_GetRTCPBandwidthValues()
*  The framework will provide these required information through this structure
*/
typedef struct
{
	u_int16 uAS;				/**< Indicates the AS value. */
	u_int16 uRS;				/**< Indicates the RS value. */
	u_int16 uRR;				/**< Indicates the RR value. */
} EcrioOOMRTCPBandwidthStruct;

/**
* Function is used to get the operator recommended T1, T2 and F timer values. Other timer values will be
* generated as per 3GPP 24.229 Section 7.7 and table 7.7.1. Timer values will be return in milliseconds.
*
* @param[in,out] pTimers       Ponter to the EcrioSipTimerStruct
*
* @return Returns EcrioOOMErrors
*/

typedef u_int32 (*ec_oom_GetSIPTimersPtr)
(
	EcrioSipTimerStruct *pTimers
);

/**
* Function is used to get the operator recommended Session timer interval and minimum session expiry
* timer value. Timer values will be return in milliseconds.
*
* @param[in,out] pTimers       Ponter to the EcrioSipSessionTimerStruct
*
* @return Returns EcrioOOMErrors
*/

typedef u_int32 (*ec_oom_GetSessionTimersPtr)
(
	EcrioSipSessionTimerStruct *pTimers
);

/**
* The function is to get the customized volte feature tags.
* This function will provides the header detail and corresponding customized feature tags
* for requested SIP method. Caller will take care the memory handling. If allocated memory is not enough, function will return
* the memory error with the required size. Caller will reallocate the memory and invoke the function again.
*
* @param[in]  eMethod  Represents SIP Message Type.
* @param[in]  eHeader  Represents SIP Header Type.
*
* @param[out] pFeatureTag The supported VoLTE feature tag
* @param[out] pSize       The size of VoLTE feature tag string
*
* @return Returns EcrioOOMErrors
*/

typedef u_int32 (*ec_oom_GetVolteFeatureTagsPtr)
(
	EcrioSipMessageTypeEnum eMethod,
	EcrioSipHeaderTypeEnum eHeader,
	u_char *pFeatureTag,
	u_int32 *pSize
);

/**
* Function provides the other customized feature tags which needs to include in the requested method. Caller will take care the memory handling.
* If allocated memory is not enough, function will return* the memory error with the required size. Caller will reallocate the memory and invoke
* the function again.
*
* @param[in]  eMethod  Represents SIP Message Type.
* @param[in]  eHeader  Represents SIP Header Type.
*
* @param[out] pFeatureTag The supported feature tag
* @param[out] pSize       The size of feature tag string
*
* @return Returns EcrioOOMErrors
*/

typedef u_int32 (*ec_oom_GetOtherFeatureTagsPtr)
(
	EcrioSipMessageTypeEnum eMethod,
	EcrioSipHeaderTypeEnum eHeader,
	u_char *pFeatureTag,
	u_int32 *pSize
);

/**
* This function provides which SIP messages needs to include the P-Access-Network-Info header.
* returns TRUE if supported for Method/Header combination . otherwise FALSE
*
* @param[in]  eMethod  Represents SIP Message Type.
* @param[in]  eHeader  Represents SIP Header Type.
*
* @return Returns bitmask of EcrioSipMessageTypeEnum
*/
typedef BoolEnum (*ec_oom_HasHeaderSupportPtr)
(
	EcrioSipMessageTypeEnum eMethod,
	EcrioSipHeaderTypeEnum eHeader
);

/**
* This function is used to get the supported methods in Allow header. Function returns bitmask of
* EcrioSipMessageTypeEnum. It will return EcrioSipMessageTypeNone if there is no method allowed.
*
* @param[in]  void  None .
*
* @return Returns bitmask of EcrioSipMessageTypeEnum
*/
typedef u_int32 (*ec_oom_GetAllowMethodsPtr)
(
	void
);

/**
* This function is used for registration retry mechanism handling in the framework
* The function provides retry related values, on the basis of these values, the framrwork
* will take decision to rerty
*
* @param[in]  stRetryQueryStruct		   Structure to EcrioOOMRetryQueryStruct .
* @param[out] pRegRetryValuesStruct		   Pointer to EcrioOOMRetryValuesStruct.

* @return Returns EcrioOOMErrors
*/

typedef u_int32 (*ec_oom_GetRegRetryValuesPtr)
(
	EcrioOOMRetryQueryStruct stRetryQueryStruct,
	EcrioOOMRetryValuesStruct *pRegRetryValuesStruct
);

/**
* This function is used for subscribe retry mechanism handling in the framework
* The function provides retry related values, on the basis of these values, the framrwork
* will take decision to rerty
*
* @param[in]  stRetryQueryStruct		   Structure to EcrioOOMRetryQueryStruct .
* @param[out] pSubRetryValuesStruct		   Pointer to EcrioOOMRetryValuesStruct.

* @return Returns EcrioOOMErrors
*/

typedef u_int32 (*ec_oom_GetSubRetryValuesPtr)
(
	EcrioOOMRetryQueryStruct stRetryQueryStruct,
	EcrioOOMRetryValuesStruct *pSubRetryValuesStruct
);

/**
* This function provides which header for a SIP messages needs to include user = phone parameter.
* returns TRUE if supported for Method/Header/URI Type combination . otherwise FALSE
*
* @param[in]  eMethod  Represents SIP Message Type.
* @param[in]  eHeader  Represents SIP Header Type.
* @param[in]  eURIType  Represents SIP URI Type.
*
* @return Returns TRUE/FALSE
*/
typedef BoolEnum (*ec_oom_HasUserPhoneSupportPtr)
(
	EcrioSipMessageTypeEnum eMethodType,
	EcrioSipHeaderTypeEnum eHeaderType,
	EcrioSipURITypeEnums eURIType
);

/**
* This function checks the anonymous value of CPIM-To header. If it return Enum_TRUE 
* then lims adds anonymous To and From header in CPIM Pager Message.
*
* @return Returns TRUE/FALSE
*/
typedef BoolEnum (*ec_oom_IsAnonymousCPIMHeaderPtr)
(
);


typedef u_int32 (*ec_oom_GetSMSMessageRetryValuesPtr)
(
	EcrioOOMRetryQueryStruct stRetryQueryStruct,
	EcrioOOMRetryValuesStruct *pSMSRetryValuesStruct
);

/**
* Function is used to get the operator recommended waiting timer for meeting precondition.
* Timer values will be return in milliseconds.
*
* @param[in,out] pValue       Pointer to precondition wait-timer 
*
* @return Returns EcrioOOMErrors
*/

typedef u_int32(*ec_oom_GetPreconditionWaitTimerValuePtr)
(
	u_int32 *pValue
);

/**
* This function determines if we need to terminate forked calls for early media or not.
* If it return Enum_TRUE then we need to terminate early forked calls with BYE .
*
* @return Returns TRUE/FALSE
*/
typedef BoolEnum(*ec_oom_IsTerminateEarlyForkedCallsPtr)
(
);

/**
* This function is used to get the bandwidth attribute values to describe in SDP.
* The function provides RTCP bandwidth related values that specified in RFC 3556.
*
* @param[in]  eCodec					Represents codec Type.
* @param[out] pRtcpBwValuesStruct		Pointer to EcrioOOMRTCPBandwidthStruct.

* @return Returns EcrioOOMErrors
*/

typedef u_int32 (*ec_oom_GetRTCPBandwidthValuesPtr)
(
	EcrioOOMCodecEnum eCodec,
	EcrioOOMRTCPBandwidthStruct *pRtcpBwValuesStruct
);

typedef BoolEnum (*ec_oom_GetReasonHeaderValuesPtr)
(
	u_char **ppReasonProto,
	u_char **ppReasonCause,
	u_char **ppReasonText
);

/** @struct EcrioOOMStruct
 *  The structure represents the function pointers of OOM module
 */
typedef struct
{
	ec_oom_GetSIPTimersPtr ec_oom_GetSipTimers;					/**< Function pointer of type ec_oom_GetSIPTimersPtr*/
	ec_oom_GetSessionTimersPtr ec_oom_GetSessionTimers;			/**< Function pointer of type ec_oom_GetSessionTimers*/
	ec_oom_GetVolteFeatureTagsPtr ec_oom_GetVolteFeatureTags;	/**< Function pointer of type ec_oom_GetVolteFeatureTags*/
	ec_oom_GetOtherFeatureTagsPtr ec_ooms_GetOtherFeatureTags;	/**< Function pointer of type ec_ooms_GetOtherFeatureTags*/
	ec_oom_HasHeaderSupportPtr ec_oom_HasHeaderSupport;			/**< Function pointer of type ec_oom_HasHeaderSupport*/
	ec_oom_GetAllowMethodsPtr ec_oom_GetAllowMethod;			/**< Function pointer of type ec_oom_GetAllowMethod*/
	ec_oom_GetRegRetryValuesPtr ec_oom_GetRegRetryValues;		/**< Function pointer of type ec_oom_GetRegRetryValues*/
	ec_oom_GetSubRetryValuesPtr ec_oom_GetSubRetryValues;		/**< Function pointer of type ec_oom_GetSubRetryValues*/
	ec_oom_HasUserPhoneSupportPtr ec_oom_HasUserPhoneSupport;		/**< Function pointer of type ec_oom_HasUserPhoneSupport*/
	ec_oom_IsAnonymousCPIMHeaderPtr ec_oom_IsAnonymousCPIMHeader; /**< Function pointer of type ec_oom_IsAnonymousCPIMHeader*/
	ec_oom_GetSMSMessageRetryValuesPtr ec_oom_GetSMSMessageRetryValues; /**< Function pointer of type ec_oom_GetSMSMessageRetryValues*/
	ec_oom_GetPreconditionWaitTimerValuePtr ec_oom_GetPreconditionWaitTimerValue; /**< Function pointer of type ec_oom_GetPreconditionWaitTimerValue*/
	ec_oom_IsTerminateEarlyForkedCallsPtr ec_oom_IsTerminateEarlyForkedCalls; /**< Function pointer of type ec_oom_IsTerminateEarlyForkedCalls*/
	ec_oom_GetRTCPBandwidthValuesPtr ec_oom_GetRTCPBandwidthValues; /**< Function pointer of type ec_oom_GetRTCPBandwidthValues*/
	ec_oom_GetReasonHeaderValuesPtr ec_oom_GetReasonHeaderValues;	/**< Function pointer of type ec_oom_GetReasonHeaderValues*/
} EcrioOOMStruct;

/**
 * This function is used to get the default oom object.
 *
 */
EcrioOOMStruct *default_oom_GetObject
(
	void
);

/**
 * This function is used to get the vzw oom object.
 *
 */
EcrioOOMStruct *vzw_oom_GetObject
(
	void
);

/**
 * This function is used to get the tmo oom object.
 *
 */
EcrioOOMStruct *tmo_oom_GetObject
(
	void
);

/**
 * This function is used to get the kddi oom object.
 *
 */
EcrioOOMStruct *kddi_oom_GetObject
(
	void
);

/**
 * This function is used to get the att oom object.
 *
 */
EcrioOOMStruct *att_oom_GetObject
(
	void
);

/**
* This function is used to get the att oom object.
*
*/
EcrioOOMStruct *undefined_oom_GetObject
(
	void
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__ECRIOMOIP_H__ */
