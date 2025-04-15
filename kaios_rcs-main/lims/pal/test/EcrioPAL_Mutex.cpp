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
 * @brief This is a test version of Ecrio PAL's Mutex Module.
 */

// @note This is currently the same as the reference implementation, no test hooks are needed yet.

/* Standard Windows includes. */
#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdlib.h>

/* Ignore deprecated function warnings. */
#pragma warning(disable: 4996)

/*lint -e970 In the Ecrio Win32/WM PAL implementation, no typedef policy is needed. */
#else
#if defined(__linux__)
#include <cstdlib>
#include <pthread.h>
#endif
#endif

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
#if defined(WIN32)
	CRITICAL_SECTION criticalSection;	/**< Reference to a Win32 Critical Section object, used to act as a mutex. */
#else
#if defined(__linux__)
	pthread_mutex_t criticalSection;	/**< Reference to a pthread_mutex_t object, used to act as a mutex. */
#endif
#endif
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
#if defined(__linux__)
	int res = 0;
#endif

	(void)pal;
	(void)logHandle;

	if (instance == NULL)
	{
		return KPALInvalidParameters;
	}

	*instance = NULL;

#if defined(WIN32)
	m = (MutexInstanceStruct *)malloc(sizeof(MutexInstanceStruct));
#else
#if defined(__linux__)
	m = (MutexInstanceStruct *)calloc(1, sizeof(MutexInstanceStruct));
#endif
#endif
	if (m == NULL)
	{
		return KPALMemoryError;
	}

#if defined(WIN32)
	/** Initialize the Critical Section. */
	InitializeCriticalSection(&m->global_mutex.criticalSection);
	/* No return value is available. */

	/* @note Someday we may add some array management for pre-allocated mutex objects. */
#else
#if defined(__linux__)
	/** Initialize the Critical Section. */
	res = pthread_mutex_init(&(m->global_mutex.criticalSection), NULL);
	if (res != 0)
	{
		/* pthread_mutex_init() function failed. */
		free(m);
		return KPALInternalError;
	}
#endif
#endif

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
#if defined(WIN32)
	DeleteCriticalSection(&m->global_mutex.criticalSection);
#else
#if defined(__linux__)
	pthread_mutex_destroy(&m->global_mutex.criticalSection);
#endif
#endif
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
#if defined(__linux__)
	int res = 0;
#endif

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
#if defined(WIN32)
	InitializeCriticalSection(&h->criticalSection);
#else
#if defined(__linux__)
	res = pthread_mutex_init(&(h->criticalSection), NULL);
	if (res != 0)
	{
		/* pthread_mutex_init() function failed. */
		free(h);

		return KPALInternalError;
	}
#endif
#endif
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
#if defined(WIN32)
	DeleteCriticalSection(&h->criticalSection);
#else
#if defined(__linux__)
	pthread_mutex_destroy(&(h->criticalSection));
#endif
#endif
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
#if defined(__linux__)
	int res = 0;
#endif

	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidHandle;
	}

	/** Enter the critical section which will imply "locking the mutex". If
	    another thread has the lock already, this function will blcok until
	    the operating system gives this thread the lock. */
#if defined(WIN32)
	EnterCriticalSection(&h->criticalSection);
#else
#if defined(__linux__)
	res = pthread_mutex_lock(&(h->criticalSection));
	if (res != 0)
	{
		/* pthread_mutex_lock() function failed. */
		return KPALInternalError;
	}
#endif
#endif
	/* No return value is available. */

	return KPALErrorNone;
}

u_int32 pal_MutexUnlock
(
	MUTEXHANDLE handle
)
{
	MutexStruct *h = (MutexStruct *)handle;
#if defined(__linux__)
	int res = 0;
#endif

	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidHandle;
	}

	/** Leave the critical section which will imply "unlocking the mutex". */
#if defined(WIN32)
	LeaveCriticalSection(&h->criticalSection);
#else
#if defined(__linux__)
	res = pthread_mutex_unlock(&(h->criticalSection));
	if (res != 0)
	{
		/* pthread_mutex_unlock() function failed. */
		return KPALInternalError;
	}
#endif
#endif
	/* No return value is available. */

	return KPALErrorNone;
}

u_int32 pal_MutexGlobalLock
(
	PALINSTANCE pal
)
{
	MutexInstanceStruct *m = NULL;
#if defined(__linux__)
	int res = 0;
#endif

	if (pal == NULL)
	{
		return KPALInvalidParameters;
	}

	m = (MutexInstanceStruct *)((EcrioPALStruct *)pal)->mutex;

	if (m == NULL)
	{
		return KPALInvalidHandle;
	}

#if defined(WIN32)
	EnterCriticalSection(&m->global_mutex.criticalSection);
#else
#if defined(__linux__)
	res = pthread_mutex_lock(&(m->global_mutex.criticalSection));
	if (res != 0)
	{
		/* pthread_mutex_lock() function failed. */
		return KPALInternalError;
	}
#endif
#endif

	return KPALErrorNone;
}

u_int32 pal_MutexGlobalUnlock
(
	PALINSTANCE pal
)
{
	MutexInstanceStruct *m = NULL;
#if defined(__linux__)
	int res = 0;
#endif

	if (pal == NULL)
	{
		return KPALInvalidParameters;
	}

	m = (MutexInstanceStruct *)((EcrioPALStruct *)pal)->mutex;

	if (m == NULL)
	{
		return KPALInvalidHandle;
	}

#if defined(WIN32)
	LeaveCriticalSection(&m->global_mutex.criticalSection);
#else
#if defined(__linux__)
	/** Leave the critical section which will imply "unlocking the mutex". */
	res = pthread_mutex_unlock(&(m->global_mutex.criticalSection));
	if (res != 0)
	{
		/* pthread_mutex_unlock() function failed. */
		return KPALInternalError;
	}
#endif
#endif

	return KPALErrorNone;
}
