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

/**
 * @file EcrioPFD_Shared.h
 * @brief This is the header file of shared code for modules implemented
 * on the Android platform.
 *
 */

#ifndef __ECRIOPFD_SHARED_H__
#define __ECRIOPFD_SHARED_H__

/* Include common PFD definitions for building. */
// #include "EcrioPFD_Common.h"
#include "EcrioPAL.h"

/* Allow this module to be called directly from C++ compiled code. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 */
u_int32 GetTickCount
	();

/**
 *
 */
u_char *CreateString
(
	const char *pSourceString
);

/** @name Message Queue
 * \n A message queue that can be copied and used by any PFD module is
 * contained in this section. The message queue does not perform its own
 * thread synchronization. Therefore, the Calling Component should insure
 * that calls to any of the message queue functions are protected and that
 * no other entity modifies the memory of the parent structure at any time
 * between the calls to MsgQInit and MsgQDeinit.
 */
/*@{*/

/** @struct MsgQ
 * Each message in the message queue will have this structure, which is
 * inherently part of the message queue.
 */
typedef struct tagMsgQ
{
	int command;			/**< The id of the message, but still user specific. */
	u_int32 uParam;	/**< A user parameter for storing an u_int32eger value. */
	void *pParam;			/**< A user parameter for storing a void* to a user specific structure. */

	struct tagMsgQ *next;	/**< The next message in the message queue. */
} MsgQ;

/** @struct MsgQStruct
 * The parent structure for the message queue and meta data. Someday this
 * structure might hold the size and other elements.
 */
typedef struct tagMsgQStruct
{
	MsgQ *msgQ;
	MsgQ *msgQTail;
} MsgQStruct;

/** @struct MsgQMessage
 * A generic container for message data.
 */
typedef struct tagMsgQMessage
{
	int command;			/**< The id of the message, but still user specific. */
	u_int32 uParam;	/**< A user parameter for storing an u_int32eger value. */
	void *pParam;			/**< A user parameter for storing a void* to a user specific structure. */
} MsgQMessage;

/**
 *
 */
void MsgQInit
(
	MsgQStruct *msgQStruct
);

/**
 *
 */
int MsgQAddMessage
(
	MsgQStruct *msgQStruct,
	int command,
	u_int32 uParam,
	void *pParam
);

/**
 *
 */
int MsgQRemoveMessage
(
	MsgQStruct *msgQStruct
);

/**
 *
 */
void MsgQClear
(
	MsgQStruct *msgQStruct
);

/**
 *
 */
int GetMessage
(
	MsgQStruct *msgQStruct,
	MsgQMessage *msg
);

/**
 *
 */
void MsgQDeinit
(
	MsgQStruct *msgQStruct
);

/**
 *
 */
char *StripRightWhiteSpace
(
	char *s
);

/**
 *
 */
char *StripLeftWhiteSpace
(
	const char *s
);

/**
 *
 */
char *Trim
(
	char *strg
);

/**
 *
 */
u_int32 TrimCRLF
(
	char *pBuffer
);

/**
 *
 */
char *Find_Char
(
	const char *s,
	char c
);

/**
 *
 */
s_int32 IsWSpace
(
	char c
);

/**
 *
 */
s_int32 SkipLead
(
	char **pBuffer,
	s_int32 len
);

/**
 *
 */
s_int32 StringCopyUsingDelimiter
(
	char *pBuffer,
	s_int32 iBufLen,
	char *pValue,
	s_int32 *pValLen,
	char delim,
	s_int32 bIgnoreWS
);

#define ADDRESS_STRING_SIZE	   (64)

typedef struct
{
	char pIfName[ADDRESS_STRING_SIZE];
	char pIPv6[ADDRESS_STRING_SIZE];
	char pIPv4[ADDRESS_STRING_SIZE];
} InterfaceAddressStruct;

u_int32 getAllInterfaces
(
	InterfaceAddressStruct **ppIf,
	u_int32 *pCount
);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __ECRIOPFD_SHARED_H__ */
