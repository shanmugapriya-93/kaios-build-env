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
 * @file EcrioPAL_Common.h
 * @brief This is the common header file for the all the public interfaces of
 * the Ecrio PAL.
 *
 * This header file holds common definitions relevant to all PAL modules. It
 * does not contain any module specific information. This header file can also
 * define constants needed to build the PAL across multiple platforms.
 */

#ifndef __ECRIOPAL_COMMON_H__
#define __ECRIOPAL_COMMON_H__

/** Include the Ecrio Data Type definition policy as part of the Ecrio
 * PAL interface.
 */
#include "EcrioDataTypes.h"

/** @enum EcrioPALErrors
 * The EcrioPALErrors enumeration defines the possible return values that the
 * APIs of the PAL can return. KPALErrorNone is the only success code,
 * however, different PAL Modules can define alternative meanings to
 * some of the more generic error codes. All errors listed here are meant
 * to be generic and compatible with all PAL Modules.
 */
typedef enum
{
	KPALErrorNone = 0,			/**< Indicates success when an API is called. */
	KPALInvalidParameters,		/**< Indicates that one or more parameters are not valid (might be NULL). */
	KPALInvalidSize,			/**< Indicates that a size or length parameter is not acceptable input. */
	KPALInsufficientParameters,	/**< Indicates that not enough information was provided or data is missing. */
	KPALInvalidHandle,			/**< Indicates that a Module handle has not been initialized. */
	KPALNotInitialized,			/**< Indicates that the Module handle is corrupt or the Module's Deinit() function has been called. */
	KPALMemoryError,			/**< Indicates that a memory allocation or other operation has failed. */
	KPALPointerNotNULL,			/**< Indicates that a pointer was passed to be used for a memory allocation, but it is not NULL. */
	KPALNotSupported,			/**< Indicates that the desired operation/property is not supported by the interface. */
	KPALNotAvailable,			/**< Indicates that a requested operation/property is not available for use. */
	KPALNoDeviceSupport,		/**< Indicates that an operation that is desired is not supported by the device or OS. */
	KPALNotAllowed,				/**< Indicates that the requested operation is not allowed. */
	KPALEntityNotFound,			/**< Indicates that the requested entity cannot be found by the implementation. */
	KPALEntityInvalid,			/**< Indicates that the requested entity is not valid for reasons specific to the implementation. */
	KPALEntityClosed,			/**< Indicates that the entity closed its resource (for example, a communications socket was closed.) */
	KPALResourcesInUse,			/**< Indicates that all requested resources are currently in use. */
	KPALBadFormat,				/**< Indicates that the requested format specification is not valid. */
	KPALBufferFull,				/**< Indicates that a buffer or a set of buffers is full (may not be fatal). */
	KPALAlreadyInUse,			/**< Indicates that the requested resource is already in use (may not be fatal). */
	KPALDataSendErrorBenign,	/**< Indicates that data could not be sent, for the specific context (i.e. a socket can't send data), but it may be recoverable. */
	KPALDataSendErrorFatal,		/**< Indicates that data could not be sent, for the specific context (i.e. a socket can't send data), and the context should be closed (i.e. a socket should be closed.) */
	KPALDataReceiveErrorBenign,	/**< Indicates that data could not be received, for the specific context (i.e. a socket receive failed), but it may be recoverable. */
	KPALDataReceiveErrorFatal,	/**< Indicates that data could not be received, for the specific context (i.e. a socket receive failed), and the context should be closed (i.e. a socket should be closed.) */
	KPALInternalError,			/**< Indicates that some sort of implementation specific internal error has occurred. */
	KPALTimeOut,				/**< Indicates that something in the the implementation has timed out. */
	KPALFileError,				/**< Indicates that a file operation (File open, read, write, seek and close) has failed. */
	KPALStarted,				/**< Indicates that a resource is already or currently started. */
	KPALStopped,				/**< Indicates that a resource is already or currently stopped. */
	KPALTLSInternalError,			/**< Indicates failure in TLS during connection, reading, writing or any other SSL operation */
	KPALTLSPeerCertificateError,	/**< Indicates failure, Either peer certificate not found Or when peer verification is set and peer certificate is failed in verification. */
	KPALTLSConnectTimeout,			/**< Indicates failure if time-out is occurred and TLS connection is in incomplete state. */
	KPALUnknownError			/**< Indicates than an unknown error has occurred. */
} EcrioPALErrors;

/** Temporary integration fix-ups for the String Module. */
#define pal_SubString(a, b)										pal_StringFindSubString((a), (b))
#define pal_StringToNum(a, b)									pal_StringConvertToUNum((a), (b), 10)
#define pal_NumToString(a, b, c)									pal_StringSNPrintf((char *)(b), c, "%u", (a))
#define pal_SNumPrintf(a, b, c, d)									pal_StringSNPrintf((a), (b), (c), (d))
#define pal_FindCharInString(a, b)								pal_StringFindChar((a), (b))
#define pal_StringToSignedInt(a)								pal_StringConvertToSNum((a), NULL, 10)
#define pal_SStringPrintf(a, b, c, ...)							pal_StringSNPrintf((a), (b), (c), ##__VA_ARGS__)
#define pal_FindLastCharInString(a, b)							pal_StringFindLastChar((a), (b))
#define pal_StringToNumHex(a, b)								pal_StringConvertToUNum((a), (b), 16)

#define ECRIO_STRING_CREATE(moduleId, srcString, destString)	((destString[]) = (pal_StringCreate((srcString), pal_StringLength((srcString)))))

#define pal_IsDigit(c)											((c) - '0' + 0U <= 9U)

#endif /* #ifndef __ECRIOPAL_COMMON_H__ */
