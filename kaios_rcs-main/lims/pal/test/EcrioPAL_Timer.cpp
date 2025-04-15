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
 * @file EcrioPAL_Timer.cpp
 * @brief This is a test version of Ecrio PAL's Timer Module.
 */

#include "EcrioPAL.h"

#include "pal_test.h"

#include <vector>

class TimerItem
{
public:
void *pRef;
TimerStartConfigStruct config;
};

class TimerClass
{
public:
LOGHANDLE logHandle;

unsigned int uStartCount;
unsigned int uStopCount;

unsigned int uNextRef;
std::vector<TimerItem> timers;
};

// Define the timer testing object globally.
TimerClass timerTest;

u_int32 pal_timerInit
(
	PALINSTANCE pal,
	LOGHANDLE logHandle,
	TIMERINSTANCE *instance
)
{
	(void)pal;

	if (instance == NULL)
	{
		return KPALInvalidParameters;
	}

	// We don't use this function for testing (see timer_reset).

	timerTest.logHandle = logHandle;

	*instance = &timerTest;

	return KPALErrorNone;
}

void pal_timerDeinit
(
	TIMERINSTANCE instance
)
{
	(void)instance;

	// We don't use this function for testing
}

u_int32 pal_TimerStart
(
	PALINSTANCE pal,
	TimerStartConfigStruct *pConfig,
	TIMERHANDLE *handle
)
{
	/* On Windows we require the PAL handle to be valid. */
	if (pal == NULL)
	{
		return KPALInvalidHandle;
	}

	/* The callback pointer must not be NULL. */
	if (pConfig == NULL || pConfig->timerCallback == NULL)
	{
		return KPALInvalidParameters;
	}

	// Only record entry to the function if parameter checks pass.
	timerTest.uStartCount++;

	TimerItem timerItem;

	// Cast the reference value as a pointer.
	timerItem.pRef = (void *)timerTest.uNextRef;
	timerTest.uNextRef++;

	// @todo We may want functions to check and verify the timers being created...

	timerItem.config.uInitialInterval = pConfig->uInitialInterval;
	timerItem.config.uPeriodicInterval = pConfig->uPeriodicInterval;
	timerItem.config.timerCallback = pConfig->timerCallback;
	timerItem.config.pCallbackData = pConfig->pCallbackData;
	timerItem.config.bEnableGlobalMutex = pConfig->bEnableGlobalMutex;

	timerTest.timers.push_back(timerItem);

	*handle = timerItem.pRef;

	return KPALErrorNone;
}

u_int32 pal_TimerStop
(
	TIMERHANDLE handle
)
{
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidHandle;
	}

	// Only record entry to the function if parameter checks pass.
	timerTest.uStopCount++;

	std::vector<TimerItem>::iterator it;
	bool bFound = false;

	// Find the socket that we should work with.
	for (it = timerTest.timers.begin(); it != timerTest.timers.end(); ++it)
	{
		if (it->pRef == handle)
		{
			// Erase this element from the sockets vector.
			timerTest.timers.erase(it);

			bFound = true;

			break;
		}
	}

	if (!bFound)
	{
		// @todo The created socket was not found!
	}

	return KPALErrorNone;
}

void pal_timer_reset
(
	void
)
{
	timerTest.uNextRef = 1;

	timerTest.uStartCount = 0;
	timerTest.uStopCount = 0;

	timerTest.timers.clear();
}

void pal_timer_reset_StartCount
(
	void
)
{
	timerTest.uStartCount = 0;
}

unsigned int pal_timer_get_StartCount
(
	void
)
{
	return timerTest.uStartCount;
}

void pal_timer_reset_StopCount
(
	void
)
{
	timerTest.uStopCount = 0;
}

unsigned int pal_timer_get_StopCount
(
	void
)
{
	return timerTest.uStopCount;
}

int pal_timer_get_timerCount
(
	void
)
{
	return timerTest.timers.size();
}

unsigned int pal_timer_get_timerInterval
(
	unsigned int timerIndex
)
{
	return timerTest.timers.at(timerIndex).config.uInitialInterval;
}
