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

#include "EcrioPAL.h"
#include "EcrioSUEEngine.h"
#include "EcrioSUEInternalFunctions.h"

/******************************************************************************

                API EcrioSUEDeinit

******************************************************************************/

/**
 * This function is used to de-initialize the SUE. This function will clean
 * up all resources and free all memory used by the SUE. This is a synchronous
 * function and can be called at any time. Calling this function without initializing
 * the SUE Handle using the EcrioSUEInit() API could result in unexpected
 * behavior.
 *
 * @param[in,out] ppSUEHandle		Pointer to the SUE Handle. If the referenced
 *									pointer is NULL, then the function will return
 *									successfully. If not NULL, an attempt will be
 *									made to clean up resources and memory.
 * @return                          ECRIO_SUE_NO_ERROR if successful otherwise a
 *									non-zero value indicating the reason of failure.
 */
u_int32 EcrioSUEDeinit
(
	SUEENGINEHANDLE *ppSUEHandle
)
{
	u_int32 uError = ECRIO_SUE_NO_ERROR;
	_EcrioSUEGlobalDataStruct *pSUEGlobalData = NULL;
	void *pLogHandle = NULL;

	if (!ppSUEHandle || !(*ppSUEHandle))
	{
		return ECRIO_SUE_INVALID_INPUT_ERROR;
	}

	pSUEGlobalData = (_EcrioSUEGlobalDataStruct *)(*ppSUEHandle);

	pLogHandle = pSUEGlobalData->pLogHandle;

	SUELOGI( pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__ );

	if (pSUEGlobalData->eEngineState == _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_None/* ||
																				pSUEGlobalData->eEngineState == _ECRIO_SUE_INTERNAL_ENGINE_STATE_ENUM_Deinitialized*/)
	{
		uError = ECRIO_SUE_INVALID_OPERATION_ERROR;

		ECRIO_SUE_LOG_INVALID_OPERATION_ERROR(pSUEGlobalData, (u_char *)"EcrioSUEDeinit");

		ECRIO_SUE_LOG_ALL_STATES(pSUEGlobalData, (u_char *)"EcrioSUEDeinit");

		goto EndTag;
	}

	uError = _EcrioSUEExit(&pSUEGlobalData);
	if (uError != ECRIO_SUE_NO_ERROR)
	{
		goto EndTag;
	}

EndTag:

	*ppSUEHandle = NULL;

	if (pLogHandle)
	{
		SUELOGI( pLogHandle, KLogTypeFuncExit, "%s:%u\t Exit --> %u", __FUNCTION__, __LINE__, uError);
	}

	return uError;
}
