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

#include "EcrioSigMgrCommon.h"
#include "EcrioSigMgrInit.h"
#include "EcrioSigMgrInternal.h"
#include "EcrioSigMgrCallbacks.h"
#include "EcrioSigMgrIMSLibHandler.h"
#include "EcrioSigMgrUtilities.h"
#include "EcrioTXN.h"

/*****************************************************************************
                Internal API Declaration Section - Begin
*****************************************************************************/

/*****************************************************************************
                Internal API Declaration Section - End
*****************************************************************************/

/*****************************************************************************
                    API Definition Section - Begin
*****************************************************************************/

/*****************************************************************************

Function:		_EcrioSigMgrUDPSocketErrorCB()
Purpose:		Socket error call back from PAL.

Description:	Socket error call back from PAL..

Input:			SOCKETHANDLE socket - Socket Handle.
                void* pCallbackFnData - Callback data.
                u_char* pErrorMessage - error message.
                u_int32 error - error code.

OutPut:			None

Returns:		error code.
*****************************************************************************/

void _EcrioSigMgrSocketErrorCB
(
	SOCKETHANDLE socket,
	void *pCallbackFnData,
	u_int32 error
)
{
	EcrioSigMgrStruct *pSigMgr = NULL;

	error = error;
	socket = socket;

	if (pCallbackFnData == NULL)
	{
		return;
	}

	pSigMgr = (EcrioSigMgrStruct *)pCallbackFnData;

	if (pSigMgr->bIsDeInitializing == Enum_TRUE)
	{
		return;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tsocket=%p, pCallbackFnData=%p, error=%u",
		__FUNCTION__, __LINE__, socket, pCallbackFnData, error);

	if (error == KPALEntityClosed || error == KPALDataSendErrorFatal || error == KPALDataReceiveErrorFatal)
	{
		error = ECRIO_SIG_MGR_SIG_SOCKET_ERROR;

		_EcrioSigMgrSendInfo(pSigMgr, EcrioSigMgrError,
			(s_int32)ECRIO_SIG_MGR_SIG_SOCKET_ERROR, NULL);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);
}

/*****************************************************************************

Function:		_EcrioSigMgrUDPSocketReceieveCB()
Purpose:		Socket send callback call back from PAL.

Description:	Socket send callback call back from PAL.

Input:			SOCKETHANDLE socket - Socket Handle.
                void* pCallbackFnData - Callback data.
                u_char* pReceiveData - received buffer.
                u_int32 receiveLength - received length.
                u_int32 error - error code.

OutPut:			None

Returns:		error code.
*****************************************************************************/
void _EcrioSigMgrSocketReceieveCB
(
	void *pCallbackFnData,
	SocketReceiveStruct *pReceive
)
{
	EcrioSigMgrStruct *pSigMgr = NULL;
	u_int32	i = 0, uChannelIndex = 0;
	BoolEnum bChannelFound = Enum_FALSE;

	if (pCallbackFnData == NULL || pReceive == NULL)
	{
		return;
	}

	pSigMgr = (EcrioSigMgrStruct *)pCallbackFnData;

	if (pSigMgr->bIsDeInitializing == Enum_TRUE)
	{
		return;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tsocket=%p, pCallbackFnData=%p, receiveLength=%u pFromIP=%s fromPort=%u",
		__FUNCTION__, __LINE__, pReceive->handle, pCallbackFnData, pReceive->receiveLength, pReceive->pFromIP ? pReceive->pFromIP : (u_char *)"null", pReceive->fromPort);

	if (pReceive->pReceiveData == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tNo data received.",
			__FUNCTION__, __LINE__);

		goto Error_Level_01;
	}

	/* Check and skip is HTTP message is received. */
	if (pal_StringNCompare((const u_char*)pReceive->pReceiveData,
						   (const u_char*)"HTTP", pal_StringLength((const u_char*)"HTTP")) == 0)
	{
		SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tHTTP Data Received",
				   __FUNCTION__, __LINE__);
		SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeHTTP, pReceive->pReceiveData, pReceive->receiveLength);
		goto Error_Level_01;
	}

	if (pSigMgr->pSignalingInfo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tNo signalling information, error=%u",
			__FUNCTION__, __LINE__, pReceive->result);

		goto Error_Level_01;
	}

	if (pSigMgr->pSigMgrTransportStruct == NULL || pSigMgr->pSigMgrTransportStruct->pCommunicationStruct == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tpCommunicationStruct is NULL",
			__FUNCTION__, __LINE__);

		goto Error_Level_01;
	}

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tnoOfChannels : %d",
		__FUNCTION__, __LINE__, pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->noOfChannels);

	for (i = 0; i < pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->noOfChannels; i++)
	{
		if (pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[i].hChannelHandle == pReceive->handle)
		{
			uChannelIndex = pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[i].channelIndex;
			bChannelFound = Enum_TRUE;
			break;
		}
	}

	if (bChannelFound == Enum_TRUE)
	{
		if (pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[i].pChannelInfo->eSocketType == ProtocolType_TCP_Client)
		{
			// Handle the received TCP data buffer.
			EcrioTxnMgrSignalingCommunicationChannelInfoStruct *pChannelInfo = pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[i].pChannelInfo;
			u_char *pLoc;
			u_char *pCrlf;

			if ((pSigMgr->pSignalingInfo->bEnableTls == Enum_TRUE) && (pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex != 0))
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tDropping non-TLS receive TCP buffer!",__FUNCTION__, __LINE__);

				goto Error_Level_01;
			}

			// First, pre-allocate a large buffer for TCP storage (to simplify the algorithm and reduce memory operations).
			if (pChannelInfo->pTcpBuffer == NULL)
			{
				pChannelInfo->tcpBufferSize = 16384;
				// Is 16K large enough? If not, how large should it be?
				// Be sure to free in the channel structure release, if not NULL.
				// We may want another way to define this value.

				// @note We allocate here only when the TCP channel actually gets used.
				// This needs some review. We should pre-allocate earlier...
				pal_MemoryAllocate(pChannelInfo->tcpBufferSize, (void **)&pChannelInfo->pTcpBuffer);
				if (pChannelInfo->pTcpBuffer == NULL)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tCan't allocate TCP buffer!",
						__FUNCTION__, __LINE__);

					goto Error_Level_01;
				}

				pChannelInfo->tcpConsumedSize = 0;

				// Unfortunately, we do string operations on the buffer...
				pChannelInfo->pTcpBuffer[0] = 0;
			}

			if ((pReceive->receiveLength + pChannelInfo->tcpConsumedSize) >= pChannelInfo->tcpBufferSize)
			{
				// There is not enough room to store the new data! (The '=' is so we have room for a null terminator.)

				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tToo much TCP data for buffer!",
					__FUNCTION__, __LINE__);

				// Drop all the current data in hopes that we can recover with subsequent packets.
				pChannelInfo->tcpConsumedSize = 0;
				pChannelInfo->pTcpBuffer[0] = 0;

				goto Error_Level_01;
			}

			// Copy the new data to the TCP buffer. Wish we didn't have to add another copy to the mix.
			pal_MemoryCopy(&pChannelInfo->pTcpBuffer[pChannelInfo->tcpConsumedSize], pReceive->receiveLength, pReceive->pReceiveData, pReceive->receiveLength);
			pChannelInfo->tcpConsumedSize += pReceive->receiveLength;

			// Unfortunately, we do string operations on the buffer...
			pChannelInfo->pTcpBuffer[pChannelInfo->tcpConsumedSize] = 0;

			// Loop for all possible SIP messages we might find in the buffer.
			// We may want a limit, but should be safe...
			for (;;)
			{
				// Basic design is this:
				// - Look for Content-Length (as per RFC, it must be present for TCP).
				// - Look for the colon after Content-Length.
				// - Look for line terminator after Content-Length.
				// - Get Content-Length value.
				// - Based on the value, find the ending/terminator.
				// - Wait for the rest of data and then invoke callback.

				// We aren't handling upper/lower case, is it an issue?
				pLoc = pal_StringFindSubString(pChannelInfo->pTcpBuffer, (const u_char *)"Content-Length");
				if (pLoc != NULL)
				{
					// Is there enough data to get a Content-Length value?
					// Find the colon from "Content-Length: 0". There may be whitespace before and after the colon...
					pLoc = pal_StringFindSubString(pLoc, (const u_char *)":");
					if (pLoc != NULL)
					{
						// Find the line terminator after "Content-Length: 0".
						pCrlf = pal_StringFindSubString(pLoc, (const u_char *)"\r\n");
						if (pCrlf != NULL)
						{
							u_int32 uContentLength;

							uContentLength = pal_StringConvertToUNum(pLoc + 1, NULL, 10);

							// Find the next CRLFCRLF. If content length is 0, it terminates the frame. Otherwise, it is a separator.
							pCrlf = pal_StringFindSubString(pLoc, (const u_char *)"\r\n\r\n");
							if (pCrlf != NULL)
							{
								u_int32 cbLength;
								u_char *pTemp;

								if (uContentLength == 0)
								{
									// Length is the found pointer minus head of buffer, plus 4 for the CRLFCRLF.
									cbLength = (pCrlf - pChannelInfo->pTcpBuffer) + 4;
								}
								else
								{
									// Do we have all the content?
									if (uContentLength > (pChannelInfo->tcpConsumedSize - ((pCrlf - pChannelInfo->pTcpBuffer) + 4)))
									{
										// Not enough yet!
										break;
									}

									// Length is the found pointer minus head of buffer, plus 4 for the CRLFCRLF plus content length.
									cbLength = ((pCrlf - pChannelInfo->pTcpBuffer) + 4) + uContentLength;
								}

								// In this implementation, we will allocate a temporary buffer to pass back
								// the frame.
								// @todo Evaluate if the caller can handle an unterminated buffer. If so, we maybe don't need this allocation.
								// @note We may want to pre-allocate the buffer (or a buffer). But what size? Need some measurement and design.
								pal_MemoryAllocate(cbLength + 1, (void **)&pTemp);
								if (pTemp != NULL)
								{
									pal_MemoryCopy(pTemp, cbLength, pChannelInfo->pTcpBuffer, cbLength);
									pTemp[cbLength] = 0;	// Add the guard. Again, do we really need this???

									SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tIncoming TCP message received (%u bytes):",
										__FUNCTION__, __LINE__, cbLength);
									SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pTemp, cbLength);

									_EcrioSigMgrSetReceivedOnChannelIndexAndRemoteSendingPort(pSigMgr, uChannelIndex, pReceive->fromPort);
									_EcrioSigMgrIMSHandleMessage(pSigMgr, pTemp, cbLength);

									pal_MemoryFree((void **)&pTemp);
								}
								else
								{
									// Log this situation, but otherwise act as though it was consumed.
									SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tCan't allocate callback buffer for TCP data!",
										__FUNCTION__, __LINE__);
								}

								// We should have had a check there somewhere, to ensure the length doesn't exceed the amount of data we have!
								pChannelInfo->tcpConsumedSize = pChannelInfo->tcpConsumedSize - cbLength;
								if (pChannelInfo->tcpConsumedSize > 0)
								{
									// We need to shift any remaining data to the beginning of the buffer.
									// We could implement a circular buffer!!!

									// Ensure this is a memmove operation...
									pal_MemoryCopy(pChannelInfo->pTcpBuffer, pChannelInfo->tcpConsumedSize, &pChannelInfo->pTcpBuffer[cbLength], pChannelInfo->tcpConsumedSize);
								}

								pChannelInfo->pTcpBuffer[pChannelInfo->tcpConsumedSize] = 0;

								if (pChannelInfo->tcpConsumedSize == 0)
								{
									// No more data.
									break;
								}
								else
								{
									// See if we have more data to consume.
									continue;
								}
							}

							// else We didn't find crlfcrlf so wait for the next packet.
						}

						// else We didn't find crlf so wait for the next packet.
					}

					// else We didn't find colon so wait for the next packet.
				}

				// else We didn't find Content-Length so wait for the next packet.

				break;
			}
		}
		else if (pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[i].pChannelInfo->eSocketType == ProtocolType_TLS_Client)
		{
			// Handle the received TLS data buffer.
			EcrioTxnMgrSignalingCommunicationChannelInfoStruct *pChannelInfo = pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[i].pChannelInfo;
			u_char *pLoc;
			u_char *pCrlf;

			if ((pSigMgr->pSignalingInfo->bEnableTls == Enum_FALSE) && (pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex == 0))
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tDropping TLS message as TLS is disabled!", __FUNCTION__, __LINE__);

				goto Error_Level_01;
			}
			
			if (pChannelInfo->pTlsBuffer == NULL)
			{
				pChannelInfo->tlsBufferSize = 16384;
				// Is 16K large enough? If not, how large should it be?
				// Be sure to free in the channel structure release, if not NULL.
				// We may want another way to define this value.

				// @note We allocate here only when the TLS channel actually gets used.
				// This needs some review. We should pre-allocate earlier...
				pal_MemoryAllocate(pChannelInfo->tlsBufferSize, (void **)&pChannelInfo->pTlsBuffer);
				if (pChannelInfo->pTlsBuffer == NULL)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tCan't allocate TLS buffer!",
						__FUNCTION__, __LINE__);

					goto Error_Level_01;
				}

				pChannelInfo->tlsConsumedSize = 0;

				// Unfortunately, we do string operations on the buffer...
				pChannelInfo->pTlsBuffer[0] = 0;
			}

			if ((pReceive->receiveLength + pChannelInfo->tlsConsumedSize) >= pChannelInfo->tlsBufferSize)
			{
				// There is not enough room to store the new data! (The '=' is so we have room for a null terminator.)

				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tToo much TLS data for buffer!",
					__FUNCTION__, __LINE__);

				// Drop all the current data in hopes that we can recover with subsequent packets.
				pChannelInfo->tlsConsumedSize = 0;
				pChannelInfo->pTlsBuffer[0] = 0;

				goto Error_Level_01;
			}

			// Copy the new data to the TLS buffer. Wish we didn't have to add another copy to the mix.
			pal_MemoryCopy(&pChannelInfo->pTlsBuffer[pChannelInfo->tlsConsumedSize], pReceive->receiveLength, pReceive->pReceiveData, pReceive->receiveLength);

			pChannelInfo->tlsConsumedSize += pReceive->receiveLength;

			// Unfortunately, we do string operations on the buffer...
			pChannelInfo->pTlsBuffer[pChannelInfo->tlsConsumedSize] = 0;

			// Loop for all possible SIP messages we might find in the buffer.
			// We may want a limit, but should be safe...
			for (;;)
			{
				// Basic design is this:
				// - Look for Content-Length (as per RFC, it must be present for TCP).
				// - Look for the colon after Content-Length.
				// - Look for line terminator after Content-Length.
				// - Get Content-Length value.
				// - Based on the value, find the ending/terminator.
				// - Wait for the rest of data and then invoke callback.

				// We aren't handling upper/lower case, is it an issue?
				pLoc = pal_StringFindSubString(pChannelInfo->pTlsBuffer, (const u_char *)"Content-Length");
				if (pLoc != NULL)
				{
					// Is there enough data to get a Content-Length value?
					// Find the colon from "Content-Length: 0". There may be whitespace before and after the colon...
					pLoc = pal_StringFindSubString(pLoc, (const u_char *)":");
					if (pLoc != NULL)
					{
						// Find the line terminator after "Content-Length: 0".
						pCrlf = pal_StringFindSubString(pLoc, (const u_char *)"\r\n");
						if (pCrlf != NULL)
						{
							u_int32 uContentLength;

							uContentLength = pal_StringConvertToUNum(pLoc + 1, NULL, 10);

							// Find the next CRLFCRLF. If content length is 0, it terminates the frame. Otherwise, it is a separator.
							pCrlf = pal_StringFindSubString(pLoc, (const u_char *)"\r\n\r\n");
							if (pCrlf != NULL)
							{
								u_int32 cbLength;
								u_char *pTemp;

								if (uContentLength == 0)
								{
									// Length is the found pointer minus head of buffer, plus 4 for the CRLFCRLF.
									cbLength = (pCrlf - pChannelInfo->pTlsBuffer) + 4;
								}
								else
								{
									// Do we have all the content?
									if (uContentLength > (pChannelInfo->tlsConsumedSize - ((pCrlf - pChannelInfo->pTlsBuffer) + 4)))
									{
										// Not enough yet!
										break;
									}

									// Length is the found pointer minus head of buffer, plus 4 for the CRLFCRLF plus content length.
									cbLength = ((pCrlf - pChannelInfo->pTlsBuffer) + 4) + uContentLength;
								}

								// In this implementation, we will allocate a temporary buffer to pass back
								// the frame.
								// @todo Evaluate if the caller can handle an unterminated buffer. If so, we maybe don't need this allocation.
								// @note We may want to pre-allocate the buffer (or a buffer). But what size? Need some measurement and design.
								pal_MemoryAllocate(cbLength + 1, (void **)&pTemp);
								if (pTemp != NULL)
								{
									pal_MemoryCopy(pTemp, cbLength, pChannelInfo->pTlsBuffer, cbLength);
									pTemp[cbLength] = 0;	// Add the guard. Again, do we really need this???

									SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tIncoming TLS message received (%u bytes):",
										__FUNCTION__, __LINE__, cbLength);
									SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pTemp, cbLength);

									_EcrioSigMgrSetReceivedOnChannelIndexAndRemoteSendingPort(pSigMgr, uChannelIndex, pReceive->fromPort);
									_EcrioSigMgrIMSHandleMessage(pSigMgr, pTemp, cbLength);

									pal_MemoryFree((void **)&pTemp);
								}
								else
								{
									// Log this situation, but otherwise act as though it was consumed.
									SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tCan't allocate callback buffer for TLS data!",
										__FUNCTION__, __LINE__);
								}

								// We should have had a check there somewhere, to ensure the length doesn't exceed the amount of data we have!
								pChannelInfo->tlsConsumedSize = pChannelInfo->tlsConsumedSize - cbLength;
								if (pChannelInfo->tlsConsumedSize > 0)
								{
									// We need to shift any remaining data to the beginning of the buffer.
									// We could implement a circular buffer!!!

									// Ensure this is a memmove operation...
									pal_MemoryCopy(pChannelInfo->pTlsBuffer, pChannelInfo->tlsConsumedSize, &pChannelInfo->pTlsBuffer[cbLength], pChannelInfo->tlsConsumedSize);
								}

								pChannelInfo->pTlsBuffer[pChannelInfo->tlsConsumedSize] = 0;

								if (pChannelInfo->tlsConsumedSize == 0)
								{
									// No more data.
									break;
								}
								else
								{
									// See if we have more data to consume.
									continue;
								}
							}

							// else We didn't find crlfcrlf so wait for the next packet.
						}

						// else We didn't find crlf so wait for the next packet.
					}

					// else We didn't find colon so wait for the next packet.
				}

				// else We didn't find Content-Length so wait for the next packet.

				break;
			}
		}
		else
		{
			// Handle the received UDP data buffer.
			if ((pSigMgr->pSignalingInfo->bEnableTls == Enum_TRUE) && (pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex != 0))
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tDropping non-TLS receive UDP buffer!",__FUNCTION__, __LINE__);

				goto Error_Level_01;
			}

			SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tIncoming UDP message received (%u bytes):",
				__FUNCTION__, __LINE__, pReceive->receiveLength);
			SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pReceive->pReceiveData, pReceive->receiveLength);

			_EcrioSigMgrSetReceivedOnChannelIndexAndRemoteSendingPort(pSigMgr, uChannelIndex, pReceive->fromPort);
			_EcrioSigMgrIMSHandleMessage(pSigMgr, pReceive->pReceiveData, pReceive->receiveLength);
		}
	}

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, pReceive->result);
}

void _EcrioSigMgrSocketAcceptCB
(
	SOCKETHANDLE listenHandle,
	SOCKETHANDLE acceptHandle,
	void *pCallbackFnData,
	ProtocolVersionEnum remoteIPVersion,
	const u_char *pRemoteIP,
	u_int16 remotePort,
	BoolEnum *pAccept,
	u_int32 result
)
{
	EcrioSigMgrStruct *pSigMgr = NULL;
	EcrioSigMgrNetworkInfoStruct *pNetworkInfo = NULL;
	EcrioTxnMgrSignalingCommunicationStruct	*pCommunicationStruct = NULL;
	u_int32	i = 0, uNoChannels = 0;
	BoolEnum bFound = Enum_FALSE;

	*pAccept = Enum_FALSE;
	result = result;

	if (pCallbackFnData == NULL)
	{
		return;
	}

	pSigMgr = (EcrioSigMgrStruct *)pCallbackFnData;

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u - listenHandle:%d, acceptHandle : %d, remoteIPVersion : %d, pRemoteIP:%s, remotePort:%d",
		__FUNCTION__, __LINE__, listenHandle, acceptHandle, remoteIPVersion, pRemoteIP, remotePort);

	/* validate incoming handles and remote IP */
	if (listenHandle == NULL || acceptHandle == NULL || pRemoteIP == NULL)
	{
		goto Error_Level_01;
	}

	if (pSigMgr->pSigMgrTransportStruct != NULL)
	{
		pCommunicationStruct = pSigMgr->pSigMgrTransportStruct->pCommunicationStruct;
		pNetworkInfo = pSigMgr->pNetworkInfo;

		if (pCommunicationStruct != NULL && pNetworkInfo != NULL)
		{
			/* Check arithmetic overflow */
			if (pal_UtilityArithmeticOverflowDetected(pCommunicationStruct->noOfChannels, 1) == Enum_TRUE)
			{
				goto Error_Level_01;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((pCommunicationStruct->noOfChannels + 1), sizeof(EcrioTxnMgrSignalingCommunicationChannelStruct)) == Enum_TRUE)
			{
				goto Error_Level_01;
			}

			for (i = 0; i < pCommunicationStruct->noOfChannels; i++)
			{
				if (pCommunicationStruct->pChannels[i].hChannelHandle == listenHandle)
				{
					bFound = Enum_TRUE;
					break;
				}
			}

			if (bFound == Enum_TRUE)
			{
				pal_MemoryReallocate((pCommunicationStruct->noOfChannels + 1) * sizeof(EcrioTxnMgrSignalingCommunicationChannelStruct), (void **)&pCommunicationStruct->pChannels);
				if (NULL == pCommunicationStruct->pChannels)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u - memory reallocate to pChannels failed",
						__FUNCTION__, __LINE__);
					goto Error_Level_01;
				}

				uNoChannels = pCommunicationStruct->noOfChannels;

				SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u - noOfChannels : %d",
					__FUNCTION__, __LINE__, pCommunicationStruct->noOfChannels);

				pal_MemoryAllocate(sizeof(EcrioTxnMgrSignalingCommunicationChannelInfoStruct),
					(void **)&pCommunicationStruct->pChannels[uNoChannels].pChannelInfo);
				if (NULL == pCommunicationStruct->pChannels[uNoChannels].pChannelInfo)
				{
					goto Error_Level_01;
				}

				pCommunicationStruct->noOfChannels = uNoChannels + 1;

				SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u - noOfChannels : %d",
					__FUNCTION__, __LINE__, pCommunicationStruct->noOfChannels);

				pCommunicationStruct->pChannels[uNoChannels].pChannelInfo->eSocketType = ProtocolType_TCP_Client;
				pCommunicationStruct->pChannels[uNoChannels].pChannelInfo->localPort = pNetworkInfo->uLocalPort;
				pCommunicationStruct->pChannels[uNoChannels].pChannelInfo->pLocalIp = pal_StringCreate(pNetworkInfo->pLocalIp,
					pal_StringLength(pNetworkInfo->pLocalIp));
				pCommunicationStruct->pChannels[uNoChannels].pChannelInfo->remotePort = remotePort;
				pCommunicationStruct->pChannels[uNoChannels].pChannelInfo->pRemoteIp = pal_StringCreate(pRemoteIP, pal_StringLength(pRemoteIP));

				pCommunicationStruct->pChannels[uNoChannels].pChannelInfo->pTcpBuffer = NULL;
				pCommunicationStruct->pChannels[uNoChannels].pChannelInfo->tcpBufferSize = 0;
				pCommunicationStruct->pChannels[uNoChannels].pChannelInfo->tcpConsumedSize = 0;

				pCommunicationStruct->pChannels[uNoChannels].hChannelHandle = acceptHandle;
				pCommunicationStruct->pChannels[uNoChannels].channelIndex = pCommunicationStruct->noOfChannels;

				SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u - noOfChannels : %d, channelIndex : %d",
					__FUNCTION__, __LINE__, pCommunicationStruct->noOfChannels, pCommunicationStruct->pChannels[uNoChannels].channelIndex);
				*pAccept = Enum_TRUE;
			}
		}
	}

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u",
		__FUNCTION__, __LINE__);
}

#ifdef ENABLE_QCMAPI
void _EcrioSigMgrQcmErrorCB
(
	PALINSTANCE pal,
	void *pCallbackFnData,
	u_int32 error
)
{
	EcrioSigMgrStruct *pSigMgr = NULL;

	error = error;

	if (pCallbackFnData == NULL)
	{
		return;
	}

	pSigMgr = (EcrioSigMgrStruct *)pCallbackFnData;

	if (pSigMgr->bIsDeInitializing == Enum_TRUE)
	{
		return;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tpCallbackFnData=%p, error=%u",
		__FUNCTION__, __LINE__, pCallbackFnData, error);

	if (error == KPALEntityClosed || error == KPALDataSendErrorFatal || error == KPALDataReceiveErrorFatal)
	{
		error = ECRIO_SIG_MGR_SIG_SOCKET_ERROR;

		_EcrioSigMgrSendInfo(pSigMgr, EcrioSigMgrError,
			(s_int32)ECRIO_SIG_MGR_SIG_SOCKET_ERROR, NULL);
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\terror=%u",
		__FUNCTION__, __LINE__, error);
}

void _EcrioSigMgrQcmReceiveCB
(
	void *pCallbackFnData,
	QcmReceiveStruct *pReceive
)
{
	EcrioSigMgrStruct *pSigMgr = NULL;
	u_int32	i = 0, uChannelIndex = 0;
	BoolEnum bChannelFound = Enum_FALSE;

	if (pCallbackFnData == NULL || pReceive == NULL)
	{
		return;
	}

	pSigMgr = (EcrioSigMgrStruct *)pCallbackFnData;

	if (pSigMgr->bIsDeInitializing == Enum_TRUE)
	{
		return;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u",
		__FUNCTION__, __LINE__);

//	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\tsocket=%p, pCallbackFnData=%p, receiveLength=%u pFromIP=%s",
//		__FUNCTION__, __LINE__, pReceive->handle, pCallbackFnData, pReceive->receiveLength, pReceive->pFromIP ? pReceive->pFromIP : (u_char *)"null");

	if (pReceive->pReceiveData == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tNo data received.",
			__FUNCTION__, __LINE__);

		goto Error_Level_01;
	}

	if (pSigMgr->pSignalingInfo == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tNo signalling information, error=%u",
			__FUNCTION__, __LINE__, pReceive->result);

		goto Error_Level_01;
	}

	if (pSigMgr->pSigMgrTransportStruct == NULL || pSigMgr->pSigMgrTransportStruct->pCommunicationStruct == NULL)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tpCommunicationStruct is NULL",
			__FUNCTION__, __LINE__);

		goto Error_Level_01;
	}

	SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tnoOfChannels : %d",
		__FUNCTION__, __LINE__, pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->noOfChannels);

	for (i = 0; i < pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->noOfChannels; i++)
	{
		if (pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[i].hChannelHandle == pReceive->handle)
		{
			uChannelIndex = pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[i].channelIndex;
			bChannelFound = Enum_TRUE;
			break;
		}
	}

	if (bChannelFound == Enum_TRUE)
	{
#ifndef ENABLE_QCMAPI
		if (pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[i].pChannelInfo->eSocketType == ProtocolType_TCP_Client)
		{
			// Handle the received TCP data buffer.
			EcrioTxnMgrSignalingCommunicationChannelInfoStruct *pChannelInfo = pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[i].pChannelInfo;
			u_char *pLoc;
			u_char *pCrlf;

			// First, pre-allocate a large buffer for TCP storage (to simplify the algorithm and reduce memory operations).
			if (pChannelInfo->pTcpBuffer == NULL)
			{
				pChannelInfo->tcpBufferSize = 16384;
				// Is 16K large enough? If not, how large should it be?
				// Be sure to free in the channel structure release, if not NULL.
				// We may want another way to define this value.

				// @note We allocate here only when the TCP channel actually gets used.
				// This needs some review. We should pre-allocate earlier...
				pal_MemoryAllocate(pChannelInfo->tcpBufferSize, (void **)&pChannelInfo->pTcpBuffer);
				if (pChannelInfo->pTcpBuffer == NULL)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tCan't allocate TCP buffer!",
						__FUNCTION__, __LINE__);

					goto Error_Level_01;
				}

				pChannelInfo->tcpConsumedSize = 0;

				// Unfortunately, we do string operations on the buffer...
				pChannelInfo->pTcpBuffer[0] = 0;
			}

			if ((pReceive->receiveLength + pChannelInfo->tcpConsumedSize) >= pChannelInfo->tcpBufferSize)
			{
				// There is not enough room to store the new data! (The '=' is so we have room for a null terminator.)

				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tToo much TCP data for buffer!",
					__FUNCTION__, __LINE__);

				// Drop all the current data in hopes that we can recover with subsequent packets.
				pChannelInfo->tcpConsumedSize = 0;
				pChannelInfo->pTcpBuffer[0] = 0;

				goto Error_Level_01;
			}

			// Copy the new data to the TCP buffer. Wish we didn't have to add another copy to the mix.
			pal_MemoryCopy(&pChannelInfo->pTcpBuffer[pChannelInfo->tcpConsumedSize], pReceive->receiveLength, pReceive->pReceiveData, pReceive->receiveLength);
			pChannelInfo->tcpConsumedSize += pReceive->receiveLength;

			// Unfortunately, we do string operations on the buffer...
			pChannelInfo->pTcpBuffer[pChannelInfo->tcpConsumedSize] = 0;

			// Loop for all possible SIP messages we might find in the buffer.
			// We may want a limit, but should be safe...
			for (;;)
			{
				// Basic design is this:
				// - Look for Content-Length (as per RFC, it must be present for TCP).
				// - Look for the colon after Content-Length.
				// - Look for line terminator after Content-Length.
				// - Get Content-Length value.
				// - Based on the value, find the ending/terminator.
				// - Wait for the rest of data and then invoke callback.

				// We aren't handling upper/lower case, is it an issue?
				pLoc = pal_StringFindSubString(pChannelInfo->pTcpBuffer, (const u_char *)"Content-Length");
				if (pLoc != NULL)
				{
					// Is there enough data to get a Content-Length value?
					// Find the colon from "Content-Length: 0". There may be whitespace before and after the colon...
					pLoc = pal_StringFindSubString(pLoc, (const u_char *)":");
					if (pLoc != NULL)
					{
						// Find the line terminator after "Content-Length: 0".
						pCrlf = pal_StringFindSubString(pLoc, (const u_char *)"\r\n");
						if (pCrlf != NULL)
						{
							u_int32 uContentLength;

							uContentLength = pal_StringConvertToUNum(pLoc + 1, NULL, 10);

							// Find the next CRLFCRLF. If content length is 0, it terminates the frame. Otherwise, it is a separator.
							pCrlf = pal_StringFindSubString(pLoc, (const u_char *)"\r\n\r\n");
							if (pCrlf != NULL)
							{
								u_int32 cbLength;
								u_char *pTemp;

								if (uContentLength == 0)
								{
									// Length is the found pointer minus head of buffer, plus 4 for the CRLFCRLF.
									cbLength = (pCrlf - pChannelInfo->pTcpBuffer) + 4;
								}
								else
								{
									// Do we have all the content?
									if (uContentLength > (pChannelInfo->tcpConsumedSize - ((pCrlf - pChannelInfo->pTcpBuffer) + 4)))
									{
										// Not enough yet!
										break;
									}

									// Length is the found pointer minus head of buffer, plus 4 for the CRLFCRLF plus content length.
									cbLength = ((pCrlf - pChannelInfo->pTcpBuffer) + 4) + uContentLength;
								}

								// In this implementation, we will allocate a temporary buffer to pass back
								// the frame.
								// @todo Evaluate if the caller can handle an unterminated buffer. If so, we maybe don't need this allocation.
								// @note We may want to pre-allocate the buffer (or a buffer). But what size? Need some measurement and design.
								pal_MemoryAllocate(cbLength + 1, (void **)&pTemp);
								if (pTemp != NULL)
								{
									pal_MemoryCopy(pTemp, cbLength, pChannelInfo->pTcpBuffer, cbLength);
									pTemp[cbLength] = 0;	// Add the guard. Again, do we really need this???

									SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tIncoming TCP message received (%u bytes):",
										__FUNCTION__, __LINE__, cbLength);
									SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pTemp, cbLength);

									_EcrioSigMgrSetReceivedOnChannelIndexAndRemoteSendingPort(pSigMgr, uChannelIndex, 0);
									_EcrioSigMgrIMSHandleMessage(pSigMgr, pTemp, cbLength);

									pal_MemoryFree((void **)&pTemp);
								}
								else
								{
									// Log this situation, but otherwise act as though it was consumed.
									SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tCan't allocate callback buffer for TCP data!",
										__FUNCTION__, __LINE__);
								}

								// We should have had a check there somewhere, to ensure the length doesn't exceed the amount of data we have!
								pChannelInfo->tcpConsumedSize = pChannelInfo->tcpConsumedSize - cbLength;
								if (pChannelInfo->tcpConsumedSize > 0)
								{
									// We need to shift any remaining data to the beginning of the buffer.
									// We could implement a circular buffer!!!

									// Ensure this is a memmove operation...
									pal_MemoryCopy(pChannelInfo->pTcpBuffer, pChannelInfo->tcpConsumedSize, &pChannelInfo->pTcpBuffer[cbLength], pChannelInfo->tcpConsumedSize);
								}

								pChannelInfo->pTcpBuffer[pChannelInfo->tcpConsumedSize] = 0;

								if (pChannelInfo->tcpConsumedSize == 0)
								{
									// No more data.
									break;
								}
								else
								{
									// See if we have more data to consume.
									continue;
								}
							}

							// else We didn't find crlfcrlf so wait for the next packet.
						}

						// else We didn't find crlf so wait for the next packet.
					}

					// else We didn't find colon so wait for the next packet.
				}

				// else We didn't find Content-Length so wait for the next packet.

				break;
			}
		}
		else
#endif
		{
#ifdef ENABLE_QCMAPI
			u_char *pProtocol = NULL;
			EcrioTxnMgrSignalingCommunicationChannelInfoStruct *pChannelInfo = pSigMgr->pSigMgrTransportStruct->pCommunicationStruct->pChannels[i].pChannelInfo;
			pProtocol = pal_StringFindSubString(pReceive->pReceiveData, (const u_char *)"TCP");
			if (pProtocol != NULL)
			{
				SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tIncoming TCP message received (%u bytes):",
					__FUNCTION__, __LINE__, pReceive->receiveLength);
			}
			else
			{
				SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tIncoming UDP message received (%u bytes):",
					__FUNCTION__, __LINE__, pReceive->receiveLength);
			}
			SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pReceive->pReceiveData, pReceive->receiveLength);
			if ((pProtocol != NULL) && (pChannelInfo->remoteClientPort != 0))
			{
				_EcrioSigMgrSetReceivedOnChannelIndexAndRemoteSendingPort(pSigMgr, uChannelIndex, pChannelInfo->remoteClientPort);
			}
			else
			{
				_EcrioSigMgrSetReceivedOnChannelIndexAndRemoteSendingPort(pSigMgr, uChannelIndex, pChannelInfo->remotePort);
			}
			_EcrioSigMgrIMSHandleMessage(pSigMgr, pReceive->pReceiveData, pReceive->receiveLength);
#else
			// Handle the received UDP data buffer.
			SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tIncoming UDP message received (%u bytes):",
				__FUNCTION__, __LINE__, pReceive->receiveLength);
			SIGMGRLOGDUMP(pSigMgr->pLogHandle, KLogTypeSIP, pReceive->pReceiveData, pReceive->receiveLength);

			_EcrioSigMgrSetReceivedOnChannelIndexAndRemoteSendingPort(pSigMgr, uChannelIndex, 0);
			_EcrioSigMgrIMSHandleMessage(pSigMgr, pReceive->pReceiveData, pReceive->receiveLength);
#endif
		}
	}

Error_Level_01:
	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, pReceive->result);
}
#endif

/*****************************************************************************
                    API Definition Section - End
*****************************************************************************/
u_int32 _EcrioSigMgrTerminateChannel
(
	EcrioSigMgrStruct *pSigMgr,
	SOCKETHANDLE socketHandle
)
{
	u_int32 error = ECRIO_SIGMGR_NO_ERROR;

	if (socketHandle == NULL)
	{
		error = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto EndTag;
	}

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u\t- Function entry with parameters,pCMData = %x,channelIndex = %d", __FUNCTION__, __LINE__, pSigMgr, socketHandle);

#ifndef ENABLE_QCMAPI
	error = pal_SocketClose(&socketHandle);
	if (error != KPALErrorNone)
	{
		SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t- pal_SocketClose() failed ... Error = %d", __FUNCTION__, __LINE__, error);
		error = ECRIO_SIG_MGR_SIG_SOCKET_ERROR;
		goto EndTag;
	}
#endif

EndTag:
	if (pSigMgr)
	{
		SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t- Exit Error Code = %d", __FUNCTION__, __LINE__, error);
	}

	return error;
}

SOCKETHANDLE _EcrioSigMgrCreateChannel
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioTxnMgrSignalingCommunicationChannelInfoStruct *pChannelInfo,
	u_int32	*pError
)
{
	u_int32	uError = ECRIO_SIGMGR_NO_ERROR;
	SOCKETHANDLE pSock = NULL;

#ifndef ENABLE_QCMAPI
	SocketCreateConfigStruct socketCreateConfig = { .protocolVersion = ProtocolVersion_IPv4, .protocolType = ProtocolType_UDP, .socketType = SocketType_Default, .socketMediaType = SocketMediaType_Default };
	SocketCallbackFnStruct fnStruct = { 0 };
	BoolEnum bCallGetLocalHost = Enum_FALSE;
#else
	QcmCallbackFnStruct fnStruct = { 0 };
#endif

	SocketTLSSettingsStruct		sspl = { 0 };

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	if (pChannelInfo == NULL || pChannelInfo->pLocalIp == NULL || pChannelInfo->pRemoteIp == NULL)
	{
		uError = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto EndTag;
	}

	SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeFuncParams, "%s:%u\t- local-ip = %s local-port=%u remote-ip=%s remote-port=%u remote-tls-port=%u socket-type=%u", __FUNCTION__, __LINE__,
		pChannelInfo->pLocalIp ? pChannelInfo->pLocalIp : (u_char *)"null",
		pChannelInfo->localPort,
		pChannelInfo->pRemoteIp ? pChannelInfo->pRemoteIp : (u_char *)"null",
		pChannelInfo->remotePort,
		pChannelInfo->remoteTLSPort,
		pChannelInfo->eSocketType);


#ifdef ENABLE_QCMAPI
	fnStruct.pReceiveCallbackFn = _EcrioSigMgrQcmReceiveCB;
	fnStruct.pErrorCallbackFn = _EcrioSigMgrQcmErrorCB;
	fnStruct.pCallbackFnData = pSigMgr;

	uError = pal_QcmSetCallback(pSigMgr->pal, &fnStruct, &pSock);
	if (uError != KPALErrorNone)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t-pal_QcmSetCallback() failed ... Error = %d", __FUNCTION__, __LINE__, uError);
		uError = ECRIO_SIG_MGR_SIG_SOCKET_ERROR;
		goto EndTag;
	}
#else
	socketCreateConfig.protocolVersion = (pSigMgr->pNetworkInfo->bIsIPv6) ? ProtocolVersion_IPv6 : ProtocolVersion_IPv4;
	socketCreateConfig.protocolType = pChannelInfo->eSocketType;
	socketCreateConfig.socketType = SocketType_Default;
	socketCreateConfig.socketMediaType = SocketMediaType_Default;
	socketCreateConfig.bufferSize = 4096;
	socketCreateConfig.bEnableGlobalMutex = Enum_TRUE;

	if (pChannelInfo->eSocketType != ProtocolType_TCP_Server)
	{
		fnStruct.pAcceptCallbackFn = NULL;
	}
	else
	{
		fnStruct.pAcceptCallbackFn = _EcrioSigMgrSocketAcceptCB;
	}

	if (pChannelInfo->eSocketType == ProtocolType_TLS_Client)
	{
		sspl.bVerifyPeer = pSigMgr->pSignalingInfo->bTLSPeerVerification;
		if (sspl.bVerifyPeer == Enum_TRUE)
		{
			sspl.pCert = (const u_char*)pSigMgr->pSignalingInfo->pTLSCertificate;
		}
		else
		{
			sspl.pCert = NULL;
		}
		sspl.uNumOfFingerPrint = 0;
		sspl.socketTLSVersion = SocketTLSVersion_TLSv1_2;
		sspl.uNumOfFingerPrint = 0;
		sspl.ppFingerPrint = NULL;
		socketCreateConfig.tlsSetting = &sspl;
	}
	fnStruct.pErrorCallbackFn = _EcrioSigMgrSocketErrorCB;
	fnStruct.pReceiveCallbackFn = _EcrioSigMgrSocketReceieveCB;
	fnStruct.pCallbackFnData = pSigMgr;

	uError = pal_SocketCreate(pSigMgr->pal, &socketCreateConfig, &fnStruct, &pSock);
	if (uError != KPALErrorNone)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t- pal_SocketCreate() failed ... Error = %d", __FUNCTION__, __LINE__, uError);
		uError = ECRIO_SIG_MGR_SIG_SOCKET_ERROR;
		goto EndTag;
	}

	if (pChannelInfo->localPort == 0)
	{
		bCallGetLocalHost = Enum_TRUE;
	}

	uError = pal_SocketSetLocalHost(pSock,
		pChannelInfo->pLocalIp,
		(u_int16)pChannelInfo->localPort);
	if (uError != KPALErrorNone)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t- pal_SocketSetLocalHost() failed ... Error = %d", __FUNCTION__, __LINE__, uError);
		uError = ECRIO_SIG_MGR_SIG_SOCKET_ERROR;
		goto EndTag;
	}

	if (pChannelInfo->eSocketType != ProtocolType_TCP_Server)
	{
		if (pChannelInfo->eSocketType == ProtocolType_TLS_Client)
		{
			uError = pal_SocketSetRemoteHost(pSock,
			pChannelInfo->pRemoteIp,
			(u_int16)pChannelInfo->remoteTLSPort);
		}
		else
		{
			uError = pal_SocketSetRemoteHost(pSock,
				pChannelInfo->pRemoteIp,
				(u_int16)pChannelInfo->remotePort);
		}

		if (uError != KPALErrorNone)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t- pal_SocketSetRemoteHost() failed ... Error = %d", __FUNCTION__, __LINE__, uError);
			uError = ECRIO_SIG_MGR_SIG_SOCKET_ERROR;
			goto EndTag;
		}
	}

	/* Use socket open extension API only if proxy routing enabled */
	if(pChannelInfo->bIsProxyRouteEnabled == Enum_TRUE)
		uError = pal_SocketOpenEx(pSock);
	else
		uError = pal_SocketOpen(pSock);
	if (uError != KPALErrorNone)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t- pal_SocketOpen() failed ... Error = %d", __FUNCTION__, __LINE__, uError);
		uError = ECRIO_SIG_MGR_SIG_SOCKET_ERROR;
		goto EndTag;
	}

	if (bCallGetLocalHost == Enum_TRUE)
	{
		u_int16 port = 0;
		u_char localIp[52];	// maximum length of IPV6 address plus some extra bytes

// quiet		SIGMGRLOGV(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t pal_SocketGetLocalHost() pChannelInfo->pRemoteIp(%s) and pChannelInfo->pLocalIp(%s)", __FUNCTION__, __LINE__, pChannelInfo->pRemoteIp, pChannelInfo->pLocalIp);
		uError = pal_SocketGetLocalHost(pSock, (u_char *)&localIp, &port);
		if (uError != KPALErrorNone)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t- pal_SocketGetLocalHost() failed ... Error = %d", __FUNCTION__, __LINE__, uError);
			uError = ECRIO_SIG_MGR_SIG_SOCKET_ERROR;
			goto EndTag;
		}

		pChannelInfo->localPort = port;
		SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tpChannelInfo-> pRemoteIp:%s remotePort:%d pLocalIp:%s localPort:%d", __FUNCTION__, __LINE__, pChannelInfo->pRemoteIp, pChannelInfo->remotePort, pChannelInfo->pLocalIp, pChannelInfo->localPort);
	}

	if (pChannelInfo->eSocketType == ProtocolType_TCP_Client &&
			pChannelInfo->bIsProxyRouteEnabled == Enum_TRUE)
	{
		uError = pal_SocketSendData(pSock, (const u_char*)pChannelInfo->pHttpConnectMsg, pal_StringLength((const u_char*)pChannelInfo->pHttpConnectMsg));
		if (uError != KPALErrorNone)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t- pal_SocketSendData() failed ... Error = %d", __FUNCTION__, __LINE__, uError);
			/* Supress the error.*/
			uError = ECRIO_SIGMGR_NO_ERROR;
		} else{
			SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t- pal_SocketSendData() = %s", __FUNCTION__, __LINE__, pChannelInfo->pHttpConnectMsg);
		}
	}
#endif

EndTag:

	SIGMGRLOGI(pSigMgr->pLogHandle, KLogTypeFuncExit, "%s:%u\t- Function exit with parameters socket handle = %x pError = %u", __FUNCTION__, __LINE__, pSock, uError);

	if (uError != ECRIO_SIGMGR_NO_ERROR)
	{
		_EcrioSigMgrTerminateChannel(pSigMgr, pSock);
		pSock = NULL;
	}

	if (pError)
	{
		*pError = uError;
	}

	return pSock;
}

u_int32 _EcrioSigMgrCreateEachCommunicationChannel
(
	EcrioSigMgrStruct *pSigMgr,
	EcrioTxnMgrSignalingCommunicationStruct	*pCommunicationStruct,
	EcrioSigMgrNetworkInfoStruct *pNetworkInfo,
	u_int16 index,
	ProtocolTypeEnum type,
	BoolEnum bProtected
)
{
	u_int32	uError = ECRIO_SIGMGR_NO_ERROR;
	u_char httpMsg[256] = {0};

	pal_MemoryAllocate(sizeof(EcrioTxnMgrSignalingCommunicationChannelInfoStruct), (void **)&pCommunicationStruct->pChannels[index].pChannelInfo);
	if (NULL == pCommunicationStruct->pChannels[index].pChannelInfo)
	{
		uError = ECRIO_SIGMGR_NO_MEMORY;
		goto EndTag;
	}

	if (type == ProtocolType_TCP_Client)
	{
		pCommunicationStruct->pChannels[index].pChannelInfo->pTcpBuffer = NULL;
		pCommunicationStruct->pChannels[index].pChannelInfo->tcpBufferSize = 0;
		pCommunicationStruct->pChannels[index].pChannelInfo->tcpConsumedSize = 0;
	}

	if (type == ProtocolType_TLS_Client)
	{
		pCommunicationStruct->pChannels[index].pChannelInfo->pTlsBuffer = NULL;
		pCommunicationStruct->pChannels[index].pChannelInfo->tlsBufferSize = 0;
		pCommunicationStruct->pChannels[index].pChannelInfo->tlsConsumedSize = 0;
	}

	if (bProtected == Enum_TRUE)
	{
		if (type == ProtocolType_TCP_Server)
		{
			pCommunicationStruct->pChannels[index].pChannelInfo->localPort = pSigMgr->pIPSecDetails->pIPsecNegParams->uPort_us;
		}
		else if (type == ProtocolType_TCP_Client)
		{
			pCommunicationStruct->pChannels[index].pChannelInfo->localPort = pSigMgr->pIPSecDetails->pIPsecNegParams->uPort_uc;
			pCommunicationStruct->pChannels[index].pChannelInfo->remotePort = pSigMgr->pIPSecDetails->pIPsecNegParams->uPort_ps;
		}
		else
		{
			pCommunicationStruct->pChannels[index].pChannelInfo->localPort = pSigMgr->pIPSecDetails->pIPsecNegParams->uPort_us;
			pCommunicationStruct->pChannels[index].pChannelInfo->remotePort = pSigMgr->pIPSecDetails->pIPsecNegParams->uPort_ps;
		}
	}
	else
	{
		pCommunicationStruct->pChannels[index].pChannelInfo->localPort = pNetworkInfo->uLocalPort;
		pCommunicationStruct->pChannels[index].pChannelInfo->remoteTLSPort = pNetworkInfo->uRemoteTLSPort;
		if(pNetworkInfo->bIsProxyRouteEnabled == Enum_TRUE)
			pCommunicationStruct->pChannels[index].pChannelInfo->remotePort = pNetworkInfo->uProxyRoutePort;
		else
			pCommunicationStruct->pChannels[index].pChannelInfo->remotePort = pNetworkInfo->uRemotePort;
		pCommunicationStruct->pChannels[index].pChannelInfo->remoteClientPort = pNetworkInfo->uRemoteClientPort;
	}
	
	pCommunicationStruct->pChannels[index].pChannelInfo->eSocketType = type;
	pCommunicationStruct->pChannels[index].channelIndex = index + 1;
	pCommunicationStruct->pChannels[index].pChannelInfo->pLocalIp = pal_StringCreate(pNetworkInfo->pLocalIp,
																		pal_StringLength(pNetworkInfo->pLocalIp));

	if(pNetworkInfo->bIsProxyRouteEnabled == Enum_TRUE) {
		pCommunicationStruct->pChannels[index].pChannelInfo->pRemoteIp = pal_StringCreate(
				pNetworkInfo->pProxyRouteAddress,
				pal_StringLength(pNetworkInfo->pProxyRouteAddress));

		pCommunicationStruct->pChannels[index].pChannelInfo->bIsProxyRouteEnabled = pNetworkInfo->bIsProxyRouteEnabled;

		if (0 >= pal_StringSNPrintf((char *)&httpMsg[0], 256, (char *)"CONNECT [%s]:%d HTTP/1.1\r\n\n", pNetworkInfo->pRemoteIP, pNetworkInfo->uRemotePort))
		{
			pal_MemoryFree((void**)&pCommunicationStruct->pChannels[index].pChannelInfo->pRemoteIp);
			pCommunicationStruct->pChannels[index].pChannelInfo->bIsProxyRouteEnabled = Enum_FALSE;
			pCommunicationStruct->pChannels[index].pChannelInfo->pRemoteIp = pal_StringCreate(pNetworkInfo->pRemoteIP,
																							  pal_StringLength(pNetworkInfo->pRemoteIP));
			pCommunicationStruct->pChannels[index].pChannelInfo->pHttpConnectMsg = NULL;
		}
		pCommunicationStruct->pChannels[index].pChannelInfo->pHttpConnectMsg = pal_StringCreate((const u_char*)&httpMsg[0],
                                                                                                pal_StringLength((const u_char*)&httpMsg[0]));
        SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tProxy Route Enabled, Connect Msg = %s", __FUNCTION__, __LINE__, pCommunicationStruct->pChannels[index].pChannelInfo->pHttpConnectMsg);
	}
	else {
		pCommunicationStruct->pChannels[index].pChannelInfo->pRemoteIp = pal_StringCreate(
				pNetworkInfo->pRemoteIP,
				pal_StringLength(pNetworkInfo->pRemoteIP));
		pCommunicationStruct->pChannels[index].pChannelInfo->bIsProxyRouteEnabled = Enum_FALSE;
		pCommunicationStruct->pChannels[index].pChannelInfo->pHttpConnectMsg = NULL;
        SIGMGRLOGD(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\tProxy Route Disabled", __FUNCTION__, __LINE__);
	}

	pCommunicationStruct->pChannels[index].pChannelInfo->bIPsecProtected = bProtected;
	pCommunicationStruct->pChannels[index].hChannelHandle = _EcrioSigMgrCreateChannel(pSigMgr, pCommunicationStruct->pChannels[index].pChannelInfo, &uError);
	if (NULL == pCommunicationStruct->pChannels[index].hChannelHandle || uError == ECRIO_SIG_MGR_SIG_SOCKET_ERROR)
	{
		SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t- _EcrioSigMgrCreateEachCommunicationChannel() failed ... Error = %d", __FUNCTION__, __LINE__, uError);
		goto EndTag;
	}

EndTag:
	return uError;
}

u_int32	_EcrioSigMgrCreateCommunicationChannels
(
	EcrioSigMgrStruct *pSigMgr
)
{
	u_int32	uError = ECRIO_SIGMGR_NO_ERROR;

	u_int32	uTLSConnectionError = ECRIO_SIGMGR_NO_ERROR;

	EcrioSigMgrNetworkInfoStruct *pNetworkInfo = NULL;
	EcrioTxnMgrSignalingCommunicationStruct	*pCommunicationStruct = NULL;

	pNetworkInfo = pSigMgr->pNetworkInfo;
	if (pSigMgr->pSigMgrTransportStruct == NULL)
	{
		pal_MemoryAllocate(sizeof(EcrioTxnMgrTransportStruct), (void **)&pSigMgr->pSigMgrTransportStruct);
		if (pSigMgr->pSigMgrTransportStruct == NULL)
		{
			uError = ECRIO_SIGMGR_NO_MEMORY;
			goto End_Tag;
		}
	}

	pal_MemoryAllocate(sizeof(EcrioTxnMgrSignalingCommunicationStruct), (void **)&pCommunicationStruct);
	if (pCommunicationStruct == NULL)
	{
		uError = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Tag;
	}

	pSigMgr->pSigMgrTransportStruct->pCommunicationStruct = pCommunicationStruct;

#ifndef ENABLE_QCMAPI
	/*Both UDP and TCP enabled. */
	if (pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE)
	{
		if (pSigMgr->pSignalingInfo->bEnableTls == Enum_TRUE)
			pal_MemoryAllocate(4 * sizeof(EcrioTxnMgrSignalingCommunicationChannelStruct), (void **)&pCommunicationStruct->pChannels);
		else
			pal_MemoryAllocate(3 * sizeof(EcrioTxnMgrSignalingCommunicationChannelStruct), (void **)&pCommunicationStruct->pChannels);

		if (pCommunicationStruct->pChannels != NULL)
		{
			if (pSigMgr->pSignalingInfo->bEnableTls == Enum_TRUE)
				pCommunicationStruct->noOfChannels = 4;
			else
				pCommunicationStruct->noOfChannels = 3;

			// create UDP
			uError = _EcrioSigMgrCreateEachCommunicationChannel(pSigMgr, pCommunicationStruct, pNetworkInfo, 0, ProtocolType_UDP, Enum_FALSE);
			if (uError != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t-failed to create UDP Channel with error:%d", __FUNCTION__, __LINE__, uError);
				goto Error_Tag;
			}

			// create TCP client
			uError = _EcrioSigMgrCreateEachCommunicationChannel(pSigMgr, pCommunicationStruct, pNetworkInfo, 1, ProtocolType_TCP_Client, Enum_FALSE);
			if (uError != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t-failed to create TCP client Channel with error:%d", __FUNCTION__, __LINE__, uError);
				goto Error_Tag;
			}

			// create TCP server
			uError = _EcrioSigMgrCreateEachCommunicationChannel(pSigMgr, pCommunicationStruct, pNetworkInfo, 2, ProtocolType_TCP_Server, Enum_FALSE);
			if (uError != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t-failed to create TCP Server Channel with error:%d", __FUNCTION__, __LINE__, uError);
				goto Error_Tag;
			}

			// create TLS client
			if (pSigMgr->pSignalingInfo->bEnableTls == Enum_TRUE)
			{
				uError = _EcrioSigMgrCreateEachCommunicationChannel(pSigMgr, pCommunicationStruct, pNetworkInfo, 3, ProtocolType_TLS_Client, Enum_FALSE);
				if (uError != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t-failed to create TLS client Channel with error:%d", __FUNCTION__, __LINE__, uError);
					uTLSConnectionError = ECRIO_SIGMGR_TLS_CONNECTION_FAILURE;
					pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex = 0;
					//goto Error_Tag;
				}
				else
				{
					pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex = 4;
				}
			}

			pSigMgr->pSigMgrTransportStruct->sendingChannelIndex = 1;
			pSigMgr->pSigMgrTransportStruct->sendingTCPChannelIndex = 2;
			if (pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE)
			{
				
			}
			else if (pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_FALSE)
			{
				
			}
			else if (pSigMgr->pSignalingInfo->bEnableUdp == Enum_FALSE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE)
			{
				
			}
		}
	} /* Only UDP enabled. */
	else if(pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_FALSE)
	{
		pal_MemoryAllocate(2 * sizeof(EcrioTxnMgrSignalingCommunicationChannelStruct), (void **)&pCommunicationStruct->pChannels);
		if (pCommunicationStruct->pChannels != NULL)
		{
			if (pSigMgr->pSignalingInfo->bEnableTls == Enum_TRUE)
				pCommunicationStruct->noOfChannels = 2;
			else
				pCommunicationStruct->noOfChannels = 1;

			// create UDP
			uError = _EcrioSigMgrCreateEachCommunicationChannel(pSigMgr, pCommunicationStruct, pNetworkInfo, 0, ProtocolType_UDP, Enum_FALSE);
			if (uError != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t-failed to create UDP Channel with error:%d", __FUNCTION__, __LINE__, uError);
				goto Error_Tag;
			}

			// create TLS client
			if (pSigMgr->pSignalingInfo->bEnableTls == Enum_TRUE)
			{
				uError = _EcrioSigMgrCreateEachCommunicationChannel(pSigMgr, pCommunicationStruct, pNetworkInfo, 1, ProtocolType_TLS_Client, Enum_FALSE);
				if (uError != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t-failed to create TLS client Channel with error:%d", __FUNCTION__, __LINE__, uError);
					uTLSConnectionError = ECRIO_SIGMGR_TLS_CONNECTION_FAILURE;
					pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex = 0;
					//goto Error_Tag;
				}
				else
				{
					pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex = 2;
				}
			}

			pSigMgr->pSigMgrTransportStruct->sendingChannelIndex = 1;
		}
	} /* Only TCP enabled. */
	else if (pSigMgr->pSignalingInfo->bEnableUdp == Enum_FALSE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE)
	{
		if(pNetworkInfo->bIsProxyRouteEnabled != Enum_TRUE) {
			pal_MemoryAllocate(3 * sizeof(EcrioTxnMgrSignalingCommunicationChannelStruct),
							   (void **) &pCommunicationStruct->pChannels);
			if (pCommunicationStruct->pChannels != NULL) {

				if (pSigMgr->pSignalingInfo->bEnableTls == Enum_TRUE)
					pCommunicationStruct->noOfChannels = 3;
				else
					pCommunicationStruct->noOfChannels = 2;

				// create TCP client
				uError = _EcrioSigMgrCreateEachCommunicationChannel(pSigMgr, pCommunicationStruct,
																	pNetworkInfo, 0,
																	ProtocolType_TCP_Client,
																	Enum_FALSE);
				if (uError != ECRIO_SIGMGR_NO_ERROR) {
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
							   "%s:%u\t-failed to create TCP client Channel with error:%d",
							   __FUNCTION__, __LINE__, uError);
					goto Error_Tag;
				}

				// create TCP server
				uError = _EcrioSigMgrCreateEachCommunicationChannel(pSigMgr, pCommunicationStruct,
																	pNetworkInfo, 1,
																	ProtocolType_TCP_Server,
																	Enum_FALSE);
				if (uError != ECRIO_SIGMGR_NO_ERROR) {
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
							   "%s:%u\t-failed to create TCP Server Channel with error:%d",
							   __FUNCTION__, __LINE__, uError);
					goto Error_Tag;
				}

				// create TLS client
				if (pSigMgr->pSignalingInfo->bEnableTls == Enum_TRUE) {
					uError = _EcrioSigMgrCreateEachCommunicationChannel(pSigMgr,
																		pCommunicationStruct,
																		pNetworkInfo, 2,
																		ProtocolType_TLS_Client,
																		Enum_FALSE);
					if (uError != ECRIO_SIGMGR_NO_ERROR) {
						SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral,
								   "%s:%u\t-failed to create TLS client Channel with error:%d",
								   __FUNCTION__, __LINE__, uError);
						uTLSConnectionError = ECRIO_SIGMGR_TLS_CONNECTION_FAILURE;
						//goto Error_Tag;
						pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex = 0;
					} else {
						pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex = 3; // TCP Server being 2
					}
				}

				pSigMgr->pSigMgrTransportStruct->sendingTCPChannelIndex = 1;
			}
		} else{ /* Messages gets routed to proxy. So, only TCP channel to proxy is created. On the
 				* channel created, HTTP Connect message of actual SIP server is passed to route messages */
			pal_MemoryAllocate(sizeof(EcrioTxnMgrSignalingCommunicationChannelStruct), (void **)&pCommunicationStruct->pChannels);
			if (pCommunicationStruct->pChannels != NULL)
			{
				pCommunicationStruct->noOfChannels = 1;

				// create TCP client
				uError = _EcrioSigMgrCreateEachCommunicationChannel(pSigMgr, pCommunicationStruct, pNetworkInfo, 0, ProtocolType_TCP_Client, Enum_FALSE);
				if (uError != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t-failed to create TCP client Channel with error:%d", __FUNCTION__, __LINE__, uError);
					goto Error_Tag;
				}

				pSigMgr->pSigMgrTransportStruct->sendingTCPChannelIndex = 1;
			}
		}
	}
	else /* Only TLS enabled. */
	{
		if (pSigMgr->pSignalingInfo->bEnableTls == Enum_TRUE)
		{
			pal_MemoryAllocate(1 * sizeof(EcrioTxnMgrSignalingCommunicationChannelStruct), (void **)&pCommunicationStruct->pChannels);
			if (pCommunicationStruct->pChannels != NULL)
			{
				pCommunicationStruct->noOfChannels = 1;


				// create TLS client
				uError = _EcrioSigMgrCreateEachCommunicationChannel(pSigMgr, pCommunicationStruct, pNetworkInfo, 0, ProtocolType_TLS_Client, Enum_FALSE);
				if (uError != ECRIO_SIGMGR_NO_ERROR)
				{
					SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t-failed to create TLS client Channel with error:%d", __FUNCTION__, __LINE__, uError);
					uTLSConnectionError = ECRIO_SIGMGR_TLS_CONNECTION_FAILURE;
					pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex = 0;
					//goto Error_Tag;
				}
				else
				{
					pSigMgr->pSigMgrTransportStruct->sendingTLSChannelIndex = 1;
				}

			}
		}
	}
#else
	pal_MemoryAllocate(1 * sizeof(EcrioTxnMgrSignalingCommunicationChannelStruct), (void **)&pCommunicationStruct->pChannels);
	if (pCommunicationStruct->pChannels != NULL)
	{
		pCommunicationStruct->noOfChannels = 1;
		uError = _EcrioSigMgrCreateEachCommunicationChannel(pSigMgr, pCommunicationStruct, pNetworkInfo, 0, ProtocolType_TCP_Client, Enum_FALSE);
		if (uError != ECRIO_SIGMGR_NO_ERROR)
		{
			SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t-failed to create UDP Channel with error:%d", __FUNCTION__, __LINE__, uError);
			goto Error_Tag;
		}
		pSigMgr->pSigMgrTransportStruct->sendingChannelIndex = 1;
		pSigMgr->pSigMgrTransportStruct->sendingTCPChannelIndex = 1;
	}
#endif

	if (uTLSConnectionError != ECRIO_SIGMGR_NO_ERROR)
	{
		uError = uTLSConnectionError;
	}

	if (pCommunicationStruct->pChannels == NULL)
	{
		uError = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Tag;
	}

	goto End_Tag;
Error_Tag:

	_EcrioSigMgrTerminateCommunicationChannels(pSigMgr);

	if (pSigMgr->pSigMgrTransportStruct != NULL)
	{
		pal_MemoryFree((void**)&pSigMgr->pSigMgrTransportStruct);
		pSigMgr->pSigMgrTransportStruct = NULL;
	}

End_Tag:
	return uError;
}

u_int32	_EcrioSigMgrTerminateCommunicationChannels
(
	EcrioSigMgrStruct *pSigMgr
)
{
	u_int32	uError = ECRIO_SIGMGR_NO_ERROR;
	EcrioTxnMgrSignalingCommunicationStruct	*pCommunicationStruct = NULL;
	u_int32 index = 0;

	if (NULL == pSigMgr->pSigMgrTransportStruct || NULL == pSigMgr->pSigMgrTransportStruct->pCommunicationStruct)
	{
		uError = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto EndTag;
	}

	pCommunicationStruct = pSigMgr->pSigMgrTransportStruct->pCommunicationStruct;

	for (index = 0; index < pCommunicationStruct->noOfChannels; index++)
	{
		if (pCommunicationStruct->pChannels[index].hChannelHandle)
		{
			uError = _EcrioSigMgrTerminateChannel(pSigMgr, pCommunicationStruct->pChannels[index].hChannelHandle);
			if (uError != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t- _EcrioSigMgrTerminateChannel() failed ... Error = %d", __FUNCTION__, __LINE__, uError);
			}
		}
	}

	EcrioTXNStructRelease(pSigMgr->pTransactionMgrHndl, (void **)&pCommunicationStruct, EcrioTXNStructType_EcrioTxnMgrSignalingCommunicationStruct, Enum_TRUE);
	pSigMgr->pSigMgrTransportStruct->pCommunicationStruct = NULL;

EndTag:

	return uError;
}

u_int32	_EcrioSigMgrCreateIPSecCommunicationChannels
(
	EcrioSigMgrStruct *pSigMgr
)
{
	u_int32	uError = ECRIO_SIGMGR_NO_ERROR;
	EcrioSigMgrNetworkInfoStruct *pNetworkInfo = NULL;
	EcrioTxnMgrSignalingCommunicationStruct	*pCommunicationStruct = NULL;

	pNetworkInfo = pSigMgr->pNetworkInfo;

	pCommunicationStruct = pSigMgr->pSigMgrTransportStruct->pCommunicationStruct;

	/* Check arithmetic overflow */
	if (pal_UtilityArithmeticOverflowDetected(pCommunicationStruct->noOfChannels, 3) == Enum_TRUE)
	{
		uError = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Tag;
	}

	/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
	if (pal_UtilityDataOverflowDetected((pCommunicationStruct->noOfChannels + 3), sizeof(EcrioTxnMgrSignalingCommunicationChannelStruct)) == Enum_TRUE)
	{
		uError = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Tag;
	}

	/*Both UDP and TCP enabled. */
	if (pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE)
	{
		pal_MemoryReallocate((pCommunicationStruct->noOfChannels + 3) * sizeof(EcrioTxnMgrSignalingCommunicationChannelStruct), 
			(void **)&pCommunicationStruct->pChannels);
		if (pCommunicationStruct->pChannels != NULL)
		{
			pCommunicationStruct->noOfChannels += 3;

			// create UDP
			uError = _EcrioSigMgrCreateEachCommunicationChannel(pSigMgr, pCommunicationStruct, pNetworkInfo, 3,ProtocolType_UDP, Enum_TRUE);
			if (uError != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t-failed to create UDP Channel with error:%d", __FUNCTION__, __LINE__, uError);
				goto Error_Tag;
			}
			pSigMgr->pSigMgrTransportStruct->sendingIPsecUdpChannelIndex = 4;

			// create TCP client
			uError = _EcrioSigMgrCreateEachCommunicationChannel(pSigMgr, pCommunicationStruct, pNetworkInfo, 4,ProtocolType_TCP_Client, Enum_TRUE);
			if (uError != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t-failed to create TCP client Channel with error:%d", __FUNCTION__, __LINE__, uError);
				goto Error_Tag;
			}
			pSigMgr->pSigMgrTransportStruct->sendingIPsecTCPChannelIndex = 5;

			// create TCP server
			uError = _EcrioSigMgrCreateEachCommunicationChannel(pSigMgr, pCommunicationStruct, pNetworkInfo, 5,ProtocolType_TCP_Server, Enum_TRUE);
			if (uError != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t-failed to create TCP Server Channel with error:%d", __FUNCTION__, __LINE__, uError);
				goto Error_Tag;
			}
			
		}
	} /* Only UDP enabled. */
	else if (pSigMgr->pSignalingInfo->bEnableUdp == Enum_TRUE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_FALSE)
	{
		pal_MemoryReallocate((pCommunicationStruct->noOfChannels + 1) * sizeof(EcrioTxnMgrSignalingCommunicationChannelStruct),
			(void **)&pCommunicationStruct->pChannels);
		if (pCommunicationStruct->pChannels != NULL)
		{
			pCommunicationStruct->noOfChannels += 1;
			// create UDP
			pNetworkInfo->uLocalPort = pSigMgr->pIPSecDetails->pIPsecNegParams->uPort_uc;
			pNetworkInfo->uRemotePort = pSigMgr->pIPSecDetails->pIPsecNegParams->uPort_ps;
			uError = _EcrioSigMgrCreateEachCommunicationChannel(pSigMgr, pCommunicationStruct, pNetworkInfo, 1,ProtocolType_UDP, Enum_TRUE);
			if (uError != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t-failed to create UDP Channel with error:%d", __FUNCTION__, __LINE__, uError);
				goto Error_Tag;
			}
			pSigMgr->pSigMgrTransportStruct->sendingIPsecUdpChannelIndex = 2;
		}
	} /* Only TCP enabled. */
	else if (pSigMgr->pSignalingInfo->bEnableUdp == Enum_FALSE && pSigMgr->pSignalingInfo->bEnableTcp == Enum_TRUE)
	{
		pal_MemoryReallocate((pCommunicationStruct->noOfChannels + 2) * sizeof(EcrioTxnMgrSignalingCommunicationChannelStruct),
			(void **)&pCommunicationStruct->pChannels);
		if (pCommunicationStruct->pChannels != NULL)
		{
			pCommunicationStruct->noOfChannels += 2;

			// create TCP client
			uError = _EcrioSigMgrCreateEachCommunicationChannel(pSigMgr, pCommunicationStruct, pNetworkInfo, 2,ProtocolType_TCP_Client, Enum_TRUE);
			if (uError != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t-failed to create TCP client Channel with error:%d", __FUNCTION__, __LINE__, uError);
				goto Error_Tag;
			}
			pSigMgr->pSigMgrTransportStruct->sendingIPsecTCPChannelIndex = 3;

			// create TCP server
			uError = _EcrioSigMgrCreateEachCommunicationChannel(pSigMgr, pCommunicationStruct, pNetworkInfo, 3,ProtocolType_TCP_Server, Enum_TRUE);
			if (uError != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t-failed to create TCP Server Channel with error:%d", __FUNCTION__, __LINE__, uError);
				goto Error_Tag;
			}
		}
	}

	if (pCommunicationStruct->pChannels == NULL)
	{
		uError = ECRIO_SIGMGR_NO_MEMORY;
		goto Error_Tag;
	}

	goto End_Tag;
Error_Tag:

	_EcrioSigMgrTerminateIPSecCommunicationChannels(pSigMgr);

	if (pSigMgr->pSigMgrTransportStruct != NULL)
	{
		pal_MemoryFree((void**)&pSigMgr->pSigMgrTransportStruct);
		pSigMgr->pSigMgrTransportStruct = NULL;
	}

End_Tag:
	return uError;
}

u_int32	_EcrioSigMgrTerminateIPSecCommunicationChannels
(
	EcrioSigMgrStruct *pSigMgr
)
{
	u_int32	uError = ECRIO_SIGMGR_NO_ERROR;
	EcrioTxnMgrSignalingCommunicationStruct	*pCommunicationStruct = NULL;
	u_int32 index = 0;

	if (NULL == pSigMgr->pSigMgrTransportStruct || NULL == pSigMgr->pSigMgrTransportStruct->pCommunicationStruct)
	{
		uError = ECRIO_SIGMGR_INSUFFICIENT_DATA;
		goto EndTag;
	}

	pCommunicationStruct = pSigMgr->pSigMgrTransportStruct->pCommunicationStruct;

	for (index = 0; index < pCommunicationStruct->noOfChannels; index++)
	{
		if ((pCommunicationStruct->pChannels[index].hChannelHandle) && (pCommunicationStruct->pChannels[index].pChannelInfo->bIPsecProtected == Enum_TRUE))
		{
			uError = _EcrioSigMgrTerminateChannel(pSigMgr, pCommunicationStruct->pChannels[index].hChannelHandle);
			if (uError != ECRIO_SIGMGR_NO_ERROR)
			{
				SIGMGRLOGE(pSigMgr->pLogHandle, KLogTypeGeneral, "%s:%u\t- _EcrioSigMgrTerminateChannel() failed ... Error = %d", __FUNCTION__, __LINE__, uError);
			}
		}
	}
	EcrioTXNStructRelease(pSigMgr->pTransactionMgrHndl, (void **)&pCommunicationStruct, EcrioTXNStructType_EcrioTxnMgrSignalingCommunicationStruct, Enum_TRUE);
	pSigMgr->pSigMgrTransportStruct->pCommunicationStruct = NULL;

EndTag:

	return uError;
}
