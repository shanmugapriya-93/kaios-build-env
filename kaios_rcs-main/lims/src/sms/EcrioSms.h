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
 * @file EcrioSms.h
 * @brief This is the header file for the public interface of the SMS ALC.
 *
 * The SMS module acts as a layer between a LIMS module and the IMS Client Framework,
 * providing SMS messaging capability specific to a particular network or operator.
 */

#ifndef __ECRIOSMS_H__
#define __ECRIOSMS_H__

/* Import Data Types */
#include "EcrioPAL.h"
#include "EcrioSmsDataTypes.h"

/* Allow this module to be called directly from C++ compiled code. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * The SMS Handle is a void pointer which will point to a structure known only internally
 * to the SMS module. The calling layer should always reference the handle by this type.
 */
typedef void *ECRIOSMSHANDLE;

/**********************************************************************************************/
/*                                     Error Definition - START                               */
/**********************************************************************************************/

/** @name General Return Code Definitions
 *
 */
// @{
#define ECRIO_SMS_NO_ERROR						(0x00)		/**< Indicates success when an API is called. */
#define ECRIO_SMS_MEMORY_ERROR					(0x01)		/**< Indicates memory allocation failed when an API is called. */
#define ECRIO_SMS_INTERNAL_ERROR				(0x02)		/**< Indicates internal error occured when an API is called. */
#define ECRIO_SMS_NOT_INITIALIZED_ERROR			(0x03)		/**< Indicates SMS not initialized when an API is called. */
#define ECRIO_SMS_BUSY_ERROR					(0x04)		/**< Indicates another SMS is in progress when a SMS is initiated. */
#define ECRIO_SMS_INSUFFICIENT_DATA_ERROR		(0x05)		/**< Indicates parameters not passed proprly when an API is called. */
// @}

/**********************************************************************************************/
/*                                     Error Definition - END                               */
/**********************************************************************************************/

/**********************************************************************************************/
/*                         Callback Functions Definition - START                              */
/**********************************************************************************************/

/** @name Callback Functions
 * Callback functions must be implemented by the calling layer and are invoked
 * from the SMS in response to various actions and events, described
 * elsewhere in this document.
 */
// @{

/**
 * This function will be called whenever the SMS must notify the calling
 * layer about the status of actions and events. This function must be
 * implemented by the calling layer and provided to the EcrioSMSInit()
 * function by function pointer.
 *
 * @param[in] status		The type of status notification. This enumerated
 *							value should be used to decide which type the
 *							associated statusData parameter points to.
 * @param[in] data			A void pointer to the associated data structure
 *							for the status notification.
 * @param[in] pContext		Pointer to any optional data provided by the
 *							calling layer, opaque to the SMS module. This pointer
 *							would have been provided to the SMS during
 *							initialization.
 */
typedef void (*EcrioSmsCallback)
(
	EcrioSmsMessageStatusType status,
	void *data,
	void *pContext
);

// @}
/**********************************************************************************************/
/*                           Callback Functions Definition - END                              */
/**********************************************************************************************/

/**********************************************************************************************/
/*                             Data Structure Definition - START                                  */
/**********************************************************************************************/

typedef struct
{
	LOGHANDLE hLogHandle;	/**< Pointer to the log handle. */
} EcrioSmsParamStruct;

/** @struct EcrioSmsCallbackStruct
 * Callback structure used when the SMS is initialized so all callback
 * function pointers and associated data can be given to the SMS ALC.
 */
typedef struct
{
	EcrioSmsCallback pCallback;								/**< Callback for notifying the status, an action or event. */
	void *pContext;											/**< Void pointer to some opaque data that should always be
															     communicated back to the calling layer. */
} EcrioSmsCallbackStruct;

/**********************************************************************************************/
/*                             Data Structure Definition - END                                  */
/**********************************************************************************************/

/**********************************************************************************************/
/*                        Interface APIS - START                                             */
/**********************************************************************************************/

/**
 * This function is used to initialize the SMS module. This is a synchronous
 * function which should be called first before any other SMS API
 * function is called. This function will perform basic checks of input
 * parameters and perform basic component initialization. This function
 * returns an SMS Handle as an output parameter which points to the
 * complete internal state of the SMS ALC.
 *
 * @param[in] hLogHandle			Handle to log file to log specific to SMS ALC
 *									this function returns.
 * @param[in] hSigMgrHandle			pointer to SIGMGRHANDLE. EcrioSigMgrInit() function
 *									output this handle. Valid handle should be pass and it should
 *									not be NULL.
 * @param[in] pSmsCallback			Pointer to a structure holding the callback
 *									information used to communicate status information
 *									to the calling layer. This structure should be
 *									allocated and filled by the calling function. The
 *									calling function can free the memory associated
 *									with this pointer after this function returns.
 * @param[out] hSmsHandle			The SMS Handle an output parameter. This is
 *									a pointer to which the SMS layer will allocate
 *									memory. If the referenced pointer is not NULL,
 *									an error will be returned. The calling layer should
 *									never free the memory associated with the handle
 *									since it is needed when invoking any other SMS ALC
 *									API. The EcrioSMSDeinit() should be called to free
 *									the memory.
  * @return							ECRIO_SMS_SUCCESS if successful otherwise a
 *									non-zero value indicating the reason for failure.
 */
u_int32 EcrioSmsInit
(
	EcrioSmsParamStruct *pSmsParam,
	EcrioSmsCallbackStruct *pSmsCallback,
	void *hSigMgrHandle,
	ECRIOSMSHANDLE *hSmsHandle
);

/**
 * This function is used to de-initialize the SMS ALC. This function will clean
 * up all resources and free all memory used by the SMS ALC. This is a synchronous
 * function and can be called at any time. Calling this function without initializing
 * the SMS Handle using the EcrioSMSInit() API could result in unexpected
 * behavior.
 *
 * @param[in,out] hSmsHandle		Pointer to the SMS Handle. If the referenced
 *									pointer is NULL, then the function will return
 *									successfully. If not NULL, an attempt will be
 *									made to clean up resources and memory.
 * @return							ECRIO_SMS_SUCCESS if successful otherwise a
 *									non-zero value indicating the reason of failure.
 */
u_int32 EcrioSmsDeinit
(
	ECRIOSMSHANDLE *hSmsHandle
);

/**
 * This function will initiate the sending of a binary SMS message. This is
 * an asynchronous function and the result will be communicated to the
 * calling layer via a call to the EcrioSMSMessageStatusCallback() callback
 * function. This function can only be called if the
 * EcrioSMSStartDataConnection() function was called and an active data
 * connection has been established and still connected.
 *
 * @param[in] hSmsHandle			The SMS Handle that was created by calling the
 *									EcrioSMSInit() function. If NULL, the function
 *									will return an error.
 * @param[in] pMsgStruct			Pointer to EcrioSmsMessageStruct. Which includes the
 *									message, message length, message format and receipient
 *									uri.
 * @param[out] ppMessageId			SMS will give this parameter as an output for the calling
 *									function. This can be used to track the response for the message
 *                                  sent to the network. The calling component is responsible for
 *                                  freeing the memory, when message ID is no longer needed.
 * @return							ECRIO_SMS_SUCCESS if successful otherwise a
 *									non-zero value indicating the reason of failure.
 */
u_int32 EcrioSmsSendMessage
(
	ECRIOSMSHANDLE hSmsHandle,
	EcrioSmsMessageStruct *pMsgStruct
);

/**********************************************************************************************/
/*                        Interface APIS - END                                             */
/**********************************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*  _ECRIO_SMS_H_ */
