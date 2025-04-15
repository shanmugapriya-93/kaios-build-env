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
 * @file EcrioSDP.c
 * @brief Implementation of the Ecrio SDP Interface implementation instance
 * functions.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"

SDPHANDLE EcrioSDPInit
(
	const EcrioSDPConfigStruct *pConfig,
	u_int32 *pError
)
{
	EcrioSDPStruct *s = NULL;
	u_int32 uSDPError = ECRIO_SDP_NO_ERROR;

	if (pError == NULL)
	{
		return NULL;
	}

	if (pConfig == NULL)
	{
		*pError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
		return NULL;
	}

	SDPLOGI(pConfig->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Assign memory for the primary instance structure. */
	if (pConfig->uAllocationSize >= (sizeof(EcrioSDPStruct) + MAX_IPADDR_LENGTH + MAX_USERNAME_LENGTH))
	{
		s = (EcrioSDPStruct *)pConfig->pAllocated;
	}
	else
	{
		uSDPError = ECRIO_SDP_MEMORY_ERROR;
		goto END;
	}

	if (s == NULL)
	{
		uSDPError = ECRIO_SDP_MEMORY_ERROR;
		goto END;
	}

	pal_MemorySet(s, 0, sizeof(EcrioSDPStruct) + MAX_IPADDR_LENGTH + MAX_USERNAME_LENGTH);

	s->logHandle = pConfig->logHandle;

	s->strings.pData = (u_char *)s + sizeof(EcrioSDPStruct);
	s->strings.uContainerSize = pConfig->uAllocationSize;
	s->strings.uSize = 0;

	uSDPError = ec_sdp_StringCopy(&s->strings,
		(void **)&s->pUsername,
		(u_char *)pConfig->pUsername,
		pal_StringLength((u_char *)pConfig->pUsername));
	if (uSDPError != ECRIO_SDP_NO_ERROR)
	{
		goto END;
	}

	s->ip.eAddressType = pConfig->ip.eAddressType;
	if (pConfig->ip.pAddress != NULL)
	{
		uSDPError = ec_sdp_StringCopy(&s->strings,
			(void **)&s->ip.pAddress,
			(u_char *)pConfig->ip.pAddress,
			pal_StringLength((u_char *)pConfig->ip.pAddress));
		if (uSDPError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}
	else
	{
		s->ip.pAddress = NULL;
	}

	SDPLOGI(s->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

END:
	*pError = uSDPError;
	return (SDPHANDLE)s;
}

u_int32 EcrioSDPDeinit
(
	SDPHANDLE handle
)
{
	EcrioSDPStruct *s = NULL;
	LOGHANDLE logHandle = NULL;

	if (handle == NULL)
	{
		return ECRIO_SDP_NO_ERROR;
	}

	s = (EcrioSDPStruct *)handle;

	logHandle = s->logHandle;

	SDPLOGI(logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Cleanup the SDP instance handle, s. */

	SDPLOGI(logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return ECRIO_SDP_NO_ERROR;
}

u_int32 EcrioSDPUpdateUserName
(
	SDPHANDLE handle,
	unsigned char *pUserName
)
{
	EcrioSDPStruct *s = NULL;
	u_int32 uSDPError = ECRIO_SDP_NO_ERROR;
	LOGHANDLE logHandle = NULL;
	u_int32 uOldContainerSize = 0;
	EcrioSDPAddressTypeEnum	eOldAddressType;
	u_char *pOldAddress;

	if (handle == NULL)
	{
		return ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
	}

	s = (EcrioSDPStruct *)handle;

	SDPLOGI(s->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	logHandle = s->logHandle;
	uOldContainerSize = s->strings.uContainerSize;
	eOldAddressType = s->ip.eAddressType;
	pOldAddress = pal_StringCreate((u_char*)s->ip.pAddress, pal_StringLength((u_char*)s->ip.pAddress));

	pal_MemorySet(s, 0, sizeof(EcrioSDPStruct) + MAX_IPADDR_LENGTH + MAX_USERNAME_LENGTH);

	s->logHandle = logHandle;

	s->strings.pData = (u_char *)s + sizeof(EcrioSDPStruct);
	s->strings.uContainerSize = uOldContainerSize;
	s->strings.uSize = 0;

	uSDPError = ec_sdp_StringCopy(&s->strings,
		(void **)&s->pUsername,
		(u_char *)pUserName,
		pal_StringLength((u_char *)pUserName));
	if (uSDPError != ECRIO_SDP_NO_ERROR)
	{
		goto END;
	}

	s->ip.eAddressType = eOldAddressType;
	if (pOldAddress != NULL)
	{
		uSDPError = ec_sdp_StringCopy(&s->strings,
			(void **)&s->ip.pAddress,
			(u_char *)pOldAddress,
			pal_StringLength((u_char *)pOldAddress));
		if (uSDPError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}
	else
	{
		s->ip.pAddress = NULL;
	}

END:
	if(pOldAddress != NULL)
		pal_MemoryFree((void**)&pOldAddress);

	SDPLOGI(s->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return ECRIO_SDP_NO_ERROR;
}
