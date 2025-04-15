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

#ifndef _ECRIO_SIG_MGR_IMSLIB_HANDLER_H_
#define  _ECRIO_SIG_MGR_IMSLIB_HANDLER_H_

#include "EcrioSigMgrRegister.h"
#include "EcrioSigMgrRegisterInternal.h"
#include "EcrioSigMgrInstantMessage.h"
#include "EcrioSigMgrInstantMessageInternal.h"
#include "EcrioSigMgrPublishInternal.h"
#include "EcrioSigMgrMacroDefination.h"
#include "EcrioSigMgrSipMessage.h"
#include "EcrioSigMgrDialogHandler.h"
#include "EcrioSigMgrCallControl.h"
#include "EcrioSigMgrCallControlInternal.h"

/**************************************************************************

Function:		_EcrioSigMgrPopulateCommonInfo()

Purpose:		Populates the Common info from the request / response.

Description:	Populates the Common info from the request / response.

Input:			EcrioSigMgrStruct* pSigMgr - Signaling Manager Instance.
Input:			EcrioIMSLibrarySignalingMandatoryStruct* pManHdrs  - IMS Mandatory struct.
Input:			EcrioSigMgrMandatoryHeaderStruct* pSrcManHdrs, -  Signaling Manager Mandatory struct.
Output:			EcrioSigMgrCommonInfoStruct* pCommonInfo - CommonInfoStruct.

Returns:		error code.
**************************************************************************/
u_int32 _EcrioSigMgrPopulateCommonInfo
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrMandatoryHeaderStruct *pSrcManHdrs,
	EcrioSigMgrCommonInfoStruct *pCommonInfo
);

u_int32 _EcrioSigMgrFormTelParams
(
	EcrioSigMgrStruct *pSigMgr,
	u_int16	numTelParams,
	EcrioSigMgrTelParStruct **ppTelParams,
	u_char **ppUri
);

u_int32 _EcrioSigMgrFormSipUri
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipURIStruct *pSipUri,
	u_char **ppUri
);

u_int32 _EcrioSigMgrFormTelUri
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrTelURIStruct *pTelUri,
	u_char **ppUri
);

u_int32 _EcrioSigMgrFormUri
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUriStruct *pUriStruct,
	u_char **ppUri
);

u_int32 _EcrioSigMgrParseUri
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pUri,
	EcrioSigMgrUriStruct *pUriStruct
);

u_int32 _EcrioSigMgrPopulateMandatoryHeaders
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessage,
	EcrioSigMgrSIPMessageTypeEnum eMsgType,
	BoolEnum generateToTag,
	u_char *pBranch
);

u_char *_EcrioSigMgrGetMthdName
(
	EcrioSipMessageTypeEnum methodType
);

u_int32 _EcrioSigMgrCompareSIPURI
(
	EcrioSigMgrSipURIStruct *pSipUriStruct1,
	EcrioSigMgrSipURIStruct *pSipUriStruct2,
	BoolEnum *pbMatchedUri
);

u_int32 _EcrioSigMgrCompareTelURI
(
	EcrioSigMgrTelURIStruct *pTelURI1,
	EcrioSigMgrTelURIStruct *pTelURI2,
	BoolEnum *pbMatchedUri
);

u_int32 _EcrioSigMgrCompareGlobalTelURI
(
	EcrioSigMgrTelGlobalNumberStruct *pGlobalNumber1,
	EcrioSigMgrTelGlobalNumberStruct *pGlobalNumber2,
	BoolEnum *pbMatchedUri
);

u_int32 _EcrioSigMgrCompareTelUriNumber
(
	u_char *pNumber1,
	u_char *pNumber2
);

u_int32 _EcrioSigMgrRemoveVisualSeparatorTelURI
(
	u_char *pInStr,
	u_char **ppOutStr
);

u_int32 _EcrioSigMgrCompareLocalTelURI
(
	EcrioSigMgrTelLocalNumberStruct *pLocalNumber1,
	EcrioSigMgrTelLocalNumberStruct *pLocalNumber2,
	BoolEnum *pbMatchedUri
);

u_int32 _EcrioSigMgrCompareTelUriContext
(
	EcrioSigMgrTelContextStruct *pContext1,
	EcrioSigMgrTelContextStruct *pContext2
);

u_int32 _EcrioSigMgrCompareTelUriParam
(
	EcrioSigMgrTelParStruct *pTelUriParam1,
	EcrioSigMgrTelParStruct *pTelUriParam2,
	BoolEnum *pbContinue
);

u_int32 _EcrioSigMgrGenerateRequestUri
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrUriStruct *pRemoteTarget,
	EcrioSigMgrRouteStruct *pRouteHdr,
	EcrioSigMgrUriStruct **ppRequestUri
);

void _EcrioSigMgrIMSHandleMessage
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pReceiveData,
	u_int32 receiveLength
);

u_int32 _EcrioSigMgrHandleIncomingRequests
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessage
);

u_int32 _EcrioSigMgrHandleIncomingResponses
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessage
);

u_int32 _EcrioSigMgrIMSCheckIsRetranMsg
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrMandatoryHeaderStruct *pMandatoryHdrs,
	u_char *pMethod,
	EcrioSigMgrRoleEnum role,
	u_char **pTxnSessionMatchingString,
	u_int32 responseCode
);

u_int32 _EcrioSigMgrCreateTCPBasedBuffer
(
	EcrioSigMgrStruct *pSigMgr,
	u_char **pMsg,
	EcrioSigMgrSIPMessageTypeEnum eMsgType
);

u_int32 _EcrioSigMgrValidateIncomingRequests
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrValidationStruct *pValidate
);

u_int32 _EcrioSigMgrUpdateMessageRequestStruct
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pMessageReq,
	BoolEnum bServiceRoute
);

u_int32 _EcrioSigMgrCheckTLSTransport
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessage
);

void _EcrioSigMgrGetTxnSMSRetryParameter
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrCommonInfoStruct *pSigMgrCmnInfo,
	EcrioSigMgrTXNInfoStruct *pTXNInfo,
	u_char ** 	ppBuffer,
	u_int32*	SmsRetryCount
);

#endif
