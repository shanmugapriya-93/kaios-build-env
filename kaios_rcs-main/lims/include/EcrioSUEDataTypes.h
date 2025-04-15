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

#ifndef	_ECRIO_SUE_DATA_TYPES_H_
#define	_ECRIO_SUE_DATA_TYPES_H_

#include "EcrioDataTypes.h"

// #ifndef _ECRIO_WITHOUT_CS_CALL_
// #define _ECRIO_WITHOUT_CS_CALL_
// #endif

/******************************************************************************

                Enumeration constant defination start

******************************************************************************/

/** \enum EcrioSUERegInfoAORContactStateEnum
* @brief This enum set defines the registration state of a particular contact corresponding to a specific AOR. Reference rfc 3680.
*/
typedef enum
{
	ECRIO_SUE_REGINFO_CONTACT_STATE_ENUM_None = 0,		/**< Default value during initialization state. */
	ECRIO_SUE_REGINFO_CONTACT_STATE_ENUM_Active,		/**< The contact address is bound to an aor. */
	ECRIO_SUE_REGINFO_CONTACT_STATE_ENUM_Terminated		/**< The contact address is no longer bound to an aor. */
} EcrioSUERegInfoAORContactStateEnum;

/** \enum EcrioSUERegInfoAORContactEventEnum
* @brief This enum set defines the event which caused the contact to reach the state and consequently triggers reginfo notification. Reference rfc 3680.
*/
typedef enum
{
	ECRIO_SUE_REGINFO_CONTACT_EVENT_ENUM_None = 0,		/**< Default value during initialization state. */
	ECRIO_SUE_REGINFO_CONTACT_EVENT_ENUM_Registered,	/**< The contact is bound to the aor through an actual SIP REGISTER request. */
	ECRIO_SUE_REGINFO_CONTACT_EVENT_ENUM_Created,		/**< The contact is created administratively, or through some non-SIP means */
	ECRIO_SUE_REGINFO_CONTACT_EVENT_ENUM_Refreshed,		/**< Contact is refreshed through a REGISTER request */
	ECRIO_SUE_REGINFO_CONTACT_EVENT_ENUM_Shortened,		/**< The duration of the registration is shortened for this contact to perform re-authentication. */
	ECRIO_SUE_REGINFO_CONTACT_EVENT_ENUM_Expired,		/**< The registration is expired for this contact address as it is not refreshed. */
	ECRIO_SUE_REGINFO_CONTACT_EVENT_ENUM_Deactivated,	/**< The registration is removed administratively to perform re-registration. */
	ECRIO_SUE_REGINFO_CONTACT_EVENT_ENUM_Probation,		/**< The registration is removed administratively to perform re-registration at some later point of time. */
	ECRIO_SUE_REGINFO_CONTACT_EVENT_ENUM_Unregistered,	/**< The contact address is de-registered through an actual SIP REGISTER request with expires value 0. */
	ECRIO_SUE_REGINFO_CONTACT_EVENT_ENUM_Rejected		/**< The registration is removed administratively but re-registration will not help to re-establish it. */
} EcrioSUERegInfoAORContactEventEnum;

/** \struct EcrioSUENotifyRegisterResponseStruct
* @brief This structure contains the details of the received REGISTER response. The REGISTER response could arrive for registration, re-registration or de-registration.
*/
typedef struct
{
	u_int32 uRspCode;					/**< The sip response code of the REGISTER/DE-REGISTER response. */
} EcrioSUENotifyRegisterResponseStruct;

/** \struct EcrioSUENotifyNotifyRequestStruct
* @brief This structure contains the details of the received NOTIFY request.
*/
typedef struct
{
	EcrioSUERegInfoAORContactStateEnum eRegInfoContactState;/**< The current registration state of the contact of default AOR. It could be active or terminated. */
	EcrioSUERegInfoAORContactEventEnum eRegInfoContactEvent;/**< The event which triggers the reginfo notification. It could be registered, created,deactivated,rejected,unregistered. */
	u_int32 uExpires;										/**< The number of seconds remaining until the binding is due to expire. */
	u_int32 uRetryAfter;									/**< The number of seconds after which the owner of the contact is expected to retry its registration. */
} EcrioSUENotifyNotifyRequestStruct;

#endif /*  _ECRIO_SUE_DATA_TYPES_H_    */
