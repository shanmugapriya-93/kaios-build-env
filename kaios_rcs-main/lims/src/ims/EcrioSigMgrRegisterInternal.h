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

#ifndef _ECRIO_SIG_MGR_RGISTER_INTERNAL_H_
#define  _ECRIO_SIG_MGR_RGISTER_INTERNAL_H_

/**
*	Headers
*/

/**
* @def
*Constant Definition - START
*/

/**
*Constant Definition - END
*/

/**
*Structure Definition - START
*/

/* structure for storing contact uri info*/
typedef struct
{
	/** Contact Uris*/
	EcrioSigMgrNameAddrWithParamsStruct *pUserContactInfo;
} EcrioSigMgrRegisterContactInfoStruct;

typedef struct
{
	/** number of Contact URIs to be registered*/
	u_int16 numRegContactUris;
	/** list of Contact URIs to be registered*/
	EcrioSigMgrRegisterContactInfoStruct **ppContactHdrs;
} EcrioSigMgrRegisterContactHeaderStruct;

typedef	struct
{
	u_char *pCallID;
	EcrioSigMgrStruct *pSigMgr;
} EcrioSigMgrCallIDStruct;
/* structure representing registration information for a party.*/
typedef struct
{
	/** Register request headers*/
	EcrioSigMgrSipMessageStruct *pRegisterReq;
	/* registration state*/
	EcrioSigMgrRegistrationStateEnum registrationState;
	/* Registration identifier*/
	u_int32 regId;
	/** Expires Header Value*/
	u_int32 *pExpires;
	/** Call-ID Header Value */
	EcrioSigMgrCallIDStruct	*pCallIDStruct;
	/** CSeqNo Header VAlue */
	u_int32 cSeqNo;
	u_int32	authRetryCount;
	/** Reregistration TimerID*/
	TIMERHANDLE reregistrationTimerId;
	EcrioSigMgrRegisterRequestEnum reqType;
	BoolEnum bIsRegExpireTimerRunning;
	TIMERHANDLE uRegExpireTimerID;
} EcrioSigMgrUserRegisterInfoStruct;

/* structure for storing registration information for different party's*/
typedef struct
{
	/** Register information list - _EcrioSigMgrUserRegisterInfoStruct*/
	void *registerInformationList;
} EcrioSigMgrRegisterInfoStruct;

/**
*Structure Definition - END
*/

/**
* @def
*Function Definition - START
*/
/**
*API for Registering either local party or third party using Signaling manager.
*@Param IN SIGMGRHANDLE* pSigMgrHandle - Handle to signaling manager.
*@Param IN EcrioSigMgrRegisterRequestStruct* pRegReq- Registration info.
*@Param OUT void** ppRegisterIdentifier - Registration identifier.
*@Return u_int32 Error code.
*/
u_int32 _EcrioSigMgrSendRegister
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrRegisterRequestEnum eRegReq,
	void *pRegisterIdentifier
);
/**
*Function for initalizing registration info structure.
*@Param IN _EcrioSigMgrRegisterInfoStruct** ppRegInfo - Handle to registeration info structure.
*@Return	u_int32 Error code.
*/
u_int32 _EcrioSigMgrInitializeRegistrationInfo
(
	EcrioSigMgrRegisterInfoStruct **ppRegInfo
);

/**
*Function for deinitalizing registration info structure.
*@Param IN _EcrioSigMgrRegisterInfoStruct** ppRegInfo - Handle to registeration info structure.
*@Return	u_int32 Error code.
*/
u_int32 _EcrioSigMgrDeInitializeRegistrationInfo
(
	EcrioSigMgrRegisterInfoStruct **ppRegInfo
);

/**
*Function for creating registeration info for a user to registartion info list.
*@Param IN _EcrioSigMgrRegisterInfoStruct* pRegInfo - pointer to registeration info structure.
*@Param OUT _EcrioSigMgrUserRegisterInfoStruct** pUserRegInfo - pointer to user registeration info structure
*@Return	u_int32 Error code.
*/
u_int32 _EcrioSigMgrCreateUserRegInfo
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUserRegisterInfoStruct **ppUserRegInfo
);

/**
*Function for removing registeration info for a user to registartion info list.
*@Param IN _EcrioSigMgrRegisterInfoStruct* pRegInfo - pointer to registeration info structure.
*@Param IN u_char* pToUserId -  User id for user info is to be deleted.
*@Return	u_int32 Error code.
*/
u_int32 _EcrioSigMgrRemoveUserRegInfo
(
	EcrioSigMgrStruct *pSigMgr,
	u_int32 *pRegId
);

/**
*Function for getting registeration info for a user to registartion info list.
*@Param IN _EcrioSigMgrRegisterInfoStruct* ppRegInfo - pointer to registeration info structure.
*@Param IN void* ppRegistrationId - registration id.
*@Param OUT _EcrioSigMgrUserRegisterInfoStruct** ppUserRegInfo - handle to user registeration info structure
*@Return	u_int32 Error code.
*/
u_int32 _EcrioSigMgrGetUserRegInfo
(
	EcrioSigMgrStruct *pSigMgr,
	void *pRegistrationId,
	EcrioSigMgrUserRegisterInfoStruct **ppUserRegInfo
);

/**
*Function for getting registeration state from a user's registartion info.
*@Param IN _EcrioSigMgrRegisterInfoStruct* pRegInfo - pointer to registeration info structure.
*@Param IN u_char* toUserId - user id for which registration info is to get
*@Param OUT EcrioSigMgrRegistrationStateEnum* pState - registration state.
*@Return	u_int32 Error code.
*/
u_int32 _EcrioSigMgrGetUserRegState
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *toUserId,
	EcrioSigMgrRegistrationStateEnum *pState
);

/**
*Function for setting registeration state from a user's registartion.
*@Param IN _EcrioSigMgrRegisterInfoStruct* pRegInfo - pointer to registeration info structure.
*@Param IN _EcrioSigMgrUserRegisterInfoStruct* pUserRegInfo - user's registration info.
*@Param OUT EcrioSigMgrRegistrationStateEnum* pState - registration state.
*@Return	u_int32 Error code.
*/

/* Function for comaparing data in list Nodes */
s_int32 _EcrioSigMgrCompareRegInfoFunc
(
	void *pData1,
	void *pData2,
	void *pCallbackData
);

/* Function for Deleting data in list Nodes */
void _EcrioSigMgrDestroyRegInfoFunc
(
	void *pData,
	void *pCallbackData
);

u_int32 _EcrioSigMgrUpdateRegInfo
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrRegisterRequestEnum reqType,
	EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo
);

u_int32 _EcrioSigMgrRegCheckContactUri
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo,
	BoolEnum isReRegistration
);

u_int32 _EcrioSigMgrReleaseRegInfo
(
	EcrioSigMgrStruct *pSigMgr,
	void **ppRegisterInfo
);

u_int32 _EcrioSigMgrHandleRegResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pRegResp
);

BoolEnum _EcrioSigMgrIsDeRegResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pRegResp
);

u_int32 _EcrioSigMgrStartReRegTimer
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pRegRsp,
	EcrioSigMgrCallIDStruct *pCallIdStruct,
	EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo,
	BoolEnum bPositiveResponse
);

u_int32 _EcrioSigMgrPopulateContactUris
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pRegReq,
	EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo
);

u_int32 _EcrioSigMgrUpdateExpirationTime
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo,
	EcrioSigMgrRegisterRequestEnum regReqType
);

void _EcrioSigMgrReRegistrationCallBack
(
	void *pData,
	TIMERHANDLE timerID
);

void _EcrioSigMgrRegExpireCallBack
(
	void *pData,
	TIMERHANDLE timerID
);

void _EcrioSigMgrHandleRegTimeOut
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pCallId
);

u_int32 _EcrioSigMgrInitRegObject
(
	EcrioSigMgrStruct *pSigMgr,
	void **ppRegisterIdentifier
);

u_int32 _EcrioSigMgrGetUserRegInfoRegId
(
	EcrioSigMgrStruct *pSigMgr,
	void *pRegistrationId,
	EcrioSigMgrUserRegisterInfoStruct **ppUserRegInfo
);

u_int32 _EcrioSigMgrGetUserRegStateRegId
(
	EcrioSigMgrStruct *pSigMgr,
	u_int32 *pRegId,
	EcrioSigMgrRegistrationStateEnum *pState
);

u_int32 _EcrioSigMgrUpdateUserRegInfoOnInitState
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo,
	BoolEnum bDelateRegInfo
);

u_int32 _EcrioSigMgrUpdateUserRegInfoOnRegisteredState
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUserRegisterInfoStruct *pUserRegInfo,
	BoolEnum bDelateRegInfo
);

/**
*Function Definition - END
*/

/************************************************************************
*						Structure Init APIs
************************************************************************/

/************************************************************************
*						Structure Release APIs
************************************************************************/

u_int32 _EcrioSigMgrReleaseRegisterInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrRegisterInfoStruct *pRegisterInfo
);

u_int32 _EcrioSigMgrReleaseUserRegisterInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUserRegisterInfoStruct *pUserRegisterInfo
);

u_int32 _EcrioSigMgrReleaseRegContactHdrStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrRegisterContactHeaderStruct *pRegContactHdr
);

u_int32 _EcrioSigMgrReleaseRegContactInfoStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrRegisterContactInfoStruct *pRegContactInfo
);
/*Newly added for multiple expire support*/
u_int32 _EcrioSigMgrAddExpiryParam
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pUserName,
	BoolEnum bIsHdrParam,
	EcrioSigMgrRegisterContactInfoStruct *pContactHdrs
);

u_int32 _EcrioSigMgrFindExpiryInNameAddrStuct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrNameAddrWithParamsStruct *pContactDetails,
	BoolEnum *pIsHdrParam,
	BoolEnum *pIsZeroValue,
	BoolEnum *pExpireFound,
	u_int32	*pExpiresVal
);

u_char *_EcrioSigMgrGetRegistrationStateString
(
	EcrioSigMgrRegistrationStateEnum registrationState
);

#endif /* _ECRIO_SIG_MGR_RGISTER_INTERNAL_H_ */
