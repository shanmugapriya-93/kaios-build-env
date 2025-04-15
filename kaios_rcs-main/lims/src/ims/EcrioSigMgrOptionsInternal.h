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

#include "EcrioSigMgrOptions.h"

#ifndef _ECRIO_SIG_MGR_OPTIONS_MESSAGE_INTERNAL_H_
#define _ECRIO_SIG_MGR_OPTIONS_MESSAGE_INTERNAL_H_

/**
*	Headers
*/

typedef struct
{
	EcrioSigMgrStruct *pSigMgr;
	u_int32 minExpires;
	TIMERHANDLE uPubExpireTimerID;
	BoolEnum bIsPubExpireTimerRunning;
	u_char* pMssgBody;
	u_int16 uMssgLen;
}EcrioSigMgrOptionsInfo;


/**
* @def
*Function Definition - START
*/

/**
*API for send Options Message.
*@Param IN EcrioSigMgrStruct* pSigMgr - Pointer to signaling manager.
*@Param IN u_char *pCallId - Call Id.
*@Param IN EcrioSigMgrSipMessageStruct* pOptionsReq - Options Info.
*@Return	u_int32 Error code
*/
u_int32 _EcrioSigMgrSendOptionsRequest
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pCallId,
	EcrioSigMgrSipMessageStruct *pOptionsReq,
	u_char *pDest
);


/**
*API for send Options Message.
*@Param IN EcrioSigMgrStruct* pSigMgr - Pointer to signaling manager.
*@Param IN EcrioSigMgrSipMessageStruct* pOptionsReq - Options Info.
*@Return	u_int32 Error code
*/
u_int32 _EcrioSigMgrSendOptionsImpl
(
EcrioSigMgrStruct *pSigMgr,
EcrioSigMgrSipMessageStruct *pMessageReq,
u_char *pDest
);

/**
*API for send Options Response.
*@Param IN EcrioSigMgrStruct* pSigMgr - Pointer to signaling manager.
*@Param IN EcrioSigMgrMessageResponseStruct* pOptionsResp - Options Response Info.
*@Return	u_int32 Error code
*/
u_int32 _EcrioSigMgrSendOptionsResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pOptionsResp
);


/**
*API for handling incoming Options Request.
*@Param IN EcrioSigMgrStruct* pSigMgr - Pointer to signaling manager.
*@Param IN EcrioSigMgrSipMessageStruct* pOptionsReqNtf - Options Info.
*@Param IN EcrioSigMgrCommonInfoStruct* pCmnInfo - Common Header info.
*@Param IN u_char* pReasonPhrase - Parsed error phrase (if error).
*@Param IN u_int32 reasonCode - Parsed error code (if error).
*@Return	u_int32 Error code
*/
u_int32 _EcrioSigMgrHandleOptionsRequest
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pOptionsReqNtf,
	EcrioSigMgrCommonInfoStruct *pCmnInfo,
	u_int32 reasonCode,
	u_char *pReasonPhrase
);



/**
*API for handling incoming Options Responses.
*@Param IN EcrioSigMgrStruct* pSigMgr - Pointer to signaling manager.
*@Param IN EcrioSigMgrMessageResponseStruct* pMessageRespNtf - Message Response Info.
*@Param IN EcrioSigMgrCommonInfoStruct* pCmnInfo - Common Header info.
*@Return	u_int32 Error code
*/
u_int32 _EcrioSigMgrHandleOptionsResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pMessageRespNtf,
	EcrioSigMgrCommonInfoStruct *pCmnInfo
);


/**
*API to send automatic error response to incoming OPTIONS request.
*@Param IN EcrioSigMgrStruct* pSigMgr - Pointer to signaling manager.
*@Param IN EcrioSigMgrSipMessageStruct* pOptionsReq - incoming OPTIONS request info.
*@Param IN u_char* pReasonPhrase - reason phrase of error Response.
*@Param IN u_int32 reasonCode - status code of error Response
*@Return	u_int32 Error code
*/
u_int32 _EcrioSigMgrSendOptionsErrorResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pOptionsReq,
	u_char *pReasonPhrase,
	u_int32 reasonCode
);

u_int32 _EcrioSigMgrSendOptionsResponseImpl
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pOptionsResp
);

/**
*Function Definition - END
*/

#endif /* _ECRIO_SIG_MGR_OPTIONS_MESSAGE_INTERNAL_H_ */
