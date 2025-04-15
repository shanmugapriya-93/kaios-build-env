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

#ifndef _ECRIO_SIG_MGR_REFER_MESSAGE_H_
#define _ECRIO_SIG_MGR_REFER_MESSAGE_H_


typedef enum
{
	EcrioSigMgrReferMethodNone = 0,
	EcrioSigMgrReferMethodINVITE,
	EcrioSigMgrReferMethodBYE
} EcrioSigMgrReferMethodEnum;

typedef struct
{
	BoolEnum bIsMultiRecipient;				/** TRUE if more than one principal is valid in list. */
	EcrioSigMgrReferMethodEnum eMethod;		/** Refer method. */
	u_char *pRecipient;						/** The URI of recipient, if only one principal is valid. */
	u_char *pContentId;						/** Pointer of URI-list, if more than one principal is valid. */
} EcrioSigMgrReferRecipientInfoStruct;

/** \brief This function is used to send REFER request.
 *
 *
 *
 * @pre                                 EcrioSigMgrInit() must be call.
 *
 * <b>Memory allocation:</b>	        <br> None.
 *
 * @param[in] sigMgrHandle				- Handle to the signaling manager.
 * @param[in] pCallId					- Call Id.
 * @param[in] pStruct					- Information for recipient.
 * @param[in] pReferReq					- Refer information.
 * @param[in] ppReferId					- ID to identify the REFER
 *
 * <b>Transaction:</b>                  <br> This function is used create and send Refer request.
 * @return						        One of the pre-defined Error Codes.
 */

u_int32 EcrioSigMgrSendRefer
(
	SIGMGRHANDLE sigMgrHandle,
	u_char *pCallId,
	EcrioSigMgrReferRecipientInfoStruct *pStruct,
	EcrioSigMgrSipMessageStruct *pReferReq,
	u_char** ppReferId
);

#endif /* _ECRIO_SIG_MGR_REFER_MESSAGE_H_ */
