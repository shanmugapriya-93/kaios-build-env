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

#ifndef __ECRIOSMSDATATYPES_H__
#define __ECRIOSMSDATATYPES_H__

typedef enum
{
	ECRIO_SMS_MSG_FORMAT_3GPP2 = 0,							/**<! 3GPP2 message format type. It is treated as default format type in SMS. */
	ECRIO_SMS_MSG_FORMAT_3GPP								/**<! 3GPP message format type. */
} EcrioSmsMsgFormatTypeEnums;

/** @enum EcrioSmsContentTransferEncodingEnum
* This enumeration defines different types of Content-Transfer-Encoding used for
* sending and reciving SMS.
*/
typedef enum
{
	EcrioSmsContentTransferEncoding_None = 0,				/**< No encoding type defined. */
	EcrioSmsContentTransferEncoding_Binary					/**< Binary encoding. */
} EcrioSmsContentTransferEncodingEnum;

/** @enum EcrioSmsMessageStatusType
* This enumeration defines different types of status notifications that can be
* sent to the calling layer via the EcrioSmsCallback function.
*/
typedef enum
{
	EcrioSmsMessage_Status_SEND,								/**< A message send was attempted by calling EcrioSMSSendMessage()
																and this status indicates the result. The actual result reason is
																a EcrioSMSMessageSendEnum enumeration encapsulated
																by a EcrioSMSGenericReasonStruct structure. */
	EcrioSmsMessage_Status_RECV									/**< An incoming message was received and validated and this status
																provides the message to the calling layer. The message and associated
																parameters are encapsulated by a EcrioSMSReceiveMsgStruct structure. */
} EcrioSmsMessageStatusType;

/** @struct EcrioSmsMessageStruct
* This structure provides the parameters which is used to send/receive the Message to/from the network.
*
*/
typedef struct
{
	u_char *pDest;											/**< Pointer to a null - terminated string holding the Mobile Device Number or MDN(which is basically a phone number). */
	u_int32 uMsgLen;										/**< The length, in bytes, of the binary SMS message to send.The length must not exceed 256 bytes, otherwise an error will be returned. */
	u_char *pMessage;										/**< Pointer to the binary  message data itself.Only the first dwMessageLen bytes will be processed. */
	EcrioSmsMsgFormatTypeEnums format;						/**< Message format type.*/
	EcrioSmsContentTransferEncodingEnum eEncoding;			/**< Enumeration that indicates the message encoding described in "Content-Transfer-Encoding" header. */
} EcrioSmsMessageStruct;

/** @struct EcrioSmsStatusStruct
* A structure used to convey basic status codes to be associated with a
* specific status notification. This simply encapsulates an enumerated
* integer type but can be expanded in the future.
*/
typedef struct
{
	u_int32 uStatusCode;										/**< The specific reason for the status notification. */
} EcrioSmsStatusStruct;

#endif /* #ifndef __ECRIOSMSDATATYPES_H__ */
