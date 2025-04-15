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
 * @file EcrioPAL_Memory.c
 * @brief This is the implementation of the Ecrio PAL's Memory Module.
 *
 * This implementation of the Memory Module uses the C runtime library
 * functions for memory management. There are no Win32 API calls used.
 *
 * <b>Design Overview</b>
 *
 * There is no overall design of the Memory Module since each function stands
 * on its own. Therefore, each function will have its own documentation in
 * regards to how it is designed and how it operates.
 *
 * However, in general, each function will first check the validity of
 * all parameters, and then call the relevant function provided by the
 * platform.
 *
 * <b>Synchronization</b>
 *
 * The interface and implementation dictate that the Memory Module is a
 * synchronous set of APIs that can be fully re-entrant. In other words,
 * multiple threads can call any of the APIs in any order and simultaneously.
 * It is important that the underlying C runtime library also have multi-
 * threaded support. For Win32 and Windows Mobile builds, it is therefore
 * extremely important to always link with the "Multi-Threaded" version of
 * the libraries. There is no synchronization performed by the Memory Module
 * especially since the module does not maintain any state and because there
 * are no callback functions to call or process.
 *
 * <b>Logging</b>
 *
 * There is no logging performed within the Memory Module functions.
 */

#include <stdlib.h>
#include <memory.h>
#include <string.h>

/* Ignore deprecated function warnings. */
#pragma warning(disable: 4996)

/*lint -e970 In the Ecrio Win32/WM PAL implementation, no typedef policy is needed. */

#include "EcrioPAL.h"

#ifdef _DEBUG

/* Standard Windows includes. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <unordered_map>
using namespace std;

typedef struct
{
	LOGHANDLE logHandle;
	unordered_map<void *, u_int32> allocations;
	CRITICAL_SECTION mutex;
	u_int32 uCurrentAlloc;
	u_int32 uMaxAlloc;
	u_int32 uTotalOperations;
	u_int32 uTotalErrors;
} MemoryStruct;

// Define the memory testing object globally.
MemoryStruct memoryTest;

#endif

u_int32 pal_MemoryAllocate
(
	u_int32 memLength,
	void **ppMem
)
{
	/** The native type of most memory allocation primitives is void. */
	void *pPtr = NULL;

	/** This function checks all parameters passed to it. */
	/* The pointer to the memory pointer must not be NULL. */
	if (ppMem == NULL)
	{
		return KPALInvalidParameters;
	}

	/** The memory pointer must be NULL itself. Otherwise we might be
	    overwriting some allocated memory which may cause a memory leak. */
	/*
	if (*ppMem != NULL)
	{
	    return KPALPointerNotNULL;
	}
	*/
	/** Although the memory pointer should be NULL, too many components
	    aren't ensuring this. Therefore, we will set it to NULL
	    ourselves. Perhaps memory leaks will be shown as originating
	    from this line of code, but don't blame the memory module.
	    For now, we will set this to NULL as early as possible and
	    let lazy callers that don't check the return value succeed
	    or fail based on the pointer itself. If we even do the
	    *ppMem != NULL evaluation, our analysis tools will return
	    uninitialized pointer errors! */
	*ppMem = NULL;

	/* The length to allocation must not be 0. */
	if (memLength == 0)
	{
		return KPALInvalidSize;
	}

	/** Allocate the requested memory. Note that the calloc() function is the
	    function that is used. calloc() will allocate and initialize memory to
	    0 (zero) byte values. In many cases, this is a waste of CPU cycles as
	    it is expensive. Therefore, the interface shall note this behavior,
	    but that it is deprecated. In the future, simply use malloc() and then
	    the caller can do a pal_MemorySet() if desired. */
	pPtr = calloc(memLength, 1);
	if (pPtr == NULL)
	{
		/* The memory allocation was unsuccessful. The only thing that can be
		   assumed is that there is no memory available. */
		return KPALMemoryError;
	}

#ifdef _DEBUG
	{
		EnterCriticalSection(&memoryTest.mutex);

		memoryTest.allocations.insert({pPtr, memLength});

		memoryTest.uCurrentAlloc += memLength;
		memoryTest.uTotalOperations++;
		if (memoryTest.uMaxAlloc < memoryTest.uCurrentAlloc)
		{
			memoryTest.uMaxAlloc = memoryTest.uCurrentAlloc;
		}

		pal_LogMessage(memoryTest.logHandle, KLogLevelInfo, KLogComponentPAL, KLogTypeMemory, "Alloc:\t%p\t%p\t%u\t%u\t%u\t%u",
			pPtr, pPtr, memLength, 0, memoryTest.uCurrentAlloc, memoryTest.uTotalOperations);

		LeaveCriticalSection(&memoryTest.mutex);
	}
#endif

	/** Set the output parameter to the allocated memory. */
	*ppMem = pPtr;

	return KPALErrorNone;
}

u_int32 pal_MemoryReallocate
(
	u_int32 memLength,
	void **ppMem
)
{
	/** The native type of most memory allocation primitives is void. */
	void *pPtr = NULL;

	/** This function checks all parameters passed to it. */
	/* The pointer to the memory pointer must not be NULL. */
	if (ppMem == NULL)
	{
		return KPALInvalidParameters;
	}

	/* The length to allocation must not be 0. */
	if (memLength == 0)
	{
		return KPALInvalidSize;
	}

	/** Reallocate the requested memory. Note that the act of reallocating
	    memory is not recommended. Avoid if at all possible. This function
	    uses the realloc() function. */
	pPtr = realloc(*ppMem, memLength);
	if (pPtr == NULL)
	{
#ifdef _DEBUG
		u_int32 uPrevLength = 0;

		EnterCriticalSection(&memoryTest.mutex);

		if (*ppMem != NULL)
		{
			unordered_map<void *, u_int32>::const_iterator it = memoryTest.allocations.find(*ppMem);
			if (it == memoryTest.allocations.end())
			{
				/* The memory was not found... */

				memoryTest.uTotalErrors++;

				pal_LogMessage(memoryTest.logHandle, KLogLevelInfo, KLogComponentPAL, KLogTypeMemory, "Realloc: Corruption\t%p\t%p",
					pPtr, *ppMem);
			}
			else
			{
				uPrevLength = it->second;

				memoryTest.uCurrentAlloc -= it->second;

				memoryTest.allocations.erase(it);
			}
		}

		memoryTest.uTotalOperations++;

		pal_LogMessage(memoryTest.logHandle, KLogLevelInfo, KLogComponentPAL, KLogTypeMemory, "Realloc Failure:\t%p\t%p\t%u\t%u\t%u\t%u",
			pPtr, *ppMem, memLength, uPrevLength, memoryTest.uCurrentAlloc, memoryTest.uTotalOperations);

		LeaveCriticalSection(&memoryTest.mutex);
#endif

		if (*ppMem != NULL)
		{
			/** Free the actual memory that was allocated for the original allocation using free(). */
			free(*ppMem);

			/** Return NULL to force handling in caller. */
			*ppMem = NULL;
		}

		/* The memory allocation was unsuccessful. The only thing that can be
		   assumed is that there is no memory available. */
		return KPALMemoryError;
	}

#ifdef _DEBUG
	{
		u_int32 uPrevLength = 0;

		EnterCriticalSection(&memoryTest.mutex);

		if (*ppMem != NULL)
		{
			unordered_map<void *, u_int32>::const_iterator it = memoryTest.allocations.find(*ppMem);
			if (it == memoryTest.allocations.end())
			{
				/* The memory was not found... */

				memoryTest.uTotalErrors++;

				pal_LogMessage(memoryTest.logHandle, KLogLevelInfo, KLogComponentPAL, KLogTypeMemory, "Realloc: Corruption\t%p\t%p",
					pPtr, *ppMem);
			}
			else
			{
				uPrevLength = it->second;

				memoryTest.uCurrentAlloc -= it->second;

				memoryTest.allocations.erase(it);
			}
		}

		memoryTest.allocations.insert({ pPtr, memLength });

		memoryTest.uCurrentAlloc += memLength;
		memoryTest.uTotalOperations++;
		if (memoryTest.uMaxAlloc < memoryTest.uCurrentAlloc)
		{
			memoryTest.uMaxAlloc = memoryTest.uCurrentAlloc;
		}

		pal_LogMessage(memoryTest.logHandle, KLogLevelInfo, KLogComponentPAL, KLogTypeMemory, "Realloc:\t%p\t%p\t%u\t%u\t%u\t%u",
			pPtr, *ppMem, memLength, uPrevLength, memoryTest.uCurrentAlloc, memoryTest.uTotalOperations);

		LeaveCriticalSection(&memoryTest.mutex);
	}
#endif

	/** Set the output parameter to the reallocated memory. */
	*ppMem = pPtr;

	return KPALErrorNone;
}

u_int32 pal_MemorySet
(
	void *pMem,
	s_int32 value,
	u_int32 memLength
)
{
	/** This function checks all parameters passed to it. */
	/* The memory pointer must not be NULL. */
	if (pMem == NULL)
	{
		return KPALInvalidParameters;
	}

	/* The length to set must not be 0. */
	if (memLength == 0)
	{
		return KPALInvalidSize;
	}

	/** Set the memory to the requested value using the memset() function. */
	memset(pMem, value, memLength);

	return KPALErrorNone;
}

u_int32 pal_MemoryCopy
(
	void *pDest,
	u_int32 destLength,
	const void *pSource,
	u_int32 memLength
)
{
	/** This function checks all parameters passed to it. */
	/* The destination memory pointer must not be NULL. */
	if (pDest == NULL)
	{
		return KPALInvalidParameters;
	}

	/* The source memory pointer must not be NULL. */
	if (pSource == NULL)
	{
		return KPALInvalidParameters;
	}

	/* The length to copy must not be 0. */
	if (memLength == 0)
	{
		return KPALInvalidSize;
	}

	/* The length of destination must not be less than memLength. */
	if (destLength < memLength)
	{
		return KPALInvalidSize;
	}

	/** Copy the memory using memmove(), which ensures that if any region of
	    the source and destination memory overlaps, then the original
	    source bytes will be copied before being overwritten. */
	memmove(pDest, pSource, memLength);

	return KPALErrorNone;
}

s_int32 pal_MemoryCompare
(
	const void* pBuf1,
	const void* pBuf2,
	u_int32 memLength
)
{
	/** For the memory compare function, we have no way to inform the caller
		of problematic parameters. Therefore some assumptions are made in the
		behavior. In general, it is assumed that valid parameters will be
		provided. Therefore, the caller should check the parameters before
		passing them to this function. The other assumptions are described in
		the different return values generated. */

	if (memLength == 0)
	{
		/** If the length to compare is 0, we will return 0 (zero) because it
			can be implied that 0 length memory buffers match. At least that
			is the behavior agreed to and specified by the interface. */
		return 0;
	}

	if (pBuf1 == NULL)
	{
		if (pBuf2 == NULL)
		{
			/** If both memory buffers are NULL, they will be treated as
				equal. */
			return 0;
		}
		else
		{
			/** If pBuf1 is NULL and pBuf2 isn't, it is agreed behavior to
				specify that pBuf1 is less than pBuf2. */
			return -1;
		}
	}
	else
	{
		if (pBuf2 == NULL)
		{
			/** If pBuf2 is NULL and pBuf1 isn't, it is agreed behavior to
				specify that pBuf1 is greater than pBuf2. */
			return 1;
		}
		else
		{
			/** If all parameters are valid, we can use memcmp() to do the
				true memory comparison. The memcmp() function returns values
				following the interface definition so it can be returned
				directly. */
			return memcmp(pBuf1, pBuf2, memLength);
		}
	}
}

u_char *pal_MemorySearch
(
	const void *pSource,
	u_int32 memLength,
	const u_char *pSearchMem,
	u_int32 searchLength
)
{
	u_int32 uLen = 0;
	u_char *pPos = NULL;

	/** This function checks all parameters passed to it. */
	/* All pointers must not be NULL and all sizes must not be 0. */
	if ((pSource == NULL) ||
		(memLength == 0) ||
		(pSearchMem == NULL) ||
		(searchLength == 0) )
	{
		return NULL;
	}

	/** First search for the first character of the search string for a
		potential match. */
	pPos = (u_char*)memchr(pSource, *pSearchMem, memLength);
	if (pPos != NULL)
	{
		/* Loop for each character of the buffer. */
		while ((pPos != NULL) &&
			   ((pPos - (u_char*)pSource) < (s_int32)memLength))
		{
			/* Update the length that is left to search. */
			uLen = (u_int32)(pPos - (u_char*)pSource);

			/* Check length of the buffer that is being searched to make sure
			   it won't overflow. */
			if ((memLength - uLen) < searchLength)
			{
				return NULL;
			}

			/* Perform a memory comparison to see if there is a match. */
			if (memcmp(pPos, pSearchMem, searchLength) == 0)
			{
				return pPos;
			}
			else
			{
				/* There wasn't a match, find the next starting location. */
				pPos = (u_char*)memchr(pPos + 1, *pSearchMem, memLength - uLen - 1);
			}
		}
	}

	/** If no match is found, return NULL. */
	return NULL;
}

u_int32 pal_MemoryFree
(
	void **ppPtr
)
{
	/** This function checks all parameters passed to it. */
	/* The pointer to the memory pointer must not be NULL. */
	if (ppPtr == NULL)
	{
		return KPALInvalidParameters;
	}

	/* The memory pointer itself must not be NULL. */
	if (*ppPtr == NULL)
	{
		/* In this case, the memory is already NULL, and we want it to be
		   NULL, so we won't return any error. */
		return KPALErrorNone;
	}

#ifdef _DEBUG
	{
		EnterCriticalSection(&memoryTest.mutex);

		memoryTest.uTotalOperations++;

		unordered_map<void *, u_int32>::const_iterator it = memoryTest.allocations.find(*ppPtr);
		if (it == memoryTest.allocations.end())
		{
			/* The memory was not found... */

			memoryTest.uTotalErrors++;

			pal_LogMessage(memoryTest.logHandle, KLogLevelInfo, KLogComponentPAL, KLogTypeMemory, "Free: Memory Corruption");
		}
		else
		{
			pal_LogMessage(memoryTest.logHandle, KLogLevelInfo, KLogComponentPAL, KLogTypeMemory, "Free:\t%p\t%p\t%u\t%u\t%u\t%u",
				*ppPtr, *ppPtr, 0, it->second, memoryTest.uCurrentAlloc, memoryTest.uTotalOperations);

			memoryTest.uCurrentAlloc -= it->second;

			memoryTest.allocations.erase(it);
		}

		LeaveCriticalSection(&memoryTest.mutex);
	}
#endif

	/** Free the actual memory that was allocated using free(). */
	free(*ppPtr);

	/** Set the pointer to NULL in the caller's context as well. */
	*ppPtr = NULL;

	return KPALErrorNone;
}

#ifdef _DEBUG

void pal_memory_init
(
	void
)
{
	InitializeCriticalSection(&memoryTest.mutex);
}

void pal_memory_deinit
(
	void
)
{
	DeleteCriticalSection(&memoryTest.mutex);
}

void pal_memory_reset
(
	LOGHANDLE logHandle
)
{
	memoryTest.logHandle = logHandle;

	memoryTest.uCurrentAlloc = 0;
	memoryTest.uMaxAlloc = 0;
	memoryTest.uTotalOperations = 0;
	memoryTest.uTotalErrors = 0;

	memoryTest.allocations.clear();
}

unsigned int pal_memory_get_errorCount
(
	void
)
{
	return memoryTest.uTotalErrors;
}

unsigned int pal_memory_get_operationCount
(
	void
)
{
	return memoryTest.uTotalOperations;
}

unsigned int pal_memory_get_currentAllocation
(
	void
)
{
	return memoryTest.uCurrentAlloc;
}

void pal_memory_log_dump
(
	void
)
{
	EnterCriticalSection(&memoryTest.mutex);

	pal_LogMessage(memoryTest.logHandle, KLogLevelInfo, KLogComponentPAL, KLogTypeMemory, "Memory Report:");
	pal_LogMessage(memoryTest.logHandle, KLogLevelInfo, KLogComponentPAL, KLogTypeMemory, "\tOperations: %u, Error: %u, Max Allocated: %u, Currently Allocated: %u",
		memoryTest.uTotalOperations, memoryTest.uTotalErrors, memoryTest.uMaxAlloc, memoryTest.uCurrentAlloc);

	// @note Since this is unordered (!), the allocations aren't shown in allocated order!
	for (unordered_map<void *, u_int32>::const_iterator it = memoryTest.allocations.begin(); it != memoryTest.allocations.end(); ++it)
	{
		pal_LogMessage(memoryTest.logHandle, KLogLevelInfo, KLogComponentPAL, KLogTypeMemory, "Item:\t%p\t%u",
			it->first, it->second);
	}

	LeaveCriticalSection(&memoryTest.mutex);
}

#endif
