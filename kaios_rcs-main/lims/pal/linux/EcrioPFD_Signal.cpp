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
 * @file EcrioPFD_Signal.c
 * @brief This is the implementation of the Ecrio PFD's Signal Module.
 *
 */
/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

/* Include the Memory module PFD header file since it is a shared dependency. */
#include "EcrioPAL.h"

/* Include this module's PFD header file. */
#include "EcrioPFD_Signal.h"

/* Include the shared platform header file since we use shared functions. */
#include "EcrioPFD_Shared.h"

#define GLOBAL_MUTEX_IMPLEMENTATION_ENABLED
#ifdef GLOBAL_MUTEX_IMPLEMENTATION_ENABLED
/* Include the Mutex module PFD header file since it is used to define the
   global mutex interface. */
// #include "EcrioPFD_Mutex.h"
#endif

/* Logging defines and helper functions. */
// #ifdef ENABLE_LOG
// #define SIGNALLOGD(a,b,c, ...)		DRVLogMessageX((a),KLogLevelDebug,KLogComponentPFD,(b),(c),##__VA_ARGS__);
// #define SIGNALLOGE(a,b,c, ...)   DRVLogMessageX((a),KLogLevelError,KLogComponentPFD,(b),(c),##__VA_ARGS__);
// #define SIGNALLOGV(a,b,c, ...)		DRVLogMessageX((a),KLogLevelVerbose,KLogComponentPFD,(b),(c),##__VA_ARGS__);
// #define SIGNALLOGW(a,b,c, ...)   DRVLogMessageX((a),KLogLevelWarning,KLogComponentPFD,(b),(c),##__VA_ARGS__);
// #else /* ENABLE_LOG */
#define SIGNALLOGD(a, b, c, ...)
#define SIGNALLOGE(a, b, c, ...)
#define SIGNALLOGV(a, b, c, ...)
#define SIGNALLOGW(a, b, c, ...)
// #endif /* ENABLE_LOG */

/* Definitions. */
#define SIGNAL_VERSION_REVISION		1				/**< The revision of this implementation, ever incrementing. */
#define SIGNAL_SEND_EVENT			1				/**< Message used to indicate that a signal should be sent. */
#define SIGNAL_EXIT_EVENT			2				/**< Message used to indicate that the Signal Thread should exit. */

/** @enum EcrioPFDModuleStateEnum
 * Enumeration holding the module level initialization state.
 */
typedef enum
{
	ECRIO_PFD_MODULE_STATE_UNINITIALIZED = 0,	/**< The state is uninitialized as the Init() function has not completed. */
	ECRIO_PFD_MODULE_STATE_INITIALIZED,			/**< The state is initialized, since the Init() function was successful. */
	ECRIO_PFD_MODULE_STATE_DEINITIALIZING		/**< The state is deinitializing, since the Deinit() function has been called and is either executing or completed. */
} EcrioPFDModuleStateEnum;

/** @struct SignalStruct
 * This is the internal structure maintained by the Signal Module and represents
 * a Signal Module instance. Memory is allocated for it when the Init() function
 * is called and it is what the Signal Module Handle points to. All subsequent
 * interface functions receive this pointer as parameter input.
 */
typedef struct
{
	PALINSTANCE pal;

	MsgQStruct msgQStruct;				/**< A message queue parent structure. **/

	LOGHANDLE logHandle;			/**< If logging is enabled, provide a place to store the logHandle from the caller. */

	volatile bool bEnding;				/**< Flag to indicate if the Signal Module is being deinitialized. */

	SignalCallback callback;			/**< Pointer to the callback function to call when signaled. */

	bool bEnableGlobalMutex;			/**< Flag which indicates if the Global Mutex should be used. */

	pthread_t hThread;					/**< The handle of the Signal thread. */
	pthread_cond_t hThreadReady;		/**< The event flag used to signal to/from the primary thread. */

	volatile EcrioPFDModuleStateEnum initState;	/**< The overall module state. */
	pthread_mutex_t initCS;				/**< The initialization critical section, used to synchronize the initialization state of the module. */
	pthread_mutex_t operationalCS;		/**< The operational critical section, used to synchronize overall public function execution. */
} SignalStruct;

/**
 * This function serves as the Signal Thread for handling all signal events
 * for the Signal Module instance. The primary focus of this function is
 * to invoke the Calling Component's callback function which will actually
 * handle the signal.
 */
void *SignalThread
(
	void *lpParam
)
{
	SignalStruct *h = NULL;
	MsgQMessage msg;
	int res = 0;

	SignalCallback callback = NULL;
	void *pCallbackData = NULL;
	unsigned int signalId = 0;

	/* The parameter does not need to be checked for NULL because we require
	   the init function to properly set it. */
	h = (SignalStruct *)lpParam;

	SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - Entry, module 0x%08x\n", __FUNCTION__, __LINE__, lpParam);

	/* Starting the message loop for this thread, fire the event to let the
	   initialization code know the thread is ready. */
	SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - locking mutex\n", __FUNCTION__, __LINE__);
	pthread_mutex_lock(&(h->operationalCS));

	SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - signaling main thread\n", __FUNCTION__, __LINE__);
	pthread_cond_signal(&(h->hThreadReady));

	/* Prepare the enter the loop by locking the operational mutex for the
	   module. The pthread_cond_wait() function will unlock so other APIs
	   can get access. Note that the mutex is already locked. */

	/* Loop forever while we wait for signals indicating messages that are
	   available in the message queue. */
	for (;;)
	{
		SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - waiting for thread ready on mutex\n", __FUNCTION__, __LINE__);
		res = pthread_cond_wait(&(h->hThreadReady), &(h->operationalCS));
		if (res != 0)
		{
			SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - pthread_cond_wait() failure\n", __FUNCTION__, __LINE__);

			/* There is something suspicious with pthread_cond_wait, we will
			   just wait for another signal. */
			/* @todo Evaluate whether or not we should bail here. */
			continue;
		}

		/* Loop for all messages in the message queue. */
		while (GetMessage(&(h->msgQStruct), &msg) > 0)
		{
			SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - new message...\n", __FUNCTION__, __LINE__);

			/* Check the msg structure for type of message. */
			switch (msg.command)
			{
				/** The SIGNAL_SEND_EVENT message is handled which indicates
				    that an event has been set and should be communicated back
				    to the Calling Component. */
				case SIGNAL_SEND_EVENT:
				{
					SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - SIGNAL_SEND_EVENT\n", __FUNCTION__, __LINE__);

					/** Some notes about handling messages. First of all, the
					    module's operational mutex is locked already. It should
					    be unlocked before any callback is issued. Also, it is
					    best to make local copies of all variables needed to
					    issue the callback, including the callback itself, and
					    these should be copied early while in the callback. And
					    never do anything here if the callback is NULL! */
					if ((h->callback != NULL) &&
						(!h->bEnding))
					{
						callback = h->callback;
						pCallbackData = msg.pParam;
						signalId = msg.uParam;

						/** Leave the operational critical section. */
						SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - unlocking mutex\n", __FUNCTION__, __LINE__);
						pthread_mutex_unlock(&(h->operationalCS));

#ifdef GLOBAL_MUTEX_IMPLEMENTATION_ENABLED
						if (h->bEnableGlobalMutex)
						{
							/* Global mutex locked. */
							// DRVGlobalMutexLock();
							pal_MutexGlobalLock(h->pal);
						}
#endif

						/* Invoke the callback. */
						callback(h, signalId, pCallbackData);

#ifdef GLOBAL_MUTEX_IMPLEMENTATION_ENABLED
						if (h->bEnableGlobalMutex)
						{
							/* Global mutex unlocked. */
							// DRVGlobalMutexUnlock();
							pal_MutexGlobalUnlock(h->pal);
						}
#endif

						/** Enter the operational critical section. */
						SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - locking mutex\n", __FUNCTION__, __LINE__);
						pthread_mutex_lock(&(h->operationalCS));
					}
				}
				break;

				case SIGNAL_EXIT_EVENT:
				{
					SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - SIGNAL_EXIT_EVENT\n", __FUNCTION__, __LINE__);

					/** Leave the operational critical section. */
					SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - unlocking mutex\n", __FUNCTION__, __LINE__);
					pthread_mutex_unlock(&(h->operationalCS));

					SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - Exit, Got SIGNAL_EXIT_EVENT\n", __FUNCTION__, __LINE__);
					pthread_exit(NULL);
				}
				break;

				default:
				{
					SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - default handler\n", __FUNCTION__, __LINE__);
				}
				break;
			}
		}
	}

	/* Should never get here. */
	SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - Exit, Should never get here\n", __FUNCTION__, __LINE__);
	pthread_exit(NULL);
	return NULL;
}

/**
 * Implementation of the DRVSignalGetVersion() function. See the
 * EcrioPFD_Signal.h file for interface definitions.
 *
 * This function returns a read-only static const version structure. The
 * revision value should be changed every time this file is modified,
 * except for comment modifications and formatting, in which case the
 * revision does not need to be adjusted.
 */
// PFD_DLL_API const EcrioPFDVersionStruct* DRVSignalGetVersion
// (
// void
// )
// {
///* The version is read-only. */
// static const EcrioPFDVersionStruct ec_ro_version = {ECRIOPFD_SIGNAL_VERSION_MAJOR, ECRIOPFD_SIGNAL_VERSION_MINOR, SIGNAL_VERSION_REVISION};
// return &ec_ro_version;
// }

/**
 * Implementation of the DRVSignalInit() function. See the EcrioPFD_Signal.h
 * file for interface definitions.
 */
SIGNALHANDLE DRVSignalInit
(
	PALINSTANCE pal,
	SignalCallback pSignalFn,
	LOGHANDLE logHandle,
	BoolEnum bEnableGlobalMutex,
	u_int32 *pError
)
{
	SignalStruct *h = NULL;
	int res = 0;

	/** This function checks all parameters passed to it. */

	/* pError must be a valid location for error values. */
	if (pError == NULL)
	{
		/* If pError is NULL, return NULL, which should be detected as a
		   failure. */
		return NULL;
	}

	/* The signal callback must be a valid pointer. */
	if (pSignalFn == NULL)
	{
		*pError = KPALInvalidParameters;
	}

	/* By default, the error value returned will indicate success. */
	*pError = KPALErrorNone;

	/** Allocate memory for this instance of the Signal Module. */
	// returnValue = DRVMemoryAllocate(sizeof(SignalStruct), (void **)&h);
	// if ( (h == NULL) || (returnValue != KPFDErrorNone) )
	h = (SignalStruct *)malloc(sizeof(SignalStruct));
	if (h == NULL)
	{
		/* Memory for the Signal Module is not available. */
		*pError = KPALMemoryError;
		return NULL;
	}

	/* Initialize structure attributes. */
	h->initState = ECRIO_PFD_MODULE_STATE_UNINITIALIZED;

	if (bEnableGlobalMutex == Enum_TRUE)
	{
		h->bEnableGlobalMutex = true;
	}
	else
	{
		h->bEnableGlobalMutex = false;
	}

	h->logHandle = logHandle;

	SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - Entry\n", __FUNCTION__, __LINE__);

	MsgQInit(&(h->msgQStruct));
	h->callback = pSignalFn;
	h->bEnding = false;
	h->hThread = 0;

	h->pal = pal;

	/** Initialize the initialization critical section. */
	res = pthread_mutex_init(&(h->initCS), NULL);
	if (res != 0)
	{
		/* pthread_mutex_init() function failed. */
		SIGNALLOGE(h->logHandle, KLogTypeGeneral, "%s:%u - Unable to create initialization mutex, error - %d\n", __FUNCTION__, __LINE__, res);
		*pError = KPALInternalError;
		goto Error;
	}

	/** Initialize the operational critical section. */
	res = pthread_mutex_init(&(h->operationalCS), NULL);
	if (res != 0)
	{
		/* pthread_mutex_init() function failed. */
		SIGNALLOGE(h->logHandle, KLogTypeGeneral, "%s:%u - Unable to create operational mutex, error - %d\n", __FUNCTION__, __LINE__, res);
		*pError = KPALInternalError;
		goto Error;
	}

	/** Initialize the thready event flag. */
	res = pthread_cond_init(&(h->hThreadReady), 0);
	if (res != 0)
	{
		/* pthread_cond_init() function failed. */
		SIGNALLOGE(h->logHandle, KLogTypeGeneral, "%s:%u - Unable to create conditional variable, error - %d\n", __FUNCTION__, __LINE__, res);
		*pError = KPALInternalError;
		goto Error;
	}

	/* Before creating the primary thread, lock for synchronization with
	   the thread. */
	SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - locking mutex\n", __FUNCTION__, __LINE__);
	pthread_mutex_lock(&(h->operationalCS));

	/* Create the module's primary thread. */
	SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - creating thread\n", __FUNCTION__, __LINE__);
	res = pthread_create(&(h->hThread), NULL, SignalThread, (void *)h);
	if (res != 0)
	{
		/* pthread_create() function failed. */
		SIGNALLOGE(h->logHandle, KLogTypeGeneral, "%s:%u - Failed to create primary thread error - %d\n", __FUNCTION__, __LINE__, res);
		pthread_mutex_unlock(&(h->operationalCS));
		*pError = KPALInternalError;
		goto Error;
	}

	/* Wait until the thread is running */
	SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - waiting for ready on mutex\n", __FUNCTION__, __LINE__);
	pthread_cond_wait(&(h->hThreadReady), &(h->operationalCS));

	SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - unlocking mutex\n", __FUNCTION__, __LINE__);
	pthread_mutex_unlock(&(h->operationalCS));

	/** Set the state as initialized. */
	h->initState = ECRIO_PFD_MODULE_STATE_INITIALIZED;

	SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - Exit, Signal Handle - 0x%08x\n", __FUNCTION__, __LINE__, h);
	return (SIGNALHANDLE)h;

Error:

	if (h != NULL)
	{
		/* @todo Isn't there a good way to see if these have been initialized first? */
		pthread_cond_destroy(&(h->hThreadReady));
		pthread_mutex_destroy(&(h->operationalCS));
		pthread_mutex_destroy(&(h->initCS));

		SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - Exiting with error code %u\n", __FUNCTION__, __LINE__, *pError);

		/* Free the memory allocated for the Signal Module instance. */
		// returnValue = DRVMemoryFree((void **)&h);
		free(h);
		/* Ignoring the return value. */
	}

	return NULL;
}

/**
 * Implementation of the DRVSignalDeinit() function. See the EcrioPFD_Signal.h
 * file for interface definitions.
 */
u_int32 DRVSignalDeinit
(
	SIGNALHANDLE *handle
)
{
	SignalStruct *h = NULL;
	void *value_ptr;

	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidParameters;
	}

	/* For Deinit() functions, the Handle must NOT be NULL. */
	if (*handle == NULL)
	{
		return KPALInvalidHandle;
	}

	/* Set the handle pointer to the proper internal pointer. */
	h = (SignalStruct *)*handle;

	SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - Entry\n", __FUNCTION__, __LINE__);

	/* Enter the initialization critical section. */
	pthread_mutex_lock(&(h->initCS));
	if (h->initState != ECRIO_PFD_MODULE_STATE_INITIALIZED)
	{
		/* Not in the initialized state, there is no valid case where
		   Deinit() would/should be called. This is a corner case if
		   the Calling Component erroneously called Deinit() multiple
		   times. */
		SIGNALLOGE(h->logHandle, KLogTypeGeneral, "%s:%u - Exit, already de-initializing...\n", __FUNCTION__, __LINE__);
		pthread_mutex_unlock(&(h->initCS));
		return KPALNotInitialized;
	}

	/* Set the state to deinitializing to prevent any other interface
	   function from executing. */
	h->initState = ECRIO_PFD_MODULE_STATE_DEINITIALIZING;
	pthread_mutex_unlock(&(h->initCS));

	/** Enter the operational critical section. */
	SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - locking mutex\n", __FUNCTION__, __LINE__);
	pthread_mutex_lock(&(h->operationalCS));

	/** Set the instance flag that all further processing must be stopped. The
	    currently executing signal can not be stopped. Instead, the flag will
	    take effect when processing the next signal, if any. */
	h->bEnding = true;

	if (h->hThread != 0)
	{
		/** Post a message to the message queue. */
		MsgQAddMessage(&(h->msgQStruct), SIGNAL_EXIT_EVENT, 0, NULL);

		/** Signal the thread (the operational mutex is locked). */
		SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - signaling thread\n", __FUNCTION__, __LINE__);
		pthread_cond_signal(&(h->hThreadReady));
		/* @todo Handle return. */
	}

	/** Leave the operational critical section. */
	SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - unlocking mutex\n", __FUNCTION__, __LINE__);
	pthread_mutex_unlock(&(h->operationalCS));

	if (h->hThread != 0)
	{
#ifdef GLOBAL_MUTEX_IMPLEMENTATION_ENABLED
		if (h->bEnableGlobalMutex)
		{
			/* Global mutex unlocked. */
			// DRVGlobalMutexUnlock();
			pal_MutexGlobalUnlock(h->pal);
		}
#endif

		/** Wait for the primary thread to exit. */
		pthread_join(h->hThread, &value_ptr);

#ifdef GLOBAL_MUTEX_IMPLEMENTATION_ENABLED
		if (h->bEnableGlobalMutex)
		{
			/* Global mutex locked. */
			// DRVGlobalMutexLock();
			pal_MutexGlobalLock(h->pal);
		}
#endif

		h->hThread = 0;
	}

	/** Cleanup the event flag. */
	pthread_cond_destroy(&(h->hThreadReady));

	/** Delete the critical sections. */
	pthread_mutex_destroy(&(h->operationalCS));
	pthread_mutex_destroy(&(h->initCS));

	/** Cleanup the message queue. */
	MsgQDeinit(&(h->msgQStruct));

	SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - Exiting soon...\n", __FUNCTION__, __LINE__);
	h->logHandle = NULL;

	/** Free the memory allocated for the Timer Module instance. */
	// returnValue = DRVMemoryFree((void **)&h);
	free(h);
	/* Ignoring the return value. */

	/** Set the output parameter to NULL, to force NULL in the Calling
	    Component's context. */
	*handle = NULL;

	return KPALErrorNone;
}

/**
 * Implementation of the DRVSignalSend() function. See the
 * EcrioPFD_Signal.h file for interface definitions.
 */
u_int32 DRVSignalSend
(
	SIGNALHANDLE handle,
	u_int32 signalId,
	void *pCallbackData
)
{
	SignalStruct *h = NULL;

	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidHandle;
	}

	/* The other parameters only have meaning to the Calling Component. */

	/* Set the handle pointer to the proper internal pointer. */
	h = (SignalStruct *)handle;

	SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - Entry\n", __FUNCTION__, __LINE__);

	/* Enter the initialization critical section. */
	pthread_mutex_lock(&(h->initCS));
	if (h->initState != ECRIO_PFD_MODULE_STATE_INITIALIZED)
	{
		/* Not in the initialized state, there is no valid case where
		   a function API should be allowed to run if not yet
		   initialized. */
		SIGNALLOGE(h->logHandle, KLogTypeGeneral, "%s:%u - Exit, Not initialized\n", __FUNCTION__, __LINE__);
		pthread_mutex_unlock(&(h->initCS));
		return KPALNotInitialized;
	}

	pthread_mutex_unlock(&(h->initCS));

	/** Enter the operational critical section. */
	SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - locking mutex\n", __FUNCTION__, __LINE__);
	pthread_mutex_lock(&(h->operationalCS));

	/** Post a message to the message queue. */
	MsgQAddMessage(&(h->msgQStruct), SIGNAL_SEND_EVENT, signalId, pCallbackData);

	/** Signal the thread (the operational mutex is locked). */
	SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - signaling thread\n", __FUNCTION__, __LINE__);
	pthread_cond_signal(&(h->hThreadReady));
	/* @todo Handle return. */

	/** Leave the operational critical section. */
	SIGNALLOGD(h->logHandle, KLogTypeGeneral, "%s:%u - unlocking mutex\n", __FUNCTION__, __LINE__);
	pthread_mutex_unlock(&(h->operationalCS));

	return KPALErrorNone;
}
