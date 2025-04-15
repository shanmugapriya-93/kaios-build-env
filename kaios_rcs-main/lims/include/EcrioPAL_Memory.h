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
 * @file EcrioPAL_Memory.h
 * @brief This is the header file for the public interface of the Ecrio PAL's
 * Memory Module.
 *
 * The Memory Module of the Ecrio PAL supports the interfaces outlined in this
 * document. The Memory Module is responsible for wrapping memory management
 * features of the operating system and/or C runtime library. The functions
 * include memory allocation, duplication, and comparison. In general, the
 * functions will wrap well-known memory management functions. An
 * implementation may alternatively implement its own heap management for a
 * given memory range, although this approach would be rare.
 *
 * <b>Multithreading</b>
 *
 * All Memory Module API functions are thread safe, particularly since most
 * implementations will not maintain any state. However, the underlying
 * memory management features provided by the platform must ensure thread
 * safety. This is generally the case when linking against "Multithreaded"
 * libraries.
 *
 * <b>Logging</b>
 *
 * No direct logging support is offered by the Memory Module.
 */

#ifndef __ECRIOPAL_MEMORY_H__
#define __ECRIOPAL_MEMORY_H__

/* Include common PAL definitions for building. */
#include "EcrioPAL_Common.h"

/* Allow this module to be called directly from C++ compiled code. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * This function is used to allocate memory dynamically from a heap. Typically
 * the heap is managed by either the operating system or C runtime library. The
 * caller specifies the number of bytes to allocate and will get pointer to the
 * allocated memory in return. The underlying functionality should mimic the
 * well-known function, malloc().
 *
 * The caller is responsible for calling the pal_MemoryFree() function to free
 * the memory.
 *
 * The currently expected behavior is that the memory, upon allocation, shall
 * be initialized to 0 (zero) byte values. This behavior is deprecated. Future
 * implementations of the Memory Module may not provide this ability.
 * Therefore, callers that need the memory initialized should call the
 * pal_MemorySet() function immediately after calling this function in order
 * to achieve the same level of initialization. This deprecation is due to
 * performance requirements, since initializing every memory allocation is
 * an expensive operation.
 *
 * @param[in] memLength			Length of the memory block to be allocated, in
 *								bytes.
 * @param[in] ppMem				Pointer to a reference of where to store the
 *								pointer to the newly allocated memory. If the
 *								reference already holds a valid pointer to
 *								memory, it is ignored and will be overwritten.
 *								Therefore, the caller should guard against
 *								this possibility since it will result in a
 *								memory leak.
 * @return The function will return the result of the function call. An
 * implementation can set any error enumerated by KPALMemoryError. At minimum,
 * an implementation must support the following errors:
 * - KPALErrorNone if successful
 * - KPALInvalidParameters if any parameter is not valid
 * - KPALInvalidSize if the memLength is 0
 * - KPALMemoryError if the resulting allocation returned NULL
 */
u_int32 pal_MemoryAllocate
(
	u_int32 memLength,
	void **ppMem
);

/**
 * This function "re-allocates" memory dynamically from a heap. The caller
 * specifies the number of bytes to re-allocate and will get pointer to the
 * re-allocated memory in return. The underlying functionality should mimic
 * the well-known function, realloc(). The new allocation size might be
 * smaller or larger and the contents of the original block are unchanged.
 * The newly allocated memory is not initialized and the pointer provided
 * as input may not be the same as the pointer provided as output.
 *
 * The caller is responsible for calling the pal_MemoryFree() function to free
 * the memory.
 *
 * @param[in] memLength			Length of the memory block to be re-allocated,
 *								in bytes.
 * @param[in,out] ppMem			Pointer to a reference of where to obtain the
 *								previously allocated memory pointer and where
 *								to store the pointer to the newly re-allocated
 *								memory. If the reference holds a NULL pointer,
 *								the function shall behave like a call to
 *								pal_MemoryAllocate() and allocate the block of
 *								memory newly.
 * @return The function will return the result of the function call. An
 * implementation can set any error enumerated by KPALMemoryError. At minimum,
 * an implementation must support the following errors:
 * - KPALErrorNone if successful
 * - KPALInvalidParameters if any parameter is not valid
 * - KPALInvalidSize if the memLength is 0
 * - KPALMemoryError if the resulting allocation returned NULL
 */
u_int32 pal_MemoryReallocate
(
	u_int32 memLength,
	void **ppMem
);

/**
 * This function will set all bytes in the referenced memory block to the
 * specified value. The underlying functionality should mimic the well-known
 * memset() function. The value parameter is a signed integer type, but only
 * the least significant byte of the value is used. The memory block is
 * identified by the memory pointer and length.
 *
 * @param[in] pMem				Pointer to the memory block to be processed.
 * @param[in] value				The value to set each byte in the memory block
 *								to, using only the least significant byte.
 * @param[in] memLength			Length of the memory block to be set, in bytes.
 * @return The function will return the result of the function call. An
 * implementation can set any error enumerated by KPALMemoryError. At minimum,
 * an implementation must support the following errors:
 * - KPALErrorNone if successful
 * - KPALInvalidParameters if any parameter is not valid
 * - KPALInvalidSize if the memLength is 0
 */
u_int32 pal_MemorySet
(
	void *pMem,
	s_int32 value,
	u_int32 memLength
);

/**
 * This function will copy byte data from one memory block to another. The
 * underlying functionality should mimic the well-known memmove() function.
 * This function will copy one memory block to another without losing any
 * data in case the two blocks overlap. There is a source memory block, from
 * which bytes are copied from, a destination memory block which bytes are
 * copied to, and the length.
 *
 * @param[in] pDest				Pointer to the destination memory block, to
 *								copy byte data to.
 * @param[in] destLength			Length of the pDest.
 * @param[in] pSource			Pointer to the source memory block, to copy
 *								byte data from.
 * @param[in] memLength			Length of the memory block to be copied, in
 *								bytes.
 * @return The function will return the result of the function call. An
 * implementation can set any error enumerated by KPALMemoryError. At minimum,
 * an implementation must support the following errors:
 * - KPALErrorNone if successful
 * - KPALInvalidParameters if any parameter is not valid
 * - KPALInvalidSize if the memLength is 0
 */
u_int32 pal_MemoryCopy
(
	void *pDest,
	u_int32 destLength,
	const void *pSource,
	u_int32 memLength
);

/**
 * This function will compare two memory blocks with one another, byte by byte.
 * The underlying functionality should mimic the well-known memcmp() function.
 * In fact, the signature of this function is equivalent to the standard
 * memcmp(). This function will return the comparison result directly, without
 * any error code. Therefore, if any of the parameters are not valid, a certain
 * behavior is followed.
 *
 * If the first block is NULL while the second block is not NULL, the first
 * block is considered "less" than the second and hence -1 is returned.
 *
 * If the second block is NULL while the first block is not NULL, the first
 * block is considered "greater" than the second and hence 1 is returned.
 *
 * If both blocks are NULL, or, if the length to compare is 0, 0 is returned
 * because each block is considered identical with the information provided.
 *
 * If a Component can't handle this behavior, it should perform parameter
 * validation or ensure valid parameters before calling this function.
 *
 * @param[in] pBuf1				The first memory block to be compared.
 * @param[in] pBuf2				The second memory block to be compared.
 * @param[in] memLength			Length of the memory block to be compared, in
 *								bytes.
 * @return A return less than 0 (zero) means pBuf1 is less than	pBuf2. A
 * return equal to 0 (zero) means pBuf1 is identical to	pBuf2. A return
 * greater than 0 (zero) means pBuf1 is greater than pBuf2.
 */
s_int32 pal_MemoryCompare
(
    const void *pBuf1,
    const void *pBuf2,
    u_int32 memLength
);

/**
 * This function will search memory starting at pSource which matches the
 * memory starting at pSearchMem. The function will return NULL if no
 * match can be found. A match can only be made if it the same size as
 * the searchLength.
 *
 * If either pSource or pSearchMem is NULL or if either lengths are 0 then
 * NULL is returned.
 *
 * @param[in] pSource			Pointer to the memory to search.
 * @param[in] memLength			Valid size of the memory to search.
 * @param[in] pSearchMem		Pointer to pattern to search for.
 * @param[in] searchLength		Size of the pattern to search for.
 *
 * @return NULL if no match is found.
 */
u_char *pal_MemorySearch
(
    const void *pSource,
    u_int32 memLength,
    const u_char *pSearchMem,
    u_int32 searchLength
);

/**
 * This function will free memory that was dynamically allocated from a heap.
 * This function must be called for each call to pal_MemoryAllocate() and
 * pal_MemoryReallocate(). The underlying functionality should mimic the well-
 * known free() function. This function might return errors based on invalid
 * parameter input. If the memory pointer is NULL, the function will return
 * successfully, since the result is what is desired.
 *
 * @param[in,out] ppPtr			Pointer to a reference of where to obtain the
 *								previously allocated memory pointer. If the
 *								reference holds a NULL pointer, nothing is done
 *								and the function will return successfully.
 *								Otherwise the function will free the allocated
 *								memory and set the reference to NULL, so the
 *								pointer is set to NULL in the caller's context.
 * @return The function will return the result of the function call. An
 * implementation can set any error enumerated by KPALMemoryError. At minimum,
 * an implementation must support the following errors:
 * - KPALErrorNone if successful
 * - KPALInvalidParameters if any parameter is not valid
 */
u_int32 pal_MemoryFree
(
	void **ppPtr
);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __ECRIOPAL_MEMORY_H__ */
