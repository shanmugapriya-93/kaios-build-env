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
 * @file EcrioPAL_Timer.h
 * @brief This is the interface of the Ecrio PAL's Timer Module.
 *
 * The purpose of the Timer Module is to provide wrappers around common
 * Timer functionality provided by the platform.
 *
 * The Timer implementation must support N timers simultaneously. It is
 * preferred that a software solution to track N timers be used. Only one or a
 * very few hardware timers should be used, since the consume limited
 * resources.
 */

#ifndef __ECRIOPAL_TIMER_H__
#define __ECRIOPAL_TIMER_H__

/* Include common PAL definitions for building. */
#include "EcrioPAL_Common.h"

/* Include the Ecrio PAL Logging Module definition as a direct dependency. */
#include "EcrioPAL_Log.h"

/* Allow this module to be called directly from C++ compiled code. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * A definition for the Timer instance, for use internally by the PAL.
 */
typedef void *TIMERINSTANCE;

/**
 * A Timer Handle is assigned to every new timer when it is started.
 */
typedef void *TIMERHANDLE;

/** @name Callback Functions
 * \n Callback functions must be implemented by the Calling Component and are
 * invoked in response to various actions and events. The callback functions
 * supported by this module include:
 */
/*@{*/

/**
 * This function will be called whenever a timer expires. This function must
 * be implemented by the Calling Component and provided as a parameter to the
 * pal_TimerStart() function. For a repeating timer, the next cycle begins as
 * soon as the timer expires, not after the callback returns.
 *
 * @param[in] pCallbackData		Pointer to opaque data that the Calling
 *								Component will use. This pointer was provided
 *								by the Calling Component in the call to
 *								pal_TimerStart(). This is typically used to
 *								point to the instance of the Calling Component
 *								that is utilizing the Timer.
 * @param[in] handle			The handle of the timer that expired. The
 *								handle is the same value that was provided to
 *								the Calling Component when the pal_TimerStart()
 *								function returned.
 * @return None.
 */
typedef void (*TimerCallback)
(
	void *pCallbackData,
	TIMERHANDLE handle
);

/*@}*/

/** @struct TimerStartConfigStruct
 * This structure is used to provide configuration details to the
 * pal_TimerStart() function.
 */
typedef struct
{
	u_int32 uInitialInterval;		/**< The initial interval to set for the timer, in milliseconds. */
	u_int32 uPeriodicInterval;		/**< The interval to set for the timer if repeating, in milliseconds. If the timer should not repeat, set to 0. */
	TimerCallback timerCallback;	/**< The callback function to invoke when this timer expires. */
	void *pCallbackData;			/**< Pointer to opaque data that should be provided in the callback. */
	BoolEnum bEnableGlobalMutex;	/**< Set to Enum_TRUE if the Global Mutex should be locked before invoking the callback upon timer expiry. */
} TimerStartConfigStruct;

/**
 * This function starts a timer as described by the configuration structure.
 *
 * @param[in] pal
 * @param[in] pConfig			Pointer to a configuration structure. Fields
 *								from the structure must be copied since the
 *								pointer may expire after the function returns.
 * @param[out] handle			Pointer to the timer handle that will be
 *								maintained until the timer expires (if not
 *								repeating) or the pal_TimerStop() function is
 *								called.
 *
 * @return KPALErrorNone is successful, otherwise a specific error code.
 */
u_int32 pal_TimerStart
(
	PALINSTANCE pal,
	TimerStartConfigStruct *pConfig,
	TIMERHANDLE *handle
);

/**
 * This function stops the designated timer.
 *
 * @param[in] handle			Pointer to the timer handle that must be
 *								stopped. The handle will not be valid after
 *								the function returns.
 *
 * @return KPALErrorNone is successful, otherwise a specific error code.
 */
u_int32 pal_TimerStop
(
	TIMERHANDLE handle
);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __ECRIOPAL_TIMER_H__ */
