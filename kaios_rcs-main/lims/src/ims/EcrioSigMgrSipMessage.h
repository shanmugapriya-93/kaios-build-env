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

#ifndef _ECRIO_SIG_MGR_SIP_MESSAGE_H_
#define  _ECRIO_SIG_MGR_SIP_MESSAGE_H_

u_int32 _EcrioSigMgrFormNameAddrWithParams
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrNameAddrWithParamsStruct *pAddrWithParamsStruct,
	u_char **ppUri
);

u_int32 _EcrioSigMgrAddRunTimeContact
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageStruct *pSipMessage,
	EcrioSigMgrNameAddrWithParamsStruct *pAddrWithParamsStruct,
	u_char **ppOut
);

/*SIP message forming function*/
u_int32 _EcrioSigMgrSipMessageForm
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrSipMessageModeEnum mode,
	EcrioSigMgrSipMessageStruct *pSipMessage,
	u_char **ppSipBuf,
	u_int32 *pSipLen
);

u_int32 _EcrioSigMgrSipMessageParse
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pMessage,
	EcrioSigMgrSipMessageStruct **ppDecodedMsg,
	u_int32 dMessageLength
);

u_int32 _EcrioSigMgrParseAddrSpec
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSigMgrUriStruct *pOutAddr
);

BoolEnum _EcrioSigMgrIsAccessWithinRange
(
	u_char *pStart,
	u_char *pEnd,
	u_int32 uJump
);

/************************************************************************************************************************************/

EcrioSipMessageTypeEnum _EcrioSigMgrGetMethodTypeFromString
(
	u_char *pMethod
);

u_int32	_EcrioSigMgrParseStringHeader
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd, \
	EcrioSipHeaderTypeEnum dType,
	EcrioSigMgrSipMessageStruct *pSipMessage
);

u_int32	_EcrioSigMgrParseAuthHeaders
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd, \
	EcrioSigMgrAuthenticationStruct **ppAuth
);

u_int32	_EcrioSigMgrParseAuthenticationInfoHeaders
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd, \
	EcrioSigMgrHeaderStruct **ppAuthenticationInfo
);

u_int32	_EcrioSigMgrParseContactHeader
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSigMgrContactStruct **ppContactHeader
);

u_int32	_EcrioSigMgrParseRouteHeader
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSigMgrRouteStruct **ppRouteSet
);

u_int32	_EcrioSigMgrParseReasonHeader
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSigMgrSipMessageStruct *pSipMessage
);

u_int32	_EcrioSigMgrParseCSeqHeader
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSigMgrSipMessageStruct *pSipMessage
);

u_int32	_EcrioSigMgrParseViaHeader
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	void *pOutHdr
);

u_int32	_EcrioSigMgrParseContentLengthHeader
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd, \
	void *pOutHdr,
	EcrioSipHeaderTypeEnum dType
);

u_int32 _EcrioSigMgrParseNameAddr
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd, \
	EcrioSigMgrNameAddrStruct *pNameAddr
);

u_int32 _EcrioSigMgrParseNameAddrWithParams
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd, \
	EcrioSigMgrNameAddrWithParamsStruct *pNameAddrParam
);

u_int32 _EcrioSigMgrParseSipParam
(
	EcrioSigMgrStruct *pSigMgr, \
	u_int16 *pNumParams,
	EcrioSigMgrParamStruct ***pppParams, \
	u_char *pStart,
	u_char *pEnd,
	BoolEnum valueReqd,
	u_char dLimiter
);

u_int32	_EcrioSigMgrParseOptionalHeaders
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSigMgrHeaderStruct **ppHeader
);

u_int32	_EcrioSigMgrParseWarningHeader
(
	EcrioSigMgrStruct *pSigMgr,
	u_char *pStart,
	u_char *pEnd,
	EcrioSigMgrHeaderStruct **ppHeader
);

#endif	// _ECRIO_SIG_MGR_SIP_MESSAGE_H_
