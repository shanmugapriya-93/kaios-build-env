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

#ifndef _ECRIO_SIG_MGR_REGISTER_H_
#define  _ECRIO_SIG_MGR_REGISTER_H_

/** Enumeration of constants that is used to specify registration states for a local / third party.*/
typedef enum
{
	/**Registration object is not initialized*/
	EcrioSigMgrStateUnInitialized = 0,
	/**Registration object has been initialized or response to a deregister request has been received.*/
	EcrioSigMgrStateInitialized,
	/**Registration request has been sent but response has not been received*/
	EcrioSigMgrStateRegistrationInProgress,
	/**407 response has been received for the register request sent earlier*/
	EcrioSigMgrStateProxyAuthenticationRequired,
	/**401 response has been received for the register request sent earlier*/
	EcrioSigMgrStateAuthenticationRequired,
	/** 200 response has been received for the register request sent earlier.*/
	EcrioSigMgrStateRegistered,
	/** 423 response has been recieved*/
	EcrioSigMgrStateExpiresIntervalTooBrief,
	/** DeRegistration request has been sent but response has not been received*/
	EcrioSigMgrStateDeRegistrationInProgress,
	/**Redirection in progress.*/
	EcrioSigMgrRegisterRedirectionInProgress
} EcrioSigMgrRegistrationStateEnum;

/**
*	Enumeration of constants that is used to identify REGISTER request type.
*/
typedef enum
{
	/** Enumeration that is used to Identifies a request as register*/
	EcrioSigMgrRegisterRequestType_Reg,
	/** Enumeration that is used to identify request as De-REGISTER request.*/
	EcrioSigMgrDeRegisterRequestType_DeReg,
	/** Enumeration that is used to identify request to fetch bindings.*/
	EcrioSigMgrRegisterRequestType_Fetching
} EcrioSigMgrRegisterRequestEnum;
/** This structure will be passed to the upper layer during registration / deregistration so
*that upper layer can modify header values.
*@brief passed to the upper layer during registration / deregistration*/
typedef struct
{
	/** Structure that stores the registration information..*/
	void *pRequestHdrs;

	/** Upper Layer's data.*/
	void *pAppData;

	/** registration identifier*/
	void *pRegisterIdentifier;
} EcrioSigMgrUpdateRegisterRequestStruct;

/*
API for creating registration object.
@Param[in] SIGMGRHANDLE sigMgrHandle - Handle to signaling manager.
@Param[in,out] void** ppRegisterIdentifier - Registration identifier.
@Return u_int32 Error code.*/

/** \brief This function is used to create registration object.
 *
 *
 *
 * @pre                                 EcrioSigMgrInit() must be call.
 *
 * <b>Memory allocation:</b>	        <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;Allocates memory for ppRegisterIdentifier
 *
 * @param[in] sigMgrHandle				- Handle to the signaling manager.
 * @param[in,out] ppRegisterIdentifier		- Registration identifier.
 *
 * <b>Transaction:</b>                 <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; This API is used for creating registration object.
 * @return						        One of the pre-defined Error Codes.
 * @note                                Mandatory Structure Member.
 * - ppRegisterIdentifier
 */

u_int32 EcrioSigMgrInitRegObject
(
	SIGMGRHANDLE sigMgrHandle,
	void **ppRegisterIdentifier
);

/*
API for Registration /deregistration of either local party or third party using Signaling manager.
@Param [IN] SIGMGRHANDLE sigMgrHandle - Handle to signaling manager.
@Param [IN] EcrioSigMgrRegRequestStruct* pRegReq- Registration info.
@Param [IN] void* pRegisterIdentifier - Registration identifier.
@Return u_int32 Error code.*/

/** \brief This function is used to register / deregister either local party or third party using signaling manager.
 *
 *
 *
 * @pre                                 EcrioSigMgrInitRegObject() must be call.
 *
 * <b>Memory allocation:</b>	        <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;None.
 *
 * @param[in] sigMgrHandle				- Handle to the signaling manager.
 * @param[in] pRegReq					- Registration information.
 * @param[out] pRegisterIdentifier		- Pointer to Registration Identifier.
 *
 * <b>Transaction:</b>                  <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This function creates and sends REGISTER request to the network.
 *
 *                                      When UAE received 503 response against REGISTER request it will
 *                                      automatically send a new request after the duration which is specified
 *                                      in Retry-After header. If UAE received 503 again then it will treat as
 *										registration failed that means it won't retry. Registrar Domain name will
 *										be used to create request URI only. If no response is arrived then UAE will
 *										send register to a new registrar which pre-configured by set signaling
 *										parameter. This back up facility is available in over SAC only.
 * @return						        One of the pre-defined Error Codes.
 * @note                                Mandatory Structure Member.
 * - pRegReq
 * - pRegisterIdentifier
 */

u_int32 EcrioSigMgrSendRegister
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrRegisterRequestEnum eRegReq,
	void *pRegisterIdentifier
);

// **
// *API for releasing registration object. Also release registration id.
// *@Param IN SIGMGRHANDLE sigMgrHandle - Handle to signaling manager.
// *@Param IN void** ppRegisterIdentifier - Registartion identifier.
// *@Return u_int32 Error code.

/** \brief This function is used to releasing registration object.
 *
 *
 *
 * @pre                                 EcrioSigMgrInitRegObject() must be call.
 *
 * <b>Memory allocation:</b>	        <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;De-allocates memory for ppRegisterIdentifier.
 *
 * @param[in] sigMgrHandle				- Handle to the signaling manager.
 * @param[in] ppRegisterIdentifier		- Registration identifier.
 *
 * <b>Transaction:</b>                  <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This API is also used to release the
 *                                      registration object. It also releases the registration ID.
 * @return						        One of the pre-defined Error Codes.
 * @note                                Mandatory Structure Member.
 * - ppRegisterIdentifier.
 * - *ppRegisterIdentifier
 */

u_int32 EcrioSigMgrReleaseRegObject
(
	SIGMGRHANDLE sigMgrHandle,
	void **ppRegisterIdentifier
);

/**
*	Enumeration of constants that is used to identify REGISTER request Timer type.
*/
typedef enum
{
	EcrioSigMgrRegisterTimerNone,
	/** Enumeration that is used to stop re-registeration timer*/
	EcrioSigMgrReRegisterTimer,
	/** Enumeration that is used to stop registeration expiry timer*/
	EcrioSigMgrRegisterExpiryTimer,
	/** Enumeration that is used to stop registeration related timer*/
	EcrioSigMgrRegisterTimerAll
} EcrioSigMgrRegisterTimerType;

/** \brief This function is used to reset registration timer based on new registration expiry.
 *
 *
 *
 * @pre                                 EcrioSigMgrInitRegObject() must be call.
 *
 * <b>Memory allocation:</b>	        <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;None.
 *
 * @param[in] sigMgrHandle				- Handle to the signaling manager.
 *
 * <b>Transaction:</b>                  <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This API is used to reset registration timer based on new expiry.
 * @return						        One of the pre-defined Error Codes.
 * @note                                Mandatory Structure Member
 */
u_int32 EcrioSigMgrStopRegistrationTimer
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrRegisterTimerType eRegTimerType,
	void *pRegIdentifier
);

#endif /* _ECRIO_SIG_MGR_REGISTER_H_ */
