/******************************************************************************

Copyright (c) 2005-2020 Ecrio, Inc. All Rights Reserved.

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
 * @file EcrioDataTypes.h
 * @brief This header file defines the C data type policy used by all Ecrio
 * Components.
 */

#ifndef __ECRIODATATYPES_H__
#define __ECRIODATATYPES_H__

#include <limits.h>
/** The DISABLE_IMS_IPSEC_SUPPORT compilation switch must be defined
* if the MoIP and Media functionalities needs to be disabled.
*/

#ifndef DISABLE_IMS_IPSEC_SUPPORT
#define DISABLE_IMS_IPSEC_SUPPORT
#endif

#ifndef NULL
#define NULL    0x0							/**< NULL pointer definition. */
#endif

#ifndef U_CHAR
#define U_CHAR
typedef unsigned char u_char;				/**< unsigned char type ("char" is different than both "signed char" and "unsigned char" and can be used directly.) */
#endif

#ifndef S_CHAR
#define S_CHAR
typedef signed char s_char;					/**< signed char type ("char" is different than both "signed char" and "unsigned char" and can be used directly.) */
#endif

#ifndef U_INT64
#define U_INT64
typedef unsigned long long int u_int64;		/**< 64-bit unsigned integer type */
#endif

#ifndef U_INT32
#define U_INT32
typedef unsigned int u_int32;				/**< 32-but unsigned integer type */
#endif

#ifndef U_INT16
#define U_INT16
typedef unsigned short u_int16;				/**< 16-bit unsigned integer type */
#endif

#ifndef U_INT8
#define U_INT8
typedef unsigned char u_int8;				/**< 8-bit unsigned integer type */
#endif

#ifndef S_INT64
#define S_INT64
typedef signed long long int s_int64;		/**< 64-bit signed integer type */
#endif

#ifndef S_INT32
#define S_INT32
typedef signed int s_int32;					/**< 32-bit signed integer type */
#endif

#ifndef S_INT16
#define S_INT16
typedef signed short s_int16;				/**< 16-bit signed integer type */
#endif

#ifndef S_INT8
#define S_INT8
typedef signed char s_int8;					/**< 8-bit signed integer type */
#endif

#ifndef FLOAT32
#define FLOAT32
typedef float float32;						/**< 32-bit floating point type */
#endif

#ifndef FLOAT64
#define FLOAT64
typedef double float64;						/**< 64-bit floating point type */
#endif

#ifndef BOOLENUM
#define BOOLENUM

/** @enum BoolEnum
 * This enumeration defines a standard set of Boolean values that all
 * Ecrio components should use.
 */
typedef enum
{
	Enum_INVALID	= -1,					/**< Invalid Boolean designation, implies uninitialized or unused state.  */
	Enum_FALSE		= 0,					/**< False Boolean designation. */
	Enum_TRUE		= 1						/**< True Boolean designation. */
} BoolEnum;
#endif

#endif /* #ifndef __ECRIODATATYPES_H__ */
