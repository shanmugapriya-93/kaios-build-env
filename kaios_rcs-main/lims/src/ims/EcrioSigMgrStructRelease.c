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

#include "EcrioSigMgrCommon.h"
#include "EcrioSigMgrInit.h"
#include "EcrioSigMgrInternal.h"
#include "EcrioSigMgrRegister.h"
#include "EcrioSigMgrRegisterInternal.h"
#include "EcrioSigMgrInstantMessage.h"
#include "EcrioSigMgrInstantMessageInternal.h"

/*****************************************************************************
                Internal API Declaration Section - Begin
*****************************************************************************/

/*****************************************************************************
                Internal API Declaration Section - End
*****************************************************************************/

/*****************************************************************************
                    API Definition Section - Begin
*****************************************************************************/

/*****************************************************************************

Function:		EcrioSigMgrStructRelease ()

Purpose:		to release the members of the provided structure

Description:	This function interprets the incoming structure based on the
                enum parameter that indicates the type of structure. The
                function then release the members of the structure.

Input:			void* ppStruct - pointer to the structure to be de-allocated
                EcrioVoIPTkStructEnum type - enumerated value that indicates
                the type of structure.
                BoolEnum release - if set to Enum_TRUE shall de-allocate
                the pointer memory too.

Output:			None

Returns:		Error Code

*****************************************************************************/
u_int32 EcrioSigMgrStructRelease
(
	SIGMGRHANDLE pSigMgrHandle,
	EcrioSigMgrStructTypeEnum structType,
	void **ppStruct,
	BoolEnum release
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrStruct *pSigMgrStruct = NULL;

	if (pSigMgrHandle == NULL)
	{
		error = ECRIO_SIGMGR_BAD_HANDLE;
		return error;
	}

	pSigMgrStruct = (EcrioSigMgrStruct *)pSigMgrHandle;

	if ((ppStruct == NULL) || (*ppStruct == NULL))
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto Error_Level_01;
	}

	if ((release != Enum_TRUE) &&
		(release != Enum_FALSE))
	{
		error = ECRIO_SIGMGR_INVALID_DATA;
		goto Error_Level_01;
	}

	switch (structType)
	{
		case EcrioSigMgrStructType_InitStruct:
		{
			EcrioSigMgrInitStruct *pSigMgrInitStruct =
				(EcrioSigMgrInitStruct *)(*ppStruct);
			pSigMgrInitStruct->pLogEngineHandle = NULL;
		}
		break;

		case EcrioSigMgrStructType_SignalingInfo:
		{
			_EcrioSigMgrReleaseSignalingInfoStruct(pSigMgrStruct,
				(EcrioSigMgrSignalingInfoStruct *)(*ppStruct));
		}
		break;

		case EcrioSigMgrStructType_SignalingChangeParamInfo:
		{
			_EcrioSigMgrReleaseSignalingChangeParamInfoStruct(pSigMgrStruct,
				(EcrioSigMgrSignalingChangeParamInfoStruct *)(*ppStruct));
		}
		break;

		case EcrioSigMgrStructType_EngineCallback:
		{
			/*_EcrioSigMgrInitEngineCBStruct(pSigMgrStruct,
			    (EcrioSigMgrEngineCallbackStruct*) (*ppStruct));*/
			((EcrioSigMgrEngineCallbackStruct *)(*ppStruct))->getPropertyCallbackFn = NULL;
			((EcrioSigMgrEngineCallbackStruct *)(*ppStruct))->infoCallbackFn = NULL;
			((EcrioSigMgrEngineCallbackStruct *)(*ppStruct))->pCallbackData = NULL;
			((EcrioSigMgrEngineCallbackStruct *)(*ppStruct))->statusCallbackFn = NULL;
		}
		break;

		/*case EcrioSigMgrStructType_Info:
		    _EcrioSigMgrReleaseInfoStruct(pSigMgrStruct,
		    (EcrioSigMgrInfoStruct *)(*ppStruct));
		    break;

		case EcrioSigMgrStructType_Message:
		    _EcrioSigMgrReleaseMessageStruct(pSigMgrStruct,
		    (EcrioSigMgrMessageStruct *)(*ppStruct));
		    break;

		case EcrioSigMgrStructType_IPAddr:
		    _EcrioSigMgrReleaseIPAddrStruct(pSigMgrStruct,
		    (EcrioSigMgrIPAddrStruct *)(*ppStruct));
		    break;*/

		case EcrioSigMgrStructType_Params:
		{
			_EcrioSigMgrReleaseParamsStruct(pSigMgrStruct,
				(EcrioSigMgrParamStruct *)(*ppStruct));
		}
		break;

		/*case EcrioSigMgrStructType_SipURI:
		    _EcrioSigMgrReleaseSipUriStruct(pSigMgrStruct,
		    (EcrioSigMgrSipURIStruct *)(*ppStruct));
		    break;

		case EcrioSigMgrStructType_Uri:
		    _EcrioSigMgrReleaseUriStruct(pSigMgrStruct,
		    (EcrioSigMgrUriStruct *)(*ppStruct));
		    break;*/

		case EcrioSigMgrStructType_NameAddr:
		{
			_EcrioSigMgrReleaseNameAddrStruct(pSigMgrStruct,
				(EcrioSigMgrNameAddrStruct *)(*ppStruct));
		}
		break;

		case EcrioSigMgrStructType_NameAddrWithParams:
		{
			_EcrioSigMgrReleaseNameAddrWithParamsStruct(pSigMgrStruct,
				(EcrioSigMgrNameAddrWithParamsStruct *)(*ppStruct));
		}
		break;

		/*case EcrioSigMgrStructType_HeaderValues:
		    _EcrioSigMgrReleaseHdrValueStruct(pSigMgrStruct,
		    (EcrioSigMgrHeaderValueStruct *)(*ppStruct));
		    break;*/

		case EcrioSigMgrStructType_Header:
		{
			_EcrioSigMgrReleaseHdrStruct(pSigMgrStruct,
				(EcrioSigMgrHeaderStruct *)(*ppStruct));
		}
		break;

		/*case EcrioSigMgrStructType_MandatoryHeaders:
		    _EcrioSigMgrReleaseManHdrStruct(pSigMgrStruct,
		    (EcrioSigMgrMandatoryHeaderStruct *)(*ppStruct));
		    break;*/

		case EcrioSigMgrStructType_Route:
		{
			_EcrioSigMgrReleaseRouteStruct(pSigMgrStruct,
				(EcrioSigMgrRouteStruct *)(*ppStruct));
		}
		break;

		case EcrioSigMgrStructType_Contact:
		{
			_EcrioSigMgrReleaseContactStruct(pSigMgrStruct,
				(EcrioSigMgrContactStruct *)(*ppStruct));
		}
		break;

		case EcrioSigMgrStructType_Via:
		{
			_EcrioSigMgrReleaseViaStruct(pSigMgrStruct,
				(EcrioSigMgrViaStruct *)(*ppStruct));
		}
		break;

		case EcrioSigMgrStructType_Authorization:
		{
			_EcrioSigMgrReleaseAuthorizationStruct(pSigMgrStruct,
				(EcrioSigMgrAuthorizationStruct *)(*ppStruct));
		}
		break;

		case EcrioSigMgrStructType_AKA:
		{
			_EcrioSigMgrReleaseAKAStruct(pSigMgrStruct,
				(EcrioSipAuthStruct *)(*ppStruct));
		}
		break;

		case EcrioSigMgrStructType_Authentication:
		{
			_EcrioSigMgrReleaseAuthenticationStruct(pSigMgrStruct,
				(EcrioSigMgrAuthenticationStruct *)(*ppStruct));
		}
		break;

		/*case EcrioSigMgrStructType_MessageBody:
		    _EcrioSigMgrReleaseMsgBodyStruct(pSigMgrStruct,
		        (EcrioSigMgrMessageBodyStruct*)(*ppStruct));
		    break;*/

		/*	Support for Authorization header in all request	start	*/
		case EcrioSigMgrStructType_SharedCredentials:
		{
			_EcrioSigMgrReleaseSharedCredentialsStruct(pSigMgrStruct,
				(EcrioSigMgrSharedCredentialsStruct *)(*ppStruct));
		}
		break;

		case EcrioSigMgrStructType_SipMessage:
		{
			_EcrioSigMgrReleaseSipMessage(pSigMgrStruct,
				(EcrioSigMgrSipMessageStruct *)(*ppStruct));
		}
		break;

		/*	Support for Authorization header in all request	end	*/
		default:
			error = ECRIO_SIGMGR_STRUCT_TYPE_UNDEFINED;
			goto Error_Level_01;
	}

	if (release == Enum_TRUE)
	{
		pal_MemoryFree((void **)ppStruct);
	}

Error_Level_01:

	return error;
}

/*****************************************************************************
                    API Definition Section - End
*****************************************************************************/
