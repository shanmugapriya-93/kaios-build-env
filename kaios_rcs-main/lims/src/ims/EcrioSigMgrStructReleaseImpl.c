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

#include "EcrioDSListInterface.h"
#include "EcrioSigMgrCommon.h"
#include "EcrioSigMgrInit.h"
#include "EcrioSigMgrInternal.h"
#include "EcrioSigMgrUtilities.h"
#include "EcrioSigMgrIMSLibHandler.h"
#include "EcrioSigMgrTransactionHandler.h"
#include "EcrioSigMgrDialogHandler.h"
#include "EcrioSigMgrSubscribe.h"
#include "EcrioSigMgrSubscribeInternal.h"

/*****************************************************************************

Function:		_EcrioSigMgrInternalStructRelease ()

Purpose:		to release the members of the provided structure

Description:	This function interprets the incoming structure based on the
                enum parameter that indicates the type of structure. The
                function then release the members of the structure.

Input:			void* ppStruct - pointer to the structure to be de-allocated
                EcrioSigMgrInternalStructTypeEnum type - enumerated value that indicates
                the type of structure.
                BoolEnum release - if set to Enum_TRUE shall de-allocate
                the pointer memory too.

Output:			None

Returns:		Error Code

*****************************************************************************/
u_int32 _EcrioSigMgrInternalStructRelease
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrInternalStructTypeEnum structType,
	void **ppStruct,
	BoolEnum release
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	if (pSigMgr == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	if (ppStruct == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	switch (structType)
	{
		case EcrioSigMgrStructType_SigMgr:
		{
			_EcrioSigMgrReleaseSigMgrStruct((EcrioSigMgrStruct *)(*ppStruct));
		}
		break;

		case EcrioSigMgrStructType_RegisterInfo:
		{
			_EcrioSigMgrReleaseRegisterInfoStruct(pSigMgr,
				(EcrioSigMgrRegisterInfoStruct *)(*ppStruct));
		}
		break;

		case EcrioSigMgrStructType_RegContactInfo:
		{
			_EcrioSigMgrReleaseRegContactInfoStruct(pSigMgr,
				(EcrioSigMgrRegisterContactInfoStruct *)(*ppStruct));
		}
		break;

		case EcrioSigMgrStructType_UserRegisterInfo:
		{
			_EcrioSigMgrReleaseUserRegisterInfoStruct(pSigMgr,
				(EcrioSigMgrUserRegisterInfoStruct *)(*ppStruct));
		}
		break;

		case EcrioSigMgrStructType_SessionHandle:
		{
			//_EcrioSigMgrReleaseSessionHandleStruct(pSigMgr,
				//(EcrioSigMgrSessionHandleStruct *)(*ppStruct));
		}
		break;

		case EcrioSigMgrStructType_CommonInfo:
		{
			_EcrioSigMgrReleaseCommonInfoStruct(pSigMgr,
				(EcrioSigMgrCommonInfoStruct *)(*ppStruct));
		}
		break;

		case EcrioSigMgrStructType_TXNInfo:
		{
			_EcrioSigMgrReleaseTXNInfoStruct(pSigMgr,
				(EcrioSigMgrTXNInfoStruct *)(*ppStruct));
		}
		break;

		case EcrioSigMgrStructType_InviteUsageInfo:
		{
			_EcrioSigMgrReleaseInviteUsageInfoStruct(pSigMgr,
				(_EcrioSigMgrInviteUsageInfoStruct *)(*ppStruct));
		}
		break;

		case EcrioSigMgrStructType_SubscribeUsageInfo:
		{
			_EcrioSigMgrReleaseSubscribeUsageInfoStruct(pSigMgr,
				(_EcrioSigMgrSubscribeUsageInfoStruct *)(*ppStruct));
		}
		break;

		default:
		{
			error = ECRIO_SIGMGR_STRUCT_TYPE_UNDEFINED;
		}
		break;
	}

	if (release == Enum_TRUE)
	{
		pal_MemoryFree((void **)ppStruct);
	}

Error_Level_01:
	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrReleaseSigMgrStruct()

Purpose:		To derelease the signaling manager structure members

Description:	This function frees the memory allocated for the members
                of the signaling manager structure and dereleases the
                members and deallocates the memory for the signaling
                structure.

Input:			IN EcrioSigMgrStruct** pSigMgr - Signalling manager handle

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrReleaseSigMgrStruct
(
	EcrioSigMgrStruct *pSigMgr
)
{
	if (pSigMgr->pRegisterInfo != NULL)
	{
		/* Release the register info */
		_EcrioSigMgrReleaseRegInfo(pSigMgr, &(pSigMgr->pRegisterInfo));
	}

	/* Release the signaling info */
	if (pSigMgr->pSignalingInfo != NULL)
	{
		_EcrioSigMgrReleaseInternalSignalingInfoStruct(pSigMgr, pSigMgr->pSignalingInfo);
		pal_MemoryFree((void **)&pSigMgr->pSignalingInfo);
	}

	/* Service-Route */
	if (pSigMgr->pServiceRoute)
	{
		_EcrioSigMgrReleaseRouteStruct(pSigMgr, pSigMgr->pServiceRoute);
		pal_MemoryFree((void **)&pSigMgr->pServiceRoute);
	}

	if (pSigMgr->pSharedCredentials)
	{
		_EcrioSigMgrReleaseSharedCredentialsStruct(pSigMgr, pSigMgr->pSharedCredentials);
		pal_MemoryFree((void **)&pSigMgr->pSharedCredentials);
	}

	if (pSigMgr->pNetworkInfo)
	{
		if (pSigMgr->pNetworkInfo->pLocalIp)
		{
			pal_MemoryFree((void **)&pSigMgr->pNetworkInfo->pLocalIp);
		}

		if (pSigMgr->pNetworkInfo->pRemoteIP)
		{
			pal_MemoryFree((void **)&pSigMgr->pNetworkInfo->pRemoteIP);
		}

		pal_MemoryFree((void **)&pSigMgr->pNetworkInfo);
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:		_EcrioSigMgrReleaseSignalingInfoStruct ()

Purpose:		to release the members of the EcrioSigMgrSignalingInfoStruct.

Description:	to release the members of the EcrioSigMgrSignalingInfoStruct.

Input:			EcrioSigMgrSignalingInfoStruct* pSignalingInfo - SignalingInfo struct.

Output:			None

Returns:		None

**************************************************************************/
u_int32 _EcrioSigMgrReleaseSignalingInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSignalingInfoStruct *pSignalingInfo
)
{
	// @todo Do we need pSigMgr parameter in this function?
	(void)pSigMgr;

	if (!pSignalingInfo)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pSignalingInfo->pDisplayName != NULL)
	{
		pal_MemoryFree((void **)&(pSignalingInfo->pDisplayName));
	}

	if (pSignalingInfo->pPrivateId != NULL)
	{
		pal_MemoryFree((void **)&(pSignalingInfo->pPrivateId));
	}

	if (pSignalingInfo->pPassword != NULL)
	{
		pal_MemoryFree((void **)&(pSignalingInfo->pPassword));
	}

	if (pSignalingInfo->pHomeDomain != NULL)
	{
		pal_MemoryFree((void **)&(pSignalingInfo->pHomeDomain));
	}

	/*if (pSignalingInfo->pFeatureTag != NULL)
	{
	    pal_MemoryFree((void **)&(pSignalingInfo->pFeatureTag));
	}*/

	if (pSignalingInfo->pRegistrarDomain != NULL)
	{
		pal_MemoryFree((void **)&(pSignalingInfo->pRegistrarDomain));
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:		_EcrioSigMgrReleaseInternalSignalingInfoStruct ()

Purpose:		to release the members of the EcrioSigMgrSignalingInfoStruct.

Description:	to release the members of the EcrioSigMgrSignalingInfoStruct.

Input:			EcrioSigMgrSignalingInfoStruct* pSignalingInfo - SignalingInfo struct.

Output:			None

Returns:		None

**************************************************************************/
u_int32 _EcrioSigMgrReleaseInternalSignalingInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrInternalSignalingInfoStruct *pSignalingInfo
)
{
	if (!pSignalingInfo)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pSignalingInfo->pUserId != NULL)
	{
		pal_MemoryFree((void **)&(pSignalingInfo->pUserId));
	}

	/*if (pSignalingInfo->pFeatureTag != NULL)
	{
	    pal_MemoryFree((void **)&(pSignalingInfo->pFeatureTag));
	}*/

	if (pSignalingInfo->pFromSipURI)
	{
		_EcrioSigMgrReleaseUriStruct(pSigMgr, pSignalingInfo->pFromSipURI);
		pal_MemoryFree((void **)&pSignalingInfo->pFromSipURI);
		pSignalingInfo->pFromSipURI = NULL;
	}

	if (pSignalingInfo->ppPAssociatedURI)
	{
		u_int32 iterator = 0;
		if (pSignalingInfo->ppPAssociatedURI)
		{
			for (iterator = 0; iterator < pSignalingInfo->numPAssociatedURIs; iterator++)
			{
				EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_NameAddrWithParams,
					(void **)&(pSignalingInfo->ppPAssociatedURI[iterator]), Enum_TRUE);
			}

			pal_MemoryFree((void **)&(pSignalingInfo->ppPAssociatedURI));
		}
	}
	if (pSignalingInfo->pPublishInfo)
	{
		_EcrioSigMgrPublishInfoStruct((EcrioSigMgrPublishInfo**)&pSignalingInfo->pPublishInfo);
		pSignalingInfo->pPublishInfo = NULL;
	}
	return ECRIO_SIGMGR_NO_ERROR;
}

void _EcrioSigMgrPublishInfoStruct
(
EcrioSigMgrPublishInfo **ppPublishInfo
)
{
	if (*ppPublishInfo)
	{
		EcrioSigMgrPublishInfo *pPublishInfo = *ppPublishInfo;

		if (pPublishInfo->SipETag)
		{
			pal_MemoryFree((void **)&(pPublishInfo->SipETag));
		}
		if (pPublishInfo->pMssgBody)
		{
			pal_MemoryFree((void **)&(pPublishInfo->pMssgBody));
		}

		if(pPublishInfo->bIsPubExpireTimerRunning && pPublishInfo->uPubExpireTimerID != NULL)
		{
			pal_TimerStop(pPublishInfo->uPubExpireTimerID);
			pPublishInfo->uPubExpireTimerID = NULL;
			pPublishInfo->bIsPubExpireTimerRunning = Enum_FALSE;
		}

		pal_MemoryFree((void**)ppPublishInfo);
	}
}
/**************************************************************************

Function:		_EcrioSigMgrReleaseSignalingChangeParamInfoStruct ()

Purpose:		to release the members of the EcrioSigMgrSignalingChangeParamInfoStruct.

Description:	to release the members of the EcrioSigMgrSignalingChangeParamInfoStruct.

Input:			EcrioSigMgrSignalingChangeParamInfoStruct* pChangeParamInfo - ChangeParamInfo struct.

Output:			None

Returns:		None

**************************************************************************/
u_int32 _EcrioSigMgrReleaseSignalingChangeParamInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSignalingChangeParamInfoStruct *pChangeParamInfo
)
{
	// @todo Do we need pSigMgr parameter in this function?
	(void)pSigMgr;

	if (!pChangeParamInfo)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	/* release the IP address structure */
	if (pChangeParamInfo->pCustomSIPTimers != NULL)
	{
		pal_MemoryFree((void **)&(pChangeParamInfo->pCustomSIPTimers));
		pChangeParamInfo->pCustomSIPTimers = NULL;
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:		_EcrioSigMgrReleaseInfoStruct ()

Purpose:		to release the members of the EcrioSigMgrInfoStruct.

Description:	to release the members of the EcrioSigMgrInfoStruct.

Input:			EcrioSigMgrInfoStruct* pInfo - Info struct.

Output:			None

Returns:		None

**************************************************************************/
u_int32 _EcrioSigMgrReleaseInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrInfoStruct *pInfo
)
{
	pSigMgr = pSigMgr;	/* Removing level-4 warning */

	if (!pInfo)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	pInfo->eventType = EcrioSigMgrMessageNone;
	pInfo->pEventIdentifier = NULL;

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:		_EcrioSigMgrReleaseMessageStruct ()

Purpose:		to release the members of the EcrioSigMgrMessageStruct.

Description:	to release the members of the EcrioSigMgrMessageStruct.

Input:			EcrioSigMgrMessageStruct* pMessage - Message struct.

Output:			None

Returns:		None

**************************************************************************/
u_int32 _EcrioSigMgrReleaseMessageStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrMessageStruct *pMessage
)
{
	pSigMgr = pSigMgr;	/* Removing level-4 warning */

	if (!pMessage)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	pMessage->msgCmd = EcrioSigMgrMessageNone;
	pMessage->pData = NULL;

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:		_EcrioSigMgrReleaseIPAddrStruct ()

Purpose:		to release the members of the EcrioSigMgrIPAddrStruct.

Description:	to release the members of the EcrioSigMgrIPAddrStruct.

Input:			EcrioSigMgrIPAddrStruct* pIPAddrStruct - IP Addr struct.

Output:			None

Returns:		None

**************************************************************************/
u_int32 _EcrioSigMgrReleaseIPAddrStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrIPAddrStruct *pIPAddrStruct
)
{
	// @todo Do we need pSigMgr parameter in this function?
	(void)pSigMgr;

	if (!pIPAddrStruct)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pIPAddrStruct->pIPAddr != NULL)
	{
		pal_MemoryFree((void **)&(pIPAddrStruct->pIPAddr));
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:		_EcrioSigMgrReleaseParamsStruct ()

Purpose:		to release the members of the EcrioSigMgrParamStruct.

Description:	to release the members of the EcrioSigMgrParamStruct.

Input:			EcrioSigMgrParamStruct* pParamStruct - Params struct.

Output:			None

Returns:		None

**************************************************************************/
u_int32 _EcrioSigMgrReleaseParamsStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrParamStruct *pParamStruct
)
{
	// @todo Do we need pSigMgr parameter in this function?
	(void)pSigMgr;

	if (!pParamStruct)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pParamStruct->pParamName != NULL)
	{
		pal_MemoryFree((void **)&(pParamStruct->pParamName));
	}

	if (pParamStruct->pParamValue != NULL)
	{
		pal_MemoryFree((void **)&(pParamStruct->pParamValue));
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

u_int32 _EcrioSigMgrReleaseTelParamArrayStruct
(
	EcrioSigMgrStruct *pSigMgr,
	u_int16 numTelParams,
	EcrioSigMgrTelParStruct **ppTelParams
)
{
	u_int32 i = 0;

	// @todo Do we need pSigMgr parameter in this function?
	(void)pSigMgr;

	for (i = 0; i < numTelParams; i++)
	{
		if (ppTelParams[i]->telParType == EcrioSigMgrTelParExtension)
		{
			pal_MemoryFree((void **)&ppTelParams[i]->u.pExtension);
		}
		else if (ppTelParams[i]->telParType == EcrioSigMgrTelParIsdnSubAddress)
		{
			pal_MemoryFree((void **)&ppTelParams[i]->u.pIsdnSubaddress);
		}
		else if (ppTelParams[i]->telParType == EcrioSigMgrTelParParameter)
		{
			pal_MemoryFree((void **)&ppTelParams[i]->u.pParameter->pParamName);
			pal_MemoryFree((void **)&ppTelParams[i]->u.pParameter->pParamValue);
			pal_MemoryFree((void **)&ppTelParams[i]->u.pParameter);
		}

		pal_MemoryFree((void **)&ppTelParams[i]);
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

u_int32 _EcrioSigMgrReleaseTelUriStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrTelURIStruct *pTelUri
)
{
	if (!pTelUri)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pTelUri->subscriberType == EcrioSigMgrTelSubscriberGlobal)
	{
		if (pTelUri->u.pGlobalNumber->pGlobalNumberDigits)
		{
			pal_MemoryFree((void **)&pTelUri->u.pGlobalNumber->pGlobalNumberDigits);
		}

		if (pTelUri->u.pGlobalNumber->ppTelPar)
		{
			_EcrioSigMgrReleaseTelParamArrayStruct(pSigMgr, pTelUri->u.pGlobalNumber->numTelPar, pTelUri->u.pGlobalNumber->ppTelPar);
			pal_MemoryFree((void **)&pTelUri->u.pGlobalNumber->ppTelPar);
		}

		pal_MemoryFree((void **)&pTelUri->u.pGlobalNumber);
	}
	else if (pTelUri->subscriberType == EcrioSigMgrTelSubscriberLocal)
	{
		if (pTelUri->u.pLocalNumber->pLocalNumberDigits)
		{
			pal_MemoryFree((void **)&pTelUri->u.pLocalNumber->pLocalNumberDigits);
		}

		if (pTelUri->u.pLocalNumber->ppTelPar1)
		{
			_EcrioSigMgrReleaseTelParamArrayStruct(pSigMgr, pTelUri->u.pLocalNumber->numTelPar1, pTelUri->u.pLocalNumber->ppTelPar1);
			pal_MemoryFree((void **)&pTelUri->u.pLocalNumber->ppTelPar1);
		}

		if (pTelUri->u.pLocalNumber->pContext)
		{
			if (pTelUri->u.pLocalNumber->pContext->u.pDomainName)
			{
				pal_MemoryFree((void **)&pTelUri->u.pLocalNumber->pContext->u.pDomainName);
			}

			pal_MemoryFree((void **)&pTelUri->u.pLocalNumber->pContext);
		}

		if (pTelUri->u.pLocalNumber->ppTelPar2)
		{
			_EcrioSigMgrReleaseTelParamArrayStruct(pSigMgr, pTelUri->u.pLocalNumber->numTelPar2, pTelUri->u.pLocalNumber->ppTelPar2);
			pal_MemoryFree((void **)&pTelUri->u.pLocalNumber->ppTelPar2);
		}

		pal_MemoryFree((void **)&pTelUri->u.pLocalNumber);
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:		_EcrioSigMgrReleaseSipUriStruct ()

Purpose:		to release the members of the EcrioSigMgrSipURIStruct.

Description:	to release the members of the EcrioSigMgrSipURIStruct.

Input:			EcrioSigMgrSipURIStruct* pSipUri - SIP URI struct.

Output:			None

Returns:		None

**************************************************************************/
u_int32 _EcrioSigMgrReleaseSipUriStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipURIStruct *pSipUri
)
{
	u_int32 i = 0;

	if (!pSipUri)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pSipUri->pUserId != NULL)
	{
		pal_MemoryFree((void **)&(pSipUri->pUserId));
	}

	if (pSipUri->pPassword != NULL)
	{
		pal_MemoryFree((void **)&(pSipUri->pPassword));
	}

	if (pSipUri->pDomain != NULL)
	{
		pal_MemoryFree((void **)&(pSipUri->pDomain));
	}

	if (pSipUri->pIPAddr != NULL)
	{
		_EcrioSigMgrReleaseIPAddrStruct(pSigMgr, pSipUri->pIPAddr);
		pal_MemoryFree((void **)&(pSipUri->pIPAddr));
	}

	if (pSipUri->ppURIParams != NULL)
	{
		for (i = 0; i < pSipUri->numURIParams; ++i)
		{
			_EcrioSigMgrReleaseParamsStruct(pSigMgr, (pSipUri->ppURIParams[i]));
			pal_MemoryFree((void **)&(pSipUri->ppURIParams[i]));
		}

		pal_MemoryFree((void **)&(pSipUri->ppURIParams));
	}

	if (pSipUri->ppURIHeaders != NULL)
	{
		for (i = 0; i < pSipUri->numURIHeaders; ++i)
		{
			_EcrioSigMgrReleaseParamsStruct(pSigMgr, (pSipUri->ppURIHeaders[i]));
			pal_MemoryFree((void **)&(pSipUri->ppURIHeaders[i]));
		}

		pal_MemoryFree((void **)&(pSipUri->ppURIHeaders));
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:		EcrioSigMgrReleaseUriStruct ()

Purpose:		to release the members of the EcrioSigMgrSipURIStruct.

Description:	API to release the members of the EcrioSigMgrSipURIStruct.

Input:			sigMgrHandle - handle to signaling manager pSipUri - SIP URI struct.

Output:			None

Returns:		Error-codes

**************************************************************************/
u_int32 EcrioSigMgrReleaseUriStruct
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrUriStruct *pUri
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgr = NULL;

	if (sigMgrHandle == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	pSigMgr = (EcrioSigMgrStruct *)sigMgrHandle;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	error = _EcrioSigMgrReleaseUriStruct(pSigMgr, pUri);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);

	return error;
}

/**************************************************************************

Function:		_EcrioSigMgrReleaseUriStruct ()

Purpose:		to release the members of the EcrioSigMgrSipURIStruct.

Description:	to release the members of the EcrioSigMgrSipURIStruct.

Input:			EcrioSigMgrSipURIStruct* pSipUri - SIP URI struct.

Output:			None

Returns:		None

**************************************************************************/
u_int32 _EcrioSigMgrReleaseUriStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUriStruct *pUri
)
{
	if (!pUri)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pUri->uriScheme == EcrioSigMgrURISchemeSIP)
	{
		_EcrioSigMgrReleaseSipUriStruct(pSigMgr, pUri->u.pSipUri);
		pal_MemoryFree((void **)&(pUri->u.pSipUri));
	}
	else if (pUri->uriScheme == EcrioSigMgrURISchemeTEL)
	{
		_EcrioSigMgrReleaseTelUriStruct(pSigMgr, pUri->u.pTelUri);
		pal_MemoryFree((void **)&(pUri->u.pTelUri));
	}
	else if (pUri->uriScheme == EcrioSigMgrURISchemeCustom)
	{
		if (pUri->u.pAbsUri)
		{
			pal_MemoryFree((void **)&(pUri->u.pAbsUri));
		}
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:		_EcrioSigMgrReleaseNameAddrStruct ()

Purpose:		to release the members of the EcrioSigMgrSipURIStruct.

Description:	to release the members of the EcrioSigMgrSipURIStruct.

Input:			EcrioSigMgrSipURIStruct* pSipUri - SIP URI struct.

Output:			None

Returns:		None

**************************************************************************/
u_int32 _EcrioSigMgrReleaseNameAddrStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrNameAddrStruct *pNameAddr
)
{
	if (!pNameAddr)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	_EcrioSigMgrReleaseUriStruct(pSigMgr, &pNameAddr->addrSpec);
	if (pNameAddr->pDisplayName)
	{
		pal_MemoryFree((void **)&(pNameAddr->pDisplayName));
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:		_EcrioSigMgrReleaseNameAddrWithParamsStruct ()

Purpose:		to release the members of the EcrioSigMgrSipURIStruct.

Description:	to release the members of the EcrioSigMgrSipURIStruct.

Input:			EcrioSigMgrSipURIStruct* pSipUri - SIP URI struct.

Output:			None

Returns:		None

**************************************************************************/
u_int32 _EcrioSigMgrReleaseNameAddrWithParamsStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrNameAddrWithParamsStruct *pNameAddrWithParams
)
{
	if (!pNameAddrWithParams)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	_EcrioSigMgrReleaseNameAddrStruct(pSigMgr, &pNameAddrWithParams->nameAddr);
	if (pNameAddrWithParams->ppParams != NULL)
	{
		u_int32 i = 0;

		for (i = 0; i < pNameAddrWithParams->numParams; ++i)
		{
			_EcrioSigMgrReleaseParamsStruct(pSigMgr, pNameAddrWithParams->ppParams[i]);
			pal_MemoryFree((void **)&(pNameAddrWithParams->ppParams[i]));
		}

		pal_MemoryFree((void **)&(pNameAddrWithParams->ppParams));
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:		_EcrioSigMgrReleaseHdrValueStruct ()

Purpose:		to release the members of the EcrioSigMgrSipURIStruct.

Description:	to release the members of the EcrioSigMgrSipURIStruct.

Input:			EcrioSigMgrSipURIStruct* pSipUri - SIP URI struct.

Output:			None

Returns:		None

**************************************************************************/
u_int32 _EcrioSigMgrReleaseHdrValueStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrHeaderValueStruct *pHdrValue
)
{
	if (!pHdrValue)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pHdrValue->pHeaderValue != NULL)
	{
		pal_MemoryFree((void **)&(pHdrValue->pHeaderValue));
	}

	if (pHdrValue->ppParams != NULL)
	{
		u_int32 i = 0;

		for (i = 0; i < pHdrValue->numParams; ++i)
		{
			_EcrioSigMgrReleaseParamsStruct(pSigMgr, pHdrValue->ppParams[i]);
			pal_MemoryFree((void **)&(pHdrValue->ppParams[i]));
		}

		pal_MemoryFree((void **)&(pHdrValue->ppParams));
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:		_EcrioSigMgrReleaseHdrDetailStruct ()

Purpose:		to release the members of the EcrioSigMgrSipURIStruct.

Description:	to release the members of the EcrioSigMgrSipURIStruct.

Input:			EcrioSigMgrSipURIStruct* pSipUri - SIP URI struct.

Output:			None

Returns:		None

**************************************************************************/
u_int32 _EcrioSigMgrReleaseHdrStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrHeaderStruct *pHdr
)
{
	if (!pHdr)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pHdr->pHeaderName != NULL)
	{
		pal_MemoryFree((void **)&(pHdr->pHeaderName));
	}

	if (pHdr->ppHeaderValues != NULL)
	{
		u_int32 i = 0;

		for (i = 0; i < pHdr->numHeaderValues; ++i)
		{
			_EcrioSigMgrReleaseHdrValueStruct(pSigMgr, pHdr->ppHeaderValues[i]);
			pal_MemoryFree((void **)&(pHdr->ppHeaderValues[i]));
		}

		pal_MemoryFree((void **)&(pHdr->ppHeaderValues));
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:		_EcrioSigMgrReleaseManHdrStruct ()

Purpose:		to release the members of the EcrioSigMgrSipURIStruct.

Description:	to release the members of the EcrioSigMgrSipURIStruct.

Input:			EcrioSigMgrSipURIStruct* pSipUri - SIP URI struct.

Output:			None

Returns:		None

**************************************************************************/
u_int32 _EcrioSigMgrReleaseManHdrStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrMandatoryHeaderStruct *pMandatoryHdrs
)
{
	if (!pMandatoryHdrs)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pMandatoryHdrs->pRequestUri != NULL)
	{
		_EcrioSigMgrReleaseUriStruct(pSigMgr, pMandatoryHdrs->pRequestUri);
		pal_MemoryFree((void **)&(pMandatoryHdrs->pRequestUri));
	}

	if (pMandatoryHdrs->pCallId != NULL)
	{
		pal_MemoryFree((void **)&(pMandatoryHdrs->pCallId));
	}

	if (pMandatoryHdrs->pTo != NULL)
	{
		_EcrioSigMgrReleaseNameAddrWithParamsStruct(pSigMgr, pMandatoryHdrs->pTo);
		pal_MemoryFree((void **)&(pMandatoryHdrs->pTo));
	}

	if (pMandatoryHdrs->pFrom != NULL)
	{
		_EcrioSigMgrReleaseNameAddrWithParamsStruct(pSigMgr, pMandatoryHdrs->pFrom);
		pal_MemoryFree((void **)&(pMandatoryHdrs->pFrom));
	}

	if (pMandatoryHdrs->ppVia != NULL)
	{
		u_int32 i = 0;

		for (i = 0; i < pMandatoryHdrs->numVia; ++i)
		{
			_EcrioSigMgrReleaseViaStruct(pSigMgr, pMandatoryHdrs->ppVia[i]);
			pal_MemoryFree((void **)&(pMandatoryHdrs->ppVia[i]));
		}

		pal_MemoryFree((void **)&(pMandatoryHdrs->ppVia));
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:		_EcrioSigMgrReleaseRouteStruct ()

Purpose:		to release the members of the EcrioSigMgrSipURIStruct.

Description:	to release the members of the EcrioSigMgrSipURIStruct.

Input:			EcrioSigMgrSipURIStruct* pSipUri - SIP URI struct.

Output:			None

Returns:		None

**************************************************************************/
u_int32 _EcrioSigMgrReleaseRouteStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrRouteStruct *pRoute
)
{
	if (!pRoute)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pRoute->ppRouteDetails != NULL)
	{
		u_int32 i = 0;

		for (i = 0; i < pRoute->numRoutes; ++i)
		{
			_EcrioSigMgrReleaseNameAddrWithParamsStruct(pSigMgr, pRoute->ppRouteDetails[i]);
			pal_MemoryFree((void **)&(pRoute->ppRouteDetails[i]));
		}

		pal_MemoryFree((void **)&(pRoute->ppRouteDetails));
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:		_EcrioSigMgrReleaseContactStruct ()

Purpose:		to release the members of the EcrioSigMgrSipURIStruct.

Description:	to release the members of the EcrioSigMgrSipURIStruct.

Input:			EcrioSigMgrSipURIStruct* pSipUri - SIP URI struct.

Output:			None

Returns:		None

**************************************************************************/
u_int32 _EcrioSigMgrReleaseContactStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrContactStruct *pContact
)
{
	if (!pContact)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pContact->ppContactDetails != NULL)
	{
		u_int32 i = 0;

		for (i = 0; i < pContact->numContactUris; ++i)
		{
			_EcrioSigMgrReleaseNameAddrWithParamsStruct(pSigMgr, pContact->ppContactDetails[i]);
			pal_MemoryFree((void **)&(pContact->ppContactDetails[i]));
		}

		pal_MemoryFree((void **)&(pContact->ppContactDetails));
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:		_EcrioSigMgrReleaseViaStruct ()

Purpose:		to release the members of the EcrioSigMgrSipURIStruct.

Description:	to release the members of the EcrioSigMgrSipURIStruct.

Input:			EcrioSigMgrSipURIStruct* pSipUri - SIP URI struct.

Output:			None

Returns:		None

**************************************************************************/
u_int32 _EcrioSigMgrReleaseViaStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrViaStruct *pVia
)
{
	if (!pVia)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pVia->pSipVersion != NULL)
	{
		pal_MemoryFree((void **)&(pVia->pSipVersion));
	}

	if (pVia->pBranch != NULL)
	{
		pal_MemoryFree((void **)&(pVia->pBranch));
	}

	if (pVia->ppParams != NULL)
	{
		u_int32 i = 0;

		for (i = 0; i < pVia->numParams; ++i)
		{
			_EcrioSigMgrReleaseParamsStruct(pSigMgr, pVia->ppParams[i]);
			pal_MemoryFree((void **)&(pVia->ppParams[i]));
		}

		/* Mem Leak fix pVia->numParams, Enum_TRUE, (void**)(pVia->ppParams)); */
		pal_MemoryFree((void **)&(pVia->ppParams));
	}

	_EcrioSigMgrReleaseIPAddrStruct(pSigMgr, &(pVia->pIPaddr));

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:		_EcrioSigMgrReleaseAuthorizationStruct ()

Purpose:		to release the members of the EcrioSigMgrSipURIStruct.

Description:	to release the members of the EcrioSigMgrSipURIStruct.

Input:			EcrioSigMgrSipURIStruct* pSipUri - SIP URI struct.

Output:			None

Returns:		None

**************************************************************************/
u_int32 _EcrioSigMgrReleaseAuthorizationStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrAuthorizationStruct *pAuth
)
{
	// @todo Do we need pSigMgr parameter in this function?
	(void)pSigMgr;

	if (!pAuth)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pAuth->pAuthenticationScheme != NULL)
	{
		pal_MemoryFree(
			(void **)&(pAuth->pAuthenticationScheme));
	}

	if (pAuth->pUserName != NULL)
	{
		pal_MemoryFree(
			(void **)&(pAuth->pUserName));
	}

	if (pAuth->pRealm != NULL)
	{
		pal_MemoryFree((void **)&(pAuth->pRealm));
	}

	if (pAuth->pNonce != NULL)
	{
		pal_MemoryFree((void **)&(pAuth->pNonce));
	}

	if (pAuth->pOpaque != NULL)
	{
		pal_MemoryFree((void **)&(pAuth->pOpaque));
	}

	if (pAuth->pURI != NULL)
	{
		pal_MemoryFree((void **)&(pAuth->pURI));
	}

	if (pAuth->pResponse != NULL)
	{
		pal_MemoryFree(
			(void **)&(pAuth->pResponse));
	}

	if (pAuth->pCNonce != NULL)
	{
		pal_MemoryFree((void **)&(pAuth->pCNonce));
	}

	if (pAuth->pQoP != NULL)
	{
		pal_MemoryFree((void **)&(pAuth->pQoP));
	}

	if (pAuth->pCountNonce != NULL)
	{
		pal_MemoryFree((void **)&(pAuth->pCountNonce));
	}

	if (pAuth->pAuts != NULL)
	{
		pal_MemoryFree((void **)&(pAuth->pAuts));
	}

	if (pAuth->pAKARes != NULL)
	{
		_EcrioSigMgrReleaseAKAStruct(pSigMgr, pAuth->pAKARes);
		pal_MemoryFree((void **)&(pAuth->pAKARes));
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:		_EcrioSigMgrReleaseAuthenticationStruct ()

Purpose:		to release the members of the EcrioSigMgrSipURIStruct.

Description:	to release the members of the EcrioSigMgrSipURIStruct.

Input:			EcrioSigMgrSipURIStruct* pSipUri - SIP URI struct.

Output:			None

Returns:		None

**************************************************************************/
u_int32 _EcrioSigMgrReleaseAuthenticationStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrAuthenticationStruct *pAuth
)
{
	// @todo Do we need pSigMgr parameter in this function?
	(void)pSigMgr;

	if (!pAuth)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pAuth->pAuthenticationScheme != NULL)
	{
		pal_MemoryFree(
			(void **)&(pAuth->pAuthenticationScheme));
	}

	if (pAuth->pRealm != NULL)
	{
		pal_MemoryFree((void **)&(pAuth->pRealm));
	}

	if (pAuth->pDomain != NULL)
	{
		pal_MemoryFree((void **)&(pAuth->pDomain));
	}

	if (pAuth->pNonce != NULL)
	{
		pal_MemoryFree((void **)&(pAuth->pNonce));
	}

	if (pAuth->pOpaque != NULL)
	{
		pal_MemoryFree((void **)&(pAuth->pOpaque));
	}

	if (pAuth->ppListOfQoP != NULL)
	{
		u_int32 i = 0;

		for (i = 0; i < pAuth->countQoP; ++i)
		{
			pal_MemoryFree((void **)(&pAuth->ppListOfQoP[i]));
		}

		pal_MemoryFree((void **)&(pAuth->ppListOfQoP));
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

u_int32 _EcrioSigMgrReleaseAKAStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSipAuthStruct *pAKA
)
{
	// @todo Do we need pSigMgr parameter in this function?
	(void)pSigMgr;

	if (!pAKA)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pAKA->pAutn != NULL)
	{
		pal_MemoryFree(
			(void **)&(pAKA->pAutn));
	}

	if (pAKA->pAuts != NULL)
	{
		pal_MemoryFree((void **)&(pAKA->pAuts));
	}

	if (pAKA->pCK != NULL)
	{
		pal_MemoryFree((void **)&(pAKA->pCK));
	}

	if (pAKA->pIK != NULL)
	{
		pal_MemoryFree((void **)&(pAKA->pIK));
	}

	if (pAKA->pRand != NULL)
	{
		pal_MemoryFree((void **)&(pAKA->pRand));
	}

	if (pAKA->pRes != NULL)
	{
		pal_MemoryFree((void **)&(pAKA->pRes));
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

u_int32 _EcrioSigMgrReleaseCommonInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrCommonInfoStruct *pCmnInfo
)
{
	// @todo Do we need pSigMgr parameter in this function?
	(void)pSigMgr;

	if (!pCmnInfo)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pCmnInfo->pBranch != NULL)
	{
		pal_MemoryFree((void **)&(pCmnInfo->pBranch));
	}

	if (pCmnInfo->pCallId != NULL)
	{
		pal_MemoryFree((void **)&(pCmnInfo->pCallId));
	}

	if (pCmnInfo->pFromTag != NULL)
	{
		pal_MemoryFree((void **)&(pCmnInfo->pFromTag));
	}

	if (pCmnInfo->pMethodName != NULL)
	{
		pal_MemoryFree((void **)&(pCmnInfo->pMethodName));
	}

	if (pCmnInfo->pToTag != NULL)
	{
		pal_MemoryFree((void **)&(pCmnInfo->pToTag));
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

u_int32 _EcrioSigMgrReleaseTXNInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrTXNInfoStruct *pTXNInfo
)
{
	// @todo Do we need pSigMgr parameter in this function?
	(void)pSigMgr;

	if (!pTXNInfo)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pTXNInfo->pMsg != NULL)
	{
		pal_MemoryFree((void **)&(pTXNInfo->pMsg));
	}

	if (pTXNInfo->pSessionMappingStr != NULL)
	{
		pal_MemoryFree((void **)&(pTXNInfo->pSessionMappingStr));
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

#if 0
u_int32 _EcrioSigMgrReleaseSessionHandleStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSessionHandleStruct *pSessionHandle
)
{
	pSigMgr = pSigMgr;	/* Removing level-4 warning */

	if (!pSessionHandle)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	pSessionHandle->pDialogHandle = NULL;
	/*pSessionHandle->pUsagehandle = NULL;*/

	return ECRIO_SIGMGR_NO_ERROR;
}
#endif

u_int32 _EcrioSigMgrReleaseRegisterInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrRegisterInfoStruct *pRegisterInfo
)
{
	pSigMgr = pSigMgr;	/* Removing level-4 warning */

	if (!pRegisterInfo)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pRegisterInfo->registerInformationList != NULL)
	{
		EcrioDSListDestroy(&(pRegisterInfo->registerInformationList));
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

u_int32 _EcrioSigMgrReleaseUserRegisterInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUserRegisterInfoStruct *pUserRegisterInfo
)
{
	if (!pUserRegisterInfo)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pUserRegisterInfo->pRegisterReq != NULL)
	{
		EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_SipMessage,
			(void **)&(pUserRegisterInfo->pRegisterReq), Enum_TRUE);
	}

	if (pUserRegisterInfo->pExpires != NULL)
	{
		pal_MemoryFree((void **)&pUserRegisterInfo->pExpires);
	}

	if (pUserRegisterInfo->pCallIDStruct != NULL)
	{
		if (pUserRegisterInfo->pCallIDStruct->pCallID != NULL)
		{
			pal_MemoryFree((void **)&pUserRegisterInfo->pCallIDStruct->pCallID);
		}

		pal_MemoryFree((void **)&pUserRegisterInfo->pCallIDStruct);
	}

	if (pUserRegisterInfo->reregistrationTimerId != NULL)
	{
		pal_TimerStop(pUserRegisterInfo->reregistrationTimerId);
	}

	if (pUserRegisterInfo->uRegExpireTimerID != NULL)
	{
		pal_TimerStop(pUserRegisterInfo->uRegExpireTimerID);
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

u_int32 _EcrioSigMgrReleaseRegContactInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrRegisterContactInfoStruct *pRegContactInfo
)
{
	if (!pRegContactInfo)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pRegContactInfo->pUserContactInfo != NULL)
	{
		EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_NameAddrWithParams,
			(void **)&(pRegContactInfo->pUserContactInfo), Enum_TRUE);
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

/*	Support for Authorization header in all request	start	*/
u_int32 _EcrioSigMgrReleaseSharedCredentialsStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSharedCredentialsStruct *pSharedCredentials
)
{
	if ((NULL == pSigMgr) || (NULL == pSharedCredentials))
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pSharedCredentials->pAuth)
	{
		_EcrioSigMgrReleaseAuthorizationStruct(pSigMgr,
			(EcrioSigMgrAuthorizationStruct *)pSharedCredentials->pAuth);
		pal_MemoryFree((void **)&(pSharedCredentials->pAuth));
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

/**************************************************************************

Function:		_EcrioSigMgrReleaseMsgBodyStruct ()

Purpose:		to release the members of the EcrioSigMgrSipURIStruct.

Description:	to release the members of the EcrioSigMgrSipURIStruct.

Input:			EcrioSigMgrSipURIStruct* pSipUri - SIP URI struct.

Output:			None

Returns:		None

**************************************************************************/
u_int32 _EcrioSigMgrReleaseMsgBodyStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrMessageBodyStruct *pMsgBody
)
{
	if (!pMsgBody)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pMsgBody->pMessageBody != NULL)
	{
		if ((pMsgBody->messageBodyType == EcrioSigMgrMessageBodyUnknown) ||
			(pMsgBody->messageBodyType == EcrioSigMgrMessageBodySDP))
		{
			EcrioSigMgrUnknownMessageBodyStruct *pUnknownMsgBody = (EcrioSigMgrUnknownMessageBodyStruct *)pMsgBody->pMessageBody;
			if (pUnknownMsgBody->pBuffer != NULL)
			{
				pal_MemoryFree((void **)&pUnknownMsgBody->pBuffer);
			}

			_EcrioSigMgrReleaseHdrValueStruct(pSigMgr, &pUnknownMsgBody->contentType);
			pal_MemoryFree((void **)&pMsgBody->pMessageBody);
		}
	}

	/*if (pMsgBody->pMimeInfo != NULL)
	{
	    _EcrioSigMgrReleaseMimeHdrStruct(pSigMgr, pMsgBody->pMimeInfo);
	    pal_MemoryFree((void**)&pMsgBody->pMimeInfo);
	}

	pMsgBody->messageBodyType = EcrioSigMgrMessageBodyUnknown;
	pMsgBody->pMessageBody = NULL;
	pMsgBody->pMimeInfo = NULL;*/

	return ECRIO_SIGMGR_NO_ERROR;
}

u_int32 _EcrioSigMgrReleaseSipMessage
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessage
)
{
	if (pSipMessage->pMandatoryHdrs)
	{
		_EcrioSigMgrReleaseManHdrStruct(pSigMgr, pSipMessage->pMandatoryHdrs);
		pal_MemoryFree((void **)&pSipMessage->pMandatoryHdrs);
	}

	if (pSipMessage->pReasonPhrase)
	{
		pal_MemoryFree((void **)&pSipMessage->pReasonPhrase);
	}

	if (pSipMessage->pSipVersion)
	{
		pal_MemoryFree((void **)&pSipMessage->pSipVersion);
	}

	if (pSipMessage->pContact != NULL)
	{
		EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_Contact,
			(void **)&(pSipMessage->pContact), Enum_TRUE);
	}

	if (pSipMessage->pOptionalHeaderList)
	{
		EcrioDSListDestroy(&pSipMessage->pOptionalHeaderList);
	}

	if (pSipMessage->pAuthorization != NULL)
	{
		EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_Authorization,
			(void **)&(pSipMessage->pAuthorization), Enum_FALSE);
	}

	if (pSipMessage->pRouteSet != NULL)
	{
		EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_Route,
			(void **)&(pSipMessage->pRouteSet), Enum_TRUE);
	}

	if (pSipMessage->pRecordRouteSet != NULL)
	{
		EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_Route,
			(void **)&(pSipMessage->pRecordRouteSet), Enum_TRUE);
	}

	if (pSipMessage->pExpires != NULL)
	{
		pal_MemoryFree((void **)&pSipMessage->pExpires);
	}

	if (pSipMessage->pAuthentication != NULL)
	{
		EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_Authentication,
			(void **)&(pSipMessage->pAuthentication), Enum_TRUE);
	}

	if (pSipMessage->pMinExpires != NULL)
	{
		pal_MemoryFree((void **)&pSipMessage->pMinExpires);
	}

	if (pSipMessage->pAuthenticationInfo != NULL)
	{
		EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_Header,
			(void **)&pSipMessage->pAuthenticationInfo, Enum_TRUE);
	}

	if (pSipMessage->pContentType != NULL)
	{
		EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_Header,
			(void **)&pSipMessage->pContentType, Enum_TRUE);
	}

	if (pSipMessage->pServiceRoute != NULL)
	{
		EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_Route,
			(void **)&(pSipMessage->pServiceRoute), Enum_TRUE);
	}

	if (pSipMessage->ppPAssociatedURI != NULL)
	{
		u_int32 i = 0;

		for (i = 0; i < pSipMessage->numPAssociatedURIs; ++i)
		{
			EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_NameAddrWithParams,
				(void **)&(pSipMessage->ppPAssociatedURI[i]), Enum_TRUE);
		}

		pal_MemoryFree((void **)&(pSipMessage->ppPAssociatedURI));
	}

	if (pSipMessage->ppPPreferredIdentity != NULL)
	{
		u_int32 i = 0;

		for (i = 0; i < pSipMessage->numPPreferredIdentity; ++i)
		{
			EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_NameAddr,
				(void **)&(pSipMessage->ppPPreferredIdentity[i]), Enum_TRUE);
		}

		pal_MemoryFree((void **)&(pSipMessage->ppPPreferredIdentity));
	}

	if (pSipMessage->ppPAssertedIdentity != NULL)
	{
		u_int32 i = 0;

		for (i = 0; i < pSipMessage->numPAssertedIdentities; ++i)
		{
			EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_NameAddr,
				(void **)&(pSipMessage->ppPAssertedIdentity[i]), Enum_TRUE);
		}

		pal_MemoryFree((void **)&(pSipMessage->ppPAssertedIdentity));
	}

	if (pSipMessage->ppReferredBy != NULL)
	{
		u_int32 i = 0;

		for (i = 0; i < pSipMessage->numReferredBy; ++i)
		{
			EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_NameAddrWithParams,
				(void **)&(pSipMessage->ppReferredBy[i]), Enum_TRUE);
		}

		pal_MemoryFree((void **)&(pSipMessage->ppReferredBy));
	}

	if (pSipMessage->pFetaureTags)
	{
		if (pSipMessage->pFetaureTags->ppFeatureTagHeaderValue != NULL)
		{
			u_int32 i = 0;

			for (i = 0; i < pSipMessage->pFetaureTags->uNumberOfTags; ++i)
			{
				if (pSipMessage->pFetaureTags->ppFeatureTagHeaderValue[i]){
					pal_MemoryFree((void **)&(pSipMessage->pFetaureTags->ppFeatureTagHeaderValue[i]->pParamName));
					pal_MemoryFree((void **)&(pSipMessage->pFetaureTags->ppFeatureTagHeaderValue[i]->pParamValue));

					pal_MemoryFree((void **)&(pSipMessage->pFetaureTags->ppFeatureTagHeaderValue[i]));
				}
			}

			pal_MemoryFree((void **)&(pSipMessage->pFetaureTags->ppFeatureTagHeaderValue));
		}

		pSipMessage->pFetaureTags->uNumberOfTags = 0;

		pal_MemoryFree((void **)&(pSipMessage->pFetaureTags));
	}

	if (pSipMessage->pOptionalHeaderList)
	{
		EcrioSigMgrReleaseOptionalHeaderList(pSigMgr, pSipMessage->pOptionalHeaderList);
		pal_MemoryFree((void **)&(pSipMessage->pOptionalHeaderList));
	}

	if (pSipMessage->pMessageBody)
	{
		_EcrioSigMgrReleaseMsgBodyStruct(pSigMgr, pSipMessage->pMessageBody);
		pal_MemoryFree((void **)&(pSipMessage->pMessageBody));
	}
#if 0
	if (pSipMessage->hSessionHandle)
	{
		/* Free session structures other than invite request
		*/
		if (!(pSipMessage->eMethodType == EcrioSipMessageTypeInvite &&
			pSipMessage->eReqRspType == EcrioSigMgrSIPRequest))
		{
			_EcrioSigMgrInternalStructRelease(pSigMgr, EcrioSigMgrStructType_SessionHandle,
				(void **)&pSipMessage->hSessionHandle, Enum_TRUE);
		}
	}
#endif
	pSipMessage->eMethodType = EcrioSipMessageTypeNone;
	pSipMessage->responseCode = 0;

	return 0;
}

u_int32 _EcrioSigMgrReleaseInviteUsageInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrInviteUsageInfoStruct *pInviteUsageInfoHdr
)
{
	if (pInviteUsageInfoHdr == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pInviteUsageInfoHdr->pBranch != NULL)
	{
		pal_MemoryFree((void **)&pInviteUsageInfoHdr->pBranch);
	}

	if (pInviteUsageInfoHdr->pMsg != NULL)
	{
		pal_MemoryFree((void **)&pInviteUsageInfoHdr->pMsg);
	}

	if (pInviteUsageInfoHdr->pAckMsg != NULL)
	{
		pal_MemoryFree((void **)&pInviteUsageInfoHdr->pAckMsg);
	}

	if (pInviteUsageInfoHdr->pInviteReq != NULL)
	{
		EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_SipMessage,
			(void **)&pInviteUsageInfoHdr->pInviteReq, Enum_TRUE);
	}

	if (pInviteUsageInfoHdr->pPrackDetails)
	{
		if (pInviteUsageInfoHdr->pPrackDetails->pRseqArray)
		{
			pal_MemoryFree((void **)&pInviteUsageInfoHdr->pPrackDetails->pRseqArray);
		}

		pal_MemoryFree((void **)&pInviteUsageInfoHdr->pPrackDetails);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStoping INVITE response TU retransmission timer in state %u ",
		__FUNCTION__, __LINE__, pInviteUsageInfoHdr->callState);
	if (NULL != pInviteUsageInfoHdr->retransmitTimerId)
	{
		pal_TimerStop(pInviteUsageInfoHdr->retransmitTimerId);
		pInviteUsageInfoHdr->retransmitTimerId = NULL;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStoping INVITE response TU timeout timer in state %u",
		__FUNCTION__, __LINE__, pInviteUsageInfoHdr->callState);
	if (NULL != pInviteUsageInfoHdr->expireTimerId)
	{
		pal_TimerStop(pInviteUsageInfoHdr->expireTimerId);
		pInviteUsageInfoHdr->expireTimerId = NULL;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStoping INVITE 1xx response TU periodic timer in state %u",
		__FUNCTION__, __LINE__, pInviteUsageInfoHdr->callState);
	if (NULL != pInviteUsageInfoHdr->periodic1xxTimerId)
	{
		pal_TimerStop(pInviteUsageInfoHdr->periodic1xxTimerId);
		pInviteUsageInfoHdr->periodic1xxTimerId = NULL;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStoping INVITE sssion expire timer in state %u",
		__FUNCTION__, __LINE__, pInviteUsageInfoHdr->callState);
	if (NULL != pInviteUsageInfoHdr->sessionExpireTimerId)
	{
		pal_TimerStop(pInviteUsageInfoHdr->sessionExpireTimerId);
		pInviteUsageInfoHdr->sessionExpireTimerId = NULL;
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

u_int32 _EcrioSigMgrReleaseSubscribeUsageInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrSubscribeUsageInfoStruct *pSubscribeUsageInfoHdr
)
{
	if (pSubscribeUsageInfoHdr == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	if (pSubscribeUsageInfoHdr->pBranch != NULL)
	{
		pal_MemoryFree((void **)&pSubscribeUsageInfoHdr->pBranch);
	}

	if (pSubscribeUsageInfoHdr->pSubscribeReq != NULL)
	{
		EcrioSigMgrStructRelease(pSigMgr, EcrioSigMgrStructType_SipMessage,
			(void **)&pSubscribeUsageInfoHdr->pSubscribeReq, Enum_TRUE);
	}

	if (NULL != pSubscribeUsageInfoHdr->refreshTimerId)
	{
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tStoping SUBSCRIBE refresh timer ",
			__FUNCTION__, __LINE__);
		pal_TimerStop(pSubscribeUsageInfoHdr->refreshTimerId);
		pSubscribeUsageInfoHdr->refreshTimerId = NULL;
	}

	return ECRIO_SIGMGR_NO_ERROR;
}

u_int32	_EcrioSigMgrReleaseDialogNodeStruct
(
	EcrioSigMgrStruct *pSigMgr,
	_EcrioSigMgrDialogNodeStruct *pDialogNode
)
{
	_EcrioSigMgrForkedMessageListNodeStruct *pForkedMsgHead = NULL;
	_EcrioSigMgrForkedMessageListNodeStruct *pTempNode = NULL;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	if (pDialogNode == NULL)
	{
		return ECRIO_SIGMGR_INSUFFICIENT_DATA;
	}

	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\ttimerMId = %x",
		 __FUNCTION__, __LINE__, pDialogNode->timerMId);
	if (pDialogNode->timerMId)
	{
		pal_TimerStop(pDialogNode->timerMId);
		pDialogNode->timerMId = NULL;
	}

	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpCallId = %x",
		 __FUNCTION__, __LINE__, pDialogNode->pCallId);
	if (pDialogNode->pCallId)
	{
		pal_MemoryFree((void **)&pDialogNode->pCallId);
	}

	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpLocalContact = %x",
		 __FUNCTION__, __LINE__, pDialogNode->pLocalContact);
	if (pDialogNode->pLocalContact)
	{
		_EcrioSigMgrReleaseContactStruct(pSigMgr, pDialogNode->pLocalContact);
		pal_MemoryFree((void **)&pDialogNode->pLocalContact);
	}

	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpLocalTag = %x",
		 __FUNCTION__, __LINE__, pDialogNode->pLocalTag);
	if (pDialogNode->pLocalTag)
	{
		pal_MemoryFree((void **)&pDialogNode->pLocalTag);
	}

	pForkedMsgHead = pDialogNode->pForkedMsgHead;
	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpForkedMsgHead = %x",
		 __FUNCTION__, __LINE__, pForkedMsgHead);
	while (pForkedMsgHead != NULL)
	{
		SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\t\tpRemoteContact = %x",
			 __FUNCTION__, __LINE__, pForkedMsgHead->pRemoteContact);
		if (pForkedMsgHead->pRemoteContact)
		{
			_EcrioSigMgrReleaseContactStruct(pSigMgr, pForkedMsgHead->pRemoteContact);
			pal_MemoryFree((void **)&pForkedMsgHead->pRemoteContact);
		}

		SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\t\tpRemoteTag = %x",
			 __FUNCTION__, __LINE__, pForkedMsgHead->pRemoteTag);
		if (pForkedMsgHead->pRemoteTag)
		{
			pal_MemoryFree((void **)&pForkedMsgHead->pRemoteTag);
		}

		pTempNode = pForkedMsgHead;
		pForkedMsgHead = pTempNode->pNext;
		pal_MemoryFree((void **)&pTempNode);
	}

	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpLocalUri = %x",
		 __FUNCTION__, __LINE__, pDialogNode->pLocalUri);
	if (pDialogNode->pLocalUri)
	{
		_EcrioSigMgrReleaseNameAddrWithParamsStruct(pSigMgr, pDialogNode->pLocalUri);
		pal_MemoryFree((void **)&pDialogNode->pLocalUri);
	}

	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpRemoteUri = %x",
		 __FUNCTION__, __LINE__, pDialogNode->pRemoteUri);
	if (pDialogNode->pRemoteUri)
	{
		_EcrioSigMgrReleaseNameAddrWithParamsStruct(pSigMgr, pDialogNode->pRemoteUri);
		pal_MemoryFree((void **)&pDialogNode->pRemoteUri);
	}

	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpRouteSet = %x",
		 __FUNCTION__, __LINE__, pDialogNode->pRouteSet);
	if (pDialogNode->pRouteSet)
	{
		_EcrioSigMgrReleaseRouteStruct(pSigMgr, pDialogNode->pRouteSet);
		pal_MemoryFree((void **)&pDialogNode->pRouteSet);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);

	return ECRIO_SIGMGR_NO_ERROR;
}
