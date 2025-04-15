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
 * @file EcrioPAL_Utility.cpp
 * @brief This is the implementation of the Ecrio PAL's Utility Module.
 */

#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <list>
#include <thread>

using namespace std;

#include "EcrioPAL.h"

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

#define _ECRIO_UTILITY_SIGNAL_EXIT						100
#define _ECRIO_UTILITY_SIGNAL_NOTIFY					101
#define _ECRIO_UTILITY_SIGNAL_ERROR					102


typedef void* _ECRIO_UTILITY_SIGNAL_HANDLE;

/* Forward declarations. */
/* messageQ message Structure */
typedef struct
{
	unsigned int uCommand;
	unsigned int uParam;
	void* pParam;
} _EcrioUtilityMessageStruct;

/* messageQ structure */
typedef struct
{
	mutex m;
	list <_EcrioUtilityMessageStruct> messageQ;
} _EcrioUtilityMessageQStruct;

typedef unsigned int(*_EcrioUtilitySignalCallback)
(
	_ECRIO_UTILITY_SIGNAL_HANDLE handle,
	unsigned int uCommand,
	unsigned int uParam,
	void* pParam
);

typedef struct
{
	mutex m;
	condition_variable cond;
	thread hThread;
	_EcrioUtilityMessageQStruct messageQStruct;
	_EcrioUtilitySignalCallback callback;
	BoolEnum bEnding;
	LOGHANDLE logHandle;
} _EcrioUtilitySignalStruct;

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

	PALINSTANCE pal;
	SOCKETHANDLE handle;
	_ECRIO_UTILITY_SIGNAL_HANDLE hSignal;

	EcrioUtilityDnsAnswerCallback pCallback;
	void *pCallbackFnData;

	char *pHostName;
	char *pLocalIP;
	char *pServerIP;
	u_int32 uDnsPayloadLen;

	ECRIO_UTILITY_DNS_FQDN_States state;
} _EcrioUtilityDnsFqdnInfo;

u_int32 pal_UtilityRandomNumber
(
	void
)
{
	int fd = 0;
	int error = 0;
	u_int32 bufIm;
	u_int32 *buf = &bufIm;
  
	fd = open("/dev/urandom", O_RDONLY);
	if (fd == -1)
	{
		return KPALUnknownError;
	}

	error = read(fd, buf, sizeof(int));
	if (error == -1)
	{
		return KPALUnknownError;
	}

	return bufIm;
}

u_char *pal_UtilityRandomBin16
(
	void
)
{
	int fd = 0;
	int error = 0;
	u_char buf[_ECRIO_UTILITY_SIZE_16];
	u_char *pBuff = NULL;
 
	fd = open("/dev/urandom", O_RDONLY);
	if (fd == -1)
	{
		return NULL;
	}

	error = read(fd, buf, _ECRIO_UTILITY_SIZE_16);
	if (error == -1)
	{
		return NULL;
	}

	pal_MemoryAllocate(_ECRIO_UTILITY_SIZE_16, (void **)&pBuff);
	if (pBuff == NULL)
	{
		return NULL;
	}

	pal_MemoryCopy(pBuff, _ECRIO_UTILITY_SIZE_16, buf, _ECRIO_UTILITY_SIZE_16);
	
	return pBuff;
}

u_int32 pal_UtilityGetMillisecondCount
(
	void
)
{
	struct timespec tickTime;

	clock_gettime(CLOCK_MONOTONIC, &tickTime);

	return (tickTime.tv_sec * 1000) + (tickTime.tv_nsec / 1000000);
}

u_int32 pal_UtilityGetDateAndTime
(
	EcrioDateAndTimeStruct* pDateAndTime
)
{
	struct tm *sTime = NULL;
	struct timeval tv;

	/* Validate incoming pointers. */
	if (pDateAndTime == NULL)
	{
		return KPALInvalidParameters;
	}

	/* Get the current UTC time. */
    gettimeofday(&tv, NULL);
	sTime = gmtime(&tv.tv_sec);

	pDateAndTime->second = sTime->tm_sec;
	pDateAndTime->hour = sTime->tm_hour;
	pDateAndTime->minute = sTime->tm_min;
	pDateAndTime->day = sTime->tm_mday;
	pDateAndTime->month = sTime->tm_mon + 1;
	pDateAndTime->year = sTime->tm_year + 1900;
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

/* MESSAGE QUEUE IMPLEMENTATION - START */
static void _pal_UtilityMessageInit
(
	_EcrioUtilityMessageQStruct *pMessageQStruct
)
{
	(void)pMessageQStruct;
	//TBD
}

static void _pal_UtilityMessageDeinit
(
	_EcrioUtilityMessageQStruct *pMessageQStruct
)
{
	(void)pMessageQStruct;
	//TBD
}

static int _pal_UtilityMessageAdd
(
	_EcrioUtilityMessageQStruct *pMessageQStruct,
	unsigned int uCommand,
	unsigned int uParam,
	void* pParam
)
{
	lock_guard<mutex> lock(pMessageQStruct->m);
	_EcrioUtilityMessageStruct messageStruct = {};
	messageStruct.uCommand = uCommand;
	messageStruct.uParam = uParam;
	messageStruct.pParam = pParam;
	pMessageQStruct->messageQ.push_back(messageStruct);
	return 0;
}

static int _pal_UtilityMessageGet
(
	_EcrioUtilityMessageQStruct *pMessageQStruct,
	_EcrioUtilityMessageStruct *pStruct
)
{
	unique_lock<mutex> lock(pMessageQStruct->m);
	if (pMessageQStruct->messageQ.size() == 0)
	{
		lock.unlock();
		return -1;
	}
	_EcrioUtilityMessageStruct messageStruct = {};
	messageStruct = pMessageQStruct->messageQ.front();
	pStruct->uCommand = messageStruct.uCommand;
	pStruct->uParam = messageStruct.uParam;
	pStruct->pParam = messageStruct.pParam;
	pMessageQStruct->messageQ.pop_front();
	lock.unlock();
	return 0;
}
/* MESSAGE QUEUE IMPLEMENTATION - END */

/* SIGNAL IMPLEMENTATION - START */
static void _pal_UtilitySignalWorker
(
	void *pStruct
)
{
	_EcrioUtilitySignalStruct *h = (_EcrioUtilitySignalStruct*)pStruct;
	while (1)
	{
		unique_lock<mutex> lock(h->m);
		h->cond.wait(lock);
		lock.unlock();
		_EcrioUtilityMessageStruct messageStruct = {};
		while (_pal_UtilityMessageGet(&(h->messageQStruct), &messageStruct) == 0)
		{
			switch (messageStruct.uCommand)
			{
			case _ECRIO_UTILITY_SIGNAL_EXIT:
			{
				return;
			}
			default:
			{
				if ((h->callback != NULL) && (!h->bEnding))
				{
					h->callback(h, messageStruct.uCommand, messageStruct.uParam, messageStruct.pParam);
				}
			}
			break;
			}
		}
	}
}

static _ECRIO_UTILITY_SIGNAL_HANDLE _pal_UtilitySignalInit
(
	_EcrioUtilitySignalCallback pSignalFn,
	LOGHANDLE logHandle,
	u_int32 *pError
)
{
	_EcrioUtilitySignalStruct *h = NULL;
	int returnValue = 0;

	if (pError == NULL)
	{
		return NULL;
	}

	if (pSignalFn == NULL)
	{
		*pError = 1;
		return NULL;
	}

	/* By default, the error value returned will indicate success. */
	*pError = returnValue;

	h = new (_EcrioUtilitySignalStruct);

	h->logHandle = logHandle;
	h->bEnding = Enum_FALSE;

	h->callback = pSignalFn;

	_pal_UtilityMessageInit(&(h->messageQStruct));

	h->hThread = thread(_pal_UtilitySignalWorker, (void *)h);
	return h;
}

static int _pal_UtilitySignalDeinit
(
	_ECRIO_UTILITY_SIGNAL_HANDLE *handle
)
{
	_EcrioUtilitySignalStruct *h = NULL;
	if (handle == NULL)
	{
		return -1;
	}
	if (*handle == NULL)
	{
		return -1;
	}
	h = (_EcrioUtilitySignalStruct*)*handle;
	h->bEnding = Enum_TRUE;
	_pal_UtilityMessageAdd(&(h->messageQStruct), _ECRIO_UTILITY_SIGNAL_EXIT, 0, NULL);
	unique_lock<mutex> lock(h->m);
	h->cond.notify_one();
	lock.unlock();
	h->hThread.join();
	_pal_UtilityMessageDeinit(&(h->messageQStruct));
	delete h;
	return 0;
}

static int _pal_UtilitySignalSend
(
	_ECRIO_UTILITY_SIGNAL_HANDLE handle,
	unsigned int uCommand,
	unsigned int uParam,
	void* pParam
)
{
	_EcrioUtilitySignalStruct *h = NULL;
	if (handle == NULL)
	{
		return -1;
	}
	h = (_EcrioUtilitySignalStruct*)handle;
	_pal_UtilityMessageAdd(&(h->messageQStruct), uCommand, uParam, pParam);
	unique_lock<mutex> lock(h->m);
	h->cond.notify_one();
	lock.unlock();
	return 0;
}
/* SIGNAL QUEUE IMPLEMENTATION - END */

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

	dns->id = (unsigned short) htons(getpid());
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

static u_int32 _pal_UtilityLookupHost(_EcrioUtilityDnsDetails *pDnsDetails)
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

static unsigned int _pal_UtilityFQDNHandler
(
	_ECRIO_UTILITY_SIGNAL_HANDLE handle,
	unsigned int uCommand,
	unsigned int uParam,
	void *pParam
)
{
	_EcrioUtilityDnsFqdnInfo *i = (_EcrioUtilityDnsFqdnInfo *)pParam;
	unsigned int error = KPALErrorNone;
	thread hThread;

	if (i == NULL)
	{
		return KPALInvalidHandle;
	}

	if(uCommand == _ECRIO_UTILITY_SIGNAL_NOTIFY)
	{
		if (i->pCallback != NULL)
		{
			i->pCallback(i->pCallbackFnData, i->tlsDnsDetails.addrCount, i->tlsDnsDetails.pIpAddrStructList, i->nonTlsDnsDetails.addrCount, i->nonTlsDnsDetails.pIpAddrStructList);
		}
	}
	else
	{
		if (i->pCallback != NULL)
		{
			i->pCallback(i->pCallbackFnData, 0, NULL, 0, NULL);
		}
	}

	if (i->tlsDnsDetails.addrCount > 0 && i->tlsDnsDetails.pIpAddrStructList != NULL)
	{
		for (u_int32 j=0; j<i->tlsDnsDetails.addrCount; j++)
		{
			pal_MemoryFree((void **)&i->tlsDnsDetails.pIpAddrStructList[j].pIpAddr);
		}
		pal_MemoryFree((void **)&i->tlsDnsDetails.pIpAddrStructList);
	}

	if (i->nonTlsDnsDetails.addrCount > 0 && i->nonTlsDnsDetails.pIpAddrStructList != NULL)
	{
		for (u_int32 j=0; j<i->nonTlsDnsDetails.addrCount; j++)
		{
			pal_MemoryFree((void **)&i->nonTlsDnsDetails.pIpAddrStructList[j].pIpAddr);
		}
		pal_MemoryFree((void **)&i->nonTlsDnsDetails.pIpAddrStructList);
	}

	pal_MutexGlobalLock(i->pal);
	pal_SocketClose(&i->handle);
	pal_MutexGlobalUnlock(i->pal);

	return KPALErrorNone;
}

static void _pal_UtilityDnsSocketErrorCB
(
	SOCKETHANDLE socket,
	void *pCallbackFnData,
	u_int32 error
)
{
	_EcrioUtilityDnsFqdnInfo *pFqdnInfo = (_EcrioUtilityDnsFqdnInfo *)pCallbackFnData;
	s_int32  i = 0;

	if (pFqdnInfo->pCallback != NULL)
	{
		pFqdnInfo->pCallback(pFqdnInfo->pCallbackFnData, 0, NULL, 0, NULL);
	}

	if (pFqdnInfo->tlsDnsDetails.pNaptrAddr != NULL)
	{
		pal_MemoryFree((void **)&pFqdnInfo->tlsDnsDetails.pNaptrAddr);
	}

	if (pFqdnInfo->tlsDnsDetails.pSrvAddr != NULL)
	{
		pal_MemoryFree((void **)&pFqdnInfo->tlsDnsDetails.pSrvAddr);
	}

	if (pFqdnInfo->nonTlsDnsDetails.pNaptrAddr != NULL)
	{
		pal_MemoryFree((void **)&pFqdnInfo->nonTlsDnsDetails.pNaptrAddr);
	}

	if (pFqdnInfo->nonTlsDnsDetails.pSrvAddr != NULL)
	{
		pal_MemoryFree((void **)&pFqdnInfo->nonTlsDnsDetails.pSrvAddr);
	}

	_pal_UtilitySignalSend(pFqdnInfo->hSignal, _ECRIO_UTILITY_SIGNAL_ERROR, KPALInternalError, (void *)pFqdnInfo);
}

static void _pal_UtilityDnsSocketReceieveCB
(
	void *pCallbackFnData,
	SocketReceiveStruct *pReceive
)
{
	_EcrioUtilityDnsFqdnInfo *pFqdnInfo = (_EcrioUtilityDnsFqdnInfo *)pCallbackFnData;
	u_int32 uError = KPALErrorNone;
	unsigned char dnsPayload[_ECRIO_UTILITY_DNS_UDP_PAYLOAD_MAX] = {0};
	s_int32  i = 0, stop = 0;
	unsigned char *reader = NULL;
	struct _EcrioUtilityDnsHederStruct *dns = NULL;
	struct _EcrioUtilityDnsResRecordStruct answers[20];

	if (pReceive->result != KPALErrorNone || pReceive->receiveLength <= 0 || pReceive->pReceiveData == NULL)
	{
		goto Done;
	}

	dns = (struct _EcrioUtilityDnsHederStruct*)pReceive->pReceiveData;

	//move ahead of the dns header and the query field
	reader = &pReceive->pReceiveData[pFqdnInfo->uDnsPayloadLen - _ECRIO_UTILITY_DNS_EDNS_SIZE];
	stop=0;
 
	switch (pFqdnInfo->state)
	{
		case ECRIO_UTILITY_DNS_FQDN_States_Naptr_Res :
		{
 			for(i=0;i<ntohs(dns->ans_count);i++)
			{
				u_int8 serviceLen = 0; 
				u_char *service = NULL;

				answers[i].pName = NULL;
				answers[i].pName=_pal_UtilityDnsReadName(reader, pReceive->pReceiveData, &stop);
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
						pFqdnInfo->nonTlsDnsDetails.pNaptrAddr = _pal_UtilityDnsReadName(reader, pReceive->pReceiveData, &stop);
						if (pal_StringFindSubString(pFqdnInfo->nonTlsDnsDetails.pNaptrAddr, (const u_char*)_ECRIO_UTILITY_DNS_PROTO_TYPE_STR_UDP) != NULL)
						{
							pFqdnInfo->nonTlsDnsDetails.type = ECRIO_UTILITY_DNS_Protocol_Type_Udp;
						}
						else if (pal_StringFindSubString(pFqdnInfo->nonTlsDnsDetails.pNaptrAddr, (const u_char*)_ECRIO_UTILITY_DNS_PROTO_TYPE_STR_TCP) != NULL)
						{
							pFqdnInfo->nonTlsDnsDetails.type = ECRIO_UTILITY_DNS_Protocol_Type_Tcp;
						}
						reader = reader + stop;
 					}
					else if (pal_StringNCompare(service, (const u_char*)_ECRIO_UTILITY_DNS_NAPTR_SERVICE_SIPS, pal_StringLength((const u_char*)_ECRIO_UTILITY_DNS_NAPTR_SERVICE_SIPS)) == 0)
					{
 						reader += (serviceLen + 1);
						pFqdnInfo->tlsDnsDetails.pNaptrAddr = _pal_UtilityDnsReadName(reader, pReceive->pReceiveData, &stop);
						if (pal_StringFindSubString(pFqdnInfo->tlsDnsDetails.pNaptrAddr, (const u_char*)_ECRIO_UTILITY_DNS_PROTO_TYPE_STR_UDP) != NULL)
						{
							pFqdnInfo->tlsDnsDetails.type = ECRIO_UTILITY_DNS_Protocol_Type_Udp;
						}
						else if (pal_StringFindSubString(pFqdnInfo->tlsDnsDetails.pNaptrAddr, (const u_char*)_ECRIO_UTILITY_DNS_PROTO_TYPE_STR_TCP) != NULL)
						{
							pFqdnInfo->tlsDnsDetails.type = ECRIO_UTILITY_DNS_Protocol_Type_Tcp;
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
					}

					if (service != NULL)
					{
						pal_MemoryFree((void **)&service);
					}
				}
				else
				{
 					answers[i].pRData = _pal_UtilityDnsReadName(reader, pReceive->pReceiveData, &stop);
					reader = reader + stop;
					pal_MemoryFree((void **)&answers[i].pRData);
				}

				if (answers[i].pName != NULL)
				{
					pal_MemoryFree((void **)&answers[i].pName);
				}
			}

			pFqdnInfo->state = ECRIO_UTILITY_DNS_FQDN_States_Srv_Tls_Req;

			if (pFqdnInfo->tlsDnsDetails.pNaptrAddr == NULL)
			{
				goto Done;
			}

			pFqdnInfo->uDnsPayloadLen = 0;
			pal_MemorySet(&dnsPayload, 0, _ECRIO_UTILITY_DNS_UDP_PAYLOAD_MAX);
			_pal_UtilityDnsCreateQuery((u_char *)&dnsPayload, &pFqdnInfo->uDnsPayloadLen, (char *)pFqdnInfo->tlsDnsDetails.pNaptrAddr, _ECRIO_UTILITY_DNS_TYPE_SRV); // creates a DNS query for the SRV type.

			uError = pal_SocketSendData(pFqdnInfo->handle, dnsPayload, pFqdnInfo->uDnsPayloadLen);
			if (uError != KPALErrorNone)
			{
				goto Done;
			}

			pFqdnInfo->state = ECRIO_UTILITY_DNS_FQDN_States_Srv_Tls_Res;
		}
		break;

		case ECRIO_UTILITY_DNS_FQDN_States_Srv_Tls_Res :
		{
			for(i=0;i<ntohs(dns->ans_count);i++)
			{
				unsigned short port = 0; 

				answers[i].pName = NULL;
				answers[i].pName=_pal_UtilityDnsReadName(reader, pReceive->pReceiveData, &stop);
				reader = reader + stop;

				answers[i].pResource = (struct _EcrioUtilityDnsRDataStruct*)(reader);
				reader = reader + sizeof(struct _EcrioUtilityDnsRDataStruct);

				if(ntohs(answers[i].pResource->type) == _ECRIO_UTILITY_DNS_TYPE_SRV) //if its an ipv4 address
				{
					reader += _ECRIO_UTILITY_DNS_SRV_PORT_INDEX;
					port = _ECRIO_UTILITY_DNS_16BIT(reader);
					pFqdnInfo->tlsDnsDetails.uPort = port;
					reader += sizeof(unsigned short);

					pFqdnInfo->tlsDnsDetails.pSrvAddr = _pal_UtilityDnsReadName(reader, pReceive->pReceiveData, &stop);
					reader = reader + stop;
				}
				else
				{
					answers[i].pRData = _pal_UtilityDnsReadName(reader, pReceive->pReceiveData, &stop);
					reader = reader + stop;
					pal_MemoryFree((void **)&answers[i].pRData);
				}

				if (answers[i].pName != NULL)
				{
					pal_MemoryFree((void **)&answers[i].pName);
				}
			}

			pFqdnInfo->state = ECRIO_UTILITY_DNS_FQDN_States_Srv_Nontls_Req;

			if (pFqdnInfo->nonTlsDnsDetails.pNaptrAddr == NULL)
			{
				goto Done;
			}

			pFqdnInfo->uDnsPayloadLen = 0;
			pal_MemorySet(&dnsPayload, 0, _ECRIO_UTILITY_DNS_UDP_PAYLOAD_MAX);
			_pal_UtilityDnsCreateQuery((u_char *)&dnsPayload, &pFqdnInfo->uDnsPayloadLen, (char *)pFqdnInfo->nonTlsDnsDetails.pNaptrAddr, _ECRIO_UTILITY_DNS_TYPE_SRV); // creates a DNS query for the SRV type.

			uError = pal_SocketSendData(pFqdnInfo->handle, dnsPayload, pFqdnInfo->uDnsPayloadLen);
			if (uError != KPALErrorNone)
			{
				goto Done;
			}

			pFqdnInfo->state = ECRIO_UTILITY_DNS_FQDN_States_Srv_Nontls_Res;
		}
		break;

		case ECRIO_UTILITY_DNS_FQDN_States_Srv_Nontls_Res :
		{
			for(i=0;i<ntohs(dns->ans_count);i++)
			{
				unsigned short port = 0; 

				answers[i].pName = NULL;
				answers[i].pName=_pal_UtilityDnsReadName(reader, pReceive->pReceiveData, &stop);
				reader = reader + stop;

				answers[i].pResource = (struct _EcrioUtilityDnsRDataStruct*)(reader);
				reader = reader + sizeof(struct _EcrioUtilityDnsRDataStruct);
				if(ntohs(answers[i].pResource->type) == _ECRIO_UTILITY_DNS_TYPE_SRV) //if its an ipv4 address
				{
					reader += _ECRIO_UTILITY_DNS_SRV_PORT_INDEX;
					port = _ECRIO_UTILITY_DNS_16BIT(reader);
					pFqdnInfo->nonTlsDnsDetails.uPort = port;
					reader += sizeof(unsigned short);

					pFqdnInfo->nonTlsDnsDetails.pSrvAddr = _pal_UtilityDnsReadName(reader, pReceive->pReceiveData, &stop);
					reader = reader + stop;
				}
				else
				{
					answers[i].pRData = _pal_UtilityDnsReadName(reader, pReceive->pReceiveData, &stop);
					reader = reader + stop;
					pal_MemoryFree((void **)&answers[i].pRData);
				}

				if (answers[i].pName != NULL)
				{
					pal_MemoryFree((void **)&answers[i].pName);
				}
			}

			pFqdnInfo->state = ECRIO_UTILITY_DNS_FQDN_States_Tls_Host;

			if (pFqdnInfo->tlsDnsDetails.pSrvAddr == NULL)
			{
				goto Done;
			}

			uError = _pal_UtilityLookupHost(&pFqdnInfo->tlsDnsDetails);
			if (uError != KPALErrorNone)
			{
				goto Done;
			}

			pFqdnInfo->state = ECRIO_UTILITY_DNS_FQDN_States_Nontls_Host;

			if (pFqdnInfo->nonTlsDnsDetails.pSrvAddr == NULL)
			{
				goto Done;
			}

			uError = _pal_UtilityLookupHost(&pFqdnInfo->nonTlsDnsDetails);
			if (uError != KPALErrorNone)
			{
				goto Done;
			}

			pFqdnInfo->state = ECRIO_UTILITY_DNS_FQDN_States_Done;
			_pal_UtilitySignalSend(pFqdnInfo->hSignal, _ECRIO_UTILITY_SIGNAL_NOTIFY, KPALErrorNone, (void *)pFqdnInfo);
			goto Done;
		}
		break;

		default:
			goto Done;
	}

	return;

Done:
	if (pFqdnInfo->tlsDnsDetails.pNaptrAddr != NULL)
	{
		pal_MemoryFree((void **)&pFqdnInfo->tlsDnsDetails.pNaptrAddr);
	}

	if (pFqdnInfo->tlsDnsDetails.pSrvAddr != NULL)
	{
		pal_MemoryFree((void **)&pFqdnInfo->tlsDnsDetails.pSrvAddr);
	}

	if (pFqdnInfo->nonTlsDnsDetails.pNaptrAddr != NULL)
	{
		pal_MemoryFree((void **)&pFqdnInfo->nonTlsDnsDetails.pNaptrAddr);
	}

	if (pFqdnInfo->nonTlsDnsDetails.pSrvAddr != NULL)
	{
		pal_MemoryFree((void **)&pFqdnInfo->nonTlsDnsDetails.pSrvAddr);
	}
}

u_int32 pal_UtilityStartQueryFQDN
(
	PALINSTANCE pal,
	char *pHostName,
	char *pLocalIP,
	char *pServerIP,
	EcrioUtilityDnsAnswerCallbackStruct *pFnStruct,
	FQDNHANDLE *handle
)
{
	u_int32 uError = KPALErrorNone;
	_EcrioUtilityDnsFqdnInfo *pFqdnInfo = NULL;
	SocketCallbackFnStruct fnStruct;
	unsigned char dnsPayload[_ECRIO_UTILITY_DNS_UDP_PAYLOAD_MAX] = {0};
	u_int32 uDnsPayloadLen = 0;
	SocketCreateConfigStruct socketCreateConfig = { .protocolVersion = ProtocolVersion_IPv4, .protocolType = ProtocolType_UDP, .socketType = SocketType_Default, .socketMediaType = SocketMediaType_Default };
	SocketTLSSettingsStruct sspl = { 0 };
	SOCKETHANDLE sock;

	if (pHostName == NULL || pServerIP == NULL || pLocalIP == NULL)
	{
		return KPALInvalidParameters;
	}

	if (pFnStruct == NULL || pFnStruct->pCallback == NULL)
	{
		return KPALInvalidParameters;
	}

	*handle = NULL;

	pal_MemoryAllocate(sizeof(_EcrioUtilityDnsFqdnInfo), (void **)&pFqdnInfo);
	if (pFqdnInfo == NULL)
	{
		return KPALMemoryError;
	}

	pFqdnInfo->pal = pal;
	pFqdnInfo->handle = NULL;

	pFqdnInfo->pCallbackFnData = pFnStruct->pCallbackFnData;
	pFqdnInfo->pCallback = pFnStruct->pCallback;

	pFqdnInfo->state = ECRIO_UTILITY_DNS_FQDN_States_None;

	pFqdnInfo->hSignal = _pal_UtilitySignalInit(_pal_UtilityFQDNHandler, NULL, &uError);
	if (pFqdnInfo->hSignal == NULL)
	{
		goto Error;
	}

	fnStruct.pAcceptCallbackFn = NULL;
	fnStruct.pErrorCallbackFn = _pal_UtilityDnsSocketErrorCB;
	fnStruct.pReceiveCallbackFn = _pal_UtilityDnsSocketReceieveCB;
	fnStruct.pCallbackFnData = (void *)pFqdnInfo;

	if (strstr(pServerIP, ":") != NULL)
	{
		socketCreateConfig.protocolVersion = ProtocolVersion_IPv6;
	}
	else
	{
		socketCreateConfig.protocolVersion = ProtocolVersion_IPv4;
	}

	socketCreateConfig.protocolType = ProtocolType_UDP;
	socketCreateConfig.socketType = SocketType_Default;
	socketCreateConfig.socketMediaType = SocketMediaType_Default;
	socketCreateConfig.bufferSize = 4096;
	socketCreateConfig.bEnableGlobalMutex = Enum_TRUE;

	sspl.bVerifyPeer = Enum_FALSE;
	sspl.pCert = NULL;
	sspl.socketTLSVersion = SocketTLSVersion_Default;
	socketCreateConfig.tlsSetting = &sspl;

	uError = pal_SocketCreate(pal, &socketCreateConfig, &fnStruct, &sock);
	if (uError != KPALErrorNone)
	{
		goto Error;
	}

	uError = pal_SocketSetLocalHost(sock, (const u_char *)pLocalIP, 0);
	if (uError != KPALErrorNone)
	{
		goto Error;
	}

	uError = pal_SocketSetRemoteHost(sock, (const u_char *)pServerIP, (u_int16)_ECRIO_UTILITY_DNS_SERVER_PORT);
	if (uError != KPALErrorNone)
	{
		goto Error;
	}

	uError = pal_SocketOpen(sock);
	if (uError != KPALErrorNone)
	{
		goto Error;
	}

	pFqdnInfo->state = ECRIO_UTILITY_DNS_FQDN_States_Naptr_Req;

	pal_MemorySet(&dnsPayload, 0, _ECRIO_UTILITY_DNS_UDP_PAYLOAD_MAX);
	_pal_UtilityDnsCreateQuery((u_char *)&dnsPayload, &uDnsPayloadLen, pHostName, _ECRIO_UTILITY_DNS_TYPE_NAPTR); // creates a DNS query for the NAPTR type.
	pFqdnInfo->uDnsPayloadLen = uDnsPayloadLen;

	uError = pal_SocketSendData(sock, dnsPayload, uDnsPayloadLen);
	if (uError != KPALErrorNone)
	{
		goto Error;
	}

	pFqdnInfo->state = ECRIO_UTILITY_DNS_FQDN_States_Naptr_Res;
	pFqdnInfo->handle = sock;

	*handle = (FQDNHANDLE)pFqdnInfo;

	return uError;

Error:
	if (pFqdnInfo->hSignal != NULL)
	{
		_pal_UtilitySignalDeinit(&pFqdnInfo->hSignal);
	}
	
	if (pFqdnInfo->handle != NULL)
	{
		pal_SocketClose(&sock);
	}

	pal_MemoryFree((void **)&pFqdnInfo);

	return uError;
}

u_int32 pal_UtilityStopQueryFQDN
(
	FQDNHANDLE *handle
)
{
	u_int32 uError = KPALErrorNone;
	_EcrioUtilityDnsFqdnInfo *pFqdnInfo = (_EcrioUtilityDnsFqdnInfo *)*handle;

	_pal_UtilitySignalDeinit(&pFqdnInfo->hSignal);
	pal_MemoryFree((void **)&pFqdnInfo);

	return uError;
}

u_int32 gTlsAddrCount;
EcrioUtilityDnsAddrStruct *gTlsDnsAddrList;
u_int32 gNonTlsAddrCount;
EcrioUtilityDnsAddrStruct *gNonTlsDnsAddrList;

mutex gFqdnM;
condition_variable gFqdnCond;

static void QueryDnsGetInfoFromFqdnCallback
(
	void *pCallbackFnData,
	u_int32 uTlsAddrCount,
	EcrioUtilityDnsAddrStruct *pTlsDnsAddrList,
	u_int32 uNonTlsAddrCount,
	EcrioUtilityDnsAddrStruct *pNonTlsDnsAddrList
)
{
	if (uTlsAddrCount > 0)
	{
		gTlsAddrCount = uTlsAddrCount;
		pal_MemoryAllocate((sizeof(EcrioUtilityDnsAddrStruct) * gTlsAddrCount),(void**)&gTlsDnsAddrList);
		for (u_int32 i=0; i<gTlsAddrCount; i++)
		{
			gTlsDnsAddrList[i].pIpAddr = pal_StringCreate(pTlsDnsAddrList[i].pIpAddr, pal_StringLength((const u_char *)pTlsDnsAddrList[i].pIpAddr));
			gTlsDnsAddrList[i].type = pTlsDnsAddrList[i].type;
			gTlsDnsAddrList[i].uPort = pTlsDnsAddrList[i].uPort;
			gTlsDnsAddrList[i].version = pTlsDnsAddrList[i].version;
		}
	}
	else
	{
		uTlsAddrCount = 0;
		gTlsDnsAddrList = NULL;
	}

	if (uNonTlsAddrCount > 0)
	{
		gNonTlsAddrCount = uNonTlsAddrCount;
		pal_MemoryAllocate((sizeof(EcrioUtilityDnsAddrStruct) * gNonTlsAddrCount),(void**)&gNonTlsDnsAddrList);
		for (u_int32 i=0; i<gNonTlsAddrCount; i++)
		{
			gNonTlsDnsAddrList[i].pIpAddr = pal_StringCreate(pNonTlsDnsAddrList[i].pIpAddr, pal_StringLength((const u_char *)pNonTlsDnsAddrList[i].pIpAddr));
			gNonTlsDnsAddrList[i].type = pNonTlsDnsAddrList[i].type;
			gNonTlsDnsAddrList[i].uPort = pNonTlsDnsAddrList[i].uPort;
			gNonTlsDnsAddrList[i].version = pNonTlsDnsAddrList[i].version;
		}
	}
	else
	{
		gNonTlsAddrCount = 0;
		gNonTlsDnsAddrList = NULL;
	}

	unique_lock<mutex> lock(gFqdnM);
	gFqdnCond.notify_one();
	lock.unlock();
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
	u_int32 uPalError = KPALErrorNone;
	FQDNHANDLE handle;
	EcrioUtilityDnsAnswerCallbackStruct fnStruct;

	gTlsAddrCount = 0;
	gNonTlsAddrCount = 0;
	gTlsDnsAddrList = NULL;
	gNonTlsDnsAddrList = NULL;

	fnStruct.pCallback = QueryDnsGetInfoFromFqdnCallback;
	fnStruct.pCallbackFnData = NULL;

	pal_MutexGlobalLock(pal);
	uPalError = pal_UtilityStartQueryFQDN(pal, (char *)pHostName, (char *)pLocal, (char *)pDnsAddress, &fnStruct, &handle);
	if (uPalError != KPALErrorNone)
	{
		*pTlsAddrCount = 0;
		*pNonTlsAddrCount = 0;
		*ppTlsDnsAddrList = NULL;
		*ppNonTlsDnsAddrList = NULL;
		pal_MutexGlobalUnlock(pal);
		return uPalError;
	}
	pal_MutexGlobalUnlock(pal);

	unique_lock<mutex> lock(gFqdnM);
	gFqdnCond.wait(lock);
	lock.unlock();
	
	pal_UtilityStopQueryFQDN(&handle);

	*pTlsAddrCount = gTlsAddrCount;
	*pNonTlsAddrCount = gNonTlsAddrCount;
	*ppTlsDnsAddrList = gTlsDnsAddrList;
	*ppNonTlsDnsAddrList = gNonTlsDnsAddrList;

	return uPalError;
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
