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
 * @file EcrioPAL_Utility.c
 * @brief This is the implementation of the Ecrio PAL's Utility Module.
 */

/* Ignore deprecated function warnings. */
#pragma warning(disable: 4996)

/* Indicate which libraries this implementation needs. */
#pragma comment(lib, "winmm.lib")

/* Standard Windows includes. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Wincrypt.h>

#pragma warning(disable:4201)
#include <mmsystem.h>
#pragma warning(default:4201)

#define _CRT_RAND_S

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "EcrioPAL.h"

#define _ECRIO_UTILITY_SIZE_16		16

#define _ECRIO_UTILITY_DNS_SET_16BIT(p, v)  (((p)[0] = (unsigned char)(((v) >> 8) & 0xff)), \
											((p)[1] = (unsigned char)((v) & 0xff)))

#define _ECRIO_UTILITY_DNS_16BIT(p)  ((unsigned short)((unsigned int) 0xffff & \
										(((unsigned int)((unsigned char)(p)[0]) << 8U) | \
										((unsigned int)((unsigned char)(p)[1])))))

#define _ECRIO_UTILITY_SIZE_16		16
#define _ECRIO_UTILITY_DNS_EDNS_SIZE				11
#define _ECRIO_UTILITY_DNS_UDP_SIZE_MAX			512
#define _ECRIO_UTILITY_DNS_UDP_PAYLOAD_MAX		65535
#define _ECRIO_UTILITY_DNS_SERVER_PORT			53

#define _ECRIO_UTILITY_DNS_TYPE_A					1
#define _ECRIO_UTILITY_DNS_TYPE_AAAA				28
#define _ECRIO_UTILITY_DNS_TYPE_SRV				33
#define _ECRIO_UTILITY_DNS_TYPE_NAPTR				35
#define _ECRIO_UTILITY_DNS_TYPE_OPT				41

#define _ECRIO_UTILITY_DNS_NAPTR_SERVICE_INDEX	6
#define _ECRIO_UTILITY_DNS_SRV_PORT_INDEX		4

#define _ECRIO_UTILITY_DNS_PROTO_TYPE_STR_UDP	"udp"
#define _ECRIO_UTILITY_DNS_PROTO_TYPE_STR_TCP	"tcp"

#define _ECRIO_UTILITY_DNS_NAPTR_SERVICE_SIP		"SIP+D2T"
#define _ECRIO_UTILITY_DNS_NAPTR_SERVICE_SIPS	"SIPS+D2T"

typedef enum
{
	ECRIO_UTILITY_DNS_FQDN_States_None = 0,
	ECRIO_UTILITY_DNS_FQDN_States_Naptr_Req = 1,
	ECRIO_UTILITY_DNS_FQDN_States_Naptr_Res,
	ECRIO_UTILITY_DNS_FQDN_States_Srv_Tls_Req,
	ECRIO_UTILITY_DNS_FQDN_States_Srv_Tls_Res,
	ECRIO_UTILITY_DNS_FQDN_States_Srv_Nontls_Req,
	ECRIO_UTILITY_DNS_FQDN_States_Srv_Nontls_Res,
	ECRIO_UTILITY_DNS_FQDN_States_Tls_Host,
	ECRIO_UTILITY_DNS_FQDN_States_Nontls_Host,
	ECRIO_UTILITY_DNS_FQDN_States_Done
} ECRIO_UTILITY_DNS_FQDN_States;

/* DNS header structure */
struct _EcrioUtilityDnsHederStruct
{
	unsigned short id; // identification number

	unsigned char rd :1; // recursion desired
	unsigned char tc :1; // truncated message
	unsigned char aa :1; // authoritive answer
	unsigned char opcode :4; // purpose of message
	unsigned char qr :1; // query/response flag

	unsigned char rcode :4; // response code
	unsigned char cd :1; // checking disabled
	unsigned char ad :1; // authenticated data
	unsigned char z :1; // its z! reserved
	unsigned char ra :1; // recursion available

	unsigned short q_count; // number of question entries
	unsigned short ans_count; // number of answer entries
	unsigned short auth_count; // number of authority entries
	unsigned short add_count; // number of resource entries
};

/* Constant sized fields of DNS query structure */
struct _EcrioUtilityDnsQuestionStruct
{
	unsigned short qtype;
	unsigned short qclass;
};

/*Constant sized DNS fields of the resource record structure */
#pragma pack(push, 1)
struct _EcrioUtilityDnsRDataStruct
{
	unsigned short type;
	unsigned short _class;
	unsigned int uTtl;
	unsigned short data_len;
};
#pragma pack(pop)

/* Pointers to DNS resource record contents */
struct _EcrioUtilityDnsResRecordStruct
{
	unsigned char *pName;
	struct _EcrioUtilityDnsRDataStruct *pResource;
	unsigned char *pRData;
};

/* Structure of a DNS Query */
typedef struct
{
        unsigned char *pName;
        struct _EcrioUtilityDnsQuestionStruct *pQues;
} _EcrioUtilityDnsQuery;

typedef struct
{
	u_char *pNaptrAddr;
	u_char *pSrvAddr;
	ECRIO_UTILITY_DNS_Protocol_Type type;
	u_int32 uPort;
	u_int32 addrCount;
	EcrioUtilityDnsAddrStruct *pIpAddrStructList;
} _EcrioUtilityDnsDetails;

typedef struct
{
	u_char *pFqdnAddr;
	_EcrioUtilityDnsDetails tlsDnsDetails;
	_EcrioUtilityDnsDetails nonTlsDnsDetails;
} _EcrioUtilityDnsFqdnInfo;

/**
 * Implementation of the pal_UtilityRandomNumber() function. See the
 * EcrioPAL_Utility.h file for interface definitions.
 */
u_int32 pal_UtilityRandomNumber
(
	void
)
{
	unsigned int randomValue;

	/** Return a pseudorandom number using the rand_s() function. */
	rand_s(&randomValue);

	return (randomValue);
}

u_char *pal_UtilityRandomBin16
(
	void
)
{
	HCRYPTPROV hProvider = 0;

	if (!::CryptAcquireContextW(&hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
	{
		return NULL;
	}

	BYTE pbBuffer[_ECRIO_UTILITY_SIZE_16] = {};
	u_char *pBuff = NULL;

	if (!::CryptGenRandom(hProvider, _ECRIO_UTILITY_SIZE_16, pbBuffer))
	{
		::CryptReleaseContext(hProvider, 0);
		return NULL;
	}

	pal_MemoryAllocate(_ECRIO_UTILITY_SIZE_16, (void **)&pBuff);
	if (pBuff == NULL)
	{
		::CryptReleaseContext(hProvider, 0);
		return NULL;
	}

	pal_MemoryCopy(pBuff, _ECRIO_UTILITY_SIZE_16, pbBuffer, _ECRIO_UTILITY_SIZE_16);

	::CryptReleaseContext(hProvider, 0);

	return pBuff;
}

u_int32 pal_UtilityGetMillisecondCount
(
	void
)
{
	return timeGetTime();
}


u_int32 pal_UtilityGetDateAndTime
(
	EcrioDateAndTimeStruct* pDateAndTime
)
{
	SYSTEMTIME sTime;
	if(pDateAndTime == NULL)
		return KPALInvalidParameters;

	/* Get the current Local time */
	GetLocalTime(&sTime);

	pDateAndTime->second = sTime.wSecond;
	pDateAndTime->hour = sTime.wHour;
	pDateAndTime->minute = sTime.wMinute;
	pDateAndTime->day = sTime.wDay;
	pDateAndTime->month = sTime.wMonth;
	pDateAndTime->year = sTime.wYear;
	return KPALErrorNone;
}

void pal_UtilityUsleep
(
	u_int32 usec
)
{
#ifdef ENABLE_QCMAPI_SLEEP
	usleep(usec);
#endif
}

//this will convert www.google.com to 3www6google3com ;got it
static void _pal_UtilityDnsChangetoDnsNameFormat(unsigned char* dns,unsigned char* host) {
    int lock = 0;
    int i;

    strcat((char *)host, ".");

    for(i = 0 ; i < strlen((const char *)host) ; i++) {
        if(host[i]=='.') {
            *dns++=i-lock;

            for(;lock<i;lock++) {
                *dns++=host[lock];
            }

            lock++; //or lock=i+1;
        }
    }

    *dns++='\0';

}

static unsigned char* _pal_UtilityDnsReadName(unsigned char* reader,unsigned char* buffer,int* count)
{
    unsigned char *name = NULL;
    unsigned int p=0,jumped=0,offset;
    int i , j;

    *count = 1;
	pal_MemoryAllocate(256 * sizeof(char *), (void **)&name);
	if (name == NULL)
	{
		return NULL;
	}
	pal_MemorySet(name, 0, 256);

    name[0]='\n';

    //read the names in 3www6google3com format
    while(*reader!=0)
    {
        if(*reader>=192)
        {
            offset = (*reader)*256 + *(reader+1) - 49152;
            reader = buffer + offset - 1;
            jumped = 1; //we have jumped to another location so counting wont go up!
        }
        else
            name[p++]=*reader;

        reader=reader+1;

        if(jumped==0)
            *count = *count + 1; //if we havent jumped to another location then we can count up
    }

    name[p]='\0'; //string complete
    if(jumped==1)
        *count = *count + 1; //number of steps we actually moved forward in the packet

    //now convert 3www6google3com0 to www.google.com
    for(i=0;i<strlen((const char *)name);i++) {
        p=name[i];
        for(j=0;j<(int)p;j++) {
            name[i]=name[i+1];
            i=i+1;
        }
        name[i]='.';
    }
    name[i-1]='\0'; //remove the last dot
    return name;
}

static void _pal_UtilityDnsCreateQuery(unsigned char *pQeury, u_int32 *uQueryLen, char *pHostName, u_int32 utype)
{
	unsigned char *qname = NULL, *reader = NULL, *hostNameTmp = NULL, *arecords = NULL;
	s_int32 s, i, j, stop;
	struct _EcrioUtilityDnsResRecordStruct answers[20];
	struct _EcrioUtilityDnsHederStruct *dns = NULL;
	struct _EcrioUtilityDnsQuestionStruct *qinfo = NULL;

	//Set the DNS structure to standard queries
	dns = (struct _EcrioUtilityDnsHederStruct *)pQeury;

	dns->id = (unsigned short)htons(pal_UtilityRandomNumber());
	dns->qr = 0; //This is a query
	dns->opcode = 0; //This is a standard query
	dns->aa = 0; //Not Authoritative
	dns->tc = 0; //This message is not truncated
	dns->rd = 1; //Recursion Desired
	dns->ra = 0; //Recursion not available! hey we dont have it (lol)
	dns->z = 0;
	dns->ad = 0;
	dns->cd = 0;
	dns->rcode = 0;
	dns->q_count = htons(1); //we have only 1 question
	dns->ans_count = 0;
	dns->auth_count = 0;
	dns->add_count = htons(1);;

	//point to the query portion
	qname =(unsigned char *)&pQeury[sizeof(struct _EcrioUtilityDnsHederStruct)];

	/* In the _pal_UtilityDnsChangetoDnsNameFormat(), "." will be added into the pHostname, so make a temporary buffer enough for the adding */
	pal_MemoryAllocate(pal_StringLength((const u_char *)pHostName) +2, (void **)&hostNameTmp);
	if (hostNameTmp == NULL)
	{
		return;
	}
	pal_MemorySet(hostNameTmp, 0, pal_StringLength((const u_char *)pHostName) +2);
	pal_StringNCopy((u_char *)hostNameTmp, pal_StringLength((const u_char *)pHostName) +2, (const u_char *)pHostName, pal_StringLength((const u_char *)pHostName));
	_pal_UtilityDnsChangetoDnsNameFormat(qname , (u_char *)hostNameTmp);
	pal_MemoryFree((void **)&hostNameTmp);

	qinfo = (struct _EcrioUtilityDnsQuestionStruct *)&pQeury[sizeof(struct _EcrioUtilityDnsHederStruct) + (pal_StringLength((const u_char*)qname) + 1)];

	qinfo->qtype = htons(utype);
	qinfo->qclass = htons(1);

	arecords = (unsigned char *)&pQeury[sizeof(struct _EcrioUtilityDnsHederStruct) + (pal_StringLength((const u_char*)qname) + 1) + sizeof(struct _EcrioUtilityDnsQuestionStruct)];
	memset(arecords, 0, _ECRIO_UTILITY_DNS_EDNS_SIZE);
	arecords++;
	_ECRIO_UTILITY_DNS_SET_16BIT(arecords, _ECRIO_UTILITY_DNS_TYPE_OPT);
	_ECRIO_UTILITY_DNS_SET_16BIT(arecords + 2, _ECRIO_UTILITY_DNS_UDP_SIZE_MAX);
	arecords += (_ECRIO_UTILITY_DNS_EDNS_SIZE - 1);

	*uQueryLen = sizeof(struct _EcrioUtilityDnsHederStruct) + (pal_StringLength((const u_char*)qname) + 1) + sizeof(struct _EcrioUtilityDnsQuestionStruct) + _ECRIO_UTILITY_DNS_EDNS_SIZE;
}

u_int32 _pal_UtilityLookupHost(_EcrioUtilityDnsDetails *pDnsDetails)
{
	unsigned int addrCount = 0;
	EcrioUtilityDnsAddrStruct *pAddrList = NULL;
	struct addrinfo hints, *pRes, *pResult;
	u_int32 error = KPALErrorNone;
	int res = 0;

	if (pDnsDetails == NULL)
	{
		return KPALInvalidParameters;
	}

	if (pDnsDetails->pSrvAddr  == NULL)
	{
		return KPALInvalidParameters;
	}

	pal_MemorySet(&hints, 0, sizeof (hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = 0;
	hints.ai_flags = AI_PASSIVE;

	res = getaddrinfo((const char *)pDnsDetails->pSrvAddr, NULL, &hints, &pResult);
	if (res != 0)
	{
		return KPALInternalError;
	}

	pRes = pResult;
	while (pRes)
	{
		char	ipString[101] = {0};
		void *ptr = NULL;
		u_int32 ipLen = 0;
		BoolEnum bMatched = Enum_FALSE;

		inet_ntop(pRes->ai_family, pRes->ai_addr->sa_data, ipString, 100);
		switch (pRes->ai_family)
		{
			case AF_INET:
				ptr = &((struct sockaddr_in *)pRes->ai_addr)->sin_addr;
				break;
			case AF_INET6:
				ptr = &((struct sockaddr_in6 *)pRes->ai_addr)->sin6_addr;
				break;
		}
		inet_ntop(pRes->ai_family, ptr, ipString, 100);

		ipLen = pal_StringLength((u_char*)ipString);

		if (ipLen != 0)
		{
			for (u_int32 i = 0; i < addrCount; i++)
			{
				if (pal_StringNCompare((u_char*)ipString, pAddrList[i].pIpAddr, ipLen) == 0)
				{
					bMatched = Enum_TRUE;
					break;
				}
			}

			if (bMatched == Enum_TRUE)
			{
				pRes = pRes->ai_next;
				continue;
			}

			if(pAddrList)
			{
				pal_MemoryReallocate((addrCount+1)*sizeof(EcrioUtilityDnsAddrStruct),(void**)&pAddrList);
			}
			else
			{
				pal_MemoryAllocate(sizeof(EcrioUtilityDnsAddrStruct),(void**)&pAddrList);
			}

			pAddrList[addrCount].pIpAddr = (u_char*)pal_StringCreate((u_char*)ipString, ipLen);
			switch (pRes->ai_family)
			{
				case AF_INET:
					pAddrList[addrCount].version = ECRIO_UTILITY_DNS_Protocol_Version_IPv4;
					break;
				case AF_INET6:
					pAddrList[addrCount].version = ECRIO_UTILITY_DNS_Protocol_Version_IPv6;
					break;
			}

			pAddrList[addrCount].type =  pDnsDetails->type;
			pAddrList[addrCount].uPort =  pDnsDetails->uPort;

			addrCount++;
		}

		pRes = pRes->ai_next;
	}

	freeaddrinfo(pResult);

	pDnsDetails->pIpAddrStructList = pAddrList;
	pDnsDetails->addrCount = addrCount;

	return error;
}

u_int32 pal_UtilityDnsGetInfoFromFqdn
(
	PALINSTANCE pal,
	const char *pHostName,
	const char *pLocal,
	const char *pDnsAddress,
	u_int32* pTlsAddrCount,
	EcrioUtilityDnsAddrStruct **ppTlsDnsAddrList,
	u_int32* pNonTlsAddrCount,
	EcrioUtilityDnsAddrStruct **ppNonTlsDnsAddrList
)
{
	u_int32 error = KPALErrorNone;
	unsigned int addrCount = 0;
	EcrioUtilityDnsAddrStruct *pAddrList = NULL;
	unsigned char dnsPayload[_ECRIO_UTILITY_DNS_UDP_PAYLOAD_MAX] = {0};
	unsigned char *reader = NULL;
	u_int32 uDnsPayloadLen = 0;
	s_int32 s = 0;
	struct sockaddr_in dest;
	BoolEnum bDone = Enum_FALSE;
	ECRIO_UTILITY_DNS_FQDN_States state = ECRIO_UTILITY_DNS_FQDN_States_None;
	u_char *pTlsNaptrName = NULL;
	u_char *pNonTlsNaptrName = NULL;
	_EcrioUtilityDnsFqdnInfo fqdnInfo;

	if (pHostName == NULL || pDnsAddress == NULL)
	{
		return KPALInvalidParameters;
	}

	if (pTlsAddrCount == NULL || ppTlsDnsAddrList == NULL)
	{
		return KPALInvalidParameters;
	}

	if (pNonTlsAddrCount == NULL || ppNonTlsDnsAddrList == NULL)
	{
		return KPALInvalidParameters;
	}

	pal_MemorySet((void *)&fqdnInfo, 0, sizeof(_EcrioUtilityDnsFqdnInfo));
	fqdnInfo.pFqdnAddr = (u_char *)pHostName;

        s = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP); //UDP packet for DNS queries
        if (s == -1)
    	{
    		return KPALInternalError;
    	}
        memset((char *)&dest, 0, sizeof(dest));
        dest.sin_family = AF_INET;
        dest.sin_port = htons(_ECRIO_UTILITY_DNS_SERVER_PORT);
        dest.sin_addr.s_addr = inet_addr((const char*)pDnsAddress); //dns servers

	state = ECRIO_UTILITY_DNS_FQDN_States_Naptr_Req;
	while (!bDone)
	{
		s_int32 flags = 0, i = 0, stop = 0;
		timeval recvTimeOut;
		fd_set recvFds;
		struct _EcrioUtilityDnsHederStruct *dns = NULL;
		struct _EcrioUtilityDnsQuestionStruct *qinfo = NULL;
		struct _EcrioUtilityDnsResRecordStruct answers[20];

		switch (state)
		{
			case ECRIO_UTILITY_DNS_FQDN_States_Naptr_Req :
			{
				uDnsPayloadLen = 0;
				pal_MemorySet(&dnsPayload, 0, _ECRIO_UTILITY_DNS_UDP_PAYLOAD_MAX);
				_pal_UtilityDnsCreateQuery((u_char *)&dnsPayload, &uDnsPayloadLen, (char *)pHostName, _ECRIO_UTILITY_DNS_TYPE_NAPTR); // creates a DNS query for the NAPTR type.

				if (sendto(s,(char*)dnsPayload,uDnsPayloadLen,0, (struct sockaddr*)&dest , sizeof(dest)) == 0)
				{
					closesocket(s);
					error = KPALInternalError;
					goto Error;
				}

				state = ECRIO_UTILITY_DNS_FQDN_States_Naptr_Res;
			}
			break;

			case ECRIO_UTILITY_DNS_FQDN_States_Naptr_Res :
			{
				flags = sizeof(dest);

				/* In case of no responding from DNS server, set the maximum waiting time here */
				recvTimeOut.tv_sec = 3;  // Maximum waiting itmer will be 3 seconds.
				recvTimeOut.tv_usec = 0;
				FD_ZERO(&recvFds);
				FD_SET(s, &recvFds);

				switch(select(FD_SETSIZE, &recvFds, 0, 0, &recvTimeOut))
				{
				case 0 : /* 3 seconds elapsed, so just return no result from server. */
					closesocket(s);
					error = KPALInternalError;
					goto Error;

				case -1 : /* Internal error occurred */
					closesocket(s);
					error = KPALInternalError;
					goto Error;

				default : /* Data is ready to receive */

					pal_MemorySet(&dnsPayload, 0, _ECRIO_UTILITY_DNS_UDP_PAYLOAD_MAX);
					if(recvfrom(s,(char*)dnsPayload, _ECRIO_UTILITY_DNS_UDP_PAYLOAD_MAX, 0, (struct sockaddr*)&dest, (socklen_t *)&flags) == 0)
					{
						closesocket(s);
						error = KPALInternalError;
						goto Error;
					}
				}

				dns = (struct _EcrioUtilityDnsHederStruct*)dnsPayload;

				//move ahead of the dns header and the query field
				reader = &dnsPayload[uDnsPayloadLen - _ECRIO_UTILITY_DNS_EDNS_SIZE];

				stop=0;
				for(i=0;i<ntohs(dns->ans_count);i++)
				{
					u_int8 serviceLen = 0; 
					u_char *service = NULL;

					answers[i].pName = NULL;
					answers[i].pName=_pal_UtilityDnsReadName(reader, dnsPayload, &stop);
					reader = reader + stop;

					answers[i].pResource = (struct _EcrioUtilityDnsRDataStruct*)(reader);
					reader = reader + sizeof(struct _EcrioUtilityDnsRDataStruct);

					if(ntohs(answers[i].pResource->type) == _ECRIO_UTILITY_DNS_TYPE_NAPTR) //if its an ipv4 address
					{
						reader += _ECRIO_UTILITY_DNS_NAPTR_SERVICE_INDEX;
						serviceLen = reader[0];
						reader++;
						service = pal_StringCreate(reader, serviceLen);
						if (pal_StringNCompare(service, (const u_char*)_ECRIO_UTILITY_DNS_NAPTR_SERVICE_SIP, pal_StringLength((const u_char*)_ECRIO_UTILITY_DNS_NAPTR_SERVICE_SIP)) == 0)
						{
							reader += (serviceLen + 1);
							fqdnInfo.nonTlsDnsDetails.pNaptrAddr = _pal_UtilityDnsReadName(reader, dnsPayload, &stop);
							if (pal_StringFindSubString(fqdnInfo.nonTlsDnsDetails.pNaptrAddr, (const u_char*)_ECRIO_UTILITY_DNS_PROTO_TYPE_STR_UDP) != NULL)
							{
								fqdnInfo.nonTlsDnsDetails.type = ECRIO_UTILITY_DNS_Protocol_Type_Udp;
							}
							else if (pal_StringFindSubString(fqdnInfo.nonTlsDnsDetails.pNaptrAddr, (const u_char*)_ECRIO_UTILITY_DNS_PROTO_TYPE_STR_TCP) != NULL)
							{
								fqdnInfo.nonTlsDnsDetails.type = ECRIO_UTILITY_DNS_Protocol_Type_Tcp;
							}
							reader = reader + stop;
						}
						else if (pal_StringNCompare(service, (const u_char*)_ECRIO_UTILITY_DNS_NAPTR_SERVICE_SIPS, pal_StringLength((const u_char*)_ECRIO_UTILITY_DNS_NAPTR_SERVICE_SIPS)) == 0)
						{
							reader += (serviceLen + 1);
							fqdnInfo.tlsDnsDetails.pNaptrAddr = _pal_UtilityDnsReadName(reader, dnsPayload, &stop);
							if (pal_StringFindSubString(fqdnInfo.tlsDnsDetails.pNaptrAddr, (const u_char*)_ECRIO_UTILITY_DNS_PROTO_TYPE_STR_UDP) != NULL)
							{
								fqdnInfo.tlsDnsDetails.type = ECRIO_UTILITY_DNS_Protocol_Type_Udp;
							}
							else if (pal_StringFindSubString(fqdnInfo.tlsDnsDetails.pNaptrAddr, (const u_char*)_ECRIO_UTILITY_DNS_PROTO_TYPE_STR_TCP) != NULL)
							{
								fqdnInfo.tlsDnsDetails.type = ECRIO_UTILITY_DNS_Protocol_Type_Tcp;
							}
							reader = reader + stop;
						}
						else
						{
							if (service != NULL)
							{
								pal_MemoryFree((void **)&service);
							}

							if (answers[i].pName != NULL)
							{
								pal_MemoryFree((void **)&answers[i].pName);
							}

							closesocket(s);
							error = KPALInternalError;
							goto Error;
						}

						if (service != NULL)
						{
							pal_MemoryFree((void **)&service);
						}
					}
					else
					{
						answers[i].pRData = _pal_UtilityDnsReadName(reader, dnsPayload, &stop);
						reader = reader + stop;
						pal_MemoryFree((void **)&answers[i].pRData);
					}

					if (answers[i].pName != NULL)
					{
						pal_MemoryFree((void **)&answers[i].pName);
					}
				}

				if (fqdnInfo.nonTlsDnsDetails.pNaptrAddr == NULL || fqdnInfo.tlsDnsDetails.pNaptrAddr == NULL)
				{
					closesocket(s);
					error = KPALInternalError;
					goto Error;
				}

				state = ECRIO_UTILITY_DNS_FQDN_States_Srv_Tls_Req;
			}
			break;

			case ECRIO_UTILITY_DNS_FQDN_States_Srv_Tls_Req :
			{
				if (fqdnInfo.tlsDnsDetails.pNaptrAddr == NULL)
				{
					closesocket(s);
					error = KPALInternalError;
					goto Error;
				}

				uDnsPayloadLen = 0;
				pal_MemorySet(&dnsPayload, 0, _ECRIO_UTILITY_DNS_UDP_PAYLOAD_MAX);
				_pal_UtilityDnsCreateQuery((u_char *)&dnsPayload, &uDnsPayloadLen, (char *)fqdnInfo.tlsDnsDetails.pNaptrAddr, _ECRIO_UTILITY_DNS_TYPE_SRV); // creates a DNS query for the SRV type.

				if (sendto(s,(char*)dnsPayload,uDnsPayloadLen,0, (struct sockaddr*)&dest , sizeof(dest)) == 0)
				{
					closesocket(s);
					error = KPALInternalError;
					goto Error;
				}

				state = ECRIO_UTILITY_DNS_FQDN_States_Srv_Tls_Res;
			}
			break;

			case ECRIO_UTILITY_DNS_FQDN_States_Srv_Tls_Res :
			{
				flags = sizeof(dest);

				/* In case of no responding from DNS server, set the maximum waiting time here */
				recvTimeOut.tv_sec = 3;  // Maximum waiting itmer will be 3 seconds.
				recvTimeOut.tv_usec = 0;
				FD_ZERO(&recvFds);
				FD_SET(s, &recvFds);

				switch(select(FD_SETSIZE, &recvFds, 0, 0, &recvTimeOut))
				{
				case 0 : /* 3 seconds elapsed, so just return no result from server. */
					closesocket(s);
					error = KPALInternalError;
					goto Error;

				case -1 : /* Internal error occurred */
					closesocket(s);
					error = KPALInternalError;
					goto Error;

				default : /* Data is ready to receive */

					pal_MemorySet(&dnsPayload, 0, _ECRIO_UTILITY_DNS_UDP_PAYLOAD_MAX);
					if(recvfrom(s,(char*)dnsPayload, _ECRIO_UTILITY_DNS_UDP_PAYLOAD_MAX, 0, (struct sockaddr*)&dest, (socklen_t *)&flags) == 0)
					{
						closesocket(s);
						error = KPALInternalError;
						goto Error;
					}
				}

				dns = (struct _EcrioUtilityDnsHederStruct*)dnsPayload;

				//move ahead of the dns header and the query field
				reader = &dnsPayload[uDnsPayloadLen - _ECRIO_UTILITY_DNS_EDNS_SIZE];

				stop=0;
				for(i=0;i<ntohs(dns->ans_count);i++)
				{
					unsigned short port = 0; 

					answers[i].pName = NULL;
					answers[i].pName=_pal_UtilityDnsReadName(reader, dnsPayload, &stop);
					reader = reader + stop;

					answers[i].pResource = (struct _EcrioUtilityDnsRDataStruct*)(reader);
					reader = reader + sizeof(struct _EcrioUtilityDnsRDataStruct);

					if(ntohs(answers[i].pResource->type) == _ECRIO_UTILITY_DNS_TYPE_SRV) //if its an ipv4 address
					{
						reader += _ECRIO_UTILITY_DNS_SRV_PORT_INDEX;
						port = _ECRIO_UTILITY_DNS_16BIT(reader);
						fqdnInfo.tlsDnsDetails.uPort = port;
						reader += sizeof(unsigned short);

						fqdnInfo.tlsDnsDetails.pSrvAddr = _pal_UtilityDnsReadName(reader, dnsPayload, &stop);
						reader = reader + stop;
					}
					else
					{
						answers[i].pRData = _pal_UtilityDnsReadName(reader, dnsPayload, &stop);
						reader = reader + stop;
						pal_MemoryFree((void **)&answers[i].pRData);
					}

					if (answers[i].pName != NULL)
					{
						pal_MemoryFree((void **)&answers[i].pName);
					}
				}

				state = ECRIO_UTILITY_DNS_FQDN_States_Srv_Nontls_Req;
			}
			break;

			case ECRIO_UTILITY_DNS_FQDN_States_Srv_Nontls_Req :
			{
				if (fqdnInfo.nonTlsDnsDetails.pNaptrAddr == NULL)
				{
					closesocket(s);
					error = KPALInternalError;
					goto Error;
				}

				uDnsPayloadLen = 0;
				pal_MemorySet(&dnsPayload, 0, _ECRIO_UTILITY_DNS_UDP_PAYLOAD_MAX);
				_pal_UtilityDnsCreateQuery((u_char *)&dnsPayload, &uDnsPayloadLen, (char *)fqdnInfo.nonTlsDnsDetails.pNaptrAddr, _ECRIO_UTILITY_DNS_TYPE_SRV); // creates a DNS query for the SRV type.

				if (sendto(s,(char*)dnsPayload,uDnsPayloadLen,0, (struct sockaddr*)&dest , sizeof(dest)) == 0)
				{
					closesocket(s);
					error = KPALInternalError;
					goto Error;
				}

				state = ECRIO_UTILITY_DNS_FQDN_States_Srv_Nontls_Res;
			}
			break;

			case ECRIO_UTILITY_DNS_FQDN_States_Srv_Nontls_Res :
			{
				flags = sizeof(dest);

				/* In case of no responding from DNS server, set the maximum waiting time here */
				recvTimeOut.tv_sec = 3;  // Maximum waiting itmer will be 3 seconds.
				recvTimeOut.tv_usec = 0;
				FD_ZERO(&recvFds);
				FD_SET(s, &recvFds);

				switch(select(FD_SETSIZE, &recvFds, 0, 0, &recvTimeOut))
				{
				case 0 : /* 3 seconds elapsed, so just return no result from server. */
					closesocket(s);
					error = KPALInternalError;
					goto Error;

				case -1 : /* Internal error occurred */
					closesocket(s);
					error = KPALInternalError;
					goto Error;

				default : /* Data is ready to receive */

					pal_MemorySet(&dnsPayload, 0, _ECRIO_UTILITY_DNS_UDP_PAYLOAD_MAX);
					if(recvfrom(s,(char*)dnsPayload, _ECRIO_UTILITY_DNS_UDP_PAYLOAD_MAX, 0, (struct sockaddr*)&dest, (socklen_t *)&flags) == 0)
					{
						closesocket(s);
						error = KPALInternalError;
						goto Error;
					}
				}

				dns = (struct _EcrioUtilityDnsHederStruct*)dnsPayload;

				//move ahead of the dns header and the query field
				reader = &dnsPayload[uDnsPayloadLen - _ECRIO_UTILITY_DNS_EDNS_SIZE];

				stop=0;
				for(i=0;i<ntohs(dns->ans_count);i++)
				{
					unsigned short port = 0; 

					answers[i].pName = NULL;
					answers[i].pName=_pal_UtilityDnsReadName(reader, dnsPayload, &stop);
					reader = reader + stop;

					answers[i].pResource = (struct _EcrioUtilityDnsRDataStruct*)(reader);
					reader = reader + sizeof(struct _EcrioUtilityDnsRDataStruct);

					if(ntohs(answers[i].pResource->type) == _ECRIO_UTILITY_DNS_TYPE_SRV) //if its an ipv4 address
					{
						reader += _ECRIO_UTILITY_DNS_SRV_PORT_INDEX;
						port = _ECRIO_UTILITY_DNS_16BIT(reader);
						fqdnInfo.nonTlsDnsDetails.uPort = port;
						reader += sizeof(unsigned short);

						fqdnInfo.nonTlsDnsDetails.pSrvAddr = _pal_UtilityDnsReadName(reader, dnsPayload, &stop);
						reader = reader + stop;
					}
					else
					{
						answers[i].pRData = _pal_UtilityDnsReadName(reader, dnsPayload, &stop);
						reader = reader + stop;
						pal_MemoryFree((void **)&answers[i].pRData);
					}

					if (answers[i].pName != NULL)
					{
						pal_MemoryFree((void **)&answers[i].pName);
					}
				}

				state = ECRIO_UTILITY_DNS_FQDN_States_Tls_Host;
			}
			break;

			case ECRIO_UTILITY_DNS_FQDN_States_Tls_Host :
			{
				error = _pal_UtilityLookupHost(&fqdnInfo.tlsDnsDetails);
				if (error != KPALErrorNone)
				{
					closesocket(s);
					error = KPALInternalError;
					goto Error;
				}

				*pTlsAddrCount = fqdnInfo.tlsDnsDetails.addrCount;
				*ppTlsDnsAddrList = fqdnInfo.tlsDnsDetails.pIpAddrStructList;

				state = ECRIO_UTILITY_DNS_FQDN_States_Nontls_Host;
			}
			break;

			case ECRIO_UTILITY_DNS_FQDN_States_Nontls_Host :
			{
				error = _pal_UtilityLookupHost(&fqdnInfo.nonTlsDnsDetails);
				if (error != KPALErrorNone)
				{
					closesocket(s);
					error = KPALInternalError;
					goto Error;
				}

				*pNonTlsAddrCount = fqdnInfo.nonTlsDnsDetails.addrCount;
				*ppNonTlsDnsAddrList = fqdnInfo.nonTlsDnsDetails.pIpAddrStructList;

				state = ECRIO_UTILITY_DNS_FQDN_States_Done;
			}
			break;

			case ECRIO_UTILITY_DNS_FQDN_States_Done :
			{
				closesocket(s);

				bDone = Enum_TRUE;
			}
			break;

			default :
				bDone = Enum_TRUE;
			break;
		}
	}

	goto Done;

Error:
	*pTlsAddrCount = 0;
	*ppTlsDnsAddrList = NULL;
	*pNonTlsAddrCount = 0;
	*ppNonTlsDnsAddrList = NULL;

Done:
	if (fqdnInfo.tlsDnsDetails.pNaptrAddr != NULL)
	{
		pal_MemoryFree((void **)&fqdnInfo.tlsDnsDetails.pNaptrAddr);
	}

	if (fqdnInfo.tlsDnsDetails.pSrvAddr != NULL)
	{
		pal_MemoryFree((void **)&fqdnInfo.tlsDnsDetails.pSrvAddr);
	}

	if (fqdnInfo.nonTlsDnsDetails.pNaptrAddr != NULL)
	{
		pal_MemoryFree((void **)&fqdnInfo.nonTlsDnsDetails.pNaptrAddr);
	}

	if (fqdnInfo.nonTlsDnsDetails.pSrvAddr != NULL)
	{
		pal_MemoryFree((void **)&fqdnInfo.nonTlsDnsDetails.pSrvAddr);
	}

	return error;
}

BoolEnum pal_UtilityDataOverflowDetected(u_int32 index, u_int32 dataSize)
{
	u_int32 boundary = UINT_MAX / dataSize;
	if (index > boundary)
		return Enum_TRUE;
	else
		return Enum_FALSE;
}

BoolEnum pal_UtilityArithmeticOverflowDetected(u_int32 uSrc, u_int32 uValue)
{
	u_int32 boundary = UINT_MAX / 2;

	if (uSrc > uValue)
	{
		/* Check if uSrc > uValue and the result of additive arithmetic makes
		 * uSrc smaller than the original value, it detects overflow. */
		if ((uSrc + uValue) < uSrc)
			return Enum_TRUE;
	}
	else if (uSrc < uValue)
	{
		/* Check if uSrc < uValue and the result of additive arithmetic makes
		 * uValue smaller than the original value, it detects overflow. */
		if ((uSrc + uValue) < uValue)
			return Enum_TRUE;
	}
	else
	{
		/* If uSrc = uValue, check if uSrc and uValue greater than half of
		 * UINT_MAX. if true then uSrc+uValue will result in overflow. */
		if (uSrc >= boundary && uValue >= boundary)
			return Enum_TRUE;
	}

	return Enum_FALSE;
}

BoolEnum pal_UtilityArithmeticUnderflowDetected(u_int32 uSrc, u_int32 uValue)
{
    /* Check if uSrc is less than uValue. If true than uSrc-uValue will be
     * value less than 0 resulting in underflow */
	if(uSrc < uValue)
		return Enum_TRUE;
	else
		return Enum_FALSE;
}
