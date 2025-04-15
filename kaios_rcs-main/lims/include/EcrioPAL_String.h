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
 * @file EcrioPAL_String.h
 * @brief This is the interface of the Ecrio PAL's String Module.
 *
 * The purpose of the String Module is to provide wrappers around common
 * string management and manipulation routines. Usually, string functions
 * closely mimic well-known string library functions provided by most
 * C runtime libraries.
 *
 * The String Module supports C Strings, which are arrays of 8-bit characters
 * ended by a NULL terminator, a character with the byte value of 0 (zero).
 * These functions are useful when dealing with ASCII characters.
 *
 */

#ifndef __ECRIOPAL_STRING_H__
#define __ECRIOPAL_STRING_H__

/* Include common PAL definitions for building. */
#include "EcrioPAL_Common.h"

/* Allow this module to be called directly from C++ compiled code. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * This function will allocate a buffer for a null terminated C string of the
 * size specified and copy the referenced source string to the new buffer.
 * When the caller is finished with the string, it should be freed using a
 * call to the pal_MemoryFree() function. The implementation of this function
 * must either use the pal_MemoryAllocate() or the same memory system that is
 * used to implement that function. If there are any memory allocation errors
 * or if the parameters are not valid, the function will return NULL.
 *
 * The implementation will typically use the well known strncpy() function
 * to perform the string copy. If the specified length is greater than the
 * size of the source string, the buffer should be padded with null characters.
 * If the length is less than the actual size of the source string, the
 * string should be truncated and a null terminator should always terminate
 * the string.
 *
 * It is expected that the buffer that is allocated will be one byte larger
 * than the specified size in order to accommodate the null terminator.
 *
 * @param[in] pSourceString		Pointer to a source string, which will be
 *								used to fill the newly created string. If this
 *								pointer is NULL, the string will still be
 *								created but will be empty. The function will
 *								not attempt to free or modify this pointer.
 * @param[in] count				The length of the new string to create. If the
 *								length is less than or equal to 0 (zero), the
 *								function will not allocate any memory and
 *								return NULL instead.
 * @return A pointer to the newly allocated string or NULL if there was an
 * error.
 */
u_char *pal_StringCreate
(
	const u_char *pSourceString,
	s_int32 count
);

/**
 * This function will obtain the number of characters in the null terminated C
 * string. If the string parameter is NULL, 0 will be returned. The
 * implementation will typically use the well known strlen() function.
 *
 * @param[in] pString			Pointer to the string to evaluate. If this
 *								pointer is NULL, the function will return 0.
 *								The function will not attempt to free of modify
 *								this pointer.
 * @return The length of the given string or 0 if there was an error.
 */
s_int32 pal_StringLength
(
	const u_char *pString
);

/**
 * This function will copy a null terminated C string to another, up to a
 * specified count. It is assumed that memory for the destination string is
 * already allocated and can accommodate the count, including a null
 * terminator if applicable. The implementation will typically use the well
 * known strncpy() function. However, if either of the string parameters are
 * NULL, the function will return NULL and not perform any modification to
 * the destination string.
 *
 * If the count is less than 0 (zero), NULL is returned and the
 * destination string will not be modified.
 *
 * If the destLength is less than count + 1, NULL is returned and the
 * destination string will not be modified.
 *
 * The destination string's null terminator must be appended.
 *
 * It can be assumed that the behavior of this function is undefined if
 * the source and destination strings overlap.
 *
 * @param[in] pDestString		Pointer to the destination string. If this
 *								pointer is NULL, the function will return NULL.
 *								Otherwise, after the function returns, the
 *								memory the pointer points to will hold a copy
 *								of the source string.
 * @param[in] destLength			Length of the pDestString. It should be bigger than count + 1.
 * @param[in] pSourceString		Pointer to the source string. If this pointer
 *								is NULL, the function will return NULL. The
 *								function will not attempt to free of modify
 *								this pointer.
 * @param[in] count				The number of characters to copy. If less than
 *								0 (zero), the function will return NULL.
 * @return A pointer to the copied string, the pDestString pointer, or NULL if
 * there was any error.
 */
u_char *pal_StringNCopy
(
	u_char *pDestString,
	u_int32 destLength,
	const u_char *pSourceString,
	s_int32 count
);

/**
 * This function will append a source string to a destination string, up to a
 * specified count. It is assumed that memory for the destination string is
 * already allocated and can accommodate the appended source string up to the
 * count, including a null terminator. The implementation will typically use
 * the well known strncat() function. However, if either of the string
 * parameters are NULL, the function will return NULL and not perform any
 * modification to the destination string.
 *
 * The destination string's null terminator will be overwritten and the
 * resulting destination string, with the appended source string, will be
 * null terminated. If a null terminator is found in the source string before
 * reaching the count, the source string is appended up to the null character.
 * If count is greater than the length of the source string, the source
 * string is appended up to its length.
 *
 * If the count is less than 0 (zero), NULL is returned and the
 * destination string will not be modified.
 *
 *
 * If the destLength is less than count + 1, NULL is returned and the
 * destination string will not be modified.
 *
 * The destination string's null terminator must be appended.
 *
 * It can be assumed that the behavior of this function is undefined if
 * the source and destination strings overlap.
 *
 * @param[in] pDestString		Pointer to the destination string. If this
 *								pointer is NULL, the function will return NULL.
 *								Otherwise, after the function returns, the
 *								memory the pointer points to will hold a the
 *								final string.
 * @param[in] destLength			Length of the pDestString. It should be bigger than count + 1.
 * @param[in] pSourceString		Pointer to the source string. If this pointer
 *								is NULL, the function will return NULL. The
 *								function will not attempt to free of modify
 *								this pointer.
 * @param[in] count				The number of characters to append. If less
 *								than 0 (zero), the function will return NULL.
 * @return A pointer to the concatenated string, the pDestString pointer, or
 * NULL if there was any error.
 */
u_char *pal_StringNConcatenate
(
	u_char *pDestString,
	u_int32 destLength,
	const u_char *pSourceString,
	s_int32 count
);

/**
 * This function compares two null terminated C strings. If the strings are
 * equal, 0 (zero) is returned. If the first string is lexicographically less
 * than the second string, -1 is returned. If the first string is greater
 * than the second string, +1 is returned.
 *
 * This function is to be case sensitive, considering upper and lower case
 * distinctly.
 *
 * The implementation will typically use the well known strcmp() function.
 * However, if both strings are NULL, they will be considered equal. If the
 * first string is NULL while the second string is not NULL, the first string
 * will be considered "less than" the second string. If the first string is
 * not NULL and the second string is NULL, then the first string will be
 * considered "greater than" the second string.
 *
 * @param[in] str1				Pointer to the first string.
 * @param[in] str2				Pointer to the second string.
 * @return The comparison result, 0 if the strings are equal, otherwise
 * -1 or +1.
 */
s_int32 pal_StringCompare
(
	const u_char *str1,
	const u_char *str2
);

/**
 * This function compares two null terminated C strings. If the strings are
 * equal, 0 (zero) is returned. If the first string is lexicographically less
 * than the second string, -1 is returned. If the first string is greater
 * than the second string, +1 is returned.
 *
 * This function is to be case insensitive, converting the strings to lower
 * case before evaluating them.
 *
 * The implementation will typically use the well known stricmp() function.
 * However, if both strings are NULL, they will be considered equal. If the
 * first string is NULL while the second string is not NULL, the first string
 * will be considered "less than" the second string. If the first string is
 * not NULL and the second string is NULL, then the first string will be
 * considered "greater than" the second string.
 *
 * @param[in] str1				Pointer to the first string.
 * @param[in] str2				Pointer to the second string.
 * @return The comparison result, 0 if the strings are equal, otherwise
 * -1 or +1.
 */
s_int32 pal_StringICompare
(
	const u_char *str1,
	const u_char *str2
);

/**
 * This function compares two null terminated C strings, up to the specified
 * count. If the strings are equal, 0 (zero) is returned. If the first string
 * is lexicographically less than the second string, -1 is returned. If the
 * first string is greater than the second string, +1 is returned.
 *
 * If the specified count is less than 0, -1 (negative 1) is returned, to
 * ensure that the caller won't assume the strings are equal even though there
 * has been some obvious error.
 *
 * This function is to be case sensitive, considering upper and lower case
 * distinctly.
 *
 * The implementation will typically use the well known strncmp() function.
 * However, if both strings are NULL, they will be considered equal. If the
 * first string is NULL while the second string is not NULL, the first string
 * will be considered "less than" the second string. If the first string is
 * not NULL and the second string is NULL, then the first string will be
 * considered "greater than" the second string.
 *
 * @param[in] str1				Pointer to the first string.
 * @param[in] str2				Pointer to the second string.
 * @param[in] count				The number of characters to compare. If less
 *								than 0 (zero), -1 will be returned.
 * @return The comparison result, 0 if the strings are equal, otherwise
 * -1 or +1.
 */
s_int32 pal_StringNCompare
(
	const u_char *str1,
	const u_char *str2,
	s_int32 count
);

/**
 * This function compares two null terminated C strings, up to the specified
 * count. If the strings are equal, 0 (zero) is returned. If the first string
 * is lexicographically less than the second string, -1 is returned. If the
 * first string is greater than the second string, +1 is returned.
 *
 * If the specified count is less than 0, -1 (negative 1) is returned, to
 * ensure that the caller won't assume the strings are equal even though there
 * has been some obvious error.
 *
 * This function is to be case insensitive, converting the strings to lower
 * case before evaluating them.
 *
 * The implementation will typically use the well known strnicmp() function.
 * However, if both strings are NULL, they will be considered equal. If the
 * first string is NULL while the second string is not NULL, the first string
 * will be considered "less than" the second string. If the first string is
 * not NULL and the second string is NULL, then the first string will be
 * considered "greater than" the second string.
 *
 * @param[in] str1				Pointer to the first string.
 * @param[in] str2				Pointer to the second string.
 * @param[in] count				The number of characters to compare. If less
 *								than 0 (zero), -1 will be returned.
 * @return The comparison result, 0 if the strings are equal, otherwise
 * -1 or +1.
 */
s_int32 pal_StringNICompare
(
	const u_char *str1,
	const u_char *str2,
	s_int32 count
);

/**
 * This function will search a string for the first occurrence of a specified
 * string.
 *
 * The implementation will typically use the well known strstr() function.
 * However, if either the string to search or the string to search for are
 * NULL, the function will return NULL. If the sub string is found in the
 * string, then the pointer to the first character of the sub string is
 * returned. If the sub string is not found in the string, NULL is returned.
 *
 * This function is to be case sensitive, considering upper and lower case
 * distinctly.
 *
 * @param[in] pString			Pointer to the string to search. If this is
 *								NULL, the function will return NULL.
 * @param[in] pSubString		Pointer to the string to search for. If this
 *								is NULL, the function will return NULL.
 * @return Pointer to the found string, or NULL if the sub string was not
 * found in the string or if there was an error.
 */
u_char *pal_StringFindSubString
(
	const u_char *pString,
	const u_char *pSubString
);

/**
 * This function will convert a string to an unsigned, 32-bit numeric value.
 * If the provided string is NULL, the function will return 0. If there is any
 * other problem in the conversion, the function should return 0. The
 * implementation will typically use a function like strtoul(). The conversion
 * routine should look at all characters that it can process as a number or
 * until a null terminator is found. The pEnd pointer reference will be set
 * to the character that stopped the scan, if it is not NULL.
 *
 * The provided numeric string should be in base 10 decimal format if the base
 * is 10 or in binary format if the base is 2, or in octal format if the base is 8,
 * or in hexadecimal format if the base is 16.
 *
 * @param[in] pString			Pointer to the string holding the string
 *								version of the number to convert. If this
 *								is NULL, the function will return 0.
 * @param[in,out] pEnd			Pointer reference to a pointer that will be
 *								set to the character that stops the scan for
 *								valid numeric characters that make up the
 *								unsigned integer value. If pEnd is NULL, it
 *								is not used.
 * @param[in] base				The base format of the provided string (i.e.
 *								10 for decimal, 2 for binary, etc.)
 * @return The unsigned 32-bit numeric value after the conversion or 0 if there
 * was an error.
 */
u_int32 pal_StringConvertToUNum
(
	const u_char *pString,
	u_char **pEnd,
	u_int32 base
);

/**
 * This function is used to find a character in a given string.
 * If the provided string is NULL, the function will return NULL. If there is any
 * other problem in the conversion, the function should return NULL. The
 * implementation will typically use a function like strchr(). This function returns
 * a pointer to the first occurence of char in string, or NULL if char is not found.
 *
 *
 * @param[in] pString			Pointer to the Null terminated source string from
 *								which characters needs to find.
 * @param[in] ch				Character to be located.
 * @returns a pointer to the first occurence of charecter in string, or NULL if
 * character is not found from the string.
 */
u_char *pal_StringFindChar
(
    const u_char *pString,
    const u_char ch
);

/**
 * This function is used to scan a string for the last occurence of character.
 * If the provided string is NULL, the function will return NULL. If there is any
 * other problem in the conversion, the function should return NULL. The
 * implementation will typically use a function like strrchr(). This function
 * returns a pointer to the last occurence of char in string, or NULL if char
 * is not found in given string.
 *
 * @param[in] pString			Pointer to the Null-terminated string to search.
 *								If this is NULL, the function will return NULL.
 * @param[in] ch				Character to be located.
 * @returns a pointer to the last occurence of character in string. or NULL if
 * character is not found from the string.
 */
u_char *pal_StringFindLastChar
(
    const u_char *pString,
    const u_char ch
);

/**
 * This function will convert a variable number of parameters of a given
 * format into a null terminated C string. The output will be placed in the
 * given string buffer. This function is intended to wrap the well known
 * vsnprintf() function.
 *
 * The function will return the string length of the resulting string that is
 * placed in the pString buffer. If pString is NULL, the function will
 * return 0. A null character is appended to the end of the string, but is
 * not included in the returned length. If there is any error, a -1 will
 * be returned. If format is NULL, 0 is returned.
 *
 * If pString is not NULL, it is assumed to be of adequate size to act as a
 * container for all format conversions. Therefore the Calling Component
 * should ensure that the buffer is allocated for a proper size. The pString
 * pointer should not be passed as an argument to be formatted.
 *
 * The destination string's null terminator must be appended.
 *
 * @param[in,out] pString		Pointer to a string to write the conversion
 *								output. If this is NULL, the function will
 *								return 0 (zero). The string must be allocated and
 *								large enough to accommodate the resulting string
 *								and a null terminator.
 * @param[in] size			Length of the pString.
 * @param[in] format			The format control string to use, decorated with
 *								a variable number of format types.
 * @param[in] ...				The variable argument list to convert.
 * @return The number of bytes stored in the buffer, 0 if there was an error
 * or -1 if the buffer was filled and data was lost due to null termination.
 */
s_int32 pal_StringSNPrintf
(
	char *pString,
	u_int32 size,
	const char *format,
	...
);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __ECRIOPAL_STRING_H__ */
