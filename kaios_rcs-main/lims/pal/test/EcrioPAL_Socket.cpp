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
 * @file EcrioPAL_Socket.cpp
 * @brief This is a test version of Ecrio PAL's Socket Module.
 */

#include "EcrioPAL.h"

#include "pal_test.h"

#include <vector>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

class BufferItem
{
public:
unsigned int size;
unsigned char *data;

BufferItem
(
	unsigned int s,
	const unsigned char *d
)
	: size{ s }, data{ new unsigned char[s] }
{
	// Copy the buffer data.
	for (unsigned int i = 0; i < s; ++i)
	{
		data[i] = d[i];
	}
}

~BufferItem
	()
{
	delete[] data;
}
};

class SocketItem
{
public:
void *pRef;
SocketCreateConfigStruct config;
SocketCallbackFnStruct callbacks;

char localIP[64];
unsigned short uLocalPort;

char remoteIP[64];
unsigned short uRemotePort;

std::vector<BufferItem *> buffers;
};

class SocketClass
{
public:
LOGHANDLE logHandle;

unsigned int uCreateCount;
unsigned int uSetOptionCount;
unsigned int uSetCallbacksCount;
unsigned int uSetLocalHostCount;
unsigned int uSetRemoteHostCount;
unsigned int uGetLocalHostCount;
unsigned int uGetRemoteHostCount;
unsigned int uOpenCount;
unsigned int uSendDataCount;
unsigned int uCloseCount;

unsigned int uNextRef;
std::vector<SocketItem> sockets;
};

// Define the socket testing object globally.
SocketClass socketTest;

u_int32 pal_socketInit
(
	PALINSTANCE pal,
	LOGHANDLE logHandle,
	SOCKETINSTANCE *instance
)
{
	(void)pal;

	if (instance == NULL)
	{
		return KPALInvalidParameters;
	}

	// We don't use this function for testing (see socket_reset).

	socketTest.logHandle = logHandle;

	*instance = &socketTest;

	return KPALErrorNone;
}

void pal_socketDeinit
(
	SOCKETINSTANCE instance
)
{
	(void)instance;

	// We don't use this function for testing
}

u_int32 pal_SocketCreate
(
	PALINSTANCE pal,
	SocketCreateConfigStruct *pConfig,
	SocketCallbackFnStruct *pFnStruct,
	SOCKETHANDLE *handle
)
{
	/* On Windows we require the PAL handle to be valid. */
	if (pal == NULL)
	{
		return KPALInvalidHandle;
	}

	/* The pointer to the socket type must not be NULL. */
	if (pConfig == NULL)
	{
		return KPALInvalidParameters;
	}

	// Only record entry to the function if parameter checks pass.
	socketTest.uCreateCount++;

	SocketItem socketItem;

	// Cast the reference value as a pointer.
	socketItem.pRef = (void *)socketTest.uNextRef;
	socketTest.uNextRef++;

	// @todo We may want functions to check and verify the types of sockets being created...

	socketItem.config.protocolVersion = pConfig->protocolVersion;
	socketItem.config.protocolType = pConfig->protocolType;
	socketItem.config.socketType = pConfig->socketType;
	socketItem.config.socketMediaType = pConfig->socketMediaType;
	socketItem.config.bufferSize = pConfig->bufferSize;
	socketItem.config.bEnableGlobalMutex = pConfig->bEnableGlobalMutex;

	socketItem.callbacks.pAcceptCallbackFn = pFnStruct->pAcceptCallbackFn;
	socketItem.callbacks.pReceiveCallbackFn = pFnStruct->pReceiveCallbackFn;
	socketItem.callbacks.pErrorCallbackFn = pFnStruct->pErrorCallbackFn;
	socketItem.callbacks.pCallbackFnData = pFnStruct->pCallbackFnData;

	socketItem.localIP[0] = 0;
	socketItem.uLocalPort = 0;

	socketItem.remoteIP[0] = 0;
	socketItem.uRemotePort = 0;

	socketTest.sockets.push_back(socketItem);

	*handle = socketItem.pRef;

	return KPALErrorNone;
}

u_int32 pal_SocketSetLocalHost
(
	SOCKETHANDLE handle,
	const u_char *pLocalIP,
	u_int16 localPort
)
{
	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidHandle;
	}

	// Only record entry to the function if parameter checks pass.
	socketTest.uSetLocalHostCount++;

	std::vector<SocketItem>::iterator it;
	bool bFound = false;

	// Find the socket that we should work with.
	for (it = socketTest.sockets.begin(); it != socketTest.sockets.end(); ++it)
	{
		if (it->pRef == handle)
		{
			strncpy(&it->localIP[0], (const char *)pLocalIP, 64);
			it->uLocalPort = localPort;

			bFound = true;

			break;
		}
	}

	if (!bFound)
	{
		// @todo The created socket was not found!
	}

	return KPALErrorNone;
}

u_int32 pal_SocketSetRemoteHost
(
	SOCKETHANDLE handle,
	const u_char *pRemoteIP,
	u_int16 remotePort
)
{
	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidHandle;
	}

	/* The remote IP string must not be NULL. If it was, the underlying socket
	   would assign the localhost as the IP, which is not desired behavior. */
	if (pRemoteIP == NULL)
	{
		return KPALInvalidParameters;
	}

	// Only record entry to the function if parameter checks pass.
	socketTest.uSetRemoteHostCount++;

	std::vector<SocketItem>::iterator it;
	bool bFound = false;

	// Find the socket that we should work with.
	for (it = socketTest.sockets.begin(); it != socketTest.sockets.end(); ++it)
	{
		if (it->pRef == handle)
		{
			strncpy(&it->remoteIP[0], (const char *)pRemoteIP, 64);
			it->uRemotePort = remotePort;

			bFound = true;

			break;
		}
	}

	if (!bFound)
	{
		// @todo The created socket was not found!
	}

	return KPALErrorNone;
}

u_int32 pal_SocketGetLocalHost
(
	SOCKETHANDLE handle,
	u_char *pLocalIP,
	u_int16 *pLocalPort
)
{
	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidHandle;
	}

	/* The pointer to the local IP must not be NULL. */
	if (pLocalIP == NULL)
	{
		return KPALInvalidParameters;
	}

	/* The pointer to the local port must not be NULL. */
	if (pLocalPort == NULL)
	{
		return KPALInvalidParameters;
	}

	// Only record entry to the function if parameter checks pass.
	socketTest.uGetLocalHostCount++;

	std::vector<SocketItem>::iterator it;
	bool bFound = false;

	// Find the socket that we should work with.
	for (it = socketTest.sockets.begin(); it != socketTest.sockets.end(); ++it)
	{
		if (it->pRef == handle)
		{
			pLocalIP = (unsigned char *)&it->localIP[0];
			*pLocalPort = it->uLocalPort;

			bFound = true;

			break;
		}
	}

	if (!bFound)
	{
		// @todo The created socket was not found!
	}

	return KPALErrorNone;
}

u_int32 pal_SocketGetRemoteHost
(
	SOCKETHANDLE handle,
	u_char *pRemoteIp,
	u_int16 *pRemotePort
)
{
	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidHandle;
	}

	/* The pointer to the remote IP must not be NULL. */
	if (pRemoteIp == NULL)
	{
		return KPALInvalidParameters;
	}

	/* The pointer to the remote port must not be NULL. */
	if (pRemotePort == NULL)
	{
		return KPALInvalidParameters;
	}

	// Only record entry to the function if parameter checks pass.
	socketTest.uGetRemoteHostCount++;

	std::vector<SocketItem>::iterator it;
	bool bFound = false;

	// Find the socket that we should work with.
	for (it = socketTest.sockets.begin(); it != socketTest.sockets.end(); ++it)
	{
		if (it->pRef == handle)
		{
			pRemoteIp = (unsigned char *)&it->remoteIP[0];
			*pRemotePort = it->uRemotePort;

			bFound = true;

			break;
		}
	}

	if (!bFound)
	{
		// @todo The created socket was not found!
	}

	return KPALErrorNone;
}

u_int32 pal_SocketOpen
(
	SOCKETHANDLE handle
)
{
	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidHandle;
	}

	// Only record entry to the function if parameter checks pass.
	socketTest.uOpenCount++;

	std::vector<SocketItem>::iterator it;
	bool bFound = false;

	// Find the socket that we should work with.
	for (it = socketTest.sockets.begin(); it != socketTest.sockets.end(); ++it)
	{
		if (it->pRef == handle)
		{
			// @note For testing, we will assume that local port is never 0.
			// Otherwise we would need to take some action here.

			// Not much to do, we just assume that it is open.

			bFound = true;

			break;
		}
	}

	if (!bFound)
	{
		// @todo The created socket was not found!
	}

	return KPALErrorNone;
}

u_int32 pal_SocketSendData
(
	SOCKETHANDLE handle,
	const u_char *pDataBuffer,
	u_int32 dataLength
)
{
	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidHandle;
	}

	/** We will allow a NULL pointer only if the length is 0. */
	if (pDataBuffer == NULL)
	{
		if (dataLength != 0)
		{
			return KPALInvalidParameters;
		}
	}

	// Only record entry to the function if parameter checks pass.
	socketTest.uSendDataCount++;

	if (dataLength == 0)
	{
		// @todo Do we need to handle this and interact with the test at all?
		return KPALErrorNone;
	}

	std::vector<SocketItem>::iterator it;
	bool bFound = false;

	// Find the socket that we should work with.
	for (it = socketTest.sockets.begin(); it != socketTest.sockets.end(); ++it)
	{
		if (it->pRef == handle)
		{
			// Save the send buffer.

			BufferItem *bufferItem = new BufferItem(dataLength, pDataBuffer);

			it->buffers.push_back(bufferItem);

			bFound = true;

			break;
		}
	}

	if (!bFound)
	{
		// @todo The created socket was not found!
	}

	return KPALErrorNone;
}

u_int32 pal_SocketClose
(
	SOCKETHANDLE *handle
)
{
	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidParameters;
	}

	/* For Deinit() functions, the Handle must NOT be NULL. */
	if (*handle == NULL)
	{
		return KPALInvalidHandle;
	}

	// Only record entry to the function if parameter checks pass.
	socketTest.uCloseCount++;

	// Hang on to all the memory unless reset or otherwise controlled, for test review.

	return KPALErrorNone;
}

void pal_socket_reset
(
	void
)
{
	socketTest.uNextRef = 1;

	socketTest.uCreateCount = 0;
	socketTest.uSetOptionCount = 0;
	socketTest.uSetCallbacksCount = 0;
	socketTest.uSetLocalHostCount = 0;
	socketTest.uSetRemoteHostCount = 0;
	socketTest.uGetLocalHostCount = 0;
	socketTest.uGetRemoteHostCount = 0;
	socketTest.uOpenCount = 0;
	socketTest.uSendDataCount = 0;
	socketTest.uCloseCount = 0;

	std::vector<SocketItem>::iterator it;

	// Find the socket that we should work with.
	for (it = socketTest.sockets.begin(); it != socketTest.sockets.end(); ++it)
	{
		std::vector<BufferItem *>::iterator bit;

		// Find and release all buffers.
		for (bit = it->buffers.begin(); bit != it->buffers.end(); ++bit)
		{
			// Free the BufferItem that we created. The internal data will be released by destructor.
			delete(*bit);
		}

		// Clear the buffers vector.
		it->buffers.clear();
	}

	socketTest.sockets.clear();
}

void pal_socket_reset_CreateCount
(
	void
)
{
	socketTest.uCreateCount = 0;
}

unsigned int pal_socket_get_CreateCount
(
	void
)
{
	return socketTest.uCreateCount;
}

void pal_socket_reset_SetOptionCount
(
	void
)
{
	socketTest.uSetOptionCount = 0;
}

unsigned int pal_socket_get_SetOptionCount
(
	void
)
{
	return socketTest.uSetOptionCount;
}

void pal_socket_reset_SetCallbacksCount
(
	void
)
{
	socketTest.uSetCallbacksCount = 0;
}

unsigned int pal_socket_get_SetCallbacksCount
(
	void
)
{
	return socketTest.uSetCallbacksCount;
}

void pal_socket_reset_SetLocalHostCount
(
	void
)
{
	socketTest.uSetLocalHostCount = 0;
}

unsigned int pal_socket_get_SetLocalHostCount
(
	void
)
{
	return socketTest.uSetLocalHostCount;
}

void pal_socket_reset_SetRemoteHostCount
(
	void
)
{
	socketTest.uSetRemoteHostCount = 0;
}

unsigned int pal_socket_get_SetRemoteHostCount
(
	void
)
{
	return socketTest.uSetRemoteHostCount;
}

void pal_socket_reset_GetLocalHostCount
(
	void
)
{
	socketTest.uGetLocalHostCount = 0;
}

unsigned int pal_socket_get_GetLocalHostCount
(
	void
)
{
	return socketTest.uGetLocalHostCount;
}

void pal_socket_reset_GetRemoteHostCount
(
	void
)
{
	socketTest.uGetRemoteHostCount = 0;
}

unsigned int pal_socket_get_GetRemoteHostCount
(
	void
)
{
	return socketTest.uGetRemoteHostCount;
}

void pal_socket_reset_OpenCount
(
	void
)
{
	socketTest.uOpenCount = 0;
}

unsigned int pal_socket_get_OpenCount
(
	void
)
{
	return socketTest.uOpenCount;
}

void pal_socket_reset_SendDataCount
(
	void
)
{
	socketTest.uSendDataCount = 0;
}

unsigned int pal_socket_get_SendDataCount
(
	void
)
{
	return socketTest.uSendDataCount;
}

void pal_socket_reset_CloseCount
(
	void
)
{
	socketTest.uCloseCount = 0;
}

unsigned int pal_socket_get_CloseCount
(
	void
)
{
	return socketTest.uCloseCount;
}

int pal_socket_get_socketCount
(
	void
)
{
	return socketTest.sockets.size();
}

int pal_socket_get_bufferCount
(
	unsigned int socketIndex
)
{
	if (socketTest.sockets.size() >= (socketIndex + 1))
	{
		return socketTest.sockets.at(socketIndex).buffers.size();
	}
	else
	{
		return -1;
	}
}

unsigned char *pal_socket_get_buffer
(
	unsigned int socketIndex,
	unsigned int bufferIndex,
	unsigned int *uSize
)
{
	*uSize = socketTest.sockets.at(socketIndex).buffers.at(bufferIndex)->size;
	return socketTest.sockets.at(socketIndex).buffers.at(bufferIndex)->data;
}

unsigned int pal_socket_issue_accept_callback
(
	unsigned int socketIndex,
	ProtocolVersionEnum eProto,
	unsigned char *pIp,
	unsigned short uPort
)
{
	if (socketTest.sockets.at(socketIndex).config.protocolType == ProtocolType_TCP_Server)
	{
		SocketItem socketItem;

		// Cast the reference value as a pointer.
		socketItem.pRef = (void *)socketTest.uNextRef;
		socketTest.uNextRef++;

		BoolEnum bAccept = Enum_FALSE;

		socketTest.sockets.at(socketIndex).callbacks.pAcceptCallbackFn(
			socketTest.sockets.at(socketIndex).pRef,
			socketItem.pRef,
			socketTest.sockets.at(socketIndex).callbacks.pCallbackFnData,
			eProto,
			pIp,
			uPort,
			&bAccept,
			KPALErrorNone);

		if (bAccept == Enum_TRUE)
		{
			socketItem.config.protocolVersion = eProto;
			socketItem.config.protocolType = ProtocolType_TCP_Client;
			socketItem.config.socketType = SocketType_Default;
			socketItem.config.socketMediaType = SocketMediaType_Default;
			socketItem.config.bufferSize = 2048;	// Doesn't really matter.
			socketItem.config.bEnableGlobalMutex = socketTest.sockets.at(socketIndex).config.bEnableGlobalMutex;

			socketItem.callbacks.pAcceptCallbackFn = NULL;
			socketItem.callbacks.pReceiveCallbackFn = socketTest.sockets.at(socketIndex).callbacks.pReceiveCallbackFn;
			socketItem.callbacks.pErrorCallbackFn = socketTest.sockets.at(socketIndex).callbacks.pErrorCallbackFn;
			socketItem.callbacks.pCallbackFnData = socketTest.sockets.at(socketIndex).callbacks.pCallbackFnData;

			// Doesn't matter...
			socketItem.localIP[0] = 0;
			socketItem.uLocalPort = 0;

			// Doesn't matter, but we will maintain it.
			strncpy(socketItem.remoteIP, (const char *)pIp, strlen((const char *)pIp));
			socketItem.uRemotePort = uPort;

			socketTest.sockets.push_back(socketItem);

			// Return the index, of the new socket item.
			return (socketTest.sockets.size() - 1);
		}
		else
		{
			// @todo Do we need to properly convey this information back to the test?
			socketTest.uNextRef--;
		}
	}

	// else Maybe we should have a better way to control this in testing...

	// @todo Someday make it more predictable for the caller to know about error.
	return 0;
}

void pal_socket_issue_recv_callback
(
	unsigned int socketIndex,
	unsigned char *pData,
	unsigned int uSize
)
{
	SocketReceiveStruct recv;

	// Simulate a real socket receive buffer, such that we don't clobber original data.
	// @note We are giving it to C, so we are using malloc for now.
	unsigned char *pReal = (unsigned char *)malloc(uSize);

	memcpy(pReal, pData, uSize);

	// @todo We may want to expose some of these options to the API so that
	// we can test different situations.
	recv.handle = socketTest.sockets.at(socketIndex).pRef;
	recv.pReceiveData = pReal;
	recv.receiveLength = uSize;
	recv.pFromIP = (const u_char *)"0.0.0.0";
	recv.fromPort = 0;
	recv.uTTL_HL = 0;
	recv.result = 0;

	socketTest.sockets.at(socketIndex).callbacks.pReceiveCallbackFn(socketTest.sockets.at(socketIndex).callbacks.pCallbackFnData, &recv);

	free(pReal);
}
