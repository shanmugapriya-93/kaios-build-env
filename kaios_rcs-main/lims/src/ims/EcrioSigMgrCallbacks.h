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

#ifndef _ECRIO_SIG_MGR_CALLBACK_H_
#define _ECRIO_SIG_MGR_CALLBACK_H_

/** Notifies upper Layer about the Error, Warnings and Informations*/
void _EcrioSigMgrSendInfo
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrInfoTypeEnum	infoType,
	s_int32 infoCode,
	void *pData
);

void _EcrioSigMgrUpdateStatus
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrMessageStruct *pMessage
);

/** Notifies upper layer about request or response events*/
void _EcrioSigMgrUpdateStatusSMS
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrMessageStruct *pMessage
);


#ifdef _ENABLE_DEFAULT_CALL_HANDLER_MOIP_
void _EcrioSigMgrUpdateStatusMoIP
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrMessageStruct *pMessage
);
#endif /* _ENABLE_DEFAULT_CALL_HANDLER_MOIP_ */

#ifdef _ENABLE_DEFAULT_CALL_HANDLER_CPM_
void _EcrioSigMgrUpdateStatusCPM
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioSigMgrMessageStruct *pMessage
);
#endif /* _ENABLE_DEFAULT_CALL_HANDLER_CPM_ */

void _EcrioSigMgrUpdateStatusUCE
(
EcrioSigMgrStruct *pSigMgr,
EcrioSigMgrMessageStruct *pMessage
);


#endif /* _ECRIO_SIG_MGR_CALLBACK_H_*/
