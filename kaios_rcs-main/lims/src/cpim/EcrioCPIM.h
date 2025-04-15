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
 * @file EcrioCPIM.h
 * @brief This file defines the public interface of the CPIM module.
 *
 */

#ifndef __ECRIOCPIM_H__
#define __ECRIOCPIM_H__

/* Allow this interface to be called directly from C++ compiled code. */
#ifdef __cplusplus
extern "C" {
#endif

/** @name Constant Number Definitions
 *
 */


/** @name Status Codes
 * 
 */
#define ECRIO_CPIM_NO_ERROR	((u_int32)0)		/**< Implies that the CPIM API function call was successful. */
#define CPIM_MSG_NUM						2	/**< Maximum number of the CPIM message bodies per one data. */

/**
 * The overall CPIM instance handle. There should only be one.
 */
typedef void *CPIM_HANDLE;


/** @enum CPIMContentTypeEnum
 * This enumeration describes the content types used to identify the
 * MIME content type which described the "Content-Type" header.
 */
typedef enum
{
	CPIMContentType_None					= 0x00,	/**< No content-type defined. */
	CPIMContentType_Text					= 0x01,	/**< Plain text message, indicate the "text/plain". */
	CPIMContentType_IMDN					= 0x02,	/**< Disposition Notification, indicate the "message/imdn+xml". */
	CPIMContentType_FileTransferOverHTTP	= 0x03,	/**< File Transfer, indicate the "application/vnd.gsma.rcs-ft-http+xml". */
	CPIMContentType_PushLocation			= 0x04,	/**< Geolocation PUSH, indicate the "application/vnd.gsma.rcspushlocation+xml". */
	CPIMContentType_MessageRevoke			= 0x05,	/**< Message Revoke, indicate the "application/vnd.gsma.rcsrevoke+xml". */
	CPIMContentType_PrivacyManagement		= 0x06,	/**< Privacy Management, indicate the "application/vnd.gsma.rcsalias-mgmt+xml". */
	CPIMContentType_LinkReport				= 0x07,	/**< Link Report, indicate the "application/vnd.gsma.rcslink-report+xml". */
	CPIMContentType_SpamReport				= 0x08,	/**< Spam Report, indicate the "application/vnd.gsma.rcsspam-report+xml". */
	CPIMContentType_RichCard				= 0x09,	/**< Rich Cards, indicate the "application/vnd.gsma.botmessage.v1.0+json". */
	CPIMContentType_SuggestedChipList		= 0x0A,	/**< Suggested Chip List, indicate the "application/vnd.gsma.botsuggestion.v1.0+json". */
	CPIMContentType_SuggestionResponse		= 0x0B,	/**< Client Response to Suggestion, indicate the "application/vnd.gsma.botsuggestion.response.v1.0+json". */
	CPIMContentType_SharedClientData		= 0x0C,	/**< Data shared by the client to the Chatbot, indicate the "application/vnd.gsma.botsharedclientdata.v1.0+json". */
	CPIMContentType_CpmGroupData			= 0x0D,	/**< CPM Group Data, indicate the "application/vnd.oma.cpm-groupdata+xml". */
	CPIMContentType_Specified				= 0xFF	/**< Specified Content-Type. */
} CPIMContentTypeEnum;

/** @enum CPIMDispoNotifReqEnum
 * This enumeration describes the type of Disposition Notification requested
 * by the message sender to add the "imdn.Disposition-Notification" header.
 * There can specify multiple notifications by bitmask.
 *
 * If this request was included in received message, the message recipient
 * should send the corresponding IMDN message.
 */
typedef enum
{
	CPIMDispoNotifReq_None		= 0x00,	/**< No request defined. */
	CPIMDispoNotifReq_Negative	= 0x01,	/**< "negative-delivery" notification. */
	CPIMDispoNotifReq_Positive	= 0x02,	/**< "positive-delivery" notification. */
	CPIMDispoNotifReq_Display	= 0x04,	/**< "display" notification. */
	CPIMDispoNotifReq_Last		= 0x04,	/**< The last enumerated value, used for validation. Not used in bitmasks. */
	CPIMDispoNotifReq_All		= 0x07	/**< All directions, provided for convenience. */
} CPIMDispoNotifReqEnum;

/** @enum CPIMTrafficTypeEnum
 * This enumeration describes traffic type to add CPIM namespace for new
 * Messaging as a Platform (MaaP) related CPIM headers.
 */
typedef enum
{
	CPIMTrafficType_None			= 0x00,	/**< No traffic type defined. */
	CPIMTrafficType_Advertisement	= 0x01,	/**< "advertisement" token. */
	CPIMTrafficType_Payment			= 0x02,	/**< "payment" token. */
	CPIMTrafficType_Premium			= 0x03,	/**< "premium" token. */
	CPIMTrafficType_Subscription	= 0x04,	/**< "subscription" token. */
	CPIMTrafficType_Plugin			= 0x05	/**< "plugin" token. */
} CPIMTrafficTypeEnum;


/** @struct CPIMConfigStruct
 * This structure is provided to the EcrioCPIMInit() function to supply various
 * configuration parameters.
 */
typedef struct
{
	LOGHANDLE logHandle;				/**< The log handle provided by the Ecrio PAL logging mechanism. */
	u_int32 uAllocationSize;			/**< The size of allocated buffer which the CPIM module can consume at the initializing as instance. */
	void *pAllocated;					/**< Pointer to the allocated buffer. */
} CPIMConfigStruct;

/** @struct CPIMMessageBufferStruct
 * This structure is used to compose the MIME message body for CPIM message.
 */
typedef struct
{
	CPIMContentTypeEnum eContentType;	/**< MIME content type. */
	u_char *pContentType;				/**< Content-Type string if other than specified MIME content type. */
	u_char *pMsgBody;					/**< Pointer to the buffer of message body. */
	u_int32 uMsgLen;					/**< The length of the message, in bytes. */
	u_char *pContentId;					/**< The content Id string. */
	u_char *pContentDisposition;		/**< The content disposition string. */
	u_char *pContentTransferEncoding;	/**< The content transfer encoding string. */
} CPIMMessageBufferStruct;

/** @struct CPIMMessageStruct
 * This structure is used to compose the CPIM message.
 *
 * When we send only plain text message, it should fill the message body and
 * length, and set 1 to uNumOfBuffers. If CPIM body including the multiple
 * message bodies as XML and json, it will set number of message bodies to
 * uNumOfBuffers and fill two structures to buff. The CPIM module appropriately
 * sets Content-Type header and message body as the multipart CPIM body.
 *
 * This will be also used for receiving notification when receive plain text
 * message, these parameters will be filled by received message. If message
 * body is divided into multiple parts, the stored message types will be set
 * to appropriate values and each message body will store as well.
 */
typedef struct
{
	/** Note: CPIM To and From header will be set to the identity of the sender
	 * of the message for Delivery and Display notifications within Group Chat.
	 * Conversely, these headers should be set to "anonymous" within 1-to-1 Chat
	 * for privacy protection. If pToAddr or pFromAddr set to NULL then it will
	 * be automatically set to "anonymous".
	 */
	u_char *pToAddr;						/**< Pointer to the string of SIP To-address. This is used for adding CPIM-To header. */
	u_char *pFromAddr;						/**< Pointer to the string of SIP From-address. This is used for adding CPIM-From header. */
	u_char *pDisplayName;					/**< Pointer to the string of display anme. This is used for adding in CPIM-From header. */
	u_char *pDateTime;						/**< Pointer to the date-time string. The syntax of date-time should be a profile of ISO8601 style (yyyyMMdd'T'HHmmss.SSSZ). */
	u_char *pMsgId;							/**< Pointer to the Message-ID string which assigned to this message. */
	u_int32 uMsgIdLen;						/**< The length of the Message-ID, in bytes. */
	u_char *pPANI;							/**< Pointer to the Message-ID string which assigned to this message. */
	CPIMDispoNotifReqEnum dnType;			/**< Enumeration that indicates the Dispositino-Notification type. */
	CPIMTrafficTypeEnum trfType;			/**< Enumeration that indicates the traffic type. */

	u_int32 uNumOfBuffers;					/**< Number of message bodies. */
	CPIMMessageBufferStruct buff[CPIM_MSG_NUM];	/**< Structure of message bodies. */
} CPIMMessageStruct;


/**
 * This function is used to initialize an overall CPIM implementation instance.
 * This function acts synchronously, and upon return the CPIM instance should be
 * prepared to process sessions.
 *
 * The function will use the pAllocated pointer as the instance memory and
 * return it. The caller should not modify this memory or pointer until after
 * calling EcrioCPIMDeinit() function.
 *
 * @param[in] pConfig			Pointer to a CPIMConfigStruct structure. Must be
 *								valid. The caller owns the structure and the CPIM
 *								implementation will consume or copy from it.
 * @param[out] pError			Pointer to a u_int32 to place the result code in
 *								upon return. Must be valid.
 * @return Returns a valid CPIM instance handle. If NULL, pError will provide
 *	the error code. Callers only need to rely on the function return, it is not
 *	necessary to check both the return and pError. They will be guaranteed to
 *	be consistent.
 */
CPIM_HANDLE EcrioCPIMInit
(
	CPIMConfigStruct *pConfig,
	u_int32 *pError
);

/**
 * This function is used to deinitialize the CPIM implementation instance,
 * cleaning up all consumed resources. The caller may release the instance
 * memory after calling this function.
 *
 * @param[in] handle			The CPIM instance handle.
 * @return Returns ECRIO_CPIM_NO_ERROR if successful, otherwise an error.
 */
u_int32 EcrioCPIMDeinit
(
	CPIM_HANDLE handle
);

/**
 * This function is used to form the CPIM message. In this function, a CPIM
 * header and message body specified by CPIMMessageStruct structure. The CPIM
 * module is not concern about contents of message body, it only adds the
 * specified Content-Type and message body. Therefore, the contents of each
 * messages should ensure by the function caller.
 *
 * @param[in] handle			The CPIM instance handle.
 * @param[in] pStruct			Pointer to a CPIMMessageStruct structure.
 *								Must be valid.
 * @param[out] pLen				Pointer to the variable indicating length of
 *								generated CPIM message.
 * @param[out] pError			Pointer to the error code.
 * @return Returns a valid pointer indicated to the CPIM message strings if
 *  successful. This pointer is allocated into the CPIM module, it should not
 *  free by function caller. If NULL, pError will provide the error code.
 */
u_char *EcrioCPIMForm
(
	CPIM_HANDLE handle,
	CPIMMessageStruct *pStruct,
	u_int32 *pLen,
	u_int32 *pError
);

/**
 * This function is used to parse the CPIM message, it is parsed into the
 * CPIM header part and the message body part. Since the contents of CPIM
 * message body are not parsed, it need to handle the content of each CPIM
 * message with reference to the MIME content type by the function caller.
 *
 * @param[in] handle			The CPIM instance handle.
 * @param[out] pStruct			Pointer to a CPIMMessageStruct structure.
 *								Must be valid.
 * @param[in] pData				Pointer to the buffer of the CPIM message.
 * @param[in] uLen				The length of the CPIM message.
 * @return ECRIO_CPIM_NO_ERROR if successful, otherwise a specific error.
 */
u_int32 EcrioCPIMHandler
(
	CPIM_HANDLE handle,
	CPIMMessageStruct *pStruct,
	u_char *pData,
	u_int32 uLen
);

#ifdef __cplusplus
}
#endif

#endif /* #ifdef __ECRIOCPIM_H__ */

