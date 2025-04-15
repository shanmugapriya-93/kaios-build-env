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
 * @file iota_test_utility.cpp
 * @brief Utility functions for the iota Main Test App.
 */

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "Iphlpapi.lib")
#else
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "EcrioPAL.h"
#include "lims.h"

#include "iota_test.h"

#ifdef WIN32
// There is a problem since we are linking some release libraries (such as codecs) in
// that the release heap is also loaded. For some reason strdup falls for the release
// library in debug mode. So we will remap to _strdup which seems to fix the problem.
// https://stackoverflow.com/questions/8740500/heap-corruption-with-strdup
// @todo Maybe there is a better way to avoid the extra library from being loaded?
#define strdup    _strdup
#endif

extern iotaTestStateStruct iotaState;

void iota_test_getRandomString
(
	unsigned char *pStr,
	unsigned int uLength
)
{
	unsigned int i = 0;

	for (i = 0; i < uLength; i++)
	{
		*(pStr + i) = iotaState.seed[pal_UtilityRandomNumber() % 62];
	}
}

void iota_test_getRandomStringHex
(
	unsigned char *pStr,
	unsigned int uLength
)
{
	unsigned int i = 0;

	for (i = 0; i < uLength; i++)
	{
		*(pStr + i) = iotaState.seedHex[pal_UtilityRandomNumber() % 16];
	}
}


/* Find the network information from FQDN*/
int iota_test_fqdn_getNetwork
(
	lims_ConfigStruct *pConfig,
	lims_NetworkConnectionStruct *pNetwork
)
{
	u_int32 AddressCount = 0;
	u_int32 tlsAddressCount = 0;
	EcrioUtilityDnsAddrStruct *pAddrList = NULL;
	EcrioUtilityDnsAddrStruct *pTlsAddrList = NULL;
	u_int32 palError = KPALErrorNone;
	u_int32 i = 0;
	int error = 0;

	if (pConfig == NULL || pNetwork == NULL)
	{
		error = 1;
		goto EndTag;
	}
	/* Find network information*/
	palError = pal_UtilityDnsGetInfoFromFqdn(iotaState.palLimsInstance, IOTA_TEST_FQDN_HOST_NAME, iotaState.localAddress, IOTA_TEST_FQDN_DNS_SERVER, &tlsAddressCount, &pTlsAddrList, &AddressCount, &pAddrList);
	if (palError != KPALErrorNone)
	{
		iota_test_printf("pal_UtilityGetIpFromFqdn() error!\n");
		error = 1;
		goto EndTag;
	}
	if (AddressCount == 0 && tlsAddressCount == 0)
	{
		iota_test_printf(" IP Not Found! \n");
		error = 1;
		goto EndTag;
	}
	/* Only UDP or UDP and TCP */
	if (pConfig->bEnableUdp == Enum_TRUE && pConfig->bTLSEnabled == Enum_FALSE) 
	{
		pNetwork->uNoOfRemoteIps = AddressCount;
		pNetwork->ppRemoteIPs = (char **)calloc(pNetwork->uNoOfRemoteIps, sizeof(char *));
		for (i = 0; i<AddressCount; i++)
		{
			iota_test_printf("%d. Interface : IP = %s, port = %d, version = %d, type = %d\r\n", i, pAddrList[i].pIpAddr, pAddrList[i].uPort, pAddrList[i].version, pAddrList[i].type);
			if (pAddrList[i].type == ECRIO_UTILITY_DNS_Protocol_Type_Udp)
			{
				pNetwork->ppRemoteIPs[i] = strdup((char*)pAddrList[i].pIpAddr);
				pNetwork->uRemotePort = pAddrList[i].uPort;
			}
			else
			{
				iota_test_printf(" UDP IP Not Found! \n");
				error = 1;
				goto EndTag;
			}
		}
	} /* Only TCP */
	else if (pConfig->bEnableUdp == Enum_FALSE && pConfig->bEnableTcp == Enum_TRUE && pConfig->bTLSEnabled == Enum_FALSE)
	{   
		pNetwork->uNoOfRemoteIps = AddressCount;
		pNetwork->ppRemoteIPs = (char **)calloc(pNetwork->uNoOfRemoteIps, sizeof(char *));
		for (i = 0; i<AddressCount; i++)
		{
			iota_test_printf("%d. Interface : IP = %s, port = %d, version = %d, type = %d\r\n", i, pAddrList[i].pIpAddr, pAddrList[i].uPort, pAddrList[i].version, pAddrList[i].type);
			if (pAddrList[i].type == ECRIO_UTILITY_DNS_Protocol_Type_Tcp)
			{
				pNetwork->ppRemoteIPs[i] = strdup((char*)pAddrList[i].pIpAddr);
				pNetwork->uRemotePort = pAddrList[i].uPort;
			}
			else
			{
				iota_test_printf(" TCP IP Not Found! \n");
				error = 1;
				goto EndTag;
			}
		}
	} /* Only TLS */
	else if (pConfig->bEnableUdp == Enum_FALSE && pConfig->bEnableTcp == Enum_FALSE && pConfig->bTLSEnabled == Enum_TRUE)
	{
		pNetwork->uNoOfRemoteIps = tlsAddressCount;
		pNetwork->ppRemoteIPs = (char **)calloc(pNetwork->uNoOfRemoteIps, sizeof(char *));
		for (i = 0; i<tlsAddressCount; i++)
		{
			pNetwork->ppRemoteIPs[i] = strdup((char*)pTlsAddrList[i].pIpAddr);
			pNetwork->uRemotePort = pTlsAddrList[i].uPort;
			pNetwork->uRemoteTLSPort = pTlsAddrList[i].uPort;
		}
	}
	else
	{
		iota_test_printf(" Wrong Configuration SetUp! \n");
		error = 1;
		goto EndTag;
	}
EndTag:
	for (i = 0; i<AddressCount; i++)
	{
		if (pAddrList[i].pIpAddr != NULL)
		{
			pal_MemoryFree((void **)&pAddrList[i].pIpAddr);
		}
	}
	if (pAddrList != NULL)
	{
		pal_MemoryFree((void **)&pAddrList);
	}

	for (i = 0; i<tlsAddressCount; i++)
	{
		if (pTlsAddrList[i].pIpAddr != NULL)
		{
			pal_MemoryFree((void **)&pTlsAddrList[i].pIpAddr);
		}
	}
	if (pTlsAddrList != NULL)
	{
		pal_MemoryFree((void **)&pTlsAddrList);
	}
	return error;
}


#ifdef WIN32
#define WORKING_BUFFER_SIZE		16384
#define MAX_TRIES				3

char *iota_test_getLocalIpAddress
(
	unsigned int uipType,
	char *interfaceName
)
{
	DWORD dwRetVal = 0;
	ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST |
		GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_INCLUDE_PREFIX;
	ULONG outBufLen = 0;
	ULONG Iterations = 0;
	ULONG family = AF_INET;
	PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
	PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
	PIP_ADAPTER_ADDRESSES pipAdapterAddress = NULL;
	char buff[64];
	DWORD bufflen = 64;
	char nwIFDesc[128];
	char nwIFFriendlyName[128];
	PCSTR pIpAddress;
	char *pLocalIp = NULL;

	// The interfaceName is not currently used on Windows.
	(void)interfaceName;

	outBufLen = WORKING_BUFFER_SIZE;

	switch (uipType)
	{
		case 4:
		{
			family = AF_INET;
		}
		break;

		case 6:
		{
			family = AF_INET6;
		}
		break;
	}

	do
	{
		pipAdapterAddress = (IP_ADAPTER_ADDRESSES *)malloc(outBufLen);
		if (pipAdapterAddress == NULL)
		{
			return NULL;
		}

		dwRetVal = GetAdaptersAddresses(family, flags, NULL, pipAdapterAddress, &outBufLen);
		if (dwRetVal == ERROR_SUCCESS)
		{
			break;
		}

		Iterations++;
	} while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

	if (dwRetVal == NO_ERROR)
	{
		pCurrAddresses = pipAdapterAddress;

		while (pCurrAddresses)
		{
			wcstombs(nwIFDesc, pCurrAddresses->Description, wcslen(pCurrAddresses->Description));
			nwIFDesc[wcslen(pCurrAddresses->Description)] = '\0';
			iota_test_printf("network Interface Description:%s\r\n", nwIFDesc);
			wcstombs(nwIFFriendlyName, pCurrAddresses->FriendlyName, wcslen(pCurrAddresses->FriendlyName));
			nwIFFriendlyName[wcslen(pCurrAddresses->FriendlyName)] = '\0';
			iota_test_printf("network Interface Friendly Name:%s\r\n", nwIFFriendlyName);

			if (pCurrAddresses->OperStatus == 1)
			{
				iota_test_printf("Is this interface Active:YES\r\n");
			}
			else
			{
				iota_test_printf("Is this interface Active:NO\r\n");
			}

			if (((strstr(nwIFFriendlyName, "Ethernet") != NULL) || (_stricmp(nwIFFriendlyName, "Local Area Connection") == NULL) || (strstr(nwIFDesc, "Wireless") != NULL)
				|| (strstr(nwIFFriendlyName, "Wi-Fi") != NULL) || (strstr(nwIFFriendlyName, "Wireless") != NULL)) && pCurrAddresses->OperStatus == 1)
			{
				pUnicast = pCurrAddresses->FirstUnicastAddress;
				if (pUnicast != NULL)
				{
					if (pUnicast->Address.lpSockaddr->sa_family == AF_INET6)
					{
						sockaddr_in6 *sa_in6 = (sockaddr_in6 *)pUnicast->Address.lpSockaddr;
						pIpAddress = inet_ntop(AF_INET6, &(sa_in6->sin6_addr), buff, bufflen);

						pLocalIp = (char *)malloc(strlen(buff) + 1);
						if (pLocalIp != NULL)
						{
							sprintf(pLocalIp, "%s", buff);
							pLocalIp[strlen(buff)] = '\0';
						}

						goto End;
					}
					else
					{
						sockaddr_in *sa_in = (sockaddr_in *)pUnicast->Address.lpSockaddr;
						pIpAddress = inet_ntop(AF_INET, &(sa_in->sin_addr), buff, bufflen);

						pLocalIp = (char *)malloc(strlen(buff) + 1);
						if (pLocalIp != NULL)
						{
							sprintf(pLocalIp, "%s", buff);
							pLocalIp[strlen(buff)] = '\0';
						}

						goto End;
					}
				}
			}

			pCurrAddresses = pCurrAddresses->Next;
		}
	}

End:
	if (pipAdapterAddress)
	{
		free(pipAdapterAddress);
	}

	return pLocalIp;
}

#else
char *iota_test_getLocalIpAddress
(
	unsigned int uipType,
	char *interfaceName
)
{
	struct ifaddrs *ifAddrStruct = NULL;
	struct ifaddrs *ifa = NULL;
	void *tmpAddrPtr = NULL;
	char *pLocalIp = NULL;

	getifaddrs(&ifAddrStruct);

	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (!ifa->ifa_addr)
		{
			continue;
		}

		if (ifa->ifa_addr->sa_family == AF_INET)
		{
			if (uipType == 4)
			{
				tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
				char addressBuffer[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

				iota_test_printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);

				if (strcmp(ifa->ifa_name, interfaceName) == 0)
				{
					pLocalIp = (char *)malloc(strlen(addressBuffer) + 1);
					if (pLocalIp != NULL)
					{
						sprintf(pLocalIp, "%s", addressBuffer);
						pLocalIp[strlen(addressBuffer)] = '\0';
					}

					// We won't break, just to print out all interfaces and their names...
				}
			}
		}
		else if (ifa->ifa_addr->sa_family == AF_INET6)
		{
			if (uipType == 6)
			{
				tmpAddrPtr = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
				char addressBuffer[INET6_ADDRSTRLEN];
				inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);

				iota_test_printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);

				if (strcmp(ifa->ifa_name, interfaceName) == 0)
				{
					pLocalIp = (char *)malloc(strlen(addressBuffer) + 1);
					if (pLocalIp != NULL)
					{
						sprintf(pLocalIp, "%s", addressBuffer);
						pLocalIp[strlen(addressBuffer)] = '\0';
					}

					// We won't break, just to print out all interfaces and their names...
				}
			}
		}
	}

	if (ifAddrStruct != NULL)
	{
		freeifaddrs(ifAddrStruct);
	}

	return pLocalIp;
}

char *iota_test_getIpv6DeviceName
(
	void
)
{
	return iotaState.localInterface;
}

void iota_test_getAudioDeviceName
(
	char *audioCaptureName,
	char *audioRenderName
)
{
	if (audioCaptureName == NULL || audioRenderName == NULL)
	{
		return;
	}

	pal_MemoryCopy(audioCaptureName, 128, iotaState.audioCaptureDeviceName, pal_StringLength((const u_char *)iotaState.audioCaptureDeviceName));
	pal_MemoryCopy(audioRenderName, 128, iotaState.audioRenderDeviceName, pal_StringLength((const u_char *)iotaState.audioRenderDeviceName));
}
#endif
