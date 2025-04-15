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
 * @file EcrioIMDNCommon.h
 * @brief This file defines the primary internal definitions for the Ecrio
 * IMDN implementation.
 */

#ifndef __ECRIOIMDNCOMMON_H__
#define __ECRIOIMDNCOMMON_H__

/* Allow this module to be called directly from C++ compiled code. */
#ifdef __cplusplus
extern "C" {
#endif

/* Implementation level errors. */

#define ECRIO_IMDN_INVALID_HANDLE							8000	/**<  */
#define ECRIO_IMDN_INVALID_PARAMETER_1						8001	/**<  */
#define ECRIO_IMDN_INVALID_PARAMETER_2						8002	/**<  */
#define ECRIO_IMDN_INVALID_PARAMETER_3						8003	/**<  */
#define ECRIO_IMDN_INVALID_PARAMETER_4						8004	/**<  */
#define ECRIO_IMDN_INVALID_PARAMETER_5						8005	/**<  */
#define ECRIO_IMDN_INSUFFICIENT_PARAMETER					8006	/**<  */
#define ECRIO_IMDN_MEMORY_ALLOCATION_ERROR					8010	/**<  */
#define ECRIO_IMDN_PARSING_INSUFFICIENT_ERROR				8020	/**<  */
#define ECRIO_IMDN_MESSAGE_ID_TOO_LONG						8021	/**<  */
#define ECRIO_IMDN_MUTEX_CREATE_FAILED						8030	/**<  */


#define IMDN_XML_LENGTH										1024
#define IMDN_STRING_LENGTH									128
#define IMDN_MAX_MESSAGE_ID_LENGTH							64

#define IMDN_XML_VERSION									(u_char*)"1.0"
#define IMDN_XML_ENCODING									(u_char*)"UTF-8"
#define IMDN_XML_OPENING_ANGLE_BRACE						(u_char*)"<"
#define IMDN_XML_CLOSING_ANGLE_BRACE						(u_char*)">"
#define IMDN_XML_QUESTIONMARK								(u_char*)"?"
#define IMDN_XML_VERSION1									(u_char*)"xml version"
#define IMDN_XML_ENCODING1									(u_char*)"encoding"
#define IMDN_XML_EQUALTO									(u_char*)"="
#define IMDN_XML_SPACE										(u_char*)" "
#define IMDN_XML_NEWLINE									(u_char*)"\n"
#define IMDN_XML_DOUBLEQUOTES								(u_char*)"\""
#define IMDN_XML_SLASH										(u_char*)"/"

#define IMDN_COMPOSING_STRING								(u_char*)"imdn"
#define IMDN_COMPOSING_ATTRIBUTE_DEFAULTNS					(u_char*)"xmlns"
#define IMDN_COMPOSING_VALUE_DEFAULTNS						(u_char*)"urn:ietf:params:xml:ns:imdn"
#define IMDN_MESSAGE_ID_STRING								(u_char*)"message-id"
#define IMDN_DATETIME_STRING								(u_char*)"datetime"
#define IMDN_DELIVERY_NOTIFICATION_STRING					(u_char*)"delivery-notification"
#define IMDN_DISPLAY_NOTIFICATION_STRING					(u_char*)"display-notification"
#define IMDN_STATUS_STRING									(u_char*)"status"
#define IMDN_DELIVERED_STRING								(u_char*)"delivered"
#define IMDN_DISPLAYED_STRING								(u_char*)"displayed"
#define IMDN_FAILED_STRING									(u_char*)"failed"
#define IMDN_FORBIDDEN_STRING								(u_char*)"forbidden"
#define IMDN_ERROR_STRING									(u_char*)"error"
#define IMDN_RECIPIENTURI_STRING							(u_char*)"recipient-uri"
#define IMDN_ORIGINALRECIPIENTURI_STRING					(u_char*)"original-recipient-uri"

/* @note Please note that macro definitions must be defined on single lines so
   that line references remain intact during debugging and log output. */

/* Macros for logging. */

#ifdef ENABLE_LOG
/* The build project should define ENABLE_LOG to enable non-error logging
   using the Ecrio PAL logging mechanism. */
#define IMDNLOGV(a, b, c, ...)			pal_LogMessage((a), KLogLevelVerbose, KLogComponentRCS, (b), (c),##__VA_ARGS__);
#define IMDNLOGI(a, b, c, ...)			pal_LogMessage((a), KLogLevelInfo, KLogComponentRCS, (b), (c),##__VA_ARGS__);
#define IMDNLOGD(a, b, c, ...)			pal_LogMessage((a), KLogLevelDebug, KLogComponentRCS, (b), (c),##__VA_ARGS__);
#define IMDNLOGW(a, b, c, ...)			pal_LogMessage((a), KLogLevelWarning, KLogComponentRCS, (b), (c),##__VA_ARGS__);
#define IMDNLOGE(a, b, c, ...)			pal_LogMessage((a), KLogLevelError, KLogComponentRCS, (b), (c),##__VA_ARGS__);

#define IMDNLOGDUMPI(a, b, c, d)		pal_LogMessageDump((a), KLogLevelInfo, KLogComponentRCS, (b), (c), (d));
#else
#define IMDNLOGV(a, b, c, ...)			if (a != NULL) {;}
#define IMDNLOGI(a, b, c, ...)			if (a != NULL) {;}
#define IMDNLOGD(a, b, c, ...)			if (a != NULL) {;}
#define IMDNLOGW(a, b, c, ...)			if (a != NULL) {;}
#define IMDNLOGE(a, b, c, ...)			if (a != NULL) {;}

#define IMDNLOGDUMPI(a, b, c, d)
#endif

/* Macros for mutex management. */

#define IMDN_MUTEX_CREATE(mtx, err, pfderr, label, log)	(pfderr) = pal_MutexCreate(s->pal, &(mtx)); \
	if ((pfderr) != KPALErrorNone) \
	{ \
		IMDNLOGE((log), KLogTypeGeneral, "%s:%u\tpal_MutexCreate() failed %u", __FUNCTION__, __LINE__, (pfderr)); \
		(err) = ECRIO_IMDN_MUTEX_CREATE_FAILED; \
		goto label; \
	}

#define IMDN_MUTEX_LOCK(mtx, log)	if (pal_MutexLock((mtx)) != KPALErrorNone) \
	{ \
		IMDNLOGE((log), KLogTypeMutex, "%s:%u\tpal_MutexLock() failed for %p", __FUNCTION__, __LINE__, (mtx)); \
	}

#define IMDN_MUTEX_UNLOCK(mtx, log)	if (pal_MutexUnlock((mtx)) != KPALErrorNone) \
	{ \
		IMDNLOGE((log), KLogTypeMutex, "%s:%u\tpal_MutexUnlock() failed for %p", __FUNCTION__, __LINE__, (mtx)); \
	}

#define IMDN_MUTEX_DELETE(mtx, log)	if (pal_MutexDelete(&(mtx)) != KPALErrorNone) \
	{ \
		IMDNLOGE((log), KLogTypeGeneral, "%s:%u\tpal_MutexDelete() failed for %p", __FUNCTION__, __LINE__, (mtx)); \
		(mtx) = NULL; \
	} \
	else \
	{ \
		(mtx) = NULL; \
	}

/**
 *
 */
typedef struct
{
	u_char *pData;									/** Pointer to an allocated buffer. */
	u_int32	uContainerSize;							/** Size of the allocated memory. */
	u_int32	uSize;									/** Current size of the data in the buffer. */
} EcrioIMDNBufferStruct;

/**
 *
 */
typedef struct
{
	PALINSTANCE pal;
	LOGHANDLE logHandle;
	MUTEXHANDLE mutexAPI;

	EcrioIMDNBufferStruct work;
	EcrioIMDNBufferStruct strings;
} EcrioIMDNStruct;


u_int32 ec_imdn_MaintenanceBuffer
(
	EcrioIMDNBufferStruct *pBuff,
	u_int32 uTotal
)
;

u_int32 ec_imdn_StringCopy
(
	EcrioIMDNBufferStruct *pStrings,
	void **ppDest,
	u_int32 uDestlen,
	const void *pSource,
	u_int32 strLength
);

void ec_imdn_StringConcatenate
(
	u_char *pStart,
	u_int32 uStartlen,
	u_char *pSubString,
	u_char **ppEnd
);

u_int32 ec_imdn_ParseIMDNXmlBody
(
	EcrioIMDNStruct *s,
	IMDNDispoNotifStruct *pStruct,
	u_char *pData,
	u_int32 uLen,
	u_int32 uIndex
);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __ECRIOIMDNCOMMON_H__ */
