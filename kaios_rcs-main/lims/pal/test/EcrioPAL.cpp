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
 * @file EcrioPAL.c
 * @brief This is a test version of Ecrio PAL's internal control for Windows.
 */

// @note This is currently the same as the reference implementation, no test hooks are needed yet.

/* Standard Windows includes. */
#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <stdlib.h>

#include "EcrioPAL.h"
#include "EcrioPAL_Internal.h"

/**
 * This function is used to initialize the Windows PAL. All PAL modules must be
 * successfully initialized or else the entire initialization will fail. On the
 * Windows platform, the PAL instance is dynamically allocated.
 */
u_int32 pal_Init
(
	LOGHANDLE logHandle,
	PALINSTANCE *pal
)
{
	EcrioPALStruct *h = NULL;
	unsigned int error = KPALErrorNone;

	if (pal == NULL)
	{
		return KPALInvalidParameters;
	}

	*pal = NULL;

	h = (EcrioPALStruct *)malloc(sizeof(EcrioPALStruct));
	if (h == NULL)
	{
		return KPALMemoryError;
	}

	error = pal_mutexInit(h, logHandle, &h->mutex);
	if (error != KPALErrorNone)
	{
		free(h);

		return error;
	}

	error = pal_socketInit(h, logHandle, &h->socket);
	if (error != KPALErrorNone)
	{
		pal_mutexDeinit(h->mutex);
		free(h);

		return error;
	}

	error = pal_timerInit(h, logHandle, &h->timer);
	if (error != KPALErrorNone)
	{
		pal_socketDeinit(h->socket);
		pal_mutexDeinit(h->mutex);
		free(h);

		return error;
	}

	*pal = (PALINSTANCE)h;

	return KPALErrorNone;
}

/**
 * Cleanup the PAL instance.
 */
void pal_Deinit
(
	PALINSTANCE handle
)
{
	EcrioPALStruct *h = (EcrioPALStruct *)handle;

	if (h == NULL)
	{
		return;
	}

	pal_timerDeinit(h->timer);
	pal_socketDeinit(h->socket);
	pal_mutexDeinit(h->mutex);

	free(h);
}
