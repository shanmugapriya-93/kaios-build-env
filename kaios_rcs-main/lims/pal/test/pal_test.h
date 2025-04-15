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

#ifndef __PAL_TEST_H__
#define __PAL_TEST_H__

/* Allow this module to be called directly from C++ compiled code. */
#ifdef __cplusplus
extern "C" {
#endif

// Functions to adjust and monitor the Memory PAL behavior.
void pal_memory_init
(
	void
);
void pal_memory_deinit
(
	void
);
void pal_memory_reset
(
	LOGHANDLE logHandle
);
unsigned int pal_memory_get_errorCount
(
	void
);
unsigned int pal_memory_get_allocationCount
(
	void
);
unsigned int pal_memory_get_operationCount
(
	void
);
unsigned int pal_memory_get_currentAllocation
(
	void
);
void pal_memory_set_failOnAlloc
(
	unsigned int count
);
bool pal_memory_get_failOnAllocTriggered
(
	void
);
void pal_memory_log_dump
(
	void
);

// Functions to adjust and monitor the Timer PAL behavior.
void pal_timer_reset
(
	void
);
void pal_timer_reset_StartCount
(
	void
);
unsigned int pal_timer_get_StartCount
(
	void
);
void pal_timer_reset_StopCount
(
	void
);
unsigned int pal_timer_get_StopCount
(
	void
);

// Gets the number of timers that are currently started.
int pal_timer_get_timerCount
(
	void
);

// Gets the interval for a specific timer, by index.
unsigned int pal_timer_get_timerInterval
(
	unsigned int timerIndex
);

// Functions to adjust and monitor the Socket PAL behavior.
void pal_socket_reset
(
	void
);
void pal_socket_reset_CreateCount
(
	void
);
unsigned int pal_socket_get_CreateCount
(
	void
);
void pal_socket_reset_SetOptionCount
(
	void
);
unsigned int pal_socket_get_SetOptionCount
(
	void
);
void pal_socket_reset_SetCallbacksCount
(
	void
);
unsigned int pal_socket_get_SetCallbacksCount
(
	void
);
void pal_socket_reset_SetLocalHostCount
(
	void
);
unsigned int pal_socket_get_SetLocalHostCount
(
	void
);
void pal_socket_reset_SetRemoteHostCount
(
	void
);
unsigned int pal_socket_get_SetRemoteHostCount
(
	void
);
void pal_socket_reset_GetLocalHostCount
(
	void
);
unsigned int pal_socket_get_GetLocalHostCount
(
	void
);
void pal_socket_reset_GetRemoteHostCount
(
	void
);
unsigned int pal_socket_get_GetRemoteHostCount
(
	void
);
void pal_socket_reset_OpenCount
(
	void
);
unsigned int pal_socket_get_OpenCount
(
	void
);
void pal_socket_reset_SendDataCount
(
	void
);
unsigned int pal_socket_get_SendDataCount
(
	void
);
void pal_socket_reset_CloseCount
(
	void
);
unsigned int pal_socket_get_CloseCount
(
	void
);

// Gets the number of sockets that are currently created.
int pal_socket_get_socketCount
(
	void
);

// Gets the number of buffers for a specific socket, by index.
int pal_socket_get_bufferCount
(
	unsigned int socketIndex
);

// Gets the pointer to and size of a specific buffer, by index.
unsigned char *pal_socket_get_buffer
(
	unsigned int socketIndex,
	unsigned int bufferIndex,
	unsigned int *uSize
);

// Issues accept callback to caller to setup a new TCP client channel upon connection.
unsigned int pal_socket_issue_accept_callback
(
	unsigned int socketIndex,
	ProtocolVersionEnum eProto,
	unsigned char *pIp,
	unsigned short uPort
);

// Sends a buffer via callback for a specific socket, by index.
void pal_socket_issue_recv_callback
(
	unsigned int socketIndex,
	unsigned char *pData,
	unsigned int uSize
);

// Functions to adjust and monitor the Utility PAL behavior.
void pal_utility_reset
(
	void
);
void pal_utility_reset_RandomNumberCount
(
	void
);
unsigned int pal_utility_get_RandomNumberCount
(
	void
);
void pal_utility_set_RandomNumberValue
(
	unsigned int value
);
void pal_utility_set_RandomNumberIncrement
(
	unsigned int value
);
void pal_utility_reset_GetMillisecondCount
(
	void
);
unsigned int pal_utility_get_GetMillisecondCount
(
	void
);
void pal_utility_set_GetMillisecondValue
(
	unsigned int value
);
void pal_utility_set_GetMillisecondIncrement
(
	unsigned int value
);

#ifdef __cplusplus
}
#endif

#endif /** __PAL_TEST_H__ */
