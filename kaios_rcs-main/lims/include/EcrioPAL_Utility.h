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
 * @file EcrioPAL_Utility.h
 * @brief This is interface of the Ecrio PAL's Utility Module.
 *
 * The purpose of the Utility Module is to provide wrappers around common
 * functions routines that don't fit in to other specific categories.
 *
 */

#ifndef __ECRIOPAL_UTILITY_H__
#define __ECRIOPAL_UTILITY_H__

/* Include common PAL definitions for building. */
#include "EcrioPAL_Common.h"

/* Allow this module to be called directly from C++ compiled code. */
#ifdef __cplusplus
extern "C" {
#endif

typedef void *FQDNHANDLE;

/** @enum ECRIO_UTILITY_DNS_Protocol_Version
 * Enumeration holding the information of protocol version.
 */
typedef enum
{
	ECRIO_UTILITY_DNS_Protocol_Version_IPv4			= 0,	/**< IPv4 Protocol Version. */
	ECRIO_UTILITY_DNS_Protocol_Version_IPv6			= 1,	/**< IPv6 Protocol Version. */
} ECRIO_UTILITY_DNS_Protocol_Version;

typedef enum
{
	ECRIO_UTILITY_DNS_Protocol_Type_Udp = 0,
	ECRIO_UTILITY_DNS_Protocol_Type_Tcp
} ECRIO_UTILITY_DNS_Protocol_Type;

/** @struct EcrioUtilityDnsAddrStruct
 * This structure is used to convey IP string and port, version in pal_UtilityStartQueryFQDN
 */
typedef struct
{
	ECRIO_UTILITY_DNS_Protocol_Version version;
	ECRIO_UTILITY_DNS_Protocol_Type type;
	u_int32 uPort;
	u_char *pIpAddr;
} EcrioUtilityDnsAddrStruct;

typedef void (*EcrioUtilityDnsAnswerCallback)
(
	void *pCallbackFnData,
	u_int32 uTlsAddrCount,
	EcrioUtilityDnsAddrStruct *pTlsDnsAddrList,
	u_int32 uNonTlsAddrCount,
	EcrioUtilityDnsAddrStruct *pNonTlsDnsAddrList
);

typedef struct
{
	EcrioUtilityDnsAnswerCallback pCallback;
	void *pCallbackFnData;
} EcrioUtilityDnsAnswerCallbackStruct;

/** @struct EcrioDateAndTimeStruct
 * This structure is used to convey Date and Time. used in pal_UtilityGetDateAndTime
 */
typedef struct
{
	u_int16 year;                       /**< The year including all digits. */
	u_int16 month;                      /**< The month, 1 through 12. */
	u_int16 day;                        /**< The day, 1 through 31. */
	u_int16 hour;                       /**< The hour, 0 through 23. */
	u_int16 minute;                     /**< The minute, 0 through 59. */
	u_int16 second;                     /**< The second, 0 through 59. */
} EcrioDateAndTimeStruct;

/**
 * This function generates a 128-bit unsigned char pseudo-random binary.
 *
 * @return The random binary value.
 */
u_char *pal_UtilityRandomBin16
(
	void
);

/**
 * This function generates a 32-bit unsigned integer pseudo-random number.
 *
 * @return The random number value.
 */
u_int32 pal_UtilityRandomNumber
(
	void
);

u_int32 pal_UtilityGetMillisecondCount
(
	void
);

/**
* This function provides the current date and time in year, month, day and 
* hour, minute and seconds. 
*
*/
u_int32 pal_UtilityGetDateAndTime
(
	EcrioDateAndTimeStruct* pDateAndTime
);

void pal_UtilityUsleep
(
	u_int32 usec
);

/**
* This function will be used to lookup NAPTR and SRV, A, AAAA records from a 
* DNS server. This function provides the IP string and port, version from a DNS 
* server.
*
* This function is to start to query the FQDN address to DNS server. It 
* is asynchronous API.
*
*/
u_int32 pal_UtilityStartQueryFQDN
(
	PALINSTANCE pal,
	char *pHostName,
	char *pLocalIP,
	char *pServerIP,
	EcrioUtilityDnsAnswerCallbackStruct *pFnStruct,
	FQDNHANDLE *handle
);

/**
* This function is to stop to query.
*
*/
u_int32 pal_UtilityStopQueryFQDN
(
	FQDNHANDLE *handle
);

/**
* This function will be used to lookup NAPTR and SRV, A, AAAA records from a 
* DNS server. This function provides the IP string and port, version from a DNS 
* server.
*
* This function must block until the DNS lookup is complete.
*
*/
u_int32 pal_UtilityDnsGetInfoFromFqdn
(
	PALINSTANCE pal,
	const char *pHostName,
	const char *pLocal,
	const char *pDnsAddress,
	u_int32* pTlsAddrCount,
	EcrioUtilityDnsAddrStruct **ppTlsDnsAddrList,
	u_int32* pNonTlsAddrCount,
	EcrioUtilityDnsAddrStruct **ppNonTlsDnsAddrList
);

BoolEnum pal_UtilityDataOverflowDetected
(
	u_int32 index,
	u_int32 dataSize
);

BoolEnum pal_UtilityArithmeticOverflowDetected
(
	u_int32 uSrc,
	u_int32 uValue
);

BoolEnum pal_UtilityArithmeticUnderflowDetected
(
	u_int32 uSrc,
	u_int32 uValue
);
#ifdef __cplusplus
}
#endif

#endif /* #ifndef __ECRIOPAL_UTILITY_H__ */
