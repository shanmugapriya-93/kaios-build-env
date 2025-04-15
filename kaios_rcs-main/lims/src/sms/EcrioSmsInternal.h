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

#ifndef  _ECRIO_SMS_INTERNAL_H_
#define  _ECRIO_SMS_INTERNAL_H_

/* Import data types */
#include "EcrioSms.h"
#include "EcrioPAL.h"
#include "EcrioSigMgrCommon.h"
#include "EcrioSigMgrInit.h"
#include "EcrioSigMgrInstantMessage.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* response codes provided by Sig Mgr*/
#define SMS_ALC_1XX_RESP_RCVD							100
#define SMS_ALC_200_OK_RESP_RCVD						200
#define SMS_ALC_202_ACCEPTED_RESP_RCVD					202
#define SMS_ALC_3xx_RESP_RCVD							300
#define SMS_ALC_403_NOT_FOUND_RESP_RCVD					403
#define SMS_ALC_404_NOT_FOUND_RESP_RCVD					404
#define SMS_ALC_503_SERVICE_UNAVAILABLE_RESP_RCVD		503
#define SMS_ALC_6XX_RESP_RCVD							600
#define SMS_ALC_400_RESP_RCVD							400
#define SMS_ALC_401_RESP_RCVD							401
#define SMS_ALC_405_RESP_RCVD							405
#define SMS_ALC_406_RESP_RCVD							406
#define SMS_ALC_407_RESP_RCVD							407
#define SMS_ALC_408_RESP_RCVD							408
#define SMS_ALC_415_RESP_RCVD							415
#define SMS_ALC_416_RESP_RCVD							416
#define SMS_ALC_480_RESP_RCVD							480
#define SMS_ALC_500_RESP_RCVD							500
#define SMS_ALC_502_BAD_GATEWAY_RESP_RCVD				502
#define SMS_ALC_504_RESP_RCVD							504

#ifdef ENABLE_LOG
#define SMSLOGV(a, b, c, ...)		pal_LogMessage((a), KLogLevelVerbose, KLogComponentSMS | KLogComponent_ALC, (b), (c),##__VA_ARGS__);
#define SMSLOGI(a, b, c, ...)		pal_LogMessage((a), KLogLevelInfo, KLogComponentSMS | KLogComponent_ALC, (b), (c),##__VA_ARGS__);
#define SMSLOGD(a, b, c, ...)		pal_LogMessage((a), KLogLevelDebug, KLogComponentSMS | KLogComponent_ALC, (b), (c),##__VA_ARGS__);
#define SMSLOGW(a, b, c, ...)		pal_LogMessage((a), KLogLevelWarning, KLogComponentSMS | KLogComponent_ALC, (b), (c),##__VA_ARGS__);
#define SMSLOGE(a, b, c, ...)		pal_LogMessage((a), KLogLevelError, KLogComponentSMS | KLogComponent_ALC, (b), (c),##__VA_ARGS__);
#else
#define SMSLOGV(a, b, c, ...)		if (a != NULL) {;}
#define SMSLOGI(a, b, c, ...)		if (a != NULL) {;}
#define SMSLOGD(a, b, c, ...)		if (a != NULL) {;}
#define SMSLOGW(a, b, c, ...)		if (a != NULL) {;}
#define SMSLOGE(a, b, c, ...)		if (a != NULL) {;}
#endif

#define ECRIO_SMS_CONTENTTYPE_3GPP2SMS					(u_char *)"application/vnd.3gpp2.sms"
#define ECRIO_SMS_CONTENTTYPE_3GPPSMS					(u_char *)"application/vnd.3gpp.sms"
#define ECRIO_SMS_CONTENT_TRANSFER_ENCODING_BINARY		(u_char *)"binary"

/**********************************************************************************************/
/*                  SMS Internal Enumerated Constants Definition - START                    */
/**********************************************************************************************/

/**********************************************************************************************/
/*                  SMS Internal Enumerated Constants Definition - END                      */
/**********************************************************************************************/

/**********************************************************************************************/
/*                  SMS Internal Data Structure Definition - START                          */
/**********************************************************************************************/

/** Context manitained for SMS ALC. This is given to the application as
    ECRIOSMSHANDLE */

typedef struct
{
	EcrioSmsCallbackStruct callbackStruct;

	/** SAC / SUE / Signaling Manager bookkeeping */
	void *hEcrioSigMgrHandle;

	/* PAL and platform handles and bookkeeping*/
	LOGHANDLE hLogHandle;
} EcrioSMSContext;

/**********************************************************************************************/
/*                  SMS Internal Data Structure Definition - END                            */
/**********************************************************************************************/

/**********************************************************************************************/
/*                        Internal SMS-ALC FUNCTIONS - START                                             */
/**********************************************************************************************/

/** Function that will be called when the Sig Mgr calls the Status Callback of the SMS **/
void SMSSigMgrStatusCallback
(
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct,
	void *pCallbackData
);

/** Function that will be called when the Sig Mgr calls the InfoCallback of the SMS **/
void SMSSigMgrInfoCallback
(
	EcrioSigMgrInfoTypeEnum infoType,
	s_int32 infoCode,
	EcrioSigMgrInfoStruct *pData,
	void *pCallbackData
);

/**********************************************************************************************/
/*                        Internal SMS-ALC FUNCTIONS - END                                             */
/**********************************************************************************************/

void _EcrioSMSHandleMessageResponse
(
	EcrioSMSContext *pContextStruct,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
);

void _EcrioSMSHandleMessageRequest
(
	EcrioSMSContext *pContextStruct,
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*  _ECRIO_SMS_INTERNAL_H_ */
