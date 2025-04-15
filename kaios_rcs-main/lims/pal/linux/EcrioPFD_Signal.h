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
 * @file EcrioPFD_Signal.h
 * @brief This is the header file for the public interface of the Ecrio PFD's
 * Signal Module.
 *
 */

#ifndef __ECRIOPFD_SIGNAL_H__
#define __ECRIOPFD_SIGNAL_H__

/* Include common PFD definitions for building. */
// #include "EcrioPFD_Common.h"

/* Include the Ecrio PFD Logging Module definition as a direct dependency. */
// #include "EcrioPFD_Log.h"

/* Allow this module to be called directly from C++ compiled code. */
#ifdef __cplusplus
extern "C" {
#endif

/** @name Module Interface Version */
/*@{*/
#define ECRIOPFD_SIGNAL_VERSION_MAJOR		1	/**< Major version of the Signal Module interface. */
#define ECRIOPFD_SIGNAL_VERSION_MINOR		5	/**< Minor version of the Signal Module interface. */
/*@}*/

/** @name Module Handles
 * \n Handles are void pointers which will point to structures known
 * only internally to the Module. The Calling Component should only
 * reference the returned handles by these defined types. The handles
 * supported by this module include:
 */
/*@{*/

/**
 * The Signal handle refers to the overall Signal Module instance that is
 * created after calling DRVSignalInit().
 */
typedef void *SIGNALHANDLE;

/*@}*/

/** @name Callback Functions
 * \n Callback functions must be implemented by the Calling Component and are
 * invoked in response to various actions and events. The callback functions
 * supported by this module include:
 */
/*@{*/

/**
 * This callback is invoked whenever a signal is to be sent. The Calling
 * Component, the one that initialized this module, is responsible for
 * processing the signals that it gets. Typically, the Calling Component
 * will signal itself, so that processing can be de-coupled from execution of
 * other Components. The Calling Component must not call DRVSignalDeinit()
 * from this callback (otherwise, there will be a deadlock).
 *
 * @param[in] handle			Pointer to the Signal instance handle for which
 *								the callback is being invoked.
 * @param[in] signalId			The identifier of the specific signal which can
 *								be attributed to the callback.
 * @param[in] pCallbackData		Pointer to opaque data that the Calling
 *								Component will use. This pointer was provided
 *								by the Calling Component in the call to
 *								DRVSignalInit(). This pointer should not be
 *								freed by the Calling Component, because it will
 *								still be used for other expiration events.
 *
 * @return None.
 */
typedef void (*SignalCallback)
(
	SIGNALHANDLE handle,
	u_int32 signalId,
	void *pCallbackData
);

/*@}*/

/**
 * This function will get the implementation version of the module. The
 * major and minor versions should match that of the interface and the
 * revision represents possible implementation modifications.
 *
 * @return Pointer to a read only version structure.
 */
// const EcrioPFDVersionStruct *DRVSignalGetVersion
// (
// void
// );

/**
 * This function will initialize a Signal instance. This process typically
 * creates a thread to serve as a 'signal' for asynchronous events. A Signal
 * instance doesn't do anything other than notify the Calling Component when
 * "signaled".
 *
 * @param[in] pSignalFn
 * @param[in] logHandle			Handle to the Calling Component's Log Module
 *								instance. If it is not NULL, the Calling
 *								Component should guarantee that the instance
 *								will not be deinitialized before the Signal
 *								Module is deinitialized.
 * @param[in] bEnableGlobalMutex	If true, then the Global Mutex operations
 *								will be used such as locking before invoking
 *								callbacks. Otherwise, there will be no use of
 *								the Global Mutex.
 * @param[out] pError			Pointer to the location to place an error code
 *								indicating the result of the DRVSignalInit()
 *								call. This parameter must not be NULL. If it
 *								is, the function will return NULL.
 *
 * @return Pointer to the Signal instance handle or NULL if there was an error.
 */
SIGNALHANDLE DRVSignalInit
(
	PALINSTANCE pal,
	SignalCallback pSignalFn,
	LOGHANDLE logHandle,
	BoolEnum bEnableGlobalMutex,
	u_int32 *pError
);

/**
 * This function will deinitialize a Signal instance. If there are any signal
 * callbacks currently issues, this function won't return until they return.
 * When this function returns, no further signal callbacks can be issued.
 *
 * @param[in,out] handle		Pointer to a Signal instance handle.
 *								If the pointer is NULL or if the instance
 *								handle is NULL, then the function will return
 *								an error. If it is not NULL, an attempt will be
 *								made to clean up resources and memory used by
 *								the instance. The instance handle will be set
 *								to NULL before returning.
 *
 * @return The function will return the result of the function call.
 */
u_int32 DRVSignalDeinit
(
	SIGNALHANDLE *handle
);

/**
 * This function will invoke the SignalCallback function specified by the call
 * to the DRVSignalInit asynchronously (from a different thread). Generally,
 * the pCallbackData pointer is enough for a Calling Component to implement a
 * rich signaling system. The signalId is provided for extra convenience. The
 * Calling Component defines the ids it wants, in case it wants to support more
 * than one signal. There are no pre-defined signal events.
 *
 * There is no time guarantee when the signal callback will be invoked, it
 * depends on sheduling by the operating system.
 *
 * @param[in] handle			The handle.
 * @param[in] signalId			Identifier for the signal.
 * @param[in] pCallbackData		If non-null, information describing the callback.
 *
 * @return The function will return the result of the function call.
 */
u_int32 DRVSignalSend
(
	SIGNALHANDLE handle,
	u_int32 signalId,
	void *pCallbackData
);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __ECRIOPFD_SIGNAL_H__ */
