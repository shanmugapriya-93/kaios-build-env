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

#include "EcrioPAL.h"

u_int32 pal_Init
(
	LOGHANDLE logHandle,
	PALINSTANCE *pal
)
{
	return KPALErrorNone;
}

void pal_Deinit
(
	PALINSTANCE handle
)
{
}

LOGHANDLE pal_LogInit
(
	void *pParameters,
	EcrioLogOutput logOutput,
	u_int32 uLogFormat,
	u_int32 uLogLevel,
	u_int32 uLogComponent,
	u_int32 uLogType
)
{
	return NULL;
}

void pal_LogDeinit
(
	LOGHANDLE *handle
)
{

}

void pal_LogMessage
(
	LOGHANDLE handle,
	EcrioLogLevel level,
	u_int32 uLogComponent,
	u_int32 uLogType,
	const char *format,
	...
)
{

}

void pal_LogMessageDump
(
	LOGHANDLE handle,
	EcrioLogLevel level,
	u_int32 uLogComponent,
	u_int32 uLogType,
	u_char *pBuffer,
	u_int32 length
)
{

}

u_int32 pal_MemoryAllocate
(
	u_int32 memLength,
	void** ppMem
)
{
	return KPALErrorNone;
}

u_int32 pal_MemoryReallocate
(
	u_int32 memLength,
	void** ppMem
)
{
	return KPALErrorNone;
}

u_int32 pal_MemorySet
(
	void* pMem,
	s_int32 value,
	u_int32 memLength
)
{
	return KPALErrorNone;
}

u_int32 pal_MemoryCopy
(
	void* pDest,
	const void* pSource,
	u_int32 memLength
)
{
	return KPALErrorNone;
}

u_int32 pal_MemoryFree
(
	void** ppPtr
)
{
	return KPALErrorNone;
}

s_int32 pal_MemoryCompare
(
	const void* pBuf1,
	const void* pBuf2,
	u_int32 memLength
)
{
	return 0;
}

u_char *pal_MemorySearch
(
	const void *pSource,
	u_int32 memLength,
	const u_char *pSearchMem,
	u_int32 searchLength
)
{
	return NULL;
}

u_int32 pal_MutexCreate
(
	PALINSTANCE pal,
	MUTEXHANDLE *handle
)
{
	return KPALErrorNone;
}

u_int32 pal_MutexDelete
(
	MUTEXHANDLE* handle
)
{
	return KPALErrorNone;
}

u_int32 pal_MutexLock
(
	MUTEXHANDLE handle
)
{
	return KPALErrorNone;
}

u_int32 pal_MutexUnlock
(
	MUTEXHANDLE handle
)
{
	return KPALErrorNone;
}

u_int32 pal_MutexGlobalLock
(
	PALINSTANCE pal
)
{
	return KPALErrorNone;
}

u_int32 pal_MutexGlobalUnlock
(
	PALINSTANCE pal
)
{
	return KPALErrorNone;
}

u_int32 pal_SocketCreate
(
	PALINSTANCE pal,
	SocketCreateConfigStruct *pConfig,
	SocketCallbackFnStruct *pFnStruct,
	SOCKETHANDLE *handle
)
{
	return KPALErrorNone;
}

u_int32 pal_SocketSetOption
(
	SOCKETHANDLE handle,
	SocketOptionEnum option,
	void *value
)
{
	return KPALErrorNone;
}

u_int32 pal_SocketSetCallbacks
(
	SOCKETHANDLE handle,
	SocketCallbackFnStruct* pFnStruct
)
{
	return KPALErrorNone;
}

u_int32 pal_SocketSetLocalHost
(
	SOCKETHANDLE handle,
	const u_char* pLocalIP,
	u_int16 localPort
)
{
	return KPALErrorNone;
}

u_int32 pal_SocketSetRemoteHost
(
	SOCKETHANDLE handle,
	const u_char* pRemoteIP,
	u_int16 remotePort
)
{
	return KPALErrorNone;
}

u_int32 pal_SocketGetLocalHost
(
	SOCKETHANDLE handle,
	u_char* pLocalIP,
	u_int16* pLocalPort
)
{
	return KPALErrorNone;
}

u_int32 pal_SocketGetRemoteHost
(
	SOCKETHANDLE handle,
	u_char* pRemoteIp,
	u_int16* pRemotePort
)
{
	return KPALErrorNone;
}

u_int32 pal_SocketOpen
(
	SOCKETHANDLE handle
)
{
	return KPALErrorNone;
}

u_int32 pal_SocketSendData
(
	SOCKETHANDLE handle,
	const u_char* pDataBuffer,
	u_int32 dataLength
)
{
	return KPALErrorNone;
}

u_int32 pal_SocketClose
(
	SOCKETHANDLE* handle
)
{
	return KPALErrorNone;
}


u_char* pal_StringCreate
(
	const u_char* pSourceString,
	s_int32 stringLength
)
{
	return NULL;
}

s_int32 pal_StringLength
(
	const u_char* pString
)
{
	return 0;
}

u_char* pal_StringNCopy
(
	u_char* pDestString,
	const u_char* pSourceString,
	s_int32 count
)
{
	return NULL;
}

u_char* pal_StringNConcatenate
(
	u_char* pDestString,
	const u_char* pSourceString,
	s_int32 count
)
{
	return NULL;
}

s_int32 pal_StringCompare
(
	const u_char* str1,
	const u_char* str2
)
{
	return 0;
}

s_int32 pal_StringICompare
(
	const u_char* str1,
	const u_char* str2
)
{
	return 0;
}

s_int32 pal_StringNCompare
(
	const u_char* str1,
	const u_char* str2,
	s_int32 count
)
{
	return 0;
}

s_int32 pal_StringNICompare
(
	const u_char* str1,
	const u_char* str2,
	s_int32 count
)
{
	return 0;
}

u_char* pal_StringFindSubString
(
	const u_char* pString,
	const u_char* pSubString
)
{
	return NULL;
}

u_int32 pal_StringConvertToUNum
(
	const u_char* pString,
	u_char** pEnd,
	u_int32 base
)
{
	return 0;
}

u_char *pal_StringFindChar
(
	const u_char *pString,
	const u_char ch
)
{
	return NULL;
}

u_char *pal_StringFindLastChar
(
	const u_char *pString,
	const u_char ch 
)
{
	return NULL;
}

u_int32 pal_TimerStart
(
	PALINSTANCE pal,
	TimerStartConfigStruct *pConfig,
	TIMERHANDLE *handle
)
{
	return KPALErrorNone;
}

u_int32 pal_TimerStop
(
	TIMERHANDLE handle
)
{
	return KPALErrorNone;
}


u_int32 pal_UtilityRandomNumber
(
	void
)
{
	return 0;
}

u_int32 pal_UtilityGetMillisecondCount
(
	void
)
{
	return 0;
}

u_int32 pal_UtilityGetDateAndTime
(
	EcrioDateAndTimeStruct* pDateAndTime
)
{
	return 0;
}