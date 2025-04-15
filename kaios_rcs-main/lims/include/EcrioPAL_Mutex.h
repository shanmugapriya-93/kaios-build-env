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
 * @file EcrioPAL_Mutex.h
 * @brief This is the header file for the public interface of the Ecrio PAL's
 * Mutex Module.
 *
 * The Ecrio PAL Mutex Module must support the threading concept of mutual
 * exclusion in that access to a shared resource can be synchronized. The APIs
 * provided include the capability to create and delete a mutex, as well as
 * the actual locking and unlocking of the mutex. A caller can create as many
 * mutex "objects" as desired.
 *
 * Since the Mutex Module is itself used to support multithreading, the Mutex
 * Module implementation should not get in to a situation where it needs to
 * lock and unlock mutexes for the sake of locking and unlocking mutexes on
 * behalf of the caller. Therefore, the following behavior characteristics
 * must be supported, whether or not the underlying mutex support of the
 * operating systems behaves this way or not:
 *  - Multiple calls to lock the mutex are allowed from the same thread and
 *    should not block
 *  - Each call to lock a mutex must have a corresponding call to unlock
 *  - Lock attempts will never timeout
 *
 * The implementation is free to use any operating system mechanism to achieve
 * mutual exclusion. This module definition does not support locks across
 * process boundaries or "named mutexes", even though some operating systems
 * support those concepts. The definition of the Ecrio PAL's Mutex Module is
 * meant to synchronize threads within a single process.
 *
 * When multiple threads attempt to lock the same mutex, it should be noted
 * that there is no guarantee of order as to who gets the lock after the
 * previous thread unlocks the mutex. This is generally the behavior in most
 * operating system implementations and this interface definition does not
 * make any other claims beyond that behavior. Additionally, most operating
 * system mutex implementations document unpredictable behavior when locking
 * a mutex that hasn't been properly created or deleting a mutex that is
 * still locked. Again, this interface definition does not offer any
 * alternative behavior.
 *
 * For platforms that do not support multithreading, the Mutex Module is not
 * useful and can be stubbed (i.e. empty).
 *
 * <b>Multithreading</b>
 *
 * All Mutex Module API functions are thread safe. However, the underlying
 * features provided by the platform must ensure thread safety. This is
 * generally the case when linking against "Multithreaded" libraries.
 *
 * <b>Logging</b>
 *
 * No direct logging support is offered by the Mutex Module.
 */

#ifndef __ECRIOPAL_MUTEX_H__
#define __ECRIOPAL_MUTEX_H__

/* Include common PAL definitions for building. */
#include "EcrioPAL_Common.h"

/* Allow this module to be called directly from C++ compiled code. */
#ifdef __cplusplus
extern "C" {
#endif

/** @name Module Handles
 * \n Handles are void pointers which will point to structures known
 * only internally to the Module. The Calling Component should only
 * reference the returned handles by these defined types. The handles
 * supported by this module include:
 */
/*@{*/

/**
 * A definition for the Mutex instance, for use internally by the PAL.
 */
typedef void *MUTEXINSTANCE;

/**
 * The Mutex handle refers to the overall Mutex Module instance that is
 * created after calling pal_MutexCreate().
 */
typedef void *MUTEXHANDLE;

/*@}*/

/**
 * This function is used to create a mutex.
 *
 *
 * @param[in] pal
 * @param[out] handle
 * @return KPALErrorNone if successful, otherwise an error value.
 */
u_int32 pal_MutexCreate
(
	PALINSTANCE pal,
	MUTEXHANDLE *handle
);

/**
 * This function is used to delete a mutex.
 *
 * @param[in, out] handle		Pointer to a reference of the Mutex Module
 *								handle. If the referenced pointer is NULL, then
 *								the function will return an error. If not NULL,
 *								an attempt will be	made to clean up resources
 *								and memory. The reference will be set to NULL
 *								before returning.
 * @return KPALErrorNone if successful, otherwise an error value.
 */
u_int32 pal_MutexDelete
(
	MUTEXHANDLE *handle
);

/**
 * This function will attempt to lock a mutex. If the mutex is already locked
 * by another thread, the function will block, forever, until the thread is
 * given the lock. If the mutex is already locked by the current thread, the
 * function should return successfully. If the mutex is not yet locked, the
 * thread shall get the lock and the function should return immediately.
 *
 * @param[in] handle			Pointer to a Mutex Module instance handle.
 *								If the pointer is NULL, then the function
 *								will return an error.
 * @return KPALErrorNone if successful, otherwise an error value.
 */
u_int32 pal_MutexLock
(
	MUTEXHANDLE handle
);

/**
 * This function will unlock a mutex. If the mutex was previously locked, it
 * should be unlocked and any other threads that are waiting for the lock
 * will have a chance to obtain the lock. This function should not be called
 * unless the thread has locked the mutex.
 *
 * @param[in] handle			Pointer to a Mutex Module instance handle.
 *								If the pointer is NULL, then the function
 *								will return an error.
 * @return KPALErrorNone if successful, otherwise an error value.
 */
u_int32 pal_MutexUnlock
(
	MUTEXHANDLE handle
);

/**
 * This function will attempt to lock the global mutex, in the scope of the
 * given PAL instance.
 *
 * @param[in] pal				PAL instance to use.
 * @return KPALErrorNone if successful, otherwise an error value.
 */
u_int32 pal_MutexGlobalLock
(
	PALINSTANCE pal
);

/**
 * This function will attempt to unlock the global mutex, in the scope of the
 * given PAL instance.
 *
 * @param[in] pal				PAL instance to use.
 * @return KPALErrorNone if successful, otherwise an error value.
 */
u_int32 pal_MutexGlobalUnlock
(
	PALINSTANCE pal
);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __ECRIOPAL_MUTEX_H__ */
