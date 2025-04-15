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
 * @file EcrioPAL.h
 * @brief This file references all public interfaces of the PAL.
 *
 * All headers are included in dependent order.
 */

#ifndef __ECRIOPAL_H__
#define __ECRIOPAL_H__

/** Include all data types for the system. */
#include "EcrioDataTypes.h"

/** Include the common header before all others. */
#include "EcrioPAL_Common.h"

/** Logging definitions are a shared dependency, define first. */
#include "EcrioPAL_Log.h"

/* Allow this module to be called directly from C++ compiled code. */
#ifdef __cplusplus
extern "C" {
#endif

typedef void *PALINSTANCE;

/**
 * This function can be used to initialize a PAL instance, for which resources
 * can be either statically or dynamically allocated.
 *
 * @param[in] logHandle			An optional log handle for logging.
 * @param[out] pal				A pointer to a PALINSTANCE or NULL.
 * @return KPALErrorNone is successful, otherwise a specific error code.
 */
u_int32 pal_Init
(
	LOGHANDLE logHandle,
	PALINSTANCE *pal
);

/**
 * This function is used to deinitialize a PAL instance and release resources
 * for it.
 *
 * @param[in] handle			The PAL instance handle. Can be NULL depending
 *								on the platform.
 * @return None.
 */
void pal_Deinit
(
	PALINSTANCE pal
);

#ifdef _DEBUG
// We will use the memory statistics mechanism for Windows debug builds.
void pal_memory_init
(
	void
);
void pal_memory_deinit
(
	void
);
void pal_memory_reset
(
	LOGHANDLE logHandle
);
void pal_memory_log_dump
(
	void
);
#endif

#ifdef __cplusplus
}
#endif

/* Include the remaining core module definitions. */
#include "EcrioPAL_Memory.h"
#include "EcrioPAL_Mutex.h"
#include "EcrioPAL_Socket.h"
#include "EcrioPAL_String.h"
#include "EcrioPAL_Timer.h"
#include "EcrioPAL_Utility.h"
#ifdef ENABLE_QCMAPI
#include "EcrioPAL_QCM.h"
#endif
#endif /* #ifndef __ECRIOPAL_H__ */
