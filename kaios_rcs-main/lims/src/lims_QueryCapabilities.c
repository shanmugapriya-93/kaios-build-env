/******************************************************************************

Copyright (c) 2019-2020 Ecrio, Inc. All Rights Reserved.

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
* @file lims_QueryCapabilities.c
* @brief Implementation of lims Query Capabilities.
*/

#include "lims.h"
#include "lims_internal.h"

/**
* This function is called to query the capabilities of an user OR list of users.
*
* @param[in] handle				The lims instance handle.
* @param[in] pQueryCapabilties	Query capability data such as RLS URI, list of contacts, features required
*								to form SUBSCRIBE or OPTION to query the capabilities.
* @param[in] ppQueryId			Id associated with the query. Same Id will be passed in the callback
*								for matching.
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
*/
u_int32 lims_QueryCapabilities
(
	LIMSHANDLE handle,
	EcrioUCEQueryCapabilitiesStruct *pQueryCapabilties,
	u_char** ppQueryId
)
{

	u_int32 uLimsError = LIMS_NO_ERROR;
	lims_moduleStruct *m = NULL;

#if defined(ENABLE_RCS)
	u_int32 uUCEError = ECRIO_UCE_NO_ERROR;
#endif
	

	if ((m = (lims_moduleStruct *)handle) == NULL) return LIMS_INVALID_PARAMETER1;

	pal_MutexGlobalLock(m->pal);

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

#if defined(ENABLE_RCS)
	if (ppQueryId == NULL)
	{
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\tpQueryId is NULL", __FUNCTION__, __LINE__);
		uLimsError = LIMS_INVALID_PARAMETER3;
		goto EndTag;
	}

	/* Invoke UCE API */
	uUCEError = EcrioUCEQueryCapabilities(m->pUCEHandle, pQueryCapabilties, ppQueryId);
	if (uUCEError != ECRIO_UCE_NO_ERROR)
	{
		uLimsError = LIMS_UCE_CAPABILITY_QUERY_ERROR;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t EcrioUCEQueryCapabilities() failed with error code uUCEError:%d",
			__FUNCTION__, __LINE__, uUCEError);
		goto EndTag;
	}


#else
	LIMSLOGW(m->logHandle, KLogTypeGeneral, "%s:%u-  UCE feature disabled from this build", __FUNCTION__, __LINE__);
	uLimsError = LIMS_UCE_FEATURE_NOT_SUPPORTED;
	goto EndTag;
#endif

EndTag:
	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);

	pal_MutexGlobalUnlock(m->pal);

	return uLimsError;
}
