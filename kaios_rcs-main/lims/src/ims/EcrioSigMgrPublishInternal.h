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

#include "EcrioSigMgrPublish.h"

#ifndef _ECRIO_SIG_MGR_PUBLISH_MESSAGE_INTERNAL_H_
#define  _ECRIO_SIG_MGR_PUBLISH_MESSAGE_INTERNAL_H_

/**
*	Headers
*/
#define _ECRIO_PUBLISH_EVENT												(u_char *)"presence"
#define _ECRIO_PUBLISH_EXPIRY												(3600)	

typedef enum {
	ECRIO_SIG_MGR_PUBLISH_STATE_None = 0,
	ECRIO_SIG_MGR_PUBLISH_STATE_Initial,
	ECRIO_SIG_MGR_PUBLISH_STATE_Established,
	ECRIO_SIG_MGR_PUBLISH_STATE_Refresh,
	ECRIO_SIG_MGR_PUBLISH_STATE_Modify,
	ECRIO_SIG_MGR_PUBLISH_STATE_Remove
}EcrioSigMgrPublishState;

typedef enum {
	ECRIO_SIG_MGR_PUBLISH_Module_Event_None = 0,
	ECRIO_SIG_MGR_PUBLISH_STATE_Module_Event_Request,
	ECRIO_SIG_MGR_PUBLISH_STATE_Module_Event_response
}EcrioSigMgrPublishModuleEvent;

typedef struct
{
	EcrioSigMgrStruct *pSigMgr;
	u_char* SipETag;
	u_int32 minExpires;
	TIMERHANDLE uPubExpireTimerID;
	BoolEnum bIsPubExpireTimerRunning;
	u_char* pMssgBody;
	u_int16 uMssgLen;
	EcrioSigMgrPublishState eState;
}EcrioSigMgrPublishInfo;

/**
* @def
*Function Definition - START
*/
u_int32 _EcrioSigMgrStartRePublishTimer
(
EcrioSigMgrStruct *pSigMgr,
EcrioSigMgrSipMessageStruct *pPubRsp,
EcrioSigMgrPublishInfo *pPubInfo
);
void _EcrioSigMgrRePublishCallBack
(
void *pData,
TIMERHANDLE timerID
);
/**
*API for send Publish Message.
*@Param IN EcrioSigMgrStruct* pSigMgr - Pointer to signaling manager.
*@Param IN EcrioSigMgrSipMessageStruct* pMessageReq - Message Info.
*@Param IN u_char* pMessageId - Message Information List Id.
*@Return	u_int32 Error code
*/
u_int32 _EcrioSigMgrRespValidateStateTakeAction
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pMessageResp,
	EcrioSigMgrPublishEvents ePubEvents,
	EcrioSigMgrSipMessageStruct *pSendMssg
);
u_int32 _EcrioSigMgrRequValidateStateTakeAction
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pMessageReq,
	EcrioSigMgrPublishEvents eState
);

u_int32 _EcrioSigMgrSendPublish
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pMessageReq
);


/**
*API for handling incoming Publish Responses.
*@Param IN EcrioSigMgrStruct* pSigMgr - Pointer to signaling manager.
*@Param IN EcrioSigMgrMessageResponseStruct* pMessageRespNtf - Message Response Info.
*@Param IN EcrioSigMgrCommonInfoStruct* pCmnInfo - Common Header info.
*@Return	u_int32 Error code
*/
u_int32 _EcrioSigMgrHandlePublishResponse
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pMessageRespNtf,
	EcrioSigMgrCommonInfoStruct *pCmnInfo
);

void _EcrioSigMgrPublishInfoStruct
(
	EcrioSigMgrPublishInfo **ppPublishInfo
);

/**
*Function Definition - END
*/

#endif /* _ECRIO_SIG_MGR_INSTANT_MESSAGE_INTERNAL_H_ */
