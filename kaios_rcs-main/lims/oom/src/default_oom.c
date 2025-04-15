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
 * @file default_oom.c
 * @brief Implementation of the default Operator Object Model.
 */

#include "EcrioOOM.h"
#include "EcrioPAL.h"

/**
*
*/
/* internal function */
static u_int16 ec_default_oom_GetRegRetryTime
(
	u_int8 uRetryCount
);

/**
* This function is used to retrieve the SIP timer values T1, T2 and F.
*
* @param[in] pTimers		Pointer to EcrioSipTimerStruct.
*
* @return Returns KOOMErrorNone if successful, otherwise an error
* specific to the oom implementation.
*/
static u_int32 ec_oom_GetSipTimers
(
	EcrioSipTimerStruct *pTimers
)
{
	u_int32 uError = KOOMErrorNone;
	if(pTimers != NULL) {
		pTimers->uTimerT1 = 2000;                             /* 2 seconds as per 24.229 section 7.7 and table 7.7.1.	*/
		pTimers->uTimerT2 = 16000;                            /* 16 seconds as per 24.229 section 7.7 and table 7.7.1.	*/
		pTimers->uTimerF = 64 * pTimers->uTimerT1;            /* 64*T1 as per 24.229 section 7.7 and table 7.7.1. */
	} else
	{
		uError = KOOMInvalidParameters;
	}
	return uError;
}

/**
* This function is used to retrieve the SIP Session Expire timer values.
*
* @param[in] pTimers		Pointer to EcrioSipSessionTimerStruct.
*
* @return Returns KOOMErrorNone if successful, otherwise an error
* specific to the oom implementation.
*/
static u_int32 ec_oom_GetSessionTimers
(
	EcrioSipSessionTimerStruct *pTimers
)
{
	u_int32 uError = KOOMErrorNone;

	/* Values are in seconds*/
	if(pTimers != NULL) {
		pTimers->uTimerMinSE = 90;                       /* RECOMMENDED value based on RFC 4028. */
		pTimers->uTimerSE = 600;                        /* RECOMMENDED value based on RFC 4028. */
	} else{
		uError = KOOMInvalidParameters;
	}
	return uError;
}

/**
* This function is used to retrieve the VoLTE feature tags to be included in
* Contact header.
*
* @param[in] eMethod		Enum of type EcrioSipMessageTypeEnum.
* @param[in] eHeader		Enum of type EcrioSipHeaderTypeEnum.
* @param[in] pFeatureTag	Pointer of type u_char to hold feature tags.
* @param[in] pSize			Pointer of type u_int32 to hold size of feature tags.
*
* @return Returns KOOMErrorNone if successful, otherwise an error
* specific to the oom implementation.
*/
static u_int32 ec_oom_GetVolteFeatureTags
(
	EcrioSipMessageTypeEnum eMethod,
	EcrioSipHeaderTypeEnum eHeader,
	u_char *pFeatureTag,
	u_int32 *pSize
)
{
	u_int32 uError = KOOMErrorNone;

	if(pFeatureTag == NULL || pSize == NULL)
	{
		return KOOMInvalidParameters;
	}

	if (eMethod == EcrioSipMessageTypeRegister && eHeader == EcrioSipHeaderTypeContact)
	{
		u_int32 uSize = pal_StringLength((const u_char *)"+g.gsma.rcs.telephony=\"cs,volte\"");

		// first check the allocated size. If it is larger or equal to
		if (*pSize >= uSize)
		{
			/* +g.gsma.rcs.telephony=\"cs, volte\" feature tag will be added only when network support VoLTE */
			if (NULL == pal_StringNCopy(pFeatureTag, (*pSize)+1, (const u_char *)"+g.gsma.rcs.telephony=\"cs,volte\"", pal_StringLength((const u_char *)"+g.gsma.rcs.telephony=\"cs,volte\"")))
			{
				return KOOMInvalidSize;
			}
			//pal_StringNCopy(pFeatureTag, (const u_char *)"+g.gsma.rcs.telephony=\"none\"");
			*pSize = uSize;
		}
		else
		{
			*pSize = uSize;
			uError = KOOMInvalidSize;
		}
	}

	return uError;
}

/**
* This function is used to retrieve the any other feature tags to be included in
* Contact header specific to ATT.
*
* @param[in] eMethod		Enum of type EcrioSipMessageTypeEnum.
* @param[in] eHeader		Enum of type EcrioSipHeaderTypeEnum.
* @param[in] pFeatureTag	Pointer of type u_char to hold feature tags.
* @param[in] pSize			Pointer of type u_int32 to hold size of feature tags.
*
* @return Returns KOOMErrorNone if successful, otherwise an error
* specific to the oom implementation.
*/
static u_int32 ec_ooms_GetOtherFeatureTags
(
	EcrioSipMessageTypeEnum eMethod,
	EcrioSipHeaderTypeEnum eHeader,
	u_char *pFeatureTag,
	u_int32 *pSize
)
{
	u_int32 uError = KOOMErrorNone;
	(void)eMethod;
	(void)eHeader;
	(void)pFeatureTag;

	*pSize = 0;
	return uError;
}

/**
* This function is used to check if specific header need to be included to a
* SIP message or not.
*
* @param[in] eMethod		Enum of type EcrioSipMessageTypeEnum.
* @param[in] eHeader		Enum of type EcrioSipHeaderTypeEnum.
*
* @return Returns Enum_TRUE or Enum_FALSE based on if specific header need
* to be included or not.
*/
static BoolEnum ec_oom_HasHeaderSupport
(
	EcrioSipMessageTypeEnum eMethod,
	EcrioSipHeaderTypeEnum eHeader
)
{
	BoolEnum bHasHeader = Enum_FALSE;

	/* Header gets the preference */
	switch (eHeader)
	{
		case EcrioSipHeaderTypeP_AccessNetworkInfo:
		{
			switch (eMethod)
			{
				case EcrioSipMessageTypeRegister:
				case EcrioSipMessageTypeInvite:
				case EcrioSipMessageTypeMessage:
				case EcrioSipMessageTypeBye:
				case EcrioSipMessageTypeSubscribe:
				case EcrioSipMessageTypePublish:
				case EcrioSipMessageTypeInvite_1xx:
				case EcrioSipMessageTypeInvite_2xx:
				case EcrioSipMessageTypeInvite_4xx:
				case EcrioSipMessageTypeInvite_6xx:
				{
					bHasHeader = Enum_TRUE;
				}
				break;

				default:
				{
					bHasHeader = Enum_FALSE;
				}
				break;
			}
		}
		break;

		case EcrioSipHeaderTypeReason:
		{
			switch (eMethod)
			{
				case EcrioSipMessageTypeBye:
				case EcrioSipMessageTypeCancel:
				{
					bHasHeader = Enum_TRUE;
				}
				break;

				default:
				{
					bHasHeader = Enum_FALSE;
				}
				break;
			}
		}
		break;

		case EcrioSipHeaderTypeP_PreferredService:
		{
			switch (eMethod)
			{
				case EcrioSipMessageTypeMessage:
				case EcrioSipMessageTypeInvite:
				{
					bHasHeader = Enum_TRUE;
				}
				break;

				default:
				{
					bHasHeader = Enum_FALSE;
				}
				break;
			}
		}
		break;

		case EcrioSipHeaderTypeP_EarlyMedia:
		{
			switch (eMethod)
			{
			case EcrioSipMessageTypeInvite:
			{
				bHasHeader = Enum_TRUE;
			}
			break;

			default:
			{
				bHasHeader = Enum_FALSE;
			}
			break;
			}
		}
		break;

		case EcrioSipHeaderTypeContributionID:
		{
			switch (eMethod)
			{
				case EcrioSipMessageTypeMessage:
				case EcrioSipMessageTypeInvite:
				case EcrioSipMessageTypeInvite_2xx:
				{
					bHasHeader = Enum_FALSE;
				}
				break;

				default:
				{
					bHasHeader = Enum_FALSE;
				}
				break;
			}
		}
		break;

		case EcrioSipHeaderTypeConversationID:
		{
			switch (eMethod)
			{
				case EcrioSipMessageTypeMessage:
				case EcrioSipMessageTypeInvite:
				case EcrioSipMessageTypeInvite_2xx:
				{
					bHasHeader = Enum_FALSE;
				}
				break;

				default:
				{
					bHasHeader = Enum_FALSE;
				}
				break;
			}
		}
		break;

		case EcrioSipHeaderTypeAcceptContact:
		{
			switch (eMethod)
			{
				case EcrioSipMessageTypeMessage:
				case EcrioSipMessageTypeInvite:
				case EcrioSipMessageTypeSubscribe:
				{
					bHasHeader = Enum_TRUE;
				}
				break;

				default:
				{
					bHasHeader = Enum_FALSE;
				}
				break;
			}
		}
		break;

		case EcrioSipHeaderTypeInReplyToContributionID:
		{
			switch (eMethod)
			{
				case EcrioSipMessageTypeMessage:
				case EcrioSipMessageTypeInvite:
				case EcrioSipMessageTypeInvite_2xx:
				{
					bHasHeader = Enum_FALSE;
				}
				break;

				default:
				{
					bHasHeader = Enum_FALSE;
				}
				break;
			}
		}
		break;

		case EcrioSipHeaderTypeSessionExpires:
		{
			switch (eMethod)
			{
				case EcrioSipMessageTypeInvite:
				case EcrioSipMessageTypeInvite_2xx:
				case EcrioSipMessageTypeUpdate:
				case EcrioSipMessageTypeUpdate_2xx:
				{
					bHasHeader = Enum_TRUE;
				}
				break;

				default:
				{
					bHasHeader = Enum_FALSE;
				}
				break;
			}
		}
		break;

		case EcrioSipHeaderTypeMinSE:
		{
			switch (eMethod)
			{
				case EcrioSipMessageTypeInvite:
				case EcrioSipMessageTypeInvite_2xx:
				case EcrioSipMessageTypeMessage:
				{
					bHasHeader = Enum_TRUE;
				}
				break;

				default:
				{
					bHasHeader = Enum_FALSE;
				}
				break;
			}
		}
		break;

		case EcrioSipHeaderTypeRequestDisposition:
		{
			switch (eMethod)
			{
				case EcrioSipMessageTypeMessage:
				{
					bHasHeader = Enum_TRUE;
				}
				break;

				default:
				{
					bHasHeader = Enum_FALSE;
				}
				break;
			}
		}
		break;

		case EcrioSipHeaderTypeContentTransferEncoding:
		{
			switch (eMethod)
			{
				case EcrioSipMessageTypeMessage:
				{
					bHasHeader = Enum_TRUE;
				}
				break;

				default:
				{
					bHasHeader = Enum_FALSE;
				}
				break;
			}
		}
		break;

		case EcrioSipHeaderTypeUserAgent:
		{
			switch (eMethod)
			{
				case EcrioSipMessageTypeRegister:
				case EcrioSipMessageTypeInvite:
				case EcrioSipMessageTypeMessage:
				case EcrioSipMessageTypeBye:
				case EcrioSipMessageTypeSubscribe:
				case EcrioSipMessageTypePublish:
				case EcrioSipMessageTypeCancel:
				case EcrioSipMessageTypeUpdate:
				{
					bHasHeader = Enum_TRUE;
				}
				break;

				default:
				{
					bHasHeader = Enum_FALSE;
				}
				break;
			}
		}
		break;
		
		default:
		{
			bHasHeader = Enum_FALSE;
		}
		break;
	}

	return bHasHeader;
}

/**
* This function is used to retrieve list of methods to be included in Allow header.
*
* @return Returns enumerated list of methods to be included in allow header.
*/
static u_int32 ec_oom_GetAllowMethod
(
	void
)
{
	return EcrioSipMessageTypeRegister | EcrioSipMessageTypeInvite |
		   EcrioSipMessageTypeNotify | EcrioSipMessageTypeBye |
		   EcrioSipMessageTypeCancel |
		   EcrioSipMessageTypeAck | EcrioSipMessageTypeUpdate |
		   EcrioSipMessageTypeMessage | EcrioSipMessageTypeOptions;
}

/**
* This function is used to retrieve the retry timer value to be used in case
* of REGISTER failures.
*
* @param[in] stRetryQueryStruct		EcrioOOMRetryQueryStruct.
* @param[in] pRegRetryValuesStruct		Pointer to EcrioOOMRetryValuesStruct.
*
* @return Returns KOOMErrorNone if successful, otherwise an error
* specific to the oom implementation.
*/
static u_int32 ec_oom_GetRegRetryValues
(
	EcrioOOMRetryQueryStruct stRetryQueryStruct,
	EcrioOOMRetryValuesStruct *pRegRetryValuesStruct
)
{
	/* Return invalid input for NULL pointer */
	/*Register error handling is not defined in RCC v7 document so we are working as vzw requirement*/
	if (!pRegRetryValuesStruct)
	{
		return KOOMInvalidParameters;
	}

	switch (stRetryQueryStruct.uResponseCode)
	{
		/* 4xx */
	case 400:
	case 402:
	case 421:
	case 484:
	{
		if (stRetryQueryStruct.uRetryCount == 2)
		{
			pRegRetryValuesStruct->eRetryType = EcrioOOMRetry_None;
			pRegRetryValuesStruct->uRetryInterval = 0;

			return KOOMErrorNone;
		}

		if (stRetryQueryStruct.bIsInitialRequest == Enum_TRUE)
		{
			pRegRetryValuesStruct->eRetryType = EcrioOOMRetry_InitialRequest_NextPCSCF;
		}
		else if (stRetryQueryStruct.bIsInitialRequest == Enum_FALSE)
		{
			pRegRetryValuesStruct->eRetryType = EcrioOOMRetry_ReRequest_SamePCSCF;
		}
		else
		{
			return KOOMInvalidParameters;
		}

		pRegRetryValuesStruct->uRetryInterval = ec_default_oom_GetRegRetryTime(stRetryQueryStruct.uRetryCount);
	}
	break;

	case 403:
	case 404:
	{
		/*TODO: IMSI based registration error handling*/
		if (stRetryQueryStruct.uRetryCount == 1)
		{
			/* Assuming MSISDN URI based registration is failed. The lims will inform registration failed to the application.
			The application will call lims_deinit first then it will call lims_Init API using IMSI based URI and send register
			using lims_register API. */
			pRegRetryValuesStruct->eRetryType = EcrioOOMRetry_None;
			pRegRetryValuesStruct->uRetryInterval = 0;

			return KOOMErrorNone;
		}

		/*TODO: IMSI based registration.*/
	}
	break;

	case 481:
	{
		if (stRetryQueryStruct.bIsInitialRequest == Enum_FALSE)
		{
			pRegRetryValuesStruct->eRetryType = EcrioOOMRetry_None;
			pRegRetryValuesStruct->uRetryInterval = 0;

			return KOOMErrorNone;
		}

		pRegRetryValuesStruct->eRetryType = EcrioOOMRetry_InitialRequest_NextPCSCF;
		pRegRetryValuesStruct->uRetryInterval = ec_default_oom_GetRegRetryTime(stRetryQueryStruct.uRetryCount);
	}
	break;

	/* Other 3xx - 6xx  */
	default:
	{
		if (stRetryQueryStruct.uResponseCode >= 300 && stRetryQueryStruct.uResponseCode < 700)
		{
			/*480, 482, 486, 491, 494, 500, 503, 504, 600 responses*/
			if (stRetryQueryStruct.bIsInitialRequest == Enum_TRUE)
			{
				pRegRetryValuesStruct->eRetryType = EcrioOOMRetry_InitialRequest_NextPCSCF;
			}
			else if (stRetryQueryStruct.bIsInitialRequest == Enum_FALSE)
			{
				/*If reregister is failed 2nd times then send initial register to next PCSCF*/

				if (stRetryQueryStruct.uRetryCount < 2)
				{
					pRegRetryValuesStruct->eRetryType = EcrioOOMRetry_ReRequest_SamePCSCF;
				}
				else
				{
					pRegRetryValuesStruct->eRetryType = EcrioOOMRetry_InitialRequest_NextPCSCF;
				}
			}
			else
			{
				return KOOMInvalidParameters;
			}

			/*480, 482, 486, 491, 494, 500, 503, 504, 600: the lims will overwrite this interval
			with Retry-After value.*/
			pRegRetryValuesStruct->uRetryInterval = ec_default_oom_GetRegRetryTime(stRetryQueryStruct.uRetryCount);
		}
		else
		{
			return KOOMInvalidParameters;
		}
	}
	break;
	}	/* switch */

	return KOOMErrorNone;
}

/**
* This function is used to retrieve the retry timer value to be used in case
* of SUBSCRIBE failures.
*
* @param[in] stRetryQueryStruct		EcrioOOMRetryQueryStruct.
* @param[in] pSubRetryValuesStruct		Pointer to EcrioOOMRetryValuesStruct.
*
* @return Returns KOOMErrorNone if successful, otherwise an error
* specific to the oom implementation.
*/
static u_int32 ec_oom_GetSubRetryValues
(
	EcrioOOMRetryQueryStruct stRetryQueryStruct,
	EcrioOOMRetryValuesStruct *pSubRetryValuesStruct
)
{
	/* Return invalid input for NULL pointer */
	/*Register error handling is not defined in RCC v7 document so we are working as vzw requirement*/
	if (!pSubRetryValuesStruct)
	{
		return KOOMInvalidParameters;
	}

	switch (stRetryQueryStruct.uResponseCode)
	{
		/*
		The client will send a Subscribe request to the same pcscf address after a
		timer interval value according to the "Default Retry timer interval" table.
		*/
	case 400:
	case 403:
	case 404:
	case 420:
	{
		if (stRetryQueryStruct.uRetryCount == 1)
		{
			if (stRetryQueryStruct.bIsInitialRequest == Enum_TRUE)
			{
				pSubRetryValuesStruct->eRetryType = EcrioOOMRetry_InitialRequest_SamePCSCF;
			}
			else if (stRetryQueryStruct.bIsInitialRequest == Enum_FALSE)
			{
				pSubRetryValuesStruct->eRetryType = EcrioOOMRetry_ReRequest_SamePCSCF;
			}
			else
			{
				return KOOMInvalidParameters;
			}

			pSubRetryValuesStruct->uRetryInterval = ec_default_oom_GetRegRetryTime(stRetryQueryStruct.uRetryCount);
		}
		else
		{
			if (stRetryQueryStruct.bIsInitialRequest == Enum_TRUE)
			{
				pSubRetryValuesStruct->eRetryType = EcrioOOMRetry_InitialRequest_SamePCSCF;
			}
			else
			{
				pSubRetryValuesStruct->eRetryType = EcrioOOMRetry_None;
				pSubRetryValuesStruct->uRetryInterval = 0;
			}

			return KOOMErrorNone;
		}
	}
	break;

	case 481:
	{
		pSubRetryValuesStruct->eRetryType = EcrioOOMRetry_InitialRequest_SamePCSCF;
		pSubRetryValuesStruct->uRetryInterval = 0;
	}
	break;

	case 408:
	case 482:
	case 487:
	case 500:
	{
		if (stRetryQueryStruct.bIsInitialRequest == Enum_TRUE)
		{
			pSubRetryValuesStruct->eRetryType = EcrioOOMRetry_InitialRequest_SamePCSCF;
		}
		else if (stRetryQueryStruct.bIsInitialRequest == Enum_FALSE)
		{
			pSubRetryValuesStruct->eRetryType = EcrioOOMRetry_None;
			pSubRetryValuesStruct->uRetryInterval = 0;
		}
		else
		{
			return KOOMInvalidParameters;
		}

		pSubRetryValuesStruct->uRetryInterval = ec_default_oom_GetRegRetryTime(stRetryQueryStruct.uRetryCount);
	}
	break;

	/* Other 3xx - 6xx  */
	default:
	{
		if (stRetryQueryStruct.uResponseCode >= 300 && stRetryQueryStruct.uResponseCode < 700)
		{
			if (stRetryQueryStruct.bIsInitialRequest == Enum_TRUE)
			{
				pSubRetryValuesStruct->eRetryType = EcrioOOMRetry_InitialRequest_SamePCSCF;
			}
			/*
			If the network rejects a re - SUBSCRIBE attempt for the reg events package with any cause code other than SIP
			481, the device shall maintain the current subscription to the reg events package until it expires(i.e.the device
			shall not send any further re - SUBSCRIBE attempts).Upon expiration of the current subscription to the reg
			events package, the device shall initiate a new subscription request for the reg events package.
			*/
			else if (stRetryQueryStruct.bIsInitialRequest == Enum_FALSE)
			{
				pSubRetryValuesStruct->eRetryType = EcrioOOMRetry_None;
				pSubRetryValuesStruct->uRetryInterval = 0;
			}
			else
			{
				return KOOMInvalidParameters;
			}

			pSubRetryValuesStruct->uRetryInterval = ec_default_oom_GetRegRetryTime(stRetryQueryStruct.uRetryCount);
		}
		else
		{
			return KOOMInvalidParameters;
		}
	}
	break;
	}	/* switch */

	return KOOMErrorNone;
}

/**
* This function provides which header for a SIP messages needs to include user = phone parameter.
* returns TRUE if supported for Method/Header/URI Type combination . otherwise FALSE
*
* @param[in]  eMethod  Represents SIP Message Type.
* @param[in]  eHeader  Represents SIP Header Type.
* @param[in]  eURIType  Represents SIP URI Type.
*
* @return Returns TRUE/FALSE
*/
static BoolEnum ec_oom_HasUserPhoneSupport
(
	EcrioSipMessageTypeEnum eMethodType,
	EcrioSipHeaderTypeEnum eHeaderType,
	EcrioSipURITypeEnums eURIType
)
{
	BoolEnum bHasSupport = Enum_FALSE;

	if (eURIType != EcrioSipURIType_SIP)
	{
		return bHasSupport;
	}

	/* Header gets the preference */
	switch (eHeaderType)
	{
	case EcrioSipHeaderTypeTo:
	{
		switch (eMethodType)
		{
		case EcrioSipMessageTypeRegister:
		case EcrioSipMessageTypeSubscribe:
		case EcrioSipMessageTypeMessage:
		case EcrioSipMessageTypeInvite:
		{
			bHasSupport = Enum_FALSE;
		}
		break;

		default:
		{
		}
		break;
		}
	}
	break;

	case EcrioSipHeaderTypeFrom:
	{
		switch (eMethodType)
		{
		case EcrioSipMessageTypeRegister:
		case EcrioSipMessageTypeSubscribe:
		case EcrioSipMessageTypeMessage:
		case EcrioSipMessageTypeInvite:
		{
			bHasSupport = Enum_FALSE;
		}
		break;

		default:
		{
		}
		break;
		}
	}
	break;

	case EcrioSipHeaderTypeContact:
	case EcrioSipHeaderTypeP_PreferredIdentity:
	{
		/* TODO */
		/* should not add user=phone as pf now*/
	}
	break;

	default:
	{
	}
	break;
	}

	return bHasSupport;
}

/*

Vz Reg-Retry Timer values
Fail Count	Timer value
1	30 second
2	30 second
3	75 second
4	2 mins
5	8 mins
6 and next	15 mins


*/
static u_int16 ec_default_oom_GetRegRetryTime
(
u_int8 uRetryCount
)
{
	u_int8 uRand = 0;

	switch (uRetryCount)
	{
	case 1:
	case 2:
	{
		return 30;
	}
	break;

	case 3:
	{
		/* 60 + 0<rand<16 */
		uRand = pal_UtilityRandomNumber();
		uRand = uRand % 15;
		return (60 + uRand);
	}
	break;

	case 4:
	{
		return (2 * 60);
	}
	break;

	case 5:
	{
		return (8 * 60);
	}
	break;

	default:
	{
		if (uRetryCount >= 6)
		{
			return (15 * 60);
		}
		else
		{
			return 0;
		}
	}
	}	/* switch */
}

/**
* This function is used to retrieve the Anonymity setting of CPIM headers.
*
* @return Returns Enum_TRUE/Enum_FALSE based on operator specification.
*/
static BoolEnum ec_oom_IsAnonymousCPIMHeader
(
)
{	
	return Enum_TRUE;
}

/**
* This function is used to retrieve the retry timer value to be used in case
* of SIP MESSAGE failures for SMS.
*
* @param[in] stRetryQueryStruct		EcrioOOMRetryQueryStruct.
* @param[in] pSmsRetryValuesStruct		Pointer to EcrioOOMRetryValuesStruct.
*
* @return Returns KOOMErrorNone if successful, otherwise an error
* specific to the oom implementation.
*/
static u_int32 ec_oom_GetSMSMessageRetryValues
(
	EcrioOOMRetryQueryStruct stRetryQueryStruct,
	EcrioOOMRetryValuesStruct *pSmsRetryValuesStruct
)
{
	/* Return invalid input for NULL pointer */
	/*Register error handling is not defined in RCC v7 document so we are working as vzw requirement*/
	pSmsRetryValuesStruct->uRetryInterval = 0;
	
	if (!pSmsRetryValuesStruct)
	{
		return KOOMInvalidParameters;
	}

	switch (stRetryQueryStruct.uResponseCode)
	{

		/* Other 4xx - 5xx  */
		default:
		{
			if (stRetryQueryStruct.uResponseCode >= 400 && stRetryQueryStruct.uResponseCode < 600)
			{
				if (stRetryQueryStruct.uRetryCount < 1)
				{
					pSmsRetryValuesStruct->uRetryInterval = 30000; //30sec
				}
				else
				{
					pSmsRetryValuesStruct->uRetryInterval = 0;
				}
			}
				
		}
		break;
	}	/* switch */

	return KOOMErrorNone;
}

/**
* This function is used to retrieve the wait timer value for pre-condition.
*
* @param[in] pValue		Pointer to u_int32 holding the timer value.
*
* @return Returns KOOMErrorNone if successful, otherwise an error
* specific to the oom implementation.
*/
static u_int32 ec_oom_GetPreconditionWaitTimerValue
(
	u_int32 *pValue
)
{
	if (pValue == NULL)
		return KOOMInvalidParameters;

	*pValue = 8000; //8sec

	return KOOMErrorNone;
}

/**
* This function is used to retrieve the setting to terminate forked dialogs.
*
* @return Returns Enum_TRUE/Enum_FALSE.
*/
static BoolEnum ec_oom_IsTerminateEarlyForkedCalls
(
)
{
	return Enum_TRUE;
}


/**
* This function is used to retrieve RTCP bandwidth values.
*
* @param[in] eCodec		Enumerated value of type EcrioOOMCodecEnum.
* @param[in] pRtcpBwValuesStruct		pointer of type EcrioOOMRTCPBandwidthStruct.
*
* @return Returns KOOMErrorNone if successful, otherwise an error
* specific to the oom implementation.
*/
static u_int32 ec_oom_GetRTCPBandwidthValues
(
	EcrioOOMCodecEnum eCodec,
	EcrioOOMRTCPBandwidthStruct *pRtcpBwValuesStruct
)
{
	if (pRtcpBwValuesStruct == NULL)
	{
		return KOOMInvalidParameters;
	}

	switch (eCodec)
	{
		case EcrioOOMCodec_AMR_NB:
		{
			pRtcpBwValuesStruct->uAS = 31;
			pRtcpBwValuesStruct->uRS = 775;
			pRtcpBwValuesStruct->uRR = 2325;
		}
		break;

		case EcrioOOMCodec_AMR_WB:
		{
			pRtcpBwValuesStruct->uAS = 38;
			pRtcpBwValuesStruct->uRS = 800;
			pRtcpBwValuesStruct->uRR = 2400;
		}
		break;

		case EcrioOOMCodec_General:
		{
			pRtcpBwValuesStruct->uAS = 0;
			pRtcpBwValuesStruct->uRS = 0;
			pRtcpBwValuesStruct->uRR = 0;
		}
		break;

		case EcrioOOMCodec_None:
		default:
		{
			return KOOMInvalidParameters;
		}
		break;
	}

	return KOOMErrorNone;
}

/**
* This function is used to retrieve the Reason cause to be used while
* terminating session using BYE.
*
* @param[in] ppReasonProto		Pointer to type u_char to hold Protocol.
* @param[in] ppReasonCause		Pointer to type u_char to hold cause.
* @param[in] ppReasonText		Pointer to type u_char to hold text.
*
* @return Returns KOOMErrorNone if successful, otherwise an error
* specific to the oom implementation.
*/
static BoolEnum ec_oom_GetReasonHeaderValues
(
	u_char **ppReasonProto,
	u_char **ppReasonCause,
	u_char **ppReasonText
)
{
	return Enum_FALSE;
}

// Define a global static holder for all pointers.
EcrioOOMStruct default_oom_object =
{
	ec_oom_GetSipTimers,
	ec_oom_GetSessionTimers,
	ec_oom_GetVolteFeatureTags,
	ec_ooms_GetOtherFeatureTags,
	ec_oom_HasHeaderSupport,
	ec_oom_GetAllowMethod,
	ec_oom_GetRegRetryValues,
	ec_oom_GetSubRetryValues,
	ec_oom_HasUserPhoneSupport,
	ec_oom_IsAnonymousCPIMHeader,
	ec_oom_GetSMSMessageRetryValues,
	ec_oom_GetPreconditionWaitTimerValue,
	ec_oom_IsTerminateEarlyForkedCalls,
	ec_oom_GetRTCPBandwidthValues,
	ec_oom_GetReasonHeaderValues
};

/**
* This function is used to retrieve the handle to instance of OOM object.
*
* @return Returns pointer to EcrioOOMStruct.
*/
EcrioOOMStruct *default_oom_GetObject
(
	void
)
{
	return &default_oom_object;
}


