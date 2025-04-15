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

/** Timer wheel is based on code from the following copyright. */

/*------------------------------------------------------------------
* stw_timer.c -- Single Timer Wheel Timer
*
* February 2005, Bo Berry
*
* Copyright (c) 2002-2009 by Cisco Systems, Inc.
* All rights reserved.
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom
* the Software is furnished to do so, subject to the following
* conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*------------------------------------------------------------------
*/

/**
 * @file EcrioPAL_Timer.cpp
 * @brief This is the implementation of the Ecrio PAL's Timer Module.
 *
 */

/* Standard Linux includes. */
#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <list>
#include <condition_variable>

#include <signal.h>
#include <time.h>

using namespace std;

#include "EcrioPAL.h"
#include "EcrioPAL_Internal.h"

// In our example, we have 5 spokes (our Timer Wheel is 1 second long).
#define STW_SPOKES			(5)

// In our example, each timer tick is 200ms.
#define STW_GRANULARITY		200

// Define the Timer List size.
#define STW_TIMERS			(64)

/** Logging defines and helper functions */
#ifdef ENABLE_LOG
#define TIMERLOGD(a, b, c, ...)			pal_LogMessage((a), KLogLevelDebug, KLogComponentPAL, (b), (c),##__VA_ARGS__);
#define TIMERLOGI(a, b, c, ...)			pal_LogMessage((a), KLogLevelInfo, KLogComponentPAL, (b), (c),##__VA_ARGS__);
#define TIMERLOGV(a, b, c, ...)			pal_LogMessage((a), KLogLevelVerbose, KLogComponentPAL, (b), (c),##__VA_ARGS__);
#define TIMERLOGW(a, b, c, ...)			pal_LogMessage((a), KLogLevelWarning, KLogComponentPAL, (b), (c),##__VA_ARGS__);
#define TIMERLOGE(a, b, c, ...)			pal_LogMessage((a), KLogLevelError, KLogComponentPAL, (b), (c),##__VA_ARGS__);

#define TIMERLOGDUMP(a, b, c, ...)		pal_LogMessageDump((a), KLogLevelInfo, KLogComponentPAL, (b), (c),##__VA_ARGS__);
#else /* ENABLE_LOG */
#define TIMERLOGD(a, b, c, ...)
#define TIMERLOGI(a, b, c, ...)
#define TIMERLOGV(a, b, c, ...)
#define TIMERLOGW(a, b, c, ...)
#define TIMERLOGE(a, b, c, ...)

#define TIMERLOGDUMP(a, b, c, ...)
#endif /* ENABLE_LOG */

/* Definitions. */
typedef void* TIMER_SIGNAL_HANDLE;

#define TIMER_SIGNAL_EXIT		100
#define TIMER_SIGNAL_TICK		101
#define TIMER_SIGNAL_EXPIRE	102

/* Forward declarations. */
/* messageQ message Structure */
typedef struct
{
	unsigned int uCommand;
	unsigned int uParam;
	void* pParam;
} timer_messageStruct;

/* messageQ structure */
typedef struct
{
	mutex m;
	list <timer_messageStruct> messageQ;
} timer_messageQStruct;

typedef unsigned int(*timer_signalCallback)
(
	TIMER_SIGNAL_HANDLE handle,
	unsigned int command,
	unsigned int uParam,
	void* pParam
);

typedef struct
{
	mutex m;
	condition_variable cond;
	thread hThread;
	timer_messageQStruct messageQStruct;
	timer_signalCallback callback;
	unsigned int isEnding;
	LOGHANDLE logHandle;
} timer_signalStruct;

/*
* This file provides the public definitions for external users of the
* Timer Wheel Timer facility.
*
* The Single Timer Wheel Timer facility is optimized to support embedded
* timer structures, ie where the timer structure is integrated into the
* structure it is associated with.
*
* Definitions
*
* Spoke - A queue of timers set to expire. A timer wheel consists of
*  multiple spokes.
*
* Granularity - The amount of time between the processing of each
*  spoke.   Granularity is measured in time units between ticks.
*
* Rotation - one complete turn around the wheel
*
* Timer Duration given 32 bits
*    Time per Tick      Duration
*   ------------------------------------
*        10ms          497.1 days
*        20ms          994.2
*        50ms         2485.5
*
*   milliseconds per day [1000 * 60 * 60 * 24]
*
*/

/*
* stw_links
*  Definition of the pointers used to link a timer into
*  a spoke.  Double-linked list for efficiency.
*/
typedef struct stw_links_t_
{
	struct stw_links_t_ *stw_next;
	struct stw_links_t_ *stw_prev;
} stw_links_t;

typedef struct
{
	u_int32 spoke_index;			/* mod index around wheel */
	u_int32 ticks;					/* absolute ticks */
	/*
	* few book keeping parameters to help engineer the wheel
	*/
	u_int32 timer_active;
	u_int32 timer_cancelled;
	u_int32 timer_expired;
	u_int32 timer_starts;
	stw_links_t spokes[STW_SPOKES];
} stw_t;

/*
* stw_tmr_t
*  Definition of a timer element.
*  This can be malloc'ed or embedded into an existing
*  application structure.
*/
typedef struct
{
	stw_links_t links;
	u_int32 rotation_count;
	u_int32 delay;				/* initial delay       */
	u_int32 periodic_delay;		/* auto-restart if > 0 */
	TimerCallback func_ptr;
	void *parm;
	BoolEnum bEnableGlobalMutex;
	TIMERINSTANCE instance;
} stw_tmr_t;

/** @struct TimerInstanceStruct
* This is the internal structure maintained by the Timer Module and represents
* a Timer Module instance. Memory is allocated for it when the Init() function
* is called and it is what the Timer Module Handle points to. All subsequent
* interface functions receive this pointer as parameter input.
*/
typedef struct
{
	// The Timer Wheel.
	stw_t stw;

	// The Timer List.
	stw_tmr_t timer_list[STW_TIMERS];

	// Pointer to the Free List.
	stw_tmr_t *pFree;

	PALINSTANCE pal;
	LOGHANDLE logHandle;

	// @todo Add platform subsystem specific variables here.
	BoolEnum started;		/**< Flag to indicate if the timer is started, to prevent new timers from being started until the current timer is stopped. */
	timer_t timerId;
	TIMER_SIGNAL_HANDLE hTimer;
	TIMER_SIGNAL_HANDLE hTimerCallback;
	mutex m;

	bool init;					/** init flag. */
} TimerInstanceStruct;

/* MESSAGE QUEUE IMPLEMENTATION - START */
void timer_message_init
(
	timer_messageQStruct *messageQStruct
)
{
	(void)messageQStruct;
	//TBD
}

void timer_message_deinit
(
	timer_messageQStruct *messageQStruct
)
{
	(void)messageQStruct;
	//TBD
}

int timer_message_add
(
	timer_messageQStruct *messageQStruct,
	unsigned int uCommand,
	unsigned int uParam,
	void* pParam
)
{
	lock_guard<mutex> lock(messageQStruct->m);
	timer_messageStruct messageStruct = {};
	messageStruct.uCommand = uCommand;
	messageStruct.uParam = uParam;
	messageStruct.pParam = pParam;
	messageQStruct->messageQ.push_back(messageStruct);
	return 0;
}

int timer_message_get
(
	timer_messageQStruct *messageQStruct,
	timer_messageStruct *pStruct
)
{
	unique_lock<mutex> lock(messageQStruct->m);
	if (messageQStruct->messageQ.size() == 0)
	{
		lock.unlock();
		return -1;
	}
	timer_messageStruct messageStruct = {};
	messageStruct = messageQStruct->messageQ.front();
	pStruct->uCommand = messageStruct.uCommand;
	pStruct->uParam = messageStruct.uParam;
	pStruct->pParam = messageStruct.pParam;
	messageQStruct->messageQ.pop_front();
	lock.unlock();
	return 0;
}
/* MESSAGE QUEUE IMPLEMENTATION - END */

/* SIGNAL IMPLEMENTATION - START */

static void timer_signal_worker
(
	void *pStruct
)
{
	timer_signalStruct *h = (timer_signalStruct*)pStruct;
	while (1)
	{
		unique_lock<mutex> lock(h->m);
		h->cond.wait(lock);
		lock.unlock();
		timer_messageStruct messageStruct = {};
		while (timer_message_get(&(h->messageQStruct), &messageStruct) == 0)
		{
			switch (messageStruct.uCommand)
			{
			case TIMER_SIGNAL_EXIT:
			{
				return;
			}
			default:
			{
				if ((h->callback != NULL) && (!h->isEnding))
				{
					h->callback(h, messageStruct.uCommand, messageStruct.uParam, messageStruct.pParam);
				}
			}
			break;
			}
		}
	}
}

TIMER_SIGNAL_HANDLE timer_signal_init
(
	timer_signalCallback pSignalFn,
	LOGHANDLE logHandle,
	u_int32 *pError
)
{
	timer_signalStruct *h = NULL;
	int returnValue = 0;

	if (pError == NULL)
	{
		return NULL;
	}

	if (pSignalFn == NULL)
	{
		*pError = 1;
		return NULL;
	}

	/* By default, the error value returned will indicate success. */
	*pError = returnValue;

	h = new (timer_signalStruct);

	h->logHandle = logHandle;
	h->isEnding = 0;

	h->callback = pSignalFn;

	timer_message_init(&(h->messageQStruct));

	h->hThread = thread(timer_signal_worker, (void *)h);
	return h;
}

int timer_signal_deinit
(
	TIMER_SIGNAL_HANDLE *handle
)
{
	timer_signalStruct *h = NULL;
	if (handle == NULL)
	{
		return -1;
	}
	if (*handle == NULL)
	{
		return -1;
	}
	h = (timer_signalStruct*)*handle;
	h->isEnding = 1;
	timer_message_add(&(h->messageQStruct), TIMER_SIGNAL_EXIT, 0, NULL);
	unique_lock<mutex> lock(h->m);
	h->cond.notify_one();
	lock.unlock();
	h->hThread.join();
	timer_message_deinit(&(h->messageQStruct));
	delete h;

	return 0;
}

int timer_signal_send
(
	TIMER_SIGNAL_HANDLE handle,
	unsigned int command,
	unsigned int uParam,
	void* pParam
)
{
	timer_signalStruct *h = NULL;
	if (handle == NULL)
	{
		return -1;
	}
	h = (timer_signalStruct*)handle;
	timer_message_add(&(h->messageQStruct), command, uParam, pParam);
	unique_lock<mutex> lock(h->m);
	h->cond.notify_one();
	lock.unlock();
	return 0;
}
/* SIGNAL QUEUE IMPLEMENTATION - END */

/*
* NAME
*    tmr_enqueue
*
* SYNOPSIS
*    #include "stw_timer.h"
*    static void
*    tmr_enqueue(stw_t *stw, stw_tmr_t *tmr, uint32_t delay)
*
* DESCRIPTION
*    Enqueues the timer to the proper spoke per delay.
*
* INPUT PARAMETERS
*    *stw - pointer to the timer wheel that the timer
*          will run on
*
*    *tmr - pointer to the timer element
*
*    delay - delay in milliseconds
*
* OUTPUT PARAMETERS
*    none
*
* RETURN VALUE
*    none
*/
static void tmr_enqueue
(
	TimerInstanceStruct *m,
	stw_tmr_t *tmr,
	u_int32 delay
)
{
	stw_links_t *prev, *spoke;

	u_int32 cursor;
	u_int32 ticks;
	u_int32 td;

	if (delay < STW_GRANULARITY)
	{
		// must delay at least one tick, can not delay in past...
		ticks = 1;
	}
	else
	{
		// compute number ticks required to expire the duration
		ticks = (delay / STW_GRANULARITY);
	}

	// tick displacement from current cursor
	td = ((ticks) % STW_SPOKES);

	// times around the wheel required to expire duration
	// @note Removing 1 since the timeout was coming a rotation too late. Is it correct in all cases?
	tmr->rotation_count = ((ticks - 1) / (STW_SPOKES));

	// calculate cursor to place the timer
	cursor = ((m->stw.spoke_index + td) % STW_SPOKES);

	spoke = &m->stw.spokes[cursor];

	/*
	* We have a timer and now we have a spoke.  All that is left is to
	* link the timer to the spoke's list of timers.  With a doubly linked
	* list, there is no need to check for an empty list.  We simply link
	* it to the end of the list.  This is the same price as putting it
	* on the front of the list but feels more 'right'.
	*/
	prev = spoke->stw_prev;
	tmr->links.stw_next = spoke;		/* append to end of spoke  */
	tmr->links.stw_prev = prev;

	prev->stw_next = (stw_links_t *)tmr;
	spoke->stw_prev = (stw_links_t *)tmr;
}

static unsigned int timerCallbackHandler
(
	TIMER_SIGNAL_HANDLE handle,
	unsigned int command,
	unsigned int uParam,
	void *pParam
)
{
	stw_tmr_t *tmr = (stw_tmr_t *)pParam;
	TimerInstanceStruct *m = (TimerInstanceStruct *)tmr->instance;
	TimerCallback user_call_back;
	BoolEnum bEnableGlobalMutex = tmr->bEnableGlobalMutex;

	if(command == TIMER_SIGNAL_EXPIRE)
	{
		TIMERLOGD(m->logHandle, KLogTypeGeneral, "%s:%u", __FUNCTION__, __LINE__);
		if (tmr->func_ptr != NULL)
		{
			user_call_back = (TimerCallback)tmr->func_ptr;						
		}

		TIMERLOGD(m->logHandle, KLogTypeGeneral, "%s:%u", __FUNCTION__, __LINE__);
		if (bEnableGlobalMutex)
		{
			pal_MutexGlobalLock(m->pal);
		}
		TIMERLOGD(m->logHandle, KLogTypeGeneral, "%s:%u tmr=0x%x", __FUNCTION__, __LINE__, tmr);

		// Invoke the user expiration handler to do the actual work.
		(*user_call_back)(tmr->parm, (TIMERHANDLE)tmr);

		TIMERLOGD(m->logHandle, KLogTypeGeneral, "%s:%u", __FUNCTION__, __LINE__);

		if (bEnableGlobalMutex)
		{
			pal_MutexGlobalUnlock(m->pal);
		}
		TIMERLOGD(m->logHandle, KLogTypeGeneral, "%s:%u", __FUNCTION__, __LINE__);
	}

	return 0;
}

static unsigned int timerHandler
(
	TIMER_SIGNAL_HANDLE handle,
	unsigned int command,
	unsigned int uParam,
	void *pParam
)
{
	TimerInstanceStruct *m = (TimerInstanceStruct *)pParam;

	/* Enter the operational critical section. */
	unique_lock<mutex> lock(m->m);

	if(command == TIMER_SIGNAL_TICK)
	{
		/* Check the flag to make sure we should be handling Timer
		   expirations. */
		if (m->timerId != 0)
		{
			stw_links_t *spoke, *next, *prev;
			stw_tmr_t *tmr;
			TimerCallback user_call_back;

			// keep track of rolling the wheel
			m->stw.ticks++;

			// advance the index to the next spoke
			m->stw.spoke_index = (m->stw.spoke_index + 1) % STW_SPOKES;

			/*
			* Process the spoke, removing timers that have expired.
			* If the timer rotation count is positive
			* decrement and catch the timer on the next wheel revolution.
			*/
			spoke = &m->stw.spokes[m->stw.spoke_index];
			tmr = (stw_tmr_t *)spoke->stw_prev;

			while (tmr && (stw_links_t *)tmr != spoke)
			{
				BoolEnum bEnableGlobalMutex = tmr->bEnableGlobalMutex;

				next = (stw_links_t *)tmr->links.stw_next;
				prev = (stw_links_t *)tmr->links.stw_prev;

				/*
				* if the timer is a long one and requires one or more rotations
				* decrement rotation count and leave for next turn.
				*/
				if (tmr->rotation_count != 0)
				{
					tmr->rotation_count--;
				}
				else
				{
					if (tmr->func_ptr == NULL)
					{
						tmr = (stw_tmr_t *)prev;
						TIMERLOGD(m->logHandle, KLogTypeGeneral, "%s:%u", __FUNCTION__, __LINE__);
						continue;
					}

					prev->stw_next = next;
					next->stw_prev = prev;

					// book keeping
					m->stw.timer_expired++;

					if (tmr->func_ptr != NULL)
					{
						user_call_back = (TimerCallback)tmr->func_ptr;						
					}

					// Add the timer to the Free List.
					tmr->links.stw_next = &m->pFree->links;
					tmr->links.stw_prev = NULL;
					m->pFree->links.stw_prev = &tmr->links;
					m->pFree = tmr;

					// stats bookkeeping
					m->stw.timer_active--;
//					tmr->func_ptr = NULL;

					TIMERLOGD(m->logHandle, KLogTypeGeneral, "%s:%u tmr=0x%x", __FUNCTION__, __LINE__, tmr);
					timer_signal_send(m->hTimerCallback, TIMER_SIGNAL_EXPIRE, 0, (void *)tmr);	
					TIMERLOGD(m->logHandle, KLogTypeGeneral, "%s:%u tmr=0x%x", __FUNCTION__, __LINE__, tmr);
#if 0
					/* Leave the operational critical section. */
					lock.unlock();

					if (bEnableGlobalMutex)
					{
						pal_MutexGlobalLock(m->pal);
					}

					// Invoke the user expiration handler to do the actual work.
					(*user_call_back)(tmr->parm, (TIMERHANDLE)tmr);

					if (bEnableGlobalMutex)
					{
						pal_MutexGlobalUnlock(m->pal);
					}

					/* Enter the operational critical section. */
					lock.lock();
#endif

					// automatically restart the timer if periodic_delay > 0
					if (tmr->periodic_delay > 0)
					{
						tmr->links.stw_next = NULL;
						tmr->links.stw_prev = NULL;

						tmr_enqueue(m, tmr, tmr->periodic_delay);
					}
				}

				tmr = (stw_tmr_t *)prev;
			}
		}
	}
	else
	{
		TIMERLOGI(m->logHandle, KLogTypeGeneral, "%s:%u invalid command=%u", __FUNCTION__, __LINE__, command);
	}

	/* Leave the operational critical section. */
	lock.unlock();
	return 0;
}

static void notify_function(union sigval sv)
{
        TimerInstanceStruct *m = (TimerInstanceStruct *)sv.sival_ptr;

	timer_signal_send(m->hTimer, TIMER_SIGNAL_TICK, 0, (void *)m);	
}

/**
 * Implementation of the pal_TimerInit() function. See the EcrioPAL_Timer.h file
 * for interface definitions.
 */
u_int32 pal_timerInit
(
	PALINSTANCE pal,
	LOGHANDLE logHandle,
	TIMERINSTANCE *instance
)
{
	TimerInstanceStruct *m = NULL;
	unsigned int i;
	stw_links_t *spoke;
	u_int32 error = KPALErrorNone;

	TIMERLOGI(logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (instance == NULL)
	{
		return KPALInvalidParameters;
	}

	*instance = NULL;

	pal_MemoryAllocate(sizeof(TimerInstanceStruct), (void **)&m);
	if (m == NULL)
	{
		return KPALMemoryError;
	}

	m->pal = pal;
	m->logHandle = logHandle;

	// Initialize the Timer List, set it up to be the Free List.
	for (i = 0; i < STW_TIMERS; ++i)
	{
		if (i == 0)
		{
			m->timer_list[i].links.stw_next = &m->timer_list[i + 1].links;
			m->timer_list[i].links.stw_prev = NULL;
		}
		else if (i == (STW_TIMERS - 1))
		{
			m->timer_list[i].links.stw_next = NULL;
			m->timer_list[i].links.stw_prev = &m->timer_list[i - 1].links;
		}
		else
		{
			m->timer_list[i].links.stw_next = &m->timer_list[i + 1].links;
			m->timer_list[i].links.stw_prev = &m->timer_list[i - 1].links;
		}
	}

	// The Free List is initially the entire Timer List.
	m->pFree = &m->timer_list[0];

	// Initialize the internal tick count at zero
	m->stw.ticks = 0;
	m->stw.spoke_index = 0;

	// timer stats to tune wheel
	m->stw.timer_active = 0;
	m->stw.timer_cancelled = 0;
	m->stw.timer_expired = 0;
	m->stw.timer_starts = 0;

	// Set all spokes to empty
	spoke = &m->stw.spokes[0];

	for (i = 0; i < STW_SPOKES; i++)
	{
		spoke->stw_next = spoke;	/* empty spoke points to itself */
		spoke->stw_prev = spoke;
		spoke++;
	}

	// @note We don't need a name! Get rid of it.
	m->started = Enum_FALSE;
	m->timerId = 0;

	m->hTimerCallback = timer_signal_init(timerCallbackHandler, m->logHandle, &error);
	if (m->hTimerCallback == NULL)
	{
		pal_MemoryFree((void**) &m);
		return KPALMemoryError;
	}

	m->hTimer = timer_signal_init(timerHandler, m->logHandle, &error);
	if (m->hTimer == NULL)
	{
		timer_signal_deinit(&(m->hTimerCallback));
		pal_MemoryFree((void**) &m);
		return KPALMemoryError;
	}

	*instance = (TIMERINSTANCE)m;

	TIMERLOGI(logHandle, KLogTypeFuncExit, "%s:%u error=%u", __FUNCTION__, __LINE__, error);

	return error;
}

/**
 * Implementation of the pal_TimerDeinit() function. See the EcrioPAL_Timer.h
 * file for interface definitions.
 */
void pal_timerDeinit
(
	TIMERINSTANCE instance
)
{
	TimerInstanceStruct *m = (TimerInstanceStruct *)instance;
	LOGHANDLE logHandle;

	/* Conserve the log handle. */
	logHandle = m->logHandle;

	TIMERLOGI(logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

#if 0 // Emscripten asm.js does not support system timer.
	unique_lock<mutex> lock(m->m);
	if (m->timerId != 0)
	{
		timer_delete(m->timerId);
		m->timerId = 0;
	}
	lock.unlock();
#endif

	timer_signal_deinit(&(m->hTimer));

	timer_signal_deinit(&(m->hTimerCallback));

	pal_MemoryFree((void**) &m);

	TIMERLOGI(logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
}

u_int32 pal_TimerStart
(
	PALINSTANCE pal,
	TimerStartConfigStruct *pConfig,
	TIMERHANDLE *handle
)
{
	TimerInstanceStruct *m = NULL;
	stw_tmr_t *pTmr;

	/* Preset the handle to NULL so it isn't used if any error. */
	*handle = NULL;

	/* On Linux we require the PAL handle to be valid. */
	if (pal == NULL)
	{
		return KPALInvalidHandle;
	}

	/* The callback pointer must not be NULL. */
	if (pConfig == NULL || pConfig->timerCallback == NULL)
	{
		return KPALInvalidParameters;
	}

	m = (TimerInstanceStruct *)((EcrioPALStruct *)pal)->timer;

	/* The pointer to the Module Handle must not be NULL. */
	if (m == NULL)
	{
		return KPALInvalidHandle;
	}

	TIMERLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	TIMERLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u config=%p, timerCallback=%p, pCallbackData=%p",
		__FUNCTION__, __LINE__, pConfig, pConfig->timerCallback, pConfig->pCallbackData);
	TIMERLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u uInitialInterval=%u, uPeriodicInterval=%u, bEnableGlobalMutex=%u",
		__FUNCTION__, __LINE__, pConfig->uInitialInterval, pConfig->uPeriodicInterval, pConfig->bEnableGlobalMutex);

	/* Enter the operational critical section. */
	unique_lock<mutex> lock(m->m);
	/** If the timer is already started, return an error because only one timer
	    can be operational at a time. */
#if 0 // Emscripten asm.js does not support system timer.
	if (!m->started)
	{
		struct sigevent sigEvt;
		struct itimerspec trigger;

		m->started = Enum_TRUE;

		pal_MemorySet(&sigEvt, 0x00, sizeof(sigEvt));

		sigEvt.sigev_notify = SIGEV_THREAD;
		sigEvt.sigev_notify_function = &notify_function;
		sigEvt.sigev_value.sival_ptr = (void *)m;

		timer_create(CLOCK_REALTIME, &sigEvt, &m->timerId);

		trigger.it_value.tv_sec = 0;
		trigger.it_value.tv_nsec = STW_GRANULARITY * 1000000;
		trigger.it_interval.tv_sec = 0;
		trigger.it_interval.tv_nsec = STW_GRANULARITY * 1000000;

		timer_settime(m->timerId, 0, &trigger, NULL);
	}
#endif
	pTmr = m->pFree;
	if (pTmr == NULL || pTmr->links.stw_next == NULL)
	{
		TIMERLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tNo free timer slot!", __FUNCTION__, __LINE__);
		/* Leave the operational critical section. */
		lock.unlock();
		return KPALResourcesInUse;
	}

	m->pFree = (stw_tmr_t *)pTmr->links.stw_next;

	pTmr->links.stw_next = NULL;
	pTmr->links.stw_prev = NULL;

	// set user call_back and parameter
	pTmr->func_ptr = pConfig->timerCallback;
	pTmr->parm = pConfig->pCallbackData;
	pTmr->delay = pConfig->uInitialInterval;

	if (pConfig->uPeriodicInterval == 1)
	{
		pTmr->periodic_delay = pConfig->uInitialInterval;
	}
	else
	{
		pTmr->periodic_delay = pConfig->uPeriodicInterval;
	}

	if (pConfig->bEnableGlobalMutex == Enum_TRUE)
	{
		pTmr->bEnableGlobalMutex = Enum_TRUE;
	}
	else
	{
		pTmr->bEnableGlobalMutex = Enum_FALSE;
	}

	pTmr->instance = (TIMERINSTANCE)m;

	tmr_enqueue(m, pTmr, pTmr->delay);

	m->stw.timer_starts++;
	m->stw.timer_active++;

	TIMERLOGD(m->logHandle, KLogTypeGeneral, "%s:%u Active timers=%u", __FUNCTION__, __LINE__, m->stw.timer_active);
	TIMERLOGD(m->logHandle, KLogTypeGeneral, "%s:%u Expired timers=%u", __FUNCTION__, __LINE__, m->stw.timer_expired);
	TIMERLOGD(m->logHandle, KLogTypeGeneral, "%s:%u Started timers=%u", __FUNCTION__, __LINE__, m->stw.timer_starts);
	TIMERLOGD(m->logHandle, KLogTypeGeneral, "%s:%u Cancelled timers=%u", __FUNCTION__, __LINE__, m->stw.timer_cancelled);

	*handle = (TIMERHANDLE)pTmr;

	/* Leave the operational critical section. */
	lock.unlock();
	TIMERLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return KPALErrorNone;
}

u_int32 pal_TimerStop
(
	TIMERHANDLE handle
)
{
	TimerInstanceStruct *m = NULL;
	stw_tmr_t *tmr;
	stw_links_t *next, *prev;

	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidHandle;
	}

	tmr = (stw_tmr_t *)handle;

	/* Set the module handle pointer to the proper internal pointer. */
	m = (TimerInstanceStruct *)tmr->instance;

	TIMERLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/* Enter the operational critical section. */
	unique_lock<mutex> lock(m->m);

	prev = tmr->links.stw_prev;
	if (tmr->func_ptr == NULL || prev == NULL)
	{
		/* The timer has already expired. Return normally. */
		TIMERLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
		/* Leave the operational critical section. */
		lock.unlock();
		return KPALErrorNone;
	}

	next = tmr->links.stw_next;
	next->stw_prev = prev;
	prev->stw_next = next;

	// Add the timer to the Free List.
	tmr->links.stw_next = &m->pFree->links;
	tmr->links.stw_prev = NULL;
	m->pFree->links.stw_prev = &tmr->links;
	m->pFree = tmr;

	// stats bookkeeping
	m->stw.timer_active--;
	m->stw.timer_cancelled++;

	tmr->func_ptr = NULL;

	/* Leave the operational critical section. */
	lock.unlock();

	TIMERLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return KPALErrorNone;
}
