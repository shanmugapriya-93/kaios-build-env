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

#include "EcrioCPM.h"
#include "EcrioCPMInternal.h"
#include "khash.h"

const u_int32 uKHCPMName = 16;

KHASH_MAP_INIT_STR(uKHCPMName, void*);

u_int32 ec_CPM_MapInit(CPMMAPHANDLE* ppHandle)
{
	khash_t(uKHCPMName) *h = kh_init(uKHCPMName);

	*ppHandle = (CPMMAPHANDLE)h;

	return 0;
}

static u_int32 ec_CPM_MapInsertData(CPMMAPHANDLE pHandle, u_char* key, void* pData)
{
	s_int32 error = 0;
	khiter_t iterator;

	khash_t(uKHCPMName) *h = (khash_t(uKHCPMName) *)pHandle;

	// Add a value Key to the hashtable
	iterator = kh_put(uKHCPMName, h, (kh_cstr_t)key, &error);

	kh_value(h, iterator) = (void*)pData; // set the value of the key

	return error;
}

static u_int32 ec_CPM_MapGetKeyData(CPMMAPHANDLE pHandle, u_char* key, void** ppData)
{
	u_int32 error = 0;
	khiter_t iterator;

	khash_t(uKHCPMName) *h = (khash_t(uKHCPMName) *)pHandle;

	// Retrieve the value for key "apple"
	iterator = kh_get(uKHCPMName, h, (kh_cstr_t)key);  // first have to get iterator

	if (iterator != kh_end(h))
	{
		// iterator will be equal to kh_end if key not present
		*ppData = (void*)kh_val(h, iterator); // next have to fetch  the actual value

		error = 0;
	}
	else
	{
		error = 1;
	}
	return error;
}

/* Note - ppData need to be deleted separately by caller */

u_int32 ec_CPM_MapDeleteKeyData(CPMMAPHANDLE pHandle, u_char* key)
{
	u_int32 error = 0;
	khiter_t iterator;

	khash_t(uKHCPMName) *h = (khash_t(uKHCPMName) *)pHandle;

	// Retrieve the value for key "apple"

	iterator = kh_get(uKHCPMName, h, (kh_cstr_t)key);  // first have to get iterator

	if (iterator != kh_end(h))
	{
		kh_del(uKHCPMName, h, iterator); // Delete the key and data

		error = 0;
	}
	else
	{
		error = 1;
	}

	return error;
}

u_int32 ec_CPM_MapDeInit(CPMMAPHANDLE pHandle)
{
	khash_t(uKHCPMName) *h = (khash_t(uKHCPMName) *)pHandle;

	kh_destroy(uKHCPMName, h);

	return 0;
}

u_int32 ec_CPM_MapSetData
(
	EcrioCPMContextStruct *pContext,
	u_char* pSessionID,
	void* pAppData
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;

	if (pSessionID == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (ec_CPM_MapInsertData(pContext->hHashMap, pSessionID, pAppData) != 0)
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
	}

	return uError;
}

u_int32 ec_CPM_MapGetData
(
	EcrioCPMContextStruct *pContext,
	u_char* pSessionID,
	void** ppAppData
)
{
	u_int32	uError = ECRIO_CPM_NO_ERROR;
	u_char* pCallId = NULL;

	if ((pSessionID == NULL) || (ppAppData == NULL))
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	ec_CPM_MapGetKeyData(pContext->hHashMap, pSessionID, (void **)&pCallId);
	if (pCallId)
	{
		*ppAppData = pCallId;
	}
	else
	{
		uError = ECRIO_CPM_INTERNAL_ERROR;
	}

	return uError;
}