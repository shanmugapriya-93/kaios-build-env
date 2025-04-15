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
 * @file EcrioPAL_String.c
 * @brief This is the implementation of the Ecrio PAL's String Module.
 *
 * This implementation of the String Module uses the C runtime library
 * functions for memory management. There are no Win32 API calls used.
 *
 * <b>Design Overview</b>
 *
 * There is no overall design of the String Module since each function stands
 * on its own. Therefore, each function will have its own documentation in
 * regards to how it is designed and how it operates.
 *
 * However, in general, each function will first check the validity of
 * all parameters, and then call the relevant function provided by the
 * platform.
 *
 * <b>Synchronization</b>
 *
 * The interface and implementation dictate that the String Module is a
 * synchronous set of APIs that can be fully re-entrant. In other words,
 * multiple threads can call any of the APIs in any order and simultaneously.
 * It is important that the underlying C runtime library also have multi-
 * threaded support. For Win32 and Windows Mobile builds, it is therefore
 * extremely important to always link with the "Multi-Threaded" version of
 * the libraries. There is no synchronization performed by the String Module
 * especially since the module does not maintain any state and because there
 * are no callback functions to call or process.
 *
 * <b>Logging</b>
 *
 * There is no logging performed within the String Module functions.
 */

/* Standard Windows includes. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

/* Ignore deprecated function warnings. */
#pragma warning(disable: 4996)

/*lint -e970 In the Ecrio Win32/WM PAL implementation, no typedef policy is needed. */

#include "EcrioPAL.h"

/**
 * Implementation of the pal_StringCreate() function. See the EcrioPAL_String.h
 * file for interface definitions.
 */
u_char *pal_StringCreate
(
	const u_char *pSourceString,
	s_int32 stringLength
)
{
	unsigned int returnValue = 0;
	char *buffer = NULL;

	/** This function checks all parameters passed to it. */
	/** The string length must be greater than zero (0).
	    The following has been deprecated: Sometimes a string will be
	    provided of zero length, but is truly empty (with a null terminator),
	    hence algorithms would still expect a new string to be created, even
	    if empty. This is thus allowed, but should be avoided since it is
	    inefficient. */
	if (stringLength <= 0)
	{
		return NULL;
	}

	/** Allocate memory for the string buffer, adding a byte for null termination. */
	returnValue = pal_MemoryAllocate(stringLength + 1, (void **)&buffer);
	if ((buffer == NULL) || (returnValue != KPALErrorNone))
	{
		/* Memory for the string is not available. */
		return NULL;
	}

	if (pSourceString != NULL)
	{
		if (stringLength > 0)
		{
			/** Use strncpy() to copy the string to the newly allocated buffer. */
			strncpy(buffer, (char *)pSourceString, stringLength);
			/** Note: If the length is less than the actual size of the source string,
			    the resulting string will be truncated and a null terminator is
			    already present to terminate the string. If the length is greater than
			    the source string, the strncpy() function will pad the buffer with
			    null characters up to the length, resulting in a properly null
			    terminated string. */
		}

		/* Place a null terminator at the very end of the string. */
		buffer[stringLength] = '\0';
	}
	else
	{
		/** Initialize the newly allocated memory with 0/null bytes if no
		    source string is provided. Do not assume that pal_MemoryAllocate()
		    sets the memory. */
		returnValue = pal_MemorySet(buffer, 0, stringLength + 1);
		/* Ignore return value. */
	}

	/** Return the pointer to the newly allocated buffer. */
	return (unsigned char *)buffer;
}

/**
 * Implementation of the pal_StringLength() function. See the EcrioPAL_String.h
 * file for interface definitions.
 */
s_int32 pal_StringLength
(
	const u_char *pString
)
{
	/** This function checks all parameters passed to it. */
	/* The string must not be NULL. */
	if (pString == NULL)
	{
		return 0;
	}

	/** Get the string length using the strlen() function. */
	return (signed int)strlen((char *)pString);
}

/**
 * Implementation of the pal_StringNCopy() function. See the EcrioPAL_String.h
 * file for interface definitions.
 */
u_char *pal_StringNCopy
(
	u_char *pDestString,
	u_int32 destLength,
	const u_char *pSourceString,
	s_int32 count
)
{
	size_t sourceSzie = 0;

	/** This function checks all parameters passed to it. */
	/* The destination string must not be NULL. */
	if (pDestString == NULL)
	{
		return NULL;
	}

	/* The source string must not be NULL. */
	if (pSourceString == NULL)
	{
		return NULL;
	}

	/* The number of characters to copy must not be less than 0. */
	if (count < 0)
	{
		return NULL;
	}

	sourceSzie = strlen((const char *)pSourceString);
	/* The length of source must not be less than count. */
	if (sourceSzie < count)
	{
		return NULL;
	}

	/* The length of destination must not be less than count + 1. */
	if (destLength < count + 1)
	{
		return NULL;
	}

	/** Perform a string copy using the strncpy() function. Note that if count
	    is less than or equal to the length of pSourceString. */
	strncpy((char *)pDestString, (char *)pSourceString, count);
	pDestString[count] = '\0';

	return pDestString;
}

/**
 * Implementation of the pal_StringNConcatenate() function. See the
 * EcrioPAL_String.h file for interface definitions.
 */
u_char *pal_StringNConcatenate
(
	u_char *pDestString,
	u_int32 destLength,
	const u_char *pSourceString,
	s_int32 count
)
{
	size_t sourceSzie = 0;

	/** This function checks all parameters passed to it. */
	/* The destination string must not be NULL. */
	if (pDestString == NULL)
	{
		return NULL;
	}

	/* The source string must not be NULL. */
	if (pSourceString == NULL)
	{
		return NULL;
	}

	/* The number of characters to copy must not be less than 0. */
	if (count < 0)
	{
		return NULL;
	}

	sourceSzie = strlen((const char *)pSourceString);
	/* The length of source must not be less than count. */
	if (sourceSzie < count)
	{
		return NULL;
	}

	/* The length of destination must not be less than count + 1. */
	if (destLength < count + 1)
	{
		return NULL;
	}

	/** Perform a string concatenation using the strncat() function. */
	strncat((char *)pDestString, (char *)pSourceString, count);
//	pDestString[count] = '\0';

	return pDestString;
}

/**
 * Implementation of the pal_StringCompare() function. See the EcrioPAL_String.h
 * file for interface definitions.
 */
s_int32 pal_StringCompare
(
	const u_char *str1,
	const u_char *str2
)
{
	/** If both string pointers are NULL, then we will treat the "strings" as
	    equal. */
	if ((str1 == NULL) && (str2 == NULL))
	{
		return 0;
	}

	/** If the first string is NULL but the second string is not NULL, then
	    we will treat the first string as less than the second string. */
	if ((str1 == NULL) && (str2 != NULL))
	{
		return -1;
	}

	/** If the second string is NULL but the first string is not NULL, then
	    we will treat the second string as less than the first string. */
	if ((str1 != NULL) && (str2 == NULL))
	{
		return 1;
	}

	/** Perform a string comparison using the strcmp() function. */
	return strcmp((char *)str1, (char *)str2);
}

/**
 * Implementation of the pal_StringICompare() function. See the
 * EcrioPAL_String.h file for interface definitions.
 */
s_int32 pal_StringICompare
(
	const u_char *str1,
	const u_char *str2
)
{
	/** If both string pointers are NULL, then we will treat the "strings" as
	    equal. */
	if ((str1 == NULL) && (str2 == NULL))
	{
		return 0;
	}

	/** If the first string is NULL but the second string is not NULL, then
	    we will treat the first string as less than the second string. */
	if ((str1 == NULL) && (str2 != NULL))
	{
		return -1;
	}

	/** If the second string is NULL but the first string is not NULL, then
	    we will treat the second string as less than the first string. */
	if ((str1 != NULL) && (str2 == NULL))
	{
		return 1;
	}

	/** Perform a string comparison using the _stricmp() function. */
	return _stricmp((char *)str1, (char *)str2);
}

/**
 * Implementation of the pal_StringNCompare() function. See the
 * EcrioPAL_String.h file for interface definitions.
 */
s_int32 pal_StringNCompare
(
	const u_char *str1,
	const u_char *str2,
	s_int32 count
)
{
	/** If both string pointers are NULL, then we will treat the "strings" as
	    equal. */
	if ((str1 == NULL) && (str2 == NULL))
	{
		return 0;
	}

	/** If the first string is NULL but the second string is not NULL, then
	    we will treat the first string as less than the second string. */
	if ((str1 == NULL) && (str2 != NULL))
	{
		return -1;
	}

	/** If the second string is NULL but the first string is not NULL, then
	    we will treat the second string as less than the first string. */
	if ((str1 != NULL) && (str2 == NULL))
	{
		return 1;
	}

	/** The number of characters to compare must not be less than 0. */
	if (count < 0)
	{
		/** If the number of characters to compare is less than 0, return
		    negative 1 so that there is no mistake that the strings are
		    not matched. */
		return -1;
	}

	/** Perform a string comparison using the strncmp() function. */
	return strncmp((char *)str1, (char *)str2, count);
}

/**
 * Implementation of the pal_StringNICompare() function. See the
 * EcrioPAL_String.h file for interface definitions.
 */
s_int32 pal_StringNICompare
(
	const u_char *str1,
	const u_char *str2,
	s_int32 count
)
{
	/** If both string pointers are NULL, then we will treat the "strings" as
	    equal. */
	if ((str1 == NULL) && (str2 == NULL))
	{
		return 0;
	}

	/** If the first string is NULL but the second string is not NULL, then
	    we will treat the first string as less than the second string. */
	if ((str1 == NULL) && (str2 != NULL))
	{
		return -1;
	}

	/** If the second string is NULL but the first string is not NULL, then
	    we will treat the second string as less than the first string. */
	if ((str1 != NULL) && (str2 == NULL))
	{
		return 1;
	}

	/** The number of characters to compare must not be less than 0. */
	if (count < 0)
	{
		/** If the number of characters to compare is less than 0, return
		    negative 1 so that there is no mistake that the strings are
		    not matched. */
		return -1;
	}

	/** Perform a string comparison using the _strnicmp() function. */
	return _strnicmp((char *)str1, (char *)str2, count);
}

/**
 * Implementation of the pal_StringFindSubString() function. See the
 * EcrioPAL_String.h file for interface definitions.
 */
u_char *pal_StringFindSubString
(
	const u_char *pString,
	const u_char *pSubString
)
{
	/** This function checks all parameters passed to it. */
	/* The string must not be NULL. */
	if (pString == NULL)
	{
		return NULL;
	}

	/* The sub string to search for must not be NULL. */
	if (pSubString == NULL)
	{
		return NULL;
	}

	/** Use the strstr() function to find the first occurrence of the matching
	    sub string in the string. */
	return (unsigned char *)strstr((char *)pString, (char *)pSubString);
}

/**
 * Implementation of the pal_StringConvertToUNum() function. See the
 * EcrioPAL_String.h file for interface definitions.
 */
u_int32 pal_StringConvertToUNum
(
	const u_char *pString,
	u_char **pEnd,
	u_int32 base
)
{
	/** This function checks all parameters passed to it. */
	/* The pString must not be NULL. */
	if (pString == NULL)
	{
		return 0;
	}

	/* The base must be between 2 and 16. */
	if ((base < 2) ||
		(base > 16))
	{
		return KPALInvalidParameters;
	}

	/** Use the strtoul() function to convert the string to a numeric value. */
	return (u_int32)strtoul((char *)pString, (char **)pEnd, base);
}

u_char *pal_StringFindChar
(
	const u_char *pString,
	const u_char ch
)
{
	/** This function checks all parameters passed to it. */
	/* The pString must not be NULL. */
	if (pString == NULL)
	{
		return NULL;
	}

	return (u_char*)strchr((char*)pString, ch);
}

u_char *pal_StringFindLastChar
(
	const u_char *pString,
	const u_char ch 
)
{
	/** This function checks all parameters passed to it. */
	/* The pString must not be NULL. */
	if (pString == NULL)
	{
		return NULL;
	}
	return (u_char*)strrchr((char*)pString, ch);
}

/**
 * Implementation of the pal_StringSNPrintf() function. See the
 * EcrioPAL_String.h file for interface definitions.
 */
s_int32 pal_StringSNPrintf
(
	char *pString,
	u_int32 size,
	const char *format,
	...
)
{
	va_list args;
	int written;

	/** This function checks all parameters passed to it. */
	/* The pString must not be NULL. */
	if (pString == NULL)
	{
		return 0;
	}

	if (size <= 0)
	{
		return 0;
	}

	/* The format must not be NULL. */
	if (format == NULL)
	{
		return 0;
	}

	/** Perform the string conversion for all the arguments using the
	    vsprintf() function. */
	va_start(args, format);
	written = vsnprintf(&pString[0], size, format, args);
	va_end(args);

	return written;
}
