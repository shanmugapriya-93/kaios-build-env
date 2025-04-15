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
 * @file EcrioCPIMCommon.h
 * @brief This file defines the primary internal definitions for the Ecrio
 * CPIM implementation.
 */

#ifndef __ECRIOCPIMCOMMON_H__
#define __ECRIOCPIMCOMMON_H__

/* Allow this module to be called directly from C++ compiled code. */
#ifdef __cplusplus
extern "C" {
#endif

/* Implementation level errors. */

#define ECRIO_CPIM_INVALID_HANDLE							7000	/**<  */
#define ECRIO_CPIM_INVALID_PARAMETER_1						7001	/**<  */
#define ECRIO_CPIM_INVALID_PARAMETER_2						7002	/**<  */
#define ECRIO_CPIM_INVALID_PARAMETER_3						7003	/**<  */
#define ECRIO_CPIM_INVALID_PARAMETER_4						7004	/**<  */
#define ECRIO_CPIM_INVALID_PARAMETER_5						7005	/**<  */
#define ECRIO_CPIM_INSUFFICIENT_PARAMETER					7006	/**<  */
#define ECRIO_CPIM_MEMORY_ALLOCATION_ERROR					7010	/**<  */
#define ECRIO_CPIM_INSUFFICIENT_MEMORY_ERROR				7011	/**<  */
#define ECRIO_CPIM_PARSING_INSUFFICIENT_ERROR				7020	/**<  */
#define ECRIO_CPIM_MUTEX_CREATE_FAILED						7030	/**<  */


#define CPIM_HEADER_LENGTH									512
#define CPIM_CONTENT_HEADER_LENGTH							256
#define CPIM_MAX_MESSAGE_LENGTH								1536
#define CPIM_STRING_BUFFER_LENGTH							1024
#define CPIM_WORKING_BUFFER_LENGTH							CPIM_HEADER_LENGTH + CPIM_MAX_MESSAGE_LENGTH
#define CPIM_MESSAGE_BUFFER_LENGTH							CPIM_CONTENT_HEADER_LENGTH + CPIM_MAX_MESSAGE_LENGTH
#define CPIM_BUFFER_LENGTH									1024
#define CPIM_BOUNDARY_LENGTH								32

#define CPIM_CRLF_SYMBOL									(u_char*)"\r\n"
#define CPIM_SPACE_SYMBOL									(u_char*)" "
#define CPIM_SLASH_SYMBOL									(u_char*)"/"
#define CPIM_COLON_SYMBOL									(u_char*)":"
#define CPIM_SEMICOLON_SYMBOL								(u_char*)";"
#define CPIM_COMMA_SYMBOL									(u_char*)","
#define CPIM_DOT_SYMBOL										(u_char*)"."
#define CPIM_DASH_SYMBOL									(u_char*)"-"
#define CPIM_EQUAL_SYMBOL									(u_char*)"="
#define CPIM_DOUBLEQUOTES_SYMBOL							(u_char*)"\""
#define CPIM_ASTERISK_SYMBOL								(u_char*)"*"
#define CPIM_OPENING_ANGLE_BRACE							(u_char*)"<"
#define CPIM_CLOSING_ANGLE_BRACE							(u_char*)">"

#define CPIM_BOUNDARY_STRING								(u_char*)"boundary"
#define CPIM_TO_STRING										(u_char*)"To"
#define CPIM_FROM_STRING									(u_char*)"From"
#define CPIM_DATETIME_STRING								(u_char*)"DateTime"
#define CPIM_NAMESPACE_STRING								(u_char*)"NS"
#define CPIM_NAMESPACE_IMDN_STRING							(u_char*)"imdn"
#define CPIM_NAMESPACE_MAAP_STRING							(u_char*)"maap"
#define CPIM_MESSAGE_ID_STRING								(u_char*)"Message-ID"
#define CPIM_DISPOSITION_NOTIFICATION_STRING				(u_char*)"Disposition-Notification"
#define CPIM_NEGATIVE_DELIVERY_STRING						(u_char*)"negative-delivery"
#define CPIM_POSITIVE_DELIVERY_STRING						(u_char*)"positive-delivery"
#define CPIM_DISPLAY_STRING									(u_char*)"display"
#define CPIM_TRAFFIC_TYPE_STRING							(u_char*)"Traffic-Type"
#define CPIM_ADVERTISEMENT_STRING							(u_char*)"advertisement"
#define CPIM_PAYMENT_STRING									(u_char*)"payment"
#define CPIM_PREMIUM_STRING									(u_char*)"premium"
#define CPIM_SUBSCRIPTION_STRING							(u_char*)"subscription"
#define CPIM_PLUGIN_STRING									(u_char*)"plugin"
#define CPIM_CONTENT_TYPE_STRING							(u_char*)"Content-Type"
#define CPIM_CONTENT_LENGTH_STRING							(u_char*)"Content-Length"
#define CPIM_CONTENT_DISPOSITION_STRING						(u_char*)"Content-Disposition"
#define CPIM_CONTENT_ID_STRING								(u_char*)"Content-ID"
#define CPIM_CONTENT_TRANSFER_ENCODING_STRING				(u_char*)"Content-Transfer-Encoding"
#define CPIM_CONTENT_DISPOSITION_NOTIFICATION_STRING		(u_char*)"notification"
#define CPIM_CONTENT_TYPE_MULTIPART_MIXED_STRING			(u_char*)"multipart/related"
#define CPIM_CONTENT_TYPE_TEXT_STRING						(u_char*)"text/plain;charset=UTF-8"
#define CPIM_CONTENT_TYPE_IMDN_STRING						(u_char*)"message/imdn+xml"
#define CPIM_CONTENT_TYPE_FILE_TRANSFER_HTTP_STRING			(u_char*)"application/vnd.gsma.rcs-ft-http+xml"
#define CPIM_CONTENT_TYPE_PUSH_LOCATION_STRING				(u_char*)"application/vnd.gsma.rcspushlocation+xml"
#define CPIM_CONTENT_TYPE_MESSAGE_REVOKE_STRING				(u_char*)"application/vnd.gsma.rcsrevoke+xml"
#define CPIM_CONTENT_TYPE_PRIVACY_MANAGEMENT_STRING			(u_char*)"application/vnd.gsma.rcsalias-mgmt+xml"
#define CPIM_CONTENT_TYPE_LINK_REPORT_STRING				(u_char*)"application/vnd.gsma.rcslink-report+xml"
#define CPIM_CONTENT_TYPE_SPAM_REPORT_STRING				(u_char*)"application/vnd.gsma.rcsspam-report+xml"
#define CPIM_CONTENT_TYPE_RICHCARD_STRING					(u_char*)"application/vnd.gsma.botmessage.v1.0+json"
#define CPIM_CONTENT_TYPE_SUGGESTED_CHIPLIST_STRING			(u_char*)"application/vnd.gsma.botsuggestion.v1.0+json"
#define CPIM_CONTENT_TYPE_SUGGESTED_RESPONSE_STRING			(u_char*)"application/vnd.gsma.botsuggestion.response.v1.0+json"
#define CPIM_CONTENT_TYPE_SHARED_CLIENT_DATA_STRING			(u_char*)"application/vnd.gsma.botsharedclientdata.v1.0+json"
#define CPIM_CONTENT_TYPE_CPM_GROUPDATA_STRING				(u_char*)"application/vnd.oma.cpm-groupdata+xml"

#define CPIM_ANONYMOUS_URI_STRING							(u_char*)"sip:anonymous@anonymous.invalid"
#define CPIM_IMDN_URN_STRING								(u_char*)"urn:ietf:params:imdn"
#define CPIM_MAAP_URN_STRING								(u_char*)"urn:<http://www.gsma.com/rcs/maap/>"

#define CPIM_NAMESPACE_MYFEATURE_STRING						(u_char*)"MyFeatures"
#define CPIM_MYFEATURE_MAILTO_STRING							(u_char*)"mailto:RCSFeatures@att.com"
#define CPIM_PANI_ID_STRING									(u_char*)"PANI"

/* @note Please note that macro definitions must be defined on single lines so
   that line references remain intact during debugging and log output. */

/* Macros for logging. */

#ifdef ENABLE_LOG
/* The build project should define ENABLE_LOG to enable non-error logging
   using the Ecrio PAL logging mechanism. */
#define CPIMLOGV(a, b, c, ...)			pal_LogMessage((a), KLogLevelVerbose, KLogComponentRCS, (b), (c),##__VA_ARGS__);
#define CPIMLOGI(a, b, c, ...)			pal_LogMessage((a), KLogLevelInfo, KLogComponentRCS, (b), (c),##__VA_ARGS__);
#define CPIMLOGD(a, b, c, ...)			pal_LogMessage((a), KLogLevelDebug, KLogComponentRCS, (b), (c),##__VA_ARGS__);
#define CPIMLOGW(a, b, c, ...)			pal_LogMessage((a), KLogLevelWarning, KLogComponentRCS, (b), (c),##__VA_ARGS__);
#define CPIMLOGE(a, b, c, ...)			pal_LogMessage((a), KLogLevelError, KLogComponentRCS, (b), (c),##__VA_ARGS__);

#define CPIMLOGDUMPI(a, b, c, d)		pal_LogMessageDump((a), KLogLevelInfo, KLogComponentRCS, (b), (c), (d));
#else
#define CPIMLOGV(a, b, c, ...)			if (a != NULL) {;}
#define CPIMLOGI(a, b, c, ...)			if (a != NULL) {;}
#define CPIMLOGD(a, b, c, ...)			if (a != NULL) {;}
#define CPIMLOGW(a, b, c, ...)			if (a != NULL) {;}
#define CPIMLOGE(a, b, c, ...)			if (a != NULL) {;}

#define CPIMLOGDUMPI(a, b, c, d)
#endif

/* Macros for mutex management. */

#define CPIM_MUTEX_CREATE(mtx, err, pfderr, label, log)	(pfderr) = pal_MutexCreate(c->pal, &(mtx)); \
	if ((pfderr) != KPALErrorNone) \
	{ \
		CPIMLOGE((log), KLogTypeGeneral, "%s:%u\tpal_MutexCreate() failed %u", __FUNCTION__, __LINE__, (pfderr)); \
		(err) = ECRIO_CPIM_MUTEX_CREATE_FAILED; \
		goto label; \
	}

#define CPIM_MUTEX_LOCK(mtx, log)	if (pal_MutexLock((mtx)) != KPALErrorNone) \
	{ \
		CPIMLOGE((log), KLogTypeMutex, "%s:%u\tpal_MutexLock() failed for %p", __FUNCTION__, __LINE__, (mtx)); \
	}

#define CPIM_MUTEX_UNLOCK(mtx, log)	if (pal_MutexUnlock((mtx)) != KPALErrorNone) \
	{ \
		CPIMLOGE((log), KLogTypeMutex, "%s:%u\tpal_MutexUnlock() failed for %p", __FUNCTION__, __LINE__, (mtx)); \
	}

#define CPIM_MUTEX_DELETE(mtx, log)	if (pal_MutexDelete(&(mtx)) != KPALErrorNone) \
	{ \
		CPIMLOGE((log), KLogTypeGeneral, "%s:%u\tpal_MutexDelete() failed for %p", __FUNCTION__, __LINE__, (mtx)); \
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
	u_int32	uOpaque;								/** The opaque data which desire to store in the buffer structure. */
} EcrioCPIMBufferStruct;

/**
 *
 */
typedef struct
{
	PALINSTANCE pal;
	LOGHANDLE logHandle;
	MUTEXHANDLE mutexAPI;

	EcrioCPIMBufferStruct strings;
	EcrioCPIMBufferStruct msgBuff;
	EcrioCPIMBufferStruct cmnBuff;
	EcrioCPIMBufferStruct work;

	u_int32 uNumOfBuffers;
	EcrioCPIMBufferStruct buff[CPIM_MSG_NUM];		/**< Structure of message bodies. */
} EcrioCPIMStruct;


u_int32 ec_cpim_MaintenanceBuffer
(
	EcrioCPIMBufferStruct *pBuff,
	u_int32 uTotal
);

u_int32 ec_cpim_StringCopy
(
	EcrioCPIMBufferStruct *pStrings,
	void **ppDest,
	const void *pSource,
	u_int32 strLength
);

void ec_cpim_StringConcatenate
(
	u_char *pStart,
	u_int32 uStartlen,
	u_char *pSubString,
	u_char **ppEnd
)
;

u_int32 ec_cpim_ParseMIMEMessage
(
	EcrioCPIMStruct *c,
	CPIMMessageStruct *pStruct,
	u_char *pData,
	u_int32 uLen,
	u_int32 uIndex
);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __ECRIOCPIMCOMMON_H__ */
