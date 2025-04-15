/******************************************************************************

Copyright (c) 2018-2020 Ecrio, Inc. All Rights Reserved.

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
 * @file EcrioIMDN.h
 * @brief This file defines the public interface of the IMDN module.
 *
 */

#ifndef __ECRIOIMDN_H__
#define __ECRIOIMDN_H__

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
#define ECRIO_IMDN_NO_ERROR	((u_int32)0)					/**< Implies that the IMDN API function call was successful. */

/**
 * The overall IMDN instance handle. There should only be one.
 */
typedef void *IMDN_HANDLE;

/** @enum IMDNDisNtfTypeReqEnum
 * This enumeration describes the type of Disposition Notification requested
 * by the message sender.
 */
typedef enum
{
	IMDNDisNtfTypeReq_None,					/**< No disposition-Notification type defined. */
	IMDNDisNtfTypeReq_Delivery,				/**< <delivery-notification> element. */
	IMDNDisNtfTypeReq_Display				/**< <display-notification> element. */
} IMDNDisNtfTypeReqEnum;

/** @enum IMDNDeliveryNotifEnum
 * This enumeration describes the result of the disposition request for
 * delivery-notification element.
 */
typedef enum
{
	IMDNDeliveryNotif_None,					/**< No Delivery-Notification type defined. */
	IMDNDeliveryNotif_Delivered,			/**< Delivery notification indicating "delivered". */
	IMDNDeliveryNotif_Failed,				/**< Delivery notification indicating "failed". */
	IMDNDeliveryNotif_Forbidden,			/**< Delivery notification indicating "forbidden". */
	IMDNDeliveryNotif_Error					/**< Delivery notification indicating "error". */
} IMDNDeliveryNotifEnum;

/** @enum IMDNDisplayNotifEnum
 * This enumeration describes the display notification for delivery-notification
 * element.
 */
typedef enum
{
	IMDNDisplayNotif_None,					/**< No Display-Notification type defined. */
	IMDNDisplayNotif_Displayed,				/**< Display notification indicating "displayed". */
	IMDNDisplayNotif_Forbidden,				/**< Display notification indicating "forbidden". */
	IMDNDisplayNotif_Error					/**< Display notification indicating "error". */
} IMDNDisplayNotifEnum;


/** @struct IMDNDispoNotifStruct
 * This structure is used to compose the Disposition Notification. This will be
 * used to build disposition-notification for sending and receiving disposition-
 * notification when received it.
 *
 * The Disposition-Notification can describe multiple elements in one message,
 * to set multiple notifications, specify them to eDispoNotif with bitmask. When
 * any values are contained, each value will set by valid number.
 */
typedef struct
{
	IMDNDisNtfTypeReqEnum eDisNtf;			/**< Enumeration that indicates the "Disposition-Notification" element. */

	char *pMsgId;							/**< Pointer to the Message-ID string that indicating which message ID was processed. */
	u_char *pDateTime;						/**< Pointer to the date-time string. The syntax of date-time should be a profile of ISO8601 style (yyyyMMdd'T'HHmmss.SSSZ). */

	IMDNDeliveryNotifEnum eDelivery;		/**< Enumeration that indicates the result of delivery request. */
	IMDNDisplayNotifEnum eDisplay;			/**< Enumeration that indicates the display notification. */
	char *pRecipientUri;							/**  <recipient-uri value. */
	char *pOriginalRecipientUri;					/**  < original-recipient-uri value */
} IMDNDispoNotifStruct;

/** @struct IMDNConfigStruct
 * This structure is provided to the EcrioIMDNInit() function to supply various
 * configuration parameters.
 */
typedef struct
{
	LOGHANDLE logHandle;				/**< The log handle provided by the Ecrio PAL logging mechanism. */
	u_int32 uAllocationSize;			/**< The size of allocated buffer which the IMDN module can consume at the initializing as instance. */
	void *pAllocated;					/**< Pointer to the allocated buffer. */
} IMDNConfigStruct;

/**
 * This function is used to initialize an overall IMDN implementation instance.
 * This function acts synchronously, and upon return the IMDN instance should be
 * prepared to process sessions.
 *
 * The function will use the pAllocated pointer as the instance memory and
 * return it. The caller should not modify this memory or pointer until after
 * calling EcrioIMDNDeinit() function.
 *
 * @param[in] pConfig			Pointer to a IMDNConfigStruct structure. Must be
 *								valid. The caller owns the structure and the IMDN
 *								implementation will consume or copy from it.
 * @param[out] pError			Pointer to a u_int32 to place the result code in
 *								upon return. Must be valid.
 * @return Returns a valid IMDN instance handle. If NULL, pError will provide
 *	the error code. Callers only need to rely on the function return, it is not
 *	necessary to check both the return and pError. They will be guaranteed to
 *	be consistent.
 */
IMDN_HANDLE EcrioIMDNInit
(
	IMDNConfigStruct *pConfig,
	u_int32 *pError
);

/**
 * This function is used to deinitialize the IMDN implementation instance,
 * cleaning up all consumed resources. The caller may release the instance
 * memory after calling this function.
 *
 * @param[in] handle			The IMDN instance handle.
 * @return Returns ECRIO_IMDN_NO_ERROR if successful, otherwise an error.
 */
u_int32 EcrioIMDNDeinit
(
	IMDN_HANDLE handle
);

/**
 * This function is used to form the XML string for IMDN message that indicating
 * MSRP packets associated with the Message-ID have been delivered or displayed.
 *
 * @param[in] handle			The IMDN instance handle.
 * @param[in] pStruct			Pointer to a IMDNDispoNotifStruct structure.
 *								Must be valid.
 * @param[out] pLen				Pointer to the variable indicating length of
 *								generated IMDN message.
 * @param[out] pError			Pointer to the error code.
 * @return Returns a valid pointer indicated to the IMDN message if successful.
 *  This pointer is allocated into the IMDN module, it should not free by
 *  function caller. If NULL, pError will provide the error code.
 */
u_char *EcrioIMDNForm
(
	IMDN_HANDLE handle,
	IMDNDispoNotifStruct *pStruct,
	u_int32 *pLen,
	u_int32 *pError
);

/**
 * This function is used to parse the IMDN message. All parsed elements are
 * stored in the IMDNDispoNotifStruct structure.
 *
 * @param[in] handle			The IMDN instance handle.
 * @param[out] pStruct			Pointer to a IMDNDispoNotifStruct structure.
 *								Must be valid.
 * @param[in] pData				Pointer to the buffer of the IMDN message.
 * @param[in] uLen				The length of the IMDN message.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 EcrioIMDNHandler
(
	IMDN_HANDLE handle,
	IMDNDispoNotifStruct *pStruct,
	u_char *pData,
	u_int32 uLen
);

#ifdef __cplusplus
}
#endif

#endif /* #ifdef __ECRIOIMDN_H__ */

