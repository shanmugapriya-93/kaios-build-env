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

#ifndef _ECRIO_SIG_MGR_CALL_CONTROL_H_
#define  _ECRIO_SIG_MGR_CALL_CONTROL_H_

/**
*	Headers
*/

/**
* @def
*Enumerated Constant Definition - START
*/

/** Enumeration of constants that is used to indicate the call states. */
typedef enum
{
	EcrioSigMgrCallStateUnInitialized = 0,
	/**Call object has been initialized (INVITE request sent) .*/
	EcrioSigMgrCallStateInitialized,
	/**1xx response received / send for the Invite request*/
	EcrioSigMgrCallStateSetupInProgress,
	/**Cancel request received / send to cancel the Invite request*/
	EcrioSigMgrCallStateCancelInProgress,
	/** response send for the Invite request*/
	EcrioSigMgrCallStateAcceptedAckPending,
	/** 200 response has been received for the Invite request sent earlier and ACK send.*/
	/**200 response sent for the Invite request and ACK received.*/
	EcrioSigMgrCallStateRejectedAckPending,
	EcrioSigMgrCallStateCallEstablished,
	/** BYE request received to terminate the call*/
	EcrioSigMgrCallStateTerminationInProgress,
	/** Re-Invite request has been sent.*/
	EcrioSigMgrCallStateReInviteInProgress,
	/** 200 response sent for the ReInvite request waiting for ACK.*/
	EcrioSigMgrCallStateReInviteAcceptedAckPending,
	/**4xx - 6xx response send for the ReInvite request*/
	EcrioSigMgrCallStateReInviteRejectedAckPending,
} EcrioSigMgrCallStateEnum;

/**Enumeration of constants that is used to specify the answer states.*/
typedef enum
{
	/** offer has not been made for current call*/
	EcrioSigMgrOfferPending,
	/** Answer has not been received for current call*/
	EcrioSigMgrAnswerPending,
	/** offer / Answer has been completed for current call*/
	EcrioSigMgrOfferAnswerCompleted
} EcrioSigMgrOfferAnswerState;

/**
*Enumerated Constant Definition - END
*/

/*
 @def
Function Definition - START
*/

/** \brief This function is used to send INVITE / RE-INVITE request to initiate or update a session. It will add session timer parameters automatically.
 *
 *
 *
 * @pre                                 EcrioSigMgrInit()must be call
 *
 * <b>Memory allocation:</b>	        <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;None.
 *
 * \headerfile                          EcrioSigMgrCallControl.h
 * @param[in] sigMgrHandle				- Handle to signaling manager.
 *
 * @param[in] pInviteReq				- INVITE request information.
 * @param[in,out] pSessionHandle			- Handle to the session.
 *
 * <b>Transaction:</b>                  <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This function is used to create and sends an INVITE request to the network.
*										 If valid session handle is passed then Re-Invite will be sent.
*										UAE supports session refreshing as per RFC 4028. UAE will send refresh request automatically.
 * @return						        One of the pre-defined Error Codes.
 * @note                                Mandatory Structure Member.
 * - pInviteReq
 * - pSessionHandle
 */
u_int32 EcrioSigMgrSendInvite
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSipMessageStruct *pInviteReq,
	u_char** ppCallID
);

/** \brief This function is used to respond to an incoming INVITE request.
 *
 *
 *
 * @pre                                 EcrioSigMgrInit()must be call
 *
 * <b>Memory allocation:</b>	        <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;None.
 *
 * \headerfile                          EcrioSigMgrCallControl.h
 * @param[in] sigMgrHandle				- Handle to signaling manager.
 *
 * @param[in] pInviteResp				- INVITE response information
 *
 * <b>Transaction:</b>                  <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This function is used to create and send an INVITE response to the network.
 * @return						        One of the pre-defined Error Codes.
 * @note                                Mandatory Structure Member.
 * - pInviteResp
 * - pInviteResp->responseCode
 * - pInviteResp->pReasonPhrase
 * - pInviteResp->pSessionHandle
 */

u_int32 EcrioSigMgrSendInviteResponse
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSipMessageStruct *pInviteResp
);
// *
// *API to Terminate an ongoing call.
// *@Param IN SIGMGRHANDLE sigMgrHandle - Handle to signaling manager.
// *@Param IN EcrioSigMgrByeRequestStruct* pByeReq - Bye request hdr values.
// *@Param IN SIGSESSIONHANDLE sessionHandle - Handle to session.
// *@Return	u_int32 Error code

/** \brief This function is used to terminate an ongoing call by sending BYE request.
 *
 *
 *
 * @pre                                 EcrioSigMgrSendInvite() or EcrioSigMgrSendInviteResponse() must be call
 *
 * <b>Memory allocation:</b>	        <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;None.
 *
 * \headerfile                          EcrioSigMgrCallControl.h
 * @param[in] sigMgrHandle				- Handle to signaling manager.
 *
 * @param[in] pByeReq					- BYE request header values.
 * @param[in] sessionHandle				- Handle to the session.
 *
 * <b>Transaction:</b>                  <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This function is used to create and send a BYE request.
 * @return						        One of the pre-defined Error Codes.
 * @note                                Mandatory Structure Member.
 * - sessionHandle
 */

u_int32 EcrioSigMgrSendBye
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSipMessageStruct *pByeReq
);

u_int32 EcrioSigMgrSendUpdate
(
	SIGMGRHANDLE sigMgrHandle,
	u_char* pCallID,
	EcrioSigMgrSipMessageStruct *pUpdateReq
);

u_int32 EcrioSigMgrSendPrackRequest
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSipMessageStruct *pPrackReq
);

u_int32 EcrioSigMgrSendPrackResponse
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSipMessageStruct *pPrackReq,
	u_char *pReasonPhrase,
	u_int32	responseCode
);

u_int32 EcrioSigMgrSendUpdateResponse
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSipMessageStruct *pUpdateResp
);

// *
// * API to terminate an ongoing call. This API will internally determine whether bye / cancel
// * should be send. It will also invoke EcrioSigMgrUpdateMessageCallback and will pass
// * EcrioSigMgrByeRequestHeaderStruct or EcrioSigMgrCancelRequestHeaderStruct depending on the
// * constructed message.
// *@Param IN SIGMGRHANDLE sigMgrHandle - Handle to signaling manager.
// *@Param IN SIGSESSIONHANDLE sessionHandle - Handle to session.
// *@Return	u_int32 Error code

/** \brief This function is used to terminate an ongoing session by either sending BYE or CANCEL based on current session state.
 *
 *
 *
 * @pre                                 EcrioSigMgrSendInvite() or EcrioSigMgrSendInviteResponse()must be call.
 *
 * <b>Memory allocation:</b>	        <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;None.
 *
 * \headerfile                          EcrioSigMgrCallControl.h
 * @param[in] sigMgrHandle				- Handle to signaling manager.
 *
 * @param[in] sessionHandle				- Handle to session.
 * @param[in] eReason				    - Describing reason of termination.
 *
 * <b>Transaction:</b>                  <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This function is used to create and send either BYE or CANCEL request based on session state.
 * @return						        One of the pre-defined Error Codes.
 * @note                                Mandatory Structure Member.
 * - sessionHandle
 *
 */
u_int32 EcrioSigMgrTerminateSession
(
	SIGMGRHANDLE sigMgrHandle,
	u_char* pCallID,
	BoolEnum bIsTerminatedByUser
);

/** \brief This function is used to terminate all early dialogs, by sending a BYE.
*
* \headerfile                          EcrioSigMgrCallControl.h
* @param[in] sigMgrHandle				- Handle to signaling manager.
*
* @param[in] sessionHandle				- Handle to session.
*
* <b>Transaction:</b>                  <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This function is used to create and send BYE to all early dialogs.
* @return						        One of the pre-defined Error Codes.
* @note                                Mandatory Structure Member.
* - SIGMGRHANDLE
*
*/
u_int32 EcrioSigMgrTerminateForkedEarlyDialogs
(
	SIGMGRHANDLE sigMgrHandle,
	u_char* pCallID
);

#endif /* _ECRIO_SIG_MGR_CALL_CONTROL_H_ */
