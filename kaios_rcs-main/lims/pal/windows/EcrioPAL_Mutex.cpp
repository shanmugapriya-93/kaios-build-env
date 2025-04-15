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
 * @file EcrioPAL_Mutex.c
 * @brief This is the implementation of the Ecrio PAL's Mutex Module.
 *
 * This implementation uses the Win32 Critical Section APIs to achieve mutex
 * behavior. Note that unlocking a mutex (leaving a critical section) that has
 * not been locked (entered) or deleting a mutex/critical section that is
 * still locked/entered will cause unpredictable behavior.
 *
 * <b>Design Overview</b>
 *
 * The design is straightforward since there are no callback functions and no
 * special multithreading concerns. Creating a mutex is implemented by
 * initializing a critical section, deleting a mutex is implemented by
 * deleting a critical section, locking a mutex is implemented by entering
 * a critical section, and unlocking a mutex is implemented by leaving a
 * critical section.
 *
 * Critical sections are used because they use much less resources than the
 * Mutex objects provided by the Win32 API.
 *
 * Other code is used to manage the Mutex Module's instance handle and to
 * check that it is valid upon entry to the functions.
 *
 * <b>Synchronization</b>
 *
 * The interface and implementation dictate that the Mutex Module is a
 * synchronous set of APIs that can be fully re-entrant. In other words,
 * multiple threads can call any of the APIs in any order and simultaneously.
 * There is no synchronization performed by the Mutex Module.
 *
 * <b>Logging</b>
 *
 * There is no logging performed within the Mutex Module functions.
 */

/* Standard Windows includes. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdlib.h>

/* Ignore deprecated function warnings. */
#pragma warning(disable: 4996)

/*lint -e970 In the Ecrio Win32/WM PAL implementation, no typedef policy is needed. */

#include "EcrioPAL.h"
#include "EcrioPAL_Internal.h"

/** @struct MutexStruct
 * This is the internal structure maintained by the Mutex Module and represents
 * a Mutex Module instance. Memory is allocated for it when the Create()
 * function is called and it is what the Mutex Module Handle points to. All
 * subsequent interface functions receive this pointer as parameter input.
 */
typedef struct
{
	CRITICAL_SECTION criticalSection;	/**< Reference to a Win32 Critical Section object, used to act as a mutex. */
} MutexStruct;

/** @struct MutexInstanceStruct
 * This is the instance structure for the Mutex PAL, one of the PAL modules
 * that will allocate memory.
 */
typedef struct
{
	MutexStruct global_mutex;			/**< Reference to a Global Mutex accessed as a singleton for the instance. */
} MutexInstanceStruct;

/**
 * For the Mutex module on Windows, we will just allocate the instance structure
 * and setup the associated Global Mutex.
 */
u_int32 pal_mutexInit
(
	PALINSTANCE pal,
	LOGHANDLE logHandle,
	MUTEXINSTANCE *instance
)
{
	MutexInstanceStruct *m = NULL;

	(void)pal;
	(void)logHandle;

	if (instance == NULL)
	{
		return KPALInvalidParameters;
	}

	*instance = NULL;

	m = (MutexInstanceStruct *)malloc(sizeof(MutexInstanceStruct));
	if (m == NULL)
	{
		return KPALMemoryError;
	}

	/** Initialize the Critical Section. */
	InitializeCriticalSection(&m->global_mutex.criticalSection);
	/* No return value is available. */

	/* @note Someday we may add some array management for pre-allocated mutex objects. */

	*instance = (MUTEXINSTANCE)m;

	return KPALErrorNone;
}

/**
 * Clean up the instance.
 */
void pal_mutexDeinit
(
	MUTEXINSTANCE instance
)
{
	MutexInstanceStruct *m = (MutexInstanceStruct *)instance;

	/** Delete the critical section. */
	DeleteCriticalSection(&m->global_mutex.criticalSection);
	/* No return value is available. */

	free(m);
}

u_int32 pal_MutexCreate
(
	PALINSTANCE pal,
	MUTEXHANDLE *handle
)
{
	MutexStruct *h = NULL;

	/** The pal instance is not used for mutex resource allocations at this time. */
	(void)pal;

	/** This function checks all parameters passed to it. */
	/* pError must be a valid location for error values. */
	if (handle == NULL)
	{
		/* If pError is NULL, return NULL, which should be detected as a failure. */
		return KPALInvalidParameters;
	}

	*handle = NULL;

	/** Allocate memory for this instance of the Mutex Module. */
	h = (MutexStruct *)calloc(1, sizeof(MutexStruct));
	if (h == NULL)
	{
		/* Memory for the Mutex Module is not available. */
		return KPALMemoryError;
	}

	/** Initialize the Critical Section. */
	InitializeCriticalSection(&h->criticalSection);
	/* No return value is available. */

	*handle = h;

	return KPALErrorNone;
}

u_int32 pal_MutexDelete
(
	MUTEXHANDLE *handle
)
{
	MutexStruct *h = NULL;

	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidParameters;
	}

	/* For Deinit() functions, the Handle must NOT be NULL. */
	if (NULL == *handle)
	{
		return KPALInvalidHandle;
	}

	/* Set the handle pointer to the proper internal pointer. */
	h = (MutexStruct *)*handle;

	/** Delete the critical section. */
	DeleteCriticalSection(&h->criticalSection);
	/* No return value is available. */

	/** Free the memory allocated for the Mutex Module instance. */
	free(h);

	/** Set the output parameter to NULL, to force NULL in the Calling
	    Component's context. */
	*handle = NULL;

	return KPALErrorNone;
}

u_int32 pal_MutexLock
(
	MUTEXHANDLE handle
)
{
	MutexStruct *h = (MutexStruct *)handle;

	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidHandle;
	}

	/** Enter the critical section which will imply "locking the mutex". If
	    another thread has the lock already, this function will blcok until
	    the operating system gives this thread the lock. */
	EnterCriticalSection(&h->criticalSection);
	/* No return value is available. */

	return KPALErrorNone;
}

u_int32 pal_MutexUnlock
(
	MUTEXHANDLE handle
)
{
	MutexStruct *h = (MutexStruct *)handle;

	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidHandle;
	}

	/** Leave the critical section which will imply "unlocking the mutex". */
	LeaveCriticalSection(&h->criticalSection);
	/* No return value is available. */

	return KPALErrorNone;
}

u_int32 pal_MutexGlobalLock
(
	PALINSTANCE pal
)
{
	MutexInstanceStruct *m = NULL;

	if (pal == NULL)
	{
		return KPALInvalidParameters;
	}

	m = (MutexInstanceStruct *)((EcrioPALStruct *)pal)->mutex;

	if (m == NULL)
	{
		return KPALInvalidHandle;
	}

	EnterCriticalSection(&m->global_mutex.criticalSection);

	return KPALErrorNone;
}

u_int32 pal_MutexGlobalUnlock
(
	PALINSTANCE pal
)
{
	MutexInstanceStruct *m = NULL;

	if (pal == NULL)
	{
		return KPALInvalidParameters;
	}

	m = (MutexInstanceStruct *)((EcrioPALStruct *)pal)->mutex;

	if (m == NULL)
	{
		return KPALInvalidHandle;
	}

	LeaveCriticalSection(&m->global_mutex.criticalSection);

	return KPALErrorNone;
}
